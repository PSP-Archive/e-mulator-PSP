//#define SECRET
#define SILENT

//*****************************************************************************
// 
// WIN32 HAL
// 
//*****************************************************************************
#include "cstring.h"
#include "hal.h"


#include <io.h>
#include <fcntl.h>
#include <stdio.h>

#include "sdl.h"
#include "SDL_framerate.h"
#include "SDL_thread.h"
#include "SDL_audio.h"

FPSmanager g_fps;
static int pic_nX=0;
extern int g_EmuFlag;

// internal

static WORD  hal_fb[0x40000];

extern SDL_Surface *sdl_screen;

extern void pgSetFontFb(void* pfb,int w,int h,int bpp);

u8* pgGetVramAddr(int x,int y)
{
	u8* ptr = sdl_screen->pixels;

	return  ptr + sdl_screen->w * y + x;
}

// 
// 
// 
int HAL_fb2_init(u32 fbw,u32 fbh,FBFORMAT* pFb,u32 type)
{
    pFb->width   = fbw;
    pFb->height  = fbh;
    pFb->bpp     = sizeof(PIXEL_FORMAT); // System dependent
    
    pFb->fb      = (PIXEL_FORMAT*)hal_fb;
    pFb->fb_tmp  = (PIXEL_FORMAT*)hal_fb;
    pFb->pic_x   = 0;
    pFb->pic_y   = 0;
    pFb->pic_w   = fbw;
    pFb->pic_h   = fbh;
    pFb->hardware= type;

    return 1;
}

//*****************************************************************************
// 
//*****************************************************************************
int HAL_fb2_bitblt(FBFORMAT* pFb)
{
	int n,m,y,x,chg=0;
	WORD *pScr,*top;
	static int width=0;
	static int height=0;
	static int onx=0;

    if( pFb->hardware==HW_WSC ) {
        pFb->rotate = isWonderSwanRotate();
    }

	if(pFb->rotate) {
		if( (width!=pFb->pic_h) || (height!=pFb->pic_w) ){
			chg=1;
			width  = pFb->pic_h;
			height = pFb->pic_w;
		}
	} else {
		if( (width!=pFb->pic_w) || (height!=pFb->pic_h) ){
			chg=1;
			width  = pFb->pic_w;
			height = pFb->pic_h;
		}
	}

	if(chg || onx!=pic_nX){
		int vw,vh;

		vw = width*(pic_nX+1)+20;
		vh = height*(pic_nX+1)+20;

		onx = pic_nX;
		sdl_screen = SDL_SetVideoMode(vw,vh, 15, 
													SDL_DOUBLEBUF | 
													//SDL_HWSURFACE |
													SDL_SWSURFACE |
													0
			);

		pgSetFontFb(sdl_screen->pixels,vw,vh,2);
	}

	//
	//
	if ( SDL_MUSTLOCK(sdl_screen) ) {
		if ( SDL_LockSurface(sdl_screen) >= 0 ) {
			
		}
	}

	pScr = (WORD*)sdl_screen->pixels + 10 + (sdl_screen->w * 10);

	if(pFb->rotate) {
		for(y=0;y<pFb->pic_w;y++) {
			for(x=0;x<pFb->pic_h;x++) {
				top = &pFb->fb[ pFb->height * x + pFb->pic_w - y ];
				pScr[x] = *top;
			}
			pScr += sdl_screen->pitch/2;
		}
	}
	else {
		for(y=0;y<pFb->pic_h;y++) {
			top = &pFb->fb[ (y+ pFb->pic_y) * pFb->width + pFb->pic_x ];

			for(n=0;n<(pic_nX+1);n++){
				for(x=0;x<pFb->pic_w;x++) {
					for(m=0;m<(pic_nX+1);m++){
						pScr[x*(pic_nX+1)+m] = top[x];
					}
				}
				pScr += sdl_screen->pitch/2;
			}
		}
	}

//	mh_print_dec(0,0,g_fps.rate,-1);

	if ( SDL_MUSTLOCK(sdl_screen) ) {
		SDL_UnlockSurface(sdl_screen);
	}

	
	SDL_Flip(sdl_screen);
	sdl_Event();          // SDLのイベント処理を行う

	return 1;
}

//-----------------------------------------------------------------------------
// SYSTEM DEPENDENT
//-----------------------------------------------------------------------------
PIXEL_FORMAT HAL_fb2_Color(u8 r,u8 g,u8 b,u8 type)
{
	int color=0;

    switch(type) {
      case RGB888:  b=(b>>3)&0x1f;  g=(g>>3)&0x1f;  r=(r>>3)&0x1f;  break;
      case RGB555:  b=(b   )&0x1f;  g=(g   )&0x1f;  r=(r   )&0x1f;  break;
      case RGB444:  b=(b<<1)&0x1f;  g=(g<<1)&0x1f;  r=(r<<1)&0x1f;  break;
    }

	color = ((r)<<10) | ((g)<<5) | ((b));

#ifdef SECRET // 秘密にこっそり
	color = (color & 0x7bde)>>1;
#endif
	return color;
}


