
/////////////////////////////////////////////////////////////////////
// Mapper NSF - private mapper number = 12 (decimal)
STATIC void NES_mapperNSF_Reset();
STATIC void NES_mapperNSF_MemoryWriteLow(u32 addr, u8 data);
STATIC void NES_mapperNSF_MemoryWriteSaveRAM(u32 addr, u8 data);
STATIC void NES_mapperNSF_MemoryWrite(u32 addr, u8 data);
STATIC u8 NES_mapperNSF_MemoryReadLow(u32 addr);
STATIC void NES_mapperNSF_BankSwitch(u8 num, u8 bank);
STATIC void NES_mapperNSF_LoadPlayer();

STATIC void NES_mapperNSF_Init();
/////////////////////////////////////////////////////////////////////

