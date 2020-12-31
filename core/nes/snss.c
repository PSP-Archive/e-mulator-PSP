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

#include "debug.h"
#include "snss.h"
#include "nes6502.h"
#include "nes.h"
#include "hal.h"
#include "cstring.h"
#include "libsnss.h"

void NES_mapper4_SNSS_fixup();
void NES_mapper9_SNSS_fixup();
void NES_mapper10_SNSS_fixup();
void NES_mapper13_SNSS_fixup();
void NES_mapper18_SNSS_fixup();
void NES_mapper20_SNSS_fixup();
void NES_mapper21_SNSS_fixup();
void NES_mapper23_SNSS_fixup();
void NES_mapper25_SNSS_fixup();
void NES_mapper49_SNSS_fixup();
void NES_mapper95_SNSS_fixup();

// these functions apply a SNSS block to the current emulated NES

static void adopt_BASR(SnssBaseBlock* block, NES* nes)
{
	// BASR - Base Registers
	nes6502_context context;

	NES6502_GetContext(&context);

	context.a_reg = block->regA;
	context.x_reg = block->regX;
	context.y_reg = block->regY;
	context.p_reg = block->regFlags;
	context.s_reg = block->regStack;
	context.pc_reg = block->regPc;

	context.int_pending = 0;
	context.jammed = 0;
	context.burn_cycles = 0;
	//context.dma_cycles = 0;
	NES6502_SetContext(&context);

	// registers $2000 and $2001
	NES_MemoryWrite(0x2000, block->reg2000);
	NES_MemoryWrite(0x2001, block->reg2001);

	// RAM
	core_memcpy(g_NES.RAM, block->cpuRam, 0x800);

	// SPR-RAM
	core_memcpy(g_PPU.spr_ram, block->spriteRam, 0x100);

	// PPU $2000-$2FFF (Name Tables/Attrib Tables)
	core_memcpy(g_PPU.PPU_nametables, block->ppuRam, 4*0x400);

	// palettes
	core_memcpy(g_PPU.bg_pal,  &block->palette[0x00], 0x10);
	core_memcpy(g_PPU.spr_pal, &block->palette[0x10], 0x10);

	// mirroring
	NES_PPU_set_mirroring((u32)block->mirrorState[0]&0x03,
	                        (u32)block->mirrorState[1]&0x03,
	                        (u32)block->mirrorState[2]&0x03,
	                        (u32)block->mirrorState[3]&0x03);

	// VRAM address
	g_PPU.loopy_t = block->vramAddress;

	// OAM (spr) address
	g_PPU.spr_ram_rw_ptr = block->spriteRamAddress;

	// tile X offset
	g_PPU.loopy_x = block->tileXOffset;
}

static void adopt_VRAM(SnssVramBlock* block, NES* nes)
{
	// VRAM

	// read MAX 32KB
	core_memcpy(NES_PPU_get_patt(), &block->vram, block->vramSize);

#ifdef NESTER_DEBUG
	if(block->vramSize > 0x2000)
	{
		LOG("SNSS VRAM size greater than 8K; unsupported" << endl);
	}
#endif
}

static void adopt_SRAM(SnssSramBlock* block, NES* nes)
{
	// Save-RAM
	nes6502_context context;

	// read SRAM
	NES6502_GetContext(&context);
	core_memcpy(context.mem_page[3], block->sram, (block->sramSize <= 0x2000) ? block->sramSize : 0x2000);
#ifdef NESTER_DEBUG
	if(block->sramSize > 0x2000)
	{
		LOG("SNSS SRAM size greater than 8K; unsupported" << endl);
	}
#endif
}

