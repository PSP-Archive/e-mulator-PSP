
/////////////////////////////////////////////////////////////////////
// Mapper 90
STATIC void NES_mapper90_Reset();
STATIC u8 NES_mapper90_MemoryReadLow(u32 addr);
STATIC void NES_mapper90_MemoryWriteLow(u32 addr, u8 data);
STATIC void NES_mapper90_MemoryWrite(u32 addr, u8 data);
STATIC void NES_mapper90_HSync(u32 scanline);
STATIC void NES_mapper90_Sync_Mirror();
STATIC void NES_mapper90_Sync_Chr_Banks();
STATIC void NES_mapper90_Sync_Prg_Banks();


STATIC void NES_mapper90_Init();
/////////////////////////////////////////////////////////////////////

