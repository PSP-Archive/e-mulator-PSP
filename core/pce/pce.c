/*
	Portable PC-Engine Emulator
	1998 by BERO bero@geocities.co.jp

     Modified 1998 by hmmx hmmx@geocities.co.jp
     Modified 2000 by U-TA npce@excite.co.jp
    Modified 2001 by BreezeKeeper npce@rising-force.net
    Modified 2006 by e(T.Kawamorita)  e@dev-e.sakura.ne.jp
*/


//#define OPT_SOUND
// 
// サウンドについての考察
// 
// 1second : 44100(sample)
// 1Vsync  : 735(sample)=(44100/60)
// 1Hsync  : 3(sample)=(2.8=735/262)
// 
// 逆算すると1(sample)を作るには162(clock)必要。
// 1(sample)を聞き分ける能力があるかというと無いだろうから
// サイクル数から細かく出力サンプルを計算するよりは
// Hsync毎に作るべきサンプル数を求める方が現実的。
// 
// 計算式
// CurPos = (735*scanline)/262
// 


#define EXTERN 

#include "pce.h"
#include "m6502.h"
#include "cstring.h"
#include "hal.h"

#define SP_FILL ZBUFFER_SP
#define BG_FILL ZBUFFER_BG

static FBFORMAT fb_format;

static int sweep_update=0;  /* LFO実装のための準備 */
static int sweep_ch1freq=0; /* LFO実装のための準備 */
static int sweep_N=0;       /* LFO実装のための準備 */

#define MinLine(vdc)    io.vdcregs[vdc].minline
#define MaxLine(vdc)    io.vdcregs[vdc].maxline
#define	FC_W            io.vdcregs[0].screen_w
#define	FC_H            256

int skip_frame = 0;

static void RefreshBG(int Y1,int Y2,int vdc,int b1st);
static void RefreshSP(int Y1,int Y2,int vdc);

static void RefreshSound(void);

byte IO_read(word A);
void IO_write(word A,byte V);
static void VDC_write(word A,byte V);
static void VDC_init(void);
static void VCE_init(void);
static void PSG_init(void);
static void VDC_SATB_DMA(void);
static int  VDC_SATB_DMA_CHECK(void);
static void sprite2pixel(int vdc,int no);
static void plane2pixel(int vdc,int no);


static u8* pce_syscard=0; // system card

//=============================================================================
//
//
//=============================================================================
static byte JOY_read(word A)
{
    byte ret = (io.JOY[io.joy_counter]>>io.shiftmode[io.joy_counter])^0xff;
    if (io.joy_select&1) ret>>=4;
    else {
        ret&=15;
        if(io.JOY[io.joy_counter] & 0xf000) {
            io.shiftmode[io.joy_counter] = 8 - io.shiftmode[io.joy_counter];
        }
        io.joy_counter=(io.joy_counter+1)%5;
    }
    return ret; // |Country; /* country 0:JPN 1=US */
}


//=============================================================================
//
//
//=============================================================================
static void JOY_write(word A,byte V)
{
    io.joy_select = V&1;
    if (V&2) io.joy_counter = 0;
}

//=============================================================================
//
//
//=============================================================================
static byte IRQ_read(word A)
{
    byte ret;
    
    switch(A&3){
      case 2: return io.irq_mask;
      case 3: ret = io.irq_status;
        io.irq_status=0;
        return ret;
    }

    return 0xff;
}

//=============================================================================
//
//
//=============================================================================
static void IRQ_write(word A,byte V)
{
    switch(A&3){
      case 2: io.irq_mask = V;/*TRACE("irq_mask = %02X\n", V);*/ return;
      case 3: io.irq_status= (io.irq_status&~TIRQ)|(V&0xF8); return;
    }
}


//=============================================================================
//
//
//=============================================================================
static void TMR_write(word A,byte V)
{
	//TRACE("Timer Access: A=%X,V=%X\n", A, V);
	switch(A&1){
	case 0: io.timer_reload = V&127; return;
	case 1: 
		V&=1;
		if (V && !io.timer_start)
			io.timer_counter = io.timer_reload;
		io.timer_start = V;
		return;
	}
}

//=============================================================================
//
//
//=============================================================================
static byte TMR_read(word A)
{
	switch(A&1){
    case  0: return io.timer_counter;
	default: return 0xff;
    }
	return 0xff;
}

//=============================================================================
//
//
//=============================================================================
byte TimerInt(M6502 *R)
{
	if (io.timer_start) {
		io.timer_counter--;
		if (io.timer_counter > 128) {
			io.timer_counter = io.timer_reload;
			//io.irq_status &= ~TIRQ;
			if (!(io.irq_mask&TIRQ)) {
				io.irq_status |= TIRQ;
				//TRACE("tirq=%d\n",scanline);
				//TRACE("tirq\n");
				return INT_TIMER;
			}
		}
	}
	return INT_NONE;
}



//=============================================================================
//
//
//=============================================================================
void bank_set(byte P,byte V)
{
    if (PCE_ROMMap[V]==IOAREA) {
        PCE_Page[P]=IOAREA;
    }
    else {
        PCE_Page[P]=PCE_ROMMap[V]-P*0x2000;
    }
}

//=============================================================================
//
//
//=============================================================================
byte _Rd6502(word A)
{
    if(PCE_Page[A>>13]!=IOAREA) {
        return PCE_Page[A>>13][A];
    }

    return IO_read(A);
}


//=============================================================================
// ROM_size > 256ならSF2CE
//
//=============================================================================
void _Wr6502(word A,byte V)
{
    if(PCE_Page[A>>13]==IOAREA) {
        IO_write(A,V);
    } else{
        if( ROM_size>256 ) { // SF2CE ? 
            if ((A & 0x1ffc)==0x1ff0) {
                /* support for SF2CE silliness */
                int i;
                PCE_ROMMap[0x40] = PCE_ROMMap[0] + 0x80000;
                PCE_ROMMap[0x40] += (A & 3) * 0x80000;
                
                for (i = 0x41; i <= 0x7f; i++) {
                    PCE_ROMMap[i] = PCE_ROMMap[i - 1] + 0x2000;
                }
            } else {
                PCE_Page[A >> 13][A] = V;
            }
        } else {
            PCE_Page[A>>13][A]=V;
        }
    }
}



//=============================================================================
// スキャンライン描画処理関数
// 
// [ Normal ]
// VDC内でSP0,SP1,BGを SP0<BG<SP1 という優先順位で合成する。

// [ SuperGrafx ]
// VDC1とVDC2の出力を合成し、その結果をVPIで合成する。
// 
//=============================================================================
static void DrawLine(int y1,int y2)
{
    if(skip_frame) { 
		return ;
	}

    // NORMAL PC-Engine (VDC1 only)
    if(io.vpc[0]==0x11 && io.vpc[1]==0x11) {
        RefreshBG(y1,y2,VDC1,1);
        RefreshSP(y1,y2,VDC1);
    }
    // VDC2のみ(見たことない)
    else if(io.vpc[0]==0x22 && io.vpc[1]==0x22) {
        RefreshBG(y1,y2,VDC2,1);
        RefreshSP(y1,y2,VDC2);
    }
    // その他
    else {
        // 1941 Counter Attack(OK?)
        if(io.vpc[0]==0x77 && io.vpc[1]==0x77) {
            RefreshBG(y1,y2,VDC2,1);
            RefreshBG(y1,y2,VDC1,0);
            RefreshSP(y1,y2,VDC1);
            RefreshSP(y1,y2,VDC2);
        }
        // バトルエース(OK?) 魔道王グランゾート(opening)
        else if(io.vpc[0]==0x0 && io.vpc[1]==0x30) {
            RefreshBG(y1,y2,VDC2,1); // ここが！
            RefreshSP(y1,y2,VDC2);
            RefreshBG(y1,y2,VDC1,2);
            RefreshSP(y1,y2,VDC1);
        }
        // オルディネス(OK?)
        else if(io.vpc[0]==0x33 && io.vpc[1]==0x33) {
            // $08-$09 : 33 33
            RefreshBG(y1,y2,VDC2,1);
            RefreshSP(y1,y2,VDC2);
            RefreshBG(y1,y2,VDC1,0);
            RefreshSP(y1,y2,VDC1);
        }
        // 大魔界村(OK?)
        else if(io.vpc[0]==0x75 && io.vpc[1]==0x56) {
            // $08 : 75 1111b 1001b
            // $09 : 56 1001b 1010b
            // $0A-$0E : 00 ff 03 00
            RefreshBG(y1,y2,VDC2,1);
            RefreshBG(y1,y2,VDC1,0);
            RefreshSP(y1,y2,VDC1);
            RefreshSP(y1,y2,VDC2);
        }
        // 魔道王グランゾート(ゲーム)
        else if(io.vpc[0]==0x44 && io.vpc[1]==0x74) {
            // $08-$09: 44 74
            RefreshBG(y1,y2,VDC2,1);
            RefreshBG(y1,y2,VDC1,2);
            RefreshSP(y1,y2,VDC1);
            RefreshSP(y1,y2,VDC2);
        }
        // その他(該当なしのハズ)
        else {
            RefreshBG(y1,y2,VDC1,1);
            RefreshBG(y1,y2,VDC2,0);
            RefreshSP(y1,y2,VDC2);
            RefreshSP(y1,y2,VDC1);
        }
    }
}

