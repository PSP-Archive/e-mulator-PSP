#include "syscall.h"
#include "pg.h"
#include "mad.h"
#include "main.h"
#include "hal.h"

#include <stdio.h>
#include <string.h>

static FBFORMAT fb_format;

char *Mp3Name(int track);

static void MP3_DecodeThread(void);
static void MP3_PlayThread(void);

#define PRI_MP3PLAY   0x12
#define PRI_MP3DECODE 0x21

struct madbuffer {
    unsigned char const *start;
    unsigned long length;
};

#define MP3_UNDEF     0
#define MP3_STOP     -1
#define MP3_RESUME   -2
#define MP3_PLAY      1
#define MP3_PAUSE     2

typedef struct {
    int volume;
    int loop;

    int handle;  // mp3_handle
    int dthread; // mp3_dthread
    int pthread; // mp3_pthread

    int playing;
    int request;
    int timing;
} MP3PLAYINFO;


static int mp3_loop = 1;
static int mp3_volume=0x8000;

int mp3_dthread=-1;
int mp3_pthread=-1;
static int mp3_handle=-1;

static int mp3_playing=0;
static int mp3_request=0;
static unsigned int mp3_time=0;

static int mp3_ply_slpcnt=0;
static int mp3_dec_slpcnt=0;
static int mp3_ply_slp=0; // play thread sleep status
static int mp3_dec_slp=0; // decode thread sleep status
static int   mp3_file_flag = 0;
static int   mp3_file_size = 0;
static char* mp3_file_name = 0;
static byte *mp3_file_buff = 0;
static int   mp3_file_id   =-1;


//------------------------------------------------------------------------------
// Buffer Size
// 1152 : デコード単位
//  512 : 再生単位
// 4608 : 最小公倍数
//------------------------------------------------------------------------------
#define SND_BNKSIZE  (512)                    // 
#define SND_RNGSIZE  (4608*10)               // about 1 sec buffer.

//------------------------------------------------------------------------------
// 16bit Sound Ring Buffer
//------------------------------------------------------------------------------
static int volatile snd_wr=0;           // Sound Write Pointer
static int volatile snd_rd=0;           // Sound Read  Pointer
static short sndbuffer[SND_RNGSIZE][2]; // Sound Ring Buffer

//------------------------------------------------------------------------------
// bufferの長さをゲッツ
//------------------------------------------------------------------------------
static int bufLen(void)
{
    if(snd_wr==snd_rd) return 0;
    if(snd_wr >snd_rd) return snd_wr-snd_rd;
    return SND_RNGSIZE - snd_rd + snd_wr;
}

//------------------------------------------------------------------------------
// Sound Ring Bufferから音データを拾う処理
//------------------------------------------------------------------------------
static short *bufGetLock(int size)
{
    if(bufLen()>=size) { // 必要以上のデータがあるか？
        return sndbuffer[snd_rd];
    }
    return 0;
}

//------------------------------------------------------------------------------
// [Read] 
//------------------------------------------------------------------------------
static void bufGetUnlock(void* ptr,int size)
{
    if(ptr==(void*)&sndbuffer[snd_rd]) {
        snd_rd=(snd_rd+size)%SND_RNGSIZE;
    }
}

static void mp3_play_sleep(void)
{
#if 0
    mp3_ply_slp=1;
    sceKernelSleepThread();
    mp3_ply_slpcnt++;
    mp3_ply_slp=0;
#endif
}

static void mp3_play_wakeup(void)
{
#if 0
    if(mp3_ply_slp) {
        sceKernelWakeupThread(mp3_pthread);
    }
#endif
}

static void mp3_decode_sleep(void)
{
#if 0
    mp3_dec_slp=1;
    sceKernelSleepThread();
    mp3_dec_slp=0;
    mp3_dec_slpcnt++;
#endif
}

