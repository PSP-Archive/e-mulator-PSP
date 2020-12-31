#include "stdafx.h"


//--------------------------------------------------------
// PSP�A�v����Windows��Ńf�o�b�O���Ȃ���J�����邽�߂̃��m
// 
// ��{�I�ȕ�����Windows�Ńf�o�b�O�������������������B
// �Ƃ����l�̂��߂̃��m
//--------------------------------------------------------
// ��PSP�A�v����WIN32 API���o���o���g���ƈӖ����Ȃ��̂Œ��ӁB
//--------------------------------------------------------
// �����ۏ؂�����̂ł͂Ȃ��̂ł�낷�R
//--------------------------------------------------------
// �I�������Ȃǂ�PSP�p�v���O�����̎����҂܂����B
// �I���R�[���o�b�N�o�^���Ă�ΏI�������͓�������(�K��������)
//--------------------------------------------------------
// File  I/O   : ���Ԃ񓮍�
// DIR   I/O   : ���Ԃ񓮍�
// VRAM        : ���Ԃ񓮍�
// Flip Screen : ���Ԃ񓮍�
// Vsync       : ���Ԃ񓮍�
// Audio       : �܂Ƃ��ɓ����Ȃ�
// ExitCallback: ���Ԃ񓮍�
// Key Input   : ���Ԃ񓮍�
// GPU         : ��������
//--------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <io.h>
#include <fcntl.h>
#include <string.h>
#include <windows.h>
#include <time.h>

#include "sdl.h"
#include "SDL_thread.h"
#include "SDL_audio.h"
#include "sceWrapper.h"

// �R�[���o�b�N�����֐�
void (*pFuncExitCallback)(void) = 0;
void (*pFuncPowerCallback)(void) = 0;

// internal function 
int sdl_Event(void);

////////////////////////////////////////////////////
// �ǂ����炩�p�N���Ă����֐�
// Vsync�҂����[���I�ɍ��o�����߂̊֐��ŃS�U��


////////////////////////////////////////////////////
// �C�x���g�����֐�
////////////////////////////////////////////////////
int sdl_Event(void)
{
	SDL_Event event;
	SDL_PollEvent(&event);
	
	switch (event.type) {
	//case SDL_KEYDOWN:
    //    printf("The %s key was pressed!\n", SDL_GetKeyName(event.key.keysym.sym));
    //break;
	case SDL_QUIT:
		if(pFuncExitCallback) {
			pFuncExitCallback();
		} 
		// �����I���ł�
		ExitProcess(0);
		return -1;
	break;
	}
	return 0;
}

////////////////////////////////////////////////////
SDL_Surface *sdl_screen=0;

void sdl_graph_init()
{
    if ( SDL_Init(SDL_INIT_EVERYTHING) < 0 ) {
        exit(1);
    }
	
	sdl_screen = SDL_SetVideoMode(512, 272, 15, SDL_DOUBLEBUF | SDL_SWSURFACE);
    if ( sdl_screen == NULL ) {
        fprintf(stderr, "Unable to set 512x272 video: %s\n", SDL_GetError());
        exit(1);
    }
}

void sdl_graph_close()
{

}

static char pg_vramtop[2*1024*1024];

void *sceGeEdramGetAddr(void)
{
	return pg_vramtop;
}

////////////////////////////////////////////////////////////////////////
// Thread Wrapper
// �ő�100�X���b�h����悤�ɂ���
////////////////////////////////////////////////////////////////////////
typedef int (*pg_threadfunc_t)(int args, void *argp);

int          _sdl_thd_num=0;
SDL_Thread * _sdl_thd[100];
pg_threadfunc_t _sdl_thd_func[100];
int          _sdl_thd_arg0[100];
int          _sdl_thd_arg1[100];

int sdl_thread_close(void)
{
	int i;
	for(i=0;i<100;i++) {
		if(_sdl_thd[i]) {
			SDL_KillThread(_sdl_thd[i]);
			_sdl_thd[i] = 0;
		}
	}

	return 0;
}

