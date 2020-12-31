#ifdef _NES_MAPPER_CPP_

STATIC void NES_UNIFmapper2_Init();
STATIC void NES_UNIFmapper2_Reset();
STATIC u8 NES_UNIFmapper2_MemoryReadLow(u32 addr);
STATIC void NES_UNIFmapper2_MemoryWriteLow(u32 addr, u8 data);
STATIC void NES_UNIFmapper2_MemoryWrite(u32 addr, u8 data);

// UNIF  Mario
STATIC void NES_UNIFmapper2_Init()
{
	g_NESmapper.Reset = NES_UNIFmapper2_Reset;
	g_NESmapper.MemoryWrite = NES_UNIFmapper2_MemoryWrite;
	g_NESmapper.MemoryWriteLow = NES_UNIFmapper2_MemoryWriteLow;
	g_NESmapper.MemoryReadLow = NES_UNIFmapper2_MemoryReadLow;
}


STATIC void NES_UNIFmapper2_Reset()
{
	// set CPU bank pointers
	g_NESmapper.set_CPU_banks4(0,1,2,3);
	g_NESmapper.set_CPU_bank_unif(4, 3);
	// set PPU bank pointers
	g_NESmapper.set_PPU_banks8(0,1,2,3,4,5,6,7);
	g_NESmapper.set_mirroring2(NES_PPU_MIRROR_VERT);
	core_memset(g_NESmapper.NES_UNIFmapper2.wram, 0, 0x1000);
}

STATIC u8 NES_UNIFmapper2_MemoryReadLow(u32 addr)
{
//	if(addr >= 0x7000 && addr < 0x8000)
	if((addr&0xFFF) < 0x800)
		return g_NESmapper.NES_UNIFmapper2.wram[addr&0x7FF];
	return NES6502_GetByte(addr);
}

STATIC void NES_UNIFmapper2_MemoryWriteLow(u32 addr, u8 data)
{
	if(addr >= 0x7000 && addr < 0x8000)
		g_NESmapper.NES_UNIFmapper2.wram[addr&0xFFF] = data;
}

STATIC void NES_UNIFmapper2_MemoryWrite(u32 addr, u8 data)
{
//	LOG("W " << HEX(addr,4) << "  " << HEX(data,2)  << endl);

}

#endif
