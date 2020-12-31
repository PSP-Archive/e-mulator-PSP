#ifdef _NES_MAPPER_CPP_

STATIC void NES_UNIFmapper3_Init();
STATIC void NES_UNIFmapper3_Reset();
STATIC void NES_UNIFmapper3_Setprg();
STATIC void NES_UNIFmapper3_MemoryWriteSaveRAM(u32 addr, u8 data);
STATIC void NES_UNIFmapper3_MemoryWrite(u32 addr, u8 data);


// UNIF   Supervision 16 in 1
STATIC void NES_UNIFmapper3_Init()
{
	g_NESmapper.Reset = NES_UNIFmapper3_Reset;
	g_NESmapper.MemoryWrite = NES_UNIFmapper3_MemoryWrite;
	g_NESmapper.MemoryWriteSaveRAM = NES_UNIFmapper3_MemoryWriteSaveRAM;
}

STATIC void NES_UNIFmapper3_Reset()
{
	// set CPU bank pointers
	g_NESmapper.set_CPU_banks4(0x100,0x101,0x102,0x103);
	// set PPU bank pointers
//	set_PPU_banks(0,1,2,3,4,5,6,7);
//	set_mirroring(NES_PPU_MIRROR_VERT);
	g_NESmapper.NES_UNIFmapper3.regs[0] = g_NESmapper.NES_UNIFmapper3.regs[1] = 0;
}


STATIC void NES_UNIFmapper3_Setprg()
{
	g_NESmapper.set_CPU_bank_unif(((g_NESmapper.NES_UNIFmapper3.regs[0]&0xF)<<4)|0xF, 3);
	if(g_NESmapper.NES_UNIFmapper3.regs[0]&0x10){
		u32 tmb = (g_NESmapper.NES_UNIFmapper3.regs[0]&0xF)<<4;
		g_NESmapper.set_CPU_bank_unif((tmb|(((g_NESmapper.NES_UNIFmapper3.regs[1]&7)<<1))  ), 4);
        g_NESmapper.set_CPU_bank_unif((tmb|(((g_NESmapper.NES_UNIFmapper3.regs[1]&7)<<1)+1)), 5);
		g_NESmapper.set_CPU_bank_unif((tmb|0x0E), 6);
		g_NESmapper.set_CPU_bank_unif((tmb|0x0E)+1, 7);
	}
	else{
		g_NESmapper.set_CPU_banks4(0x100,0x101,0x102,0x103);
	}

	if(g_NESmapper.NES_UNIFmapper3.regs[0]&0x20){
		g_NESmapper.set_mirroring2(NES_PPU_MIRROR_HORIZ);
	}
	else{
		g_NESmapper.set_mirroring2(NES_PPU_MIRROR_VERT);
	}
}


STATIC void NES_UNIFmapper3_MemoryWriteSaveRAM(u32 addr, u8 data)
{
	if(!(g_NESmapper.NES_UNIFmapper3.regs[0]&0x10)){
		g_NESmapper.NES_UNIFmapper3.regs[0]=data;
		NES_UNIFmapper3_Setprg();
	}
}

STATIC void NES_UNIFmapper3_MemoryWrite(u32 addr, u8 data)
{
	g_NESmapper.NES_UNIFmapper3.regs[1]=data;
	NES_UNIFmapper3_Setprg();
}

#endif