static void adopt_MPRD(SnssMapperBlock* block, NES* nes)
{
    u8 i;
    // Mapper Data

    // set PRG pages
    {
        nes6502_context context;
        NES6502_GetContext(&context);
        context.mem_page[4] = NES_ROM_get_ROM_banks() + ((u32)block->prgPages[0] << 13);
        context.mem_page[5] = NES_ROM_get_ROM_banks() + ((u32)block->prgPages[1] << 13);
        context.mem_page[6] = NES_ROM_get_ROM_banks() + ((u32)block->prgPages[2] << 13);
        context.mem_page[7] = NES_ROM_get_ROM_banks() + ((u32)block->prgPages[3] << 13);
        NES6502_SetContext(&context);
    }

    // set CHR pages
    for(i = 0; i < 8; i++) {
        if(block->chrPages[i] & 0x8000) {
            // VRAM
            g_PPU.PPU_VRAM_banks[i] = NES_PPU_get_patt() + ((u32)(block->chrPages[i] & 0x1f) << 10);
            NES_PPU_set_pattype(i, 0);
        }
        else {
            // VROM
            g_PPU.PPU_VRAM_banks[i] = NES_ROM_get_VROM_banks() + ((u32)block->chrPages[i] << 10);
            NES_PPU_set_pattype(i, 1);
        }
    }

    // handle mapper-specific data
    switch(NES_ROM_get_mapper_num()) {
      case 1: {
          struct mapper1Data* mapper_data = (struct mapper1Data*)&block->extraData;

          // last values written to the 4 registers
          core_memcpy(g_NESmapper.Mapper1.regs, mapper_data->registers, 4);

          // latch register
          g_NESmapper.Mapper1.bits = mapper_data->latch;

          // number of bits written to unfinished reg
          g_NESmapper.Mapper1.write_count = mapper_data->numberOfBits;
      }
        break;

      case 4: {
          struct mapper4Data* mapper_data = (struct mapper4Data*)&block->extraData;

          g_NESmapper.Mapper4.irq_counter = mapper_data->irqCounter;
          g_NESmapper.Mapper4.regs[4] = mapper_data->irqLatchCounter;
          g_NESmapper.Mapper4.irq_enabled = mapper_data->irqCounterEnabled;
          g_NESmapper.Mapper4.regs[0] = mapper_data->last8000Write;

          NES_mapper4_SNSS_fixup();
      }
        break;

      case 5: {
          struct mapper5Data* mapper_data = (struct mapper5Data*)&block->extraData;
          
          g_NESmapper.Mapper5.irq_line = mapper_data->irqLineLowByte;
          g_NESmapper.Mapper5.irq_line |= mapper_data->irqLineHighByte << 8;
          g_NESmapper.Mapper5.irq_enabled = mapper_data->irqEnabled;
          g_NESmapper.Mapper5.irq_status = mapper_data->irqStatus;
          g_NESmapper.Mapper5.wram_protect0 = mapper_data->wramWriteProtect0;
          g_NESmapper.Mapper5.wram_protect1 = mapper_data->wramWriteProtect1;
          g_NESmapper.Mapper5.prg_size = mapper_data->romBankSize;
          g_NESmapper.Mapper5.chr_size = mapper_data->vromBankSize;
          g_NESmapper.Mapper5.gfx_mode = mapper_data->gfxMode;
          g_NESmapper.Mapper5.split_control = mapper_data->splitControl;
          g_NESmapper.Mapper5.split_bank = mapper_data->splitBank;
          g_NESmapper.Mapper5.value0 = mapper_data->last5205Write;
          g_NESmapper.Mapper5.value1 = mapper_data->last5206Write;
          g_NESmapper.Mapper5.wb[3] = mapper_data->wramBank3;
          g_NESmapper.Mapper5.wb[4] = mapper_data->wramBank4;
          g_NESmapper.Mapper5.wb[5] = mapper_data->wramBank5;
          g_NESmapper.Mapper5.wb[6] = mapper_data->wramBank6;
          g_NESmapper.Mapper5.chr_reg[0][0] = mapper_data->vromBank[0][0];
          g_NESmapper.Mapper5.chr_reg[1][0] = mapper_data->vromBank[1][0];
          g_NESmapper.Mapper5.chr_reg[2][0] = mapper_data->vromBank[2][0];
          g_NESmapper.Mapper5.chr_reg[3][0] = mapper_data->vromBank[3][0];
          g_NESmapper.Mapper5.chr_reg[4][0] = mapper_data->vromBank[4][0];
          g_NESmapper.Mapper5.chr_reg[5][0] = mapper_data->vromBank[5][0];
          g_NESmapper.Mapper5.chr_reg[6][0] = mapper_data->vromBank[6][0];
          g_NESmapper.Mapper5.chr_reg[7][0] = mapper_data->vromBank[7][0];
          g_NESmapper.Mapper5.chr_reg[0][1] = mapper_data->vromBank[0][1];
          g_NESmapper.Mapper5.chr_reg[1][1] = mapper_data->vromBank[1][1];
          g_NESmapper.Mapper5.chr_reg[2][1] = mapper_data->vromBank[2][1];
          g_NESmapper.Mapper5.chr_reg[3][1] = mapper_data->vromBank[3][1];
          g_NESmapper.Mapper5.chr_reg[4][1] = mapper_data->vromBank[4][1];
          g_NESmapper.Mapper5.chr_reg[5][1] = mapper_data->vromBank[5][1];
          g_NESmapper.Mapper5.chr_reg[6][1] = mapper_data->vromBank[6][1];
          g_NESmapper.Mapper5.chr_reg[7][1] = mapper_data->vromBank[7][1];
      }
        break;
        
      case 6: {
          struct mapper6Data* mapper_data = (struct mapper6Data*)&block->extraData;
          
          g_NESmapper.Mapper6.irq_counter = mapper_data->irqCounterLowByte;
          g_NESmapper.Mapper6.irq_counter |= mapper_data->irqCounterHighByte << 8;
          g_NESmapper.Mapper6.irq_enabled = mapper_data->irqCounterEnabled;
      }
        break;
        
      case 9: {
          struct mapper9Data* mapper_data = (struct mapper9Data*)&block->extraData;
          
          // 2 latch registers
          g_NESmapper.Mapper9.latch_0000 = mapper_data->latch[0];
          g_NESmapper.Mapper9.latch_1000 = mapper_data->latch[1];
          
          // regs (B/C/D/E000)
          g_NESmapper.Mapper9.regs[1] = mapper_data->lastB000Write;
          g_NESmapper.Mapper9.regs[2] = mapper_data->lastC000Write;
          g_NESmapper.Mapper9.regs[3] = mapper_data->lastD000Write;
          g_NESmapper.Mapper9.regs[4] = mapper_data->lastE000Write;
          
          NES_mapper9_SNSS_fixup();
      }
        break;
        
      case 10: {
          struct mapper10Data* mapper_data = (struct mapper10Data*)&block->extraData;
          
          // 2 latch registers
          g_NESmapper.Mapper10.latch_0000 = mapper_data->latch[0];
          g_NESmapper.Mapper10.latch_1000 = mapper_data->latch[1];
          
          // regs (B/C/D/E000)
          g_NESmapper.Mapper10.regs[1] = mapper_data->lastB000Write;
          g_NESmapper.Mapper10.regs[2] = mapper_data->lastC000Write;
          g_NESmapper.Mapper10.regs[3] = mapper_data->lastD000Write;
          g_NESmapper.Mapper10.regs[4] = mapper_data->lastE000Write;
          
          NES_mapper10_SNSS_fixup();
      }
        break;
        
      case 13: {
          struct mapper13Data* mapper_data = (struct mapper13Data*)&block->extraData;
          
          g_NESmapper.Mapper13.chr_bank = mapper_data->wramBank;
          
          NES_mapper13_SNSS_fixup();
      }
        break;

      case 16: {
          struct mapper16Data* mapper_data = (struct mapper16Data*)&block->extraData;
          
          g_NESmapper.Mapper16.irq_counter = mapper_data->irqCounterLowByte;
          g_NESmapper.Mapper16.irq_counter |= mapper_data->irqCounterHighByte << 8;
          g_NESmapper.Mapper16.irq_enabled = mapper_data->irqCounterEnabled;
          g_NESmapper.Mapper16.irq_latch = mapper_data->irqLatchCounterLowByte;
          g_NESmapper.Mapper16.irq_latch |= mapper_data->irqLatchCounterHighByte << 8;
      }
        break;

      case 17: {
          struct mapper17Data* mapper_data = (struct mapper17Data*)&block->extraData;
          
          g_NESmapper.Mapper17.irq_counter = mapper_data->irqCounterLowByte;
          g_NESmapper.Mapper17.irq_counter |= mapper_data->irqCounterHighByte << 8;
          g_NESmapper.Mapper17.irq_enabled = mapper_data->irqCounterEnabled;
      }
        break;
        
      case 18: {
          struct mapper18Data* mapper_data = (struct mapper18Data*)&block->extraData;
          
          g_NESmapper.Mapper18.irq_counter = mapper_data->irqCounterLowByte;
          g_NESmapper.Mapper18.irq_counter |= mapper_data->irqCounterHighByte << 8;
          g_NESmapper.Mapper18.irq_enabled = mapper_data->irqCounterEnabled;
          
          NES_mapper18_SNSS_fixup();
      }
        break;
        
      case 19: {
          struct mapper19Data* mapper_data = (struct mapper19Data*)&block->extraData;
          
          g_NESmapper.Mapper19.irq_counter = mapper_data->irqCounterLowByte;
          g_NESmapper.Mapper19.irq_counter |= mapper_data->irqCounterHighByte << 8;
          g_NESmapper.Mapper19.irq_enabled = mapper_data->irqCounterEnabled;
          g_NESmapper.Mapper19.regs[0] = (mapper_data->lastE800Write & 0x40) >> 6;
          g_NESmapper.Mapper19.regs[1] = (mapper_data->lastE800Write & 0x80) >> 7;
          g_NESmapper.Mapper19.regs[2] = mapper_data->lastF800Write;
      }
        break;

      case 20:
        {
            struct mapper20Data* mapper_data = (struct mapper20Data*)&block->extraData;

            g_NESmapper.Mapper20.irq_counter = mapper_data->irqCounterLowByte;
            g_NESmapper.Mapper20.irq_counter |= mapper_data->irqCounterHighByte << 8;
            g_NESmapper.Mapper20.irq_enabled = mapper_data->irqCounterEnabled;
            g_NESmapper.Mapper20.irq_latch = mapper_data->irqLatchCounterLowByte;
            g_NESmapper.Mapper20.irq_latch |= mapper_data->irqLatchCounterHighByte << 8;
            g_NESmapper.Mapper20.irq_wait = mapper_data->irqWaitCounter;
            g_NESmapper.Mapper20.disk_enabled = mapper_data->last4023Write;
            g_NESmapper.Mapper20.write_reg = mapper_data->last4025Write;
            g_NESmapper.Mapper20.head_position = mapper_data->HeadPositionLowByte;
            g_NESmapper.Mapper20.head_position |= mapper_data->HeadPositionHighByte << 8;
            g_NESmapper.Mapper20.disk_status = mapper_data->DiskStatus;
            g_NESmapper.Mapper20.write_skip = mapper_data->WriteSkip;
            g_NESmapper.Mapper20.current_side = mapper_data->CurrentDiskSide;
            g_NESmapper.Mapper20.last_side = mapper_data->LastDiskSide;
            g_NESmapper.Mapper20.insert_wait = mapper_data->DiskInsertWait;

            NES_mapper20_SNSS_fixup();
        }
        break;

      case 21:
        {
            struct mapper21Data* mapper_data = (struct mapper21Data*)&block->extraData;

            g_NESmapper.Mapper21.irq_counter = mapper_data->irqCounter;
            g_NESmapper.Mapper21.irq_enabled = mapper_data->irqCounterEnabled;
            g_NESmapper.Mapper21.irq_latch = mapper_data->irqLatchCounter;
            g_NESmapper.Mapper21.regs[8] = mapper_data->last9002Write;

            NES_mapper21_SNSS_fixup();
        }
        break;

      case 23:
        {
            struct mapper23Data* mapper_data = (struct mapper23Data*)&block->extraData;

            g_NESmapper.Mapper23.irq_counter = mapper_data->irqCounter;
            g_NESmapper.Mapper23.irq_enabled = mapper_data->irqCounterEnabled;
            g_NESmapper.Mapper23.irq_latch = mapper_data->irqLatchCounter;
            g_NESmapper.Mapper23.regs[8] = mapper_data->last9008Write;

            NES_mapper23_SNSS_fixup();
        }
        break;

      case 24:
        {
            struct mapper24Data* mapper_data = (struct mapper24Data*)&block->extraData;

            g_NESmapper.Mapper24.irq_counter = mapper_data->irqCounter;
            g_NESmapper.Mapper24.irq_enabled = mapper_data->irqCounterEnabled;
            g_NESmapper.Mapper24.irq_latch = mapper_data->irqLatchCounter;
        }
        break;

      case 25:
        {
            struct mapper25Data* mapper_data = (struct mapper25Data*)&block->extraData;

            g_NESmapper.Mapper25.irq_counter = mapper_data->irqCounter;
            g_NESmapper.Mapper25.irq_enabled = mapper_data->irqCounterEnabled;
            g_NESmapper.Mapper25.irq_latch = mapper_data->irqLatchCounter;
            g_NESmapper.Mapper25.regs[10] = mapper_data->last9001Write;

            NES_mapper25_SNSS_fixup();
        }
        break;

      case 26:
        {
            struct mapper26Data* mapper_data = (struct mapper26Data*)&block->extraData;

            g_NESmapper.Mapper26.irq_counter = mapper_data->irqCounter;
            g_NESmapper.Mapper26.irq_enabled = mapper_data->irqCounterEnabled;
            g_NESmapper.Mapper26.irq_latch = mapper_data->irqLatchCounter;
        }
        break;

      case 32:
        {
            struct mapper32Data* mapper_data = (struct mapper32Data*)&block->extraData;

            g_NESmapper.Mapper32.regs[0] = mapper_data->last9000Write;
        }
        break;

      case 33:
        {
            struct mapper33Data* mapper_data = (struct mapper33Data*)&block->extraData;

            g_NESmapper.Mapper33.irq_counter = mapper_data->irqCounter;
            g_NESmapper.Mapper33.irq_enabled = mapper_data->irqCounterEnabled;
        }
        break;

      case 40:
        {
            struct mapper40Data* mapper_data = (struct mapper40Data*)&block->extraData;

            // IRQ counter
            g_NESmapper.Mapper40.lines_to_irq = mapper_data->irqCounter;
            // IRQ enabled
            g_NESmapper.Mapper40.irq_enabled = mapper_data->irqCounterEnabled;
        }
        break;

      case 41:
        {
            struct mapper41Data* mapper_data = (struct mapper41Data*)&block->extraData;

            g_NESmapper.Mapper41.regs[0] = mapper_data->last6000Write;
        }
        break;

      case 42:
        {
            struct mapper42Data* mapper_data = (struct mapper42Data*)&block->extraData;

            g_NESmapper.Mapper42.irq_counter = mapper_data->irqCounter;
            g_NESmapper.Mapper42.irq_enabled = mapper_data->irqCounterEnabled;
        }
        break;

      case 43:
        {
            struct mapper43Data* mapper_data = (struct mapper43Data*)&block->extraData;

            g_NESmapper.Mapper43.irq_counter = mapper_data->irqCounterLowByte;
            g_NESmapper.Mapper43.irq_counter |= mapper_data->irqCounterHighByte << 8;
            g_NESmapper.Mapper43.irq_enabled = mapper_data->irqCounterEnabled;
        }
        break;

      case 46:
        {
            struct mapper46Data* mapper_data = (struct mapper46Data*)&block->extraData;

            g_NESmapper.Mapper46.regs[0] = mapper_data->last6000Write & 0x0F;
            g_NESmapper.Mapper46.regs[1] = (mapper_data->last6000Write & 0xF0) >> 4;
            g_NESmapper.Mapper46.regs[2] = mapper_data->last8000Write & 0x01;
            g_NESmapper.Mapper46.regs[3] = (mapper_data->last8000Write & 0x70) >> 4;
        }
        break;

      case 48:
        {
            struct mapper48Data* mapper_data = (struct mapper48Data*)&block->extraData;

            g_NESmapper.Mapper48.regs[0] = mapper_data->lastE000Write;
        }
        break;

      case 49:
        {
            struct mapper49Data* mapper_data = (struct mapper49Data*)&block->extraData;

            g_NESmapper.Mapper49.irq_counter = mapper_data->irqCounter;
            g_NESmapper.Mapper49.irq_latch = mapper_data->irqLatchCounter;
            g_NESmapper.Mapper49.irq_enabled = mapper_data->irqCounterEnabled;
            g_NESmapper.Mapper49.regs[0] = mapper_data->last8000Write;
            g_NESmapper.Mapper49.regs[1] = mapper_data->last6000Write;
            g_NESmapper.Mapper49.regs[2] = mapper_data->lastA001Write;

            NES_mapper49_SNSS_fixup();
        }
        break;

      case 50:
        {
            struct mapper50Data* mapper_data = (struct mapper50Data*)&block->extraData;

            g_NESmapper.Mapper50.irq_enabled = mapper_data->irqCounterEnabled;
        }
        break;

      case 51:
        {
            struct mapper51Data* mapper_data = (struct mapper51Data*)&block->extraData;

            g_NESmapper.Mapper51.bank = mapper_data->BankSelect;
            g_NESmapper.Mapper51.mode = mapper_data->MapperMode;
        }
        break;

      case 57:
        {
            struct mapper57Data* mapper_data = (struct mapper57Data*)&block->extraData;

            g_NESmapper.Mapper57.regs[0] = mapper_data->last8800Write;
        }
        break;

      case 64:
        {
            struct mapper64Data* mapper_data = (struct mapper64Data*)&block->extraData;

            g_NESmapper.Mapper64.irq_counter = mapper_data->irqCounter;
            g_NESmapper.Mapper64.irq_enabled = mapper_data->irqCounterEnabled;
            g_NESmapper.Mapper64.irq_latch = mapper_data->irqLatchCounter;
            g_NESmapper.Mapper64.regs[0] = mapper_data->last8000Write & 0x0F;
            g_NESmapper.Mapper64.regs[1] = mapper_data->last8000Write & 0x40;
            g_NESmapper.Mapper64.regs[2] = mapper_data->last8000Write & 0x80;
        }
        break;

      case 65:
        {
            struct mapper65Data* mapper_data = (struct mapper65Data*)&block->extraData;

            g_NESmapper.Mapper65.irq_counter = mapper_data->irqCounterLowByte;
            g_NESmapper.Mapper65.irq_counter |= mapper_data->irqCounterHighByte << 8;
            g_NESmapper.Mapper65.irq_enabled = mapper_data->irqCounterEnabled;
            g_NESmapper.Mapper65.irq_latch = mapper_data->irqLatchCounterLowByte;
            g_NESmapper.Mapper65.irq_latch |= mapper_data->irqLatchCounterHighByte << 8;
        }
        break;

      case 67:
        {
            struct mapper67Data* mapper_data = (struct mapper67Data*)&block->extraData;

            g_NESmapper.Mapper67.irq_counter = mapper_data->irqCounter;
            g_NESmapper.Mapper67.irq_enabled = mapper_data->irqCounterEnabled;
            g_NESmapper.Mapper67.irq_latch = mapper_data->irqLatchCounter;
        }
        break;

      case 68:
        {
            struct mapper68Data* mapper_data = (struct mapper68Data*)&block->extraData;

            g_NESmapper.Mapper68.regs[0] = (mapper_data->lastE000Write & 0x10) >> 4;
            g_NESmapper.Mapper68.regs[1] = mapper_data->lastE000Write & 0x03;
            g_NESmapper.Mapper68.regs[2] = mapper_data->lastC000Write;
            g_NESmapper.Mapper68.regs[3] = mapper_data->lastD000Write;
        }
        break;

      case 69:
        {
            struct mapper69Data* mapper_data = (struct mapper69Data*)&block->extraData;

            g_NESmapper.Mapper69.irq_counter = mapper_data->irqCounterLowByte;
            g_NESmapper.Mapper69.irq_counter |= mapper_data->irqCounterHighByte << 8;
            g_NESmapper.Mapper69.irq_enabled = mapper_data->irqCounterEnabled;
            g_NESmapper.Mapper69.regs[0] = mapper_data->last8000Write;
        }
        break;

      case 73:
        {
            struct mapper73Data* mapper_data = (struct mapper73Data*)&block->extraData;

            g_NESmapper.Mapper73.irq_counter = mapper_data->irqCounterLowByte;
            g_NESmapper.Mapper73.irq_counter |= mapper_data->irqCounterHighByte << 8;
            g_NESmapper.Mapper73.irq_enabled = mapper_data->irqCounterEnabled;
        }
        break;

      case 75:
        {
            struct mapper75Data* mapper_data = (struct mapper75Data*)&block->extraData;

            g_NESmapper.Mapper75.regs[0] = mapper_data->lastE000Write;
            g_NESmapper.Mapper75.regs[1] = mapper_data->lastF000Write;
        }
        break;

      case 76:
        {
            struct mapper76Data* mapper_data = (struct mapper76Data*)&block->extraData;

            g_NESmapper.Mapper76.regs[0] = mapper_data->last8000Write;
        }
        break;

      case 82:
        {
            struct mapper82Data* mapper_data = (struct mapper82Data*)&block->extraData;

            g_NESmapper.Mapper82.regs[0] = mapper_data->last7EF6Write & 0x02;
        }
        break;

      case 83:
        {
            struct mapper83Data* mapper_data = (struct mapper83Data*)&block->extraData;

            g_NESmapper.Mapper83.irq_counter = mapper_data->irqCounterLowByte;
            g_NESmapper.Mapper83.irq_counter |= mapper_data->irqCounterHighByte << 8;
            g_NESmapper.Mapper83.irq_enabled = mapper_data->irqCounterEnabled;
            g_NESmapper.Mapper83.regs[0] = mapper_data->last8000Write;
            g_NESmapper.Mapper83.regs[1] = mapper_data->last8100Write;
            g_NESmapper.Mapper83.regs[2] = mapper_data->last5101Write;
        }
        break;

      case 85:
        {
            struct mapper85Data* mapper_data = (struct mapper85Data*)&block->extraData;

            g_NESmapper.Mapper85.irq_counter = mapper_data->irqCounter;
            g_NESmapper.Mapper85.irq_enabled = mapper_data->irqCounterEnabled;
            g_NESmapper.Mapper85.irq_latch = mapper_data->irqLatchCounter;
        }
        break;

      case 88:
        {
            struct mapper88Data* mapper_data = (struct mapper88Data*)&block->extraData;

            g_NESmapper.Mapper88.regs[0] = mapper_data->last8000Write;
        }
        break;

      case 91:
        {
            struct mapper91Data* mapper_data = (struct mapper91Data*)&block->extraData;

            g_NESmapper.Mapper91.irq_counter = mapper_data->irqCounter;
            g_NESmapper.Mapper91.irq_enabled = mapper_data->irqCounterEnabled;
        }
        break;

      case 95:
        {
            struct mapper95Data* mapper_data = (struct mapper95Data*)&block->extraData;

            g_NESmapper.Mapper95.regs[0] = mapper_data->last8000Write;
            NES_mapper95_SNSS_fixup();
        }
        break;

      case 96:
        {
            struct mapper96Data* mapper_data = (struct mapper96Data*)&block->extraData;

            g_NESmapper.Mapper96.vbank0 = mapper_data->wramBank;
        }
        break;

      case 105:
        {
            struct mapper105Data* mapper_data = (struct mapper105Data*)&block->extraData;

            g_NESmapper.Mapper105.irq_counter = mapper_data->irqCounter[0];
            g_NESmapper.Mapper105.irq_counter |= mapper_data->irqCounter[1] << 8;
            g_NESmapper.Mapper105.irq_counter |= mapper_data->irqCounter[2] << 16;
            g_NESmapper.Mapper105.irq_counter |= mapper_data->irqCounter[3] << 24;
            g_NESmapper.Mapper105.irq_enabled = mapper_data->irqCounterEnabled;
            g_NESmapper.Mapper105.init_state = mapper_data->InitialCounter;
            g_NESmapper.Mapper105.write_count = mapper_data->WriteCounter;
            g_NESmapper.Mapper105.bits = mapper_data->Bits;
            g_NESmapper.Mapper105.regs[0] = mapper_data->registers[0];
            g_NESmapper.Mapper105.regs[1] = mapper_data->registers[1];
            g_NESmapper.Mapper105.regs[2] = mapper_data->registers[2];
            g_NESmapper.Mapper105.regs[3] = mapper_data->registers[3];
        }
        break;

      case 117:
        {
            struct mapper117Data* mapper_data = (struct mapper117Data*)&block->extraData;

            g_NESmapper.Mapper117.irq_line = mapper_data->irqLine;
            g_NESmapper.Mapper117.irq_enabled1 = mapper_data->irqEnabled1;
            g_NESmapper.Mapper117.irq_enabled2 = mapper_data->irqEnabled2;
        }
        break;

      case 160:
        {
            struct mapper160Data* mapper_data = (struct mapper160Data*)&block->extraData;

            g_NESmapper.Mapper160.irq_counter = mapper_data->irqCounter;
            g_NESmapper.Mapper160.irq_enabled = mapper_data->irqCounterEnabled;
            g_NESmapper.Mapper160.irq_latch = mapper_data->irqLatchCounter;
            g_NESmapper.Mapper160.refresh_type = mapper_data->RefreshType;
        }
        break;

      case 182:
        {
            struct mapper182Data* mapper_data = (struct mapper182Data*)&block->extraData;

            g_NESmapper.Mapper182.irq_counter = mapper_data->irqCounter;
            g_NESmapper.Mapper182.irq_enabled = mapper_data->irqCounterEnabled;
            g_NESmapper.Mapper182.regs[0] = mapper_data->lastA000Write;
        }
        break;

      case 189:
        {
            struct mapper189Data* mapper_data = (struct mapper189Data*)&block->extraData;

            g_NESmapper.Mapper189.irq_counter = mapper_data->irqCounter;
            g_NESmapper.Mapper189.irq_enabled = mapper_data->irqCounterEnabled;
            g_NESmapper.Mapper189.irq_latch = mapper_data->irqLatchCounter;
            g_NESmapper.Mapper189.regs[0] = mapper_data->last8000Write;
        }
        break;

      case 226:
        {
            struct mapper226Data* mapper_data = (struct mapper226Data*)&block->extraData;

            g_NESmapper.Mapper226.regs[0] = mapper_data->registers[0];
            g_NESmapper.Mapper226.regs[1] = mapper_data->registers[1];
        }
        break;

      case 230:
        {
            struct mapper230Data* mapper_data = (struct mapper230Data*)&block->extraData;

            g_NESmapper.Mapper230.rom_switch = mapper_data->numberOfResets;
        }
        break;

      case 232:
        {
            struct mapper232Data* mapper_data = (struct mapper232Data*)&block->extraData;

            g_NESmapper.Mapper232.regs[0] = mapper_data->last9000Write;
            g_NESmapper.Mapper232.regs[1] = mapper_data->lastA000Write;
        }
        break;

      case 234:
        {
            struct mapper234Data* mapper_data = (struct mapper234Data*)&block->extraData;

            g_NESmapper.Mapper234.regs[0] = mapper_data->lastFF80Write;
            g_NESmapper.Mapper234.regs[1] = mapper_data->lastFFE8Write;
            g_NESmapper.Mapper234.regs[2] = mapper_data->lastFFC0Write;
        }
        break;

      case 236:
        {
            struct mapper236Data* mapper_data = (struct mapper236Data*)&block->extraData;

            g_NESmapper.Mapper236.bank = mapper_data->BankSelect;
            g_NESmapper.Mapper236.mode = mapper_data->MapperMode;
        }
        break;

      case 243:
        {
            struct mapper243Data* mapper_data = (struct mapper243Data*)&block->extraData;

            g_NESmapper.Mapper243.regs[0] = mapper_data->registers[0];
            g_NESmapper.Mapper243.regs[1] = mapper_data->registers[1];
            g_NESmapper.Mapper243.regs[2] = mapper_data->registers[2];
            g_NESmapper.Mapper243.regs[3] = mapper_data->registers[3];
        }
        break;

      case 255:
        {
            struct mapper255Data* mapper_data = (struct mapper255Data*)&block->extraData;

            g_NESmapper.Mapper255.regs[0] = mapper_data->registers[0];
            g_NESmapper.Mapper255.regs[1] = mapper_data->registers[1];
            g_NESmapper.Mapper255.regs[2] = mapper_data->registers[2];
            g_NESmapper.Mapper255.regs[3] = mapper_data->registers[3];
        }
        break;
    }
}