static void mp3_decode_wakeup(void)
{
#if 0
    if(mp3_dec_slp) {
        sceKernelWakeupThread(mp3_dthread);
    }
#endif
}

//#############################################################################
//# 
//# MP3 wavedata output thread
//# 
//#############################################################################
static void MP3_PlayThread(void)
{
#if 0
    short* src=0;
    int buflen;

    // Audio Initialize
    if((mp3_handle = sceAudioChReserve(-1,SND_BNKSIZE,0))<0) {
        return ;
    }
    
    while(1) {
        if((buflen = bufLen())<SND_BNKSIZE) {
            mp3_play_sleep();
            continue;
        }
        
        if((src = bufGetLock(SND_BNKSIZE))){
            sceAudioOutputPannedBlocking(mp3_handle,mp3_volume,mp3_volume,src);
            bufGetUnlock(src,SND_BNKSIZE);

            mp3_decode_wakeup();
        }
    }

    // Audio Close
    sceAudioChRelease(mp3_handle);
#endif
}

//-----------------------------------------------------------------------------
// MP3再生に必要な初期化
//-----------------------------------------------------------------------------
int MP3_Setup(void)
{
#if 0
    if(mp3_dthread==-1 || mp3_pthread==-1) {
        mp3_dthread = sceKernelCreateThread("mp3decode",MP3_DecodeThread,PRI_MP3DECODE,0x10000,0,NULL);
        mp3_pthread = sceKernelCreateThread("mp3play"  ,MP3_PlayThread,  PRI_MP3PLAY  ,0x10000,0,NULL);
        
        if(mp3_pthread>=0) {
            sceKernelStartThread(mp3_pthread,0,0);
        }
        
        if(mp3_dthread>=0) {
            sceKernelStartThread(mp3_dthread,0,0);
        }

        return 1;
    }
#endif
    return 0;
}

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
int MP3_Close(void)
{
    return 1;
}

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
static enum mad_flow input(void *data, struct mad_stream *stream)
{
    struct madbuffer *buffer = data;

    if(!buffer->length) {
        return MAD_FLOW_STOP;
    }
    
    mad_stream_buffer(stream, mp3_file_buff, buffer->length);
    
    if(!mp3_loop) {
        buffer->length=0;
    }

    mp3_time=0;
    
    return MAD_FLOW_CONTINUE;
}

// convert MAD format to signed short WAVE format
static inline signed int scale(mad_fixed_t sample)
{
    /* round */
    sample += (1L << (MAD_F_FRACBITS - 16));

    /* clip */
    if (sample >= MAD_F_ONE) sample =  MAD_F_ONE-1;  else
    if (sample < -MAD_F_ONE) sample = -MAD_F_ONE;
    
    /* quantize */
    return sample >> (MAD_F_FRACBITS + 1 - 16);
}

//-----------------------------------------------------------------------------
// mp3デコード結果は可能な限り保持する
// 保持しきれなかった場合はBUFFERが開放されるのを待つ
//-----------------------------------------------------------------------------
static enum mad_flow output(void *data,
                            struct mad_header const *header,
                            struct mad_pcm *pcm)
{
    volatile int swr;
    unsigned int nchannels = pcm->channels;
    unsigned int nsamples  = pcm->length;
    mad_fixed_t const *left_ch  = pcm->samples[0];
    mad_fixed_t const *right_ch = pcm->samples[1];

    if(mp3_request) {
        if(mp3_request==MP3_STOP ||
           mp3_request==MP3_PLAY ) {
            return MAD_FLOW_STOP;
        } else {
            // PAUSEならループする
            if(mp3_request==MP3_PAUSE) {
                while(mp3_request) {
                    switch(mp3_request) {
                      case MP3_PLAY: return MAD_FLOW_STOP;
                      case MP3_STOP: return MAD_FLOW_STOP;
                      case MP3_RESUME:
                        break;
                    }
                }
            }
        }
        mp3_request=0;
    }
    
