STATIC void NES_mapper77_Init();
STATIC void NES_mapper77_Reset();
STATIC void NES_mapper77_MemoryWrite(u32 addr, u8 data);

/////////////////////////////////////////////////////////////////////
// Mapper 77
STATIC void NES_mapper77_Init()
{
	g_NESmapper.Reset = NES_mapper77_Reset;
	g_NESmapper.MemoryWrite = NES_mapper77_MemoryWrite;
}

STATIC void NES_mapper77_Reset()
{
	// set CPU bank pointers
	g_NESmapper.set_CPU_banks4(0,1,2,3);

	// for Napoleon Senki
	g_PPU.vram_write_protect = 0;
}

STATIC void NES_mapper77_MemoryWrite(u32 addr, u8 data)
{
	u8 prg_bank = data & 0x07;
	u8 chr_bank = (data & 0xF0) >> 4;

	g_NESmapper.set_CPU_bank4(prg_bank*4+0);
	g_NESmapper.set_CPU_bank5(prg_bank*4+1);
	g_NESmapper.set_CPU_bank6(prg_bank*4+2);
	g_NESmapper.set_CPU_bank7(prg_bank*4+3);

	g_NESmapper.set_PPU_bank0(chr_bank*2+0);
	g_NESmapper.set_PPU_bank1(chr_bank*2+1);
}
/////////////////////////////////////////////////////////////////////

