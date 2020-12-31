STATIC void NES_mapper229_Init();
STATIC void NES_mapper229_Reset();
STATIC void NES_mapper229_MemoryWrite(u32 addr, u8 data);


/////////////////////////////////////////////////////////////////////
// Mapper 229
STATIC void NES_mapper229_Init()
{
	g_NESmapper.Reset = NES_mapper229_Reset;
	g_NESmapper.MemoryWrite = NES_mapper229_MemoryWrite;
}

STATIC void NES_mapper229_Reset()
{
	// set CPU bank pointers
	g_NESmapper.set_CPU_banks4(0,1,2,3);

	// set PPU bank pointers
	g_NESmapper.set_PPU_banks8(0,1,2,3,4,5,6,7);
}

STATIC void NES_mapper229_MemoryWrite(u32 addr, u8 data)
{
	addr &= 0x0FFF;

	if(addr & 0x0020)
	{
		g_NESmapper.set_mirroring2(NES_PPU_MIRROR_HORIZ);
	}
	else
	{
		g_NESmapper.set_mirroring2(NES_PPU_MIRROR_VERT);
	}

	if(addr & 0x001E)
	{
		u8 prg_bank = (u8)(addr & 0x001F);
		u32 chr_bank = addr & 0x0FFF;
		g_NESmapper.set_CPU_bank4(prg_bank*2+0);
		g_NESmapper.set_CPU_bank5(prg_bank*2+1);
		g_NESmapper.set_CPU_bank6(prg_bank*2+0);
		g_NESmapper.set_CPU_bank7(prg_bank*2+1);
		g_NESmapper.set_PPU_bank0(chr_bank*8+0);
		g_NESmapper.set_PPU_bank1(chr_bank*8+1);
		g_NESmapper.set_PPU_bank2(chr_bank*8+2);
		g_NESmapper.set_PPU_bank3(chr_bank*8+3);
		g_NESmapper.set_PPU_bank4(chr_bank*8+4);
		g_NESmapper.set_PPU_bank5(chr_bank*8+5);
		g_NESmapper.set_PPU_bank6(chr_bank*8+6);
		g_NESmapper.set_PPU_bank7(chr_bank*8+7);
	}
	else
	{
		g_NESmapper.set_CPU_banks4(0,1,2,3);
		g_NESmapper.set_PPU_banks8(0,1,2,3,4,5,6,7);
	}
}
/////////////////////////////////////////////////////////////////////