/*
	Hit Chesk Sprite#0 and others
*/
static int CheckSprites(void)
{
    int i,x,y,w,h;
    SPR *spr;
    int x0,y0,w0,h0;

    int vdc = 0;

    spr = io.vdcregs[vdc].SPRAM;
    x0 = spr->x;
    y0 = spr->y;
    w0 = (((spr->atr>>8 )&1)+1)*16;
	h0 = (((spr->atr>>12)&3)+1)*16;
    
	spr++;
    
	for(i=1;i<64;i++) {
		x = spr->x;
		y = spr->y;
		w = (((spr->atr>>8 )&1)+1)*16;
		h = (((spr->atr>>12)&3)+1)*16;
        
        if( (x<x0+w0) && (x+w>x0) && (y<y0+h0) && (y+h>y0) ){
            return 1;
        }
        spr++;
	}
    return 0;
}

//=============================================================================
//
//
//=============================================================================
static void RefreshScreen(int dispmin,int dispmax)
{
    if(!skip_frame) {
        
        int s = (WIDTH-io.vdcregs[0].screen_w)/2;
        int e = (HEIGHT-256)/2+io.vdcregs[0].minline+dispmin;
        int w = io.vdcregs[0].screen_w;
        int h = dispmax-dispmin+1;

        fb_format.pic_x = s;
        fb_format.pic_y = e;
        fb_format.pic_w = w;
        fb_format.pic_h = h;
        
        HAL_fb2_bitblt(&fb_format);
    }
}


//=============================================================================
//
//
//=============================================================================
byte Loop6502(M6502 *R)
{
	int ret = INT_NONE;
    int dispmin = ((MaxLine(0)-MinLine(0))>MAXDISP ? (MinLine(0)+((MaxLine(0)-MinLine(0)-MAXDISP+1)>>1)) : MinLine(0));
    int dispmax = ((MaxLine(0)-MinLine(0))>MAXDISP ? (MaxLine(0)-((MaxLine(0)-MinLine(0)-MAXDISP+1)>>1)) : MaxLine(0));

    io.vdcregs[0].status&=~VDC_RasHit;
    io.vdcregs[1].status&=~VDC_RasHit;
    
    ret = VDC_SATB_DMA_CHECK();

    /* io.scanline Match Interrupt */
    if( io.vdcregs[0].VDC[CR].W & 0x04 ) {
        if( io.scanline ==((io.vdcregs[0].VDC[RCR].W&1023)-64) ) {
            io.vdcregs[0].status |= VDC_RasHit;
            ret = INT_IRQ;
        }
    }
    
    // 表示する領域の１ライン目を書くときの処理
	if (io.scanline==MinLine(0)) {

        RefreshSound();

        io.vdcregs[0].status&=~VDC_InVBlank;
        io.vdcregs[1].status&=~VDC_InVBlank;
        
        io.prevline=dispmin;
        
        io.vdcregs[0].ScrollYDiff = 0;
        io.vdcregs[0].oldScrollYDiff = 0;
        io.vdcregs[1].ScrollYDiff = 0;
        io.vdcregs[1].oldScrollYDiff = 0;
	}
    // 最終ラインを書くときの処理
    else if (io.scanline==MaxLine(0)) {
        if (CheckSprites()) io.vdcregs[0].status|= VDC_SpHit;
        else                io.vdcregs[0].status&=~VDC_SpHit;
        
        if (io.prevline<dispmax) {
            DrawLine(io.prevline,dispmax+1);
        }
        io.prevline=dispmax+1;
        RefreshScreen(dispmin,dispmax);
    }
    // 描画スキャンライン範囲内の処理
    else if (io.scanline>=MinLine(0) && io.scanline<=MaxLine(0)) {
        if((io.vdcregs[0].status&VDC_RasHit)) {
            if(io.prevline<dispmax) {
                DrawLine(io.prevline,io.scanline);
            }
            io.prevline = io.scanline;
        }
    }
    
//    scroll=0;

    // 最終ラインを描画した次のタイミングで実行する部分
	if (io.scanline==MaxLine(0)+1) {
        io.vdcregs[0].status|=VDC_InVBlank;

        /* VRAM to SATB DMA */
        VDC_SATB_DMA();
        
        if (ret==INT_IRQ) {
			io.vdcregs[0].pendvsync = 1;
        }
        else if (VBlankON(0)) {
            ret = INT_IRQ;
        }
	}

    if(io.vdcregs[0].pendvsync && ret!=INT_IRQ) {
        io.vdcregs[0].pendvsync = 0;
        //io.vdc_status|=VDC_InVBlank;
        if (VBlankON(0)) {
            //TRACE("vsync=%d\n", io.scanline);
            ret = INT_IRQ;
        }
    }
    
	if(ret==INT_IRQ) {
		if (!(io.irq_mask&IRQ1)) {
			io.irq_status|=IRQ1;
			return ret;
		}
	}
	return INT_NONE;
}

//=============================================================================
// BG LINEを描画する関数でつ
// SPより先に描画する
//=============================================================================
static void RefreshBG(int Y1,int Y2,int vdc,int b1st)
{
    int i;
    int X1,XW,Line;
    int x,y,h,offset;
    PIXEL_FORMAT *PP;//,*ZP;
	Y2++;

    PP = (PIXEL_FORMAT*)fb_format.fb+WIDTH*(HEIGHT-FC_H)/2+(WIDTH-FC_W)/2+WIDTH*Y1;

    if( !ScreenON(vdc) ) {
        if(b1st==1) {
            WORD *dst = (WORD*)fb_format.fb+((HEIGHT-FC_H)/2+Y1)*WIDTH;
            for(i=0;i<(Y2-Y1)*WIDTH;i++) {
                *dst++ = io.Pal[0];
            }
        }
        else if(b1st==2) {
            WORD *dst = (WORD*)fb_format.fb+((HEIGHT-FC_H)/2+Y1)*WIDTH;
            for(i=0;i<(Y2-Y1)*WIDTH;i++) {
                *dst++ &= 0x7ffe;
            }
        }
    }
    else {
        //TRACE("ScrollY=%d,diff=%d\n", ScrollY, ScrollYDiff);
        //TRACE("ScrollX=%d\n", ScrollX);
        y = Y1+ScrollY(vdc)-io.vdcregs[vdc].ScrollYDiff;
        offset = y&7;
        h = 8-offset;
        if (h>Y2-Y1) h=Y2-Y1;
        y>>=3;
        PP-=ScrollX(vdc)&7;

        XW=io.vdcregs[vdc].screen_w/8+1;
        
        for(Line=Y1;Line<Y2;y++) {

            x = ScrollX(vdc)/8;
            y &= io.vdcregs[vdc].bg_h-1;

            for(X1=0;X1<XW;X1++){
                PIXEL_FORMAT *R,*P;
                //byte *C;//,*Z;
                unsigned int *C2;
                int no;

                x&=io.vdcregs[vdc].bg_w-1;
                no = ((word*)io.vdcregs[vdc].VRAM)[x+y*io.vdcregs[vdc].bg_w];
                
                R = &io.Pal[(no>>12)*16];
                no&=0xFFF;

                if(vchange[vdc][no]) {
                    vchange[vdc][no]=0;
                    plane2pixel(vdc,no);
                }
                C2 = &VRAM2[vdc][no*8+offset];
                //C = &VRAM[no*32+offset*2];
                P = PP;

                for(i=0;i<h;i++) {
                    unsigned int L=C2[0];

                    if(b1st==1) {
                        P[0] = R[(L>>28)   ];
                        P[1] = R[(L>>24)&15];
                        P[2] = R[(L>>20)&15];
                        P[3] = R[(L>>16)&15];
                        P[4] = R[(L>>12)&15];
                        P[5] = R[(L>> 8)&15];
                        P[6] = R[(L>> 4)&15];
                        P[7] = R[(L    )&15];
                    }
                    else if(b1st==2) {
                        if((L>>28)   ) P[0] = R[(L>>28)   ]; else P[0]&=0x7ffe;
                        if((L>>24)&15) P[1] = R[(L>>24)&15]; else P[1]&=0x7ffe;
                        if((L>>20)&15) P[2] = R[(L>>20)&15]; else P[2]&=0x7ffe;
                        if((L>>16)&15) P[3] = R[(L>>16)&15]; else P[3]&=0x7ffe;
                        if((L>>12)&15) P[4] = R[(L>>12)&15]; else P[4]&=0x7ffe;
                        if((L>> 8)&15) P[5] = R[(L>> 8)&15]; else P[5]&=0x7ffe;
                        if((L>> 4)&15) P[6] = R[(L>> 4)&15]; else P[6]&=0x7ffe;
                        if((L    )&15) P[7] = R[(L    )&15]; else P[7]&=0x7ffe;
                    }
                    else {
                        if((L>>28)   ) P[0] = R[(L>>28)   ];
                        if((L>>24)&15) P[1] = R[(L>>24)&15];
                        if((L>>20)&15) P[2] = R[(L>>20)&15];
                        if((L>>16)&15) P[3] = R[(L>>16)&15];
                        if((L>>12)&15) P[4] = R[(L>>12)&15];
                        if((L>> 8)&15) P[5] = R[(L>> 8)&15];
                        if((L>> 4)&15) P[6] = R[(L>> 4)&15];
                        if((L    )&15) P[7] = R[(L    )&15];
                    }
                    
                    P+=WIDTH;
                    C2++;
                }
                x++;
                PP+=8;
            }
            Line+=h;
            PP+=WIDTH*h-XW*8;
            offset = 0;
            h = Y2-Line;
            if (h>8) h=8;
        }
    }
}

