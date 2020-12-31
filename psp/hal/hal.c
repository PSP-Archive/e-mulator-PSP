//*****************************************************************************
// 
// PSP HAL
// 
//*****************************************************************************
#include "pg.h"
#include "main.h"
#include "string.h"
#include "hal.h"

extern int flag_MakeThumbnail;

static int enter_menu=0;
static int quit_emu=0;
extern int flag_psp;

#define PSP() flag_psp

// internal
int hal_fps = 0;

// external
extern EmuConfig eConf;

FBFORMAT* fb_thumbnail=0;

static WORD* hal_fb_ptr[2]={(WORD*)0x44110000,(WORD*)0x44154000};
static int   hal_fb_sel=0;

static int g_skip_next_vsync = 0;

extern void blt_hard(WORD* pBuf,int x,int y,int w,int h,int rot,int bw,int bh,int mode);
extern void blt_soft(WORD* pBuf,int x,int y,int w,int h,int rot,int bw,int bh);
//extern void blt_thumb(WORD* pBuf,int x,int y,int w,int h,int rot,int bw,int bh);

// 
// 
// 
int HAL_fb2_init(u32 fbw,u32 fbh,FBFORMAT* pFb,u32 type)
{
    pFb->width   = fbw;
    pFb->height  = fbh;
    pFb->bpp     = sizeof(PIXEL_FORMAT); // System dependent
    
    pFb->fb      = (PIXEL_FORMAT*)hal_fb_ptr[hal_fb_sel];
    pFb->fb_tmp  = (PIXEL_FORMAT*)hal_fb_ptr[hal_fb_sel];
    pFb->pic_x   = 0;
    pFb->pic_y   = 0;
    pFb->pic_w   = fbw;
    pFb->pic_h   = fbh;
    pFb->hardware= type;

    hal_fb_sel = 1-hal_fb_sel;

    fb_thumbnail = 0;
    
    return 1;
}

//static int ws_flip(void)
//{
//    switch(eConf.wse_vrotate) {
//      case 0:  return isWonderSwanRotate();
//      case 2:  return 1;
//      default: break;
//    }
//    return 0;
//}


