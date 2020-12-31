#define SYSTEM_CPP

#include "hal.h"
#include "system.h"
#include "lynxdef.h"
#include "hal.h"
#include "cstring.h"

static int lnx_scanline;

FBFORMAT fb_format;
static u8 nMemMap = 0;
static u8 pLynxBios[0x200];
static int nLynxBios=0;
u8	mRam_Data[RAM_SIZE]; // 0x0c000

#define RAM_PEEK(m)						(mRam_Data[(m)])
#define RAM_PEEKW(m)					(mRam_Data[(m)]+(mRam_Data[(m)+1]<<8))
#define RAM_CSusie_Poke(m1,m2)			{mRam_Data[(m1)]=(m2);}

static void CSystem_Reset(void);
static u32  CMikie_GetLfsrNext(u32 current);

static int  CCart_Init(u8 *gamedata,u32 gamesize);
static void CCart_Poke(u32 addr,u8 data);
static u8   CCart_Peek(u32 addr);
static void CCart_CartAddressStrobe(u32 strobe);
static void CSusie_Poke(u32 addr,u8 data);
static void CMikie_Update(void);

St_MIKIE mikie;

MEMORY_HANDLER mMemoryHandlers[SYSTEM_SIZE];

static		PIXEL_FORMAT mColourMap[4096];
static		u32		mAudioInputComparator;
static		u32		mTimerStatusFlags;
static		u32		mTimerInterruptMask;
static		u16          mPalette[16];

//
// Serial related variables
//
static		u32		mUART_TX_IRQ_ENABLE;
static		u32		mUART_TX_COUNTDOWN;

static		u32		mUART_SENDBREAK;
static		u32		mUART_TX_DATA;
static		u32		mUART_PARITY_ENABLE;
static		u32		mUART_PARITY_EVEN;
static		int		mUART_CABLE_PRESENT;

static		PIXEL_FORMAT *mpDisplayBits, *mpDisplayCurrent;

static		u32		mLynxLine;
static		u32		mLynxLineDMACounter;
static		u32		mLynxAddr;

//	public:
static u32 CCart_mWriteEnableBank[2];
static u32 CCart_mCartRAM;

//	private:
static EMMODE	mBank;
static u32  mMaskBank[2];
static u8   mCartBank[2][0x80000];
static u32  mShiftCount[2];
static u32  mCountMask[2];
static u32  mCounter;
static u32  mShifter;
static u32  mAddrData;
static u32  mStrobe;

static SUSIE_REGISTER suzy;
static SUSIE Susie;
static MIKIE Mikie;

static void  CSusie_DoMathDivide(void);
static void  CSusie_DoMathMultiply(void);
static u32   CSusie_LineInit(u32 voff);
static u32   CSusie_LineGetPixel(void);
static u32   CSusie_LineGetBits(u32 bits);

//static void  CSusie_ProcessPixel(u32 hoff,u32 pixel);
static void  CSusie_WritePixel(u32 hoff,u32 pixel);
static u32   CSusie_ReadPixel(u32 hoff);
static void  CSusie_WriteCollision(u32 hoff,u32 pixel);
static u32   CSusie_ReadCollision(u32 hoff);
static void  CSusie_Reset(void);

static void ProcessPixel_background_shadow(u32 hoff,u32 pixel);
static void ProcessPixel_noncollide(u32 hoff,u32 pixel);
static void ProcessPixel_boundary(u32 hoff,u32 pixel);
static void ProcessPixel_normal(u32 hoff,u32 pixel);
static void ProcessPixel_boundary_shadow(u32 hoff,u32 pixel);
static void ProcessPixel_shadow(u32 hoff,u32 pixel);
static void ProcessPixel_xor_shadow(u32 hoff,u32 pixel);

typedef void (*pPIXEL)(u32 hoff,u32 pixel);

static pPIXEL pProcessPixel = ProcessPixel_normal;

static pPIXEL pProcessPixelList[8]={
		ProcessPixel_background_shadow,
		CSusie_WritePixel,
		ProcessPixel_boundary_shadow,
		ProcessPixel_boundary,
		ProcessPixel_normal,
		ProcessPixel_noncollide,
		ProcessPixel_xor_shadow,
		ProcessPixel_shadow
};

//u32   CCart_CartGetRotate(void) { return mRotation;};
//int   CCart_CartHeaderLess(void) { return mHeaderLess;};

static void CSusie_Reset(void)
{
	int loop;

	// Fetch pointer to system RAM, faster than object access
	// and seeing as Susie only ever sees RAM.
	core_memset(&Susie,0,sizeof(Susie));
	core_memset(&suzy,0,sizeof(suzy));

	// Reset ALL variables
	suzy.mTMPADR = 0;
	suzy.mTILTACUM = 0;

	suzy.mHSIZOFF=0x007f;
	suzy.mVSIZOFF=0x007f;

	// Must be initialised to this due to
	// stun runner math initialisation bug
	// see whatsnew for 0.7
	Susie.mMATHABCD.Long=0xffffffff;
	Susie.mMATHEFGH.Long=0xffffffff;
	Susie.mMATHJKLM.Long=0xffffffff;

	Susie.mMATHNP.Long=0xffff;

	Susie.mMATHAB_sign=1;
	Susie.mMATHCD_sign=1;
	Susie.mMATHEFGH_sign=1;

	pProcessPixel = pProcessPixelList[0];

	suzy.mSPRCTL0 = 0;
	suzy.mSPRCTL1 = 0;

	suzy.mSUZYBUSEN=0;

	Susie.mSPRINIT.Byte=0;

	suzy.mSPRGO=0;
	Susie.mEVERON=0;

	for(loop=0;loop<16;loop++) Susie.mPenIndex[loop]=loop;
}


static void CSusie_DoMathMultiply(void)
{
	u32 result;

	Susie.mSPRSYS_Mathbit=0;

	// Multiplies with out sign or accumulate take 44 ticks to complete.
	// Multiplies with sign and accumulate take 54 ticks to complete. 
	//
	//    AB                                    EFGH
	//  * CD                                  /   NP
	// -------                            -----------
	//  EFGH                                    ABCD
	// Accumulate in JKLM         Remainder in (JK)LM
	//
	// Basic multiply is ALWAYS unsigned, sign conversion is done later
	result=(u32)Susie.mMATHABCD.Words.AB*(u32)Susie.mMATHABCD.Words.CD;
	Susie.mMATHEFGH.Long=result;

	if(Susie.mSPRSYS_SignedMath) {
        Susie.mMATHEFGH_sign=Susie.mMATHAB_sign+Susie.mMATHCD_sign;
		if(!Susie.mMATHEFGH_sign) {
			Susie.mMATHEFGH.Long^=0xffffffff;
			Susie.mMATHEFGH.Long++;
		}
	} else {
		////TRACE_SUSIE0("CSusie_DoMathMultiply() - UNSIGNED");
	}

	////TRACE_SUSIE2("CSusie_DoMathMultiply() AB=$%04x * CD=$%04x",Susie.mMATHABCD.Words.AB,Susie.mMATHABCD.Words.CD);

	// Check overflow, if B31 has changed from 1->0 then its overflow time
	if(Susie.mSPRSYS_Accumulate) {
		u32 tmp=Susie.mMATHJKLM.Long+Susie.mMATHEFGH.Long;
		// Let sign change indicate overflow
		if((tmp&0x80000000)!=(Susie.mMATHJKLM.Long&0x80000000)) {
			////TRACE_SUSIE0("CSusie_DoMathMultiply() - OVERFLOW DETECTED");
//			Susie.mSPRSYS_Mathbit=1;
		} else {
//			Susie.mSPRSYS_Mathbit=0;
		}
		// Save accumulated result
		Susie.mMATHJKLM.Long=tmp;
	}
}

static void CSusie_DoMathDivide(void)
{
	Susie.mSPRSYS_Mathbit=0;

	//
	// Divides take 176 + 14*N ticks
	// (N is the number of most significant zeros in the divisor.)
	//
	//    AB                                    EFGH
	//  * CD                                  /   NP
	// -------                            -----------
	//  EFGH                                    ABCD
	// Accumulate in JKLM         Remainder in (JK)LM
	//
	// Divide is ALWAYS unsigned arithmetic...
	if(Susie.mMATHNP.Long) {
		Susie.mMATHABCD.Long=Susie.mMATHEFGH.Long/Susie.mMATHNP.Long;
		Susie.mMATHJKLM.Long=Susie.mMATHEFGH.Long%Susie.mMATHNP.Long;
	} else {
		Susie.mMATHABCD.Long=0xffffffff;
		Susie.mMATHJKLM.Long=0;
		Susie.mSPRSYS_Mathbit=1;
	}
}


