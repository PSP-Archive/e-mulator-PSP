#include "neopop.h"
#include "mem.h"
#include "gfx.h"
#include "interrupt.h"
#include "./tlcs-900h/interpret.h"
#include "Z80_interface.h"
#include "dma.h"

u8 zbuffer[256];
u16* cfb_scanline;	//set = scanline * SCREEN_WIDTH


//=============================================================================
/*
u32 timer_hint;
u32 timer_clock0;
u32 timer_clock1;
u32 timer_clock2;
u32 timer_clock3;
u8 timer[4];	//Up-counters

BOOL gfx_hack = FALSE;
*/
//=============================================================================

BOOL h_int = FALSE;
BOOL timer0;
BOOL timer2;

//=============================================================================

extern void push32(u32 data);
extern void push16(u16 data);
extern unsigned char *ng_map[0x100];

void interrupt(u8 index)
{
#ifdef PC32ADR
	push32(pc32_adr);
#else
	push32(pc32);
#endif
    push16(sr);

	//Up the IFF
    if(((sr & 0x7000) >> 12) < 7) {
        setStatusIFF(((sr & 0x7000) >> 12) + 1);
    }
    
    //Access the interrupt vector table to find the jump destination
#ifdef PC32ADR
	{
		u32 f = le32toh(*(u32*)(ram + 0x6FB8 + (index * 4)));
		pc32_adr = &(ng_map[(u8)(f>>16)])[((u16)f)];
	}
#else
	pc32 = le32toh(*(u32*)(ram + 0x6FB8 + (index * 4)));
#endif

#ifdef NEOPOP_DEBUG
//	if (index != 5 && index != 7 && index != 10)
//	system_debug_message("interrupt %d: pc32 -> %06X", index, pc32);
#endif
}

//=============================================================================


void gfx_hint(void)
{
	//H_Int / Delayed settings
	if ((ram[0x8009] < SCREEN_HEIGHT-1 || ram[0x8009] == 198)) {

#if 0
//		gfx_delayed_settings();	/* Get delayed settings */
#else
		//void gfx_delayed_settings(void)
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
#endif

		/* Allowed? */
		if (ram[0x8000] & 0x40)
			h_int = 1;
	}
}


void gfx_draw(void)
{
//  if (frameskip_count == 0)
	{
		//Draw the scanline
//		if (ram[0x8009] < SCREEN_HEIGHT)
		{
			if (ram[0x6F95] == 0x10)	gfx_draw_scanline_colour();
			else						gfx_draw_scanline_mono();
		}
	}
}

//=============================================================================

void reset_timers(void)
{
//	timer_hint = 0;

	timer[0] = 0;
	timer[1] = 0;
	timer[2] = 0;
	timer[3] = 0;
	
	timer_clock0 = 0;
	timer_clock1 = 0;
	timer_clock2 = 0;
	timer_clock3 = 0;
}

//=============================================================================

