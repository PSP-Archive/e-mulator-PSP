#include "pg.h"
#include "main.h"
#include "filer.h"
#include "hal.h"
#include "psp_main.h"
#include "menu.h"
#include "cstring.h"   // core_xxx関数

//////////////////////////////////////////////////////////////////////////////
int CartLoad(char* name,void* pRomAddr,int* nRomSize,int* pState);
int MP3_Setup(void);
int flag_loadstate = 0;

typedef enum {
    RCODE_NONE       = 0x00000000,
    RCODE_ROM        = 0x10000000,
    RCODE_CONTINUE   = 0x20000000, /* back to emulator */
    RCODE_RESET      = 0x40000000, /* back to emulator & reset */
    RCODE_ESCAPE     = 0x80000000, /* 何もせず戻る場合 */
    RCODE_UPDATE     = 0x00000001, /* 変更があった場合 */
    RCODE_LOADSTATE  = 0x00000004, /* ステートロードした場合 */
    RCODE_QUIT       = 0xffffffff, /* QUIT */
} RCODE;

#define KEY_UPDATE        0xffffffff
#define KEY_SELECT        0xfffffff0
#define KEY_MENU_ENTER    0xffffff00
#define KEY_MENU_LEAVE    0xfffff000

#define  RAPID_MAX    4
#define  KEYDEF_MIN   0
#define  KEYDEF_MAX   7

static char g_ks[16][80];

#include "menu_rsrc.h"

typedef enum {
    STATE_SAVE     = 0,
    STATE_LOAD     = 1,
    STATE_MEM_SAVE = 2,
    STATE_MEM_LOAD = 3
} STATE_ACCESS;

#define TNMODE_EMULATE      0
#define TNMODE_STATE        1
#define TNMODE_STATE_CLEAR  2
#define TNMODE_OFF         -1

int flag_MakeThumbnail = 0;
int thumbnail_mode = TNMODE_EMULATE; // emulator

#define THUMBNAIL_POSX(w) (480-30-(w))
#define THUMBNAIL_POSY(h) (30)

#define TRANS(img)        (((img)&0x7bde)>>1)
#define TRANS2(img1,img2) TRANS( TRANS(img1) + TRANS(img2) )

#define THUMB_WIDTH   256
#define THUMB_HEIGHT  256

#define FB_0 ((u16*)0x44088000)
#define FB_1 ((u16*)0x440CC000)

u16* menu_bgbuffer[2]={ FB_0, FB_1 };

typedef struct {
    u16 ptr[272][512];
} PSPFRAME;

#define GETFB_PTR(pfb,x,y) (&((pfb)->fb[  ((pfb)->pic_x + (x)) + \
                                          ((pfb)->pic_y + (y)) * (pfb)->width ]))

extern FBFORMAT* fb_thumbnail;

//----------------------------------------------------------------------------
// サムネイルを更新
//----------------------------------------------------------------------------
void thumbnail_update(int flag)
{
    u16 x,y,dx=0,dy=0;

    switch(flag) {
      case TNMODE_EMULATE: flag=0; break;
      case TNMODE_STATE  : flag=1; break;
      case TNMODE_STATE_CLEAR:
        core_memset(menu_bgbuffer[1],0,FRAMESIZE);
        return;
        break;
      default:
        return;
    }

    if( flag==0 ) {
        core_memset(menu_bgbuffer[0],0,FRAMESIZE);
        core_memset(menu_bgbuffer[1],0,FRAMESIZE);
    }
    
    if( fb_thumbnail ) {
        
        if(flag==1) {
            core_memset(menu_bgbuffer[flag],0,FRAMESIZE);
        }
        
        if( fb_thumbnail->pic_h<200 ) {
            for(y=0;y<fb_thumbnail->pic_h;y++) {
                PSPFRAME* pPspFrm = (PSPFRAME*)menu_bgbuffer[flag];
                u16 *L0 = GETFB_PTR(fb_thumbnail,0,y);
                u16 *B0 = &pPspFrm->ptr[y+30][0] + 480-30-fb_thumbnail->pic_w;
                
                for(x=0;x<fb_thumbnail->pic_w;x++) {
                    *B0++ = TRANS(*L0); L0++;
                }
            }
        } else {
            for(dy=y=0;y<fb_thumbnail->pic_h;y+=3) {
                for(dx=x=0;x<fb_thumbnail->pic_w;x+=3) {
                    PSPFRAME* pPspFrm = (PSPFRAME*)menu_bgbuffer[flag];
                    u16 *L0,*L1,*L2;
                    u16 *B0,*B1;
                    
                    L0 = GETFB_PTR(fb_thumbnail,x,y+0);
                    L1 = L0 + fb_thumbnail->width;
                    L2 = L1 + fb_thumbnail->width;
                    
                    B0 = &pPspFrm->ptr[dy+30][dx] + 480-30-fb_thumbnail->pic_w*2/3;
                    B1 = B0 + 512;
                    
                    B0[0x0000] = TRANS ( TRANS2(L0[0],L0[1]) + TRANS2(L1[0],L1[1]) );
                    B0[0x0001] = TRANS ( TRANS2(L0[1],L0[2]) + TRANS2(L1[1],L1[2]) );
                    B1[0x0000] = TRANS ( TRANS2(L1[0],L1[1]) + TRANS2(L2[0],L2[1]) );
                    B1[0x0001] = TRANS ( TRANS2(L1[1],L1[2]) + TRANS2(L2[1],L2[2]) );
                    
                    dx+=2;
                    if(x>THUMB_WIDTH) { break; }
                }
                dy+=2;
                if(y>THUMB_HEIGHT) { break; }
            }
        }
    }
}