int thd_func(int tid)
{
	if(_sdl_thd_func[tid]) {
		(_sdl_thd_func[tid])(_sdl_thd_arg0[tid],&_sdl_thd_arg1[tid]);
	}
	return 0;
}

//
// �X�^�b�N�|�C���^�Ƃ��v���C�I���e�B�Ƃ��Z�b�g���邯��SDL�ł͎����ł����B
// �֐��|�C���^���������z��ɓo�^���āA�o�^index��TID�Ƃ��ĕԂ��B
int  sceKernelCreateThread(const char *name, pg_threadfunc_t func, unsigned long priority, unsigned long stacksize, unsigned long attrib, void *unk)
{
	int ret;
	_sdl_thd_func[_sdl_thd_num] = func;
	ret = _sdl_thd_num;
	_sdl_thd_num++;
	return ret;
}

// sceKernelCreateThread�̖ߒl�œn�����l��TID�Ƃ��ăX���b�h�N��������͂��Ȃ̂�
// ���̏������Ɋ֐��|�C���^�z����Q�Ƃ��ăX���b�h���N�����Ă܂��B
int  sceKernelStartThread(int hthread, int arg0, void *arg1)
{
	_sdl_thd_arg0[hthread] = arg0;
	_sdl_thd_arg1[hthread] = (arg1)? *(int*)arg1 : 0;
	_sdl_thd[hthread] = SDL_CreateThread(thd_func,hthread);
	return (_sdl_thd[hthread]!=0)?0:-1;
}

int sceKernelWakeupThread(int tid)
{
	return 1;
}

int sceKernelSleepThread(void)
{
	SDL_Delay(1);
	return 1;
}

////////////////////////////////////////////////////////////////////////


void KernelPollCallbacks()
{
}

void sceCtrlSetAnalogMode()
{

}

void sceCtrlInit()
{
}

void scePowerRegisterCallback()
{
}

void sceExitSetCallback()
{
}

void sceKernelExitGame()
{
}


////////////////////////////////////////////////////////////////////////
// FILE I/O
////////////////////////////////////////////////////////////////////////
int sceIoOpen(char *name,int flag,int mode)
{
	int ret;
	char sn[1024];
	int f=_O_BINARY;

	strcpy(sn,MS_BASE);
	strcat(sn,&name[4]);
	//ret = _open(sn,flag | O_BINARY , mode);

	switch(flag&0x0f){
	case 0x01: f |= _O_RDONLY; break;
	case 0x02: f |= _O_WRONLY; break;
	case 0x03: f |= _O_RDWR;   break;
	}

	if(flag&0x0200) f |= _O_CREAT;
	if(flag&0x0400) f |= _O_TRUNC;

	ret = _open(sn,f , mode);
	return ret;
}

int sceIoClose(int fd)
{
	int ret = _close(fd);
	return ret;
}

int sceIoRead(int fd,void* buf,int length)
{
	int ret = _read(fd,buf,length);
	return ret;
}

int sceIoLseek(int fd,int pos,int base)
{
	return _lseek(fd,pos,base);
}

int sceIoWrite(int fd,void* p,int n)
{
	int ret = _write(fd,p,n);
	return ret;
}

////////////////////////////////////////////////////////////////////////
// Directory Wrapper
////////////////////////////////////////////////////////////////////////
int finded = 0;
struct _finddata_t c_file;

int sceIoDopen(const char *fn)
{
	int fd;
	char sn[512];

	strcpy(sn,MS_BASE);
	strcat(sn,&fn[4]);
	strcat(sn,"\\*.*");

	fd  = _findfirst(sn,&c_file);
	if(fd>=0) { 
		finded=1;
		return fd;
	}
	finded=0;
	return -1;
}

void sceIoDclose(int fd)
{
	_findclose(fd);
}

int sceIoDread(int fd, struct dirent *de)
{
	int ret=0;
	ret = (finded) ? 0 : _findnext(fd,&c_file);
	finded=0;

	if(ret!=0) return -1;

	de->type = (c_file.attrib&0x10) ? TYPE_DIR : TYPE_FILE;

	strcpy(de->name,c_file.name);
	return 1;
}