static u32 CSusie_PaintSprites(void)
{
	int loop,sprcount=0,data=0,everonscreen=0;

	if(!suzy.mSUZYBUSEN || !suzy.mSPRGO){
		return 0;
	}

	everonscreen=0;

	do {
		if(!(suzy.mSCBNEXT&0xff00)) {
			////TRACE_SUSIE0("PaintSprites() suzy.mSCBNEXT==0 - FINISHED");
			Susie.mSPRSYS_Status=0;	// Engine has finished
			suzy.mSPRGO=0;
			break;
		} else {
			Susie.mSPRSYS_Status=1;
		}

		suzy.mTMPADR=suzy.mSCBNEXT;	// Copy SCB pointer
		suzy.mSCBADR=suzy.mSCBNEXT;	// Copy SCB pointer
		////TRACE_SUSIE1("PaintSprites() SCBADDR $%04x",suzy.mSCBADR);

		suzy.mSPRCTL0 = RAM_PEEK(suzy.mTMPADR);			// Fetch control 0
	
		suzy.mTMPADR+=1;
		pProcessPixel = pProcessPixelList[mSPRCTL0_Type];

		suzy.mSPRCTL1 = RAM_PEEK(suzy.mTMPADR);			// Fetch control 1
		suzy.mTMPADR+=1;

		data=RAM_PEEK(suzy.mTMPADR);			// Collision num
		////TRACE_SUSIE1("PaintSprites() SPRCOLL $%02x",data);
		Susie.mSPRCOLL_Number=data&0x000f;
		Susie.mSPRCOLL_Collide=data&0x0020;
		suzy.mTMPADR+=1;

		suzy.mSCBNEXT=RAM_PEEKW(suzy.mTMPADR);	// Next SCB
		////TRACE_SUSIE1("PaintSprites() SCBNEXT $%04x",suzy.mSCBNEXT);
		suzy.mTMPADR+=2;

		// Initialise the collision depositary

// Although Tom Schenck says this is correct, it doesnt appear to be
//		if(!Susie.mSPRCOLL_Collide && !Susie.mSPRSYS_NoCollide)
//		{
//			Susie.mCollision=RAM_PEEK((suzy.mSCBADR+suzy.mCOLLOFF)&0xffff);
//			Susie.mCollision&=0x0f;
//		}
		Susie.mCollision=0;

		// Check if this is a skip sprite

		if(!mSPRCTL1_SkipSprite) {
            u32 enable_sizing=0;
            u32 enable_stretch=0;
            u32 enable_tilt=0;
            int screen_h_start;
            int screen_h_end;
            int screen_v_start;
            int screen_v_end;
            u32 superclip;
            int quadrant;
            int hsign,vsign;
            int world_h_mid;
            int world_v_mid;

            suzy.mSPRDLINE=RAM_PEEKW(suzy.mTMPADR);	suzy.mTMPADR+=2; // Sprite pack data
            suzy.mHPOSSTRT=RAM_PEEKW(suzy.mTMPADR);	suzy.mTMPADR+=2; // Sprite horizontal start position
            suzy.mVPOSSTRT=RAM_PEEKW(suzy.mTMPADR);	suzy.mTMPADR+=2; // Sprite vertical start position

            // Optional section defined by reload type in Control 1

            switch(mSPRCTL1_ReloadDepth) {
              case 1:
                ////TRACE_SUSIE0("PaintSprites() Sizing Enabled");
                enable_sizing=1;
                suzy.mSPRHSIZ=RAM_PEEKW(suzy.mTMPADR); suzy.mTMPADR+=2; // Sprite Horizontal size
                suzy.mSPRVSIZ=RAM_PEEKW(suzy.mTMPADR); suzy.mTMPADR+=2; // Sprite Verticalal size
                break;

              case 2:
                enable_sizing=1;
                enable_stretch=1;
                suzy.mSPRHSIZ=RAM_PEEKW(suzy.mTMPADR); suzy.mTMPADR+=2;	// Sprite Horizontal size
                suzy.mSPRVSIZ=RAM_PEEKW(suzy.mTMPADR); suzy.mTMPADR+=2;	// Sprite Verticalal size
                suzy.mSTRETCH=RAM_PEEKW(suzy.mTMPADR); suzy.mTMPADR+=2;	// Sprite stretch
                break;

              case 3:
                enable_sizing=1;
                enable_stretch=1;
                enable_tilt=1;

                suzy.mSPRHSIZ=RAM_PEEKW(suzy.mTMPADR); suzy.mTMPADR+=2; // Sprite Horizontal size
                suzy.mSPRVSIZ=RAM_PEEKW(suzy.mTMPADR); suzy.mTMPADR+=2; // Sprite Verticalal size
                suzy.mSTRETCH=RAM_PEEKW(suzy.mTMPADR); suzy.mTMPADR+=2; // Sprite stretch
                suzy.mTILT=RAM_PEEKW(suzy.mTMPADR);    suzy.mTMPADR+=2; // Sprite tilt
                break;

              default:
                break;
            }
            

			// Optional Palette reload
            if(!mSPRCTL1_ReloadPalette) {
                int loop;

                ////TRACE_SUSIE0("PaintSprites() Palette reloaded");
                for(loop=0;loop<8;loop++){
					u8 data=RAM_PEEK(suzy.mTMPADR++);
					Susie.mPenIndex[loop*2]=(data>>4)&0x0f;
					Susie.mPenIndex[(loop*2)+1]=data&0x0f;
				}
                // Increment cycle count for the reads
            }

            // Now we can start painting

            // Quadrant drawing order is: SE,NE,NW,SW
            // start quadrant is given by sprite_control1:0 & 1

            // Setup screen start end variables
            screen_h_start=(s16)suzy.mHOFF;
            screen_h_end=(s16)suzy.mHOFF+SCREEN_WIDTH;
            screen_v_start=(s16)suzy.mVOFF;
            screen_v_end=(s16)suzy.mVOFF+SCREEN_HEIGHT;

            world_h_mid=screen_h_start+0x8000+(SCREEN_WIDTH/2);
            world_v_mid=screen_v_start+0x8000+(SCREEN_HEIGHT/2);

			superclip=0;
            quadrant=0;
            hsign,vsign;

            if(mSPRCTL1_StartLeft) {
                if(mSPRCTL1_StartUp) quadrant=2; else quadrant=3;
            } else {
                if(mSPRCTL1_StartUp) quadrant=1; else quadrant=0;
            }
            ////TRACE_SUSIE1("PaintSprites() Quadrant=%d",quadrant);

            // Check ref is inside screen area

            if((s16)suzy.mHPOSSTRT<screen_h_start || (s16)suzy.mHPOSSTRT>=screen_h_end ||
               (s16)suzy.mVPOSSTRT<screen_v_start || (s16)suzy.mVPOSSTRT>=screen_v_end) superclip=1;

            ////TRACE_SUSIE1("PaintSprites() Superclip=%d",superclip);


            // Quadrant mapping is:	SE	NE	NW	SW
            //						0	1	2	3
            // hsign				+1	+1	-1	-1
            // vsign				+1	-1	-1	+1
            //
            //
            //		2 | 1
            //     -------
            //      3 | 0
            //

            // Loop for 4 quadrants
            for(loop=0;loop<4;loop++) {
                int sprite_v=suzy.mVPOSSTRT;
                int sprite_h=suzy.mHPOSSTRT;

                u32 render=0;
                static int pixel_height=0;
                static int pixel_width=0;
                static int pixel=0;
                static int hoff=0,voff=0;
                static int hloop=0,vloop=0;
                static u32 onscreen=0;
                static int vquadoff=0;
                static int hquadoff=0;

                // Set quadrand multipliers
                hsign=(quadrant==0 || quadrant==1)?1:-1;
                vsign=(quadrant==0 || quadrant==3)?1:-1;

                // Preflip		//TRACE_SUSIE2("PaintSprites() hsign=%d vsign=%d",hsign,vsign);

                //Use h/v flip to invert v/hsign

                if(mSPRCTL0_Vflip) vsign=-vsign;
                if(mSPRCTL0_Hflip) hsign=-hsign;

                // Two different rendering algorithms used, on-screen & superclip
                // when on screen we draw in x until off screen then skip to next
                // line, BUT on superclip we draw all the way to the end of any
                // given line checking each pixel is on screen.

                if(superclip) {
                    // Check on the basis of each quad, we only render the quad
                    // IF the screen is in the quad, relative to the centre of
                    // the screen which is calculated below.

                    // Quadrant mapping is:	SE	NE	NW	SW
                    //						0	1	2	3
                    // hsign				+1	+1	-1	-1
                    // vsign				+1	-1	-1	+1
                    //
                    //
                    //		2 | 1
                    //     -------
                    //      3 | 0
                    //
                    // Quadrant mapping for superclipping must also take into account
                    // the hflip, vflip bits & negative tilt to be able to work correctly
                    //
                    int	modquad=quadrant;
                    static int vquadflip[4]={1,0,3,2};
                    static int hquadflip[4]={3,2,1,0};

                    if(mSPRCTL0_Vflip) modquad=vquadflip[modquad];
                    if(mSPRCTL0_Hflip) modquad=hquadflip[modquad];

                    // This is causing Eurosoccer to fail!!
                    //					if(enable_tilt && suzy.mTILT&0x8000) modquad=hquadflip[modquad];

                    switch(modquad) {
                     case 3:  if((sprite_h>=screen_h_start || sprite_h<world_h_mid) && (sprite_v<screen_v_end    || sprite_v>world_v_mid)) render=1;  break;
                     case 2:  if((sprite_h>=screen_h_start || sprite_h<world_h_mid) && (sprite_v>=screen_v_start || sprite_v<world_v_mid)) render=1;  break;
                     case 1:  if((sprite_h<screen_h_end    || sprite_h>world_h_mid) && (sprite_v>=screen_v_start || sprite_v<world_v_mid)) render=1;  break;
                     default: if((sprite_h<screen_h_end    || sprite_h>world_h_mid) && (sprite_v<screen_v_end    || sprite_v>world_v_mid)) render=1;  break;
                    }
                }
                else {
                    render=1;
                }

                // Is this quad to be rendered ??

                if(render) {
                    // Set the vertical position & offset
                    voff=(s16)suzy.mVPOSSTRT-screen_v_start;

                    // Zero the stretch,tilt & acum values
                    suzy.mTILTACUM=0;

                    // Perform the SIZOFF
                    if(vsign==1) suzy.mVSIZACUM=suzy.mVSIZOFF; else suzy.mVSIZACUM=0;

                    // Take the sign of the first quad (0) as the basic
                    // sign, all other quads drawing in the other direction
                    // get offset by 1 pixel in the other direction, this
                    // fixes the squashed look on the multi-quad sprites.
                    //					if(vsign==-1 && loop>0) voff+=vsign;
                    if(loop==0)	vquadoff=vsign;
                    if(vsign!=vquadoff) voff+=vsign;

                    for(;;) {
                        // Vertical scaling is done here
                        suzy.mVSIZACUM+=suzy.mSPRVSIZ;
                        pixel_height=suzy.mVSIZACUM_H;
                        suzy.mVSIZACUM_H=0;

                        // Update the next data line pointer and initialise our line
                        suzy.mSPRDOFF=(u16)CSusie_LineInit(0);

                        // If 1 == next quad, ==0 end of sprite, anyways its END OF LINE
                        if(suzy.mSPRDOFF==1) {	// End of quad
                            suzy.mSPRDLINE+=suzy.mSPRDOFF;
                            break;
                        }

                        if(suzy.mSPRDOFF==0) {	// End of sprite
                            loop=4;		// Halt the quad loop
                            break;
                        }

                        // Draw one horizontal line of the sprite
                        for(vloop=0;vloop<pixel_height;vloop++) {
                            // Early bailout if the sprite has moved off screen, terminate quad
                            if(vsign==1 && voff>=SCREEN_HEIGHT)	break;
                            if(vsign==-1 && voff<0)	break;

                            // Only allow the draw to take place if the line is visible
                            if(voff>=0 && voff<SCREEN_HEIGHT) {
                                // Work out the horizontal pixel start position, start + tilt
                                suzy.mHPOSSTRT+=((s16)suzy.mTILTACUM>>8);
                                suzy.mTILTACUM_H=0;
                                hoff=(int)((s16)suzy.mHPOSSTRT)-screen_h_start;

                                // Zero/Force the horizontal scaling accumulator
                                if(hsign==1) Susie.mHSIZACUM.Word=suzy.mHSIZOFF; else Susie.mHSIZACUM.Word=0;

                                // Take the sign of the first quad (0) as the basic
                                // sign, all other quads drawing in the other direction
                                // get offset by 1 pixel in the other direction, this
                                // fixes the squashed look on the multi-quad sprites.
                                //								if(hsign==-1 && loop>0) hoff+=hsign;
                                if(loop==0)	hquadoff=hsign;
                                if(hsign!=hquadoff) hoff+=hsign;

                                // Initialise our line
                                CSusie_LineInit(voff);
                                onscreen=0;

                                // Now render an individual destination line
                                while((pixel=CSusie_LineGetPixel())!=LINE_END) {
                                    // This is allowed to update every pixel
                                    Susie.mHSIZACUM.Word+=suzy.mSPRHSIZ;
                                    pixel_width=Susie.mHSIZACUM.Byte.High;
                                    Susie.mHSIZACUM.Byte.High=0;

                                    for(hloop=0;hloop<pixel_width;hloop++) {
                                        // Draw if onscreen but break loop on transition to offscreen
                                        if(hoff>=0 && hoff<SCREEN_WIDTH)  {

											pProcessPixel(hoff,pixel);

                                            //CSusie_ProcessPixel(hoff,pixel);
                                            
											onscreen=everonscreen=1;
                                        }
                                        else {
                                            if(onscreen) break;
                                        }
                                        hoff+=hsign;
                                    }
                                }
                            }
                            voff+=vsign;

                            // For every destination line we can modify SPRHSIZ & SPRVSIZ & TILTACUM
                            if(enable_stretch) {
                                suzy.mSPRHSIZ+=suzy.mSTRETCH;
                                //if(Susie.mSPRSYS_VStretch) suzy.mSPRVSIZ+=suzy.mSTRETCH;
                            }
                            if(enable_tilt) {
                                // Manipulate the tilt stuff
                                suzy.mTILTACUM+=suzy.mTILT;
                            }
                        }
                        // According to the docs this increments per dest line
                        // but only gets set when the source line is read
                        if(Susie.mSPRSYS_VStretch) suzy.mSPRVSIZ+=suzy.mSTRETCH*pixel_height;

                        // Update the line start for our next run thru the loop
                        suzy.mSPRDLINE+=suzy.mSPRDOFF;
                    }
                }
                else {
                    // Skip thru data to next quad
                    for(;;) {
                        // Read the start of line offset
                        suzy.mSPRDOFF=(u16)CSusie_LineInit(0);
                        // We dont want to process data so suzy.mSPRDLINE is useless to us
                        suzy.mSPRDLINE+=suzy.mSPRDOFF;

                        // If 1 == next quad, ==0 end of sprite, anyways its END OF LINE
                        if(suzy.mSPRDOFF==1) break;	// End of quad
                        if(suzy.mSPRDOFF==0) {	// End of sprite
                            loop=4;		// Halt the quad loop
                            break;
                        }
                    }
                }

                // Increment quadrant and mask to 2 bit value (0-3)
                quadrant++;
                quadrant&=0x03;
            }

            // Write the collision depositary if required

            if(!Susie.mSPRCOLL_Collide && !Susie.mSPRSYS_NoCollide)  {
                switch(mSPRCTL0_Type) {
                  case sprite_xor_shadow:
                  case sprite_boundary:
                  case sprite_normal:
                  case sprite_boundary_shadow:
                  case sprite_shadow: {
                      u16 coldep=suzy.mSCBADR+suzy.mCOLLOFF;
                      RAM_CSusie_Poke(coldep,(u8)Susie.mCollision);
                  }
                    break;
                  default:
                    break;
                }
            }

            if(Susie.mEVERON)  {
                u16 coldep=suzy.mSCBADR+suzy.mCOLLOFF;
                u8 coldat=RAM_PEEK(coldep);
                if(!everonscreen) coldat|=0x80; else coldat&=0x7f;
                RAM_CSusie_Poke(coldep,coldat);
                ////TRACE_SUSIE0("PaintSprites() EVERON IS ACTIVE");
                ////TRACE_SUSIE2("PaintSprites() Wrote $%02x to SCB collision depositary at $%04x",coldat,coldep);
            }
        }

		// Increase sprite number
		sprcount++;

		// Check if we abort after 1st sprite is complete

//		if(suzy.mSPRSYS.Read.StopOnCurrent) {
//			suzy.mSPRSYS.Read.Status=0;	// Engine has finished
//			suzy.mSPRGO=0;
//			break;
//		}

		// Check sprcount for looping SCB, random large number chosen
		if(sprcount>4096) {
			// Stop the system, otherwise we may just come straight back in.....
			return 0;
		}
	}
	while(1);

	// Fudge factor to fix many flickering issues, also the keypress
	// problem with Hard Drivin and the strange pause in Dirty Larry.

	return 0;
}

