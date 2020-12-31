/*
** nester - NES emulator
** Copyright (C) 2000  Darren Ranalli
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
** Library General Public License for more details.  To obtain a
** copy of the GNU Library General Public License, write to the Free
** Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
**
** Any permitted reproduction of these routines, in whole or in part,
** must bear this legend.
*/

/*
	v1.02 (g_PPU.LowRegs[1] & 0x01) の状態を事前にチェックし高速化 by ruka
	      BG描画にキャッシュ機構導入し高速化 by ruka
	v1.03 WRITEフラグをビット管理に変更、タイルキャッシュ、他 by ruka

*/


#include "nes.h"
#include "nes_ppu.h"
#include "nes_config.h"
#include "cstring.h"
#include "debug.h"


static void NES_PPU_render_bg_internal1(PIXEL_FORMAT* buf, u32 CYCLES_PER_DRAW);
static void NES_PPU_render_bg_internal2(PIXEL_FORMAT* buf, u32 CYCLES_PER_DRAW);
static void NES_PPU_render_spr_internal1(PIXEL_FORMAT* buf);
static void NES_PPU_render_spr_internal2(PIXEL_FORMAT* buf);


NES_PPU g_PPU;
//#define ONYONIPUU		// Test

extern PIXEL_FORMAT g_Pal[256];

#ifdef ONYONIPUU

extern DWORD dwAdd_OnyoniPuu;
extern unsigned char ucDat_OnyoniPuu;


#define VRAM_R(addr) \
  (g_PPU.PPU_VRAM_banks[((addr | dwAdd_OnyoniPuu) >> 10 )][(addr | dwAdd_OnyoniPuu) & 0x3FF] | g_PPU.ucDat_OnyoniPuu )

#define VRAM_W(addr, data) \
  g_PPU.PPU_VRAM_banks[((addr | dwAdd_OnyoniPuu) >> 10 )][(addr | dwAdd_OnyoniPuu) & 0x3FF] = data | g_PPU.ucDat_OnyoniPuu

#else

#define VRAM_R(addr) \
  g_PPU.PPU_VRAM_banks[((addr) >> 10 )][(addr) & 0x3FF]

#define VRAM_W(addr, data) \
  g_PPU.PPU_VRAM_banks[((addr) >> 10 )][(addr) & 0x3FF] = data

#endif // ONYONIPUU


//  PPU_VRAM_banks[(addr) >> 10][(addr) & 0x3FF]

/*
scanline start (if background or sprites are enabled):
	v:0000010000011111=t:0000010000011111
*/
#define LOOPY_SCANLINE_START(v,t) \
  { \
    v = (v & 0xFBE0) | (t & 0x041F); \
  }

/*
bits 12-14 are the tile Y offset.
you can think of bits 5,6,7,8,9 as the "y scroll"(*8).  this functions
slightly different from the X.  it wraps to 0 and bit 11 is switched when
it's incremented from _29_ instead of 31.  there are some odd side effects
from this.. if you manually set the value above 29 (from either 2005 or
2006), the wrapping from 29 obviously won't happen, and attrib data will be
used as name table data.  the "y scroll" still wraps to 0 from 31, but
without switching bit 11.  this explains why writing 240+ to 'Y' in 2005
appeared as a negative scroll value.
*/
#define LOOPY_NEXT_LINE(v) \
  { \
    if((v & 0x7000) == 0x7000) /* is subtile y offset == 7? */ \
    { \
      v &= 0x8FFF; /* subtile y offset = 0 */ \
      if((v & 0x03E0) == 0x03A0) /* name_tab line == 29? */ \
      { \
        v ^= 0x0800;  /* switch nametables (bit 11) */ \
        v &= 0xFC1F;  /* name_tab line = 0 */ \
      } \
      else \
      { \
        if((v & 0x03E0) == 0x03E0) /* line == 31? */ \
        { \
          v &= 0xFC1F;  /* name_tab line = 0 */ \
        } \
        else \
        { \
          v += 0x0020; \
        } \
      } \
    } \
    else \
    { \
      v += 0x1000; /* next subtile y offset */ \
    } \
  }

/*
you can think of bits 0,1,2,3,4 of the vram address as the "x scroll"(*8)
that the ppu increments as it draws.  as it wraps from 31 to 0, bit 10 is
switched.  you should see how this causes horizontal wrapping between name
tables (0,1) and (2,3).
*/
#define LOOPY_NEXT_TILE(v) \
  { \
    if((v & 0x001F) == 0x001F) \
    { \
      v ^= 0x0400; /* switch nametables (bit 10) */ \
      v &= 0xFFE0; /* tile x = 0 */ \
    } \
    else \
    { \
      v++; /* next tile */ \
    } \
  }

#define LOOPY_NEXT_PIXEL(v,x) \
  { \
    if(x == 0x07) \
    { \
      LOOPY_NEXT_TILE(v); \
      x = 0x00; \
    } \
    else \
    { \
      x++; \
    } \
  }

#define CHECK_MMC2(addr) \
  if(((addr) & 0x0FC0) == 0x0FC0) \
  { \
    if((((addr) & 0x0FF0) == 0x0FD0) || (((addr) & 0x0FF0) == 0x0FE0)) \
    { \
      g_NESmapper.PPU_Latch_FDFE(addr); \
    } \
  }

int NES_PPU_getTopMargin(unsigned char c)
{
	return (g_NESConfig.graphics.show_all_scanlines||c==2) ? 0 : 8;
}

int NES_PPU_getViewableHeight(unsigned char c)
{
	return NES_SCREEN_HEIGHT-(2*NES_PPU_getTopMargin(c));
}

/*
static u8 NES_PPU_getBGColor()
{
	return NES_COLOR_BASE + g_PPU.bg_pal[0];
}
*/

