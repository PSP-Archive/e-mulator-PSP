/* HuC6280 additional */
/* BRK addres = 0xFFF6 */

#define MC_Id(Rg)	K.W=MCZp(); \
			Rg.B.l=RdRAM(K.W);Rg.B.h=RdRAM(K.W+1)
#define MR_Id(Rg)	MC_Id(J);Rg=Rd6502(J.W)
//#define	M_FL2(Rg)	_P&=~(Z_FLAG|N_FLAG|V_FLAG);_P|=(Rg&0xc0)|ZNTable[Rg]
#define	TSB(Rg)	\
	_NF=_VF=Rg; \
	Rg|=_A; _ZF=Rg;
#define	TRB(Rg) \
	_NF=_VF=Rg; \
	Rg&=~_A; _ZF=Rg;
#define T_INIT() \
	word src,dist,length; \
	src  = Op6502w(_PC_); _PC_+=2; \
	dist = Op6502w(_PC_); _PC_+=2; \
	length = Op6502w(_PC_); _PC_+=2; \
	cycle=length*6
#define M_ADCx(Rg) \
 { byte *Mx=AdrRAM(_X+ZP); \
  if(_P&D_FLAG) \
  { \
    K.B.l=(*Mx&0x0F)+(Rg&0x0F)+(_P&C_FLAG); \
    K.B.h=(*Mx>>4)+(Rg>>4);/*+(K.B.l>>4);*/ \
    if(K.B.l>9) { K.B.l+=6;K.B.h++; } \
    if(K.B.h>9) K.B.h+=6; \
    *Mx=(K.B.l&0x0F)|(K.B.h<<4); \
    _P=(_P&~C_FLAG)|(K.B.h>15? C_FLAG:0); \
	_ZF=_NF=*Mx; \
	cycle++; \
  } \
  else \
  { \
    K.W=*Mx+Rg+(_P&C_FLAG); \
    _P&=~C_FLAG; \
    _P|=(K.B.h? C_FLAG:0); \
	_VF=(~(*Mx^Rg)&(*Mx^K.B.l))>>1; \
	_ZF=_NF=K.B.l; \
    *Mx=K.B.l; \
  } \
 }

#define M_ANDx(Rg)	*AdrRAM(_X+ZP)&=Rg;M_FL(RdRAM(_X+ZP))
#define M_EORx(Rg)	*AdrRAM(_X+ZP)^=Rg;M_FL(RdRAM(_X+ZP))
#define M_ORAx(Rg)	*AdrRAM(_X+ZP)|=Rg;M_FL(RdRAM(_X+ZP))

OPCODE_BEGIN(D4)OPCODE_END /* CSH set clock highspeed */
OPCODE_BEGIN(54)OPCODE_END /* CSL set clock lowspeed */

OPCODE_BEGIN(44) M_PUSH(_PC.B.h);M_PUSH(_PC.B.l); /* BSR * REL */
OPCODE_BEGIN(80) _PC_+=(offset)Op6502(_PC_)+1; OPCODE_END /* BRA * REL */
/* JMP ($ssss,x) */
OPCODE_BEGIN(7C)
	M_LDWORD(K);K.W+=_X;
	_PC.B.l = Op6502(K.W);
	_PC.B.h = Op6502(K.W+1);
  OPCODE_END

OPCODE_BEGIN(DA) M_PUSH(_X);OPCODE_END               /* PHX */
OPCODE_BEGIN(5A) M_PUSH(_Y);OPCODE_END               /* PHY */
OPCODE_BEGIN(FA) M_POP(_X);M_FL(_X);OPCODE_END       /* PLX */
OPCODE_BEGIN(7A) M_POP(_Y);M_FL(_Y);OPCODE_END       /* PLY */

OPCODE_BEGIN(62) _A=0; OPCODE_END /* CLA */
OPCODE_BEGIN(82) _X=0; OPCODE_END /* CLX */
OPCODE_BEGIN(C2) _Y=0; OPCODE_END /* CLY */

