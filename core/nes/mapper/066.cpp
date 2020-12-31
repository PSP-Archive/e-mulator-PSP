STATIC void NES_mapper66_Init();
STATIC void NES_mapper66_Reset();
STATIC void NES_mapper66_MemoryWriteSaveRAM(u32 addr, u8 data);
STATIC void NES_mapper66_MemoryWrite(u32 addr, u8 data);

/////////////////////////////////////////////////////////////////////
// Mapper 66
STATIC void NES_mapper66_Init()
{
	g_NESmapper.Reset = NES_mapper66_Reset;
	g_NESmapper.MemoryWriteSaveRAM = NES_mapper66_MemoryWriteSaveRAM;
	g_NESmapper.MemoryWrite = NES_mapper66_MemoryWrite;
}

STATIC void NES_mapper66_Reset()
{
	// set CPU bank pointers
	g_NESmapper.set_CPU_banks4(0,1,2,3);
	g_NESmapper.set_PPU_banks8(0,1,2,3,4,5,6,7);

	if(NES_crc32() == 0xe30552db) // Paris-Dakar Rally Special
	{
		g_NES.frame_irq_disenabled = 1;
	}
}

STATIC void NES_mapper66_MemoryWriteSaveRAM(u32 addr, u8 data)
{
	u8 chr_bank = data & 0x0F;
	u8 prg_bank = (data & 0xF0) >> 4;

	g_NESmapper.set_CPU_bank4(prg_bank*4+0);
	g_NESmapper.set_CPU_bank5(prg_bank*4+1);
	g_NESmapper.set_CPU_bank6(prg_bank*4+2);
	g_NESmapper.set_CPU_bank7(prg_bank*4+3);

	g_NESmapper.set_PPU_bank0(chr_bank*8+0);
	g_NESmapper.set_PPU_bank1(chr_bank*8+1);
	g_NESmapper.set_PPU_bank2(chr_bank*8+2);
	g_NESmapper.set_PPU_bank3(chr_bank*8+3);
	g_NESmapper.set_PPU_bank4(chr_bank*8+4);
	g_NESmapper.set_PPU_bank5(chr_bank*8+5);
	g_NESmapper.set_PPU_bank6(chr_bank*8+6);
	g_NESmapper.set_PPU_bank7(chr_bank*8+7);
}

STATIC void NES_mapper66_MemoryWrite(u32 addr, u8 data)
{
	u8 chr_bank = data & 0x0F;
	u8 prg_bank = (data & 0xF0) >> 4;

	g_NESmapper.set_CPU_bank4(prg_bank*4+0);
	g_NESmapper.set_CPU_bank5(prg_bank*4+1);
	g_NESmapper.set_CPU_bank6(prg_bank*4+2);
	g_NESmapper.set_CPU_bank7(prg_bank*4+3);

	g_NESmapper.set_PPU_bank0(chr_bank*8+0);
	g_NESmapper.set_PPU_bank1(chr_bank*8+1);
	g_NESmapper.set_PPU_bank2(chr_bank*8+2);
	g_NESmapper.set_PPU_bank3(chr_bank*8+3);
	g_NESmapper.set_PPU_bank4(chr_bank*8+4);
	g_NESmapper.set_PPU_bank5(chr_bank*8+5);
	g_NESmapper.set_PPU_bank6(chr_bank*8+6);
	g_NESmapper.set_PPU_bank7(chr_bank*8+7);
}
/////////////////////////////////////////////////////////////////////

