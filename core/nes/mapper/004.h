
/////////////////////////////////////////////////////////////////////
// Mapper 4
STATIC void NES_mapper4_Reset();
STATIC u8 NES_mapper4_MemoryReadLow(u32 addr);
STATIC void NES_mapper4_MemoryWrite(u32 addr, u8 data);
STATIC void NES_mapper4_HSync(u32 scanline);
STATIC void NES_mapper4_MMC3_set_CPU_banks();
STATIC void NES_mapper4_MMC3_set_PPU_banks();
void NES_mapper4_SNSS_fixup(); // HACK HACK HACK HACK

STATIC void NES_mapper4_Init();
/////////////////////////////////////////////////////////////////////

