//------------------------------------------------------------------------------
// Wonderswan emulator based on OSwan
//------------------------------------------------------------------------------
// Modified 2006.11.18 by e (T.Kawamorita)
// 
//------------------------------------------------------------------------------


#define DEF_QUICK   
//#define DEBUG_QUICK // 一筆書き継続しているラインをひと目でチェック

//
// 速度低下の原因
// (1) DMAでパレット更新を行う場合の処理が若干無駄になっている
// (2) 
//

#include "hal.h"

#include "ws.h"
#include "nec.h"
#include "necintrf.h"
#include "initval.h"

#include "cstring.h"

#ifdef DEBUG_QUICK
PIXEL_FORMAT deb_qcol=0x1f;
#endif

#define PSG_VOLUME       80  // 120
#define VOICE_VOLUME     80  // 100
#define NOISE_VOLUME     380 // 400

#define BGFG_CLIP_MASK   0x30
#define BGFG_CLIP_SIDE   0x10

#define BGFG_CLIP_ENABLE 0x20
#define BGFG_CLIP_OUT    0x30
#define BGFG_CLIP_IN     0x20

#define VBLANK_BEGIN_LINE     145
#define SCANLINE_MAX          159

#define FBPTR(y) (fb_format.fb+(y)*fb_format.width+8)

static void drawLine(int beg,int end);
static FBFORMAT fb_format;

//-------------------------------------------------------------------
// TILE CACHE (temporary buffer)
//-------------------------------------------------------------------
typedef struct {
    u32 pal_flag;
    u8 wsc_n[0x10000];
    u8 wsc_h[0x10000];
    u8 modify[1024];
    
    PIXEL_FORMAT ws_shades[16];
    PIXEL_FORMAT system_c_pal[256]; // 環境依存用のパレットテーブル
    PIXEL_FORMAT system_m_pal[8];   // 環境依存用のパレットテーブル

	// 高速化に利用
	u32 trans[1024][8];
	u8 scanline_num[160];

} SYSTEM_CACHE_T;

//-------------------------------------------------------------------
// WonderSwan Structure : for State Save
//-------------------------------------------------------------------
typedef struct {
    u8 tag_iram[4];   // State Tag
    u8 iRam[0x10000];

	u8 tag_ioram[4];  // State Tag

	union {
	    u8 ioRam[0x100];
		struct {
			u8 disp_ctl;
			u8 bg_color;
			u8 line;
			u8 line_cmp;
			u8 spr_base;
			u8 spr_cnt;
			u8 tile_ram;
			window fg,spr;
			scroll bg_scr,fg_scr;
			u8 lcd;
			u8 lcd_icon;
		};
	};

    u8 tag_spr[4];    // State Tag
    u8 spTableCnt;
    u32 spTable[128];

    u8 videoMode;

    u8 tag_snd[4];    // State Tag
    // Audio Work Parameter
    int noise_k;
    int noise_v;
    int noise_r;
    int noise_c;
    int sweep_pitch;
    int sweep_step;
    int sweep_value;
    int sweep_upd;
    int sweep_count;
    int snd_n[4];
    int snd_k[4];

    // 上記までを一括保存し下記をサイズ計算して保存
    u8 tag_eE2P[4];    // State Tag
    u8 romE2P[0x800];   // E2P(16kb=2048Byte) in ROM Cart

    u8 tag_eRAM[4];    // State Tag
    u8 romRam[0x40000]; // RAM( 2Mb=256KByte) in ROM Cart

} WONDERSWAN_T;



static int ws_reset(void);
void ws_set_colour_scheme(int scheme);
static void ws_gpu_changeVideoMode(u8 value);
static void ws_gpu_clearCache(void);
static void ws_memory_init(u8 *rom, u32 wsRomSize);

// internal values
static WONDERSWAN_T* pWS=0;
static SYSTEM_CACHE_T* pWC=0;
static BYTE* ws_RomAddr = 0;
static DWORD ws_RomSize = 0;

// export functions
void cpu_writeport(DWORD port,BYTE value);
void cpu_writemem20(DWORD addr,BYTE value);
static void swan_sound(int ch,int do_cursor);
static void ws_voice_dma(void);

// export values
static u32 ws_key;
int rtcDataRegisterReadCount=0;
u8* pWsRomMap[0x10] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };

static int g_bgbgBase=0;
static int g_bgfgBase=0;

// テンポラリ領域
static u32 sramAddressMask;
static u32 romAddressMask;
static u32 eEepromAddressMask;

static int waveTable[SWAN_SOUND_CHANNELS][WAVE_SAMPLES];

#define VOLUME_R(ch)     (pWS->ioRam[0x88+ch]&15)
#define VOLUME_L(ch)    ((pWS->ioRam[0x88+ch]>>4)&15)
#define GET_PITCH(ch)   ((((int)pWS->ioRam[0x81+(ch)*2]&7)<<8)|((int)pWS->ioRam[0x80+(ch)*2]))

//------------------------------------------------------------------------------
//-
//------------------------------------------------------------------------------
static int ws_reset(void)
{
    if(pWS && pWC) {
		int i;

		ws_key = 0;
		core_memset(pWS->iRam,0,sizeof(pWS->iRam));
		ws_set_colour_scheme(0);  /* default scheme setup */

		pWS->spTableCnt=0;
		for(i=0;i<0x0c9;i++) {
			cpu_writeport(i,initialIoValue[i]);
		}
    
		rtcDataRegisterReadCount=0;
    
		/* ws_gpu_init */
		ws_gpu_clearCache();
		ws_gpu_changeVideoMode(2);

		nec_reset(NULL);
		nec_set_reg(NEC_SP,0x2000);
	}

	return 1;
}


//------------------------------------------------------------------------------
//-
//------------------------------------------------------------------------------
void ws_set_colour_scheme(int scheme)
{
    if(pWS) {
	    int r,g,b,i,R,G,B;
        switch(scheme) {
          default:
          case 0: r=100; g=100; b=100;  break;// white
          case 1: r= 60; g= 61; b=  0;  break;// amber
          case 2: r= 20; g= 90; b= 20;  break;// green
          case 3: r=  0; g= 61; b= 60;  break;// blue
        }
        
        for(i=0;i<16;i++) {
            R = (((15-i) * r)/100);
            G = (((15-i) * g)/100);
            B = (((15-i) * b)/100);
            pWC->ws_shades[i] = HAL_fb2_Color(R,G,B,RGB444);
        }
    }
}


//------------------------------------------------------------------------------
//- ビデオモードの扱いに問題があった。
//- 0 : 透明色は4,5,6,7パレットに存在, I/Oパレットを利用
//- 4 : 透明色は4,5,6,7パレットに存在, MEMパレットを利用
//- 6 : 透明色は全パレットに存在, 7とはタイルの生成方法が違う
//- 7 : 透明色は全パレットに存在, 6とはタイルの生成方法が違う
//------------------------------------------------------------------------------
static void ws_gpu_changeVideoMode(u8 value)
{
    value = (value>>5) & 7;

    switch(value) {
      case 7:  pWS->videoMode=7; break;
      case 6:  pWS->videoMode=6; break;
      case 4:  pWS->videoMode=4; break;
      default: pWS->videoMode=0; break;
	}
	// cache clearしなかったのでSAGAでタイトルが化けていた
	ws_gpu_clearCache();
}

//------------------------------------------------------------------------------
// ここでパレットを消去するとバグる。
// テラーズ２で文字が表示されなくなる問題は
// core_memset(pWC,0,sizeof(*pWC));
// とやってしまったのが原因
//------------------------------------------------------------------------------
static void ws_gpu_clearCache(void)
{
    pWC->pal_flag = -1;
	core_memset(pWC->wsc_n,0,sizeof(*pWC->wsc_n));
	core_memset(pWC->wsc_h,0,sizeof(*pWC->wsc_h));
	core_memset(pWC->modify,-1,sizeof(pWC->modify));
}

//------------------------------------------------------------------------------
//- Get Palette Pointer (COLOR / MONO)
//------------------------------------------------------------------------------
static PIXEL_FORMAT* ws_gpu_palette(int pal_idx)
{
    // 0 : Palette未更新
    if( (pWC->pal_flag & (1<<pal_idx)) ) {

		int p = 0x20 + pal_idx * 2;

        pWC->pal_flag &= ~(1<<pal_idx);
        
        if(pWS->videoMode==0) {
			pWC->system_c_pal[pal_idx*16 + 0] = pWC->system_m_pal[ (pWS->ioRam[p+0]   )&7 ];
			pWC->system_c_pal[pal_idx*16 + 1] = pWC->system_m_pal[ (pWS->ioRam[p+0]>>4)&7 ];
			pWC->system_c_pal[pal_idx*16 + 2] = pWC->system_m_pal[ (pWS->ioRam[p+1]   )&7 ];
            pWC->system_c_pal[pal_idx*16 + 3] = pWC->system_m_pal[ (pWS->ioRam[p+1]>>4)&7 ];
        }
    }

    return &pWC->system_c_pal[pal_idx*16];
}