void NES_PPU_reset()
{
//    static u8 bCreateBit2RevTable = FALSE;
    // reset registers
    core_memset(g_PPU.LowRegs, 0x00, sizeof(g_PPU.LowRegs));
    g_PPU.HighReg0x4014 = 0x00;

    // clear sprite RAM
    core_memset(g_PPU.spr_ram, 0x00, sizeof(g_PPU.spr_ram));

    // clear palettes
    core_memset(g_PPU.bg_pal,  0x00, sizeof(g_PPU.bg_pal));
    core_memset(g_PPU.spr_pal, 0x00, sizeof(g_PPU.spr_pal));

    // clear pattern tables
    core_memset(g_PPU.PPU_patterntables, 0x00, sizeof(g_PPU.PPU_patterntables));
    core_memset(g_PPU.PPU_patterntype, 0x00, sizeof(g_PPU.PPU_patterntype));

    // clear internal name tables
    core_memset(g_PPU.PPU_nametables, 0x00, sizeof(g_PPU.PPU_nametables));

    // clear VRAM page table
    core_memset(g_PPU.PPU_VRAM_banks, 0x00, sizeof(g_PPU.PPU_VRAM_banks));

    // set up PPU memory space table
    g_PPU.PPU_VRAM_banks[0x00] = g_PPU.PPU_patterntables + (0*0x400);
    g_PPU.PPU_VRAM_banks[0x01] = g_PPU.PPU_patterntables + (1*0x400);
    g_PPU.PPU_VRAM_banks[0x02] = g_PPU.PPU_patterntables + (2*0x400);
    g_PPU.PPU_VRAM_banks[0x03] = g_PPU.PPU_patterntables + (3*0x400);

    g_PPU.PPU_VRAM_banks[0x04] = g_PPU.PPU_patterntables + (4*0x400);
    g_PPU.PPU_VRAM_banks[0x05] = g_PPU.PPU_patterntables + (5*0x400);
    g_PPU.PPU_VRAM_banks[0x06] = g_PPU.PPU_patterntables + (6*0x400);
    g_PPU.PPU_VRAM_banks[0x07] = g_PPU.PPU_patterntables + (7*0x400);

    // point nametables at internal name table 0
    g_PPU.PPU_VRAM_banks[0x08] = g_PPU.PPU_nametables;
    g_PPU.PPU_VRAM_banks[0x09] = g_PPU.PPU_nametables;
    g_PPU.PPU_VRAM_banks[0x0A] = g_PPU.PPU_nametables;
    g_PPU.PPU_VRAM_banks[0x0B] = g_PPU.PPU_nametables;

    g_PPU.read_2007_buffer = 0x00;
    g_PPU.in_vblank = 0;
    g_PPU.bg_pattern_table_addr = 0;
    g_PPU.spr_pattern_table_addr = 0;
    g_PPU.ppu_addr_inc = 1;
    g_PPU.loopy_v = 0;
    g_PPU.loopy_t = 0;
    g_PPU.loopy_x = 0;
    g_PPU.toggle_2005_2006 = 0;
    g_PPU.spr_ram_rw_ptr = 0;
    g_PPU.read_2007_buffer = 0;
    g_PPU.current_frame_line = 0;
    g_PPU.rgb_bak = 0;

    // default は(g_PPU.LowRegs[1] & 1)がFALSE
    g_PPU.NES_PPU_render_bg = NES_PPU_render_bg_internal2;
    g_PPU.NES_PPU_render_spr = NES_PPU_render_spr_internal2;

    // set mirroring
    NES_PPU_set_mirroring2(NES_ROM_get_mirroring());

    // reset emphasised palette
    NES_ppu_rgb();

    // 左右反転マスクテーブル生成
//    if (!bCreateBit2RevTable)
	{
        int i, j;
        for( i = 0; i < 256; i++ ) {
            u8	m = 0x80;
            u8	c = 0;
            for( j = 0; j < 8; j++ ) {
                if( i&(1<<j) ) {
                    c |= m;
                }
                m >>= 1;
            }
            g_PPU.Bit2Rev[i] = c;
        }
//        bCreateBit2RevTable = TRUE;
    }
}

void NES_PPU_set_mirroring(u32 nt0, u32 nt1, u32 nt2, u32 nt3)
{
    ASSERT(nt0 < 4); ASSERT(nt1 < 4); ASSERT(nt2 < 4); ASSERT(nt3 < 4);
    g_PPU.PPU_VRAM_banks[0x08] = g_PPU.PPU_nametables + (nt0 << 10); // * 0x0400
    g_PPU.PPU_VRAM_banks[0x09] = g_PPU.PPU_nametables + (nt1 << 10);
    g_PPU.PPU_VRAM_banks[0x0A] = g_PPU.PPU_nametables + (nt2 << 10);
    g_PPU.PPU_VRAM_banks[0x0B] = g_PPU.PPU_nametables + (nt3 << 10);
}

void NES_PPU_set_mirroring2(mirroring_type m)
{
    if(NES_PPU_MIRROR_FOUR_SCREEN == m) {   NES_PPU_set_mirroring(0,1,2,3);  }
	else if(NES_PPU_MIRROR_HORIZ == m)  {   NES_PPU_set_mirroring(0,0,1,1);  }
    else if(NES_PPU_MIRROR_VERT == m)   {   NES_PPU_set_mirroring(0,1,0,1);  }
    else {
		LOG("Invalid mirroring type" << endl);
		NES_PPU_set_mirroring2(NES_PPU_MIRROR_FOUR_SCREEN);
	}
}


void NES_PPU_start_frame()
{
    g_PPU.current_frame_line = 0;

    if(NES_PPU_spr_enabled() || NES_PPU_bg_enabled()) {
        g_PPU.loopy_v = g_PPU.loopy_t;
    }
}

void NES_PPU_do_scanline_and_draw(PIXEL_FORMAT* buf, u32 CYCLES_PER_DRAW)
{
    if(!NES_PPU_bg_enabled())  {
        // set to background color
        int i;
        PIXEL_FORMAT bg = g_Pal[ NES_COLOR_BASE + g_PPU.bg_pal[0] ];
        
        for(i=0;i<NES_BACKBUF_WIDTH;i++) {
            buf[i] = bg;
        }
    }

    if(NES_PPU_spr_enabled() || NES_PPU_bg_enabled()) {
        LOOPY_SCANLINE_START(g_PPU.loopy_v, g_PPU.loopy_t);

        if(NES_PPU_bg_enabled()) {
            // draw background
            g_PPU.NES_PPU_render_bg(buf, CYCLES_PER_DRAW);
        }
        else {
            // clear out solid buffer
            core_memset(g_PPU.solid_bits, 0x00, sizeof(g_PPU.solid_bits));
            NES_emulate_CPU_cycles(CYCLES_PER_DRAW);
        }

        if(NES_PPU_spr_enabled()) {
            // draw sprites
            g_PPU.NES_PPU_render_spr(buf);
        }

        LOOPY_NEXT_LINE(g_PPU.loopy_v);
    }

    g_PPU.current_frame_line++;
}

