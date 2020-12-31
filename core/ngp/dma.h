//---------------------------------------------------------------------------
// NEOPOP : Emulator as in Dreamland
//
// Copyright (c) 2001-2002 by neopop_uk
//---------------------------------------------------------------------------

#ifndef __DMA__
#define __DMA__
//=============================================================================

void reset_dma(void);

void DMA_update(int channel);

//extern u32 dmaS[4], dmaD[4];
//extern u16 dmaC[4];
//extern u8 dmaM[4];

u8  dmaLoadB(u8 cr);
u16 dmaLoadW(u8 cr);
u32 dmaLoadL(u8 cr);

void dmaStoreB(u8 cr, u8 data);
void dmaStoreW(u8 cr, u16 data);
void dmaStoreL(u8 cr, u32 data);

//=============================================================================
#endif
