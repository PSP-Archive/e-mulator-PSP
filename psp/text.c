// 
// テキストファイル表示ハンドラ
// 
// 
#include "pg.h"

static int line_count=0;  // 行数
static int line_width=0;  // 行幅(一番大きいヤツ)

static char* pLastTop=0;  // 最後に表示した文字の先頭ポインタ
static char* pTxtAddr=0;
static int   nTxtSize=0;
static char* pTxtLast=0;

int TXT_Init(int nRomSize,byte* pRomAddr);
int TXT_Loop(void);
int TXT_Exit(void);


int TXT_Init(int nRomSize,byte* pRomAddr)
{
    int i;
    int n=0;

    pTxtAddr=pRomAddr;
    nTxtSize=nRomSize;
    pTxtLast=pRomAddr+nRomSize;

    pLastTop=pRomAddr;

    line_count=0;
    line_width=0;
    
    for(i=0;i<nRomSize;i++) {
        if(pRomAddr[i]==0x0d) {
            if(n>line_width) {
                line_width=n;
            }
            n=0;
            line_count++;
        }
        n++;
    }

    return 1;
}

int TXT_Loop(void)
{
    int i,nline;
    char msg[97],*pWr;
    char* pPtr;

    nline=0;
    pPtr=pLastTop;

    pgFillvram(0);
    
    while(nline<27) {
        pWr=msg;
        for(i=0;i<96;i++) {
            if(*pPtr==0x0a) {
                i--;
                pPtr++;
                continue;
            }
            if(*pPtr==0x0d ||
               *pPtr==0x00 ) {
                break;
            }
            
            if(*pPtr=='\t') {
                *pWr++ = ' ';
            } else {
                *pWr++ = *pPtr;
            }
            pPtr++;
        }

        pPtr++;
        if(nline==0) {
            pLastTop=pPtr;
        }
        
        *pWr=0;
        mh_print(0,nline*10,msg,-1);
        nline++;
    }

    pgScreenFlipV();
    pgWaitVn(60);

//    pLastTop = pPtr;

    {
        ctrl_data_t pd;
        sceCtrlReadBufferPositive(&pd,1);
        
//        if(pd.buttons & CTRL_DOWN) start_line++;
//        if(pd.buttons & CTRL_UP)   start_line--;
//        if(pd.buttons & CTRL_LEFT) start_line-=10;
//        if(pd.buttons & CTRL_RIGHT)start_line+=10;
//        
//        if(start_line<0) {
//            start_line=0;
//            }else {
//                if(start_line>scroll_max) {
//                    start_line=scroll_max;
//                }
//            }
        
        // EXIT text viewer
        if(pd.buttons & CTRL_SELECT) return 1;
    }
    
    return 0;
}


int TXT_Exit(void)
{

    return 1;
}
