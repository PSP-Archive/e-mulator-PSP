/** M6502: portable 6502 emulator ****************************/
/**                                                         **/
/**                         M6502.c                         **/
/**                                                         **/
/** This file contains implementation for 6502 CPU. Don't   **/
/** forget to provide Rd6502(), Wr6502(), Loop6502(), and   **/
/** possibly Op6502() functions to accomodate the emulated  **/
/** machine's architecture.                                 **/
/**                                                         **/
/** Copyright (C) Marat Fayzullin 1996                      **/
/**               Alex Krasivsky  1996                      **/
/** Modyfied      BERO            1998                      **/
/** Modyfied      hmmx            1998                      **/
/**     You are not allowed to distribute this software     **/
/**     commercially. Please, notify me, if you make any    **/
/**     changes to this file.                               **/
/*************************************************************/

#include "M6502.h"
//#include "debug.h"
//#include "pcedebug.h"

#ifdef HuC6280
#include "HuTables.h"
#define	BANK_SET(P,V)	bank_set(P,V)


extern void bank_set(byte P,byte V);
//Page[P]=ROMMap[V]-(P)*0x2000/*;printf("bank%x,%02x ",P,V)*/
extern byte *PCE_ROMMap[256];
extern void  IO_write(word A,byte V);
extern void  VDC_d_write(word A,byte V);
#else
#include "Tables.h"
#endif
#include <stdio.h>

/** System-Dependent Stuff ***********************************/
/** This is system-dependent code put here to speed things  **/
/** up. It has to stay inlined to be fast.                  **/
/*************************************************************/
#define FAST_RDOP
extern byte *PCE_Page[8];
extern byte *PCE_RAM;
//extern byte RAM[0x8000];
byte Op6502( unsigned short A){ return (PCE_Page[A>>13][A]); }
unsigned short Op6502w(unsigned short A) { return (PCE_Page[A>>13][A]) | (PCE_Page[(A+1)>>13][A+1]<<8); }
unsigned short RdRAMw(unsigned short A) { return PCE_RAM[A]|(PCE_RAM[A+1]<<8); }
#define	RdRAM(A)	PCE_RAM[A]
#define	WrRAM(A,V)	PCE_RAM[A]=V
#define	AdrRAM(A)	&PCE_RAM[A]

/** FAST_RDOP ************************************************/
/** With this #define not present, Rd6502() should perform  **/
/** the functions of Rd6502().                              **/
/*************************************************************/
#ifndef FAST_RDOP
#define Op6502(A) Rd6502(A)
#endif

#define	C_SFT	0
#define	Z_SFT	1
#define	I_SFT	2
#define	D_SFT	3
#define	B_SFT	4
#define	R_SFT	5
#define	V_SFT	6
#define	N_SFT	7

#define	_A	R->A
#define	_P	R->P
#define	_X	R->X
#define	_Y	R->Y
#define	_S	R->S
#define	_PC	R->PC
#define	_PC_	R->PC.W
#define _ZF	R->ZF
#define _VF	R->VF
#define _NF	R->NF
#define	_IPeriod	R->IPeriod
#define	_ICount		R->ICount
#define	_IRequest	R->IRequest
#define	_IPeriod	R->IPeriod
#define	_IBackup	R->IBackup
#define	_TrapBadOps	R->TrapBadOps
#define	_Trace	R->Trace
#define	_Trap	R->Trap
#define	_AfterCLI	R->AfterCLI
#define	_User	R->User
#define _CycleCount	(*(int*)&_User)

#define	ZP	0
#define	SP	0x100