void NES_PPU_do_scanline_and_dont_draw()
{
    // mmc2 / punchout -- we must simulate the ppu for every line
    // なぜか9のパンチアウトのみこの設定になってるようだそのためPSPでは激重
    // になってる お試しのためコメントアウトしておく 2005/6/7 ruka
    /*if(NES_ROM_get_mapper_num() == 9) {
          NES_PPU_do_scanline_and_draw(g_PPU.dummy_buffer, 0);
    }
    else*/
    // if sprite 0 flag not set and sprite 0 on current line
    if((!NES_PPU_sprite0_hit()) &&
       (g_PPU.current_frame_line >= ((u32)(g_PPU.spr_ram[0]+1))) &&
       (g_PPU.current_frame_line <  ((u32)(g_PPU.spr_ram[0]+1+(NES_PPU_sprites_8x16()?16:8))))
       ) {
        // render line to dummy buffer
        NES_PPU_do_scanline_and_draw(g_PPU.dummy_buffer, 0);
    }
    else
      {
          if(NES_PPU_spr_enabled() || NES_PPU_bg_enabled()) {
              LOOPY_SCANLINE_START(g_PPU.loopy_v, g_PPU.loopy_t);
              LOOPY_NEXT_LINE(g_PPU.loopy_v);
          }
          g_PPU.current_frame_line++;
      }
}


// these functions read from/write to VRAM using loopy_v
u8 NES_PPU_read_2007()
{
    u16 addr;
    u8 temp;

    addr = g_PPU.loopy_v;
    g_PPU.loopy_v += g_PPU.ppu_addr_inc;

    ASSERT(addr < 0x4000);
    addr &= 0x3FFF;

    if(addr >= 0x3000) {
        // is it a palette entry?
        if(addr >= 0x3F00) {
            // palette

            // handle palette mirroring
            if(0x0000 == (addr & 0x0010)) {
                // background palette
                return g_PPU.bg_pal[addr & 0x000F];
            }
            else {
                // sprite palette
                return g_PPU.spr_pal[addr & 0x000F];
            }
        }

        // handle mirroring
        addr &= 0xEFFF;
    }

    temp = g_PPU.read_2007_buffer;
    g_PPU.read_2007_buffer = VRAM_R(addr);
    return temp;
}

void NES_PPU_write_2007(u8 data)
{
    u16 addr;

    addr = g_PPU.loopy_v;
    g_PPU.loopy_v += g_PPU.ppu_addr_inc;

    addr &= 0x3FFF;

    if(addr >= 0x3000) {
        // is it a palette entry?
        if(addr >= 0x3F00) {
            // palette
            data &= 0x3F;

            if(0x0000 == (addr & 0x000F)) {// is it THE 0 entry?
                g_PPU.bg_pal[0] = g_PPU.spr_pal[0] = data;
            }
            else if(0x0000 == (addr & 0x0010)) {
                // background palette
                g_PPU.bg_pal[addr & 0x000F] = data;
            }
            else {
                // sprite palette
                g_PPU.spr_pal[addr & 0x000F] = data;
            }
            g_PPU.bg_pal[0x04] = g_PPU.bg_pal[0x08] = g_PPU.bg_pal[0x0C] = g_PPU.bg_pal[0x00];
            g_PPU.spr_pal[0x00] = g_PPU.spr_pal[0x04] = g_PPU.spr_pal[0x08] = g_PPU.spr_pal[0x0C] = g_PPU.bg_pal[0x00];
            return;
        }

        // handle mirroring
        addr &= 0xEFFF;
    }

    if(!(g_PPU.vram_write_protect && addr < 0x2000)) {
        VRAM_W(addr, data);
    }
}

u8 NES_PPU_ReadLowRegs(u32 addr)
{
    ASSERT((addr >= 0x2000) && (addr < 0x2008));

    //  LOG("PPU Read " << HEX(addr,4) << endl);

    switch(addr)
      {
        case 0x2002:
          {
              u8 temp;

              // clear toggle
              g_PPU.toggle_2005_2006 = 0;

              temp = g_PPU.LowRegs[2];

              // clear v-blank flag
              g_PPU.LowRegs[2] &= 0x7F;

              return temp;
          }
          break;

        case 0x2007:
          return NES_PPU_read_2007();
          break;

      }

    return g_PPU.LowRegs[addr & 0x0007];
}

