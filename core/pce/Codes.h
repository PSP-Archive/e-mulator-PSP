/** M6502: portable 6502 emulator ****************************/
/**                                                         **/
/**                          Codes.h                        **/
/**                                                         **/
/** This file contains implementation for the main table of **/
/** 6502 commands. It is included from 6502.c.              **/
/**                                                         **/
/** Copyright (C) Marat Fayzullin 1996                      **/
/**               Alex Krasivsky  1996                      **/
/** Modyfied      BERO            1998                      **/
/**     You are not allowed to distribute this software     **/
/**     commercially. Please, notify me, if you make any    **/
/**     changes to this file.                               **/
/*************************************************************/

OPCODE_BEGIN(10) if(_NF&N_FLAG) _PC_++; else { M_JR; } OPCODE_END /* BPL * REL */
OPCODE_BEGIN(30) if(_NF&N_FLAG) { M_JR; } else _PC_++; OPCODE_END /* BMI * REL */
OPCODE_BEGIN(D0) if(!_ZF)       _PC_++; else { M_JR; } OPCODE_END /* BNE * REL */
OPCODE_BEGIN(F0) if(!_ZF)       { M_JR; } else _PC_++; OPCODE_END /* BEQ * REL */
OPCODE_BEGIN(90) if(_P&C_FLAG)  _PC_++; else { M_JR; } OPCODE_END /* BCC * REL */
OPCODE_BEGIN(B0) if(_P&C_FLAG)  { M_JR; } else _PC_++; OPCODE_END /* BCS * REL */
OPCODE_BEGIN(50) if(_VF&V_FLAG) _PC_++; else { M_JR; } OPCODE_END /* BVC * REL */
OPCODE_BEGIN(70) if(_VF&V_FLAG) { M_JR; } else _PC_++; OPCODE_END /* BVS * REL */

/* RTI */
OPCODE_BEGIN(40)
  I=_P;
  M_POP_P(_P);
  if((_IRequest!=INT_NONE)&&(I&I_FLAG)&&!(_P&I_FLAG))
  {
    _AfterCLI=1;
    _IBackup=_ICount;
    _ICount=0;
  }
  M_POP(_PC.B.l);M_POP(_PC.B.h);
  after_cli(R);
  OPCODE_END

/* RTS */
OPCODE_BEGIN(60)
  M_POP(_PC.B.l);M_POP(_PC.B.h);_PC_++;OPCODE_END

/* JSR $ssss ABS */
OPCODE_BEGIN(20)
  K.B.l=Op6502(_PC_++);
  K.B.h=Op6502(_PC_);
  M_PUSH(_PC.B.h);
  M_PUSH(_PC.B.l);
  _PC=K;OPCODE_END

/* JMP $ssss ABS */
OPCODE_BEGIN(4C) M_LDWORD(K);_PC=K;OPCODE_END

/* JMP ($ssss) ABDINDIR */
OPCODE_BEGIN(6C)
  M_LDWORD(K);
  _PC.B.l=Op6502(K.W++);
  _PC.B.h=Op6502(K.W);
  OPCODE_END

/* BRK */
OPCODE_BEGIN(00)
  _PC_++;
  M_PUSH(_PC.B.h);
  M_PUSH(_PC.B.l);
  M_PUSH_P(_P&~T_FLAG|B_FLAG);
  _P=(_P|I_FLAG)&~D_FLAG;
  _PC.B.l=Op6502(VEC_BRK);
  _PC.B.h=Op6502(VEC_BRK+1);
//  TRACE("BRK instruction\n");
  OPCODE_END

/* CLI */
OPCODE_BEGIN(58)
  if((_IRequest!=INT_NONE)&&(_P&I_FLAG))
  {
    _AfterCLI=1;
    _IBackup=_ICount;
    _ICount=0;
  }
  _P&=~I_FLAG;
  after_cli(R);
  OPCODE_END

/* PLP */
OPCODE_BEGIN(28)
  M_POP_P(I);
  if((_IRequest!=INT_NONE)&&((I^_P)&~I&I_FLAG))
  {
    _AfterCLI=1;
    _IBackup=_ICount;
    _ICount=0;
  }
  _P=I;
  after_cli(R);
  OPCODE_END

