//---------------------------------------------------------------------------
// NEOPOP : Emulator as in Dreamland
//
// Copyright (c) 2001-2002 by neopop_uk
//---------------------------------------------------------------------------

/************************************************************************
 *                                                                      *
 *	Portions, but not all of this source file are based on MAME v0.60	*
 *	File "sn76496.c". All copyright goes to the original author.		*
 *	The remaining parts, including DAC processing, by neopop_uk			*
 *                                                                      *
 ************************************************************************/

#include "neopop.h"
#include "mem.h"
#include "sound.h"
#include "cstring.h"

//=============================================================================

BOOL mute;

SoundChip toneChip;
SoundChip noiseChip;

//==== DAC
#define DAC_BUFFERSIZE		256 * 1024 

int dacBufferRead, dacBufferWrite, dacBufferCount;
u8 dacBufferL[DAC_BUFFERSIZE];

//=============================================================================

#define SOUNDCHIPCLOCK	(3072000)	//Unverified / sounds correct

#define MAX_OUTPUT 0x7fff
#define STEP 0x10000		//Fixed point adjuster

static u32 VolTable[16]={10922,8675,6891,5473,4348,3453,2743,2179,1731,1374,1092,867,689,547,434,0};
static u32 UpdateStep;	//Number of steps during one sample.

/* Formulas for noise generator */
/* bit0 = output */

/* noise feedback for white noise mode (verified on real SN76489 by John Kortink) */
#define FB_WNOISE 0x14002	/* (16bits) bit16 = bit0(out) ^ bit2 ^ bit15 */

/* noise feedback for periodic noise mode */
#define FB_PNOISE 0x08000	/* 15bit rotate */

/* noise generator start preset (for periodic noise) */
#define NG_PRESET 0x0f35

//=============================================================================

static u16 sample_chip_tone(void)
{
	int i;

	int vol[4];
	unsigned int out;
#if 1
	int left;
#endif

	/* vol[] keeps track of how long each square wave stays */
	/* in the 1 position during the sample period. */
	vol[0] = vol[1] = vol[2] = vol[3] = 0;

	for (i = 0; i < 3; i++)
	{
		if (toneChip.Output[i]) vol[i] += toneChip.Count[i];
		toneChip.Count[i] -= STEP;

		/* Period[i] is the half period of the square wave. Here, in each */
		/* loop I add Period[i] twice, so that at the end of the loop the */
		/* square wave is in the same status (0 or 1) it was at the start. */
		/* vol[i] is also incremented by Period[i], since the wave has been 1 */
		/* exactly half of the time, regardless of the initial position. */
		/* If we exit the loop in the middle, Output[i] has to be inverted */
		/* and vol[i] incremented only if the exit status of the square */
		/* wave is 1. */

		while (toneChip.Count[i] <= 0)
		{
			toneChip.Count[i] += toneChip.Period[i];
			if (toneChip.Count[i] > 0)
			{
				toneChip.Output[i] ^= 1;
				if (toneChip.Output[i]) vol[i] += toneChip.Period[i];
				break;
			}
			toneChip.Count[i] += toneChip.Period[i];
			vol[i] += toneChip.Period[i];
		}
		if (toneChip.Output[i]) vol[i] -= toneChip.Count[i];
	}

#if 1
	left = STEP;
	do
	{
		int nextevent;

		if (toneChip.Count[3] < left) nextevent = toneChip.Count[3];
		else nextevent = left;

		if (toneChip.Output[3]) vol[3] += toneChip.Count[3];
		toneChip.Count[3] -= nextevent;
		if (toneChip.Count[3] <= 0)
		{
			if (toneChip.RNG & 1) toneChip.RNG ^= toneChip.NoiseFB;
			toneChip.RNG >>= 1;
			toneChip.Output[3] = toneChip.RNG & 1;
			toneChip.Count[3] += toneChip.Period[3];
			if (toneChip.Output[3]) vol[3] += toneChip.Period[3];
		}
		if (toneChip.Output[3]) vol[3] -= toneChip.Count[3];

		left -= nextevent;
	} while (left > 0);
#endif


	out = vol[0] * toneChip.Volume[0] + vol[1] * toneChip.Volume[1] +
		vol[2] * toneChip.Volume[2];

	if (out > MAX_OUTPUT * STEP) out = MAX_OUTPUT * STEP;

	return out / STEP;
}

//=============================================================================