static void adopt_ExMPRD(int fn, NES* nes)
{
    u32 i;
    u32 block_num;
#if 1
    u8 bf[4];
    HANDLE hFile = fn;
#else
//    HANDLE hFile = NES_fopen(fn, FILE_MODE_READ);
#endif

    // pass other block
#if 1
    HAL_sts_seek(hFile,4,HAL_SEEK_SET);
#else
//    NES_fseek(hFile, 4, FILE_SEEK_SET);
#endif

    block_num = 0;

#if 1
    HAL_sts_read(hFile,bf,4);
    block_num |= bf[0] << 24;
    block_num |= bf[1] << 16;
    block_num |= bf[2] << 8;
    block_num |= bf[3] << 0;
#else
//    block_num |= NES_fgetc(hFile) << 24;
//    block_num |= NES_fgetc(hFile) << 16;
//    block_num |= NES_fgetc(hFile) << 8;
//    block_num |= NES_fgetc(hFile) << 0;
#endif
    
    for(i = 0; i < block_num; i++) {
        u32 block_length = 0;
#if 1
        HAL_sts_seek(hFile, 8, HAL_SEEK_CUR);
        HAL_sts_read(hFile,bf,4);
        block_length |= bf[0] << 24;
        block_length |= bf[1] << 16;
        block_length |= bf[2] << 8;
        block_length |= bf[3] << 0;
        HAL_sts_seek(hFile, block_length, HAL_SEEK_CUR);
#else
//        NES_fseek(hFile, 8, FILE_SEEK_CUR);
//        block_length |= NES_fgetc(hFile) << 24;
//        block_length |= NES_fgetc(hFile) << 16;
//        block_length |= NES_fgetc(hFile) << 8;
//        block_length |= NES_fgetc(hFile) << 0;
//        NES_fseek(hFile, block_length, FILE_SEEK_CUR);
#endif
    }

    if(NES_ROM_get_mapper_num() == 1 && NES_crc32() == 0xb8747abf) {
        // Best Play - Pro Yakyuu Special (J)
        
        // save WRAM
#if 1
        HAL_sts_seek(hFile, 12, HAL_SEEK_CUR);
        HAL_sts_read(hFile,&g_NES.SaveRAM[0x2000], 0x2000);
#else
//        NES_fseek(hFile, 12, FILE_SEEK_CUR);
//        NES_fread(&g_NES.SaveRAM[0x2000], 0x2000, 1, hFile);
#endif
    }
    else if(NES_ROM_get_mapper_num() == 5) {
        // read WRAM
#if 1
        HAL_sts_seek(hFile, 12, HAL_SEEK_CUR);
        HAL_sts_read(hFile,g_NESmapper.Mapper5.wram, 0x10000);
#else
//        NES_fseek(hFile, 12, FILE_SEEK_CUR);
//        NES_fread(g_NESmapper.Mapper5.wram, 0x10000, 1, hFile);
#endif
    }
    else if(NES_ROM_get_mapper_num() == 20) {
        // read WRAM
#if 1
        HAL_sts_seek(hFile, 12, HAL_SEEK_CUR);
        HAL_sts_read(hFile,g_NESmapper.Mapper20.wram, 0x8000);
#else
//        NES_fseek(hFile, 12, FILE_SEEK_CUR);
//        NES_fread(g_NESmapper.Mapper20.wram, 0x8000, 1, hFile);
#endif
        
        // read Disk #1 - #4
#if 1
        HAL_sts_seek(hFile, 12, HAL_SEEK_CUR);
        HAL_sts_read(hFile,g_NESmapper.Mapper20.disk, 65500*4);
#else
//        NES_fseek(hFile, 12, FILE_SEEK_CUR);
//        NES_fread(g_NESmapper.Mapper20.disk, 65500*4, 1, hFile);
#endif
    }

    //999NES_fclose(hFile);

}