//
// Collision code modified by KW 22/11/98
// Collision buffer cler added if there is no
// apparent collision, I have a gut feeling this
// is the wrong solution to the inv07.com bug but
// it seems to work OK.
//
// Shadow-------------------------------+
// Boundary-Shadow--------------------+ |
// Normal---------------------------+ | |
// Boundary-----------------------+ | | |
// Background-Shadow------------+ | | | |
// Background-No Collision----+ | | | | |
// Non-Collideable----------+ | | | | | |
// Exclusive-or-Shadow----+ | | | | | | |
//                        | | | | | | | |
//                        1 1 1 1 0 1 0 1   F is opaque 
//                        0 0 0 0 1 1 0 0   E is collideable 
//                        0 0 1 1 0 0 0 0   0 is opaque and collideable 
//                        1 0 0 0 1 1 1 1   allow collision detect 
//                        1 0 0 1 1 1 1 1   allow coll. buffer access 
//                        1 0 0 0 0 0 0 0   exclusive-or the data 
//

static void ProcessPixel_background_shadow(u32 hoff,u32 pixel)
{
	CSusie_WritePixel(hoff,pixel);

	if(!Susie.mSPRCOLL_Collide && !Susie.mSPRSYS_NoCollide && pixel!=0x0e) {
		CSusie_WriteCollision(hoff,Susie.mSPRCOLL_Number);
	}
}

static void ProcessPixel_noncollide(u32 hoff,u32 pixel)
{
	if(pixel) CSusie_WritePixel(hoff,pixel);
}

static void ProcessPixel_boundary(u32 hoff,u32 pixel)
{
	if(pixel!=0x00 && pixel!=0x0f){
		CSusie_WritePixel(hoff,pixel);
	}
       
	if(pixel!=0x00) {
		if(!Susie.mSPRCOLL_Collide && !Susie.mSPRSYS_NoCollide) {
			int collision=CSusie_ReadCollision(hoff);
			if(collision>Susie.mCollision) {
				Susie.mCollision=collision;
			}
			CSusie_WriteCollision(hoff,Susie.mSPRCOLL_Number);
		}
	}
}

static void ProcessPixel_normal(u32 hoff,u32 pixel)
{
	if(pixel!=0x00){
		CSusie_WritePixel(hoff,pixel);
		if(!Susie.mSPRCOLL_Collide && !Susie.mSPRSYS_NoCollide){
			int collision=CSusie_ReadCollision(hoff);
			if(collision>Susie.mCollision){
				Susie.mCollision=collision;
			}
			CSusie_WriteCollision(hoff,Susie.mSPRCOLL_Number);
		}
	}
}

static void ProcessPixel_boundary_shadow(u32 hoff,u32 pixel)
{
	if(pixel!=0x00 && pixel!=0x0e && pixel!=0x0f){
		CSusie_WritePixel(hoff,pixel);
	}
	if(pixel!=0x00 && pixel!=0x0e){
		if(!Susie.mSPRCOLL_Collide && !Susie.mSPRSYS_NoCollide){
			int collision=CSusie_ReadCollision(hoff);
			if(collision>Susie.mCollision){
				Susie.mCollision=collision;
			}
			CSusie_WriteCollision(hoff,Susie.mSPRCOLL_Number);
		}
	}
}

static void ProcessPixel_shadow(u32 hoff,u32 pixel)
{
	if(pixel!=0x00){
		CSusie_WritePixel(hoff,pixel);
	}
	if(pixel!=0x00 && pixel!=0x0e){
		if(!Susie.mSPRCOLL_Collide && !Susie.mSPRSYS_NoCollide){
			int collision=CSusie_ReadCollision(hoff);
			if(collision>Susie.mCollision){
				Susie.mCollision=collision;
			}
			CSusie_WriteCollision(hoff,Susie.mSPRCOLL_Number);
		}
	}
}

static void ProcessPixel_xor_shadow(u32 hoff,u32 pixel)
{
	if(pixel!=0x00){
		CSusie_WritePixel(hoff,CSusie_ReadPixel(hoff)^pixel);
	}
	if(pixel!=0x00 && pixel!=0x0e){
		if(!Susie.mSPRCOLL_Collide && !Susie.mSPRSYS_NoCollide && pixel!=0x0e){
			int collision=CSusie_ReadCollision(hoff);
			if(collision>Susie.mCollision){
				Susie.mCollision=collision;
			}
			CSusie_WriteCollision(hoff,Susie.mSPRCOLL_Number);
		}
	}
}

static void CSusie_WritePixel(u32 hoff,u32 pixel)
{
	u32 scr_addr=Susie.mLineBaseAddress+(hoff/2);
	u8 dest=RAM_PEEK(scr_addr);

	if(!(hoff&0x01)) { dest = (dest&0x0f) | (pixel<<4); } 
	else             { dest = (dest&0xf0) |  pixel;     }

	RAM_CSusie_Poke(scr_addr,dest);
	// Increment cycle count for the read/modify/write
}

static void CSusie_WriteCollision(u32 hoff,u32 pixel)
{
	u32 col_addr=Susie.mLineCollisionAddress+(hoff/2);
	u8 dest=RAM_PEEK(col_addr);

	if(!(hoff&0x01)) {  dest = (dest&0x0f) | (pixel<<4); }
	else             {  dest = (dest&0xf0) |  pixel;     }

	RAM_CSusie_Poke(col_addr,dest);
	// Increment cycle count for the read/modify/write
}

static u32 CSusie_ReadPixel(u32 hoff)
{
	u32 scr_addr=Susie.mLineBaseAddress+(hoff/2);
	u32 data=RAM_PEEK(scr_addr);
	if(!(hoff&0x01)) {  data>>=4;   } 
	else             {  data&=0x0f; }
	// Increment cycle count for the read/modify/write
	return data;
}


static u32 CSusie_ReadCollision(u32 hoff)
{
	u32 col_addr=Susie.mLineCollisionAddress+(hoff/2);
	u32 data=RAM_PEEK(col_addr);
	if(!(hoff&0x01)) { data>>=4;   }
	else             { data&=0x0f; }
	// Increment cycle count for the read/modify/write
	return data;
}

static u32 CSusie_LineInit(u32 voff)
{
	u32 offset;

	Susie.mLineShiftReg=0;
	Susie.mLineShiftRegCount=0;
	Susie.mLineRepeatCount=0;
	Susie.mLinePixel=0;
	Susie.mLineType=line_error;
	Susie.mLinePacketBitsLeft=0xffff;

	// Initialise the temporary pointer
	suzy.mTMPADR = suzy.mSPRDLINE;

	// First read the Offset to the next line
	offset=CSusie_LineGetBits(8);

	// Specify the MAXIMUM number of bits in this packet, it
	// can terminate early but can never use more than this
	// without ending the current packet, we count down in CSusie_LineGetBits()
	Susie.mLinePacketBitsLeft=(offset-1)*8;

	// Literals are a special case and get their count set on a line basis
	if(mSPRCTL1_Literal){
		Susie.mLineType=line_abs_literal;
//		Susie.mLineRepeatCount=((offset-1)*8)//*Susie.*/mSPRCTL0_PixelBits;
		Susie.mLineRepeatCount=((offset-1)*8)/mSPRCTL0_PixelBits;
		// Why is this necessary, is this compensating for the 1,1 offset bug
//		Susie.mLineRepeatCount--;
	}

	// Set the line base address for use in the calls to pixel painting
	if(voff>101) {
		voff=0;
	}

	Susie.mLineBaseAddress=suzy.mVIDBAS+(voff*(SCREEN_WIDTH/2));
	Susie.mLineCollisionAddress=suzy.mCOLLBAS+(voff*(SCREEN_WIDTH/2));

	// Return the offset to the next line

	return offset;
}

static u32 CSusie_LineGetPixel()
{
    if(!Susie.mLineRepeatCount) {
        // Normal sprites fetch their counts on a packet basis
        if(Susie.mLineType!=line_abs_literal) {
            u32 literal=CSusie_LineGetBits(1);
            if(literal) Susie.mLineType=line_literal; else Susie.mLineType=line_packed;
        }

        // Pixel store is empty what should we do
        switch(Susie.mLineType)	{
          case line_abs_literal:
            // This means end of line for us
            Susie.mLinePixel=LINE_END;
            return Susie.mLinePixel;		// SPEEDUP
            break;
          case line_literal:
            Susie.mLineRepeatCount=CSusie_LineGetBits(4);
            Susie.mLineRepeatCount++;
            break;
          case line_packed:
            // From reading in between the lines only a packed line with
            // a zero size i.e 0b00000 as a header is allowable as a packet end
            Susie.mLineRepeatCount=CSusie_LineGetBits(4);
            if(!Susie.mLineRepeatCount)  {
                Susie.mLinePixel=LINE_END;
            } else {
                Susie.mLinePixel=Susie.mPenIndex[CSusie_LineGetBits(mSPRCTL0_PixelBits)];
            }
            Susie.mLineRepeatCount++;
            break;
          default:
            return 0;
        }
    }

    if(Susie.mLinePixel!=LINE_END) {
        Susie.mLineRepeatCount--;

        switch(Susie.mLineType) {
          case line_abs_literal:
            Susie.mLinePixel=CSusie_LineGetBits(mSPRCTL0_PixelBits);
            // Check the special case of a zero in the last pixel
            if(!Susie.mLineRepeatCount && !Susie.mLinePixel) Susie.mLinePixel=LINE_END;
            else                                             Susie.mLinePixel=Susie.mPenIndex[Susie.mLinePixel];
            break;
          case line_literal:
            Susie.mLinePixel=Susie.mPenIndex[CSusie_LineGetBits(mSPRCTL0_PixelBits)];
            break;
          case line_packed:
            break;
          default:
            return 0;
        }
    }

    return Susie.mLinePixel;
}

