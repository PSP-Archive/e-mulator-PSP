#ifdef _NES_MAPPER_CPP_

STATIC void NES_mapper246_Init();
STATIC void NES_mapper246_Reset();
STATIC void NES_mapper246_MemoryWriteSaveRAM(u32 addr, u8 data);

/////////////////////////////////////////////////////////////////////
// Mapper 246
STATIC void NES_mapper246_Init()
{
	g_NESmapper.Reset = NES_mapper246_Reset;
	g_NESmapper.MemoryWriteSaveRAM = NES_mapper246_MemoryWriteSaveRAM;
}

STATIC void NES_mapper246_Reset()
{
	// set CPU bank pointers
	g_NESmapper.set_CPU_banks4(0,1,g_NESmapper.num_8k_ROM_banks-2,g_NESmapper.num_8k_ROM_banks-1);
}

STATIC void NES_mapper246_MemoryWriteSaveRAM(u32 addr, u8 data)
{
	switch(addr)
	{
	case 0x6000:
		{
			g_NESmapper.set_CPU_bank4(data);
		}
		break;

	case 0x6001:
		{
			g_NESmapper.set_CPU_bank5(data);
		}
		break;

	case 0x6002:
		{
			g_NESmapper.set_CPU_bank6(data);
		}
		break;

	case 0x6003:
		{
			g_NESmapper.set_CPU_bank7(data);
		}
		break;

	case 0x6004:
		{
			g_NESmapper.set_PPU_bank0(data*2+0);
			g_NESmapper.set_PPU_bank1(data*2+1);
		}
		break;

	case 0x6005:
		{
			g_NESmapper.set_PPU_bank2(data*2+0);
			g_NESmapper.set_PPU_bank3(data*2+1);
		}
		break;

	case 0x6006:
		{
			g_NESmapper.set_PPU_bank4(data*2+0);
			g_NESmapper.set_PPU_bank5(data*2+1);
		}
		break;

	case 0x6007:
		{
			g_NESmapper.set_PPU_bank6(data*2+0);
			g_NESmapper.set_PPU_bank7(data*2+1);
		}
		break;
	}
}
/////////////////////////////////////////////////////////////////////

#endif