static void SP1_Put(PIXEL_FORMAT* pFb,PIXEL_FORMAT* pPalette,unsigned int L,int shift)
{
    if((L=(L>>shift)&15)) {
        // SPではない場合に描画する
        if( !((*pFb) & 0x8000) ) {
            *pFb = pPalette[L];// | 0x8000;
        }
    }
}

static void SP0_Put(PIXEL_FORMAT* pFb,PIXEL_FORMAT* pPalette,unsigned int L,int shift)
{
    if((L = (L>>shift)&15)) {
        if( (*pFb&0x8001) ) *pFb = *pFb|0x8000;
        else                *pFb = pPalette[L];
    }
}

#define SPX_PUT_N( NAME, PP, RR, LL, BB ) \
  if(LL) { \
      NAME(&PP[0+BB],RR,LL,28); NAME(&PP[1+BB],RR,LL,24); NAME(&PP[2+BB],RR,LL,20); NAME(&PP[3+BB],RR,LL,16); \
      NAME(&PP[4+BB],RR,LL,12); NAME(&PP[5+BB],RR,LL, 8); NAME(&PP[6+BB],RR,LL, 4); NAME(&PP[7+BB],RR,LL, 0); \
  }

#define SPX_PUT_H( NAME, PP, RR, LL, BB ) \
  if(LL) { \
      NAME(&PP[0+BB],RR,LL, 0); NAME(&PP[1+BB],RR,LL, 4); NAME(&PP[2+BB],RR,LL, 8); NAME(&PP[3+BB],RR,LL,12); \
      NAME(&PP[4+BB],RR,LL,16); NAME(&PP[5+BB],RR,LL,20); NAME(&PP[6+BB],RR,LL,24); NAME(&PP[7+BB],RR,LL,28); \
  }


//=============================================================================
//
//
//=============================================================================
static void PutSprite(PIXEL_FORMAT *P,int *C2,PIXEL_FORMAT *R,int h,int inc,int hflip,int spbg)
{
	int i,L0,L1;
    
    if(spbg) {
        for(i=0;i<h;i++) {
            L0 = C2[0]; L1 = C2[1];
            if(hflip) { SPX_PUT_H(SP1_Put,P,R,L0,0); SPX_PUT_H(SP1_Put,P,R,L1,8); }
            else      { SPX_PUT_N(SP1_Put,P,R,L1,0); SPX_PUT_N(SP1_Put,P,R,L0,8); }
            C2+=inc;
            P+=WIDTH;
        }
    } else {
        for(i=0;i<h;i++) {
            L0 = C2[0]; L1 = C2[1];
            if(hflip) { SPX_PUT_H(SP0_Put,P,R,L0,0); SPX_PUT_H(SP0_Put,P,R,L1,8); }
            else      { SPX_PUT_N(SP0_Put,P,R,L1,0); SPX_PUT_N(SP0_Put,P,R,L0,8); }
            C2+=inc;
            P+=WIDTH;
        }
    }
}

#define ATTR_SPBG(attr)  (((attr)>>7)&1)
#define ATTR_PAL(attr)   (((attr)&15)*16+256)
#define ATTR_HFLIP(attr) ((atr)&H_FLIP);

//=============================================================================
// BGが描画されたフレームにSPを
//
//=============================================================================
//
// FrameBufferにスプライトを描画する
//
static void RefreshSP(int Y1,int Y2,int vdc)
{
	int n,inc,hflip,spbg;
	SPR *spr;
    PIXEL_FORMAT *pDst;
    unsigned int *C2;
    int h,t,i,j,pos,y_sum;
    int cx,cy,yoffset,xoffset,atr;
    int x,y,no,cgx,cgy;

    if(!SpriteON(vdc)) return;

    for(n=0;n<64;n++) {
        spr = &io.vdcregs[vdc].SPRAM[n];
        atr = spr->atr;
        y = (spr->y&1023)-64;       // x coordinate    (10bit)
        x = (spr->x&1023)-32;       // y coordinate    (10bit)
        cgx = (atr>>8)&1;           // SPRITE-W (0:16,1:32)
		cgy = (atr>>12)&3;          // SPRITE-H (00:16,01:32,10:Inv,11:64)
		cgy |= cgy>>1;              // 

        if (y>=Y2 || y+(cgy+1)*16<Y1 || x>=FC_W || x+(cgx+1)*16<0) continue;
        //y--; /* スプライトが１ドット下に描画されるっぽいので調整 */

        no= spr->no&0x7ff;           // Pattern address (10bit)

        // 512-1023はゴミデータ?
        // if((no/2)>511) { continue; }
        no = (no>>1)&~(cgy*2+cgx);

        // sprite cache
        for(i=0;i<cgy*2+cgx+1;i++) {
			if (vchanges[vdc][no+i]) {
				vchanges[vdc][no+i]=0;
				sprite2pixel(vdc,no+i);
			}
			if (!cgx) i++;
		}

		C2 = &VRAMS[vdc][no*32];

		pos = WIDTH*(HEIGHT-FC_H)/2+(WIDTH-FC_W)/2+WIDTH*y+x;
		inc = 2;
        
		if (atr&V_FLIP) {
            inc=-2;
            C2+=15*2+cgy*64;
        }
        
		y_sum = 0;

        cy = y; // スプライトはスクロール影響なし？ //cy = y+ScrollY-ScrollYDiff;
		yoffset = cy&7;
		cy>>=3;
        xoffset = x&7; // スプライトはスクロール影響なし？//xoffset=(x+ScrollX)&7;
        cx = x / 8; //スプライトはスクロール影響なし？ //cx = (x+ScrollX)/8;

        hflip= ATTR_HFLIP(atr);
        spbg = ATTR_SPBG(atr);
        
		for(i=0;i<=cgy;i++) {

            cy = cy&(io.vdcregs[vdc].bg_h-1);
            
            t = Y1-y-y_sum;
			h = 16;
			if (t>0) {
				C2+=t*inc;
				h-=t;
				pos+=t*WIDTH;
				cy+=(yoffset+t)>>3;
				yoffset=(yoffset+t)&7;
			}
			if (h>Y2-y-y_sum) h = Y2-y-y_sum;
            
            for(j=0;j<=cgx;j++) {
                if(hflip) pDst = fb_format.fb+pos+(cgx-j)*16;
                else      pDst = fb_format.fb+pos+j*16;
                
                PutSprite(pDst,C2+j*32,
                          &io.Pal[ATTR_PAL(atr)],
                          h,inc,hflip,
                          spbg
                          );
            }
            
            pos+=h*WIDTH;
            C2+=h*inc+16*inc;
            y_sum+=16;
            cy+=(yoffset+h)>>3;
			yoffset=(yoffset+h)&7;
        }
    }
}


//=============================================================================
//
//
//=============================================================================
static void ResetPCE(M6502* p6502)
{
	core_memset(&io, 0, sizeof(PCE_IO));
	core_memset(IOAREA,0xFF,0x2000);
    core_memset(p6502,0,sizeof(M6502));

	TimerCount = TimerPeriod;
    p6502->IPeriod = IPeriod;
	p6502->TrapBadOps = 1;
//	CycleOld = 0;
    
	Reset6502(p6502);

    VDC_init();
    VCE_init();
    PSG_init();
    
/*  JOY_init();*/
    {
        int i;
        for(i=0;i<5;i++) {
            io.shiftmode[i] = 0;
        }
    }
    
/*  IRQ_init();*/
	io.irq_mask = 0;
    io.irq_status = 0;

/*  TMR_init();*/
	io.timer_counter = 0;
	io.timer_reload  = 0;
	io.timer_start   = 0;
    
    
    CD_init();
    ACD_init();

}