void  NES_PPU_WriteLowRegs(u32 addr, u8 data)
{
    ASSERT((addr >= 0x2000) && (addr < 0x2008));

    //  LOG("PPU Write " << HEX(addr,4) << " = " << HEX(data,2) << endl);

    g_PPU.LowRegs[addr & 0x0007] = data;

    switch(addr)
      {
        case 0x2000:
          g_PPU.bg_pattern_table_addr  = (data & 0x10) ? 0x1000 : 0x0000;
          g_PPU.spr_pattern_table_addr = (data & 0x08) ? 0x1000 : 0x0000;
          g_PPU.ppu_addr_inc = (data & 0x04) ? 32 : 1;

          // t:0000110000000000=d:00000011
          g_PPU.loopy_t = (g_PPU.loopy_t & 0xF3FF) | (((u16)(data & 0x03)) << 10);
          break;

        case 0x2001:
          if (g_PPU.rgb_bak != (data & 0xE0)) NES_ppu_rgb();
          g_PPU.rgb_bak = data & 0xE0;
          if (g_PPU.LowRegs[1] & 1) {
              g_PPU.NES_PPU_render_bg = NES_PPU_render_bg_internal1;
              g_PPU.NES_PPU_render_spr = NES_PPU_render_spr_internal1;
          }
          else {
              g_PPU.NES_PPU_render_bg = NES_PPU_render_bg_internal2;
              g_PPU.NES_PPU_render_spr = NES_PPU_render_spr_internal2;
          }
          break;

        case 0x2003:
          g_PPU.spr_ram_rw_ptr = data;
          break;

        case 0x2004:
          g_PPU.spr_ram[g_PPU.spr_ram_rw_ptr++] = data;
          break;

        case 0x2005:
          g_PPU.toggle_2005_2006 = !g_PPU.toggle_2005_2006;

          if(g_PPU.toggle_2005_2006)
            {
                // first write

                // t:0000000000011111=d:11111000
                g_PPU.loopy_t = (g_PPU.loopy_t & 0xFFE0) | (((u16)(data & 0xF8)) >> 3);

                // x=d:00000111
                g_PPU.loopy_x = data & 0x07;
            }
          else
            {
                // second write

                // t:0000001111100000=d:11111000
                g_PPU.loopy_t = (g_PPU.loopy_t & 0xFC1F) | (((u16)(data & 0xF8)) << 2);

                // t:0111000000000000=d:00000111
                g_PPU.loopy_t = (g_PPU.loopy_t & 0x8FFF) | (((u16)(data & 0x07)) << 12);
            }
          break;

        case 0x2006:
          g_PPU.toggle_2005_2006 = !g_PPU.toggle_2005_2006;

          if(g_PPU.toggle_2005_2006)
            {
                // first write

                // t:0011111100000000=d:00111111
                // t:1100000000000000=0
                g_PPU.loopy_t = (g_PPU.loopy_t & 0x00FF) | (((u16)(data & 0x3F)) << 8);
            }
          else
            {
                // second write

                // t:0000000011111111=d:11111111
                g_PPU.loopy_t = (g_PPU.loopy_t & 0xFF00) | ((u16)data);

                // v=t
                g_PPU.loopy_v = g_PPU.loopy_t;

                // for mapper 96
                g_NESmapper.PPU_Latch_Address(g_PPU.loopy_v);
            }
          break;

        case 0x2007:
          NES_PPU_write_2007(data);
          break;
      }
}

void NES_PPU_Write0x4014(u8 data)
{
    u32 addr;
    u32 i;

    //  LOG("PPU Write 0x4014 = " << HEX(data,2) << endl);

    g_PPU.HighReg0x4014 = data;

    addr = ((u32)data) << 8;

    // do SPR-RAM DMA
    for(i = 0; i < 256; i++)
      {
          g_PPU.spr_ram[i] = NES6502_GetByte(addr++);
      }
}

