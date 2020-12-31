enum {
    MENU_UNDEF             = 0x00,
    MENU_MAIN_ROM_SELECT,
    MENU_MAIN_PCE_CONFIG   ,
    MENU_MAIN_WSE_CONFIG   ,
    MENU_MAIN_GBC_CONFIG   ,
    MENU_MAIN_NES_CONFIG   ,
    MENU_MAIN_NGP_CONFIG   ,
    MENU_MAIN_SMS_CONFIG   ,

    MENU_MAIN_LOAD_STATE   ,
    MENU_MAIN_SAVE_STATE   ,
    MENU_MAIN_CONTINUE     ,
    MENU_MAIN_COMMON       ,
    MENU_MAIN_RESET        ,
    
    MENU_STATE_L0          ,
    MENU_STATE_L1          ,
    MENU_STATE_L2          ,
    MENU_STATE_L3          ,
    MENU_STATE_L4          ,
    MENU_STATE_L5          ,
    MENU_STATE_L6          ,
    MENU_STATE_L7          ,
    MENU_STATE_L8          ,
    MENU_STATE_L9          ,

    MENU_STATE_S0          ,
    MENU_STATE_S1          ,
    MENU_STATE_S2          ,
    MENU_STATE_S3          ,
    MENU_STATE_S4          ,
    MENU_STATE_S5          ,
    MENU_STATE_S6          ,
    MENU_STATE_S7          ,
    MENU_STATE_S8          ,
    MENU_STATE_S9          ,

    MENU_COM_CPU           ,
    MENU_COM_VIDEO         ,
    MENU_COM_SOUND         ,
    MENU_COM_FPS           ,
    MENU_COM_LIMIT         ,
    MENU_COM_VSYNC         ,
      
    MENU_PCE_TOC           ,
    MENU_PCE_PADNO         ,
    MENU_PCE_BT6           ,
    
    MENU_WSE_CTRL          , // コントローラの方向
    MENU_WSE_MONO          , // モノクロカラー設定
    MENU_WSE_VROT          , // ビデオ表示の方向
    MENU_WSE_KCONF         , // キーコンフィグ

//    MENU_MAIN_keyconfig,
  
    MENU_MAIN_EXIT         = 0xfe
};





typedef struct {
    int tag;
    int size;
} TAG_HEADER_T;

typedef struct {
    short w,h;
} TAG_IMAGE_T;

typedef struct {
    int   key;
    char* name;
} KEYDEF;

typedef struct {
    unsigned char r,g,b;
} COLOR;

typedef struct {
    int x,y,c;
} POINT;



typedef struct menuitem {
    char*    string;
    char*    sub_str;
    int      sub_prm;
    int      color;
    int      id;
    int     (*pFunc)(int key,struct menuitem* pItem);
    int      opt;
} MENUITEM;


typedef struct menulist {
  char* title;                 
  int (*pHandler)(int key);    
  MENUITEM* pMenuItem;         
  
  
} MENULIST;

