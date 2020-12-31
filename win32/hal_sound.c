//---------------------------------------------------------------------------------------
// make sound buffer
//---------------------------------------------------------------------------------------
//#include "string.h"
#include "syscall.h"
#include "pg.h"
//#include "main.h"

#include "SDL.h"
#include "SDL_audio.h"
#include "hal.h"
#include "cstring.h"

static int prev_skip_frame=0;
void   pgaSetChannelCallback(int channel, void *callback);

#define    SND_L            1
#define    SND_R            0

//------------------------------------------------------------------------------
// Buffer Size
//------------------------------------------------------------------------------
#define    SND_FRMSIZE       736              // 1frame(1/60s) samples
#define    SND_BNKSIZE       512              // buffer read size (hardware requirement)
//#define    SND_UNDERLIMIT   (10*SND_FRMSIZE)  // underflow limit
#define    SND_OVERLIMIT   (40*SND_BNKSIZE)  //  5120 byte 
#define    SND_RNGSIZE     (80*SND_BNKSIZE)  // 20480 byte 

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
static int   snd_sound_skip=0;          // sound write skip (overflow limit check)
static int   snd_vsync_skip=0;          // underflow vsync skip counter
static int   snd_frame_skip=0;          // underflow frame skip counter

// internal function
static int    bufLen(void);
static short *bufGetLock(int size);
static void   bufGetUnlock(void* p,int size);

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

static short saturate(int wav)
{
	if(wav<-32766) return -32766;
	if(wav> 32766) return  32766;
	return (short)wav;
}

//------------------------------------------------------------------------------
// Emulatorから呼び出される関数
//------------------------------------------------------------------------------
void HAL_Sound_Proc32(int* waveR,int* waveL,int nSamples)
{
    int i,wave;
    int swr;
	
	swr = snd_wr;
    
    for(i=0;i<nSamples;i++) {
        sndbuffer[swr][SND_R] = saturate(waveR[i]);
        sndbuffer[swr][SND_L] = saturate(waveL[i]);
        swr = (swr+1) % SND_RNGSIZE;
        waveR[i]=0;
        waveL[i]=0;
    }
    
    // buffer overflow limit 未満なら追加
    if(bufLen()<SND_OVERLIMIT) {
		SDL_LockAudio();
        snd_wr = swr;
		SDL_UnlockAudio();
    }else {
        snd_sound_skip++;
    }
    
}

void HAL_Sound_Proc16(s16* waveR,s16* waveL,int nSamples)
{
    int i,wave;
    int swr;
	
	swr = snd_wr;
    
    for(i=0;i<nSamples;i++) {
        wave = waveR[i];
        sndbuffer[swr][SND_R] = waveR[i];
        sndbuffer[swr][SND_L] = waveL[i];
        swr = (swr+1) % SND_RNGSIZE;
        waveR[i]=0;
        waveL[i]=0;
    }
    
    // buffer overflow limit 未満なら追加
    if(bufLen()<SND_OVERLIMIT) {
		SDL_LockAudio();
        snd_wr = swr;
		SDL_UnlockAudio();
    }else {
        snd_sound_skip++;
    }
    
}

void HAL_Sound_Proc32m(s32* waveR,int nSamples)
{
    int i,wave;
    int swr;
	
	swr = snd_wr;
    
    for(i=0;i<nSamples;i++) {
        wave = waveR[i];
        sndbuffer[swr][SND_R] = saturate(wave);
        sndbuffer[swr][SND_L] = saturate(wave);
        swr = (swr+1) % SND_RNGSIZE;
        waveR[i]=0;
    }
    
    // buffer overflow limit 未満なら追加
    if(bufLen()<SND_OVERLIMIT) {
		SDL_LockAudio();
        snd_wr = swr;
		SDL_UnlockAudio();
    }else {
        snd_sound_skip++;
    }
    
}

void MP3_GetBuffer(u16* dst,int length);

void mix_audio(void* unused,Uint8 *stream8, int len)
{
	short* p=0,*s=0;

	p = (short*)stream8;

	if((s=bufGetLock(len/4))) {
		memcpy(p,s,len);
		bufGetUnlock(s,len/4);
	} else {
		core_memset(stream8,0,len);
	}

	MP3_GetBuffer(p,len);


}

int HAL_Sound_Init(void)
{
	SDL_AudioSpec fmt,spc;

	core_memset(&fmt,0,sizeof(fmt));
	core_memset(&spc,0,sizeof(spc));

	fmt.freq     = 44100;
	fmt.format   = AUDIO_S16;
	fmt.channels = 2;
	fmt.samples  = 512;
	fmt.callback = mix_audio;
	fmt.userdata = 0;

	if(SDL_OpenAudio(&fmt,&spc)<0) {
		exit(0);
		return -1;
	}

	MP3_StartThread();

	// init
    snd_wr=snd_rd=0;
    core_memset(sndbuffer,0,sizeof(sndbuffer));

	SDL_PauseAudio(0);

	return 1;
}
