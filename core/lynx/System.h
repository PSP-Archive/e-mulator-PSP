#ifndef _SYSTEM_H_
#define _SYSTEM_H_


#define CPU_RDWR_CYC	5
#define DMA_RDWR_CYC	4
#define SPR_RDWR_CYC	3
typedef enum  {bank0,bank1,ram,cpu} EMMODE;

//#include "susie.h"
#include "c65c02.h"

//extern u8	pLynxBios[0x200];

#define RAM_SIZE				0x10000
#define RAM_ADDR_MASK			0xffff
#define DEFAULT_RAM_CONTENTS	0xff

typedef struct
{
   u16   jump;
   u16   load_address;
   u16   size;
   u8   magic[4];
}HOME_HEADER;

#define ROM_SIZE				0x200
#define ROM_ADDR_MASK			0x01ff
#define DEFAULT_ROM_CONTENTS	0x88

#define BROM_START		0xfe00
#define BROM_SIZE		0x200
#define VECTOR_START	0xfffa
#define VECTOR_SIZE		0x6

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
#define MEMMAP_SIZE				0x1


#define HANDY_SYSTEM_FREQ						16000000
#define HANDY_TIMER_FREQ						20
#define HANDY_AUDIO_SAMPLE_FREQ					22050
#define HANDY_AUDIO_SAMPLE_PERIOD				(HANDY_SYSTEM_FREQ/HANDY_AUDIO_SAMPLE_FREQ)
#define HANDY_AUDIO_WAVESHAPER_TABLE_LENGTH		0x200000

#ifdef LINUX_PATCH
#define HANDY_AUDIO_BUFFER_SIZE					4096	// Needed forSDL
#else
#define HANDY_AUDIO_BUFFER_SIZE					(HANDY_AUDIO_SAMPLE_FREQ/4)
#endif


#define HANDY_FILETYPE_LNX		0
#define HANDY_FILETYPE_HOMEBREW	1
#define HANDY_FILETYPE_SNAPSHOT	2
#define HANDY_FILETYPE_ILLEGAL	3

//#define HANDY_SCREEN_WIDTH	512
//#define HANDY_SCREEN_HEIGHT	102
//
// Define the global variable list
//

#ifdef SYSTEM_CPP
	u32	gSystemCycleCount=0;
	u32	gSystemIRQ=FALSE;
	u8	gAudioBuffer[HANDY_AUDIO_BUFFER_SIZE];
	u32	gAudioBufferPointer=0;
	u32	gAudioLastUpdateCycle=0;
#else
	extern u32	gSystemCycleCount;
	extern u32	gSystemIRQ;
	extern u8	gAudioBuffer[HANDY_AUDIO_BUFFER_SIZE];
	extern u32	gAudioBufferPointer;
	extern u32	gAudioLastUpdateCycle;
#endif

//typedef struct lssfile{
//	u8 *memptr;
//	u32 index;
//	u32 index_limit;
//} LSS_FILE;
//
//int lss_read(void* dest,int varsize, int varcount,LSS_FILE *fp);

//
// Now pull in the parts that build the system
//
#define TOP_START	0xfc00
#define TOP_MASK	0x03ff
#define TOP_SIZE	0x400
#define SYSTEM_SIZE	65536

#define LSS_VERSION_OLD	"LSS2"
#define LSS_VERSION	"LSS3"

//extern		u32			mFileType;
//extern		u32			mCycleCountBreakpoint;


typedef u8   (*pPEEK_FUNC)(u32 addr);
typedef void (*pPOKE_FUNC)(u32 addr,u8 data);

typedef struct {
	pPEEK_FUNC  pPeek;
	pPOKE_FUNC  pPoke;
} MEMORY_HANDLER;

extern MEMORY_HANDLER mMemoryHandlers[SYSTEM_SIZE];
extern u8 mRam_Data[RAM_SIZE];

//extern  int mMemoryHandlers[SYSTEM_SIZE];

#define DEFAULT_CART_CONTENTS	0x11

typedef enum  {UNUSED,C64K,C128K,C256K,C512K,C1024K} CTYPE;

#define CART_NO_ROTATE		0
#define CART_ROTATE_LEFT	1
#define	CART_ROTATE_RIGHT	2

typedef struct {
   u8   magic[4];
   u16   page_size_bank[2];
   u16   version;
   u8   cartname[32];
   u8   manufname[16];
   u8   rotation; 
   u8   spare[5];
} LYNX_HEADER;


//class CSystem;

#define MIKIE_START	0xfd00
#define MIKIE_SIZE	0x100

