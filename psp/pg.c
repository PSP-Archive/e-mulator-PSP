#include "syscall.h"
#include "pg.h"
#include "psp_main.h"

#include "cstring.h"

#define pad_New  0
#define pad_Old  1
#define pad_Now  2

static char* pg_vramtop = ((char*)0x04000000);

static long pg_screenmode;
static long pg_showframe;
static long pg_drawframe;

static char pg_workdir[MAX_PATH];

//==================================================================
// 
//==================================================================
void pgWaitV(void)
{
    if(PSP_Is()) {
#if 1
        sceDisplayWaitVblankCB();
#else
        static int pv = 0;
        int        cv = sceDisplayGetVcount();
        if(pv==cv) {
            sceDisplayWaitVblankCB();
        }
        pv = sceDisplayGetVcount();
#endif        
    } else {
        sceDisplayWaitVblankStart();
    }
}

//==================================================================
// Wait vsync [count] times
// ämé¿Ç…VblankStartÇë“ÇøÇΩÇ¢
//==================================================================
void pgWaitVn(u32 count)
{
	for(; count>0; --count) {
        if(PSP_Is()) {
            sceDisplayWaitVblankStartCB();
        } else {
            sceDisplayWaitVblankStart();
        }
        // pgWaitV();
    }
}

//==================================================================
// 
//==================================================================
char *pgGetVramAddr(unsigned long x,unsigned long y)
{
    u32 addr;

    if(x>479 || y>271) {
        return (char*)0;
    }

    addr  = pg_drawframe?FRAMESIZE:0;
    addr += x*PIXELSIZE*2+y*LINESIZE*2;

    if(PSP_Is()) {
        addr |= 0x04000000UL;
    } else {
        addr |= 0x44000000UL;
    }

    return (char*)addr;
}

//==================================================================
// 
//==================================================================
void pgPset(int px,int py,int color)
{
    *(WORD*)pgGetVramAddr(px,py) = (WORD)color;
}


//==================================================================
// ê¸Çï`Ç≠
// énì_(sx,sy)Ç∆èIì_(ex,ey)
// (1) í∑ï”ÇäÓèÄÇ∆ÇµÇƒï`âÊÇ∑ÇÈ
//==================================================================
void pgDrawLine(int sx,int sy,int ex,int ey,int color)
{
    int x,y,lx,ly,px,py;
    int vx = ex - sx; /* åXÇ´ÇãÅÇﬂÇÈ */
    int vy = ey - sy;

    /* í∑Ç≥ÇãÅÇﬂÇÈ */
    if(vx<0) lx=-vx; else lx=vx;
    if(vy<0) ly=-vy; else ly=vy;

    /* í∑Ç¢é≤ÇäÓèÄÇ…ÇµÇƒê¸Çï`Ç≠ */
    if(lx>=ly) {
        /* xé≤Ç™í∑Ç¢ */
        for(x=0;x<lx;x++) {
            /* ç¿ïW */
            px = sx + (vx/lx) * x;
            py = sy + (vy/ly) * (ly*x/lx) ;
            pgPset(px,py,color);
        }
    } else {
        // OK Ç¡Ç€Ç¢
        /* yé≤Ç™í∑Ç¢ */
        for(y=0;y<ly;y++) {
            /* ç¿ïW */
            py = sy + (vy/ly) * y;
            px = sx + (vx/lx) * (lx*y/ly) ;
            pgPset(px,py,color);
        }
    }
}
    
//==================================================================
// 
//==================================================================
void pgDrawFrame(unsigned long x1, unsigned long y1, unsigned long x2, unsigned long y2, unsigned long color)
{
	unsigned char *vptr0;		//pointer to vram
	unsigned long i;

	vptr0=pgGetVramAddr(0,0);
	for(i=x1; i<=x2; i++){
		((unsigned short *)vptr0)[i*PIXELSIZE + y1*LINESIZE] = (unsigned short)color;
		((unsigned short *)vptr0)[i*PIXELSIZE + y2*LINESIZE] = (unsigned short)color;
	}
	for(i=y1; i<=y2; i++){
		((unsigned short *)vptr0)[x1*PIXELSIZE + i*LINESIZE] = (unsigned short)color;
		((unsigned short *)vptr0)[x2*PIXELSIZE + i*LINESIZE] = (unsigned short)color;
	}
}

