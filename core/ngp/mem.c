//---------------------------------------------------------------------------
// NEOPOP : Emulator as in Dreamland
//
// Copyright (c) 2001-2002 by neopop_uk
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 2 of the License, or
//	(at your option) any later version. See also the license.txt file for
//	additional informations.
//---------------------------------------------------------------------------

#include "neopop.h"
#include "./tlcs-900h/interpret.h"
#include "Z80_interface.h"
#include "bios.h"
#include "gfx.h"
#include "mem.h"
#include "interrupt.h"
#include "sound.h"
#include "flash.h"
#include "cstring.h"

//=============================================================================

//Hack way of returning good EEPROM status.
BOOL eepromStatusEnable = FALSE;
static u32 eepromStatus;	

//u8 ram[1 + RAM_END - RAM_START];

BOOL debug_abort_memory = FALSE;
BOOL debug_mask_memory_error_messages = FALSE;

BOOL memory_unlock_flash_write = FALSE;
BOOL memory_flash_error = FALSE;
BOOL memory_flash_command = FALSE;

extern void ngp_make_map(void);

void ngp_memmap(void)
{
    core_memset(ngp_c,0,sizeof(NGP_CACHE));
    core_memset(ngp_m,0,sizeof(NGP_MEM));

    ngp_m->language_english = 0;

	ngp_make_map();
}

//=============================================================================

//=============================================================================
BYTE r_null[8]={0,0,0,0,0,0,0,0};
BYTE w_null[8]={0,0,0,0,0,0,0,0};

//#define RAM_START    0x000000
//#define RAM_END      0x00BFFF
//#define ROM_START    0x200000
//#define ROM_END      0x3FFFFF
//#define HIROM_START  0x800000
//#define HIROM_END    0x9FFFFF
void* translate_address_read(u32 address)
{
	address &= 0x00FFFFFF;

	//Get EEPROM status?
	switch(address & 0x00F00000){
	case 0x00000000:
		if(address>RAM_END) { return r_null; }

#if 0
		if(address==0x8008) { //RAS.H read (Simulated horizontal raster position)
			ram[0x8008] = (u8)((abs(TIMER_HINT_RATE/*515*/ - (int)timer_hint)) >> 2);
		}
#endif
		return &ram[address];
		break;

	case 0x00F00000:
		return &bios[address & 0xFFFF]; // BIOS ROM
		break;

	case 0x00200000:
		if (eepromStatusEnable) {
			eepromStatusEnable = FALSE;
			if (address == 0x220000 || address == 0x230000) {
				eepromStatus = 0xFFFFFFFF;
				return &eepromStatus;
			}
		}
	case 0x00300000:
		return &rom.data[(address & 0x1fffff)];
		break;
         
	case 0x00800000:
	case 0x00900000:
		return &rom.data[address-0x00600000];
		break;

	default:
		// ===================================
		//Signal a flash memory error
		if (memory_unlock_flash_write) {
			memory_flash_error = TRUE;
		}
		return r_null;
		break;
	}
}

//
//#define ROM_START     0x00200000
//#define ROM_END       0x003FFFFF
//#define HIROM_START   0x00800000
//#define HIROM_END     0x009FFFFF
//
void* translate_address_write(u32 address)
{	
	address &= 0x00FFFFFF;

	if (address <= RAM_END) {
		return &ram[address];
	}

	if (memory_unlock_flash_write) {

		//ROM (LOW)
		if (rom.data && address >= ROM_START && address <= ROM_END) {
			if (address <= ROM_START + rom.length) {
				return &rom.data[(address - ROM_START)];
			}
			return w_null;
		}

		//ROM (HIGH)
		if (rom.data && address >= HIROM_START && address <= HIROM_END) {
			if (address <= HIROM_START + (rom.length - 0x200000)) {
				return &rom.data[0x200000 + (address - HIROM_START)];
			}
			return w_null;
		}

		//Signal a flash memory error
		memory_flash_error = TRUE;
	}
	else {
		//ROM (LOW)
		if (rom.data && address >= ROM_START && address <= ROM_END) {
			//Ignore EEPROM commands
			if (address == 0x202AAA || address == 0x205555) {
				memory_flash_command = TRUE;
				return w_null;
			}

			//Set EEPROM status reading?
			if (address == 0x220000 || address == 0x230000) {
				eepromStatusEnable = TRUE;
				return w_null;
			}

			if (memory_flash_command) {
				//Write the 256byte block around the flash data
				flash_write(address & 0xFFFF00, 256);
				
				//Need to issue a new command before writing will work again.
				memory_flash_command = FALSE;
		
				//Write to the rom itself.
				if (address <= ROM_START + rom.length) {
					return &rom.data[(address - ROM_START)];
				}
			}
		}
	}

	return w_null;
}