static void adopt_CNTR(SnssControllersBlock* block, NES* nes)
{
}

static void adopt_SOUN(SnssSoundBlock* block, NES* nes)
{
	// Sound Data

	// give them to the apu
	NES_APU_load_regs(block->soundRegisters);
}

#if 1 //3333
u8 LoadSNSS(int fn, NES* nes)
{
    int i;
    SNSS_BLOCK_TYPE blockType;
    SNSS_FILE* snssFile = NULL;
    
    if(SNSS_OK != SNSS_OpenFile(&snssFile, fn, SNSS_OPEN_READ)) {
        goto OpenError;
    }
    
    // at this point, it's too late to go back, and the NES must be reset on failure
    for(i = 0; i < (int)snssFile->headerBlock.numberOfBlocks; i++) {
        if(SNSS_OK != SNSS_GetNextBlockType(&blockType, snssFile)) { goto ReadError; }
        if(SNSS_OK != SNSS_ReadBlock(snssFile, blockType))         { goto ReadError; }
        
        switch(blockType) {
          case SNSS_BASR:  adopt_BASR(&snssFile->baseBlock, nes);    break;
          case SNSS_VRAM:  adopt_VRAM(&snssFile->vramBlock, nes);    break;
          case SNSS_SRAM:  adopt_SRAM(&snssFile->sramBlock, nes);    break;
          case SNSS_MPRD:  adopt_MPRD(&snssFile->mapperBlock,nes);   break;
          case SNSS_CNTR:  adopt_CNTR(&snssFile->contBlock, nes);    break;
          case SNSS_SOUN:  adopt_SOUN(&snssFile->soundBlock, nes);   break;
          case SNSS_UNKNOWN_BLOCK:                                   break;
          default:         goto ReadError;                           break;
		}
	}
    
    // SNSS_CloseFile(&snssFile);

	// read Extra Mapper Data
	adopt_ExMPRD(fn, nes);

	DEBUG("Loaded done");

	return TRUE;
ReadError:
	NES_reset(0);
OpenError:
	DEBUG("Error reading ");
	if(snssFile) SNSS_CloseFile(&snssFile);
	return FALSE;
}
#endif

// these functions create a SNSS block from the current emulated NES
// return 0 if block is valid

static int extract_BASR(SnssBaseBlock* block, NES* nes)
{
	nes6502_context context;

	// get the CPU context
	NES6502_GetContext(&context);

	// CPU data
	block->regA = context.a_reg;
	block->regX = context.x_reg;
	block->regY = context.y_reg;
	block->regFlags = context.p_reg;
	block->regStack = context.s_reg;
	block->regPc = context.pc_reg;

	// $2000 and $2001
	block->reg2000 = g_PPU.LowRegs[0];
	block->reg2001 = g_PPU.LowRegs[1];

	// RAM
	core_memcpy(block->cpuRam, g_NES.RAM, 0x800);

	// SPR-RAM
	core_memcpy(block->spriteRam, g_PPU.spr_ram, 0x100);

	// PPU $2000-$2FFF (Name Tables/Attrib Tables)
	core_memcpy(block->ppuRam, g_PPU.PPU_nametables, 4*0x400);

	// palettes
	core_memcpy(&block->palette[0x00], g_PPU.bg_pal,  0x10);
	core_memcpy(&block->palette[0x10], g_PPU.spr_pal, 0x10);

	// mirroring
	block->mirrorState[0] = (g_PPU.PPU_VRAM_banks[0x08] - g_PPU.PPU_nametables) >> 10;
	block->mirrorState[1] = (g_PPU.PPU_VRAM_banks[0x09] - g_PPU.PPU_nametables) >> 10;
	block->mirrorState[2] = (g_PPU.PPU_VRAM_banks[0x0A] - g_PPU.PPU_nametables) >> 10;
	block->mirrorState[3] = (g_PPU.PPU_VRAM_banks[0x0B] - g_PPU.PPU_nametables) >> 10;
	ASSERT(block->mirrorState[0] < 4); ASSERT(block->mirrorState[1] < 4);
	ASSERT(block->mirrorState[2] < 4); ASSERT(block->mirrorState[3] < 4);

	// VRAM address
	block->vramAddress = g_PPU.loopy_t;

	// OAM (sprite) address
	block->spriteRamAddress = g_PPU.spr_ram_rw_ptr;

	// tile X offset
	block->tileXOffset = g_PPU.loopy_x;

	return 0;
}

static int extract_VRAM(SnssVramBlock* block, NES* nes)
{
	// if cart has VROM, don't write any VRAM
	//if(NES_ROM_get_num_8k_VROM_banks()) return -1;
	u32 i;
	u8* patterntables = NES_PPU_get_patt();

	for(i = 0; i < 0x8000; i++)
	{
		if(patterntables[i] != 0x00) break;
	}
	if(i == 0x8000) return -1;

	// 8K of VRAM data
	block->vramSize = g_PPU.vram_size;
	core_memcpy(&block->vram, NES_PPU_get_patt(), block->vramSize);

	return 0;
}

static int extract_SRAM(SnssSramBlock* block, NES* nes)
{
	nes6502_context context;
	u32 i;
	u32 *dp;

	// if nothing has been written to SRAM, don't write it out
	// has anything been written to Save RAM?
#if 0
	for(i = 0; i < sizeof(g_NES.SaveRAM); i++)
	{
		if(g_NES.SaveRAM[i] != 0x00) break;
	}
	if(i == sizeof(g_NES.SaveRAM)) return -1;
#endif

	dp = (u32 *)g_NES.SaveRAM;
	for(i = 0; i < sizeof(g_NES.SaveRAM)/4; i++)
	{
		if(dp[i] != 0x00) break;
	}
	if(i == sizeof(g_NES.SaveRAM)/4) return -1;

	// SRAM writeable flag
	block->sramEnabled = 1;

	// SRAM size (8k)
	block->sramSize = 0x2000;

	// SRAM data
	NES6502_GetContext(&context);
	core_memcpy(block->sram, context.mem_page[3], 0x2000);

	return 0;
}