//==================================================================
// 
//==================================================================
void pgFillBox(unsigned long x1, unsigned long y1, unsigned long x2, unsigned long y2, unsigned long color)
{
	unsigned char *vptr0;		//pointer to vram
	unsigned long i, j;

	vptr0=pgGetVramAddr(0,0);
	for(i=y1; i<=y2; i++){
		for(j=x1; j<=x2; j++){
			((unsigned short *)vptr0)[j*PIXELSIZE + i*LINESIZE] = (unsigned short)color;
		}
	}
}

//==================================================================
// 
//==================================================================
void pgFillvram(unsigned long color)
{
	unsigned char *vptr0;		//pointer to vram
	unsigned long i;
	vptr0=pgGetVramAddr(0,0);
	for (i=0; i<FRAMESIZE/2; i++) {
		*(unsigned short *)vptr0=(unsigned short)color;
		vptr0+=PIXELSIZE*2;
	}
}

//==================================================================
// 
//==================================================================
void pgCls(unsigned long color)
{
    pgFillvram(color);
    pgScreenFlip();
    pgFillvram(color);
    pgScreenFlip();
}


//==================================================================
// 
//==================================================================
void pgScreenFrame(long mode,long frame)
{
	pg_screenmode=mode;
	frame=(frame?1:0);
	pg_showframe=frame;
	if (mode==0) {
		//screen off
		pg_drawframe=frame;
		sceDisplaySetFrameBuf(0,0,0,1);
	} else if (mode==1) {
		//show/draw same
        pg_drawframe=frame;
        sceDisplaySetFrameBuf(pg_vramtop+(pg_showframe?FRAMESIZE:0),LINESIZE,PIXELSIZE,1);
	} else if (mode==2) {
        //show/draw different
		pg_drawframe=(frame?0:1);
        sceDisplaySetFrameBuf(pg_vramtop+(pg_showframe?FRAMESIZE:0),LINESIZE,PIXELSIZE,1);
	}
}


//==================================================================
// 
//==================================================================
void pgScreenFlip()
{
    if(PSP_Is()) {
        sceKernelDcacheWritebackAll();
    }

	pg_showframe=1-pg_showframe;
	pg_drawframe=1-pg_drawframe;
    
	sceDisplaySetFrameBuf(pg_vramtop+(pg_showframe?FRAMESIZE:0),LINESIZE,PIXELSIZE,0);
}


//==================================================================
// 
//==================================================================
void pgScreenFlipV()
{
	pgWaitV();
	pgScreenFlip();
}

//==================================================================
// 
//==================================================================
static int readpad0(int kind)
{
    static u32 pad[3]={0,0,0};
	static int n=0;
	ctrl_data_t paddata;

    if(PSP_Is()) {
        sceCtrlPeekBufferPositive(&paddata, 1);
        if (paddata.Ly == 0xff) paddata.buttons|=CTRL_DOWN; else
        if (paddata.Ly == 0x00) paddata.buttons|=CTRL_UP;  
        if (paddata.Lx == 0x00) paddata.buttons|=CTRL_LEFT; else
        if (paddata.Lx == 0xff) paddata.buttons|=CTRL_RIGHT;
    } else {
        sceCtrlReadBufferPositive(&paddata, 1);
    }

    pad[pad_Now] = paddata.buttons;
    pad[pad_New] = pad[pad_Now] & ~pad[pad_Old]; // now_pad & ~old_pad;
    
	if(pad[pad_Old]==pad[pad_Now]){
		n++;
		if(n>=25){
			pad[pad_New]=pad[pad_Now];
			n = 20;
		}
	}else{
		n=0;
        pad[pad_Old] = pad[pad_Now];
	}

    
    return pad[kind];
}

int readpad_now(void) { return readpad0(pad_Now); }
int readpad_old(void) { return readpad0(pad_Old); }
int readpad_new(void) { return readpad0(pad_New); }

static DWORD*   GEcmd = (u32*)0x441f0000;
static Vertex16 v16[2] __attribute__((aligned(64)))={{0,0,0,0,0},{0,0,0,0,0}};

/******************************************************************************/