//=============================================================================
//
//
//=============================================================================
static int LoadROM(void* pRomAddr,int nRomSize)
{
	int i,ROMmask;

    ROM = pRomAddr;
    ROM_size = nRomSize;
    
	core_memset(PCE_ROMMap,0,sizeof(PCE_ROMMap));
    IPeriod = BaseClock/(SCANLINES_PER_FRAME*60);
	TimerPeriod = BaseClock/1000*3*1024/21480;

    populus = 0;
    
	ROMmask = 1;
	while(ROMmask<ROM_size) ROMmask<<=1;
	ROMmask--;
    //TRACE("ROMmask=%02X, ROM_size=%02X\n", ROMmask, ROM_size);

    for(i=0;i<0xF7;i++) {
        if (ROM_size == 0x30) {
			switch (i&0x70) {
			case 0x00:
			case 0x10:
			case 0x50:
				PCE_ROMMap[i]=ROM+(i&ROMmask)*0x2000;
				break;
			case 0x20:
			case 0x60:
				PCE_ROMMap[i]=ROM+((i-0x20)&ROMmask)*0x2000;;
				break;
			case 0x30:
			case 0x70:
				PCE_ROMMap[i]=ROM+((i-0x10)&ROMmask)*0x2000;
				break;
			case 0x40:
				PCE_ROMMap[i]=ROM+((i-0x20)&ROMmask)*0x2000;
				break;
			}
		}
        else {
			PCE_ROMMap[i]=ROM+(i&ROMmask)*0x2000;
        }
	}
    
//		PCE_ROMMap[i]=ROM+(i%ROM_size+i/ROM_size*0x10)*0x2000;
/*		if (((i&ROMmask)+i/(ROMmask+1)) < ROM_size)
			PCE_ROMMap[i]=ROM+((i&ROMmask)+i/(ROMmask+1)*0x20)*0x2000;
		else
			PCE_ROMMap[i]=ROM;
*///		PCE_ROMMap[i]=ROM+(i&ROMmask)*0x2000;
   
	if (populus) {
		PCE_ROMMap[0x40] = PopRAM + (0)*0x2000;
		PCE_ROMMap[0x41] = PopRAM + (1)*0x2000;
		PCE_ROMMap[0x42] = PopRAM + (2)*0x2000;
		PCE_ROMMap[0x43] = PopRAM + (3)*0x2000;
	}

#if 1
	if (1) {

        for(i=0x68;i<0x88;i++) {
            core_memcpy(&cd.cd_extra_mem[0x2000*(i-0x68)],PCE_ROMMap[i], 0x2000);
			PCE_ROMMap[i] = &cd.cd_extra_mem[0x2000*(i-0x68)];
            
		}
	}
#else
	if (1) {
		for(i=0;i<8;i++)
          core_memcpy(cd.cd_extra_mem + i*0x2000, PCE_ROMMap[0x80+i], 0x2000);
        
        PCE_ROMMap[0x80] = cd.cd_extra_mem;
        PCE_ROMMap[0x81] = cd.cd_extra_mem + 0x2000;
        PCE_ROMMap[0x82] = cd.cd_extra_mem + 0x4000;
        PCE_ROMMap[0x83] = cd.cd_extra_mem + 0x6000;
        PCE_ROMMap[0x84] = cd.cd_extra_mem + 0x8000;
        PCE_ROMMap[0x85] = cd.cd_extra_mem + 0xA000;
        PCE_ROMMap[0x86] = cd.cd_extra_mem + 0xC000;
        PCE_ROMMap[0x87] = cd.cd_extra_mem + 0xE000;

        for(i=0x68;i<0x80;i++) {
            core_memcpy(cd.cd_extra_mem+0xE000+0x2000*(i-0x68),PCE_ROMMap[i], 0x2000);
			PCE_ROMMap[i] = cd.cd_extra_mem+0xE000+0x2000*(i-0x68);
		}
	}
#endif

#if 0
	PCE_ROMMap[0x80] = PopRAM + (0)*0x2000;
	PCE_ROMMap[0x81] = PopRAM + (1)*0x2000;
	PCE_ROMMap[0x82] = PopRAM + (2)*0x2000;
	PCE_ROMMap[0x83] = PopRAM + (3)*0x2000;
	PCE_ROMMap[0x84] = PopRAM + (4)*0x2000;
	PCE_ROMMap[0x85] = PopRAM + (5)*0x2000;
	PCE_ROMMap[0x86] = PopRAM + (6)*0x2000;
	PCE_ROMMap[0x87] = PopRAM + (7)*0x2000;
#endif
   
	PCE_ROMMap[0xF7] = PCE_WRAM;
	PCE_ROMMap[0xF8] = PCE_RAM;
	PCE_ROMMap[0xF9] = PCE_RAM+0x2000;
	PCE_ROMMap[0xFA] = PCE_RAM+0x4000;
	PCE_ROMMap[0xFB] = PCE_RAM+0x6000;
	PCE_ROMMap[0xFF] = IOAREA; //NULL; /* NULL = I/O area */

	return 1;
}


//#define WAVETABLE
//#define VOLUMETABLE

#define CALC_WAVE(v) (((int)(v)-16) * 702)

// http://vsync.org/pe/index.html

//
// PSG CODE
//


#define SOUND_SamplesPerFrame  736      // [44100/59.94]=736

#define SAMPLE_RATE    ((DWORD)44100)  // sampling rate
#define SAMPLE10       (SAMPLE_RATE*10)

static int CycleLimit;            // 音(1byte)合成に必要な最少Clock値 (7160000/44100=162.3582...)

static DWORD dwOldPos[6];

#if !defined(OPT_SOUND)
static DWORD CycleLocal[6];
#endif

static DWORD snd_n[6];
static DWORD snd_k[6];
static DWORD snd_r[6];
static DWORD rand_val[6];

#ifdef WAVETABLE
static int waveTable[6][32];
#endif

#ifdef VOLUMETABLE
static int volTableR[6];
static int volTableL[6];
#endif

static int	 vol_tbl[32] = {
    100, 451, 508,  573,  646,  728,  821,  925,
    1043,1175,1325, 1493, 1683, 1898, 2139, 2411,
    2718,3064,3454, 3893, 4388, 4947, 5576, 6285,
    7085,7986,9002,10148,11439,12894,14535,16384,
};

static void WriteSoundData(int ch, DWORD dwNewPos);
static void write_psg(int ch);
static void WriteBuffer2(int ch, DWORD dwSize);

static void VOL_ALL(void)
{
#if defined(VOLUMETABLE)
    int ch;
    int lvol,rvol;
    
    for(ch=0;ch<0;ch++) {
        int psgv = io.psg_volume;
        int psg4 = io.PSG[ch][4] & 0x1f;
        int psg5 = io.PSG[ch][5];
        
        lvol = ((psgv>>3)&0x1E) + psg4 + ((psg5>>3)&0x1E) - 60;
        if(lvol<0) lvol = vol_tbl[0];
        else       lvol = vol_tbl[lvol];
        
        rvol = ((psgv<<1)&0x1E) + psg4 + ((psg5<<1)&0x1E) - 60;
        if (rvol<0) rvol = vol_tbl[0];
        else        rvol = vol_tbl[rvol];

        volTableR[ch]=rvol;
        volTableL[ch]=lvol;
    }
#endif
}

static void VOL_CH(int ch)
{
#if defined(VOLUMETABLE)
    int psgv = io.psg_volume;
    int psg4 = io.PSG[ch][4] & 0x1f;
    int psg5 = io.PSG[ch][5];
    int lvol,rvol;
    
    lvol = ((psgv>>3)&0x1E) + psg4 + ((psg5>>3)&0x1E) - 60;
    if(lvol<0) lvol = vol_tbl[0];
    else       lvol = vol_tbl[lvol];
    
    rvol = ((psgv<<1)&0x1E) + psg4 + ((psg5<<1)&0x1E) - 60;
    if (rvol<0) rvol = vol_tbl[0];
    else        rvol = vol_tbl[rvol];

    volTableR[ch]=rvol;
    volTableL[ch]=lvol;
#endif
}

//-------------------------------------------------------------------
// 
// 1/60タイミングで更新処理を実施
// 
//-------------------------------------------------------------------
static void RefreshSound(void)
{
    if( HAL_Sound() ) {
        WriteSoundData(0,SOUND_SamplesPerFrame);
        WriteSoundData(1,SOUND_SamplesPerFrame);
        WriteSoundData(2,SOUND_SamplesPerFrame);
        WriteSoundData(3,SOUND_SamplesPerFrame);
        WriteSoundData(4,SOUND_SamplesPerFrame);
        WriteSoundData(5,SOUND_SamplesPerFrame);
    
        // Callback Function
        HAL_Sound_Proc32(halSnd.R32,halSnd.L32,SOUND_SamplesPerFrame);
    }

    core_memset(halSnd.ch,0,sizeof(halSnd.ch));
}


//-------------------------------------------------------------------
// 
// 
// 
//-------------------------------------------------------------------
static void PSG_init(void)
{
    int i;

    for(i=0;i<8;i++) {
        io.PSG[i][4] = 0x80;
    }
    
    io.psg_volume = 0;
    io.psg_ch = 0;

    CycleLimit = BaseClock / SAMPLE_RATE;

    for(i=0;i<6;i++) {
        dwOldPos[i]=0;
#if !defined(OPT_SOUND)
        CycleLocal[i]=0;
#endif
        snd_n[i]=0;
        snd_k[i]=0;
        snd_r[i]=0;
    }

    rand_val[4] = rand_val[5] = 0x51f631e4;

#if defined(WAVETABLE)
    core_memset(waveTable,0,sizeof(waveTable));
#endif
}