////////////////////////////////////////////////////////////////////////
// Input Interface
////////////////////////////////////////////////////////////////////////
int sceCtrlPeek(ctrl_data_t *dat,int num)
{
	Uint8 *keystate;
	memset(dat,0,sizeof(ctrl_data_t));

	keystate = SDL_GetKeyState(NULL);

	if(keystate[SDLK_UP])    dat->buttons |= 0x0010; //#define CTRL_UP         0x0010 
	if(keystate[SDLK_DOWN])  dat->buttons |= 0x0040; //#define CTRL_DOWN      0x0040 
	if(keystate[SDLK_LEFT])  dat->buttons |= 0x0080; //#define CTRL_LEFT      0x0080 
	if(keystate[SDLK_RIGHT]) dat->buttons |= 0x0020; //#define CTRL_RIGHT      0x0020 
	if(keystate[SDLK_F1])    dat->buttons |= 0x0008; //#define CTRL_START      0x0008 
	if(keystate[SDLK_F2])    dat->buttons |= 0x0001; //#define CTRL_SELECT      0x0001 
	if(keystate[SDLK_RSHIFT])dat->buttons |= 0x0200; //#define CTRL_RTRIGGER   0x0200 
	if(keystate[SDLK_LSHIFT])dat->buttons |= 0x0100; //#define CTRL_LTRIGGER   0x0100 

	if(keystate[SDLK_a])     dat->buttons |= 0x8000; //#define CTRL_SQUARE      0x8000 
	if(keystate[SDLK_w])     dat->buttons |= 0x1000; //#define CTRL_TRIANGLE   0x1000 
	if(keystate[SDLK_s])     dat->buttons |= 0x4000; //#define CTRL_CROSS      0x4000 
	if(keystate[SDLK_d])     dat->buttons |= 0x2000; //#define CTRL_CIRCLE      0x2000 

	return 1;
}

int sceCtrlRead(ctrl_data_t* dat, int num)
{
	return sceCtrlPeek(dat,num);
}


////////////////////////////////////////////////////////////////////////
// Audio Interface
////////////////////////////////////////////////////////////////////////
// ����...�ǂ��������邩����肾��...
// [1] SDL����R�[���o�b�N�֐������s�����͉̂䖝����B
// [2] �u���b�L���O�֐����g��Ȃ��v���O���������e���Ȃ��B
// [3] �I�[�f�B�I�o�b�t�@�͖ʓ|�Ȃ̂Ń����O�ŊǗ����Ȃ��B
//     buffer�̓��b�N����������l���ӔC�������đO�Ɉړ����邱��
//

#define BANK   (512*736)

#define ARING (1024*10)           // audio ring 10240(word)
int   sdl_audio_ch[8];            // 8ch�̃��U�[�u��
short sdl_audio_buffer[BANK][2]; // 8ch����Audio buffer : �����O�ɂ���̖ʓ|����...
int   sdl_audio_buflen[8];
int   sdl_audio_len[8];           


int bankw=0;
int bankr=0;

int bnkCnt(void)
{
	if(bankw==bankr) return 0;
	if(bankw>bankr)  return (bankw-bankr);
	return (BANK+bankw-bankr);

}

//SDL_mutex * sdl_audio_mutex=0;


// �u���b�L���O���I�[�f�B�I�f�[�^���L���[�ɐς�
int sceAudioOutputPannedBlocking(int handle,int vol1,int vol2,void* buf)
{
#if 1
	short * ptr = (short*)buf;
	int len = 736;
	int nbank;

	while(len) {
		nbank = (bankw+1)%BANK;

		// �����Ȃ��ꍇ�͓ǂݏo����҂�
		while(nbank==bankr) { SDL_Delay(1); }
			
		//SDL_LockAudio();
		sdl_audio_buffer[bankw][0] = *ptr++;
		sdl_audio_buffer[bankw][1] = *ptr++;
		bankw=nbank;
		//SDL_UnlockAudio();
		len--;
	}

	while(1) {
		if( bnkCnt()>(1024*4)) {
			SDL_Delay(1);
		} else {
			break;
		}
	}

#else
	// �����������Ƃ���Ƃ��ɍX�V��̃o���N�������ɂȂ�ꍇ�͏������܂Ȃ�
	int nbank=(bankw+736)%BANK;
	
	while(1) {
		if( bnkCnt() <= 736*2 ) {
		//if( bnkCnt() < (BANK-736) ) {
			break;
		}
		SDL_Delay(1);
	}

	SDL_LockAudio();
	memcpy(sdl_audio_buffer[bankw],buf,736*4);
	bankw=nbank;
	SDL_UnlockAudio();
#endif
	return 1;
}

