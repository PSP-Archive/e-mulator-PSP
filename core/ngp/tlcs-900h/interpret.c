#define DEF_PARITY

//---------------------------------------------------------------------------
// NEOPOP : Emulator as in Dreamland
//
// Copyright (c) 2001-2002 by neopop_uk
//---------------------------------------------------------------------------

#define STATIC static
#define OPT_FETCH


#include "../neopop.h"
#include "../interrupt.h"
#include "../mem.h"
#include "../bios.h"
#include "interpret.h"
#include "cstring.h"

STATIC u8 rCodeConversionB[8] = { 0xE1, 0xE0, 0xE5, 0xE4, 0xE9, 0xE8, 0xED, 0xEC };
STATIC u8 rCodeConversionW[8] = { 0xE0, 0xE4, 0xE8, 0xEC, 0xF0, 0xF4, 0xF8, 0xFC };
STATIC u8 rCodeConversionL[8] = { 0xE0, 0xE4, 0xE8, 0xEC, 0xF0, 0xF4, 0xF8, 0xFC };

#ifdef DEF_PARITY
static u8 parity16[0x10000];
#endif

//static u8 statusRFP;

#if 1

typedef struct tlcs900h {
    u32 mem;
    u32 size; //int  size;
    u8  first;
    u8  second;
    u8  R;
    BOOL brCode;
    u8  rCode;
    u8  cycles_extra;
    u32 rErr;
    u8 f_dash;
} TLCS900h;

TLCS900h tl9h;

u8  cycles;
u32 gprBank[4][4], gpr[4];
u16 sr;


#ifdef PC32ADR
u8* pc32_adr=0;
#else
u32 pc32;
#endif


#define mem          tl9h.mem
#define size         tl9h.size
#define first        tl9h.first
#define second       tl9h.second
#define R            tl9h.R
#define brCode       tl9h.brCode
#define rCode        tl9h.rCode
#define cycles_extra tl9h.cycles_extra
#define rErr         tl9h.rErr
#define f_dash       tl9h.f_dash
//#define pc32           tl9h.p
//#define sr           tl9h.sr
#else
//STATIC u32     mem;		  // Result of addressing mode
//STATIC int      size;		  // operand size, 0 = Byte, 1 = Word, 2 = Long
//STATIC u8      first;		  // The first byte
//STATIC u8      R;			  // big R
//STATIC u8      second;       // The second opcode
//STATIC BOOL     brCode;       // Register code used?
//STATIC u8      rCode;        // The code
//
//STATIC u8		cycles_extra; // How many extra state changes?
//u8		        cycles;       // How many state changes?
//
//STATIC u32     rErr;
//
//u32 pc32, gprBank[4][4], gpr[4];
//u16 sr;
//u8 f_dash;
//
#endif

unsigned char *ng_map[0x100];
unsigned char map_null[0x10000];

void ngp_make_map(void)
{
	int i;
    for(i=0;i<0x100;i++) {
        ng_map[i] = map_null;
    }
    
    for(i=0;i<0x20;i++) {
        ng_map[0x20+i] = &rom.data[0x00000000 + (i<<16)/**0x00010000*/];
        ng_map[0x80+i] = &rom.data[0x00200000 + (i<<16)/**0x00010000*/];
    }
	ng_map[0x00] = ram;
    ng_map[0xff] = bios;
}

#if 1

#define FETCH8() fetch8()

#if 1
#define ADR_CHECK()
#else
#define ADR_CHECK() \
	if( p != &(ng_map[(u8)(pc32>>16)])[((u16)pc32)] ) { \
		p=p;\
	}
#endif

//
// ROMからのロードに限定される機能は高速化が可能
//
u8  fetch8(void)
{
#ifdef OPT_FETCH
#ifdef PC32ADR
	u8* p = pc32_adr; 
	ADR_CHECK();
	pc32_adr++;
#else
	u8* p = &(ng_map[(u8)(pc32>>16)])[((u16)pc32)];
	pc32++;
#endif
	return p[0];
#else//OPT_FETCH
	_u8 a = loadB(pc32);
	pc32 += 1;
	return a;
#endif//OPT_FETCH
}

u16 fetch16(void)
{
#ifdef OPT_FETCH
#ifdef PC32ADR
	u8* p = pc32_adr; 
	ADR_CHECK();
	pc32_adr+=2;
#else
	u8* p = &(ng_map[(u8)(pc32>>16)])[((u16)pc32)];
	pc32+=2;
#endif
	return ((u16)p[1]<<8) | p[0];
#else//OPT_FETCH
	_u16 a = loadW(pc32);
	pc32 += 2;
	return a;
#endif//OPT_FETCH
}

u32 fetch24(void)
{
#ifdef OPT_FETCH
#ifdef PC32ADR
	u8* p = pc32_adr; 
	ADR_CHECK();
	pc32_adr+=3;
#else
	u8* p = &(ng_map[(u8)(pc32>>16)])[((u16)pc32)];
	pc32+=3;
#endif
	return ((u32)p[2]<<16) | ((u32)p[1]<<8) | p[0];
#else//OPT_FETCH
	_u32 b, a = loadW(pc32);
	pc32 += 2;
	b = loadB(pc32++);
	return (b << 16) | a;
#endif//OPT_FETCH
}

u32 fetch32(void)
{
#ifdef OPT_FETCH
#ifdef PC32ADR
	u8* p = pc32_adr;
	ADR_CHECK();
	pc32_adr+=4;
#else
	u8* p = &(ng_map[(u8)(pc32>>16)])[((u16)pc32)];
	pc32+=4;
#endif
	return ((u32)p[3]<<24) |((u32)p[2]<<16) | ((u32)p[1]<<8) | p[0];
#else//OPT_FETCH
	_u32 a = loadL(pc32);
	pc32 += 4;
	return a;
#endif//OPT_FETCH
}

#else

#define FETCH8()    loadB(pc32++)
STATIC u16 fetch16(void)
{
	u16 a = loadW(pc32);
	pc32 += 2;
	return a;
}

STATIC u32 fetch24(void)
{
	u32 b, a = loadW(pc32);
	pc32 += 2;
	b = loadB(pc32++);
	return (b << 16) | a;
}

STATIC u32 fetch32(void)
{
	u32 a = loadL(pc32);
	pc32 += 4;
	return a;
}
#endif

#if 1
//
// PUSH/POPは内部メモリにしかできないのでメモリアクセスの改善が可能
// 
void push8(u8 data)	   { REGXSP-=1; ram[REGXSP]=data; }
void push16(u16 data)  { REGXSP-=2; ram[REGXSP]=data; ram[REGXSP+1]=data>>8; }
void push32(u32 data)  { REGXSP-=4; ram[REGXSP]=data; ram[REGXSP+1]=data>>8; ram[REGXSP+2]=data>>16; ram[REGXSP+3]=data>>24; }

#define R_GXSP0()       (((u32)ram[REGXSP]))
#define R_GXSP(n,shift) (((u32)ram[REGXSP+n])<<(shift))

u8 pop8(void)	{  u8 temp = R_GXSP0(); REGXSP+=1; return temp;}
u16 pop16(void) { u16 temp = R_GXSP(1,8)|R_GXSP0(); REGXSP+=2; return temp;}
u32 pop32(void) { u32 temp = R_GXSP(3,24)|R_GXSP(2,16)|R_GXSP(1,8)|R_GXSP0(); REGXSP+=4; return temp; }

#else /* 1 */
//
//void push8(u8 data)     { REGXSP-=1; storeB(REGXSP, data); }
//void push16(u16 data)   { REGXSP-=2; storeW(REGXSP, data); }
//void push32(u32 data)   { REGXSP-=4; storeL(REGXSP, data); }
//u8   pop8(void)         {  u8 temp = loadB(REGXSP); REGXSP+=1; return temp; }
//u16  pop16(void)        { u16 temp = loadW(REGXSP); REGXSP+=2; return temp; }
//u32  pop32(void)        { u32 temp = loadL(REGXSP); REGXSP+=4; return temp; }
//
#endif/* 1 */



STATIC void parityB(u8 value)
{
#ifdef DEF_PARITY
    if( parity16[value] ) SETFLAG_V1
	else                  SETFLAG_V0
#else
    u8 count = 0, i;
    
    for (i = 0; i < 8; i++){
		if (value & 1) count++;
        value >>= 1;
    }

    // if (count & 1) == FALSE, means even, thus SET
	SETFLAG_V((count & 1) == 0);
#endif
}

STATIC void parityW(u16 value)
{
#ifdef DEF_PARITY
	if( parity16[value] ) SETFLAG_V1
	else                  SETFLAG_V0
#else
	u8 count = 0, i;

	for (i = 0; i < 16; i++) {
		if (value & 1) count++;
		value >>= 1;
	}

	// if (count & 1) == FALSE, means even, thus SET
	SETFLAG_V((count & 1) == 0);
#endif
}

STATIC u16 generic_DIV_B(u16 val, u8 div)
{
	if (div == 0) { 
		SETFLAG_V1
		return (val << 8) | ((val >> 8) ^ 0xFF);
	}
	else {
		u16 quo = val / (u16)div;
		u16 rem = val % (u16)div;
		if (quo > 0xFF) SETFLAG_V1 else SETFLAG_V0
		return (quo & 0xFF) | ((rem & 0xFF) << 8);
	}
}

STATIC u32 generic_DIV_W(u32 val, u16 div)
{
	if (div == 0)
	{ 
		SETFLAG_V1
		return (val << 16) | ((val >> 16) ^ 0xFFFF);
	}
	else
	{
		u32 quo = val / (u32)div;
		u32 rem = val % (u32)div;
		if (quo > 0xFFFF) SETFLAG_V1 else SETFLAG_V0
		return (quo & 0xFFFF) | ((rem & 0xFFFF) << 16);
	}
}

//=============================================================================

STATIC u16 generic_DIVS_B(s16 val, s8 div)
{
	if (div == 0)
	{
		SETFLAG_V1
		return (val << 8) | ((val >> 8) ^ 0xFF);
	}
	else
	{
		s16 quo = val / (s16)div;
		s16 rem = val % (s16)div;
		if (quo > 0xFF) SETFLAG_V1 else SETFLAG_V0
		return (quo & 0xFF) | ((rem & 0xFF) << 8);
	}
}

STATIC u32 generic_DIVS_W(s32 val, s16 div)
{
	if (div == 0)
	{
		SETFLAG_V1
		return (val << 16) | ((val >> 16) ^ 0xFFFF);
	}
	else
	{
		s32 quo = val / (s32)div;
		s32 rem = val % (s32)div;
		if (quo > 0xFFFF) SETFLAG_V1 else SETFLAG_V0
		return (quo & 0xFFFF) | ((rem & 0xFFFF) << 16);
	}
}

//=============================================================================

STATIC u8 generic_ADD_B(u8 dst, u8 src)
{
	u8 half = (dst & 0xF) + (src & 0xF);
	u32 resultC = (u32)dst + (u32)src;
	u8 result = (u8)(resultC & 0xFF);

	SETFLAG_S(result & 0x80);
	SETFLAG_Z(result == 0);
	SETFLAG_H(half > 0xF);

	if ((((s8)dst >= 0) && ((s8)src >= 0) && ((s8)result < 0)) ||
		(((s8)dst < 0)  && ((s8)src < 0) && ((s8)result >= 0)))
	{SETFLAG_V1} else {SETFLAG_V0}

	SETFLAG_N0;
	SETFLAG_C(resultC > 0xFF);

	return result;
}

STATIC u16 generic_ADD_W(u16 dst, u16 src)
{
	u16 half = (dst & 0xF) + (src & 0xF);
	u32 resultC = (u32)dst + (u32)src;
	u16 result = (u16)(resultC & 0xFFFF);

	SETFLAG_S(result & 0x8000);
	SETFLAG_Z(result == 0);
	SETFLAG_H(half > 0xF);

	if ((((s16)dst >= 0) && ((s16)src >= 0) && ((s16)result < 0)) ||
		(((s16)dst < 0)  && ((s16)src < 0) && ((s16)result >= 0)))
	{SETFLAG_V1} else {SETFLAG_V0}

	SETFLAG_N0;
	SETFLAG_C(resultC > 0xFFFF);

	return result;
}

STATIC u32 generic_ADD_L(u32 dst, u32 src)
{
#if 1
	u32 result = (dst + src) & 0xFFFFFFFF;

	SETFLAG_S(result & 0x80000000);
	SETFLAG_Z(result == 0);

	if ((((s32)dst >= 0) && ((s32)src >= 0) && ((s32)result < 0)) || 
		(((s32)dst < 0)  && ((s32)src < 0) && ((s32)result >= 0)))
	{SETFLAG_V1} else {SETFLAG_V0}
	
	SETFLAG_N0;
	SETFLAG_C( (0xffffffff-dst)<src );
#else
	_u64 resultC = (_u64)dst + (_u64)src;
	u32 result = (u32)(resultC & 0xFFFFFFFF);

	SETFLAG_S(result & 0x80000000);
	SETFLAG_Z(result == 0);

	if ((((s32)dst >= 0) && ((s32)src >= 0) && ((s32)result < 0)) || 
		(((s32)dst < 0)  && ((s32)src < 0) && ((s32)result >= 0)))
	{SETFLAG_V1} else {SETFLAG_V0}
	
	SETFLAG_N0;
	SETFLAG_C(resultC > 0xFFFFFFFF);
#endif
	return result;
}

//=============================================================================

STATIC u8 generic_ADC_B(u8 dst, u8 src)
{
	u8 half = (dst & 0xF) + (src & 0xF) + FLAG_C;
	u32 resultC = (u32)dst + (u32)src + (u32)FLAG_C;
	u8 result = (u8)(resultC & 0xFF);

	SETFLAG_S(result & 0x80);
	SETFLAG_Z(result == 0);
	SETFLAG_H(half > 0xF);

	if ((((s8)dst >= 0) && ((s8)src >= 0) && ((s8)result < 0)) || 
		(((s8)dst < 0)  && ((s8)src < 0) && ((s8)result >= 0)))
	{SETFLAG_V1} else {SETFLAG_V0}

	SETFLAG_N0;
	SETFLAG_C(resultC > 0xFF);

	return result;
}

STATIC u16 generic_ADC_W(u16 dst, u16 src)
{
	u16 half = (dst & 0xF) + (src & 0xF) + FLAG_C;
	u32 resultC = (u32)dst + (u32)src + (u32)FLAG_C;
	u16 result = (u16)(resultC & 0xFFFF);

	SETFLAG_S(result & 0x8000);
	SETFLAG_Z(result == 0);
	SETFLAG_H(half > 0xF);

	if ((((s16)dst >= 0) && ((s16)src >= 0) && ((s16)result < 0)) || 
		(((s16)dst < 0)  && ((s16)src < 0) && ((s16)result >= 0)))
	{SETFLAG_V1} else {SETFLAG_V0}

	SETFLAG_N0;
	SETFLAG_C(resultC > 0xFFFF);

	return result;
}

STATIC u32 generic_ADC_L(u32 dst, u32 src)
{
	_u64 resultC = (_u64)dst + (_u64)src + (_u64)FLAG_C;
	u32 result = (u32)(resultC & 0xFFFFFFFF);

	SETFLAG_S(result & 0x80000000);
	SETFLAG_Z(result == 0);

	if ((((s32)dst >= 0) && ((s32)src >= 0) && ((s32)result < 0)) || 
		(((s32)dst < 0)  && ((s32)src < 0) && ((s32)result >= 0)))
	{SETFLAG_V1} else {SETFLAG_V0}
	
	SETFLAG_N0;
	SETFLAG_C(resultC > 0xFFFFFFFF);

	return result;
}

//=============================================================================

STATIC u8 generic_SUB_B(u8 dst, u8 src)
{
	u8 half = (dst & 0xF) - (src & 0xF);
	u32 resultC = (u32)dst - (u32)src;
	u8 result = (u8)(resultC & 0xFF);

	SETFLAG_S(result & 0x80);
	SETFLAG_Z(result == 0);
	SETFLAG_H(half > 0xF);

	if ((((s8)dst >= 0) && ((s8)src < 0) && ((s8)result < 0)) ||
		(((s8)dst < 0) && ((s8)src >= 0) && ((s8)result >= 0)))
	{SETFLAG_V1} else {SETFLAG_V0}

	SETFLAG_N1;
	SETFLAG_C(resultC > 0xFF);

	return result;
}

STATIC u16 generic_SUB_W(u16 dst, u16 src)
{
	u16 half = (dst & 0xF) - (src & 0xF);
	u32 resultC = (u32)dst - (u32)src;
	u16 result = (u16)(resultC & 0xFFFF);

	SETFLAG_S(result & 0x8000);
	SETFLAG_Z(result == 0);
	SETFLAG_H(half > 0xF);

	if ((((s16)dst >= 0) && ((s16)src < 0) && ((s16)result < 0)) ||
		(((s16)dst < 0) && ((s16)src >= 0) && ((s16)result >= 0)))
	{SETFLAG_V1} else {SETFLAG_V0}

	SETFLAG_N1;
	SETFLAG_C(resultC > 0xFFFF);

	return result;
}

STATIC u32 generic_SUB_L(u32 dst, u32 src)
{
	_u64 resultC = (_u64)dst - (_u64)src;
	u32 result = (u32)(resultC & 0xFFFFFFFF);

	SETFLAG_S(result & 0x80000000);
	SETFLAG_Z(result == 0);

	if ((((s32)dst >= 0) && ((s32)src < 0) && ((s32)result < 0)) ||
		(((s32)dst < 0) && ((s32)src >= 0) && ((s32)result >= 0)))
	{SETFLAG_V1} else {SETFLAG_V0}
	
	SETFLAG_N1;
	SETFLAG_C(resultC > 0xFFFFFFFF);

	return result;
}

//=============================================================================

STATIC u8 generic_SBC_B(u8 dst, u8 src)
{
	u8 half = (dst & 0xF) - (src & 0xF) - FLAG_C;
	u32 resultC = (u32)dst - (u32)src - (u32)FLAG_C;
	u8 result = (u8)(resultC & 0xFF);

	SETFLAG_S(result & 0x80);
	SETFLAG_Z(result == 0);
	SETFLAG_H(half > 0xF);

	if ((((s8)dst >= 0) && ((s8)src < 0) && ((s8)result < 0)) ||
		(((s8)dst < 0) && ((s8)src >= 0) && ((s8)result >= 0)))
	{SETFLAG_V1} else {SETFLAG_V0}

	SETFLAG_N1;
	SETFLAG_C(resultC > 0xFF);

	return result;
}

STATIC u16 generic_SBC_W(u16 dst, u16 src)
{
	u16 half = (dst & 0xF) - (src & 0xF) - FLAG_C;
	u32 resultC = (u32)dst - (u32)src - (u32)FLAG_C;
	u16 result = (u16)(resultC & 0xFFFF);

	SETFLAG_S(result & 0x8000);
	SETFLAG_Z(result == 0);
	SETFLAG_H(half > 0xF);

	if ((((s16)dst >= 0) && ((s16)src < 0) && ((s16)result < 0)) ||
		(((s16)dst < 0) && ((s16)src >= 0) && ((s16)result >= 0)))
	{SETFLAG_V1} else {SETFLAG_V0}

	SETFLAG_N1;
	SETFLAG_C(resultC > 0xFFFF);

	return result;
}

