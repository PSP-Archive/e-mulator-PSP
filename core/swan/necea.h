
static u32 EA;
static u16 EO;
static u16 E16;

static unsigned int EA_000(void) { EO=I.regs.w[BW]+I.regs.w[IX]; EA=DefaultBase(DS)+EO; return EA; }
static unsigned int EA_001(void) { EO=I.regs.w[BW]+I.regs.w[IY]; EA=DefaultBase(DS)+EO; return EA; }
static unsigned int EA_002(void) { EO=I.regs.w[BP]+I.regs.w[IX]; EA=DefaultBase(SS)+EO; return EA; }
static unsigned int EA_003(void) { EO=I.regs.w[BP]+I.regs.w[IY]; EA=DefaultBase(SS)+EO; return EA; }
static unsigned int EA_004(void) { EO=I.regs.w[IX]; EA=DefaultBase(DS)+EO; return EA; }
static unsigned int EA_005(void) { EO=I.regs.w[IY]; EA=DefaultBase(DS)+EO; return EA; }
static unsigned int EA_006(void) { EO=FETCH; EO+=FETCH<<8; EA=DefaultBase(DS)+EO; return EA; }
static unsigned int EA_007(void) { EO=I.regs.w[BW]; EA=DefaultBase(DS)+EO; return EA; }

static unsigned int EA_100(void) { EO=(I.regs.w[BW]+I.regs.w[IX]+(s8)FETCH); EA=DefaultBase(DS)+EO; return EA; }
static unsigned int EA_101(void) { EO=(I.regs.w[BW]+I.regs.w[IY]+(s8)FETCH); EA=DefaultBase(DS)+EO; return EA; }
static unsigned int EA_102(void) { EO=(I.regs.w[BP]+I.regs.w[IX]+(s8)FETCH); EA=DefaultBase(SS)+EO; return EA; }
static unsigned int EA_103(void) { EO=(I.regs.w[BP]+I.regs.w[IY]+(s8)FETCH); EA=DefaultBase(SS)+EO; return EA; }
static unsigned int EA_104(void) { EO=(I.regs.w[IX]+(s8)FETCH); EA=DefaultBase(DS)+EO; return EA; }
static unsigned int EA_105(void) { EO=(I.regs.w[IY]+(s8)FETCH); EA=DefaultBase(DS)+EO; return EA; }
static unsigned int EA_106(void) { EO=(I.regs.w[BP]+(s8)FETCH); EA=DefaultBase(SS)+EO; return EA; }
static unsigned int EA_107(void) { EO=(I.regs.w[BW]+(s8)FETCH); EA=DefaultBase(DS)+EO; return EA; }

static unsigned int EA_200(void) { E16=FETCH; E16+=FETCH<<8; EO=I.regs.w[BW]+I.regs.w[IX]+(s16)E16; EA=DefaultBase(DS)+EO; return EA; }
static unsigned int EA_201(void) { E16=FETCH; E16+=FETCH<<8; EO=I.regs.w[BW]+I.regs.w[IY]+(s16)E16; EA=DefaultBase(DS)+EO; return EA; }
static unsigned int EA_202(void) { E16=FETCH; E16+=FETCH<<8; EO=I.regs.w[BP]+I.regs.w[IX]+(s16)E16; EA=DefaultBase(SS)+EO; return EA; }
static unsigned int EA_203(void) { E16=FETCH; E16+=FETCH<<8; EO=I.regs.w[BP]+I.regs.w[IY]+(s16)E16; EA=DefaultBase(SS)+EO; return EA; }
static unsigned int EA_204(void) { E16=FETCH; E16+=FETCH<<8; EO=I.regs.w[IX]+(s16)E16; EA=DefaultBase(DS)+EO; return EA; }
static unsigned int EA_205(void) { E16=FETCH; E16+=FETCH<<8; EO=I.regs.w[IY]+(s16)E16; EA=DefaultBase(DS)+EO; return EA; }
static unsigned int EA_206(void) { E16=FETCH; E16+=FETCH<<8; EO=I.regs.w[BP]+(s16)E16; EA=DefaultBase(SS)+EO; return EA; }
static unsigned int EA_207(void) { E16=FETCH; E16+=FETCH<<8; EO=I.regs.w[BW]+(s16)E16; EA=DefaultBase(DS)+EO; return EA; }

static unsigned int (*GetEA[192])(void)={
	EA_000, EA_001, EA_002, EA_003, EA_004, EA_005, EA_006, EA_007,
	EA_000, EA_001, EA_002, EA_003, EA_004, EA_005, EA_006, EA_007,
	EA_000, EA_001, EA_002, EA_003, EA_004, EA_005, EA_006, EA_007,
	EA_000, EA_001, EA_002, EA_003, EA_004, EA_005, EA_006, EA_007,
	EA_000, EA_001, EA_002, EA_003, EA_004, EA_005, EA_006, EA_007,
	EA_000, EA_001, EA_002, EA_003, EA_004, EA_005, EA_006, EA_007,
	EA_000, EA_001, EA_002, EA_003, EA_004, EA_005, EA_006, EA_007,
	EA_000, EA_001, EA_002, EA_003, EA_004, EA_005, EA_006, EA_007,

	EA_100, EA_101, EA_102, EA_103, EA_104, EA_105, EA_106, EA_107,
	EA_100, EA_101, EA_102, EA_103, EA_104, EA_105, EA_106, EA_107,
	EA_100, EA_101, EA_102, EA_103, EA_104, EA_105, EA_106, EA_107,
	EA_100, EA_101, EA_102, EA_103, EA_104, EA_105, EA_106, EA_107,
	EA_100, EA_101, EA_102, EA_103, EA_104, EA_105, EA_106, EA_107,
	EA_100, EA_101, EA_102, EA_103, EA_104, EA_105, EA_106, EA_107,
	EA_100, EA_101, EA_102, EA_103, EA_104, EA_105, EA_106, EA_107,
	EA_100, EA_101, EA_102, EA_103, EA_104, EA_105, EA_106, EA_107,

	EA_200, EA_201, EA_202, EA_203, EA_204, EA_205, EA_206, EA_207,
	EA_200, EA_201, EA_202, EA_203, EA_204, EA_205, EA_206, EA_207,
	EA_200, EA_201, EA_202, EA_203, EA_204, EA_205, EA_206, EA_207,
	EA_200, EA_201, EA_202, EA_203, EA_204, EA_205, EA_206, EA_207,
	EA_200, EA_201, EA_202, EA_203, EA_204, EA_205, EA_206, EA_207,
	EA_200, EA_201, EA_202, EA_203, EA_204, EA_205, EA_206, EA_207,
	EA_200, EA_201, EA_202, EA_203, EA_204, EA_205, EA_206, EA_207,
	EA_200, EA_201, EA_202, EA_203, EA_204, EA_205, EA_206, EA_207
};
