STATIC void NES_mapper2_Init();
STATIC void NES_mapper2_Reset();
STATIC void NES_mapper2_MemoryWrite(u32 addr, u8 data);


/////////////////////////////////////////////////////////////////////
// Mapper 2
STATIC void NES_mapper2_Init()
{
	g_NESmapper.Reset = NES_mapper2_Reset;
	g_NESmapper.MemoryWrite = NES_mapper2_MemoryWrite;
}

STATIC void NES_mapper2_Reset()
{
	// set CPU bank pointers
	g_NESmapper.set_CPU_banks4(0,1,g_NESmapper.num_8k_ROM_banks-2,g_NESmapper.num_8k_ROM_banks-1);
}

STATIC void NES_mapper2_MemoryWrite(u32 addr, u8 data)
{
	data &= g_NESmapper.num_8k_ROM_banks-1;
	g_NESmapper.set_CPU_banks4(data*2,(data*2)+1,g_NESmapper.num_8k_ROM_banks-2,g_NESmapper.num_8k_ROM_banks-1);
}
/////////////////////////////////////////////////////////////////////