//
// Define counter types and defines
//

#define CTRL_A_IRQEN	0x80
#define CTRL_A_RTD		0x40
#define CTRL_A_RELOAD	0x10
#define CTRL_A_COUNT	0x08
#define CTRL_A_DIVIDE	0x07

#define CTRL_B_TDONE	0x08
#define CTRL_B_LASTCK	0x04
#define CTRL_B_CIN		0x02
#define CTRL_B_COUT		0x01

#define LINE_TIMER		0x00
#define SCREEN_TIMER	0x02

#define LINE_WIDTH		256
#define	LINE_SIZE		80

#define UART_TX_INACTIVE	0x80000000
//#define UART_RX_INACTIVE	0x80000000
#define UART_BREAK_CODE		0x00008000
//#define	UART_MAX_RX_QUEUE	32
#define UART_TX_TIME_PERIOD	(11)
//#define UART_RX_TIME_PERIOD	(11)
//#define UART_RX_NEXT_DELAY	(44)

//typedef struct {
//	u8	backup;
//	u8	count;
//	u8	controlA;
//	u8	controlB;
//	u32	linkedlastcarry;
//} MTIMER;

/*typedef struct {
	union {
		struct {
#ifdef MSB_FIRST
			u8 unused:4;
			u8 Colour:1;
			u8 FourColour:1;
			u8 Flip:1;
			u8 DMAEnable:1;
#else
			u8 DMAEnable:1;
			u8 Flip:1;
			u8 FourColour:1;
			u8 Colour:1;
			u8 unused:4;
#endif
		}Bits;
		u8 Byte;
	};
}TDISPCTL;*/


//
// Emumerated types for possible mikie windows independant modes
//
enum {
	MIKIE_BAD_MODE=0,
	MIKIE_NO_ROTATE,
	MIKIE_ROTATE_L,
	MIKIE_ROTATE_R
};


		

#define SUSIE_START		0xfc00
#define SUSIE_SIZE		0x100
#define SCREEN_WIDTH	160
#define SCREEN_HEIGHT	102
#define LINE_END		0x80

//
// Define button values
//
#define BUTTON_A		0x0001
#define BUTTON_B		0x0002
#define BUTTON_OPT2		0x0004
#define BUTTON_OPT1		0x0008
#define BUTTON_LEFT		0x0010
#define BUTTON_RIGHT	0x0020
#define BUTTON_UP		0x0040
#define BUTTON_DOWN		0x0080
#define BUTTON_PAUSE	0x0100


enum {line_error=0,line_abs_literal,line_literal,line_packed};
enum {math_finished=0,math_divide,math_multiply,math_init_divide,math_init_multiply};

enum {sprite_background_shadow=0,
      sprite_background_noncollide,
      sprite_boundary_shadow,
      sprite_boundary,
	  sprite_normal,
	  sprite_noncollide,
	  sprite_xor_shadow,
	  sprite_shadow};

// Define register typdefs

typedef struct {
	union	{
		struct		{
#ifdef MSB_FIRST
			u8	High,Low;
#else
			u8	Low,High;
#endif
		}Byte;
		u16	Word;
	};
}UUWORD;


typedef struct{
	union	{
		struct		{
#ifdef MSB_FIRST
			u8	Fc1:1;
			u8	Fc2:1;
			u8	Fc3:1;
			u8	reserved:1;
			u8	Ac1:1;
			u8	Ac2:1;
			u8	Ac3:1;
			u8	Ac4:1;
#else
			u8	Ac4:1;
			u8	Ac3:1;
			u8	Ac2:1;
			u8	Ac1:1;
			u8	reserved:1;
			u8	Fc3:1;
			u8	Fc2:1;
			u8	Fc1:1;
#endif
		}Bits;
		u8	Byte;
	};
}TSPRINIT;

/*
typedef struct{
	union	{
		struct		{
#ifdef MSB_FIRST
			u8	Up:1;
			u8	Down:1;
			u8	Left:1;
			u8	Right:1;
			u8	Option1:1;
			u8	Option2:1;
			u8	Inside:1;
			u8	Outside:1;
#else
			u8	Outside:1;
			u8	Inside:1;
			u8	Option2:1;
			u8	Option1:1;
			u8	Right:1;
			u8	Left:1;
			u8	Down:1;
			u8	Up:1;
#endif
		}Bits;
		u8	Byte;
	};
}TJOYSTICK;*/
/*
typedef struct{
	union	{
		struct		{
#ifdef MSB_FIRST
			u8	spare:5;
			u8	Cart1IO:1;
			u8	Cart0IO:1;
			u8	Pause:1;
#else
			u8	Pause:1;
			u8	Cart0IO:1;
			u8	Cart1IO:1;
			u8	spare:5;
#endif
		}Bits;
		u8	Byte;
	};
}TSWITCHES;*/

