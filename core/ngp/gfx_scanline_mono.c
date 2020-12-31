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
#include "neopop.h"
#include "mem.h"
#include "gfx.h"
#include "hal.h"
#include "cstring.h"

//=============================================================================

static void Plot(u8 x, u8* palette_ptr, u16 pal_hi, u8 index, u8 depth)
{
	u8 data8;
    
	//Clip
	if (index == 0 || x < winx || x >= (winw + winx) || x >= SCREEN_WIDTH)
		return;

	//Depth check, <= to stop later sprites overwriting pixels!
	if (depth <= zbuffer[x]) return;
	zbuffer[x] = depth;

	//Get the colour of the pixel
	if (pal_hi) data8 = palette_ptr[4 + index];
	else        data8 = palette_ptr[0 + index];

#if 1 // defined(PAL16m)
//    data8 = (data8 & 7);
//    if (negative) cfb_scanline[x] =  PAL16m(data8);
//	else          cfb_scanline[x] = ~PAL16m(data8);
	{
		u16 pal = (data8&7)<<1;
		u16 fmt = HAL_fb2_Color(pal,pal,pal,RGB444);
		if (negative) cfb_scanline[x] = fmt;
		else          cfb_scanline[x] =~fmt;
	}
#else
	r = (data8 & 7) << 1;
	g = (data8 & 7) << 5;
	b = (data8 & 7) << 9;

    if (negative)
		cfb_scanline[x] = (r | g | b);
	else
		cfb_scanline[x] = ~(r | g | b);
#endif
    
}

static void drawPattern(u8 screenx, u16 tile, u8 tiley, u16 mirror, 
				 u8* palette_ptr, u16 pal, u8 depth)
{
	//Get the data for th e "tiley'th" line of "tile".
	u16 data = le16toh(*(u16*)(ram + 0xA000 + (tile * 16) + (tiley * 2)));

	//Horizontal Flip
	if (mirror) {
		Plot(screenx + 7, palette_ptr, pal, (data & 0xC000) >> 0xE, depth);
		Plot(screenx + 6, palette_ptr, pal, (data & 0x3000) >> 0xC, depth);
		Plot(screenx + 5, palette_ptr, pal, (data & 0x0C00) >> 0xA, depth);
		Plot(screenx + 4, palette_ptr, pal, (data & 0x0300) >> 0x8, depth);
		Plot(screenx + 3, palette_ptr, pal, (data & 0x00C0) >> 0x6, depth);
		Plot(screenx + 2, palette_ptr, pal, (data & 0x0030) >> 0x4, depth);
		Plot(screenx + 1, palette_ptr, pal, (data & 0x000C) >> 0x2, depth);
		Plot(screenx + 0, palette_ptr, pal, (data & 0x0003) >> 0x0, depth);
    } else {
        //Normal
        Plot(screenx + 0, palette_ptr, pal, (data & 0xC000) >> 0xE, depth);
		Plot(screenx + 1, palette_ptr, pal, (data & 0x3000) >> 0xC, depth);
		Plot(screenx + 2, palette_ptr, pal, (data & 0x0C00) >> 0xA, depth);
		Plot(screenx + 3, palette_ptr, pal, (data & 0x0300) >> 0x8, depth);
		Plot(screenx + 4, palette_ptr, pal, (data & 0x00C0) >> 0x6, depth);
		Plot(screenx + 5, palette_ptr, pal, (data & 0x0030) >> 0x4, depth);
		Plot(screenx + 6, palette_ptr, pal, (data & 0x000C) >> 0x2, depth);
		Plot(screenx + 7, palette_ptr, pal, (data & 0x0003) >> 0x0, depth);
	}
}

static void gfx_draw_scroll1(u8 depth)
{
	u8 tx, row, line;
	u16 data16;

	line = scanline + scroll1y;
	row = line & 7;	//Which row?

	//Draw Foreground scroll plane (Scroll 1)
    for (tx = 0; tx < 32; tx++) {
		data16 = le16toh(*(u16*)(ram + 0x9000 + ((tx + ((line >> 3) << 5)) << 1)));
		
		//Draw the line of the tile
        drawPattern((tx << 3) - scroll1x,
                    data16 & 0x01FF, 
                    (data16 & 0x4000) ? 7 - row : row,
                    data16 & 0x8000,
                    ram + 0x8108,
                    data16 & 0x2000,
                    depth);
    }
}

static void gfx_draw_scroll2(u8 depth)
{
	u8 tx, row, line;
	u16 data16;

	line = scanline + scroll2y;
	row = line & 7;	//Which row?

	//Draw Background scroll plane (Scroll 2)
    for (tx = 0; tx < 32; tx++) {
        data16 = le16toh(*(u16*)(ram + 0x9800 + ((tx + ((line >> 3) << 5)) << 1)));
        
        //Draw the line of the tile
        drawPattern((tx << 3) - scroll2x,
                    data16 & 0x01FF, 
                    (data16 & 0x4000) ? 7 - row : row,
                    data16 & 0x8000,
                    ram + 0x8110,
                    data16 & 0x2000,
                    depth);
	}
}