static int extract_MPRD(SnssMapperBlock* block, NES* nes)
{
	nes6502_context context;
	u8 i;

	if(0 == NES_ROM_get_mapper_num()) return -1;

	// 8K PRG page numbers
	NES6502_GetContext(&context);
	block->prgPages[0] = (context.mem_page[4] - NES_ROM_get_ROM_banks()) >> 13;
	block->prgPages[1] = (context.mem_page[5] - NES_ROM_get_ROM_banks()) >> 13;
	block->prgPages[2] = (context.mem_page[6] - NES_ROM_get_ROM_banks()) >> 13;
	block->prgPages[3] = (context.mem_page[7] - NES_ROM_get_ROM_banks()) >> 13;

	// 1K CHR page numbers
    for(i = 0; i < 8; i++) {
        if(NES_PPU_get_pattype(i)) {
            // VROM
            block->chrPages[i] = (g_PPU.PPU_VRAM_banks[i] - NES_ROM_get_VROM_banks()) >> 10;
        }
        else {
            // VRAM
            block->chrPages[i] = ((g_PPU.PPU_VRAM_banks[i] - NES_PPU_get_patt()) >> 10) | 0x8000;
        }
    }
    
    switch(NES_ROM_get_mapper_num()) {
      case 1: {
          struct mapper1Data* mapper_data = (struct mapper1Data*)&block->extraData;
          
          // last values written to the 4 registers
          core_memcpy(mapper_data->registers, g_NESmapper.Mapper1.regs, 4);
          
          // latch register
          mapper_data->latch = g_NESmapper.Mapper1.bits;
          
          // number of bits written to unfinished reg
          mapper_data->numberOfBits = g_NESmapper.Mapper1.write_count;
      }
        break;

      case 4: {
          struct mapper4Data* mapper_data = (struct mapper4Data*)&block->extraData;
          
          mapper_data->irqCounter = g_NESmapper.Mapper4.irq_counter;
          mapper_data->irqLatchCounter = g_NESmapper.Mapper4.regs[4];
          mapper_data->irqCounterEnabled = g_NESmapper.Mapper4.irq_enabled;
          mapper_data->last8000Write = g_NESmapper.Mapper4.regs[0];
      }
        break;

      case 5: {
          struct mapper5Data* mapper_data = (struct mapper5Data*)&block->extraData;
          
          mapper_data->irqLineLowByte = g_NESmapper.Mapper5.irq_line & 0x00ff;
          mapper_data->irqLineHighByte = (g_NESmapper.Mapper5.irq_line & 0xff00) >> 8;
          mapper_data->irqEnabled = g_NESmapper.Mapper5.irq_enabled;
          mapper_data->irqStatus = g_NESmapper.Mapper5.irq_status;
          mapper_data->wramWriteProtect0 = g_NESmapper.Mapper5.wram_protect0;
          mapper_data->wramWriteProtect1 = g_NESmapper.Mapper5.wram_protect1;
          mapper_data->romBankSize = g_NESmapper.Mapper5.prg_size;
          mapper_data->vromBankSize = g_NESmapper.Mapper5.chr_size;
          mapper_data->gfxMode = g_NESmapper.Mapper5.gfx_mode;
          mapper_data->splitControl = g_NESmapper.Mapper5.split_control;
          mapper_data->splitBank = g_NESmapper.Mapper5.split_bank;
          mapper_data->last5205Write = g_NESmapper.Mapper5.value0;
          mapper_data->last5206Write = g_NESmapper.Mapper5.value1;
          mapper_data->wramBank3 = g_NESmapper.Mapper5.wb[3];
          mapper_data->wramBank4 = g_NESmapper.Mapper5.wb[4];
          mapper_data->wramBank5 = g_NESmapper.Mapper5.wb[5];
          mapper_data->wramBank6 = g_NESmapper.Mapper5.wb[6];
          mapper_data->vromBank[0][0] = g_NESmapper.Mapper5.chr_reg[0][0];
          mapper_data->vromBank[1][0] = g_NESmapper.Mapper5.chr_reg[1][0];
          mapper_data->vromBank[2][0] = g_NESmapper.Mapper5.chr_reg[2][0];
          mapper_data->vromBank[3][0] = g_NESmapper.Mapper5.chr_reg[3][0];
          mapper_data->vromBank[4][0] = g_NESmapper.Mapper5.chr_reg[4][0];
          mapper_data->vromBank[5][0] = g_NESmapper.Mapper5.chr_reg[5][0];
          mapper_data->vromBank[6][0] = g_NESmapper.Mapper5.chr_reg[6][0];
          mapper_data->vromBank[7][0] = g_NESmapper.Mapper5.chr_reg[7][0];
          mapper_data->vromBank[0][1] = g_NESmapper.Mapper5.chr_reg[0][1];
          mapper_data->vromBank[1][1] = g_NESmapper.Mapper5.chr_reg[1][1];
          mapper_data->vromBank[2][1] = g_NESmapper.Mapper5.chr_reg[2][1];
          mapper_data->vromBank[3][1] = g_NESmapper.Mapper5.chr_reg[3][1];
          mapper_data->vromBank[4][1] = g_NESmapper.Mapper5.chr_reg[4][1];
          mapper_data->vromBank[5][1] = g_NESmapper.Mapper5.chr_reg[5][1];
          mapper_data->vromBank[6][1] = g_NESmapper.Mapper5.chr_reg[6][1];
          mapper_data->vromBank[7][1] = g_NESmapper.Mapper5.chr_reg[7][1];
      }
        break;
        
      case 6: {
          struct mapper6Data* mapper_data = (struct mapper6Data*)&block->extraData;
          
          mapper_data->irqCounterLowByte = g_NESmapper.Mapper6.irq_counter & 0x00FF;
          mapper_data->irqCounterHighByte = (g_NESmapper.Mapper6.irq_counter & 0xFF00) >> 8;
          mapper_data->irqCounterEnabled = g_NESmapper.Mapper6.irq_enabled;
      }
        break;
        
      case 9: {
          struct mapper9Data* mapper_data = (struct mapper9Data*)&block->extraData;
          
          // 2 latch registers
          mapper_data->latch[0] = g_NESmapper.Mapper9.latch_0000;
          mapper_data->latch[1] = g_NESmapper.Mapper9.latch_1000;
          
          // regs (B/C/D/E000)
          mapper_data->lastB000Write = g_NESmapper.Mapper9.regs[1];
          mapper_data->lastC000Write = g_NESmapper.Mapper9.regs[2];
          mapper_data->lastD000Write = g_NESmapper.Mapper9.regs[3];
          mapper_data->lastE000Write = g_NESmapper.Mapper9.regs[4];
      }
        break;
        
      case 10: {
          struct mapper10Data* mapper_data = (struct mapper10Data*)&block->extraData;
          
          // 2 latch registers
          mapper_data->latch[0] = g_NESmapper.Mapper10.latch_0000;
          mapper_data->latch[1] = g_NESmapper.Mapper10.latch_1000;
          
          // regs (B/C/D/E000)
          mapper_data->lastB000Write = g_NESmapper.Mapper10.regs[1];
          mapper_data->lastC000Write = g_NESmapper.Mapper10.regs[2];
          mapper_data->lastD000Write = g_NESmapper.Mapper10.regs[3];
          mapper_data->lastE000Write = g_NESmapper.Mapper10.regs[4];
      }
        break;
        
      case 13: {
          struct mapper13Data* mapper_data = (struct mapper13Data*)&block->extraData;
          
          mapper_data->wramBank = g_NESmapper.Mapper13.chr_bank;
      }
        break;
        
	case 16:
		{
			struct mapper16Data* mapper_data = (struct mapper16Data*)&block->extraData;

			mapper_data->irqCounterLowByte = g_NESmapper.Mapper16.irq_counter & 0x00FF;
			mapper_data->irqCounterHighByte = (g_NESmapper.Mapper16.irq_counter & 0xFF00) >> 8;
			mapper_data->irqCounterEnabled = g_NESmapper.Mapper16.irq_enabled;
			mapper_data->irqLatchCounterLowByte = g_NESmapper.Mapper16.irq_latch & 0x00FF;
			mapper_data->irqLatchCounterHighByte = (g_NESmapper.Mapper16.irq_latch & 0xFF00) >> 8;
		}
		break;

	case 17:
		{
			struct mapper17Data* mapper_data = (struct mapper17Data*)&block->extraData;

			mapper_data->irqCounterLowByte = g_NESmapper.Mapper17.irq_counter & 0x00FF;
			mapper_data->irqCounterHighByte = (g_NESmapper.Mapper17.irq_counter & 0xFF00) >> 8;
			mapper_data->irqCounterEnabled = g_NESmapper.Mapper17.irq_enabled;
		}
		break;

	case 18:
		{
			struct mapper18Data* mapper_data = (struct mapper18Data*)&block->extraData;

			mapper_data->irqCounterLowByte = g_NESmapper.Mapper18.irq_counter & 0x00FF;
			mapper_data->irqCounterHighByte = (g_NESmapper.Mapper18.irq_counter & 0xFF00) >> 8;
			mapper_data->irqCounterEnabled = g_NESmapper.Mapper18.irq_enabled;
		}
		break;

	case 19:
		{
			struct mapper19Data* mapper_data = (struct mapper19Data*)&block->extraData;

			mapper_data->irqCounterLowByte = g_NESmapper.Mapper19.irq_counter & 0x00FF;
			mapper_data->irqCounterHighByte = (g_NESmapper.Mapper19.irq_counter & 0xFF00) >> 8;
			mapper_data->irqCounterEnabled = g_NESmapper.Mapper19.irq_enabled;
			mapper_data->lastE800Write = (g_NESmapper.Mapper19.regs[0] & 0x01) << 6;
			mapper_data->lastE800Write |= (g_NESmapper.Mapper19.regs[1] & 0x01) << 7;
			mapper_data->lastF800Write = g_NESmapper.Mapper19.regs[2];
		}
		break;

	case 20:
		{
			struct mapper20Data* mapper_data = (struct mapper20Data*)&block->extraData;

			mapper_data->irqCounterLowByte = g_NESmapper.Mapper20.irq_counter & 0x00FF;
			mapper_data->irqCounterHighByte = (g_NESmapper.Mapper20.irq_counter & 0xFF00) >> 8;
			mapper_data->irqCounterEnabled = g_NESmapper.Mapper20.irq_enabled;
			mapper_data->irqLatchCounterLowByte = g_NESmapper.Mapper20.irq_latch & 0x00FF;
			mapper_data->irqLatchCounterHighByte = (g_NESmapper.Mapper20.irq_latch & 0xFF00) >> 8;
			mapper_data->irqWaitCounter = g_NESmapper.Mapper20.irq_wait;
			mapper_data->last4023Write = g_NESmapper.Mapper20.disk_enabled;
			mapper_data->last4025Write = g_NESmapper.Mapper20.write_reg;
			mapper_data->HeadPositionLowByte = g_NESmapper.Mapper20.head_position & 0x00FF;
			mapper_data->HeadPositionHighByte = (g_NESmapper.Mapper20.head_position & 0xFF00) >> 8;
			mapper_data->DiskStatus = g_NESmapper.Mapper20.disk_status;
			mapper_data->WriteSkip = g_NESmapper.Mapper20.write_skip;
			mapper_data->CurrentDiskSide = g_NESmapper.Mapper20.current_side;
			mapper_data->LastDiskSide = g_NESmapper.Mapper20.last_side;
			mapper_data->DiskInsertWait = g_NESmapper.Mapper20.insert_wait;
		}
		break;

	case 21:
		{
			struct mapper21Data* mapper_data = (struct mapper21Data*)&block->extraData;

			mapper_data->irqCounter = g_NESmapper.Mapper21.irq_counter;
			mapper_data->irqCounterEnabled = g_NESmapper.Mapper21.irq_enabled;
			mapper_data->irqLatchCounter = g_NESmapper.Mapper21.irq_latch;
			mapper_data->last9002Write = g_NESmapper.Mapper21.regs[8];
		}
		break;

	case 23:
		{
			struct mapper23Data* mapper_data = (struct mapper23Data*)&block->extraData;

			mapper_data->irqCounter = g_NESmapper.Mapper23.irq_counter;
			mapper_data->irqCounterEnabled = g_NESmapper.Mapper23.irq_enabled;
			mapper_data->irqLatchCounter = g_NESmapper.Mapper23.irq_latch;
			mapper_data->last9008Write = g_NESmapper.Mapper23.regs[8];
		}
		break;

	case 24:
		{
			struct mapper24Data* mapper_data = (struct mapper24Data*)&block->extraData;

			mapper_data->irqCounter = g_NESmapper.Mapper24.irq_counter;
			mapper_data->irqCounterEnabled = g_NESmapper.Mapper24.irq_enabled;
			mapper_data->irqLatchCounter = g_NESmapper.Mapper24.irq_latch;
		}
		break;

	case 25:
		{
			struct mapper25Data* mapper_data = (struct mapper25Data*)&block->extraData;

			mapper_data->irqCounter = g_NESmapper.Mapper25.irq_counter;
			mapper_data->irqCounterEnabled = g_NESmapper.Mapper25.irq_enabled;
			mapper_data->irqLatchCounter = g_NESmapper.Mapper25.irq_latch;
			mapper_data->last9001Write = g_NESmapper.Mapper25.regs[10];
		}
		break;

	case 26:
		{
			struct mapper26Data* mapper_data = (struct mapper26Data*)&block->extraData;

			mapper_data->irqCounter = g_NESmapper.Mapper26.irq_counter;
			mapper_data->irqCounterEnabled = g_NESmapper.Mapper26.irq_enabled;
			mapper_data->irqLatchCounter = g_NESmapper.Mapper26.irq_latch;
		}
		break;

	case 32:
		{
			struct mapper32Data* mapper_data = (struct mapper32Data*)&block->extraData;

			mapper_data->last9000Write = g_NESmapper.Mapper32.regs[0];
		}
		break;

	case 33:
		{
			struct mapper33Data* mapper_data = (struct mapper33Data*)&block->extraData;

			mapper_data->irqCounter = g_NESmapper.Mapper33.irq_counter;
			mapper_data->irqCounterEnabled = g_NESmapper.Mapper33.irq_enabled;
		}
		break;

	case 40:
		{
			struct mapper40Data* mapper_data = (struct mapper40Data*)&block->extraData;

			// IRQ counter
			mapper_data->irqCounter = g_NESmapper.Mapper40.lines_to_irq;

			// IRQ enabled
			mapper_data->irqCounterEnabled = g_NESmapper.Mapper40.irq_enabled;
		}
		break;

	case 41:
		{
			struct mapper41Data* mapper_data = (struct mapper41Data*)&block->extraData;

			mapper_data->last6000Write = g_NESmapper.Mapper41.regs[0];
		}
		break;

	case 42:
		{
			struct mapper42Data* mapper_data = (struct mapper42Data*)&block->extraData;

			mapper_data->irqCounter = g_NESmapper.Mapper42.irq_counter;
			mapper_data->irqCounterEnabled = g_NESmapper.Mapper42.irq_enabled;
		}
		break;

	case 43:
		{
			struct mapper43Data* mapper_data = (struct mapper43Data*)&block->extraData;

			mapper_data->irqCounterLowByte = g_NESmapper.Mapper43.irq_counter & 0x00FF;
			mapper_data->irqCounterHighByte = (g_NESmapper.Mapper43.irq_counter & 0xFF00) >> 8;
			mapper_data->irqCounterEnabled = g_NESmapper.Mapper43.irq_enabled;
		}
		break;

	case 46:
		{
			struct mapper46Data* mapper_data = (struct mapper46Data*)&block->extraData;

			mapper_data->last6000Write = g_NESmapper.Mapper46.regs[0] | (g_NESmapper.Mapper46.regs[1] << 4);
			mapper_data->last8000Write = g_NESmapper.Mapper46.regs[2] | (g_NESmapper.Mapper46.regs[1] << 4);
		}
		break;

	case 48:
		{
			struct mapper48Data* mapper_data = (struct mapper48Data*)&block->extraData;

			mapper_data->lastE000Write = g_NESmapper.Mapper48.regs[0];
		}
		break;

	case 49:
		{
			struct mapper49Data* mapper_data = (struct mapper49Data*)&block->extraData;

			mapper_data->irqCounter = g_NESmapper.Mapper49.irq_counter;
			mapper_data->irqLatchCounter = g_NESmapper.Mapper49.irq_latch;
			mapper_data->irqCounterEnabled = g_NESmapper.Mapper49.irq_enabled;
			mapper_data->last8000Write = g_NESmapper.Mapper49.regs[0];
			mapper_data->last6000Write = g_NESmapper.Mapper49.regs[1];
			mapper_data->lastA001Write = g_NESmapper.Mapper49.regs[2];
		}
		break;

	case 50:
		{
			struct mapper50Data* mapper_data = (struct mapper50Data*)&block->extraData;

			mapper_data->irqCounterEnabled = g_NESmapper.Mapper50.irq_enabled;
		}
		break;

	case 51:
		{
			struct mapper51Data* mapper_data = (struct mapper51Data*)&block->extraData;

			mapper_data->BankSelect = g_NESmapper.Mapper51.bank;
			mapper_data->MapperMode = g_NESmapper.Mapper51.mode;
		}
		break;

	case 57:
		{
			struct mapper57Data* mapper_data = (struct mapper57Data*)&block->extraData;

			mapper_data->last8800Write = g_NESmapper.Mapper57.regs[0];
		}
		break;

	case 64:
		{
			struct mapper64Data* mapper_data = (struct mapper64Data*)&block->extraData;

			mapper_data->irqCounter = g_NESmapper.Mapper64.irq_counter;
			mapper_data->irqCounterEnabled = g_NESmapper.Mapper64.irq_enabled;
			mapper_data->irqLatchCounter = g_NESmapper.Mapper64.irq_latch;
			mapper_data->last8000Write = g_NESmapper.Mapper64.regs[0] | g_NESmapper.Mapper64.regs[1] | g_NESmapper.Mapper64.regs[2];
		}
		break;

	case 65:
		{
			struct mapper65Data* mapper_data = (struct mapper65Data*)&block->extraData;

			mapper_data->irqCounterLowByte = g_NESmapper.Mapper65.irq_counter & 0x00FF;
			mapper_data->irqCounterHighByte = (g_NESmapper.Mapper65.irq_counter & 0xFF00) >> 8;
			mapper_data->irqCounterEnabled = g_NESmapper.Mapper65.irq_enabled;
			mapper_data->irqLatchCounterLowByte = g_NESmapper.Mapper65.irq_latch & 0x00FF;
			mapper_data->irqLatchCounterHighByte = (g_NESmapper.Mapper65.irq_latch & 0xFF00) >> 8;
		}
		break;

	case 67:
		{
			struct mapper67Data* mapper_data = (struct mapper67Data*)&block->extraData;

			mapper_data->irqCounter = g_NESmapper.Mapper67.irq_counter;
			mapper_data->irqCounterEnabled = g_NESmapper.Mapper67.irq_enabled;
			mapper_data->irqLatchCounter = g_NESmapper.Mapper67.irq_latch;
		}
		break;

	case 68:
		{
			struct mapper68Data* mapper_data = (struct mapper68Data*)&block->extraData;

			mapper_data->lastC000Write = g_NESmapper.Mapper68.regs[2];
			mapper_data->lastD000Write = g_NESmapper.Mapper68.regs[3];
			mapper_data->lastE000Write = (g_NESmapper.Mapper68.regs[0] << 4) | g_NESmapper.Mapper68.regs[1];
		}
		break;

	case 69:
		{
			struct mapper69Data* mapper_data = (struct mapper69Data*)&block->extraData;

			mapper_data->irqCounterLowByte = g_NESmapper.Mapper69.irq_counter & 0x00FF;
			mapper_data->irqCounterHighByte = (g_NESmapper.Mapper69.irq_counter & 0xFF00) >> 8;
			mapper_data->irqCounterEnabled = g_NESmapper.Mapper69.irq_enabled;
			mapper_data->last8000Write = g_NESmapper.Mapper69.regs[0];
		}
		break;

	case 73:
		{
			struct mapper73Data* mapper_data = (struct mapper73Data*)&block->extraData;

			mapper_data->irqCounterLowByte = g_NESmapper.Mapper73.irq_counter & 0x00FF;
			mapper_data->irqCounterHighByte = (g_NESmapper.Mapper73.irq_counter & 0xFF00) >> 8;
			mapper_data->irqCounterEnabled = g_NESmapper.Mapper73.irq_enabled;
		}
		break;

	case 75:
		{
			struct mapper75Data* mapper_data = (struct mapper75Data*)&block->extraData;

			mapper_data->lastE000Write = g_NESmapper.Mapper75.regs[0];
			mapper_data->lastF000Write = g_NESmapper.Mapper75.regs[1];
		}
		break;

	case 76:
		{
			struct mapper76Data* mapper_data = (struct mapper76Data*)&block->extraData;

			mapper_data->last8000Write = g_NESmapper.Mapper76.regs[0];
		}
		break;

	case 82:
		{
			struct mapper82Data* mapper_data = (struct mapper82Data*)&block->extraData;

			mapper_data->last7EF6Write = g_NESmapper.Mapper82.regs[0];
		}
		break;

	case 83:
		{
			struct mapper83Data* mapper_data = (struct mapper83Data*)&block->extraData;

			mapper_data->irqCounterLowByte = g_NESmapper.Mapper83.irq_counter & 0x00FF;
			mapper_data->irqCounterHighByte = (g_NESmapper.Mapper83.irq_counter & 0xFF00) >> 8;
			mapper_data->irqCounterEnabled = g_NESmapper.Mapper83.irq_enabled;
			mapper_data->last8000Write = g_NESmapper.Mapper83.regs[0];
			mapper_data->last8100Write = g_NESmapper.Mapper83.regs[1];
			mapper_data->last5101Write = g_NESmapper.Mapper83.regs[2];
		}
		break;

	case 85:
		{
			struct mapper85Data* mapper_data = (struct mapper85Data*)&block->extraData;

			mapper_data->irqCounter = g_NESmapper.Mapper85.irq_counter;
			mapper_data->irqCounterEnabled = g_NESmapper.Mapper85.irq_enabled;
			mapper_data->irqLatchCounter = g_NESmapper.Mapper85.irq_latch;
		}
		break;

	case 88:
		{
			struct mapper88Data* mapper_data = (struct mapper88Data*)&block->extraData;

			mapper_data->last8000Write = g_NESmapper.Mapper88.regs[0];
		}
		break;

	case 91:
		{
			struct mapper91Data* mapper_data = (struct mapper91Data*)&block->extraData;

			mapper_data->irqCounter = g_NESmapper.Mapper91.irq_counter;
			mapper_data->irqCounterEnabled = g_NESmapper.Mapper91.irq_enabled;
		}
		break;

	case 95:
		{
			struct mapper95Data* mapper_data = (struct mapper95Data*)&block->extraData;

			mapper_data->last8000Write = g_NESmapper.Mapper95.regs[0];
		}
		break;

	case 96:
		{
			struct mapper96Data* mapper_data = (struct mapper96Data*)&block->extraData;

			mapper_data->wramBank = g_NESmapper.Mapper96.vbank0;
		}
		break;

	case 105:
		{
			struct mapper105Data* mapper_data = (struct mapper105Data*)&block->extraData;

			mapper_data->irqCounter[0] = (g_NESmapper.Mapper105.irq_counter &0x000000ff) >> 0;
			mapper_data->irqCounter[1] = (g_NESmapper.Mapper105.irq_counter &0x0000ff00) >> 8;
			mapper_data->irqCounter[2] = (g_NESmapper.Mapper105.irq_counter &0x00ff0000) >> 16;
			mapper_data->irqCounter[3] = (g_NESmapper.Mapper105.irq_counter &0xff000000) >> 24;
			mapper_data->irqCounterEnabled = g_NESmapper.Mapper105.irq_enabled;
			mapper_data->InitialCounter = g_NESmapper.Mapper105.init_state;
			mapper_data->WriteCounter = g_NESmapper.Mapper105.write_count;
			mapper_data->Bits = g_NESmapper.Mapper105.bits;
			mapper_data->registers[0] = g_NESmapper.Mapper105.regs[0];
			mapper_data->registers[1] = g_NESmapper.Mapper105.regs[1];
			mapper_data->registers[2] = g_NESmapper.Mapper105.regs[2];
			mapper_data->registers[3] = g_NESmapper.Mapper105.regs[3];
		}
		break;

	case 117:
		{
			struct mapper117Data* mapper_data = (struct mapper117Data*)&block->extraData;

			mapper_data->irqLine = g_NESmapper.Mapper117.irq_line;
			mapper_data->irqEnabled1 = g_NESmapper.Mapper117.irq_enabled1;
			mapper_data->irqEnabled2 = g_NESmapper.Mapper117.irq_enabled2;
		}
		break;

	case 160:
		{
			struct mapper160Data* mapper_data = (struct mapper160Data*)&block->extraData;

			mapper_data->irqCounter = g_NESmapper.Mapper160.irq_counter;
			mapper_data->irqCounterEnabled = g_NESmapper.Mapper160.irq_enabled;
			mapper_data->irqLatchCounter = g_NESmapper.Mapper160.irq_latch;
			mapper_data->RefreshType = g_NESmapper.Mapper160.refresh_type;
		}
		break;

	case 182:
		{
			struct mapper182Data* mapper_data = (struct mapper182Data*)&block->extraData;

			mapper_data->irqCounter = g_NESmapper.Mapper182.irq_counter;
			mapper_data->irqCounterEnabled = g_NESmapper.Mapper182.irq_enabled;
			mapper_data->lastA000Write = g_NESmapper.Mapper182.regs[0];
		}
		break;

	case 189:
		{
			struct mapper189Data* mapper_data = (struct mapper189Data*)&block->extraData;

			mapper_data->irqCounter = g_NESmapper.Mapper189.irq_counter;
			mapper_data->irqCounterEnabled = g_NESmapper.Mapper189.irq_enabled;
			mapper_data->irqLatchCounter = g_NESmapper.Mapper189.irq_latch;
			mapper_data->last8000Write = g_NESmapper.Mapper189.regs[0];
		}
		break;

	case 226:
		{
			struct mapper226Data* mapper_data = (struct mapper226Data*)&block->extraData;

			mapper_data->registers[0] = g_NESmapper.Mapper226.regs[0];
			mapper_data->registers[1] = g_NESmapper.Mapper226.regs[1];
		}
		break;

	case 230:
		{
			struct mapper230Data* mapper_data = (struct mapper230Data*)&block->extraData;

			mapper_data->numberOfResets = g_NESmapper.Mapper230.rom_switch;
		}
		break;

	case 232:
		{
			struct mapper232Data* mapper_data = (struct mapper232Data*)&block->extraData;

			mapper_data->last9000Write = g_NESmapper.Mapper232.regs[0];
			mapper_data->lastA000Write = g_NESmapper.Mapper232.regs[1];
		}
		break;

	case 234:
		{
			struct mapper234Data* mapper_data = (struct mapper234Data*)&block->extraData;

			mapper_data->lastFF80Write = g_NESmapper.Mapper234.regs[0];
			mapper_data->lastFFE8Write = g_NESmapper.Mapper234.regs[1];
			mapper_data->lastFFC0Write = g_NESmapper.Mapper234.regs[2];
		}
		break;

	case 236:
		{
			struct mapper236Data* mapper_data = (struct mapper236Data*)&block->extraData;

			mapper_data->BankSelect = g_NESmapper.Mapper236.bank;
			mapper_data->MapperMode = g_NESmapper.Mapper236.mode;
		}
		break;

	case 243:
		{
			struct mapper243Data* mapper_data = (struct mapper243Data*)&block->extraData;

			mapper_data->registers[0] = g_NESmapper.Mapper243.regs[0];
			mapper_data->registers[1] = g_NESmapper.Mapper243.regs[1];
			mapper_data->registers[2] = g_NESmapper.Mapper243.regs[2];
			mapper_data->registers[3] = g_NESmapper.Mapper243.regs[3];
		}
		break;

	case 255:
		{
			struct mapper255Data* mapper_data = (struct mapper255Data*)&block->extraData;

			mapper_data->registers[0] = g_NESmapper.Mapper255.regs[0];
			mapper_data->registers[1] = g_NESmapper.Mapper255.regs[1];
			mapper_data->registers[2] = g_NESmapper.Mapper255.regs[2];
			mapper_data->registers[3] = g_NESmapper.Mapper255.regs[3];
		}
		break;
	}

	return 0;
}