static u32 CSusie_LineGetBits(u32 bits)
{
	u32 retval;

	// Sanity, not really needed
	// if(bits>32) return 0;
	// Only return data IF there is enought bits left in the packet

	if(Susie.mLinePacketBitsLeft<bits) return 0;

	// Make sure shift reg has enough bits to fulfil the request

	if(Susie.mLineShiftRegCount<bits) {
// This assumes data comes into LSB and out of MSB
//		Susie.mLineShiftReg&=0x000000ff;	// Has no effect
        Susie.mLineShiftReg<<=24;
        Susie.mLineShiftReg|=RAM_PEEK(suzy.mTMPADR++)<<16;
        Susie.mLineShiftReg|=RAM_PEEK(suzy.mTMPADR++)<<8;
        Susie.mLineShiftReg|=RAM_PEEK(suzy.mTMPADR++);

        Susie.mLineShiftRegCount+=24;
    }

	// Extract the return value
	retval=Susie.mLineShiftReg>>(Susie.mLineShiftRegCount-bits);
	retval&=(1<<bits)-1;

	// Update internal vars;
	Susie.mLineShiftRegCount-=bits;
	Susie.mLinePacketBitsLeft-=bits;

	return retval;
}

static void CCart_Poke_bnk(u8 bnk,u8 data)
{
	if(CCart_mWriteEnableBank[bnk]) {
		u32 address=(mShifter<<mShiftCount[bnk])+(mCounter&mCountMask[bnk]);
		mCartBank[bnk][address&mMaskBank[bnk]]=data;		
	}
	
	if(!mStrobe) {
		mCounter = (mCounter+1)&0x07ff;
	}
}

static u8 CCart_Peek_bnk(u8 bnk)
{
	u32 address=(mShifter<<mShiftCount[bnk])+(mCounter&mCountMask[bnk]);
	u8 data=mCartBank[bnk][address&mMaskBank[bnk]];		

	if(!mStrobe){
		mCounter = (mCounter+1)&0x07ff;
	}

	return data;
}

static void CSusie_Poke(u32 addr,u8 data)
{
    suzy.data[addr&0xff]=data;

    switch(addr&0xff) {
	case (RCART0&0xff): CCart_Poke_bnk(0,data); break;
	case (RCART1&0xff): CCart_Poke_bnk(1,data); break;

	case (MATHD&0xff):
		Susie.mMATHABCD.Bytes.D=data;
		CSusie_Poke(MATHC,0);
		break;

	case (MATHC&0xff):
		////TRACE_SUSIE2("CSusie_Poke(MATHC,%02x) at PC=$%04x",data,CSystem_suzy.mCpu->GetPC());
		Susie.mMATHABCD.Bytes.C=data;
		// Perform sign conversion if required
		if(Susie.mSPRSYS_SignedMath){
			// Account for the math bug that 0x8000 is +ve & 0x0000 is -ve by subracting 1
			if((Susie.mMATHABCD.Words.CD-1)&0x8000){
				u16 conv;
				conv=Susie.mMATHABCD.Words.CD^0xffff;
				conv++;
				Susie.mMATHCD_sign=-1;
				////TRACE_SUSIE2("MATH CD signed conversion complete %04x to %04x",Susie.mMATHABCD.Words.CD,conv);
				Susie.mMATHABCD.Words.CD=conv;
			}
			else{
				Susie.mMATHCD_sign=1;
			}
		}
		break;

	case (MATHB&0xff):
		Susie.mMATHABCD.Bytes.B=data;
		Susie.mMATHABCD.Bytes.A=0;
		break;
	case (MATHA&0xff):
		////TRACE_SUSIE2("CSusie_Poke(MATHA,%02x) at PC=$%04x",data,CSystem_suzy.mCpu->GetPC());
		Susie.mMATHABCD.Bytes.A=data;
		// Perform sign conversion if required
		if(Susie.mSPRSYS_SignedMath) {
			// Account for the math bug that 0x8000 is +ve & 0x0000 is -ve by subracting 1
			if((Susie.mMATHABCD.Words.AB-1)&0x8000) {
				u16 conv=Susie.mMATHABCD.Words.AB^0xffff;
				conv++;
				Susie.mMATHAB_sign=-1;
				////TRACE_SUSIE2("MATH AB signed conversion complete %04x to %04x",Susie.mMATHABCD.Words.AB,conv);
				Susie.mMATHABCD.Words.AB=conv;
			} else {
				Susie.mMATHAB_sign=1;
			}
		}
		CSusie_DoMathMultiply();
		break;

	case (MATHP&0xff):  Susie.mMATHNP.Bytes.P=data;   Susie.mMATHNP.Bytes.N=0;   break;
	case (MATHN&0xff):  Susie.mMATHNP.Bytes.N=data;                              break;
	case (MATHH&0xff):  Susie.mMATHEFGH.Bytes.H=data; Susie.mMATHEFGH.Bytes.G=0; break;
	case (MATHG&0xff):  Susie.mMATHEFGH.Bytes.G=data;                            break;
	case (MATHF&0xff):  Susie.mMATHEFGH.Bytes.F=data; Susie.mMATHEFGH.Bytes.E=0; break;
	case (MATHE&0xff):  Susie.mMATHEFGH.Bytes.E=data; CSusie_DoMathDivide();     break;
	case (MATHM&0xff):  Susie.mMATHJKLM.Bytes.M=data; Susie.mMATHJKLM.Bytes.L=0; Susie.mSPRSYS_Mathbit=0; break;
	case (MATHL&0xff):  Susie.mMATHJKLM.Bytes.L=data;                            break;
	case (MATHK&0xff):  Susie.mMATHJKLM.Bytes.K=data; Susie.mMATHJKLM.Bytes.J=0; break;
	case (MATHJ&0xff):  Susie.mMATHJKLM.Bytes.J=data;                            break;

	case (SPRCOLL&0xff):
		Susie.mSPRCOLL_Number=data&0x000f;
		Susie.mSPRCOLL_Collide=data&0x0020;
		break;
	case (SPRINIT&0xff):   Susie.mSPRINIT.Byte=data;  break;
	case (SUZYBUSEN&0xff): suzy.mSUZYBUSEN=data&0x01; break;
	case (SPRGO&0xff):
		suzy.mSPRGO=data&0x01;
		Susie.mEVERON=data&0x04;
		////TRACE_SUSIE2("CSusie_Poke(SPRGO,%02x) at PC=$%04x",data,CSystem_suzy.mCpu->GetPC());
		break;
	case (SPRSYS&0xff):
		Susie.mSPRSYS = data;
//		Susie.mSPRSYS_StopOnCurrent=data&0x0002;
		if(data&0x0004) Susie.mSPRSYS_UnsafeAccess=0;
//		Susie.mSPRSYS_LeftHand=data&0x0008;
		Susie.mSPRSYS_VStretch=data&0x0010;
		Susie.mSPRSYS_NoCollide=data&0x0020;
		Susie.mSPRSYS_Accumulate=data&0x0040;
		Susie.mSPRSYS_SignedMath=data&0x0080;
		////TRACE_SUSIE2("CSusie_Poke(SPRSYS,%02x) at PC=$%04x",data,CSystem_suzy.mCpu->GetPC());
		break;
//	case (SPRCTL0&0xff): suzy.mSPRCTL0 = data; break;
//	case (SPRCTL1&0xff): suzy.mSPRCTL1 = data; break;
// Errors on illegal location accesses
	default:
		break;
	}
}

static u8 CSusie_Peek(u32 addr)
{
	u8 retval=0;

    switch(addr&0xff) {
    case (RCART0&0xff):    return CCart_Peek_bnk(0);
    case (RCART1&0xff):    return CCart_Peek_bnk(1);
    case (SPRSYS&0xff):
#if 0
		return Susie.mSPRSYS;
#else
        retval=0x0000;
        //	retval+=(Susie.mSPRSYS_Status)?0x0001:0x0000;
        // Use gSystesuzy.mCPUSleep to signal the status instead, if we are asleep then
        // we must be rendering sprites
//        retval+=(gSystemCPUSleep)?0x0001:0x0000;
        retval+=(/*Susie.*/mSPRSYS_StopOnCurrent)?0x0002:0x0000;
        retval+=(Susie.mSPRSYS_UnsafeAccess)?0x0004:0x0000;
        retval+=(/*Susie.*/mSPRSYS_LeftHand)?0x0008:0x0000;
        retval+=(Susie.mSPRSYS_VStretch)?0x0010:0x0000;
        retval+=(Susie.mSPRSYS_LastCarry)?0x0020:0x0000;
        retval+=(Susie.mSPRSYS_Mathbit)?0x0040:0x0000;
        retval+=(Susie.mSPRSYS_MathInProgress)?0x0080:0x0000;
        ////TRACE_SUSIE2("Peek(SPRSYS)=$%02x at PC=$%04x",retval,CSystem_suzy.mCpu->GetPC());
        return retval;
#endif

      case (MATHD&0xff):          return Susie.mMATHABCD.Bytes.D;
      case (MATHC&0xff):          return Susie.mMATHABCD.Bytes.C;
      case (MATHB&0xff):          return Susie.mMATHABCD.Bytes.B;
      case (MATHA&0xff):          return Susie.mMATHABCD.Bytes.A;
      case (MATHP&0xff):          return Susie.mMATHNP.Bytes.P;  
      case (MATHN&0xff):          return Susie.mMATHNP.Bytes.N;  
      case (MATHH&0xff):          return Susie.mMATHEFGH.Bytes.H;
      case (MATHG&0xff):          return Susie.mMATHEFGH.Bytes.G;
      case (MATHF&0xff):          return Susie.mMATHEFGH.Bytes.F;
      case (MATHE&0xff):          return Susie.mMATHEFGH.Bytes.E;
      case (MATHM&0xff):          return Susie.mMATHJKLM.Bytes.M;
      case (MATHL&0xff):          return Susie.mMATHJKLM.Bytes.L;
      case (MATHK&0xff):          return Susie.mMATHJKLM.Bytes.K;
      case (MATHJ&0xff):          return Susie.mMATHJKLM.Bytes.J;
      case (SUZYHREV&0xff):       return 0x01;
      default:                    return suzy.data[addr&0xff];
    }

	return 0xff;
}


