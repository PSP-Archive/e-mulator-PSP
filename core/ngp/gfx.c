//---------------------------------------------------------------------------
// NEOPOP : Emulator as in Dreamland
//
// Copyright (c) 2001-2002 by neopop_uk
//---------------------------------------------------------------------------
#if 0

#include "neopop.h"
#include "mem.h"
#include "gfx.h"

//=============================================================================

//u16 cfb[256*256]; /* __attribute__((aligned(32))); */
//extern FBFORMAT fb_format;

u8 zbuffer[256];

u16* cfb_scanline;	//set = scanline * SCREEN_WIDTH

//=============================================================================

/*
void gfx_delayed_settings(void)
{
	//Window dimensions
	winx = ram[0x8002];
	winy = ram[0x8003];
	winw = ram[0x8004];
	winh = ram[0x8005];

#if defined(WINXW)
    if( (winx+winw) < SCREEN_WIDTH ) {
        winxw = winx+winw;
    } else {
        winxw = SCREEN_WIDTH;
    }
#endif

	//Scroll Planes (Confirmed delayed)
	scroll1x = ram[0x8032];
	scroll1y = ram[0x8033];
	scroll2x = ram[0x8034];
	scroll2y = ram[0x8035];

	//Sprite offset (Confirmed delayed)
	scrollsprx = ram[0x8020];
	scrollspry = ram[0x8021];

	//Plane Priority (Confirmed delayed)
	planeSwap = ram[0x8030] & 0x80;

	//Background colour register (Confirmed delayed)
	bgc = ram[0x8118];

	//2D Control register (Confirmed delayed)
	oowc = ram[0x8012] & 7;
	negative = ram[0x8012] & 0x80;
}
*/
#endif

//=============================================================================
