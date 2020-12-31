//---------------------------------------------------------------------------
// NEOPOP : Emulator as in Dreamland
//
// Copyright (c) 2001-2002 by neopop_uk
//---------------------------------------------------------------------------
#include "neopop.h"
#include "mem.h"
#include "sound.h"
#include "Z80_interface.h"
#include "./tlcs-900h/interpret.h"
#include "interrupt.h"
#include "dma.h"

//=============================================================================

int DAsm(char *S,byte *A);

Z80 Z80_regs;

//=============================================================================

u8 RdZ80(u16 address)
{
	if (address <= 0xFFF) {
		return ram[0x7000 + address];
	}

	if (address == 0x8000) {
#ifdef NEOPOP_DEBUG
		if (filter_sound)
			system_debug_message("z80 <- TLCS900h: Read ... %02X", ram[0xBC]);
#endif
		return ram[0xBC];
	}

	return 0;
}

//=============================================================================

void WrZ80(u16 address, u8 value)
{
	if (address <= 0x0FFF) {
		ram[0x7000 + address] = value;
		return;
	}

	if (address == 0x8000) {
#ifdef NEOPOP_DEBUG
		if (filter_sound)
			system_debug_message("z80 -> TLCS900h: Write ... %02X", value);
#endif
		ram[0xBC] = value;
		return;
	}

	if (address == 0x4001)	{	
		Write_SoundChipTone(value);
		return; 
	}

	if (address == 0x4000)	{	
		Write_SoundChipNoise(value); 
		return; 
	}

	if (address == 0xC000 && (statusIFF() <= (ram[0x71] & 0x7))){
		interrupt(6); // Z80 Int.

		if(ram[0x007C]==0x0C) DMA_update(0); else  
		if(ram[0x007D]==0x0C) DMA_update(1); else  
		if(ram[0x007E]==0x0C) DMA_update(2); else  
		if(ram[0x007F]==0x0C) DMA_update(3);	
	}
}

//=============================================================================
/* // Žg‚í‚È‚¢‚È‚çŒÄ‚Î‚È‚¢

void OutZ80(u16 port, u8 value)
{
#ifdef NEOPOP_DEBUG
//	if (filter_sound) system_debug_message("Z80: Port out %04X <= %02X", port, value);
#endif
}

u8 InZ80(u16 port)
{
#ifdef NEOPOP_DEBUG
//	if (filter_sound) system_debug_message("Z80: Port in %04X", port);
#endif
	return 0;
}
//=============================================================================
void PatchZ80(register Z80 *R)
{
	// Empty
}

word LoopZ80(register Z80 *R)
{
	return INT_QUIT;
}
*/

//=============================================================================

void Z80_nmi(void)
{
	IntZ80(&Z80_regs, INT_NMI);
}

void Z80_irq(void)
{
	Z80_regs.IFF |= IFF_1;
	IntZ80(&Z80_regs, INT_IRQ);
}

void Z80_reset(void)
{
	ResetZ80(&Z80_regs);
	Z80_regs.SP.W = 0;
}

//=============================================================================

u16 Z80_getReg(u8 reg)
{
	u16* r = (u16*)&Z80_regs;
	return r[reg];
}

void Z80_setReg(u8 reg, u16 value)
{
	u16* r = (u16*)&Z80_regs;
	r[reg] = value;
}

//=============================================================================

#if 0
char* Z80_disassemble(u16* pc_in)
{
	int bcnt, i;
	u16 pc = *pc_in;
	char instr[64];	//Print the disassembled instruction to this string
	u8 str[80];
	core_memset(str, 0, 80);
	
	//Add the program counter
	sprintf(str, "<z80> %03X: ", pc);

	//Disassemble instruction
    bcnt = DAsm(instr,ram + 0x7000 + pc);

	//Add the instruction
	strcat(str, instr);

	//Add the bytes used
	for (i = core_strlen(str); i < 32; i++)
		str[i] = ' ';
	str[32] = '\"';
	for (i = 0; i < bcnt; i++)
	{
		u8 tmp[80];
		sprintf(tmp, "%02X ", *(ram + 0x7000 + pc + i));
		strcat(str, tmp);
	}
	str[core_strlen(str) - 1] = '\"';

	*pc_in = pc + bcnt;
	return strdup(str);
}
#endif

//=============================================================================