static void CMikie_Reset(void)
{
	int loop;
	mAudioInputComparator=0;	// Initialises to unknown
	mikie.mDISPADRL=0;
	mikie.mDISPADRH=0;
//	mDisplayAddress=0x00;			// Initialises to unknown
	mLynxLine=0;
	mLynxLineDMACounter=0;
	mLynxAddr=0;

	mTimerStatusFlags=0x00;		// Initialises to ZERO, i.e No IRQ's
	mTimerInterruptMask=0x00;

//	core_memset(Mikie.m_TIM,0,sizeof(Mikie.m_TIM));
	core_memset(Mikie.m_AUDIO,0,sizeof(Mikie.m_AUDIO));

	mikie.mMSTEREO=0xff;

	// Start with an empty palette

	for(loop=0;loop<16;loop++){
		mPalette[loop] = loop;
	}

	// Initialise IODAT register

	mikie.mIODAT=0x00;
	mikie.mIODIR=0x00;
	mikie.mDISPCTL = 0;

	//
	// Initialise the UART variables
	//
//	mUART_RX_IRQ_ENABLE=0;
	mUART_TX_IRQ_ENABLE=0;
	mUART_TX_COUNTDOWN=UART_TX_INACTIVE;
//	mUART_RX_COUNTDOWN=UART_RX_INACTIVE;
//	mUART_Rx_input_ptr=0;
//	mUART_Rx_output_ptr=0;
//	mUART_Rx_waiting=0;
//	mUART_Rx_framing_error=0;
//	mUART_Rx_overun_error=0;
	mUART_SENDBREAK=0;
	mUART_TX_DATA=0;
//	mUART_RX_DATA=0;
//	mUART_RX_READY=0;

	mUART_PARITY_ENABLE=0;
	mUART_PARITY_EVEN=0;
}

// for AUDIO
static u32 CMikie_GetLfsrNext(u32 current)
{
	// The table is built thus:
	//	Bits 0-11  LFSR					(12 Bits)
	//  Bits 12-20 Feedback switches	(9 Bits)
	//     (Order = 7,0,1,2,3,4,5,10,11)
	//  Order is mangled to make peek/poke easier as
	//  bit 7 is in a seperate register
	//
	// Total 21 bits = 2MWords @ 4 Bytes/Word = 8MB !!!!!
	//
	// If the index is a combination of Current LFSR+Feedback the
	// table will give the next value.
	static u32 switches,lfsr,next,swloop,result;
	static u32 switchbits[9]={7,0,1,2,3,4,5,10,11};

	switches=current>>12;
	lfsr=current&0xfff;
	result=0;
	for(swloop=0;swloop<9;swloop++){
		if((switches>>swloop)&0x001) result^=(lfsr>>switchbits[swloop])&0x001;
	}
	result=(result)?0:1;
	next=(switches<<12)|((lfsr<<1)&0xffe)|result;
	return next;
}

// for Display
static u32 CMikie_DisplayRenderLine(void)
{
	PIXEL_FORMAT *bitmap_tmp=NULL;

// Logic says it should be 101 but testing on an actual lynx shows the rest
// persiod is between lines 102,101,100 with the new line being latched at
// the beginning of count==99 hence the code below !!

	if((mikie.mDISPCTL&0x01)) { 
		// Set the timer interrupt flag
		mTimerStatusFlags |= (mTimerInterruptMask&0x01);


		// Emulate REST signal
		// VBLANK中なのかを示すフラグっぽい
//		if(mLynxLine==Mikie.m_TIM[2].BKUP-2 || 
//		   mLynxLine==Mikie.m_TIM[2].BKUP-3 || 
//		   mLynxLine==Mikie.m_TIM[2].BKUP-4) mikie.mIODAT |= 0x08; //mIODAT_REST_SIGNAL=1;
//		else                                 mikie.mIODAT &=~0x08; //mIODAT_REST_SIGNAL=0;

		if(mLynxLine==(0x68/*Mikie.m_TIM[2].BKUP*/-3)) {
			mLynxAddr=((u16)mikie.mDISPADRH<<8)|mikie.mDISPADRL;
			if(mikie.mDISPCTL&0x02) {
				mLynxAddr+=3;
			}
			// Trigger line rending to start
			mLynxLineDMACounter=102;
		}

		// Decrement line counter logic
		if(mLynxLine) mLynxLine--;

		// Do 102 lines, nothing more, less is OK.
		if(mLynxLineDMACounter) {
			u8 source,loop;
			u8* pSrc = &mRam_Data[mLynxAddr];

			mLynxLineDMACounter--;

			// Mikie screen DMA can only see the system RAM....
			// (Step through bitmap, line at a time)

			// Assign the temporary pointer;
			bitmap_tmp=mpDisplayCurrent;
			
			if(mikie.mDISPCTL&0x02) {
				for(loop=0;loop<SCREEN_WIDTH/2;loop++) {
					source=*pSrc--;
					*bitmap_tmp++ = mColourMap[mPalette[source&15]];
					*bitmap_tmp++ = mColourMap[mPalette[source>>4]];
				}
				mLynxAddr-=SCREEN_WIDTH/2;
			} else {
				for(loop=0;loop<SCREEN_WIDTH/2;loop++) {
					source=*pSrc++;
					*bitmap_tmp++ = mColourMap[mPalette[source>>4]];
					*bitmap_tmp++ = mColourMap[mPalette[source&15]];
				}
				mLynxAddr+=SCREEN_WIDTH/2;
			}

			mpDisplayCurrent += fb_format.width; 
		}
	}
	return 0;//work_done;
}


// for Display
static u32 CMikie_DisplayEndOfFrame(void)
{
	mLynxLineDMACounter=0;
	mLynxLine=0x68;//Mikie.m_TIM[2].BKUP;

	// Set the timer status flag
	mTimerStatusFlags |= (mTimerInterruptMask&0x04);

	HAL_fb2_bitblt(&fb_format);

#ifndef WIN32
	HAL_fps(75);
#endif	

	mpDisplayCurrent = mpDisplayBits = fb_format.fb;

	return 0;
}

static void CCart_CartAddressData(u32 data)
{
	mAddrData=data;
}

void update_ctla(int n) 
{
	int data = mikie.mTIM[n].CTLA; //Mikie.m_TIM[n].CTLA;

	mTimerInterruptMask&=((1<<n)^0xff);
	mTimerInterruptMask|=(data&0x80)?(1<<n):0x00;
}

void upd_aud_ctl(int n) 
{
	int data = mikie.mAUD[n].CTL; // mAUD_CTL[n];
	Mikie.m_AUDIO[n].ENABLE_RELOAD=data&0x10;
	Mikie.m_AUDIO[n].ENABLE_COUNT=data&0x08;
	Mikie.m_AUDIO[n].LINKING=data&0x07;
	Mikie.m_AUDIO[n].INTEGRATE_ENABLE=data&0x20;
	if(data&0x40) Mikie.m_AUDIO[n].TIMER_DONE=0;
	Mikie.m_AUDIO[n].WAVESHAPER&=0x1fefff;
	Mikie.m_AUDIO[n].WAVESHAPER|=(data&0x80)?0x001000:0x000000;
	if(data&0x48){
		Mikie.m_AUDIO[n].LAST_COUNT=gSystemCycleCount;
	}
}

void upd_aud_misc(int n) 
{
	int data = mikie.mAUD[n].MISC; // mAUD_MISC[n];
	Mikie.m_AUDIO[n].WAVESHAPER&=0x1ff0ff;
	Mikie.m_AUDIO[n].WAVESHAPER|=(data&0xf0)<<4;
	Mikie.m_AUDIO[n].BORROW_IN=data&0x02;
	Mikie.m_AUDIO[n].BORROW_OUT=data&0x01;
	Mikie.m_AUDIO[n].LAST_CLOCK=data&0x04;
}

static u8 CMikie_Peek(u32 addr)
{
    u8 retval=0;

	addr&=0xff;

	// TIMに関する処理
	if(addr<0x20) { return mikie.regs[addr]; }
	if(addr<0x40) { return mikie.regs[addr]; }

    switch(addr) {
    case (MSTEREO&0xff):   return mikie.mMSTEREO^0xff;
    case (INTRST&0xff):    return (u8)mTimerStatusFlags;
    case (INTSET&0xff):    return (u8)mTimerStatusFlags;
    case (MAGRDY0&0xff):   return 0x00;
    case (MAGRDY1&0xff):   return 0x00;
    case (AUDIN&0xff):     return 0x80;
    case (MIKEYHREV&0xff): return 0x01;

    case (SERCTL&0xff): {
        u32 retval=0;
        retval|=(mUART_TX_COUNTDOWN&UART_TX_INACTIVE)?0xA0:0x00;	// Indicate TxDone & TxAllDone
        //            retval|=(mUART_RX_READY)?0x40:0x00;							// Indicate Rx data ready
        //            retval|=(mUART_Rx_overun_error)?0x08:0x0;					// Framing error
        //            retval|=(mUART_Rx_framing_error)?0x04:0x00;					// Rx overrun
        //            retval|=(mUART_RX_DATA&UART_BREAK_CODE)?0x02:0x00;			// Indicate break received
        //            retval|=(mUART_RX_DATA&0x0100)?0x01:0x00;					// Add parity bit
        //TRACE_MIKIE2("Peek(SERCTL  ,%02x) at PC=%04x",retval,CSystem_mCpu->GetPC());
        return (u8)retval;
    } break;

    case (SERDAT&0xff):	return 0;

	case (IODAT&0xff): {
		u32 retval=0;
		retval|=(mikie.mIODIR&0x10)?mikie.mIODAT&0x10:0x10;									// IODIR  = output bit : input high (eeprom write done)
//		retval|=(mikie.mIODIR&0x08)?(((mikie.mIODAT&0x08)&&mIODAT_REST_SIGNAL)?0x00:0x08):0x00;	// REST   = output bit : input low
		retval|=(mikie.mIODIR&0x04)?mikie.mIODAT&0x04:((mUART_CABLE_PRESENT)?0x04:0x00);	// NOEXP  = output bit : input low
		retval|=(mikie.mIODIR&0x02)?mikie.mIODAT&0x02:0x00;									// CARTAD = output bit : input low
		retval|=(mikie.mIODIR&0x01)?mikie.mIODAT&0x01:0x01;									// EXTPW  = output bit : input high (Power connected)
		//TRACE_MIKIE2("Peek(IODAT   ,%02x) at PC=%04x",retval,CSystem_mCpu->GetPC());
		return (u8)retval;
	}  break;

		// Register to let programs know handy is running
	case (0xfd97&0xff):
        return 0x42;
        break;

//	case (DISPADRL&0xff): return mikie.mDISPADRL;
//	case (DISPADRH&0xff): return mikie.mDISPADRH;

    default:
        //TRACE_MIKIE2("Peek(%04x) - Peek from illegal location at PC=$%04x",addr,CSystem_mCpu->GetPC());
        break;
    }
	return mikie.regs[addr];
//    return 0xff;
}