/** Addressing Methods ***************************************/
/** These macros calculate and return effective addresses.  **/
/*************************************************************/
#define MCZp()	(Op6502(_PC_++))+ZP
#define MCZx()	(byte)(Op6502(_PC_++)+_X)+ZP
#define MCZy()	(byte)(Op6502(_PC_++)+_Y)+ZP
//#define MCZx()	((Op6502(_PC_++)+_X)&0xff)
//#define MCZy()	((Op6502(_PC_++)+_Y)&0xff)
#define	MCIx()	(RdRAMw(MCZx()))
#define	MCIy()	(RdRAMw(MCZp())+_Y)
#define	MCAb()	(Op6502w(_PC_))
#define MCAx()	(Op6502w(_PC_)+_X)
#define MCAy()	(Op6502w(_PC_)+_Y)

#define MC_Ab(Rg)	M_LDWORD(Rg)
#define MC_Zp(Rg)	Rg.B.l=Op6502(_PC_++);Rg.B.h=ZP>>8
#define MC_Zx(Rg)	Rg.B.l=Op6502(_PC_++)+R->X;Rg.B.h=ZP>>8
#define MC_Zy(Rg)	Rg.B.l=Op6502(_PC_++)+R->Y;Rg.B.h=ZP>>8
#define MC_Ax(Rg)	M_LDWORD(Rg);Rg.W+=_X
#define MC_Ay(Rg)	M_LDWORD(Rg);Rg.W+=_Y
#if 1
#define MC_Ix(Rg)	K.W=MCZx(); \
			Rg.B.l=RdRAM(K.W);Rg.B.h=RdRAM(K.W+1)
#define MC_Iy(Rg)	K.W=MCZp(); \
			Rg.B.l=RdRAM(K.W);Rg.B.h=RdRAM(K.W+1); \
			Rg.W+=_Y
#else
#define	MC_Ix(Rg)	Rg.W=(RdRAMw(MCZx()))
#define	MC_Iy(Rg)	Rg.W=(RdRAMw(MCZp())+_Y)
#endif
/*
#define MC_Ix(Rg)	{byte *p=AdrRAM(MCZx()); \
			Rg.B.l=p[0];Rg.B.h=p[1]; }
#define MC_Iy(Rg)	{byte *p=AdrRAM(MCZp()); \
			Rg.B.l=p[0];Rg.B.h=p[1]; } \
			Rg.W+=_Y;
*/

/** Reading From Memory **************************************/
/** These macros calculate address and read from it.        **/
/*************************************************************/
#define MR_Ab(Rg)	MC_Ab(J);Rg=Rd6502(J.W)
//#define MR_Ab(Rg)	Rg=Rd6502(MCAb());_PC_+=2
#define MR_Im(Rg)	Rg=Op6502(_PC_++)
#define	MR_Zp(Rg)	Rg=RdRAM(MCZp())
#define MR_Zx(Rg)	Rg=RdRAM(MCZx())
#define MR_Zy(Rg)	Rg=RdRAM(MCZy())
#define	MR_Ax(Rg)	MC_Ax(J);Rg=Rd6502(J.W)
#define MR_Ay(Rg)	MC_Ay(J);Rg=Rd6502(J.W)
//#define MR_Ax(Rg)	Rg=Rd6502(MCAx());_PC_+=2;
//#define MR_Ay(Rg)	Rg=Rd6502(MCAy());_PC_+=2;
//#define MR_Ix(Rg)	Rg=Rd6502(MCIx())
//#define MR_Iy(Rg)	Rg=Rd6502(MCIy())
#define MR_Ix(Rg)	MC_Ix(J);Rg=Rd6502(J.W)
#define MR_Iy(Rg)	MC_Iy(J);Rg=Rd6502(J.W)

