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
#include "./tlcs-900h/interpret.h"
#include "Z80_interface.h"
#include "interrupt.h"
#include "mem.h"
#include "hal.h"

extern u8 TLCS900h_interpret(void);

FBFORMAT fb_format;

int ngp_frameskip=0;

#if defined(QMEM)

NGP_CACHE* ngp_c=0;
NGP_MEM  * ngp_m=0;

#endif

//=============================================================================

//BOOL language_english;

COLOURMODE system_colour;

//=============================================================================

static BOOL debug_abort_instruction = FALSE;

//void __cdecl instruction_error(char* vaMessage,...)
//{
//#if 0
//	char message[1000];
//	va_list vl;
//
//	va_start(vl, vaMessage);
//	vsprintf(message, vaMessage, vl);
//	va_end(vl);
//
//#ifdef NEOPOP_DEBUG
//	system_debug_message(message);
//	debug_abort_instruction = TRUE;
//#else
////	system_message("[PC %06X] %s", pc, message);
//#endif
//#endif // 0
//}

//=============================================================================

u16 mir_pat[0x10000];

void build_mirror_pat(void)
{
	int i,j;
	u16 pat;

	for(i=0;i<0x10000;i++) {

		pat = 0;
		for(j=0;j<8;j++) {
			pat = (pat<<2) | (3&(i>>(j*2)));
		}

		mir_pat[i] = pat;
	}
}

static int NGX_INIT(int nRomSize,byte* pRomAddr,int type)
{
    // SETUP FrameBuffer
    HAL_fb2_init(256,256,&fb_format,HW_NGP);

    fb_format.pic_x = 0;
    fb_format.pic_y = 0;
    fb_format.pic_w = 160;
    fb_format.pic_h = 152;

    //----------------------------------------------------
    rom.length = nRomSize;
    rom.data   = pRomAddr;
    rom_header = (RomHeader*)(pRomAddr);

    system_colour = type;

    if(!ngp_c) {
        ngp_c = (NGP_CACHE*)HAL_mem_malloc(sizeof(NGP_CACHE));
    }

    if(!ngp_m) {
        ngp_m = (NGP_MEM*)HAL_mem_malloc(sizeof(NGP_MEM));
    }


    ngp_memmap();

	sound_init(44100);
    	
	bios_install();
    
    rom_hack();
    rom_loaded();
    ngp_reset();

	//-----------------------
	build_mirror_pat();

    return 1;
}

// color
static int NPC_INIT(int nRomSize,byte* pRomAddr)
{
    return NGX_INIT(nRomSize,pRomAddr,COLOURMODE_COLOUR);
}

// normal
static int NGP_INIT(int nRomSize,byte* pRomAddr)
{
    return NGX_INIT(nRomSize,pRomAddr,COLOURMODE_GREYSCALE);
}

//
// NeoGeo Pocket Emulator main loop
//
// TLCS-900h : 6.144MHz => 6144000Hz / 60fps / 200hsync = 512
// Z80       : 3.072MHz => 3072000Hz / 60fps / 200hsync = 256
// 

#define Z80_CYCLE  256
#define TLCS_CYCLE 512

extern s32 ExecZ80withCycle(Z80 *R,s32 cycle);

#if 1 ////////////////////////////////////////////////////////////////////////

extern BOOL h_int;
extern BOOL timer0;
extern BOOL timer2;
extern void gfx_hint(void);
extern void gfx_draw(void);

//int timer_hint=0;

