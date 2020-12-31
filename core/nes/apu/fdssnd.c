
/*
** Nintendo FDS ExSound by TAKEDA, toshiya
**
** original: s_fds.c in nezp0922
*/

/* modifyed by stun 2000.10.24 */
/* bug fix  by stun 2000.11.27 */


#include "fdssnd.h"
#include "nes_apu.h"
#include "nes_extsound.h"
#include "cstring.h"


// virtuanes FDS plugin
#include "fdsplugin.h"

//#include <stdlib.h>
//#include <windows.h>
// abs function
#define abs(a) ((a>=0) ? a: -a)
//DCR
extern apu_t g_apu_t;


//DCR
// ----------------------------------------------------------------------------
// FDS Sound struct

typedef struct {
	u32 wave[0x40];
	u32 envspd;
	s32 envphase;
	u32 envout;
	u32 outlvl;

	u32 phase;
	u32 spd;
	u32 volume;
	s32 sweep;

	u8 enable;
	u8 envmode;
	u8 xxxxx;
	u8 xxxxx2;

	s32 timer;
	u32 last_spd;
} FDS_FMOP;

typedef struct FDSSOUND {
	u32 cps;
	s32 cycles;
	u32 mastervolume;
	s32 output;
	s32 fade;

	FDS_FMOP op[2];

	u32 waveaddr;
	u8 mute;
	u8 key;
	u8 reg[0x10];
	u8 reg_cur[0x10];
} FDSSOUND;


static FDSSOUND fds_sound;


static s32 FDSSoundRender(void)
{
	FDS_FMOP *pop;
	u32 vol;

	pop = &fds_sound.op[0];
	if(pop->timer>=0) pop->timer--;
	if(pop->timer==0)
	{
		fds_sound.op[1].sweep=pop->last_spd;
		pop->envmode=1;
	}
	if(pop->envmode==1)
	{
		fds_sound.op[1].spd = fds_sound.op[1].sweep;
	}
	else if(pop->envmode & 0x80)
	{
		u32 sweeps=(u32)fds_sound.op[1].sweep;
		pop->envphase++;
		if (!(pop->envmode & 0x40))
		{
			if((u32)pop->envphase==pop->envspd)
			{
				pop->envphase=0;
				if(fds_sound.op[1].spd <sweeps)
				{
					fds_sound.op[1].spd += pop->volume;
					if(fds_sound.op[1].spd >sweeps)
						fds_sound.op[1].spd=sweeps;
				}
			}
		}
		else
		{
			if((u32)pop->envphase==pop->envspd)
			{
				pop->envphase=0;
				if(fds_sound.op[1].spd > sweeps/2)
				{
					fds_sound.op[1].spd -= pop->volume;
					if(fds_sound.op[1].spd < sweeps)
						fds_sound.op[1].spd=sweeps;
				}
			}
		}
	}
	vol = pop->volume;
	if (pop->sweep)
	{
		vol += pop->sweep;
		if (vol < 0)
			vol = 0;
		else if (vol > 0x3f)
			vol = 0x3f;
	}
//	pop->envout = LinearToLog(vol);
	pop->envout = LinearToLog(0);
	pop = &fds_sound.op[1];
	{
		u32 vol;
		if (pop->envmode && fds_sound.fade)
		{
			pop->envphase -= fds_sound.cps >> (11 - 1);
			if (pop->envmode & 0x40)
				while (pop->envphase < 0)
				{
					pop->envphase += pop->envspd;
					pop->volume += (pop->volume < 0x1f);
				}
			else
				while (pop->envphase < 0)
				{
					pop->envphase += pop->envspd;
					pop->volume -= (pop->volume > 0x00);
				}
		}
		vol = pop->volume;
		pop->envout = LinearToLog(vol);
	}

	fds_sound.op[1].envout += fds_sound.mastervolume;

	fds_sound.cycles -= fds_sound.cps;
	while (fds_sound.cycles < 0)
	{
		fds_sound.cycles += 1 << 23;
		fds_sound.output = 0;
		for (pop = &fds_sound.op[0]; pop < &fds_sound.op[2]; pop++)
		{
			if (!pop->spd || !pop->enable)
			{
				fds_sound.output = 0;
				continue;
			}
			pop->phase += pop->spd + fds_sound.output;
			fds_sound.output = LogToLinear(pop->envout + pop->wave[(pop->phase >> (23 - 1)) & 0x3f], pop->outlvl);
		}
	}
	if (fds_sound.mute) return 0;
	return fds_sound.output;
}

