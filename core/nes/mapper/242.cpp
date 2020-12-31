STATIC void NES_mapper242_Init();
STATIC void NES_mapper242_Reset();
STATIC void NES_mapper242_MemoryWrite(u32 addr, u8 data);


/////////////////////////////////////////////////////////////////////
// Mapper 242
STATIC void NES_mapper242_Init()
{
	g_NESmapper.Reset = NES_mapper242_Reset;
	g_NESmapper.MemoryWrite = NES_mapper242_MemoryWrite;
}

STATIC void NES_mapper242_Reset()
{
	// set CPU bank pointers
	g_NESmapper.set_CPU_banks4(0,1,2,3);

	// set PPU bank pointers
	if(g_NESmapper.num_1k_VROM_banks)
	{
		g_NESmapper.set_PPU_banks8(0,1,2,3,4,5,6,7);
	}
}

STATIC void NES_mapper242_MemoryWrite(u32 addr, u8 data)
{
	if(addr & 0x0001)
	{
		g_NESmapper.set_CPU_bank4(((addr & 0x78) >> 1) + 0);
		g_NESmapper.set_CPU_bank5(((addr & 0x78) >> 1) + 1);
		g_NESmapper.set_CPU_bank6(((addr & 0x78) >> 1) + 2);
		g_NESmapper.set_CPU_bank7(((addr & 0x78) >> 1) + 3);
	}
}
/////////////////////////////////////////////////////////////////////