typedef struct{
	union	{
		struct		{
#ifdef MSB_FIRST
			u8	A,B,C,D;
#else
			u8	D,C,B,A;
#endif
		}Bytes;
		struct		{
#ifdef MSB_FIRST
			u16	AB,CD;
#else
			u16	CD,AB;
#endif
		}Words;
		u32	Long;
	};
}TMATHABCD;

typedef struct{
	union	{
		struct		{
#ifdef MSB_FIRST
			u8	E,F,G,H;
#else
			u8	H,G,F,E;
#endif
		}Bytes;
		struct		{
#ifdef MSB_FIRST
			u16	EF,GH;
#else
			u16	GH,EF;
#endif
		}Words;
		u32	Long;
	};
}TMATHEFGH;

typedef struct {
	union {
		struct {
#ifdef MSB_FIRST
			u8	J,K,L,M;
#else
			u8	M,L,K,J;
#endif
		}Bytes;
		struct {
#ifdef MSB_FIRST
			u16	JK,LM;
#else
			u16	LM,JK;
#endif
		}Words;
		u32	Long;
	};
}TMATHJKLM;

typedef struct {
	union {
		struct {
#ifdef MSB_FIRST
			u8	xx2,xx1,N,P;
#else
			u8	P,N,xx1,xx2;
#endif
		}Bytes;
		struct {
#ifdef MSB_FIRST
			u16	xx1,NP;
#else
			u16	NP,xx1;
#endif
		}Words;
		u32	Long;
	};
}TMATHNP;



#define DEF_MEMB16(name)  union { struct { u8 m##name##_L,m##name##_H; }; u16 m##name; }

typedef struct {
    union {
        u8 data[0x100];
        
        struct {
            // 0x00-0x0F
            DEF_MEMB16(TMPADR);    // 00,01
            DEF_MEMB16(TILTACUM);  // 02,03
            DEF_MEMB16(HOFF);      // 04,05
            DEF_MEMB16(VOFF);      // 06,07
            DEF_MEMB16(VIDBAS);    // 08,09
            DEF_MEMB16(COLLBAS);   // 0a,0b
            DEF_MEMB16(VIDADR);    // 0c,0d
            DEF_MEMB16(COLLADR);  // 0e,0f

            // 0x10-0x1F
            DEF_MEMB16(SCBNEXT);   // 10,11
            DEF_MEMB16(SPRDLINE);  // 12,13
            DEF_MEMB16(HPOSSTRT);  // 14,15
            DEF_MEMB16(VPOSSTRT);  // 16,17
            DEF_MEMB16(SPRHSIZ);   // 18,19
            DEF_MEMB16(SPRVSIZ);   // 1a,1b
            DEF_MEMB16(STRETCH);   // 1c,1d
            DEF_MEMB16(TILT);      // 1e,1f

            // 0x20-0x2F
            DEF_MEMB16(SPRDOFF);   // 20,21
            DEF_MEMB16(SPRVPOS);   // 22,23
            DEF_MEMB16(COLLOFF);   // 24,25
            DEF_MEMB16(VSIZACUM);  // 26,27
            DEF_MEMB16(HSIZOFF);   // 28,29
            DEF_MEMB16(VSIZOFF);   // 2a,2b
            DEF_MEMB16(SCBADR);    // 2c,2d
            DEF_MEMB16(PROCADR);   // 2e,2f

            // 0x30-0x3F
            u8 r0[0x10];           // 30-3f
            // 0x40-0x4F
            u8 r1[0x10];           // 40-4f

            // 0x50-0x5F
            u8 r2,r3;              // 50,51
            u8 mMATHD;             // 52
            u8 mMATHC;             // 53
            u8 mMATHB;             // 54
            u8 mMATHA;             // 55
            u8 mMATHP;             // 56
            u8 mMATHN;             // 57
            u8 r4[8];              // 58-5f

            // 0x60-0x6F
            u8 mMATHH;             // 60
            u8 mMATHG;             // 61
            u8 mMATHF;             // 62
            u8 mMATHE;             // 63
            u8 r5[8];              // 64-6b
            u8 mMATHM;             // 6c
            u8 mMATHL;             // 6d
            u8 mMATHK;             // 6e
            u8 mMATHJ;             // 6f

            // 0x70-0x7F
            u8 r6[0x10];           // 70-7f

            // 0x80-0x8F
            u8 mSPRCTL0;           // 80
            u8 mSPRCTL1;           // 81
            u8 mSPRCOLL;           // 82
            u8 mSPRINIT;           // 83
            u8 mr84[4];            // 84-87

            u8 mSUZYHREV;          // 88
            u8 mSUZYSREV;          // 89
            u8 mr8a[6];            // 8a-8f

            // 0x90-0x9F
            u8 mSUZYBUSEN;         // 90
            u8 mSPRGO;             // 91
            u8 mSPRSYS;            // 92
            u8 u93[13];            // 93-9f

            // 0xA0-0xAF
            u8 ra[0x10];           // a0-af

            // 0xB0-0xBF
            u8 mJOYSTICK;          // b0
            u8 mSWITCHES;         // b1
            u8 mPCART0;            // b2
            u8 mPCART1;            // b3
            u8 rb[12];             // b4-bf

            // 0xC0-0xCF
            u8 mLEDS;              // c0
            u8 rc1;                // c1
            u8 mPPORTSTAT;         // c2
            u8 mPPORTDATA;         // c3
            u8 mHOWIE;             // c4
            u8 rc5[11];            // c5-cf
            
            // 0xD0-0xDF
            u8 rd[0x10];           // d0-df
            // 0xE0-0xEF
            u8 re[0x10];           // e0-ef
            // 0xF0-0xFF
            u8 rf[0x10];           // f0-ff
        };
    };
} SUSIE_REGISTER;

