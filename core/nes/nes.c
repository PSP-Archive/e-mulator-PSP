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

#include "nes.h"
#include "nes_rom.h"
#include "nes_ppu.h"
#include "snss.h"
#include "nes_config.h"
#include "debug.h"
#include "hal.h"
#include "cstring.h"

static void Scr_AssertPalette(u8 pal[][3]);

static u8 NES_loadROM(int nSize,u8* pRomAddr);
//static void NES_freeROM();
static int NES_EXIT(void);

static FBFORMAT fb_format;

#define NTSC_SOUND_SAMPLES 736
#define PAL_SOUND_SAMPLES  883

const u8 NES_preset_palette[2][64][3] =
{{  // include the NES palette
	#include "nes_pal.h"
},
{   // include the Nintendulator palette
	#include "nintendulator_pal.h"
}};

NES * pg_NES=0;

//NES g_NES; // NES 実体

NES *NES_Init(int nSize,u8* pRomAddr)
{
	DEBUG("nester - NES emulator by Darren Ranalli, (c) 2000");

	// Param init...
	g_NES.mapper_extramsize = 0;
	g_NES.mapper = NULL;

	DEBUG("NES_APU_Init...");

	NES_APU_Init();
	DEBUG("NES_loadROM...");
    NES_loadROM(nSize,pRomAddr);
    
	// set up palette and assert it
	g_NES.use_vs_palette = 0;
	NES_calculate_palette();
	DEBUG("Scr_AssertPalette...");
	Scr_AssertPalette(g_NES.NES_RGB_pal);

	g_NES.pad3_count=0, g_NES.pad4_count=0;
	g_NES.is_frozen = FALSE;
    
	return &g_NES;
}

void NES_set_pad(unsigned char* c)
{
	g_NES.pad1 = c; 
	g_NES.pad2 = c+1; 
	g_NES.pad3 = c+2; 
	g_NES.pad4 = c+3;
	g_NES.mic_bits = c+4; 
	g_NES.coin_bits = c+5;
}

static u8 NES_loadROM(int nSize,u8* pRomAddr)
{
    long crc;
    
    DEBUG("NES_APU_Init...");
    NES_APU_Init();
    
    // init genie
	g_NES.genie_num = 0;
    
    DEBUG("NES_ROM_LoadRom");
    
    if (!NES_ROM_LoadRom(nSize,pRomAddr)) {
        DEBUG("NES_ROM_LoadRom ERROR");
        return FALSE;
    }
    
    // set up the mapper
    crc = NES_crc32();
    
#include "nes_set_cycles.cpp"
    g_NES.mapper = GetMapper();
    if(!g_NES.mapper) {
        DEBUGVALUE("unsupported mapper", NES_ROM_get_mapper_num());
        return FALSE;
    }
    
	// load datas to save it at the top of reset()
	NES_Load_SaveRAM();
	NES_Load_Disk();

//	Scr_SetScreenMode(NES_ROM_get_screen_mode());
    
	if(g_NESConfig.preferences.TV_Mode){
		g_NES.nes_type = g_NESConfig.preferences.TV_Mode;
	}else {
		g_NES.nes_type = NES_ROM_get_screen_mode();
	}
	NES_reset(0);
	DEBUG("Starting emulation...");
	return TRUE;
}

/*
static void NES_freeROM()
{
	NES_Save_SaveRAM();
	NES_Save_Disk();

	DEBUG("Freeing ROM...");

	g_NES.mapper = NULL;
//	Scr_ClearScreen(0x00);
//	Scr_BltScreen();
	DEBUG("Done");
}
*/


void NES_reset(unsigned char softreset)
{
	long crc;
	DEBUG("NES_reset in");
	// save SaveRAM
	NES_Save_SaveRAM();
	NES_Save_Disk();

	if(!softreset || NES_ROM_get_mapper_num()==20){
		DEBUG("clear RAM");
		// clear RAM
		core_memset(g_NES.RAM, 0x00, sizeof(g_NES.RAM));
	}

	// set up CPU
	{
		nes6502_context context;
		DEBUG("set up CPU");

		core_memset((void*)&context, 0x00, sizeof(context));
		NES6502_GetContext(&context);

		context.mem_page[0] = g_NES.RAM;
		context.mem_page[3] = g_NES.SaveRAM;

		NES6502_SetContext(&context);
	}

	DEBUG("NES_PPU_reset");
	// reset the PPU
	NES_PPU_reset();

	// VROM write protect
	g_PPU.vram_write_protect = NES_ROM_get_num_8k_VROM_banks() ? 1 : 0;
	g_PPU.vram_size = 0x2000;

	DEBUG("NES_APU_reset");
	// reset the APU
	NES_APU_reset();

	g_NES.frame_irq_enabled = 0xFF;
	g_NES.frame_irq_disenabled = 0;

	if(g_NES.mapper)
	{
		DEBUG("reset the mapper");
		// reset the mapper
		g_NES.mapper->Reset();
	}
    
    DEBUG("NES6502_Reset");
    // reset the CPU
	NES6502_Reset();
    
	DEBUG("NES_Load_SaveRAM");
	// load SaveRAM
	NES_Load_SaveRAM();
	DEBUG("NES_Load_Disk");
	NES_Load_Disk();

	// set up the trainer if present
	if(NES_ROM_has_trainer())
	{
		DEBUG("trainer is located at 0x7000; SaveRAM is 0x2000 bytes at 0x6000");
		// trainer is located at 0x7000; SaveRAM is 0x2000 bytes at 0x6000
		core_memcpy(&g_NES.SaveRAM[0x1000], NES_ROM_get_trainer(), TRAINER_LEN);
	}
	crc = NES_crc32();
    
#include "nes_set_vspalette.h"

	g_NES.ideal_cycle_count  = 0;
	g_NES.emulated_cycle_count = 0;

	g_NES.disk_side_flag = 0;

	// pad1 & pad2 reset
	g_NES.net_pad1_bits = 0;
	g_NES.net_pad2_bits = 0;
	g_NES.net_past_pad1_bits = 0;
	g_NES.net_past_pad2_bits = 0;
	g_NES.net_past_disk_side = 0;
	g_NES.net_syncframe = 0;

	g_NES.pad_strobe = FALSE;
	g_NES.pad1_bits = 0;
	g_NES.pad2_bits = 0;

	g_NES.pad3_bits=0;
	g_NES.pad4_bits=0;
	g_NES.pad3_bitsnes=0;
	g_NES.pad4_bitsnes=0;
	DEBUG("NES_reset out");
}

