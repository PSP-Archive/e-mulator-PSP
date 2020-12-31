//---------------------------------------------------------------------------
// NEOPOP : Emulator as in Dreamland
//
// Copyright (c) 2001-2002 by neopop_uk
//---------------------------------------------------------------------------

#include "neopop.h"
//#include "tlcs-900h/disassemble.h"
#include "dma.h"
#include "mem.h"
#include "interrupt.h"
#include "hal.h"
#include "cstring.h"

//=============================================================================

//u32 dmaS[4], dmaD[4];
//u16 dmaC[4];
//u8 dmaM[4];

//=============================================================================

void reset_dma(void)
{
	core_memset(dmaS, 0, sizeof(dmaS));
	core_memset(dmaD, 0, sizeof(dmaD));
	core_memset(dmaC, 0, sizeof(dmaC));
	core_memset(dmaM, 0, sizeof(dmaM));
}

//=============================================================================

void DMA_update(int channel)
{
	u8 mode = (dmaM[channel] & 0x1C) >> 2;
	u8 size = (dmaM[channel] & 0x03); //byte, word or long

	// Correct?
	if (dmaC[channel] == 0)
		return;

	switch (mode) {
	case 0:	// Destination INC mode, I/O to Memory transfer
		switch(size) {
		case 0: storeB(dmaD[channel], loadB(dmaS[channel])); dmaD[channel]+=1; /* Byte increment */ break;
		case 1:	storeW(dmaD[channel], loadW(dmaS[channel])); dmaD[channel]+=2; /* Word increment */ break;
		case 2:	storeL(dmaD[channel], loadL(dmaS[channel])); dmaD[channel]+=4; /* Long increment */ break;
		}
		break;
	
	case 1:	// Destination DEC mode, I/O to Memory transfer
		switch(size) {
		case 0:	storeB(dmaD[channel], loadB(dmaS[channel])); dmaD[channel]-=1; /* Byte decrement */ break;
		case 1:	storeW(dmaD[channel], loadW(dmaS[channel])); dmaD[channel]-=2; /* Word decrement */ break;
		case 2:	storeL(dmaD[channel], loadL(dmaS[channel])); dmaD[channel]-=4; /* Long decrement */ break;
		}
		break;

	case 2:	// Source INC mode, Memory to I/O transfer
		switch(size) {
		case 0:	storeB(dmaD[channel], loadB(dmaS[channel])); dmaS[channel]+=1; /* Byte increment */ break;
		case 1:	storeW(dmaD[channel], loadW(dmaS[channel])); dmaS[channel]+=2; /* Word increment */ break;
		case 2:	storeL(dmaD[channel], loadL(dmaS[channel])); dmaS[channel]+=4; /* Long increment */ break;
		}
		break;

	case 3:	// Source DEC mode, Memory to I/O transfer
		switch(size) {
		case 0:	storeB(dmaD[channel], loadB(dmaS[channel])); dmaS[channel]-=1; /* Byte decrement */ break;
		case 1:	storeW(dmaD[channel], loadW(dmaS[channel])); dmaS[channel]-=2; /* Word decrement */ break;
		case 2:	storeL(dmaD[channel], loadL(dmaS[channel])); dmaS[channel]-=4; /* Long decrement */ break;
		}
		break;

	case 4:	// Fixed Address Mode
		switch(size) {
		case 0:	storeB(dmaD[channel], loadB(dmaS[channel])); break;
		case 1:	storeW(dmaD[channel], loadW(dmaS[channel])); break;
		case 2:	storeL(dmaD[channel], loadL(dmaS[channel])); break;
		}
		break;

	case 5: // Counter Mode
		dmaS[channel] ++;
		break;

	default:
//		system_message("Bad DMA mode %d\nPlease report this to the author.", dmaM[channel]);
		return;
	}

	// Perform common counter decrement,
	// vector clearing, and interrupt handling.

	dmaC[channel] --;
	if (dmaC[channel] == 0){
		interrupt(14 + channel);
		ram[0x7C + channel] = 0;
	}
}

//=============================================================================

void dmaStoreB(u8 cr, u8 data)
{
	switch(cr){
	case 0x22:	dmaM[0] = data;	break;
	case 0x26:	dmaM[1] = data;	break;
	case 0x2A:	dmaM[2] = data;	break;
	case 0x2E:	dmaM[3] = data;	break;
	default:                    break;
	}
}

void dmaStoreW(u8 cr, u16 data)
{
	switch(cr){
	case 0x20:	dmaC[0] = data;	break;
	case 0x24:	dmaC[1] = data;	break;
	case 0x28:	dmaC[2] = data;	break;
	case 0x2C:	dmaC[3] = data;	break;
	default:                    break;
	}
}

void dmaStoreL(u8 cr, u32 data)
{
	switch(cr) {
	case 0x00: dmaS[0] = data; break;	
	case 0x04: dmaS[1] = data; break;	
	case 0x08: dmaS[2] = data; break;	
	case 0x0C: dmaS[3] = data; break;	
	case 0x10: dmaD[0] = data; break;	
	case 0x14: dmaD[1] = data; break;	
	case 0x18: dmaD[2] = data; break;	
	case 0x1C: dmaD[3] = data; break;
	default:                   break;
	}
}

//=============================================================================

u8 dmaLoadB(u8 cr)
{
    switch(cr) {
    case 0x22: return dmaM[0];	break;
    case 0x26: return dmaM[1];	break;
    case 0x2A: return dmaM[2];	break;
    case 0x2E: return dmaM[3];	break;
    default:   return 0;
    }
}

u16 dmaLoadW(u8 cr)
{
	switch(cr){
	case 0x20:	return dmaC[0];	break;
	case 0x24:	return dmaC[1];	break;
	case 0x28:	return dmaC[2];	break;
	case 0x2C:	return dmaC[3];	break;
	default:    return 0;
	}
}

u32 dmaLoadL(u8 cr)
{
	switch(cr){
	case 0x00: return dmaS[0]; break;	
	case 0x04: return dmaS[1]; break;	
	case 0x08: return dmaS[2]; break;	
	case 0x0C: return dmaS[3]; break;	
	case 0x10: return dmaD[0]; break;	
	case 0x14: return dmaD[1]; break;	
	case 0x18: return dmaD[2]; break;	
	case 0x1C: return dmaD[3]; break;
	default:   return 0;
	}
}

//=============================================================================
