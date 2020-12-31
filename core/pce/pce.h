// pce.h

#ifndef __PCE_H_DEFINED__
#define __PCE_H_DEFINED__

#include "M6502.h"

#define VDC1     0
#define VDC2     1

#define MAXDISP	 227
#define	VRR	     2
#define SCANLINES_PER_FRAME  263


typedef struct {
  u16 y;
  u16 x;
  u16 no;
  u16 atr;
} SPR;


enum _VDC_REG {
  MAWR, // 0x00
  MARR, // 0x01
  VWR,  // 0x02
  vdc3, // 0x03 
  vdc4, // 004
  CR,   // 05
  RCR,  // 06
  BXR,  // 07
  BYR,  // 08
  MWR,  // 09
  HSR,  // 0a
  HDR,  // 0b
  VPR,  // 0c
  VDW,  // 0d
  VCR,  // 0e
  DCR,  // 0f
  SOUR, // 10
  DISTR,// 11
  LENR, // 12
  SATB  // 13
};

#define	V_FLIP	0x8000
#define	H_FLIP	0x0800
#define	SPBG	0x0080
#define	CGX		0x0100

#define	NODATA	0xff
#define	ENABLE	1
#define	DISABLE	0

#define	VDC_CR      0x01
#define	VDC_OR      0x02
#define	VDC_RR      0x04
#define	VDC_DS      0x08
#define	VDC_DV      0x10
#define	VDC_VD      0x20
#define	VDC_BSY	    0x40

#define	VDC_SpHit	VDC_CR
#define	VDC_Over	VDC_OR
#define	VDC_RasHit	VDC_RR
#define	VDC_InVBlank VDC_VD
#define	VDC_DMAfinish	VDC_DV
#define	VDC_SATBfinish	VDC_DS

#define	SpHitON(v)     (io.vdcregs[v].VDC[CR].W&0x01)
#define	OverON(v)      (io.vdcregs[v].VDC[CR].W&0x02)
#define	VBlankON(v)    (io.vdcregs[v].VDC[CR].W&0x08)
#define	RasHitON(v)    (io.vdcregs[v].VDC[CR].W&0x04)
#define	SpriteON(v)    (io.vdcregs[v].VDC[CR].W&0x40)
#define	ScreenON(v)    (io.vdcregs[v].VDC[CR].W&0x80)
#define SATBIntON(v)   (io.vdcregs[v].VDC[DCR].W&0x01)
#define DMAIntON(v)    (io.vdcregs[v].VDC[DCR].W&0x02)
#define	ScrollX(v)      io.vdcregs[v].VDC[BXR].W
#define	ScrollY(v)      io.vdcregs[v].VDC[BYR].W

#define	IRQ2	1
#define	IRQ1	2
#define	TIRQ	4

#define	WIDTH  	512 //(352+64)
#define	HEIGHT	256

// pad allocation
// _|_              (4) (5) (6)
//  |   (SEL) (RUN) (3) (2) (1)
#define JOY_1       0x0001
#define JOY_2       0x0002
#define JOY_3       0x0100
#define JOY_4       0x0800
#define JOY_5       0x0400
#define JOY_6       0x0200
#define JOY_RUN     0x0008
#define	JOY_SELECT	0x0004
#define	JOY_UP		0x0010
#define	JOY_DOWN	0x0040
#define	JOY_LEFT	0x0080
#define	JOY_RIGHT	0x0020
#define JOY_SIX		0xf000


#define	VRAMSIZE	0x10000

typedef struct tagVDC{
  SPR  SPRAM[64];
  byte VRAM[VRAMSIZE];
  pair VDC[32];

  word inc;
  byte  satb_dma_counter;
  byte reg;
  byte status;
  byte satb;
  byte pendvsync;

  short bg_h;
  short bg_w;
  short screen_w;
  short screen_h;
  short scroll_y;
  short minline;
  short maxline;

  short ScrollYDiff;
  short oldScrollX;
  short oldScrollY;
  short oldScrollYDiff;
  short scroll;

} VDC_REG;


