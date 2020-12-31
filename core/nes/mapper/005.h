
/////////////////////////////////////////////////////////////////////
// Mapper 5
STATIC void NES_mapper5_Reset();
STATIC u8 NES_mapper5_MemoryReadLow(u32 addr);
STATIC void NES_mapper5_MemoryWriteLow(u32 addr, u8 data);
STATIC void NES_mapper5_MemoryWriteSaveRAM(u32 addr, u8 data);
STATIC void NES_mapper5_MemoryWrite(u32 addr, u8 data);
STATIC void NES_mapper5_HSync(u32 scanline);
STATIC void NES_mapper5_MMC5_set_CPU_bank(u8 page, u8 bank);
STATIC void NES_mapper5_MMC5_set_WRAM_bank(u8 page, u8 bank);
STATIC u8 NES_mapper5_PPU_Latch_RenderScreen(u8 mode, u32 addr);
STATIC void NES_mapper5_sync_Chr_banks(u8 mode);

STATIC void NES_mapper5_Init();
/////////////////////////////////////////////////////////////////////

