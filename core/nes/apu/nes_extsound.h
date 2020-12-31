
#ifndef _NES_APU_EXT_SOUND_H_
#define _NES_APU_EXT_SOUND_H_


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

//#define SAMPLE_RATE 44100
#define SAMPLE_RATE g_apu_t.sample_rate
#define NES_BASECYCLES (21477270)
#define M_PI 3.14159265358979323846

#define LOG_BITS 12
#define LIN_BITS 6
#define LOG_LIN_BITS 30

typedef struct
{
   u32 min_range, max_range;
   u8 (*read_func)(u32 address);
} apu_memread;

typedef struct
{
   u32 min_range, max_range;
   void (*write_func)(u32 address, u8 value);
} apu_memwrite;

typedef struct apuext_s
{
   int   (*init)(void);
   void  (*shutdown)(void);
   void  (*reset)(void);
   //DCR
   void  (*paramschanged)(void);
   s32 (*process)(void);
   apu_memread *mem_read;
   apu_memwrite *mem_write;
   apu_memwrite *mem_writesync;
} apuext_t;


#ifdef __cplusplus
extern "C" {
#endif


u32 LinearToLog(s32 l);
s32 LogToLinear(u32 l, u32 sft);
u32 DivFix(u32 p1, u32 p2, u32 fix);
void LogTableInitialize(void);
#ifdef __cplusplus
}
#endif

#endif
