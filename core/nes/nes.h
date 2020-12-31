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

#ifndef _NES_H_
#define _NES_H_

#include "nes_6502.h"
#include "nes_mapper.h"
#include "nes_rom.h"
#include "nes_apu_wrapper.h"
#include "libsnss.h"

#ifdef __cplusplus
extern "C" {
#endif

enum {
	NES_NUM_VBLANK_LINES = 20,
	NES_NUM_FRAME_LINES = 240,

	// these are 0-based, and actions occur at start of line
	NES_NMI_LINE = 241,
	NES_VBLANK_FLAG_SET_LINE = 241,
	NES_VBLANK_FLAG_RESET_LINE = 261,
	NES_SPRITE0_FLAG_RESET_LINE = 261,

// Palette indexのベースを0にしたほうが加算処理が無くなるので高速化される ruka
	NES_COLOR_BASE = 0x00, // NES palette is set starting at color 0x00
//	NES_COLOR_BASE = 0x40, // NES palette is set starting at color 0x40 (64)
	NES_NUM_COLORS = 64    // 64 colors in the NES palette
};

// 下記値2つを変更すると止まるソフトが出てくるので注意 ruka
#define CYCLES_SHIFT  15	// bit shift count
// ((float)((double)1364.0/(double)12.0)) << CYCLES_SHIFT;
// つまり 113.6666666... * (2^CYCLES_SHIFT) の値をセットする
#define CYCLES_DEFAULT_CYCLES_PER_LINE 3724629

typedef struct /*_NES*/ {

	u32 CYCLES_PER_LINE;				// CYCLES_SHIFT
	u32 CYCLES_BEFORE_NMI;			// CYCLES_SHIFT
	u8 BANKSWITCH_PER_TILE;
	u8 DPCM_IRQ;
	// frame-IRQ
	u8 frame_irq_enabled;
	u8 frame_irq_disenabled;
	u8 NES_RGB_pal[NES_NUM_COLORS][3];

	NES_mapper* mapper;

//	NES_settings settings;

	u8 is_frozen;

	u8 nes_type;		// 1 = NTSC, 2 = PAL

	u32 ideal_cycle_count;  // CYCLES_SHIFT // number of cycles that should have executed so far
	u32 emulated_cycle_count; //CYCLES_SHIFT // number of cycles that have executed so far

	// internal memory
	u8 RAM[0x800];
	u8 SaveRAM[0x10000];

	u8 mapper_extram[0x10000];
	u32 mapper_extramsize;

	// joypad stuff
	unsigned char *pad1,*pad2,*pad3,*pad4;

	u8  pad_strobe;
	u8 pad1_bits;
	u8 pad2_bits;

	//  u8 mic_bits;
	u8 *mic_bits;
	u8 *coin_bits;


	u8 pad3_count;
	u8 pad4_count;
	u8 pad3_bits;
	u8 pad4_bits;
	u8 pad3_bitsnes;
	u8 pad4_bitsnes;


	// network joypad stuff
	u8 net_pad1_bits;
	u8 net_pad2_bits;
	u8 net_past_pad1_bits;
	u8 net_past_pad2_bits;
	u8 net_past_disk_side;
	u8 net_syncframe;

	u8 net_pad3_bits;
	u8 net_pad4_bits;
	u8 net_past_pad3_bits;
	u8 net_past_pad4_bits;

	// Disk System
	u8 disk_side_flag;

	// Game Genie
	u8 genie_num;
	u32 genie_code[256];

	// VS-Unisystem
	u8 vs_palette[192];
	u8 use_vs_palette;
	u8 pad_swap;
	u8 vstopgun_ppu;
	u8 vstopgun_value;
}NES;

#define g_NES (*pg_NES)
#define g_ROM (*pg_ROM)

extern NES *pg_NES; // 実体は NES.cpp
extern NES_ROM *pg_ROM;

extern const u8 NES_preset_palette[2][64][3];

NES *NES_Init(int nSize,u8* pRomAddr);
void NES_set_pad(unsigned char* c);

/*
static u8 NES_emulate_frame(u8 draw);
static u8 NES_emulate_NTSC_frame(u8 draw);
static u8 NES_emulate_PAL_frame(u8 draw);
*/
void NES_reset(unsigned char);

//#if 1
//#define NES_getROMname() ((char*)NES_ROM_GetRomName())
//#define NES_getROMpath() ((char*) NES_ROM_GetRomPath())
//#else
//static const char* NES_getROMname() {return NES_ROM_GetRomName();}
//static const char* NES_getROMpath() {return NES_ROM_GetRomPath();}
//#endif

u8 NES_loadState(int fd);
u8 NES_saveState(int fd);

//void NES_GetCPUContext(unsigned char *context){ NES6502_GetContext((nes6502_context*)context);}
//void NES_SetCPUContext(unsigned char *context){ NES6502_SetContext((nes6502_context*)context);}
#define NES_GetCPUContext(context) NES6502_GetContext((nes6502_context*)context)
#define NES_SetCPUContext(context) NES6502_SetContext((nes6502_context*)context)

#if 1
#define NES_GetScreenMode()      ((u8)g_NES.nes_type)
#define  NES_SetScreenMode(mode)      g_NES.nes_type = (u8)(mode)
#else
static inline u8 NES_GetScreenMode() {return g_NES.nes_type;}
static inline void NES_SetScreenMode(u8 mode) { g_NES.nes_type = mode;}
#endif

void NES_GetROMInfoStr(char *wt);
//void NES_GetGameTitleName(char *s){ strcpy(s, NES_ROM_get_GameTitleName()); };
#define NES_GetGameTitleName(s) strcpy(s, NES_ROM_get_GameTitleName())

void NES_freeze();
void NES_thaw();
u8 NES_frozen();

void NES_calculate_palette();

//u8 NES_getBGColor() { return NES_PPU_getBGColor(); }
//#define NES_getBGColor() NES_PPU_getBGColor()
void  NES_ppu_rgb();


//u32 NES_crc32() { return NES_ROM_crc32(); }
//u32 NES_crc32_all() { return NES_ROM_crc32_all(); }
//u32 NES_fds_id() { return NES_ROM_fds_id(); }
#define NES_crc32() NES_ROM_crc32()
#define NES_crc32_all() NES_ROM_crc32_all()
#define NES_fds_id() NES_ROM_fds_id()

// Disk System
u8 NES_GetDiskSideNum();
u8 NES_GetDiskSide();
void NES_SetDiskSide(u8 side);
u8 NES_DiskAccessed();

#if 1
// Game Genie
#define NES_GetGenieCodeNum()  ((u8)g_NES.genie_num)
#define NES_GetGenieCode(num) ((u32)g_NES.genie_code[(u8)num])

// SaveRAM control
#define NES_WriteSaveRAM(addr,data)      g_NES.SaveRAM[addr] = (u8)(data)
#define NES_ReadSaveRAM(addr)       ((u8)g_NES.SaveRAM[addr])
#else
// Game Genie
static inline u8 NES_GetGenieCodeNum() { return g_NES.genie_num; }
static inline u32 NES_GetGenieCode(u8 num) { return g_NES.genie_code[num]; }

// SaveRAM control
static inline void  NES_WriteSaveRAM(u32 addr, u8 data) { g_NES.SaveRAM[addr] = data;}
static inline u8 NES_ReadSaveRAM(u32 addr) { return g_NES.SaveRAM[addr]; }
#endif

void NES_emulate_CPU_cycles(u32 num_cycles);


// these are called by the CPU
u8 NES_MemoryRead(u32 addr);
void  NES_MemoryWrite(u32 addr, u8 data);

// internal read/write functions
#if 1
#define NES_ReadRAM(addr)        ((u8)g_NES.RAM[addr & 0x7FF])
#define NES_WriteRAM(addr,data)       g_NES.RAM[addr & 0x7FF] = (u8)(data)
#else
static inline u8 NES_ReadRAM(u32 addr) {return g_NES.RAM[addr & 0x7FF];};
static inline void  NES_WriteRAM(u32 addr, u8 data) {g_NES.RAM[addr & 0x7FF] = data;};
#endif

u8 NES_ReadLowRegs(u32 addr);
void  NES_WriteLowRegs(u32 addr, u8 data);

u8 NES_ReadHighRegs(u32 addr);
void  NES_WriteHighRegs(u32 addr, u8 data);

void  NES_trim_cycle_counts();

// file stuff
void NES_Save_SaveRAM();
void NES_Load_SaveRAM();
void NES_Save_Disk();
void NES_Load_Disk();

int NES_Load_Genie(const char *szLastGeniePath);

#ifdef __cplusplus
}
#endif


#define NESDISKROM_SIZE 0x2000

#endif
