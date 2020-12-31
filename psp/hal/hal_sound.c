#define static
//#define DEF_DELAY  // 定義されていればSleepではなくDelayで動作

//---------------------------------------------------------------------------------------
// make sound buffer
//---------------------------------------------------------------------------------------
#include "syscall.h"
#include "pg.h"

#define    SOUND_THREAD_PRIORITY  (0x12)
#define    SOUND_THREAD_STACK     (0x10000)

#define    SND_L            1
#define    SND_R            0

//------------------------------------------------------------------------------
// Buffer Size
//------------------------------------------------------------------------------
#define    SND_FRMSIZE       736              // 1frame(1/60s) samples
#define    SND_BNKSIZE       512              // buffer read size (hardware requirement)

//#define    SND_UNDERLIMIT   ( 3*SND_FRMSIZE)  // underflow limit
#define    SND_OVERLIMIT    (10*SND_BNKSIZE)  //  5120 byte 
#define    SND_RNGSIZE      (40*SND_BNKSIZE)  // 20480 byte 

//------------------------------------------------------------------------------
// 16bit Sound Ring Buffer
//------------------------------------------------------------------------------
static int   snd_wr=0;                  // Sound Write Pointer
static int   snd_rd=0;                  // Sound Read  Pointer
static short sndbuffer[SND_RNGSIZE][2]; // Sound Ring Buffer
static int   over_wr=0,over_rd=0;       // (Debug) overrun counter
static int   unlk_wr=0,unlk_rd=0;       // (Debug) unlock error

//------------------------------------------------------------------------------
// debug 
//------------------------------------------------------------------------------
static int   snd_vsync_skip=0;          // underflow vsync skip counter
static int   snd_frame_skip=0;          // underflow frame skip counter

// internal function
static int    bufLen(void);
static short *bufGetLock(int size);
static void   bufGetUnlock(void* p,int size);

// thread info
static int snd_thread = -1;
static int snd_handle = -1;
static int snd_volume = 0x8000;

// 
// サウンドスレッドは高優先度なのでコアスレッド動作中は
// (1) ブロッキング中
// (2) SLEEP中
// のいずれかの状態であることは確定している。
// 
// ブロッキング中はWAKEUPさせる必要がない。
//
static int snd_sleep=0; // thread sleep status

static void hal_snd_sleep(void)
{
    snd_sleep = 1;
    sceKernelSleepThread(); // if no sound, sleep...
    snd_sleep = 0;
}

static void hal_snd_wakeup(void)
{
    if(snd_sleep) {
        sceKernelWakeupThread(snd_thread);
    }
}

//------------------------------------------------------------------------------
// 再生スレッドループ
//------------------------------------------------------------------------------
static int hal_sound_thread(int args, void *argp)
{
    short* src=0;
    int buflen;
    
    while(1) {

        if((buflen = bufLen())<SND_BNKSIZE) {
#ifdef DEF_DELAY
            sceKernelDelayThread(1000); /* (usec) */
#else
            hal_snd_sleep();
#endif
            continue;
        }

        src = bufGetLock(SND_BNKSIZE);

        if(src) {
            sceAudioOutputPannedBlocking(snd_handle,snd_volume,snd_volume,src);
            bufGetUnlock(src,SND_BNKSIZE);
            src=0;
        }
    }

    return 0;
}

//------------------------------------------------------------------------------
// bufferの長さをゲッツ
//------------------------------------------------------------------------------
static int bufLen(void)
{
    int rd=snd_rd;
    int wr=snd_wr;
    if(wr==rd) return 0;
    if(wr >rd) return wr-rd;;
    return SND_RNGSIZE - rd + wr;
}

int sndBufLen(void)
{
    return bufLen();
}

//------------------------------------------------------------------------------
// Sound Ring Bufferから音データを拾う処理
//------------------------------------------------------------------------------
static short *bufGetLock(int size)
{
    if(bufLen()>=size) { // 必要以上のデータがあるか？
        return sndbuffer[snd_rd];
    }
    over_rd++;
    return 0;
}

//------------------------------------------------------------------------------
// [Read] 
//------------------------------------------------------------------------------
static void bufGetUnlock(void* ptr,int size)
{
    if(ptr==(void*)&sndbuffer[snd_rd]) {
        snd_rd=(snd_rd+size)%SND_RNGSIZE;
    } else {
        unlk_rd++;
    }
}


