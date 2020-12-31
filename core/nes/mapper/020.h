
/////////////////////////////////////////////////////////////////////
// Mapper 20
STATIC void NES_mapper20_Reset();
STATIC u8 NES_mapper20_MemoryReadLow(u32 addr);
STATIC void NES_mapper20_MemoryWriteLow(u32 addr, u8 data);
STATIC void NES_mapper20_MemoryWriteSaveRAM(u32 addr, u8 data);
STATIC void NES_mapper20_MemoryWrite(u32 addr, u8 data);
STATIC void NES_mapper20_HSync(u32 scanline);
STATIC void NES_mapper20_VSync();
void NES_mapper20_SNSS_fixup();

STATIC u8 NES_mapper20_GetDiskSideNum();
STATIC u8 NES_mapper20_GetDiskSide();
STATIC void  NES_mapper20_SetDiskSide(u8 side);
STATIC u8 *NES_mapper20_GetDiskDatap();
STATIC u8 NES_mapper20_DiskAccessed();

STATIC void NES_mapper20_Init();
/////////////////////////////////////////////////////////////////////

