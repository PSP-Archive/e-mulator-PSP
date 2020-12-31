
/////////////////////////////////////////////////////////////////////
// Mapper 9
STATIC void NES_mapper9_Reset();
STATIC void NES_mapper9_PPU_Latch_FDFE(u32 addr);
STATIC void NES_mapper9_set_VROM_0000();
STATIC void NES_mapper9_set_VROM_1000();
STATIC void NES_mapper9_MemoryWrite(u32 addr, u8 data);
void NES_mapper9_SNSS_fixup();

STATIC void NES_mapper9_Init();
/////////////////////////////////////////////////////////////////////

