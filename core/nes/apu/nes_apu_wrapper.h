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

#ifndef _NES_APU_WRAPPER_H_
#define _NES_APU_WRAPPER_H_

#include "nes_apu.h"
#include "fdssnd.h"

//#include "libsnss.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct //_NES_APU
{
  u8 regs[0x16];
  int CurrentPlayingBank;
} NES_APU;

extern NES_APU g_APU; // nes_apu_wrapper.c

void NES_APU_reset();
void NES_APU_snd_mgr_changed();

#if 1
#define NES_APU_setchan(chan,enabled) apu_setchan((int)chan, (u8)enabled);
#else
void NES_APU_setchan(int chan, u8 enabled) {apu_setchan(chan, enabled);}
#endif

u8 NES_APU_Read(u32 addr);
void  NES_APU_Write(u32 addr, u8 data);

u8 NES_APU_ExRead(u32 addr);
void  NES_APU_ExWrite(u32 addr, u8 data);
void  NES_APU_SelectExSound(u8 data);

void NES_APU_SyncAPURegister();
u8 NES_APU_SyncDMCRegister(u32 cpu_cycles);

void NES_APU_DoFrame(int length);

void NES_APU_freeze();
//void NES_APU_thaw();


void NES_APU_Init();
void NES_APU_ShutDown();
void NES_APU_AssertParams();

// this function should be called by a state loading function
// with an array of the apu registers
// reg 0x14 is not used
void NES_APU_load_regs(const u8 new_regs[0x16]);

// this function should be called by a state saving function
// to fill an array of the apu registers
// reg 0x14 is not used
void NES_APU_get_regs(u8 reg_array[0x16]);

#ifdef __cplusplus
}
#endif


#endif