// Peek/Poke memory handlers
static void CMikie_Poke(u32 addr,u8 data)
{
	mikie.regs[addr&=0xff] = data;

	switch(addr) {
	case (CPUSLEEP&0xff): 
		CSusie_PaintSprites(); 
		break;

	case 0x01: case 0x05: case 0x09: case 0x0d: 
	case 0x11: case 0x15: case 0x19: case 0x1d: 
		update_ctla(addr/4); 
		break;

	case 0x25: case 0x2d: case 0x35: case 0x3d: 
		upd_aud_ctl(addr/8); 
		break;

	case 0x27: case 0x2f: case 0x37: case 0x3f: 
		upd_aud_misc(addr/8); 
		break;

//	case (ATTEN_A&0xff):
//	case (ATTEN_B&0xff):
//	case (ATTEN_C&0xff):
//	case (ATTEN_D&0xff):
//	case (MPAN&0xff):
//		////TRACE_MIKIE2("Poke(ATTEN_A/B/C/D/MPAN,%02x) at PC=%04x",data,CSystem_mCpu->GetPC());
//		break;

	case (MSTEREO&0xff):
		data^=0xff;
//		if(!(mikie.mSTEREO&0x11) && (data&0x11)) { mikie.m_AUDIO[0].LAST_COUNT=gSystemCycleCount; }
//		if(!(mikie.mSTEREO&0x22) && (data&0x22)) { mikie.m_AUDIO[1].LAST_COUNT=gSystemCycleCount; }
//		if(!(mikie.mSTEREO&0x44) && (data&0x44)) { mikie.m_AUDIO[2].LAST_COUNT=gSystemCycleCount; }
//		if(!(mikie.mSTEREO&0x88) && (data&0x88)) { mikie.m_AUDIO[3].LAST_COUNT=gSystemCycleCount; }
		mikie.mMSTEREO=data;
		break;

	case (INTRST&0xff): data^=0xff;  mTimerStatusFlags&=data; break;
	case (INTSET&0xff): mTimerStatusFlags|=data;              break;

	case (SYSCTL1&0xff):
		////TRACE_MIKIE2("Poke(SYSCTL1 ,%02x) at PC=%04x",data,CSystem_mCpu->GetPC());
		if(!(data&0x02)) {
//			char addr[256];
//			C6502_REGS regs;
//			CSystem_GetRegs(&regs);
//			sprintf(addr,"Runtime Alert - System Halted\nCMikie_Poke(SYSCTL1) - Lynx power down occured at PC=$%04x.\nResetting system.",regs.PC);
			//gError->Warning(addr);
			CSystem_Reset();
		}
		CCart_CartAddressStrobe((data&0x01)?1:0);
		break;

//	case (MIKEYSREV&0xff):  break;

//	case (IODIR&0xff): mikie.mIODIR=data; /* TRACE_MIKIE2("Poke(IODIR   ,%02x) at PC=%04x",data,CSystem_mCpu->GetPC()); */ break;
	case (IODAT&0xff): 
		CCart_CartAddressData((mikie.mIODAT&0x02)?1:0);
		if(mikie.mIODIR&0x10) CCart_mWriteEnableBank[1]=(mikie.mIODAT&0x10)?1:0;
		break;

	case (SERCTL&0xff): 
		////TRACE_MIKIE2("Poke(SERCTL  ,%02x) at PC=%04x",data,CSystem_mCpu->GetPC());
		mUART_TX_IRQ_ENABLE=(data&0x80)?1:0;
//		mUART_RX_IRQ_ENABLE=(data&0x40)?1:0;
		mUART_PARITY_ENABLE=(data&0x10)?1:0;
		mUART_SENDBREAK=data&0x02;
		mUART_PARITY_EVEN=data&0x01;

		// Reset all errors if required
		if(data&0x08) {
//			mUART_Rx_overun_error=0;
//			mUART_Rx_framing_error=0;
		}

		if(mUART_SENDBREAK) {
			// Trigger send break, it will self sustain as long as sendbreak is set
			mUART_TX_COUNTDOWN=UART_TX_TIME_PERIOD;
			// Loop back what we transmitted
//			CMikie_ComLynxTxLoopback(UART_BREAK_CODE);
		}
		break;

	case (SERDAT&0xff):
		////TRACE_MIKIE2("Poke(SERDAT ,%04x) at PC=%04x",data,CSystem_mCpu->GetPC());
		//
		// Fake transmission, set counter to be decremented by Timer 4
		//
		// ComLynx only has one output pin, hence Rx & Tx are shorted
		// therefore any transmitted data will loopback
		//
		mUART_TX_DATA=data;
		// Calculate Parity data
		if(mUART_PARITY_ENABLE) {
			// Calc parity value
			// Leave at zero !!
		} else {
			// If disabled then the PAREVEN bit is sent
			if(mUART_PARITY_EVEN) data|=0x0100;
		}
		// Set countdown to transmission
		mUART_TX_COUNTDOWN=UART_TX_TIME_PERIOD;
		// Loop back what we transmitted
//		CMikie_ComLynxTxLoopback(mUART_TX_DATA);
		break;

	default:
		if(0xa0<=addr && addr<=0xaf) {
			mPalette[addr&0x0f] &= 0x00FF;
			mPalette[addr&0x0f] |= (data&15)<<8;
		}
		else if(0xb0<=addr && addr<=0xbf) {
			mPalette[addr&0x0f] &= 0xFF00;
			mPalette[addr&0x0f] |= data;
		}
		break;
	}
}

static void CMikie_Update(void)
{
    s32 divide=0,decval;
	int nA;

    //
    // To stop problems with cycle count wrap we will check and then correct the
    // cycle counter.
    //
    if(gSystemCycleCount>0xf0000000)      {
        gSystemCycleCount-=0x80000000;
        gAudioLastUpdateCycle-=0x80000000;
        Mikie.m_AUDIO[0].LAST_COUNT-=0x80000000;
        Mikie.m_AUDIO[1].LAST_COUNT-=0x80000000;
        Mikie.m_AUDIO[2].LAST_COUNT-=0x80000000;
        Mikie.m_AUDIO[3].LAST_COUNT-=0x80000000;
    }
    //	Timer updates, rolled out flat in group order
    //
    //	Group A:
    //	Timer 0 -> Timer 2 -> Timer 4.
    //
    //	Group B:
    //	Timer 1 -> Timer 3 -> Timer 5 -> Timer 7 -> Audio 0 -> Audio 1-> Audio 2 -> Audio 3 -> Timer 1.
    //
    // Within each timer code block we will predict the cycle count number of
    // the next timer event
    //
    // We don't need to count linked timers as the timer they are linked
    // from will always generate earlier events.
    //
    // As Timer 4 (UART) will generate many events we will ignore it
    //
    // We set the next event to the end of time at first and let the timers
    // overload it. Any writes to timer controls will force next event to
    // be immediate and hence a new preidction will be done. The prediction
    // causes overflow as opposed to zero i.e. current+1
    // (In reality T0 line counter should always be running.)
    //

	//
	// Timer 1,3,5,7 of Group A
	//
	for(nA=1;nA<=7;nA+=2){
		// KW bugfix 13/4/99 added (mTIM_x_ENABLE_RELOAD ||  ..)
		if( TIM_ENABLE_COUNT(nA) && (TIM_ENABLE_RELOAD(nA) || !TIM_TIMER_DONE(nA))) {

			// カウンタを減算する場合の判定を変更する必要がある
			decval=1;


/*
			if(nA==5) {
				if(lnx_scanline%(TIM_LINKING(nA)+1)) {
					decval=0;
				} else {
					decval=1;
				}
			}
*/
			
/*			decval=0;

			if(nA==1) {
				if(TIM_LINKING(nA)!=0x07) {
					divide=(4+TIM_LINKING(nA));
					decval=(gSystemCycleCount-Mikie.m_TIM[nA].LAST_COUNT)>>divide;
				}
			} else {
				if(TIM_LINKING(nA)!=0x07) {
					// Ordinary clocked mode as opposed to linked mode
					// 16MHz clock downto 1us == cyclecount >> 4
					divide=(4+TIM_LINKING(nA));
					decval=(gSystemCycleCount-Mikie.m_TIM[nA].LAST_COUNT)>>divide;
				} else {
					decval = Mikie.m_TIM[nA-2].BORROW_OUT; //if(Mikie.m_TIM[nA-2].BORROW_OUT) decval=1;
					Mikie.m_TIM[nA].LAST_LINK_CARRY=Mikie.m_TIM[nA-2].BORROW_OUT;
				}
			}*/

			if(decval) {
				mikie.mTIM[nA].CNT--;

				if(mikie.mTIM[nA].CNT==0) {
					SET_BORROW_OUT(nA);//Mikie.m_TIM[nA].tBORROW_OUT=1;  // Set carry out
					mTimerStatusFlags |= (mTimerInterruptMask&(1<<nA));  // Set the timer status flag

					// Reload if neccessary
					if(TIM_ENABLE_RELOAD(nA)) { mikie.mTIM[nA].CNT=mikie.mTIM[nA].BKUP; } 
					else                      { mikie.mTIM[nA].CNT=0;                   }
					mikie.mTIM[nA].CTLB|=0x08;
				} else {
					CLR_BORROW_OUT(nA); //Mikie.m_TIM[nA].tBORROW_OUT=0;
				}
				// Set carry in as we did a count
				SET_BORROW_IN(nA); //Mikie.m_TIM[nA].tBORROW_IN=1;
			}
			else {
				CLR_BORROW_IN(nA); //Mikie.m_TIM[nA].tBORROW_IN=0;  // Clear carry in as we didn't count
				CLR_BORROW_OUT(nA);//Mikie.m_TIM[nA].tBORROW_OUT=0; // Clear carry out
			}
		}
	}

    //
    // Timer 6 has no group
    //
    // KW bugfix 13/4/99 added (mTIM_x_ENABLE_RELOAD ||  ..)
    if( TIM_ENABLE_COUNT(6) && 
		(TIM_ENABLE_RELOAD(6) || !TIM_TIMER_DONE(6)/*!Mikie.m_TIM[6].TIMER_DONE*/)) {
        //				if(Mikie.m_TIM[6].LINKING!=0x07)
        // Ordinary clocked mode as opposed to linked mode
        // 16MHz clock downto 1us == cyclecount >> 4
        divide=(4+TIM_LINKING(6));

		decval=1;
//        decval=(gSystemCycleCount-Mikie.m_TIM[6].LAST_COUNT)>>divide;

        if(decval) {
            mikie.mTIM[6].CNT--;
            if(mikie.mTIM[6].CNT==0) {
                SET_BORROW_OUT(6) // Mikie.m_TIM[6].tBORROW_OUT=1;                    // Set carry out
                mTimerStatusFlags |= (mTimerInterruptMask&0x40); // Set the timer status flag

                // Reload if neccessary
                if(TIM_ENABLE_RELOAD(6)) { mikie.mTIM[6].CNT=mikie.mTIM[6].BKUP; }
                else                     { mikie.mTIM[6].CNT=0;                  }
                mikie.mTIM[6].CTLB|=0x08;
            }
            else {
                CLR_BORROW_OUT(6); // Mikie.m_TIM[6].tBORROW_OUT=0;
            }
            // Set carry in as we did a count
            SET_BORROW_IN(6); //Mikie.m_TIM[6].tBORROW_IN=1;
        }
        else {
            CLR_BORROW_IN(6);  // Mikie.m_TIM[6].tBORROW_IN=0;  // Clear carry in as we didn't count
            CLR_BORROW_OUT(6); // Mikie.m_TIM[6].tBORROW_OUT=0; // Clear carry out
        }
    }

//	lynx_audio_work();

    // Update system IRQ status as a result of timer activity
    // OR is required to ensure serial IRQ's are not masked accidentally

	if(mTimerStatusFlags) {
		gSystemIRQ = 1;
		C65C02_IRQ_Check(); /* add by e */
	} else {
		gSystemIRQ = 0;
	}
//    gSystemIRQ=(mTimerStatusFlags)?1:0;

    // Now all the timer updates are done we can increment the system
    // counter for any work done within the Update() function, gSystemCycleCounter
    // cannot be updated until this point otherwise it screws up the counters.
//    gSystemCycleCount+=mikie_work_done;
}



