// 
// main.c‚©‚çinclude‚³‚ê‚é–‚ğŠú‘Ò‚µ‚Ä‚¢‚Ü‚·
// 

#define COLOR_SELECTABLE  (RGB_WHITE | 0x8000)
#define COLOR_DISABLE     (RGB_GRAY & ~0x8000)
#define COLOR_BLUEBLACK   (RGB_GRAY  | 0x8000)

#define IF_SELECTABLE(pI,isCode)     { (pI)->color = (isCode) ? COLOR_SELECTABLE : COLOR_DISABLE; }


static char *msgOnOff[]={"Off","On"};


char *menu_frame_string[] = {
  "„¬„ª„ª„ª„ª„ª„ª„ª„ª„ª„ª„ª„ª„ª„ª„ª„ª„ª„ª„ª„ª„ª„ª„ª„ª„ª„ª„ª„ª„ª„ª„ª„ª„ª„ª„ª„ª„ª„ª„ª„ª„ª„ª„ª„ª„ª„ª„­",
  "„«                @                                                                          „«",
  "„°„ª„ª„ª„ª„ª„ª„ª„ª„ª„ª„ª„ª„ª„ª„ª„ª„ª„ª„ª„ª„ª„ª„ª„ª„ª„ª„ª„ª„ª„ª„ª„ª„ª„ª„ª„ª„ª„ª„ª„ª„ª„ª„ª„ª„ª„±„²",
  "„«                @                                                                        „«„«",
  "„«                                                                                          „«„«",
  "„«                                                                                          „«„«",
  "„«                                                                                          „«„«",
  "„«                @                                                                        „«„«",
  "„«               @                                                                         „«„«",
  "„«                                                                                          „«„«",
  "„«                @                                                                        „«„«",
  "„«                @                                                                        „«„«",
  "„«                @                                                                        „«„«",
  "„«                @                                                                        „«„«",
  "„«                @                                                                        „«„«",
  "„«                @                                                                        „«„«",
  "„«                @                                                                        „«„«",
  "„«                @                                                                        „«„«",
  "„«                @                                                                        „«„«",
  "„«                @                                                                        „«„«",
  "„«                @                                                                        „«„«",
  "„«                @                                                                        „«„«",
  "„«                @                                                                        „«„«",
  "„«                @                                                                        „«„«",
  "„°„ª„ª„ª„ª„ª„ª„ª„ª„ª„ª„ª„ª„ª„ª„ª„ª„ª„ª„ª„ª„ª„ª„ª„ª„ª„ª„ª„ª„ª„ª„ª„ª„ª„ª„ª„±„ª„ª„ª„ª„ª„ª„ª„ª„ª„³„²",
  "„«                                                                      „« e[mulator] for PSP „«",
  "„¯„ª„ª„ª„ª„ª„ª„ª„ª„ª„ª„ª„ª„ª„ª„ª„ª„ª„ª„ª„ª„ª„ª„ª„ª„ª„ª„ª„ª„ª„ª„ª„ª„ª„ª„ª„³„ª„ª„ª„ª„ª„ª„ª„ª„ª„ª„®",
  0
};