/*
static u8 NES_emulate_frame(u8 draw)
{
	if(1==NES_ROM_get_screen_mode()) //g_NESConfig.graphics.show_all_scanlines
		return NES_emulate_NTSC_frame(draw);
	else
		return NES_emulate_PAL_frame(draw);
}
*/


static u8 NES_emulate_NTSC_frame(u8 draw)
{
	u32 i;
    PIXEL_FORMAT* cur_line; // ptr to screen buffer
	u8 retval = draw;
	DEBUG("NES_emulate_NTSC_frame");

	NES_trim_cycle_counts();

	// do frame
	NES_PPU_start_frame();

    //cur_line = g_NesVram;
    cur_line = fb_format.fb;

	// LINES 0-239
    for(i = 0; i < NES_NUM_FRAME_LINES; i++) {
        // do one line's worth of CPU cycles
        //NES_emulate_CPU_cycles(g_NES.CYCLES_PER_LINE);
		//mapper->HSync(i);

        if(retval) {
			// Bankswitch per line Support for Mother
			if(g_NES.BANKSWITCH_PER_TILE) {
				// render line
				NES_PPU_do_scanline_and_draw(cur_line, g_NES.CYCLES_PER_LINE * 32 / 42);
				// do half line's worth of CPU cycles (hblank)
				NES_emulate_CPU_cycles(13 << CYCLES_SHIFT);
				g_NESmapper.HSync(i);
                NES_emulate_CPU_cycles(g_NES.CYCLES_PER_LINE * 10 / 42 - (13 << CYCLES_SHIFT));
                
				if(i == 0) {
					NES_emulate_CPU_cycles(g_NES.CYCLES_PER_LINE * 32 / 42 + (13 << CYCLES_SHIFT));
					g_NESmapper.HSync(i);
					NES_emulate_CPU_cycles(g_NES.CYCLES_PER_LINE * 10 / 42 - (13 << CYCLES_SHIFT));
				}
			} else {
				// do one line's worth of CPU cycles
				NES_emulate_CPU_cycles(g_NES.CYCLES_PER_LINE);
				g_NESmapper.HSync(i);
				// render line
				NES_PPU_do_scanline_and_draw(cur_line, 0);
			}
			// point to next line
			cur_line += fb_format.width;
		} else {
			// do one line's worth of CPU cycles
			NES_emulate_CPU_cycles(g_NES.CYCLES_PER_LINE);
			g_NESmapper.HSync(i);
			NES_PPU_do_scanline_and_dont_draw();
		}
	}

    NES_PPU_end_frame();

	// fram_IRQ
	if(!(g_NES.frame_irq_enabled & 0xC0))
	{
		NES6502_DoPendingIRQ();
	}

	for(i = 240; i <= 261; i++) {
        if(i == 261){
            NES_PPU_end_vblank();
        }  else if(i == 241) {
            // do v-blank
			NES_PPU_start_vblank();
            g_NESmapper.VSync();
            
            // 1 instruction between vblank flag and NMI
			NES_emulate_CPU_cycles(g_NES.CYCLES_BEFORE_NMI);
            if(NES_PPU_NMI_enabled()) NES6502_DoNMI();
            NES_emulate_CPU_cycles(g_NES.CYCLES_PER_LINE - g_NES.CYCLES_BEFORE_NMI);
			g_NESmapper.HSync(i);
			continue;
		}

		NES_emulate_CPU_cycles(g_NES.CYCLES_PER_LINE);
		g_NESmapper.HSync(i);
	}
	// HALF-LINE 262.5
	//NES_emulate_CPU_cycles(g_NES.CYCLES_PER_LINE/2);

	NES_APU_DoFrame(NTSC_SOUND_SAMPLES);
	NES_APU_SyncAPURegister();

	return retval;
}

