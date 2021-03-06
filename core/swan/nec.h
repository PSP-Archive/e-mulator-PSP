#ifndef __NEC_H__
#define __NEC_H__

int ws_int_check(void);

#define OPTIMIZE_CLK_JMP  // dis:1474,ena:1394
#define OPTIMIZE_CF       // dis:2189,ena:1886

typedef enum { ES, CS, SS, DS } SREGS;
typedef enum { AW, CW, DW, BW, SP, BP, IX, IY } WREGS;
typedef enum { AL,AH,CL,CH,DL,DH,BL,BH,SPL,SPH,BPL,BPH,IXL,IXH,IYL,IYH } BREGS;

// external function
BYTE cpu_readport(BYTE);
void cpu_writeport(DWORD,BYTE);
void cpu_writemem20(DWORD,BYTE);

// external values
extern BYTE* pWsRomMap[0x10];

void cpu_writeport2(DWORD port,WORD value);

// internal function
static BYTE cpu_readmem20(DWORD addr) 
{ 
	return * (pWsRomMap[(addr>>16)&0x0f] + (u16)addr); 
}
//#define cpu_readmem20(addr) (*(pWsRomMap[((addr)>>16)&0x0f] + (u16)(addr)))




// Optimize fetch
#define cpu_readop(addr)           cpu_readmem20(addr)
#define cpu_readop_arg(addr)       cpu_readmem20(addr)

#define NEC_NMI_INT_VECTOR 2

/* Cpu types, steps of 8 to help the cycle count calculation */
#ifndef FALSE
#define FALSE 0
#define TRUE 1
#endif

/* parameter x = result, y = source 1, z = source 2 */

#define SetTF(x)		(I.TF = (x))
#define SetIF(x)		(I.IF = (x))
#define SetDF(x)		(I.DF = (x))
#define SetMD(x)		(I.MF = (x))	/* OB [19.07.99] Mode Flag V30 */

#ifdef  OPTIMIZE_CF
#define SetCFB(x)		(I.CarryVal = (((x)>> 8) & 1))
#define SetCFW(x)		(I.CarryVal = (((x)>>16) & 1))
#else //OPTIMIZE_CF
#define SetCFB(x)		(I.CarryVal = (x) & 0x100)
#define SetCFW(x)		(I.CarryVal = (x) & 0x10000)
#endif//OPTIMIZE_CF

#define SetAF(x,y,z)	(I.AuxVal = ((x) ^ ((y) ^ (z))) & 0x10)

#define SetSF(x)		(I.SignVal = (x))
#define SetZF(x)		(I.ZeroVal = (x))
#define SetPF(x)		(I.ParityVal = (x))

#define SetSZPF_Byte(x) (I.SignVal=I.ZeroVal=I.ParityVal=(s8)(x))
#define SetSZPF_Word(x) (I.SignVal=I.ZeroVal=I.ParityVal=(s16)(x))

#define SetOFW_Add(x,y,z)	(I.OverVal = ((x) ^ (y)) & ((x) ^ (z)) & 0x8000)
#define SetOFB_Add(x,y,z)	(I.OverVal = ((x) ^ (y)) & ((x) ^ (z)) & 0x80)
#define SetOFW_Sub(x,y,z)	(I.OverVal = ((z) ^ (y)) & ((z) ^ (x)) & 0x8000)
#define SetOFB_Sub(x,y,z)	(I.OverVal = ((z) ^ (y)) & ((z) ^ (x)) & 0x80)

#define ADDB { u32 res=dst+src; SetCFB(res); SetOFB_Add(res,src,dst); SetAF(res,src,dst); SetSZPF_Byte(res); dst=(BYTE)res; }
#define ADDW { u32 res=dst+src; SetCFW(res); SetOFW_Add(res,src,dst); SetAF(res,src,dst); SetSZPF_Word(res); dst=(WORD)res; }

#define SUBB { u32 res=dst-src; SetCFB(res); SetOFB_Sub(res,src,dst); SetAF(res,src,dst); SetSZPF_Byte(res); dst=(BYTE)res; }
#define SUBW { u32 res=dst-src; SetCFW(res); SetOFW_Sub(res,src,dst); SetAF(res,src,dst); SetSZPF_Word(res); dst=(WORD)res; }

