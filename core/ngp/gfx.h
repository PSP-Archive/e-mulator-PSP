//---------------------------------------------------------------------------
// NEOPOP : Emulator as in Dreamland
//
// Copyright (c) 2001-2002 by neopop_uk
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 2 of the License, or
//	(at your option) any later version. See also the license.txt file for
//	additional informations.
//---------------------------------------------------------------------------

#ifndef __GFX__
#define __GFX__
//=============================================================================

#if 0

#define ZDEPTH_BACK_SPRITE			0x01
#define ZDEPTH_BACKGROUND_SCROLL	0x02
#define ZDEPTH_MIDDLE_SPRITE		0x04
#define ZDEPTH_FOREGROUND_SCROLL	0x08
#define ZDEPTH_FRONT_SPRITE			0x10

#else

#define ZDEPTH_BACK_SPRITE			2
#define ZDEPTH_BACKGROUND_SCROLL	3
#define ZDEPTH_MIDDLE_SPRITE		4
#define ZDEPTH_FOREGROUND_SCROLL	5
#define ZDEPTH_FRONT_SPRITE			6

#endif

//=============================================================================

//---------------------------
// Common Graphics Variables
//---------------------------

extern u8 zbuffer[256];	//Line z-buffer
extern u16* cfb_scanline;	//set = cfb + (scanline * SCREEN_WIDTH)

//void gfx_delayed_settings(void);

//=============================================================================

void gfx_draw_scanline_colour(void);
void gfx_draw_scanline_mono(void);

//=============================================================================
#endif

