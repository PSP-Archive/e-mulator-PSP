STATIC void NES_mapper73_Init();
STATIC void NES_mapper73_Reset();
STATIC void NES_mapper73_MemoryWrite(u32 addr, u8 data);
STATIC void NES_mapper73_HSync(u32 scanline);

/////////////////////////////////////////////////////////////////////
// Mapper 73
STATIC void NES_mapper73_Init()
{
	g_NESmapper.Reset = NES_mapper73_Reset;
	g_NESmapper.MemoryWrite = NES_mapper73_MemoryWrite;
	g_NESmapper.HSync = NES_mapper73_HSync;
}

STATIC void NES_mapper73_Reset()
{
	// set CPU bank pointers
	g_NESmapper.set_CPU_banks4(0,1,g_NESmapper.num_8k_ROM_banks-2,g_NESmapper.num_8k_ROM_banks-1);

	g_NESmapper.Mapper73.irq_counter = 0;
	g_NESmapper.Mapper73.irq_enabled = 0;
}

STATIC void NES_mapper73_MemoryWrite(u32 addr, u8 data)
{
	switch(addr)
	{
	case 0x8000:
		{
			g_NESmapper.Mapper73.irq_counter = (g_NESmapper.Mapper73.irq_counter & 0xFFF0) | (data & 0x0F);
		}
		break;

	case 0x9000:
		{
			g_NESmapper.Mapper73.irq_counter = (g_NESmapper.Mapper73.irq_counter & 0xFF0F) | ((data & 0x0F) << 4);
		}
		break;

	case 0xA000:
		{
			g_NESmapper.Mapper73.irq_counter = (g_NESmapper.Mapper73.irq_counter & 0xF0FF) | ((data & 0x0F) << 8);
		}
		break;

	case 0xB000:
		{
			g_NESmapper.Mapper73.irq_counter = (g_NESmapper.Mapper73.irq_counter & 0x0FFF) | ((data & 0x0F) << 12);
		}
		break;

	case 0xC000:
		{
			g_NESmapper.Mapper73.irq_enabled = data;
		}
		break;

	case 0xF000:
		{
			g_NESmapper.set_CPU_bank4(data*2+0);
			g_NESmapper.set_CPU_bank5(data*2+1);
		}
		break;
	}
}

STATIC void NES_mapper73_HSync(u32 scanline)
{
	if(g_NESmapper.Mapper73.irq_enabled & 0x02)
	{
		if(g_NESmapper.Mapper73.irq_counter > 0xFFFF - 114)
		{
			NES6502_DoIRQ();
			g_NESmapper.Mapper73.irq_enabled = 0;
		}
		else
		{
			g_NESmapper.Mapper73.irq_counter += 114;
		}
	}
}
/////////////////////////////////////////////////////////////////////