#define ORB dst|=src; I.CarryVal=I.OverVal=I.AuxVal=0; SetSZPF_Byte(dst)
#define ORW dst|=src; I.CarryVal=I.OverVal=I.AuxVal=0; SetSZPF_Word(dst)

#define ANDB dst&=src; I.CarryVal=I.OverVal=I.AuxVal=0; SetSZPF_Byte(dst)
#define ANDW dst&=src; I.CarryVal=I.OverVal=I.AuxVal=0; SetSZPF_Word(dst)

#define XORB dst^=src; I.CarryVal=I.OverVal=I.AuxVal=0; SetSZPF_Byte(dst)
#define XORW dst^=src; I.CarryVal=I.OverVal=I.AuxVal=0; SetSZPF_Word(dst)

#ifdef OPTIMIZE_CF
#define CF      (I.CarryVal)
#else
#define CF		(I.CarryVal!=0)
#endif

#define SF		(I.SignVal<0)
#define ZF		(I.ZeroVal==0)
#define PF		parity_table[(BYTE)I.ParityVal]
#define AF		(I.AuxVal!=0)
#define OF		(I.OverVal!=0)
#define MD		(I.MF!=0)

/************************************************************************/

#define GetMemB(Seg,Off)  ((u8 )cpu_readmem20((DefaultBase(Seg)+(Off))))

/* Todo:  Remove these later - plus readword could overflow */
#define ReadByte(ea)       ( (BYTE)cpu_readmem20((ea)) )
#define PutMemB(Seg,Off,x) {  cpu_writemem20((DefaultBase(Seg)+(Off)),(x)); }
#define WriteByte(ea,val)  { cpu_writemem20((ea),val); }

#define GetMemW(Seg,Off)   ( (u16)cpu_readmem20((DefaultBase(Seg)+(Off))) + (cpu_readmem20((DefaultBase(Seg)+((Off)+1)))<<8) )
#define ReadWord(ea)       ( cpu_readmem20((ea))+(cpu_readmem20(((ea)+1))<<8))

//#define GetMemW(Seg,Off)   (cpu_readmem20w((DefaultBase(Seg)+(Off))))
//#define ReadWord(ea)       (cpu_readmem20w(ea))

#define PutMemW(Seg,Off,x) { PutMemB(Seg,Off,(x)&0xff); PutMemB(Seg,(Off)+1,(BYTE)((x)>>8));      }
#define WriteWord(ea,val)  { cpu_writemem20((ea),(BYTE)(val)); cpu_writemem20(((ea)+1),(val)>>8); }

#define read_port(port)        cpu_readport(port)
#define write_port(port,val)   cpu_writeport(port,val)
#define write_portw(port,val)  cpu_writeport2(port,val)

#define DefaultBase(Seg) ((seg_prefix && (Seg==DS || Seg==SS)) ? (prefix_base<<4) : I.sregs[Seg] << 4)

#define SegBase(Seg)     (I.sregs[Seg] << 4)

#define FETCH            (cpu_readop_arg((I.sregs[CS]<<4)+I.ip++))
#define FETCHOP          (cpu_readop((I.sregs[CS]<<4)+I.ip++))
#define FETCHWORD(var) { var=cpu_readop_arg((((I.sregs[CS]<<4)+I.ip)))+(cpu_readop_arg((((I.sregs[CS]<<4)+I.ip+1)))<<8); I.ip+=2; }

#define GetModRM         u32 ModRM=cpu_readop_arg((I.sregs[CS]<<4)+I.ip++)

#define PUSH(val) { I.regs.w[SP]-=2; WriteWord((((I.sregs[SS]<<4)+I.regs.w[SP])),val); }
#define POP(var)  { var = ReadWord((((I.sregs[SS]<<4)+I.regs.w[SP]))); I.regs.w[SP]+=2; }

#define PEEK(addr)   ((BYTE)cpu_readop_arg(addr))
#define PEEKOP(addr) ((BYTE)cpu_readop(addr))

