#ifndef __MAIN_H__
#define __MAIN_H__


//--------------------------------------
// configuration
typedef struct {
  u8  clock;           // cpu clock
  u8  vsync;           // vsync wait
  u8  sound;           // sound [stereo/mono/disable]
  u8  limit;           // frame skip timing
  u8  video;           // video mode
  u8  fps;             // show fps

  u8  pce_bt6;         // 6button enable
  u8  pce_pad;         // pad number
  s8  pce_toc[512];    // CDROM image name
  
  u8  wse_vrotate;     // video rotate
  u8  wse_mono;        // monochrome color
  u8  wse_control;     // control

} EmuConfig;


typedef struct {
  u8 pce[2][16]; // 0:key,1:rapid
  u8 pc6[2][16];
  u8 nes[2][16];
  u8 gbc[2][16];
  u8 wsn[2][16];
  u8 wsf[2][16];
  u8 wsu[2][16];
  u8 ngp[2][16];
  u8 sms[2][16];
  u8 lnx[2][16];
} KEY_CONFIG;


extern EmuConfig eConf,pConf;

// èÛë‘ëJà⁄óp
enum {
  STATE_MAIN = 0,
  STATE_PLAY = 1,
  STATE_CONT = 2,
  STATE_QUIT = 3,
  STATE_ROM  = 4,
  STATE_RESET= 5
};

int menu_Main(void);
int psp_ExitCheck(void);

#endif /* __MAIN_H__ */