// (g_PPU.LowRegs[1] & 1)がTRUEの場合こっち
void NES_PPU_render_bg_internal1(PIXEL_FORMAT* buf, u32 CYCLES_PER_DRAW)
{
    PIXEL_FORMAT *p;
    u32 i;

    u8 *solid_bits;

    u32 tile_x, tile_y; // tile_x pixel coords within nametable
    u32 name_addr;

    u32 pattern_addr, attrib_addr;
    u8  pattern_lo, pattern_hi, attrib_bits;

    // cache(attrib_bitsは有効ビットが0x0Cのみだから初回は絶対ヒットしない)
    u8 attrib_bits_cache = 0xFF, pattern_lo_cache = 0xFF, pattern_hi_cache = 0xFF;
    u32 realCYCLES_PER_DRAW;

    tile_x = (g_PPU.loopy_v & 0x001F);
    tile_y = (g_PPU.loopy_v & 0x03E0) >> 5;

    name_addr = 0x2000 + (g_PPU.loopy_v & 0x0FFF);

    attrib_addr = 0x2000 + (g_PPU.loopy_v & 0x0C00) + 0x03C0 + ((tile_y & 0xFFFC)<<1) + (tile_x>>2);

    if(0x0000 == (tile_y & 0x0002))
      if(0x0000 == (tile_x & 0x0002))
        attrib_bits = (VRAM_R(attrib_addr) & 0x03) << 2;
      else
        attrib_bits = (VRAM_R(attrib_addr) & 0x0C);
  else
    if(0x0000 == (tile_x & 0x0002))
      attrib_bits = (VRAM_R(attrib_addr) & 0x30) >> 2;
    else
      attrib_bits = (VRAM_R(attrib_addr) & 0xC0) >> 4;

    p     = buf       + (SIDE_MARGIN - g_PPU.loopy_x);
    solid_bits = g_PPU.solid_bits;

    realCYCLES_PER_DRAW = CYCLES_PER_DRAW >> CYCLES_SHIFT;
    // draw 33 tiles
    for(i = 33; i; i--) {
        u8 MMC5_pal;
        u8 *pPalBase;
        if(realCYCLES_PER_DRAW) {
            if(i != 1) NES_emulate_CPU_cycles(CYCLES_PER_DRAW / 32);
        }

        // for MMC5 VROM switch
#if 1
        if((MMC5_pal = g_NESmapper.PPU_Latch_RenderScreen(1,name_addr & 0x03FF))) {
            attrib_bits = MMC5_pal & 0x0C;
        }
#else
        if(MMC5_pal = g_NESmapper.PPU_Latch_RenderScreen(1,name_addr & 0x03FF)) {
            attrib_bits = MMC5_pal & 0x0C;
        }
#endif
        // for mapper 96
        g_NESmapper.PPU_Latch_Address(name_addr);

        // 例外処理もどき
        if (name_addr >= (12<<10)) {
            continue;
        }

        pattern_addr = g_PPU.bg_pattern_table_addr + ((s32)VRAM_R(name_addr) << 4) + ((g_PPU.loopy_v & 0x7000) >> 12);
        pattern_lo   = VRAM_R(pattern_addr);
        pattern_hi   = VRAM_R(pattern_addr+8);
        if (attrib_bits_cache != attrib_bits || pattern_lo_cache != pattern_lo || pattern_hi_cache != pattern_hi) {
            register int	c1,c2;
            // キャッシュ判定保存
            attrib_bits_cache = attrib_bits; pattern_lo_cache = pattern_lo; pattern_hi_cache = pattern_hi;
            // スプライトヒット判定用
            *solid_bits = pattern_hi|pattern_lo;

            pPalBase = &g_PPU.bg_pal[attrib_bits];
            // c1=hlhlhlhl  c2=hlhlhlhl
            c1 = ((pattern_lo>>1)&0x55)|(pattern_hi&0xAA);
            c2 = (pattern_lo&0x55)|((pattern_hi<<1)&0xAA);
            p[0] = g_Pal[ NES_COLOR_BASE + (pPalBase[(c1>>6)] & 0xF0)   ];
            p[4] = g_Pal[ NES_COLOR_BASE + (pPalBase[(c1>>2)&3] & 0xF0) ];
            p[1] = g_Pal[ NES_COLOR_BASE + (pPalBase[(c2>>6)] & 0xF0)   ];
            p[5] = g_Pal[ NES_COLOR_BASE + (pPalBase[(c2>>2)&3] & 0xF0) ];
            p[2] = g_Pal[ NES_COLOR_BASE + (pPalBase[(c1>>4)&3] & 0xF0) ];
            p[6] = g_Pal[ NES_COLOR_BASE + (pPalBase[c1&3] & 0xF0)      ];
            p[3] = g_Pal[ NES_COLOR_BASE + (pPalBase[(c2>>4)&3] & 0xF0) ];
            p[7] = g_Pal[ NES_COLOR_BASE + (pPalBase[c2&3] & 0xF0)      ];
        }
        else {
            // cache hit!
            *p     = *(p-8);
            *(p+4) = *(p-4);
            *(p+1) = *(p-7);
            *(p+5) = *(p-3);
            *(p+2) = *(p-6);
            *(p+6) = *(p-2);
            *(p+3) = *(p-5);
            *(p+7) = *(p-1);
            *solid_bits = *(solid_bits-1);
        }
        solid_bits++;
        p += 8;
        CHECK_MMC2(pattern_addr);

        tile_x++;
        name_addr++;

        // are we crossing a dual-tile boundary?
        if(0x0000 == (tile_x & 0x0001)) {
            // are we crossing a quad-tile boundary?
            if(0x0000 == (tile_x & 0x0003)) {
                // are we crossing a name table boundary?
                if(0x0000 == (tile_x & 0x001F)) {
                    name_addr ^= 0x0400; // switch name tables
                    attrib_addr ^= 0x0400;
                    name_addr -= 0x0020;
                    attrib_addr -= 0x0008;
                    tile_x -= 0x0020;
                }
                attrib_addr++;
            }
            if(0x0000 == (tile_y & 0x0002)) {
                if(0x0000 == (tile_x & 0x0002)) {
                    attrib_bits = (VRAM_R(attrib_addr) & 0x03) << 2;
                }
                else {
                    attrib_bits = (VRAM_R(attrib_addr) & 0x0C);
                }
            }
            else {
                if(0x0000 == (tile_x & 0x0002)) {
                    attrib_bits = (VRAM_R(attrib_addr) & 0x30) >> 2;
                }
                else {
                    attrib_bits = (VRAM_R(attrib_addr) & 0xC0) >> 4;
                }
            }
        }
    }
    
    if(!g_NESConfig.graphics.DisableBackGClipping && NES_PPU_bg_clip_left8()) {
        // clip left 8 pixels
#if 1
        buf[SIDE_MARGIN+0] = buf[SIDE_MARGIN+1] = buf[SIDE_MARGIN+2] =
        buf[SIDE_MARGIN+3] = buf[SIDE_MARGIN+4] = buf[SIDE_MARGIN+5] =
        buf[SIDE_MARGIN+6] = buf[SIDE_MARGIN+7] = g_Pal[ NES_COLOR_BASE + g_PPU.bg_pal[0] ];
        
#else
        core_memset(buf + SIDE_MARGIN, NES_COLOR_BASE + g_PPU.bg_pal[0], 8);
#endif
        g_PPU.solid_bits[0] = 0xFF;
    }
}