OPCODE_BEGIN(02) I=_X;_X=_Y;_Y=I;OPCODE_END  /* SXY */
OPCODE_BEGIN(22) I=_A;_A=_X;_X=I;OPCODE_END  /* SAX */
OPCODE_BEGIN(42) I=_A;_A=_Y;_Y=I;OPCODE_END  /* SAY */

OPCODE_BEGIN(3A) M_DEC(_A); OPCODE_END /* DEC A */
OPCODE_BEGIN(1A) M_INC(_A); OPCODE_END /* INC A */

OPCODE_BEGIN(72) MR_Id(I);M_ADC(I);OPCODE_END       /* ADC ($ss) INDIR */
OPCODE_BEGIN(32) MR_Id(I);M_AND(I);OPCODE_END       /* AND ($ss) INDIR */
OPCODE_BEGIN(D2) MR_Id(I);M_CMP(_A,I);OPCODE_END       /* CMP ($ss) INDIR */
OPCODE_BEGIN(52) MR_Id(I);M_EOR(I);OPCODE_END       /* EOR ($ss) INDIR */
OPCODE_BEGIN(B2) MR_Id(_A);M_FL(_A);OPCODE_END       /* LDA ($ss) INDIR */
OPCODE_BEGIN(12) MR_Id(I);M_ORA(I);OPCODE_END       /* ORA ($ss) INDIR */
OPCODE_BEGIN(92) MC_Id(J);Wr6502(J.W,_A);OPCODE_END  /* STA ($ss) INDIR */
OPCODE_BEGIN(F2) MR_Id(I);M_SBC(I);OPCODE_END      /* SBC ($ss) INDIR */

OPCODE_BEGIN(89) MR_Im(I);M_BIT(I);OPCODE_END       /* BIT #$ss IMM */
OPCODE_BEGIN(34) MR_Zx(I);M_BIT(I);OPCODE_END       /* BIT $ss,x ZP,x */
OPCODE_BEGIN(3C) MR_Ax(I);M_BIT(I);OPCODE_END       /* BIT $ssss,x ABS,x */

OPCODE_BEGIN(64) MW_Zp(0x00);OPCODE_END             /* STZ $ss ZP */
OPCODE_BEGIN(74) MW_Zx(0x00);OPCODE_END             /* STZ $ss,x ZP,x */
OPCODE_BEGIN(9C) MW_Ab(0x00);OPCODE_END             /* STZ $ssss ABS */
OPCODE_BEGIN(9E) MW_Ax(0x00);OPCODE_END             /* STZ $ssss,x ABS,x */