STATIC u32 generic_SBC_L(u32 dst, u32 src)
{
	_u64 resultC = (_u64)dst - (_u64)src - (_u64)FLAG_C;
	u32 result = (u32)(resultC & 0xFFFFFFFF);

	SETFLAG_S(result & 0x80000000);
	SETFLAG_Z(result == 0);

	if ((((s32)dst >= 0) && ((s32)src < 0) && ((s32)result < 0)) ||
		(((s32)dst < 0) && ((s32)src >= 0) && ((s32)result >= 0)))
	{SETFLAG_V1} else {SETFLAG_V0}
	
	SETFLAG_N1;
	SETFLAG_C(resultC > 0xFFFFFFFF);

	return result;
}

/*****************************************************************************/
//===== PUSH (mem)
STATIC void srcPUSH()
{
    switch(size) {
      case 0: push8(loadB(mem));  break;
      case 1: push16(loadW(mem)); break;
    }
    cycles = 7;
}

//===== RLD A,(mem)
STATIC void srcRLD()
{
	u8 al = REGA & 0xF, m, mh, ml;

	m = loadB(mem);
	mh = (m & 0xF0) >> 4;
	ml = (m & 0x0F) << 4;
	
	REGA = (REGA & 0xF0) | mh;
	storeB(mem, ml | al);

	SETFLAG_S(REGA & 0x80);
	SETFLAG_Z(REGA == 0);
	SETFLAG_H0
	SETFLAG_N0
	parityB(REGA);

	cycles = 12;
}

//===== RRD A,(mem)
STATIC void srcRRD()
{
	u8 al = (REGA & 0xF) << 4, m, mh, ml;

	m = loadB(mem);
	mh = (m & 0xF0) >> 4;
	ml = m & 0x0F;
	
	REGA = (REGA & 0xF0) | ml;
	storeB(mem, al | mh);

	SETFLAG_S(REGA & 0x80);
	SETFLAG_Z(REGA == 0);
	SETFLAG_H0
	SETFLAG_N0
	parityB(REGA);

	cycles = 12;
}

//===== LDI
STATIC void srcLDI()
{
	u8 dst = 2/*XDE*/, src = 3/*XHL*/;
	if ((first & 0xF) == 5) { dst = 4/*XIX*/; src = 5/*XIY*/; }

#if 22
    if(!size) { storeB(regL(dst), loadB(regL(src))); }
    else      { storeW(regL(dst), loadW(regL(src))); }

    regL(dst) += (size+1);
    regL(src) += (size+1);
#else
    switch(size) {
      case 0:
        storeB(regL(dst), loadB(regL(src)));
        regL(dst) += 1;
        regL(src) += 1;
        break;
        
      case 1:
        storeW(regL(dst), loadW(regL(src)));
        regL(dst) += 2;
        regL(src) += 2;
        break;
    }
#endif
    
	REGBC --;
	SETFLAG_V(REGBC);

	SETFLAG_H0;
	SETFLAG_N0;
	cycles = 10;
}

//===== LDIR
STATIC void srcLDIR()
{
	u8 dst = 2/*XDE*/, src = 3/*XHL*/;
	if ((first & 0xF) == 5) { dst = 4/*XIX*/; src = 5/*XIY*/; }

	cycles = 10;

	do {
        switch(size) {
          case 0:
            if (debug_abort_memory == FALSE)
              storeB(regL(dst), loadB(regL(src)));
            regL(dst) += 1;
            regL(src) += 1;
            break;
            
		case 1:
            if (debug_abort_memory == FALSE)
              storeW(regL(dst), loadW(regL(src)));
            regL(dst) += 2;
            regL(src) += 2;
            break;
        }
        
        REGBC --;
        SETFLAG_V(REGBC);
        
        cycles += 14;
    }
	while (FLAG_V);

	SETFLAG_H0;
	SETFLAG_N0;
}

//===== LDD
STATIC void srcLDD()
{
	u8 dst = 2/*XDE*/, src = 3/*XHL*/;
	if ((first & 0xF) == 5) { dst = 4/*XIX*/; src = 5/*XIY*/; }

    switch(size) {
      case 0:
        storeB(regL(dst), loadB(regL(src)));
        regL(dst) -= 1;
        regL(src) -= 1;
        break;
        
      case 1:
        storeW(regL(dst), loadW(regL(src)));
        regL(dst) -= 2;
        regL(src) -= 2;
        break;
    }
    
    REGBC --;
    SETFLAG_V(REGBC);
    
	SETFLAG_H0;
	SETFLAG_N0;
	cycles = 10;
}

//===== LDDR
STATIC void srcLDDR()
{
	u8 dst = 2/*XDE*/, src = 3/*XHL*/;
	if ((first & 0xF) == 5)	{ dst = 4/*XIX*/; src = 5/*XIY*/; }

	cycles = 10;

    do {
        switch(size) {
          case 0:
            if (debug_abort_memory == FALSE)
              storeB(regL(dst), loadB(regL(src)));
            regL(dst) -= 1;
            regL(src) -= 1;
            break;
            
          case 1:
            if (debug_abort_memory == FALSE)
              storeW(regL(dst), loadW(regL(src)));
            regL(dst) -= 2;
            regL(src) -= 2;
            break;
        }
        
        REGBC --;
        SETFLAG_V(REGBC);

		cycles += 14;
	}
	while (FLAG_V);

	SETFLAG_H0;
	SETFLAG_N0;
}

//===== CPI
STATIC void srcCPI()
{
	u8 _R = first & 7;

    switch(size) {
      case 0:
        generic_SUB_B(REGA, loadB(regL(_R)));
        regL(_R) ++;
        break;
        
      case 1:
        generic_SUB_W(REGWA, loadW(regL(_R)));
        regL(_R) += 2;
        break;
    }
    
    REGBC --;
    SETFLAG_V(REGBC);
    
	cycles = 8;
}

//===== CPIR
STATIC void srcCPIR()
{
	u8 _R = first & 7;

	cycles = 10;

    do {
        switch(size) {
          case 0:
            if (debug_abort_memory == FALSE)
              generic_SUB_B(REGA, loadB(regL(_R)));
            regL(_R) ++;
            break;
            
          case 1:
            if (debug_abort_memory == FALSE)
              generic_SUB_W(REGWA, loadW(regL(_R)));
            regL(_R) += 2;
            break;
        }
        
        REGBC --;
        SETFLAG_V(REGBC);
        
        cycles += 14;
    }
	while (FLAG_V && (FLAG_Z == FALSE));
}

//===== CPD
STATIC void srcCPD()
{
	u8 _R = first & 7;

	switch(size) {
      case 0:
        generic_SUB_B(REGA, loadB(regL(_R)));
        regL(_R) --;
        break;
        
      case 1:
        generic_SUB_W(REGWA, loadW(regL(_R)));
        regL(_R) -= 2;
        break;
    }
    
    REGBC --;
    SETFLAG_V(REGBC);
    
    cycles = 8;
}

//===== CPDR
STATIC void srcCPDR()
{
    u8 _R = first & 7;

	cycles = 10;

	do
	{
		switch(size)
		{
		case 0:	if (debug_abort_memory == FALSE)
					generic_SUB_B(REGA, loadB(regL(_R)));
				regL(_R) -= 1; break;

		case 1: if (debug_abort_memory == FALSE)
					generic_SUB_W(REGWA, loadW(regL(_R)));
				regL(_R) -= 2; break;
		}

		REGBC --;
		SETFLAG_V(REGBC);

		cycles += 14;
	}
	while (FLAG_V && (FLAG_Z == FALSE));
}

//===== LD (nn),(mem)
STATIC void srcLD16m()
{
    switch(size) {
      case 0: storeB(fetch16(), loadB(mem)); break;
      case 1: storeW(fetch16(), loadW(mem)); break;
    }

	cycles = 8;
}

//===== LD R,(mem)
STATIC void srcLD()
{
	switch(size) {
	case 0: regB(R) = loadB(mem); cycles = 4; break;
	case 1: regW(R) = loadW(mem); cycles = 4; break;
	case 2: regL(R) = loadL(mem); cycles = 6; break;
	}
}

//===== EX (mem),R
STATIC void srcEX()
{
    switch(size) {
      case 0: {
          u8 temp = regB(R); 
          regB(R) = loadB(mem); 
          storeB(mem, temp);
          break;
      }
        
      case 1: {
          u16 temp = regW(R); 
          regW(R) = loadW(mem); 
          storeW(mem, temp);
          break;
      }
    }

	cycles = 6;
}

//===== ADD (mem),#
STATIC void srcADDi()
{
	switch(size) {
      case 0:	storeB(mem, generic_ADD_B(loadB(mem), FETCH8())); cycles = 7;break;
      case 1:	storeW(mem, generic_ADD_W(loadW(mem), fetch16())); cycles = 8;break;
	}
}

//===== ADC (mem),#
STATIC void srcADCi()
{
    switch(size){
      case 0:	storeB(mem, generic_ADC_B(loadB(mem), FETCH8())); cycles = 7;break;
      case 1:	storeW(mem, generic_ADC_W(loadW(mem), fetch16())); cycles = 8;break;
	}
}

//===== SUB (mem),#
STATIC void srcSUBi()
{
	switch(size) {
	case 0:	storeB(mem, generic_SUB_B(loadB(mem), FETCH8())); cycles = 7;break;
	case 1:	storeW(mem, generic_SUB_W(loadW(mem), fetch16())); cycles = 8;break;
	}
}

//===== SBC (mem),#
STATIC void srcSBCi()
{
	switch(size) {
	case 0:	storeB(mem, generic_SBC_B(loadB(mem), FETCH8())); cycles = 7;break;
	case 1:	storeW(mem, generic_SBC_W(loadW(mem), fetch16())); cycles = 8;break;
	}
}

//===== AND (mem),#
STATIC void srcANDi()
{
	switch(size)
	{
	case 0: {	u8 result = loadB(mem) & FETCH8();
				storeB(mem, result);
				SETFLAG_S(result & 0x80);
				SETFLAG_Z(result == 0);
				parityB(result);
				cycles = 7;
				break; }

	case 1: {	u16 result = loadW(mem) & fetch16();
				storeW(mem, result);
				SETFLAG_S(result & 0x8000);
				SETFLAG_Z(result == 0);
				parityW(result);
				cycles = 8;
				break; }
	}

	SETFLAG_H1;
	SETFLAG_N0;
	SETFLAG_C0;
}

//===== OR (mem),#
STATIC void srcORi()
{
	switch(size)
	{
	case 0: {	u8 result = loadB(mem) | FETCH8();
				storeB(mem, result);
				SETFLAG_S(result & 0x80);
				SETFLAG_Z(result == 0);
				parityB(result);
				cycles = 7;
				break; }

	case 1: {	u16 result = loadW(mem) | fetch16();
				storeW(mem, result);
				SETFLAG_S(result & 0x8000);
				SETFLAG_Z(result == 0);
				parityW(result);
				cycles = 8;
				break; }
	}

	SETFLAG_H0;
	SETFLAG_N0;
	SETFLAG_C0;
}

//===== XOR (mem),#
STATIC void srcXORi()
{
	switch(size)
	{
	case 0: {	u8 result = loadB(mem) ^ FETCH8();
				storeB(mem, result);
				SETFLAG_S(result & 0x80);
				SETFLAG_Z(result == 0);
				parityB(result);
				cycles = 7;
				break; }

	case 1: {	u16 result = loadW(mem) ^ fetch16();
				storeW(mem, result);
				SETFLAG_S(result & 0x8000);
				SETFLAG_Z(result == 0);
				parityW(result);
				cycles = 8;
				break; }
	}

	SETFLAG_H0;
	SETFLAG_N0;
	SETFLAG_C0;
}

//===== CP (mem),#
STATIC void srcCPi()
{
	switch(size) {
	case 0:	generic_SUB_B(loadB(mem), FETCH8());	break;
	case 1:	generic_SUB_W(loadW(mem), fetch16());	break;
	}

	cycles = 6;
}

//===== MUL RR,(mem)
STATIC void srcMUL()
{
    u8 target = get_RR_Target();
    if (target == 0x80){
        return;
	}

	switch(size) {
      case 0:
        rCodeW(target) = (rCodeW(target) & 0xFF) * loadB(mem);
        cycles = 18; break;
      case 1:
        rCodeL(target) = (rCodeL(target) & 0xFFFF) * loadW(mem);
        cycles = 26; break;
	}
}

//===== MULS RR,(mem)
STATIC void srcMULS()
{
	u8 target = get_RR_Target();
	if (target == 0x80) {
		return;
	}

	switch(size){
      case 0:
        rCodeW(target) = (s8)(rCodeW(target) & 0xFF) * (s8)loadB(mem);
        cycles = 18; break;
      case 1:
        rCodeL(target) = (s16)(rCodeL(target) & 0xFFFF) * (s16)loadW(mem);
        cycles = 26; break;
    }
}

//===== DIV RR,(mem)
STATIC void srcDIV()
{
	u8 target = get_RR_Target();
	if (target == 0x80){
		return;
	}

    switch(size) {
      case 0: {
          rCodeW(target) = generic_DIV_B(rCodeW(target), loadB(mem));
          cycles = 22;
          break;	}
        
      case 1: {
          rCodeL(target) = generic_DIV_W(rCodeL(target), loadW(mem));
          cycles = 30;
          break;	}
	}
}

//===== DIVS RR,(mem)
STATIC void srcDIVS()
{
	u8 target = get_RR_Target();
    if (target == 0x80) {
        return;
    }
    
    switch(size) {
      case 0:
        rCodeW(target) = generic_DIVS_B(rCodeW(target), loadB(mem));
        cycles = 24;
        break;
      case 1:
        rCodeL(target) = generic_DIVS_W(rCodeL(target), loadW(mem));
        cycles = 32;
        break;
    }
}

//===== INC #3,(mem)
STATIC void srcINC()
{
    u8 val = R;
    if (val == 0) {
        val = 8;
    }
    
    switch(size) {
      case 0: {
          u8 dst = loadB(mem);
          u32 resultC = dst + val;
          u8 half = (dst & 0xF) + val;
          u8 result = (u8)(resultC & 0xFF);
          SETFLAG_Z(result == 0);
          SETFLAG_H(half > 0xF);
          SETFLAG_S(result & 0x80);
          SETFLAG_N0;
          
          if (((s8)dst >= 0) && ((s8)result < 0)) { SETFLAG_V1 }
          else                                    { SETFLAG_V0 }
          
          storeB(mem, result);
          break; }

	case 1: {
        u16 dst = loadW(mem);
        u32 resultC = dst + val;
        u8 half = (dst & 0xF) + val;
        u16 result = (u16)(resultC & 0xFFFF);
        SETFLAG_Z(result == 0);
        SETFLAG_H(half > 0xF);
        SETFLAG_S(result & 0x8000);
        SETFLAG_N0;
        
        if (((s16)dst >= 0) && ((s16)result < 0)) { SETFLAG_V1 }
        else                                      { SETFLAG_V0 }
        
        storeW(mem, result);
        break; }
	}

	cycles = 6;
}

//===== DEC #3,(mem)
STATIC void srcDEC()
{
	u8 val = R;
	if (val==0) val=8;

	switch(size) {
      case 0: {
        u8 dst = loadB(mem);
        u32 resultC = dst - val;
        u8 half = (dst & 0xF) - val;
        u8 result = (u8)(resultC & 0xFF);
        SETFLAG_Z(result == 0);
        SETFLAG_H(half > 0xF);
        SETFLAG_S(result & 0x80);
        SETFLAG_N1;
        
        if (((s8)dst < 0) && ((s8)result >= 0)) { SETFLAG_V1 }
        else                                    { SETFLAG_V0 }
        
          storeB(mem, result);
        break;
      }

      case 1: {
          u16 dst = loadW(mem);
          u32 resultC = dst - val;
          u8 half = (dst & 0xF) - val;
          u16 result = (u16)(resultC & 0xFFFF);
          SETFLAG_Z(result == 0);
          SETFLAG_H(half > 0xF);
          SETFLAG_S(result & 0x8000);
          SETFLAG_N1;
          
          if (((s16)dst < 0) && ((s16)result >= 0)) { SETFLAG_V1 }
          else                                      { SETFLAG_V0 }

          storeW(mem, result);
          break;
      }
	}

	cycles = 6;
}

//===== RLC (mem)
STATIC void srcRLC()
{
    switch(size) {
      case 0:	{
          u8 result = loadB(mem);
          SETFLAG_C(result & 0x80);
          result <<= 1;
          if (FLAG_C) result |= 1;
          storeB(mem, result);
          SETFLAG_S(result & 0x80);
          SETFLAG_Z(result == 0);
          parityB(result);
          break; }
        
      case 1:	{
          u16 result = loadW(mem);
          SETFLAG_C(result & 0x8000);
          result <<= 1;
          if (FLAG_C) result |= 1;
          storeW(mem, result);
          SETFLAG_S(result & 0x8000);
          SETFLAG_Z(result == 0);
          parityW(result);
          break; }
    }
    
    SETFLAG_H0;
    SETFLAG_N0;
    
    cycles = 8;
}

//===== RRC (mem)
STATIC void srcRRC()
{
    switch(size){
	case 0:	{
        u8 data = loadB(mem), result;
        SETFLAG_C(data & 1);
        result = data >> 1;
        if (FLAG_C) result |= 0x80;
        storeB(mem, result);
        SETFLAG_S(result & 0x80);
        SETFLAG_Z(result == 0);
        parityB(result);
        break; }
        
      case 1:	{
          u16 data = loadW(mem), result;
          SETFLAG_C(data & 1);
          result = data >> 1;
          if (FLAG_C) result |= 0x8000;
          storeW(mem, result);
          SETFLAG_S(result & 0x8000);
          SETFLAG_Z(result == 0);
          parityW(result);
          break; }
    }
    
    SETFLAG_H0;
    SETFLAG_N0;
    
    cycles = 8;
}

//===== RL (mem)
STATIC void srcRL()
{
	BOOL tempC;

	switch(size)
	{
	case 0:	{	u8 result = loadB(mem);
				tempC = FLAG_C;
				SETFLAG_C(result & 0x80);
				result <<= 1;
				if (tempC) result |= 1;
				storeB(mem, result);
				SETFLAG_S(result & 0x80);
				SETFLAG_Z(result == 0);
				parityB(result);
				break; }
		
	case 1:	{	u16 result = loadW(mem);
				tempC = FLAG_C;
				SETFLAG_C(result & 0x8000);
				result <<= 1;
				if (tempC) result |= 1;
				storeW(mem, result);
				SETFLAG_S(result & 0x8000);
				SETFLAG_Z(result == 0);
				parityW(result);
				break; }
	}

	cycles = 8;
}

//===== RR (mem)
STATIC void srcRR()
{
	BOOL tempC;

	switch(size)
	{
	case 0:	{	u8 result = loadB(mem);
				tempC = FLAG_C;
				SETFLAG_C(result & 1);
				result >>= 1;
				if (tempC) result |= 0x80;
				storeB(mem, result);
				SETFLAG_S(result & 0x80);
				SETFLAG_Z(result == 0);
				parityB(result);
				break; }
		
	case 1:	{	u16 result = loadW(mem);
				tempC = FLAG_C;
				SETFLAG_C(result & 1);
				result >>= 1;
				if (tempC) result |= 0x8000;
				storeW(mem, result);
				SETFLAG_S(result & 0x8000);
				SETFLAG_Z(result == 0);
				parityW(result);
				break; }
	}

	cycles = 8;
}

