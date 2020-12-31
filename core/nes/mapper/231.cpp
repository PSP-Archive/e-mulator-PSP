STATIC void NES_mapper231_Init();
STATIC void NES_mapper231_Reset();
STATIC void NES_mapper231_MemoryWrite(u32 addr, u8 data);

/////////////////////////////////////////////////////////////////////
// Mapper 231
STATIC void NES_mapper231_Init()
{
	g_NESmapper.Reset = NES_mapper231_Reset;
	g_NESmapper.MemoryWrite = NES_mapper231_MemoryWrite;
}

STATIC void NES_mapper231_Reset()
{
	// set CPU bank pointers
	g_NESmapper.set_CPU_banks4(0,1,2,3);

	// set PPU bank pointers
	if(g_NESmapper.num_1k_VROM_banks)
	{
		g_NESmapper.set_PPU_banks8(0,1,2,3,4,5,6,7);
	}
}

STATIC void NES_mapper231_MemoryWrite(u32 addr, u8 data)
{
	if(addr & 0x0020)
	{
		u8 prg_bank = (u8)(addr >> 1) & 0x0F;
		g_NESmapper.set_CPU_bank4(prg_bank*4+0);
		g_NESmapper.set_CPU_bank5(prg_bank*4+1);
		g_NESmapper.set_CPU_bank6(prg_bank*4+2);
		g_NESmapper.set_CPU_bank7(prg_bank*4+3);
	}
	else
	{
		u8 prg_bank = addr & 0x001E;
		g_NESmapper.set_CPU_bank4(prg_bank*2+0);
		g_NESmapper.set_CPU_bank5(prg_bank*2+1);
		g_NESmapper.set_CPU_bank6(prg_bank*2+0);
		g_NESmapper.set_CPU_bank7(prg_bank*2+1);
	}

	if(addr & 0x0080)
	{
		g_NESmapper.set_mirroring2(NES_PPU_MIRROR_HORIZ);
	}
	else
	{
		g_NESmapper.set_mirroring2(NES_PPU_MIRROR_VERT);
	}
}
/////////////////////////////////////////////////////////////////////

