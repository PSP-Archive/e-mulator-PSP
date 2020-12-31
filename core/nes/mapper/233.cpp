STATIC void NES_mapper233_Init();
STATIC void NES_mapper233_Reset();
STATIC void NES_mapper233_MemoryWrite(u32 addr, u8 data);

/////////////////////////////////////////////////////////////////////
// Mapper 233
STATIC void NES_mapper233_Init()
{
	g_NESmapper.Reset = NES_mapper233_Reset;
	g_NESmapper.MemoryWrite = NES_mapper233_MemoryWrite;
}

STATIC void NES_mapper233_Reset()
{
	// set CPU bank pointers
	g_NESmapper.set_CPU_banks4(0,1,2,3);
}

STATIC void NES_mapper233_MemoryWrite(u32 addr, u8 data)
{
	if(data & 0x20)
	{
		u8 prg_bank = data & 0x1F;
		g_NESmapper.set_CPU_bank4(prg_bank*2+0);
		g_NESmapper.set_CPU_bank5(prg_bank*2+1);
		g_NESmapper.set_CPU_bank6(prg_bank*2+0);
		g_NESmapper.set_CPU_bank7(prg_bank*2+1);
	}
	else
	{
		u8 prg_bank = (data & 0x1E) >> 1;
		g_NESmapper.set_CPU_bank4(prg_bank*4+0);
		g_NESmapper.set_CPU_bank5(prg_bank*4+1);
		g_NESmapper.set_CPU_bank6(prg_bank*4+2);
		g_NESmapper.set_CPU_bank7(prg_bank*4+3);
	}

	if((data & 0xC0) == 0x00)
	{
		g_NESmapper.set_mirroring(0,0,0,1);
	}
	else if((data & 0xC0) == 0x40)
	{
		g_NESmapper.set_mirroring2(NES_PPU_MIRROR_VERT);
	}
	else if((data & 0xC0) == 0x80)
	{
		g_NESmapper.set_mirroring2(NES_PPU_MIRROR_HORIZ);
	}
	else
	{
		g_NESmapper.set_mirroring(1,1,1,1);
	}
}
/////////////////////////////////////////////////////////////////////

