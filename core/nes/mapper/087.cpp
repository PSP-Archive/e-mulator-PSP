#ifdef _NES_MAPPER_CPP_

STATIC void NES_mapper87_Init();
STATIC void NES_mapper87_Reset();
STATIC void NES_mapper87_MemoryWriteSaveRAM(u32 addr, u8 data);

/////////////////////////////////////////////////////////////////////
// Mapper 87
STATIC void NES_mapper87_Init()
{
	g_NESmapper.Reset = NES_mapper87_Reset;
	g_NESmapper.MemoryWriteSaveRAM = NES_mapper87_MemoryWriteSaveRAM;
}

STATIC void NES_mapper87_Reset()
{
	// set CPU bank pointers
	g_NESmapper.set_CPU_banks4(0,1,2,3);

	// set PPU bank pointers
	if(g_NESmapper.num_1k_VROM_banks)
	{
		g_NESmapper.set_PPU_banks8(0,1,2,3,4,5,6,7);
	}
}

STATIC void NES_mapper87_MemoryWriteSaveRAM(u32 addr, u8 data)
{
	if(addr == 0x6000)
	{
		u8 chr_bank = (data & 0x02) >> 1;

		g_NESmapper.set_PPU_bank0(chr_bank*8+0);
		g_NESmapper.set_PPU_bank1(chr_bank*8+1);
		g_NESmapper.set_PPU_bank2(chr_bank*8+2);
		g_NESmapper.set_PPU_bank3(chr_bank*8+3);
		g_NESmapper.set_PPU_bank4(chr_bank*8+4);
		g_NESmapper.set_PPU_bank5(chr_bank*8+5);
		g_NESmapper.set_PPU_bank6(chr_bank*8+6);
		g_NESmapper.set_PPU_bank7(chr_bank*8+7);
	}
}
/////////////////////////////////////////////////////////////////////

#endif