//------------------------------------------------------------------------------
// 飽和演算で比較しない処理方法
//------------------------------------------------------------------------------
static int saturate(int high, int low, int n)
{
    if( n>high ) { return high; }
    if( n<low  ) { return low;  }
    return n;
}


//------------------------------------------------------------------------------
// Init Parameter
//------------------------------------------------------------------------------
int HAL_Sound_Init(void)
{
    snd_wr=snd_rd=0;
    core_memset(sndbuffer,0,sizeof(sndbuffer));

    if ((snd_handle=sceAudioChReserve(-1,SND_BNKSIZE,0))<0) {
        snd_handle = -1;
        return -1;
    }

    snd_thread = sceKernelCreateThread("emu_snd0",
                                       (pg_threadfunc_t)&hal_sound_thread,
                                       SOUND_THREAD_PRIORITY,
                                       SOUND_THREAD_STACK,
                                       0,NULL);

    if(snd_thread<0) {
        return -2;
    }
    
    sceKernelStartThread(snd_thread,0,0);

    return 1;
}


//------------------------------------------------------------------------------
// Cleanup Parameter
//------------------------------------------------------------------------------
void HAL_Sound_Close(void)
{
    if(snd_handle>=0) {
        sceAudioChRelease(snd_handle);
        snd_handle=-1;
    }

    if(snd_thread>=0) {
        sceKernelStartThread(snd_thread,0,0);
        snd_thread=-1;
    }
}


//------------------------------------------------------------------------------
// Emulatorから呼び出される関数
// 直接16bitのデータとしてサウンドバッファに登録する
//------------------------------------------------------------------------------
void HAL_Sound_Proc16(s16* waveR,s16* waveL,int nSamples)
{
    int i;
    int swr=snd_wr;
    
    for(i=0;i<nSamples;i++) {
        sndbuffer[swr][SND_R] = waveR[i];
        sndbuffer[swr][SND_L] = waveL[i];
        swr = (swr+1) % SND_RNGSIZE;
        
        waveR[i]=0;
        waveL[i]=0;
    }

    // buffer overflow limit 未満なら追加
    if(bufLen()<SND_OVERLIMIT) {
        snd_wr = swr;
    }
    
#if !defined(DEF_DELAY)
    if(PSP_Is()){
        hal_snd_wakeup();
    }
#endif

}

//------------------------------------------------------------------------------
// Emulatorから呼び出される関数
// 32bit -> 16bit 変換を行う
//------------------------------------------------------------------------------
void HAL_Sound_Proc32(s32* waveR,s32* waveL,int nSamples)
{
    int i;
    int swr=snd_wr;
    
    for(i=0;i<nSamples;i++) {
        sndbuffer[swr][SND_R] = (short)saturate(32766,-32766,waveR[i]);
        sndbuffer[swr][SND_L] = (short)saturate(32766,-32766,waveL[i]);
        swr = (swr+1) % SND_RNGSIZE;
        
        waveR[i]=0;
        waveL[i]=0;
    }

    // buffer overflow limit 未満なら追加
    if(bufLen()<SND_OVERLIMIT) {
        snd_wr = swr;
    }
    
#if !defined(DEF_DELAY)
    if(PSP_Is()){
        hal_snd_wakeup();
    }
#endif
}

//------------------------------------------------------------------------------
// Emulatorから呼び出される関数
// 32bit -> 16bit 変換を行う
//------------------------------------------------------------------------------
void HAL_Sound_Proc32m(s32* wave,int nSamples)
{
    int i;
    int swr=snd_wr;
    
    for(i=0;i<nSamples;i++) {
        sndbuffer[swr][SND_R] = 
        sndbuffer[swr][SND_L] = (short)saturate(32766,-32766,wave[i]);
        swr = (swr+1) % SND_RNGSIZE;
        
        wave[i]=0;
    }

    // buffer overflow limit 未満なら追加
    if(bufLen()<SND_OVERLIMIT) {
        snd_wr = swr;
    }
    
#if !defined(DEF_DELAY)
    if(PSP_Is()){
        hal_snd_wakeup();
    }
#endif
}