static u8 NES_emulate_PAL_frame(u8 draw)
{
	u32 i;
	PIXEL_FORMAT* cur_line; // ptr to screen buffer
	u8 retval = draw;

	NES_trim_cycle_counts();
	DEBUG("NES_emulate_PAL_frame");

	// do frame
	NES_PPU_start_frame();

    //cur_line = g_NesVram;
    cur_line = fb_format.fb;
    
	// LINES 0-239
	for(i = 0; i < 240; i++) {
		if(retval) {
			// do one line's worth of CPU cycles
			NES_emulate_CPU_cycles(g_NES.CYCLES_PER_LINE);
			g_NESmapper.HSync(i);
			// render line
			NES_PPU_do_scanline_and_draw(cur_line, 0);
			cur_line += fb_format.width;
		} else {
			// do one line's worth of CPU cycles
			NES_emulate_CPU_cycles(g_NES.CYCLES_PER_LINE);
			g_NESmapper.HSync(i);
			NES_PPU_do_scanline_and_dont_draw();
		}
	}

	NES_PPU_end_frame();

	// fram_IRQ
	if(!(g_NES.frame_irq_enabled & 0xC0)) {
		NES6502_DoPendingIRQ();
	}

    for(i = 240; i <= 311; i++) {
        if(i == 311) {
            NES_PPU_end_vblank();
        }
        else if(i == 241) {
			// do v-blank
            NES_PPU_start_vblank();
            g_NESmapper.VSync();
			// 1 instruction between vblank flag and NMI
			NES_emulate_CPU_cycles(g_NES.CYCLES_BEFORE_NMI);
			if(NES_PPU_NMI_enabled()) NES6502_DoNMI();
			NES_emulate_CPU_cycles(g_NES.CYCLES_PER_LINE - g_NES.CYCLES_BEFORE_NMI);
			g_NESmapper.HSync(i);
			continue;
		}

		NES_emulate_CPU_cycles(g_NES.CYCLES_PER_LINE);
		g_NESmapper.HSync(i);
	}

	NES_APU_DoFrame(PAL_SOUND_SAMPLES);
	NES_APU_SyncAPURegister();

	return retval;
}



void NES_freeze()
{
	NES_APU_freeze();
	g_NES.is_frozen = TRUE;
}

void NES_thaw()
{
//	NES_APU_thaw();
	g_NES.is_frozen = FALSE;
}

u8 NES_frozen()
{
	return g_NES.is_frozen;
}

u8 NES_MemoryRead(u32 addr)
{
	//  LOG("Read " << HEX(addr,4) << endl);

	if(addr < 0x2000) // RAM
	{
		return NES_ReadRAM(addr);
	}
	else if(addr < 0x4000) // low registers
	{
		return NES_ReadLowRegs(addr);
	}
	else if(addr < 0x4018) // high registers
	{
		return NES_ReadHighRegs(addr);
	}
	else if(addr < 0x6000) // mapper low
	{
		//    LOG("MAPPER LOW READ: " << HEX(addr,4) << endl);
		//    return((u8)(addr >> 8)); // what's good for conte is good for me
		return g_NESmapper.MemoryReadLow(addr);
	}
	else // save RAM, or ROM (mapper 40)
	{
		g_NESmapper.MemoryReadSaveRAM(addr);
		return NES6502_GetByte(addr);
	}
}

void NES_MemoryWrite(u32 addr, u8 data)
{
	//  LOG("Write " << HEX(addr,4) << " " << HEX(data,2) << endl);

	if(addr < 0x2000) // RAM
	{
		NES_WriteRAM(addr, data);
	}
	else if(addr < 0x4000) // low registers
	{
		NES_WriteLowRegs(addr, data);
	}
	else if(addr < 0x4018) // high registers
	{
		NES_WriteHighRegs(addr, data);
		g_NESmapper.WriteHighRegs(addr, data);
	}
	else if(addr < 0x6000) // mapper low
	{
		g_NESmapper.MemoryWriteLow(addr, data);
	}
	else if(addr < 0x8000) // save RAM
	{
		g_NES.SaveRAM[addr - 0x6000] = data;
		g_NESmapper.MemoryWriteSaveRAM(addr, data);
	}
	else // mapper
	{
		g_NESmapper.MemoryWrite(addr, data);
	}
}

u8 NES_ReadLowRegs(u32 addr)
{
	//return NES_PPU_ReadLowRegs(addr & 0xE007);
	if(g_NES.vstopgun_ppu)
	{
		if(addr == 0x2002)
		{
			return NES_PPU_ReadLowRegs(addr & 0x2002) | 0x1b;
		}
	}
	return NES_PPU_ReadLowRegs(addr & 0xE007);
}

void NES_WriteLowRegs(u32 addr, u8 data)
{
	if(g_NES.vstopgun_ppu)
	{
		if(addr == 0x2000)
		{
			g_NES.vstopgun_value = data & 0x7F;
			NES_PPU_WriteLowRegs(0x2000, (u8)(data & 0x7F));
			return;
		}
		if(addr == 0x2001)
		{
			NES_PPU_WriteLowRegs(0x2000, (u8)(g_NES.vstopgun_value | (data & 0x80)));
			NES_PPU_WriteLowRegs(0x2001, data);
			return;
		}
	}
	if (addr & 0xFFFF0000) {
		addr = 123;
	}
	NES_PPU_WriteLowRegs(addr & 0xE007, data);
}


