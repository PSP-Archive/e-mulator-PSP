
//
STATIC void NES_UNIFmapper4_Reset();
STATIC void NES_UNIFmapper4_setchr1(u8 pn, u8 data);
STATIC void NES_UNIFmapper4_setchr2(u8 pn, u8 data);
STATIC void NES_UNIFmapper4_oMMC3PRG(u8 data);
STATIC void NES_UNIFmapper4_oMMC3CHR(u8 data);
STATIC void NES_UNIFmapper4_MemoryWriteLow(u32 addr, u8 data);
STATIC void NES_UNIFmapper4_MemoryWrite(u32 addr, u8 data);
STATIC void NES_UNIFmapper4_HSync(u32 scanline);

STATIC void NES_UNIFmapper4_Init();
/////////////////////////////////////////////////////////////////////