OPCODE_BEGIN(08) M_PUSH_P(_P&~T_FLAG|B_FLAG);OPCODE_END               /* PHP */
OPCODE_BEGIN(18) _P&=~C_FLAG;OPCODE_END              /* CLC */
OPCODE_BEGIN(B8) _VF=0;OPCODE_END              /* CLV */
OPCODE_BEGIN(D8) _P&=~D_FLAG;OPCODE_END              /* CLD */
OPCODE_BEGIN(38) _P|=C_FLAG;OPCODE_END               /* SEC */
OPCODE_BEGIN(F8) _P|=D_FLAG;OPCODE_END               /* SED */
OPCODE_BEGIN(78) _P|=I_FLAG;OPCODE_END               /* SEI */
OPCODE_BEGIN(48) M_PUSH(_A);OPCODE_END               /* PHA */
OPCODE_BEGIN(68) M_POP(_A);M_FL(_A);OPCODE_END     /* PLA */
OPCODE_BEGIN(98) _A=_Y;M_FL(_A);OPCODE_END       /* TYA */
OPCODE_BEGIN(A8) _Y=_A;M_FL(_Y);OPCODE_END       /* TAY */
OPCODE_BEGIN(C8) _Y++;M_FL(_Y);OPCODE_END          /* INY */
OPCODE_BEGIN(88) _Y--;M_FL(_Y);OPCODE_END          /* DEY */
OPCODE_BEGIN(8A) _A=_X;M_FL(_A);OPCODE_END       /* TXA */
OPCODE_BEGIN(AA) _X=_A;M_FL(_X);OPCODE_END       /* TAX */
OPCODE_BEGIN(E8) _X++;M_FL(_X);OPCODE_END          /* INX */
OPCODE_BEGIN(CA) _X--;M_FL(_X);OPCODE_END          /* DEX */
OPCODE_BEGIN(EA) OPCODE_END                            /* NOP */
OPCODE_BEGIN(9A) _S=_X;OPCODE_END                  /* TXS */
OPCODE_BEGIN(BA) _X=_S;M_FL(_X);OPCODE_END                  /* TSX */

OPCODE_BEGIN(24) MR_Zp(I);M_BIT(I);OPCODE_END       /* BIT $ss ZP */
OPCODE_BEGIN(2C) MR_Ab(I);M_BIT(I);OPCODE_END       /* BIT $ssss ABS */

OPCODE_BEGIN(05) MR_Zp(I);M_ORA(I);OPCODE_END       /* ORA $ss ZP */
OPCODE_BEGIN(06) MM_Zp(M_ASL);OPCODE_END            /* ASL $ss ZP */
OPCODE_BEGIN(25) MR_Zp(I);M_AND(I);OPCODE_END       /* AND $ss ZP */
OPCODE_BEGIN(26) MM_Zp(M_ROL);OPCODE_END            /* ROL $ss ZP */
OPCODE_BEGIN(45) MR_Zp(I);M_EOR(I);OPCODE_END       /* EOR $ss ZP */
OPCODE_BEGIN(46) MM_Zp(M_LSR);OPCODE_END            /* LSR $ss ZP */
OPCODE_BEGIN(65) MR_Zp(I);M_ADC(I);OPCODE_END       /* ADC $ss ZP */
OPCODE_BEGIN(66) MM_Zp(M_ROR);OPCODE_END            /* ROR $ss ZP */
OPCODE_BEGIN(84) MW_Zp(_Y);OPCODE_END             /* STY $ss ZP */
OPCODE_BEGIN(85) MW_Zp(_A);OPCODE_END             /* STA $ss ZP */
OPCODE_BEGIN(86) MW_Zp(_X);OPCODE_END             /* STX $ss ZP */
OPCODE_BEGIN(A4) MR_Zp(_Y);M_FL(_Y);OPCODE_END  /* LDY $ss ZP */
OPCODE_BEGIN(A5) MR_Zp(_A);M_FL(_A);OPCODE_END  /* LDA $ss ZP */
OPCODE_BEGIN(A6) MR_Zp(_X);M_FL(_X);OPCODE_END  /* LDX $ss ZP */
OPCODE_BEGIN(C4) MR_Zp(I);M_CMP(_Y,I);OPCODE_END  /* CPY $ss ZP */
OPCODE_BEGIN(C5) MR_Zp(I);M_CMP(_A,I);OPCODE_END  /* CMP $ss ZP */
OPCODE_BEGIN(C6) MM_Zp(M_DEC);OPCODE_END            /* DEC $ss ZP */
OPCODE_BEGIN(E4) MR_Zp(I);M_CMP(_X,I);OPCODE_END  /* CPX $ss ZP */
OPCODE_BEGIN(E5) MR_Zp(I);M_SBC(I);OPCODE_END       /* SBC $ss ZP */
OPCODE_BEGIN(E6) MM_Zp(M_INC);OPCODE_END            /* INC $ss ZP */

