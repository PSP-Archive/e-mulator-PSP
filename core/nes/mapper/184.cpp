#ifdef _NES_MAPPER_CPP_

STATIC void NES_mapper184_Init();
STATIC void NES_mapper184_Reset();
STATIC void NES_mapper184_MemoryWriteSaveRAM(u32 addr, u8 data);


/////////////////////////////////////////////////////////////////////
// Mapper 184
STATIC void NES_mapper184_Init()
{

}

STATIC void NES_mapper184_Reset()
{
	// set CPU bank pointers
	set_CPU_banks(0, 1, g_NESmapper.num_8k_ROM_banks-2, g_NESmapper.num_8k_ROM_banks-1);

	// set PPU bank pointers
	if(g_NESmapper.num_1k_VROM_banks)
	{
		set_PPU_banks(0,1,2,3,4,5,6,7);
	}
}


STATIC void NES_mapper184_MemoryWriteSaveRAM(u32 addr, u8 data)
{
//	LOG("W " << HEX(addr,4) << "  " << HEX(data,2) << endl);
	int h = (data&0x20)>>2;
	int l = ((data&2)<<2) | (data&4);
	set_PPU_banks(l, l+1, l+2, l+3, h, h+1, h+2, h+3);
}


/////////////////////////////////////////////////////////////////////

#endif
