#ifndef _COMMON_H_
#define _COMMON_H_

#define WORKPATH_LENGTH  2048

//****************************************************************************
//** SOUND SECTION 
//****************************************************************************
#define SZ_MIXWAVE   2048
#define SZ_MIXCH     16

typedef struct {
  s32 ch[SZ_MIXCH];
  
  union {
    s32 R32[SZ_MIXWAVE];
    s16 R16[SZ_MIXWAVE*2];
    s8  R8 [SZ_MIXWAVE*4];
  };
  
  union {
    s32 L32[SZ_MIXWAVE];
    s16 L16[SZ_MIXWAVE*2];
    s8  L8 [SZ_MIXWAVE*4];
  };
  
} HAL_SOUNDBUFFER;


#if !defined(COM_DEFINE)
#define C_EXTERN extern
#else
#define C_EXTERN
#endif

C_EXTERN HAL_SOUNDBUFFER halSnd;

#endif