//===== SLA (mem)
STATIC void srcSLA()
{
	switch(size)
	{
	case 0:	{	u8 result, data = loadB(mem);
				SETFLAG_C(data & 0x80);
				result = ((s8)data << 1);
				SETFLAG_S(result & 0x80);
				storeB(mem, result);
				SETFLAG_Z(result == 0);
				parityB(result);
				break;	}

	case 1:	{	u16 result, data = loadW(mem);
				SETFLAG_C(data & 0x8000);
				result = ((s16)data << 1);
				SETFLAG_S(result & 0x8000);
				storeW(mem, result);
				SETFLAG_Z(result == 0);
				parityW(result);
				break;	}
	}

	SETFLAG_H0;
	SETFLAG_N0;

	cycles = 8;
}

//===== SRA (mem)
STATIC void srcSRA()
{
	switch(size)
	{
	case 0:	{	u8 result, data = loadB(mem);
				SETFLAG_C(data & 0x1);
				result = ((s8)data >> 1);
				SETFLAG_S(result & 0x80);
				storeB(mem, result);
				SETFLAG_Z(result == 0);
				parityB(result);
				break;	}
	
	case 1:	{	u16 result, data = loadW(mem);
				SETFLAG_C(data & 0x1);
				result = ((s16)data >> 1);
				SETFLAG_S(result & 0x8000);
				storeW(mem, result);
				SETFLAG_Z(result == 0);
				parityW(result);
				break;	}
	}

	SETFLAG_H0;
	SETFLAG_N0;

	cycles = 8;
}

//===== SLL (mem)
STATIC void srcSLL()
{
	switch(size)
	{
	case 0:	{	u8 result, data = loadB(mem);
				SETFLAG_C(data & 0x80);
				result = (data << 1);
				SETFLAG_S(result & 0x80);
				storeB(mem, result);
				SETFLAG_Z(result == 0);
				parityB(result);
				break;	}
	
	case 1:	{	u16 result, data = loadW(mem);
				SETFLAG_C(data & 0x8000);
				result = (data << 1);
				SETFLAG_S(result & 0x8000);
				storeW(mem, result);
				SETFLAG_Z(result == 0);
				parityW(result);
				break;	}
	}

	SETFLAG_H0;
	SETFLAG_N0;

	cycles = 8;
}

//===== SRL (mem)
STATIC void srcSRL()
{
	switch(size)
	{
	case 0:	{	u8 result, data = loadB(mem);
				SETFLAG_C(data & 0x01);
				result = (data >> 1);
				SETFLAG_S(result & 0x80);
				storeB(mem, result);
				SETFLAG_Z(result == 0);
				parityB(result);
				break;	}

	case 1:	{	u16 result, data = loadW(mem);
				SETFLAG_C(data & 0x0001);
				result = (data >> 1);
				SETFLAG_S(result & 0x8000);
				storeW(mem, result);
				SETFLAG_Z(result == 0);
				parityW(result);
				break;	}
	}

	SETFLAG_H0;
	SETFLAG_N0;

	cycles = 8;
}

//===== ADD R,(mem)
STATIC void srcADDRm()
{
	switch(size){
      case 0: regB(R) = generic_ADD_B(regB(R), loadB(mem)); cycles = 4; break;
      case 1: regW(R) = generic_ADD_W(regW(R), loadW(mem)); cycles = 4; break;
      case 2: regL(R) = generic_ADD_L(regL(R), loadL(mem)); cycles = 6; break;
	}
}

//===== ADD (mem),R
STATIC void srcADDmR()
{
	switch(size) {
      case 0: storeB(mem, generic_ADD_B(loadB(mem), regB(R))); cycles = 6;  break;
      case 1: storeW(mem, generic_ADD_W(loadW(mem), regW(R))); cycles = 6;  break;
      case 2: storeL(mem, generic_ADD_L(loadL(mem), regL(R))); cycles = 10; break;
	}
}

//===== ADC R,(mem)
STATIC void srcADCRm()
{
	switch(size) {
	case 0: regB(R) = generic_ADC_B(regB(R), loadB(mem)); cycles = 4;break;
	case 1: regW(R) = generic_ADC_W(regW(R), loadW(mem)); cycles = 4;break;
	case 2: regL(R) = generic_ADC_L(regL(R), loadL(mem)); cycles = 6;break;
	}
}

//===== ADC (mem),R
STATIC void srcADCmR()
{
    switch(size) {
      case 0: storeB(mem, generic_ADC_B(loadB(mem), regB(R))); cycles = 6;  break;
      case 1: storeW(mem, generic_ADC_W(loadW(mem), regW(R))); cycles = 6;  break;
      case 2: storeL(mem, generic_ADC_L(loadL(mem), regL(R))); cycles = 10; break;
	}
}

//===== SUB R,(mem)
STATIC void srcSUBRm()
{
	switch(size) {
      case 0: regB(R) = generic_SUB_B(regB(R), loadB(mem)); cycles = 4; break;
      case 1: regW(R) = generic_SUB_W(regW(R), loadW(mem)); cycles = 4; break;
      case 2: regL(R) = generic_SUB_L(regL(R), loadL(mem)); cycles = 6; break;
	}
}

//===== SUB (mem),R
STATIC void srcSUBmR()
{
	switch(size) {
      case 0:	storeB(mem, generic_SUB_B(loadB(mem), regB(R))); cycles = 6;  break;
      case 1:	storeW(mem, generic_SUB_W(loadW(mem), regW(R))); cycles = 6;  break;
      case 2:	storeL(mem, generic_SUB_L(loadL(mem), regL(R))); cycles = 10; break;
	}
}

//===== SBC R,(mem)
STATIC void srcSBCRm()
{
	switch(size) {
      case 0: regB(R) = generic_SBC_B(regB(R), loadB(mem)); cycles = 4; break;
      case 1: regW(R) = generic_SBC_W(regW(R), loadW(mem)); cycles = 4; break;
      case 2: regL(R) = generic_SBC_L(regL(R), loadL(mem)); cycles = 6; break;
	}
}

//===== SBC (mem),R
STATIC void srcSBCmR()
{
    switch(size) {
      case 0:	storeB(mem, generic_SBC_B(loadB(mem), regB(R))); cycles = 6;  break;
      case 1:	storeW(mem, generic_SBC_W(loadW(mem), regW(R))); cycles = 6;  break;
      case 2:	storeL(mem, generic_SBC_L(loadL(mem), regL(R))); cycles = 10; break;
	}
}

//===== AND R,(mem)
STATIC void srcANDRm()
{
    switch(size) {
      case 0:	{
          u8 result = regB(R) & loadB(mem);
          regB(R) = result;
          SETFLAG_Z(result == 0);
          SETFLAG_S(result & 0x80);
          parityB(result);
          cycles = 4;
          break; }
        
      case 1: {
          u16 result = regW(R) & loadW(mem);
          regW(R) = result;
          SETFLAG_Z(result == 0);
          SETFLAG_S(result & 0x8000);
          parityW(result);
          cycles = 4;
          break; }
        
      case 2:	{
          u32 result = regL(R) & loadL(mem);
          regL(R) = result;
          SETFLAG_Z(result == 0);
          SETFLAG_S(result & 0x80000000);
          cycles = 6;
          break; }
    }

	SETFLAG_H1;
	SETFLAG_N0;
	SETFLAG_C0;
}

//===== AND (mem),R
STATIC void srcANDmR()
{
	switch(size) {
      case 0:	{
          u8 result = regB(R) & loadB(mem);
          storeB(mem, result);
          SETFLAG_Z(result == 0);
          SETFLAG_S(result & 0x80);
          parityB(result);
          cycles = 6;
          break; }
	
      case 1: {
        u16 result = regW(R) & loadW(mem);
        storeW(mem, result);
        SETFLAG_Z(result == 0);
        SETFLAG_S(result & 0x8000);
        parityW(result);
        cycles = 6;
        break; }

      case 2:	{
          u32 result = regL(R) & loadL(mem);
          storeL(mem, result);
          SETFLAG_Z(result == 0);
          SETFLAG_S(result & 0x80000000);
          cycles = 10;
          break; }
    }

    SETFLAG_H1;
	SETFLAG_N0;
	SETFLAG_C0;
}

//===== XOR R,(mem)
STATIC void srcXORRm()
{
    switch(size){
      case 0:	{
          u8 result = regB(R) ^ loadB(mem);
          regB(R) = result;
          SETFLAG_Z(result == 0);
          SETFLAG_S(result & 0x80);
          parityB(result);
          cycles = 4;
          break; }
        
      case 1: {
          u16 result = regW(R) ^ loadW(mem);
          regW(R) = result;
          SETFLAG_Z(result == 0);
          SETFLAG_S(result & 0x8000);
          parityW(result);
          cycles = 4;
          break; }
        
      case 2:	{
          u32 result = regL(R) ^ loadL(mem);
          regL(R) = result;
          SETFLAG_Z(result == 0);
          SETFLAG_S(result & 0x80000000);
          cycles = 6;
          break; }
    }
    
    SETFLAG_H0;
    SETFLAG_N0;
    SETFLAG_C0;
}

//===== XOR (mem),R
STATIC void srcXORmR()
{
	switch(size)
	{
	case 0:	{	u8 result = regB(R) ^ loadB(mem);
				storeB(mem, result);
				SETFLAG_Z(result == 0);
				SETFLAG_S(result & 0x80);
				parityB(result);
				cycles = 6;
				break; }
	
	case 1: {	u16 result = regW(R) ^ loadW(mem);
				storeW(mem, result);
				SETFLAG_Z(result == 0);
				SETFLAG_S(result & 0x8000);
				parityW(result);
				cycles = 6;
				break; }

	case 2:	{	u32 result = regL(R) ^ loadL(mem);
				storeL(mem, result);
				SETFLAG_Z(result == 0);
				SETFLAG_S(result & 0x80000000);
				cycles = 10;
				break; }
	}

	SETFLAG_H0;
	SETFLAG_N0;
	SETFLAG_C0;
}

//===== OR R,(mem)
STATIC void srcORRm()
{
	switch(size)
	{
	case 0:	{	u8 result = regB(R) | loadB(mem);
				regB(R) = result;
				SETFLAG_Z(result == 0);
				SETFLAG_S(result & 0x80);
				parityB(result);
				cycles = 4;
				break; }
	
	case 1: {	u16 result = regW(R) | loadW(mem);
				regW(R) = result;
				SETFLAG_Z(result == 0);
				SETFLAG_S(result & 0x8000);
				parityW(result);
				cycles = 4;
				break; }

	case 2:	{	u32 result = regL(R) | loadL(mem);
				regL(R) = result;
				SETFLAG_Z(result == 0);
				SETFLAG_S(result & 0x80000000);
				cycles = 6;
				break; }
	}

	SETFLAG_H0;
	SETFLAG_N0;
	SETFLAG_C0;
}

//===== OR (mem),R
STATIC void srcORmR()
{
	switch(size){
	case 0:	{	u8 result = regB(R) | loadB(mem);
				storeB(mem, result);
				SETFLAG_Z(result == 0);
				SETFLAG_S(result & 0x80);
				parityB(result);
				cycles = 6;
				break; }
	
	case 1: {	u16 result = regW(R) | loadW(mem);
				storeW(mem, result);
				SETFLAG_Z(result == 0);
				SETFLAG_S(result & 0x8000);
				parityW(result);
				cycles = 6;
				break; }

	case 2:	{	u32 result = regL(R) | loadL(mem);
				storeL(mem, result);
				SETFLAG_Z(result == 0);
				SETFLAG_S(result & 0x80000000);
				cycles = 10;
				break; }
	}

	SETFLAG_H0;
	SETFLAG_N0;
	SETFLAG_C0;
}

//===== CP R,(mem)
STATIC void srcCPRm()
{
	switch(size)
	{
	case 0: generic_SUB_B(regB(R), loadB(mem)); cycles = 4; break;
	case 1: generic_SUB_W(regW(R), loadW(mem)); cycles = 4; break;
	case 2: generic_SUB_L(regL(R), loadL(mem)); cycles = 6; break;
	}
}

//===== CP (mem),R
STATIC void srcCPmR()
{
	switch(size)
	{
	case 0: generic_SUB_B(loadB(mem), regB(R));		break;
	case 1: generic_SUB_W(loadW(mem), regW(R));		break;
	case 2: generic_SUB_L(loadL(mem), regL(R));		break;
	}
	
	cycles = 6;
}

//=============================================================================
//=========================================================================

//===== LD (mem),#
STATIC void dstLDBi()
{
	storeB(mem, FETCH8());
	cycles = 5;
}

//===== LD (mem),#
STATIC void dstLDWi()
{
	storeW(mem, fetch16());
	cycles = 6;
}

//===== POP (mem)
STATIC void dstPOPB()
{
	storeB(mem, pop8());
	cycles = 6;
}

//===== POP (mem)
STATIC void dstPOPW()
{
	storeW(mem, pop16());
	cycles = 6;
}

//===== LD (mem),(nn)
STATIC void dstLDBm16()
{
	storeB(mem, loadB(fetch16()));
	cycles = 8;
}

//===== LD (mem),(nn)
STATIC void dstLDWm16()
{
	storeW(mem, loadW(fetch16()));
	cycles = 8;
}

//===== LDA R,mem
STATIC void dstLDAW()
{
	regW(R) = (u16)mem;
	cycles = 4;
}

//===== LDA R,mem
STATIC void dstLDAL()
{
	regL(R) = (u32)mem;
	cycles = 4;
}

//===== ANDCF A,(mem)
STATIC void dstANDCFA()
{
	u8 bit = REGA & 0xF;
	u8 mbit = (loadB(mem) >> bit) & 1;
	if (bit < 8) SETFLAG_C(mbit & FLAG_C);
	cycles = 8;
}

//===== ORCF A,(mem)
STATIC void dstORCFA()
{
	u8 bit = REGA & 0xF;
	u8 mbit = (loadB(mem) >> bit) & 1;
	if (bit < 8) SETFLAG_C(mbit | FLAG_C);
	cycles = 8;
}

//===== XORCF A,(mem)
STATIC void dstXORCFA()
{
	u8 bit = REGA & 0xF;
	u8 mbit = (loadB(mem) >> bit) & 1;
	if (bit < 8) SETFLAG_C(mbit ^ FLAG_C);
	cycles = 8;
}

//===== LDCF A,(mem)
STATIC void dstLDCFA()
{
	u8 bit = REGA & 0xF;
	u8 mask = (1 << bit);
	if (bit < 8) SETFLAG_C(loadB(mem) & mask);
	cycles = 8;
}

//===== STCF A,(mem)
STATIC void dstSTCFA()
{
	u8 bit = REGA & 0xF;
	u8 cmask = ~(1 << bit);
	u8 set = FLAG_C << bit;
	if (bit < 8) storeB(mem, (loadB(mem) & cmask) | set); 
	cycles = 8;
}

//===== LD (mem),R
STATIC void dstLDBR()
{
	storeB(mem, regB(R));
	cycles = 4;
}

//===== LD (mem),R
STATIC void dstLDWR()
{
	storeW(mem, regW(R));
	cycles = 4;
}

//===== LD (mem),R
STATIC void dstLDLR()
{
	storeL(mem, regL(R));
	cycles = 6;
}

//===== ANDCF #3,(mem)
STATIC void dstANDCF()
{
	u8 bit = R;
	u8 mbit = (loadB(mem) >> bit) & 1;
	SETFLAG_C(mbit & FLAG_C);
	cycles = 8;
}

//===== ORCF #3,(mem)
STATIC void dstORCF()
{
	u8 bit = R;
	u8 mbit = (loadB(mem) >> bit) & 1;
	SETFLAG_C(mbit | FLAG_C);
	cycles = 8;
}

//===== XORCF #3,(mem)
STATIC void dstXORCF()
{
	u8 bit = R;
	u8 mbit = (loadB(mem) >> bit) & 1;
	SETFLAG_C(mbit ^ FLAG_C);
	cycles = 8;
}

//===== LDCF #3,(mem)
STATIC void dstLDCF()
{
	u8 bit = R;
	u32 mask = (1 << bit);
	SETFLAG_C(loadB(mem) & mask);
	cycles = 8;
}

//===== STCF #3,(mem)
STATIC void dstSTCF()
{
	u8 bit = R;
	u8 cmask = ~(1 << bit);
	u8 set = FLAG_C << bit;
	storeB(mem, (loadB(mem) & cmask) | set); 
	cycles = 8;
}

//===== TSET #3,(mem)
STATIC void dstTSET()
{
	SETFLAG_Z(! (loadB(mem) & (1 << R)) );
	storeB(mem, loadB(mem) | (1 << R));

	SETFLAG_H1
	SETFLAG_N0
	cycles = 10;
}

//===== RES #3,(mem)
STATIC void dstRES()
{
	storeB(mem, loadB(mem) & (~(1 << R)));
	cycles = 8;
}

//===== SET #3,(mem)
STATIC void dstSET()
{
	storeB(mem, loadB(mem) | (1 << R));
	cycles = 8;
}

//===== CHG #3,(mem)
STATIC void dstCHG()
{
	storeB(mem, loadB(mem) ^ (1 << R));
	cycles = 8;
}

//===== BIT #3,(mem)
STATIC void dstBIT()
{
	SETFLAG_Z(! (loadB(mem) & (1 << R)) );
	SETFLAG_H1;
	SETFLAG_N0;
	cycles = 8;
}

//===== JP cc,mem
STATIC void dstJP()
{
	if (conditionCode(second & 0xF)){
#ifdef PC32ADR
		pc32_adr = &(ng_map[(u8)((mem)>>16)])[((u16)(mem))];
#else
		pc32 = mem;
#endif
		cycles = 9;
	} else {
		cycles = 6;
	}
}

//===== CALL cc,mem
STATIC void dstCALL()
{
	if (conditionCode(second & 0xF)) {
#ifdef PC32ADR
		push32(pc32_adr);
		pc32_adr = &(ng_map[(u8)((mem)>>16)])[((u16)(mem))];
#else
		push32(pc32);
		pc32=mem;    
#endif
		cycles = 12;
	} else {
		cycles = 6;
	}
}

//===== RET cc
STATIC void dstRET()
{
	if (conditionCode(second & 0xF)) {
#ifdef PC32ADR
		pc32_adr = pop32(); 
#else
		pc32 = pop32(); 
#endif
		cycles = 12;
	} else {
		cycles = 6;
	}
}

//=============================================================================

//=========================================================================

//===== LD r,#
STATIC void regLDi()
{
	switch(size){
	case 0:	rCodeB(rCode) = FETCH8(); 	cycles = 4; break;
	case 1:	rCodeW(rCode) = fetch16();	cycles = 4;	break;
	case 2: rCodeL(rCode) = fetch32();	cycles = 6;	break;
	}
}

//===== PUSH r
STATIC void regPUSH()
{
	switch(size) {
	case 0:	push8(rCodeB(rCode));  cycles = 5;break;
	case 1:	push16(rCodeW(rCode)); cycles = 5;break;
	case 2: push32(rCodeL(rCode)); cycles = 7;break;
	}
}

//===== POP r
STATIC void regPOP()
{
    switch(size){
    case 0: rCodeB(rCode) = pop8();		cycles = 6;break;
    case 1: rCodeW(rCode) = pop16();	cycles = 6;break;
    case 2: rCodeL(rCode) = pop32(); 	cycles = 8;break;
    }
}

//===== CPL r
STATIC void regCPL()
{
    switch(size){
    case 0: rCodeB(rCode) = ~ rCodeB(rCode);	break;
    case 1: rCodeW(rCode) = ~ rCodeW(rCode);	break;
    }
    
    SETFLAG_H1;
    SETFLAG_N1;
    cycles = 4;
}

