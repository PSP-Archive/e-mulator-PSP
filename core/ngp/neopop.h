#define QMEM  // MEMORY OPTIMIZE
#define QPAL2
#define WINXW
#define NEGATIVE
//#define QSTORE


//---------------------------------------------------------------------------
// NEOPOP : Emulator as in Dreamland
//
// Copyright (c) 2001-2002 by neopop_uk
//---------------------------------------------------------------------------

#define min(a,b)   ((a<b)?a:b)

#ifndef __NEOPOP__
#define __NEOPOP__

//#include <stdio.h>
//#include <stdlib.h>
//#include <string.h>
//#include <stdarg.h>

#if defined(QMEM)

#define ROMMAP_UNIT  0x100 
#define ROMMAP_SIZE (0x1000000/ROMMAP_UNIT)

typedef struct {
  char* pNgpRomMap[ROMMAP_SIZE];
  char  pMapUndef[ROMMAP_UNIT];

  BYTE qpal2;

  PIXEL_FORMAT p_spr[64],n_spr[64];
  PIXEL_FORMAT p_sc1[64],n_sc1[64];
  PIXEL_FORMAT p_sc2[64],n_sc2[64];

} NGP_CACHE;

extern NGP_CACHE* ngp_c;

#if 0
#define RAM_SIZE   0x10000
#else
#define RAM_SIZE   0x0c000
#endif


typedef struct {
  int language_english;
  u8  ram[RAM_SIZE];
  u8  bios[0x10000];

  /* move from gfx.c */
  u8 winx,winy;
  u8 winw,winh;
#if defined(WINXW)
  u8 winxw;
#endif
  u8 scanline;
  u8 scroll1x  , scroll1y ;
  u8 scroll2x  , scroll2y ;
  u8 scrollsprx, scrollspry;
  u8 bgc,oowc,negative;
  u8 planeSwap;

  /* move from interrupt.c */
  unsigned int  timer_hint;
  u8 timer[4];	//Up-counters
  unsigned int  timer_clock0, timer_clock1, timer_clock2, timer_clock3;
  unsigned int  gfx_hack;

  /* move from dma.c */
  unsigned int   dmaS[4], dmaD[4];
  unsigned short dmaC[4];
  u8  dmaM[4];
  
  
  unsigned int c_cycle;
} NGP_MEM;

extern NGP_MEM* ngp_m;


#define ram           ngp_m->ram
#define bios          ngp_m->bios
#define winx          ngp_m->winx
#define winy          ngp_m->winy
#define winw          ngp_m->winw
#define winh          ngp_m->winh
#define winxw         ngp_m->winxw
#define scanline      ngp_m->scanline

#define scroll1x      ngp_m->scroll1x
#define scroll1y      ngp_m->scroll1y
#define scroll2x      ngp_m->scroll2x
#define scroll2y      ngp_m->scroll2y
#define scrollsprx    ngp_m->scrollsprx
#define scrollspry    ngp_m->scrollspry

#define negative      ngp_m->negative
#define bgc           ngp_m->bgc
#define oowc          ngp_m->oowc
#define planeSwap     ngp_m->planeSwap

#define timer_hint    ngp_m->timer_hint
#define timer         ngp_m->timer
#define timer_clock0  ngp_m->timer_clock0
#define timer_clock1  ngp_m->timer_clock1
#define timer_clock2  ngp_m->timer_clock2
#define timer_clock3  ngp_m->timer_clock3
#define gfx_hack      ngp_m->gfx_hack

#define dmaS          ngp_m->dmaS
#define dmaD          ngp_m->dmaD
#define dmaC          ngp_m->dmaC
#define dmaM          ngp_m->dmaM

#endif

#define PAL16(v)  ((((int)(v)<<1) & 0x001e) | (((int)(v)<<2) & 0x03c0) | (((int)(v)<<3) & 0x7800))
#define PAL16m(v) (int)((((int)(v))<<12)|(((int)(v))<<7)|(((int)(v))<<2))


//=============================================================================

//===========================
// GCC specific 
//===========================
#ifdef __GNUC__

	typedef unsigned long long	_u64;
	typedef long long			_s64;

#ifndef max
#define max(a,b) ((a)>(b)?(a):(b))
#endif
#ifndef min
#define min(a,b) ((a)<(b)?(a):(b))
#endif

//===========================
#endif


//===========================
// Visual C specific 
//===========================
#ifdef _MSC_VER

	typedef unsigned __int64	_u64;
	typedef signed __int64		_s64;

//===========================
#endif


//-----------------------------------------------------------------------------
// Global Definitions
//-----------------------------------------------------------------------------

/*
//TYPES
typedef unsigned char		u8;
typedef unsigned short		u16;
typedef unsigned int		u32;
typedef	signed char			s8;
typedef signed short		s16;
typedef signed int			s32;
*/  