OPCODE_BEGIN(0D) MR_Ab(I);M_ORA(I);OPCODE_END       /* ORA $ssss ABS */
OPCODE_BEGIN(0E) MM_Ab(M_ASL);OPCODE_END            /* ASL $ssss ABS */
OPCODE_BEGIN(2D) MR_Ab(I);M_AND(I);OPCODE_END       /* AND $ssss ABS */
OPCODE_BEGIN(2E) MM_Ab(M_ROL);OPCODE_END            /* ROL $ssss ABS */
OPCODE_BEGIN(4D) MR_Ab(I);M_EOR(I);OPCODE_END       /* EOR $ssss ABS */
OPCODE_BEGIN(4E) MM_Ab(M_LSR);OPCODE_END            /* LSR $ssss ABS */
OPCODE_BEGIN(6D) MR_Ab(I);M_ADC(I);OPCODE_END       /* ADC $ssss ABS */
OPCODE_BEGIN(6E) MM_Ab(M_ROR);OPCODE_END            /* ROR $ssss ABS */
OPCODE_BEGIN(8C) MW_Ab(_Y);OPCODE_END             /* STY $ssss ABS */
OPCODE_BEGIN(8D) MW_Ab(_A);OPCODE_END             /* STA $ssss ABS */
OPCODE_BEGIN(8E) MW_Ab(_X);OPCODE_END             /* STX $ssss ABS */
OPCODE_BEGIN(AC) MR_Ab(_Y);M_FL(_Y);OPCODE_END  /* LDY $ssss ABS */
OPCODE_BEGIN(AD) MR_Ab(_A);M_FL(_A);OPCODE_END  /* LDA $ssss ABS */
OPCODE_BEGIN(AE) MR_Ab(_X);M_FL(_X);OPCODE_END  /* LDX $ssss ABS */
OPCODE_BEGIN(CC) MR_Ab(I);M_CMP(_Y,I);OPCODE_END  /* CPY $ssss ABS */
OPCODE_BEGIN(CD) MR_Ab(I);M_CMP(_A,I);OPCODE_END  /* CMP $ssss ABS */
OPCODE_BEGIN(CE) MM_Ab(M_DEC);OPCODE_END            /* DEC $ssss ABS */
OPCODE_BEGIN(EC) MR_Ab(I);M_CMP(_X,I);OPCODE_END  /* CPX $ssss ABS */
OPCODE_BEGIN(ED) MR_Ab(I);M_SBC(I);OPCODE_END       /* SBC $ssss ABS */
OPCODE_BEGIN(EE) MM_Ab(M_INC);OPCODE_END            /* INC $ssss ABS */

