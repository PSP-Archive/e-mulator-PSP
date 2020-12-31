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

#ifndef NESPPU_H_
#define NESPPU_H_

#include "libsnss.h"

#ifdef _WINDOWS
 #define static
 #define inline
#endif

enum {
NES_SCREEN_WIDTH  = 256,
NES_SCREEN_HEIGHT = 240,

SIDE_MARGIN = 8,

NES_SCREEN_WIDTH_VIEWABLE  = NES_SCREEN_WIDTH,

NES_BACKBUF_WIDTH = NES_SCREEN_WIDTH + (2*SIDE_MARGIN),
NES_SCREEN_HEIGHTMAX = NES_SCREEN_HEIGHT+8*2
};

typedef enum
{
NES_PPU_MIRROR_HORIZ,
NES_PPU_MIRROR_VERT,
NES_PPU_MIRROR_FOUR_SCREEN
}mirroring_type;

enum { BG_WRITTEN_FLAG = 0x01, SPR_WRITTEN_FLAG = 0x02 };

#ifdef __cplusplus
extern "C" {
#endif

// SPRITE RAM STRUCT
typedef	struct	tagSPRITE {
	u8	y;
	u8	tile;
	u8	attr;
	u8	x;
} SPRITE, *LPSPRITE;


typedef struct _NES_PPU
{
  u8  rgb_bak;
  // vram / PPU ram

  // bank ptr table
  // 0-7     = pattern table
  // 8       = name table 0
  // 9       = name table 1
  // A       = name table 2
  // B       = name table 3
  // THE FOLLOWING IS SPECIAL-CASED AND NOT PHYSICALLY IN THE BANK TABLE
  // C       = mirror of name table 0
  // D       = mirror of name table 1
  // E       = mirror of name table 2
  // F       = mirror of name table 3 (0x3F00-0x3FFF are palette info)
  u8* PPU_VRAM_banks[12];

  u8 bg_pal[0x10];
  u8 spr_pal[0x10];

  // sprite ram
  u8 spr_ram[0x100];

  u8 vram_write_protect;
  u32 vram_size;


  // internal registers
  u8 LowRegs[0x08];
  u8 HighReg0x4014;

  // 2 VRAM pattern tables
  u8 PPU_patterntables[0x8000];
  u8 PPU_patterntype[8];

  // 4 internal name tables (2 of these really are in the NES)
  u8 PPU_nametables[4*0x400];


  u32  in_vblank;

  u16  bg_pattern_table_addr;
  u16  spr_pattern_table_addr;

  u16  ppu_addr_inc;

  // loopy's internal PPU variables
  u16  loopy_v;  // vram address -- used for reading/writing through $2007
                    // see loopy-2005.txt
  u16  loopy_t;  // temp vram address
  u8   loopy_x;  // 3-bit subtile x-offset

  u8   toggle_2005_2006;

  u8 spr_ram_rw_ptr;  // sprite ram read/write pointer

  u8 read_2007_buffer;

  // rendering stuff
  u32 current_frame_line;
  u8 solid_bits[36];
  PIXEL_FORMAT dummy_buffer[NES_BACKBUF_WIDTH]; // used to do sprite 0 hit detection when we aren't supposed to draw
  u8 Bit2Rev[256]; // pattern bits”½“]—p
  void (*NES_PPU_render_bg)(PIXEL_FORMAT* buf, u32 CYCLES_PER_DRAW);
  void (*NES_PPU_render_spr)(PIXEL_FORMAT* buf);
}NES_PPU;

// extern
extern NES_PPU g_PPU;


int NES_PPU_getTopMargin(unsigned char c);
int NES_PPU_getViewableHeight(unsigned char c);
void NES_PPU_reset();

void NES_PPU_set_mirroring(u32 nt0, u32 nt1, u32 nt2, u32 nt3);
void NES_PPU_set_mirroring2(mirroring_type m);
u32 NES_PPU_vblank_NMI_enabled();
u8 NES_PPU_ReadLowRegs(u32 addr);
void  NES_PPU_WriteLowRegs(u32 addr, u8 data);

#if 1
#define NES_PPU_Read0x4014() ((u8)(g_PPU.HighReg0x4014))
#else
static inline u8 NES_PPU_Read0x4014() {return g_PPU.HighReg0x4014;}
#endif


void  NES_PPU_Write0x4014(u8 data);

// these are the rendering functions
// screen is drawn a line at a time
void NES_PPU_start_frame();
void NES_PPU_do_scanline_and_draw(PIXEL_FORMAT* buf, u32 CYCLES_PER_DRAW);
void NES_PPU_do_scanline_and_dont_draw();

#if 1
#define NES_PPU_end_frame()
#else
static inline void NES_PPU_end_frame() {};
#endif


// set vblank register flag
#if 1
#define NES_PPU_start_vblank() {g_PPU.in_vblank = 1; g_PPU.LowRegs[2] |= 0x80;}
#else
static inline void NES_PPU_start_vblank() {g_PPU.in_vblank = 1; g_PPU.LowRegs[2] |= 0x80;}
#endif

// reset vblank register flag and sprite0 hit flag1
#if 1
#define NES_PPU_end_vblank() {g_PPU.in_vblank = 0; g_PPU.LowRegs[2] &= 0x3F;}
#else
static inline void NES_PPU_end_vblank() {g_PPU.in_vblank = 0; g_PPU.LowRegs[2] &= 0x3F;}
#endif

// 0x2000
#if 1
#define NES_PPU_NMI_enabled()  (u32)(g_PPU.LowRegs[0] & 0x80)
#define NES_PPU_sprites_8x16() (u32)(g_PPU.LowRegs[0] & 0x20)
#else
static inline u32 NES_PPU_NMI_enabled()  { return g_PPU.LowRegs[0] & 0x80; }
static inline u32 NES_PPU_sprites_8x16() { return g_PPU.LowRegs[0] & 0x20; }
#endif

// 0x2001
#if 1
#define NES_PPU_spr_enabled()    (u32)(g_PPU.LowRegs[1] & 0x10)
#define NES_PPU_bg_enabled()     (u32)(g_PPU.LowRegs[1] & 0x08)
#define NES_PPU_spr_clip_left8() (u32)(!(g_PPU.LowRegs[1] & 0x04))
#define NES_PPU_bg_clip_left8()  (u32)(!(g_PPU.LowRegs[1] & 0x02))
#define NES_PPU_rgb_pal()        (u32)(g_PPU.LowRegs[1] & 0xE0)
#else
static inline u32 NES_PPU_spr_enabled()    { return g_PPU.LowRegs[1] & 0x10; }
static inline u32 NES_PPU_bg_enabled()     { return g_PPU.LowRegs[1] & 0x08; }
static inline u32 NES_PPU_spr_clip_left8() { return !(g_PPU.LowRegs[1] & 0x04); }
static inline u32 NES_PPU_bg_clip_left8()  { return !(g_PPU.LowRegs[1] & 0x02); }
static inline u32 NES_PPU_rgb_pal()        { return g_PPU.LowRegs[1] & 0xE0;}
#endif


// 0x2002
#if 1
#define NES_PPU_sprite0_hit()                     (u32)(g_PPU.LowRegs[2] & 0x40)
#define NES_PPU_more_than_8_sprites_on_cur_line() (u32)(g_PPU.LowRegs[2] & 0x20)
#define NES_PPU_VRAM_accessible()                 (u32)(g_PPU.LowRegs[2] & 0x10)
#else
static inline u32 NES_PPU_sprite0_hit()                     { return g_PPU.LowRegs[2] & 0x40; }
static inline u32 NES_PPU_more_than_8_sprites_on_cur_line() { return g_PPU.LowRegs[2] & 0x20; }
static inline u32 NES_PPU_VRAM_accessible()                 { return g_PPU.LowRegs[2] & 0x10; }
#endif

// by rinao
#if 1
#define NES_PPU_get_patt()               ((u8*)g_PPU.PPU_patterntables)
#define NES_PPU_get_namt()               ((u8*)g_PPU.PPU_nametables)
#define NES_PPU_get_pattype(bank)        ((u8 )g_PPU.PPU_patterntype[bank])
#define NES_PPU_set_pattype(bank,data)         g_PPU.PPU_patterntype[bank] = (u8)(data)
#else
static inline u8* NES_PPU_get_patt() { return g_PPU.PPU_patterntables; }
static inline u8* NES_PPU_get_namt() { return g_PPU.PPU_nametables; }
static inline u8 NES_PPU_get_pattype(u8 bank) { return g_PPU.PPU_patterntype[bank]; }
static inline void NES_PPU_set_pattype(u8 bank, u8 data) { g_PPU.PPU_patterntype[bank] = data; }
#endif

//static u8 NES_PPU_getBGColor();
// these functions read from/write to VRAM using loopy_v
u8 NES_PPU_read_2007();
void NES_PPU_write_2007(u8 data);

#ifdef __cplusplus
}
#endif

#endif
