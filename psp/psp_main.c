//*****************************************************************************
// 
// PSPを使う上でのお決まり事項をココに書くことにする
// 
//*****************************************************************************
#include "psp_main.h"

#ifndef POWER_CB_POWER /* POWER_CB_POWER */
#define POWER_CB_POWER		0x80000000 
#define POWER_CB_HOLDON		0x40000000 
#define POWER_CB_STANDBY	0x00080000 
#define POWER_CB_RESCOMP	0x00040000 
#define POWER_CB_RESUME		0x00020000 
#define POWER_CB_SUSPEND	0x00010000 
#define POWER_CB_EXT		0x00001000 
#define POWER_CB_BATLOW		0x00000100 
#define POWER_CB_BATTERY	0x00000080 
#define POWER_CB_BATTPOWER	0x0000007F
#endif /* POWER_CB_POWER */


static int bSleep=0;

typedef int (*pg_threadfunc_t)(int args, void *argp);

void sceDisplayWaitVblankStart(void);
void scePowerRegisterCallback(int zero, int cbid);
long scePowerSetClockFrequency(long,long,long);
void sceKernelExitGame();
int  sceKernelCreateThread(const char *name, pg_threadfunc_t func, unsigned long priority, unsigned long stacksize, unsigned long attrib, void *unk);
int  sceKernelStartThread(int hthread, int arg0, void *arg1);
void sceKernelExitThread(int ret);
int  sceKernelWaitThreadEnd(int hthread, void *unk);
int  sceKernelDeleteThread(int hthread);
int  sceKernelCreateCallback(const char *name, void* func, void *arg);
void sceKernelSetExitCallback(int cbid);
int  sceExitSetCallback(int);
int  KernelPollCallbacks(void);



//------------------------------------------------------------------------
// PSP MAIN ROUTINE
//------------------------------------------------------------------------
extern int pspMain(int argc,char **argv);

//------------------------------------------------------------------------
// 
//------------------------------------------------------------------------
/*static*/ int flag_psp = 1;     // PSPEで実行されているかを示すフラグ
static int flag_end = 0;     // HOME KEYなどで停止状態になった場合1になる
static pPowerCB pPSP_PowerCB = 0;


//==========================================================================
// Run On PSP 
//==========================================================================
int PSP_Is(void)
{
    return flag_psp;
}

//==========================================================================
// RUN
//==========================================================================
int PSP_IsEsc(void)
{
    return flag_end;
}

//==========================================================================
// RUN
//==========================================================================
void PSP_GoHome(void)
{
    flag_end = 1;
}

//==========================================================================
// Register PowerCallback
//==========================================================================
void PSP_SetCallback(pPowerCB pFunc)
{
    if(pFunc) {
        pPSP_PowerCB = pFunc;
    }
}

//--------------------------------------------------------------------------
//
//--------------------------------------------------------------------------
static void on_exit_game(void)
{
    sceKernelPowerLock(0);
    
    pspExit();
    
    scePowerSetClockFrequency(222,222,111);
    sceKernelPowerUnlock(0);
}


//--------------------------------------------------------------------------
//
//--------------------------------------------------------------------------
static int exit_callback(int arg1, int arg2)
{
    flag_end = 1;
    bSleep=1;
    
    on_exit_game();
    sceKernelExitGame();
    return 0;
}

//--------------------------------------------------------------------------
//
//--------------------------------------------------------------------------
int PSP_Power_CheckSleep(void)
{
    if( bSleep ) {
        while( bSleep ) {
            sceDisplayWaitVblankStart();
        }
        return 1 ;
    }
    
	return 0 ;
}



//--------------------------------------------------------------------------
//
// 
//--------------------------------------------------------------------------
static void power_callback(int unknown, int pwrflags)
{
    if(pwrflags & POWER_CB_POWER) {
        if(!bSleep) {
            bSleep=1;
        }
//        on_exit_game();
    }
    else if(pwrflags & POWER_CB_BATLOW) {
        if(!bSleep) {
            bSleep=1;
//            on_exit_game();
            scePowerRequestSuspend();
        }
    }
    else if(pwrflags & POWER_CB_RESCOMP) {
        bSleep=0;
        scePowerSetClockFrequency(333,333,166);
    }

	int cbid = sceKernelCreateCallback("Power Callback", power_callback,0);
	scePowerRegisterCallback(0, cbid);
/*
    if(pPSP_PowerCB) {
        pPSP_PowerCB(pwrflags);
    }
    
    sceDisplayWaitVblankStart();
    return 0;
*/
}

//--------------------------------------------------------------------------
//
//--------------------------------------------------------------------------
static int callbackthread(void *arg) 
{
	int cbid = sceKernelCreateCallback("Exit Callback", exit_callback,0);
    sceKernelRegisterExitCallback(cbid);

    cbid = sceKernelCreateCallback("Power Callback", power_callback,0);
    scePowerRegisterCallback(0, cbid);
    
    KernelPollCallbacks();
	return 0;
}


//--------------------------------------------------------------------------
//
//--------------------------------------------------------------------------
static int SetupCallback(void)
{
	int thid = 0;
    if ( ( thid = sceKernelCreateThread("Update Thread", (void*)callbackthread, 0x11, 0xFA0, 0, 0) ) < 0 ) {
        return thid;
    }
    sceKernelStartThread(thid, 0, 0);
	return thid;
}


//--------------------------------------------------------------------------
// setup
//--------------------------------------------------------------------------
static void setup(void)
{
    SetupCallback();
}

//--------------------------------------------------------------------------
// cleanup
//--------------------------------------------------------------------------
static void cleanup(void)
{
    if(PSP_Is()) {
        scePowerSetClockFrequency(222,222,111);
    }
    
    sceKernelExitGame();
}




//==========================================================================
// PSPで必須となる処理をココでヤル
// アプリケーション側に面倒な処理を押し付けない
//==========================================================================
int main(int argc,char ** argv)
{
    flag_psp = argc;  // PSPE(argc=0)

    setup();

    pspMain(argc,argv);

    cleanup();
    
    return 0;
}