//------------------------------------------------------------------------------
//-
// (tInfo&0x1ff, offsetY, tInfo&0x8000, tInfo&0x4000, tInfo&0x2000);
//------------------------------------------------------------------------------
static u8* getTileRow(u32 tInfo,u32 line)
{
#define PHN(h,n) pH[(h)] = pN[(n)]
    u32 i,tL,tLP;
	u32 tileIndex = tInfo & 0x01ff;
	u32 vFlip     = tInfo & 0x8000;
	u32 hFlip     = tInfo & 0x4000;
	u32 bank      =(tInfo & 0x2000)?1:0;

    if(pWS->videoMode>=4 && bank){
        tileIndex+=512;
    }
    
    if(pWC->modify[tileIndex]) {
        u8* pN = &pWC->wsc_n[tileIndex<<6];
        u8* pH = &pWC->wsc_h[tileIndex<<6];

        pWC->modify[tileIndex]=0;

		switch( pWS->videoMode ) {
		case 7: { // 1pixel = 4bit
            u32 *tIRP = (u32*)&pWS->iRam[0x4000+(tileIndex<<5)];
			for(i=0;i<8;i++) {
                tL=*tIRP++;
                PHN(7,0) = (tL>> 4)&0x0f;
                PHN(6,1) = (tL    )&0x0f;
                PHN(5,2) = (tL>>12)&0x0f;
                PHN(4,3) = (tL>> 8)&0x0f;
                PHN(3,4) = (tL>>20)&0x0f;
                PHN(2,5) = (tL>>16)&0x0f;
                PHN(1,6) = (tL>>28)     ;
                PHN(0,7) = (tL>>24)&0x0f;
                pN+=8;
                pH+=8;
				pWC->trans[tileIndex][i] = tL;
            }
		} break;

		case 6: { // 1pixel = 4bit
            u32 *tIRP = (u32*)&pWS->iRam[0x4000+(tileIndex<<5)];
			for(i=0;i<8;i++) {
                tL=*tIRP++;
                tLP = (tL>>7)&0x01010101; PHN(7,0) = tLP|(tLP>>7)|(tLP>>14)|(tLP>>21);
                tLP = (tL>>6)&0x01010101; PHN(6,1) = tLP|(tLP>>7)|(tLP>>14)|(tLP>>21);
                tLP = (tL>>5)&0x01010101; PHN(5,2) = tLP|(tLP>>7)|(tLP>>14)|(tLP>>21);
                tLP = (tL>>4)&0x01010101; PHN(4,3) = tLP|(tLP>>7)|(tLP>>14)|(tLP>>21);
                tLP = (tL>>3)&0x01010101; PHN(3,4) = tLP|(tLP>>7)|(tLP>>14)|(tLP>>21);
                tLP = (tL>>2)&0x01010101; PHN(2,5) = tLP|(tLP>>7)|(tLP>>14)|(tLP>>21);
                tLP = (tL>>1)&0x01010101; PHN(1,6) = tLP|(tLP>>7)|(tLP>>14)|(tLP>>21);
                tLP = (tL>>0)&0x01010101; PHN(0,7) = tLP|(tLP>>7)|(tLP>>14)|(tLP>>21);
                pN+=8;
                pH+=8;
				pWC->trans[tileIndex][i] = tL;
            }				
		} break;

		default: {  // 1pixel = 2bit
			u16 *tIRP = (u16*)&pWS->iRam[0x2000+(tileIndex<<4)];

			for(i=0;i<8;i++) {
				tL=*tIRP++;
                tLP = (tL>>7)&0x0101; PHN(7,0) = tLP|(tLP>>7);
	            tLP = (tL>>6)&0x0101; PHN(6,1) = tLP|(tLP>>7);
		        tLP = (tL>>5)&0x0101; PHN(5,2) = tLP|(tLP>>7);
			    tLP = (tL>>4)&0x0101; PHN(4,3) = tLP|(tLP>>7);
				tLP = (tL>>3)&0x0101; PHN(3,4) = tLP|(tLP>>7);
				tLP = (tL>>2)&0x0101; PHN(2,5) = tLP|(tLP>>7);
				tLP = (tL>>1)&0x0101; PHN(1,6) = tLP|(tLP>>7);
				tLP = (tL>>0)&0x0101; PHN(0,7) = tLP|(tLP>>7);
				pN+=8;
				pH+=8;
				pWC->trans[tileIndex][i] = tL;
			}
		}break;
		}
    }

    if(vFlip) line=7-line;

	if(pWC->trans[tileIndex][line]==0) {
		return(NULL);
	}

    if (hFlip) return (&pWC->wsc_h[(tileIndex<<6)+(line<<3)]);
    else       return (&pWC->wsc_n[(tileIndex<<6)+(line<<3)]);

    return(NULL);
#undef PHN
}

//------------------------------------------------------------------------------
//-
//------------------------------------------------------------------------------
BYTE cpu_readport(BYTE port)
{
    switch(port) {
	  case 0x02: 
		  return pWC->scanline_num[pWS->line];

      case 0x92: /* Noise Counter Shift Register */
	  case 0x93: /* チャネル４が有効なら適当に変化させる */
		  if(pWS->ioRam[0x90]&0x08) { // near random value
			  pWS->ioRam[port] += ((port&1)*2) + 1;
		  }
		  break;


//-----------------------------------------------------------------------------
/*    case 0x00: case 0x01:            case 0x03: case 0x04: case 0x05: case 0x06: case 0x07:
      case 0x08: case 0x09: case 0x0A: case 0x0B: case 0x0C: case 0x0D: case 0x0E: case 0x0F:
      case 0x10: case 0x11: case 0x12: case 0x13: case 0x14: case 0x15: case 0x16: case 0x17:
      case 0x18: case 0x19: case 0x1A: case 0x1B: case 0x1C: case 0x1D: case 0x1E: case 0x1F:
      case 0x20: case 0x21: case 0x22: case 0x23: case 0x24: case 0x25: case 0x26: case 0x27:
      case 0x28: case 0x29: case 0x2A: case 0x2B: case 0x2C: case 0x2D: case 0x2E: case 0x2F:
      case 0x30: case 0x31: case 0x32: case 0x33: case 0x34: case 0x35: case 0x36: case 0x37:
      case 0x38: case 0x39: case 0x3A: case 0x3B: case 0x3C: case 0x3D: case 0x3E: case 0x3F:
      case 0x40: case 0x41: case 0x42: case 0x43: case 0x44: case 0x45: case 0x46: case 0x47:
      case 0x48: case 0x49: case 0x4A: case 0x4B: case 0x4C: case 0x4D: case 0x4E: case 0x4F:
      case 0x50: case 0x51: case 0x52: case 0x53: case 0x54: case 0x55: case 0x56: case 0x57:
      case 0x58: case 0x59: case 0x5A: case 0x5B: case 0x5C: case 0x5D: case 0x5E: case 0x5F:
      case 0x60: case 0x61: case 0x62: case 0x63: case 0x64: case 0x65: case 0x66: case 0x67:
      case 0x68: case 0x69: case 0x6A: case 0x6B: case 0x6C: case 0x6D: case 0x6E: case 0x6F:
      case 0x70: case 0x71: case 0x72: case 0x73: case 0x74: case 0x75: case 0x76: case 0x77:
      case 0x78: case 0x79: case 0x7A: case 0x7B: case 0x7C: case 0x7D: case 0x7E: case 0x7F:
      case 0x80: case 0x81: case 0x82: case 0x83: case 0x84: case 0x85: case 0x86: case 0x87:
      case 0x88: case 0x89: case 0x8A: case 0x8B: case 0x8C: case 0x8D: case 0x8E: case 0x8F:
      case 0x90: case 0x91: case 0x94: case 0x95: case 0x96: case 0x97:
      case 0x98: case 0x99: case 0x9A: case 0x9B: case 0x9C: case 0x9D: case 0x9E: case 0x9F:       
      case 0xA1: case 0xA2: case 0xA3: case 0xA4: case 0xA5: case 0xA6: case 0xA7: case 0xA8: case 0xA9: break;
      case 0xAB: case 0xAC: case 0xAD: case 0xAE: case 0xAF: case 0xB0: case 0xB1: case 0xB2: break;
      case 0xB4: case 0xB5: case 0xB6: case 0xB7: case 0xB8: case 0xB9: break;
      case 0xBC: case 0xBD: case 0xBE: case 0xBF:// break;
      case 0xC0: case 0xC1: case 0xC2: case 0xC3: case 0xC6: case 0xC7: case 0xC9: case 0xCC: case 0xCD: case 0xCE: case 0xCF:
      case 0xD0: case 0xD1: case 0xD2: case 0xD3: case 0xD4: case 0xD5: case 0xD6: case 0xD7:
      case 0xD8: case 0xD9: case 0xDA: case 0xDB: case 0xDC: case 0xDD: case 0xDE: case 0xDF:
      case 0xE0: case 0xE1: case 0xE2: case 0xE3: case 0xE4: case 0xE5: case 0xE6: case 0xE7:
      case 0xE8: case 0xE9: case 0xEA: case 0xEB: case 0xEC: case 0xED: case 0xEE: case 0xEF:
        break;
*/ //-----------------------------------------------------------------------------
      case 0xA0: return 0x87;
      case 0xAA: return 0xff;
      case 0xB3: 
        if (pWS->ioRam[0xb3]<0x80) return 0;
        if (pWS->ioRam[0xb3]<0xc0) return 0x84;
        return 0xc4;
        
      case 0xBA:{/* eeprom even byte read */
          int w1=((((u16)pWS->ioRam[0xbd])<<8)|((u16)pWS->ioRam[0xbc]));
          w1=(w1<<1)&0x3ff;
          return internalEeprom[w1];
      }
        
      case 0xBB:{/* eeprom odd byte read */
          int w1=((((u16)pWS->ioRam[0xbd])<<8)|((u16)pWS->ioRam[0xbc]));
          w1=((w1<<1)+1)&0x3ff;
          return internalEeprom[w1];
      }

      case 0xC4:
        if(eEepromAddressMask) {
            int w1=(((((WORD)pWS->ioRam[0xc7])<<8)|((WORD)pWS->ioRam[0xc6]))<<1)&(eEepromAddressMask);
            return pWS->romE2P[w1];
        }
#ifdef WIN32
//        *(int*)0xc4 = -1;
#endif
        return 0xff;
        
      case 0xC5:
        if(eEepromAddressMask) {
            int w1=(((((WORD)pWS->ioRam[0xc7])<<8)|((WORD)pWS->ioRam[0xc6]))<<1)&(eEepromAddressMask);
            return pWS->romE2P[w1+1];
        }
#ifdef WIN32
//        *(int*)0xc4 = -1;
#endif
        return 0xff;

      case 0xC8:
        if(eEepromAddressMask) {
            // ack eeprom write
            if(pWS->ioRam[0xc8]&0x20)
              return pWS->ioRam[0xc8]|2;
            
            // ack eeprom read
            if(pWS->ioRam[0xc8]&0x10)
              return pWS->ioRam[0xc8]|1;

            // else ack both
            return pWS->ioRam[0xc8]|3;
        }
#ifdef WIN32
        *(int*)0xc8 = -1;
#endif

      case 0xCA:
		return pWS->ioRam[0xca] | 0x80;
		  break;
        
      case 0xCB:  // RTC data register
		if( pWS->ioRam[0xca]==0x15 ) {
			switch(rtcDataRegisterReadCount) {
			case 0: rtcDataRegisterReadCount++; return 0; /* BCD : year + 2000 */
			case 1: rtcDataRegisterReadCount++; return 0; /* BCD : month       */
			case 2: rtcDataRegisterReadCount++; return 0; /* BCD : day         */
			case 3: rtcDataRegisterReadCount++; return 0; /* BCD : day of week */
			case 4: rtcDataRegisterReadCount++; return 0; /* BCD : hour        */
			case 5: rtcDataRegisterReadCount++; return 0; /* BCD : min         */
			case 6: rtcDataRegisterReadCount=0; return 0; /* BCD : sec         */
			}
		} else {
            return (pWS->ioRam[0xcb]|0x80);
		}
        break;
        
      case 0xF0: case 0xF1: case 0xF2: case 0xF3: case 0xF4: case 0xF5: case 0xF6: case 0xF7:
      case 0xF8: case 0xF9: case 0xFA: case 0xFB: case 0xFC: case 0xFD: case 0xFE: case 0xFF:
        return 0xd1; // F0-FFを作業用として使用
		  break;
    }

    return pWS->ioRam[port];
}