static void extract_ExMPRD(int fn, NES* nes)
{
#if 1
    HANDLE hFile = (HANDLE)fn;
    char h_wram0[12]="WRAM\x00\x00\x00\x01\x00\x01\x00\x00";
    char h_wram1[12]="WRAM\x00\x00\x00\x01\x00\x00\x80\x00";
    char fds1[12]="FDS1\x00\x00\x00\x01\x00\x00\xFF\xDC";
    char fds2[12]="FDS2\x00\x00\x00\x01\x00\x00\xFF\xDC";
    char fds3[12]="FDS3\x00\x00\x00\x01\x00\x00\xFF\xDC";
    char fds4[12]="FDS4\x00\x00\x00\x01\x00\x00\xFF\xDC";
#else
//    HANDLE hFile = NES_fopen(fn, FILE_MODE_APPEND);
#endif


    if(NES_ROM_get_mapper_num() == 1 && NES_crc32() == 0xb8747abf) {
        // Best Play - Pro Yakyuu Special (J)
#if 1
        HAL_sts_write(hFile,h_wram0,sizeof(h_wram0));
        HAL_sts_write(hFile,&g_NES.SaveRAM[0x2000],0x2000);
#else
//		// save WRAM
//      NES_fputc('W',  hFile); NES_fputc('R',  hFile); NES_fputc('A',  hFile); NES_fputc('M',  hFile);
//      NES_fputc(0x00, hFile); NES_fputc(0x00, hFile); NES_fputc(0x00, hFile); NES_fputc(0x01, hFile);
//      NES_fputc(0x00, hFile); NES_fputc(0x01, hFile); NES_fputc(0x00, hFile); NES_fputc(0x00, hFile);
//      NES_fwrite(&g_NES.SaveRAM[0x2000], 1, 0x2000, hFile);
#endif
    }
    else if(NES_ROM_get_mapper_num() == 5) {
        // save WRAM
#if 1
        HAL_sts_write(hFile,h_wram0,sizeof(h_wram0));
		HAL_sts_write(hFile,g_NESmapper.Mapper5.wram, 0x10000);
#else
//        NES_fputc('W',  hFile); NES_fputc('R',  hFile); NES_fputc('A',  hFile); NES_fputc('M',  hFile);
//		NES_fputc(0x00, hFile); NES_fputc(0x00, hFile); NES_fputc(0x00, hFile); NES_fputc(0x01, hFile);
//		NES_fputc(0x00, hFile); NES_fputc(0x01, hFile); NES_fputc(0x00, hFile); NES_fputc(0x00, hFile);
//		NES_fwrite(g_NESmapper.Mapper5.wram, 1, 0x10000, hFile);
#endif
	}
    else if(NES_ROM_get_mapper_num() == 20) {
        // save WRAM
#if 1
        HAL_sts_write(hFile,h_wram1,sizeof(h_wram1));
		HAL_sts_write(hFile,g_NESmapper.Mapper20.wram, 0x8000);
#else
//		NES_fputc('W',  hFile); NES_fputc('R',  hFile); NES_fputc('A',  hFile); NES_fputc('M',  hFile);
//		NES_fputc(0x00, hFile); NES_fputc(0x00, hFile); NES_fputc(0x00, hFile); NES_fputc(0x01, hFile);
//		NES_fputc(0x00, hFile); NES_fputc(0x00, hFile); NES_fputc(0x80, hFile); NES_fputc(0x00, hFile);
//		NES_fwrite(g_NESmapper.Mapper20.wram, 1, 0x8000, hFile);
#endif

#if 1
        // write Disk #1 - #4
        HAL_sts_write(hFile,fds1,sizeof(fds1));
        HAL_sts_write(hFile,g_NESmapper.Mapper20.disk, 65500*4);
        
        HAL_sts_write(hFile,fds2,sizeof(fds2));
        HAL_sts_write(hFile,fds3,sizeof(fds3));
        HAL_sts_write(hFile,fds4,sizeof(fds4));
#else
//        // write Disk #1 - #4
//        NES_fputc('F',  hFile); NES_fputc('D',  hFile); NES_fputc('S',  hFile); NES_fputc('1',  hFile);
//        NES_fputc(0x00, hFile); NES_fputc(0x00, hFile); NES_fputc(0x00, hFile); NES_fputc(0x01, hFile);
//        NES_fputc(0x00, hFile); NES_fputc(0x00, hFile); NES_fputc(0xFF, hFile); NES_fputc(0xDC, hFile);
//        NES_fwrite(g_NESmapper.Mapper20.disk, 65500*4, 1, hFile);
//        
//        NES_fputc('F',  hFile); NES_fputc('D',  hFile); NES_fputc('S',  hFile); NES_fputc('2',  hFile);
//        NES_fputc(0x00, hFile); NES_fputc(0x00, hFile); NES_fputc(0x00, hFile); NES_fputc(0x01, hFile);
//        NES_fputc(0x00, hFile); NES_fputc(0x00, hFile); NES_fputc(0xFF, hFile); NES_fputc(0xDC, hFile);
//        
//        NES_fputc('F',  hFile); NES_fputc('D',  hFile); NES_fputc('S',  hFile); NES_fputc('3',  hFile);
//        NES_fputc(0x00, hFile); NES_fputc(0x00, hFile); NES_fputc(0x00, hFile); NES_fputc(0x01, hFile);
//        NES_fputc(0x00, hFile); NES_fputc(0x00, hFile); NES_fputc(0xFF, hFile); NES_fputc(0xDC, hFile);
//        
//        NES_fputc('F',  hFile); NES_fputc('D',  hFile); NES_fputc('S',  hFile); NES_fputc('4',  hFile);
//        NES_fputc(0x00, hFile); NES_fputc(0x00, hFile); NES_fputc(0x00, hFile); NES_fputc(0x01, hFile);
//        NES_fputc(0x00, hFile); NES_fputc(0x00, hFile); NES_fputc(0xFF, hFile); NES_fputc(0xDC, hFile);
#endif

	}
    //999NES_fclose(hFile);
}