// (g_PPU.LowRegs[1] & 1)がFALSEの場合こっち
static void NES_PPU_render_bg_internal2(PIXEL_FORMAT* buf, u32 CYCLES_PER_DRAW)
{
    PIXEL_FORMAT *p;
    u32 i;
    u8 *solid_bits;
    u32 tile_x, tile_y; // tile_x pixel coords within nametable
    u32 name_addr;

    u32 pattern_addr, attrib_addr;
    u8  pattern_lo, pattern_hi, attrib_bits;

    // cache(attrib_bitsは有効ビットが0x0Cのみだから初回は絶対ヒットしない)
    u8 attrib_bits_cache = 0xFF, pattern_lo_cache = 0xFF, pattern_hi_cache = 0xFF;
    u32 realCYCLES_PER_DRAW;

    tile_x = (g_PPU.loopy_v & 0x001F);
    tile_y = (g_PPU.loopy_v & 0x03E0) >> 5;

    name_addr = 0x2000 + (g_PPU.loopy_v & 0x0FFF);

    attrib_addr = 0x2000 + (g_PPU.loopy_v & 0x0C00) + 0x03C0 + ((tile_y & 0xFFFC)<<1) + (tile_x>>2);

    if(0x0000 == (tile_y & 0x0002)) {
        if(0x0000 == (tile_x & 0x0002)) attrib_bits = (VRAM_R(attrib_addr) & 0x03) << 2;
        else                            attrib_bits = (VRAM_R(attrib_addr) & 0x0C);
    }
    else {
        if(0x0000 == (tile_x & 0x0002)) attrib_bits = (VRAM_R(attrib_addr) & 0x30) >> 2;
        else                            attrib_bits = (VRAM_R(attrib_addr) & 0xC0) >> 4;
    }
    
    p     = buf       + (SIDE_MARGIN - g_PPU.loopy_x);
    solid_bits = g_PPU.solid_bits;

    realCYCLES_PER_DRAW = CYCLES_PER_DRAW >> CYCLES_SHIFT;
    
    // draw 33 tiles
    for(i = 33; i; i--) {
        u8 MMC5_pal;
        u8 *pPalBase;
        if(realCYCLES_PER_DRAW) {
            if(i != 1) NES_emulate_CPU_cycles(CYCLES_PER_DRAW / 32);
        }

        // for MMC5 VROM switch
#if 1
        if((MMC5_pal = g_NESmapper.PPU_Latch_RenderScreen(1,name_addr & 0x03FF))) {
            attrib_bits = MMC5_pal & 0x0C;
        }
#else
        if(MMC5_pal = g_NESmapper.PPU_Latch_RenderScreen(1,name_addr & 0x03FF)) {
            attrib_bits = MMC5_pal & 0x0C;
        }
#endif
        // for mapper 96
        g_NESmapper.PPU_Latch_Address(name_addr);

        // 例外処理もどき
        if (name_addr >= (12<<10)) {
            continue;
        }

        pattern_addr = g_PPU.bg_pattern_table_addr + ((s32)VRAM_R(name_addr) << 4) + ((g_PPU.loopy_v & 0x7000) >> 12);
        pattern_lo   = VRAM_R(pattern_addr);
        pattern_hi   = VRAM_R(pattern_addr+8);
        
        if (attrib_bits_cache != attrib_bits || pattern_lo_cache != pattern_lo || pattern_hi_cache != pattern_hi) {
            register int	c1,c2;
            // キャッシュ判定保存
            attrib_bits_cache = attrib_bits; pattern_lo_cache = pattern_lo; pattern_hi_cache = pattern_hi;
            // スプライトヒット判定用
            *solid_bits = pattern_hi|pattern_lo;

            pPalBase = &g_PPU.bg_pal[attrib_bits];
            // c1=hlhlhlhl  c2=hlhlhlhl
            c1 = ((pattern_lo>>1)&0x55)|(pattern_hi&0xAA);
            c2 = (pattern_lo&0x55)|((pattern_hi<<1)&0xAA);
            p[0] = g_Pal[ NES_COLOR_BASE + pPalBase[(c1>>6)]   ];
            p[4] = g_Pal[ NES_COLOR_BASE + pPalBase[(c1>>2)&3] ];
            p[1] = g_Pal[ NES_COLOR_BASE + pPalBase[(c2>>6)]   ];
            p[5] = g_Pal[ NES_COLOR_BASE + pPalBase[(c2>>2)&3] ];
            p[2] = g_Pal[ NES_COLOR_BASE + pPalBase[(c1>>4)&3] ];
            p[6] = g_Pal[ NES_COLOR_BASE + pPalBase[c1&3]      ];
            p[3] = g_Pal[ NES_COLOR_BASE + pPalBase[(c2>>4)&3] ];
            p[7] = g_Pal[ NES_COLOR_BASE + pPalBase[c2&3]      ];
        }
        else {
            // cache hit!
            p[0] = *(p-8);
            p[4] = *(p-4);
            p[1] = *(p-7);
            p[5] = *(p-3);
            p[2] = *(p-6);
            p[6] = *(p-2);
            p[3] = *(p-5);
            p[7] = *(p-1);
            *solid_bits = *(solid_bits-1);
        }
        solid_bits++;
        p += 8;
        CHECK_MMC2(pattern_addr);

        tile_x++;
        name_addr++;

        // are we crossing a dual-tile boundary?
        if(0x0000 == (tile_x & 0x0001)) {
            // are we crossing a quad-tile boundary?
            if(0x0000 == (tile_x & 0x0003)) {
                // are we crossing a name table boundary?
                if(0x0000 == (tile_x & 0x001F)) {
                    name_addr ^= 0x0400; // switch name tables
                    attrib_addr ^= 0x0400;
                    name_addr -= 0x0020;
                    attrib_addr -= 0x0008;
                    tile_x -= 0x0020;
                }
                attrib_addr++;
            }
            if(0x0000 == (tile_y & 0x0002)) {
                if(0x0000 == (tile_x & 0x0002)) {
                    attrib_bits = (VRAM_R(attrib_addr) & 0x03) << 2;
                }
                else {
                    attrib_bits = (VRAM_R(attrib_addr) & 0x0C);
                }
            }
            else {
                if(0x0000 == (tile_x & 0x0002)) {
                    attrib_bits = (VRAM_R(attrib_addr) & 0x30) >> 2;
                }
                else {
                    attrib_bits = (VRAM_R(attrib_addr) & 0xC0) >> 4;
                }
            }
        }
    }
    
    if(!g_NESConfig.graphics.DisableBackGClipping && NES_PPU_bg_clip_left8()) {
        // clip left 8 pixels
#if 1
        buf[SIDE_MARGIN+0] = buf[SIDE_MARGIN+1] = buf[SIDE_MARGIN+2] =
        buf[SIDE_MARGIN+3] = buf[SIDE_MARGIN+4] = buf[SIDE_MARGIN+5] =
        buf[SIDE_MARGIN+6] = buf[SIDE_MARGIN+7] = g_Pal[ NES_COLOR_BASE + g_PPU.bg_pal[0] ];
        
#else
        core_memset(buf + SIDE_MARGIN, NES_COLOR_BASE + g_PPU.bg_pal[0], 8);
#endif
        g_PPU.solid_bits[0] = 0xFF;
    }
}