//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void HAL_fb2_close(void)
{

}

//-----------------------------------------------------------------------------
// FPS(frame/sec)を計算する関数
//-----------------------------------------------------------------------------
int HAL_fps(int fps)
{
#if 0
	return 0;
#else
	static int nfps=0;
	int frame;

	if(nfps==0 || fps!=nfps) {
		SDL_initFramerate(&g_fps);
		SDL_setFramerate(&g_fps,fps);
		nfps = fps;
	}

	SDL_framerateDelay(&g_fps);

	if((frame=SDL_getFramerate(&g_fps))<fps) {
		return 1;
	}
	return 0;
#endif
}

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
char* HAL_DataName(char* pDst,char *pName)
{
/*    strcpy(pDst,pguGetWorkdir());
    strcat(pDst,pName);
	*/
	core_strcpy(pDst,"c:\\test.dat");
    return pDst;
}


u32 HAL_Input(u32 player,u32 type)
{
	Uint8 *keystate;
	u32 key=0;

	keystate = SDL_GetKeyState(NULL);

	switch(type){
	case HW_WSC:
		if(isWonderSwanRotate())  {
			if(keystate[SDLK_F1])    key |= 1<<WSC_S;
			if(keystate[SDLK_UP])    key |= 1<<WSC_YR;
			if(keystate[SDLK_RIGHT]) key |= 1<<WSC_YD;
			if(keystate[SDLK_DOWN])  key |= 1<<WSC_YL;
			if(keystate[SDLK_LEFT])  key |= 1<<WSC_YU;
			if(keystate[SDLK_w])     key |= 1<<WSC_XR;
			if(keystate[SDLK_d])     key |= 1<<WSC_XD;
			if(keystate[SDLK_s])     key |= 1<<WSC_XL;
			if(keystate[SDLK_a])     key |= 1<<WSC_XU;
		} else {
			if(keystate[SDLK_F1])    key |= 1<<WSC_S;
			if(keystate[SDLK_UP])    key |= 1<<WSC_XU;
			if(keystate[SDLK_DOWN])  key |= 1<<WSC_XD;
			if(keystate[SDLK_LEFT])  key |= 1<<WSC_XL;
			if(keystate[SDLK_RIGHT]) key |= 1<<WSC_XR;
			if(keystate[SDLK_d])     key |= 1<<WSC_A;
			if(keystate[SDLK_s])     key |= 1<<WSC_B;
		}
		break;

//enum { NGP_U=0,NGP_D,NGP_L,NGP_R,NGP_A,NGP_B,NGP_O };

	case HW_NGP:
		if(keystate[SDLK_F1])    key |= 1<<NGP_O;
		if(keystate[SDLK_UP])    key |= 1<<NGP_U;
		if(keystate[SDLK_DOWN])  key |= 1<<NGP_D;
		if(keystate[SDLK_LEFT])  key |= 1<<NGP_L;
		if(keystate[SDLK_RIGHT]) key |= 1<<NGP_R;
		if(keystate[SDLK_d])     key |= 1<<NGP_B;
		if(keystate[SDLK_s])     key |= 1<<NGP_A;
		break;

//enum { PCE_1=0,PCE_2,PCE_SEL,PCE_RUN,PCE_U,PCE_R,PCE_D,PCE_L,PCE_3,PCE_6,PCE_5,PCE_4 };
	case HW_PCE:
		if(keystate[SDLK_F1])    key |= 1<<PCE_RUN;
		if(keystate[SDLK_F2])    key |= 1<<PCE_SEL;
		if(keystate[SDLK_UP])    key |= 1<<PCE_U;
		if(keystate[SDLK_DOWN])  key |= 1<<PCE_D;
		if(keystate[SDLK_LEFT])  key |= 1<<PCE_L;
		if(keystate[SDLK_RIGHT]) key |= 1<<PCE_R;
		if(keystate[SDLK_d])     key |= 1<<PCE_1;
		if(keystate[SDLK_s])     key |= 1<<PCE_2;
		break;

//enum { GBC_A=0,GBC_B,GBC_SEL,GBC_STA,GBC_D,GBC_U,GBC_L,GBC_R };
	case HW_GBC:
		if(keystate[SDLK_F1])    key |= 1<<GBC_STA;
		if(keystate[SDLK_F2])    key |= 1<<GBC_SEL;
		if(keystate[SDLK_UP])    key |= 1<<GBC_U;
		if(keystate[SDLK_DOWN])  key |= 1<<GBC_D;
		if(keystate[SDLK_LEFT])  key |= 1<<GBC_L;
		if(keystate[SDLK_RIGHT]) key |= 1<<GBC_R;
		if(keystate[SDLK_d])     key |= 1<<GBC_A;
		if(keystate[SDLK_s])     key |= 1<<GBC_B;
		break;

	case HW_NES:
		if(keystate[SDLK_F1])    key |= 1<<NES_STA;
		if(keystate[SDLK_F2])    key |= 1<<NES_SEL;
		if(keystate[SDLK_UP])    key |= 1<<NES_U;
		if(keystate[SDLK_DOWN])  key |= 1<<NES_D;
		if(keystate[SDLK_LEFT])  key |= 1<<NES_L;
		if(keystate[SDLK_RIGHT]) key |= 1<<NES_R;
		if(keystate[SDLK_d])     key |= 1<<NES_A;
		if(keystate[SDLK_s])     key |= 1<<NES_B;
		break;

//enum { SMS_U=0,SMS_D,SMS_L,SMS_R,SMS_2,SMS_1,SMS_STA,SMS_RES };
	case HW_SMS:
		if(keystate[SDLK_F1])    key |= 1<<SMS_STA;
		if(keystate[SDLK_F2])    key |= 1<<SMS_PAUSE;
		if(keystate[SDLK_UP])    key |= 1<<SMS_U;
		if(keystate[SDLK_DOWN])  key |= 1<<SMS_D;
		if(keystate[SDLK_LEFT])  key |= 1<<SMS_L;
		if(keystate[SDLK_RIGHT]) key |= 1<<SMS_R;
		if(keystate[SDLK_d])     key |= 1<<SMS_1;
		if(keystate[SDLK_s])     key |= 1<<SMS_2;
		break;

	case HW_LNX:
		if(keystate[SDLK_F1])    key |= 1<<LNX_1;
		if(keystate[SDLK_F2])    key |= 1<<LNX_2;
		if(keystate[SDLK_UP])    key |= 1<<LNX_U;
		if(keystate[SDLK_DOWN])  key |= 1<<LNX_D;
		if(keystate[SDLK_LEFT])  key |= 1<<LNX_L;
		if(keystate[SDLK_RIGHT]) key |= 1<<LNX_R;
		if(keystate[SDLK_d])     key |= 1<<LNX_o;
		if(keystate[SDLK_s])     key |= 1<<LNX_i;
		break;
	}

	if(keystate[SDLK_ESCAPE])    key |= 1<<31;

	if(keystate[SDLK_F9])       pic_nX=(pic_nX+1)%5;

	if(keystate[SDLK_F6]) g_EmuFlag |= (1<< 0);
	if(keystate[SDLK_F7]) g_EmuFlag |= (1<< 1);
	if(keystate[SDLK_F8]) g_EmuFlag |= (1<<31);

	return key;
}

