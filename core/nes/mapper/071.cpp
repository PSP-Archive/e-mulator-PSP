#ifdef _NES_MAPPER_CPP_

STATIC void NES_mapper71_Init();
STATIC void NES_mapper71_Reset();
STATIC void NES_mapper71_MemoryWriteSaveRAM(u32 addr, u8 data);
STATIC void NES_mapper71_MemoryWrite(u32 addr, u8 data);


/////////////////////////////////////////////////////////////////////
// Mapper 71
STATIC void NES_mapper71_Init()
{
	g_NESmapper.Reset = NES_mapper71_Reset;
	g_NESmapper.MemoryWriteSaveRAM = NES_mapper71_MemoryWriteSaveRAM;
	g_NESmapper.MemoryWrite = NES_mapper71_MemoryWrite;
}

STATIC void NES_mapper71_Reset()
{
	// set CPU bank pointers
	g_NESmapper.set_CPU_banks4(0,1,g_NESmapper.num_8k_ROM_banks-2,g_NESmapper.num_8k_ROM_banks-1);
}

STATIC void NES_mapper71_MemoryWriteSaveRAM(u32 addr, u8 data)
{
	g_NESmapper.set_CPU_bank4(data*2+0);
	g_NESmapper.set_CPU_bank5(data*2+1);
}

STATIC void NES_mapper71_MemoryWrite(u32 addr, u8 data)
{
	switch(addr & 0xF000)
	{
	case 0x9000:
		{
			if(data & 0x10)
			{
				g_NESmapper.set_mirroring(1,1,1,1);
			}
			else
			{
				g_NESmapper.set_mirroring(0,0,0,0);
			}
		}
		break;

	case 0xC000:
	case 0xD000:
	case 0xE000:
	case 0xF000:
		{
			g_NESmapper.set_CPU_bank4(data*2+0);
			g_NESmapper.set_CPU_bank5(data*2+1);
		}
		break;
	}
}
/////////////////////////////////////////////////////////////////////

#endif
