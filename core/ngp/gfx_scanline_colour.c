//---------------------------------------------------------------------------
// NEOPOP : Emulator as in Dreamland
//
// Copyright (c) 2001-2002 by neopop_uk
//---------------------------------------------------------------------------

#include "neopop.h"
#include "mem.h"
#include "gfx.h"
#include "hal.h"

#include "cstring.h"

extern u16 mir_pat[0x10000];

//=============================================================================
// 
// パターンの表示方法を変更
// 
static void drawPattern(u8 scx, u16 tile, u8 tiley, u16 mirror, u16* palette_ptr, u8 depth)
{
	//Get the data for th e "tiley'th" line of "tile".
	u16 data = *(u16*)(ram + 0xA000 + (tile * 16) + (tiley * 2));
    
	//Horizontal Flip
	if(data) {
		u8 idx,i,max=8;

		if(mirror) { data = mir_pat[data]; }

		for(i=0;i<max;i++) {
			idx = data>>14;

			if(idx) {
				if((depth>zbuffer[scx])) { 
					cfb_scanline[scx] = palette_ptr[idx]; 
					zbuffer[scx]=depth; 
				}
			}
			data<<=2; 
			scx++;
		}
	}
}


static void drawPatternBg(u8 scx, u16 tile, u8 tiley, u16 mirror, u16* palette_ptr, u8 depth)
{
	//Get the data for th e "tiley'th" line of "tile".
	u16 data = *(u16*)(ram + 0xA000 + (tile * 16) + (tiley * 2));
    
	//Horizontal Flip
	if(data) {
		u8 idx,i,max=8;

		if(mirror) { data = mir_pat[data]; }

		if(depth==3) {
			for(i=0;i<max;i++) {
				idx = data>>14;

				if(idx) {
					cfb_scanline[scx] = palette_ptr[idx]; 
					zbuffer[scx]=depth; 
				}
				data<<=2; 
				scx++;
			}
		} else {
			for(i=0;i<max;i++) {
				idx = data>>14;

				if(idx) {
					if((depth>zbuffer[scx])) { 
						cfb_scanline[scx] = palette_ptr[idx]; 
						zbuffer[scx]=depth; 
					}
				}
				data<<=2; 
				scx++;
			}
		}
	}
}


static void gfx_draw_scroll(u8 depth,int num)
{
	s8 tx;
	u8 row, line;
	u16 data16;
	PIXEL_FORMAT* pfmt;
	u16* pBg;
	u8 x,scrx;

	if(num==0) {
		scrx = scroll1x;
		line = scanline + scroll1y;
		pBg  = (u16*)(ram + 0x9000 + ((((line >> 3) << 5)) << 1));
		pfmt = (negative)? ngp_c->n_sc1 : ngp_c->p_sc1;
	} else {
		scrx = scroll2x;
		line = scanline + scroll2y;
		pBg  = (u16*)(ram + 0x9800 + ((((line >> 3) << 5)) << 1));
		pfmt = (negative)? ngp_c->n_sc2 : ngp_c->p_sc2;
	}

	row = line & 7;	//Which row?


	//Draw Background scroll plane (Scroll 2)
    for(tx=0;tx<32;tx++){
	//for(tx=31;tx>=0;tx--){
        data16 = pBg[tx];

		x = tx*8-scrx;

		if(x>160) {
			continue;
		}

        //Draw the line of the tile
        drawPatternBg(x, //(tx << 3) - scrx,
                    data16 & 0x01FF, 
                    (data16 & 0x4000) ? (7 - row) : row,
                    data16 & 0x8000,
#if defined(QPAL2)
                    pfmt + ((data16 & 0x1E00) >> 7),
#else
                    (u16*)(ram + 0x8300) + ((data16 & 0x1E00) >> 7),
#endif
                    depth);
	}
}

