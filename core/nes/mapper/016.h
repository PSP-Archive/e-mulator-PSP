
/////////////////////////////////////////////////////////////////////
// Mapper 16
STATIC void NES_mapper16_Reset();
STATIC void NES_mapper16_MemoryWrite(u32 addr, u8 data);
STATIC void NES_mapper16_MemoryWriteSaveRAM(u32 addr, u8 data);
STATIC void NES_mapper16_MemoryReadSaveRAM(u32 addr);
STATIC void NES_mapper16_HSync(u32 scanline);
STATIC void NES_mapper16_MemoryWrite2(u32 addr, u8 data);
STATIC void NES_mapper16_MemoryWrite3(u32 addr, u8 data);
STATIC void NES_mapper16_SetBarcodeValue(u32 value_low, u32 value_high);


STATIC void NES_mapper16_Init();
/////////////////////////////////////////////////////////////////////