extern char mp3_name[1024],old_name[1024];


//=============================================================================
// HAL for CDDA
//=============================================================================
void HAL_PCE_CD_Play(int track,int option)
{
	char name[512];
	char* p;

	core_strcpy(name,HAL_GetRomsPath());
	p = core_strrchr(name,'\\');

	if(p) {
		*++p = 0x30 + ((track/10) % 10);
		*++p = 0x30 + ((track   ) % 10);
		*++p = '.';
		*++p = 'm';
		*++p = 'p';
		*++p = '3';
		*++p = 0;

		core_strcpy(mp3_name,name);
	}
}

void HAL_PCE_CD_Stop(void)
{
	core_strcpy(mp3_name,"unloaded");
}

static int HAL_Com_Load(char* name,byte* adr,int size)
{
    int fd,len=size+1;

    if(name && adr && size) {
        if((fd = HAL_fd_open(name,HAL_MODE_READ))>=0) {
            len = HAL_fd_read(fd,adr,size);
            HAL_fd_close(fd);
        }
    }
    return (len==size);
}

//
static int HAL_Com_Save(char* name,byte* adr,int size)
{
    int fd,len=size+1;
    
    if(name) {
        if((fd=HAL_fd_open(name,HAL_MODE_WRITE))) {
            len=HAL_fd_write(fd,adr,size);
            HAL_fd_close(fd);
        }
    }
    return (len==size);
}

int HAL_Sound(void)
{
#ifdef SILENT
	return 0;
#endif
	return 1;
}

