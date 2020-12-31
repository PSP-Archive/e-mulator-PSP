STATIC void NES_mapper7_Init();
STATIC void NES_mapper7_Reset();
STATIC void NES_mapper7_MemoryWrite(u32 addr, u8 data);

/////////////////////////////////////////////////////////////////////
// Mapper 7
STATIC void NES_mapper7_Init()
{
	g_NESmapper.Reset = NES_mapper7_Reset;
	g_NESmapper.MemoryWrite = NES_mapper7_MemoryWrite;
}

STATIC void NES_mapper7_Reset()
{
	// set CPU bank pointers
	g_NESmapper.set_CPU_banks4(0,1,2,3);
}

STATIC void NES_mapper7_MemoryWrite(u32 addr, u8 data)
{
	u32 bank;

	bank = (data & 0x07)<< 2;
	g_NESmapper.set_CPU_banks4(bank+0,bank+1,bank+2,bank+3);

	if(data & 0x10)
	{
		g_NESmapper.set_mirroring(1,1,1,1);
	}
	else
	{
		g_NESmapper.set_mirroring(0,0,0,0);
	}

}
/////////////////////////////////////////////////////////////////////