/* Cycle count macros:
	CLK  - cycle count is the same on all processors
	CLKS - cycle count differs between processors, list all counts
	CLKW - cycle count for word read/write differs for odd/even source/destination address
	CLKM - cycle count for reg/mem instructions
	CLKR - cycle count for reg/mem instructions with different counts for odd/even addresses

	Prefetch & buswait time is not emulated.
	Extra cycles for PUSH'ing or POP'ing registers to odd addresses is not emulated.
*/

#define PSTALL(v30)                nec_ICount-=clk_Stall;

#define CLKS(v30)                { nec_ICount-=v30; }
#define CLK(all)                 { nec_ICount-=all; }
#define CLKW(v30MZo,v30MZe)      { nec_ICount-=(I.ip&1)?v30MZo:v30MZe; }
#define CLKM(v30MZm,v30MZ)       { nec_ICount-=( ModRM >=0xc0 )?v30MZ:v30MZm; }
#define CLKR(v30MZo,v30MZe,vall) { if (ModRM >=0xc0) nec_ICount-=vall; else nec_ICount-=(I.ip&1)?v30MZo:v30MZe; }

#define CompressFlags() (WORD)(CF | (PF << 2) | (AF << 4) | (ZF << 6) \
				| (SF << 7) | (I.TF << 8) | (I.IF << 9) \
				| (I.DF << 10) | (OF << 11))


#define ExpandFlags(f) \
{ \
	I.CarryVal = (f) & 1; \
	I.ParityVal = !((f) & 4); \
	I.AuxVal = (f) & 16; \
	I.ZeroVal = !((f) & 64); \
	I.SignVal = (f) & 128 ? -1 : 0; \
	I.TF = ((f) & 256) == 256; \
	I.IF = ((f) & 512) == 512; \
	I.DF = ((f) & 1024) == 1024; \
	I.OverVal = (f) & 2048; \
	I.MF = ((f) & 0x8000) == 0x8000; \
	ws_int_check(); \
}



#define IncWordReg(Reg) 					\
	unsigned tmp = (unsigned)I.regs.w[Reg]; \
	unsigned tmp1 = tmp+1;					\
	I.OverVal = (tmp == 0x7fff); 			\
	SetAF(tmp1,tmp,1);						\
	SetSZPF_Word(tmp1); 					\
	I.regs.w[Reg]=tmp1



#define DecWordReg(Reg) 					\
	unsigned tmp = (unsigned)I.regs.w[Reg]; \
    unsigned tmp1 = tmp-1; 					\
	I.OverVal = (tmp == 0x8000); 			\
    SetAF(tmp1,tmp,1); 						\
    SetSZPF_Word(tmp1); 					\
	I.regs.w[Reg]=tmp1

#define ADJ4(param1,param2)					\
    if (AF || ((I.regs.b[AL] & 0xf) > 9)) { \
		I.regs.b[AL] = I.regs.b[AL] + param1;	\
		I.AuxVal = 1;						\
	}										\
    if (CF || (I.regs.b[AL] > 0x9f)) { 	    \
		I.regs.b[AL] += param2;				\
		I.CarryVal = 1;						\
	}										\
	SetSZPF_Byte(I.regs.b[AL])

#define ADJB(param1,param2)					\
    if (AF || ((I.regs.b[AL] & 0xf) > 9)) { \
		I.regs.b[AL] += param1;				\
		I.regs.b[AH] += param2;				\
		I.AuxVal = 1;						\
		I.CarryVal = 1;						\
    } else {								\
		I.AuxVal = 0;						\
		I.CarryVal = 0;						\
    }										\
	I.regs.b[AL] &= 0x0F

#define BITOP_BYTE							\
	ModRM = FETCH;							\
	if (ModRM >= 0xc0) {					\
		tmp=I.regs.b[Mod_RM.RM.b[ModRM]];	\
	}										\
	else {									\
		(*GetEA[ModRM])();					\
		tmp=ReadByte(EA);					\
    }

#define BITOP_WORD							\
	ModRM = FETCH;							\
	if (ModRM >= 0xc0) {					\
		tmp=I.regs.w[Mod_RM.RM.w[ModRM]];	\
	}										\
	else {									\
		(*GetEA[ModRM])();					\
		tmp=ReadWord(EA);					\
    }

#define BIT_NOT								\
	if (tmp & (1<<tmp2))					\
		tmp &= ~(1<<tmp2);					\
	else									\
		tmp |= (1<<tmp2)