static u32 GeInit[] = {
	0x01000000, 0x02000000,
	0x10000000, 0x12000000, 0x13000000, 0x15000000, 0x16000000, 0x17000000,
	0x18000000, 0x19000000, 0x1A000000, 0x1B000000, 0x1C000000, 0x1D000000,
	0x1E000000, 0x1F000000,
	0x20000000, 0x21000000, 0x22000000, 0x23000000, 0x24000000, 0x25000000,
	0x26000000, 0x27000000, 0x28000000, 0x2A000000, 0x2B000000, 0x2C000000,
	0x2D000000, 0x2E000000, 0x2F000000,
	0x30000000, 0x31000000, 0x32000000, 0x33000000, 0x36000000, 0x37000000,
	0x38000000, 0x3A000000, 0x3B000000, 0x3C000000, 0x3D000000, 0x3E000000,
	0x3F000000,
	0x40000000, 0x41000000, 0x42000000, 0x43000000, 0x44000000, 0x45000000,
	0x46000000, 0x47000000, 0x48000000, 0x49000000, 0x4A000000, 0x4B000000,
	0x4C000000, 0x4D000000,
	0x50000000, 0x51000000, 0x53000000, 0x54000000, 0x55000000, 0x56000000,
	0x57000000, 0x58000000, 0x5B000000, 0x5C000000, 0x5D000000, 0x5E000000,
	0x5F000000,
	0x60000000, 0x61000000, 0x62000000, 0x63000000, 0x64000000, 0x65000000,
	0x66000000, 0x67000000, 0x68000000, 0x69000000, 0x6A000000, 0x6B000000,
	0x6C000000, 0x6D000000, 0x6E000000, 0x6F000000,
	0x70000000, 0x71000000, 0x72000000, 0x73000000, 0x74000000, 0x75000000,
	0x76000000, 0x77000000, 0x78000000, 0x79000000, 0x7A000000, 0x7B000000,
	0x7C000000, 0x7D000000, 0x7E000000, 0x7F000000,
	0x80000000, 0x81000000, 0x82000000, 0x83000000, 0x84000000, 0x85000000,
	0x86000000, 0x87000000, 0x88000000, 0x89000000, 0x8A000000, 0x8B000000,
	0x8C000000, 0x8D000000, 0x8E000000, 0x8F000000,
	0x90000000, 0x91000000, 0x92000000, 0x93000000, 0x94000000, 0x95000000,
	0x96000000, 0x97000000, 0x98000000, 0x99000000, 0x9A000000, 0x9B000000,
	0x9C000000, 0x9D000000, 0x9E000000, 0x9F000000,
	0xA0000000, 0xA1000000, 0xA2000000, 0xA3000000, 0xA4000000, 0xA5000000,
	0xA6000000, 0xA7000000, 0xA8000000, 0xA9000000, 0xAA000000, 0xAB000000,
	0xAC000000, 0xAD000000, 0xAE000000, 0xAF000000,
	0xB0000000, 0xB1000000, 0xB2000000, 0xB3000000, 0xB4000000, 0xB5000000,
	0xB8000000, 0xB9000000, 0xBA000000, 0xBB000000, 0xBC000000, 0xBD000000,
	0xBE000000, 0xBF000000,
	0xC0000000, 0xC1000000, 0xC2000000, 0xC3000000, 0xC4000000, 0xC5000000,
	0xC6000000, 0xC7000000, 0xC8000000, 0xC9000000, 0xCA000000, 0xCB000000,
	0xCC000000, 0xCD000000, 0xCE000000, 0xCF000000,
	0xD0000000, 0xD2000000, 0xD3000000, 0xD4000000, 0xD5000000, 0xD6000000,
	0xD7000000, 0xD8000000, 0xD9000000, 0xDA000000, 0xDB000000, 0xDC000000,
	0xDD000000, 0xDE000000, 0xDF000000,
	0xE0000000, 0xE1000000, 0xE2000000, 0xE3000000, 0xE4000000, 0xE5000000,
	0xE6000000, 0xE7000000, 0xE8000000, 0xE9000000, 0xEB000000, 0xEC000000,
	0xEE000000,
	0xF0000000, 0xF1000000, 0xF2000000, 0xF3000000, 0xF4000000, 0xF5000000,
	0xF6000000,	0xF7000000, 0xF8000000, 0xF9000000,
	0x0F000000, 0x0C000000};