//-----------------------------------------
//
//-----------------------------------------
typedef struct tagIO {
  M6502  m6502;
  byte   RAM[0x8000];

  short scanline;
  short prevline;
  
  // VCE registers
  pair vce_reg;
  byte vce_cr;
  pair VCE[0x200];
  PIXEL_FORMAT Pal[512];

  // VPC registers
  byte vpc[8];

  // VDC registers
  VDC_REG vdcregs[2];

  // Joypad registers
  short JOY[16];
  byte shiftmode[16];
  byte joy_select,joy_counter;

  // PSG registers
  byte PSG[6][8];
  byte wave[6][32];
  byte wavofs[6];
  byte psg_ch;
  byte psg_volume;
  byte psg_lfo_freq;
  byte psg_lfo_ctrl;
  
  // TIMER registers
  byte timer_reload;
  byte timer_start;
  byte timer_counter;

  // IRQ registers
  byte irq_mask;
  byte irq_status;

//  // SAVE RAM
//  byte WRAM[0x2000];
} PCE_IO;

//
typedef struct tagACD {
  byte  ac_extra_mem[0x200000]; // 2MB
  DWORD ac_base[4];     /* base address for AC ram accessing */
  word  ac_offset[4];   /* offset address for AC ram accessing */
  word  ac_incr[4];     /* incrment value after read or write accordingly to the control bit */
  byte  ac_control[4];  /* bit 7: unused */
  DWORD ac_shift;
  byte  ac_shiftbits;   /* number of bits to shift by */
  byte  ac_unknown4;
} PCE_ACD;

//-----------------------------------------
//
//-----------------------------------------
typedef struct tagCD {
  // CDROM extention
  int backup;
  int adpcm_firstread;
  pair adpcm_ptr;
  word adpcm_rptr,adpcm_wptr;
  
  word adpcm_dmaptr; /* CAREFUL, added variable */
  byte adpcm_rate;   // CAREFUL, added variable
  DWORD adpcm_pptr; /* to know where to begin playing adpcm (in nibbles) */
  DWORD adpcm_psize; /* to know how many 4-bit samples to play */

  byte cd_port_1800;
  byte cd_port_1801;
  byte cd_port_1802;
  byte cd_port_1804;
  
  byte cd_sector_buffer[0x2000];
  byte cd_extra_mem[0x40000];
  byte *cd_read_buffer;

  DWORD pce_cd_read_datacnt;
  DWORD pce_cd_sectoraddy;
  byte cd_sectorcnt;
  byte new_adpcm_play;

  byte cd_buf[/*CD_BUF_LENGTH*/ 8*2048];
  byte PCM[0x10000];

} PCE_CD;


// 
// 
// 
typedef struct {
  byte         IOAREA[0x2000];
  unsigned int VRAM2[2][VRAMSIZE / sizeof(int)];
  unsigned int VRAMS[2][VRAMSIZE / sizeof(int)];
  byte         vchange[2][VRAMSIZE / 32];
  byte         vchanges[2][VRAMSIZE/128];
  byte         PopRAM[0x10000];
} PCE_CACHE;

DWORD  msf2nb_sect(byte min, byte sec, byte fra);
void   fill_cd_info();
void   nb_sect2msf(DWORD lsn,byte *min, byte *sec, byte *frm);

void pce_cd_read_sector (void);

typedef struct {
  DWORD offset;
  byte  new_val;
} PatchEntry;


typedef struct {
  DWORD StartTime;
  DWORD Duration;
  char data[32];
} SubtitleEntry;


typedef enum {
  HCD_SOURCE_REGULAR_FILE,
  HCD_SOURCE_CD_TRACK
} hcd_source_type;

typedef struct {
  byte beg_min;
  byte beg_sec;
  byte beg_fra;
  
  byte type;
  
  DWORD beg_lsn;
  DWORD length;
  
  hcd_source_type source_type;
  char filename[256];
  
  DWORD patch_number;
  DWORD subtitle_number;
  
  byte subtitle_synchro_type;
  
  PatchEntry *patch;
  SubtitleEntry *subtitle;
} Track;



//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
void ACD_init(void);
void ACD_write(word A,byte V);
byte ACD_read(word A);
void CD_init(void);
void CD_write(word A,byte V);
byte CD_read(word A);

#include "pceregs.h"

#endif