//
// If sound is enabled then update the sound subsystem
//
static void lynx_audio_work()
{
	int nA;

    if(1) {
        static s32 sample=0;
        u32 mix=0;
		s32 divide;
		s32 decval;

        //
        // Catch audio buffer up to current time
        //
        // Mix the sample
        sample=0;
        if(mikie.mMSTEREO&0x11) { sample+=Mikie.m_AUDIO[0].OUTPUT; mix++; }
        if(mikie.mMSTEREO&0x22) { sample+=Mikie.m_AUDIO[1].OUTPUT; mix++; }
        if(mikie.mMSTEREO&0x44) { sample+=Mikie.m_AUDIO[2].OUTPUT; mix++; }
        if(mikie.mMSTEREO&0x88) { sample+=Mikie.m_AUDIO[3].OUTPUT; mix++; }
        if(mix) {
            sample+=128*mix; // Correct for sign
            sample/=mix;	// Keep the audio volume at max
        }
        else {
            sample=128;
        }

        // sample+=(Mikie.mSTEREO&0x11)?Mikie.m_AUDIO[0].OUTPUT:0;
        // sample+=(Mikie.mSTEREO&0x22)?Mikie.m_AUDIO[1].OUTPUT:0;
        // sample+=(Mikie.mSTEREO&0x44)?Mikie.m_AUDIO[2].OUTPUT:0;
        // sample+=(Mikie.mSTEREO&0x88)?Mikie.m_AUDIO[3].OUTPUT:0;
        // sample=sample>>2;
        // sample+=128;

        for(;gAudioLastUpdateCycle+HANDY_AUDIO_SAMPLE_PERIOD<gSystemCycleCount;gAudioLastUpdateCycle+=HANDY_AUDIO_SAMPLE_PERIOD) {
            // Output audio sample
            gAudioBuffer[gAudioBufferPointer++]=(u8)sample;

            // Check buffer overflow condition, stick at the endpoint
            // teh audio output system will reset the input pointer
            // when it reads out the data.

            // We should NEVER overflow, this buffer holds 0.25 seconds
            // of data if this happens the the multimedia system above
            // has failed so the corruption of the buffer contents wont matter

            gAudioBufferPointer%=HANDY_AUDIO_BUFFER_SIZE;
        }

	for(nA=0;nA<4;nA++){
        //
        // Audio 1
        //
        if(  Mikie.m_AUDIO[nA].ENABLE_COUNT && 
			(Mikie.m_AUDIO[nA].ENABLE_RELOAD || !Mikie.m_AUDIO[nA].TIMER_DONE) && 
			mikie.mAUD[nA].VOL && 
			Mikie.m_AUDIO[nA].BKUP) {
            decval=0;

            if(Mikie.m_AUDIO[nA].LINKING==0x07) {
				if(nA==0){ 
	                if(TIM_BORROW_OUT(7)/*Mikie.m_TIM[7].tBORROW_OUT*/) decval=1;
		            Mikie.m_AUDIO[nA].LAST_LINK_CARRY=TIM_BORROW_OUT(7);//Mikie.m_TIM[7].tBORROW_OUT;
				} else {
	                if(Mikie.m_AUDIO[nA-1].BORROW_OUT) decval=1;
		            Mikie.m_AUDIO[nA].LAST_LINK_CARRY=Mikie.m_AUDIO[nA-1].BORROW_OUT;
				}
			} else {
                // Ordinary clocked mode as opposed to linked mode
                // 16MHz clock downto 1us == cyclecount >> 4
                divide=(4+Mikie.m_AUDIO[nA].LINKING);
                decval=(gSystemCycleCount-Mikie.m_AUDIO[nA].LAST_COUNT)>>divide;
            }

            if(decval) {
                Mikie.m_AUDIO[nA].LAST_COUNT+=decval<<divide;
                Mikie.m_AUDIO[nA].CURRENT-=decval;
                if(Mikie.m_AUDIO[nA].CURRENT&0x80000000) {
                    // Set carry out
                    Mikie.m_AUDIO[nA].BORROW_OUT=1;

                    // Reload if neccessary
                    if(Mikie.m_AUDIO[nA].ENABLE_RELOAD) {
                        Mikie.m_AUDIO[nA].CURRENT+=Mikie.m_AUDIO[nA].BKUP+1;
                        if(Mikie.m_AUDIO[nA].CURRENT&0x80000000) Mikie.m_AUDIO[nA].CURRENT=0;
                    }
                    else {
                        // Set timer done
                        Mikie.m_AUDIO[nA].TIMER_DONE=1;
                        Mikie.m_AUDIO[nA].CURRENT=0;
                    }

                    //
                    // Update audio circuitry
                    //
                    Mikie.m_AUDIO[nA].WAVESHAPER=CMikie_GetLfsrNext(Mikie.m_AUDIO[nA].WAVESHAPER);

                    if(Mikie.m_AUDIO[nA].INTEGRATE_ENABLE) {
                        s32 temp=Mikie.m_AUDIO[nA].OUTPUT;
                        if(Mikie.m_AUDIO[nA].WAVESHAPER&0x0001) temp+=mikie.mAUD[nA].VOL; //Mikie.m_AUDIO[nA].VOLUME; 
						else                                    temp-=mikie.mAUD[nA].VOL; //Mikie.m_AUDIO[nA].VOLUME;
                        if(temp>127) temp=127;
                        if(temp<-128) temp=-128;
                        Mikie.m_AUDIO[nA].OUTPUT=(s8)temp;
                    }
                    else {
                        if(Mikie.m_AUDIO[nA].WAVESHAPER&0x0001) Mikie.m_AUDIO[nA].OUTPUT= mikie.mAUD[nA].VOL; //Mikie.m_AUDIO[nA].VOLUME; 
						else                                    Mikie.m_AUDIO[nA].OUTPUT=-mikie.mAUD[nA].VOL; //Mikie.m_AUDIO[nA].VOLUME;
                    }
                }
                else {
                    Mikie.m_AUDIO[nA].BORROW_OUT=0;
                }
                // Set carry in as we did a count
                Mikie.m_AUDIO[nA].BORROW_IN=1;
            }
            else {
				Mikie.m_AUDIO[nA].BORROW_IN=0;
				Mikie.m_AUDIO[nA].BORROW_OUT=0;
			}
		}
	}
    }
}

//-----------------------------------------------------------------------------
//
//
//-----------------------------------------------------------------------------
static void uart_update(void)
{
    s32 divide, decval;

    //
    // Timer 4 of Group A
    //
    // For the sake of speed it is assumed that Timer 4 (UART timer)
    // never uses one-shot mode, never uses linking, hence the code
    // is commented out. Timer 4 is at the end of a chain and seems
    // no reason to update its carry in-out variables
    //

    // KW bugfix 13/4/99 added (mTIM_x_ENABLE_RELOAD ||  ..)
    if(TIM_ENABLE_COUNT(4))  {
        decval=0;
        // Ordinary clocked mode as opposed to linked mode
        // 16MHz clock downto 1us == cyclecount >> 4
        // Additional /8 (+3) for 8 clocks per bit transmit
        divide=4+3+(TIM_LINKING(4));
//        decval=(gSystemCycleCount-Mikie.m_TIM[4].LAST_COUNT)>>divide;

		decval=1;

        if(decval) {
//            Mikie.m_TIM[4].LAST_COUNT+=decval<<divide;
            mikie.mTIM[4].CNT--; // Mikie.m_TIM[4].CURRENT-=decval;
            if(mikie.mTIM[4].CNT==0/*Mikie.m_TIM[4].CURRENT&0x80000000*/) {
                // Set carry out
                SET_BORROW_OUT(4); // Mikie.m_TIM[4].tBORROW_OUT=1;

                //
                // According to the docs IRQ's are level triggered and hence will always assert
                // what a pain in the arse
                //
                // Rx & Tx are loopedback due to comlynx structure

                //
                // Receive
                //
#if 0
//                if(!mUART_RX_COUNTDOWN) {
//                    // Fetch a byte from the input queue
//                    if(mUART_Rx_waiting>0) {
//                        mUART_RX_DATA=mUART_Rx_input_queue[mUART_Rx_output_ptr];
//                        mUART_Rx_output_ptr=(++mUART_Rx_output_ptr)%UART_MAX_RX_QUEUE;
//                        mUART_Rx_waiting--;
//                        //TRACE_MIKIE2("Update() - RX Byte output ptr=%02d waiting=%02d",mUART_Rx_output_ptr,mUART_Rx_waiting);
//                    } else  {
//                        //TRACE_MIKIE0("Update() - RX Byte but no data waiting ????");
//                    }
//
//                    // Retrigger input if more bytes waiting
//                    if(mUART_Rx_waiting>0)  {
//                        mUART_RX_COUNTDOWN=UART_RX_TIME_PERIOD+UART_RX_NEXT_DELAY;
//                        //TRACE_MIKIE1("Update() - RX Byte retriggered, %d waiting",mUART_Rx_waiting);
//                    } else {
//                        mUART_RX_COUNTDOWN=UART_RX_INACTIVE;
//                        //TRACE_MIKIE0("Update() - RX Byte nothing waiting, deactivated");
//                    }
//
//                    // If RX_READY already set then we have an overrun
//                    // as previous byte hasnt been read
//                    if(mUART_RX_READY) mUART_Rx_overun_error=1;
//
//                    // Flag byte as being recvd
//                    mUART_RX_READY=1;
//                }
//                else if(!(mUART_RX_COUNTDOWN&UART_RX_INACTIVE))  {
//                    mUART_RX_COUNTDOWN--;
//                }
#endif

                if(!mUART_TX_COUNTDOWN)  {
                    if(mUART_SENDBREAK)  {
                        mUART_TX_DATA=UART_BREAK_CODE;
                        // Auto-Respawn new transmit
                        mUART_TX_COUNTDOWN=UART_TX_TIME_PERIOD;
                        // Loop back what we transmitted
//                        CMikie_ComLynxTxLoopback(mUART_TX_DATA);
                    }
                    else  {
                        // Serial activity finished
                        mUART_TX_COUNTDOWN=UART_TX_INACTIVE;
                    }

                    // If a networking object is attached then use its callback to send the data byte.
//                    if(mpUART_TX_CALLBACK)  {
//                        //TRACE_MIKIE0("Update() - UART_TX_CALLBACK");
//                        (*mpUART_TX_CALLBACK)(mUART_TX_DATA,mUART_TX_CALLBACK_OBJECT);
//                    }

                }
                else if(!(mUART_TX_COUNTDOWN&UART_TX_INACTIVE))  {
                    mUART_TX_COUNTDOWN--;
                }

                // Set the timer status flag
                // Timer 4 is the uart timer and doesn't generate IRQ's using this method

                // 16 Clocks = 1 bit transmission. Hold separate Rx & Tx counters

                // Reload if neccessary
                //						if(Mikie.m_TIM[4].ENABLE_RELOAD)
                //						{
                mikie.mTIM[4].CNT=mikie.mTIM[4].BKUP; //Mikie.m_TIM[4].CURRENT+=mikie.mTIM[4].BKUP/*Mikie.m_TIM[4].BKUP*/+1;
                // The low reload values on TIM4 coupled with a longer
                // timer service delay can sometimes cause
                // an underun, check and fix
                if(mikie.mTIM[4].CNT==0/*Mikie.m_TIM[4].CURRENT&0x80000000*/) {
                    mikie.mTIM[4].CNT=mikie.mTIM[4].BKUP; // Mikie.m_TIM[4].CURRENT=Mikie.m_TIM[4].BKUP;
//                    Mikie.m_TIM[4].LAST_COUNT=gSystemCycleCount;
                }
            }
        }
	}

    // Emulate the UART bug where UART IRQ is level sensitive
    // in that it will continue to generate interrupts as long
    // as they are enabled and the interrupt condition is 1

    // If Tx is inactive i.e ready for a byte to eat and the
    // IRQ is enabled then generate it always
    if((mUART_TX_COUNTDOWN&UART_TX_INACTIVE) && mUART_TX_IRQ_ENABLE)  {
        //TRACE_MIKIE0("Update() - UART TX IRQ Triggered");
        mTimerStatusFlags|=0x10;
    }
    // Is data waiting and the interrupt enabled, if so then
    // what are we waiting for....
//    if(mUART_RX_READY && mUART_RX_IRQ_ENABLE) {
//        //TRACE_MIKIE0("Update() - UART RX IRQ Triggered");
//        mTimerStatusFlags|=0x10;
//    }
}