u8 NES_ReadHighRegs(u32 addr)
{
	if(addr == 0x4014) // SPR-RAM DMA
	{
#ifdef NESTER_DEBUG
		LOG("Read from SPR-RAM DMA reg??" << endl);
#endif
		return NES_PPU_Read0x4014();
	}
	else if(addr == 0x4015 && !(g_NES.frame_irq_enabled & 0xC0)) // frame_IRQ
	{
		return NES_APU_Read(0x4015) | 0x40;
	}
	else if(addr < 0x4016) // APU
	{
		//    LOG("APU READ:" << HEX(addr,4) << endl);
		return NES_APU_Read(addr);
	}
	else // joypad regs
	{
		u8 retval=0;

		if(addr == 0x4016)
		{
			{
				if(g_NES.pad3_count<8){
					// joypad 1
					retval |=  g_NES.pad1_bits & 0x01;
					g_NES.pad1_bits >>= 1;

					// joypad 3 (Famicom)
					retval |=  (g_NES.pad3_bits & 0x01)<<1;
					g_NES.pad3_bits >>= 1;
				}
				else if(g_NES.pad3_count<16){		//NES 4play (joypad 3)
					retval |=  (g_NES.pad3_bitsnes & 0x01);
					g_NES.pad3_bitsnes >>= 1;
				}
//				else if(pad3_count==19)		//NES 4play adapter info?
//					retval |= 1;

				// mic on joypad 2
				retval |= *g_NES.mic_bits;
				retval |= *g_NES.coin_bits;

				retval |= 0x40;		// ?
			}
			//	  LOG("4016READ: " << HEX(addr,4) << "  "<< HEX(pad3_count,2)<<"  "<< HEX(retval,2)<< endl);
			++g_NES.pad3_count;
		}
		else if(addr == 0x4017)
		{
			{
				if(g_NES.pad4_count<8){		// joypad 2
					retval |=  (g_NES.pad2_bits & 0x01);
					g_NES.pad2_bits >>= 1;
					//joypad 4 (Famicom)
					retval |= ((g_NES.pad4_bits & 0x01) <<1);
					g_NES.pad4_bits >>= 1;
				}
				else if(g_NES.pad4_count<16){		//NES 4play (joypad 4)
					retval |=  (g_NES.pad4_bitsnes & 0x01);
					g_NES.pad4_bitsnes >>= 1;
				}
//				else if(pad4_count==18)		//NES 4play adapter info?
//					retval |= 1;
			}
			//	  LOG("4017READ: " << HEX(addr,4) << "  "<< HEX(pad4_count,2)<<"  "<< HEX(retval,2)<< endl);
			++g_NES.pad4_count;
		}
		return retval;
	}
}

void NES_WriteHighRegs(u32 addr, u8 data)
{
	if(addr == 0x4014) // SPR-RAM DMA
	{
		NES_PPU_Write0x4014(data);
		NES6502_SetDMA(514);		//CYCLES_PER_DMA
	}
	else if(addr < 0x4016) // APU
	{
		NES_APU_Write(addr, data);
	}
	else if(addr == 0x4016) // joy pad
	{
		// bit 0 == joypad strobe
		if(data & 0x01)
		{
			g_NES.pad_strobe = TRUE;
		}
		else
		{
			if(g_NES.pad_strobe)
			{
				g_NES.pad_strobe = FALSE;
				g_NES.pad3_count=0, g_NES.pad4_count=0;		//Pad read counter clear

				// get input states
				g_NES.pad1_bits = *g_NES.pad1;
				g_NES.pad2_bits = *g_NES.pad2;
				g_NES.pad3_bitsnes = g_NES.pad3_bits = *g_NES.pad3;
				g_NES.pad4_bitsnes = g_NES.pad4_bits = *g_NES.pad4;

				// MIC, Insert Coin
				if(*g_NES.mic_bits){
					g_NES.pad2_bits = 0x00;
				}
				if(*g_NES.coin_bits){
					*g_NES.coin_bits = 0x20;
				}

				// swap pad_bits for VS Super Sky Kid, VS Dr.Mario
				if(g_NES.pad_swap == 1)
				{
					g_NES.pad_swap = g_NES.pad1_bits;
					g_NES.pad1_bits = (g_NES.pad1_bits & 0x0C) | (g_NES.pad2_bits & 0xF3);
					g_NES.pad2_bits = (g_NES.pad2_bits & 0x0C) | (g_NES.pad_swap & 0xF3);
					g_NES.pad_swap = 1;
				}
				// swap pad_bits for VS Pinball (Alt)
				if(g_NES.pad_swap == 2)
				{
					g_NES.pad_swap = g_NES.pad1_bits;
					g_NES.pad1_bits = (g_NES.pad1_bits & 0xFD) | ((g_NES.pad2_bits & 0x01) << 1);
					g_NES.pad2_bits = (g_NES.pad2_bits & 0xFE) | ((g_NES.pad_swap & 0x02) >> 1);
					g_NES.pad_swap = 2;
				}
				// swap pad_bits for Nintendo World Championship (#105)
				if(NES_crc32() == 0x0b0e128f)
				{
					g_NES.pad2_bits |= g_NES.pad1_bits & 0x08;
				}
			}
		}
	}
	else if(addr == 0x4017) // frame-IRQ
	{
		if(!g_NES.frame_irq_disenabled)
		{
			g_NES.frame_irq_enabled = data;
		}
		NES_APU_Write(addr, data);
	}
}

//#include "NES_external_device.cpp"

void NES_emulate_CPU_cycles(u32 num_cycles)
{
	s32 cycle_deficit;

	g_NES.ideal_cycle_count += num_cycles;

	if(g_NES.ideal_cycle_count > g_NES.emulated_cycle_count) {
		cycle_deficit = (u32)(g_NES.ideal_cycle_count - g_NES.emulated_cycle_count) >> CYCLES_SHIFT;
		g_NES.emulated_cycle_count += (NES6502_Execute(cycle_deficit) << CYCLES_SHIFT);
		if(NES_APU_SyncDMCRegister(cycle_deficit) && g_NES.DPCM_IRQ)
		{
			NES6502_DoPendingIRQ();
		}
	}
}