// (g_PPU.LowRegs[1] & 1)がTRUEの場合こっち
static void NES_PPU_render_spr_internal1(PIXEL_FORMAT* buf)
{
    u8 spr_bits[36];   // sprite bits
    u32 s, i, spr_y;   // sprite #, loop count, sprite y
    LPSPRITE pSpr;        // pointer to sprite RAM entry
    PIXEL_FORMAT* p;             // draw pointer
    s32 y;             // in-sprite coords

    u32 num_sprites = 0;
    u32 tile_addr;

    u8  pattern_lo, pattern_hi;
    u32 spr_height;
    u8 *pSPPAL, SPpat;

    spr_height = NES_PPU_sprites_8x16() ? 16 : 8;
    // for MMC5 VROM switch
    g_NESmapper.PPU_Latch_RenderScreen(0,0);

    for (i = 0; i < sizeof(spr_bits)/4; i++) {
        ((u32*)spr_bits)[i] = 0;
    }
    
    if(!g_NESConfig.graphics.DisableBackGClipping && NES_PPU_bg_clip_left8()) {
        spr_bits[0] = 0xFF;
    }

    for(s = 0; s < 64; s++) {
        pSpr = (LPSPRITE)&g_PPU.spr_ram[s<<2];

        // get y coord
        spr_y = pSpr->y+1;

        // on current scanline?
        if((spr_y > g_PPU.current_frame_line) || ((spr_y+(spr_height)) <= g_PPU.current_frame_line))
          continue;

        num_sprites++;
        if(num_sprites > 8) {
            if(!g_NESConfig.graphics.show_more_than_8_sprites) break;
        }

        // clip left
        if((pSpr->x < 8) && !g_NESConfig.graphics.DisableSpriteClipping && (NES_PPU_spr_clip_left8())) {
            if(0 == pSpr->x) continue;
        }

        y = g_PPU.current_frame_line - spr_y;

        CHECK_MMC2(pSpr->tile << 4);

        // calc offsets into buffers
        p = &buf[SIDE_MARGIN + pSpr->x];

        // flip vertically?
        if(pSpr->attr & 0x80) {// yes
            y = (spr_height-1) - y;
        }

        if(NES_PPU_sprites_8x16()) {
            tile_addr = pSpr->tile << 4;
            if(pSpr->tile & 0x01) {
                tile_addr += 0x1000;
                if(y < 8) tile_addr -= 16;
            }
            else {
                if(y >= 8) tile_addr += 16;
            }
            tile_addr += y & 0x07;
        }
        else {
            tile_addr = pSpr->tile << 4;
            tile_addr += y & 0x07;
            tile_addr += g_PPU.spr_pattern_table_addr;
        }

        pattern_lo = VRAM_R(tile_addr);
        pattern_hi = VRAM_R(tile_addr+8);
        // 水平反転FlagがONならビット反転
        if(pSpr->attr & 0x40) {// yes
            pattern_lo = g_PPU.Bit2Rev[pattern_lo];
            pattern_hi = g_PPU.Bit2Rev[pattern_hi];
        }
        // 描画するBits
        SPpat = pattern_lo | pattern_hi;

        // Sprite hit check
        if( s == 0 && !(g_PPU.LowRegs[2] & 0x40) ) {
            int	BGpos = ((pSpr->x&0xF8)+((g_PPU.loopy_x+(pSpr->x&7))&8))>>3;
            int	BGsft = 8-((g_PPU.loopy_x+pSpr->x)&7);
            u8	BGmsk = (((u16)g_PPU.solid_bits[BGpos+0]<<8)|(u16)g_PPU.solid_bits[BGpos+1])>>BGsft;
            if( SPpat & BGmsk ) g_PPU.LowRegs[2] |= 0x40;
        }

        {
            // 描画するスプライト割り出し
            int	SPpos = pSpr->x/8;
            int	SPsft = 8-(pSpr->x&7);
            u8	SPmsk = (((u16)spr_bits[SPpos+0]<<8)|(u16)spr_bits[SPpos+1])>>SPsft;
            u16	SPwrt = (u16)SPpat<<SPsft;
            spr_bits[SPpos+0] |= SPwrt >> 8;
            spr_bits[SPpos+1] |= SPwrt & 0xFF;
            SPpat &= ~SPmsk;
        }

        // priority bit
        if (pSpr->attr & 0x20) {
            int	BGpos = ((pSpr->x&0xF8)+((g_PPU.loopy_x+(pSpr->x&7))&8))>>3;
            int	BGsft = 8-((g_PPU.loopy_x+pSpr->x)&7);
            u8	BGmsk = (((u16)g_PPU.solid_bits[BGpos+0]<<8)|(u16)g_PPU.solid_bits[BGpos+1])>>BGsft;
            SPpat &= ~BGmsk;
        }

        // PalIndex;
        pSPPAL = &g_PPU.spr_pal[(pSpr->attr&0x03)<<2];
        {
            register int	c1 = ((pattern_lo>>1)&0x55)|(pattern_hi&0xAA);
            register int	c2 = (pattern_lo&0x55)|((pattern_hi<<1)&0xAA);
            if( SPpat&0x80 ) p[0] = g_Pal[ NES_COLOR_BASE + (pSPPAL[(c1>>6)] & 0xF0)    ];
            if( SPpat&0x08 ) p[4] = g_Pal[ NES_COLOR_BASE + (pSPPAL[(c1>>2)&3] & 0xF0)  ];
            if( SPpat&0x40 ) p[1] = g_Pal[ NES_COLOR_BASE + (pSPPAL[(c2>>6)] & 0xF0)    ];
            if( SPpat&0x04 ) p[5] = g_Pal[ NES_COLOR_BASE + (pSPPAL[(c2>>2)&3] & 0xF0)  ];
            if( SPpat&0x20 ) p[2] = g_Pal[ NES_COLOR_BASE + (pSPPAL[(c1>>4)&3] & 0xF0)  ];
            if( SPpat&0x02 ) p[6] = g_Pal[ NES_COLOR_BASE + (pSPPAL[c1&3] & 0xF0)       ];
            if( SPpat&0x10 ) p[3] = g_Pal[ NES_COLOR_BASE + (pSPPAL[(c2>>4)&3] & 0xF0)  ];
            if( SPpat&0x01 ) p[7] = g_Pal[ NES_COLOR_BASE + (pSPPAL[c2&3] & 0xF0)       ];
        }
    }
    // added by rinao
    if(num_sprites >= 8) {
        g_PPU.LowRegs[2] |= 0x20;
    }
    else {
        g_PPU.LowRegs[2] &= 0xDF;
    }
}

