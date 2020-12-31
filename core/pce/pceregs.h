// ワークデータをmallocで取得するように！

//
// pceregs.h
//
// 変数定義と実体定義
//
#ifndef __PCEREGS_H_DEFINED__
#define __PCEREGS_H_DEFINED__

#ifndef EXTERN
#define EXTERN extern
#define INIT_VALUE(value)
#endif//EXTERN

#ifndef INIT_VALUE
#define INIT_VALUE(value) =(value)
#endif

#define io  (*pIO)
#define cd  (*pCD)
#define acd (*pACD)

EXTERN PCE_IO  *pIO;
EXTERN PCE_CD  *pCD;
EXTERN PCE_ACD *pACD;

#define SIZEOF_PCE_WRAM   0x2000

EXTERN byte* PCE_WRAM; // [0x2000];
EXTERN byte* PCE_RAM;//           INIT_VALUE(pIO->RAM);

EXTERN int BaseClock;




#define pcc (*pCache)

EXTERN PCE_CACHE *pCache;

#define IOAREA   pcc.IOAREA
#define VRAM2    pcc.VRAM2
#define VRAMS    pcc.VRAMS
#define vchange  pcc.vchange
#define vchanges pcc.vchanges
#define PopRAM   pcc.PopRAM


EXTERN byte *PCE_Page[8];
EXTERN byte *PCE_ROMMap[256];


EXTERN int TimerCount;
//EXTERN int CycleOld;
EXTERN int TimerPeriod;
EXTERN int scanlines_per_frame INIT_VALUE(263);
//EXTERN int prevline;
//EXTERN int scanline;

EXTERN byte populus;
EXTERN byte *ROM;
EXTERN int ROM_size;
EXTERN int Country;
EXTERN int IPeriod;


#endif