#define XchgAWReg(Reg) 						\
    WORD tmp; 								\
	tmp = I.regs.w[Reg]; 					\
	I.regs.w[Reg] = I.regs.w[AW]; 			\
	I.regs.w[AW] = tmp


#ifdef OPTIMIZE_CF
#define ROL_BYTE I.CarryVal = (dst&0x80)>>7; dst = (dst << 1)+CF
#define ROL_WORD I.CarryVal = (dst&0x8000)>>15; dst = (dst << 1)+CF
#define ROR_BYTE I.CarryVal = dst & 0x1; dst = (dst >> 1)+(CF<<7)
#define ROR_WORD I.CarryVal = dst & 0x1; dst = (dst >> 1)+(CF<<15)
#else
#define ROL_BYTE I.CarryVal = dst & 0x80; dst = (dst << 1)+CF
#define ROL_WORD I.CarryVal = dst & 0x8000; dst = (dst << 1)+CF
#define ROR_BYTE I.CarryVal = dst & 0x1; dst = (dst >> 1)+(CF<<7)
#define ROR_WORD I.CarryVal = dst & 0x1; dst = (dst >> 1)+(CF<<15)
#endif

#define ROLC_BYTE dst = (dst << 1) + CF; SetCFB(dst)
#define ROLC_WORD dst = (dst << 1) + CF; SetCFW(dst)
#define RORC_BYTE dst = (CF<<8)+dst; I.CarryVal = dst & 0x01; dst >>= 1
#define RORC_WORD dst = (CF<<16)+dst; I.CarryVal = dst & 0x01; dst >>= 1
#define SHL_BYTE(c) dst <<= c;	SetCFB(dst); SetSZPF_Byte(dst);	PutbackRMByte(ModRM,(BYTE)dst)
#define SHL_WORD(c) dst <<= c;	SetCFW(dst); SetSZPF_Word(dst);	PutbackRMWord(ModRM,(WORD)dst)
#define SHR_BYTE(c) dst >>= c-1; I.CarryVal = dst & 0x1; dst >>= 1; SetSZPF_Byte(dst); PutbackRMByte(ModRM,(BYTE)dst)
#define SHR_WORD(c) dst >>= c-1; I.CarryVal = dst & 0x1; dst >>= 1; SetSZPF_Word(dst); PutbackRMWord(ModRM,(WORD)dst)
#define SHRA_BYTE(c) dst = ((s8)dst) >> (c-1);	I.CarryVal = dst & 0x1;	dst = ((s8)((BYTE)dst)) >> 1; SetSZPF_Byte(dst); PutbackRMByte(ModRM,(BYTE)dst)
#define SHRA_WORD(c) dst = ((s16)dst) >> (c-1);	I.CarryVal = dst & 0x1;	dst = ((s16)((WORD)dst)) >> 1; SetSZPF_Word(dst); PutbackRMWord(ModRM,(WORD)dst)

#define DIVUB												\
	uresult = I.regs.w[AW];									\
	uresult2 = uresult % tmp;								\
	if ((uresult /= tmp) > 0xff) {							\
		nec_interrupt(0,0); break;							\
	} else {												\
		I.regs.b[AL] = uresult;								\
		I.regs.b[AH] = uresult2;							\
	}

#define DIVB												\
	result = (s16)I.regs.w[AW];							\
	result2 = result % (s16)((s8)tmp);					\
	if ((result /= (s16)((s8)tmp)) > 0xff) {			\
		nec_interrupt(0,0); break;							\
	} else {												\
		I.regs.b[AL] = result;								\
		I.regs.b[AH] = result2;								\
	}

#define DIVUW												\
	uresult = (((u32)I.regs.w[DW]) << 16) | I.regs.w[AW];\
	uresult2 = uresult % tmp;								\
	if ((uresult /= tmp) > 0xffff) {						\
		nec_interrupt(0,0); break;							\
	} else {												\
		I.regs.w[AW]=uresult;								\
		I.regs.w[DW]=uresult2;								\
	}