//BOOL
typedef int					BOOL;
#define TRUE				1
#define FALSE				0

//COLOURMODE
typedef enum
{
	COLOURMODE_GREYSCALE,
	COLOURMODE_COLOUR,
	COLOURMODE_AUTO
}
COLOURMODE;

//RomInfo
typedef struct 
{
  u8* data;		//Pointer to the rom data
  u32 length;	//Length of the rom
  
  u8 name[16];	//Null terminated string, holding the Game name
  
  //For use as flash file and default state name
  u8 filename[256];	
}
RomInfo;

//RomHeader
typedef struct
{
  u8		licence[28];		// 0x00 - 0x1B
  u32	startPC;			// 0x1C - 0x1F
  u16	catalog;			// 0x20 - 0x21
  u8		subCatalog;			// 0x22
  u8		mode;				// 0x23
  u8		name[12];			// 0x24 - 0x2F
  
  u32	reserved1;			// 0x30 - 0x33
  u32	reserved2;			// 0x34 - 0x37
  u32	reserved3;			// 0x38 - 0x3B
  u32	reserved4;			// 0x3C - 0x3F
}
RomHeader;

//=============================================================================

//-----------------------------------------------------------------------------
// Core <--> System-Main Interface
//-----------------------------------------------------------------------------

void ngp_reset(void);

/* Fill the bios rom area with a bios. call once at program start */
BOOL bios_install(void);

#define RAM_START	0x000000
#define RAM_END		0x00BFFF

extern RomInfo rom;
extern RomHeader* rom_header;

//	// false = Japanese.
//	extern BOOL language_english;

/*!	Emulate a single instruction with correct TLCS900h:Z80 timing */

	void ngp_emulate(void);

/*! Call this function when a rom has just been loaded, it will perform
	the system independent actions required. */

	void rom_loaded(void);

/*!	Tidy up the rom and free the resources used. */

	void rom_unload(void);

//=========================================

	typedef enum 
	{
		IDS_DEFAULT,
		IDS_ROMFILTER,
		IDS_STATEFILTER,
		IDS_FLASHFILTER,
		IDS_BADFLASH,
		IDS_POWER,
		IDS_BADSTATE,
		IDS_ERROR1,		//Application init
		IDS_ERROR2,		//DirectDraw
		IDS_ERROR3,		//DirectInput
		IDS_TIMER,
		IDS_WRONGROM,
		IDS_EROMFIND,
		IDS_EROMOPEN,
		IDS_EZIPNONE,
		IDS_EZIPBAD,
		IDS_EZIPFIND,

		IDS_ABORT,
		IDS_DISCONNECT,
		IDS_CONNECTED,

		STRINGS_MAX,
	} 
	STRINGS;

/*! Get a string that may possibly be translated */

	char* system_get_string(STRINGS string_id);

/*! Used to generate a critical message for the user. After the message
	has been displayed, the function should return. The message is not
	necessarily a fatal error. */
	
//	void __cdecl system_message(char* vaMessage,...);

/*! Called at the start of the vertical blanking period, this function is
	designed to perform many of the critical hardware interface updates
	Here is a list of recommended actions to take:
	
	- The frame buffer should be copied to the screen.
	- The frame rate should be throttled to 59.95hz
	- The sound chips should be polled for the next chunk of data
	- Input should be polled and the current status written to "ram[0x6F82]" */
	
	void system_VBL(void);


//-----------------------------------------------------------------------------
// Core <--> System-Graphics Interface
//-----------------------------------------------------------------------------

	// Physical screen dimensions
#define SCREEN_WIDTH	160
#define SCREEN_HEIGHT	152


	//16-bit Frame buffer: Format X4B4G4R4
	extern u16 cfb[256*256]; 

	extern COLOURMODE system_colour;

/*! Increased in 'interrupt.c', never more than system_frameskip_key */

	extern u8 frameskip_count;

		//=========================================

/*! The system sets this value to the number of the frameskip keyframe */

//	extern u8 system_frameskip_key;

	
//-----------------------------------------------------------------------------
// Core <--> System-Sound Interface
//-----------------------------------------------------------------------------

	// Speed of DAC playback
#define DAC_FREQUENCY		8000 //hz

	extern BOOL mute;

/*!	Fills the given buffer with sound data */

	void sound_update(u16* pR,u16* pL,int length_bytes);
	void dac_update(u8* dac_buffer, int length_bytes);

/*! Initialises the sound chips using the given SampleRate */
	
	void sound_init(int SampleRate);

		//=========================================

/*! Callback for "sound_init" with the system sound frequency */
	
	void system_sound_chipreset(void);

/*! Clears the sound output. */
	
	void system_sound_silence(void);


