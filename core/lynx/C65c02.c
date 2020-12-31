// 8bit 3.6MHz CPU

#include "system.h"
#include "c65c02.h"
#include "c6502mak.h"

#include "c65c02cy.h"




#if 1
//static	CSystemBase	*mSystem;

	// CPU Flags & status

static	int mA;		// Accumulator                 8 bits
static	int mX;		// X index register            8 bits
static	int mY;		// Y index register            8 bits
static	int mSP;		// Stack Pointer               8 bits
static	int mOpcode;  // Instruction opcode          8 bits
static	int mOperand; // Intructions operand		  16 bits
static	int mPC;		// Program Counter            16 bits

static	int mN;		// N flag for processor status register
static	int mV;		// V flag for processor status register
static	int mB;		// B flag for processor status register
static	int mD;		// D flag for processor status register
static	int mI;		// I flag for processor status register
static	int mZ;		// Z flag for processor status register
static	int mC;		// C flag for processor status register

static	int mIRQActive;

extern u8 mRam_Data[RAM_SIZE];

	// Associated lookup tables

static    int mBCDTable[2][256];
#endif


void C65C02_Init(void)//:mSystem(parent)
{
	u16  t;

	// Compute the BCD lookup table
	for(t=0;t<256;++t) {
		mBCDTable[0][t]=((t >> 4) * 10) + (t & 0x0f);
		mBCDTable[1][t]=(((t % 100) / 10) << 4) | (t % 10);
	}
//	C65C02_Reset();
}

void C65C02_Reset(void)
{
	mA=0;
    mX=0;
    mY=0;
	mSP=0xff;
	mOpcode=0;
	mOperand=0;
	mPC=CPU_PEEKW(BOOT_VECTOR);
	mN=0;
	mV=0;
	mB=0;
	mD=0;
	mI=1;
	mZ=1;
	mC=0;
	mIRQActive=0;

	gSystemIRQ=0;
}

void C65C02_IRQ_Check(void)
{
	// 割り込みが存在する場合の処理
	// これは毎回実行する必要が無いのではないかと思う
	if(gSystemIRQ && !mI && !mIRQActive) {
		// IRQ signal clearance is handled by CMikie::Update() as this
		// is the only source of interrupts

		// Push processor status
		PUSH(mPC>>8);
		PUSH(mPC&0xff);
		PUSH(C65C02_PS0()&0xef);		// Clear B flag on stack

		mI=1;				// Stop further interrupts
		mD=0;				// Clear decimal mode

		// Pick up the new PC
		mPC=CPU_PEEKW(IRQ_VECTOR);

		// Log the irq entry time

		// Indicate IRQ activity to stop reentrance
		mIRQActive++;
	}
}

