/*
** Nofrendo (c) 1998-2000 Matthew Conte (matt@conte.com)
**
**
** This program is free software; you can redistribute it and/or
** modify it under the terms of version 2 of the GNU Library General 
** Public License as published by the Free Software Foundation.
**
** This program is distributed in the hope that it will be useful, 
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU 
** Library General Public License for more details.  To obtain a 
** copy of the GNU Library General Public License, write to the Free 
** Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
**
** Any permitted reproduction of these routines, in whole or in part,
** must bear this legend.
**
**
** nes_apu.h
**
** NES APU emulation header file
** $Id: nes_apu.h,v 1.21 2000/08/11 02:27:21 matt Exp $
*/

#ifndef _NES_APU_H_
#define _NES_APU_H_

#include "nes_extsound.h"


///* HACK to get Win32 SDL to compile */
//#if defined(WIN32) && !defined(_MSC_VER) // && !defined(u8)
//#define INLINE static inline
//#define s8 char
//#define s16 short
//#define s32 int
//#define u8 unsigned char
//#define u16 unsigned short
//#define u32 unsigned int
////#define u8 u8
//#endif /* a whole bunch of crap */

#ifdef __GNUC__
#define  INLINE      static inline
#elif defined(WIN32)
#define  INLINE      static __inline
#else
#define  INLINE      static
#endif

/* define this for realtime generated noise */
#define  REALTIME_NOISE

/* define this for enabling sound code by T.Yano */
//#define  APU_YANO

#define  APU_WRA0       0x4000
#define  APU_WRA1       0x4001
#define  APU_WRA2       0x4002
#define  APU_WRA3       0x4003
#define  APU_WRB0       0x4004
#define  APU_WRB1       0x4005
#define  APU_WRB2       0x4006
#define  APU_WRB3       0x4007
#define  APU_WRC0       0x4008
#define  APU_WRC2       0x400A
#define  APU_WRC3       0x400B
#define  APU_WRD0       0x400C
#define  APU_WRD2       0x400E
#define  APU_WRD3       0x400F
#define  APU_WRE0       0x4010
#define  APU_WRE1       0x4011
#define  APU_WRE2       0x4012
#define  APU_WRE3       0x4013

#define  APU_SMASK      0x4015

/* length of generated noise */
#define  APU_NOISE_32K  0x7FFF
#define  APU_NOISE_93   93

//#define  APU_BASEFREQ   1789772.7272727272727272
#define  APU_BASEFREQ   1789772.5

/* to/from 16.16 fixed point */
#define  APU_TO_FIXED(x)    ((x) << 16)
#define  APU_FROM_FIXED(x)  ((x) >> 16)

// ----------------------------------------------------------------------------
// APU Sound struct

/* channel structures */
/* As much data as possible is precalculated,
** to keep the sample processing as lean as possible
*/
 
typedef struct rectangle_s
{
   u8 regs[4];

   u8 enabled;
   
   s32 phaseacc;
   s32 freq;
   s32 output_vol;
   u8 fixed_envelope;
   u8 holdnote;
   u8 volume;

   s32 sweep_phase;
   s32 sweep_delay;
   u8 sweep_on;
   u8 sweep_shifts;
   u8 sweep_length;
   u8 sweep_inc;

   /* this may not be necessary in the future */
   s32 freq_limit;

   /* rectangle 0 uses a complement addition for sweep
   ** increases, while rectangle 1 uses subtraction
   */
   u8 sweep_complement;

   s32 env_phase;
   s32 env_delay;
   u8 env_vol;

   int vbl_length;
   u8 adder;
   int duty_flip;

   /* for sync read $4105 */
   u8 enabled_cur;
   u8 holdnote_cur;
   int vbl_length_cur;
} rectangle_t;

typedef struct triangle_s
{
   u8 regs[3];

   u8 enabled;

   s32 freq;
   s32 phaseacc;
   s32 output_vol;

   u8 adder;

   u8 holdnote;
   u8 counter_started;
   /* quasi-hack */
   int write_latency;

   int vbl_length;
   int linear_length;

   /* for sync read $4105 */
   u8 enabled_cur;
   u8 holdnote_cur;
   u8 counter_started_cur;
   int vbl_length_cur;

#ifdef APU_YANO
   /* less compatibility, clearer sound if enabled */
   u8 ideal_triangle;
#endif
} triangle_t;

