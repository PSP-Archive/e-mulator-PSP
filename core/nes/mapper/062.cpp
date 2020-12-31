#ifdef _NES_MAPPER_CPP_

STATIC void NES_mapper62_Init();
STATIC void NES_mapper62_Reset();
STATIC void NES_mapper62_MemoryWriteSaveRAM(u32 addr, u8 data);
STATIC void NES_mapper62_MemoryWriteLow(u32 addr, u8 data);
STATIC void NES_mapper62_MemoryWrite(u32 addr, u8 data);

/////////////////////////////////////////////////////////////////////
// Mapper 62
STATIC void NES_mapper62_Init()
{
	g_NESmapper.Reset = NES_mapper62_Reset;
	g_NESmapper.MemoryWrite = NES_mapper62_MemoryWrite;
	g_NESmapper.MemoryWriteSaveRAM = NES_mapper62_MemoryWriteSaveRAM;
	g_NESmapper.MemoryWriteLow = NES_mapper62_MemoryWriteLow;
}

STATIC void NES_mapper62_Reset()
{
	g_NESmapper.set_CPU_banks4(0,1,2,3);
	g_NESmapper.set_PPU_banks8(0,1,2,3,4,5,6,7);
}

STATIC void NES_mapper62_MemoryWriteSaveRAM(u32 addr, u8 data)
{
	LOG("W " << HEX(addr,4) << "  " << HEX(data,2)  << endl);
}

STATIC void NES_mapper62_MemoryWriteLow(u32 addr, u8 data)
{
	LOG("W " << HEX(addr,4) << "  " << HEX(data,2)  << endl);
}

STATIC void NES_mapper62_MemoryWrite(u32 addr, u8 data)
{
	LOG("W " << HEX(addr,4) << "  " << HEX(data,2)  << endl);
	switch(addr & 0xFF00){
		case 0x8100:
			g_NESmapper.set_CPU_bank4(data);
			g_NESmapper.set_CPU_bank5(data+1);
			break;
		case 0x8500:
			g_NESmapper.set_CPU_bank4(data);
			break;
		case 0x8700:
			g_NESmapper.set_CPU_bank5(data);
			break;
		g_NESmapper.set_PPU_bank0(data);
		g_NESmapper.set_PPU_bank1(data+1);
		g_NESmapper.set_PPU_bank2(data + 2);
		g_NESmapper.set_PPU_bank3(data + 3);
		g_NESmapper.set_PPU_bank4(data + 4);
		g_NESmapper.set_PPU_bank5(data + 5);
		g_NESmapper.set_PPU_bank6(data + 6);
		g_NESmapper.set_PPU_bank7(data + 7);
	}

}
/////////////////////////////////////////////////////////////////////

#endif