OPCODE_BEGIN(09) MR_Im(I);M_ORA(I);OPCODE_END       /* ORA #$ss IMM */
OPCODE_BEGIN(29) MR_Im(I);M_AND(I);OPCODE_END       /* AND #$ss IMM */
OPCODE_BEGIN(49) MR_Im(I);M_EOR(I);OPCODE_END       /* EOR #$ss IMM */
OPCODE_BEGIN(69) MR_Im(I);M_ADC(I);OPCODE_END       /* ADC #$ss IMM */
OPCODE_BEGIN(A0) MR_Im(_Y);M_FL(_Y);OPCODE_END  /* LDY #$ss IMM */
OPCODE_BEGIN(A2) MR_Im(_X);M_FL(_X);OPCODE_END  /* LDX #$ss IMM */
OPCODE_BEGIN(A9) MR_Im(_A);M_FL(_A);OPCODE_END  /* LDA #$ss IMM */
OPCODE_BEGIN(C0) MR_Im(I);M_CMP(_Y,I);OPCODE_END  /* CPY #$ss IMM */
OPCODE_BEGIN(C9) MR_Im(I);M_CMP(_A,I);OPCODE_END  /* CMP #$ss IMM */
OPCODE_BEGIN(E0) MR_Im(I);M_CMP(_X,I);OPCODE_END  /* CPX #$ss IMM */
OPCODE_BEGIN(E9) MR_Im(I);M_SBC(I);OPCODE_END       /* SBC #$ss IMM */

OPCODE_BEGIN(15) MR_Zx(I);M_ORA(I);OPCODE_END       /* ORA $ss,x ZP,x */
OPCODE_BEGIN(16) MM_Zx(M_ASL);OPCODE_END            /* ASL $ss,x ZP,x */
OPCODE_BEGIN(35) MR_Zx(I);M_AND(I);OPCODE_END       /* AND $ss,x ZP,x */
OPCODE_BEGIN(36) MM_Zx(M_ROL);OPCODE_END            /* ROL $ss,x ZP,x */
OPCODE_BEGIN(55) MR_Zx(I);M_EOR(I);OPCODE_END       /* EOR $ss,x ZP,x */
OPCODE_BEGIN(56) MM_Zx(M_LSR);OPCODE_END            /* LSR $ss,x ZP,x */
OPCODE_BEGIN(75) MR_Zx(I);M_ADC(I);OPCODE_END       /* ADC $ss,x ZP,x */
OPCODE_BEGIN(76) MM_Zx(M_ROR);OPCODE_END            /* ROR $ss,x ZP,x */
OPCODE_BEGIN(94) MW_Zx(_Y);OPCODE_END             /* STY $ss,x ZP,x */
OPCODE_BEGIN(95) MW_Zx(_A);OPCODE_END             /* STA $ss,x ZP,x */
OPCODE_BEGIN(96) MW_Zy(_X);OPCODE_END             /* STX $ss,y ZP,y */
OPCODE_BEGIN(B4) MR_Zx(_Y);M_FL(_Y);OPCODE_END  /* LDY $ss,x ZP,x */
OPCODE_BEGIN(B5) MR_Zx(_A);M_FL(_A);OPCODE_END  /* LDA $ss,x ZP,x */
OPCODE_BEGIN(B6) MR_Zy(_X);M_FL(_X);OPCODE_END  /* LDX $ss,y ZP,y */
OPCODE_BEGIN(D5) MR_Zx(I);M_CMP(_A,I);OPCODE_END  /* CMP $ss,x ZP,x */
OPCODE_BEGIN(D6) MM_Zx(M_DEC);OPCODE_END            /* DEC $ss,x ZP,x */
OPCODE_BEGIN(F5) MR_Zx(I);M_SBC(I);OPCODE_END       /* SBC $ss,x ZP,x */
OPCODE_BEGIN(F6) MM_Zx(M_INC);OPCODE_END            /* INC $ss,x ZP,x */