typedef struct noise_s
{
   u8 regs[3];

   u8 enabled;

   s32 freq;
   s32 phaseacc;
   s32 output_vol;

   s32 env_phase;
   s32 env_delay;
   u8 env_vol;
   u8 fixed_envelope;
   u8 holdnote;

   u8 volume;

   int vbl_length;

#ifdef REALTIME_NOISE
   u8 xor_tap;
#else
   u8 short_sample;
   int cur_pos;
#endif /* REALTIME_NOISE */

   /* for sync read $4105 */
   u8 enabled_cur;
   u8 holdnote_cur;
   int vbl_length_cur;
} noise_t;

typedef struct dmc_s
{
   u8 regs[4];

   /* bodge for timestamp queue */
   u8 enabled;
   
   s32 freq;
   s32 phaseacc;
   s32 output_vol;

   u32 address;
   u32 cached_addr;
   int dma_length;
   int cached_dmalength;
   u8 cur_byte;

   u8 looping;
   u8 irq_gen;
   u8 irq_occurred;

   /* for sync read $4105 and DPCM IRQ */
   s32 freq_cur;
   s32 phaseacc_cur;
   int dma_length_cur;
   int cached_dmalength_cur;
   u8 enabled_cur;
   u8 looping_cur;
   u8 irq_gen_cur;
   u8 irq_occurred_cur;
} dmc_t;

typedef struct apusound_s
{
   rectangle_t rectangle[2];
   triangle_t triangle;
   noise_t noise;
   dmc_t dmc;
} APUSOUND;

// ----------------------------------------------------------------------------
// VRC6 Sound struct
/*
typedef struct {
	u32 cps;
	s32 cycles;
	u32 spd;
	u8 regs[3];
	u8 update;
	u8 adr;
	u8 mute;
} VRC6_SQUARE;

typedef struct {
	u32 cps;
	s32 cycles;
	u32 spd;
	u32 output;
	u8 regs[3];
	u8 update;
	u8 adr;
	u8 mute;
} VRC6_SAW;

typedef struct {
	VRC6_SQUARE square[2];
	VRC6_SAW saw;
	u32 mastervolume;
} VRC6SOUND;

// ----------------------------------------------------------------------------
// APU Sound struct

typedef struct {
	u32	pg_phase;
	u32	pg_spd;
	s32	vib_cycles;
	u32	input;
	u32	eg_phase;
	u32	eg_sl;
	u32	eg_arr;
	u32	eg_drr;
	u32	eg_rrr;
	u8	pg_vib;
	u32	*sintblp;
	u8	tl;
	u8	eg_mode;
	u8	eg_type;
	u8	su_type;
	u8	eg_ar;
	u8	eg_dr;
	u8	eg_rr;
	u8	eg_ks;
	u8	eg_am;
} OPLL_OP;

typedef struct {
	u32	cps;
	u32	spd;
	s32	cycles;
	u32	adr;
	u32	adrmask;
	u32	*table;
	u32	output;
} OPLL_LFO;

typedef struct {
	u32 cps;
	s32 cycles;
	u32 fbbuf[2];
	u32 output;
	OPLL_OP op[2];
	u32 mastervolume;
	u8 tone[8];
	u8 key;
	u8 toneno;
	u8 freql;
	u8 freqh;
	u8 fb;
	u8 update;
} OPLL_CH;

typedef struct {
	OPLL_CH ch[6];
	OPLL_LFO lfo[2];
	u32 mastervolume;
	u8 usertone[8];
	u8 adr;
	u8 rhythmc;
	u8 toneupdate;
} OPLLSOUND;

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

// ----------------------------------------------------------------------------
// MMC5 Sound struct

typedef struct {
	u32 cps;
	s32 cycles;
	s32 sweepphase;
	s32 envphase;

	u32 spd;
	u32 envspd;
	u32 sweepspd;

	u32 length;
	u32 freq;
	u32 mastervolume;
	u32 release;

	u8 regs[4];
	u8 update;
	u8 key;
	u8 adr;
	u8 envadr;
	u8 duty;
	u8 mute;
} MMC5_SQUARE;

typedef struct {
	s32 output;
	u8 key;
	u8 mute;
} MMC5_DA;

typedef struct {
	MMC5_SQUARE square[2];
	MMC5_DA da;
} MMC5SOUND;

// ----------------------------------------------------------------------------
// N106 Sound struct

typedef struct {
	u32 logvol;
	s32 cycles;
	u32 spd;
	u32 phase;
	u32 tlen;
	u8 update;
	u8 freql;
	u8 freqm;
	u8 freqh;
	u8 vreg;
	u8 tadr;
	u8 nazo;
	u8 mute;
} N106_WM;

typedef struct {
	u32 cps;
	u32 mastervolume;

	N106_WM ch[8];

	u8 addressauto;
	u8 address;
	u8 chinuse;

	u32 tone[0x100];	// TONE DATA
	u8 data[0x80];
} N106SOUND;

// ----------------------------------------------------------------------------
// FME7 Sound struct

typedef struct {
	u32 cps;
	s32 cycles;
	u32 spd;
	u8 regs[3];
	u8 update;
	u8 adr;
	u8 mute;
	u8 key;
} PSG_SQUARE;

typedef struct {
	u32 cps;
	s32 cycles;
	u32 spd;
	u32 noiserng;
	u8 regs[1];
	u8 update;
	u8 noiseout;
} PSG_NOISE;

typedef struct {
	u32 cps;
	s32 cycles;
	u32 spd;
	u32 envout;
	s8 *adr;
	u8 regs[3];
	u8 update;
} PSG_ENVELOPE;

typedef struct {
	PSG_SQUARE square[3];
	PSG_ENVELOPE envelope;
	PSG_NOISE noise;
	u32 mastervolume;
	u32 adr;
} PSGSOUND;
*/
// ----------------------------------------------------------------------------
// APU Sound struct