/** Writing To Memory ****************************************/
/** These macros calculate address and write to it.         **/
/*************************************************************/
//#define MW_Ab(Rg)	Wr6502(MCAb(),Rg);_PC_+=2
#define MW_Ab(Rg)	MC_Ab(J);Wr6502(J.W,Rg)
#define MW_Zp(Rg)	WrRAM(MCZp(),Rg)
#define MW_Zx(Rg)	WrRAM(MCZx(),Rg)
#define MW_Zy(Rg)	WrRAM(MCZy(),Rg)
#define MW_Ax(Rg)	MC_Ax(J);Wr6502(J.W,Rg)
#define MW_Ay(Rg)	MC_Ay(J);Wr6502(J.W,Rg)
//#define MW_Ax(Rg)	Wr6502(MCAx(),Rg);_PC_+=2
//#define MW_Ay(Rg)	Wr6502(MCAy(),Rg);_PC_+=2
//#define MW_Ix(Rg)	Wr6502(MCIx(),Rg)
//#define MW_Iy(Rg)	Wr6502(MCIy(),Rg)
#define MW_Ix(Rg)	MC_Ix(J);Wr6502(J.W,Rg)
#define MW_Iy(Rg)	MC_Iy(J);Wr6502(J.W,Rg)

/** Modifying Memory *****************************************/
/** These macros calculate address and modify it.           **/
/*************************************************************/
#define MM_Ab(Cmd)	MC_Ab(J);I=Rd6502(J.W);Cmd(I);Wr6502(J.W,I)
#define MM_Zp(Cmd)	J.W=MCZp();I=RdRAM(J.W);Cmd(I);WrRAM(J.W,I)
#define MM_Zx(Cmd)	J.W=MCZx();I=RdRAM(J.W);Cmd(I);WrRAM(J.W,I)
//#define MM_Zp(Cmd)	{unsigned A=MCZp();I=RdRAM(A);Cmd(I);WrRAM(A,I); }
//#define MM_Zx(Cmd)	{unsigned A=MCZx();I=RdRAM(A);Cmd(I);WrRAM(A,I); }
//#define MM_Zp(Cmd)	{byte *p=AdrRAM(MCZp());I=*p;Cmd(I);*p=I;}
//#define MM_Zx(Cmd)	{byte *p=AdrRAM(MCZx());I=*p;Cmd(I);*p=I;}
#define MM_Ax(Cmd)	MC_Ax(J);I=Rd6502(J.W);Cmd(I);Wr6502(J.W,I)

/** Other Macros *********************************************/
/** Calculating flags, stack, jumps, arithmetics, etc.      **/
/*************************************************************/
#define M_FL(Rg)	_ZF=_NF=Rg
#define M_LDWORD(Rg)	Rg.B.l=Op6502(_PC_);Rg.B.h=Op6502(_PC_+1);_PC_+=2

#define M_PUSH(Rg)	WrRAM(SP+_S,Rg);_S--
#define M_POP(Rg)	_S++;Rg=RdRAM(SP+_S)
#define M_PUSH_P(Rg)	M_PUSH(((Rg)&~(N_FLAG|V_FLAG|Z_FLAG))|(_NF&N_FLAG)|(_VF&V_FLAG)|(_ZF? 0:Z_FLAG))
#define M_POP_P(Rg)		M_POP(Rg);_NF=_VF=Rg;_ZF=(Rg&Z_FLAG? 0:1)
#ifdef HuC6280
#define M_JR		_PC_+=(offset)Op6502(_PC_)+1;cycle+=2
#else
#define M_JR		_PC_+=(offset)Op6502(_PC_)+1;cycle++
#endif

#define M_ADC(Rg) \
  if(_P&D_FLAG) \
  { \
    K.B.l=(_A&0x0F)+(Rg&0x0F)+(_P&C_FLAG); \
    K.B.h=(_A>>4)+(Rg>>4);/*+(K.B.l>15? 1:0);*/ \
    if(K.B.l>9) { K.B.l+=6;K.B.h++; } \
    if(K.B.h>9) K.B.h+=6; \
    _A=(K.B.l&0x0F)|(K.B.h<<4); \
    _P=(_P&~C_FLAG)|(K.B.h>15? C_FLAG:0); \
	_ZF=_NF=_A; \
	cycle++; \
  } \
  else \
  { \
    K.W=_A+Rg+(_P&C_FLAG); \
    _P&=~C_FLAG; \
    _P|=(K.B.h? C_FLAG:0); \
    _VF=(~(_A^Rg)&(_A^K.B.l))>>1; \
	_ZF=_NF=K.B.l; \
    _A=K.B.l; \
  }