//===== NEG r
STATIC void regNEG()
{
    switch(size){
      case 0: rCodeB(rCode) = generic_SUB_B(0, rCodeB(rCode)); break;
      case 1: rCodeW(rCode) = generic_SUB_W(0, rCodeW(rCode)); break;
    }
    cycles = 5;
}

//===== MUL rr,#
STATIC void regMULi()
{
    u8 target = get_rr_Target();
    if (target == 0x80){
#ifdef NEOPOP_DEBUG
//		instruction_error("reg: MULi bad \'rr\' dst code");
#endif
		return;
	}

    switch(size){
      case 0: rCodeW(target) = (rCodeW(target) & 0x00FF) * FETCH8();   cycles = 18; break;
      case 1: rCodeL(target) = (rCodeL(target) & 0xFFFF) * fetch16();  cycles = 26; break;
    }
}

//===== MULS rr,#
STATIC void regMULSi()
{
	u8 target = get_rr_Target();
	if (target == 0x80)
	{
//		instruction_error("reg: MULSi bad \'rr\' dst code");
		return;
	}

	switch(size) {
      case 0: rCodeW(target) = (s8 )(rCodeW(target) & 0x00FF) * (s8 )FETCH8();  cycles = 18; break;
      case 1: rCodeL(target) = (s16)(rCodeL(target) & 0xFFFF) * (s16)fetch16(); cycles = 26; break;
    }
}

//===== DIV rr,#
STATIC void regDIVi()
{
	u8 target = get_rr_Target();
	if (target == 0x80){
//		instruction_error("reg: DIVi bad \'rr\' dst code");
		return;
	}

	switch(size) {
	case 0: { rCodeW(target) =  generic_DIV_B(rCodeW(target), FETCH8());  cycles = 22; break; }
	case 1: { rCodeL(target) =  generic_DIV_W(rCodeL(target), fetch16()); cycles = 30; break; }
	}
}

//===== DIVS rr,#
STATIC void regDIVSi()
{
	u8 target = get_rr_Target();
	if (target == 0x80) {
//		instruction_error("reg: DIVSi bad \'rr\' dst code");
		return;
	}

    switch(size){
    case 0: { rCodeW(target) =  generic_DIVS_B(rCodeW(target), FETCH8());  cycles = 24; break; }
    case 1: { rCodeL(target) =  generic_DIVS_W(rCodeL(target), fetch16()); cycles = 32; break; }
    }
}

//===== LINK r,dd
STATIC void regLINK()
{
    s16 d = (s16)fetch16();
    push32(rCodeL(rCode));
    rCodeL(rCode) = REGXSP;
    REGXSP += d;
    cycles = 10;
}

//===== UNLK r
STATIC void regUNLK()
{
	REGXSP = rCodeL(rCode);
	rCodeL(rCode) = pop32();
	cycles = 8;
}

//===== BS1F A,r
STATIC void regBS1F()
{
    u16 data = rCodeW(rCode), mask = 0x0001;
    u8 i;
    
    SETFLAG_V0;
    for (i = 0; i < 15; i++){
        if (data & mask){
            REGA = i;
            return;
        }
        
		mask <<= 1;
    }

	SETFLAG_V1;
	cycles = 4;
}

//===== BS1B A,r
STATIC void regBS1B()
{
	u16 data = rCodeW(rCode), mask = 0x8000;
	u8 i;
	
	SETFLAG_V0;
	for (i = 0; i < 15; i++){
		if (data & mask) {
            REGA = 15 - i;
            return;
        }
        
		mask >>= 1;
	}

	SETFLAG_V1;
	cycles = 4;
}

//===== DAA r
STATIC void regDAA()
{
	u16 resultC;
	u8 src = rCodeB(rCode), result, added, half;
	BOOL setC = FALSE;

	u8 upper4 = (src & 0xF0);
	u8 lower4 = (src & 0x0F);

	if (FLAG_C)	// {C = 1}
	{
		if (FLAG_H)	// {H = 1}
		{
			setC = TRUE;
			added = 0x66;
		}
		else		// {H = 0}
		{
			if      (lower4 < 0x0a)		{ added = 0x60; }
			else						{ added = 0x66; }
			setC = TRUE;
		}
	}
	else	// {C = 0}
	{
		if (FLAG_H)	// {H = 1}
		{
			if		(src < 0x9A)		{ added = 0x06; }
			else						{ added = 0x66; }
		}
		else		// {H = 0}
		{
			if		((upper4 < 0x90) && (lower4 > 0x9))	{ added = 0x06; }
			else if ((upper4 > 0x80) && (lower4 > 0x9))	{ added = 0x66; }
			else if ((upper4 > 0x90) && (lower4 < 0xa))	{ added = 0x60; }
		}
	}

	if (FLAG_N)
	{
		resultC = (u16)src - (u16)added;
		half = (src & 0xF) - (added & 0xF);
	}
	else
	{
		resultC = (u16)src + (u16)added;
		half = (src & 0xF) + (added & 0xF);
	}

	result = (u8)(resultC & 0xFF);	

	SETFLAG_S(result & 0x80);
	SETFLAG_Z(result == 0);
	SETFLAG_H(half > 0xF);

	if (FLAG_N)		SETFLAG_C(result > src || setC)
	else			SETFLAG_C(result < src || setC)
	
	parityB(result);
	rCodeB(rCode) = result;
	cycles = 6;
}

//===== EXTZ r
STATIC void regEXTZ()
{
	switch(size)
	{
	case 1:	rCodeW(rCode) &= 0xFF;	break;
	case 2: rCodeL(rCode) &= 0xFFFF;	break;
	}

	cycles = 4;
}

//===== EXTS r
STATIC void regEXTS()
{
	switch(size)
	{
	case 1:	if (rCodeW(rCode) & 0x0080) 
				{ rCodeW(rCode) |= 0xFF00; } else 
				{ rCodeW(rCode) &= 0x00FF; }
		break;
		
	case 2:	if (rCodeL(rCode) & 0x00008000) 
				{ rCodeL(rCode) |= 0xFFFF0000; } else 
				{ rCodeL(rCode) &= 0x0000FFFF; }
		break;
	}

	cycles = 5;
}

//===== PAA r
STATIC void regPAA()
{
	switch(size)
	{
	case 1:	if (rCodeW(rCode) & 0x1) rCodeW(rCode)++; break;
	case 2:	if (rCodeL(rCode) & 0x1) rCodeL(rCode)++; break;
	}
	cycles = 4;
}

//===== MIRR r
STATIC void regMIRR()
{
	u16 src = rCodeW(rCode), dst = 0, bit;

	//Undocumented - see p165 of CPU .PDF
	//Seems to mirror bits completely, ie. 1234 -> 4321

	for (bit = 0; bit < 16; bit++)
		if (src & (1 << bit))
			dst |= (1 << (15 - bit));

	rCodeW(rCode) = dst;
	cycles = 4;
}

//===== MULA rr
STATIC void regMULA()
{
	u32 src = (s16)loadW(regL(2/*XDE*/)) * (s16)loadW(regL(3/*XHL*/));
	u32 dst = rCodeL(rCode);
	u32 result = dst + src;

	SETFLAG_S(result & 0x80000000);
	SETFLAG_Z(result == 0);

	if ((((s32)dst >= 0) && ((s32)src >= 0) && ((s32)result < 0)) || 
		(((s32)dst < 0)  && ((s32)src < 0) && ((s32)result >= 0)))
	{SETFLAG_V1} else {SETFLAG_V0}
	
	cycles = 31;
}

//===== DJNZ r,d
STATIC void regDJNZ()
{
	s8 offset = FETCH8();

	cycles = 7;

	switch(size){
	case 0: 
		rCodeB(rCode) --;
		if (rCodeB(rCode) != 0)	{
			cycles = 11;
#ifdef PC32ADR
			pc32_adr+=offset;
#else
			pc32+=offset; 
#endif
		}
		break;

	case 1: 
		rCodeW(rCode) --;
		if (rCodeW(rCode) != 0)	{
			cycles = 11;
#ifdef PC32ADR
			pc32_adr+=offset;
#else
			pc32+=offset; 
#endif
		}
		break;
	}
}

//===== ANDCF #,r
STATIC void regANDCFi()
{
	u8 data, bit = FETCH8() & 0xF;
	switch(size){
	case 0: {	data = (rCodeB(rCode) >> bit) & 1;
				if (bit < 8) SETFLAG_C(FLAG_C & data); 
				break; }

	case 1: {	data = (rCodeW(rCode) >> bit) & 1;
				SETFLAG_C(FLAG_C & data); 
				break; }
	}
	cycles = 4;
}

//===== ORCF #,r
STATIC void regORCFi()
{
	u8 data, bit = FETCH8() & 0xF;

	switch(size) {
	case 0: {	data = (rCodeB(rCode) >> bit) & 1;
				if (bit < 8) SETFLAG_C(FLAG_C | data); 
				break; }

	case 1: {	data = (rCodeW(rCode) >> bit) & 1;
				SETFLAG_C(FLAG_C | data); 
				break; }
	}
	cycles = 4;
}

//===== XORCF #,r
STATIC void regXORCFi()
{
	u8 data, bit = FETCH8() & 0xF;
	switch(size) {
	case 0: {	data = (rCodeB(rCode) >> bit) & 1;
				if (bit < 8) SETFLAG_C(FLAG_C ^ data); 
				break; }

	case 1: {	data = (rCodeW(rCode) >> bit) & 1;
				SETFLAG_C(FLAG_C ^ data); 
				break; }
	}
	cycles = 4;
}

//===== LDCF #,r
STATIC void regLDCFi()
{
	u8 bit = FETCH8() & 0xF;
	switch(size) {
	case 0: {	u8 mask = (1 << bit);
				if (bit < 8)
					SETFLAG_C(rCodeB(rCode) & mask);
				break; }


	case 1: {	u16 mask = (1 << bit);
				SETFLAG_C(rCodeW(rCode) & mask);
				break; }
	}

	cycles = 4;
}

//===== STCF #,r
STATIC void regSTCFi()
{
	u8 bit = FETCH8() & 0xF;
	switch(size) {
	case 0: {	u8 cmask = ~(1 << bit);
				u8 set = FLAG_C << bit;
				if (bit < 8) rCodeB(rCode) = (rCodeB(rCode) & cmask) | set;
				break;	}

	case 1: {	u16 cmask = ~(1 << bit);
				u16 set = FLAG_C << bit;
				rCodeW(rCode) = (rCodeW(rCode) & cmask) | set;
				break;	}
	}

	cycles = 4;
}

//===== ANDCF A,r
STATIC void regANDCFA()
{
	u8 data, bit = REGA & 0xF;
	switch(size) {
	case 0: {	data = (rCodeB(rCode) >> bit) & 1;
				if (bit < 8) SETFLAG_C(FLAG_C & data); 
				break; }

	case 1: {	data = (rCodeW(rCode) >> bit) & 1;
				SETFLAG_C(FLAG_C & data); 
				break; }
	}
	cycles = 4;
}

//===== ORCF A,r
STATIC void regORCFA()
{
	u8 data, bit = REGA & 0xF;
	switch(size) {
	case 0: {	data = (rCodeB(rCode) >> bit) & 1;
				if (bit < 8) SETFLAG_C(FLAG_C | data); 
				break; }

	case 1: {	data = (rCodeW(rCode) >> bit) & 1;
				SETFLAG_C(FLAG_C | data); 
				break; }
	}
	cycles = 4;
}

//===== XORCF A,r
STATIC void regXORCFA()
{
	u8 data, bit = REGA & 0xF;
	switch(size) {
	case 0: {	data = (rCodeB(rCode) >> bit) & 1;
				if (bit < 8) SETFLAG_C(FLAG_C ^ data); 
				break; }

	case 1: {	data = (rCodeW(rCode) >> bit) & 1;
				SETFLAG_C(FLAG_C ^ data); 
				break; }
	}
	cycles = 4;
}

//===== LDCF A,r
STATIC void regLDCFA()
{
	u8 bit = REGA & 0xF;
	u32 mask = (1 << bit);

	switch(size) {
	case 0: if (bit < 8) SETFLAG_C(rCodeB(rCode) & mask); break;
	case 1: SETFLAG_C(rCodeW(rCode) & mask); break;
	}

	cycles = 4;
}

//===== STCF A,r
STATIC void regSTCFA()
{
	switch(size) {
	case 0: {	u8 bit = REGA & 0xF;
				u8 cmask = ~(1 << bit);
				u8 set = FLAG_C << bit;
				if (bit < 8) rCodeB(rCode) = (rCodeB(rCode) & cmask) | set;
				break;	}

	case 1: {	u8 bit = REGA & 0xF;
				u16 cmask = ~(1 << bit);
				u16 set = FLAG_C << bit;
				rCodeW(rCode) = (rCodeW(rCode) & cmask) | set;
				break;	}
	}

	cycles = 4;
}

//===== LDC cr,r
STATIC void regLDCcrr()
{
	u8 cr = FETCH8();

	switch(size) {
	case 0: dmaStoreB(cr, rCodeB(rCode)); break;
	case 1: dmaStoreW(cr, rCodeW(rCode)); break;
	case 2: dmaStoreL(cr, rCodeL(rCode)); break;
	}

	cycles = 8;
}

//===== LDC r,cr
STATIC void regLDCrcr()
{
	u8 cr = FETCH8();

	switch(size) {
	case 0: rCodeB(rCode) = dmaLoadB(cr); break;
	case 1: rCodeW(rCode) = dmaLoadW(cr); break;
	case 2: rCodeL(rCode) = dmaLoadL(cr); break;
	}

	cycles = 8;
}

//===== RES #,r
STATIC void regRES()
{
	u8 b = FETCH8() & 0xF;

	switch(size) {
	case 0: rCodeB(rCode) &= ~(u8)(1 << b); break;
	case 1: rCodeW(rCode) &= ~(u16)(1 << b); break;
	}

	cycles = 4;
}

//===== SET #,r
STATIC void regSET()
{
	u8 b = FETCH8() & 0xF;

	switch(size){
	case 0: rCodeB(rCode) |= (1 << b); break;
	case 1: rCodeW(rCode) |= (1 << b); break;
	}

	cycles = 4;
}

//===== CHG #,r
STATIC void regCHG()
{
	u8 b = FETCH8() & 0xF;

	switch(size){
	case 0: rCodeB(rCode) ^= (1 << b); break;
	case 1: rCodeW(rCode) ^= (1 << b); break;
	}

	cycles = 4;
}

//===== BIT #,r
STATIC void regBIT()
{
	u8 b = FETCH8() & 0xF;
	
	switch(size){
	case 0:	SETFLAG_Z(! (rCodeB(rCode) & (1 << b))	);	break;
	case 1:	SETFLAG_Z(! (rCodeW(rCode) & (1 << b))	);	break;
	}

	SETFLAG_H1;
	SETFLAG_N0;
	cycles = 4;
}

//===== TSET #,r
STATIC void regTSET()
{
	u8 b = FETCH8() & 0xF;
	
	switch(size) {
	case 0:	SETFLAG_Z(! (rCodeB(rCode) & (1 << b))	);	
			rCodeB(rCode) |= (1 << b);
			break;

	case 1:	SETFLAG_Z(! (rCodeW(rCode) & (1 << b))	);
			rCodeW(rCode) |= (1 << b);
			break;
	}

	SETFLAG_H1
	SETFLAG_N0
	cycles = 6;
}

//===== MINC1 #,r
STATIC void regMINC1()
{
	u16 num = fetch16() + 1;

	if (size == 1) {
		if ((rCodeW(rCode) % num) == (num - 1))
			rCodeW(rCode) -= (num - 1);
		else
			rCodeW(rCode) += 1;
	}

	cycles = 8;
}

//===== MINC2 #,r
STATIC void regMINC2()
{
	u16 num = fetch16() + 2;

	if (size == 1){
		if ((rCodeW(rCode) % num) == (num - 2))
			rCodeW(rCode) -= (num - 2);
		else
			rCodeW(rCode) += 2;
	}

	cycles = 8;
}

//===== MINC4 #,r
STATIC void regMINC4()
{
	u16 num = fetch16() + 4;

	if (size == 1)
	{
		if ((rCodeW(rCode) % num) == (num - 4))
			rCodeW(rCode) -= (num - 4);
		else
			rCodeW(rCode) += 4;
	}

	cycles = 8;
}

//===== MDEC1 #,r
STATIC void regMDEC1()
{
	u16 num = fetch16() + 1;

	if (size == 1){
		if ((rCodeW(rCode) % num) == 0)
			rCodeW(rCode) += (num - 1);
		else
			rCodeW(rCode) -= 1;
	}
	cycles = 7;
}

//===== MDEC2 #,r
STATIC void regMDEC2()
{
	u16 num = fetch16() + 2;

	if (size == 1){
		if ((rCodeW(rCode) % num) == 0)
			rCodeW(rCode) += (num - 2);
		else
			rCodeW(rCode) -= 2;
	}
	cycles = 7;
}

//===== MDEC4 #,r
STATIC void regMDEC4()
{
	u16 num = fetch16() + 4;

	if (size == 1) {
		if ((rCodeW(rCode) % num) == 0)
			rCodeW(rCode) += (num - 4);
		else
			rCodeW(rCode) -= 4;
	}

	cycles = 7;
}

//===== MUL RR,r
STATIC void regMUL()
{
	u8 target = get_RR_Target();
	if (target == 0x80) {
//		instruction_error("reg: MUL bad \'RR\' dst code");
		return;
	}

	switch(size) {
	case 0: rCodeW(target) = (rCodeW(target) & 0xFF) * rCodeB(rCode);	cycles = 18; break;
	case 1: rCodeL(target) = (rCodeL(target) & 0xFFFF) * rCodeW(rCode);	cycles = 26; break;
	}
}

//===== MULS RR,r
STATIC void regMULS()
{
	u8 target = get_RR_Target();
	if (target == 0x80) {
//		instruction_error("reg: MUL bad \'RR\' dst code");
		return;
	}

	switch(size) {
	case 0: rCodeW(target) = (s8)(rCodeW(target) & 0xFF) * (s8)rCodeB(rCode);	  cycles = 18; break;
	case 1: rCodeL(target) = (s16)(rCodeL(target) & 0xFFFF) * (s16)rCodeW(rCode); cycles = 26; break;
	}
}

//===== DIV RR,r
STATIC void regDIV()
{
	u8 target = get_RR_Target();
	if (target == 0x80){
//		instruction_error("reg: DIV bad \'RR\' dst code");
		return;
	}

	switch(size){
	case 0: { rCodeW(target) =  generic_DIV_B(rCodeW(target), rCodeB(rCode)); cycles = 22; break; }
	case 1: { rCodeL(target) =  generic_DIV_W(rCodeL(target), rCodeW(rCode)); cycles = 30; break; }
	}
}

//===== DIVS RR,r
STATIC void regDIVS()
{
	u8 target = get_RR_Target();
	if (target == 0x80){
//		instruction_error("reg: DIVS bad \'RR\' dst code");
		return;
	}

	switch(size){
	case 0: {	rCodeW(target) = generic_DIVS_B(rCodeW(target), rCodeB(rCode)); cycles = 24; break; }
	case 1: {	rCodeL(target) = generic_DIVS_W(rCodeL(target), rCodeW(rCode)); cycles = 32; break; }
	}
}