static int extract_CNTR(SnssControllersBlock* block, NES* nes)
{
	return -1;
}

static int extract_SOUN(SnssSoundBlock* block, NES* nes)
{
	// get sound registers
	NES_APU_get_regs(block->soundRegisters);

	return 0;
}

#if 1 //3333
u8 SaveSNSS(int fn, NES* nes)
{
	SNSS_FILE* snssFile;
	DEBUG("Enter SaveSNSS");

    if(SNSS_OK != SNSS_OpenFile(&snssFile, fn, SNSS_OPEN_WRITE)) {
        goto OpenError;
    }
    
    // write BASR
    if(!extract_BASR(&snssFile->baseBlock, nes)) {
        if(SNSS_OK != SNSS_WriteBlock(snssFile, SNSS_BASR)) {
            goto WriteError;
        }
    }
    
    // write VRAM
    if(!extract_VRAM(&snssFile->vramBlock, nes)) {
        if(SNSS_OK != SNSS_WriteBlock(snssFile, SNSS_VRAM)) {
            goto WriteError;
        }
    }

	// write SRAM
    if(!extract_SRAM(&snssFile->sramBlock, nes)) {
        if(SNSS_OK != SNSS_WriteBlock(snssFile, SNSS_SRAM)) {
            goto WriteError;
        }
    }
    
	// write MPRD
    if(!extract_MPRD(&snssFile->mapperBlock, nes)) {
        if(SNSS_OK != SNSS_WriteBlock(snssFile, SNSS_MPRD))
          goto WriteError;
    }
    
    // write CNTR
    if(!extract_CNTR(&snssFile->contBlock, nes)) {
        if(SNSS_OK != SNSS_WriteBlock(snssFile, SNSS_CNTR))
          goto WriteError;
    }

    // write SOUN
    if(!extract_SOUN(&snssFile->soundBlock, nes)) {
        if(SNSS_OK != SNSS_WriteBlock(snssFile, SNSS_SOUN)) {
            goto WriteError;
        }
	}

    //if(SNSS_OK != SNSS_CloseFile(&snssFile))
	//    goto CloseError;
    
	// write Extra Mapper Data
	extract_ExMPRD(fn, nes);

	// ヘッダーをクローズしなくては！
	SNSS_CloseFile(&snssFile);

	DEBUG("Saved done");

	return TRUE;
CloseError:
WriteError:
OpenError:
	DEBUG("Error writing");
	if(snssFile) SNSS_CloseFile(&snssFile);
	return FALSE;
}
#endif // 3333


