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

#ifndef __MEM__
#define __MEM__
//=============================================================================

#define ROM_START	0x200000
#define ROM_END		0x3FFFFF

#define HIROM_START	0x800000
#define HIROM_END	0x9FFFFF

#define BIOS_START	0xFF0000
#define BIOS_END	0xFFFFFF

void reset_memory(void);

void* translate_address_read(u32 address);
void* translate_address_write(u32 address);

void dump_memory(u32 start, u32 length);

extern BOOL debug_abort_memory;
extern BOOL debug_mask_memory_error_messages;

extern BOOL memory_unlock_flash_write;
extern BOOL memory_flash_error;
extern BOOL memory_flash_command;

extern BOOL eepromStatusEnable;

//=============================================================================

u8  loadB(u32 address);
u16 loadW(u32 address);
u32 loadL(u32 address);

void storeB(u32 address, u8 data);
void storeW(u32 address, u16 data);
void storeL(u32 address, u32 data);

//=============================================================================
#endif
