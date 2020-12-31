STATIC void NES_mapper117_Init();
STATIC void NES_mapper117_Reset();
STATIC void NES_mapper117_MemoryWrite(u32 addr, u8 data);
STATIC void NES_mapper117_HSync(u32 scanline);

/////////////////////////////////////////////////////////////////////
// Mapper 117
STATIC void NES_mapper117_Init()
{
	g_NESmapper.Reset = NES_mapper117_Reset;
	g_NESmapper.MemoryWrite = NES_mapper117_MemoryWrite;
	g_NESmapper.HSync = NES_mapper117_HSync;
}

STATIC void NES_mapper117_Reset()
{
	// set CPU bank pointers
	g_NESmapper.set_CPU_banks4(0,1,g_NESmapper.num_8k_ROM_banks-2,g_NESmapper.num_8k_ROM_banks-1);

	// set PPU bank pointers
	if(g_NESmapper.num_1k_VROM_banks)
	{
		g_NESmapper.set_PPU_banks8(0,1,2,3,4,5,6,7);
	}

	g_NESmapper.Mapper117.irq_line = 0;
	g_NESmapper.Mapper117.irq_enabled1 = 0;
	g_NESmapper.Mapper117.irq_enabled2 = 1;
}

STATIC void NES_mapper117_MemoryWrite(u32 addr, u8 data)
{
	switch(addr)
	{
	case 0x8000:
		{
			g_NESmapper.set_CPU_bank4(data);
		}
		break;

	case 0x8001:
		{
			g_NESmapper.set_CPU_bank5(data);
		}
		break;

	case 0x8002:
		{
			g_NESmapper.set_CPU_bank6(data);
		}
		break;

	case 0xA000:
		{
			g_NESmapper.set_PPU_bank0(data);
		}
		break;

	case 0xA001:
		{
			g_NESmapper.set_PPU_bank1(data);
		}
		break;

	case 0xA002:
		{
			g_NESmapper.set_PPU_bank2(data);
		}
		break;

	case 0xA003:
		{
			g_NESmapper.set_PPU_bank3(data);
		}
		break;

	case 0xA004:
		{
			g_NESmapper.set_PPU_bank4(data);
		}
		break;

	case 0xA005:
		{
			g_NESmapper.set_PPU_bank5(data);
		}
		break;

	case 0xA006:
		{
			g_NESmapper.set_PPU_bank6(data);
		}
		break;

	case 0xA007:
		{
			g_NESmapper.set_PPU_bank7(data);
		}
		break;

	case 0xA008:
	case 0xA009:
	case 0xA00a:
	case 0xA00b:
	case 0xA00c:
	case 0xA00d:
	case 0xA00e:
	case 0xA00f:
		break;

	case 0xC001:
	case 0xC002:
	case 0xC003:
		{
			g_NESmapper.Mapper117.irq_enabled1 = g_NESmapper.Mapper117.irq_line = data;
		}
		break;

	case 0xE000:
		{
			g_NESmapper.Mapper117.irq_enabled2 = data & 1;
		}
		break;
	}
}

STATIC void NES_mapper117_HSync(u32 scanline)
{
	if(g_NESmapper.Mapper117.irq_enabled1 && g_NESmapper.Mapper117.irq_enabled2 && g_NESmapper.Mapper117.irq_line == scanline)
	{
		g_NESmapper.Mapper117.irq_enabled1 = 0;
		NES6502_DoIRQ();
	}
}
/////////////////////////////////////////////////////////////////////