DWORD tim = 0;

// �I�[�f�B�I�̃����O�o�b�t�@���Q�Ƃ��ĉ�������������
void mixaudio(void* unused,Uint8 *stream8, int len)
{
	DWORD t,t1;
	DWORD t2 = GetTickCount();
#if 1
	int length = len/4;
	short * ptr = (short*)stream8;
	int nbank;

	memset(stream8,0,len);

	while(length) {
		while(bankr==bankw) {
			SDL_Delay(1);
		}

		*ptr++ = sdl_audio_buffer[bankr][0];
		*ptr++ = sdl_audio_buffer[bankr][1];

		bankr = (bankr+1)%BANK;
		length--;
	}


#else
	// �I�[�f�B�I�f�[�^���o���Ă��Ȃ���Ή������Ȃ�
	while(bnkCnt()<(len/4)) {
		SDL_Delay(1);
	}
	//if(bnkCnt()<512) return;
	SDL_MixAudio(stream8, &sdl_audio_buffer[bankr],len, SDL_MIX_MAXVOLUME);
	//memcpy(stream8,sdl_audio_buffer[bankr],len);
	bankr = (bankr+(len/4))%BANK;
#endif
	
	t1 = GetTickCount();

	t = t1 - t2;
	tim += t;	
	printf("[%8d : %8d (%8d) %8d]\r",bankw,bankr,bnkCnt(),tim);
}


//---------------
//
//---------------
int sdl_audio_init(void)
{
	SDL_AudioSpec fmt;
	SDL_AudioSpec spec;

//	sdl_audio_mutex = SDL_CreateMutex();

	memset(sdl_audio_ch,0,sizeof(sdl_audio_ch));
	memset(sdl_audio_len,0,sizeof(sdl_audio_len));
	memset(sdl_audio_buflen,0,sizeof(sdl_audio_buflen));

	/* 44100hz 16-bit */
	fmt.freq = 44100;
	fmt.format = AUDIO_S16;
	fmt.channels = 2;
	fmt.samples = SAMPLING_MIN;  // �ŏ��T���v������512 (�A�v�����͂���ȏ�ɂ��Ȃ��ƃ}�Y�C)
	fmt.callback = mixaudio;
	fmt.userdata = NULL;

	/* �I�[�f�B�I�f�o�C�X���I�[�v�����ăT�E���h�̍Đ����J�n! */
	if ( SDL_OpenAudio(&fmt, &spec) < 0 ) {
//		fprintf(stderr, "Unable to open audio: %s\n", SDL_GetError());
		return -1;
	}
//	SDL_PauseAudio(0);
	return 0;
}

int sdl_audio_close(void)
{
	SDL_CloseAudio();
	return 0;
}

// ch���I�[�v�����ăn���h����Ԃ��΂���
int sceAudioChReserve(int unk,int sample,int chnum)
{
	int i,num;
	for(i=0;i<8;i++) {
		if(sdl_audio_ch[i]==0) {
			num=i;
			if(chnum==0) chnum = 2;
			else         chnum = 1;
			sdl_audio_ch[num] = chnum;
			sdl_audio_len[num] = sample*chnum*2;
			return num;
		}
	}
	return -1;
}

// �w�肳�ꂽ�z��ԍ���0�ɂ���
int sceAudioChRelease(int id)
{
	if(id>=0 && id<8) {
		sdl_audio_ch[id]=0;
		return 0;
	}
	return -1;
}

