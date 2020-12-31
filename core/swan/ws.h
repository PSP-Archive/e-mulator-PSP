#ifndef __WS_H__
#define __WS_H__

#define SWAN_INTERNAL_EEP "swan.eep"
#define SWAN_CONFIG_SRM   "swan.srm"

/*
enum {
	WS_SYSTEM_MONO			= 0,
	WS_SYSTEM_COLOR			= 1,
	WS_SYSTEM_AUTODETECT    = 2
};

enum {
	COLOUR_SCHEME_DEFAULT	= 0,
	COLOUR_SCHEME_AMBER		= 1,
	COLOUR_SCHEME_GREEN		= 2
};

 
enum { // KEYMAP
	WS_UP      = 0x1, WS_R_LEFT  = 0x1,
	WS_DOWN    = 0x4, WS_R_RIGHT = 0x4,
	WS_RIGHT   = 0x2, WS_R_UP    = 0x2,
	WS_LEFT    = 0x8, WS_R_DOWN  = 0x8,
	WS_START   = 0x2,
	WS_BT_A    = 0x4,
	WS_BT_B    = 0x8,
};

*/

void* nec_getRegPtr(int* pLen);
void  nec_reset(void*);
void  nec_setPipeline(int clk);
int   nec_getIF(void);

typedef struct ws_romHeaderStruct {
    u8	developperId;
    u8	minimumSupportSystem;
    u8	cartId;
    u8  undef_3;
    u8	romSize;
    u8	eepromSize;
    u8	additionnalCapabilities;
    u8	realtimeClock;
    u16	checksum;
} ws_romHeaderStruct;

typedef struct _window{
	u8 x0,y0,x1,y1;
} window;

typedef struct _scroll{
	u8 x,y;
} scroll;


//-------------------------------------------------------------------
// ïœçXÇÃÇ»Ç¢É}ÉNÉç
//-------------------------------------------------------------------
#define REPx8(a) a;a;a;a;a;a;a;a;
#define BETWEEN(j,min,max) ((min<=j) && (j<=max))

#define INT_HBLANK        0x80
#define INT_VBLANK        0x40
#define INT_VTIMER        0x20
#define INT_SCLINE        0x10

#define HBLANK_TIMER()   (pWS->ioRam[0xa2]&0x01)
#define HBLANK_AUTO()    (pWS->ioRam[0xa2]&0x02)
#define VBLANK_TIMER()   (pWS->ioRam[0xa2]&0x04)
#define VBLANK_AUTO()    (pWS->ioRam[0xa2]&0x08)

#ifdef WIN32
 static int is0mask = 0x07;
 #define IS_BGBG()     (pWS->ioRam[0x00]&is0mask&0x01)
 #define IS_BGFG()     (pWS->ioRam[0x00]&is0mask&0x02)
 #define IS_SPRT()     (pWS->ioRam[0x00]&is0mask&0x04)
 #define BGCOLORMOD(a) ((a)&0x7fff)
 #define BGBGMOD(a)    (((a))&0x7fff)
 #define BGFGMOD(a)    (((a))&0x7fff)
 #define BGWNMOD(a)    (((a))&0x7fff)
 #define SPMOD(a)      (((a))&0x7fff)
#else
 #define IS_BGBG()     (pWS->ioRam[0x00]&0x01)
 #define IS_BGFG()     (pWS->ioRam[0x00]&0x02)
 #define IS_SPRT()     (pWS->ioRam[0x00]&0x04)
 #define BGCOLORMOD(a) (a)
 #define BGBGMOD(a)    (a)
 #define BGFGMOD(a)    (a)
 #define BGWNMOD(a)    (a)
 #define SPMOD(a)      (a)
#endif

#define DRAW_MASK       0x8000
#define BGC_MASK(col)  (BGCOLORMOD(col))
#define BGBGMASK(col)  (BGBGMOD(col))
#define BGFGMASK(col)  (BGFGMOD(col)|DRAW_MASK)
#define BGWNMASK(col)  (BGWNMOD(col)|DRAW_MASK)
#define SPMASK(col)    (SPMOD(col))

#define CPU_CLOCK          30720000
#define PAL_TILE(tInfo)    (((tInfo)>>9)&0x0f)

#define IO_BACKUP_A4 0xf4
#define IO_BACKUP_A5 0xf5
#define IO_BACKUP_A6 0xf6
#define IO_BACKUP_A7 0xf7

#define SOUND_SamplesPerFrame 588 /* 44100/75=588 */
#define SWAN_SOUND_CHANNELS   4
#define WAVE_SAMPLES          32

#define IO_ROM_BANK_BASE_SELECTOR	0xC0

#endif