// Hardware storage
//typedef struct {
//	u32	tBORROW_IN;
//	u32	tBORROW_OUT;
//} TIM;


#define TIM_ENABLE_COUNT(n)  (mikie.mTIM[(n)].CTLA&0x08)
#define TIM_ENABLE_RELOAD(n) (mikie.mTIM[(n)].CTLA&0x10)
#define TIM_LINKING(n)       (mikie.mTIM[(n)].CTLA&0x07)

#define TIM_TIMER_DONE(n)    (mikie.mTIM[(n)].CTLB&0x08)
#define TIM_LAST_CLOCK(n)    (mikie.mTIM[(n)].CTLB&0x04)
#define TIM_BORROW_IN(n)     (mikie.mTIM[(n)].CTLB&0x02)
#define TIM_BORROW_OUT(n)    (mikie.mTIM[(n)].CTLB&0x01)

#define SET_BORROW_IN(n)   {mikie.mTIM[n].CTLB|=0x02;}
#define SET_BORROW_OUT(n)  {mikie.mTIM[n].CTLB|=0x01;}
#define CLR_BORROW_IN(n)   {mikie.mTIM[n].CTLB&=~0x02;}
#define CLR_BORROW_OUT(n)  {mikie.mTIM[n].CTLB&=~0x01;}

typedef struct {
	u32		BKUP;
	u32		ENABLE_RELOAD;
	u32		ENABLE_COUNT;
	u32		LINKING;
	u32		CURRENT;
	u32		TIMER_DONE;
	u32		LAST_CLOCK;
	u32		BORROW_IN;
	u32		BORROW_OUT;
	u32		LAST_LINK_CARRY;
	u32		LAST_COUNT;
//	s8		VOLUME;
	s8		OUTPUT;
	u32		INTEGRATE_ENABLE;
	u32		WAVESHAPER;
} AUDIO;



typedef struct {
//	TIM     m_TIM[8];
	AUDIO   m_AUDIO[4];
//	u32   mSTEREO;

} MIKIE;

#define mSPRCTL0_Type           (  suzy.mSPRCTL0&0x07       )
#define mSPRCTL0_Vflip          (  suzy.mSPRCTL0&0x10       )
#define mSPRCTL0_Hflip          (  suzy.mSPRCTL0&0x20       )
#define mSPRCTL0_PixelBits      (((suzy.mSPRCTL0&0xc0)>>6)+1)

#define mSPRCTL1_StartLeft      ((suzy.mSPRCTL1&0x01))
#define mSPRCTL1_StartUp        ((suzy.mSPRCTL1&0x02))
#define	mSPRCTL1_SkipSprite     ((suzy.mSPRCTL1&0x04))
#define	mSPRCTL1_ReloadPalette  ((suzy.mSPRCTL1&0x08))
#define	mSPRCTL1_ReloadDepth    ((suzy.mSPRCTL1&0x30)>>4)
#define	mSPRCTL1_Sizing         ((suzy.mSPRCTL1&0x40))	
#define	mSPRCTL1_Literal        ((suzy.mSPRCTL1&0x80))