OPCODE_BEGIN(F4) /* SET */
	I=Op6502(_PC_++);
	cycle+=Cycles[I]+3;
	switch(I){
	case 0x65: MR_Zp(I);M_ADCx(I);break; /* ADC $ss ZP */
	case 0x6D: MR_Ab(I);M_ADCx(I);break; /* ADC $ssss ABS */
	case 0x69: MR_Im(I);M_ADCx(I);break; /* ADC #$ss IMM */
	case 0x75: MR_Zx(I);M_ADCx(I);break; /* ADC $ss,x ZP,x */
	case 0x79: MR_Ay(I);M_ADCx(I);break; /* ADC $ssss,y ABS,y */
	case 0x7D: MR_Ax(I);M_ADCx(I);break; /* ADC $ssss,x ABS,x */
	case 0x61: MR_Ix(I);M_ADCx(I);break; /* ADC ($ss,x) INDEXINDIR */
	case 0x71: MR_Iy(I);M_ADCx(I);break; /* ADC ($ss),y INDIRINDEX */
	case 0x72: MR_Id(I);M_ADCx(I);break; /* ADC ($ss) INDIR */

	case 0x25: MR_Zp(I);M_ANDx(I);break; /* AND $ss ZP */
	case 0x2D: MR_Ab(I);M_ANDx(I);break; /* AND $ssss ABS */
	case 0x29: MR_Im(I);M_ANDx(I);break; /* AND #$ss IMM */
	case 0x35: MR_Zx(I);M_ANDx(I);break; /* AND $ss,x ZP,x */
	case 0x39: MR_Ay(I);M_ANDx(I);break; /* AND $ssss,y ABS,y */
	case 0x3D: MR_Ax(I);M_ANDx(I);break; /* AND $ssss,x ABS,x */
	case 0x21: MR_Ix(I);M_ANDx(I);break; /* AND ($ss,x) INDEXINDIR */
	case 0x31: MR_Iy(I);M_ANDx(I);break; /* AND ($ss),y INDIRINDEX */
	case 0x32: MR_Id(I);M_ANDx(I);break; /* AND ($ss) INDIR */

	case 0x45: MR_Zp(I);M_EORx(I);break; /* EOR $ss ZP */
	case 0x4D: MR_Ab(I);M_EORx(I);break; /* EOR $ssss ABS */
	case 0x49: MR_Im(I);M_EORx(I);break; /* EOR #$ss IMM */
	case 0x55: MR_Zx(I);M_EORx(I);break; /* EOR $ss,x ZP,x */
	case 0x59: MR_Ay(I);M_EORx(I);break; /* EOR $ssss,y ABS,y */
	case 0x5D: MR_Ax(I);M_EORx(I);break; /* EOR $ssss,x ABS,x */
	case 0x41: MR_Ix(I);M_EORx(I);break; /* EOR ($ss,x) INDEXINDIR */
	case 0x51: MR_Iy(I);M_EORx(I);break; /* EOR ($ss),y INDIRINDEX */
	case 0x52: MR_Id(I);M_EORx(I);break; /* EOR ($ss) INDIR */

	case 0x05: MR_Zp(I);M_ORAx(I);break; /* ORA $ss ZP */
	case 0x0D: MR_Ab(I);M_ORAx(I);break; /* ORA $ssss ABS */
	case 0x09: MR_Im(I);M_ORAx(I);break; /* ORA #$ss IMM */
	case 0x15: MR_Zx(I);M_ORAx(I);break; /* ORA $ss,x ZP,x */
	case 0x19: MR_Ay(I);M_ORAx(I);break; /* ORA $ssss,y ABS,y */
	case 0x1D: MR_Ax(I);M_ORAx(I);break; /* ORA $ssss,x ABS,x */
	case 0x01: MR_Ix(I);M_ORAx(I);break; /* ORA ($ss,x) INDEXINDIR */
	case 0x11: MR_Iy(I);M_ORAx(I);break; /* ORA ($ss),y INDIRINDEX */
	case 0x12: MR_Id(I);M_ORAx(I);break; /* ORA ($ss) INDIR */

	default:
		//TRACE("no sense SET\n");
		cycle-=Cycles[I];
		_PC_--;
		break;
	}
	OPCODE_END

#if 1 /* VDC DIRECT ACCESS : for SGX support */
OPCODE_BEGIN(03) VDC_d_write(0,Op6502(_PC_++));OPCODE_END /* ST0 */
OPCODE_BEGIN(13) VDC_d_write(2,Op6502(_PC_++));OPCODE_END /* ST1 */
OPCODE_BEGIN(23) VDC_d_write(3,Op6502(_PC_++));OPCODE_END /* ST2 */
#else
OPCODE_BEGIN(03) IO_write(0,Op6502(_PC_++));OPCODE_END /* ST0 */
OPCODE_BEGIN(13) IO_write(2,Op6502(_PC_++));OPCODE_END /* ST1 */
OPCODE_BEGIN(23) IO_write(3,Op6502(_PC_++));OPCODE_END /* ST2 */
#endif

