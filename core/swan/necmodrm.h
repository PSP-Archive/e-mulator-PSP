static struct {
	struct {
		WREGS w[256];
		BREGS b[256];
	} reg;
	struct {
		WREGS w[256];
		BREGS b[256];
	} RM;
} Mod_RM;

#define RegWord(ModRM) I.regs.w[Mod_RM.reg.w[ModRM]]
#define RegByte(ModRM) I.regs.b[Mod_RM.reg.b[ModRM]]

#define GetRMWord(ModRM) \
	((ModRM) >= 0xc0 ? I.regs.w[Mod_RM.RM.w[ModRM]] : ( (*GetEA[ModRM])(), ReadWord( EA ) ))


#define PutbackRMWord(ModRM,val)                           \
{ 							                               \
	if (ModRM >= 0xc0) I.regs.w[Mod_RM.RM.w[ModRM]]=val;   \
    else WriteWord(EA,val);                                \
}

#define GetnextRMWord ReadWord((EA&0xf0000)|((EA+2)&0xffff))

#define PutRMWord(ModRM,val)				               \
{                                                          \
	if (ModRM >= 0xc0)                                     \
		I.regs.w[Mod_RM.RM.w[ModRM]]=val;                  \
	else {                                                 \
		(*GetEA[ModRM])();                                 \
		WriteWord( EA ,val);                               \
	}                                                      \
}

#define PutImmRMWord(ModRM)                                \
{                                                          \
	WORD val;                                              \
	if (ModRM >= 0xc0)                                     \
		FETCHWORD(I.regs.w[Mod_RM.RM.w[ModRM]])            \
	else {                                                 \
		(*GetEA[ModRM])();                                 \
		FETCHWORD(val)                                     \
		WriteWord( EA , val);                              \
	}                                                      \
}
	
#define GetRMByte(ModRM) \
	((ModRM) >= 0xc0 ? I.regs.b[Mod_RM.RM.b[ModRM]] : ReadByte( (*GetEA[ModRM])() ))
	
#define PutRMByte(ModRM,val)                               \
{                                                          \
	if (ModRM >= 0xc0)                                     \
		I.regs.b[Mod_RM.RM.b[ModRM]]=val;                  \
	else                                                   \
		WriteByte( (*GetEA[ModRM])() ,val);                \
}

#define PutImmRMByte(ModRM)                                \
{                                                          \
    if (ModRM >= 0xc0)                                     \
        I.regs.b[Mod_RM.RM.b[ModRM]]=FETCH;                \
    else {                                                 \
        (*GetEA[ModRM])();                                 \
		WriteByte( EA , FETCH );                           \
	}                                                      \
}
	
#define PutbackRMByte(ModRM,val)                  \
{                                                 \
	if (ModRM >= 0xc0)                            \
		I.regs.b[Mod_RM.RM.b[ModRM]]=val;         \
	else                                          \
		WriteByte(EA,val);                        \
}

#define DEF_br8							\
	u32 ModRM = FETCH;		        \
	u32 src = RegByte(ModRM);		\
    u32 dst = GetRMByte(ModRM)
    
#define DEF_wr16						\
	u32 ModRM = FETCH;               \
	u32 src = RegWord(ModRM);		\
    u32 dst = GetRMWord(ModRM)

#define DEF_r8b							\
	u32 ModRM = FETCH;           	\
	u32 dst = RegByte(ModRM);		\
    u32 src = GetRMByte(ModRM)

#define DEF_r16w						\
	u32 ModRM = FETCH;		        \
	u32 dst = RegWord(ModRM);		\
    u32 src = GetRMWord(ModRM)

#define DEF_ald8						\
	u32 src = FETCH;					\
	u32 dst = I.regs.b[AL]

#define DEF_axd16						\
	u32 src = FETCH; 				\
	u32 dst = I.regs.w[AW];			\
    src += (FETCH << 8)


#ifdef OPTIMIZE_CLK 
#define PutbackRMByteCLK(ModRM,val,v30MZm,v30MZ)  \
{                                                 \
    if (ModRM >= 0xc0) {                          \
        I.regs.b[Mod_RM.RM.b[ModRM]]=val;         \
        CLK(v30MZ);                               \
    } else {                                      \
        WriteByte(EA,val);                        \
        CLK(v30MZm);                              \
    }                                             \
}

#define PutbackRMWordCLK(ModRM,val,v30MZm,v30MZ)  \
{                                                 \
    if (ModRM >= 0xc0) {                          \
        I.regs.w[Mod_RM.RM.w[ModRM]]=val;         \
        CLK(v30MZ);                               \
    }                                             \
    else {                                        \
        WriteWord(EA,val);                        \
        CLK(v30MZm);                              \
    }                                             \
}

#define PutRMByteCLK(ModRM,val,v30MZm,v30MZ)      \
{                                                 \
    if (ModRM >= 0xc0) {                          \
		I.regs.b[Mod_RM.RM.b[ModRM]]=val;         \
        CLK(v30MZ);                               \
    }                                             \
    else {                                        \
		WriteByte( (*GetEA[ModRM])() ,val);       \
        CLK(v30MZm);                              \
    }                                             \
}

#define PutRMWordCLK(ModRM,val,v30MZm,v30MZ)      \
{                                                 \
    if (ModRM >= 0xc0) {                          \
		I.regs.w[Mod_RM.RM.w[ModRM]]=val;         \
        CLK(v30MZ);                               \
    }                                             \
	else {                                        \
		(*GetEA[ModRM])();                        \
		WriteWord( EA ,val);                      \
        CLK(v30MZm);                              \
	}                                             \
}

#endif/*OPTIMIZE_CLK*/