//mem//#if 1 // mem
//mem//inline void SetStateChunkHeader(u32 *Memp, int Code, int Size){
//mem//#if 1
//mem//    core_memcpy(&Memp[0],&Code,sizeof(Code));
//mem//    core_memcpy(&Memp[1],&Size,sizeof(Size));
//mem//    core_memset(&Memp[2],0,4);
//mem//    core_memset(&Memp[3],0,4);
//mem//#else
//mem//	Memp[0] = Code;
//mem//	Memp[1] = Size;
//mem//	Memp[2] = 0;
//mem//	Memp[3] = 0;
//mem//#endif
//mem//}
//mem//
//mem//
//mem//static int extractMem_ExMPRD(unsigned char **extm, u32 nsize, NES* nes)
//mem//{
//mem//	int size=0;
//mem//	unsigned char *extp, *tmem;
//mem//
//mem//	if(NES_ROM_get_mapper_num() == 1 && NES_crc32() == 0xb8747abf)
//mem//	{
//mem//		// Best Play - Pro Yakyuu Special (J)
//mem//		// save WRAM
//mem//		tmem = (unsigned char *)realloc(*extm, nsize + 0x2000+0x10);
//mem//		if(tmem==NULL)
//mem//			return 0;
//mem//		extp=tmem+nsize;
//mem//		SetStateChunkHeader((u32 *)extp, 0x44504D45, 0x2000);
//mem//		core_memcpy(extm+0x10, &g_NES.SaveRAM[0x2000], 0x2000);
//mem//		size = 0x2000+0x10;
//mem//	}
//mem//	else if(NES_ROM_get_mapper_num() == 5)
//mem//	{
//mem//		// save WRAM
//mem//		tmem = (unsigned char *)realloc(*extm, nsize + 0x10000+0x10);
//mem//		if(tmem==NULL)
//mem//			return 0;
//mem//		extp=tmem+nsize;
//mem//		SetStateChunkHeader((u32 *)extp, 0x44504D45, 0x10000);
//mem//		core_memcpy(extp+0x10, g_NESmapper.Mapper5.wram, 0x10000);
//mem//		size = 0x10000+0x10;
//mem//	}
//mem//	else if(NES_ROM_get_mapper_num() == 20)
//mem//	{
//mem//		// save WRAM
//mem//		tmem = (unsigned char *)realloc(*extm, nsize + 0x8000+(65500*4)+0x10);
//mem//		if(tmem==NULL)
//mem//			return 0;
//mem//		extp=tmem+nsize;
//mem//		SetStateChunkHeader((u32 *)extp, 0x44504D45, 0x8000+(65500*4));
//mem//		core_memcpy(extp+0x10, g_NESmapper.Mapper20.wram, 0x8000);
//mem//		extp+=0x10;
//mem//
//mem//		// write Disk #1 - #4
//mem//		core_memcpy(&extp[0x8000], g_NESmapper.Mapper20.disk, 65500);
//mem//		core_memcpy(&extp[0x8000+65500], &g_NESmapper.Mapper20.disk[0x10000], 65500);
//mem//		core_memcpy(&extp[0x8000+65500*2], &g_NESmapper.Mapper20.disk[0x20000], 65500);
//mem//		core_memcpy(&extp[0x8000+65500*3], &g_NESmapper.Mapper20.disk[0x30000], 65500);
//mem//		size = 0x8000+65500*4+0x10;
//mem//	}
//mem//	else
//mem//		return 0;
//mem//	*extm=tmem;
//mem//	return size;
//mem//}
//mem//
//mem//static int adoptMem_ExMPRD(unsigned char *extm, u32 nsize, NES* nes)
//mem//{
//mem//	//  u32 i;
//mem//	int size=0;
//mem//
//mem//	if(extm==NULL)
//mem//		return 0;
//mem//	if(NES_ROM_get_mapper_num() == 1 && NES_crc32() == 0xb8747abf)
//mem//	{
//mem//		// Best Play - Pro Yakyuu Special (J)
//mem//		// copy WRAM
//mem//		core_memcpy(&g_NES.SaveRAM[0x2000],extm, 0x2000);
//mem//		size = 0x2000;
//mem//	}
//mem//	else if(NES_ROM_get_mapper_num() == 5)
//mem//	{
//mem//		// copy WRAM
//mem//		core_memcpy(g_NESmapper.Mapper5.wram, extm, 0x10000);
//mem//		core_memcpy(g_NES.SaveRAM, extm, 0x10000);
//mem//		size = 0x10000;
//mem//	}
//mem//	else if(NES_ROM_get_mapper_num() == 20)
//mem//	{
//mem//		// copy WRAM
//mem//		core_memcpy(g_NESmapper.Mapper20.wram, extm, 0x8000);
//mem//		// read Disk #1 - #4
//mem//		core_memcpy(g_NESmapper.Mapper20.disk, &extm[0x8000], 65500);
//mem//		core_memcpy(&g_NESmapper.Mapper20.disk[0x10000], &extm[0x8000+65500], 65500);
//mem//		core_memcpy(&g_NESmapper.Mapper20.disk[0x20000], &extm[0x8000+65500*2], 65500);
//mem//		core_memcpy(&g_NESmapper.Mapper20.disk[0x30000], &extm[0x8000+65500*3], 65500);
//mem//		size = 0x8000+65500*4;
//mem//	}
//mem//	return size;
//mem//}
//mem//
//mem//
//mem//
//mem//
//mem//int MemSaveSNSS(unsigned char **ssp, NES* nes){
//mem//	unsigned char *snss=NULL, *p;
//mem//	int ssize=0;
//mem//
//mem//	if(*ssp){
//mem//        HAL_mem_free(*ssp);
//mem//        *ssp=NULL;
//mem//	}
//mem//	snss = (unsigned char *)HAL_mem_malloc(0x20+0x60+sizeof(SnssBaseBlock)+sizeof(SnssVramBlock)+sizeof(SnssSramBlock)+sizeof(SnssMapperBlock)+sizeof(SnssControllersBlock)+sizeof(SnssSoundBlock));
//mem//	if(snss==NULL)
//mem//		return 0;
//mem//	//	core_memset(snss, 0, 0x10+sizeof(SnssBaseBlock)+sizeof(SnssVramBlock)+sizeof(SnssSramBlock)+sizeof(SnssMapperBlock)+sizeof(SnssControllersBlock)+sizeof(SnssSoundBlock));
//mem//	core_memset(snss, 0, 0x20);
//mem//	p = snss+0x20;
//mem//
//mem//	SetStateChunkHeader((u32 *)p, 0x52534142, sizeof(SnssBaseBlock));
//mem//	if(!extract_BASR((SnssBaseBlock *)(p+0x10), nes)){
//mem//		ssize+=(sizeof(SnssBaseBlock)+0x10);
//mem//		p+=(sizeof(SnssBaseBlock)+0x10);
//mem//	}
//mem//	SetStateChunkHeader((u32 *)p, 0x4D415256, sizeof(SnssVramBlock));
//mem//	if(!extract_VRAM((SnssVramBlock *)(p+0x10), nes)){
//mem//		ssize+=(sizeof(SnssVramBlock)+0x10);
//mem//		p+=(sizeof(SnssVramBlock)+0x10);
//mem//	}
//mem//	SetStateChunkHeader((u32 *)p, 0x4D415253, sizeof(SnssSramBlock));
//mem//	if(!extract_SRAM((SnssSramBlock *)(p+0x10), nes)){
//mem//		ssize+=(sizeof(SnssSramBlock)+0x10);
//mem//		p+=(sizeof(SnssSramBlock)+0x10);
//mem//	}
//mem//	SetStateChunkHeader((u32 *)p, 0x4452504D, sizeof(SnssMapperBlock));
//mem//	if(!extract_MPRD((SnssMapperBlock *)(p+0x10), nes)){
//mem//		ssize+=(sizeof(SnssMapperBlock)+0x10);
//mem//		p+=(sizeof(SnssMapperBlock)+0x10);
//mem//	}
//mem//	SetStateChunkHeader((u32 *)p, 0x52544E43, sizeof(SnssControllersBlock));
//mem//	if(!extract_CNTR((SnssControllersBlock *)(p+0x10), nes)){
//mem//		ssize+=(sizeof(SnssControllersBlock)+0x10);
//mem//		p+=(sizeof(SnssControllersBlock)+0x10);
//mem//	}
//mem//	SetStateChunkHeader((u32 *)p, 0x4E554F53, sizeof(SnssSoundBlock));
//mem//	if(!extract_SOUN((SnssSoundBlock *)(p+0x10), nes)){
//mem//		ssize+=(sizeof(SnssSoundBlock)+0x10);
//mem//		p+=(sizeof(SnssSoundBlock)+0x10);
//mem//	}
//mem//	snss = (unsigned char *)realloc(snss, ssize+0x20);
//mem//	ssize += extractMem_ExMPRD(&snss, ssize+0x20,nes);
//mem//	*((u32 *)(void *)&snss[0]) = 0x54445453;
//mem//	*((u32 *)(void *)&snss[4]) = ssize;
//mem//	*ssp = snss;
//mem//
//mem//	return ssize;
//mem//}
//mem//
//mem//#if 1
//mem//int MemLoadSNSS(unsigned char *ssp , NES* nes){
//mem//	unsigned char *snss = ssp;
//mem//	u32 tsize, lsize=0;
//mem//
//mem//    u32 tag,tdw;
//mem//    
//mem//	if(ssp==NULL) return 0;
//mem//	if(*(u32 *)&ssp[0]!=0x54445453) return 0;
//mem//	tsize = *((u32 *)&ssp[4])+0x20;
//mem//	snss+=0x20;
//mem//	lsize = 0x20;
//mem//    
//mem//    
//mem//	while(lsize < tsize){
//mem//        
//mem//        core_memcpy(&tag,snss,4);
//mem//        
//mem//		switch(tag){
//mem//			case 0x52534142:
//mem//				adopt_BASR((SnssBaseBlock *)(snss+0x10), nes);
//mem//				break;
//mem//			case 0x4D415256:
//mem//				adopt_VRAM((SnssVramBlock *)(snss+0x10), nes);
//mem//				break;
//mem//			case 0x4D415253:
//mem//				adopt_SRAM((SnssSramBlock *)(snss+0x10), nes);
//mem//				break;
//mem//			case 0x4452504D:
//mem//				adopt_MPRD((SnssMapperBlock *)(snss+0x10), nes);
//mem//				break;
//mem//			case 0x52544E43:
//mem//				adopt_CNTR((SnssControllersBlock *)(snss+0x10), nes);
//mem//				break;
//mem//			case 0x4E554F53:
//mem//				adopt_SOUN((SnssSoundBlock *)(snss+0x10), nes);
//mem//				break;
//mem//			case 0x44504D45:
//mem//				adoptMem_ExMPRD(snss+0x10, *(u32 *)&snss[4], nes);
//mem//				break;
//mem//		}
//mem//
//mem//        core_memcpy(&tdw,&snss[4],4);
//mem//        tdw+=0x10;
//mem//        
//mem//		snss+=tdw;
//mem//		lsize+=tdw;
//mem//
//mem////		snss+=((*(u32 *)&snss[4])+0x10);
//mem////		lsize+=((*(u32 *)&snss[4])+0x10);
//mem//
//mem//	}
//mem//	return lsize;
//mem//}
//mem//#else
//mem////int MemLoadSNSS(unsigned char *ssp , NES* nes){
//mem////	unsigned char *snss = ssp;
//mem////	u32 tsize, lsize=0;
//mem////	if(ssp==NULL) return 0;
//mem////	if(*(u32 *)&ssp[0]!=0x54445453) return 0;
//mem////	tsize = *((u32 *)&ssp[4])+0x20;
//mem////	snss+=0x20;
//mem////	lsize = 0x20;
//mem////    
//mem////	while(lsize < tsize){
//mem////		switch(*(u32 *)snss){
//mem////			case 0x52534142:
//mem////				adopt_BASR((SnssBaseBlock *)(snss+0x10), nes);
//mem////				break;
//mem////			case 0x4D415256:
//mem////				adopt_VRAM((SnssVramBlock *)(snss+0x10), nes);
//mem////				break;
//mem////			case 0x4D415253:
//mem////				adopt_SRAM((SnssSramBlock *)(snss+0x10), nes);
//mem////				break;
//mem////			case 0x4452504D:
//mem////				adopt_MPRD((SnssMapperBlock *)(snss+0x10), nes);
//mem////				break;
//mem////			case 0x52544E43:
//mem////				adopt_CNTR((SnssControllersBlock *)(snss+0x10), nes);
//mem////				break;
//mem////			case 0x4E554F53:
//mem////				adopt_SOUN((SnssSoundBlock *)(snss+0x10), nes);
//mem////				break;
//mem////			case 0x44504D45:
//mem////				adoptMem_ExMPRD(snss+0x10, *(u32 *)&snss[4], nes);
//mem////				break;
//mem////		}
//mem////		u32 tdw = (*(u32 *)&snss[4])+0x10;
//mem////		snss+=tdw;
//mem////		lsize+=tdw;
//mem////
//mem//////		snss+=((*(u32 *)&snss[4])+0x10);
//mem//////		lsize+=((*(u32 *)&snss[4])+0x10);
//mem////
//mem////	}
//mem////	return lsize;
//mem////}
//mem//#endif
//mem//
//mem//#endif//mem