//----------------------------------------------------------------------------
// サムネイル表示
//----------------------------------------------------------------------------
void thumbnail_bitblt(void)
{
    u32 flag = thumbnail_mode;
    
    switch(flag) {
      case TNMODE_EMULATE:
        flag=0;
        break;
      case TNMODE_STATE:
      case TNMODE_STATE_CLEAR:
        flag=1;
        break;
      default:
        pgFillvram(0);
        return;
    }
    
    core_memcpy(pgGetVramAddr(0,0),menu_bgbuffer[flag],FRAMESIZE);
}



//////////////////////////////////////////////////////////////////////////////
EmuConfig eConf;

int g_hardware = 0;


#define CENTER_PRINT(string)  mh_print((480-sizeof(string)*5)/2,(272-10)/2,string,RGB_WHITE);

extern int emu_handler_count;
extern CORE_HANDLER emu_handler[128];
extern int ext_list[];

#define _LOOP(v,change,min,max)  v = loop(v,change,min,max)
#define _ROUND(v,change,min,max) v = round(v,change,min,max)
#define _TOGGLE(v)               v = (v)?0:1;

char RomName[512]="";

CORE_HANDLER *g_pCoreHdl = 0;

static int loop(int value,int change,int min,int max)
{
    value += change;
    if(value<min)  value=max-1;
    if(value>=max) value=min;
    return value;
}

static int round(int value,int change,int min,int max)
{
    value += change;
    if(value<min) value=min;
    if(value>max) value=max-1;
    return value;
}

//--------------------------------------------------------------------------
//
//--------------------------------------------------------------------------
static int setup(void)
{
    // CREATE SUB DIRECTORY
    char* dir;
    if( (dir = HAL_GetSaveDir()) ) {
        HAL_fd_mkdir(dir);
    }
    
    //
    MP3_Setup();
    
    return 1;
}

//--------------------------------------------------------------------------
//
//--------------------------------------------------------------------------
static int clean(void)
{
    return 1;
}

//--------------------------------------------------------------------------
//
//--------------------------------------------------------------------------
int ConfigFile(int bLoad)
{
    char name[512];
    int fd;

    core_strcpy(name,HAL_GetWorkPath());
    core_strcat(name,"emulator.cfg");

    if(bLoad) {
        if((fd=HAL_fd_open(name,HAL_MODE_READ))>=0){
            HAL_fd_read(fd,&eConf,sizeof(eConf));
            HAL_fd_read(fd,&g_KeyCfg,sizeof(g_KeyCfg));
            HAL_fd_close(fd);
            return 1;
        }
    } else {
        if((fd=HAL_fd_open(name,HAL_MODE_WRITE))>=0) {
            HAL_fd_write(fd,&eConf,sizeof(eConf));
            HAL_fd_write(fd,&g_KeyCfg,sizeof(g_KeyCfg));
            HAL_fd_close(fd);
            return 1;
        }
    }

    return 0;
}

//--------------------------------------------------------------------------
// Quit Routine
//--------------------------------------------------------------------------
void pspExit(void)
{
    HAL_Sound_Close();
    ConfigFile(0);
    sceKernelExitGame();
}



#define IMAGE_SX(w,h) (479-(w)-8)
#define IMAGE_SY(w,h) (29)    // 272 - dHeight-31;

int rom_state=0;

static int toc_ext[] = {EXT_TOC,0};

static char pLastRomPath[512];
static char pLastTocPath[512];

static MENUITEM*   menu_ptr = 0;
static char*       menu_str = 0;
static int         menu_cur=0;
static int         menu_update=1;
//static char*       menu_str_sub = 0;
static int         menu_item_max = 0;
static char*       menu_str_info=0;


//#define DEF_COMPRESS

#ifdef DEF_COMPRESS
#define FD_OPT HAL_STS_COMPRESS
#else
#define FD_OPT 0x00
#endif

extern int memsts_bufsiz;
extern u8* memsts_buffer;
extern int memsts_seek;
extern int memsts_length;

#define DEFSIZE_TMPBUFFER (1024*1024)
u8* tmp_buffer=0; //[1024*1024];