static u16 sample_chip_noise(void)
{
#if 1
	int i;
#endif

	int vol[4];
	unsigned int out;
	int left;

	/* vol[] keeps track of how long each square wave stays */
	/* in the 1 position during the sample period. */
	vol[0] = vol[1] = vol[2] = vol[3] = 0;

#if 1
	for (i = 0; i < 3; i++)
	{
		if (noiseChip.Output[i]) vol[i] += noiseChip.Count[i];
		noiseChip.Count[i] -= STEP;

		/* Period[i] is the half period of the square wave. Here, in each */
		/* loop I add Period[i] twice, so that at the end of the loop the */
		/* square wave is in the same status (0 or 1) it was at the start. */
		/* vol[i] is also incremented by Period[i], since the wave has been 1 */
		/* exactly half of the time, regardless of the initial position. */
		/* If we exit the loop in the middle, Output[i] has to be inverted */
		/* and vol[i] incremented only if the exit status of the square */
		/* wave is 1. */

		while (noiseChip.Count[i] <= 0)
		{
			noiseChip.Count[i] += noiseChip.Period[i];
			if (noiseChip.Count[i] > 0)
			{
				noiseChip.Output[i] ^= 1;
				if (noiseChip.Output[i]) vol[i] += noiseChip.Period[i];
				break;
			}
			noiseChip.Count[i] += noiseChip.Period[i];
			vol[i] += noiseChip.Period[i];
		}
		if (noiseChip.Output[i]) vol[i] -= noiseChip.Count[i];
	}
#endif

	left = STEP;
	do
	{
		int nextevent;

		if (noiseChip.Count[3] < left) nextevent = noiseChip.Count[3];
		else nextevent = left;

		if (noiseChip.Output[3]) vol[3] += noiseChip.Count[3];
		noiseChip.Count[3] -= nextevent;
		if (noiseChip.Count[3] <= 0)
		{
			if (noiseChip.RNG & 1) noiseChip.RNG ^= noiseChip.NoiseFB;
			noiseChip.RNG >>= 1;
			noiseChip.Output[3] = noiseChip.RNG & 1;
			noiseChip.Count[3] += noiseChip.Period[3];
			if (noiseChip.Output[3]) vol[3] += noiseChip.Period[3];
		}
		if (noiseChip.Output[3]) vol[3] -= noiseChip.Count[3];

		left -= nextevent;
	} while (left > 0);


	out = vol[3] * noiseChip.Volume[3];

	if (out > MAX_OUTPUT * STEP) out = MAX_OUTPUT * STEP;

	return out / STEP;
}

//=============================================================================

void sound_update(u16* pR, u16* pL, int length_bytes)
{
	u16 data;

	while (length_bytes > 0) {
		//Mix a mono track out of: (Tone + Noise) >> 1
		//Write it to the sound buffer
		data = (sample_chip_tone() + sample_chip_noise()) >> 1;

		*pR++ = data;
		*pL++ = data;

		length_bytes --;	// 2 bytes = 16 bits
	}
}

//=============================================================================

void WriteSoundChip(SoundChip* chip, u8 data)
{
	//Command
	if (data & 0x80)
	{
		int r = (data & 0x70) >> 4;
		int c = r/2;

		chip->LastRegister = r;
		chip->Register[r] = (chip->Register[r] & 0x3f0) | (data & 0x0f);
	
		switch(r){
		case 0:	/* tone 0 : frequency */
		case 2:	/* tone 1 : frequency */
		case 4:	/* tone 2 : frequency */
			chip->Period[c] = UpdateStep * chip->Register[r];
			if (chip->Period[c] == 0) chip->Period[c] = UpdateStep;
			if (r == 4){
				/* update noise shift frequency */
				if ((chip->Register[6] & 0x03) == 0x03)
					chip->Period[3] = 2 * chip->Period[2];
			}
			break;

		case 1:	/* tone 0 : volume */
		case 3:	/* tone 1 : volume */
		case 5:	/* tone 2 : volume */
		case 7:	/* noise  : volume */
#ifdef NEOPOP_DEBUG
			if (filter_sound){
				if (chip == &toneChip)
					system_debug_message("sound (T): Set Tone %d Volume to %d (0 = min, 15 = max)", c, 15 - (data & 0xF));
				else
					system_debug_message("sound (N): Set Tone %d Volume to %d (0 = min, 15 = max)", c, 15 - (data & 0xF));
			}
#endif
			chip->Volume[c] = VolTable[data & 0xF];
			break;

		case 6:	/* noise  : frequency, mode */
			{
				int n = chip->Register[6];
#ifdef NEOPOP_DEBUG
	if (filter_sound)
	{
		char *pm, *nm = "White";
		if ((n & 4)) nm = "Periodic";

		switch(n & 3)
		{
		case 0: pm = "N/512"; break;
		case 1: pm = "N/1024"; break;
		case 2: pm = "N/2048"; break;
		case 3: pm = "Tone#2"; break;
		}
			
		if (chip == &toneChip)
			system_debug_message("sound (T): Set Noise Mode to %s, Period = %s", nm, pm);
		else
			system_debug_message("sound (N): Set Noise Mode to %s, Period = %s", nm, pm);
	}
#endif
				chip->NoiseFB = (n & 4) ? FB_WNOISE : FB_PNOISE;
				n &= 3;
				/* N/512,N/1024,N/2048,Tone #2 output */
				chip->Period[3] = (n == 3) ? 2 * chip->Period[2] : (UpdateStep << (5+n));

				/* reset noise shifter */
				chip->RNG = NG_PRESET;
				chip->Output[3] = chip->RNG & 1;

			}
			break;
		}
	}
	else
	{
		int r = chip->LastRegister;
		int c = r/2;

		switch (r)
		{
			case 0:	/* tone 0 : frequency */
			case 2:	/* tone 1 : frequency */
			case 4:	/* tone 2 : frequency */
				chip->Register[r] = (chip->Register[r] & 0x0f) | ((data & 0x3f) << 4);
				chip->Period[c] = UpdateStep * chip->Register[r];
				if (chip->Period[c] == 0) chip->Period[c] = UpdateStep;
				if (r == 4)
				{
					/* update noise shift frequency */
					if ((chip->Register[6] & 0x03) == 0x03)
						chip->Period[3] = 2 * chip->Period[2];
				}
#ifdef NEOPOP_DEBUG
	if (filter_sound)
	{
		if (chip == &toneChip)
			system_debug_message("sound (T): Set Tone %d Frequency to %d", c, chip->Register[r]);
		else
			system_debug_message("sound (N): Set Tone %d Frequency to %d", c, chip->Register[r]);
	}
#endif
				break;
		}
	}
}

