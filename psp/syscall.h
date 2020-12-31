/* system call prototype for PSP */
//#if defined(WIN32)
//#include "types.h"
//#endif

#ifndef _SYSCALL_H_INCLUDED
#define _SYSCALL_H_INCLUDED

#ifndef NULL
#define NULL 0
#endif

#ifndef true
#define true 1
#endif

#ifdef false
#define false 0
#endif



#define MAXPATH 512
#define MAXNAME 256
//#define MAX_ENTRY 1024


/******************************************************************************/
/* sceCtrl */
/* Index for the two analog directions */ 
/* Button bit masks */ 
#define CTRL_UP          0x00000010 // 4
#define CTRL_RIGHT       0x00000020 // 5
#define CTRL_DOWN        0x00000040 // 6
#define CTRL_LEFT        0x00000080 // 7

#define CTRL_LTRIGGER    0x00000100 // 8
#define CTRL_RTRIGGER    0x00000200 // 9

#define CTRL_TRIANGLE    0x00001000 // 12
#define CTRL_CIRCLE      0x00002000 // 13
#define CTRL_CROSS       0x00004000 // 14
#define CTRL_SQUARE      0x00008000 // 15

#define CTRL_SELECT      0x00000001 // 0
#define CTRL_START       0x00000008 // 3

#define CTRL_A_UP        0x00000400 // 10
#define CTRL_A_DOWN      0x00000800 // 11
#define CTRL_A_RIGHT     0x00000004 // 2
#define CTRL_A_LEFT      0x00000002 // 1

/* Returned control data */ 
typedef struct _ctrl_data {
  u32 frame; 
  u32 buttons;
  u8  Lx;
  u8  Ly;
  u8  unused[6];
} ctrl_data_t; 

/******************************************************************************/
/* IoFileMgrForUser */

#define O_RDONLY    0x0001 
#define O_WRONLY    0x0002 
#define O_RDWR      0x0003 
#define O_NBLOCK    0x0010 
#define O_APPEND    0x0100 
#define O_CREAT     0x0200 
#define O_TRUNC     0x0400 
#define O_NOWAIT    0x8000 


#define SCE_SEEK_SET	(0)
#define SCE_SEEK_CUR	(1)
#define SCE_SEEK_END	(2)

enum { 
  TYPE_DIR=0x10, 
  TYPE_FILE=0x20 
}; 


struct dirent { 
  u32  unk0; 
  u32  type; 
  u32  size; 
  u32  unk[19]; 
  char name[0x108]; 
};

typedef int (*pg_threadfunc_t)(int args, void *argp);


#ifndef PSPSDK /////////////////////////////////////////////////////////////////

/* sceDisplay */
void sceDisplayWaitVblank(void);
void sceDisplayWaitVblankCB(void);
void sceDisplayWaitVblankStart(void);
void sceDisplayWaitVblankStartCB(void);
void sceDisplaySetMode(int,int,int);
void sceDisplaySetFrameBuf(char *topaddr,long linesize,long pixelsize,long);
int sceDisplayGetVcount(void);



int sceIoOpen(const char* file, int mode, int unknown); 
int sceIoClose(int fd); 
int sceIoRead(int fd, void *data, int size); 
int sceIoWrite(int fd, void *data, int size); 
//int sceIoLseek(int fd, long long offset, int whence); 
int sceIoRemove(const char *file); 
int sceIoMkdir(const char *dir, int mode); 
int sceIoRmdir(const char *dir); 
int sceIoRename(const char *oldname, const char *newname); 


int sceIoDopen(const char *fn); 
int sceIoDread(int fd, struct dirent *de); 
void sceIoDclose(int fd);


void sceAudioOutputBlocking();//
void sceAudioOutputPanned();//
long sceAudioOutputPannedBlocking(long, long, long, void *);//
long sceAudioChReserve(long, long samplecount, long);//init buffer? returns handle, minus if error
void sceAudioChRelease(long handle);//free buffer?
void sceAudioGetChannelRestLen();//
long sceAudioSetChannelDataLen(long, long);//
void sceAudioChangeChannelConfig(int ch,int mode);//
void sceAudioChangeChannelVolume();//

void sceKernelExitGame();
int  sceKernelCreateThread(const char *name, pg_threadfunc_t func, unsigned long priority, unsigned long stacksize, unsigned long attrib, void *unk);
int  sceKernelStartThread(int hthread, int arg0, void *arg1);
void sceKernelExitThread(int ret);
int  sceKernelWaitThreadEnd(int hthread, void *unk);
int  sceKernelDeleteThread(int hthread);
int  sceKernelCreateCallback(const char *name, void* func, void *arg);

int sceKernelSuspendThread(int tid);
int sceKernelResumeThread(int tid);
int sceKernelSleepThread(void);
int sceKernelWakeupThread(int tid);
void sceKernelSetExitCallback(int cbid);


void scePowerRegisterCallback(int zero, int cbid);
long scePowerSetClockFrequency(long,long,long);

unsigned long sceKernelLibcClock(void);
unsigned long sceKernelLibcTime(unsigned long *);

int sceDmacMemcpy(void* dst,void* src,int size);

int sceExitSetCallback(int);
int KernelPollCallbacks(void);

void sceKernelDcacheWritebackAll(void);
void sceKernelDcacheWritebackInvalidateAll(void);
void sceKernelDcacheWritebackRange(void *adr,unsigned int size);
void sceKernelDcacheWritebackInvalidateRange(void *adr,unsigned int size);

int sceKernelCpuSuspendIntr(void); 
int sceKernelCpuResumeIntr(int oldstat); 
int sceKernelGetThreadId(void); 
int sceKernelCreateSema(const char *s, int flg, int val, int max, const void *p); 
int sceKernelDeleteSema(int id); 
int sceKernelWaitSema(int id, int count, void *p); 
int sceKernelPollSema(int id, int count); 
int sceKernelSignalSema(int id, int count);

// Ctrl Function
void sceCtrlSetSamplingCycle(int unknown); 
void sceCtrlSetSamplingMode(int on); 
void sceCtrlReadBufferPositive(ctrl_data_t* paddata, int unknown); 
void sceCtrlPeekBufferPositive(ctrl_data_t* paddata, int unknown);


// GE FUNCTION
//int sceGeDrawSync(int type);
//int sceGeListSync(int id,int type);
//int sceGeSetCallback(void*);
//int sceGeListEnQueue(void* pAdr,void *pData,int cbid, void* pOpt);

#endif//PSPSDK/////////////////////////////////////////////////////////////////

#define MSB_UP          (27)
#define MSB_RIGHT       (26)
#define MSB_DOWN        (25)
#define MSB_LEFT        (24)

#define MSB_LTRIGGER    (23)
#define MSB_RTRIGGER    (22)

#define MSB_TRIANGLE    (19)
#define MSB_CIRCLE      (18)
#define MSB_CROSS       (17)
#define MSB_SQUARE      (16)

#define MSB_SELECT      (31)
#define MSB_START       (28)


#endif//_SYSCALL_H_INCLUDED