enum
{
   APU_FILTER_NONE,
   APU_FILTER_LOWPASS,
   APU_FILTER_WEIGHTED,
   APU_FILTER_OLOWPASS
};

#ifdef APU_YANO
enum
{
   APUMODE_IDEAL_TRIANGLE,
   APUMODE_SMOOTH_ENVELOPE,
   APUMODE_SMOOTH_SWEEP
};
#endif /* APU_YANO */



/* APU queue structure */
#define  APUQUEUE_SIZE  4096
#define  APUQUEUE_MASK  (APUQUEUE_SIZE - 1)


#define  NESAPU_EXCHIP_MAX 8

/* apu ring buffer member */
typedef struct apudata_s
{
   u32 timestamp, address;
   u8 value;
} apudata_t;

typedef struct apu_s
{
   APUSOUND apus;
//   VRC6SOUND vrc6s;
//   OPLLSOUND ym2413s;
//   FDSSOUND fdssound;
//   MMC5SOUND mmc5;
//   N106SOUND n106s;
//   PSGSOUND psg;

   u8 enable_reg;
   u8 enable_reg_cur;

   apudata_t queue[APUQUEUE_SIZE];
   int q_head, q_tail;

   // for ExSound
   apudata_t ex_queue[APUQUEUE_SIZE];
   int ex_q_head, ex_q_tail;
   u8 ex_chip;

   u32 elapsed_cycles;

   void *buffer; /* pointer to output buffer */
   int num_samples;

   u8 mix_enable[6];
   int filter_type;
   int lowpass_filter_f;

//   float mix_volumed[6];

   s32 cycle_rate;

   int sample_rate;
   int sample_bits;
   int refresh_rate;

   void (*process)(void *buffer, int num_samples);

   /* external sound chip */
   apuext_t *ext[NESAPU_EXCHIP_MAX];
} apu_t;


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Function prototypes */
extern apu_t *apu_getcontext(void);
extern void apu_setcontext(apu_t *src_apu);

extern apu_t *apu_create(int sample_rate, int refresh_rate, int frag_size,
                         int sample_bits);
extern void apu_destroy(void);
extern void apu_setparams(int sample_rate, int refresh_rate, int frag_size,
                          int sample_bits);

extern void nes_apu_process(s32 *buffer, int num_samples);
extern void nes_apu_reset(void);

//extern void apu_setext(apu_t *apu, apuext_t *ext);
extern void apu_setfilter(int filter_type);
extern void apu_setlowpass_filter_f(int lowpass_filter_f);
extern void apu_setchan(int chan, u8 enabled);
//extern void apu_setvolumed(int chan, float);

#ifdef APU_YANO
extern void apu_setmode(int item, int mode);
#endif /* APU_YANO */

extern u8 nes_apu_read(u32 address);
extern void nes_apu_write(u32 address, u8 value);

extern u8 ex_read(u32 address);
extern void ex_write(u32 address, u8 value);