// (g_PPU.LowRegs[1] & 1)がFALSEの場合こっち
void NES_PPU_render_spr_internal2(PIXEL_FORMAT* buf)
{
    u8 spr_bits[36];   // sprite bits
    u32 i, s, spr_y;   // sprite #, loop count, sprite y
    LPSPRITE pSpr;        // pointer to sprite RAM entry
    PIXEL_FORMAT* p;             // draw pointer
    s32 y;             // in-sprite coords

    u32 num_sprites = 0;
    u32 tile_addr;

    u8  pattern_lo, pattern_hi;
    u32 spr_height;
    u8 *pSPPAL, SPpat;

    spr_height = NES_PPU_sprites_8x16() ? 16 : 8;
    // for MMC5 VROM switch
    g_NESmapper.PPU_Latch_RenderScreen(0,0);
    for (i = 0; i < sizeof(spr_bits)/4; i++) {
        ((u32*)spr_bits)[i] = 0;
    }
    
    if(!g_NESConfig.graphics.DisableBackGClipping && NES_PPU_bg_clip_left8()) {
        spr_bits[0] = 0xFF;
    }

    for(s = 0; s < 64; s++) {
        pSpr = (LPSPRITE)&g_PPU.spr_ram[s<<2];

        // get y coord
        spr_y = pSpr->y+1;

        // on current scanline?
        if((spr_y > g_PPU.current_frame_line) || ((spr_y+(spr_height)) <= g_PPU.current_frame_line))
          continue;

        num_sprites++;
        if(num_sprites > 8) {
            if(!g_NESConfig.graphics.show_more_than_8_sprites) break;
        }

        // clip left
        if((pSpr->x < 8) && !g_NESConfig.graphics.DisableSpriteClipping && (NES_PPU_spr_clip_left8())) {
            if(0 == pSpr->x) continue;
        }

        y = g_PPU.current_frame_line - spr_y;

        CHECK_MMC2(pSpr->tile << 4);

        // calc offsets into buffers
        p = &buf[SIDE_MARGIN + pSpr->x];

        // flip vertically?
        if(pSpr->attr & 0x80) {// yes
            y = (spr_height-1) - y;
        }

        if(NES_PPU_sprites_8x16()) {
            tile_addr = pSpr->tile << 4;
            if(pSpr->tile & 0x01) {
                tile_addr += 0x1000;
                if(y < 8) tile_addr -= 16;
            }
            else {
                if(y >= 8) tile_addr += 16;
            }
            tile_addr += y & 0x07;
        }
        else {
            tile_addr = pSpr->tile << 4;
            tile_addr += y & 0x07;
            tile_addr += g_PPU.spr_pattern_table_addr;
        }

        pattern_lo = VRAM_R(tile_addr);
        pattern_hi = VRAM_R(tile_addr+8);
        // 水平反転FlagがONならビット反転
        if(pSpr->attr & 0x40) {// yes
            pattern_lo = g_PPU.Bit2Rev[pattern_lo];
            pattern_hi = g_PPU.Bit2Rev[pattern_hi];
        }
        // 描画するBits
        SPpat = pattern_lo | pattern_hi;

        // Sprite hit check
        if( s == 0 && !(g_PPU.LowRegs[2] & 0x40) ) {
            int	BGpos = ((pSpr->x&0xF8)+((g_PPU.loopy_x+(pSpr->x&7))&8))>>3;
            int	BGsft = 8-((g_PPU.loopy_x+pSpr->x)&7);
            u8	BGmsk = (((u16)g_PPU.solid_bits[BGpos+0]<<8)|(u16)g_PPU.solid_bits[BGpos+1])>>BGsft;
            if( SPpat & BGmsk ) g_PPU.LowRegs[2] |= 0x40;
        }

        {
            // 描画するスプライト割り出し
            int	SPpos = pSpr->x/8;
            int	SPsft = 8-(pSpr->x&7);
            u8	SPmsk = (((u16)spr_bits[SPpos+0]<<8)|(u16)spr_bits[SPpos+1])>>SPsft;
            u16	SPwrt = (u16)SPpat<<SPsft;
            spr_bits[SPpos+0] |= SPwrt >> 8;
            spr_bits[SPpos+1] |= SPwrt & 0xFF;
            SPpat &= ~SPmsk;
        }

        // priority bit
        if (pSpr->attr & 0x20) {
            int	BGpos = ((pSpr->x&0xF8)+((g_PPU.loopy_x+(pSpr->x&7))&8))>>3;
            int	BGsft = 8-((g_PPU.loopy_x+pSpr->x)&7);
            u8	BGmsk = (((u16)g_PPU.solid_bits[BGpos+0]<<8)|(u16)g_PPU.solid_bits[BGpos+1])>>BGsft;
            SPpat &= ~BGmsk;
        }

        // PalIndex;
        pSPPAL = &g_PPU.spr_pal[(pSpr->attr&0x03)<<2];
        {
            register int c1 = ((pattern_lo>>1)&0x55)|(pattern_hi&0xAA);
            register int c2 = (pattern_lo&0x55)|((pattern_hi<<1)&0xAA);
            if( SPpat&0x80 ) p[0] = g_Pal[ NES_COLOR_BASE + pSPPAL[(c1>>6)]   ];
            if( SPpat&0x08 ) p[4] = g_Pal[ NES_COLOR_BASE + pSPPAL[(c1>>2)&3] ];
            if( SPpat&0x40 ) p[1] = g_Pal[ NES_COLOR_BASE + pSPPAL[(c2>>6)]   ];
            if( SPpat&0x04 ) p[5] = g_Pal[ NES_COLOR_BASE + pSPPAL[(c2>>2)&3] ];
            if( SPpat&0x20 ) p[2] = g_Pal[ NES_COLOR_BASE + pSPPAL[(c1>>4)&3] ];
            if( SPpat&0x02 ) p[6] = g_Pal[ NES_COLOR_BASE + pSPPAL[c1&3]      ];
            if( SPpat&0x10 ) p[3] = g_Pal[ NES_COLOR_BASE + pSPPAL[(c2>>4)&3] ];
            if( SPpat&0x01 ) p[7] = g_Pal[ NES_COLOR_BASE + pSPPAL[c2&3]      ];
        }
    }
    // added by rinao
    if(num_sprites >= 8) {
        g_PPU.LowRegs[2] |= 0x20;
    }
    else {
        g_PPU.LowRegs[2] &= 0xDF;
    }
}