//=============================================================================

void post_write(u32 address)
{
	address &= 0xFFFFFF;

#if defined(QPAL2)
    if(address>=0x8200 && address<=0x837f) {
		if(address<0x8280) ngp_c->qpal2 |= 0x01; else 
		if(address<0x8300) ngp_c->qpal2 |= 0x02;
		else               ngp_c->qpal2 |= 0x04;
    }
#endif

	//Direct Access to Sound Chips
	if ((*(u16*)(ram + 0xb8)) == 0xAA55){
		if (address == 0xA1) Write_SoundChipTone(ram[0xA1]);
		if (address == 0xA0) Write_SoundChipNoise(ram[0xA0]);
	}

	//DAC Write
	if (address == 0xA2)	dac_write();

	//Clear counters?
	if (address == 0x20){
		u8 TRUN = ram[0x20];
		if ((TRUN & 0x01) == 0) timer[0] = 0;
		if ((TRUN & 0x02) == 0) timer[1] = 0;
		if ((TRUN & 0x04) == 0) timer[2] = 0;
		if ((TRUN & 0x08) == 0) timer[3] = 0;
	}

	//z80 - NMI
	if (address == 0xBA) {
		Z80_nmi();
	}
}

//=============================================================================

u8 loadB(u32 address)
{
#if 1
	u8* p = translate_address_read(address);
	return p[0];
#else
	u8* ptr = translate_address_read(address);
	if (ptr == NULL)  return 0;
	else              return *ptr;
#endif
}

u16 loadW(u32 address)
{
#if 1
	u8* p = translate_address_read(address);
    return ((u16)p[1]<<8) | p[0];
#else
	u16* ptr = translate_address_read(address);

	if (ptr == NULL) return 0;
	else             return *ptr;
#endif
}

u32 loadL(u32 address)
{
#if 1
	u8* p = translate_address_read(address);
	return ((u32)p[3]<<24)|((u32)p[2]<<16)|((u32)p[1]<<8)|(u32)p[0];
#else
	u32* ptr = translate_address_read(address);
	if (ptr == NULL) return 0;
	else             return *ptr;
#endif
}

//=============================================================================

void storeB(u32 address, u8 data)
{
#if 1
	u8* ptr = translate_address_write(address);
	ptr[0] = data;
	post_write(address);
#else
	u8* ptr = translate_address_write(address);

	//Write
	if (ptr) {
		*ptr = data;
		post_write(address);
	}
#endif
}

void storeW(u32 address, u16 data)
{
#if 1
	u8* ptr = translate_address_write(address);
	ptr[0] = data;
	ptr[1] = data>>8;
	post_write(address);
#else
	u16* ptr = translate_address_write(address);

	//Write
	if (ptr) {
		*ptr = data;
		post_write(address);
	}
#endif
}

//	return ((u32)p[3]<<24) | ((u32)p[2]<<16) |
//		   ((u32)p[1]<< 8) |  (u32)p[0];

void storeL(u32 address, u32 data)
{
#if 1
	u8* p = translate_address_write(address);
	p[0] = data;
	p[1] = data>>8;
	p[2] = data>>16;
	p[3] = data>>24;
	post_write(address);
#else
	u32* ptr = translate_address_write(address);

	//Write
	if (ptr){
		*ptr = data;
		post_write(address);
	}
#endif
}

//=============================================================================

static u8 systemMemory[] = 
{
	0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x08, 0xFF, 0xFF, // 0x00
	0x34, 0x3C, 0xFF, 0xFF, 0xFF, 0x3F, 0x00, 0x00,		0x3F, 0xFF, 0x2D, 0x01, 0xFF, 0xFF, 0x03, 0xB2, // 0x10
	0x80, 0x00, 0x01, 0x90, 0x03, 0xB0, 0x90, 0x62,		0x05, 0x00, 0x00, 0x00, 0x0C, 0x0C, 0x4C, 0x4C, // 0x20
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		0x30, 0x00, 0x00, 0x00, 0x20, 0xFF, 0x80, 0x7F, // 0x30
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x40
	0x00, 0x20, 0x69, 0x15, 0x00, 0x00, 0x00, 0x00,		0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, // 0x50
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		0x17, 0x17, 0x03, 0x03, 0x02, 0x00, 0x00, 0x4E, // 0x60
	0x02, 0x32, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x70
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x80
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0x90
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0xA0
	0x00, 0x00, 0x00, 0x00, 0x0A, 0x00, 0x00, 0x00,		0xAA, 0xAA, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0xB0
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0xC0
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0xD0
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0xE0
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00  //0xF0
};