//===== INC #3,r
STATIC void regINC()
{
	u8 val = R;
	if(val==0) val = 8;

	switch(size){
	case 0: {	u8 dst = rCodeB(rCode);
				u8 half = (dst & 0xF) + val;
				u32 resultC = dst + val;
				u8 result = (u8)(resultC & 0xFF);
				SETFLAG_S(result & 0x80);

				if (((s8)dst >= 0) && ((s8)result < 0))
				{SETFLAG_V1} else {SETFLAG_V0}

				SETFLAG_H(half > 0xF);
				SETFLAG_Z(result == 0);
				SETFLAG_N0;
				rCodeB(rCode) = result;
				break;	}

	case 1: {	rCodeW(rCode) += val; break; }
	case 2: {	rCodeL(rCode) += val; break; }
	}

	cycles = 4;
}

//===== DEC #3,r
STATIC void regDEC()
{
	u8 val = R;
	if(val==0) val = 8;

	switch(size){
	case 0: {	u8 dst = rCodeB(rCode);
				u8 half = (dst & 0xF) - val;
				u32 resultC = dst - val;
				u8 result = (u8)(resultC & 0xFF);
				SETFLAG_S(result & 0x80);

				if (((s8)dst < 0) && ((s8)result >= 0))
				{SETFLAG_V1} else {SETFLAG_V0}

				SETFLAG_H(half > 0xF);
				SETFLAG_Z(result == 0);
				SETFLAG_N1;
				rCodeB(rCode) = result;
				cycles = 4;
				break;	}

	case 1: {	rCodeW(rCode) -= val; cycles = 4; break; }

	case 2: {	rCodeL(rCode) -= val; cycles = 5; break; }
	}
}

//===== SCC cc,r
STATIC void regSCC()
{
	u32 result;

	if (conditionCode(second & 0xF)) result = 1;
	else                             result = 0;

	switch(size){
	case 0: rCodeB(rCode) = (u8)result; break;
	case 1: rCodeW(rCode) = (u16)result; break;
	}

	cycles = 6;
}

//===== LD R,r
STATIC void regLDRr()
{
	switch(size){
	case 0:	regB(R) = rCodeB(rCode);	break;
	case 1:	regW(R) = rCodeW(rCode);	break;
	case 2: regL(R) = rCodeL(rCode);	break;
	}

	cycles = 4;
}

//===== LD r,R
STATIC void regLDrR()
{
	switch(size){
	case 0:	rCodeB(rCode) = regB(R);	break;
	case 1:	rCodeW(rCode) = regW(R);	break;
	case 2: rCodeL(rCode) = regL(R);	break;
	}

	cycles = 4;
}

//===== ADD R,r
STATIC void regADD()
{
	switch(size){
	case 0: regB(R) = generic_ADD_B(regB(R), rCodeB(rCode)); cycles = 4; break;
	case 1: regW(R) = generic_ADD_W(regW(R), rCodeW(rCode)); cycles = 4; break;
	case 2: regL(R) = generic_ADD_L(regL(R), rCodeL(rCode)); cycles = 7; break;
	}
}

//===== ADC R,r
STATIC void regADC()
{
	switch(size){
	case 0: regB(R) = generic_ADC_B(regB(R), rCodeB(rCode)); cycles = 4; break;
	case 1: regW(R) = generic_ADC_W(regW(R), rCodeW(rCode)); cycles = 4; break;
	case 2: regL(R) = generic_ADC_L(regL(R), rCodeL(rCode)); cycles = 7; break;
	}
}

//===== SUB R,r
STATIC void regSUB()
{
	switch(size){
	case 0: regB(R) = generic_SUB_B(regB(R), rCodeB(rCode)); cycles = 4; break;
	case 1: regW(R) = generic_SUB_W(regW(R), rCodeW(rCode)); cycles = 4; break;
	case 2: regL(R) = generic_SUB_L(regL(R), rCodeL(rCode)); cycles = 7; break;
	}
}

//===== SBC R,r
STATIC void regSBC()
{
	switch(size){
	case 0: regB(R) = generic_SBC_B(regB(R), rCodeB(rCode)); cycles = 4; break;
	case 1: regW(R) = generic_SBC_W(regW(R), rCodeW(rCode)); cycles = 4; break;
	case 2: regL(R) = generic_SBC_L(regL(R), rCodeL(rCode)); cycles = 7; break;
	}
}

//===== LD r,#3
STATIC void regLDr3()
{
	switch(size){
	case 0:	rCodeB(rCode) = R;	break;
	case 1:	rCodeW(rCode) = R;	break;
	case 2:	rCodeL(rCode) = R;	break;
	}

	cycles = 4;
}

//===== EX R,r
STATIC void regEX()
{
	switch(size){
	case 0:	{ u8  temp = regB(R); regB(R) = rCodeB(rCode); rCodeB(rCode) = temp; break;}
	case 1:	{ u16 temp = regW(R); regW(R) = rCodeW(rCode); rCodeW(rCode) = temp; break;}
	case 2:	{ u32 temp = regL(R); regL(R) = rCodeL(rCode); rCodeL(rCode) = temp; break;}
	}

	cycles = 5;
}

//===== ADD r,#
STATIC void regADDi()
{
	switch(size){
	case 0: rCodeB(rCode) = generic_ADD_B(rCodeB(rCode), FETCH8()); cycles = 4;break;
	case 1: rCodeW(rCode) = generic_ADD_W(rCodeW(rCode), fetch16()); cycles = 4;break;
	case 2: rCodeL(rCode) = generic_ADD_L(rCodeL(rCode), fetch32()); cycles = 7;break;
	}
}

//===== ADC r,#
STATIC void regADCi()
{
	switch(size){
	case 0: rCodeB(rCode) = generic_ADC_B(rCodeB(rCode), FETCH8()); cycles = 4;break;
	case 1: rCodeW(rCode) = generic_ADC_W(rCodeW(rCode), fetch16()); cycles = 4;break;
	case 2: rCodeL(rCode) = generic_ADC_L(rCodeL(rCode), fetch32()); cycles = 7;break;
	}
}

//===== SUB r,#
STATIC void regSUBi()
{
	switch(size){
	case 0: rCodeB(rCode) = generic_SUB_B(rCodeB(rCode), FETCH8()); cycles = 4;break;
	case 1: rCodeW(rCode) = generic_SUB_W(rCodeW(rCode), fetch16()); cycles = 4;break;
	case 2: rCodeL(rCode) = generic_SUB_L(rCodeL(rCode), fetch32()); cycles = 7;break;
	}
}

//===== SBC r,#
STATIC void regSBCi()
{
	switch(size){
	case 0: rCodeB(rCode) = generic_SBC_B(rCodeB(rCode), FETCH8()); cycles = 4;break;
	case 1: rCodeW(rCode) = generic_SBC_W(rCodeW(rCode), fetch16()); cycles = 4;break;
	case 2: rCodeL(rCode) = generic_SBC_L(rCodeL(rCode), fetch32()); cycles = 7;break;
	}
}

//===== CP r,#
STATIC void regCPi()
{
	switch(size){
	case 0:	generic_SUB_B(rCodeB(rCode), FETCH8());	cycles = 4;break;
	case 1:	generic_SUB_W(rCodeW(rCode), fetch16());cycles = 4;	break;
	case 2:	generic_SUB_L(rCodeL(rCode), fetch32());cycles = 7;	break;
	}
}

//===== AND r,#
STATIC void regANDi()
{
	switch(size){
	case 0:	{	u8 result = rCodeB(rCode) & FETCH8();
				rCodeB(rCode) = result;
				SETFLAG_Z(result == 0);
				SETFLAG_S(result & 0x80);
				parityB(result);
				cycles = 4;
				break; }
	
	case 1: {	u16 result = rCodeW(rCode) & fetch16();
				rCodeW(rCode) = result;
				SETFLAG_Z(result == 0);
				SETFLAG_S(result & 0x8000);
				parityW(result);
				cycles = 4;
				break; }

	case 2:	{	u32 result = rCodeL(rCode) & fetch32();
				rCodeL(rCode) = result;
				SETFLAG_Z(result == 0);
				SETFLAG_S(result & 0x80000000);
				cycles = 7;
				break; }
	}

	SETFLAG_H1;
	SETFLAG_N0;
	SETFLAG_C0;
}

//===== OR r,#
STATIC void regORi()
{
	switch(size){
	case 0: {	u8 result = rCodeB(rCode) | FETCH8();
				SETFLAG_S(result & 0x80);
				SETFLAG_Z(result == 0);
				rCodeB(rCode) = result;
				parityB(result);
				cycles = 4;
				break; }

	case 1: {	u16 result = rCodeW(rCode) | fetch16();
				SETFLAG_S(result & 0x8000);
				SETFLAG_Z(result == 0);
				rCodeW(rCode) = result;
				parityW(result);
				cycles = 4;
				break; }

	case 2: {	u32 result = rCodeL(rCode) | fetch32();
				SETFLAG_S(result & 0x80000000);
				SETFLAG_Z(result == 0);
				rCodeL(rCode) = result;
				cycles = 7;
				break; }
	}

	SETFLAG_H0;
	SETFLAG_N0;
	SETFLAG_C0;
}

//===== XOR r,#
STATIC void regXORi()
{
	switch(size){
	case 0: {	u8 result = rCodeB(rCode) ^ FETCH8();
				SETFLAG_S(result & 0x80);
				SETFLAG_Z(result == 0);
				rCodeB(rCode) = result;
				parityB(result);
				cycles = 4;
				break; }

	case 1: {	u16 result = rCodeW(rCode) ^ fetch16();
				SETFLAG_S(result & 0x8000);
				SETFLAG_Z(result == 0);
				rCodeW(rCode) = result;
				parityW(result);
				cycles = 4;
				break; }

	case 2: {	u32 result = rCodeL(rCode) ^ fetch32();
				SETFLAG_S(result & 0x80000000);
				SETFLAG_Z(result == 0);
				rCodeL(rCode) = result;
				cycles = 7;
				break; }
	}

	SETFLAG_H0;
	SETFLAG_N0;
	SETFLAG_C0;
}

//===== AND R,r
STATIC void regAND()
{
	switch(size){
	case 0: {	u8 result = regB(R) & rCodeB(rCode);
				SETFLAG_S(result & 0x80);
				SETFLAG_Z(result == 0);
				regB(R) = result;
				parityB(result);
				cycles = 4;
				break; }

	case 1: {	u16 result = regW(R) & rCodeW(rCode);
				SETFLAG_S(result & 0x8000);
				SETFLAG_Z(result == 0);
				regW(R) = result;
				parityW(result);
				cycles = 4;
				break; }

	case 2: {	u32 result = regL(R) & rCodeL(rCode);
				SETFLAG_S(result & 0x80000000);
				SETFLAG_Z(result == 0);
				regL(R) = result;
				cycles = 7;
				break; }
	}

	SETFLAG_H1;
	SETFLAG_N0;
	SETFLAG_C0;
}

//===== OR R,r
STATIC void regOR()
{
	switch(size){
	case 0: {	u8 result = regB(R) | rCodeB(rCode);
				SETFLAG_S(result & 0x80);
				SETFLAG_Z(result == 0);
				regB(R) = result;
				parityB(result);
				cycles = 4;
				break; }

	case 1: {	u16 result = regW(R) | rCodeW(rCode);
				SETFLAG_S(result & 0x8000);
				SETFLAG_Z(result == 0);
				regW(R) = result;
				parityW(result);
				cycles = 4;
				break; }

	case 2: {	u32 result = regL(R) | rCodeL(rCode);
				SETFLAG_S(result & 0x80000000);
				SETFLAG_Z(result == 0);
				regL(R) = result;
				cycles = 7;
				break; }
	}

	SETFLAG_H0;
	SETFLAG_N0;
	SETFLAG_C0;
}

//===== XOR R,r
STATIC void regXOR()
{
	switch(size){
	case 0: {	u8 result = regB(R) ^ rCodeB(rCode);
				SETFLAG_S(result & 0x80);
				SETFLAG_Z(result == 0);
				regB(R) = result;
				parityB(result);
				cycles = 4;
				break; }

	case 1: {	u16 result = regW(R) ^ rCodeW(rCode);
				SETFLAG_S(result & 0x8000);
				SETFLAG_Z(result == 0);
				regW(R) = result;
				parityW(result);
				cycles = 4;
				break; }

	case 2: {	u32 result = regL(R) ^ rCodeL(rCode);
				SETFLAG_S(result & 0x80000000);
				SETFLAG_Z(result == 0);
				regL(R) = result;
				cycles = 7;
				break; }
	}

	SETFLAG_H0;
	SETFLAG_N0;
	SETFLAG_C0;
}

//===== CP r,#3
STATIC void regCPr3()
{	
	switch(size){
	case 0:	generic_SUB_B(rCodeB(rCode), R);	break;
	case 1:	generic_SUB_W(rCodeW(rCode), R);	break;
	}

	cycles = 4;
}

//===== CP R,r
STATIC void regCP()
{
	switch(size){
	case 0:	generic_SUB_B(regB(R), rCodeB(rCode));cycles = 4;	break;
	case 1:	generic_SUB_W(regW(R), rCodeW(rCode));cycles = 4;	break;
	case 2:	generic_SUB_L(regL(R), rCodeL(rCode));cycles = 7;	break;
	}
}

//===== RLC #,r
STATIC void regRLCi()
{
	int i;
	u8 sa = FETCH8() & 0xF;
	if (sa == 0) sa = 16;

	switch(size){
	case 0:		for (i = 0; i < sa; i++) {
					SETFLAG_C(rCodeB(rCode) & 0x80);
					rCodeB(rCode) <<= 1;
					if (FLAG_C) rCodeB(rCode) |= 1;
				}
				SETFLAG_S(rCodeB(rCode) & 0x80);
				SETFLAG_Z(rCodeB(rCode) == 0);
				parityB(rCodeB(rCode));
				cycles = 6 + (2*sa);
				break;
		
	case 1:		for (i = 0; i < sa; i++) {	
					SETFLAG_C(rCodeW(rCode) & 0x8000); 
					rCodeW(rCode) <<= 1;
					if (FLAG_C) rCodeW(rCode) |= 1;
				}
				SETFLAG_S(rCodeW(rCode) & 0x8000);
				SETFLAG_Z(rCodeW(rCode) == 0);
				parityW(rCodeW(rCode));
				cycles = 6 + (2*sa);
				break;

	case 2:		for (i = 0; i < sa; i++) {
					SETFLAG_C(rCodeL(rCode) & 0x80000000);
					rCodeL(rCode) <<= 1;
					if (FLAG_C) rCodeL(rCode) |= 1;
				}
				SETFLAG_S(rCodeL(rCode) & 0x80000000);
				SETFLAG_Z(rCodeL(rCode) == 0);
				cycles = 8 + (2*sa);
				break;
	}

	SETFLAG_H0;
	SETFLAG_N0;
}

//===== RRC #,r
STATIC void regRRCi()
{
	int i;
	u8 sa = FETCH8() & 0xF;
	if (sa == 0) sa = 16;

	switch(size){
	case 0:		for (i = 0; i < sa; i++) {
					SETFLAG_C(rCodeB(rCode) & 1);
					rCodeB(rCode) >>= 1;
					if (FLAG_C) rCodeB(rCode) |= 0x80;
				}
				SETFLAG_S(rCodeB(rCode) & 0x80);
				SETFLAG_Z(rCodeB(rCode) == 0);
				parityB(rCodeB(rCode));
				cycles = 6 + (2*sa);
				break;
		
	case 1:		for (i = 0; i < sa; i++) {
					SETFLAG_C(rCodeW(rCode) & 1);
					rCodeW(rCode) >>= 1;
					if (FLAG_C) rCodeW(rCode) |= 0x8000;
				}
				SETFLAG_S(rCodeW(rCode) & 0x8000);
				SETFLAG_Z(rCodeW(rCode) == 0);
				parityW(rCodeW(rCode));
				cycles = 6 + (2*sa);
				break;

	case 2:		for (i = 0; i < sa; i++) {
					SETFLAG_C(rCodeL(rCode) & 1);
					rCodeL(rCode) >>= 1;
					if (FLAG_C) rCodeL(rCode) |= 0x80000000;
				}
				SETFLAG_S(rCodeL(rCode) & 0x80000000);
				SETFLAG_Z(rCodeL(rCode) == 0);
				cycles = 8 + (2*sa);
				break;
	}

	SETFLAG_H0;
	SETFLAG_N0;
}

//===== RL #,r
STATIC void regRLi()
{
	int i;
	BOOL tempC;
	u8 sa = FETCH8() & 0xF;
	if (sa == 0) sa = 16;

	switch(size){
	case 0:	{	u8 result;
				for (i = 0; i < sa; i++) {
					result = rCodeB(rCode);
					tempC = FLAG_C;
					SETFLAG_C(result & 0x80);
					result <<= 1;
					if (tempC) result |= 1;
					rCodeB(rCode) = result;
				}
				SETFLAG_S(result & 0x80);
				SETFLAG_Z(result == 0);
				parityB(result);
				cycles = 6 + (2*sa);
				break; }
		
	case 1:	{	u16 result;
				for (i = 0; i < sa; i++) {
					result = rCodeW(rCode);
					tempC = FLAG_C;
					SETFLAG_C(result & 0x8000);
					result <<= 1;
					if (tempC) result |= 1;
					rCodeW(rCode) = result;
				}
				SETFLAG_S(result & 0x8000);
				SETFLAG_Z(result == 0);
				parityW(result);
				cycles = 6 + (2*sa);
				break; }

	case 2:	{	u32 result;
				for (i = 0; i < sa; i++) {
					result = rCodeL(rCode);
					tempC = FLAG_C;
					SETFLAG_C(result & 0x80000000);
					result <<= 1;
					if (tempC) result |= 1;
					rCodeL(rCode) = result;
				}
				SETFLAG_S(result & 0x80000000);
				SETFLAG_Z(result == 0);
				cycles = 8 + (2*sa);
				break; }
	}

	SETFLAG_H0;
	SETFLAG_N0;
}

//===== RR #,r
STATIC void regRRi()
{
	int i;
	BOOL tempC;
	u8 sa = FETCH8() & 0xF;
	if (sa == 0) sa = 16;

	switch(size)
	{
	case 0:	{	u8 result;
				for (i = 0; i < sa; i++) {
					result = rCodeB(rCode);
					tempC = FLAG_C;
					SETFLAG_C(result & 0x01);
					result >>= 1;
					if (tempC) result |= 0x80;
					rCodeB(rCode) = result;
				}
				SETFLAG_S(result & 0x80);
				SETFLAG_Z(result == 0);
				cycles = 6 + (2*sa);
				parityB(result);
				break; }
		
	case 1:	{	u16 result;
				for (i = 0; i < sa; i++) {
					result = rCodeW(rCode);
					tempC = FLAG_C;
					SETFLAG_C(result & 0x01);
					result >>= 1;
					if (tempC) result |= 0x8000;
					rCodeW(rCode) = result;
				}
				SETFLAG_S(result & 0x8000);
				SETFLAG_Z(result == 0);
				cycles = 6 + (2*sa);
				parityW(result);
				break; }

	case 2:	{	u32 result;
				for (i = 0; i < sa; i++) {
					result = rCodeL(rCode);
					tempC = FLAG_C;
					SETFLAG_C(result & 0x01);
					result >>= 1;
					if (tempC) result |= 0x80000000;
					rCodeL(rCode) = result;
				}
				SETFLAG_S(result & 0x80000000);
				SETFLAG_Z(result == 0);
				cycles = 8 + (2*sa);
				break; }
	}

	SETFLAG_H0;
	SETFLAG_N0;
}

