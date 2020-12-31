
/////////////////////////////////////////////////////////////////////
// Mapper 10
STATIC void NES_mapper10_Reset();
STATIC void NES_mapper10_PPU_Latch_FDFE(u32 addr);
STATIC void NES_mapper10_set_VROM_0000();
STATIC void NES_mapper10_set_VROM_1000();
STATIC void NES_mapper10_MemoryWrite(u32 addr, u8 data);
void NES_mapper10_SNSS_fixup();

STATIC void NES_mapper10_Init();
/////////////////////////////////////////////////////////////////////