//------------------------------------------------------------------------------
// 
//------------------------------------------------------------------------------
void cpu_writeport(DWORD port,BYTE value)
{
    int w1,ix;

    // PreSet Working
    switch(port){
	  case 0x02: return; /* READ ONLY */
      case 0x52: // Audio DMA
      case 0x90:
      case 0x94: // Main Volume Control
        swan_sound(0,((int)pWS->line*3722)/1000);
        swan_sound(1,((int)pWS->line*3722)/1000);
        swan_sound(2,((int)pWS->line*3722)/1000);
        swan_sound(3,((int)pWS->line*3722)/1000);
        break;

      case 0x80: case 0x81: case 0x88:
        swan_sound(0,((int)pWS->line*3722)/1000);
        break;

      case 0x82: case 0x83:  case 0x89:
        swan_sound(1,((int)pWS->line*3722)/1000);
        break;

      case 0x8c: /* Audio 3 Sweep value */
      case 0x8d: /* Audio 3 Sweep step  */
        pWS->sweep_upd = 1;
      case 0x84:
      case 0x85:
      case 0x8a:
        swan_sound(2,((int)pWS->line*3722)/1000);
        break;

      case 0x86: case 0x87: case 0x8b:
      case 0x8e: /* Audio 4 Noise Control */
        swan_sound(3,((int)pWS->line*3722)/1000);
        break;

      case 0x4e: /* DMA address <read only> */
      case 0x4f: /* DMA address <read only> */
        break;

      case 0xb6:
        if(value) {
            pWS->ioRam[0xb6] &= ~value;
        }
        return;
    }


    pWS->ioRam[port]=value;

	//-----------------------------------------------------------------------
	// Post Working 
	//-----------------------------------------------------------------------
    switch (port) {
/*
      case 0x00: // Display Control
      case 0x01: // Background Color
      case 0x02: // Current Line
      case 0x03: // Line Compare (for Interrupt)
      case 0x04: //
      case 0x05:
      case 0x06:
#ifdef WIN32
        port = port;
#endif
        break;
*/
      case 0x07:
        g_bgbgBase = ((u32)(pWS->ioRam[7] & 0x0f)) << 11;
        g_bgfgBase = ((u32)(pWS->ioRam[7] & 0xf0)) <<  7;
        break;

      case 0x08: case 0x09: case 0x0A: case 0x0B: case 0x0C: case 0x0D: case 0x0E: case 0x0F:
      case 0x10: case 0x11: case 0x12: case 0x13:
		  break;

      case 0xa4: case 0xa5: // HBLANK
      case 0xa6: case 0xa7: // VBLANK
        pWS->ioRam[port|0xf0]=value; // backup to unused area
        break;
        
      case 0xa8: case 0xa9:
        break;

      case 0x1c: case 0x1d: case 0x1e: case 0x1f:
        pWC->system_m_pal[(port-0x1c)*2+0] = pWC->ws_shades[15 & value];
        pWC->system_m_pal[(port-0x1c)*2+1] = pWC->ws_shades[(value>>4)];
        pWC->pal_flag = -1;
        break;
        
      case 0x20:  case 0x21:  case 0x22:  case 0x23: case 0x24:  case 0x25:  case 0x26:  case 0x27:
      case 0x28:  case 0x29:  case 0x2a:  case 0x2b: case 0x2c:  case 0x2d:  case 0x2e:  case 0x2f:
      case 0x30:  case 0x31:  case 0x32:  case 0x33: case 0x34:  case 0x35:  case 0x36:  case 0x37:
      case 0x38:  case 0x39:  case 0x3a:  case 0x3b: case 0x3c:  case 0x3d:  case 0x3e:  case 0x3f:
        pWC->pal_flag |= (1<<((port-0x20)/2));
        break;

      case 0x42: case 0x43:
        pWS->ioRam[port] &= 0x0f;
        break;

      case 0x48:	// DMA
        // bit 7 set to start dma transfer
        if(value&0x80) {
            int dma_start = (((DWORD)pWS->ioRam[0x41])<<8)|(((DWORD)pWS->ioRam[0x40]))|(((DWORD)pWS->ioRam[0x42])<<16);
            int dma_end   = (((DWORD)pWS->ioRam[0x45])<<8)|(((DWORD)pWS->ioRam[0x44]))|(((DWORD)pWS->ioRam[0x43])<<16);
            int dma_size  = (((DWORD)pWS->ioRam[0x47])<<8)|(((DWORD)pWS->ioRam[0x46]));

            dma_end &= 0x000fffff; /* naruto bugfix */

            for(ix=0;ix<dma_size;ix++) {
                cpu_writemem20(dma_end,cpu_readmem20(dma_start));
				dma_end++;
				dma_start++;
            }

            pWS->ioRam[0x47]=0;
            pWS->ioRam[0x46]=0;
            pWS->ioRam[0x41]=(BYTE)(dma_start>>8);
            pWS->ioRam[0x40]=(BYTE)(dma_start&0xff);
            pWS->ioRam[0x45]=(BYTE)(dma_end>>8);
            pWS->ioRam[0x44]=(BYTE)(dma_end&0xff);
            pWS->ioRam[0x48]=0;
        }
        break;
/*
      case 0x4a: // sound DMA source address
      case 0x4b:
      case 0x4c: // DMA source memory segment bank
      case 0x4d:
      case 0x4e: // DMA Transfer size (in bytes) 
      case 0x4f: // ^^^                          
      case 0x50: 
      case 0x51:
#ifdef WIN32
		  if(value) *((int*)port) = value;
#endif
*/
      case 0x52: /* bit  7 = 1  -> DMA start */
		ws_voice_dma();
        break;

      case 0x60:
        ws_gpu_changeVideoMode(value);
        return;
/*
      case 0x80: // Audio 1 Freq : low  
      case 0x81: // ^^^          : high 
      case 0x82: // Audio 2 freq : low  
      case 0x83: // ^^^          : high 
      case 0x84: // Audio 3 Freq : low  
      case 0x85: // ^^^          : high 
      case 0x86: // Audio 4 freq : low  
      case 0x87: // ^^^          : high 
#ifdef WIN32
		  value = value;
#endif
      case 0x88: // Audio 1 volume      
      case 0x89: // Audio 2 volume      
      case 0x8a: // Audio 3 volume      
      case 0x8b: // Audio 4 volume      
      case 0x8c: // Audio 3 Sweep Value 
      case 0x8d: // Audio 3 Sweep Step  
#ifdef WIN32
		  value = value;
#endif
        break;
*/
      case 0x8e: /* Audio 4 Noise Control*/
        // Counter Enable
		pWS->noise_c=(value&7)*512;
		
		// FF1 Noise fix
		if(value&0x10){
            pWS->noise_k = 0;
            pWS->noise_r = 0;
            pWS->noise_v = 0x51f631e4;
        }
        break;

      case 0x91: /* Audio Output    */
        pWS->ioRam[0x91]|= 0x80; // stereo status
        break;

#if 0
      case 0x8f: // Sample Location 
        break;
      case 0x90: /* Audio Control   */
        break;

      case 0x92: /* Noise Counter Shift Register : low?  */
      case 0x93: /* ^^^                          : high? */
//			value = value;
		  break;

      case 0x94: /* Volume 4bit */
        //        ws_audio_port_write(port,value);
        break;
#endif

      case 0xb5: // Controls
        switch(value&0xf0) {
          case 0x10: value=0x10 | (0x0f&(ws_key>>8)); break; // read vertical
          case 0x20: value=0x20 | (0x0f&(ws_key>>4)); break; // read horizontal
          case 0x40: value=0x40 | (0x0f&(ws_key)   ); break; // read buttons
          default:   value&=0xf0;                     break;
        }
        pWS->ioRam[0xb5] = value;

        break;

      case 0xba:
        w1=(((((WORD)pWS->ioRam[0xbd])<<8)|((WORD)pWS->ioRam[0xbc])));
        w1=(w1<<1)&0x3ff;
        internalEeprom[w1]=value;
        return;

      case 0xbb:
        w1=(((((WORD)pWS->ioRam[0xbd])<<8)|((WORD)pWS->ioRam[0xbc])));
        w1=((w1<<1)+1)&0x3ff;
        internalEeprom[w1]=value;
        return;

      case 0xbe: // EEPROM
        if(value & 0x20) { value |= 0x02; }
        else if(value & 0x10) { value |= 0x01; }
        else { value|=0x03; }
        pWS->ioRam[0xbe] = value;
        break;

      case 0xc0: {
          int romBank,bank;

          for(bank=4;bank<16;bank++) {
              romBank=(256-(((value&0xf)<<4)|(bank&0xf)));
              pWsRomMap[bank] = &ws_RomAddr[ws_RomSize-(romBank<<16)];
          }

          // Read Portで必ず同じ処理をするのでWrite Port時にやってしまう
          pWS->ioRam[0xc0]=(pWS->ioRam[0xc0]&0x0f)|0x20;
      } break;

      case 0xc1: // SRAM Bank Change
		  switch(sramAddressMask) {
		    case 0x1ffff: pWsRomMap[0x01] = &pWS->romRam[(value&1)<<16]; break; 
		    case 0x3ffff: pWsRomMap[0x01] = &pWS->romRam[(value&3)<<16]; break; 
		    default:      pWsRomMap[0x01] = &pWS->romRam[0];             break;
		  }
		  break;
        
      case 0xc2:
        pWsRomMap[0x02] = &ws_RomAddr[ ((value&((ws_RomSize>>16)-1))<<16) ];
        break;

      case 0xc3:
        pWsRomMap[0x03] = &ws_RomAddr[ ((value&((ws_RomSize>>16)-1))<<16) ];
        break;

      case 0xc4:
        w1=(((((WORD)pWS->ioRam[0xc7])<<8)|((WORD)pWS->ioRam[0xc6]))<<1)&eEepromAddressMask;
        pWS->romE2P[w1]=value;
        return;

      case 0xc5:
        w1=(((((WORD)pWS->ioRam[0xc7])<<8)|((WORD)pWS->ioRam[0xc6]))<<1)&eEepromAddressMask;
        pWS->romE2P[w1+1]=value;
        return;

      case 0xca:
        if(value==0x15) {
            rtcDataRegisterReadCount=0;
        }
        break;

      default:
        break;
    }
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
void cpu_writeport2(DWORD port,WORD value)
{
	cpu_writeport(port  ,value);
	cpu_writeport(port+1,value>>8);
}



//------------------------------------------------------------------------------
// 0x4000-0x8000 : TILE BANK 0
// 0x8000-0xC000 : TILE BANK 1
//------------------------------------------------------------------------------
void cpu_writemem20(DWORD addr,BYTE value)
{
	u16 offset;
	u32 bank = addr>>16;

    /* 0 - RAM - 16 KB (WS) / 64 KB (WSC) internal RAM */
	if(bank==0) {
	    offset = addr;
        if(pWS->iRam[offset]!=value) {

			if(offset<0x02000); else 
			if(offset<0x04000) { 
				pWC->modify[((offset>>4)&0x1ff)]=1; 
			} else
			if(offset<0x0C000) { pWC->modify[(offset-0x4000)>>5] =1; } else
//			if(offset<0x08000) { pWC->modify[((offset>>5)&0x1ff)+0x000]=1; } else
//			if(offset<0x0C000) { pWC->modify[((offset>>5)&0x1ff)+0x200]=1; } else
            if(offset<0x0FE00); else {
				u16 c,adr,pal;
				pWS->iRam[offset] = value;
				adr = offset & 0x0fffe;
				pal = (adr - 0xfe00)/2;
				c = ((u16)(pWS->iRam[adr+1]<<8) | pWS->iRam[adr]);
				pWC->system_c_pal[pal] = HAL_fb2_Color((c>>8)&0xf,(c>>4)&0xf,(c&0xf),RGB444);
			}
            pWS->iRam[offset] = value;
        }
    } else if(bank==1){
	    offset = addr;
        *(pWsRomMap[0x01]+offset) = value;
    }
    
	// other banks are read-only
}

//------------------------------------------------------------------------------
// Update WaveTable 
//------------------------------------------------------------------------------
static void wave_update(int ch)
{
    int i;
    byte * wTbl = &pWS->iRam[ (((int)pWS->ioRam[0x8f])<<6) ]; // wave table
    
    for(i=0;i<WAVE_SAMPLES/2;i++) {
        waveTable[ch][i*2+0] = (((int)( wTbl[i+16*ch]&15 ))-8); // 0 to 15
        waveTable[ch][i*2+1] = (((int)((wTbl[i+16*ch]>>4)))-8); // 0 to 15
    }
}


//------------------------------------------------------------------------------
// PSG and SWEEP mixer
//------------------------------------------------------------------------------
static void mixer_psg_sweep(int ch,int do_cursor,int freq)
{
    if(freq>0) {
        int volR = VOLUME_R(ch);
        int volL = VOLUME_L(ch);
        DWORD na = pWS->snd_n[ch];// & 31;
        DWORD ka = pWS->snd_k[ch];// & 31;
        DWORD NTp,t,i;
        int wavena;
        
        // エラーチェック
        wave_update(ch);
        
        NTp = CPU_CLOCK / freq;
        
        for(i=halSnd.ch[ch];i<do_cursor;i++) {
            wavena = waveTable[ch][na];
            halSnd.R32[i] += (wavena * volR)*PSG_VOLUME;
            halSnd.L32[i] += (wavena * volL)*PSG_VOLUME;
            ka += NTp;

            // tが2以上になる場合があるので単純に書き換えてはいけない
            t = ka / (441000);
            na = (na+t)%(WAVE_SAMPLES);
            ka -= 441000*t;
        }
        
        pWS->snd_n[ch]=na;
        pWS->snd_k[ch]=ka;
    }
}

//------------------------------------------------------------------------------
// PSG Mixer (CH1,2,3,4)
//------------------------------------------------------------------------------
static void mixer_psg(int ch,int do_cursor)
{
    mixer_psg_sweep(ch,do_cursor,2048-GET_PITCH(ch));
}

//------------------------------------------------------------------------------
// Voice Mixer (CH2 ONLY)
//------------------------------------------------------------------------------
static void mixer_voice(int ch,int do_cursor)
{
    int voice = pWS->ioRam[0x89]; // 0to255
    int i;

#if 1
    voice = (voice-127) * VOICE_VOLUME;
#else
	if( voice < 64 ) voice = 64; else
	if( voice >191 ) voice = 192;
	voice = (voice - 128) * 255;
#endif

    for(i=halSnd.ch[ch];i<do_cursor;i++) {
        halSnd.R32[i] += voice;
        halSnd.L32[i] += voice;
    }
}


//------------------------------------------------------------------------------
// Sweep Mixer (CH3 ONLY)
//------------------------------------------------------------------------------
// - channel 3 - sweep - two parameters:
// - step = 2.667 x (N + 1) ms , where N = 5 bit value (0 to 31)
// - value - signed byte (-128 to 127)
//
// 1000(ms) / 75     = 13.333 (ms)
// 1000(ms) / 75 * 2 = 26.67  (ms)
//
// (N+1)=10 (2.667*10)=26.67 : 150フレームで１回減算
// (N+1)=20 (2.667*20)=53.34 : 300フレームで１回減算
// ってーことは
// (N+1)= 1 (2.667* 1)= 2.67 :  15フレームで１回減算
// (N+1)=32 (2.667*32)=85.34 : 480フレームで１回減算
// つまり
// SWEEPは設定値に15を乗算したタイミングでスウィープ処理
//------------------------------------------------------------------------------
static void mixer_sweep(int ch,int do_cursor)
{
    if(pWS->sweep_upd) {
        pWS->sweep_pitch = GET_PITCH(ch);
        pWS->sweep_value = (char)pWS->ioRam[0x8c];
        pWS->sweep_step  = pWS->ioRam[0x8d]&31;
        pWS->sweep_upd   = 0;
        pWS->sweep_count = (pWS->sweep_step+1) * 15;
    } else {
        if(do_cursor==SOUND_SamplesPerFrame) {
            if(--pWS->sweep_count==0) {
                pWS->sweep_count = (pWS->sweep_step+1) * 15;
                pWS->sweep_pitch += pWS->sweep_value;

                if(pWS->sweep_pitch<0){
                    pWS->sweep_pitch=0;
                } else {
                    if(pWS->sweep_pitch>2047) {
                        pWS->sweep_pitch=2047;
                    }
                }
				pWS->ioRam[0x90] &= ~0x40;
            }
        }
    }
    
    mixer_psg_sweep(ch,do_cursor,2048 - pWS->sweep_pitch);
}


//------------------------------------------------------------------------------
//  NOISE Mixer (CH4 ONLY)
//------------------------------------------------------------------------------
static void mixer_noise(int ch,int do_cursor)
{
    int i;
    int volR = VOLUME_R(ch);
    int volL = VOLUME_L(ch);
    
    for(i=halSnd.ch[ch];i<do_cursor;i++) {
        pWS->noise_k += 3000 + pWS->noise_c;
        
        if(pWS->noise_k>=44100) {
            if(pWS->noise_v & 0x80000) {
                pWS->noise_v = ((pWS->noise_v ^ 0x04)<<1)+1;
                pWS->noise_r = 1;
            } else {
                pWS->noise_v <<= 1;
                pWS->noise_r = 0;
            }
            pWS->noise_k -= 44100;
        }
        
        halSnd.L32[i] += ((pWS->noise_r?(NOISE_VOLUME):(-NOISE_VOLUME))*volL);
        halSnd.R32[i] += ((pWS->noise_r?(NOISE_VOLUME):(-NOISE_VOLUME))*volR);
    }
}


//------------------------------------------------------------------------------
// 
//------------------------------------------------------------------------------
static void swan_sound(int ch,int do_cursor)
{
    if( HAL_Sound() ) {
        void (*pMixerFunc)(int ch,int do_cursor);
        
		pMixerFunc = mixer_psg;
        switch(ch) {
        case 0: // CH1 : Audio
          if(!(pWS->ioRam[0x90]&0x01))    pMixerFunc = 0; // silent
          break;
        case 1: // CH2 : Audio + Voice
          if(!(pWS->ioRam[0x90]&0x02))    pMixerFunc = 0; else
          if((pWS->ioRam[0x90]&0x20))     pMixerFunc = mixer_voice; // silent
          break;
        case 2: // CH3 : Audio + Sweep
          if(!(pWS->ioRam[0x90]&0x04))    pMixerFunc = 0; else
          if( (pWS->ioRam[0x90]&0x40))    pMixerFunc = mixer_sweep; 
          break;
        case 3: // CH4 : Audio + Noise
          if(!(pWS->ioRam[0x90]&0x08))    pMixerFunc = 0; else // silent
          if( (pWS->ioRam[0x90]&0x80))    pMixerFunc = mixer_noise;
          break;
        default:
          pMixerFunc = 0; // silent
        }

        if(pMixerFunc) {
            (pMixerFunc)(ch,do_cursor);
        }
        halSnd.ch[ch]=do_cursor;
    }
}


//------------------------------------------------------------------------------
// Audio DMA work
//------------------------------------------------------------------------------
static void ws_voice_dma(void)
{
	if(pWS->ioRam[0x52]&0x80) {
		int adr = (int)((int)pWS->ioRam[0x4c]<<16) | ((int)pWS->ioRam[0x4b]<<8) | pWS->ioRam[0x4a];
		int len = (int)((int)pWS->ioRam[0x4f]<<8 ) | ((int)pWS->ioRam[0x4e]);

        pWS->ioRam[0x90] |= 0x22;
        
		cpu_writeport(0x89,cpu_readmem20(adr));

		adr++;
		len--;

        /* Sound DMAでbankを超える事があれば中止 */
        if( (adr>>16) != pWS->ioRam[0x4c] ) {
#ifdef WIN32
            *(int*)0 = 0xDADADADA;
#endif
            adr--;
            len=0;
        }

#if 1 /* oswan実装 */
        if(len<32) {
            len=0;
        }
#endif
        
//      pWS->ioRam[0x4C]=(u8)((adr>>16)&0xFF); /* Bank越えは禁止 */
		pWS->ioRam[0x4B]=(u8)((adr>>8)&0xFF);
		pWS->ioRam[0x4A]=(u8)(adr&0xFF);
		pWS->ioRam[0x4F]=(u8)((len>>8)&0xFF);
		pWS->ioRam[0x4E]=(u8)(len&0xFF);

		if(len==0) {
			pWS->ioRam[0x52]&=~0x80;
#if 0 // Terrors2で音がでない問題の修正
            pWS->ioRam[0x90]&=~0x22;
#endif
		}
	}
}

//------------------------------------------------------------------------------
// 
//------------------------------------------------------------------------------
static int ws_save(int fd)
{
    int size,len=0;
    void* pCpu = nec_getRegPtr(&len);

    core_memcpy(pWS->tag_iram, "*RAM",4);
    core_memcpy(pWS->tag_ioram,"*I/O",4);
    core_memcpy(pWS->tag_spr,  "*SPR",4);
    core_memcpy(pWS->tag_snd,  "*SND",4);
    core_memcpy(pWS->tag_eE2P, "@E2P",4);
    core_memcpy(pWS->tag_eRAM, "@RAM",4);

    size = sizeof(*pWS) 
		 - sizeof(pWS->romE2P) - sizeof(pWS->tag_eE2P) 
         - sizeof(pWS->romRam) - sizeof(pWS->tag_eRAM);

    HAL_sts_write(fd,pCpu,len);
    HAL_sts_write(fd,pWS,size);
    
    if(sramAddressMask) {
        HAL_sts_write(fd,pWS->tag_eRAM,sramAddressMask+1 + sizeof(pWS->tag_eRAM) );
    }
    else if(eEepromAddressMask) {
        HAL_sts_write(fd,pWS->tag_eE2P,eEepromAddressMask+1 + sizeof(pWS->tag_eE2P));
    }
    
    return 1;
}

//------------------------------------------------------------------------------
// 
//------------------------------------------------------------------------------
static int ws_load(int fd)
{
    int i,size,len=0;
    void *pCpu = nec_getRegPtr(&len);

    size = sizeof(*pWS)
         - sizeof(pWS->romE2P) - sizeof(pWS->tag_eE2P) 
         - sizeof(pWS->romRam) - sizeof(pWS->tag_eRAM);

    HAL_sts_read(fd,pCpu,len);
    HAL_sts_read(fd,pWS,size);

    if(sramAddressMask)    {
        HAL_sts_read(fd,pWS->tag_eRAM,sramAddressMask+1);
    }
    else if(eEepromAddressMask) {
        HAL_sts_read(fd,pWS->tag_eE2P,eEepromAddressMask+1);
    }
    
    // Tile Cache Clear
    ws_gpu_clearCache();
    core_memset(waveTable,0,sizeof(waveTable));
    
    // ROM Map Update
    cpu_writeport(0xc0,pWS->ioRam[0xc0]);
    cpu_writeport(0xc2,pWS->ioRam[0xc2]);
    cpu_writeport(0xc3,pWS->ioRam[0xc3]);

    // Update Color Palette
    if( pWS->videoMode>=4 ) {
        byte v;

        for(i=0xfe00;i<0x10000;i++) {
            v = cpu_readmem20(i);
            cpu_writemem20(i,0);
            cpu_writemem20(i,v);
        }
    } else {
        for(i=0x1c;i<=0x1f;i++) {
            cpu_writeport(i,pWS->ioRam[i]);
        }
    }

    return 1;
}

//------------------------------------------------------------------------------
//- Background fill
//------------------------------------------------------------------------------
static void DrawScreen_FillBg(int beg,int end,PIXEL_FORMAT bgc)
{
	int i,line;
	PIXEL_FORMAT* pFb;

	for(line=beg;line<end;line++){
		pFb = FBPTR(line);

		for(i=0;i<224;i++) {
			*pFb++=bgc;
		}
	}
}

//------------------------------------------------------------------------------
//- タイル表示(バックグラウンド属性)
//- 一番最初に描画するので透明BGはBG色で塗り潰す必要あり。
//- ベタ塗りしてからBG描画するより透明色=BG色と書き換えてBG描画を行う。
//------------------------------------------------------------------------------
static void DrawScreen_BackBg(int beg,int end,PIXEL_FORMAT bgc)
{
	PIXEL_FORMAT* wsPal,pa0;
	PIXEL_FORMAT *pFb;
	int line,px,scrX,curTile;
	u16 mInfo,tInfo, *pTileRam;
	u8* wsTR;
	int scrY;

	mInfo = (pWS->videoMode>4)?1:0;
	scrX = pWS->ioRam[0x10];

	scrY =(pWS->ioRam[0x11]+beg)&0xff;

	for(line=beg;line<end;line++){
		curTile = (scrX>>3);
		pTileRam = (u16*)(pWS->iRam+g_bgbgBase+((scrY&0xf8)<<3));

		pFb = FBPTR(line)-(scrX&7);

		for(px=-(scrX&7);px<224;px+=8) {
			tInfo= pTileRam[(curTile++)&0x1f];

			wsPal = ws_gpu_palette(PAL_TILE(tInfo));
			pa0 = wsPal[0];

			// 半透明が必要なら透明色をBG色に一時的に変更
			// 半透明が不要でwsTR=0ならwsPal[0]でFILLする
			if(mInfo | (tInfo&0x800)) { wsPal[0]=bgc; }

			if( (wsTR = getTileRow(tInfo,scrY&7)) ) {
				REPx8( *pFb++ = BGBGMASK(wsPal[*wsTR++]) );
			} else {
				REPx8( *pFb++ = BGBGMASK(wsPal[0]) );
			}

			// 透明色を復元する
			wsPal[0] = pa0;
		}

		scrY = (scrY+1)&0xff;
	}
}


//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
static void tile_draw_trans(PIXEL_FORMAT* pFb,u16 tInfo,u16 offsetY)
{
	u8           *wsTR = getTileRow(tInfo,offsetY);
	PIXEL_FORMAT *wsPA = ws_gpu_palette(PAL_TILE(tInfo));
		
	if((pWS->videoMode>4) || (tInfo&0x800)) {
		/* 透明判定 */
		if( wsTR ) {
			REPx8( 
				if(*wsTR) { *pFb = BGFGMASK(wsPA[*wsTR]); };
				pFb++; wsTR++; 
			);
		} else {
			pFb+=8;
			/* BackBGがなければ、ここで塗りつぶしも可能 */
		}
	} else {
		if( wsTR ) {
			REPx8( *pFb++ = BGFGMASK(wsPA[*wsTR++]) );
		} else {
			REPx8( *pFb++ = BGFGMASK(wsPA[0]) );
		}
	}
}

//------------------------------------------------------------------------------
// 
// Foreground TILE draw (with window clip(inside/outside))
// 
// ウィンドウなし : 通常通りの表示
// ウィンドウあり : ３タイプに分ける
//   (1) ウィンドウ外で非表示 
//   (2) ウィンドウ内で全表示
//   (1) ウィンドウ境界で一部表示
//------------------------------------------------------------------------------
static void DrawScreen_BackFg(int beg,int end)
{
	s32 px,line;
	u16* ws_fgScrollRamBase;
	PIXEL_FORMAT *pFb,*wsPA;
	u32 bgWin_y0 = pWS->ioRam[0x09];
	u32 bgWin_y1 = pWS->ioRam[0x0b];
	s32 scrX     = pWS->ioRam[0x12];
	s32 offsetX  = scrX&0x07;
	u16 tInfo;
	u8* wsTR;
	int bgWin_x0=pWS->ioRam[0x08];
	int bgWin_x1=pWS->ioRam[0x0a];
	u8  side,fWin,wMode;
	int scrY,curTile,offsetY;

	fWin = pWS->ioRam[0x00] & BGFG_CLIP_MASK;
	side =(pWS->ioRam[0x00] & BGFG_CLIP_SIDE)?1:0;

	// 
	// Ｘ軸のクリッピングが本当に必要なのかチェック
	//
	if( BGFG_CLIP_ENABLE & fWin ){ 
		if( (bgWin_x0==bgWin_x1) || (bgWin_x0==0 && bgWin_x1>=224) ) {
			bgWin_x0=0;
			bgWin_x1=224;
		}

		// 内側表示
		if( BGFG_CLIP_IN==fWin ) {
			if(beg<bgWin_y0) beg=bgWin_y0;
			if(end>bgWin_y1) end=bgWin_y1;
		}
	}

	for(line=beg;line<end;line++) {

		wMode = fWin;

		//
		// クリッピングモードONの場合に、Y軸方向のクリッピング状況を調べる
		//
		if( BGFG_CLIP_ENABLE & wMode ) { // CLIPPING : ON

			// YCLIPPING CHECK
			if(side==0) { /* SHOW INSIDE */
				if( !BETWEEN(line,bgWin_y0,bgWin_y1) ) {
					// 内側ウィンドウでＹ軸が外側→非表示
					continue;
				} else {
					// 内側ウィンドウでＹ軸が内側→Ｘ軸のウィンドウ処理を通す
				}
			} else {      /* SHOW OUTSIDE */
				if( !BETWEEN(line,bgWin_y0,bgWin_y1) ) {
					// 外側ウィンドウでＹ軸が外側→ウィンドウなし
					wMode = 0; // Ｙ軸がウィンドウ外なのでウィンドウ不要
				} else {
					// 外側ウィンドウでＹ軸が内側→Ｘ軸クリッピング処理を通す
				}
			}
		}

		// Setup Values
		pFb = FBPTR(line)-offsetX;
		scrY    = ((s32)pWS->ioRam[0x13]+line)&0xff;
		curTile = (scrX>>3);
		offsetY = scrY&0x07;

		ws_fgScrollRamBase=(u16*)(pWS->iRam+g_bgfgBase+((scrY&0xfff8)<<3));

		//
		// Disable Window 
		//
		if((wMode&0x20)==0) {
			for(px=-offsetX;px<224;px+=8) {
				tInfo= ws_fgScrollRamBase[curTile&0x1f]; curTile++;
				tile_draw_trans(pFb,tInfo,offsetY);
				pFb+=8;
			}
		}
		else { // foreground layer displayed with Window

			int fx1,fx2;

			// 実際にウィンドウ処理に関係するのは
			// Ｘ軸で最大２タイルに限られるのは明白
			// 
			// 【描画あり】【描画なし】の２パターン
			// 【右クリップ】【左クリップ】【左右クリップ】の３パターン

			for(px=-offsetX;px<224;curTile++) {

				fx1 = side ^ BETWEEN(px  ,bgWin_x0,bgWin_x1);
				fx2 = side ^ BETWEEN(px+7,bgWin_x0,bgWin_x1);

				// 全非表示領域
				if( !fx1 && !fx2 ) {
					pFb+=8;
					px+=8;
					continue;
				}

				tInfo= ws_fgScrollRamBase[curTile&0x1f];
				wsTR = getTileRow(tInfo,offsetY);
				wsPA = ws_gpu_palette(PAL_TILE(tInfo));

				// 全表示領域
				if( fx1 && fx2 ) {
					tile_draw_trans(pFb,tInfo,offsetY);
					pFb+=8;
					px+=8;
				}
				// 一部表示領域
				else {
					if((pWS->videoMode>4) || (tInfo&0x800)) { 
						// 透明判定
						if(wsTR) { // 判定あり
							REPx8(
								if(*wsTR) {
									if( (side ^ BETWEEN(px,bgWin_x0,bgWin_x1)) ) {
										*pFb = BGWNMASK(wsPA[*wsTR]); 
									} else {
										/* BackBGがなければ、ここで塗りつぶしも可能 */
									}
								}
								wsTR++;
								pFb++;
								px++;
							);
						} else {  // 透明確定
							pFb += 8;
							px+=8;
						}
					} else {
						// 塗りつぶし
						if(wsTR) { // 透明判定あり
							REPx8( 
								if( side ^ BETWEEN(px,bgWin_x0,bgWin_x1) ) {
									*pFb = BGWNMASK(wsPA[*wsTR]); 
								}
								wsTR++;
								pFb++;
								px++;
							);
						} else { // パレット０確定
							REPx8(
								if( side ^ BETWEEN(px,bgWin_x0,bgWin_x1) ){
									*pFb = BGWNMASK(wsPA[0]); 
								}
								pFb++;
								px++;
							);
						}
					}
				}
			}
		}
	}
}


//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
static void DrawScreen_Sprite(int beg,int end)
{
	s32 i,j,x,y;
	u8 * wsTR;
	PIXEL_FORMAT* wsPA,*pFb;
	u32* pSpRam,spr,t,p;
	int outside=0;
	int top_pri=0;
	int xbet,ybet,line;

	int spWin_x0 = pWS->ioRam[0x0c];
	int spWin_y0 = pWS->ioRam[0x0d];
	int spWin_x1 = pWS->ioRam[0x0e];
	int spWin_y1 = pWS->ioRam[0x0f];
	byte spWin   = pWS->ioRam[0x00] & 0x08; // Sprite Window Enable?

	if( spWin_x0==spWin_x1) {
		spWin &= ~0x08;
	}

	// ウィンドウなし = (0,0)-(223,143)の内側を描画のウィンドウあり条件
	if(!spWin) {
		spWin_x0 = 0;
		spWin_y0 = 0;
		spWin_x1 = 224;
		spWin_y1 = 144;
	}

	for(line=beg;line<end;line++) {
		top_pri=0;
		pFb = FBPTR(line);

		/* Y座標が範囲内かを示すフラグ */
		ybet = BETWEEN(line,spWin_y0,spWin_y1);

		pSpRam = &pWS->spTable[pWS->spTableCnt-1];

		for(i=0;i<pWS->spTableCnt;i++) {
			spr = *pSpRam--;
			x = (spr>>24) & 0x0ff;
			y = (spr>>16) & 0x0ff;
			t =  spr      & 0x1ff;
			p =((spr&0xe00)>>9) + 8;

			// cygneでやってる調整をしてみる
			if(y>(144-1+7)) {
				y = (signed char)((unsigned char)y);
			}

			if((y+8)<=(s32)line) continue;
			if( y   > (s32)line) continue;

			if(x>=(224-1+7)) {
				x = (signed char)((unsigned char)x);
			}

			if(x<=-8) continue;
			if((y>=144)||(x>=224))  continue;

			top_pri = (spr & 0x2000)<<2;
			outside = (spr&0x1000)&&(spWin);

			if( (wsTR = getTileRow(spr&~0x2000,(line-y)&0x07)) ) {
				wsPA = ws_gpu_palette(p);

				if((pWS->videoMode>4) || (p&0x04)) {
					// (表示条件)
					// if(!outside &&  BETWEEN) true 
					// if(outside  && !BETWEEN) true 
					// という事でXORを使う
					for(j=x;j<(x+8);j++,wsTR++) {
						if( (pFb[j]&DRAW_MASK) && !top_pri ) { continue; }
							
						if(*wsTR) {
							xbet = BETWEEN(j,spWin_x0,spWin_x1);
							if( outside ^ (xbet && ybet) ) {
								pFb[j]= (pFb[j]&DRAW_MASK) | SPMASK(wsPA[*wsTR]);
							}
						}
					}
				}
				else {
					for(j=x;j<(x+8);j++,wsTR++) {
						if( (pFb[j]&DRAW_MASK) && !top_pri ) continue;
						xbet = BETWEEN(j,spWin_x0,spWin_x1);
						if( outside ^ (xbet && ybet) ) {
							pFb[j] = SPMASK(wsPA[*wsTR]);
						}
					}
				}
			}
		}
	}
}

//-----------------------------------------------------------------------------
// 高速化できそうな部分
// (1) BackBG=OFF,BackFG=ONの場合に、BackFillを呼ばずにBackFGで透明色をFillする
// (2) BackFGの塗りつぶしピクセル＞BackBGの塗りつぶしピクセルの場合を最適化する
//
// 
// #define IS_BGBG()     (pWS->ioRam[0x00]&0x01)
// #define IS_BGFG()     (pWS->ioRam[0x00]&0x02)
// #define IS_SPRT()     (pWS->ioRam[0x00]&0x04)
//
//-----------------------------------------------------------------------------
static void drawLine(int beg,int end)
{
	// Scanline Video Processing
	PIXEL_FORMAT bg;
    if(pWS->videoMode) { bg = pWC->system_c_pal[pWS->ioRam[1]];   }
	else               { bg = pWC->system_m_pal[pWS->ioRam[1]&7]; }

	if(IS_BGBG()) DrawScreen_BackBg(beg,end,BGC_MASK(bg)); 
    else          DrawScreen_FillBg(beg,end,BGC_MASK(bg));

	if(IS_BGFG()) DrawScreen_BackFg(beg,end);
	if(IS_SPRT()) DrawScreen_Sprite(beg,end);

#ifdef DEBUG_QUICK
	{
		int i;
		if(beg==0) deb_qcol=0x1f;

		for(i=beg;i<end;i++) {
			fb_format.fb[beg+ 8+fb_format.width*i]=deb_qcol;
		}

		if(!(deb_qcol=(deb_qcol<<5)&0x7fff)) deb_qcol=0x1f;
	}
#endif
}

//------------------------------------------------------------------------------
// Wonderswan Interrupt flag check
// CPUで割込禁止解除を実施した際にも実行 → 実機に近い割込処理を実現
//------------------------------------------------------------------------------
int ws_int_check(void)
{
	int intr = pWS->ioRam[0xb6] & pWS->ioRam[0xb2];
	byte mask=8;

//#if 1
//	pWS->ioRam[0xb6]&=0xf0;
//	intr            &=0xf0;
//#endif

	if(intr&0xf0) {
		if(intr&INT_HBLANK) mask=7; else // HBLANK Timer
		if(intr&INT_VBLANK) mask=6; else // VBLANK begin
		if(intr&INT_VTIMER) mask=5; else // VBLANK end
		if(intr&INT_SCLINE) mask=4; else // SCANLINE
			return 0;

		if(mask<8) {
			if( nec_int((pWS->ioRam[0xb0]+mask)*4) ) {
//				pWS->ioRam[0xb6]&=~(1<<mask);
			}
			return 1;
		}
	}
	return 0;
}

//-----------------------------------------------------------------------------
//
// Interrupt flag update
//
//-----------------------------------------------------------------------------
static void ws_int_work(void)
{
	//-----------------------------------------------------
	// SPRITE TABLE FETCH
	//-----------------------------------------------------
	if( //SPRITE_UPDATE_LINE
		VBLANK_BEGIN_LINE==pWS->line ) {
		u32 base = (((u32)pWS->ioRam[4]&0x3f)<<9) + pWS->ioRam[5]*4; 
		if( (pWS->spTableCnt = pWS->ioRam[6])>0x80) {
			pWS->ioRam[6]=0x80;
		}
		core_memcpyX4(pWS->spTable,(u32*)&pWS->iRam[base],pWS->spTableCnt*4);
	}

	//-----------------------------------------------------
	// HBLANK TIMER
	//-----------------------------------------------------
	if(HBLANK_TIMER()) {
		if(pWS->ioRam[0xa4]||pWS->ioRam[0xa5]) {
			if( pWS->ioRam[0xa4]==0 ) { 
				pWS->ioRam[0xa5]--;    // 0xa5は絶対に0以外
				pWS->ioRam[0xa4]=255;
			} else {
				if((--pWS->ioRam[0xa4])==0) {
					if(pWS->ioRam[0xa5]==0) {
						pWS->ioRam[0xb6]|=(pWS->ioRam[0xb2]&INT_HBLANK);
						if(HBLANK_AUTO()) {
							pWS->ioRam[0xa4] = pWS->ioRam[IO_BACKUP_A4];
							pWS->ioRam[0xa5] = pWS->ioRam[IO_BACKUP_A5];
						}
					}
				}
			}
		} else {
//			pWS->ioRam[0xb6]|=(pWS->ioRam[0xb2]&INT_HBLANK);
		}
	}
	
	//-----------------------------------------------------
	// VBLANK BEGIN
	//-----------------------------------------------------
	if( VBLANK_BEGIN_LINE==pWS->line ) {
		pWS->ioRam[0xb6] |= pWS->ioRam[0xb2]&INT_VBLANK;
		if( !(++pWS->ioRam[0xaa]) && !(++pWS->ioRam[0xab]) && !(++pWS->ioRam[0xac]) ) {
			++pWS->ioRam[0xad];
		}
	}

	//-----------------------------------------------------
	// VBLANK TIMER
	//-----------------------------------------------------
	if( SCANLINE_MAX==pWS->line ) {
		if(VBLANK_TIMER()) {
			// カウンターがセットされている場合はカウント処理
			if( pWS->ioRam[0xa6] || pWS->ioRam[0xa7]) {
				if( pWS->ioRam[0xa6]==0 ) {
					pWS->ioRam[0xa7]--;
					pWS->ioRam[0xa6]=255;
				} else {
					if((--pWS->ioRam[0xa6])==0) {
						if(pWS->ioRam[0xa7]==0) {
							pWS->ioRam[0xb6]|=(pWS->ioRam[0xb2]&INT_VTIMER);
							if(VBLANK_AUTO()) {
								pWS->ioRam[0xa6] = pWS->ioRam[IO_BACKUP_A6];
								pWS->ioRam[0xa7] = pWS->ioRam[IO_BACKUP_A7];
							}
						}
					}
				}
			} 
			// カウンターがセットされていない場合はカウントせずに割り込み
			else {
				pWS->ioRam[0xb6]|=(pWS->ioRam[0xb2]&INT_VTIMER);
			}
		}
	}

	//-----------------------------------------------------
	// SCANLINE INT
	//-----------------------------------------------------
	if(pWS->line==pWS->line_cmp) {
		pWS->ioRam[0xb6] |= pWS->ioRam[0xb2]&INT_SCLINE; 
	}
}


//------------------------------------------------------------------------------
// 2006.10.10 : 実験結果で以下の条件が必須と判明したので変更しないこと
// SCANLINE       = 0 to 159 (total 160)
// VBLANK Timing  = 145
// 
// FF1はVBLANK TimingでVBLANK Timerをセットして動作する。
// 仙界伝２はVBLANK Timingが145で動作する事が必須になっている。
//
// <仙界伝２>
// SCANLINE=0 to 158, VBLANK Timing=144で処理が進まず
// SCANLINE=0 to 158, VBLANK Timing=145で正常処理。
// SCANLINE=144をベース処理で確認して動作しているようなので
// VBLANK Timingを145にするのは間違いないと判断。
// 
// <FF1>
// SCANLINE=0 to 158, VBLANK Timing=144で正常動作
// SCANLINE=0 to 158, VBLANK Timing=145で画像が１ドットずれる。
// 仙界伝２ではVBLANK Timing=145が必須なので、FF1でも145で１ドットずれてしまう。
// １ドットずれるのはVBlank Timerの動作が原因なので１ドットずらすには
// SCANLINE数を１ドット分だけ変更すればＯＫでは？
// ということでSCANLINE=0 to 159にすることでＯＫとなった。
// 
//------------------------------------------------------------------------------
static int SWAN_Loop(void)
{
    int renderLine = !HAL_fps(75);
//	PIXEL_FORMAT* pFb = fb_format.fb;
    
#ifdef DEF_QUICK
	int iflag=0, beg_line=0;
#endif

	pWS->line=0;

	//-----------------------------------------------------
	// Drawing Area
	//-----------------------------------------------------
	if(renderLine) {
		for(;pWS->line<VBLANK_BEGIN_LINE;pWS->line++) {
			ws_voice_dma();	
			ws_int_work();

#ifdef DEF_QUICK
			if((iflag=ws_int_check())){
				drawLine(beg_line,pWS->line+1); 
				beg_line=pWS->line+1;
			}
#else
			drawLine(pWS->line,pWS->line+1); 
#endif
			nec_execute(256);
		}
#ifdef DEF_QUICK
		if(!iflag){
			drawLine(beg_line,pWS->line);
		}
#endif
	}

	//-----------------------------------------------------
	// No Drawing Area
	//-----------------------------------------------------
    for(;pWS->line<=SCANLINE_MAX;pWS->line++) {
        ws_voice_dma();	
		ws_int_work();
		ws_int_check();
		nec_execute(256);
	}

	//-----------------------------------------------------
	// Sound Processing
	//-----------------------------------------------------
	if(HAL_Sound()) {
		swan_sound(0,SOUND_SamplesPerFrame);
		swan_sound(1,SOUND_SamplesPerFrame);
		swan_sound(2,SOUND_SamplesPerFrame);
		swan_sound(3,SOUND_SamplesPerFrame);
		HAL_Sound_Proc32(halSnd.R32,halSnd.L32,SOUND_SamplesPerFrame);
		halSnd.ch[0]=halSnd.ch[1]=halSnd.ch[2]=halSnd.ch[3]=0;
	}

    if(renderLine) {
        HAL_fb2_bitblt(&fb_format);
    }

    return (ws_key=HAL_Input(0,HW_WSC)) & (1<<31);

}

//------------------------------------------------------------------------------
//
// 実行中のＲＯＭが回転を欲しているかを示す
//
//------------------------------------------------------------------------------
int isWonderSwanRotate(void)
{
    if(ws_RomAddr) {
        return ws_RomAddr[ws_RomSize-4]&1;
    }

    return 0;
}

//------------------------------------------------------------------------------
// ROMがロードされた直後に一度だけ実行される関数
// この関数ではROMに関する処理を記述するし、
// リセットに必要な処理は別関数に実装すること
//------------------------------------------------------------------------------
static int ws_init(char *pRom,int nRom)
{
    ws_romHeaderStruct* pRomHeader = 0;

	core_memset(pWS,0,sizeof(WONDERSWAN_T));

    core_memset(pWsRomMap,0,sizeof(pWsRomMap));
    pWsRomMap[0] = pWS->iRam;
    pWsRomMap[1] = pWS->romRam;

    romAddressMask = nRom-1;
    sramAddressMask = eEepromAddressMask=0;

	pRomHeader = (ws_romHeaderStruct*)&pRom[nRom-10];

	// SRAM
	switch (pRomHeader->eepromSize) {
	  case 0x01: sramAddressMask    = 0x01fff; break; //  64kbit =  8KB (SRAM)
	  case 0x02: sramAddressMask    = 0x07fff; break; //  256kbit= 32KB 
	  case 0x03: sramAddressMask    = 0x1ffff; break; // 1024kbit=128KB
	  case 0x04: sramAddressMask    = 0x3ffff; break; // 2048kbit=256KB
	  case 0x10: eEepromAddressMask = 0x0007f; break; //    1kbit= 128B
	  case 0x20: eEepromAddressMask = 0x007ff; break; //   16kbit=2048B
	  case 0x50: eEepromAddressMask = 0x000ff; break; //    8kbit=1024B
	  case 0x00:
	  default:
		break;
	}

	// Detective Conan 
    if((pRom[nRom-10]==0x01)&&(pRom[nRom-8]==0x27)) { 
        // WS cpu is using cache/pipeline or
        //   there's protected ROM bank where 
        //   pointing CS 
        pRom[0xfffe8]=(char)0xea;
		pRom[0xfffe9]=(char)0x00;
		pRom[0xfffea]=(char)0x00;
		pRom[0xfffeb]=(char)0x00;
		pRom[0xfffec]=(char)0x20;
	}
    
    return 1;
}

//------------------------------------------------------------------------------
//-
//------------------------------------------------------------------------------
static int SWAN_Init(int nRomSize,byte* pRomAddr)
{
	int i;

    ws_RomAddr = pRomAddr;
    ws_RomSize = nRomSize;
    ws_key = 0;

    if( (pWS = (WONDERSWAN_T*)HAL_mem_malloc(sizeof(WONDERSWAN_T))) && 
        (pWC = (SYSTEM_CACHE_T*)HAL_mem_malloc(sizeof(SYSTEM_CACHE_T))) ) 
	{
		// SCANLINE NUMBER TABLE
		{
			for(i=0;i<SCANLINE_MAX;i++) {
				pWC->scanline_num[i]=i;
			}
			pWC->scanline_num[SCANLINE_MAX]=158;
		}
            
        if(ws_init(pRomAddr,nRomSize)) {

			ws_reset();
            
            HAL_fb2_init(256,256,&fb_format,HW_WSC);
            fb_format.pic_w = 224;
            fb_format.pic_h = 144;

			// 左８ピクセルをスクロール用に使用する
			fb_format.pic_x += 8;
            
            // READ : EEPROM(Internal)
            HAL_Cfg_Load(SWAN_INTERNAL_EEP,internalEeprom,sizeof(internalEeprom));
            
            // ROM [SRAM or EEPROM]
            if(sramAddressMask) {
                HAL_Mem_Load(pWS->romRam,sramAddressMask+1);
            }
            else if(eEepromAddressMask) {
                HAL_Mem_Load(pWS->romE2P,eEepromAddressMask+1);
            }
            
            return 1;
        }
    }

    return 0;
}

//------------------------------------------------------------------------------
// 
//------------------------------------------------------------------------------
static int SWAN_Exit(void)
{
    // WRITE : EEPROM(Internal)
    HAL_Cfg_Save(SWAN_INTERNAL_EEP,internalEeprom,sizeof(internalEeprom));
    
	// ROM [SRAM or EEPROM]
    if(sramAddressMask) {
        HAL_Mem_Save(pWS->romRam,sramAddressMask+1);
    }
    else if(eEepromAddressMask) {
        HAL_Mem_Save(pWS->romE2P,eEepromAddressMask+1);
    }

    ws_RomAddr = 0;
    ws_RomSize = 0;

    if(pWS) HAL_mem_free(pWS); 
    if(pWC) HAL_mem_free(pWC);

    pWS = 0;
    pWC=0;
    
    return 1;
}

//------------------------------------------------------------------------------
// 
//------------------------------------------------------------------------------
int SWAN_Setup(void)
{
    HAL_SetupExt( EXT_WS, "ws", SWAN_Init, SWAN_Loop, SWAN_Exit, ws_reset, ws_load, ws_save );
    HAL_SetupExt( EXT_WSC,"wsc",SWAN_Init, SWAN_Loop, SWAN_Exit, ws_reset, ws_load, ws_save );

    return 1;
}