//===== SLA #,r
STATIC void regSLAi()
{
	s8 sa = FETCH8() & 0xF;
	if (sa == 0) sa = 16;
	sa--;

	switch(size){
	case 0:	{	s8 result, data = (s8)rCodeB(rCode);
				result = (data << sa);
				SETFLAG_C(result & 0x80);
				result <<= 1;
				SETFLAG_S(result & 0x80);
				rCodeB(rCode) = result;
				SETFLAG_Z(result == 0);
				parityB(result);
				cycles = 6 + 2 + (2*sa);
				break;	}
	
	case 1:	{	s16 result, data = (s16)rCodeW(rCode);
				result = (data << sa);
				SETFLAG_C(result & 0x8000);
				result <<= 1;
				SETFLAG_S(result & 0x8000);
				rCodeW(rCode) = result;
				SETFLAG_Z(result == 0);
				parityW(result);
				cycles = 6 + 2 + (2*sa);
				break;	}

	case 2:	{	s32 result, data = (s32)rCodeL(rCode);
				result = (data << sa);
				SETFLAG_C(result & 0x80000000);
				result <<= 1;
				SETFLAG_S(result & 0x80000000);
				rCodeL(rCode) = result;
				SETFLAG_Z(result == 0);
				cycles = 8 + 2 + (2*sa);
				break;	}
	}

	SETFLAG_H0;
	SETFLAG_N0;
}

//===== SRA #,r
STATIC void regSRAi()
{
	s8 sa = FETCH8() & 0xF;
	if (sa == 0) sa = 16;
	sa--;

	switch(size){
	case 0:	{	s8 data = (s8)rCodeB(rCode), result;
				result = (data >> sa);
				SETFLAG_C(result & 1);
				result >>= 1;
				SETFLAG_S(result & 0x80);
				rCodeB(rCode) = result;
				SETFLAG_Z(result == 0);
				parityB(result);
				cycles = 6 + 2 + (2*sa);
				break; }

	case 1:	{	s16 data = (s16)rCodeW(rCode), result;
				result = (data >> sa);
				SETFLAG_C(result & 1);
				result >>= 1;
				SETFLAG_S(result & 0x8000);
				rCodeW(rCode) = result;
				SETFLAG_Z(result == 0);
				parityW(result);
				cycles = 6 + 2 + (2*sa);
				break; }

	case 2:	{	s32 data = (s32)rCodeL(rCode), result;
				result = (data >> sa);
				SETFLAG_C(result & 1);
				result >>= 1;
				SETFLAG_S(result & 0x80000000);
				rCodeL(rCode) = result;
				SETFLAG_Z(result == 0);
				cycles = 8 + 2 + (2*sa);
				break; }
	}

	SETFLAG_H0;
	SETFLAG_N0;
}

//===== SLL #,r
STATIC void regSLLi()
{
	u8 sa = FETCH8() & 0xF;
	if (sa == 0) sa = 16;
	sa--;

	switch(size){
	case 0:	{	u8 result, data = rCodeB(rCode);
				result = (data << sa);
				SETFLAG_C(result & 0x80);
				result <<= 1;
				SETFLAG_S(result & 0x80);
				rCodeB(rCode) = result;
				SETFLAG_Z(result == 0);
				parityB(result);
				cycles = 6 + 2 + (2*sa);
				break;	}
	
	case 1:	{	u16 result, data = rCodeW(rCode);
				result = (data << sa);
				SETFLAG_C(result & 0x8000);
				result <<= 1;
				SETFLAG_S(result & 0x8000);
				rCodeW(rCode) = result;
				SETFLAG_Z(result == 0);
				parityW(result);
				cycles = 6 + 2 + (2*sa);
				break;	}

	case 2:	{	u32 result, data = rCodeL(rCode);
				result = (data << sa);
				SETFLAG_C(result & 0x80000000);
				result <<= 1;
				SETFLAG_S(result & 0x80000000);
				rCodeL(rCode) = result;
				SETFLAG_Z(result == 0);
				cycles = 8 + 2 + (2*sa);
				break;	}
	}

	SETFLAG_H0;
	SETFLAG_N0;
}

//===== SRL #,r
STATIC void regSRLi()
{
	u8 sa = FETCH8() & 0xF;
	if (sa == 0) sa = 16;
	sa--;

	switch(size){
	case 0:	{	u8 data = rCodeB(rCode), result;
				result = (data >> sa);
				SETFLAG_C(result & 1);
				result >>= 1;
				SETFLAG_S(result & 0x80);
				rCodeB(rCode) = result;
				SETFLAG_Z(result == 0);
				parityB(result);
				cycles = 6 + 2 + (2*sa);
				break; }

	case 1:	{	u16 data = rCodeW(rCode), result;
				result = (data >> sa);
				SETFLAG_C(result & 1);
				result >>= 1;
				SETFLAG_S(result & 0x8000);
				rCodeW(rCode) = result;
				SETFLAG_Z(result == 0);
				parityW(result);
				cycles = 6 + 2 + (2*sa);
				break; }

	case 2:	{	u32 data = rCodeL(rCode), result;
				result = (data >> sa);
				SETFLAG_C(result & 1);
				result >>= 1;
				SETFLAG_S(result & 0x80000000);
				rCodeL(rCode) = result;
				SETFLAG_Z(result == 0);
				cycles = 8 + 2 + (2*sa);
				break; }
	}

	SETFLAG_H0;
	SETFLAG_N0;
}

//===== RLC A,r
STATIC void regRLCA()
{
	int i;
	u8 sa = REGA & 0xF;
	if (sa == 0) sa = 16;

	switch(size){
	case 0:	for (i = 0; i < sa; i++) {
				SETFLAG_C(rCodeB(rCode) & 0x80); 
				rCodeB(rCode) <<= 1;
				if (FLAG_C) rCodeB(rCode) |= 1;	
			}
			SETFLAG_S(rCodeB(rCode) & 0x80);
			SETFLAG_Z(rCodeB(rCode) == 0);
			cycles = 6 + (2*sa);
			parityB(rCodeB(rCode));
			break;
		
	case 1:	for (i = 0; i < sa; i++) {
				SETFLAG_C(rCodeW(rCode) & 0x8000);
				rCodeW(rCode) <<= 1;
				if (FLAG_C) rCodeW(rCode) |= 1;
			}
			SETFLAG_S(rCodeW(rCode) & 0x8000);
			SETFLAG_Z(rCodeW(rCode) == 0);
			cycles = 6 + (2*sa);
			parityW(rCodeW(rCode));
			break;

	case 2:	for (i = 0; i < sa; i++) {
				SETFLAG_C(rCodeL(rCode) & 0x80000000);
				rCodeL(rCode) <<= 1;
				if (FLAG_C) rCodeL(rCode) |= 1;
			}
			SETFLAG_S(rCodeL(rCode) & 0x80000000);
			SETFLAG_Z(rCodeL(rCode) == 0);
			cycles = 8 + (2*sa);
			break;
	}

	SETFLAG_H0;
	SETFLAG_N0;
}

//===== RRC A,r
STATIC void regRRCA()
{
	int i;
	u8 sa = REGA & 0xF;
	if (sa == 0) sa = 16;

	switch(size){
	case 0:	for (i = 0; i < sa; i++) {
				SETFLAG_C(rCodeB(rCode) & 1);
				rCodeB(rCode) >>= 1;
				if (FLAG_C) rCodeB(rCode) |= 0x80;	
			}
			SETFLAG_S(rCodeB(rCode) & 0x80);
			SETFLAG_Z(rCodeB(rCode) == 0);
			parityB(rCodeB(rCode));
			cycles = 6 + (2*sa);
			break;
		
	case 1:	for (i = 0; i < sa; i++) {
				SETFLAG_C(rCodeW(rCode) & 1); 
				rCodeW(rCode) >>= 1;
				if (FLAG_C) rCodeW(rCode) |= 0x8000;
			}
			SETFLAG_S(rCodeW(rCode) & 0x8000);
			SETFLAG_Z(rCodeW(rCode) == 0);
			parityW(rCodeW(rCode));
			cycles = 6 + (2*sa);
			break;

	case 2:	for (i = 0; i < sa; i++) {
				SETFLAG_C(rCodeL(rCode) & 1);
				rCodeL(rCode) >>= 1;
				if (FLAG_C) rCodeL(rCode) |= 0x80000000;
			}
			SETFLAG_S(rCodeL(rCode) & 0x80000000);
			SETFLAG_Z(rCodeL(rCode) == 0);
			cycles = 8 + (2*sa);
			break;
	}

	SETFLAG_H0;
	SETFLAG_N0;
}

//===== RL A,r
STATIC void regRLA()
{
	int i;
	BOOL tempC;
	u8 sa = REGA & 0xF;
	if (sa == 0) sa = 16;

	switch(size){
	case 0:	{	u8 result;
				for (i = 0; i < sa; i++){
					result = rCodeB(rCode);
					tempC = FLAG_C;
					SETFLAG_C(result & 0x80);
					result <<= 1;
					if (tempC) result |= 1;
					rCodeB(rCode) = result;
				}
				SETFLAG_S(result & 0x80);
				SETFLAG_Z(result == 0);
				cycles = 6 + (2*sa);
				parityB(result);
				break; }
		
	case 1:	{	u16 result;
				for (i = 0; i < sa; i++){
					result = rCodeW(rCode);
					tempC = FLAG_C;
					SETFLAG_C(result & 0x8000);
					result <<= 1;
					if (tempC) result |= 1;
					rCodeW(rCode) = result;
				}
				SETFLAG_S(result & 0x8000);
				SETFLAG_Z(result == 0);
				cycles = 6 + (2*sa);
				parityW(result);
				break; }

	case 2:	{	u32 result;
				for (i = 0; i < sa; i++){
					result = rCodeL(rCode);
					tempC = FLAG_C;
					SETFLAG_C(result & 0x80000000);
					result <<= 1;
					if (tempC) result |= 1;
					rCodeL(rCode) = result;
				}
				SETFLAG_S(result & 0x80000000);
				SETFLAG_Z(result == 0);
				cycles = 8 + (2*sa);
				break; }
	}

	SETFLAG_H0;
	SETFLAG_N0;
}

//===== RR A,r
STATIC void regRRA()
{
	int i;
	BOOL tempC;
	u8 sa = REGA & 0xF;
	if (sa == 0) sa = 16;

    switch(size) {
    case 0:	{
		u8 result;
        for (i = 0; i < sa; i++)  {
            result = rCodeB(rCode);
            tempC = FLAG_C;
            SETFLAG_C(result & 0x01);
            result >>= 1;
            if (tempC) result |= 0x80;
            rCodeB(rCode) = result;
        }
        SETFLAG_S(result & 0x80);
        SETFLAG_Z(result == 0);
        cycles = 6 + (2*sa);
        parityB(result);
        break;
	}
        
    case 1:	{
        u16 result;
        for (i = 0; i < sa; i++) {
            result = rCodeW(rCode);
            tempC = FLAG_C;
            SETFLAG_C(result & 0x01);
            result >>= 1;
            if (tempC) result |= 0x8000;
            rCodeW(rCode) = result;
        }
        SETFLAG_S(result & 0x8000);
        SETFLAG_Z(result == 0);
        cycles = 6 + (2*sa);
		parityW(result);
        break;
    }

    case 2:	{
        u32 result;
        for (i = 0; i < sa; i++) {
            result = rCodeL(rCode);
            tempC = FLAG_C;
            SETFLAG_C(result & 0x01);
            result >>= 1;
            if (tempC) result |= 0x80000000;
            rCodeL(rCode) = result;
        }
        SETFLAG_S(result & 0x80000000);
        SETFLAG_Z(result == 0);
        cycles = 8 + (2*sa);
        break;
    }
    }
    
	SETFLAG_H0;
	SETFLAG_N0;
}

//===== SLA A,r
STATIC void regSLAA()
{
	s8 sa = REGA & 0xF;
	if (sa == 0) sa = 16;
	sa--;

	switch(size) {
      case 0:	{
          s8 result, data = (s8)rCodeB(rCode);
          result = (data << sa);
          SETFLAG_C(result & 0x80);
          result <<= 1;
          SETFLAG_S(result & 0x80);
          rCodeB(rCode) = result;
          SETFLAG_Z(result == 0);
          parityB(result);
          cycles = 6 + 2 + (2*sa);
          break;
      }
	
	case 1:	{	s16 result, data = (s16)rCodeW(rCode);
				result = (data << sa);
				SETFLAG_C(result & 0x8000);
				result <<= 1;
				SETFLAG_S(result & 0x8000);
				rCodeW(rCode) = result;
				SETFLAG_Z(result == 0);
				parityW(result);
				cycles = 6 + 2 + (2*sa);
				break;	}

	case 2:	{	s32 result, data = (s32)rCodeL(rCode);
				result = (data << sa);
				SETFLAG_C(result & 0x80000000);
				result <<= 1;
				SETFLAG_S(result & 0x80000000);
				rCodeL(rCode) = result;
				SETFLAG_Z(result == 0);
				cycles = 8 + 2 + (2*sa);
				break;	}
	}

	SETFLAG_H0;
	SETFLAG_N0;
}

//===== SRA A,r
STATIC void regSRAA()
{
	s8 sa = REGA & 0xF;
	if (sa == 0) sa = 16;
	sa--;

	switch(size)
	{
	case 0:	{	s8 data = (s8)rCodeB(rCode), result;
				result = (data >> sa);
				SETFLAG_C(result & 1);
				result >>= 1;
				SETFLAG_S(result & 0x80);
				rCodeB(rCode) = result;
				SETFLAG_Z(result == 0);
				parityB(result);
				cycles = 6 + 2 + (2*sa);
				break; }

	case 1:	{	s16 data = (s16)rCodeW(rCode), result;
				result = (data >> sa);
				SETFLAG_C(result & 1);
				result >>= 1;
				SETFLAG_S(result & 0x8000);
				rCodeW(rCode) = result;
				SETFLAG_Z(result == 0);
				parityW(result);
				cycles = 6 + 2 + (2*sa);
				break; }

	case 2:	{	s32 data = (s32)rCodeL(rCode), result;
				result = (data >> sa);
				SETFLAG_C(result & 1);
				result >>= 1;
				SETFLAG_S(result & 0x80000000);
				rCodeL(rCode) = result;
				SETFLAG_Z(result == 0);
				cycles = 8 + 2 + (2*sa);
				break; }
	}

	SETFLAG_H0;
	SETFLAG_N0;
}

//===== SLL A,r
STATIC void regSLLA()
{
	u8 sa = REGA & 0xF;
	if (sa == 0) sa = 16;
	sa--;

	switch(size){
	case 0:	{	u8 result, data = rCodeB(rCode);
				result = (data << sa);
				SETFLAG_C(result & 0x80);
				result <<= 1;
				SETFLAG_S(result & 0x80);
				rCodeB(rCode) = result;
				SETFLAG_Z(result == 0);
				parityB(result);
				cycles = 6 + 2 + (2*sa);
				break;	}
	
	case 1:	{	u16 result, data = rCodeW(rCode);
				result = (data << sa);
				SETFLAG_C(result & 0x8000);
				result <<= 1;
				SETFLAG_S(result & 0x8000);
				rCodeW(rCode) = result;
				SETFLAG_Z(result == 0);
				parityW(result);
				cycles = 6 + 2 + (2*sa);
				break;	}

	case 2:	{	u32 result, data = rCodeL(rCode);
				result = (data << sa);
				SETFLAG_C(result & 0x80000000);
				result <<= 1;
				SETFLAG_S(result & 0x80000000);
				rCodeL(rCode) = result;
				SETFLAG_Z(result == 0);
				cycles = 8 + 2 + (2*sa);
				break;	}
	}

	SETFLAG_H0;
	SETFLAG_N0;
}

//===== SRL A,r
STATIC void regSRLA()
{
	u8 sa = REGA & 0xF;
	if (sa == 0) sa = 16;
	sa--;

	switch(size){
	case 0:	{	u8 data = rCodeB(rCode), result;
				result = (data >> sa);
				SETFLAG_C(result & 1);
				result >>= 1;
				SETFLAG_S(result & 0x80);
				rCodeB(rCode) = result;
				SETFLAG_Z(result == 0);
				parityB(result);
				cycles = 6 + 2 + (2*sa);
				break; }

	case 1:	{	u16 data = rCodeW(rCode), result;
				result = (data >> sa);
				SETFLAG_C(result & 1);
				result >>= 1;
				SETFLAG_S(result & 0x8000);
				rCodeW(rCode) = result;
				SETFLAG_Z(result == 0);
				parityW(result);
				cycles = 6 + 2 + (2*sa);
				break; }

	case 2:	{	u32 data = rCodeL(rCode), result;
				result = (data >> sa);
				SETFLAG_C(result & 1);
				result >>= 1;
				SETFLAG_S(result & 0x80000000);
				rCodeL(rCode) = result;
				SETFLAG_Z(result == 0);
				cycles = 8 + 2 + (2*sa);
				break; }
	}

	SETFLAG_H0;
	SETFLAG_N0;
}

//=============================================================================
//=========================================================================

//===== NOP
STATIC void sngNOP()
{
	cycles = 2;
}

//===== NORMAL
STATIC void sngNORMAL()
{
	//Not supported
	cycles = 4;
}

//===== PUSH SR
STATIC void sngPUSHSR()
{
	push16(sr);
	cycles = 4;
}

//===== POP SR
STATIC void sngPOPSR()
{
	sr = pop16();
	changedSP();
	cycles = 6;
}

//===== MAX
STATIC void sngMAX()
{
	//Not supported
	cycles = 4;
}

//===== HALT
STATIC void sngHALT()
{
//	system_message("CPU halt requested and ignored.\nPlease send me a saved state.");
	cycles = 8;
}

//===== EI #3
STATIC void sngEI()
{
	setStatusIFF(FETCH8());
	cycles = 5;
}

//===== RETI
STATIC void sngRETI()
{
	u16 temp = pop16();
#ifdef PC32ADR
	pc32_adr = pop32();
#else
	pc32 = pop32();
#endif
	sr = temp; 
	changedSP();
	cycles = 12;
}

//===== LD (n), n
STATIC void sngLD8_8()
{
	u8 dst = FETCH8();
	u8 src = FETCH8();
	storeB(dst, src);
	cycles = 5;
}

//===== PUSH n
STATIC void sngPUSH8()
{
	u8 data = FETCH8();
	push8(data);
	cycles = 4;
}

//===== LD (n), nn
STATIC void sngLD8_16()
{
	u8 dst = FETCH8();
	u16 src = fetch16();
	storeW(dst, src);
	cycles = 6;
}

//===== PUSH nn
STATIC void sngPUSH16()
{
	push16(fetch16());
	cycles = 5;
}

//===== INCF
STATIC void sngINCF()
{
	setStatusRFP(((sr & 0x300) >> 8) + 1);
	cycles = 2;
}

//===== DECF
STATIC void sngDECF()
{
	setStatusRFP(((sr & 0x300) >> 8) - 1);
	cycles = 2;
}

//===== RET condition
STATIC void sngRET()
{
#ifdef PC32ADR
	pc32_adr = pop32();
#else
	pc32 = pop32();
#endif
	cycles = 9;
}

//===== RETD dd
STATIC void sngRETD()
{
	s16 d = (s16)fetch16();
#ifdef PC32ADR
	pc32_adr = pop32(); 
#else
	pc32 = pop32(); 
#endif
	REGXSP += d;
	cycles = 9;
}