static void FDSSoundVolume(u32 volume)
{
	fds_sound.mastervolume = (volume << (LOG_BITS - 8)) << 1;
}

static void FDSSoundWrite(u32 address, u8 value)
{
	if (0x4040 <= address && address <= 0x407F)
	{
		fds_sound.op[1].wave[address - 0x4040] = LinearToLog(((s32)value & 0x3f) - 0x20);
	}
	else if (0x4080 <= address && address <= 0x408F)
	{
		int ch = (address < 0x4084);
		FDS_FMOP *pop = &fds_sound.op[ch];
		fds_sound.reg[address - 0x4080] = value;
		switch (address & 15)
		{
			case 0:
				if (value & 0x80)
				{
					pop->volume = (value & 0x3f);
					pop->envmode = 0;
				}
				else
				{
					pop->envspd = ((value & 0x3f) + 1) << 23;
					pop->envmode = 0x80 | value;
				}
				break;
			case 4:
				if(value & 0x80)
				{
					s32 a=fds_sound.op[1].spd;
					s32 b=fds_sound.op[1].sweep;
					pop->timer=(0x3f-(value & 0x3f)) << 10;
					if(pop->timer==0) pop->timer=1;
					pop->last_spd=a*(0x3f-(value & 0x3f))/0x3f+
								  b*(value & 0x3f)/0x3f;
				}
				else if(fds_sound.op[1].sweep)
				{
					pop->envspd = (value & 0x3f) << 5;
					if((value & 0x3f)==0) pop->envspd=1;
					pop->envphase = 0;
					pop->envmode = 0x80 | (value & 0x40) ;
					pop->volume=abs(fds_sound.op[1].sweep - fds_sound.op[1].spd);
					pop->volume/=pop->envspd;
					if((value & 0x3f)==0) pop->envmode=1;
				}
				fds_sound.waveaddr = 0;
				break;
			case 1:
				if ((value & 0x7f) < 0x60)
					fds_sound.op[0].sweep = value & 0x7f;
				else
					fds_sound.op[0].sweep = ((s32)value & 0x7f) - 0x80;
				break;
			case 5:
				if (!value) break;
				if ((value & 0x7f) < 0x60)
				{
					fds_sound.op[1].sweep = (s32)fds_sound.op[1].spd+
						((fds_sound.op[1].spd * (value & 0x7f))>>5);
				}
				else
				{
					fds_sound.op[1].sweep = (s32)fds_sound.op[1].spd-
						(((fds_sound.op[1].spd) * (((s32)value & 0x7f) - 0x80)) >> 5);
				}
				break;
			case 2:
				pop->spd &= 0x00000F00 << 7;
				pop->spd |= (value & 0xFF) << 7;
				fds_sound.op[0].envmode = 0;
				fds_sound.op[0].timer=0;
				break;
			case 6:
				pop->spd &= 0x00000F00 << 7;
				pop->spd |= (value & 0xFF) << 7;
				pop->envmode = 0;
				break;
			case 3:
				pop->spd &= 0x000000FF << 7;
				pop->spd |= (value & 0x0F) << (7 + 8);
				pop->enable = !(value & 0x80);
				fds_sound.op[0].envmode = 0;
				fds_sound.op[0].timer=0;
				break;
			case 7:
				pop->spd &= 0x000000FF << 7;
				pop->spd |= (value & 0x0F) << (7 + 8);
				pop->enable = !(value & 0x80);
				fds_sound.waveaddr = 0;
				break;
			case 8:
				{
					static s8 lfotbl[8] = { 0,1,2,3,-4,-3,-2,-1 };
					u32 v = LinearToLog(lfotbl[value & 7]);
					fds_sound.op[0].wave[fds_sound.waveaddr++] = v;
					fds_sound.op[0].wave[fds_sound.waveaddr++] = v;
					if (fds_sound.waveaddr == 0x40)
					{
						fds_sound.waveaddr = 0;
					}
				}
				break;
			case 9:
				fds_sound.op[0].outlvl = LOG_LIN_BITS - LIN_BITS - LIN_BITS - 10 - (value & 3);
				if(value & 0x80) fds_sound.mute=1;
				else             fds_sound.mute=0;
				break;
			case 10:
				fds_sound.op[1].outlvl = LOG_LIN_BITS - LIN_BITS - LIN_BITS - 10 - (value & 3);
				if(value & 0x80) fds_sound.fade=1;
				else             fds_sound.fade=0;
				break;
		}
	}
}