static void updateTimers(u32 cputicks)
{
	//increment H-INT timer
	timer_hint += cputicks;

	//=======================

	//End of scanline / Start of Next one
	if (timer_hint >= TIMER_HINT_RATE) {
		// ============= END OF CURRENT SCANLINE =============
		if(0==ngp_frameskip) {
			if(ram[0x8009] < SCREEN_HEIGHT) {
				if (gfx_hack)	{	gfx_draw(); gfx_hint(); }
				else			{	gfx_hint();	gfx_draw(); }
			}
		} 

		// ============= START OF NEXT SCANLINE =============

		ram[0x8009]++;	//Next scanline
		timer_hint = 0;	//Start of next scanline

#if 0	//Comms. Read interrupt
		if ((ram[0xB2] & 1) == 0 && system_comms_poll(&data) && 
			(statusIFF() <= (ram[0x77] & 7)))
		{
			ram[0x50] = data;
			interrupt(12); 
			if (ram[0x007C] == 0x19)		DMA_update(0);
			else { if (ram[0x007D] == 0x19)	DMA_update(1);
			else { if (ram[0x007E] == 0x19)	DMA_update(2);
			else { if (ram[0x007F] == 0x19)	DMA_update(3);	}}}
		}
#endif

		//V_Int?
		if (ram[0x8009] == SCREEN_HEIGHT) {
			ram[0x8010] = 0x40;	//Character Over / Vblank Status
			if(0==ngp_frameskip) {
			    HAL_fb2_bitblt(&fb_format);
				//system_VBL();	//Update the screen
			}

			//Vertical Interrupt? (Confirmed IRQ Level)
			if (statusIFF() <= 4 && (ram[0x8000] & 0x80)){
				interrupt(5); // VBL
				if(ram[0x007C]==0x0B) DMA_update(0); else { 
				if(ram[0x007D]==0x0B) DMA_update(1); else { 
				if(ram[0x007E]==0x0B) DMA_update(2); else { 
				if(ram[0x007F]==0x0B) DMA_update(3);	}}}
			}
		}

		//End of V_Int
		if (ram[0x8009] == 198 + 1) { //Last scanline + 1
			ram[0x8009] = 0;
			ram[0x8010] = 0;	//Character Over / Vblank Status
		}
	}

	//=======================

	//Tick the Clock Generator
	timer_clock0 += cputicks;
	timer_clock1 += cputicks;
	
	timer0 = FALSE;	//Clear the timer0 tick, for timer1 chain mode.

	//=======================

	//Run Timer 0 (TRUN)?
	if ((ram[0x20] & 0x01)) {
		//T01MOD
		switch(ram[0x24] & 0x03){
		case 0:	
			if (h_int) { /* Horizontal interrupt trigger */
				timer[0]++;
				timer_clock0 = 0;
				h_int = FALSE;	/* Stop h_int remaining active */
			}
		break;

		case 1:
			if (timer_clock0 >= TIMER_T1_RATE) {
				timer[0]++;
				timer_clock0 = 0;
			}
			break;

		case 2:	
			if (timer_clock0 >= TIMER_T4_RATE) {
				timer[0]++;
				timer_clock0 = 0;
			}
			break;

		case 3:	
			if (timer_clock0 >= TIMER_T16_RATE) {
				timer[0]++;
				timer_clock0 = 0;
			}
			break;
		}


		//Threshold check
		if (ram[0x22] && (timer[0] >= ram[0x22])) {
			timer[0] = 0;
			timer0 = TRUE;

			if (statusIFF() <= (ram[0x73] & 0x7)) {
				interrupt(7); // Timer 0 Int.
			}

			if(ram[0x007C]==0x10) DMA_update(0); else { 
			if(ram[0x007D]==0x10) DMA_update(1); else { 
			if(ram[0x007E]==0x10) DMA_update(2); else { 
			if(ram[0x007F]==0x10) DMA_update(3); }}}
		}
	}

	//=======================

	//Run Timer 1 (TRUN)?
	if ((ram[0x20] & 0x02)) {
		//T23MOD
		switch((ram[0x24] & 0x0C) >> 2){
		case 0:	
			if (timer0)	{/* Timer 0 chain mode. */
				timer[1]++;
				timer_clock1 = 0;
			}
			break;

		case 1:
			if (timer_clock1 >= TIMER_T1_RATE) {
				timer[1]++;
				timer_clock1 = 0;
			}
			break;

		case 2:	
			if (timer_clock1 >= TIMER_T16_RATE) {
				timer[1]++;
				timer_clock1 = 0;
			}
			break;

		case 3:
			if (timer_clock1 >= TIMER_T256_RATE) {
				timer[1]++;
				timer_clock1 = 0;
			}
			break;
		}

		//Threshold check
		if (ram[0x23] && timer[1] >= ram[0x23]) {
			timer[1] = 0;

			if (statusIFF() <= ((ram[0x73] & 0x70) >> 4)) {
				interrupt(8); // Timer 1 Int.
			}

			if(ram[0x007C]==0x11) DMA_update(0); else { 
			if(ram[0x007D]==0x11) DMA_update(1); else { 
			if(ram[0x007E]==0x11) DMA_update(2); else { 
			if(ram[0x007F]==0x11) DMA_update(3); }}}
		}
	}

	//=======================

	//Tick the Clock Generator
	timer_clock2 += cputicks;
	timer_clock3 += cputicks;

	timer2 = FALSE;	//Clear the timer2 tick, for timer3 chain mode.

	//=======================
	
	//Run Timer 2 (TRUN)?
	if ((ram[0x20] & 0x04)) {
		//T23MOD
		switch(ram[0x28] & 0x03) {
		case 0:	/* - */break;

		case 1:	
			if (timer_clock2 >= 56/*TIMER_T1_RATE*/) {	//HACK - Fixes DAC
				timer[2]++;
				timer_clock2 = 0;
			}
			break;

		case 2:
			if (timer_clock2 >= TIMER_T4_RATE) {
				timer[2]++;
				timer_clock2 = 0;
			}
			break;

		case 3:	
			if (timer_clock2 >= TIMER_T16_RATE) {
				timer[2]++;
				timer_clock2 = 0;
			}
			break;
		}

		//Threshold check
		if (ram[0x26] && timer[2] >= ram[0x26])
		{
			timer[2] = 0;
			timer2 = TRUE;

			if (statusIFF() <= ((ram[0x74] & 0x07))) {
				interrupt(9);	// Timer 2 Int.
			}

			if(ram[0x007C]==0x12) DMA_update(0); else { 
			if(ram[0x007D]==0x12) DMA_update(1); else { 
			if(ram[0x007E]==0x12) DMA_update(2); else { 
			if(ram[0x007F]==0x12) DMA_update(3); }}}
		}
	}

	//=======================

	//Run Timer 3 (TRUN)?
	if ((ram[0x20] & 0x08)) {
		//T23MOD
		switch((ram[0x28] & 0x0C) >> 2){
		case 0:	/* Timer 2 chain mode. */
			if (timer2) { 
				timer[3]++;
				timer_clock3 = 0;
			}
			break;

		case 1:	
			if (timer_clock3 >= TIMER_T1_RATE){
				timer[3]++;
				timer_clock3 = 0;
			}
			break;

		case 2:	
			if (timer_clock3 >= TIMER_T16_RATE){
				timer[3]++;
				timer_clock3 = 0;
			}
			break;

		case 3:
			if (timer_clock3 >= TIMER_T256_RATE){
				timer[3]++;
				timer_clock3 = 0;
			}
			break;
		}

		//Threshold check
		if (ram[0x27] && timer[3] >= ram[0x27]) {
			timer[3] = 0;

			Z80_irq();

			if (statusIFF() <= ((ram[0x74] & 0x70) >> 4))
				interrupt(10); // Timer 3 Int.

			if(ram[0x007C] == 0x13) DMA_update(0); else { 
			if(ram[0x007D] == 0x13) DMA_update(1); else { 
			if(ram[0x007E] == 0x13) DMA_update(2); else { 
			if(ram[0x007F] == 0x13) DMA_update(3);	}}}
		}
	}

	//=======================
}
#endif////////////////////////////////////////////////////////////////////////