//-------------------------------------------------------------------
// 
// PSGに対するWRITEアクセスを記述する
// 
//-------------------------------------------------------------------
static void PSG_write(word A,byte V)
{
    if(io.psg_ch>5) {
        if((A&15)==0) io.psg_ch = V&7;
    }
    else {
        // 後方のswitchで音に関する変更をしない場合はスキップする
        if (io.psg_ch<6 && (A&15)>0 && (A&15)<8 ) {
            if ((A&15)==1) {
                write_psg(0);
                write_psg(1);
                write_psg(2);
                write_psg(3);
                write_psg(4);
                write_psg(5);
            } else {
                write_psg(io.psg_ch);
            }
        }
        
        switch(A&15){
          case 0: io.psg_ch = V&7;             return; // 音に影響なし
          case 1: io.psg_volume = V;           VOL_ALL(); break;
          case 2: io.PSG[io.psg_ch][2] = V;    break;
          case 3: io.PSG[io.psg_ch][3] = V&15; break;
          case 4: io.PSG[io.psg_ch][4] = V;    VOL_CH(io.psg_ch); break;
          case 5: io.PSG[io.psg_ch][5] = V;    VOL_CH(io.psg_ch); break;
          case 6:
            if (io.PSG[io.psg_ch][4]&0x40){
                io.wave[io.psg_ch][0]=V&31;
#ifdef WAVETABLE
                waveTable[io.psg_ch][0] = CALC_WAVE(V&31);
#endif
            }else {
                io.wave[io.psg_ch][io.wavofs[io.psg_ch]]=V&31;
#ifdef WAVETABLE
                waveTable[io.psg_ch][io.wavofs[io.psg_ch]]= CALC_WAVE(V&31);
#endif
                io.wavofs[io.psg_ch]=(io.wavofs[io.psg_ch]+1)&31;
            } break;
          case 7: io.PSG[io.psg_ch][7] = V;    break;
          case 8: io.psg_lfo_freq = V;         return; // 音に影響なし
          case 9:
            if( V & 0x80 ) {
                // LFO関連のパラメータをリセットすること
                sweep_update = 1;
            }
            io.psg_lfo_ctrl = V;
            return;
            
          default: //TRACE("ignored PSG write\n");
            return; // 音に影響なし
            break;
        }
    }
}

//-------------------------------------------------------------------
// 
// PSGに対するREADアクセスを記述する
// 
//-------------------------------------------------------------------
static byte PSG_read(word A)
{
    if((A&15)==0) return io.psg_ch;

    if(io.psg_ch>5) return NODATA;

    switch(A&15){
      case 0: return io.psg_ch;
      case 1: return io.psg_volume;
      case 2: return io.PSG[io.psg_ch][2];
      case 3: return io.PSG[io.psg_ch][3];
      case 4: return io.PSG[io.psg_ch][4];
      case 5: return io.PSG[io.psg_ch][5];
      case 6: return io.wave[io.psg_ch][io.wavofs[io.psg_ch]];
      case 7: return io.PSG[io.psg_ch][7];
      case 8: return io.psg_lfo_freq;
      case 9: return io.psg_lfo_ctrl;
      default:
        return NODATA;
    }
    return NODATA;
}


//-------------------------------------------------------------------
// 
// PSGに対するREADアクセスを記述する
// 
//-------------------------------------------------------------------
static void write_psg(int ch)
{
    if(HAL_Sound()) {
#if defined(OPT_SOUND)
        int CurPos = (SOUND_SamplesPerFrame*io.scanline) / 262;

        if(dwOldPos[ch]<CurPos) {
            WriteBuffer2(ch,CurPos-dwOldPos[ch]);
            dwOldPos[ch]=CurPos;
        }
#else // OPT_SOUND
        DWORD dwNewPos;
        int Cycle = (DWORD)io.m6502.User - CycleLocal[ch];
        
    	// オーバーフローしたタイミングで変になるかなぁ
        if(Cycle<0) {
            CycleLocal[ch] = (DWORD)io.m6502.User;
        } else {
            // サウンドデータWrite間隔の閾値を超えているか？
            if(Cycle>=CycleLimit) {
                dwNewPos = Cycle/CycleLimit;
                WriteBuffer2(ch, (dwNewPos-dwOldPos[ch]));
                dwOldPos[ch] = dwNewPos;
            }
        }
#endif// OPT_SOUND
    }
}


//------------------------------------------------------------------------------
//
//
//------------------------------------------------------------------------------
static int mseq(DWORD *rand_val)
{
	if (*rand_val & 0x00080000)	{
		*rand_val = ((*rand_val ^ 0x0004) << 1) + 1;
		return 1;
	} else {
		*rand_val <<= 1;
		return 0;
	}
}

//------------------------------------------------------------------------------
// dwSizeは1chあたりのサイズを指定すること
// CHの増減管理はWriteBuffer内で収めたい
//------------------------------------------------------------------------------
static void WriteSoundData(int ch, DWORD dwNewPos)
{
#if defined(OPT_SOUND)

    WriteBuffer2(ch,dwOldPos[ch]-dwNewPos);
    dwOldPos[ch]=0;

#else // OPT_SOUND
    if(dwOldPos[ch] < dwNewPos){
        WriteBuffer2(ch, (dwNewPos-dwOldPos[ch]));
    }
    
    CycleLocal[ch] = (DWORD)io.m6502.User;
    dwOldPos[ch] = 0;
#endif// OPT_SOUND
}

//-------------------------------------------------------------------
// DDA mixer
//-------------------------------------------------------------------
static void mixer_dda(int ch,int lvol,int rvol,int dwSize)
{
    int mixch=halSnd.ch[ch];
#if 1
#ifdef WAVETABLE
    int wav0 = waveTable[ch][0];
#else
    int wav0 = (((int)io.wave[ch][0])-16)*702;
#endif
    
    lvol = (int)wav0*lvol>>14;
    rvol = (int)wav0*rvol>>14;
    
    for(;dwSize;dwSize--) {
        halSnd.L32[mixch] += lvol;
        halSnd.R32[mixch] += rvol;
        mixch++;
    }
#endif
    halSnd.ch[ch]=mixch;
}

//-------------------------------------------------------------------
// NOISE mixer
//-------------------------------------------------------------------
static void mixer_noise(int ch,int lvol,int rvol,int dwSize)
{
    DWORD ra = snd_r[ch];
    DWORD ka = snd_k[ch];
    
    int ra7020l=(7020*lvol)>>14;
    int ra7020r=(7020*rvol)>>14;
    int sndL=(ra?ra7020l:-ra7020l);
    int sndR=(ra?ra7020r:-ra7020r);
    int mixch=halSnd.ch[ch];
    DWORD Np = (io.PSG[ch][7]&0x1F);
    DWORD t;
    
    Np = 3000 + (Np<<9);
    
    for(;dwSize;dwSize--) {
        ka += Np;
        t = ka / SAMPLE_RATE;
        if (t >= 1) {
            ra = mseq(&rand_val[ch]);
            ka -= SAMPLE_RATE*t;
            sndL=(ra?+ra7020l:-ra7020l);
            sndR=(ra?+ra7020r:-ra7020r);
        }
        halSnd.L32[mixch] += sndL;
        halSnd.R32[mixch] += sndR;
        mixch++;
    }
    
    snd_r[ch]=ra;
    snd_k[ch]=ka;
    halSnd.ch[ch]=mixch;
}

//-------------------------------------------------------------------
// PSG mixer
//-------------------------------------------------------------------
static void mixer_psg(int ch,int lvol,int rvol,int dwSize)
{
    DWORD Tp = (io.PSG[ch][2]+((DWORD)io.PSG[ch][3]<<8)) & 0x0fff;
    
    DWORD t;
#if !defined(WAVETABLE)
    int i;
    static short wave[32];
#endif
    if (Tp<1) {
        halSnd.ch[ch]+=dwSize;
    }
    else {
#if !defined(WAVETABLE)
        for(i=0;i<32;i++) {
            wave[i] = ((short)io.wave[ch][i]-16)*702;
        }
#endif
        
        {
            DWORD na = snd_n[ch];
            DWORD ka = snd_k[ch];
            DWORD NTp = (35800000)/(Tp-0); // (32*118608)/Tp
            int mixch = halSnd.ch[ch];
            int wavena;
            
            for(;dwSize;dwSize--) {
#if defined(WAVETABLE)
                wavena = waveTable[ch][na];
#else
                wavena = wave[na];
#endif
                halSnd.L32[mixch] += (wavena*lvol)>>14;
                halSnd.R32[mixch] += (wavena*rvol)>>14;
                mixch++;
                ka += NTp;
                t = ka/SAMPLE10;
                na = (na+t)&31;
                ka -= SAMPLE10*t;
            }
            
            snd_n[ch]=na;
            snd_k[ch]=ka;
            halSnd.ch[ch]=mixch;
        }
    }
}

//-------------------------------------------------------------------
// LFO mixer
// 
// LFOは定期的に周波数を増減させて音に変化を与える。
// 
// LFO周波数毎にN,Nx16,Nx256を変調周波数へ追加していくものと思われる
// やっぱ面倒だから放置
//-------------------------------------------------------------------
// LFO周波数
// ch1のデータを使ってch0の音を変調するときに使います。
// $0808で変調の周波数を設定します。$0808の値をNとすると
// 
//    3.58MHz / ( 32 x ch1の波長($0802-$0803の値) x N )
// 
// になります。
// 
//   ch1の波長 = (io.PSG[1][2]+(io.PSG[1][3]<<8));
//   N         = io.psg_lfo_freq
// 
//-------------------------------------------------------------------
static void mixer_lfo(int ch,int lvol,int rvol,int dwSize)
{
    DWORD Tp = io.PSG[ch][2]+((DWORD)io.PSG[ch][3]<<8);
    DWORD t;
#if !defined(WAVETABLE)
    int i;
    static short wave[32];
#endif

    if (Tp<2) {
        halSnd.ch[ch]+=dwSize;
        return;
    }
    
    Tp-=1;

#if !defined(WAVETABLE)
    for(i=0;i<32;i++) {
        wave[i] = ((short)io.wave[ch][i]-16)*702;
    }
#endif

    if(sweep_update) {
        sweep_update=0;
        sweep_ch1freq = io.PSG[1][2]+((DWORD)io.PSG[1][3]<<8);

        switch(io.psg_lfo_ctrl&3) {
          case 1: sweep_N = io.psg_lfo_freq * 1; /* a<<0を足す */    break;
          case 2: sweep_N = io.psg_lfo_freq <<4; /* a<<4を足す */    break;
          case 3: sweep_N = io.psg_lfo_freq <<8; /* a<<8を足す */    break;
          default:
          case 0: sweep_N = 0;                   /* no modulation */ break;
        }
    }
    
    
    {
        DWORD na = snd_n[ch];
        DWORD ka = snd_k[ch];
        DWORD NTp = (32*1118608)/Tp;
        int wavena;
        int mixch = halSnd.ch[ch];
        
        for(;dwSize;dwSize--) {
#if defined(WAVETABLE)
            wavena = waveTable[ch][na];
#else
            wavena = wave[na];
#endif
            halSnd.L32[mixch] += ((int)(short)wavena*lvol>>14);
            halSnd.R32[mixch] += ((int)(short)wavena*rvol>>14);
            mixch++;
            ka += NTp;
            t = ka/SAMPLE10;
            na = (na+t)&31;
            ka -= SAMPLE10*t;
        }
        
        snd_n[ch]=na;
        snd_k[ch]=ka;
        halSnd.ch[ch]=mixch;
    }
}