void pgGeInit()
{
	int qid;
	sceKernelDcacheWritebackAll();
	qid = sceGeListEnQueue(&GeInit[0], 0, -1, 0);
	sceGeListSync(qid, 0);

    {
        static u32 GEcmd[64];
        // Draw Area
        GEcmd[ 0] = 0x15000000UL | (0 << 10) | 0;
        GEcmd[ 1] = 0x16000000UL | (271 << 10) | 479;
        // Tex Enable
        GEcmd[ 2] = 0x1E000000UL | 1;
        // Viewport
        GEcmd[ 3] = 0x42000000UL | (((int)((float)(480)) >> 8) & 0x00FFFFFF);
        GEcmd[ 4] = 0x43000000UL | (((int)((float)(-272)) >> 8) & 0x00FFFFFF);
        GEcmd[ 5] = 0x44000000UL | (((int)((float)(50000)) >> 8) & 0x00FFFFFF);
        GEcmd[ 6] = 0x45000000UL | (((int)((float)(2048)) >> 8) & 0x00FFFFFF);
        GEcmd[ 7] = 0x46000000UL | (((int)((float)(2048)) >> 8) & 0x00FFFFFF);
        GEcmd[ 8] = 0x47000000UL | (((int)((float)(60000)) >> 8) & 0x00FFFFFF);
        GEcmd[ 9] = 0x4C000000UL | (1024 << 4);
        GEcmd[10] = 0x4D000000UL | (1024 << 4);
        // Model Color
        GEcmd[11] = 0x54000000UL;
        GEcmd[12] = 0x55000000UL | 0xFFFFFF;
        GEcmd[13] = 0x56000000UL | 0xFFFFFF;
        GEcmd[14] = 0x57000000UL | 0xFFFFFF;
        GEcmd[15] = 0x58000000UL | 0xFF;
        // Depth Buffer
        GEcmd[16] = 0x9E000000UL | 0x88000;
        GEcmd[17] = 0x9F000000UL | (0x44 << 16) | 512;
        // Tex
        GEcmd[18] = 0xC2000000UL | (0 << 16) | (0 << 8) | 0;
        GEcmd[19] = 0xC3000000UL | 1;
        GEcmd[20] = 0xC6000000UL | (1 << 8) | 1;
        GEcmd[21] = 0xC7000000UL | (1 << 8) | 1;
        GEcmd[22] = 0xC9000000UL | (0 << 16) | (0 << 8) | 0;
        // Pixel Format
        GEcmd[23] = 0xD2000000UL | 1;
        // Scissor
        GEcmd[24] = 0xD4000000UL | (0 << 10) | 0;
        GEcmd[25] = 0xD5000000UL | (271 << 10) | 479;
        // Depth
        GEcmd[26] = 0xD6000000UL | 10000;
        GEcmd[27] = 0xD7000000UL | 50000;
        // List End
        GEcmd[28] = 0x0F000000UL;
        GEcmd[29] = 0x0C000000UL;
        GEcmd[30] = 0;
        sceKernelDcacheWritebackAll();
        qid = sceGeListEnQueue(&GEcmd[0], &GEcmd[30], -1, 0);
        sceGeListSync(qid, 0);
    }
}

//==================================================================
// 
//==================================================================
void pgMain(unsigned long args, void *argp)
{
    int n;
    //-------------------------------------------
    // Work DirectoryèÓïÒÇç\ízÇ∑ÇÈ              
    //-------------------------------------------
//    strcpy(pg_mypath,argp);
    core_strcpy(pg_workdir,argp);
	for (n=core_strlen(pg_workdir); n>0 && pg_workdir[n-1]!='/'; --n) pg_workdir[n-1]=0;
    
    //
    if(PSP_Is()) {
    	pgGeInit();
    }

    // init graphics
    sceDisplaySetMode(0,SCREEN_WIDTH,SCREEN_HEIGHT);
	pgScreenFrame(2,0);
    
    // init input
    sceCtrlSetSamplingCycle(0);
    sceCtrlSetSamplingMode(PSP_Is()?1:0);

    // re-init graphics
    sceDisplaySetMode(1,SCREEN_WIDTH,SCREEN_HEIGHT);
    pgScreenFrame(2,0);
}


////==================================================================
//// 
////==================================================================
//const char *pguGetMypath()
//{
//	return pg_mypath;
//}

//==================================================================
// 
//==================================================================
const char *pguGetWorkdir()
{
	return pg_workdir;
}


//==================================================================
// 
//==================================================================
void pgSetClock(int clock)
{
    if(PSP_Is()){
        scePowerSetClockFrequency(clock,clock,clock/2);
    }
}


