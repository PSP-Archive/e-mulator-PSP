
/////////////////////////////////////////////////////////////////////
// Mapper 118
STATIC void NES_mapper118_Reset();
STATIC void NES_mapper118_MemoryWrite(u32 addr, u8 data);
STATIC void NES_mapper118_HSync(u32 scanline);
STATIC void NES_mapper118_MMC3_set_CPU_banks();
STATIC void NES_mapper118_MMC3_set_PPU_banks();
void NES_mapper118_SNSS_fixup();

STATIC void NES_mapper118_Init();
/////////////////////////////////////////////////////////////////////