#define mSPRSYS_StopOnCurrent  (Susie.mSPRSYS & 0x02)
//#define mSPRSYS_UnsafeAccess   (Susie.mSPRSYS & 0x04)
#define mSPRSYS_LeftHand       (Susie.mSPRSYS & 0x08)
//#define mSPRSYS_VStretch       (Susie.mSPRSYS & 0x10)
//#define mSPRSYS_LastCarry      (Susie.mSPRSYS & 0x20)
//#define mSPRSYS_Mathbit        (Susie.mSPRSYS & 0x40)
//#define mSPRSYS_MathInProgress (Susie.mSPRSYS & 0x80)


typedef struct {
	BYTE        mSPRSYS;
//	int			mSPRSYS_StopOnCurrent;	//CPU
//	int			mSPRSYS_LeftHand;
	int			mSPRSYS_VStretch;
	int			mSPRSYS_UnsafeAccess;
	int			mSPRSYS_LastCarry;
	int			mSPRSYS_Mathbit;
	int			mSPRSYS_MathInProgress;


//	UUWORD		mTMPADR;		// ENG
//	UUWORD		mTILTACUM;		// ENG
//	UUWORD		mHOFF;			// CPU
//	UUWORD		mVOFF;			// CPU
//	UUWORD		mVIDBAS;		// CPU
//	UUWORD		mCOLLBAS;		// CPU
//	UUWORD		mVIDADR;		// ENG
//	UUWORD		mCOLLADR;		// ENG
//	UUWORD		mSCBNEXT;		// SCB
//	UUWORD		mSPRDLINE;		// SCB
//	UUWORD		mHPOSSTRT;		// SCB
//	UUWORD		mVPOSSTRT;		// SCB
//	UUWORD		mSPRHSIZ;		// SCB
//	UUWORD		mSPRVSIZ;		// SCB
//	UUWORD		mSTRETCH;		// ENG
//	UUWORD		mTILT;			// ENG
//	UUWORD		mSPRDOFF;		// ENG
//	UUWORD		mSPRVPOS;		// ENG
//	UUWORD		mCOLLOFF;		// CPU
//	UUWORD		mVSIZACUM;		// ENG
	UUWORD		mHSIZACUM;		//    K.s creation
//	UUWORD		mHSIZOFF;		// CPU
//	UUWORD		mVSIZOFF;		// CPU
//	UUWORD		mSCBADR;		// ENG
//	UUWORD		mPROCADR;		// ENG

	TMATHABCD	mMATHABCD;		// ENG
	TMATHEFGH	mMATHEFGH;		// ENG
	TMATHJKLM	mMATHJKLM;		// ENG
	TMATHNP		mMATHNP;		// ENG


	int			mMATHAB_sign;
	int			mMATHCD_sign;
	int			mMATHEFGH_sign;

//	BYTE        mSPRCTRL0;
//	BYTE        mSPRCTRL1;

//	int			mSPRCTL0_Type;			// SCB
//	int			mSPRCTL0_Vflip;
//	int			mSPRCTL0_Hflip;
//	int			mSPRCTL0_PixelBits;

//	int			mSPRCTL1_StartLeft;		// SCB
//	int			mSPRCTL1_StartUp;
//	int			mSPRCTL1_SkipSprite;
//	int			mSPRCTL1_ReloadPalette;
//	int			mSPRCTL1_ReloadDepth;
//	int			mSPRCTL1_Sizing;
//	int			mSPRCTL1_Literal;

	int			mSPRCOLL_Number;		//CPU
	int			mSPRCOLL_Collide;



	int			mSPRSYS_NoCollide;
	int			mSPRSYS_Accumulate;
	int			mSPRSYS_SignedMath;
	int			mSPRSYS_Status;

	u32		mSUZYBUSEN;		// CPU

	TSPRINIT	mSPRINIT;		// CPU

	u32		mSPRGO;			// CPU
	int			mEVERON;

	u8		mPenIndex[16];	// SCB

	// Line rendering related variables

	u32		mLineType;
	u32		mLineShiftRegCount;
	u32		mLineShiftReg;
	u32		mLineRepeatCount;
	u32		mLinePixel;
	u32		mLinePacketBitsLeft;

	int			mCollision;

//	u8*		mRamPointer;

	u32		mLineBaseAddress;
	u32		mLineCollisionAddress;

	// Joystick switches

//	TJOYSTICK	mJOYSTICK;
//	TSWITCHES	mSWITCHES;
} SUSIE;


#endif