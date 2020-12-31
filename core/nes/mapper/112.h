
/////////////////////////////////////////////////////////////////////
// Mapper 112
STATIC void NES_mapper112_Reset();
STATIC void NES_mapper112_MemoryWrite(u32 addr, u8 data);
STATIC void NES_mapper112_HSync(u32 scanline);
STATIC void NES_mapper112_MMC3_set_CPU_banks();
STATIC void NES_mapper112_MMC3_set_PPU_banks();
void NES_mapper112_SNSS_fixup();

STATIC void NES_mapper112_Init();
/////////////////////////////////////////////////////////////////////

