//---------------------------------------------------------------------------
// NEOPOP : Emulator as in Dreamland
//
// Copyright (c) 2001-2002 by neopop_uk
//---------------------------------------------------------------------------

#ifndef __INTERRUPT__
#define __INTERRUPT__
//=============================================================================

void interrupt(u8 index);

#define TIMER_HINT_RATE		515		//CPU Ticks between horizontal interrupts

#define TIMER_BASE_RATE		240		//ticks

#define TIMER_T1_RATE		(1 * TIMER_BASE_RATE)
#define TIMER_T4_RATE		(4 * TIMER_BASE_RATE)
#define TIMER_T16_RATE		(16 * TIMER_BASE_RATE)
#define TIMER_T256_RATE		(256 * TIMER_BASE_RATE)

void reset_timers(void);

//void updateTimers(u32 cputicks);

//H-INT Timer
//extern u32 timer_hint;
//extern u8 timer[4];	//Up-counters
//extern u32 timer_clock0, timer_clock1, timer_clock2, timer_clock3;

// Set this value to fix problems with glitching extra lines.
//extern BOOL gfx_hack;

//=============================================================================
#endif