// call every once in a while to avoid cycle count overflow
void NES_trim_cycle_counts()
{
	u32 trim_amount;

	trim_amount = g_NES.ideal_cycle_count;
	if(trim_amount > g_NES.emulated_cycle_count) trim_amount = g_NES.emulated_cycle_count;

	g_NES.ideal_cycle_count  -= trim_amount;
	g_NES.emulated_cycle_count -= trim_amount;
}

void NES_Save_SaveRAM()
{
    u32 i;
    
	// does the ROM use save ram?
    if(!NES_ROM_has_save_RAM()) return;
    
    // has anything been written to Save RAM?
    for(i = 0; i < sizeof(g_NES.SaveRAM); i++) {
        if(g_NES.SaveRAM[i] != 0x00) break;
    }
    
    if(i < sizeof(g_NES.SaveRAM)) {
        DEBUG("Saving Save RAM...");
        HAL_Mem_Save(g_NES.SaveRAM,NES_ROM_get_size_SaveRAM());
    }
}

void NES_Load_SaveRAM()
{
    core_memset(g_NES.SaveRAM, 0x00, sizeof(g_NES.SaveRAM));
    
    // does the ROM use save ram?
    if(!NES_ROM_has_save_RAM()) return;
    
    HAL_Mem_Load(g_NES.SaveRAM,NES_ROM_get_size_SaveRAM());
}

void NES_Save_Disk()
{
#if 0 // とりあえず未対応ってことで...
    
	// must not save before load disk image to disk[] in mapper reset
	if (!g_NES.mapper) {
		return;
	}
	u8 *diskp = g_NES.mapper->GetDiskDatap();
	if(!diskp)
		return;
    
	if(diskp[0] == 0x01)
	{
//#if 1
        byte* bf;
        u32 sn = g_NES.mapper->GetDiskSideNum();
        u32 sz = 0x10 + 65500*sn;

        if((bf = (byte*)HAL_mem_malloc(sz))) {
            core_memset(bf,0,0x10);

            core_memcpy(bf,(void*)"FDS\x1a",4);
            bf[4]=sn;

            core_memcpy(&bf[0x10],diskp,sz-0x10);
            HAL_Mem_Save(bf,sz);
            HAL_mem_free(bf);
        }
//#else
//		unsigned char headbuff[0x10] = {'F','D','S',0x1a};
//		HANDLE hFile = NULL;
//		char fn[MAX_PATH], *extp;
//
//		GetModulePath(fn, sizeof(fn));
//		strcat(fn, "SAVE/");
//		strcat(fn, NES_ROM_GetRomName());
//		strcat(fn, ".sdk");
//
//		headbuff[4] = g_NES.mapper->GetDiskSideNum();
//		hFile = NES_fopen(fn, FILE_MODE_WRITE);
//		if(hFile < 0){
//			DEBUG("can't open save disk file")
//		}
//		else
//		{
//			u8 sn = g_NES.mapper->GetDiskSideNum();
//			NES_fwrite(headbuff, 0x10, 1, hFile);
//			if (NES_fwrite(diskp, 65500*sn, 1, hFile) != 65500*sn) {
//				// セーブ失敗なので削除
//				NES_fclose(hFile);
//				NES_DeleteFile(fn);
//				return;
//			}
//			NES_fclose(hFile);
//		}
//#endif
	}
#endif
}


void NES_Load_Disk()
{
//#if 0
//	// must not load before load disk image to disk[] in mapper reset
//	u8 *diskp = g_NES.mapper->GetDiskDatap();
//
//	if(!diskp)
//		return;
//	if(diskp[0] == 0x01)
//	{
//		HANDLE hFile = NULL;
//		char fn[256];
//
//		GetModulePath(fn, sizeof(fn));
//		strcat(fn, "SAVE/");
//		strcat(fn, NES_ROM_GetRomName());
//		strcat(fn, ".sdk");
//
//		hFile = NES_fopen(fn, FILE_MODE_READ);
//		if(hFile < 0){
//			DEBUG("none found.");
//		}
//		else {
//			int d0 = NES_fgetc(hFile);
//			int d1 = NES_fgetc(hFile);
//			int d2 = NES_fgetc(hFile);
//			int d3 = NES_fgetc(hFile);
//
//			if(d0 == 'F' && d1 == 'D' && d2 == 'S' && d3 == 0x1a)
//			{
//				u32 sn= g_NES.mapper->GetDiskSideNum();
//				// new disk save format
//				NES_fseek(hFile, 16, FILE_SEEK_SET);
//				NES_fread(diskp, sn*65500, 1, hFile);
//			}
//			else {
//				DEBUG("error.");
//			}
//			NES_fclose(hFile);
//		}
//	}
//#endif
}

u8 NES_loadState(int fd)
{
    return LoadSNSS(fd, &g_NES);
}

u8 NES_saveState(int fd)
{
    return SaveSNSS(fd, &g_NES);
}