static void FDSSoundWriteCurrent(u32 address, u8 value)
{
	if (0x4080 <= address && address <= 0x408F)
	{
		fds_sound.reg_cur[address - 0x4080] = value;
	}
}

static u8 FDSSoundRead(u32 address)
{
	if (0x4090 <= address && address <= 0x409F)
	{
		return fds_sound.reg_cur[address - 0x4090];
	}
	return 0;
}

static void FDSSoundReset(void)
{
	s8 i;
	FDS_FMOP *pop;
	core_memset(&fds_sound, 0, sizeof(FDSSOUND));
	fds_sound.cps = DivFix(NES_BASECYCLES, 12 * (1 << 1) * SAMPLE_RATE, 23);
	for (pop = &fds_sound.op[0]; pop < &fds_sound.op[2]; pop++)
	{
		pop->enable = 1;
	}
	fds_sound.op[0].outlvl = LOG_LIN_BITS - LIN_BITS - LIN_BITS - 10;
	fds_sound.op[1].outlvl = LOG_LIN_BITS - LIN_BITS - LIN_BITS - 10;
	for (i = 0; i < 0x40; i++)
	{
		fds_sound.op[1].wave[i] = LinearToLog((i < 0x20)?0x1f:-0x20);
	}
}



//static HMODULE	hVirtuaNES_FDS_DLL=NULL;
//static void		*hVirtuaNES_FDS=NULL;

static FDSRESET FDS_Reset;
static FDSWRITE FDS_Write;
static FDSREAD FDS_Read;
static FDSWRITESYNC FDS_WriteSync;
static FDSSETUP FDS_Setup;
static FDSPROCESS FDS_Process;



static void fds_snd_shutdown(void){
}

/*
static void fds_dll_snd_shutdown(void){
	if(hVirtuaNES_FDS_DLL){
		if(hVirtuaNES_FDS){
			FDSCLOSE FDS_Close;
			if(FDS_Close = (FDSCLOSE)GetProcAddress(hVirtuaNES_FDS_DLL, (LPCSTR)"FDS_Close")){
				FDS_Close(hVirtuaNES_FDS);
				hVirtuaNES_FDS = NULL;
			}
		}
		FreeLibrary(hVirtuaNES_FDS_DLL);
		hVirtuaNES_FDS_DLL = NULL;
	}
}
*/

static void fds_snd_reset(void){
	FDSSoundReset();
	FDSSoundVolume(1);
}

/*
static void fds_dll_snd_reset(void){
	if(hVirtuaNES_FDS_DLL){
		if(hVirtuaNES_FDS){
			if(FDS_Reset != NULL){
				FDS_Reset(hVirtuaNES_FDS, g_apu_t.sample_rate);
			}
		}
	}
}
*/

static int fds_snd_init(void){
	FDSSoundReset();
	FDSSoundVolume(1);
	return 1;
}

/*
static int fds_dll_snd_init(void){
	HMODULE hModule;
	void *hFDS;
	if(hModule=LoadLibrary("fdsplugin")){
		FDSCREATE FDS_Create;
		FDS_Create = (FDSCREATE)GetProcAddress(hModule, "FDS_Create" );
		if(FDS_Create = (FDSCREATE)GetProcAddress(hModule, (LPCSTR)"FDS_Create")){
			hFDS = FDS_Create();
			if(hFDS != NULL){
				FDS_Reset = (FDSRESET)GetProcAddress(hModule, (LPCSTR)"FDS_Reset");
				FDS_Setup = (FDSSETUP)GetProcAddress(hModule, (LPCSTR)"FDS_Setup");
				FDS_Write = (FDSWRITE)GetProcAddress(hModule, (LPCSTR)"FDS_Write");
				FDS_Read = (FDSREAD)GetProcAddress(hModule, (LPCSTR)"FDS_Read");
				FDS_WriteSync = (FDSWRITESYNC)GetProcAddress(hModule, (LPCSTR)"FDS_WriteSync");
				FDS_Process = (FDSPROCESS)GetProcAddress(hModule, (LPCSTR)"FDS_Process");

				hVirtuaNES_FDS_DLL = hModule;
				hVirtuaNES_FDS = hFDS;
				fds_snd_reset();
			}
		}
		else{
			FreeLibrary(hModule);
		}
	}
	if(hVirtuaNES_FDS_DLL == NULL){
		return 0;
	}
	return 1;
}
*/