//-----------------------------------------------------------------------------
// Key Configuration (Default setting)
//-----------------------------------------------------------------------------
KEY_CONFIG g_KeyCfg = {
    {{  /* PCE */
        PCE_U,PCE_R,PCE_D,PCE_L,     // Digital (0-3)
        PCE_U,PCE_R,PCE_D,PCE_L,     // Analog  (4-7)
        30,                          // £
        PCE_1,                       // œ
        PCE_2,                       // ~
        30,                          // ¡
        31,                          // L trigger
        31,                          // R trigger
        PCE_SEL,PCE_RUN              // Select, Start
      } , {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
    },
    {{  /* PC6 */
        PCE_U,PCE_R,PCE_D,PCE_L,     // Digital (0-3)
        PCE_U,PCE_R,PCE_D,PCE_L,     // Analog  (4-7)
        PCE_6,                       // £
        PCE_1,                       // œ
        PCE_2,                       // ~
        PCE_5,                       // ¡
        PCE_3,                       // L trigger
        PCE_4,                       // R trigger
        PCE_SEL,PCE_RUN              // Select, Start
      } , {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
    },
    {{  /* NES */
        NES_U,NES_R,NES_D,NES_L,     // Digital (0-3)
        NES_U,NES_R,NES_D,NES_L,     // Analog  (4-7)
        30,                          // £
        NES_A,                       // œ
        NES_B,                       // ~
        30,                          // ¡
        30,                          // L trigger
        30,                          // R trigger
        NES_SEL,NES_STA              // Select, Start
      } , {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
    },
    {{  /* GBC */
        GBC_U,GBC_R,GBC_D,GBC_L,     // Digital (0-3)
        GBC_U,GBC_R,GBC_D,GBC_L,     // Analog  (4-7)
        30,                          // £
        GBC_A,                       // œ
        GBC_B,                       // ~
        30,                          // ¡
        30,                          // L trigger
        30,                          // R trigger
        GBC_SEL,GBC_STA              // Select, Start
      } , {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
    },
    {{  /* WSN(WonderSwan Normal) */
        WSC_XU,WSC_XR,WSC_XD,WSC_XL, // Digital (0-3)
        WSC_YU,WSC_YR,WSC_YD,WSC_YL, // Analog  (4-7)
        30,                          // £
        WSC_A,                       // œ
        WSC_B,                       // ~
        30,                          // ¡
        30,                          // L trigger
        30,                          // R trigger
        30,WSC_S                     // Select, Start
      } , {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
    },
    {{ /* WSF(WonderSwan flip) */
        WSC_YR,WSC_YD,WSC_YL,WSC_YU, // Digital (0-3)
        WSC_A,WSC_A,WSC_B,WSC_B,     // Analog  (4-7)
        WSC_XR,                      // £
        WSC_XD,                      // œ
        WSC_XL,                      // ~
        WSC_XU,                      // ¡
        30,                          // L trigger
        30,                          // R trigger
        30,WSC_S                     // Select, Start
      } , {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
    },
    {{ /* WSF(WonderSwan User) : PSP‚ğc‚É‚Á‚Ä—V‚Ô‚Ì‚Ég‚¤ */
        WSC_YU,WSC_YR,WSC_YD,WSC_YL, // Digital (0-3)
        WSC_XU,WSC_XR,WSC_XD,WSC_XL, // Analog  (4-7)
        30,                          // £
        WSC_A,                       // œ
        WSC_B,                       // ~
        30,                          // ¡
        30,                          // L trigger
        30,                          // R trigger
        30,WSC_S                     // Select, Start
      } , {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
    },
    {{  /* NEOGEO Pocket */
        NGP_U,NGP_R,NGP_D,NGP_L,     // Digital (0-3)
        NGP_U,NGP_R,NGP_D,NGP_L,     // Analog  (4-7)
        31,                          // £
        NGP_B,                       // œ
        NGP_A,                       // ~
        31,                          // ¡
        31,                          // L trigger
        31,                          // R trigger
        31,NGP_O                     // Select, Start
      } , {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
    },
    {{  /* GameGear */
        SMS_U,SMS_R,SMS_D,SMS_L,     // Digital (0-3)
        SMS_U,SMS_R,SMS_D,SMS_L,     // Analog  (4-7)
        31,                          // £
        SMS_2,                       // œ
        SMS_1,                       // ~
        31,                          // ¡
        31,                          // L trigger
        31,                          // R trigger
        31,SMS_STA                   // Select, Start
      } , {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
    },
    {{  /* Lynx */
        LNX_U,LNX_R,LNX_D,LNX_L,     // Digital (0-3)
        LNX_U,LNX_R,LNX_D,LNX_L,     // Analog  (4-7)
        31,                          // £
        LNX_o,                       // œ
        LNX_i,                       // ~
        31,                          // ¡
        31,                          // L trigger
        31,                          // R trigger
        LNX_1,LNX_2                  // Select, Start
      } , {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
    }
};

//
static char* pce_k[32] = {
    "I     ","II    ","SELECT","RUN   ","ª    ","¨    ","«    ","©    ",
    "III   ","VI    ","V     ","IV    ",0       ,0       ,0       ,0       ,
    0       ,0       ,0       ,0       ,0       ,0       ,0       ,0       ,
    0       ,0       ,0       ,0       ,0       ,0       ,"undef ","MENU  "
};


static char* nes_k[32] = {
    "‚`    ","‚a    ","SELECT","START ","ª    ","«    ","©    ","¨    ",
    0       ,0       ,0       ,0       ,0       ,0       ,0       ,0       ,
    0       ,0       ,0       ,0       ,0       ,0       ,0       ,0       ,
    0       ,0       ,0       ,0       ,0       ,0       ,"undef ","MENU  "
  };

static char* gbc_k[32] = {
    "‚`    ","‚a    ","SELECT","START ","«    ","ª    ","©    ","¨    ",
    0       ,0       ,0       ,0       ,0       ,0       ,0        ,0      ,
    0       ,0       ,0       ,0       ,0       ,0       ,0        ,0      ,
    0       ,0       ,0       ,0       ,0       ,0       ,"undef ","MENU  "
  };

static char* wsc_k[32] = {
    0       ,"START ","‚`    ","‚a    ","X ª  ","X ¨  ","X «  ","X ©  ",
    "Y ª  ","Y ¨  ","Y «  ","Y ©  ",0       ,0       ,0       ,0       ,
    0       ,0       ,0       ,0       ,0       ,0       ,0       ,0       ,
    0       ,0       ,0       ,0       ,0       ,0       ,"undef ","MENU  "
  };

static char* ngp_k[32] = {
    "ª    ","«    ","©    ","¨    ","‚`    ","‚a    ","OPTION",0       ,
    0       ,0       ,0       ,0       ,0       ,0       ,0       ,0       ,
    0       ,0       ,0       ,0       ,0       ,0       ,0       ,0       ,
    0       ,0       ,0       ,0       ,0       ,0       ,"undef ","MENU  "
  };

static char* sms_k[32] = {
    "ª    ","«    ","©    ","¨    ","‚Q    ","‚P    ",0       ,"RESET ",
    0       ,0       ,0       ,0       ,0       ,0       ,0       ,0       ,
    "START" ,"PAUSE" ,"SRESET","HRESET",0       ,0       ,0       ,0       ,
    0       ,0       ,0       ,0       ,0       ,0       ,"undef ","MENU  "
  };

enum {
    KEYCONF_HW_PCE=0,
    KEYCONF_HW_PC6 ,
    KEYCONF_HW_NES ,
    KEYCONF_HW_GBC ,
    KEYCONF_HW_WSN ,
    KEYCONF_HW_WSF ,
    KEYCONF_HW_WSU ,
    KEYCONF_HW_NGP ,
    KEYCONF_HW_SMS ,
};

#define KEYID_BEGIN 100


#define KEY_MENU_LIST(HANDLER) \
    {" ª          "     ,g_ks[0x0],0,COLOR_SELECTABLE, KEYID_BEGIN+0 ,HANDLER, 0},\
    {" ¨          "     ,g_ks[0x1],0,COLOR_SELECTABLE, KEYID_BEGIN+1 ,HANDLER, 0},\
    {" «          "     ,g_ks[0x2],0,COLOR_SELECTABLE, KEYID_BEGIN+2 ,HANDLER, 0},\
    {" ©          "     ,g_ks[0x3],0,COLOR_SELECTABLE, KEYID_BEGIN+3 ,HANDLER, 0},\
    {" ª (analog) "     ,g_ks[0x4],0,COLOR_SELECTABLE, KEYID_BEGIN+4 ,HANDLER, 0},\
    {" ¨ (analog) "     ,g_ks[0x5],0,COLOR_SELECTABLE, KEYID_BEGIN+5 ,HANDLER, 0},\
    {" « (analog) "     ,g_ks[0x6],0,COLOR_SELECTABLE, KEYID_BEGIN+6 ,HANDLER, 0},\
    {" © (analog) "     ,g_ks[0x7],0,COLOR_SELECTABLE, KEYID_BEGIN+7 ,HANDLER, 0},\
    {" £          "     ,g_ks[0x8],0,COLOR_SELECTABLE, KEYID_BEGIN+8 ,HANDLER, 0},\
    {" œ          "     ,g_ks[0x9],0,COLOR_SELECTABLE, KEYID_BEGIN+9 ,HANDLER, 0},\
    {" ~          "     ,g_ks[0xa],0,COLOR_SELECTABLE, KEYID_BEGIN+10 ,HANDLER, 0},\
    {" ¡          "     ,g_ks[0xb],0,COLOR_SELECTABLE, KEYID_BEGIN+11 ,HANDLER, 0},\
    {" L           "     ,g_ks[0xc],0,COLOR_SELECTABLE, KEYID_BEGIN+12 ,HANDLER, 0},\
    {" R           "     ,g_ks[0xd],0,COLOR_SELECTABLE, KEYID_BEGIN+13 ,HANDLER, 0},\
    {" SELECT      "     ,g_ks[0xe],0,COLOR_SELECTABLE, KEYID_BEGIN+14 ,HANDLER, 0},\
    {" START       "     ,g_ks[0xf],0,COLOR_SELECTABLE, KEYID_BEGIN+15 ,HANDLER, 0}


///////////////////////////////////////////////////////////////////////////////
// MENU RESOURCE
///////////////////////////////////////////////////////////////////////////////
static RCODE load_handler(int key, struct menuitem *pItem);
static RCODE save_handler(int key, struct menuitem *pItem);
static RCODE pce_handler (int key, struct menuitem* pItem);
static RCODE wse_handler (int key, struct menuitem* pItem);
static RCODE com_handler (int key, struct menuitem* pItem);
static RCODE nes_handler (int key, struct menuitem* pItem);
static RCODE gbc_handler (int key, struct menuitem* pItem);
static RCODE ngp_handler (int key, struct menuitem* pItem);
static RCODE sms_handler (int key, struct menuitem* pItem);
static RCODE main_handler(int key, struct menuitem* pItem);
static RCODE keyc_handler(int key, struct menuitem* pItem);


static MENUITEM main_menu[] = {
    {"ROM SELECT"            ,0,0,COLOR_SELECTABLE,  MENU_MAIN_ROM_SELECT ,main_handler,0},
    {"SAVE STATE"            ,0,0,COLOR_DISABLE,     MENU_MAIN_SAVE_STATE ,main_handler,0},
    {"LOAD STATE"            ,0,0,COLOR_DISABLE,     MENU_MAIN_LOAD_STATE ,main_handler,0},
    {"COMMON SETUP"          ,0,0,COLOR_SELECTABLE,  MENU_MAIN_COMMON     ,main_handler,0},
    
    {"PC-Engine"             ,0,0,COLOR_SELECTABLE,  MENU_MAIN_PCE_CONFIG ,main_handler,0},
    {"WonderSwan"            ,0,0,COLOR_SELECTABLE,  MENU_MAIN_WSE_CONFIG ,main_handler,0},
    {"Family Computer"       ,0,0,COLOR_SELECTABLE,  MENU_MAIN_NES_CONFIG ,main_handler,0},
    {"Gameboy"               ,0,0,COLOR_SELECTABLE,  MENU_MAIN_GBC_CONFIG ,main_handler,0},
    {"NEOGEO Pocket"         ,0,0,COLOR_SELECTABLE,  MENU_MAIN_NGP_CONFIG ,main_handler,0},
    {"GameGear(MasterSystem)",0,0,COLOR_SELECTABLE,  MENU_MAIN_SMS_CONFIG ,main_handler,0},
    
    {"RESET"                 ,0,0,COLOR_DISABLE,     MENU_MAIN_RESET      ,main_handler,0},
    {"CONTINUE"              ,0,0,COLOR_DISABLE,     MENU_MAIN_CONTINUE   ,main_handler,0},
    {"EXIT"                  ,0,0,COLOR_SELECTABLE,  MENU_MAIN_EXIT       ,main_handler,0},
    {0,0,0,0,0}
};

static MENUITEM com_menu[] = {
    {"CPU"               ,0,0,COLOR_SELECTABLE,  MENU_COM_CPU     ,com_handler,0},
    {"VIDEO"             ,0,0,COLOR_SELECTABLE,  MENU_COM_VIDEO   ,com_handler,0},
    {"SOUND"             ,0,0,COLOR_SELECTABLE,  MENU_COM_SOUND   ,com_handler,0},
    {"FRAME Limitter"    ,0,0,COLOR_SELECTABLE,  MENU_COM_LIMIT   ,com_handler,0},
    {"VSYNC WAIT"        ,0,0,COLOR_SELECTABLE,  MENU_COM_VSYNC   ,com_handler,0},
    {"Show fps"          ,0,0,COLOR_SELECTABLE,  MENU_COM_FPS     ,com_handler,0},
    {0,0,0,0,0}
};


static MENUITEM load_menu[] = {
    {"STATE 0"           ,0,0,COLOR_DISABLE,  MENU_STATE_L0, load_handler, 0},
    {"STATE 1"           ,0,0,COLOR_DISABLE,  MENU_STATE_L1, load_handler, 1},
    {"STATE 2"           ,0,0,COLOR_DISABLE,  MENU_STATE_L2, load_handler, 2},
    {"STATE 3"           ,0,0,COLOR_DISABLE,  MENU_STATE_L3, load_handler, 3},
    {"STATE 4"           ,0,0,COLOR_DISABLE,  MENU_STATE_L4, load_handler, 4},
    {"STATE 5"           ,0,0,COLOR_DISABLE,  MENU_STATE_L5, load_handler, 5},
    {"STATE 6"           ,0,0,COLOR_DISABLE,  MENU_STATE_L6, load_handler, 6},
    {"STATE 7"           ,0,0,COLOR_DISABLE,  MENU_STATE_L7, load_handler, 7},
    {"STATE 8"           ,0,0,COLOR_DISABLE,  MENU_STATE_L8, load_handler, 8},
    {"STATE 9"           ,0,0,COLOR_DISABLE,  MENU_STATE_L9, load_handler, 9},
    {0,0,0,0,0}
};

static MENUITEM save_menu[] = {
    {"STATE 0"           ,0,0,COLOR_BLUEBLACK,  MENU_STATE_S0, save_handler, 0},
    {"STATE 1"           ,0,0,COLOR_BLUEBLACK,  MENU_STATE_S1, save_handler, 1},
    {"STATE 2"           ,0,0,COLOR_BLUEBLACK,  MENU_STATE_S2, save_handler, 2},
    {"STATE 3"           ,0,0,COLOR_BLUEBLACK,  MENU_STATE_S3, save_handler, 3},
    {"STATE 4"           ,0,0,COLOR_BLUEBLACK,  MENU_STATE_S4, save_handler, 4},
    {"STATE 5"           ,0,0,COLOR_BLUEBLACK,  MENU_STATE_S5, save_handler, 5},
    {"STATE 6"           ,0,0,COLOR_BLUEBLACK,  MENU_STATE_S6, save_handler, 6},
    {"STATE 7"           ,0,0,COLOR_BLUEBLACK,  MENU_STATE_S7, save_handler, 7},
    {"STATE 8"           ,0,0,COLOR_BLUEBLACK,  MENU_STATE_S8, save_handler, 8},
    {"STATE 9"           ,0,0,COLOR_BLUEBLACK,  MENU_STATE_S9, save_handler, 9},
    {0,0,0,0,0}
};


static MENUITEM pce_menu[] = {
    {"CDROM image"       ,0,0,COLOR_SELECTABLE, MENU_PCE_TOC  ,pce_handler,-1},
    {"Player Select"     ,0,0,COLOR_SELECTABLE, MENU_PCE_PADNO,pce_handler,-1},
    {"6 BUTTON"          ,0,0,COLOR_SELECTABLE, MENU_PCE_BT6  ,pce_handler,-1},
    KEY_MENU_LIST(keyc_handler),
    {0,0,0,0,0}
};



static MENUITEM wse_menu[] = {
    {"Video rotate"      ,0,0,COLOR_SELECTABLE, MENU_WSE_VROT ,wse_handler, 0},
    {"Mono color"        ,0,0,COLOR_SELECTABLE, MENU_WSE_MONO ,wse_handler, 0},
    {"Controler"         ,0,0,COLOR_SELECTABLE, MENU_WSE_CTRL ,wse_handler, 0},
    KEY_MENU_LIST(keyc_handler),
    {0,0,0,0,0}
};

static MENUITEM nes_menu[] = {
    KEY_MENU_LIST(keyc_handler),
    {0,0,0,0,0}
};

static MENUITEM gbc_menu[] = {
    KEY_MENU_LIST(keyc_handler),
    {0,0,0,0,0}
};

static MENUITEM ngp_menu[] = {
    KEY_MENU_LIST(keyc_handler),
    {0,0,0,0,0}
};

static MENUITEM sms_menu[] = {
    KEY_MENU_LIST(keyc_handler),
    {0,0,0,0,0}
};

MENULIST menulist_main={  "MAIN MENU ",    main_handler, main_menu };
MENULIST menulist_psp ={  "PSP SETUP ",    com_handler,  com_menu  };
MENULIST menulist_load={  "STATE LOAD",    load_handler, load_menu };
MENULIST menulist_save={  "STATE SAVE",    save_handler, save_menu };
MENULIST menulist_pce ={  "PC-Engine ",    pce_handler , pce_menu  };
MENULIST menulist_wse ={  "WonderSwan",    wse_handler , wse_menu  };
MENULIST menulist_nes ={  "Famicom"   ,    nes_handler , nes_menu  };
MENULIST menulist_gbc ={  "Gameboy"   ,    gbc_handler , gbc_menu  };
MENULIST menulist_ngp ={  "NEOGEO Pocket", ngp_handler , ngp_menu  };
MENULIST menulist_sms ={  "GameGear"     , sms_handler , sms_menu  };