void C65C02_Update(int cycles)
{
	int cy;

	while(cycles>0) {
		// Fetch opcode
		mOpcode=CPU_PEEK(mPC);
		mPC++;

		// Execute Opcode
		cy = cycle[mOpcode&0xff];
		gSystemCycleCount += cy;
		cycles -= cy;

		switch(mOpcode) {
		case 0x00:      xBRK();									break;
		case 0x01:      xINDIRECT_X();			xORA();			break;
		case 0x02:      C65C02_xILLEGAL();						break;
		case 0x03:      C65C02_xILLEGAL();						break;
		case 0x04:      xZEROPAGE();			xTSB();			break;
		case 0x05:      xZEROPAGE();		    xORA();			break;
		case 0x06:      xZEROPAGE();			xASL();			break;
		case 0x07:      C65C02_xILLEGAL();						break;
		case 0x08:      xPHP();									break;
		case 0x09:      xIMMEDIATE();			xORA();			break;
		case 0x0A:      xASLA();								break;
		case 0x0B:      C65C02_xILLEGAL();						break;
		case 0x0C:      xABSOLUTE();			xTSB();			break;
		case 0x0D:      xABSOLUTE();			xORA();			break;
		case 0x0E:      xABSOLUTE();			xASL();			break;
		case 0x0F:      C65C02_xILLEGAL();						break;

		case 0x10:		xBPL();									break;
		case 0x11:		xINDIRECT_Y();			xORA();			break;
		case 0x12:		xINDIRECT();			xORA();			break;
		case 0x13:		C65C02_xILLEGAL();						break;
		case 0x14:      xZEROPAGE();			xTRB();			break;
		case 0x15:      xZEROPAGE_X();			xORA();			break;
		case 0x16:      xZEROPAGE_X();			xASL();			break;
		case 0x17:      C65C02_xILLEGAL();						break;
		case 0x18:      xCLC();									break;
		case 0x19:      xABSOLUTE_Y();			xORA();			break;
		case 0x1A:	    xINCA();								break;
		case 0x1B:      C65C02_xILLEGAL();						break;
		case 0x1C:      xABSOLUTE();			xTRB();			break;
		case 0x1D:		xABSOLUTE_X();			xORA();			break;
		case 0x1E:		xABSOLUTE_X();			xASL();			break;
		case 0x1F:		C65C02_xILLEGAL();						break;

		case 0x20:		xABSOLUTE();			xJSR();			break;
		case 0x21:		xINDIRECT_X();			xAND();			break;
		case 0x22:	    C65C02_xILLEGAL();						break;
		case 0x23:	    C65C02_xILLEGAL();						break;
		case 0x24:      xZEROPAGE();			xBIT();			break;
		case 0x25:		xZEROPAGE();			xAND();			break;
		case 0x26:	    xZEROPAGE();			xROL();			break;
		case 0x27:      C65C02_xILLEGAL();						break;
		case 0x28:		xPLP();									break;
		case 0x29:	    xIMMEDIATE();			xAND();			break;
		case 0x2A:	    xROLA();								break;
		case 0x2B:		C65C02_xILLEGAL();						break;
		case 0x2C:		xABSOLUTE();			xBIT();			break;
		case 0x2D:      xABSOLUTE();			xAND();			break;
		case 0x2E:		xABSOLUTE();			xROL();			break;
		case 0x2F:		C65C02_xILLEGAL();						break;

		case 0x30:								xBMI();			break;
		case 0x31:		xINDIRECT_Y();			xAND();			break;
		case 0x32:		xINDIRECT();			xAND();			break;
		case 0x33:		C65C02_xILLEGAL();						break;
		case 0x34:		xZEROPAGE_X();			xBIT();			break;
		case 0x35:      xZEROPAGE_X();			xAND();			break;
		case 0x36:		xZEROPAGE_X();			xROL();			break;
		case 0x37:      C65C02_xILLEGAL();						break;
		case 0x38:								xSEC();			break;
		case 0x39:		xABSOLUTE_Y();			xAND();			break;
		case 0x3A:								xDECA();		break;
		case 0x3B:      C65C02_xILLEGAL();						break;
		case 0x3C:	    xABSOLUTE_X();			xBIT();			break;
		case 0x3D:	    xABSOLUTE_X();			xAND();			break;
		case 0x3E:		xABSOLUTE_X();			xROL();			break;
		case 0x3F:		C65C02_xILLEGAL();						break;

		case 0x40:
			if(!mB) {
				mIRQActive--;
			}
			// IMPLIED
			xRTI();

			C65C02_IRQ_Check(); /* add by e */

			break;
		case 0x41:		xINDIRECT_X();			xEOR();			break;
		case 0x42:		C65C02_xILLEGAL();						break;
		case 0x43:      C65C02_xILLEGAL();						break;
		case 0x44:		C65C02_xILLEGAL();						break;
		case 0x45:      xZEROPAGE();			xEOR();			break;
		case 0x46:		xZEROPAGE();			xLSR();			break;
		case 0x47:		C65C02_xILLEGAL();						break;
		case 0x48:		xPHA();									break;
		case 0x49:		xIMMEDIATE();			xEOR();			break;
		case 0x4A:		xLSRA();								break;
		case 0x4B:		C65C02_xILLEGAL();						break;
		case 0x4C:		xABSOLUTE();			xJMP();			break;
		case 0x4D:		xABSOLUTE();			xEOR();			break;
		case 0x4E:      xABSOLUTE();			xLSR();			break;
		case 0x4F:		C65C02_xILLEGAL();						break;

		case 0x50:		xBVC();									break;
		case 0x51:      xINDIRECT_Y();			xEOR();			break;
		case 0x52:      xINDIRECT();			xEOR();			break;
		case 0x53:		C65C02_xILLEGAL();						break;
		case 0x54:      C65C02_xILLEGAL();						break;
		case 0x55:		xZEROPAGE_X();			xEOR();			break;
		case 0x56:		xZEROPAGE_X();			xLSR();			break;
		case 0x57:		C65C02_xILLEGAL();						break;
		case 0x58:								xCLI();			break;
		case 0x59:		xABSOLUTE_Y();			xEOR();			break;
		case 0x5A:		xPHY();									break;
		case 0x5B:		C65C02_xILLEGAL();						break;
		case 0x5C:		C65C02_xILLEGAL();						break;
		case 0x5D:		xABSOLUTE_X();			xEOR();			break;
		case 0x5E:		xABSOLUTE_X();			xLSR();			break;
		case 0x5F:		C65C02_xILLEGAL();						break;

		case 0x60:		xRTS();									break;
		case 0x61:      xINDIRECT_X();			xADC();			break;
		case 0x62:		C65C02_xILLEGAL();						break;
		case 0x63:		C65C02_xILLEGAL();						break;
		case 0x64:		xZEROPAGE();			xSTZ();			break;
		case 0x65:		xZEROPAGE();			xADC();			break;
		case 0x66:      xZEROPAGE();			xROR();			break;
		case 0x67:      C65C02_xILLEGAL();						break;
		case 0x68:		xPLA();									break;
		case 0x69:      xIMMEDIATE();			xADC();			break;
		case 0x6A:		xRORA();								break;
		case 0x6B:		C65C02_xILLEGAL();						break;
		case 0x6C:		xINDIRECT_ABSOLUTE();	xJMP();			break;
		case 0x6D:		xABSOLUTE();			xADC();			break;
		case 0x6E:		xABSOLUTE();			xROR();			break;
		case 0x6F:		C65C02_xILLEGAL();						break;

		case 0x70:		xBVS();									break;
		case 0x71:		xINDIRECT_Y();			xADC();			break;
		case 0x72:		xINDIRECT();			xADC();			break;
		case 0x73:      C65C02_xILLEGAL();						break;
		case 0x74:      xZEROPAGE_X();			xSTZ();			break;
		case 0x75:      xZEROPAGE_X();			xADC();			break;
		case 0x76:		xZEROPAGE_X();			xROR();			break;
		case 0x77:      C65C02_xILLEGAL();						break;
		case 0x78:								xSEI();			break;
		case 0x79:      xABSOLUTE_Y();			xADC();			break;
		case 0x7A:								xPLY();			break;
		case 0x7B:		C65C02_xILLEGAL();						break;
		case 0x7C:		xINDIRECT_ABSOLUTE_X();	xJMP();			break;
		case 0x7D:		xABSOLUTE_X();			xADC();			break;
		case 0x7E:		xABSOLUTE_X();			xROR();			break;
		case 0x7F:		C65C02_xILLEGAL();						break;

		case 0x80:								xBRA();			break;
		case 0x81:		xINDIRECT_X();			xSTA();			break;
		case 0x82:		C65C02_xILLEGAL();						break;
		case 0x83:		C65C02_xILLEGAL();						break;
		case 0x84:		xZEROPAGE();			xSTY();			break;
		case 0x85:      xZEROPAGE();			xSTA();			break;
		case 0x86:		xZEROPAGE();			xSTX();			break;
		case 0x87:		C65C02_xILLEGAL();						break;
		case 0x88:		xDEY();									break;
		case 0x89:		xIMMEDIATE();			xBIT();			break;
		case 0x8A:								xTXA();			break;
		case 0x8B:		C65C02_xILLEGAL();						break;
		case 0x8C:		xABSOLUTE();			xSTY();			break;
		case 0x8D:		xABSOLUTE();			xSTA();			break;
		case 0x8E:		xABSOLUTE();			xSTX();			break;
		case 0x8F:		C65C02_xILLEGAL();						break;

		case 0x90:								xBCC();			break;
		case 0x91:		xINDIRECT_Y();			xSTA();			break;
		case 0x92:		xINDIRECT();			xSTA();			break;
		case 0x93:		C65C02_xILLEGAL();						break;
		case 0x94:		xZEROPAGE_X();			xSTY();			break;
		case 0x95:		xZEROPAGE_X();			xSTA();			break;
		case 0x96:		xZEROPAGE_Y();			xSTX();			break;
		case 0x97:		C65C02_xILLEGAL();						break;
		case 0x98:								xTYA();			break;
		case 0x99:		xABSOLUTE_Y();			xSTA();			break;
		case 0x9A:								xTXS();			break;
		case 0x9B:		C65C02_xILLEGAL();						break;
		case 0x9C:		xABSOLUTE();			xSTZ();			break;
		case 0x9D:      xABSOLUTE_X();			xSTA();			break;
		case 0x9E:		xABSOLUTE_X();			xSTZ();			break;
		case 0x9F:		C65C02_xILLEGAL();						break;

		case 0xA0:		xIMMEDIATE();			xLDY();			break;
		case 0xA1:		xINDIRECT_X();			xLDA();			break;
		case 0xA2:		xIMMEDIATE();			xLDX();			break;
		case 0xA3:      C65C02_xILLEGAL();						break;
		case 0xA4:		xZEROPAGE();			xLDY();			break;
		case 0xA5:		xZEROPAGE();			xLDA();			break;
		case 0xA6:		xZEROPAGE();			xLDX();			break;
		case 0xA7:		C65C02_xILLEGAL();						break;
		case 0xA8:								xTAY();			break;
		case 0xA9:		xIMMEDIATE();			xLDA();			break;
		case 0xAA:								xTAX();			break;
		case 0xAB:		C65C02_xILLEGAL();						break;
		case 0xAC:		xABSOLUTE();			xLDY();			break;
		case 0xAD:		xABSOLUTE();			xLDA();			break;
		case 0xAE:		xABSOLUTE();			xLDX();			break;
		case 0xAF:		C65C02_xILLEGAL();						break;

		case 0xB0:								xBCS();			break;
		case 0xB1:		xINDIRECT_Y();			xLDA();			break;
		case 0xB2:		xINDIRECT();			xLDA();			break;
		case 0xB3:		C65C02_xILLEGAL();						break;
		case 0xB4:		xZEROPAGE_X();			xLDY();			break;
		case 0xB5:      xZEROPAGE_X();			xLDA();			break;
		case 0xB6:		xZEROPAGE_Y();			xLDX();			break;
		case 0xB7:		C65C02_xILLEGAL();						break;
		case 0xB8:								xCLV();			break;
		case 0xB9:      xABSOLUTE_Y();			xLDA();			break;
		case 0xBA:								xTSX();			break;
		case 0xBB:		C65C02_xILLEGAL();						break;
		case 0xBC:		xABSOLUTE_X();			xLDY();			break;
		case 0xBD:		xABSOLUTE_X();			xLDA();			break;
		case 0xBE:		xABSOLUTE_Y();			xLDX();			break;
		case 0xBF:		C65C02_xILLEGAL();						break;

		case 0xC0:		xIMMEDIATE();			xCPY();			break;
		case 0xC1:      xINDIRECT_X();			xCMP();			break;
		case 0xC2:      C65C02_xILLEGAL();						break;
		case 0xC3:      C65C02_xILLEGAL();						break;
		case 0xC4:      xZEROPAGE();			xCPY();			break;
		case 0xC5:      xZEROPAGE();			xCMP();			break;
		case 0xC6:      xZEROPAGE();			xDEC();			break;
		case 0xC7:      C65C02_xILLEGAL();						break;
		case 0xC8:								xINY();			break;
		case 0xC9:      xIMMEDIATE();			xCMP();			break;
		case 0xCA:								xDEX();			break;
		case 0xCB:								xWAI();			break;
		case 0xCC:      xABSOLUTE();			xCPY();			break;
		case 0xCD:      xABSOLUTE();			xCMP();			break;
		case 0xCE:      xABSOLUTE();			xDEC();			break;
		case 0xCF:      C65C02_xILLEGAL();						break;

		case 0xD0:								xBNE();			break;
		case 0xD1:		xINDIRECT_Y();			xCMP();			break;
		case 0xD2:		xINDIRECT();			xCMP();			break;
		case 0xD3:		C65C02_xILLEGAL();						break;
		case 0xD4:		C65C02_xILLEGAL();						break;
		case 0xD5:		xZEROPAGE_X();			xCMP();			break;
		case 0xD6:		xZEROPAGE_X();			xDEC();			break;
		case 0xD7:		C65C02_xILLEGAL();						break;
		case 0xD8:								xCLD();			break;
		case 0xD9:		xABSOLUTE_Y();			xCMP();			break;
		case 0xDA:								xPHX();			break;
		case 0xDB:								xSTP();			break;
		case 0xDC:		C65C02_xILLEGAL();						break;
		case 0xDD:		xABSOLUTE_X();			xCMP();			break;
		case 0xDE:		xABSOLUTE_X();			xDEC();			break;
		case 0xDF:		C65C02_xILLEGAL();						break;

		case 0xE0:		xIMMEDIATE();			xCPX();			break;
		case 0xE1:		xINDIRECT_X();			xSBC();			break;
		case 0xE2:		C65C02_xILLEGAL();						break;
		case 0xE3:		C65C02_xILLEGAL();						break;
		case 0xE4:		xZEROPAGE();			xCPX();			break;
		case 0xE5:		xZEROPAGE();			xSBC();			break;
		case 0xE6:		xZEROPAGE();			xINC();			break;
		case 0xE7:		C65C02_xILLEGAL();						break;
		case 0xE8:								xINX();			break;
		case 0xE9:		xIMMEDIATE();			xSBC();			break;
		case 0xEA:								xNOP();			break;
		case 0xEB:		C65C02_xILLEGAL();						break;
		case 0xEC:		xABSOLUTE();			xCPX();			break;
		case 0xED:		xABSOLUTE();			xSBC();			break;
		case 0xEE:		xABSOLUTE();			xINC();			break;
		case 0xEF:		C65C02_xILLEGAL();						break;

		case 0xF0:								xBEQ();			break;
		case 0xF1:		xINDIRECT_Y();			xSBC();			break;
		case 0xF2:		xINDIRECT();			xSBC();			break;
		case 0xF3:		C65C02_xILLEGAL();						break;
		case 0xF4:		C65C02_xILLEGAL();						break;
		case 0xF5:		xZEROPAGE_X();			xSBC();			break;
		case 0xF6:		xZEROPAGE_X();			xINC();			break;
		case 0xF7:		C65C02_xILLEGAL();						break;
		case 0xF8:								xSED();			break;
		case 0xF9:		xABSOLUTE_Y();			xSBC();			break;
		case 0xFA:								xPLX();			break;
		case 0xFB:		C65C02_xILLEGAL();						break;
		case 0xFC:		C65C02_xILLEGAL();						break;
		case 0xFD:		xABSOLUTE_X();			xSBC();			break;
		case 0xFE:		xABSOLUTE_X();			xINC();			break;
		case 0xFF:		C65C02_xILLEGAL();						break;
		}
	}
}

int C65C02_GetPC(void) { return mPC; }

void C65C02_xILLEGAL(void)
{

}

// Answers value of the Processor Status register
int C65C02_PS0()
{
	u8 ps = 0x20;
	if(mN) ps|=0x80;
	if(mV) ps|=0x40;
	if(mB) ps|=0x10;
	if(mD) ps|=0x08;
	if(mI) ps|=0x04;
	if(mZ) ps|=0x02;
	if(mC) ps|=0x01;
	return ps;
}


// Change the processor flags to correspond to the given value
void C65C02_PS1(int ps)
{
	mN=ps&0x80;
	mV=ps&0x40;
	mB=ps&0x10;
	mD=ps&0x08;
	mI=ps&0x04;
	mZ=ps&0x02;
	mC=ps&0x01;
}

