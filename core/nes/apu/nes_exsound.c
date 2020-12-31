
/*
** ExSound by TAKEDA, toshiya
**
** original: nezp0922
*/

//#include <math.h>
//#include <windows.h>
//#include <stdio.h>

#include "nes_extsound.h"



// ----------------------------------------------------------------------------
// common

// ex_chip	=  0; none
//		=  1; VRC6
//		=  2; VRC7
//		=  4; FDS
//		=  8; MMC5
//		= 16; N106
//		= 32; FME7
//		= 64; J106 (reserved)
#if 0
//#define SAMPLE_RATE 44100
#define SAMPLE_RATE apu->sample_rate
#define NES_BASECYCLES (21477270)
#define M_PI 3.14159265358979323846

#define LOG_BITS 12
#define LIN_BITS 6
#define LOG_LIN_BITS 30
#endif

//static u32 lineartbl[(1 << LIN_BITS) + 1];
//static u32 logtbl[1 << LOG_BITS];
// セット済みテーブル
#include "exsound_tbl.h"

u32 LinearToLog(s32 l)
{
	return (l < 0) ? (lineartbl[-l] + 1) : lineartbl[l];
}

s32 LogToLinear(u32 l, u32 sft)
{
	s32 ret;
	u32 ofs;
	sft += (l >> 1) >> LOG_BITS;
	if (sft >= LOG_LIN_BITS) return 0;
	ofs = (l >> 1) & ((1 << LOG_BITS) - 1);
	ret = logtbl[ofs] >> sft;
	return (l & 1) ? -ret : ret;
}

void LogTableInitialize(void)
{
/*
	static s32 initialized = 0;
	s32 i;
	double a;
	if (initialized) return;
	initialized = 1;
	for (i = 0; i < (1 << LOG_BITS); i++)
	{
		a = (1 << LOG_LIN_BITS) / pow(2, i / (double)(1 << LOG_BITS));
		logtbl[i] = (u32)a;
	}
	lineartbl[0] = LOG_LIN_BITS << LOG_BITS;
	for (i = 1; i < (1 << LIN_BITS) + 1; i++)
	{
		u32 ua;
		a = i << (LOG_LIN_BITS - LIN_BITS);
		ua = (u32)((LOG_LIN_BITS - (log(a) / log(2))) * (1 << LOG_BITS));
		lineartbl[i] = ua << 1;
	}
*/
}

u32 DivFix(u32 p1, u32 p2, u32 fix)
{
	u32 ret;
	ret = p1 / p2;
	p1  = p1 % p2;/* p1 = p1 - p2 * ret; */
	while (fix--)
	{
		p1 += p1;
		ret += ret;
		if (p1 >= p2)
		{
			p1 -= p2;
			ret++;
		}
	}
	return ret;
}

#if 0
#include "nes_vrc6.cpp"
#include "nes_vrc7.cpp"
//#include "nes_fds_new.cpp"
//#include "nes_fds.cpp"
#include "nes_mmc5.cpp"
#include "nes_n106.cpp"
#include "nes_fme7.cpp"
#endif

