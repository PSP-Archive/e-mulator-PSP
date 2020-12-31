STATIC void NES_mapper93_Init();
STATIC void NES_mapper93_Reset();
STATIC void NES_mapper93_MemoryWriteSaveRAM(u32 addr, u8 data);


/////////////////////////////////////////////////////////////////////
// Mapper 93
STATIC void NES_mapper93_Init()
{
	g_NESmapper.Reset = NES_mapper93_Reset;
	g_NESmapper.MemoryWriteSaveRAM = NES_mapper93_MemoryWriteSaveRAM;
}

STATIC void NES_mapper93_Reset()
{
	// set CPU bank pointers
	g_NESmapper.set_CPU_banks4(0,1,g_NESmapper.num_8k_ROM_banks-2,g_NESmapper.num_8k_ROM_banks-1);
	// set PPU bank pointers
	g_NESmapper.set_PPU_banks8(0,1,2,3,4,5,6,7);
}

STATIC void NES_mapper93_MemoryWriteSaveRAM(u32 addr, u8 data)
{
	if(addr == 0x6000)
	{
		g_NESmapper.set_CPU_bank4(data*2+0);
		g_NESmapper.set_CPU_bank5(data*2+1);
	}
}

/////////////////////////////////////////////////////////////////////