//-----------------------------------------------------------------------------
// Core <--> System-IO Interface
//-----------------------------------------------------------------------------

	void state_restore(char* filename);
	void state_store(char* filename);

		//=========================================

/*! Reads a byte from the other system. If no data is available or no
	high-level communications have been established, then return FALSE.
	If buffer is NULL, then no data is read, only status is returned */

	int system_comms_read(u8* buffer);


/*! Peeks at any data from the other system. If no data is available or
	no high-level communications have been established, then return FALSE.
	If buffer is NULL, then no data is read, only status is returned */

	int system_comms_poll(u8* buffer);


/*! Writes a byte from the other system. This function should block until
	the data is written. USE RELIABLE COMMS! Data cannot be re-requested. */

	void system_comms_write(u8 data);


/*! Reads as much of the file specified by 'filename' into the given, 
	preallocated buffer. This is rom data */

	BOOL system_io_rom_read(char* filename, u8* buffer, u32 bufferLength);


/*! Reads the "appropriate" (system specific) flash data into the given
	preallocated buffer. The emulation core doesn't care where from. */

	BOOL system_io_flash_read(u8* buffer, u32 bufferLength);


/*! Writes the given flash data into an "appropriate" (system specific)
	place. The emulation core doesn't care where to. */

	BOOL system_io_flash_write(u8* buffer, u32 bufferLength);


/*! Reads from the file specified by 'filename' into the given preallocated
	buffer. This is state data. */

	BOOL system_io_state_read(char* filename, u8* buffer, u32 bufferLength);
	

/*! Writes to the file specified by 'filename' from the given buffer.
	This is state data. */

	BOOL system_io_state_write(char* filename, u8* buffer, u32 bufferLength);


//-----------------------------------------------------------------------------
// Core <--> System-Debugger Interface
//-----------------------------------------------------------------------------

#ifdef NEOPOP_DEBUG

	//Debugger message filters
	//(TRUE = allow messages to be generated)
	extern BOOL filter_mem;
	extern BOOL filter_bios;
	extern BOOL filter_comms;
	extern BOOL filter_dma;
	extern BOOL filter_sound;


/*! Emulate a single instruction in Debug mode, checking for exceptions */

	void emulate_debug(BOOL dis_TLCS900h, BOOL dis_Z80);


/*!	Disassembles a single instruction from $PC, as TLCS-900h or Z80
	according to whether it lies in the 0x7000 - 0x7FFF region. 
	$PC is incremented to the start of the next instruction. */
	
	char* disassemble(void);

		//=========================================

/*!	Generates a debugger specific message that is not relevant to the
	main build. For example the memory module (mem.c) uses this function
	to indicate a memory exception */

	void __cdecl system_debug_message(char* vaMessage,...);


/*! This function pairs with the function above. This is used to associate
	an additional address to the last debug message. It was added so that
	later the message could be selected and an appropriate address retrieved.
	For example a memory exception message might have the address of the
	instruction that caused the error, this way the instruction could be
	displayed, or used as a breakpoint, etc. */

	void system_debug_message_associate_address(u32 address);


/*!	Signals the debugger to stop execution of program code and await user
	instruction. This is called when resetting, or after a instruction or 
	memory error. */

	void system_debug_stop(void);


/*! Signals the debugger to refresh it's information as the system state
	has changed. For example after loading a saved state, a memory viewer
	will no longer be displaying current information. */

	void system_debug_refresh(void);
	

/*!	Indicates to the debugger that the instruction located at the current
	program counter value is about to be executed, and this address should
	be stored in the execution history. */

	void system_debug_history_add(void);
	

/*! Signals the debugger to clear it's list of instruction history. This
	is called by the reset function to correctly indicate that there have
	been no previous instructions executed. */

	void system_debug_history_clear(void);


/*! Clears the main debugger message display list */

	void system_debug_clear(void);

#endif

/*! Macros for big- and little-endian support (defaults to little-endian). */

#ifdef MSB_FIRST
#ifdef LSB_FIRST
#error Only define one of LSB_FIRST and MSB_FIRST
#endif

#ifndef le32toh
#define le32toh(l)	((((l)>>24) & 0xff) | (((l)>>8) & 0xff00) \
			 | (((l)<<8) & 0xff0000) | (((l)<<24) & 0xff000000))
#endif
#ifndef le16toh
#define le16toh(l)	((((l)>>8) & 0xff) | (((l)<<8) & 0xff00))
#endif
#else
#ifndef le32toh
#define le32toh(l)	(l)
#endif
#ifndef le16toh
#define le16toh(l)	(l)
#endif
#endif
#ifndef htole32
#define htole32	le32toh
#endif
#ifndef htole16
#define htole16	le16toh
#endif

//=============================================================================
#endif


extern FBFORMAT fb_format;