/* Warning! C_FLAG is inverted before SBC and after it */
#define M_SBC(Rg) \
  if(_P&D_FLAG) \
  { \
    K.B.l=(_A&0x0F)-(Rg&0x0F)-(~_P&C_FLAG); \
    if(K.B.l&0x10) K.B.l-=6; \
    K.B.h=(_A>>4)-(Rg>>4)-((K.B.l&0x10)==0x10); \
    if(K.B.h&0x10) K.B.h-=6; \
    _A=(K.B.l&0x0F)|(K.B.h<<4); \
    _P=(_P&~C_FLAG)|((K.B.h&0x10)? 0:C_FLAG); \
	_ZF=_NF=_A; \
	cycle++; \
  } \
  else \
  { \
    K.W=_A-Rg-(~_P&C_FLAG); \
    _P&=~C_FLAG; \
    _P|=(K.B.h? 0:C_FLAG); \
    _VF=((_A^Rg)&(_A^K.B.l))>>1; \
    _ZF=_NF=K.B.l; \
    _A=K.B.l; \
  }

#define M_CMP(Rg1,Rg2) \
  K.W=Rg1-Rg2; \
  _P&=~C_FLAG; \
  _P|=(K.B.h? 0:C_FLAG); \
  _ZF=_NF=K.B.l
#define M_BIT(Rg) \
  _NF=_VF=Rg;_ZF=Rg&_A

#define M_AND(Rg)	_A&=Rg;M_FL(_A)
#define M_ORA(Rg)	_A|=Rg;M_FL(_A)
#define M_EOR(Rg)	_A^=Rg;M_FL(_A)
#define M_INC(Rg)	Rg++;M_FL(Rg)
#define M_DEC(Rg)	Rg--;M_FL(Rg)

#define M_ASL(Rg)	_P&=~C_FLAG;_P|=Rg>>7;Rg<<=1;M_FL(Rg)
#define M_LSR(Rg)	_P&=~C_FLAG;_P|=Rg&C_FLAG;Rg>>=1;M_FL(Rg)
#define M_ROL(Rg)	K.B.l=(Rg<<1)|(_P&C_FLAG); \
			_P&=~C_FLAG;_P|=Rg>>7;Rg=K.B.l; \
			M_FL(Rg)
#define M_ROR(Rg)	K.B.l=(Rg>>1)|(_P<<7); \
			_P&=~C_FLAG;_P|=Rg&C_FLAG;Rg=K.B.l; \
			M_FL(Rg)

/** Reset6502() **********************************************/
/** This function can be used to reset the s before **/
/** starting execution with Run6502(). It sets s to **/
/** their initial values.                                   **/
/*************************************************************/
void Reset6502(M6502 *R)
{
#ifdef HuC6280
  R->MPR[7]=0x00; BANK_SET(7,0x00);
  R->MPR[6]=0x05; BANK_SET(6,0x05);
  R->MPR[5]=0x04; BANK_SET(5,0x04);
  R->MPR[4]=0x03; BANK_SET(4,0x03);
  R->MPR[3]=0x02; BANK_SET(3,0x02);
  R->MPR[2]=0x01; BANK_SET(2,0x01);
  R->MPR[1]=0xF8; BANK_SET(1,0xF8);
  R->MPR[0]=0xFF; BANK_SET(0,0xFF);
#endif
  _A=_X=_Y=0x00;
  _P=I_FLAG;
  _NF=_VF=0;
  _ZF=0xFF;
  _S=0xFF;
  _PC.B.l=Op6502(VEC_RESET);
  _PC.B.h=Op6502(VEC_RESET+1);
  _ICount=_IPeriod;
  _IRequest=INT_NONE;
  _AfterCLI=0;
#ifdef HuC6280
  _CycleCount=0;
#endif
}