OPCODE_BEGIN(43) /* TMAi */
	I=Op6502(_PC_++);
	{int i;
	  for(i=0;i<8;i++,I>>=1){
		if (I&1) break;
	  }
	  _A = R->MPR[i];
	}
	OPCODE_END

OPCODE_BEGIN(53) /* TAMi */
	I=Op6502(_PC_++);
	{int i;
	 for(i=0;i<8;i++,I>>=1){
	 	if (I&1) {
	 		R->MPR[i]=_A;
	 		BANK_SET(i,_A);
	 	}
	 }
#ifdef _DEBUG
/*	 extern int ROM_size;
	 if (ROM_size <= _A && _A < 0xF7)
	 {
		TRACE("Illegal TAMi: A=%02X, PC=%04X\n", _A, _PC_);
		for (i = 0; i < 8; i++)
			TRACE("%02X ", R->MPR[i]);
		TRACE("\n");
	 }
*//*
	 if ((_A&0xF0) == 0x40)
	 {
		 for (i = 0; i < 8; i++)
			TRACE("%02X ", R->MPR[i]);
		TRACE("\n");
	 }
*/
#endif
	}
	OPCODE_END

OPCODE_BEGIN(C3) /* TDD */
	{ T_INIT();
	do {
		Wr6502(dist--,Rd6502(src));
		src--;
	} while(--length);
	}
	OPCODE_END

OPCODE_BEGIN(73) /* TII */
	{ T_INIT();
	do {
		Wr6502(dist++,Rd6502(src));
		src++;
	} while(--length);
	}
	OPCODE_END

OPCODE_BEGIN(E3) /* TIA */
	{ T_INIT();
	do {
		Wr6502(dist  ,Rd6502(src));
		src++;
		if (!(--length)) break;
		Wr6502(dist+1,Rd6502(src));
		src++;
	} while(--length);
	}
	OPCODE_END

OPCODE_BEGIN(F3) /* TAI */
	{ T_INIT();
	do {
		Wr6502(dist++,Rd6502(src));
		if (!(--length)) break;
		Wr6502(dist++,Rd6502(src+1));
	} while(--length);
	}
	OPCODE_END

OPCODE_BEGIN(D3) /* TIN */
	{ T_INIT();
	do {
		Wr6502(dist,Rd6502(src));
		src++;
	} while(--length);
	}
	OPCODE_END

OPCODE_BEGIN(14) MM_Zp(TRB);OPCODE_END /* TRB $ss ZP */
OPCODE_BEGIN(1C) MM_Ab(TRB);OPCODE_END /* TRB $ssss ABS */

OPCODE_BEGIN(04) MM_Zp(TSB);OPCODE_END /* TSB $ss ZP */
OPCODE_BEGIN(0C) MM_Ab(TSB);OPCODE_END /* TSB $ssss ABS */

OPCODE_BEGIN(83) /* TST #$ss,$ss IMM,ZP */
	I=Op6502(_PC_++); J.B.l=RdRAM(MCZp());
	_NF=_VF=J.B.l; _ZF=I&J.B.l;
	OPCODE_END
OPCODE_BEGIN(A3) /* TST #$ss,$ss,x IMM,ZP,x */
	I=Op6502(_PC_++); J.B.l=RdRAM(MCZx());
	_NF=_VF=J.B.l; _ZF=I&J.B.l;
	OPCODE_END
OPCODE_BEGIN(93) /* TST #$ss,$ssss IMM,ABS */
	I=Op6502(_PC_++); MR_Ab(J.B.l);
	_NF=_VF=J.B.l; _ZF=I&J.B.l;
	OPCODE_END
OPCODE_BEGIN(B3) /* TST #$ss,$ssss,x IMM,ABS,x */
	I=Op6502(_PC_++);MR_Ax(J.B.l);
	_NF=_VF=J.B.l; _ZF=I&J.B.l;
	OPCODE_END

