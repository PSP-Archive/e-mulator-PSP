
/////////////////////////////////////////////////////////////////////
// Mapper 49
STATIC void NES_mapper49_Reset();
STATIC void NES_mapper49_MemoryWriteSaveRAM(u32 addr, u8 data);
STATIC void NES_mapper49_MemoryWrite(u32 addr, u8 data);
STATIC void NES_mapper49_HSync(u32 scanline);
STATIC void NES_mapper49_MMC3_set_CPU_banks();
STATIC void NES_mapper49_MMC3_set_PPU_banks();
void NES_mapper49_SNSS_fixup();

STATIC void NES_mapper49_Init();
/////////////////////////////////////////////////////////////////////