// IGNORE THIS TEXT, now overridden by new system
//
// We will hold 16 different memory maps for the "top" area which are selected
// on the basis of mMemMap->mSelector:
//
//				Code	Vect	ROM		Mikie	Susie
//----------------------------------------------------
//	(Default)	0000	V		R		M		S
//				0001	V		R		M		RAM
//				0001	V		R		RAM		S
//				0011	V		R		RAM		RAM
//				0100	V		RAM		M		S
//				..
//				..
//				1111	RAM		RAM		RAM		RAM
//
// Get it.....
//
// We can then index with mMemoryHandlers[mMemMap->mSelector][addr] for speed
//

static void CRam_Poke(u32 addr, u8 data) { mRam_Data[addr]=data; }
static u8   CRam_Peek(u32 addr)          { return (mRam_Data[addr]); }
static void CBios_Poke(u32 addr,u8 data) { /* if(mWriteEnable) pLynxBios[addr&ROM_ADDR_MASK]=data;*/ }
static u8   CBios_Peek(u32 addr)         { return(pLynxBios[addr&ROM_ADDR_MASK]); }

static u8 CMemMap_Peek(u32 addr) { return nMemMap; }

void CMemMap_Poke(u32 addr, u8 data)
{
    int i;

	nMemMap = data;

    // 0000-FC00 Susie area
	for(i=0;i<0x10000;i++) {
		mMemoryHandlers[i].pPeek = CRam_Peek;
		mMemoryHandlers[i].pPoke = CRam_Poke;
	}

    // FC00-FCFF Susie area
	if(!(data&0x01)){
		for(i=0xfc00;i<0xfd00;i++){ 
			mMemoryHandlers[i].pPeek = CSusie_Peek;
			mMemoryHandlers[i].pPoke = CSusie_Poke;
		}
	}
	// FD00-FDFF Mikie area
	if(!(data&0x02)) {
		for(i=0xfd00;i<0xfe00;i++) {
			mMemoryHandlers[i].pPeek = CMikie_Peek;
			mMemoryHandlers[i].pPoke = CMikie_Poke;
		}
	}

    // FE00-FFF7 Rom area
	if(!(data&0x04)){
		for(i=0xfe00;i<0xfff8;i++) {
			mMemoryHandlers[i].pPeek = CBios_Peek;
			mMemoryHandlers[i].pPoke = CBios_Poke;
		}
	}

    // FFF8-FFF9 Vector area - Overload ROM space
//	mMemoryHandlers[0xFFF8].pPeek = CRam_Peek;
//	mMemoryHandlers[0xFFF8].pPoke = CRam_Poke;
	mMemoryHandlers[0xFFF9].pPeek = CMemMap_Peek;
	mMemoryHandlers[0xFFF9].pPoke = CMemMap_Poke;

    // FFFA-FFFF Vector area - Overload ROM space
	if(!(data&0x08)){
		for(i=0xfffa;i<0x10000;i++) {
			mMemoryHandlers[i].pPeek = CBios_Peek;
			mMemoryHandlers[i].pPoke = CBios_Poke;
		}
	}
}

static int CCart_Init(u8 *gamedata,u32 gamesize)
{
	LYNX_HEADER	header;
	u32 loop,bnk;
	CTYPE banktype[2];

	CCart_mWriteEnableBank[0]=FALSE;
	CCart_mWriteEnableBank[1]=FALSE;
	CCart_mCartRAM=FALSE;
//	mHeaderLess=FALSE;
	
	core_memcpy(&header,gamedata,sizeof(LYNX_HEADER));
	
	// Set the filetypes
	for(bnk=0;bnk<2;bnk++){
		switch(header.page_size_bank[bnk]) {
		case 0x000:  banktype[bnk]=UNUSED;  mMaskBank[bnk]=0x000000;  mShiftCount[bnk]=0;  mCountMask[bnk]=0x000;  break;
		case 0x100:  banktype[bnk]=C64K;    mMaskBank[bnk]=0x00ffff;  mShiftCount[bnk]=8;  mCountMask[bnk]=0x0ff;  break;
		case 0x200:  banktype[bnk]=C128K;   mMaskBank[bnk]=0x01ffff;  mShiftCount[bnk]=9;  mCountMask[bnk]=0x1ff;  break;
		case 0x400:  banktype[bnk]=C256K;   mMaskBank[bnk]=0x03ffff;  mShiftCount[bnk]=10; mCountMask[bnk]=0x3ff;  break;
		case 0x800:  banktype[bnk]=C512K;   mMaskBank[bnk]=0x07ffff;  mShiftCount[bnk]=11; mCountMask[bnk]=0x7ff;  break;
		default:     return 0;
		}
	}

	// Make some space for the new carts
	// Set default bank
	mBank=bank0;

	// Initialiase
	for(bnk=0;bnk<2;bnk++) {
		for(loop=0;loop<mMaskBank[bnk]+1;loop++){
			mCartBank[bnk][loop]=DEFAULT_CART_CONTENTS;
		}
	}

	// Copy the cart banks from the image
	// Read in the BANK0 bytes
	for(bnk=0;bnk<2;bnk++) {
		if(mMaskBank[bnk]){
			core_memcpy(mCartBank[bnk],gamedata+(sizeof(LYNX_HEADER)),mMaskBank[bnk]+1);
		}
	}

	// As this is a cartridge boot unset the boot address
	//
	// Check if this is a headerless cart
	//
//	mHeaderLess=TRUE;
//	for(loop=0;loop<32;loop++){
//		if(mCartBank[0][loop&mMaskBank[0]]!=0x00) mHeaderLess=FALSE;
//	}
	
	// Dont allow an empty Bank1 - Use it for shadow SRAM/EEPROM
	if(banktype[1]==UNUSED) {
		// Delete the single byte allocated  earlier
		// Allocate some new memory for us
		banktype[1]=C64K;
		mMaskBank[1]=0x00ffff;
		mShiftCount[1]=8;
		mCountMask[1]=0x0ff;
		for(loop=0;loop<mMaskBank[1]+1;loop++) {
			mCartBank[1][loop]=DEFAULT_RAM_CONTENTS;
		}
		CCart_mWriteEnableBank[1]=TRUE;
		CCart_mCartRAM=TRUE;
	}

	return 1;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
static void CCart_Poke(u32 addr, u8 data)
{
	if(CCart_mWriteEnableBank[mBank]) mCartBank[mBank][addr&mMaskBank[mBank]]=data;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
static u8 CCart_Peek(u32 addr)
{
	return(mCartBank[mBank][addr&mMaskBank[mBank]]);
}

//-----------------------------------------------------------------------------
//
//
//-----------------------------------------------------------------------------
static void CCart_CartAddressStrobe(u32 strobe)
{
	static int last_strobe=0;
	mStrobe=strobe;
	if(mStrobe) mCounter=0;

	// Either of the two below seem to work OK.
	// if(!strobe && last_strobe)
	if(mStrobe && !last_strobe) {
		// Clock a bit into the shifter
		mShifter=mShifter<<1;
		mShifter+=mAddrData?1:0;
		mShifter&=0xff;
	}
	last_strobe=mStrobe;
}

//-----------------------------------------------------------------------------
//
//
//-----------------------------------------------------------------------------
static void CSystem_Reset(void)
{
	gSystemCycleCount=0;
	gSystemIRQ=FALSE;
	gAudioBufferPointer=0;
	gAudioLastUpdateCycle=0;

	core_memset(gAudioBuffer,128,HANDY_AUDIO_BUFFER_SIZE);
	CMemMap_Poke(0,0);
	
	//CCart_Reset();
	mCounter=0;
	mShifter=0;
	mAddrData=0;
	mStrobe=0;
		
	//CRam_Reset(); 
	core_memset(mRam_Data,DEFAULT_RAM_CONTENTS,RAM_SIZE-512);
		
	CMikie_Reset();//	mMikie->Reset();
	CSusie_Reset();
	C65C02_Reset();
}


//-----------------------------------------------------------------------------
// for e[mulator]
//-----------------------------------------------------------------------------
static int LNX_INIT(int nRomSize,byte* pRomAddr,int type)
{
	int i;

	C65C02_Init();
	CCart_Init(pRomAddr,nRomSize);

	//void CMikie_Init()
	mUART_CABLE_PRESENT=0;
//	mpUART_TX_CALLBACK=NULL;
	for(i=0;i<16;i++)   mPalette[i]=i;
	for(i=0;i<4096;i++) mColourMap[i]=0;
	
	CSystem_Reset();

//	CMikie_DisplaySetAttributes(1,1,256,0);
//static void CMikie_DisplaySetAttributes(u32 Rotate,u32 Format,u32 Pitch,u32 objref)
	mpDisplayCurrent = mpDisplayBits = fb_format.fb;

    // Calculate the colour lookup tabes for the relevant mode
	for(i=0;i<4096;i++) {
		mColourMap[i] = HAL_fb2_Color(/*R*/((i>>0)&15),
									  /*G*/((i>>8)&15),
									  /*B*/((i>>4)&15), RGB444);
	}
   
	HAL_fb2_init(256,256,&fb_format,HW_LNX);

	fb_format.pic_w = 160;
	fb_format.pic_h = 102;

	//---------------------------------
	mpDisplayBits    = fb_format.fb;
	mpDisplayCurrent = fb_format.fb;

    return 1;
}


//-----------------------------------------------------------------------------
// CPU 4MHz(4000000) 
// SCANLINE = 102 ?  120?
// 75fps
// 4000000 / 75 / 102 = 523
// 4000000 / 75 / 110 = 485
// 4000000 / 75 / 120 = 445
//-----------------------------------------------------------------------------
static int LNX_LOOP(void)
{
	u32 key;

	//static void CSystem_Update(void)
	for(lnx_scanline=0;lnx_scanline<104;lnx_scanline++) {
		uart_update();  
			
		C65C02_Update(525);

		CMikie_Update();

		CMikie_DisplayRenderLine();

		lynx_audio_work();

	}
	CMikie_DisplayEndOfFrame();

	//CSusie_SetButtonData(key);
	suzy.mJOYSTICK = (u8)(key = HAL_Input(0,HW_LNX));
	suzy.mSWITCHES = (u8)(key>>8);

	return key & (1<<31);
}

static int LNX_RESET(void)  { return 1; }
static int LNX_SAVE(int fd) { return 1; }
static int LNX_LOAD(int fd) { return 1; }
static int LNX_EXIT(void)   { return 1; }

int LNX_Setup(void)
{
	nLynxBios = HAL_Cfg_Load("lynxboot.img",pLynxBios,512);

    HAL_SetupExt(EXT_LNX,"lnx",LNX_INIT,LNX_LOOP,LNX_EXIT,LNX_RESET,LNX_LOAD,LNX_SAVE);
    return 1;
}