//=============================================================================

void reset_memory(void)
{
	int i;

	eepromStatusEnable = FALSE;
	memory_flash_command = FALSE;

	core_memset(ram, 0, sizeof(ram));	//Clear ram

//=============================================================================
//000000 -> 000100	CPU Internal RAM (Timers/DMA/Z80)
//=====================================================
	for (i = 0; i < sizeof(systemMemory); i++) {
		ram[i] = systemMemory[i];
	}

//=============================================================================
//006C00 -> 006FFF	BIOS Workspace
//==================================

	if (rom.data) {
		*(u32*)(ram + 0x6C00) = rom_header->startPC;		//Start
		*(u16*)(ram + 0x6E82) = *(u16*)(ram + 0x6C04) = rom_header->catalog;	//Catalog
		*( u8*)(ram + 0x6E84) = *( u8*)(ram + 0x6C06) = rom_header->subCatalog;	//Sub-Cat
		core_memcpy(ram + 0x6C08, rom.data + 0x24, 12);			//name

		*(u8*)(ram + 0x6C58) = 0x01;			//LO-EEPROM present

		//32MBit cart?
		if (rom.length > 0x200000)
			*(u8*)(ram + 0x6C59) = 0x01;			//HI-EEPROM present
		else
			*(u8*)(ram + 0x6C59) = 0x00;			//HI-EEPROM not present

		ram[0x6C55] = 1;	//Commercial game
	}
	else {
		*(u32*)(ram + 0x6C00) = 0x00FF970A;	//Start
		*(u16*)(ram + 0x6C04) = 0xFFFF;		//Catalog
		*( u8*)(ram + 0x6C06) = 0x00;			//Sub-Cat
//		sprintf(ram + 0x6C08, "NEOGEOPocket");	//bios rom 'Name'
        core_memcpy(ram+0x6C08,"NEOGEOPocket\0",13);

		*(u8*)(ram + 0x6C58) = 0x00;			//LO-EEPROM not present
		*(u8*)(ram + 0x6C59) = 0x00;			//HI-EEPROM not present

		ram[0x6C55] = 0;	//Bios menu
	}
	

	ram[0x6F80] = 0xFF;	//Lots of battery power!
	ram[0x6F81] = 0x03;

	ram[0x6F84] = 0x40;	// "Power On" startup
	ram[0x6F85] = 0x00;	// No shutdown request
	ram[0x6F86] = 0x00;	// No user answer (?)

	//Language: 0 = Japanese, 1 = English
	ram[0x6F87] = 0;//(u8)language_english;	

	//Color Mode Selection: 0x00 = B&W, 0x10 = Colour
	if (system_colour == COLOURMODE_GREYSCALE)
		ram[0x6F91] = ram[0x6F95] = 0x00; //Force Greyscale
	if (system_colour == COLOURMODE_COLOUR)	
		ram[0x6F91] = ram[0x6F95] = 0x10; //Force Colour

	if (system_colour == COLOURMODE_AUTO){
		if (rom.data) ram[0x6F91] = ram[0x6F95] = rom_header->mode;	//Auto-detect
		else          ram[0x6F91] = ram[0x6F95] = 0x10; // Default = Colour
	}

	//Interrupt table
	for (i = 0; i < 0x12; i++){
		*(u32*)(ram + 0x6FB8 + (i * 4)) = 0x00FF23DF;
	}


//=============================================================================
//008000 -> 00BFFF	Video RAM
//=============================

	ram[0x8000] = 0xC0;	// Both interrupts allowed

	//Hardware window
	ram[0x8002] = 0x00;
	ram[0x8003] = 0x00;
	ram[0x8004] = 0xFF;
	ram[0x8005] = 0xFF;

	ram[0x8006] = 0xc6;	// Frame Rate Register

	ram[0x8012] = 0x00;	// NEG / OOWC setting.

	ram[0x8118] = 0x80;	// BGC on!

	ram[0x83E0] = 0xFF;	// Default background colour
	ram[0x83E1] = 0x0F;

	ram[0x83F0] = 0xFF;	// Default window colour
	ram[0x83F1] = 0x0F;

	ram[0x8400] = 0xFF;	// LED on
	ram[0x8402] = 0x80;	// Flash cycle = 1.3s
}

//=============================================================================