    mp3_time += nsamples;

    while(nsamples--) {
        if(nchannels==2) {
            sndbuffer[snd_wr][0] = (signed short)scale(*right_ch++);
            sndbuffer[snd_wr][1] = (signed short)scale(*left_ch++);
        } else {
            sndbuffer[snd_wr][0] = 
            sndbuffer[snd_wr][1] = (signed short)scale(*left_ch++);
        }
        
        swr = (snd_wr+1)%SND_RNGSIZE;

        // Ring Buffer fullまで書き込むと音が割れる
        while(bufLen()>=(SND_RNGSIZE-SND_BNKSIZE*2)){
            mp3_play_wakeup();  // fullに近いならwakeupは要らないんじゃない？
            mp3_decode_sleep();
        }
        snd_wr=swr;
    }

    mp3_play_wakeup();
    
    return MAD_FLOW_CONTINUE;
}

static enum mad_flow error(void *data,
                           struct mad_stream *stream,
                           struct mad_frame *frame)
{
//    struct buffer *buffer = data;
    return MAD_FLOW_CONTINUE;
}

//-----------------------------------------------------------------------------
// mp3 play thread
// この関数はMP3再生のスレッドとなる。
// mp3play関数を呼ぶと、あとはコールバック関数が呼ばれるだけ。
//-----------------------------------------------------------------------------
static void MP3_DecodeThread(void)
{
#if 0
    struct madbuffer buffer;
    struct mad_decoder decoder;
    
    mp3_playing = 0;

    while(!PSP_IsEsc()) {

        if(mp3_request==0) {
            sceKernelDelayThread(1000);
            continue;
        }
        
        // この先で処理するのは [再生] [バッファ再生] のみ
        if(mp3_file_flag==1) {
            // ファイル再生の場合はファイルを開く
            mp3_file_id = HAL_fd_open(mp3_file_name,HAL_MODE_READ);

            if(mp3_file_id>=0) {
                mp3_file_size = HAL_fd_size(mp3_file_id);
                mp3_file_buff = malloc(mp3_file_size);

                if(mp3_file_buff) {
                    HAL_fd_read(mp3_file_id,mp3_file_buff,mp3_file_size);
                    buffer.start = mp3_file_buff;
                    buffer.length= mp3_file_size;
                } else {
                    buffer.start = 0;
                    buffer.length= 0;
                }

                HAL_fd_close(mp3_file_id);
                mp3_file_id=-1;
            }
        } else {
            buffer.start = mp3_file_buff;
            buffer.length= mp3_file_size;
        }

        mp3_request=0;
        
        if(buffer.start && buffer.length) {
            int result;
            mp3_playing = 1;
            
            mad_decoder_init(&decoder, &buffer,
                             input, 0 /* header */, 0 /* filter */, output,
                             error, 0 /* message */);

            
            result = mad_decoder_run(&decoder, MAD_DECODER_MODE_SYNC);
            mad_decoder_finish(&decoder);
        }

        if(mp3_file_flag==1) {

            free(buffer.start);
            buffer.start=0;
            buffer.length=0;
            mp3_file_size=0;
            mp3_file_buff=0;
            mp3_file_flag=0;
        }

        // 何もなかったことにする
        mp3_request = 0;
        mp3_playing = 0;
    }
#endif
}

// 
// デコードスレッドがIDLEになるのを待ち続ける
// 
int MP3_WaitIdle(void)
{
#if 0
    if(mp3_playing) {
        mp3_request=MP3_STOP;
        while(mp3_playing) {
            sceKernelDelayThread(10000); /* 10ms */
        }
    }
#endif
    return 1;
}

// 
// 渡されたバッファに存在するMP3データを再生
// 
int MP3_PlayBuffer(BYTE* pBuffer,int size)
{
#if 0
    if(MP3_WaitIdle() || 1) {
        mp3_file_flag = 0;
        mp3_file_size = size;
        mp3_file_buff = pBuffer;
        mp3_request   = MP3_PLAY;
    }
#endif
    return 1;
}

