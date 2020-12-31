
/////////////////////////////////////////////////////////////////////
// Mapper 248
STATIC void NES_mapper248_Reset();
STATIC void NES_mapper248_MemoryWriteSaveRAM(u32 addr, u8 data);
STATIC void NES_mapper248_MemoryWrite(u32 addr, u8 data);
STATIC void NES_mapper248_HSync(u32 scanline);
STATIC void NES_mapper248_MMC3_set_CPU_banks();
STATIC void NES_mapper248_MMC3_set_PPU_banks();
void NES_mapper248_SNSS_fixup(); // HACK HACK HACK HACK

STATIC void NES_mapper248_Init();
/////////////////////////////////////////////////////////////////////