OPCODE_BEGIN(19) MR_Ay(I);M_ORA(I);OPCODE_END       /* ORA $ssss,y ABS,y */
OPCODE_BEGIN(1D) MR_Ax(I);M_ORA(I);OPCODE_END       /* ORA $ssss,x ABS,x */
OPCODE_BEGIN(1E) MM_Ax(M_ASL);OPCODE_END            /* ASL $ssss,x ABS,x */
OPCODE_BEGIN(39) MR_Ay(I);M_AND(I);OPCODE_END       /* AND $ssss,y ABS,y */
OPCODE_BEGIN(3D) MR_Ax(I);M_AND(I);OPCODE_END       /* AND $ssss,x ABS,x */
OPCODE_BEGIN(3E) MM_Ax(M_ROL);OPCODE_END            /* ROL $ssss,x ABS,x */
OPCODE_BEGIN(59) MR_Ay(I);M_EOR(I);OPCODE_END       /* EOR $ssss,y ABS,y */
OPCODE_BEGIN(5D) MR_Ax(I);M_EOR(I);OPCODE_END       /* EOR $ssss,x ABS,x */
OPCODE_BEGIN(5E) MM_Ax(M_LSR);OPCODE_END            /* LSR $ssss,x ABS,x */
OPCODE_BEGIN(79) MR_Ay(I);M_ADC(I);OPCODE_END       /* ADC $ssss,y ABS,y */
OPCODE_BEGIN(7D) MR_Ax(I);M_ADC(I);OPCODE_END       /* ADC $ssss,x ABS,x */
OPCODE_BEGIN(7E) MM_Ax(M_ROR);OPCODE_END            /* ROR $ssss,x ABS,x */
OPCODE_BEGIN(99) MW_Ay(_A);OPCODE_END             /* STA $ssss,y ABS,y */
OPCODE_BEGIN(9D) MW_Ax(_A);OPCODE_END             /* STA $ssss,x ABS,x */
OPCODE_BEGIN(B9) MR_Ay(_A);M_FL(_A);OPCODE_END  /* LDA $ssss,y ABS,y */
OPCODE_BEGIN(BC) MR_Ax(_Y);M_FL(_Y);OPCODE_END  /* LDY $ssss,x ABS,x */
OPCODE_BEGIN(BD) MR_Ax(_A);M_FL(_A);OPCODE_END  /* LDA $ssss,x ABS,x */
OPCODE_BEGIN(BE) MR_Ay(_X);M_FL(_X);OPCODE_END  /* LDX $ssss,y ABS,y */
OPCODE_BEGIN(D9) MR_Ay(I);M_CMP(_A,I);OPCODE_END  /* CMP $ssss,y ABS,y */
OPCODE_BEGIN(DD) MR_Ax(I);M_CMP(_A,I);OPCODE_END  /* CMP $ssss,x ABS,x */
OPCODE_BEGIN(DE) MM_Ax(M_DEC);OPCODE_END            /* DEC $ssss,x ABS,x */
OPCODE_BEGIN(F9) MR_Ay(I);M_SBC(I);OPCODE_END       /* SBC $ssss,y ABS,y */
OPCODE_BEGIN(FD) MR_Ax(I);M_SBC(I);OPCODE_END       /* SBC $ssss,x ABS,x */
OPCODE_BEGIN(FE) MM_Ax(M_INC);OPCODE_END            /* INC $ssss,x ABS,x */

OPCODE_BEGIN(01) MR_Ix(I);M_ORA(I);OPCODE_END       /* ORA ($ss,x) INDEXINDIR */
OPCODE_BEGIN(11) MR_Iy(I);M_ORA(I);OPCODE_END       /* ORA ($ss),y INDIRINDEX */
OPCODE_BEGIN(21) MR_Ix(I);M_AND(I);OPCODE_END       /* AND ($ss,x) INDEXINDIR */
OPCODE_BEGIN(31) MR_Iy(I);M_AND(I);OPCODE_END       /* AND ($ss),y INDIRINDEX */
OPCODE_BEGIN(41) MR_Ix(I);M_EOR(I);OPCODE_END       /* EOR ($ss,x) INDEXINDIR */
OPCODE_BEGIN(51) MR_Iy(I);M_EOR(I);OPCODE_END       /* EOR ($ss),y INDIRINDEX */
OPCODE_BEGIN(61) MR_Ix(I);M_ADC(I);OPCODE_END       /* ADC ($ss,x) INDEXINDIR */
OPCODE_BEGIN(71) MR_Iy(I);M_ADC(I);OPCODE_END       /* ADC ($ss),y INDIRINDEX */
OPCODE_BEGIN(81) MW_Ix(_A);OPCODE_END             /* STA ($ss,x) INDEXINDIR */
OPCODE_BEGIN(91) MW_Iy(_A);OPCODE_END             /* STA ($ss),y INDIRINDEX */
OPCODE_BEGIN(A1) MR_Ix(_A);M_FL(_A);OPCODE_END  /* LDA ($ss,x) INDEXINDIR */
OPCODE_BEGIN(B1) MR_Iy(_A);M_FL(_A);OPCODE_END  /* LDA ($ss),y INDIRINDEX */
OPCODE_BEGIN(C1) MR_Ix(I);M_CMP(_A,I);OPCODE_END  /* CMP ($ss,x) INDEXINDIR */
OPCODE_BEGIN(D1) MR_Iy(I);M_CMP(_A,I);OPCODE_END  /* CMP ($ss),y INDIRINDEX */
OPCODE_BEGIN(E1) MR_Ix(I);M_SBC(I);OPCODE_END       /* SBC ($ss,x) INDEXINDIR */
OPCODE_BEGIN(F1) MR_Iy(I);M_SBC(I);OPCODE_END       /* SBC ($ss),y INDIRINDEX */

