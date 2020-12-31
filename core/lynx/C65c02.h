#ifndef C65C02_H
#define C65C02_H

//
// Handy definitions
//

#define NMI_VECTOR	0xfffa
#define BOOT_VECTOR	0xfffc
#define IRQ_VECTOR	0xfffe

#define MAX_CPU_BREAKPOINTS	8

//
// ACCESS MACROS
//

#define CPU_PEEK(m)			  (((m<0xfc00)? mRam_Data[m]:mMemoryHandlers[m].pPeek(m)))
#define CPU_PEEKW(m)	      (((m<0xfc00)?(mRam_Data[m]+(mRam_Data[m+1]<<8)):((mMemoryHandlers[m].pPeek(m))+(mMemoryHandlers[m].pPeek(m+1)<<8))))
#define CPU_POKE(m1,m2)     { if(m1<0xfc00) mRam_Data[m1]=m2; else mMemoryHandlers[m1].pPoke(m1,m2); }

enum {	
	illegal=0,
	accu,
	imm,
	absl,
	zp,
	zpx,
	zpy,
	absx,
	absy,
	iabsx,
	impl,
	rel,
	zrel,
	indx,
	indy,
	iabs,
	ind
};

typedef struct {
	int PS;		// Processor status register   8 bits
	int A;		// Accumulator                 8 bits
	int X;		// X index register            8 bits
	int Y;		// Y index register            8 bits
	int SP;		// Stack Pointer               8 bits
	int Opcode;	// Instruction opcode          8 bits
	int Operand;// Intructions operand		  16 bits
	int PC;		// Program Counter            16 bits
	u32 NMI;
	u32 IRQ;
	u32 WAIT;
}C6502_REGS;

//
// The CPU emulation macros
//
//#include "c6502mak.h"
//
// The CPU emulation macros
//

void C65C02_Init(void);
void C65C02_Exit();
void C65C02_Reset(void);
void C65C02_Update(int cycles);
void C65C02_IRQ_Check(void);
int  C65C02_GetPC(void);
void C65C02_xILLEGAL(void);
int  C65C02_PS0();
void C65C02_PS1(int ps);


#endif

