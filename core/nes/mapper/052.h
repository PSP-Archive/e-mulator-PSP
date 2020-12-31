
/////////////////////////////////////////////////////////////////////
// Mapper 52
STATIC void NES_mapper52_Reset();
STATIC void NES_mapper52_MemoryWriteSaveRAM(u32 addr, u8 data);
STATIC void NES_mapper52_MemoryWrite(u32 addr, u8 data);
STATIC void NES_mapper52_HSync(u32 scanline);
STATIC void NES_mapper52_MMC3_set_CPU_banks();
STATIC void NES_mapper52_MMC3_set_PPU_banks();
void NES_mapper52_SNSS_fixup(); // HACK HACK HACK HACK

STATIC void NES_mapper52_Init();
/////////////////////////////////////////////////////////////////////