extern void dac_update(u8* dac_buffer, int length_bytes);
extern s32 TLCS900h_cycle(s32 nCycles);

unsigned char b8[736];

short buf[2][736];

// 
static int NGP_LOOP(void)
{
    int c=0,sl,cnt;
    int tlcs_count,key_upd = 0;
    int z80_count;
	int tlcs,cy,zy,sp = 32;

    ngp_frameskip = HAL_fps(60);

    ngp_m->c_cycle = 512;
    
    tlcs_count= TLCS_CYCLE;
    z80_count = Z80_CYCLE;

    // init ?
//    ram[0x8009] = 0;
    ram[0x8010] = 0;	//Character Over / Vblank Status
    
    for(sl=0;sl<200;sl++) {
		cy=zy=0;

        ram[0x8009] = sl;

		//Execute several instructions to boost performance
#if 1 /*1*/
#if 2 /*2*/
		timer_hint = 0;

		while(cy<TIMER_HINT_RATE) {
			tlcs = TLCS900h_cycle(sp);
			cy+=tlcs;

			if(Z80ACTIVE && zy<TIMER_HINT_RATE) {
				zy+= 2 * Z80EMULATES(sp/2);
			}
			updateTimers(tlcs);
		}
#else /*2*/
//		cnt2=515;
//		while(cnt2>0) {
//			cnt = TLCS900h_interpret();
//			cnt2-=cnt;
//		}
//
//		if(Z80ACTIVE) {
//			while(Z80_regs.ICount>0) {
//				Z80EMULATE;
//			}
//		}
//		updateTimers(515);
#endif/*2*/
#else /*1*/
//		//Execute several instructions to boost performance
//		for (i = 0; i < 64; i++) {
//			cnt = TLCS900h_interpret();
//			updateTimers(cnt);
//			if (Z80ACTIVE) { Z80EMULATE }
//			cnt = TLCS900h_interpret();
//			updateTimers(cnt);
//		}
#endif/*1*/
    }

	if(HAL_Sound()) {
		sound_update(buf[0],buf[1],736);

		if(0){
			int i;
			dac_update(b8,736);

			for(i=0;i<736;i++) {
				buf[0][i] += b8[i]*128;
				buf[1][i] += b8[i]*128;
			}
		}

		HAL_Sound_Proc16(buf[0],buf[1],736);
	}

	{
		u32 key = HAL_Input(0,HW_NGP);
        ram[0x6F82] = (u8)key;
	    return (key&(1<<31));
	}
}

static int NGP_EXIT(void)
{
	if(ngp_c) { HAL_mem_free(ngp_c); ngp_c=0; }
	if(ngp_m) { HAL_mem_free(ngp_m); ngp_m=0; }

    return 1;
}

static int NGP_RESET(void)
{

	return 1;
}

int NGP_Setup(void)
{
    HAL_SetupExt(EXT_NGP,"ngp",NGP_INIT, NGP_LOOP, NGP_EXIT, NGP_RESET,0,0);
    HAL_SetupExt(EXT_NPC,"ngc",NPC_INIT, NGP_LOOP, NGP_EXIT, NGP_RESET,0,0);
    HAL_SetupExt(EXT_NPC,"npc",NPC_INIT, NGP_LOOP, NGP_EXIT, NGP_RESET,0,0);
   return 1;
}

void system_message()
{
}

int system_comms_read(u8* buffer)
{
	return 0;
}

int system_comms_poll(u8* buffer)
{
	return 0;
}

void system_comms_write(u8 data)
{

}