void gfx_draw_scanline_colour(void)
{
	PIXEL_FORMAT *pfmt,bgcol;
    s16 lastSpriteX,lastSpriteY;
    int spr, x;
    u16 data16;
	u8 r,g,b;

	if(negative) pfmt = ngp_c->n_spr;
	else         pfmt = ngp_c->p_spr;


#if defined(QPAL2) // メモリ書き込み時にパレットを作った方がいいかも？
#define MAKEPAL(a) r=((a))&0x0f;  g=((a)>>4)&0x0f;  b=((a)>>8)&0x0f;

    if(ngp_c->qpal2) {
		PIXEL_FORMAT fmt,*pNor,*pNeg,*p123;

		PIXEL_FORMAT* ptr[3]={&ram[0x8200], &ram[0x8280], &ram[0x8300] };
		PIXEL_FORMAT* p_n[3]={ngp_c->p_spr, ngp_c->p_sc1, ngp_c->p_sc2 };
		PIXEL_FORMAT* n_n[3]={ngp_c->n_spr, ngp_c->n_sc1, ngp_c->n_sc2 };
		int i;

		for(i=0;i<3;i++) {
			if(ngp_c->qpal2 & (1<<i)) {
				p123 = ptr[i];
				pNor = p_n[i];
				pNeg = n_n[i];

				for(x=0;x<64;x++) {
					MAKEPAL(*p123); p123++; 
					fmt = HAL_fb2_Color(r,g,b,RGB444); 
					*pNor++=fmt; 
					*pNeg++=~fmt;
				}
			}
		}

        ngp_c->qpal2 = 0;
    }
#endif // QPAL2

	//Get the current scanline
    scanline = ram[0x8009];

#if 1
    cfb_scanline = fb_format.fb + (scanline * fb_format.width);
#else
	cfb_scanline = cfb + (scanline * 256);	//Calculate fast offset
#endif
    
//    core_memset(cfb_scanline, 0, SCREEN_WIDTH * sizeof(u16));
    core_memset(zbuffer, 0, SCREEN_WIDTH);
    
    //Window colour
    data16 = *(u16*)(ram + 0x83F0 + (oowc << 1));

#if 1
	MAKEPAL(data16);
	data16 = HAL_fb2_Color(r,g,b,RGB444);
#endif
    
    if (negative) data16 = ~data16;

	bgcol = data16; // background color

	//Ignore above and below the window's top and bottom
    if (scanline >= winy && scanline < winy + winh) {
        //Background colour Enabled?	HACK: 01 AUG 2002 - Always on!
//		if ((bgc & 0xC0) == 0x80) {
		MAKEPAL((*(u16*)&ram[0x83E0+((bgc&7)<<1)]));
		data16 = HAL_fb2_Color(r,g,b,RGB444);
//		}
//		else data16 = 0;

		if (negative) data16 = ~data16;

		for(x=0;x<winx;x++) { 
			cfb_scanline[x] = bgcol; 
		}

		//Draw background!
        for(x=winx;x<min(winx + winw, SCREEN_WIDTH); x++)	{
            cfb_scanline[x] = data16;
        }

		for(;x<SCREEN_WIDTH;x++) {
			cfb_scanline[x] = bgcol; 
		}

		//Draw Sprites
		//Last sprite position, (defaults to top-left, sure?)
		lastSpriteX = 0;
		lastSpriteY = 0;

		//Swap Front/Back scroll planes?
        gfx_draw_scroll(ZDEPTH_BACKGROUND_SCROLL,(planeSwap)?0:1);

        for(spr=0;spr<64;spr++) {
            u8 priority,row,x,y,sx,sy;
 			u8* pSPR = &ram[0x8800+spr*4];
            u16 data16 = *(u16*)pSPR;

			x = sx = pSPR[2]; // X position
            y = sy = pSPR[3]; // Y position

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

				if( (u8)x>160) { 
					continue; 
				}

                row = (scanline - y) & 7;	//Which row?
                drawPattern((u8)x,
                            data16 & 0x01FF, 
                            (data16 & 0x4000) ? 7 - row : row,
                            data16 & 0x8000,
#if defined(QPAL2)
                            pfmt + (ram[0x8C00 + spr] & 0xF) * 4,
#else
                            (u16*)(ram + 0x8200) + (ram[0x8C00 + spr] & 0xF)*4,
#endif
                            priority << 1); 
            }
		}

		//Swap Front/Back scroll planes?
        gfx_draw_scroll(ZDEPTH_FOREGROUND_SCROLL,(planeSwap)?1:0);

		//==========
	}

}