// 
// 指定トラックを再生する
// 
int MP3_PlayTrack(int track,int bLoop)
{
#if 0
    if(MP3_WaitIdle()) {
        mp3_file_flag = 1;
        mp3_file_name = Mp3Name(track);
        mp3_file_size = 0;
        mp3_file_buff = 0;
        mp3_request   = MP3_PLAY;
    }
#endif
    return 1;
}

// 再生を停止する
int MP3_PlayStop(void)
{
#if 0
    if(MP3_WaitIdle()) {
    }
#endif
    return 1;
}

// 再生を停止する
void MP3_PlayPause(void)
{
#if 0
    if(mp3_playing) {
        mp3_request=MP3_PAUSE;
    }
#endif
}

// 再生を再開する
void MP3_PlayResume(void)
{
#if 0
    if(mp3_playing) {
        if(mp3_request==MP3_PAUSE) {
            mp3_request=MP3_RESUME;
        }
    }
#endif
}

// ボリューム変更
void MP3_PlayVolume(int vol)
{
    if(vol<0x0000) { mp3_volume=0x0000; } else
    if(vol>0x8000) { mp3_volume=0x8000; }
}

//#############################################################################
//# 
//# MP3 Main Loop
//# 
//#############################################################################
void run_mp3(void* pRomAddr,int nRomSize)
{
#if 0
    WORD* fb;
    int y=0;
    
    pgCls(0);
    MP3_PlayBuffer(pRomAddr,nRomSize);

    HAL_fb2_init(256,256,&fb_format,HW_NULL);

    while(1) {

        pgFillvram(0);
        HAL_fps(60);
/*        
        y=10;  mh_print(0,y,"MP3 Player",-1);
        y+=10; mh_print(0,y," title  = ",-1);   mh_print    (50,y,HAL_GetRomsPath(),-1);
        y+=10; mh_print(0,y," file   = ",-1);   mh_print_dec(50,y,mp3_file_size,-1);
        y+=10; mh_print(0,y," status = ",-1);   mh_print_dec(50,y,mp3_playing,-1);
        y+=10; mh_print(0,y," time   = ",-1);   mh_print_dec(50,y,mp3_time/44100,-1);
        y+=10; mh_print(0,y," dthread= ",-1);   mh_print_dec(50,y,mp3_dthread,-1);
        y+=10; mh_print(0,y," pthread= ",-1);   mh_print_dec(50,y,mp3_pthread,-1);
        y+=10; mh_print(0,y," slpcnt=  ",-1);   mh_print_dec(50,y,mp3_ply_slpcnt,-1);
        y+=10; mh_print(0,y," snd_wr=  ",-1);   mh_print_dec(50,y,snd_wr,-1);
        y+=10; mh_print(0,y," wnd_rd=  ",-1);   mh_print_dec(50,y,snd_rd,-1);

        y+=10; mh_print(0,y," cnnnnt = ",-1);   mh_print_dec(50,y,cnnnnt,-1);
        y+=10; mh_print(0,y," cnnnnt2= ",-1);   mh_print_dec(50,y,cnnnnt2,-1);
        y+=10; mh_print(0,y," cnnnnt3= ",-1);   mh_print_dec(50,y,cnnnnt3,-1);
        y+=10; mh_print(0,y," cnnnnt4= ",-1);   mh_print_dec(50,y,cnnnnt4,-1);
        y+=10; mh_print(0,y," cnnnnt5= ",-1);   mh_print_dec(50,y,cnnnnt5,-1);
        y+=10; mh_print(0,y," cnnnnt6= ",-1);   mh_print_dec(50,y,cnnnnt6,-1);
        y+=10; mh_print(0,y," cnnnnt7= ",-1);   mh_print_dec(50,y,cnnnnt7,-1);
        y+=10; mh_print(0,y," cnnnnt8= ",-1);   mh_print_dec(50,y,cnnnnt8,-1);
        y+=10; mh_print(0,y," cnnnnt9= ",-1);   mh_print_dec(50,y,cnnnnt9,-1);
*/
        HAL_fb2_bitblt(&fb_format);

        //HAL_WS_Input();

        // MP3再生のためにCBで待つ必要がある
        if(PSP_Is()) {
            sceDisplayWaitVblankStartCB();
        }

        // Volume Control
        //MP3_PlayVolume();
    }

    MP3_PlayStop();
    
    pgCls(0);
#endif
}


