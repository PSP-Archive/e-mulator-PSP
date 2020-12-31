STATIC void NES_mapper3_Init();
STATIC void NES_mapper3_Reset();
STATIC void NES_mapper3_MemoryWrite(u32 addr, u8 data);

/////////////////////////////////////////////////////////////////////
// Mapper 3
STATIC void NES_mapper3_Init()
{
	g_NESmapper.Reset = NES_mapper3_Reset;
	g_NESmapper.MemoryWrite = NES_mapper3_MemoryWrite;
}

STATIC void NES_mapper3_Reset()
{
	// set CPU bank pointers
	if(g_NESmapper.num_8k_ROM_banks > 2)
	{
		g_NESmapper.set_CPU_banks4(0,1,2,3);
	}
	else
	{
		g_NESmapper.set_CPU_banks4(0,1,0,1);
	}

	// set VROM banks
	g_NESmapper.set_PPU_banks8(0,1,2,3,4,5,6,7);
}

STATIC void NES_mapper3_MemoryWrite(u32 addr, u8 data)
{
	u32 base;
	data &= (g_NESmapper.num_1k_VROM_banks>>1)-1;

	base = ((u32)data) << 3;
//LOG("W4 " << HEX(base,4)<< endl);
	g_NESmapper.set_PPU_banks8(base+0,base+1,base+2,base+3,base+4,base+5,base+6,base+7);
}
/////////////////////////////////////////////////////////////////////