void NES_calculate_palette()
{
    if( g_NES.use_vs_palette ){
        core_memcpy(g_NES.NES_RGB_pal, g_NES.vs_palette, sizeof(g_NES.NES_RGB_pal));
    }
    else{
        core_memcpy(g_NES.NES_RGB_pal, NES_preset_palette[g_NESConfig.graphics.DefaultPaletteNo], sizeof(g_NES.NES_RGB_pal));
    }
#if 0
	else {
        if( use_vs_palette ) {
            core_memcpy(g_NES.NES_RGB_pal, g_NES.vs_palette, sizeof(g_NES.NES_RGB_pal));
        } else {
            core_memcpy(g_NES.NES_RGB_pal, NES_preset_palette[g_NESConfig.graphics.DefaultPaletteNo], sizeof(g_NES.NES_RGB_pal));
        }
    }
#endif
    if(NES_PPU_rgb_pal()) {
        int i;
        for(i = 0; i < NES_NUM_COLORS; i++) {
            switch(NES_PPU_rgb_pal()) {
              case 0x20:
                g_NES.NES_RGB_pal[i][1] = ((int)(g_NES.NES_RGB_pal[i][1] * 80) /100);
                g_NES.NES_RGB_pal[i][2] = ((int)(g_NES.NES_RGB_pal[i][2] * 73) /100);
                break;
              case 0x40:
                g_NES.NES_RGB_pal[i][0] = ((int)(g_NES.NES_RGB_pal[i][0] * 73) /100);
                g_NES.NES_RGB_pal[i][2] = ((int)(g_NES.NES_RGB_pal[i][2] * 70) /100);
                break;
              case 0x60:
                g_NES.NES_RGB_pal[i][0] = ((int)(g_NES.NES_RGB_pal[i][0] * 76) /100);
                g_NES.NES_RGB_pal[i][1] = ((int)(g_NES.NES_RGB_pal[i][1] * 78) /100);
                g_NES.NES_RGB_pal[i][2] = ((int)(g_NES.NES_RGB_pal[i][2] * 58) /100);
                break;
              case 0x80:
                g_NES.NES_RGB_pal[i][0] = ((int)(g_NES.NES_RGB_pal[i][0] * 86) /100);
                g_NES.NES_RGB_pal[i][1] = ((int)(g_NES.NES_RGB_pal[i][1] * 80) /100);
                break;
              case 0xA0:
                g_NES.NES_RGB_pal[i][0] = ((int)(g_NES.NES_RGB_pal[i][0] * 83) /100);
                g_NES.NES_RGB_pal[i][1] = ((int)(g_NES.NES_RGB_pal[i][1] * 68) /100);
                g_NES.NES_RGB_pal[i][2] = ((int)(g_NES.NES_RGB_pal[i][2] * 85) /100);
                break;
              case 0xC0:
                g_NES.NES_RGB_pal[i][0] = ((int)(g_NES.NES_RGB_pal[i][0] * 67) /100);
                g_NES.NES_RGB_pal[i][1] = ((int)(g_NES.NES_RGB_pal[i][1] * 77) /100);
                g_NES.NES_RGB_pal[i][2] = ((int)(g_NES.NES_RGB_pal[i][2] * 83) /100);
                break;
              case 0xE0:
                g_NES.NES_RGB_pal[i][0] = ((int)(g_NES.NES_RGB_pal[i][0] * 68) /100);
                g_NES.NES_RGB_pal[i][1] = ((int)(g_NES.NES_RGB_pal[i][1] * 68) /100);
                g_NES.NES_RGB_pal[i][2] = ((int)(g_NES.NES_RGB_pal[i][2] * 68) /100);
                break;
            }
        }
    }

    if(g_NESConfig.graphics.black_and_white) {
        int i;
        
        for(i = 0; i < NES_NUM_COLORS; i++) {
            u8 Y;
            
            Y = (u8)(((g_NES.NES_RGB_pal[i][0] * 299) + (g_NES.NES_RGB_pal[i][1] * 587) + (g_NES.NES_RGB_pal[i][2] * 114))  / 1000);
            //			Y = (u8)(((float)g_NES.NES_RGB_pal[i][0] * 0.299) +
            //			            ((float)g_NES.NES_RGB_pal[i][1] * 0.587) +
            //			            ((float)g_NES.NES_RGB_pal[i][2] * 0.114));
            g_NES.NES_RGB_pal[i][0] = Y;
            g_NES.NES_RGB_pal[i][1] = Y;
            g_NES.NES_RGB_pal[i][2] = Y;
        }
    }
}

void NES_ppu_rgb()
{
	NES_calculate_palette();
	Scr_AssertPalette(g_NES.NES_RGB_pal);
}

u8 NES_GetDiskSideNum()
{
	if (g_NES.mapper) {
		return g_NES.mapper->GetDiskSideNum();
	}
	return 0;
}

u8 NES_GetDiskSide()
{
	if (g_NES.mapper) {
		return g_NES.mapper->GetDiskSide();
	}
	return 0;
}

void NES_SetDiskSide(u8 side)
{
	if (g_NES.mapper) {
		g_NES.disk_side_flag = side;// | 0x10;
		g_NES.mapper->SetDiskSide((u8)(g_NES.disk_side_flag & 0x0F));
	}
}

u8 NES_DiskAccessed()
{
	if (g_NES.mapper) {
		return g_NES.mapper->DiskAccessed();
	}
	return FALSE;
}

void NES_GetROMInfoStr(char *wh)
{
	NES_ROM_GetROMInfoStr(wh);
}

