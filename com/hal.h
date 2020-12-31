
#ifndef __HAL_H__
#define __HAL_H__


#include "types.h"
#include "common.h"

enum { PCE_1=0,PCE_2,PCE_SEL,PCE_RUN,PCE_U,PCE_R,PCE_D,PCE_L,PCE_3,PCE_6,PCE_5,PCE_4 };
enum { NES_A=0,NES_B,NES_SEL,NES_STA,NES_U,NES_D,NES_L,NES_R };
enum { GBC_A=0,GBC_B,GBC_SEL,GBC_STA,GBC_D,GBC_U,GBC_L,GBC_R };
enum { WSC_S=1,WSC_A,WSC_B,WSC_XU,WSC_XR,WSC_XD,WSC_XL,WSC_YU,WSC_YR,WSC_YD,WSC_YL };
enum { NGP_U=0,NGP_D,NGP_L,NGP_R,NGP_A,NGP_B,NGP_O };
enum { SMS_U=0,SMS_D,SMS_L,SMS_R,SMS_2,SMS_1,SMS_STA=16,SMS_PAUSE,SMS_SRST,SMS_HRST };
enum { LNX_o=0,LNX_i,LNX_2,LNX_1,LNX_L,LNX_R,LNX_U,LNX_D };


void* HAL_mem_malloc(u32 sz);
void  HAL_mem_free(void* ptr);



//****************************************************************************
//** FRAMEBUFFER 
//****************************************************************************
#define RGB555      0
#define RGB444      1
#define RGB888      2

#define ZBUFFER_SP  0x8000
#define ZBUFFER_BG  0x0001

int  HAL_fb2_init(u32 w,u32 h,FBFORMAT* pFb,u32 type);
int  HAL_fb2_bitblt(FBFORMAT* pFb);
void HAL_fb2_close(void);
PIXEL_FORMAT HAL_fb2_Color(u8 r,u8 g,u8 b,u8 type);

int HAL_fps(int fps);

typedef void (*STATE_HDLR)(int fd);

CORE_HANDLER* GetCoreHandler(char *szFilePath);
CORE_HANDLER* GetCoreHandlerFromType(int type);

//****************************************************************************
//** STATE SAVE
//****************************************************************************
int HAL_SetupExt(int ext,
                 char* szExt,
                 void* pInit,
                 void* pLoop,
                 void* pExit,
                 void* pReset,
                 void* pLoad,
                 void* pSave);

//****************************************************************************
//** FILE IO
//****************************************************************************
int  HAL_fd_open(char* name,int mode);
void HAL_fd_close(int fp);
int  HAL_fd_seek(int fp,int pos,int whence);
int  HAL_fd_write(int fp,void* ptr,int length);
int  HAL_fd_read (int fp,void* ptr,int length);
int  HAL_fd_size(int fp);
void HAL_fd_delete(char* name);
void HAL_fd_mkdir(char* name);
//int  HAL_cmp_write(int fp,void* ptr,int length);
//int  HAL_cmp_read(int fp,void* ptr, int length);

// for State Save
int  HAL_sts_write(int fp,void* ptr,int length);
int  HAL_sts_read(int fp,void* ptr,int length);

//----------------------------------------------------------------------------
//-- OPEN MODE
//----------------------------------------------------------------------------
#define  HAL_MODE_READ   0
#define  HAL_MODE_WRITE  1
#define  HAL_MODE_APPEND 2

//----------------------------------------------------------------------------
//-- SEEK OPTION
//----------------------------------------------------------------------------
#define  HAL_SEEK_SET    0
#define  HAL_SEEK_CUR    1
#define  HAL_SEEK_END    2

#define  HAL_FP_MEM        0x11111111
#define  HAL_STS_COMPRESS  0x80000000

//****************************************************************************
//** DIRECTORY SECTION
//****************************************************************************
int HAL_IsSupportExt(int ext);
int HAL_GetSupportExt(int num);

void  HAL_SetWorkPath(const char* path);
char* HAL_GetWorkPath(void);
char* HAL_GetRomsPath(void);
void  HAL_SetRomsPath(char* path);
char* HAL_GetSavePath(int num);
char* HAL_GetSramPath(void);

int HAL_Cfg_Load(char* name,byte* adr,int size);
int HAL_Cfg_Save(char* name,byte* adr,int size);
int HAL_Mem_Load(byte* adr,int size);
int HAL_Mem_Save(byte* adr,int size);

/* INPUT */
u32 HAL_Input(u32 player,u32 type);

/* SOUND */
int  HAL_Sound_Init(void);
void HAL_Sound_Close(void);
void HAL_Sound_Proc16(s16* waveR,s16* waveL,int nSamples);
void HAL_Sound_Proc32(s32* waveR,s32* waveL,int nSamples);
void HAL_Sound_Proc32m(s32* wave,int nSamples);
int  HAL_Sound(void);

// HANDLER define
int PCE_Setup(void);
int SWAN_Setup(void);
int NES_Setup(void);
int GBC_Setup(void);
int NGP_Setup(void);
int SMS_Setup(void);

#ifdef __cplusplus
extern "C" int LYNX_Setup(void);
#endif


//****************************************************************************
//** STATE SECTION
//****************************************************************************
//互換性を重視したデータ保存が必要なのでちと考えるべし


#endif