OPCODE_BEGIN(0F) if (RdRAM(MCZp())&0x01) _PC_++; else { M_JR; } OPCODE_END /* BBRi */
OPCODE_BEGIN(1F) if (RdRAM(MCZp())&0x02) _PC_++; else { M_JR; } OPCODE_END
OPCODE_BEGIN(2F) if (RdRAM(MCZp())&0x04) _PC_++; else { M_JR; } OPCODE_END
OPCODE_BEGIN(3F) if (RdRAM(MCZp())&0x08) _PC_++; else { M_JR; } OPCODE_END
OPCODE_BEGIN(4F) if (RdRAM(MCZp())&0x10) _PC_++; else { M_JR; } OPCODE_END
OPCODE_BEGIN(5F) if (RdRAM(MCZp())&0x20) _PC_++; else { M_JR; } OPCODE_END
OPCODE_BEGIN(6F) if (RdRAM(MCZp())&0x40) _PC_++; else { M_JR; } OPCODE_END
OPCODE_BEGIN(7F) if (RdRAM(MCZp())&0x80) _PC_++; else { M_JR; } OPCODE_END

OPCODE_BEGIN(8F) if (RdRAM(MCZp())&0x01) { M_JR; } else _PC_++; OPCODE_END /* BBSi */
OPCODE_BEGIN(9F) if (RdRAM(MCZp())&0x02) { M_JR; } else _PC_++; OPCODE_END
OPCODE_BEGIN(AF) if (RdRAM(MCZp())&0x04) { M_JR; } else _PC_++; OPCODE_END
OPCODE_BEGIN(BF) if (RdRAM(MCZp())&0x08) { M_JR; } else _PC_++; OPCODE_END
OPCODE_BEGIN(CF) if (RdRAM(MCZp())&0x10) { M_JR; } else _PC_++; OPCODE_END
OPCODE_BEGIN(DF) if (RdRAM(MCZp())&0x20) { M_JR; } else _PC_++; OPCODE_END
OPCODE_BEGIN(EF) if (RdRAM(MCZp())&0x40) { M_JR; } else _PC_++; OPCODE_END
OPCODE_BEGIN(FF) if (RdRAM(MCZp())&0x80) { M_JR; } else _PC_++; OPCODE_END

#define	M_RMB(n)	*AdrRAM(MCZp())&=~n
#define	M_SMB(n)	*AdrRAM(MCZp())|=n

OPCODE_BEGIN(07) M_RMB(0x01); OPCODE_END /* RMBi */
OPCODE_BEGIN(17) M_RMB(0x02); OPCODE_END /* RMBi */
OPCODE_BEGIN(27) M_RMB(0x04); OPCODE_END /* RMBi */
OPCODE_BEGIN(37) M_RMB(0x08); OPCODE_END /* RMBi */
OPCODE_BEGIN(47) M_RMB(0x10); OPCODE_END /* RMBi */
OPCODE_BEGIN(57) M_RMB(0x20); OPCODE_END /* RMBi */
OPCODE_BEGIN(67) M_RMB(0x40); OPCODE_END /* RMBi */
OPCODE_BEGIN(77) M_RMB(0x80); OPCODE_END /* RMBi */

OPCODE_BEGIN(87) M_SMB(0x01); OPCODE_END /* SMBi */
OPCODE_BEGIN(97) M_SMB(0x02); OPCODE_END /* SMBi */
OPCODE_BEGIN(A7) M_SMB(0x04); OPCODE_END /* SMBi */
OPCODE_BEGIN(B7) M_SMB(0x08); OPCODE_END /* SMBi */
OPCODE_BEGIN(C7) M_SMB(0x10); OPCODE_END /* SMBi */
OPCODE_BEGIN(D7) M_SMB(0x20); OPCODE_END /* SMBi */
OPCODE_BEGIN(E7) M_SMB(0x40); OPCODE_END /* SMBi */
OPCODE_BEGIN(F7) M_SMB(0x80); OPCODE_END /* SMBi */