static void fds_snd_write(u32 address, u8 value){
	FDSSoundWrite(address, value);
}

/*
static void fds_dll_snd_write(u32 address, u8 value){
	if(hVirtuaNES_FDS_DLL){
		if(hVirtuaNES_FDS){
			if(FDS_Write != NULL){
				FDS_Write(hVirtuaNES_FDS, (WORD)address, (char)value);
			}
		}
	}
}
*/

static void fds_snd_writesync(u32 address, u8 value){
	FDSSoundWriteCurrent(address, value);
}

/*
static void fds_dll_snd_writesync(u32 address, u8 value){
	if(hVirtuaNES_FDS_DLL){
		if(hVirtuaNES_FDS){
			if(FDS_WriteSync != NULL){
				FDS_WriteSync(hVirtuaNES_FDS, (WORD)address, (char)value);
			}
		}
	}
}
*/

static u8 fds_snd_read(u32 address){
	return FDSSoundRead(address);
}

/*
static u8 fds_dll_snd_read(u32 address){
	if(hVirtuaNES_FDS_DLL){
		if(hVirtuaNES_FDS){
			if(FDS_Read != NULL){
				return FDS_Read(hVirtuaNES_FDS, (WORD)address);
			}
		}
	}
	return 0;
}
*/

static void fds_snd_paramschanged(void){
}

/*
static void fds_dll_snd_paramschanged(void){
	if(hVirtuaNES_FDS_DLL){
		if(hVirtuaNES_FDS){
			if(FDS_Setup != NULL){
				FDS_Setup(hVirtuaNES_FDS, g_apu_t.sample_rate);
			}
		}
	}
}*/


static s32 fds_snd_process(void){
	return FDSSoundRender() >> 8;
}

/*
static s32 fds_dll_snd_process(void){
	if(hVirtuaNES_FDS_DLL){
		if(hVirtuaNES_FDS){
			if(FDS_Process != NULL){
				return FDS_Process(hVirtuaNES_FDS);
			}
		}
	}
	return 0;
}
*/

static apu_memwrite fds_memwrite[] =
{
   { 0x4040, 0x40FF, fds_snd_write },
   {     -1,     -1, NULL }
};


static apu_memwrite fds_memwritesync[] =
{
   { 0x4040, 0x40FF, fds_snd_writesync },
   {     -1,     -1, NULL }
};


static apu_memread fds_memread[] =
{
   { 0x4040, 0x40FF, fds_snd_read},
   {     -1,     -1, NULL }
};


apuext_t fds_ext =
{
   fds_snd_init,
   fds_snd_shutdown,
   fds_snd_reset,
   fds_snd_paramschanged,
   fds_snd_process,
   fds_memread,		/* apu_memread  */
   fds_memwrite,	/* apu_memwrite */
   fds_memwritesync	/* apu_memwrite */
};

/*
static apu_memwrite fds_dll_memwrite[] =
{
   { 0x4040, 0x40FF, fds_dll_snd_write },
   {     -1,     -1, NULL }
};


static apu_memwrite fds_dll_memwritesync[] =
{
   { 0x4040, 0x40FF, fds_dll_snd_writesync },
   {     -1,     -1, NULL }
};


static apu_memread fds_dll_memread[] =
{
   { 0x4040, 0x40FF, fds_dll_snd_read},
   {     -1,     -1, NULL }
};


apuext_t fds_dll_ext =
{
   fds_dll_snd_init,
   fds_dll_snd_shutdown,
   fds_dll_snd_reset,
   fds_dll_snd_paramschanged,
   fds_dll_snd_process,
   fds_dll_memread,		///* apu_memread
   fds_dll_memwrite,	///* apu_memwrite
   fds_dll_memwritesync	///* apu_memwrite
};

*/
