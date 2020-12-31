#ifdef _NES_MAPPER_CPP_

STATIC void NES_mapper122_Init();
STATIC void NES_mapper122_Reset();
STATIC void NES_mapper122_MemoryWriteSaveRAM(u32 addr, u8 data);


/////////////////////////////////////////////////////////////////////
// Mapper 122
STATIC void NES_mapper122_Init()
{
	g_NESmapper.Reset = NES_mapper122_Reset;
	g_NESmapper.MemoryWriteSaveRAM = NES_mapper122_MemoryWriteSaveRAM;
}

STATIC void NES_mapper122_Reset()
{
	// set CPU bank pointers
	g_NESmapper.set_CPU_banks4(0,1,2,3);
	g_NESmapper.Mapper122.patch=0;
	if(NES_crc32_all() == 0x079eca27e)		//AtlantisNoNazo
		g_NESmapper.Mapper122.patch = 1;
}


STATIC void NES_mapper122_MemoryWriteSaveRAM(u32 addr, u8 data)
{
	if(g_NESmapper.Mapper122.patch){
		int h = (data&0x20)>>2;
		int l = ((data&2)<<2) | (data&4);
		g_NESmapper.set_PPU_banks8(l, l+1, l+2, l+3, h, h+1, h+2, h+3);
		return;
	}
	if(addr == 0x6000)
	{
		u8 chr_bank0 = data & 0x07;
		u8 chr_bank1 = (data & 0x70) >> 4;

		g_NESmapper.set_PPU_bank0(chr_bank0*4+0);
		g_NESmapper.set_PPU_bank1(chr_bank0*4+1);
		g_NESmapper.set_PPU_bank2(chr_bank0*4+2);
		g_NESmapper.set_PPU_bank3(chr_bank0*4+3);
		g_NESmapper.set_PPU_bank4(chr_bank1*4+0);
		g_NESmapper.set_PPU_bank5(chr_bank1*4+1);
		g_NESmapper.set_PPU_bank6(chr_bank1*4+2);
		g_NESmapper.set_PPU_bank7(chr_bank1*4+3);
	}
}

/////////////////////////////////////////////////////////////////////

#endif