// 
// 
// 
int HAL_fb2_bitblt(FBFORMAT* pFb)
{
    fb_thumbnail = pFb;
    
    if(flag_MakeThumbnail) {
        return 1;
    }
    
    if( pFb->hardware==HW_WSC ) {
        pFb->rotate = eConf.wse_vrotate;
        
        if(eConf.wse_vrotate==2) {
            pFb->rotate = isWonderSwanRotate();
        }
    }
    
    if( PSP() && eConf.video>0 ) {
        blt_hard( pFb->fb,
                  pFb->pic_x, pFb->pic_y,
                  pFb->pic_w, pFb->pic_h, pFb->rotate,
                  pFb->width, pFb->height,
                  eConf.video  );
        
    } else {
        blt_soft( pFb->fb,
                  pFb->pic_x, pFb->pic_y,
                  pFb->pic_w, pFb->pic_h, pFb->rotate,
                  pFb->width, pFb->height );
    }
    
    if(eConf.fps) {
        char n[4];
        n[0] ='0'+((hal_fps/100));
        n[1] ='0'+((hal_fps/10)%10);
        n[2] ='0'+((hal_fps)%10);
        n[3] = 0;

        if(n[0]=='0') n[0]=' ';
        mh_print(0,0,n,RGB_WHITE);
    }

    if(eConf.vsync) {
        pgWaitV();
    }
    
    pgScreenFlip();

    //----------------------------
    // fliping
    //----------------------------
    pFb->fb     = 
    pFb->fb_tmp = (PIXEL_FORMAT*)hal_fb_ptr[hal_fb_sel];

    hal_fb_sel = 1-hal_fb_sel;

    return 1;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void HAL_fb2_close(void)
{
    hal_fb_sel = 0;
}

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
PIXEL_FORMAT HAL_fb2_Color(u8 r,u8 g,u8 b,u8 type)
{
    switch(type) {
      case RGB888:  b=(b>>3)&0x1f;  g=(g>>3)&0x1f;  r=(r>>3)&0x1f;  break;
      case RGB555:  b=(b   )&0x1f;  g=(g   )&0x1f;  r=(r   )&0x1f;  break;
      case RGB444:  b=(b<<1)&0x1f;  g=(g<<1)&0x1f;  r=(r<<1)&0x1f;  break;
    }

    return ((b)<<10) | ((g)<<5) | ((r));
//    return ((((b)<<10) | ((g)<<5) | ((r))) & 0x7BDE) >> 1;
}

extern int sndBufLen(void);
static int last_skip=0;

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
static void calc_fps(int fps)
{
    static u32 lasttick=0;
    static u32 framerate=0;
    static int _sec=0,_cal_fps=0;
    
    u32 curtick = sceKernelLibcClock();

    if(lasttick > curtick) {
        lasttick = curtick;
    }

    framerate = (framerate + (curtick-lasttick))/2;
    if(framerate==0) _cal_fps = 0;
    else             _cal_fps = ((10000000/(framerate))+9)/10;
    lasttick = curtick;

    if( (_sec+=2)>fps ) {
        hal_fps = _cal_fps;
        _sec=0;
    }
}

//-----------------------------------------------------------------------------
// fpsを基準とした安定化処理
//-----------------------------------------------------------------------------
static int frame_stable_fps(int fps)
{
    static u32 time_old=0;
    u32 time_new = sceKernelLibcClock();
    u32 fps_tm = 1000000/fps;
    u32 cntov=0;
    u32 end_tm = time_old+fps_tm;
    int ret=0;

    // 60fps : 1000000/60=16666.66
    // 75fps : 1000000/75=13333.33
    if(time_new>time_old) {

        if(end_tm>time_old) {
            if(time_new<end_tm) {
                while(time_new<end_tm){
                    time_new = sceKernelLibcClock();

                    if((cntov++)>10000000) { break; }
                }
            } else {
                ret=1;
            }
        }
    }

    time_old = time_new;

    return ret;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
static int frame_stable_sound(int fps)
{
    while( sndBufLen()>(512*6) ) {
        sceKernelDelayThread(1000);
    }

    if( sndBufLen() <= 1024 ) {
        if(g_skip_next_vsync==0) {
            g_skip_next_vsync=1;
            return 0;
        }

        if(last_skip<4) {
            last_skip++;
            return 1;
        }
        last_skip=0;
        return 0;
    }

    if(fps<=60)
      g_skip_next_vsync=0;
    last_skip=0;
    return 0;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
static int frame_stable_vsync(int fps)
{
    if(fps==75) {
        static int fcnt=0;

        if((fcnt=(fcnt+1)%5)==0) {
            return 1;
        }
    }
    
    return 0;
}

//-----------------------------------------------------------------------------
// FPS(frame/sec)を計算する関数
// 戻値がframeskipする(=1)かしない(=0)かを示す
//-----------------------------------------------------------------------------
int HAL_fps(int fps)
{
    u32 ret=0;
    static int ret1cnt=0;
    
    if(flag_MakeThumbnail) {
        return 0;
    }
    
    if(PSP()) {
        if(eConf.limit==0) {
            if( HAL_Sound() ){
                ret = frame_stable_sound(fps);
            } else {
                //if(eConf.vsync) {
                //    ret = frame_stable_vsync(fps);
                //} else {
                ret = frame_stable_fps(fps);
                //}
            }
        } else {
            if(eConf.vsync==1) { // auto
                ret = frame_stable_fps/*vsync*/(fps);
            } else {
                // no limit
            }
        }
    } 
    
    calc_fps(fps);

    if(ret){
        ret1cnt++;
        if(ret1cnt>9) {
            ret=0;
            ret1cnt=0;
        }
    } else {
        ret1cnt=0;
    }
    
    return ret;
}


extern KEY_CONFIG g_KeyCfg;

//-----------------------------------------------------------------------------
// KEY INPUT HAL
//
// 入力判定方法を検討中
// 全機種共通でボタン番号を決める
// メニュー: 0
// 方向キー: 1,2,3,4
// スタート: 5
// セレクト: 6
// ボタン  : 6,7,8,9,10,11,12,13,14,15
//
// これらを実際のビットに変換する。
// 
//-----------------------------------------------------------------------------
u32 HAL_Input(u32 player,u32 type)
{
    ctrl_data_t pd;
    u32 inp,outp=0;
    u8 *CFG_RAP,*CFG_KEY;
    static s32 rapid[]={ 0xffffffff,0x55555555,0x49249249,0x11111111 };
    static int cntRap=0; // Rapid Counter

    if(flag_MakeThumbnail) {
        return 0;
    }

    CFG_RAP = CFG_KEY = 0;
    cntRap = (cntRap+1) & 0x1f;
    
    switch(type) {
      case HW_PCE:
        if(eConf.pce_bt6) { CFG_RAP = g_KeyCfg.pc6[1];  CFG_KEY = g_KeyCfg.pc6[0]; }
        else              { CFG_RAP = g_KeyCfg.pce[1];  CFG_KEY = g_KeyCfg.pce[0]; }   break; 
      case HW_NES:        { CFG_RAP = g_KeyCfg.nes[1];  CFG_KEY = g_KeyCfg.nes[0]; }   break;
      case HW_WSC: {
          int mode = eConf.wse_control;

          if(mode==3){
              mode = isWonderSwanRotate();
          }
          
          switch(mode) {
            case 0: CFG_RAP = g_KeyCfg.wsn[1];  CFG_KEY = g_KeyCfg.wsn[0]; break;
            case 1: CFG_RAP = g_KeyCfg.wsf[1];  CFG_KEY = g_KeyCfg.wsf[0]; break;
            case 2: CFG_RAP = g_KeyCfg.wsu[1];  CFG_KEY = g_KeyCfg.wsu[0]; break;
          }
      }
        break;
      case HW_GBC:        { CFG_RAP = g_KeyCfg.gbc[1];  CFG_KEY = g_KeyCfg.gbc[0]; }   break;
      case HW_SMS:        { CFG_RAP = g_KeyCfg.sms[1];  CFG_KEY = g_KeyCfg.sms[0]; }   break;
      case HW_NGP:        { CFG_RAP = g_KeyCfg.ngp[1];  CFG_KEY = g_KeyCfg.ngp[0]; }   break;
      case HW_LNX:        { CFG_RAP = g_KeyCfg.lnx[1];  CFG_KEY = g_KeyCfg.lnx[0]; }   break;
    }
    
    if(PSP()){ sceCtrlPeekBufferPositive(&pd,1); }
    else     { sceCtrlReadBufferPositive(&pd,1); pd.Lx=pd.Ly=128; }

    inp = pd.buttons;
    
    if( pd.Ly< 43 ) /* UP    */ inp |= CTRL_A_UP;    else
    if( pd.Ly>211 ) /* DOWN  */ inp |= CTRL_A_DOWN;  
    if( pd.Lx>211 ) /* RIGHT */ inp |= CTRL_A_RIGHT; else
    if( pd.Lx< 43 ) /* LEFT  */ inp |= CTRL_A_LEFT;

    if(CFG_RAP && CFG_KEY) {
        if( inp & CTRL_UP       ) outp |= ( ((rapid[CFG_RAP[0x0]]<<cntRap)>>31) & (1<<CFG_KEY[0x0]) ); // 0
        if( inp & CTRL_RIGHT    ) outp |= ( ((rapid[CFG_RAP[0x1]]<<cntRap)>>31) & (1<<CFG_KEY[0x1]) ); // 1
        if( inp & CTRL_DOWN     ) outp |= ( ((rapid[CFG_RAP[0x2]]<<cntRap)>>31) & (1<<CFG_KEY[0x2]) ); // 2
        if( inp & CTRL_LEFT     ) outp |= ( ((rapid[CFG_RAP[0x3]]<<cntRap)>>31) & (1<<CFG_KEY[0x3]) ); // 3
        if( inp & CTRL_A_UP     ) outp |= ( ((rapid[CFG_RAP[0x4]]<<cntRap)>>31) & (1<<CFG_KEY[0x4]) ); // 4
        if( inp & CTRL_A_RIGHT  ) outp |= ( ((rapid[CFG_RAP[0x5]]<<cntRap)>>31) & (1<<CFG_KEY[0x5]) ); // 5
        if( inp & CTRL_A_DOWN   ) outp |= ( ((rapid[CFG_RAP[0x6]]<<cntRap)>>31) & (1<<CFG_KEY[0x6]) ); // 6
        if( inp & CTRL_A_LEFT   ) outp |= ( ((rapid[CFG_RAP[0x7]]<<cntRap)>>31) & (1<<CFG_KEY[0x7]) ); // 7
        if( inp & CTRL_TRIANGLE ) outp |= ( ((rapid[CFG_RAP[0x8]]<<cntRap)>>31) & (1<<CFG_KEY[0x8]) ); // 8
        if( inp & CTRL_CIRCLE   ) outp |= ( ((rapid[CFG_RAP[0x9]]<<cntRap)>>31) & (1<<CFG_KEY[0x9]) ); // 9
        if( inp & CTRL_CROSS    ) outp |= ( ((rapid[CFG_RAP[0xa]]<<cntRap)>>31) & (1<<CFG_KEY[0xa]) ); // 10
        if( inp & CTRL_SQUARE   ) outp |= ( ((rapid[CFG_RAP[0xb]]<<cntRap)>>31) & (1<<CFG_KEY[0xb]) ); // 11
        if( inp & CTRL_LTRIGGER ) outp |= ( ((rapid[CFG_RAP[0xc]]<<cntRap)>>31) & (1<<CFG_KEY[0xc]) ); // 12
        if( inp & CTRL_RTRIGGER ) outp |= ( ((rapid[CFG_RAP[0xd]]<<cntRap)>>31) & (1<<CFG_KEY[0xd]) ); // 13
        if( inp & CTRL_SELECT   ) outp |= ( ((rapid[CFG_RAP[0xe]]<<cntRap)>>31) & (1<<CFG_KEY[0xe]) ); // 14
        if( inp & CTRL_START    ) outp |= ( ((rapid[CFG_RAP[0xf]]<<cntRap)>>31) & (1<<CFG_KEY[0xf]) ); // 15
    }
    
    if( (inp & (CTRL_LTRIGGER|CTRL_SELECT))==(CTRL_LTRIGGER|CTRL_SELECT) ) {
        outp |= (1<<31);
    }
    
    return outp;
}

//------------------------------------------------------------------------------
// Sound Enable / Disable
//------------------------------------------------------------------------------
int HAL_Sound(void)
{
    return (eConf.sound>>1) & eConf.sound;
}

