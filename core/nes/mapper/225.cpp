STATIC void NES_mapper225_Init();
STATIC void NES_mapper225_Reset();
STATIC void NES_mapper225_MemoryWrite(u32 addr, u8 data);


/////////////////////////////////////////////////////////////////////
// Mapper 225
STATIC void NES_mapper225_Init()
{
	g_NESmapper.Reset = NES_mapper225_Reset;
	g_NESmapper.MemoryWrite = NES_mapper225_MemoryWrite;
}

STATIC void NES_mapper225_Reset()
{
	// set CPU bank pointers
	g_NESmapper.set_CPU_banks4(0,1,2,3);

	// set PPU bank pointers
	if(g_NESmapper.num_1k_VROM_banks)
	{
		g_NESmapper.set_PPU_banks8(0,1,2,3,4,5,6,7);
	}
}

STATIC void NES_mapper225_MemoryWrite(u32 addr, u8 data)
{
	u8 prg_bank = (addr & 0x0F80) >> 7;
	u8 chr_bank = addr & 0x003F;

	g_NESmapper.set_PPU_bank0(chr_bank*8+0);
	g_NESmapper.set_PPU_bank1(chr_bank*8+1);
	g_NESmapper.set_PPU_bank2(chr_bank*8+2);
	g_NESmapper.set_PPU_bank3(chr_bank*8+3);
	g_NESmapper.set_PPU_bank4(chr_bank*8+4);
	g_NESmapper.set_PPU_bank5(chr_bank*8+5);
	g_NESmapper.set_PPU_bank6(chr_bank*8+6);
	g_NESmapper.set_PPU_bank7(chr_bank*8+7);

	if(addr & 0x2000)
	{
		g_NESmapper.set_mirroring2(NES_PPU_MIRROR_HORIZ);
	}
	else
	{
		g_NESmapper.set_mirroring2(NES_PPU_MIRROR_VERT);
	}

	if(addr & 0x1000)
	{
		// 16KBbank
		if(addr & 0x0040)
		{
			g_NESmapper.set_CPU_bank4(prg_bank*4+2);
			g_NESmapper.set_CPU_bank5(prg_bank*4+3);
			g_NESmapper.set_CPU_bank6(prg_bank*4+2);
			g_NESmapper.set_CPU_bank7(prg_bank*4+3);
		}
		else
		{
			g_NESmapper.set_CPU_bank4(prg_bank*4+0);
			g_NESmapper.set_CPU_bank5(prg_bank*4+1);
			g_NESmapper.set_CPU_bank6(prg_bank*4+0);
			g_NESmapper.set_CPU_bank7(prg_bank*4+1);
		}
	}
	else
	{
		g_NESmapper.set_CPU_bank4(prg_bank*4+0);
		g_NESmapper.set_CPU_bank5(prg_bank*4+1);
		g_NESmapper.set_CPU_bank6(prg_bank*4+2);
		g_NESmapper.set_CPU_bank7(prg_bank*4+3);
	}
}
/////////////////////////////////////////////////////////////////////

