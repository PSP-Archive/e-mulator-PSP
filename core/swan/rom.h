//////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////////
//
//
//
//
//
//
//////////////////////////////////////////////////////////////////////////////

#ifndef __ROM_H__
#define __ROM_H__


#define WS_SYSTEM_MONO			0
#define WS_SYSTEM_COLOR			1

#define WS_ROM_SIZE_2MBIT		1
#define WS_ROM_SIZE_4MBIT		2
#define WS_ROM_SIZE_8MBIT		3
#define WS_ROM_SIZE_16MBIT		4
#define WS_ROM_SIZE_24MBIT		5
#define WS_ROM_SIZE_32MBIT		6
#define WS_ROM_SIZE_48MBIT		7
#define WS_ROM_SIZE_64MBIT		8
#define WS_ROM_SIZE_128MBIT		9

#define WS_EEPROM_SIZE_NONE		0
#define WS_SRAM_SIZE_NONE		0
#define WS_EEPROM_SIZE_64k		1
#define WS_EEPROM_SIZE_256k		2
#define WS_SRAM_SIZE_1k			10
#define WS_SRAM_SIZE_16k		20
#define WS_SRAM_SIZE_8k			50


typedef struct ws_romHeaderStruct
{
	u8	developperId;
	u8	minimumSupportSystem;
	u8	cartId;
	u8	romSize;
	u8	eepromSize;
	u8	additionnalCapabilities;
	u8	realtimeClock;
	u16	checksum;
} ws_romHeaderStruct;


u8				*ws_rom_load(char *path, u32 *romSize);
ws_romHeaderStruct	*ws_rom_getHeader(u8 *wsrom, u32 wsromSize);
u32				ws_rom_sramSize(u8 *wsrom, u32 wsromSize);
u32				ws_rom_eepromSize(u8 *wsrom, u32 wsromSize);


#endif