//----------------------------------------------------------------------------
// ステート管理関数
// ファイル管理をHOOKすることでステートをメモリ管理する
//----------------------------------------------------------------------------
int StateFunc(STATE_ACCESS access,int num)
{
    int fd;
    char* pName = HAL_GetSavePath(num);

    if(pName && g_pCoreHdl && g_pCoreHdl->pSAVE && g_pCoreHdl->pLOAD) {
        
        switch(access) {
          case STATE_SAVE:
#if 1   /* STATE_MEM_SAVEで作成したデータを保存 */
            if(memsts_length) {
                if( (fd = HAL_fd_open(pName,HAL_MODE_WRITE)) >=0 ) {
                    HAL_sts_write(fd|FD_OPT,tmp_buffer,memsts_length);
                    HAL_fd_close(fd);
                    return 1;
                }
            }
#else   /***/
            if((fd = HAL_fd_open(pName,HAL_MODE_WRITE))>=0) {
                HAL_fd_write(fd,tmp_buffer,memsts_seek);
                g_pCoreHdl->pSAVE(fd);
                HAL_fd_close(fd);
                return 1;
            }
#endif  /***/
            break;
            
          case STATE_LOAD:
            if((fd = HAL_fd_open(pName,HAL_MODE_READ))>=0) {
                g_pCoreHdl->pLOAD(fd | FD_OPT);
                HAL_fd_close(fd);
                return 1;
            }
            break;
            
          case STATE_MEM_SAVE:
            memsts_length = 0;
            memsts_seek   = 0;
            memsts_bufsiz = DEFSIZE_TMPBUFFER;//sizeof(tmp_buffer);
            memsts_buffer = tmp_buffer;
            g_pCoreHdl->pSAVE(HAL_FP_MEM);  /* WORK ON MEMORY */
//            *(int*)0=memsts_length;
            return 1;
            break;
            
          case STATE_MEM_LOAD:
            memsts_bufsiz = DEFSIZE_TMPBUFFER;//sizeof(tmp_buffer);
            memsts_buffer = tmp_buffer;
            memsts_seek   = 0;
            g_pCoreHdl->pLOAD(HAL_FP_MEM);  /* WORK ON MEMORY */
            return 1;
            break;
        }
    }
    
    return 0;
}

//----------------------------------------------------------------------------
// Update Menu Pointer 
//
//----------------------------------------------------------------------------
void set_menu(MENULIST *pMenuList)
{
    static MENULIST* pOldMenuList = 0;
    int i;

    if(!pMenuList){// || (pMenuList==pOldMenuList)) {
        return;
    }
    
    /* Leave sub menu */
    // menufunc( KEY_MENU_LEAVE );
    if(pOldMenuList) {
        pOldMenuList->pHandler(KEY_MENU_LEAVE);
    }
    
    pOldMenuList = pMenuList;
    
    /* Initialize */
    // menufunc( KEY_MENU_ENTER );
    pMenuList->pHandler(KEY_MENU_ENTER);
    
    menu_ptr = pMenuList->pMenuItem;
    menu_str = pMenuList->title;

    // 全項目をアップデート
    menu_cur = 0;
    menu_update = 1;
    
    for(i=0;i<255;i++) {
        if(menu_ptr[i].string) {
            menu_item_max=i;
            
            if( menu_ptr[i].pFunc) {
                menu_ptr[i].pFunc(KEY_UPDATE,&menu_ptr[i]);
                if(i==menu_cur) {
                    menu_ptr[i].pFunc(KEY_SELECT,&menu_ptr[i]);
                }
            }
            
        } else {
            break;
        }
    }
}


//----------------------------------------------------------------------------
//
//
//----------------------------------------------------------------------------
void print_menu_frame(char* menustr,MENUITEM* item,int select)
{
    int i;

    thumbnail_bitblt();
    
    for(i=0;i<27;i++) {
        mh_print(0,i*10,menu_frame_string[i],RGB_WHITE | 0x8000);
    }

    if(menustr) {
        mh_print(10,10,menustr,RGB_WHITE);
    }

    if(menu_str_info) {
        mh_print(10,250,menu_str_info,RGB_WHITE);
    }
    
    if(item) {
        int x,y,color;
        
        for(i=0;item[i].string;i++) {
            color = item[i].color;
            x = 26;
            y = 40 + i*10;

            if(i==select) { color&=31; mh_print(x,y,"[#]",color); }
            else          {            mh_print(x,y,"[ ]",color); }
            
            mh_print(x+15,y,item[i].string,color);

            if(item[i].sub_str) {
                mh_print(x+15+(20*5),y,item[i].sub_str,color);
            }
        }
    }

#if 0 /* PSPのメモリ残量を取得 */
    {
        int sz = 1024;
        char* buffer;
        while(1) {
            buffer = HAL_mem_malloc(sz);
            if(!buffer) {
                break;
            }
            HAL_mem_free(buffer);
            sz += 1024;
        }
        mh_print_dec(10,260,sz,RGB_WHITE);
    }
#endif
}


// ステートファイルがあるか調べる関数
int State_isExist(int num)
{
    return (rom_state & (1<<num));
}

void State_Append(int num)
{
    rom_state |= 1<<num;
}

//----------------------------------------------------------------------------
// 
// 
// 
//----------------------------------------------------------------------------
static RCODE load_handler(int key, struct menuitem *pItem )
{
    if(key==KEY_MENU_ENTER) {
        thumbnail_mode = TNMODE_STATE;
        return RCODE_NONE;
    }

    if(key==KEY_MENU_LEAVE) {
        thumbnail_mode = TNMODE_EMULATE;
        return RCODE_NONE;
    }
    
    if(key==KEY_UPDATE) {
        IF_SELECTABLE(pItem, State_isExist(pItem->opt));
        return RCODE_NONE;
    }

    if(key==KEY_SELECT) {
        // ステートファイルを読み込み表示
        if(g_pCoreHdl && g_pCoreHdl->pLOAD && State_isExist(pItem->opt)) {
            if(StateFunc(STATE_LOAD,pItem->opt)) {
                flag_MakeThumbnail=1;
                g_pCoreHdl->pLOOP();
                flag_MakeThumbnail=0;
                thumbnail_update(TNMODE_STATE);
            }
        } else {
            thumbnail_update(TNMODE_STATE_CLEAR); // thumbnail_flag[1] = 0;
        }
        
        return RCODE_UPDATE;
    }

    if(key==CTRL_CIRCLE) {
        return RCODE_LOADSTATE;
    }

    return RCODE_NONE;
}