void gfx_draw_scanline_mono(void)
{
	s16 lastSpriteX;
	s16 lastSpriteY;
	int spr, x;
	u16 data16;

	//Get the current scanline
	scanline = ram[0x8009];

#if 1
    cfb_scanline = fb_format.fb + (scanline * fb_format.width);
	core_memset(cfb_scanline, 0, (fb_format.width * fb_format.bpp));
#else
    cfb_scanline = cfb + (scanline * 256);	//Calculate fast offset
	core_memset(cfb_scanline, 0, SCREEN_WIDTH * sizeof(u16));
#endif
    
	core_memset(zbuffer, 0, SCREEN_WIDTH);

	//Window colour
#if defined(PAL16m)
    if(negative) data16 =  PAL16m(oowc);
    else         data16 =~(PAL16m(oowc));
#else
	r = (u16)oowc << 1;
	g = (u16)oowc << 5;
	b = (u16)oowc << 9;
	if (negative) data16 = (r | g | b);
	else          data16 = ~(r | g | b);
#endif
    

	//Top
    if (scanline < winy) {
        for (x = 0; x < SCREEN_WIDTH; x++) {
            cfb_scanline[x] = data16;
        }
    }
    else {
        //Middle
        if (scanline < winy + winh) {
            for (x = 0; x < min(winx, SCREEN_WIDTH); x++) {
                cfb_scanline[x] = data16;
            }
            for (x = min(winx + winw, SCREEN_WIDTH); x < SCREEN_WIDTH; x++) {
                cfb_scanline[x] = data16;
            }
        } else { //Bottom
            for (x = 0; x < SCREEN_WIDTH; x++) {
                cfb_scanline[x] = data16;
            }
		}
	}

	//Ignore above and below the window's top and bottom
	if (scanline >= winy && scanline < winy + winh) {
        //Background colour Enabled?
        if ((bgc & 0xC0) == 0x80) {
#if defined(PAL16m)
            data16 = ~PAL16m(bgc&7);
#else
			r = (u16)(bgc & 7) << 1;
			g = (u16)(bgc & 7) << 5;
			b = (u16)(bgc & 7) << 9;
			data16 = ~(r | g | b);
#endif
		}
#if defined(PAL16m)
		else data16 = 0x7FFF;
#else
		else data16 = 0x0FFF;
#endif
        
		if (negative) data16 = ~data16;
		
		//Draw background!
        for (x = winx; x < min(winx + winw, SCREEN_WIDTH); x++)	{
            cfb_scanline[x] = data16;
        }
        
        //Swap Front/Back scroll planes?
        if (planeSwap) {
            gfx_draw_scroll1(ZDEPTH_BACKGROUND_SCROLL);		//Swap
            gfx_draw_scroll2(ZDEPTH_FOREGROUND_SCROLL);
        } else {
            gfx_draw_scroll2(ZDEPTH_BACKGROUND_SCROLL);		//Normal
            gfx_draw_scroll1(ZDEPTH_FOREGROUND_SCROLL);
        }
        
		//Draw Sprites
		//Last sprite position, (defaults to top-left, sure?)
		lastSpriteX = 0;
		lastSpriteY = 0;
        
        for (spr = 0; spr < 64; spr++) {
            u8 priority, row;
            u8 sx = ram[0x8800 + (spr * 4) + 2];	//X position
            u8 sy = ram[0x8800 + (spr * 4) + 3];	//Y position
            s16 x = sx;
            s16 y = sy;
			
			data16 = le16toh(*(u16*)(ram + 0x8800 + (spr * 4)));
			priority = (data16 & 0x1800) >> 11;

			if (data16 & 0x0400) x = lastSpriteX + sx;	//Horizontal chain?
			if (data16 & 0x0200) y = lastSpriteY + sy;	//Vertical chain?

			//Store the position for chaining
			lastSpriteX = x;
			lastSpriteY = y;
			
			//Visible?
			if (priority == 0)	continue;

			//Scroll the sprite
			x += scrollsprx;
			y += scrollspry;

			//Off-screen?
			if (x > 248 && x < 256)	x = x - 256; else x &= 0xFF;
			if (y > 248 && y < 256)	y = y - 256; else y &= 0xFF;

			//In range?
            if (scanline >= y && scanline <= y + 7) {
                row = (scanline - y) & 7;	//Which row?
                drawPattern((u8)x,
                            data16 & 0x01FF, 
                            (data16 & 0x4000) ? 7 - row : row,
                            data16 & 0x8000,
                            ram + 0x8100,
                            data16 & 0x2000,
                            priority << 1); 
			}
		}

	}

	//==========
}

//=============================================================================