int sceAudio_0()
{
	return 1;
}

int sceAudio_1()
{
	return 1;
}

//	return sceAudio_2(pga_handle[channel],vol1,vol2,buf);



int sceAudio_2(int tid,int vol1,int vol2, char *buf)
{
//	SDL_MixAudio(buf, &sounds[i].data[sounds[i].dpos], 512, SDL_MIX_MAXVOLUME);
	return 1;
}

int sceAudio_3(int p1,int p2, int p3)
{
	if(0)
	{
		extern void mixaudio(void *unused, Uint8 *stream, int len);
		SDL_AudioSpec fmt;

		/* 44100hz 16-bit */
		fmt.freq = 44100;
		fmt.format = AUDIO_S16;
		if(p3==0) fmt.channels = 2;
		else      fmt.channels = 1;
		fmt.samples = p2;
		fmt.callback = mixaudio;
		fmt.userdata = NULL;

		/* �I�[�f�B�I�f�o�C�X���I�[�v�����ăT�E���h�̍Đ����J�n! */
		if ( SDL_OpenAudio(&fmt, NULL) < 0 ) {
//			fprintf(stderr, "Unable to open audio: %s\n", SDL_GetError());
			exit(1);
		}
		SDL_PauseAudio(0);
	}

	return 1;
}


int sceAudio_4()
{
	return 1;
}


int sceAudio_5()
{
	return 1;
}

/* �R�[���o�b�N�֘A���܂��߂Ɏ������邩�H
	int cbid = sceKernelCreateCallback("Exit Callback", exitcallback);
	sceExitSetCallback(cbid);
	cbid = sceKernelCreateCallback("Power Callback", powercallback);
    scePowerRegisterCallback(0, cbid);
    KernelPollCallbacks();
*/


// �܂��߂Ɏ�������̂��ʓ|�Ȃ̂ŁA�Ƃ肠����...
int sceKernelCreateCallback(char *name, void (*func)(void) )
{
	if(strcmpi(name,"Exit Callback")==0) { 
		pFuncExitCallback = func;
	}

	return 1;
}

int scePowerSetClockFrequency(int f1,int f2,int f3)
{
	return 1;
}

int sceKernelLibcClock(void)
{
	return GetTickCount();
}

int sceDisplayWaitVblankStart()
{
	return 1;
}

int sceDisplayWaitVblankStartCB()
{
	return 1;
}

int sceDisplayWaitVblank()
{
	return 1;
}

int sceDisplayWaitVblankCB()
{
	return 1;
}

int sceDisplaySetMode()
{
	return 1;
}



int sceDisplaySetFrameBuf(char *topaddr,long linesize,long pixelsize,long flag)
{
//SDL_Surface *screen=0;
	int x,y;
	short *top;
	unsigned short col;


	if(topaddr){
		if ( SDL_MUSTLOCK(sdl_screen) ) {
			if ( SDL_LockSurface(sdl_screen) < 0 ) {
				return 1;
			}
		}

		for(y=0;y<272;y++) {
			short *buf = (short*)sdl_screen->pixels + y*sdl_screen->pitch/2;
			top = (short*)topaddr + y*linesize;
			for(x=0;x<linesize;x++) {
				col = *top++;

				col = ((col    ) & 0x1f)<<10 |
					  ((col>> 5) & 0x1f)<< 5 |
					  ((col>>10) & 0x1f)<< 0 ;
				
				*buf++ = col;
			}
		}

		if ( SDL_MUSTLOCK(sdl_screen) ) {
			SDL_UnlockSurface(sdl_screen);
		}
//		SDL_UpdateRect(sdl_screen, 0, 0, 512, 272);
		SDL_Flip(sdl_screen);

		// SDL�̃C�x���g�������s��
		sdl_Event();
	}

	return 1;
}


int sceKernelDeleteThread()
{
	return 1;
}


int sceKernelWaitThreadEnd()
{
	return 1;
}


int sceKernelExitThread()
{
	return 1;
}

int sceDmacMemcpy(void* s,void* d, int n)
{
	memcpy(s,d,n);
	return 1;
}