//----------------------------------------------------------------------------
// 
// 
//----------------------------------------------------------------------------
static RCODE save_handler(int key, struct menuitem* pItem)
{
    if(key==KEY_MENU_ENTER) {
        return RCODE_NONE;
    }

    if(key==KEY_MENU_LEAVE) {
        return RCODE_NONE;
    }

    if(key==KEY_SELECT) {
        return RCODE_NONE;
    }

    if(key==KEY_UPDATE) {
        if(State_isExist(pItem->opt)) pItem->color = COLOR_SELECTABLE;
        else                          pItem->color = COLOR_BLUEBLACK;
    }
    else if(key==CTRL_CIRCLE) {
        if(StateFunc(STATE_SAVE,pItem->opt)) {
            pItem->color = COLOR_SELECTABLE;
            State_Append(pItem->opt);
        }
        return RCODE_UPDATE;
    }
    return RCODE_NONE;
}

//--------------------------------------------------------------------------
// KEY CONFIG
//--------------------------------------------------------------------------
static RCODE keyc_handler(int key,struct menuitem* pItem)
{
    int p,v;
    int chg=0,trg=0;
    u8 *pk,*rp;
    char** str;
//    char*  ttl;

    if(key==KEY_MENU_ENTER) {
        return RCODE_NONE;
    }

    if(key==KEY_MENU_LEAVE) {
        return RCODE_NONE;
    }
    
    if( key == KEY_SELECT ) {
        return RCODE_NONE;
    }

    pk = rp = str= 0;
    
    switch(g_hardware) {
      case KEYCONF_HW_PCE: pk=g_KeyCfg.pce[0]; rp=g_KeyCfg.pce[1]; str=pce_k; break;
      case KEYCONF_HW_PC6: pk=g_KeyCfg.pc6[0]; rp=g_KeyCfg.pc6[1]; str=pce_k; break;
      case KEYCONF_HW_NES: pk=g_KeyCfg.nes[0]; rp=g_KeyCfg.nes[1]; str=nes_k; break;
      case KEYCONF_HW_GBC: pk=g_KeyCfg.gbc[0]; rp=g_KeyCfg.gbc[1]; str=gbc_k; break;
      case KEYCONF_HW_WSN: pk=g_KeyCfg.wsn[0]; rp=g_KeyCfg.wsn[1]; str=wsc_k; break;
      case KEYCONF_HW_WSF: pk=g_KeyCfg.wsf[0]; rp=g_KeyCfg.wsf[1]; str=wsc_k; break;
      case KEYCONF_HW_WSU: pk=g_KeyCfg.wsu[0]; rp=g_KeyCfg.wsu[1]; str=wsc_k; break;
      case KEYCONF_HW_NGP: pk=g_KeyCfg.ngp[0]; rp=g_KeyCfg.ngp[1]; str=ngp_k; break;
      case KEYCONF_HW_SMS: pk=g_KeyCfg.sms[0]; rp=g_KeyCfg.sms[1]; str=sms_k; break;
      default:
        break;
    }

    if(key==KEY_UPDATE) {
        p=pItem->id-KEYID_BEGIN;
        if(0<=0 && p<=15) {
            if(str[pk[p]]==0) { pk[p]=31; }
            core_strcpy(g_ks[p],str[pk[p]]);

            switch(rp[p]){
              case 0:  core_strcat(g_ks[p],"( x0 )"); break;
              case 1:  core_strcat(g_ks[p],"( x1 )"); break;
              case 2:  core_strcat(g_ks[p],"( x2 )"); break;
              case 3:  core_strcat(g_ks[p],"( x3 )"); break;
              default: core_strcat(g_ks[p],"( x? )"); break;
            }

            if(0) {
                char buf[3];
                buf[0] = '0' + (pk[p]/10);
                buf[1] = '0' +  pk[p]%10;
                buf[2] = 0;
                core_strcat(g_ks[p],buf);
            }
            
        }
        return RCODE_NONE;
    }


    if( key & (CTRL_RIGHT|CTRL_LEFT|CTRL_RTRIGGER|CTRL_LTRIGGER) ) {
        if(key==CTRL_RIGHT) chg=+1;
        if(key==CTRL_LEFT ) chg=-1;
        if(key==CTRL_RTRIGGER) trg=+1;
        if(key==CTRL_LTRIGGER) trg=-1;
        
        p = pItem->id - KEYID_BEGIN;
        
        if( 0<=p && p<=15 ) {
            if(chg) {
#if 1  //////////////////////////////////////////////////////
                int i;
                v = pk[p];
                
                for(i=0;i<32;i++) {
                    v += chg;
                    if(v<0) v=31;
                    else if(v>31) v=0;

                    if(str[v]) {
                        pk[p]=v;
                        break;
                    }
                }
#else  //////////////////////////////////////////////////////
                do { v = _LOOP(v,chg,0,32); } while( str[v]==0 );
                pk[p]=v;
#endif //////////////////////////////////////////////////////
            }
            
            if(trg) {
                rp[p]=(rp[p]+1) % RAPID_MAX;
            }
            
            return RCODE_UPDATE;
        }
    }

    return RCODE_NONE;
}