int MP3_Init(int nRomSize,byte* pRomAddr)
{
#if 0
    MP3_PlayBuffer(pRomAddr,nRomSize);
#endif
    return 1;
}

int MP3_Loop(void)
{
#if 0
    int y=0;
    
    pgFillvram(0);
    HAL_fps(60);
/*
    y=10;  mh_print(0,y,"MP3 Player",-1);
    y+=10; mh_print(0,y," title  = ",-1);   mh_print    (50,y,HAL_GetRomsPath(),-1);
    y+=10; mh_print(0,y," file   = ",-1);   mh_print_dec(50,y,mp3_file_size,-1);
    y+=10; mh_print(0,y," status = ",-1);   mh_print_dec(50,y,mp3_playing,-1);
    y+=10; mh_print(0,y," time   = ",-1);   mh_print_dec(50,y,mp3_time/44100,-1);
    y+=10; mh_print(0,y," dthread= ",-1);   mh_print_dec(50,y,mp3_dthread,-1);
    y+=10; mh_print(0,y," pthread= ",-1);   mh_print_dec(50,y,mp3_pthread,-1);
    y+=10; mh_print(0,y," slpcnt=  ",-1);   mh_print_dec(50,y,mp3_ply_slpcnt,-1);
    y+=10; mh_print(0,y," snd_wr=  ",-1);   mh_print_dec(50,y,snd_wr,-1);
    y+=10; mh_print(0,y," wnd_rd=  ",-1);   mh_print_dec(50,y,snd_rd,-1);

    y+=10; mh_print(0,y," cnnnnt = ",-1);   mh_print_dec(50,y,cnnnnt,-1);
    y+=10; mh_print(0,y," cnnnnt2= ",-1);   mh_print_dec(50,y,cnnnnt2,-1);
    y+=10; mh_print(0,y," cnnnnt3= ",-1);   mh_print_dec(50,y,cnnnnt3,-1);
    y+=10; mh_print(0,y," cnnnnt4= ",-1);   mh_print_dec(50,y,cnnnnt4,-1);
    y+=10; mh_print(0,y," cnnnnt5= ",-1);   mh_print_dec(50,y,cnnnnt5,-1);
    y+=10; mh_print(0,y," cnnnnt6= ",-1);   mh_print_dec(50,y,cnnnnt6,-1);
    y+=10; mh_print(0,y," cnnnnt7= ",-1);   mh_print_dec(50,y,cnnnnt7,-1);
    y+=10; mh_print(0,y," cnnnnt8= ",-1);   mh_print_dec(50,y,cnnnnt8,-1);
    y+=10; mh_print(0,y," cnnnnt9= ",-1);   mh_print_dec(50,y,cnnnnt9,-1);
*/
    // MP3再生のためにCBで待つ必要がある
    if(PSP_Is()) {
        sceDisplayWaitVblankStartCB();
    }
    pgScreenFlipV();

    
    {
        ctrl_data_t pd;
        sceCtrlReadBufferPositive(&pd,1);
        
        // EXIT text viewer
        if(pd.buttons & CTRL_SELECT) return 1;
    }
#endif
    
    return 0;
}


int MP3_Exit(void)
{
#if 0
    MP3_PlayStop();
#endif
    return 0;
}