#if 0
/** Exec6502() ***********************************************/
/** This function will execute a single 6502 opcode. It     **/
/** will then return next PC, and current  values   **/
/** in R.                                                   **/
/*************************************************************/
word Exec6502(M6502 *R)
{
    pair J,K;
    byte I;
    
    I=Op6502(_PC_++);
    _ICount-=Cycles[I];
    switch(I) {
#include "Codes.h"
    }
    
    /* We are done */
    return(_PC_);
}
#endif

/** Int6502() ************************************************/
/** This function will generate interrupt of a given type.  **/
/** INT_NMI will cause a non-maskable interrupt. INT_IRQ    **/
/** will cause a normal interrupt, unless I_FLAG set in R.  **/
/*************************************************************/
void Int6502(M6502 *R,byte Type)
{
    pair J;
    
    if((Type==INT_NMI)||(/*(Type==INT_IRQ)&&*/!(_P&I_FLAG))) { 
        _ICount-=7;
        M_PUSH(_PC.B.h);
        M_PUSH(_PC.B.l);
        M_PUSH_P(_P&~(B_FLAG|T_FLAG));
        _P&=~D_FLAG;
        
        if (Type==INT_NMI){
            J.W=VEC_NMI;
        } else {
            _P|=I_FLAG;
            switch(Type){
              case INT_IRQ:J.W=VEC_IRQ;break;
#ifdef HuC6280
              case INT_IRQ2:J.W=VEC_IRQ2;break;
              case INT_TIMER:J.W=VEC_TIMER;break;
#endif
            }
        }
        _PC.B.l=Op6502(J.W);
        _PC.B.h=Op6502(J.W+1);
    } else {
        _IRequest|=Type;
    }
}

#ifndef WIN32 // WIN32環境だとコンパイル通らなかったヨ
#define M6502_JUMPTABLE	// Enable Jumptable
#endif //WIN32

#if 0
static void after_cli(M6502 *R)
{
}