void menu_update_keyconf(int new_hw,MENUITEM* list)
{
    int i;
    
    g_hardware = new_hw;

    for(i=0;i<128;i++) {
        if(list[i].string) {
            if(KEYID_BEGIN<=list[i].id && list[i].id<(KEYID_BEGIN+16)) {
                keyc_handler(KEY_UPDATE,&list[i]);
            }
        } else {
            break;
        }
    }
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
static RCODE nes_handler(int key,struct menuitem* pItem)
{
    if(key==KEY_MENU_ENTER) {
        menu_update_keyconf(KEYCONF_HW_NES,nes_menu);
        return RCODE_NONE;
    }
    
    if(key==KEY_MENU_LEAVE) {
        return RCODE_NONE;
    }
    
    if(key==KEY_SELECT) {
        return RCODE_NONE;
    }

    return RCODE_NONE;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
static RCODE gbc_handler(int key,struct menuitem* pItem)
{
    if(key==KEY_MENU_ENTER) {
        menu_update_keyconf(KEYCONF_HW_GBC,gbc_menu);
        return RCODE_NONE;
    }
    
    if(key==KEY_MENU_LEAVE) {
        return RCODE_NONE;
    }
    
    if(key==KEY_SELECT) {
        return RCODE_NONE;
    }

    return RCODE_NONE;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
static RCODE ngp_handler(int key,struct menuitem* pItem)
{
    if(key==KEY_MENU_ENTER) {
        menu_update_keyconf(KEYCONF_HW_NGP,ngp_menu);
        return RCODE_NONE;
    }
    
    if(key==KEY_MENU_LEAVE) {
        return RCODE_NONE;
    }
    
    if(key==KEY_SELECT) {
        return RCODE_NONE;
    }

    return RCODE_NONE;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
static RCODE sms_handler(int key,struct menuitem* pItem)
{
    if(key==KEY_MENU_ENTER) {
        menu_update_keyconf(KEYCONF_HW_SMS,sms_menu);
        return RCODE_NONE;
    }
    
    if(key==KEY_MENU_LEAVE) {
        return RCODE_NONE;
    }

    if(key==KEY_SELECT) {
        return RCODE_NONE;
    }
    
    return RCODE_NONE;
}

//-----------------------------------------------------------------------------
// pad allocation
// _|_              (4) (5) (6)
//  |   (SEL) (RUN) (3) (2) (1)
//-----------------------------------------------------------------------------
static RCODE pce_handler(int key,struct menuitem* pItem)
{
    int dif=0;
    static char *msgPadNo[]={"1","2","3","4","5"};

    if(key==KEY_MENU_ENTER) {
        if(eConf.pce_bt6) menu_update_keyconf(KEYCONF_HW_PC6,pce_menu);
        else              menu_update_keyconf(KEYCONF_HW_PCE,pce_menu);
        return RCODE_NONE;
    }
    
    if(key==KEY_MENU_LEAVE) {
        return RCODE_NONE;
    }
    
    if(key==KEY_SELECT) {
        return RCODE_NONE;
    }

    if( key==KEY_UPDATE ) {
        switch(pItem->id) {
          case MENU_PCE_PADNO:pItem->sub_str = msgPadNo[eConf.pce_pad]; break;
          case MENU_PCE_BT6:  pItem->sub_str = msgOnOff[eConf.pce_bt6]; break;
          case MENU_PCE_TOC:  pItem->sub_str = eConf.pce_toc;           break;
        }
        return RCODE_NONE;
    }

    if( key==CTRL_CIRCLE || key==CTRL_TRIANGLE ) {
        if(key==CTRL_CIRCLE)   dif=+1;
        if(key==CTRL_TRIANGLE) dif=-1;
        
        if(dif) {
            switch(pItem->id) {
              case MENU_PCE_PADNO:
                _LOOP(eConf.pce_pad,dif,0,numof(msgPadNo));
                return RCODE_NONE;
                
              case MENU_PCE_BT6:
                _TOGGLE(eConf.pce_bt6);
                
                if(eConf.pce_bt6) menu_update_keyconf(KEYCONF_HW_PC6,pce_menu);
                else              menu_update_keyconf(KEYCONF_HW_PCE,pce_menu);
                
                return RCODE_UPDATE;
                
              case MENU_PCE_TOC:
                eConf.pce_toc[0]=0;
                if(getFilePath(eConf.pce_toc,pLastTocPath,toc_ext)>0) {
                    return RCODE_UPDATE;
                }
                break;
            }
        }
    }
    

    return RCODE_NONE;
}

//-----------------------------------------------------------------------------
// WonderSwanの設定
//-----------------------------------------------------------------------------
static RCODE wse_handler(int key,struct menuitem* pItem)
{
    int dif=0;
    static char* msgWseCtrl[]= { "normal", "flip","user","auto" };
    static char* msgWseVrot[]= { "normal", "flip","auto" };
    static char* msgMono[]   = { "white","amber","green","blue" };
    static int key_mode = 0; // メニューでのキーモード選択(0,1,2)

    // Key Mode Chagne
    void wse_kmch(int m) {
        switch(m) {
          case 0: menu_update_keyconf(KEYCONF_HW_WSN,wse_menu); break;
          case 1: menu_update_keyconf(KEYCONF_HW_WSF,wse_menu); break;
          case 2: menu_update_keyconf(KEYCONF_HW_WSU,wse_menu); break;
        }
    };
    
    if(key==KEY_MENU_ENTER) {
        if((key_mode = eConf.wse_control)==3){
            key_mode = isWonderSwanRotate();
        }
        
        wse_kmch(key_mode);
        
        return RCODE_NONE;
    }

    if(key==KEY_MENU_LEAVE) {
        return RCODE_NONE;
    }
    
    if(key==KEY_SELECT) {
        return RCODE_NONE;
    }

    if(key==KEY_UPDATE) {
        switch(pItem->id) {
          case MENU_WSE_CTRL : {
              switch(eConf.wse_control){
                case 0: pItem->sub_str="normal"; break;
                case 1: pItem->sub_str="flip";   break;
                case 2: pItem->sub_str="user";   break;
                case 3:
                  if(key_mode==0) pItem->sub_str="normal(auto)";
                  else            pItem->sub_str="flip(auto)";
                  break;
              }
          }
            
            if((key_mode = eConf.wse_control)==3){
                key_mode = isWonderSwanRotate();
            }
            wse_kmch(key_mode);
            break;
            
          case MENU_WSE_MONO:
            pItem->sub_str = msgMono[eConf.wse_mono];
            break;
            
          case MENU_WSE_VROT :
            pItem->sub_str = msgWseVrot[eConf.wse_vrotate];
            break;
        }
        return RCODE_NONE;
    }
    
    if( key==CTRL_CIRCLE || key==CTRL_TRIANGLE ) {
        if(key==CTRL_CIRCLE)   dif=+1;
        if(key==CTRL_TRIANGLE) dif=-1;
        
        if(dif) {
            switch(pItem->id) {
              case MENU_WSE_VROT:
                _LOOP(eConf.wse_vrotate,dif,0,3);
                return RCODE_UPDATE;

              case MENU_WSE_MONO:
                _LOOP(eConf.wse_mono,dif,0,numof(msgMono));
                ws_set_colour_scheme(eConf.wse_mono);
                return RCODE_UPDATE;
                
              case MENU_WSE_CTRL:
                _LOOP(eConf.wse_control,dif,0,4);
                return RCODE_UPDATE;
            }
        }
    }
    return RCODE_NONE;
}


//-----------------------------------------------------------------------------
//
//
//-----------------------------------------------------------------------------
static RCODE com_handler(int key,struct menuitem* pItem)
{
    static char *msgVideo[]={"CPU","GPU","GPU FIT","GPU FULL"};
    static char *msgClock[]={"333MHz","266MHz","222MHz"};
    static char *msgLimit[]={"AUTO","NO CONTROL"};
    static char *msgVsync[]={"NO","YES"};

    if(key==KEY_MENU_ENTER) {
        return RCODE_NONE;
    }

    if(key==KEY_MENU_LEAVE) {
        return RCODE_NONE;
    }
    
    if(key==KEY_SELECT) {
        return RCODE_NONE;
    }
    
    if(key==CTRL_CIRCLE) {
        switch(pItem->id) {
          case MENU_COM_CPU:   _LOOP(eConf.clock,1,0,numof(msgClock));  return RCODE_UPDATE;
          case MENU_COM_VIDEO: _LOOP(eConf.video,1,0,numof(msgVideo));  return RCODE_UPDATE;
          case MENU_COM_FPS:   _TOGGLE(eConf.fps);                      return RCODE_UPDATE;
          case MENU_COM_SOUND: _TOGGLE(eConf.sound);                    return RCODE_UPDATE;
          case MENU_COM_LIMIT: _LOOP(eConf.limit,1,0,2);                return RCODE_UPDATE;
          case MENU_COM_VSYNC: _TOGGLE(eConf.vsync);                    return RCODE_UPDATE;
          default:
            break;
        }
    }

    if( key==KEY_UPDATE )  {
        switch(pItem->id) {
          case MENU_COM_CPU:   pItem->sub_str = msgClock[eConf.clock];  break;
          case MENU_COM_VIDEO: pItem->sub_str = msgVideo[eConf.video];  break;
          case MENU_COM_SOUND: pItem->sub_str = msgOnOff[eConf.sound];  break;

          case MENU_COM_LIMIT: pItem->sub_str = msgLimit[eConf.limit];  break;
          case MENU_COM_VSYNC: pItem->sub_str = msgVsync[eConf.vsync];  break;
          case MENU_COM_FPS:   pItem->sub_str = msgOnOff[eConf.fps];    break;
        }
    }
    
    return RCODE_NONE;
}

//-----------------------------------------------------------------------------
//
//
//-----------------------------------------------------------------------------
static RCODE main_handler(int key,struct menuitem* pItem)
{
    if(key==KEY_MENU_ENTER) {
        return RCODE_NONE;
    }

    if(key==KEY_MENU_LEAVE) {
        return RCODE_NONE;
    }

    if(key==CTRL_START) {
        if(g_pCoreHdl) {
            return RCODE_CONTINUE;
        }
        return RCODE_NONE;
    }
    
    if(key==KEY_SELECT) {
        return RCODE_NONE;
    }

    if(key==KEY_UPDATE) {
        switch(pItem->id) {
          case MENU_MAIN_LOAD_STATE:  IF_SELECTABLE(pItem,g_pCoreHdl && g_pCoreHdl->pLOAD); break;
          case MENU_MAIN_SAVE_STATE:  IF_SELECTABLE(pItem,g_pCoreHdl && g_pCoreHdl->pSAVE); break;
          case MENU_MAIN_RESET:       IF_SELECTABLE(pItem,g_pCoreHdl);                      break;
          case MENU_MAIN_CONTINUE:    IF_SELECTABLE(pItem,g_pCoreHdl);                      break;
        }
        return RCODE_NONE;
    }

    if( key==CTRL_CIRCLE ) {

        switch(pItem->id) {
          case MENU_MAIN_ROM_SELECT: {
              if(getFilePath(RomName,pLastRomPath,ext_list)>0) {
                  return RCODE_ROM;
              }
          } break;

          case MENU_MAIN_COMMON:     set_menu(&menulist_psp);  break;
          case MENU_MAIN_PCE_CONFIG: set_menu(&menulist_pce);  break;
          case MENU_MAIN_WSE_CONFIG: set_menu(&menulist_wse);  break;
          case MENU_MAIN_NES_CONFIG: set_menu(&menulist_nes);  break;
          case MENU_MAIN_GBC_CONFIG: set_menu(&menulist_gbc);  break;
          case MENU_MAIN_NGP_CONFIG: set_menu(&menulist_ngp);  break;
          case MENU_MAIN_SMS_CONFIG: set_menu(&menulist_sms);  break;
          case MENU_MAIN_LOAD_STATE: set_menu(&menulist_load); break;
          case MENU_MAIN_SAVE_STATE: set_menu(&menulist_save); break;
          case MENU_MAIN_CONTINUE:   return RCODE_CONTINUE;    
          case MENU_MAIN_RESET:      return RCODE_RESET;       
          case MENU_MAIN_EXIT:       return RCODE_QUIT;        
          default:
            return RCODE_NONE;
        }
        return RCODE_UPDATE;
    }
    


    return RCODE_NONE;
}


static void SetCpuClock(void)
{
    int clk=222;
    switch(eConf.clock) {
      default:eConf.clock=0;
      case 0: clk=333; break;
      case 1: clk=266; break;
      case 2: clk=222; break;
    }
    pgSetClock(clk);
}

//--------------------------------------------------------------------------
// 以下の切り替えタイミングで呼び出される関数
// エミュレータ⇒(1)⇒メニュー⇒(0)⇒エミュレータ
//--------------------------------------------------------------------------
#define MENU_ENTER() {  menu_enter_leave(1); }
#define MENU_LEAVE() {  menu_enter_leave(0); }
//--------------------------------------------------------------------------
static int menu_enter_leave(int bEnter)
{
    if(bEnter) {
        // ENTER MENU
        eConf.sound &= 1;
        pgSetClock(333);

        // for menu
        flag_loadstate=0;
        set_menu(&menulist_main);

        thumbnail_update( TNMODE_EMULATE );
        
        StateFunc(STATE_MEM_SAVE,0); 
        
    }
    else {
        // ESCAPE MENU
        eConf.sound |= 2;

        if(!flag_loadstate) {
            StateFunc(STATE_MEM_LOAD,0);
        }
        
        SetCpuClock();
    }
    pgCls(0);

    return 1;
}

//--------------------------------------------------------------------------
// メニュー表示関数
// この関数で全てのメニュー表示を行う
// 
// サムネイルについて
// (1) Load & Save メニューではファイルのサムネイル表示を行う
// (2) (1)以外ではエミュレート中のサムネイル表示を行う
// 
//--------------------------------------------------------------------------
// [memo]
// MENUからSTARTキー(CONTINUE)で復帰できない問題があったが
// NESのステート管理(厳密にはHAL_sts_xxx)のバグだった。
//--------------------------------------------------------------------------
int ProcMenu(void)
{
    int pad=-1;
    MENUITEM *pITEM,*pLast=0;
    RCODE rcode;

    int menu_loop = 1;
    int menu_retcode = 0;

    MENU_ENTER();

    while(menu_loop){
        PSP_Power_CheckSleep();
        
        if(PSP_IsEsc()) {
            menu_retcode = STATE_QUIT;
            break;
        }

        if( (pITEM = &menu_ptr[menu_cur]) != pLast ) {
            pITEM->pFunc(KEY_SELECT,pITEM);
            pLast=pITEM;
        }

        if(pad) {
            print_menu_frame(menu_str,menu_ptr,menu_cur);
            pgScreenFlipV();
            if(pad==-1) { while(readpad_now()); }
            pad=0;
            continue;
        }

        if( (pad = readpad_new())==0 ) {
            pgWaitVn(1);
            continue;
        }

        if(pad & (CTRL_UP|CTRL_DOWN) ) {
            if(pad & CTRL_UP  ) menu_cur--;
            if(pad & CTRL_DOWN) menu_cur++;
            if(menu_cur<0) { menu_cur=menu_item_max; }
            else           { if(menu_cur>menu_item_max) menu_cur=0; }
        }
        else if( pad & CTRL_CROSS ) {
            /* 直前のメニューに戻った方が良いかも？ */
            if(menu_ptr != main_menu) {
                set_menu(&menulist_main);
            }
        }
        else if( (pITEM->color&0x8000) && pITEM->pFunc ) {

            rcode = pITEM->pFunc(pad,pITEM);
            
            switch(rcode) {
              case RCODE_UPDATE:
                pITEM->pFunc(KEY_UPDATE,pITEM);
                break;
                
              case RCODE_LOADSTATE:
                flag_loadstate=1; // state overload
                menu_retcode = STATE_CONT;
                menu_loop=0;
                break;
                
              case RCODE_CONTINUE:
                menu_retcode = STATE_CONT;
                menu_loop = 0;
                break;
                
              case RCODE_ESCAPE:
                menu_retcode = STATE_QUIT;
                menu_loop = 0;
                break;
                
              case RCODE_RESET:
                menu_retcode = STATE_RESET;
                menu_loop = 0;
                break;
                
              case RCODE_QUIT:
                menu_retcode = STATE_QUIT;
                menu_loop = 0;
                break;

              case RCODE_ROM:
                menu_retcode=STATE_ROM;
                menu_loop = 0;
                break;
                
              case RCODE_NONE:
                break;
                
              default:
                *(u32*)0x11111111=rcode;
                break;
            }
            
            
        }
    }
    
    while(readpad_now());
    
    MENU_LEAVE();

    return menu_retcode;
}

//--------------------------------------------------------------------------
// EMULATOR CORE install
//--------------------------------------------------------------------------
void core_install(void)
{
#if defined(CORE_PCE)
    PCE_Setup();
#endif
#if defined(CORE_WS)
    SWAN_Setup();
#endif
#if defined(CORE_NES)
    NES_Setup();
#endif
#if defined(CORE_NGP)
    NGP_Setup();
#endif
#if defined(CORE_SMS)
    SMS_Setup();
#endif
#if defined(CORE_GBC)
    GBC_Setup();
#endif
#if defined(CORE_TXT)
    TXT_Setup();
#endif
#if defined(CORE_MP3)
    MP3_Setup();
#endif
#if defined(CORE_LNX)
    LNX_Setup();
#endif
}


//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
void menu_Init(int bInit)
{
    core_strcpy(pLastRomPath,pguGetWorkdir());
    core_strcpy(pLastTocPath,pguGetWorkdir());
    core_strcpy(eConf.pce_toc,pLastTocPath);

    if(bInit) {
        core_memset(&eConf,0,sizeof(eConf));
        eConf.sound=1;
        eConf.fps=1;
        eConf.pce_pad = 0;
        core_strcpy(pLastTocPath,eConf.pce_toc);
        eConf.wse_control=0;
        eConf.wse_vrotate=0;
        eConf.vsync=1;
        eConf.limit=0;
    }

    if( core_strlen(eConf.pce_toc)==0 ) {
        core_strcpy(eConf.pce_toc,"ms0:/");
        core_strcpy(pLastTocPath,"ms0:/");
    }
    
}

//--------------------------------------------------------------------------
// main routine
//--------------------------------------------------------------------------
int pspMain(int argc,char **argv)
{
    int retcode,i;
    byte* pRomAddr=0;
    int   nRomSize=0;
    int fRomChange=0;
    int fCoreLoop=0;
    int fMenuLoop=1;
    int romext=EXT_NULL;

    HAL_SetWorkPath(argv[0]);
    
    pgMain(argc,argv[0]);
    setup();

    tmp_buffer = HAL_mem_malloc( DEFSIZE_TMPBUFFER );
    
    menu_Init( !ConfigFile(1) );
    HAL_Sound_Init();

    core_install();

    //--
    // メニュー関連の処理
    //--
    int e_menu(void) {
        retcode = ProcMenu();
        
        switch(retcode) {
          case STATE_ROM:
            fRomChange=1;
            fCoreLoop=0;
            return 1;
            
          case STATE_CONT:
            if(g_pCoreHdl) {
                return 1;
            }
            break;
          case STATE_RESET:
            if(g_pCoreHdl){
                g_pCoreHdl->pRESET();
                return 1;
            }
            break;
          case STATE_QUIT:
            fCoreLoop=0;
            fMenuLoop=0;
            return 0;
            break;
          default:
            *(int*)0x100 = retcode;
            break;
        }
        return 1;
    }

    //------------
    // MAIN LOOP
    //------------
    while(fMenuLoop){
        
        // Romがメモリに存在すれば起動
        if(pRomAddr && g_pCoreHdl) {
            //===================================
            // emulator main loop
            //===================================
            if(g_pCoreHdl->pINIT(nRomSize,pRomAddr)) {
                g_pCoreHdl->pRESET();
                while(fCoreLoop) {
                    if(g_pCoreHdl->pLOOP()) {
                        e_menu();
                    }
                    PSP_Power_CheckSleep();
                }
                g_pCoreHdl->pEXIT();
            }

            CartFree(pRomAddr);
            pRomAddr=0;
            nRomSize=0;
            g_pCoreHdl=0;
        }
        else {
            if(fRomChange) {
                fRomChange=0;

                pgCls(0);
                CENTER_PRINT("Now Loading...");
                pgScreenFlip();

                romext = CartLoad(RomName,&pRomAddr,&nRomSize,&rom_state);
                g_pCoreHdl = GetCoreHandlerFromType(romext);
                fCoreLoop=1;
            } else {
                e_menu();
            }
        }

        if(PSP_IsEsc()) {
            fMenuLoop=0;
        }
    }

    if(tmp_buffer) {
        HAL_mem_free(tmp_buffer);
        tmp_buffer=0;
    }
    
    
    pspExit();
    
    return 0;
}

