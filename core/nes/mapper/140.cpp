
STATIC void NES_mapper140_Init();
STATIC void NES_mapper140_Reset();
STATIC void NES_mapper140_MemoryWriteSaveRAM(u32 addr, u8 data);


/////////////////////////////////////////////////////////////////////
// Mapper 140
STATIC void NES_mapper140_Init()
{
	g_NESmapper.Reset = NES_mapper140_Reset;
	g_NESmapper.MemoryWriteSaveRAM = NES_mapper140_MemoryWriteSaveRAM;
}

STATIC void NES_mapper140_Reset()
{
	// set CPU bank pointers
	g_NESmapper.set_CPU_banks4(0,1,2,3);
//	set_CPU_banks(0,1,g_NESmapper.num_8k_ROM_banks-2,g_NESmapper.num_8k_ROM_banks-1);
	// set PPU bank pointers
	if(g_NESmapper.num_1k_VROM_banks)
	{
		g_NESmapper.set_PPU_banks8(0,1,2,3,4,5,6,7);
	}
}



STATIC void NES_mapper140_MemoryWriteSaveRAM(u32 addr, u8 data)
{
//	LOG("W " << HEX(addr,4) << "  " << HEX(data,2)  << endl);
	u8 prg_bank = (data & 0xF0) >> 4;
	u8 chr_bank = data & 0x0F;

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

//	set_CPU_bank5(data+1);
}
/////////////////////////////////////////////////////////////////////