//-----------------------------------------------------------------------------
// GPU bitblt
//-----------------------------------------------------------------------------
void blt_hard(WORD* pBuf,int x,int y,int w,int h,int rot,int bw,int bh,int mode)
{
    int ScreenSX,ScreenSY;
    int ScreenEX,ScreenEY;

    static int gqid = 0;
    u32 *pCMD = GEcmd;
    int nh,nw;
    int cw,ch,dw,dh;

    if(gqid) {
        sceGeListSync(gqid,0);
    }
    
    if(bw==256) nw=8; else nw=9;
    if(bh==256) nh=8; else nh=9;
    
    if(rot){ cw = h; ch = w; }
    else   { cw = w; ch = h; }

    // fit
    switch(mode) {
      case 2:
        dw = SCREEN_WIDTH - cw;
        dh = SCREEN_HEIGHT- ch;

        if(dw<dh) {
            ch = (ch * ((SCREEN_WIDTH *100)/cw))/100;
            cw = SCREEN_WIDTH;
        } else {
            cw = (cw * ((SCREEN_HEIGHT*100)/ch))/100;
            ch = SCREEN_HEIGHT;
        }
        break;

      case 3:
        cw = SCREEN_WIDTH;
        ch = SCREEN_HEIGHT;
        break;
    }

    ScreenSX = (SCREEN_WIDTH - cw)/2;
    ScreenSY = (SCREEN_HEIGHT- ch)/2;
    ScreenEX = ScreenSX+cw;
    ScreenEY = ScreenSY+ch;
    
    if(rot) {
        v16[0].u = x;
        v16[0].v = y;
        v16[0].x = ScreenSX;
        v16[0].y = ScreenEY;
        v16[0].z = 0;
        v16[1].u = x+w;
        v16[1].v = y+h;
        v16[1].x = ScreenEX;
        v16[1].y = ScreenSY;
        v16[1].z = 0;
    } else {
        v16[0].u = x;
        v16[0].v = y;
        v16[0].x = ScreenSX;
        v16[0].y = ScreenSY;
        v16[0].z = 0;

        v16[1].u = x+w;
        v16[1].v = y+h;
        v16[1].x = ScreenEX;
        v16[1].y = ScreenEY;
        v16[1].z = 0;
    }

    

    // Set Draw Buffer
    *pCMD++ = 0x9C000000UL | ((u32)pgGetVramAddr(0,0) & 0x00FFFFFF);
    *pCMD++ = 0x9D000000UL |(((u32)pgGetVramAddr(0,0) & 0xFF000000) >> 8) | 512;
    
    // Set Tex Buffer
    *pCMD++ = 0xA0000000UL | ( (u32)(unsigned char *)pBuf & 0x00FFFFFF);
    *pCMD++ = 0xA8000000UL | (((u32)(unsigned char *)pBuf & 0xFF000000) >> 8) | bw;
//GEcmd[ 4] = 0xB8000000UL | (((h>256)?9:8)<< 8) | ((w>256)?9:8);
    *pCMD++ = 0xB8000000UL | (nh<<8) /*HEIGHT=256(2y8)*/ | (nw)/*WIDTH=512(2y9)*/;
    // Tex Flush
    *pCMD++ = 0xCB000000UL;
    // Set Vertex
    *pCMD++ = 0x12000000UL | (1 << 23) | (0 << 11) | (0 << 9) | (2 << 7) | (0 << 5) | (0 << 2) | 2;
    *pCMD++ = 0x10000000UL;
    *pCMD++ = 0x02000000UL;
    *pCMD++ = 0x10000000UL | (((u32)(void *)v16 & 0xFF000000) >> 8);
    *pCMD++ = 0x01000000UL | ( (u32)(void *)v16 & 0x00FFFFFF);
    // Draw Vertex
    *pCMD++ = 0x04000000UL | (6 << 16) | 2;
    // List End
    *pCMD++ = 0x0F000000UL;
    *pCMD++ = 0x0C000000UL;
    *pCMD   = 0;

    sceKernelDcacheWritebackAll();

    gqid = sceGeListEnQueue(GEcmd,pCMD,-1,NULL);
//    b = sceGeListSync(qid,0);
//    if(gqid) sceGeListSync(gqid,0);
}


//-----------------------------------------------------------------------------
// Thumbnail blit
//-----------------------------------------------------------------------------
void blt_soft(WORD* pBuf,int x,int y,int w,int h,int rot,int bw,int bh)
{
    int i,j;

    if( rot ) {
        int sx = (SCREEN_WIDTH - h) / 2;
        int sy = (SCREEN_HEIGHT- w) / 2;
        
        for(j=0;j<h;j++) {
            WORD* dst = (WORD*)pgGetVramAddr(sx,sy+j);
            WORD* src = (WORD*)&pBuf[bw*(j+y)+x];
            for(i=0;i<w;i++) {
                dst = (WORD*)pgGetVramAddr(sx+j,sy+w-i);
                *dst = *src++;
            }
        }
    } else {
        int sx = (SCREEN_WIDTH - w)/2;
        int sy = (SCREEN_HEIGHT- h)/2;

        for(j=0;j<h;j++) {
            DWORD* dst = (DWORD*)pgGetVramAddr(sx,sy+j);
            DWORD* src = (DWORD*)&pBuf[bw*(j+y)+x];
            for(i=0;i<w/2;i++) {
                *dst++ = *src++;
            }
        }
    }
}


