
#ifndef OSD_CPU_H
#define OSD_CPU_H

//typedef unsigned char						u8;
//typedef unsigned short						u16;
//typedef unsigned int						u32;
//
//typedef signed char 						s8;
//typedef signed short						s16;
//typedef signed int							s32;

typedef union {
#ifdef LSB_FIRST
	struct { u8 l,h,h2,h3; } b;
	struct { u16 l,h; } w;
#else
	struct { u8 h3,h2,h,l; } b;
	struct { u16 h,l; } w;
#endif
	u32 d;
}	PAIR;

#endif	/* defined OSD_CPU_H */