//===== RCF
STATIC void sngRCF()
{
	SETFLAG_N0;
	SETFLAG_V0;
	SETFLAG_C0;
	cycles = 2;
}

//===== SCF
STATIC void sngSCF()
{
	SETFLAG_H0;
	SETFLAG_N0;
	SETFLAG_C1;
	cycles = 2;
}

//===== CCF
STATIC void sngCCF()
{
	SETFLAG_N0;
	SETFLAG_C(!FLAG_C);
	cycles = 2;
}

//===== ZCF
STATIC void sngZCF()
{
	SETFLAG_N0;
	SETFLAG_C(!FLAG_Z);
	cycles = 2;
}

//===== PUSH A
STATIC void sngPUSHA()
{
	push8(REGA);
	cycles = 3;
}

//===== POP A
STATIC void sngPOPA()
{
	REGA = pop8();
	cycles = 4;
}

//===== EX F,F'
STATIC void sngEX()
{
	u8 f = sr & 0xFF;
	sr = (sr & 0xFF00) | f_dash;
	f_dash = f;
	cycles = 2;
}

//===== LDF #3
STATIC void sngLDF()
{
	setStatusRFP(FETCH8());
	cycles = 2;
}

//===== PUSH F
STATIC void sngPUSHF()
{
	push8(sr & 0xFF);
	cycles = 3;
}

//===== POP F
STATIC void sngPOPF()
{
	sr = (sr & 0xFF00) | pop8();
	cycles = 4;
}

//===== JP nn
STATIC void sngJP16()
{
#ifdef PC32ADR
	u16 f = fetch16();
	pc32_adr = &(ng_map[0])[f];
#else
	pc32 = fetch16();
#endif
	cycles = 7;
}

//===== JP nnn
STATIC void sngJP24()
{
#ifdef PC32ADR
	u32 f = fetch24();
	pc32_adr = &(ng_map[(u8)(f>>16)])[((u16)f)];
#else
	pc32 = fetch24();
#endif
	cycles = 7;
}

//===== CALL #16
STATIC void sngCALL16()
{
	u32 target = fetch16();
#ifdef PC32ADR
	push32(pc32_adr);
	pc32_adr = &(ng_map[(u8)(target>>16)])[((u16)target)];
#else
	push32(pc32);
	pc32 =target;
#endif
	cycles = 12;
}

//===== CALL #24
STATIC void sngCALL24()
{
	u32 target = fetch24();
#ifdef PC32ADR
	push32(pc32_adr);
	pc32_adr = &(ng_map[(u8)(target>>16)])[((u16)target)];
#else
	push32(pc32);
	pc32=target; 
#endif
	cycles = 12;
}

//===== CALR $+3+d16
STATIC void sngCALR()
{
#ifdef PC32ADR
	u32 target = (s16)fetch16() + pc32_adr;  
	push32(pc32_adr);                        
	pc32_adr = target;
#else
	u32 target = (s16)fetch16() + pc32;  
	push32(pc32);                        
	pc32=target;                         
#endif
	cycles = 12;
}

//===== LD R, n
STATIC void sngLDB()
{
	regB(first & 7) = FETCH8();
	cycles = 2;
}

//===== PUSH RR
STATIC void sngPUSHW()
{
	push16(regW(first & 7));
	cycles = 3;
}

//===== LD RR, nn
STATIC void sngLDW()
{
	regW(first & 7) = fetch16();
	cycles = 3;
}

//===== PUSH XRR
STATIC void sngPUSHL()
{
	push32(regL(first & 7));
	cycles = 5;
}

//===== LD XRR, nnnn
STATIC void sngLDL()
{
	regL(first & 7) = fetch32();
	cycles = 5;
}

//===== POP RR
STATIC void sngPOPW()
{
	regW(first & 7) = pop16();
	cycles = 4;
}

//===== POP XRR
STATIC void sngPOPL()
{
	regL(first & 7) = pop32();
	cycles = 6;
}

//===== JR cc,pc32 + d
STATIC void sngJR()
{
	if (conditionCode(first & 0xF)) {
		cycles = 8;
#ifdef PC32ADR
		pc32_adr += (s8)FETCH8();
#else
		pc32 += (s8)FETCH8(); 
#endif
	} else {
		cycles = 4;
		FETCH8();
	}
}

//===== JR cc,pc32 + dd
STATIC void sngJRL()
{
	if (conditionCode(first & 0xF)) {
		cycles = 8;
#ifdef PC32ADR
		pc32_adr += (s16)fetch16();
#else
		pc32 += (s16)fetch16(); 
#endif
	}
	else {
		cycles = 4;
#ifdef PC32ADR
		pc32_adr += 2;
#else
		pc32+=2; //fetch16();
#endif
	}
}

//===== LDX dst,src
STATIC void sngLDX()
{
	u8 dst, src;

	FETCH8();         //00
	dst = FETCH8();	  //#8
	FETCH8();         //00
	src = FETCH8();	  //#
	FETCH8();         //00

	storeB(dst, src);
	cycles = 9;
}

//===== SWI num
STATIC void sngSWI()
{
	cycles = 16;

    switch(first & 7) {
      case 1:  //System Call
#ifdef PC32ADR
		{
			u32 f;
			push32(pc32_adr);	//push32(pc32);	
			f = loadL(0xFFFE00 + (rCodeB(0x31) << 2)); 
			pc32_adr = &(ng_map[(u8)((f)>>16)])[((u16)(f))];
		}
#else
        push32(pc32);	//push32(pc32);	
        pc32 = loadL(0xFFFE00 + (rCodeB(0x31) << 2)); 
#endif
        break;
        
      case 3:
        interrupt(0);	//SWI 3
        break;
        
      case 4:
        interrupt(1);	//SWI 4
        break;
        
      case 5:
        interrupt(2);	//SWI 5
        break;
        
      case 6:
        interrupt(3);	//SWI 6
        break;
        
      default:
        //instruction_error("SWI %d is not valid.", first & 7);
        break;
    }
}

//=============================================================================


#ifdef MSB_FIRST
#define BYTE0	3
#define BYTE1	2
#define BYTE2	1
#define BYTE3	0
#define WORD0	2
#define WORD1	0
#else
#define BYTE0	0
#define BYTE1	1
#define BYTE2	2
#define BYTE3	3
#define WORD0	0
#define WORD1	2
#endif

//=============================================================================

//=============================================================================
#if 1
u8  **ex_gprMapB, **ex_regCodeMapB;
u16 **ex_gprMapW, **ex_regCodeMapW;
u32 **ex_gprMapL, **ex_regCodeMapL;
#endif


//Bank Data
static u8* gprMapB[4][8] = {
#include "./map/mapB.h"
};

static u16* gprMapW[4][8] = {
#include "./map/mapW.h"
};

static u32* gprMapL[4][8] = {
#include "./map/mapL.h"
};

//=============================================================================


static u8* regCodeMapB[4][256] = {
#include "./map/mapCodeB0.h"
#include "./map/mapCodeB1.h"
#include "./map/mapCodeB2.h"
#include "./map/mapCodeB3.h"
};

static u16* regCodeMapW[4][128] = {
#include "./map/mapCodeW0.h"
#include "./map/mapCodeW1.h"
#include "./map/mapCodeW2.h"
#include "./map/mapCodeW3.h"
};

static u32* regCodeMapL[4][64] = {
#include "./map/mapCodeL0.h"
#include "./map/mapCodeL1.h"
#include "./map/mapCodeL2.h"
#include "./map/mapCodeL3.h"
};

//=============================================================================

u8 statusIFF(void)	
{
#if 33
	u8 iff = (sr & 0x7000) >> 12;
	static const u8 ifft[8]={0,0,2,3,4,5,6,7};
	return ifft[iff];
#else
	u8 iff = (sr & 0x7000) >> 12;
	if (iff==1)
		return 0;
    return iff;
#endif
}

void setStatusIFF(u8 iff)
{
	sr = (sr & 0x8FFF) | ((iff & 0x7) << 12);
}

//=============================================================================


static void setStatusRFP(u8 rfp)
{
	sr = (sr & 0xF8FF) | ((rfp & 0x3) << 8);
	changedSP();
}

static void changedSP(void)
{
	//Store global RFP for optimisation. 
	u8 statusRFP = ((sr & 0x300) >> 8);

	// ポインタを使うことで配列からのアドレス引きを高速化(はず)
	ex_gprMapB     = gprMapB    [statusRFP];
	ex_gprMapW     = gprMapW    [statusRFP];
	ex_gprMapL     = gprMapL    [statusRFP];
	ex_regCodeMapB = regCodeMapB[statusRFP];
	ex_regCodeMapW = regCodeMapW[statusRFP];
	ex_regCodeMapL = regCodeMapL[statusRFP];
}


#ifdef DEF_PARITY
static void build_parity(void)
{
	//static u8 parity16[0x10000];
	int i,j,count;
	
	for(i=0;i<0x10000;i++) {
		count=0;
		for(j=0;j<16;j++){
			if((i & (1<<j))) count++;
		}

		if((count&1)==0) parity16[i]=1;
		else             parity16[i]=0;
	}
}
#endif

void reset_registers(void)
{
#ifdef DEF_PARITY
    build_parity();
#endif
    
	core_memset(gprBank, 0, sizeof(gprBank));
	core_memset(gpr, 0, sizeof(gpr));

#ifdef PC32ADR
	{
		u32 f;
		if (rom.data) f = le32toh(rom_header->startPC) & 0xFFFFFF;
		else          f = 0xFFFFFE;
		pc32_adr = &(ng_map[(u8)(f>>16)])[((u16)f)];
	}
#else
	if (rom.data) pc32 = le32toh(rom_header->startPC) & 0xFFFFFF;
	else          pc32 = 0xFFFFFE;
#endif

	sr = 0xF800;		// = %11111000???????? (?) are undefined in the manual)
	changedSP();
	f_dash = 00;
	rErr = RERR_VALUE;

	REGXSP = 0x00006C00; //Confirmed from BIOS, 
						//immediately changes value from default of 0x100
}


//=============================================================================


//=============================================================================

static BOOL conditionCode(int cc)
{
	switch(cc) {
	case 0:	return 0;	//(F)
	case 1:	if (FLAG_S ^ FLAG_V) return 1; else return 0;	//(LT)
	case 2:	if (FLAG_Z | (FLAG_S ^ FLAG_V)) return 1; else return 0;	//(LE)
	case 3:	if (FLAG_C | FLAG_Z) return 1; else return 0;	//(ULE)
	case 4: if (FLAG_V) return 1; else return 0;	//(OV)
	case 5:	if (FLAG_S) return 1; else return 0;	//(MI)
	case 6:	if (FLAG_Z) return 1; else return 0;	//(Z)
	case 7:	if (FLAG_C) return 1; else return 0;	//(C)
	case 8:	return 1;	//always True
	case 9:	if (FLAG_S ^ FLAG_V) return 0; else return 1;	//(GE)
	case 10:if (FLAG_Z | (FLAG_S ^ FLAG_V)) return 0; else return 1;	//(GT)
	case 11:if (FLAG_C | FLAG_Z) return 0; else return 1;	//(UGT)
	case 12:if (FLAG_V) return 0; else return 1;	//(NOV)
	case 13:if (FLAG_S) return 0; else return 1;	//(PL)
	case 14:if (FLAG_Z) return 0; else return 1;	//(NZ)
	case 15:if (FLAG_C) return 0; else return 1;	//(NC)
	}

#ifdef NEOPOP_DEBUG
	system_debug_message("Unknown Condition Code %d", cc);
#endif
	return FALSE;
}

//=============================================================================

STATIC u8 get_rr_Target(void)
{
	u8 target;//=0x80;

	if(size==0 && first==0xC7) {
		target = rCode;
	} else {
		switch(first&7) {  //Create a regCode
		case 0:	target = (size)? 0xE0: 0x80; break;
		case 1:	target = (size)? 0xE4: 0xE0; break;
		case 2:	target = (size)? 0xE8: 0x80; break;
		case 3:	target = (size)? 0xEC: 0xE4; break;
		case 4:	target = (size)? 0xF0: 0x80; break;
		case 5:	target = (size)? 0xF4: 0xE8; break;
		case 6:	target = (size)? 0xF8: 0x80; break;
		case 7:	target = (size)? 0xFC: 0xEC; break;
		}
	}

	return target;
}

STATIC u8 get_RR_Target(void)
{
	u8 target;// = 0x80;

	switch(second&7) {  //Create a regCode
	case 0: target = (size)? 0xE0: 0x80; break;
	case 1: target = (size)? 0xE4: 0xE0; break;
	case 2: target = (size)? 0xE8: 0x80; break;
	case 3: target = (size)? 0xEC: 0xE4; break;
	case 4: target = (size)? 0xF0: 0x80; break;
	case 5: target = (size)? 0xF4: 0xE8; break;
	case 6: target = (size)? 0xF8: 0x80; break;
	case 7: target = (size)? 0xFC: 0xEC; break;
	}

	return target;
}

//=========================================================================

STATIC void ExXWA()		{ mem = regL(0); }
STATIC void ExXBC()		{ mem = regL(1); }
STATIC void ExXDE()		{ mem = regL(2); }
STATIC void ExXHL()		{ mem = regL(3); }
STATIC void ExXIX()		{ mem = regL(4); }
STATIC void ExXIY()		{ mem = regL(5); }
STATIC void ExXIZ()		{ mem = regL(6); }
STATIC void ExXSP()		{ mem = regL(7); }

STATIC void ExXWAd()	{ mem = regL(0) + (s8)FETCH8(); cycles_extra = 2; }
STATIC void ExXBCd()	{ mem = regL(1) + (s8)FETCH8(); cycles_extra = 2; }
STATIC void ExXDEd()	{ mem = regL(2) + (s8)FETCH8(); cycles_extra = 2; }
STATIC void ExXHLd()	{ mem = regL(3) + (s8)FETCH8(); cycles_extra = 2; }
STATIC void ExXIXd()	{ mem = regL(4) + (s8)FETCH8(); cycles_extra = 2; }
STATIC void ExXIYd()	{ mem = regL(5) + (s8)FETCH8(); cycles_extra = 2; }
STATIC void ExXIZd()	{ mem = regL(6) + (s8)FETCH8(); cycles_extra = 2; }
STATIC void ExXSPd()	{ mem = regL(7) + (s8)FETCH8(); cycles_extra = 2; }

STATIC void Ex8()		{ mem = FETCH8();	cycles_extra = 2; }
STATIC void Ex16()		{ mem = fetch16();	cycles_extra = 2; }
STATIC void Ex24()		{ mem = fetch24();	cycles_extra = 3; }

STATIC void ExR32()
{
	u8 data = FETCH8();

	if (data == 0x03){
		u8 rIndex, r32;
		r32 = FETCH8();		//r32
		rIndex = FETCH8();	//r8
		mem = rCodeL(r32) + (s8)rCodeB(rIndex);
		cycles_extra = 8;
		return;
	}

	if (data == 0x07){
		u8 rIndex, r32;
		r32 = FETCH8();		//r32
		rIndex = FETCH8();	//r16
		mem = rCodeL(r32) + (s16)rCodeW(rIndex);
		cycles_extra = 8;
		return;
	}

	//Undocumented mode!
	if (data == 0x13){
#ifdef PC32ADR
		mem = pc32_adr + (s16)fetch16();
#else
		mem = pc32 + (s16)fetch16();
#endif
		cycles_extra = 8;	//Unconfirmed... doesn't make much difference
		return;
	}

	cycles_extra = 5;

	if ((data & 3) == 1)
		mem = rCodeL(data) + (s16)fetch16();
	else
		mem = rCodeL(data);
}

STATIC void ExDec()
{
	u8 data = FETCH8();
	u8 r32 = data & 0xFC;

	cycles_extra = 3;

	switch(data & 3){
	case 0:	rCodeL(r32) -= 1;	mem = rCodeL(r32);	break;
	case 1:	rCodeL(r32) -= 2;	mem = rCodeL(r32);	break;
	case 2:	rCodeL(r32) -= 4;	mem = rCodeL(r32);	break;
	}
}

STATIC void ExInc()
{
	u8 data = FETCH8();
	u8 r32 = data & 0xFC;

	cycles_extra = 3;

	switch(data & 3){
	case 0:	mem = rCodeL(r32);	rCodeL(r32) += 1;		break;
	case 1:	mem = rCodeL(r32);	rCodeL(r32) += 2;		break;
	case 2:	mem = rCodeL(r32);	rCodeL(r32) += 4;		break;
	}
}

STATIC void ExRC()
{
	brCode = TRUE;
	rCode = FETCH8();
	cycles_extra = 1;
}

//=========================================================================

//Address Mode & Register Code
STATIC void (*decodeExtra[256])() = 
{
/*0*/	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
/*1*/	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
/*2*/	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
/*3*/	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
/*4*/	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
/*5*/	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
/*6*/	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
/*7*/	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
/*8*/	ExXWA,	ExXBC,	ExXDE,	ExXHL,	ExXIX,	ExXIY,	ExXIZ,	ExXSP,
		ExXWAd,	ExXBCd,	ExXDEd,	ExXHLd,	ExXIXd,	ExXIYd,	ExXIZd,	ExXSPd,
/*9*/	ExXWA,	ExXBC,	ExXDE,	ExXHL,	ExXIX,	ExXIY,	ExXIZ,	ExXSP,
		ExXWAd,	ExXBCd,	ExXDEd,	ExXHLd,	ExXIXd,	ExXIYd,	ExXIZd,	ExXSPd,
/*A*/	ExXWA,	ExXBC,	ExXDE,	ExXHL,	ExXIX,	ExXIY,	ExXIZ,	ExXSP,
		ExXWAd,	ExXBCd,	ExXDEd,	ExXHLd,	ExXIXd,	ExXIYd,	ExXIZd,	ExXSPd,
/*B*/	ExXWA,	ExXBC,	ExXDE,	ExXHL,	ExXIX,	ExXIY,	ExXIZ,	ExXSP,
		ExXWAd,	ExXBCd,	ExXDEd,	ExXHLd,	ExXIXd,	ExXIYd,	ExXIZd,	ExXSPd,
/*C*/	Ex8,	Ex16,	Ex24,	ExR32,	ExDec,	ExInc,	0,		ExRC,
		0,		0,		0,		0,		0,		0,		0,		0,
/*D*/	Ex8,	Ex16,	Ex24,	ExR32,	ExDec,	ExInc,	0,		ExRC,
		0,		0,		0,		0,		0,		0,		0,		0,
/*E*/	Ex8,	Ex16,	Ex24,	ExR32,	ExDec,	ExInc,	0,		ExRC,
		0,		0,		0,		0,		0,		0,		0,		0,
/*F*/	Ex8,	Ex16,	Ex24,	ExR32,	ExDec,	ExInc,	0,		0,
		0,		0,		0,		0,		0,		0,		0,		0
};

//=========================================================================

STATIC void e(void)
{
//    instruction_error("Unknown instruction %02X", first);
}

STATIC void es(void)
{
//	instruction_error("Unknown [src] instruction %02X", second);
}

STATIC void ed(void)
{
//	instruction_error("Unknown [dst] instruction %02X", second);
}

STATIC void er(void)
{
//	instruction_error("Unknown [reg] instruction %02X", second);
}

//=========================================================================