/** Run6502() ************************************************/
/** This function will run 6502 code until Loop6502() call  **/
/** returns INT_QUIT. It will return the PC at which        **/
/** emulation stopped, and current  values in R.    **/
/*************************************************************/
word Run6502(M6502 *R)
{
    pair J,K;
    byte I;
    int  cycle;
    int  CycleCountOld=0;
    extern int TimerPeriod;

    for(;;) {
        I=Op6502(_PC_++);
        cycle = Cycles[I];
#ifdef M6502_JUMPTABLE

#define  OPCODE_BEGIN(xx)  op##xx:
#define  OPCODE_END goto end_execute;

        static void *opcode_table[256] = {
            &&op00, &&op01, &&op02, &&op03, &&op04, &&op05, &&op06, &&op07,
            &&op08, &&op09, &&op0A, &&op0B, &&op0C, &&op0D, &&op0E, &&op0F,
            &&op10, &&op11, &&op12, &&op13, &&op14, &&op15, &&op16, &&op17,
            &&op18, &&op19, &&op1A, &&op1B, &&op1C, &&op1D, &&op1E, &&op1F,
            &&op20, &&op21, &&op22, &&op23, &&op24, &&op25, &&op26, &&op27,
            &&op28, &&op29, &&op2A, &&op2B, &&op2C, &&op2D, &&op2E, &&op2F,
            &&op30, &&op31, &&op32, &&op33, &&op34, &&op35, &&op36, &&op37,
            &&op38, &&op39, &&op3A, &&op3B, &&op3C, &&op3D, &&op3E, &&op3F,
            &&op40, &&op41, &&op42, &&op43, &&op44, &&op45, &&op46, &&op47,
            &&op48, &&op49, &&op4A, &&op4B, &&op4C, &&op4D, &&op4E, &&op4F,
            &&op50, &&op51, &&op52, &&op53, &&op54, &&op55, &&op56, &&op57,
            &&op58, &&op59, &&op5A, &&op5B, &&op5C, &&op5D, &&op5E, &&op5F,
            &&op60, &&op61, &&op62, &&op63, &&op64, &&op65, &&op66, &&op67,
            &&op68, &&op69, &&op6A, &&op6B, &&op6C, &&op6D, &&op6E, &&op6F,
            &&op70, &&op71, &&op72, &&op73, &&op74, &&op75, &&op76, &&op77,
            &&op78, &&op79, &&op7A, &&op7B, &&op7C, &&op7D, &&op7E, &&op7F,
            &&op80, &&op81, &&op82, &&op83, &&op84, &&op85, &&op86, &&op87,
            &&op88, &&op89, &&op8A, &&op8B, &&op8C, &&op8D, &&op8E, &&op8F,
            &&op90, &&op91, &&op92, &&op93, &&op94, &&op95, &&op96, &&op97,
            &&op98, &&op99, &&op9A, &&op9B, &&op9C, &&op9D, &&op9E, &&op9F,
            &&opA0, &&opA1, &&opA2, &&opA3, &&opA4, &&opA5, &&opA6, &&opA7,
            &&opA8, &&opA9, &&opAA, &&opAB, &&opAC, &&opAD, &&opAE, &&opAF,
            &&opB0, &&opB1, &&opB2, &&opB3, &&opB4, &&opB5, &&opB6, &&opB7,
            &&opB8, &&opB9, &&opBA, &&opBB, &&opBC, &&opBD, &&opBE, &&opBF,
            &&opC0, &&opC1, &&opC2, &&opC3, &&opC4, &&opC5, &&opC6, &&opC7,
            &&opC8, &&opC9, &&opCA, &&opCB, &&opCC, &&opCD, &&opCE, &&opCF,
            &&opD0, &&opD1, &&opD2, &&opD3, &&opD4, &&opD5, &&opD6, &&opD7,
            &&opD8, &&opD9, &&opDA, &&opDB, &&opDC, &&opDD, &&opDE, &&opDF,
            &&opE0, &&opE1, &&opE2, &&opE3, &&opE4, &&opE5, &&opE6, &&opE7,
            &&opE8, &&opE9, &&opEA, &&opEB, &&opEC, &&opED, &&opEE, &&opEF,
            &&opF0, &&opF1, &&opF2, &&opF3, &&opF4, &&opF5, &&opF6, &&opF7,
            &&opF8, &&opF9, &&opFA, &&opFB, &&opFC, &&opFD, &&opFE, &&opFF
          };
        
#else /* !M6502_JUMPTABLE */
#define  OPCODE_BEGIN(xx)  case 0x##xx:
#define  OPCODE_END        break;
#endif /* !M6502_JUMPTABLE */

#ifdef M6502_JUMPTABLE
		goto *opcode_table[I];
#else /* !M6502_JUMPTABLE */
		switch(I) {
#endif /* !M6502_JUMPTABLE */
        #include "HuCodes.h"
        #include "Codes.h"
#ifdef M6502_JUMPTABLE
end_execute:
#else /* !M6502_JUMPTABLE */
        } // switch()
#endif /* !M6502_JUMPTABLE */

        _ICount-=cycle;
        //	*(DWORD*)&_User+=cycle;
        _CycleCount += cycle;

        /* If cycle counter expired... */
        if(_ICount<=0) {
            /* If we have come after CLI, get INT_? from IRequest */
            /* Otherwise, get it from the loop handler            */
            if(_AfterCLI) {
                if (_IRequest&INT_TIMER) {
                    _IRequest&=~INT_TIMER;
                    I=INT_TIMER;
                } else if (_IRequest&INT_IRQ) {
                      _IRequest&=~INT_IRQ;
                    I=INT_IRQ;
                }
                else if (_IRequest&INT_IRQ2) {
                    _IRequest&=~INT_IRQ2;
                    I=INT_IRQ2;
                }
                _ICount = 0;
                if (_IRequest==0) {
                    _ICount=_IBackup;  /* Restore the ICount        */
                    _AfterCLI=0;            /* Done with AfterCLI state  */
                }
            } else {
                I=Loop6502(R);            /* Call the periodic handler */
                _ICount+=_IPeriod;     /* Reset the cycle counter   */
            }

            if(I==INT_QUIT) return(_PC_); /* Exit if INT_QUIT     */
            if(I) Int6502(R,I);              /* Interrupt if needed  */

            if((DWORD)(_CycleCount-CycleCountOld) > (DWORD)TimerPeriod*2)
              CycleCountOld = _CycleCount;
        }
        else {
            if (_CycleCount-CycleCountOld >= TimerPeriod) {
                CycleCountOld += TimerPeriod;
                I=TimerInt(R);
                if(I) Int6502(R,I);
            }
        }
    }

    /* Execution stopped */
    return(_PC_);
}
#else