//-------------------------------------------------------------------
//
//
//
//-------------------------------------------------------------------
static void WriteBuffer2(int ch, DWORD dwSize)
{
    int lvol, rvol;
    
    // 書き込みサイズをオーバーする要求は切り詰める
    if((dwSize+halSnd.ch[ch])>(SOUND_SamplesPerFrame)) {
        dwSize = 0;
        return;
    }

    if(dwSize==0) {
        return;
    }
    
	if (!(io.PSG[ch][4]&0x80)) {
		snd_n[ch] = snd_k[ch] = 0;
        halSnd.ch[ch]+=dwSize;
        return ;
	}

    /* make volume data from table & register*/
#if defined(VOLUMETABLE)
    lvol = volTableL[ch];
    rvol = volTableR[ch];
#else
    {
        int psgv = io.psg_volume;
        int psg4 = io.PSG[ch][4] & 0x1f;
        int psg5 = io.PSG[ch][5];
        
        lvol = ((psgv>>3)&0x1E) + psg4 + ((psg5>>3)&0x1E) - 60;
        if(lvol<0) lvol = vol_tbl[0];
        else       lvol = vol_tbl[lvol];
        
        rvol = ((psgv<<1)&0x1E) + psg4 + ((psg5<<1)&0x1E) - 60;
        if (rvol<0) rvol = vol_tbl[0];
        else        rvol = vol_tbl[rvol];
    }
#endif

    // PCM
    if(io.PSG[ch][4]&0x40) {
        mixer_dda(ch,lvol,rvol,dwSize);
	}
    // NOISE
    else if(ch>=4 && (io.PSG[ch][7]&0x80)) {
        mixer_noise(ch,lvol,rvol,dwSize);
    }
    // PSG
    else {
        if( 0 && ch<2 && !(io.psg_lfo_ctrl&0x80)) {
            if(ch==0) {
                mixer_lfo(ch,lvol,rvol,dwSize);
            }
        } else {
            mixer_psg(ch,lvol,rvol,dwSize);
        }
    }
}

// 
// SuperGrafx対応済み
// 一部機能が正常に実装されていないけど
// 実装されていない機能が使われてなければ問題なし
//=============================================================================
// 
// VDC INITIALIZE
// 
//=============================================================================
static void VDC_init(void)
{
    io.vpc[0]=io.vpc[1]=0x11;
    io.vpc[2]=io.vpc[3]=io.vpc[4]=io.vpc[5]=io.vpc[6]=io.vpc[7] = 0;

    io.vdcregs[0].status  = io.vdcregs[1].status  = 0;
    io.vdcregs[0].inc     = io.vdcregs[1].inc     = 1;
    io.vdcregs[0].minline = io.vdcregs[1].minline = 0;
    io.vdcregs[0].maxline = io.vdcregs[1].maxline = 255;

    io.scanline = 0;
    io.prevline = 0;

    core_memset(VRAMS,0,sizeof(VRAMS));
    core_memset(VRAM2,0,sizeof(VRAM2));
    core_memset(vchanges,1,sizeof(vchanges));
    core_memset(vchange,1,sizeof(vchange));
}

//-------------------------------------------------------------------
// ST_0 / ST_1 / ST_2
// 
// VPCレジスタ 0x0E の内容でアクセス対象が変化する
//-------------------------------------------------------------------
void VDC_d_write(word A,byte V)
{
    A |= (io.vpc[6]&1)<<4;
    VDC_write(A,V);
}

//-------------------------------------------------------------------
// 
// VDC WRITE ACCESS
// 
//-------------------------------------------------------------------
static void VDC_write(word A,byte V)
{
    VDC_REG* pV=0;
    int v=0;
    int vdc=0;

    A &= 0x1f;

         if(A<0x08) { vdc=0; }
    else if(A<0x10) { io.vpc[A&7]=V; return; }
    else if(A<0x18) { vdc=1; }
    else             return;
    
    pV = &io.vdcregs[vdc];
    v = pV->reg;
    
    switch(A&3){
      case 0: pV->reg = V&31; return;
      case 1:                     return;
      case 2: // VDC 下位バイト書込み

        pV->VDC[v].B.l = V;

        switch(v){
          case VWR:
            //io.VDC[VWR].B.l = V;
            return;
          case HDR:
            pV->screen_w = ((V&0x7f)+1) * 8;// (V+1)*8;
            break;
          case MWR: {
              pV->bg_h=(V&0x40)?64:32;

              switch((V>>4)&3) {
                case 0: pV->bg_w = 32; break;
                case 1: pV->bg_w = 64; break;
                case 2: 
                case 3: pV->bg_w = 128;break;
              }

              core_memset(vchange[vdc],1,VRAMSIZE/32);
              core_memset(vchanges[vdc],1,VRAMSIZE/128);
          }
            //TRACE("bg:%dx%d, V:%X\n",io.bg_w,io.bg_h, V);
            //TRACE("MWRl: %02X\n", V);
            break;
          case BYR:
/*            if (!scroll) {
                oldScrollX = ScrollX;
                oldScrollY = ScrollY;
                oldScrollYDiff = ScrollYDiff;
            }
            //io.VDC[BYR].B.l = V;
            scroll=1;
            ScrollYDiff=scanline-1;
*/
            return;
          case BXR:
/*
            if (!scroll) {
                oldScrollX = ScrollX;
                oldScrollY = ScrollY;
                oldScrollYDiff = ScrollYDiff;
            }
            //io.VDC[BXR].B.l = V;
            scroll=1;
*/
            return;
        }
        
        return;
        
        //-------------------------------------------
        // VDC 上位バイト書込み
        //-------------------------------------------
      case 3:
        pV->VDC[v].B.h = V;

        //printf("vdc_h%d,%02x ",io.vdc_reg,V);
        switch(pV->reg){
          case VWR:
            pV->VRAM[pV->VDC[MAWR].W*2+0]=pV->VDC[VWR].B.l;
            pV->VRAM[pV->VDC[MAWR].W*2+1]=pV->VDC[VWR].B.h;
            
            vchange[vdc][pV->VDC[MAWR].W/16]=1;
            vchanges[vdc][pV->VDC[MAWR].W/64]=1;
            pV->VDC[MAWR].W+=pV->inc;
            return;
          case VDW:
            //io.VDC[VDW].B.l = io.VDC_ratch[VDW];
            //io.VDC[VDW].B.h = V;
            pV->screen_h = (pV->VDC[VDW].W&511)+1;

            MaxLine(vdc) = pV->screen_h-1;
            
            //TRACE("VDWh: %X\n", io.VDC[VDW].W);
            return;
          case LENR: // 12
            //io.VDC[LENR].B.l = io.VDC_ratch[LENR];
            //io.VDC[LENR].B.h = V;
            //TRACE("DMA:%04x %04x %04x\n",io.VDC[DISTR].W,io.VDC[SOUR].W,io.VDC[LENR].W);
            /* VRAM to VRAM DMA */
            
            // スプライトのコピー処理が実行されるぽい？
            core_memcpy(pV->VRAM+pV->VDC[DISTR].W*2,pV->VRAM+pV->VDC[SOUR].W*2,(pV->VDC[LENR].W+1)*2);

            //変更フラグをたてるっぽい
            core_memset(vchange[vdc]+pV->VDC[DISTR].W/16,1,(pV->VDC[LENR].W+1)/16);
            core_memset(vchange[vdc]+pV->VDC[DISTR].W/64,1,(pV->VDC[LENR].W+1)/64);
            pV->VDC[DISTR].W += pV->VDC[LENR].W+1;
            pV->VDC[SOUR].W  += pV->VDC[LENR].W+1;
            pV->VDC[LENR].W = 0;
            
            pV->status|=VDC_DMAfinish;
            return;
            
          case CR :{
              static byte incsize[]={1,32,64,128};
              pV->inc = incsize[(V>>3)&3];
              //TRACE("CRh: %02X\n", V);
          } break;
          case HDR:
            //io.screen_w = (io.VDC_ratch[HDR]+1)*8;
            //TRACE0("HDRh\n");
            break;
          case BYR:
            if (!pV->scroll) {
                pV->oldScrollX = ScrollX(vdc);
                pV->oldScrollY = ScrollY(vdc);
                pV->oldScrollYDiff = pV->ScrollYDiff;
            }
            pV->VDC[BYR].B.h = V&1;
            pV->scroll=1;
            pV->ScrollYDiff=io.scanline-1;
            return;

          case SATB: // 13
            //io.VDC[SATB].B.h = V;
            //TRACE("SATB=%X,scanline=%d\n", io.VDC[SATB].W, scanline);
            pV->satb=1;
            pV->status&=~VDC_SATBfinish;
            return;

          case BXR:
            if(!pV->scroll) {
                pV->oldScrollX = ScrollX(vdc);
                pV->oldScrollY = ScrollY(vdc);
                pV->oldScrollYDiff = pV->ScrollYDiff;
            }
            pV->VDC[BXR].B.h = V & 3;
            pV->scroll=1;
            //			ScrollX = io.VDC[BXR].W;
            //			TRACE("BXRh = %d, scanline = %d\n", io.VDC[BXR].W, scanline);
            //			io.VDC[BXR].W = 256;
            return;
        }
        
        //io.VDC[io.vdc_reg].B.l = io.VDC_ratch[io.vdc_reg];
        //io.VDC[io.vdc_reg].B.h = V;
        //		if (io.vdc_reg != CR)
        //			TRACE("vdc_h: %02X,%02X\n", io.vdc_reg, V);
        //if (io.vdc_reg>19) {
        //    //TRACE("ignore write hi vdc%d,%02x\n",io.vdc_reg,V);
        //}
        return;
    }
}

