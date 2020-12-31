//---------------------------------------------------------------------------
// NEOPOP : Emulator as in Dreamland
//
// Copyright (c) 2001-2002 by neopop_uk
//---------------------------------------------------------------------------


#ifndef __TLCS900H_INTERPRET__
#define __TLCS900H_INTERPRET__

//#define PC32ADR // FETCHに関連するアクセスを高速化

//=============================================================================

//Interprets a single instruction from 'pc', 
//pc is incremented to the start of the next instruction.
//Returns the number of cycles taken for this instruction
u8 interpret(void);

//=============================================================================
extern u32 mem;	
extern int size;
extern u8 first;			//First byte
extern u8 second;			//Second byte
extern u8 R;				//(second & 7)
extern u8 rCode;
extern u8 cycles;
extern BOOL brCode;

//=============================================================================
//void __cdecl instruction_error(char* vaMessage,...);
//=============================================================================

//Confirms a condition code check
static BOOL conditionCode(int cc);

//=============================================================================

//Translate an rr or RR value for MUL/MULS/DIV/DIVS
static u8 get_rr_Target(void);
static u8 get_RR_Target(void);

//=============================================================================
//=============================================================================

void reset_registers(void);
void dump_registers_TLCS900h(void);

//The value read by bad rCodes, leave 0, improves "Gals Fighters"
#define RERR_VALUE		0

//=============================================================================

extern unsigned char *ng_map[0x100];


#ifdef PC32ADR
extern u8* pc32_adr;
#else
extern u32 pc32;
#endif

extern u16	sr;
extern u8 f_dash;

extern u32 gprBank[4][4], gpr[4];
extern u32 rErr;
//extern u8 statusRFP;

////GPR Access                //Reg.Code Access
//extern u8  *gprMapB[4][8], *regCodeMapB[4][256];
//extern u16 *gprMapW[4][8], *regCodeMapW[4][128];
//extern u32 *gprMapL[4][8], *regCodeMapL[4][64];

extern u8  **ex_gprMapB, **ex_regCodeMapB;
extern u16 **ex_gprMapW, **ex_regCodeMapW;
extern u32 **ex_gprMapL, **ex_regCodeMapL;

#if 1
#define regB(x)     (*(ex_gprMapB[(x)]))
#define regW(x)     (*(ex_gprMapW[(x)]))
#define regL(x)     (*(ex_gprMapL[(x)]))
#define rCodeB(r)   (*(ex_regCodeMapB[(r)]))
#define rCodeW(r)   (*(ex_regCodeMapW[(r) >> 1]))
#define rCodeL(r)   (*(ex_regCodeMapL[(r) >> 2]))
#else
//#define regB(x)     (*(gprMapB[statusRFP][(x)]))
//#define regW(x)     (*(gprMapW[statusRFP][(x)]))
//#define regL(x)     (*(gprMapL[statusRFP][(x)]))
//#define rCodeB(r)   (*(regCodeMapB[statusRFP][(r)]))
//#define rCodeW(r)   (*(regCodeMapW[statusRFP][(r) >> 1]))
//#define rCodeL(r)   (*(regCodeMapL[statusRFP][(r) >> 2]))
#endif

//Common Registers
#define REGA		(regB(1))
#define REGWA		(regW(0))
#define REGBC		(regW(1))
#define REGXSP		(gpr[3])

//=============================================================================

u8 statusIFF(void);
void setStatusIFF(u8 iff);

void setStatusRFP(u8 rfp);
void changedSP(void);

#define FLAG_S ((sr & 0x0080) >> 7)
#define FLAG_Z ((sr & 0x0040) >> 6)
#define FLAG_H ((sr & 0x0010) >> 4)
#define FLAG_V ((sr & 0x0004) >> 2)
#define FLAG_N ((sr & 0x0002) >> 1)
#define FLAG_C (sr & 1)

#define SETFLAG_S(s) { u16 sr1 = sr & 0xFF7F; if (s) sr1 |= 0x0080; sr = sr1; }
#define SETFLAG_Z(z) { u16 sr1 = sr & 0xFFBF; if (z) sr1 |= 0x0040; sr = sr1; }
#define SETFLAG_H(h) { u16 sr1 = sr & 0xFFEF; if (h) sr1 |= 0x0010; sr = sr1; }
#define SETFLAG_V(v) { u16 sr1 = sr & 0xFFFB; if (v) sr1 |= 0x0004; sr = sr1; }
#define SETFLAG_N(n) { u16 sr1 = sr & 0xFFFD; if (n) sr1 |= 0x0002; sr = sr1; }
#define SETFLAG_C(c) { u16 sr1 = sr & 0xFFFE; if (c) sr1 |= 0x0001; sr = sr1; }

#define SETFLAG_S0		{ sr &= 0xFF7F;	}
#define SETFLAG_Z0		{ sr &= 0xFFBF;	}
#define SETFLAG_H0		{ sr &= 0xFFEF;	}
#define SETFLAG_V0		{ sr &= 0xFFFB;	}
#define SETFLAG_N0		{ sr &= 0xFFFD;	}
#define SETFLAG_C0		{ sr &= 0xFFFE;	}

#define SETFLAG_S1		{ sr |= 0x0080; }
#define SETFLAG_Z1		{ sr |= 0x0040; }
#define SETFLAG_H1		{ sr |= 0x0010; }
#define SETFLAG_V1		{ sr |= 0x0004; }
#define SETFLAG_N1		{ sr |= 0x0002; }
#define SETFLAG_C1		{ sr |= 0x0001; }


//=============================================================================



#endif