OPCODE_BEGIN(0A) M_ASL(_A);OPCODE_END             /* ASL a ACC */
OPCODE_BEGIN(2A) M_ROL(_A);OPCODE_END             /* ROL a ACC */
OPCODE_BEGIN(4A) M_LSR(_A);OPCODE_END             /* LSR a ACC */
OPCODE_BEGIN(6A) M_ROR(_A);OPCODE_END             /* ROR a ACC */
#if 0
default:
  if(_TrapBadOps) {
/*    TRACE
    (
      "[M6502 %lX] Unrecognized instruction: $%02X at PC=$%04X\n",
      _User,Op6502(_PC_-1),(word)(_PC_-1)
    );
*/    _Trace=1;
  }
  break;
#endif

// Unused opcodes
OPCODE_BEGIN(FC) if(_TrapBadOps) {_Trace=1;} OPCODE_END
OPCODE_BEGIN(FB) if(_TrapBadOps) {_Trace=1;} OPCODE_END
OPCODE_BEGIN(EB) if(_TrapBadOps) {_Trace=1;} OPCODE_END
OPCODE_BEGIN(E2) if(_TrapBadOps) {_Trace=1;} OPCODE_END
OPCODE_BEGIN(DC) if(_TrapBadOps) {_Trace=1;} OPCODE_END
OPCODE_BEGIN(DB) if(_TrapBadOps) {_Trace=1;} OPCODE_END
OPCODE_BEGIN(CB) if(_TrapBadOps) {_Trace=1;} OPCODE_END
OPCODE_BEGIN(BB) if(_TrapBadOps) {_Trace=1;} OPCODE_END
OPCODE_BEGIN(AB) if(_TrapBadOps) {_Trace=1;} OPCODE_END
OPCODE_BEGIN(9B) if(_TrapBadOps) {_Trace=1;} OPCODE_END
OPCODE_BEGIN(8B) if(_TrapBadOps) {_Trace=1;} OPCODE_END
OPCODE_BEGIN(7B) if(_TrapBadOps) {_Trace=1;} OPCODE_END
OPCODE_BEGIN(6B) if(_TrapBadOps) {_Trace=1;} OPCODE_END
OPCODE_BEGIN(63) if(_TrapBadOps) {_Trace=1;} OPCODE_END
OPCODE_BEGIN(5C) if(_TrapBadOps) {_Trace=1;} OPCODE_END
OPCODE_BEGIN(5B) if(_TrapBadOps) {_Trace=1;} OPCODE_END
OPCODE_BEGIN(4B) if(_TrapBadOps) {_Trace=1;} OPCODE_END
OPCODE_BEGIN(3B) if(_TrapBadOps) {_Trace=1;} OPCODE_END
OPCODE_BEGIN(33) if(_TrapBadOps) {_Trace=1;} OPCODE_END
OPCODE_BEGIN(2B) if(_TrapBadOps) {_Trace=1;} OPCODE_END
OPCODE_BEGIN(1B) if(_TrapBadOps) {_Trace=1;} OPCODE_END
OPCODE_BEGIN(0B) if(_TrapBadOps) {_Trace=1;} OPCODE_END

