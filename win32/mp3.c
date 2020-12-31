#include "syscall.h"
#include "pg.h"
#include "..\libs\libmad\mad.h"
#include "main.h"
#include "hal.h"
#include "SDL.h"
#include "SDL_thread.h"
#include "SDL_audio.h"
#include "hal.h"
#include "cstring.h"

#include <stdio.h>
#include <string.h>

static FBFORMAT fb_format;
char mp3_name[1024],old_name[1024];

//char *Mp3Name(int track);
//static void MP3_DecodeThread(void);
//static void MP3_PlayThread(void);
//#define PRI_MP3PLAY   0x12
//#define PRI_MP3DECODE 0x21

struct madbuffer {
    unsigned char const *start;
    unsigned long length;
};

//#define MP3_UNDEF     0
//#define MP3_STOP     -1
//#define MP3_RESUME   -2
//#define MP3_PLAY      1
//#define MP3_PAUSE     2

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
//static int mp3_volume=0x8000;

//static int mp3_playing=0;
//static int mp3_request=0;
//static unsigned int mp3_time=0;

//static int mp3_ply_slpcnt=0;
//static int mp3_dec_slpcnt=0;
//static int mp3_ply_slp=0; // play thread sleep status
//static int mp3_dec_slp=0; // decode thread sleep status
//static int   mp3_file_flag = 0;
//static int   mp3_file_size = 0;
//static char* mp3_file_name = 0;
//static byte *mp3_file_buff = 0;
//static int   mp3_file_id   =-1;


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

void MP3_GetBuffer(u16* dst,int length)
{
	int i,w;
	short* s;

	length/=4;

	if((s=bufGetLock(length))) {
		for(i=0;i<length*2;i++) {
			w = (int)s[i] + (int)dst[i] ;
			if(w> 32766) w= 32766; else
			if(w<-32766) w=-32766;
			dst[i]=w;
		}

		bufGetUnlock(s,length);
	} else {
		length = length;
	}
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
    
    mad_stream_buffer(stream, buffer->start, buffer->length);
    
    if(!mp3_loop) {
        buffer->length=0;
    }

//    mp3_time=0;
    
    return MAD_FLOW_CONTINUE;
}

// convert MAD format to signed short WAVE format
static signed int scale(mad_fixed_t sample)
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

	if(core_strcmp(mp3_name,"unloaded")==0) {
		return MAD_FLOW_STOP;
	}
    
//	mp3_time += nsamples;

    // Ring Buffer fullまで書き込むと音が割れる
    while(bufLen()>=(SND_RNGSIZE-nsamples)){
		SDL_Delay(0);
    }

	swr = snd_wr;

	while(nsamples--) {
        if(nchannels==2) {
            sndbuffer[swr][0] = (signed short)scale(*right_ch++);
            sndbuffer[swr][1] = (signed short)scale(*left_ch++);
        } else {
            sndbuffer[swr][0] = 
            sndbuffer[swr][1] = (signed short)scale(*left_ch++);
        }
        
        swr = (swr+1)%SND_RNGSIZE;
    }

SDL_LockAudio();
	snd_wr = swr;
SDL_UnlockAudio();

//    mp3_play_wakeup();
    
    return MAD_FLOW_CONTINUE;
}

static enum mad_flow error(void *data,
                           struct mad_stream *stream,
                           struct mad_frame *frame)
{
//    struct buffer *buffer = data;
    return MAD_FLOW_CONTINUE;
}


void MP3_Thread(void* pIN)
{
    struct madbuffer buffer;
    struct mad_decoder decoder;
	u8 * mem;
	int size;
	int result,br=0;
	FILE* fp;

	core_strcpy(mp3_name,"unloaded");

	while(1) {		
		size = 16*1024*1024;
		mem = malloc(size);

		core_strcpy(old_name,"unloaded");

		while(1) {
			if(core_strcmp(old_name,mp3_name)) {
				break;
			}
			SDL_Delay(1);
		}
    
		if((fp = fopen(mp3_name/*"D:\\pspe\\ms0\\roms\\ys\\03.mp3"*/,"rb"))) {
			size = fread(mem,1,size,fp);
			fclose(fp);
		}

		buffer.start = mem;
		buffer.length= size;
          
		mad_decoder_init(&decoder, &buffer,
                         input, 0 /* header */, 0 /* filter */, output,
                         error, 0 /* message */);

            
		result = mad_decoder_run(&decoder, MAD_DECODER_MODE_SYNC);
		mad_decoder_finish(&decoder);

		free(mem);
		core_strcpy(mp3_name,"unloaded");

	}

}

void MP3_StartThread(void)
{
	SDL_CreateThread(MP3_Thread,0);
}

/*
// 
// 渡されたバッファに存在するMP3データを再生
// 
int MP3_PlayBuffer(BYTE* pBuffer,int size)
{
    if(1) {
        mp3_file_flag = 0;
        mp3_file_size = size;
        mp3_file_buff = pBuffer;
        mp3_request   = MP3_PLAY;
    }
    return 1;
}
*/

/*
// 
// 指定トラックを再生する
// 
int MP3_PlayTrack(int track,int bLoop)
{
    if(MP3_WaitIdle()) {
        mp3_file_flag = 1;
        mp3_file_name = Mp3Name(track);
        mp3_file_size = 0;
        mp3_file_buff = 0;
        mp3_request   = MP3_PLAY;
    }
    return 1;
}
*/

/*
// 再生を停止する
int MP3_PlayStop(void)
{
    if(MP3_WaitIdle()) {
    }
    return 1;
}
*/
/*
// 再生を停止する
void MP3_PlayPause(void)
{
    if(mp3_playing) {
        mp3_request=MP3_PAUSE;
    }
}
*/
/*
// 再生を再開する
void MP3_PlayResume(void)
{
    if(mp3_playing) {
        if(mp3_request==MP3_PAUSE) {
            mp3_request=MP3_RESUME;
        }
    }
}
*/

/*
// ボリューム変更
void MP3_PlayVolume(int vol)
{
    if(vol<0x0000) { mp3_volume=0x0000; } else
    if(vol>0x8000) { mp3_volume=0x8000; }
}
*/
/*
int MP3_Init(int nRomSize,byte* pRomAddr)
{
    MP3_PlayBuffer(pRomAddr,nRomSize);
    return 1;
}
*/