//-------------------------------------------------------------------
// 
// VDC READ ACCESS
// 
//-------------------------------------------------------------------
static byte VDC_read(word A)
{
    byte ret;
    VDC_REG* pV=0;
    int vdc = 0;

    A &= 0x1f;

         if(A<0x08) { vdc=0; }
    else if(A<0x10) { return io.vpc[A&7]; }
    else if(A<0x18) { vdc=1; }
    else             return 0xFF;

    pV = &io.vdcregs[vdc];
    
    switch(A&3){
      case 0:
        ret = pV->status;
        pV->status=0;//&=VDC_InVBlank;//&=~VDC_BSY;
        return ret;
      case 1:
        return 0;
      case 2:
        if (pV->reg==VRR) 
          return pV->VRAM[pV->VDC[MARR].W*2];
        //else return io.VDC[io.vdc_reg].B.l;
      case 3:
        if (pV->reg==VRR) {
            ret = pV->VRAM[pV->VDC[MARR].W*2+1];
            pV->VDC[MARR].W+=pV->inc;
            return ret;
        }
        //else {
        //    return io.VDC[io.vdc_reg].B.h;
        //}
    }
    
    return 0;
}




//=============================================================================
// 
// VDC_SATB_DMA_CHECK
// 
//=============================================================================
static int VDC_SATB_DMA_CHECK(void)
{
    int vdc=0;
    
    if(io.vdcregs[vdc].satb_dma_counter ) {
        
        io.vdcregs[vdc].satb_dma_counter--;

        if(io.vdcregs[vdc].satb_dma_counter==0) {
            if(SATBIntON(vdc)) {
                io.vdcregs[vdc].status |= VDC_SATBfinish;
                return INT_IRQ;
            }
        }
    }
    return 0;
}

//=============================================================================
// 
// VDC_SATB_DMA
// 
//=============================================================================
static void VDC_SATB_DMA(void)
{
    int vdc;
    VDC_REG *pV;
    
    vdc = 0;
    pV = &io.vdcregs[vdc];
    
    if( pV->satb==1 || pV->VDC[DCR].W&0x0010) {
        core_memcpy(pV->SPRAM,pV->VRAM+pV->VDC[SATB].W*2,512);
        pV->satb = 1;
        pV->status &= ~VDC_SATBfinish;
        pV->satb_dma_counter = 4;
    }

    vdc = 1;
    pV = &io.vdcregs[vdc];
    
    if( pV->satb==1 || pV->VDC[DCR].W&0x0010) {
        core_memcpy(pV->SPRAM,pV->VRAM+pV->VDC[SATB].W*2,512);
        pV->satb = 1;
        pV->status &= ~VDC_SATBfinish;
        pV->satb_dma_counter = 4;
    }

}


//-----------------------------------------------------------------------------
// ピクセルフォーマットは RGBA(5551)でAはMSB
// ３種類(SP0/SP1/BG)のピクセル属性を判別するため以下の条件を付ける。
// ※ピクセル属性は別領域で実装可能だがメモリ帯域を考慮した上で決定です。
//
// SP描画Pixel : MSB=1
// BG描画Pixel : LSB=1
// LSBは画素の色情報を保持する有効BITであるが最大誤差+1なので無視する。
//-----------------------------------------------------------------------------

//static PIXEL_FORMAT pal_lut[512]; // Color Lookup Table
//static uint bCvtTbl[65536]; // Bit Convert Table
static PIXEL_FORMAT* pal_lut = 0; // Color Lookup Table
static u32*   bCvtTbl = 0;


#define B_TBL(a,b)    (bCvtTbl[(word)a] | (bCvtTbl[(word)b]<<2))
#define B_TBL00(a,b)  (bCvtTbl[(word)a] | (bCvtTbl[(word)b]<<2))
#define B_TBL16(a,b)  (bCvtTbl[(word)(a>>16)] | (bCvtTbl[(word)(b>>16)]<<2))


//-----------------------------------------------------------------------------
// 
// VRAMからBGをエンコードする
// 
//-----------------------------------------------------------------------------
static void plane2pixel(int vdc,int no)
{
    DWORD L0,L1;
    DWORD* C = (DWORD*)((word*)io.vdcregs[vdc].VRAM + no*16);
    DWORD* C2= (DWORD*)&VRAM2[vdc][no*8];

    L0=C[0]; L1=C[4];  C2[0]=B_TBL00(L0,L1);  C2[1]=B_TBL16(L0,L1);
    L0=C[1]; L1=C[5];  C2[2]=B_TBL00(L0,L1);  C2[3]=B_TBL16(L0,L1);
    L0=C[2]; L1=C[6];  C2[4]=B_TBL00(L0,L1);  C2[5]=B_TBL16(L0,L1);
    L0=C[3]; L1=C[7];  C2[6]=B_TBL00(L0,L1);  C2[7]=B_TBL16(L0,L1);
}

//-----------------------------------------------------------------------------
//
// VRAMからBGをエンコードする
//
//-----------------------------------------------------------------------------
static void sprite2pixel(int vdc,int no)
{
    DWORD L0,L1,L2,L3;
    byte La,Lb,Lc,Ld;
//    word Wa,Wb;
    int i;

    DWORD* C  = (DWORD*)((word*)io.vdcregs[vdc].VRAM + no*64);
    DWORD* C2 = (DWORD*)&VRAMS[vdc][no*32];

    for(i=0;i<8;i++) {
        L0=C[i]; L1=C[i+8]; L2=C[i+16]; L3=C[i+24];
#if 1
        La=L0;        Lb=L1;        Lc=L2;        Ld=L3;
        *C2++ = B_TBL00( ((word)Lb<<8|La) , ((word)Ld<<8|Lc) );

        La=L0>>8;     Lb=L1>>8;     Lc=L2>>8;     Ld=L3>>8;
        *C2++ = B_TBL00( ((word)Lb<<8|La) , ((word)Ld<<8|Lc) );

        La=L0>>16;    Lb=L1>>16;    Lc=L2>>16;    Ld=L3>>16;
        *C2++ = B_TBL00( ((word)Lb<<8|La) , ((word)Ld<<8|Lc) );
        
        La=L0>>24;    Lb=L1>>24;    Lc=L2>>24;    Ld=L3>>24;
        *C2++ = B_TBL00( ((word)Lb<<8|La) , ((word)Ld<<8|Lc) );
#else
        La=L0;      Lb=L1;      Lc=L2;     Ld=L3;
        Wa=(word)Lb<<8|La; Wb=(word)Ld<<8|Lc;  *C2++ = B_TBL00(Wa,Wb);
        
        La=L0>> 8;  Lb=L1>> 8;  Lc=L2>> 8; Ld=L3>> 8;
        Wa=(word)Lb<<8|La; Wb=(word)Ld<<8|Lc;  *C2++ = B_TBL00(Wa,Wb);
        
        La=L0>>16;  Lb=L1>>16;  Lc=L2>>16; Ld=L3>>16;
        Wa=(word)Lb<<8|La; Wb=(word)Ld<<8|Lc;  *C2++ = B_TBL00(Wa,Wb);
        
        La=L0>>24;  Lb=L1>>24;  Lc=L2>>24; Ld=L3>>24;
        Wa=(word)Lb<<8|La; Wb=(word)Ld<<8|Lc;  *C2++ = B_TBL00(Wa,Wb);
#endif
    }
}



