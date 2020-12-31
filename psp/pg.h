//*****************************************************************************
// 
// 
// 
//*****************************************************************************
#include "syscall.h"

#define RGBA(r,g,b) (short)((((b>>3) & 0x1F)<<10)|(((g>>3) & 0x1F)<<5)|(((r>>3) & 0x1F)<<0)|0x8000)
#define RGB(r,g,b)  (short)((((b>>3) & 0x1F)<<10)|(((g>>3) & 0x1F)<<5)|(((r>>3) & 0x1F)<<0))
#define RGB_WHITE   RGB(255,255,255)
#define RGB_BLACK   RGB(  0,  0,  0)
#define RGB_BLUE    RGB(  0,  0,255)
#define RGB_GREEN   RGB(  0,255,  0)
#define RGB_RED     RGB(255,  0,  0)
#define RGB_YELLOW  RGB(255,255,  0)
#define RGB_GRAY    0x3def

#define SCREEN_WIDTH  480
#define SCREEN_HEIGHT 272
#define	PIXELSIZE	1				//in short
#define	LINESIZE	512				//in short
#define	FRAMESIZE	0x44000			//in byte

//480*272 = 60*38
#define CMAX_X 60
#define CMAX_Y 34
#define CMAX2_X 30
#define CMAX2_Y 17
#define CMAX4_X 15
#define CMAX4_Y 8

#define MAX_PATH 512		//temp, not confirmed

void pgWaitV(void);
void pgSetClock(int);
void pgWaitVn(u32 count);
void pgScreenFrame(long mode,long frame);
void pgScreenFlip();
void pgScreenFlipV();
void pgFillvram(unsigned long color);
void pgFillBmp(int);
void pgDrawLine(int sx,int sy,int ex,int ey,int color);

void pgCls(unsigned long color);
void pgPutChar(unsigned long x,unsigned long y,unsigned long color,unsigned long bgcolor,unsigned char ch,char drawfg,char drawbg,char mag);
void pgDrawFrame(unsigned long x1, unsigned long y1, unsigned long x2, unsigned long y2, unsigned long color);
void pgFillBox(unsigned long x1, unsigned long y1, unsigned long x2, unsigned long y2, unsigned long color);
char *pgGetVramAddr(unsigned long x,unsigned long y);
void *pgVramAddrOfs(u32 ofs);

void pgMain(unsigned long args, void *argp);

int readpad_new(void);
int readpad_now(void);
int readpad_old(void);


void memcpy4(void* d, void* s, int c);
void memcpy2(void* d, void* s, int c);
void memset4(void* d, int   v, int c);


//-----------------------------------------------
// libfont‚ðlink‚µ‚Ä‚Ë
//-----------------------------------------------
void pgPrint(unsigned long x,unsigned long y,unsigned long color,const char *str);
void pgPrint2(unsigned long x,unsigned long y,unsigned long color,const char *str);
void pgPrint4(unsigned long x,unsigned long y,unsigned long color,const char *str);

void pgPrintHex(int x,int y,short col,u32 hex);
void pgPrintDec(int x,int y,short col,u32 dec);

void mh_print(int x,int y,const unsigned char *str,int col);
void mh_print_hex2(int x,int y,u32 hex,short col);
void mh_print_hex4(int x,int y,u32 hex,short col);
void mh_print_hex8(int x,int y,u32 hex,short col);
void mh_print_hex(int x,int y,u32 hex,short col);
void mh_print_dec(int x,int y,u32 dec,short col);

//-----------------------------------------------
// 
//-----------------------------------------------
const char *pguGetWorkdir(void);


extern int pga_threadhandle[];
extern int pga_handle[];

//-----------------------------------------------
// PSP GPU STRUCT
//-----------------------------------------------
typedef struct Vertex16 {
    unsigned short u, v;
	short x, y, z;
} Vertex16;