extern void nes_apu_write_cur(u32 address, u8 value);
extern void sync_apu_register();
extern u8 sync_dmc_register(u32 cpu_cycles);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _NES_APU_H_ */

/*
** $Log: nes_apu.h,v $
** Revision 2.16  2001/02/18 21:31:00  YANO, takashi
** added $4017:bit7 control
** fixed set chan->enabled
** fixed bug of DPCM last sample in APU_YANO
** trim channel balance again
** fixed APU_BASEFREQ
**
** Revision 2.15  2001/01/04 22:09:10  YANO, takashi
** fixed dmc bug of delta_bit
**
** Revision 2.14  2001/01/03 17:46:00  YANO, takashi
** fixed code for DC balance
** trim noise and dmc volume
**
** Revision 2.13  2000/12/23 02:00:40  YANO, takashi
** added apu_setmode()
**
** Revision 2.12  2000/12/22 23:23:40  YANO, takashi
** trim output volume balance
** re-disabled APU_IDEAL_TRIANGLE
** modify weighted filter
**
** Revision 2.11  2000/12/17 00:08:10  YANO, takashi
** cancel DC offset
** enable APU_IDEAL_TRIANGLE
**
** Revision 2.10  2000/12/12 02:07:46  YANO, takashi
** improve sound purity
**
** Revision 2.09  2000/12/09 11:41:00  TAKEDA, toshiya
** sync DPCM registers
** support DMCP IRQ
**
** Revision 2.08  2000/12/07 00:10:00  TAKEDA, toshiya
** sync DPCM registers
**
** Revision 2.07  2000/11/15 16:32:00  TAKEDA, toshiya
** fixed memory reak of ExtraSound
**
** Revision 2.06  2000/11/02 21:40:00  TAKEDA, toshiya
** fixed read $4015 (triangle.write_latency)
**
** Revision 2.05  2000/11/01 21:44:00  TAKEDA, toshiya
** fixed read $4015
**
** Revision 2.04  2000/10/26 00:05:00  TAKEDA, toshiya
** changed VRC6 volume
** changed chip number
**
** Revision 2.03  2000/10/23 16:06:00  TAKEDA, toshiya
** added ExtraSound Support of MMC5
** sync All ExtraSound
**
** Revision 2.02  2000/10/23 00:07:00  TAKEDA, toshiya
** fixed VRC6 write reg
**
** Revision 2.01  2000/10/22 21:12:00  TAKEDA, toshiya
** added ExtraSound Support of FME7
**
** Revision 2.00  2000/10/22 00:12:15  TAKEDA, toshiya
** added ExtraSound Support of N106, FDS, VRC6, VRC7
**
** ---------------------------------------------------
**
** Revision 1.21  2000/08/11 02:27:21  matt
** general cleanups, plus apu_setparams routine
**
** Revision 1.20  2000/07/30 04:32:59  matt
** no more apu_getcyclerate hack
**
** Revision 1.19  2000/07/27 02:49:50  matt
** eccentricity in sweeping hardware now emulated correctly
**
** Revision 1.18  2000/07/25 02:25:15  matt
** safer apu_destroy
**
** Revision 1.17  2000/07/23 15:10:54  matt
** hacks for win32
**
** Revision 1.16  2000/07/23 00:48:15  neil
** Win32 SDL
**
** Revision 1.15  2000/07/17 01:52:31  matt
** made sure last line of all source files is a newline
**
** Revision 1.14  2000/07/11 02:39:26  matt
** added setcontext() routine
**
** Revision 1.13  2000/07/10 05:29:34  matt
** moved joypad/oam dma from apu to ppu
**
** Revision 1.12  2000/07/04 04:54:48  matt
** minor changes that helped with MAME
**
** Revision 1.11  2000/07/03 02:18:53  matt
** much better external module exporting
**
** Revision 1.10  2000/06/26 05:00:37  matt
** cleanups
**
** Revision 1.9  2000/06/23 03:29:28  matt
** cleaned up external sound inteface
**
** Revision 1.8  2000/06/20 04:06:16  matt
** migrated external sound definition to apu module
**
** Revision 1.7  2000/06/20 00:07:35  matt
** added convenience members to apu_t struct
**
** Revision 1.6  2000/06/09 16:49:02  matt
** removed all floating point from sound generation
**
** Revision 1.5  2000/06/09 15:12:28  matt
** initial revision
**
*/