#if 0
//int NES_Load_Genie(const char *szLastGeniePath)
//{
//	HANDLE hFile;
//	int c;
//
//	g_NES.genie_num = 0;
//
//#define EOF (-1)
//
//	hFile = NES_fopen(szLastGeniePath, FILE_MODE_READ);
//	if(hFile < 0) return;
//	while((c = NES_fgetc(hFile)) != EOF)
//	{
//		u8 code[9], i, p = 0;
//		core_memset(code, 0x00, sizeof(code));
//		code[0] = c;
//		if(!(c == 0x0D || c == 0x0A || c == EOF))
//		{
//			for(;;)
//			{
//				c = NES_fgetc(hFile);
//				if(c == 0x0D || c == 0x0A || c == EOF) break;
//				p++;
//				if(p < 8) code[p] = c;
//			}
//		}
//		for(i = 0; i < 9; i++)
//		{
//			switch(code[i])
//			{
//			case 'A': code[i] = 0x00; break;
//			case 'P': code[i] = 0x01; break;
//			case 'Z': code[i] = 0x02; break;
//			case 'L': code[i] = 0x03; break;
//			case 'G': code[i] = 0x04; break;
//			case 'I': code[i] = 0x05; break;
//			case 'T': code[i] = 0x06; break;
//			case 'Y': code[i] = 0x07; break;
//			case 'E': code[i] = 0x08; break;
//			case 'O': code[i] = 0x09; break;
//			case 'X': code[i] = 0x0A; break;
//			case 'U': code[i] = 0x0B; break;
//			case 'K': code[i] = 0x0C; break;
//			case 'S': code[i] = 0x0D; break;
//			case 'V': code[i] = 0x0E; break;
//			case 'N': code[i] = 0x0F; break;
//			default:  p = i; i = 9; break;
//			}
//		}
//		if(p == 6)
//		{
//			u32 addr = 0x0000,data = 0x0000;
//			// address
//			addr |= (code[3] & 0x4) ? 0x4000 : 0x0000;
//			addr |= (code[3] & 0x2) ? 0x2000 : 0x0000;
//			addr |= (code[3] & 0x1) ? 0x1000 : 0x0000;
//			addr |= (code[4] & 0x8) ? 0x0800 : 0x0000;
//			addr |= (code[5] & 0x4) ? 0x0400 : 0x0000;
//			addr |= (code[5] & 0x2) ? 0x0200 : 0x0000;
//			addr |= (code[5] & 0x1) ? 0x0100 : 0x0000;
//			addr |= (code[1] & 0x8) ? 0x0080 : 0x0000;
//			addr |= (code[2] & 0x4) ? 0x0040 : 0x0000;
//			addr |= (code[2] & 0x2) ? 0x0020 : 0x0000;
//			addr |= (code[2] & 0x1) ? 0x0010 : 0x0000;
//			addr |= (code[3] & 0x8) ? 0x0008 : 0x0000;
//			addr |= (code[4] & 0x4) ? 0x0004 : 0x0000;
//			addr |= (code[4] & 0x2) ? 0x0002 : 0x0000;
//			addr |= (code[4] & 0x1) ? 0x0001 : 0x0000;
//			// value
//			data |= (code[0] & 0x8) ? 0x0080 : 0x0000;
//			data |= (code[1] & 0x4) ? 0x0040 : 0x0000;
//			data |= (code[1] & 0x2) ? 0x0020 : 0x0000;
//			data |= (code[1] & 0x1) ? 0x0010 : 0x0000;
//			data |= (code[5] & 0x8) ? 0x0008 : 0x0000;
//			data |= (code[0] & 0x4) ? 0x0004 : 0x0000;
//			data |= (code[0] & 0x2) ? 0x0002 : 0x0000;
//			data |= (code[0] & 0x1) ? 0x0001 : 0x0000;
//			g_NES.genie_code[g_NES.genie_num] = (addr << 16) | data;
//			g_NES.genie_num++;
//		}
//		else if(p == 8)
//		{
//			u32 addr = 0x0000,data = 0x0000;
//			// address
//			addr |= (code[3] & 0x4) ? 0x4000 : 0x0000;
//			addr |= (code[3] & 0x2) ? 0x2000 : 0x0000;
//			addr |= (code[3] & 0x1) ? 0x1000 : 0x0000;
//			addr |= (code[4] & 0x8) ? 0x0800 : 0x0000;
//			addr |= (code[5] & 0x4) ? 0x0400 : 0x0000;
//			addr |= (code[5] & 0x2) ? 0x0200 : 0x0000;
//			addr |= (code[5] & 0x1) ? 0x0100 : 0x0000;
//			addr |= (code[1] & 0x8) ? 0x0080 : 0x0000;
//			addr |= (code[2] & 0x4) ? 0x0040 : 0x0000;
//			addr |= (code[2] & 0x2) ? 0x0020 : 0x0000;
//			addr |= (code[2] & 0x1) ? 0x0010 : 0x0000;
//			addr |= (code[3] & 0x8) ? 0x0008 : 0x0000;
//			addr |= (code[4] & 0x4) ? 0x0004 : 0x0000;
//			addr |= (code[4] & 0x2) ? 0x0002 : 0x0000;
//			addr |= (code[4] & 0x1) ? 0x0001 : 0x0000;
//			// value
//			data |= (code[0] & 0x8) ? 0x0080 : 0x0000;
//			data |= (code[1] & 0x4) ? 0x0040 : 0x0000;
//			data |= (code[1] & 0x2) ? 0x0020 : 0x0000;
//			data |= (code[1] & 0x1) ? 0x0010 : 0x0000;
//			data |= (code[7] & 0x8) ? 0x0008 : 0x0000;
//			data |= (code[0] & 0x4) ? 0x0004 : 0x0000;
//			data |= (code[0] & 0x2) ? 0x0002 : 0x0000;
//			data |= (code[0] & 0x1) ? 0x0001 : 0x0000;
//			// compare value
//			data |= (code[6] & 0x8) ? 0x8000 : 0x0000;
//			data |= (code[7] & 0x4) ? 0x4000 : 0x0000;
//			data |= (code[7] & 0x2) ? 0x2000 : 0x0000;
//			data |= (code[7] & 0x1) ? 0x1000 : 0x0000;
//			data |= (code[5] & 0x8) ? 0x0800 : 0x0000;
//			data |= (code[6] & 0x4) ? 0x0400 : 0x0000;
//			data |= (code[6] & 0x2) ? 0x0200 : 0x0000;
//			data |= (code[6] & 0x1) ? 0x0100 : 0x0000;
//			g_NES.genie_code[g_NES.genie_num] = (addr << 16) | data | 0x80000000;
//			g_NES.genie_num++;
//		}
//	}
//	NES_fclose(hFile);
//	return g_NES.genie_num;
//}
#endif