#define DIVW												\
	result = ((u32)I.regs.w[DW] << 16) + I.regs.w[AW];	\
	result2 = result % (s32)((s16)tmp);					\
	if ((result /= (s32)((s16)tmp)) > 0xffff) {			\
		nec_interrupt(0,0); break;							\
	} else {												\
		I.regs.w[AW]=result;								\
		I.regs.w[DW]=result2;								\
	}

#define ADD4S {												\
	int i,v1,v2,result;										\
	int count = (I.regs.b[CL]+1)/2;							\
	unsigned di = I.regs.w[IY];								\
	unsigned si = I.regs.w[IX];								\
	I.ZeroVal = I.CarryVal = 0;								\
	for (i=0;i<count;i++) {									\
		tmp = GetMemB(DS, si);								\
		tmp2 = GetMemB(ES, di);								\
		v1 = (tmp>>4)*10 + (tmp&0xf);						\
		v2 = (tmp2>>4)*10 + (tmp2&0xf);						\
		result = v1+v2+I.CarryVal;							\
		I.CarryVal = result > 99 ? 1 : 0;					\
		result = result % 100;								\
		v1 = ((result/10)<<4) | (result % 10);				\
		PutMemB(ES, di,v1);									\
		if (v1) I.ZeroVal = 1;								\
		si++;												\
		di++;												\
	}														\
}

#define SUB4S {												\
	int count = (I.regs.b[CL]+1)/2;							\
	int i,v1,v2,result;										\
    unsigned di = I.regs.w[IY];								\
	unsigned si = I.regs.w[IX];								\
	I.ZeroVal = I.CarryVal = 0;								\
	for (i=0;i<count;i++) {									\
		tmp = GetMemB(ES, di);								\
		tmp2 = GetMemB(DS, si);								\
		v1 = (tmp>>4)*10 + (tmp&0xf);						\
		v2 = (tmp2>>4)*10 + (tmp2&0xf);						\
		if (v1 < (v2+I.CarryVal)) {							\
			v1+=100;										\
			result = v1-(v2+I.CarryVal);					\
			I.CarryVal = 1;									\
		} else {											\
			result = v1-(v2+I.CarryVal);					\
			I.CarryVal = 0;									\
		}													\
		v1 = ((result/10)<<4) | (result % 10);				\
		PutMemB(ES, di,v1);									\
		if (v1) I.ZeroVal = 1;								\
		si++;												\
		di++;												\
	}														\
}

#define CMP4S {												\
	int count = (I.regs.b[CL]+1)/2;							\
	int i,v1,v2,result;										\
    unsigned di = I.regs.w[IY];								\
	unsigned si = I.regs.w[IX];								\
	I.ZeroVal = I.CarryVal = 0;								\
	for (i=0;i<count;i++) {									\
		tmp = GetMemB(ES, di);								\
		tmp2 = GetMemB(DS, si);								\
		v1 = (tmp>>4)*10 + (tmp&0xf);						\
		v2 = (tmp2>>4)*10 + (tmp2&0xf);						\
		if (v1 < (v2+I.CarryVal)) {							\
			v1+=100;										\
			result = v1-(v2+I.CarryVal);					\
			I.CarryVal = 1;									\
		} else {											\
			result = v1-(v2+I.CarryVal);					\
			I.CarryVal = 0;									\
		}													\
		v1 = ((result/10)<<4) | (result % 10);				\
		if (v1) I.ZeroVal = 1;								\
		si++;												\
		di++;												\
	}														\
}

#ifdef OPTIMIZE_CLK_JMP
#define JMPCLK(flag,clk)                    \
    if(flag) {                              \
        int tmp = (int)((s8)FETCH);       \
        I.ip = (WORD)(I.ip+tmp);            \
		PSTALL(8);                          \
        CLK(3);                             \
    } else {                                \
        I.ip++;                             \
        CLK(clk);                           \
    }
#else/*OPTIMIZE_CLK_JMP*/
#define JMP(flag)							\
	int tmp = (int)((s8)FETCH);			\
    if (flag) { 							\
		I.ip = (WORD)(I.ip+tmp);			\
		PSTALL(8);                          \
		CLK(3);	                            \
		return;								\
	}
#endif/*OPTIMIZE_CLK_JMP*/

void* nec_getRegPtr(int* pLen);
void  nec_reset(void*);
void  nec_setPipeline(int clk);


#endif // __NEC_H__