//-------------------------------------------------------------------
// 
// VDC INITIALIZE
// 
//-------------------------------------------------------------------
static void VCE_init(void)
{
    u32 i,r,g,b;
    u32 lut[8] = {0,4,9,13,18,22,27,31};

    for(i=0;i<512;i++) {
        b = 7 &  i;
        r = 7 & (i>>3);
        g = 7 & (i>>6);

#if 1
        pal_lut[i] = HAL_fb2_Color(lut[r],lut[g],lut[b],RGB555);
#else
        pal_lut[i] = (lut[b]<<10) | (lut[g]<<5) | lut[r];
#endif

        // SPとBGのマスク処理のため上下1bitをワークとして使う
        // RGBA5551でしか実現不可能デス
        pal_lut[i]&= 0x7ffe;
    }
    

    // FAST BIT CONVERT TABLE
    for(i=0;i<256;i++) {
        bCvtTbl[i] = ((i&0x80)<<21)|((i&0x40)<<18)|((i&0x20)<<15)|((i&0x10)<<12)
          | ((i&0x08)<< 9)|((i&0x04)<< 6)|((i&0x02)<< 3)|((i&0x01)    );
        
    }
    
    for(i=256;i<65536;i++) {
        bCvtTbl[i] = (bCvtTbl[(i>>8)]<<1) | (bCvtTbl[(i&0xff)]<<0);
    }
    
    io.vce_cr = 0;
    io.vce_reg.W = 0;
}

//-------------------------------------------------------------------
// 
// VCE WRITE ACCESS
// 
//-------------------------------------------------------------------
static void VCE_write(word A, byte V)
{
    A = A & 7;
    
    switch(A) {
      case 0: io.vce_cr = V&0x87;            break;
      case 1: /*TRACE("VCE 1, V=%X\n", V);*/ break;
      case 2: io.vce_reg.B.l = V;            break;
      case 3: io.vce_reg.B.h = V&1;          break;
      case 4: io.VCE[io.vce_reg.W].B.l= V;   break;
      case 5: {
          int i;
          int n = io.vce_reg.W;
          int c;
          
          io.VCE[n].B.h = V;
          c = io.VCE[n].W & 0x1ff;
          
          // update palette
          if(n==0)        for(i=  0;i<256;i+=16)  io.Pal[i]=pal_lut[c];        // BG blank
          else if(n==256) for(i=256;i<512;i+=16)  io.Pal[i]=pal_lut[c];        // SP blank
          else if(n&0x100)                        io.Pal[n]=pal_lut[c]|0x8000; // SP color
          else if(n&15)                           io.Pal[n]=pal_lut[c]|0x0001; // BG color
          
          io.vce_reg.W=(io.vce_reg.W+1)&0x1FF;
      }
        return;
      case 6:	/*TRACE("VCE 6, V=%X\n", V);*/ break;
      case 7:	/*TRACE("VCE 7, V=%X\n", V);*/ break;
    }
    
}

//-------------------------------------------------------------------
// 
// VCE READ ACCESS
// 
//-------------------------------------------------------------------
static byte VCE_read(word A)
{
    A = A & 7;

    if(A==4) {
        return io.VCE[io.vce_reg.W].B.l;
    } else
    if(A==5) {
        byte v = io.VCE[io.vce_reg.W].B.h;
        io.vce_reg.W = (io.vce_reg.W+1) & 0x1ff;
        return v;
    }

    return 0;
}


//=============================================================================
//
//
//=============================================================================
void IO_write(word A,byte V)
{
	switch(A&0x1e00) {
	  case 0x0200: A-=0x0200;   // mirror
      case 0x0000: VDC_write(A,V); break;
	  case 0x0600: A-=0x0200;   // mirror
      case 0x0400: VCE_write(A,V); break;
      case 0x0A00: A-=0x0200;   // mirror
      case 0x0800: PSG_write(A,V); break;
	  case 0x0e00: A-=0x0200;   // mirror
      case 0x0c00: TMR_write(A,V); break;
      case 0x1200: A-=0x0200;   // mirror
      case 0x1000: JOY_write(A,V); break;
      case 0x1600: A-=0x0200;   // mirror
      case 0x1400: IRQ_write(A,V); break;
      case 0x1800: CD_write(A,V);  break;
      case 0x1A00: ACD_write(A,V); break;
      case 0x1C00: 
      case 0x1E00: 
		  return;
      default:
		  A = A;
        break;
    }
}

//=============================================================================
// 
// IO READ
// 
//=============================================================================
byte IO_read(word A)
{
    switch(A&0x1e00){ // 1F = 0001 1110
	  case 0x0200: A-=0x0200;   // mirror
      case 0x0000: return VDC_read(A);
      case 0x0400: return VCE_read(A);
      case 0x0800: return PSG_read(A);
	  case 0x0e00: A-=0x0200;   // mirror
      case 0x0c00: return TMR_read(A);
      case 0x1000: return JOY_read(A);
      case 0x1400: return IRQ_read(A);
      case 0x1800: {
		  u8 cdr = CD_read(A);

		  return cdr;

				   }
      case 0x1A00: 
		  return ACD_read(A);
	  default:
		  A=A;
		  break;
    }
	return NODATA;
}


//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
static int pce_save(int fp)
{
    HAL_sts_write(fp,&io,sizeof(io));
    return 1;
}

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
static int pce_load(int fp)
{
    int i;
    
    HAL_sts_read(fp,&io,sizeof(io));
    
    for(i=0;i<8;i++) {
        bank_set(i,io.m6502.MPR[i]);
    }

    core_memset(vchange, 1,sizeof(vchange));
    core_memset(vchanges,1,sizeof(vchanges));
    core_memset(VRAM2,0,sizeof(VRAM2));
    core_memset(VRAMS,0,sizeof(VRAMS));
    
    return 1;
}

static int TOC_INIT(int nRomSize,byte* pRomAddr);


// 
// initialize
// 
static int PCE_INIT(int nRomSize,byte* pRomAddr)
{
    pIO    = (PCE_IO*)HAL_mem_malloc(sizeof(PCE_IO));
    pCD    = (PCE_CD*)HAL_mem_malloc(sizeof(PCE_CD));
    pACD   = (PCE_ACD*)HAL_mem_malloc(sizeof(PCE_ACD));
    pCache = (PCE_CACHE*)HAL_mem_malloc(sizeof(PCE_CACHE));
    PCE_WRAM = (u8*)HAL_mem_malloc(SIZEOF_PCE_WRAM);

    PCE_RAM = pIO->RAM;

    BaseClock = 7160000;
    
	if(LoadROM((((byte*)pRomAddr)+(nRomSize&0x1fff)),nRomSize/0x2000)) {

        HAL_fb2_init(512,256,&fb_format,HW_PCE);
        
        fb_format.pic_x = 0;
        fb_format.pic_y = 0;
        fb_format.pic_w = 256;
        fb_format.pic_h = 256;
        
        if(!bCvtTbl) bCvtTbl = HAL_mem_malloc(sizeof(int)*65536);
        if(!pal_lut) pal_lut = HAL_mem_malloc(sizeof(PIXEL_FORMAT)*512);
	    
		ResetPCE(&io.m6502);
	
	    HAL_Cfg_Load("wram.dat",PCE_WRAM,SIZEOF_PCE_WRAM);
		return 1;
	}

	return 0;
}

// 
// main loop
// 
static int PCE_LOOP(void)
{
    u32 key;
    skip_frame = HAL_fps(60);
    
    Run6502(&io.m6502);

    key = HAL_Input(0,HW_PCE);
    io.JOY[0] = (u16)key;

    return (key & (1<<31));
}


// 
// exit
// 
static int PCE_EXIT(void)
{
    HAL_Cfg_Save("wram.dat",PCE_WRAM,SIZEOF_PCE_WRAM);

    if(bCvtTbl) HAL_mem_free(bCvtTbl);
    if(pal_lut) HAL_mem_free(pal_lut);
    bCvtTbl = 0;
    pal_lut = 0;

    if(pIO)    HAL_mem_free(pIO);
    if(pCD)    HAL_mem_free(pCD);
    if(pACD)   HAL_mem_free(pACD);
    if(pCache) HAL_mem_free(pCache);
    if(PCE_WRAM) HAL_mem_free(PCE_WRAM);

	if(pce_syscard) {
		HAL_mem_free(pce_syscard);
		pce_syscard=0;
	}
    
	return 1;
}

static void PCE_RESET(void)
{
    ResetPCE(&io.m6502);
}


int PCE_Setup(void)
{
    HAL_SetupExt( EXT_PCE, "pce", PCE_INIT, PCE_LOOP, PCE_EXIT, PCE_RESET, pce_load, pce_save );
    HAL_SetupExt( EXT_SGX, "sgx", PCE_INIT, PCE_LOOP, PCE_EXIT, PCE_RESET, pce_load, pce_save );
	HAL_SetupExt( EXT_TOC, "toc", TOC_INIT, PCE_LOOP, PCE_EXIT, PCE_RESET, pce_load, pce_save );
    
    return 1;
}

int cd_toc_read(int nsize,char* toc_buf);


// TOCを指定して実行する場合にはSYSCARD.PCEを読む
static int TOC_INIT(int nRomSize,byte* pRomAddr)
{
	int nsyscard=260*1024;

	if(!pce_syscard) {
		pce_syscard = HAL_mem_malloc(nsyscard);
	}

	if(pce_syscard){
		nsyscard = HAL_Cfg_Load("SYSCARD.ROM",pce_syscard,nsyscard);

		if(!nsyscard) {
			HAL_mem_free(pce_syscard);
			pce_syscard=0;
			return 0;
		}

		// TOCを読む & DISC CHANGEの場合があるので要考慮
		cd_toc_read(nRomSize,pRomAddr);

		return PCE_INIT(nsyscard,pce_syscard);
	}
	return 0;
}
