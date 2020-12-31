
/////////////////////////////////////////////////////////////////////
// Mapper 45
STATIC void NES_mapper45_Reset();
STATIC void NES_mapper45_MemoryWriteSaveRAM(u32 addr, u8 data);
STATIC void NES_mapper45_MemoryWrite(u32 addr, u8 data);
STATIC void NES_mapper45_HSync(u32 scanline);
STATIC void NES_mapper45_MAP45_set_CPU_bank4(u8 data);
STATIC void NES_mapper45_MAP45_set_CPU_bank5(u8 data);
STATIC void NES_mapper45_MAP45_set_CPU_bank6(u8 data);
STATIC void NES_mapper45_MAP45_set_CPU_bank7(u8 data);
STATIC void NES_mapper45_MAP45_set_PPU_banks();

STATIC void NES_mapper45_Init();
/////////////////////////////////////////////////////////////////////