//Secondary (SRC) Instruction decode
STATIC void (*srcDecode[256])() = 
{
/*0*/	es,			es,			es,			es,			srcPUSH,	es,			srcRLD,		srcRRD,
		es,			es,			es,			es,			es,			es,			es,			es,
/*1*/	srcLDI,		srcLDIR,	srcLDD,		srcLDDR,	srcCPI,		srcCPIR,	srcCPD,		srcCPDR,
		es,			srcLD16m,	es,			es,			es,			es,			es,			es,
/*2*/	srcLD,		srcLD,		srcLD,		srcLD,		srcLD,		srcLD,		srcLD,		srcLD,
		es,			es,			es,			es,			es,			es,			es,			es,
/*3*/	srcEX,		srcEX,		srcEX,		srcEX,		srcEX,		srcEX,		srcEX,		srcEX,
		srcADDi,	srcADCi,	srcSUBi,	srcSBCi,	srcANDi,	srcXORi,	srcORi,		srcCPi,
/*4*/	srcMUL,		srcMUL,		srcMUL,		srcMUL,		srcMUL,		srcMUL,		srcMUL,		srcMUL,
		srcMULS,	srcMULS,	srcMULS,	srcMULS,	srcMULS,	srcMULS,	srcMULS,	srcMULS,
/*5*/	srcDIV,		srcDIV,		srcDIV,		srcDIV,		srcDIV,		srcDIV,		srcDIV,		srcDIV,
		srcDIVS,	srcDIVS,	srcDIVS,	srcDIVS,	srcDIVS,	srcDIVS,	srcDIVS,	srcDIVS,
/*6*/	srcINC,		srcINC,		srcINC,		srcINC,		srcINC,		srcINC,		srcINC,		srcINC,
		srcDEC,		srcDEC,		srcDEC,		srcDEC,		srcDEC,		srcDEC,		srcDEC,		srcDEC,
/*7*/	es,			es,			es,			es,			es,			es,			es,			es,
		srcRLC,		srcRRC,		srcRL,		srcRR,		srcSLA,		srcSRA,		srcSLL,		srcSRL,
/*8*/	srcADDRm,	srcADDRm,	srcADDRm,	srcADDRm,	srcADDRm,	srcADDRm,	srcADDRm,	srcADDRm,
		srcADDmR,	srcADDmR,	srcADDmR,	srcADDmR,	srcADDmR,	srcADDmR,	srcADDmR,	srcADDmR,
/*9*/	srcADCRm,	srcADCRm,	srcADCRm,	srcADCRm,	srcADCRm,	srcADCRm,	srcADCRm,	srcADCRm,
		srcADCmR,	srcADCmR,	srcADCmR,	srcADCmR,	srcADCmR,	srcADCmR,	srcADCmR,	srcADCmR,
/*A*/	srcSUBRm,	srcSUBRm,	srcSUBRm,	srcSUBRm,	srcSUBRm,	srcSUBRm,	srcSUBRm,	srcSUBRm,
		srcSUBmR,	srcSUBmR,	srcSUBmR,	srcSUBmR,	srcSUBmR,	srcSUBmR,	srcSUBmR,	srcSUBmR,
/*B*/	srcSBCRm,	srcSBCRm,	srcSBCRm,	srcSBCRm,	srcSBCRm,	srcSBCRm,	srcSBCRm,	srcSBCRm,
		srcSBCmR,	srcSBCmR,	srcSBCmR,	srcSBCmR,	srcSBCmR,	srcSBCmR,	srcSBCmR,	srcSBCmR,
/*C*/	srcANDRm,	srcANDRm,	srcANDRm,	srcANDRm,	srcANDRm,	srcANDRm,	srcANDRm,	srcANDRm,
		srcANDmR,	srcANDmR,	srcANDmR,	srcANDmR,	srcANDmR,	srcANDmR,	srcANDmR,	srcANDmR,
/*D*/	srcXORRm,	srcXORRm,	srcXORRm,	srcXORRm,	srcXORRm,	srcXORRm,	srcXORRm,	srcXORRm,
		srcXORmR,	srcXORmR,	srcXORmR,	srcXORmR,	srcXORmR,	srcXORmR,	srcXORmR,	srcXORmR,
/*E*/	srcORRm,	srcORRm,	srcORRm,	srcORRm,	srcORRm,	srcORRm,	srcORRm,	srcORRm,
		srcORmR,	srcORmR,	srcORmR,	srcORmR,	srcORmR,	srcORmR,	srcORmR,	srcORmR,
/*F*/	srcCPRm,	srcCPRm,	srcCPRm,	srcCPRm,	srcCPRm,	srcCPRm,	srcCPRm,	srcCPRm,
		srcCPmR,	srcCPmR,	srcCPmR,	srcCPmR,	srcCPmR,	srcCPmR,	srcCPmR,	srcCPmR
};

//Secondary (DST) Instruction decode
STATIC void (*dstDecode[256])() = 
{
/*0*/	dstLDBi,	ed,			dstLDWi,	ed,			dstPOPB,	ed,			dstPOPW,	ed,
		ed,			ed,			ed,			ed,			ed,			ed,			ed,			ed,
/*1*/	ed,			ed,			ed,			ed,			dstLDBm16,	ed,			dstLDWm16,	ed,
		ed,			ed,			ed,			ed,			ed,			ed,			ed,			ed,
/*2*/	dstLDAW,	dstLDAW,	dstLDAW,	dstLDAW,	dstLDAW,	dstLDAW,	dstLDAW,	dstLDAW,
		dstANDCFA,	dstORCFA,	dstXORCFA,	dstLDCFA,	dstSTCFA,	ed,			ed,			ed,
/*3*/	dstLDAL,	dstLDAL,	dstLDAL,	dstLDAL,	dstLDAL,	dstLDAL,	dstLDAL,	dstLDAL,
		ed,			ed,			ed,			ed,			ed,			ed,			ed,			ed,
/*4*/	dstLDBR,	dstLDBR,	dstLDBR,	dstLDBR,	dstLDBR,	dstLDBR,	dstLDBR,	dstLDBR,
		ed,			ed,			ed,			ed,			ed,			ed,			ed,			ed,
/*5*/	dstLDWR,	dstLDWR,	dstLDWR,	dstLDWR,	dstLDWR,	dstLDWR,	dstLDWR,	dstLDWR,
		ed,			ed,			ed,			ed,			ed,			ed,			ed,			ed,
/*6*/	dstLDLR,	dstLDLR,	dstLDLR,	dstLDLR,	dstLDLR,	dstLDLR,	dstLDLR,	dstLDLR,
		ed,			ed,			ed,			ed,			ed,			ed,			ed,			ed,
/*7*/	ed,			ed,			ed,			ed,			ed,			ed,			ed,			ed,
		ed,			ed,			ed,			ed,			ed,			ed,			ed,			ed,
/*8*/	dstANDCF,	dstANDCF,	dstANDCF,	dstANDCF,	dstANDCF,	dstANDCF,	dstANDCF,	dstANDCF,
		dstORCF,	dstORCF,	dstORCF,	dstORCF,	dstORCF,	dstORCF,	dstORCF,	dstORCF,
/*9*/	dstXORCF,	dstXORCF,	dstXORCF,	dstXORCF,	dstXORCF,	dstXORCF,	dstXORCF,	dstXORCF,
		dstLDCF,	dstLDCF,	dstLDCF,	dstLDCF,	dstLDCF,	dstLDCF,	dstLDCF,	dstLDCF,
/*A*/	dstSTCF,	dstSTCF,	dstSTCF,	dstSTCF,	dstSTCF,	dstSTCF,	dstSTCF,	dstSTCF,	
		dstTSET,	dstTSET,	dstTSET,	dstTSET,	dstTSET,	dstTSET,	dstTSET,	dstTSET,
/*B*/	dstRES,		dstRES,		dstRES,		dstRES,		dstRES,		dstRES,		dstRES,		dstRES,
		dstSET,		dstSET,		dstSET,		dstSET,		dstSET,		dstSET,		dstSET,		dstSET,
/*C*/	dstCHG,		dstCHG,		dstCHG,		dstCHG,		dstCHG,		dstCHG,		dstCHG,		dstCHG,
		dstBIT,		dstBIT,		dstBIT,		dstBIT,		dstBIT,		dstBIT,		dstBIT,		dstBIT,
/*D*/	dstJP,		dstJP,		dstJP,		dstJP,		dstJP,		dstJP,		dstJP,		dstJP,
		dstJP,		dstJP,		dstJP,		dstJP,		dstJP,		dstJP,		dstJP,		dstJP,
/*E*/	dstCALL,	dstCALL,	dstCALL,	dstCALL,	dstCALL,	dstCALL,	dstCALL,	dstCALL,
		dstCALL,	dstCALL,	dstCALL,	dstCALL,	dstCALL,	dstCALL,	dstCALL,	dstCALL,
/*F*/	dstRET,		dstRET,		dstRET,		dstRET,		dstRET,		dstRET,		dstRET,		dstRET,
		dstRET,		dstRET,		dstRET,		dstRET,		dstRET,		dstRET,		dstRET,		dstRET
};

//Secondary (REG) Instruction decode
STATIC void (*regDecode[256])() = 
{
/*0*/	er,			er,			er,			regLDi,		regPUSH,	regPOP,		regCPL,		regNEG,
		regMULi,	regMULSi,	regDIVi,	regDIVSi,	regLINK,	regUNLK,	regBS1F,	regBS1B,
/*1*/	regDAA,		er,			regEXTZ,	regEXTS,	regPAA,		er,			regMIRR,	er,
		er,			regMULA,	er,			er,			regDJNZ,	er,			er,			er,
/*2*/	regANDCFi,	regORCFi,	regXORCFi,	regLDCFi,	regSTCFi,	er,			er,			er,
		regANDCFA,	regORCFA,	regXORCFA,	regLDCFA,	regSTCFA,	er,			regLDCcrr,	regLDCrcr,
/*3*/	regRES,		regSET,		regCHG,		regBIT,		regTSET,	er,			er,			er,
		regMINC1,	regMINC2,	regMINC4,	er,			regMDEC1,	regMDEC2,	regMDEC4,	er,
/*4*/	regMUL,		regMUL,		regMUL,		regMUL,		regMUL,		regMUL,		regMUL,		regMUL,
		regMULS,	regMULS,	regMULS,	regMULS,	regMULS,	regMULS,	regMULS,	regMULS,
/*5*/	regDIV,		regDIV,		regDIV,		regDIV,		regDIV,		regDIV,		regDIV,		regDIV,
		regDIVS,	regDIVS,	regDIVS,	regDIVS,	regDIVS,	regDIVS,	regDIVS,	regDIVS,
/*6*/	regINC,		regINC,		regINC,		regINC,		regINC,		regINC,		regINC,		regINC,
		regDEC,		regDEC,		regDEC,		regDEC,		regDEC,		regDEC,		regDEC,		regDEC,
/*7*/	regSCC,		regSCC,		regSCC,		regSCC,		regSCC,		regSCC,		regSCC,		regSCC,
		regSCC,		regSCC,		regSCC,		regSCC,		regSCC,		regSCC,		regSCC,		regSCC,
/*8*/	regADD,		regADD,		regADD,		regADD,		regADD,		regADD,		regADD,		regADD,
		regLDRr,	regLDRr,	regLDRr,	regLDRr,	regLDRr,	regLDRr,	regLDRr,	regLDRr,
/*9*/	regADC,		regADC,		regADC,		regADC,		regADC,		regADC,		regADC,		regADC,
		regLDrR,	regLDrR,	regLDrR,	regLDrR,	regLDrR,	regLDrR,	regLDrR,	regLDrR,
/*A*/	regSUB,		regSUB,		regSUB,		regSUB,		regSUB,		regSUB,		regSUB,		regSUB,
		regLDr3,	regLDr3,	regLDr3,	regLDr3,	regLDr3,	regLDr3,	regLDr3,	regLDr3,
/*B*/	regSBC,		regSBC,		regSBC,		regSBC,		regSBC,		regSBC,		regSBC,		regSBC,
		regEX,		regEX,		regEX,		regEX,		regEX,		regEX,		regEX,		regEX,
/*C*/	regAND,		regAND,		regAND,		regAND,		regAND,		regAND,		regAND,		regAND,
		regADDi,	regADCi,	regSUBi,	regSBCi,	regANDi,	regXORi,	regORi,		regCPi,
/*D*/	regXOR,		regXOR,		regXOR,		regXOR,		regXOR,		regXOR,		regXOR,		regXOR,
		regCPr3,	regCPr3,	regCPr3,	regCPr3,	regCPr3,	regCPr3,	regCPr3,	regCPr3,
/*E*/	regOR,		regOR,		regOR,		regOR,		regOR,		regOR,		regOR,		regOR,
		regRLCi,	regRRCi,	regRLi,		regRRi,		regSLAi,	regSRAi,	regSLLi,	regSRLi,
/*F*/	regCP,		regCP,		regCP,		regCP,		regCP,		regCP,		regCP,		regCP,
		regRLCA,	regRRCA,	regRLA,		regRRA,		regSLAA,	regSRAA,	regSLLA,	regSRLA
};

//=========================================================================

STATIC void src_B()
{
	second = FETCH8();			//Get the second opcode
	R = second & 7;
	size = 0;					//Byte Size

	(*srcDecode[second])();		//Call
}

STATIC void src_W()
{
	second = FETCH8();			//Get the second opcode
	R = second & 7;
	size = 1;					//Word Size

	(*srcDecode[second])();		//Call
}

STATIC void src_L()
{
    second = FETCH8();			//Get the second opcode
    R = second & 7;
    size = 2;					//Long Size

    (*srcDecode[second])();		//Call
}

STATIC void dst()
{
    second = FETCH8();			//Get the second opcode
    R = second & 7;

    (*dstDecode[second])();		//Call
}

STATIC void reg_B()
{
    second = FETCH8();			//Get the second opcode
    R = second & 7;
    size = 0;					//Byte Size

    if (brCode == FALSE) {
        brCode = TRUE;
        rCode = rCodeConversionB[first & 7];
    }

    (*regDecode[second])();		//Call
}

STATIC void reg_W()
{
    second = FETCH8();			//Get the second opcode
    R = second & 7;
    size = 1;					//Word Size

    if (brCode==FALSE){
        brCode = TRUE;
        rCode = rCodeConversionW[first & 7];
    }

    (*regDecode[second])();		//Call
}

STATIC void reg_L()
{
	second = FETCH8();			//Get the second opcode
	R = second & 7;
	size = 2;					//Long Size

	if (brCode == FALSE){
		brCode = TRUE;
		rCode = rCodeConversionL[first & 7];
	}

	(*regDecode[second])();		//Call
}

//=============================================================================

//Primary Instruction decode
STATIC void (*decode[256])() = 
{
/*0*/	sngNOP,		sngNORMAL,	sngPUSHSR,	sngPOPSR,	sngMAX,		sngHALT,	sngEI,		sngRETI,
		sngLD8_8,	sngPUSH8,	sngLD8_16,	sngPUSH16,	sngINCF,	sngDECF,	sngRET,		sngRETD,
/*1*/	sngRCF,		sngSCF,		sngCCF,		sngZCF,		sngPUSHA,	sngPOPA,	sngEX,		sngLDF,
		sngPUSHF,	sngPOPF,	sngJP16,	sngJP24,	sngCALL16,	sngCALL24,	sngCALR,	iBIOSHLE,
/*2*/	sngLDB,		sngLDB,		sngLDB,		sngLDB,		sngLDB,		sngLDB,		sngLDB,		sngLDB,
		sngPUSHW,	sngPUSHW,	sngPUSHW,	sngPUSHW,	sngPUSHW,	sngPUSHW,	sngPUSHW,	sngPUSHW,
/*3*/	sngLDW,		sngLDW,		sngLDW,		sngLDW,		sngLDW,		sngLDW,		sngLDW,		sngLDW,
		sngPUSHL,	sngPUSHL,	sngPUSHL,	sngPUSHL,	sngPUSHL,	sngPUSHL,	sngPUSHL,	sngPUSHL,
/*4*/	sngLDL,		sngLDL,		sngLDL,		sngLDL,		sngLDL,		sngLDL,		sngLDL,		sngLDL,
		sngPOPW,	sngPOPW,	sngPOPW,	sngPOPW,	sngPOPW,	sngPOPW,	sngPOPW,	sngPOPW,
/*5*/	e,			e,			e,			e,			e,			e,			e,			e,
		sngPOPL,	sngPOPL,	sngPOPL,	sngPOPL,	sngPOPL,	sngPOPL,	sngPOPL,	sngPOPL,
/*6*/	sngJR,		sngJR,		sngJR,		sngJR,		sngJR,		sngJR,		sngJR,		sngJR,
		sngJR,		sngJR,		sngJR,		sngJR,		sngJR,		sngJR,		sngJR,		sngJR,
/*7*/	sngJRL,		sngJRL,		sngJRL,		sngJRL,		sngJRL,		sngJRL,		sngJRL,		sngJRL,
		sngJRL,		sngJRL,		sngJRL,		sngJRL,		sngJRL,		sngJRL,		sngJRL,		sngJRL,
/*8*/	src_B,		src_B,		src_B,		src_B,		src_B,		src_B,		src_B,		src_B,
		src_B,		src_B,		src_B,		src_B,		src_B,		src_B,		src_B,		src_B,
/*9*/	src_W,		src_W,		src_W,		src_W,		src_W,		src_W,		src_W,		src_W,
		src_W,		src_W,		src_W,		src_W,		src_W,		src_W,		src_W,		src_W,
/*A*/	src_L,		src_L,		src_L,		src_L,		src_L,		src_L,		src_L,		src_L,
		src_L,		src_L,		src_L,		src_L,		src_L,		src_L,		src_L,		src_L,
/*B*/	dst,		dst,		dst,		dst,		dst,		dst,		dst,		dst,
		dst,		dst,		dst,		dst,		dst,		dst,		dst,		dst,
/*C*/	src_B,		src_B,		src_B,		src_B,		src_B,		src_B,		e,			reg_B,
		reg_B,		reg_B,		reg_B,		reg_B,		reg_B,		reg_B,		reg_B,		reg_B,
/*D*/	src_W,		src_W,		src_W,		src_W,		src_W,		src_W,		e,			reg_W,
		reg_W,		reg_W,		reg_W,		reg_W,		reg_W,		reg_W,		reg_W,		reg_W,
/*E*/	src_L,		src_L,		src_L,		src_L,		src_L,		src_L,		e,			reg_L,
		reg_L,		reg_L,		reg_L,		reg_L,		reg_L,		reg_L,		reg_L,		reg_L,
/*F*/	dst,		dst,		dst,		dst,		dst,		dst,		e,			sngLDX,
		sngSWI,		sngSWI,		sngSWI,		sngSWI,		sngSWI,		sngSWI,		sngSWI,		sngSWI
};

//=============================================================================

u8 TLCS900h_interpret(void)
{
	brCode = FALSE;

	first = FETCH8();	//Get the first byte

	//Is any extra data used by this instruction?
	cycles_extra = 0;
	if (decodeExtra[first])
		(*decodeExtra[first])();

	(*decode[first])();	//Decode

	return cycles + cycles_extra;
}

//int cnte=0;

s32 TLCS900h_cycle(s32 nCycles)
{
	int cy=0;

    while(nCycles>cy){
        brCode = FALSE;
        first = FETCH8();	//Get the first byte
        
        //Is any extra data used by this instruction?
        cycles_extra = 0;
        
        if (decodeExtra[first]) {
            (*decodeExtra[first])();
        }
        (*decode[first])();	//Decode

        cy += (cycles+cycles_extra);
    }

    return cy;
}

//=============================================================================
