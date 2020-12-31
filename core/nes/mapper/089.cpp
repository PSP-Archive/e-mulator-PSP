STATIC void NES_mapper89_Init();
STATIC void NES_mapper89_Reset();
STATIC void NES_mapper89_MemoryWrite(u32 addr, u8 data);

/////////////////////////////////////////////////////////////////////
// Mapper 89
STATIC void NES_mapper89_Init()
{
	g_NESmapper.Reset = NES_mapper89_Reset;
	g_NESmapper.MemoryWrite = NES_mapper89_MemoryWrite;
}

STATIC void NES_mapper89_Reset()
{
	// set CPU bank pointers
	g_NESmapper.set_CPU_banks4(0,1,g_NESmapper.num_8k_ROM_banks-2,g_NESmapper.num_8k_ROM_banks-1);
	// set PPU bank pointers
	g_NESmapper.set_PPU_banks8(0,1,2,3,4,5,6,7);
}

STATIC void NES_mapper89_MemoryWrite(u32 addr, u8 data)
{
	if((addr & 0xFF00) == 0xC000)
	{
		u8 prg_bank = (data & 0x70) >> 4;
		u8 chr_bank = ((data & 0x80) >> 4) | (data & 0x07);

		g_NESmapper.set_CPU_bank4(prg_bank*2+0);
		g_NESmapper.set_CPU_bank5(prg_bank*2+1);

		g_NESmapper.set_PPU_bank0(chr_bank*8+0);
		g_NESmapper.set_PPU_bank1(chr_bank*8+1);
		g_NESmapper.set_PPU_bank2(chr_bank*8+2);
		g_NESmapper.set_PPU_bank3(chr_bank*8+3);
		g_NESmapper.set_PPU_bank4(chr_bank*8+4);
		g_NESmapper.set_PPU_bank5(chr_bank*8+5);
		g_NESmapper.set_PPU_bank6(chr_bank*8+6);
		g_NESmapper.set_PPU_bank7(chr_bank*8+7);

		if (data & 0x08)
		{
			g_NESmapper.set_mirroring(1,1,1,1);
		}
		else
		{
			g_NESmapper.set_mirroring(0,0,0,0);
		}
	}
}

/////////////////////////////////////////////////////////////////////

