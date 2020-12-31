#ifdef _NES_MAPPER_CPP_

STATIC void NES_mapper188_Init();
STATIC void NES_mapper188_Reset();
STATIC void NES_mapper188_MemoryWrite(u32 addr, u8 data);


/////////////////////////////////////////////////////////////////////
// Mapper 188
STATIC void NES_mapper188_Init()
{
	g_NESmapper.Reset = NES_mapper188_Reset;
	g_NESmapper.MemoryWrite = NES_mapper188_MemoryWrite;
}

STATIC void NES_mapper188_Reset()
{
	nes6502_context context;
	// set CPU bank pointers
	if(g_NESmapper.num_8k_ROM_banks > 16)
	{
		g_NESmapper.set_CPU_banks4(0,1,14,15);
	}
	else
	{
		g_NESmapper.set_CPU_banks4(0,1,g_NESmapper.num_8k_ROM_banks-2,g_NESmapper.num_8k_ROM_banks-1);
	}

	NES6502_GetContext(&context);
	context.mem_page[3] = g_NESmapper.Mapper188.dummy;
	NES6502_SetContext(&context);

	g_NESmapper.Mapper188.dummy[0] = 0x03; // ?
}

// Coded by rinao
STATIC void NES_mapper188_MemoryWrite(u32 addr, u8 data)
{
	if (data)
	{
		if (data & 0x10)
		{
			data &= 0x07;
			g_NESmapper.set_CPU_bank4(data*2);
			g_NESmapper.set_CPU_bank5(data*2+1);
		}
		else
		{
			g_NESmapper.set_CPU_bank4(data*2+16);
			g_NESmapper.set_CPU_bank5(data*2+17);
		}
	}
	else
	{
		if (g_NESmapper.num_8k_ROM_banks == 0x10)
		{
			g_NESmapper.set_CPU_bank4(14);
			g_NESmapper.set_CPU_bank5(15);
		}
		else
		{
			g_NESmapper.set_CPU_bank4(16);
			g_NESmapper.set_CPU_bank5(17);
		}
	}
	//data &= g_NESmapper.num_8k_ROM_banks-1;
	//set_CPU_banks(data*2,(data*2)+1,g_NESmapper.num_8k_ROM_banks-2,g_NESmapper.num_8k_ROM_banks-1);
}
/////////////////////////////////////////////////////////////////////

#endif