#if 1 // for e[mulator]

static unsigned char g_PadBit[6];
extern int HAL_NES_Input(unsigned char* pad);

PIXEL_FORMAT g_Pal[256];	// palette index (using 0-63)
u8* nes_diskbios = 0;

//#define RGB(r,g,b) ((((b>>3) & 0x1F)<<10)|(((g>>3) & 0x1F)<<5)|(((r>>3) & 0x1F)<<0)|0x8000)

//
// パレット通知 pal[NES_MAX_COLORS][3]
// 
static void Scr_AssertPalette(u8 pal[][3])
{
	int cbI;
//	DEBUG("Scr_AssertPalette");
	core_memset(g_Pal, 0x00, sizeof(g_Pal));
    
	for(cbI = 0; cbI < NES_NUM_COLORS; cbI++) {
        //g_Pal[NES_COLOR_BASE + cbI] = RGB(pal[cbI][0], pal[cbI][1], pal[cbI][2]);
        g_Pal[NES_COLOR_BASE + cbI] = HAL_fb2_Color(pal[cbI][0], pal[cbI][1], pal[cbI][2],RGB888);
	}
}

static void NES_RESET(void)
{
    if( pg_NES ) {
        NES_reset(0);
    }
}

//----------------------------------------------------------------------------------------
static int NES_INIT(int nRomSize,byte* pRomAddr)
{
	NES_EXIT(); // free memory

	if( (pg_NES = (NES*)HAL_mem_malloc(sizeof(NES))) && 
        (pg_ROM = (NES_ROM*)HAL_mem_malloc(sizeof(NES_ROM))) &&
        (pg_NESmapper = (NES_mapper*)HAL_mem_malloc(sizeof(NES_mapper)))
        ) {

        HAL_fb2_init(512,256,&fb_format,HW_NES);
        fb_format.pic_x = 8;
        fb_format.pic_y = 8;
        fb_format.pic_w = 256;
        fb_format.pic_h = 224;

        core_memset(pg_ROM,0,sizeof(NES_ROM));
        core_memset(pg_NES,0,sizeof(NES));
        core_memset(pg_NESmapper,0,sizeof(NES_mapper));
        
        core_memset(&g_PPU,0,sizeof(g_PPU));
        core_memset(&g_APU,0,sizeof(g_APU));
        core_memset(&g_NESConfig,0,sizeof(g_NESConfig));
      
        //load_config();
        core_memset(g_PadBit,0x00,sizeof(g_PadBit));

        // パッド設定
        NES_set_pad(g_PadBit);

        NES_Config_SetDefaults_All();

        // ここ要注意!!
        NES_Init(nRomSize,pRomAddr);
        
		return 1;
    }
    return 0;
}

static int NES_LOOP(void)
{
    int draw=1;
    u32 key;

    if(1==NES_ROM_get_screen_mode()) { 
        draw = !HAL_fps(60);
        NES_emulate_NTSC_frame(draw);
    } else {
        draw = !HAL_fps(50);
        NES_emulate_PAL_frame(draw);
    }

    if(draw){
        HAL_fb2_bitblt(&fb_format);
    }

    key = HAL_Input(0,HW_NES);
    g_PadBit[0] = (u8)key;
    
    return (key&(1<<31));
}

static int NES_EXIT(void)
{
    if(pg_NES)       { HAL_mem_free(pg_NES);       pg_NES=0;       }
    if(pg_ROM)       { HAL_mem_free(pg_ROM);       pg_ROM=0;       }
    if(pg_NESmapper) { HAL_mem_free(pg_NESmapper); pg_NESmapper=0; }
    
    return 1;
}

extern int NES_openDB(char* dbname);


// プログラム起動時に１回だけ実行される
int NES_Setup(void)
{
	pg_NES=0;
	pg_ROM=0;
    pg_NESmapper=0;

    HAL_SetupExt(EXT_NES,"nes",
                 NES_INIT,NES_LOOP,NES_EXIT,NES_RESET,
                 NES_loadState,NES_saveState
                 );
    
    if(!NES_openDB("famicom.dat")){
        if(!NES_openDB("nesdbase.dat")){
            // disable db
        }
    }
    
    /* READ : DISKSYS.ROM  */
    if( !nes_diskbios ) {
        nes_diskbios = HAL_mem_malloc(NESDISKROM_SIZE);
        HAL_Cfg_Load("diskrom.sys",nes_diskbios,sizeof(nes_diskbios));
    }
    
    return 1;
}

#endif