//=============================================================================

void dac_write(void)
{
	if (mute) return;	// Ignore	

	//Write to buffer
	dacBufferL[dacBufferWrite] = ram[0xA2];
	dacBufferWrite++;
	if (dacBufferWrite == DAC_BUFFERSIZE)
		dacBufferWrite = 0;

	//Overflow?
	dacBufferCount++;
	if (dacBufferCount == DAC_BUFFERSIZE)
	{
//		system_message("dac_write: DAC buffer overflow\nPlease report this to the author.");
		dacBufferCount = 0;
	}
}
	
void dac_update(u8* dac_buffer, int length_bytes)
{
	while (length_bytes > 0)
	{
		//Copy then clear DAC data
		*(dac_buffer++) = dacBufferL[dacBufferRead];
		dacBufferL[dacBufferRead] = 0x80;
			
		length_bytes --;	// 1 byte = 8 bits

		if (dacBufferCount > 0)
		{
			dacBufferCount --;

			//Advance the DAC read
			dacBufferRead++;
			if (dacBufferRead == DAC_BUFFERSIZE)
				dacBufferRead = 0;
		}
	}
}

//=============================================================================

//Resets the sound chips, also used whenever sound options are changed
void sound_init(int SampleRate)
{
#if 1
	int i;
//	double out;

	/* the base clock for the tone generators is the chip clock divided by 16; */
	/* for the noise generator, it is clock / 256. */
	/* Here we calculate the number of steps which happen during one sample */
	/* at the given sample rate. No. of events = sample rate / (clock/16). */
	/* STEP is a multiplier used to turn the fraction into a fixed point */
	/* number. */
	

	//
	// STEP                   0x10000
	// SOUNDCHIPCLOCK         3072000
	// 
	// 
//	UpdateStep = (u32)(((double)STEP * SampleRate * 16) / SOUNDCHIPCLOCK);

	switch(SampleRate){
	case 44100: UpdateStep = 15052; break;
	case 22050: UpdateStep =  7526; break;
	case 11025: UpdateStep =  3763; break;
	default:
		SampleRate=44100;
		UpdateStep=15052;
		break;
	}

	//Initialise Left Chip
	core_memset(&toneChip, 0, sizeof(SoundChip));

	//Initialise Right Chip
	core_memset(&noiseChip, 0, sizeof(SoundChip));

	//Default register settings
	for(i=0;i<8;i+=2) {
		toneChip.Register[i] = 0;
		toneChip.Register[i + 1] = 0x0f;	/* volume = 0 */
		noiseChip.Register[i] = 0;
		noiseChip.Register[i + 1] = 0x0f;	/* volume = 0 */
	}

	for(i=0;i<4;i++){
		toneChip.Output[i] = 0;
		toneChip.Period[i] = toneChip.Count[i] = UpdateStep;
		noiseChip.Output[i] = 0;
		noiseChip.Period[i] = noiseChip.Count[i] = UpdateStep;
	}

//	//Build the volume table
//	// MAX_OUTPUT = 0x7fff
//	out = MAX_OUTPUT / 3;
//
//	/* build volume table (2dB per step) */
//	for(i=0;i<15;i++){
//		VolTable[i] = (u32)out;
//		out /= 1.258925412;	/* = 10 ^ (2/20) = 2dB */
//	}
//	VolTable[15] = 0;

	//Clear the DAC buffer
	for(i=0;i<DAC_BUFFERSIZE;i++) {
		dacBufferL[i] = 0x80;
	}

	dacBufferCount = 0;
	dacBufferRead = 0; 
	dacBufferWrite = 0;

#endif
}

//=============================================================================