#include "pce.h"

static int CycleCountOld;

static void after_cli(M6502 *R)
{
	if(_IRequest) {
		byte I=0;
		if (_IRequest&INT_TIMER) {
			_IRequest&=~INT_TIMER;
			I=INT_TIMER;
		} else if (_IRequest&INT_IRQ) {
			_IRequest&=~INT_IRQ;
			I=INT_IRQ;
		}
		else if (_IRequest&INT_IRQ2) {
			_IRequest&=~INT_IRQ2;
			I=INT_IRQ2;
		}

		_ICount = 0;

		if (_IRequest==0 || 1) {
			_ICount=_IBackup;  /* Restore the ICount        */
			_AfterCLI=0;            /* Done with AfterCLI state  */
		} else {
			_ICount = _ICount;      /* こういう場合はどうするのか？ */
		}

		if(I) Int6502(R,I);              /* Interrupt if needed  */
	}
}

/** Run6502() ************************************************/
/** This function will run 6502 code until Loop6502() call  **/
/** returns INT_QUIT. It will return the PC at which        **/
/** emulation stopped, and current  values in R.    **/
/*************************************************************/
word Run6502(M6502 *R)
{
    int  cycle;
    pair J,K;
	byte I;
    extern int TimerPeriod;

    for(io.scanline=0;io.scanline<SCANLINES_PER_FRAME;io.scanline++) {

		// _ICount = 453
		while(_ICount>0) {

			I=Op6502(_PC_++);
			cycle = Cycles[I];
	        
#ifdef M6502_JUMPTABLE

#define  OPCODE_BEGIN(xx)  op##xx:
#define  OPCODE_END goto end_execute;

			static void *opcode_table[256] = {
				&&op00, &&op01, &&op02, &&op03, &&op04, &&op05, &&op06, &&op07,
				&&op08, &&op09, &&op0A, &&op0B, &&op0C, &&op0D, &&op0E, &&op0F,
				&&op10, &&op11, &&op12, &&op13, &&op14, &&op15, &&op16, &&op17,
				&&op18, &&op19, &&op1A, &&op1B, &&op1C, &&op1D, &&op1E, &&op1F,
				&&op20, &&op21, &&op22, &&op23, &&op24, &&op25, &&op26, &&op27,
				&&op28, &&op29, &&op2A, &&op2B, &&op2C, &&op2D, &&op2E, &&op2F,
				&&op30, &&op31, &&op32, &&op33, &&op34, &&op35, &&op36, &&op37,
				&&op38, &&op39, &&op3A, &&op3B, &&op3C, &&op3D, &&op3E, &&op3F,
				&&op40, &&op41, &&op42, &&op43, &&op44, &&op45, &&op46, &&op47,
				&&op48, &&op49, &&op4A, &&op4B, &&op4C, &&op4D, &&op4E, &&op4F,
				&&op50, &&op51, &&op52, &&op53, &&op54, &&op55, &&op56, &&op57,
				&&op58, &&op59, &&op5A, &&op5B, &&op5C, &&op5D, &&op5E, &&op5F,
				&&op60, &&op61, &&op62, &&op63, &&op64, &&op65, &&op66, &&op67,
				&&op68, &&op69, &&op6A, &&op6B, &&op6C, &&op6D, &&op6E, &&op6F,
				&&op70, &&op71, &&op72, &&op73, &&op74, &&op75, &&op76, &&op77,
				&&op78, &&op79, &&op7A, &&op7B, &&op7C, &&op7D, &&op7E, &&op7F,
				&&op80, &&op81, &&op82, &&op83, &&op84, &&op85, &&op86, &&op87,
				&&op88, &&op89, &&op8A, &&op8B, &&op8C, &&op8D, &&op8E, &&op8F,
				&&op90, &&op91, &&op92, &&op93, &&op94, &&op95, &&op96, &&op97,
				&&op98, &&op99, &&op9A, &&op9B, &&op9C, &&op9D, &&op9E, &&op9F,
				&&opA0, &&opA1, &&opA2, &&opA3, &&opA4, &&opA5, &&opA6, &&opA7,
				&&opA8, &&opA9, &&opAA, &&opAB, &&opAC, &&opAD, &&opAE, &&opAF,
				&&opB0, &&opB1, &&opB2, &&opB3, &&opB4, &&opB5, &&opB6, &&opB7,
				&&opB8, &&opB9, &&opBA, &&opBB, &&opBC, &&opBD, &&opBE, &&opBF,
				&&opC0, &&opC1, &&opC2, &&opC3, &&opC4, &&opC5, &&opC6, &&opC7,
				&&opC8, &&opC9, &&opCA, &&opCB, &&opCC, &&opCD, &&opCE, &&opCF,
				&&opD0, &&opD1, &&opD2, &&opD3, &&opD4, &&opD5, &&opD6, &&opD7,
				&&opD8, &&opD9, &&opDA, &&opDB, &&opDC, &&opDD, &&opDE, &&opDF,
				&&opE0, &&opE1, &&opE2, &&opE3, &&opE4, &&opE5, &&opE6, &&opE7,
				&&opE8, &&opE9, &&opEA, &&opEB, &&opEC, &&opED, &&opEE, &&opEF,
				&&opF0, &&opF1, &&opF2, &&opF3, &&opF4, &&opF5, &&opF6, &&opF7,
				&&opF8, &&opF9, &&opFA, &&opFB, &&opFC, &&opFD, &&opFE, &&opFF
			};
	        
#else /* !M6502_JUMPTABLE */
	#define  OPCODE_BEGIN(xx)  case 0x##xx:
	#define  OPCODE_END        break;
#endif /* !M6502_JUMPTABLE */

#ifdef M6502_JUMPTABLE
			goto *opcode_table[I];
#else /* !M6502_JUMPTABLE */
			switch(I) {
#endif /* !M6502_JUMPTABLE */
			#include "HuCodes.h"
			#include "Codes.h"
#ifdef M6502_JUMPTABLE
	end_execute:
#else /* !M6502_JUMPTABLE */
			}
#endif /* !M6502_JUMPTABLE */

			_ICount-=cycle;
			//	*(DWORD*)&_User+=cycle;
			_CycleCount += cycle;

            if (_CycleCount-CycleCountOld >= TimerPeriod) {
                CycleCountOld += TimerPeriod;
				if( (I=TimerInt(R)) ) {
					Int6502(R,I);
				}
            }
		}

        /* 
        ** 各scanlineに対応したCycleが終わったら描画処理
        **/
		if((I=Loop6502(R))) {          /* Call the periodic handler */
            Int6502(R,I);              /* Interrupt if needed  */
		}

        _ICount+=_IPeriod;     /* Reset the cycle counter   */

		if((DWORD)(_CycleCount-CycleCountOld) > (DWORD)TimerPeriod*2) {
			CycleCountOld = _CycleCount;
		}
    }

    /* Execution stopped */
    return(_PC_);
}

#endif
