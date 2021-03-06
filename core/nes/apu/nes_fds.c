/*
	NSFplug

*/
#include "nes_apu.h"
#include "nes_extsound.h"
/*
#define OPT_MOD_PHASE_REFRESH 0
#define OPT_CAR_PHASE_REFRESH 1
#define OPT_USE_PWM 2
#define OPT_END 3
*/
extern apu_t g_apu_t;

// memo 
// clockは 21477270/12 = 1789772.5
// rateは 44100
// getaは 1789772.5/44100=40.584410430839002267573696

#define DOUBLE_SHIFT 20
#define FDS_SOUND_geta_44100 42555839 // 2^20(1024576)*40.584410430839002267573696=42555838.751927437641723355856896
#define FDS_SOUND_geta_22050 85111678
#define FDS_SOUND_geta_11025 170223355
/*
#define DOUBLE_SHIFT 21
#define FDS_SOUND_geta_44100 85111678 // 2^21*40.584410430839002267573696=85111678
#define FDS_SOUND_geta_22050 170223355
#define FDS_SOUND_geta_11025 340446710
*/

typedef struct {
//	s16 wave[64];

//	int option[OPT_END];
//	int mask;
//	u32 clock;   // 1789772.5
//	u32 rate;	// 44100
	u8 write_enable[2];
	u8 envelope_disable;
	u8 sound_disable;

	u32 ecounter[2];
	u32 ecounter_incr[2];
	u32 envelope_output[2];
	u32 envelope_mode[2];
	u32 envelope_amount[2];
	u32 envelope_speed;

//	Counter ccounter;

	s32 opout[2];
	s32 opval[2];
	u8 reg[0x100];

//	TrackInfoFDS trkinfo;

	/* var[0] = Carrior, var[1] = modulator */
//	double geta;
	u32 geta; 
	u32 freq[2];
	u32 incr[2];
	u32 phase[2];
	u32 pcounter[2];
	char wave[2][64];
	u8 volume;
	u32 wavidx;
//	  s32 calc ();

}FDSSOUND;

FDSSOUND FDS_SOUND;


/*
void FDS_SetClock(u32 c){
	FDS_SOUND.clock = c ;
}
*/

void FDS_SetRate (u32 r){
//	FDS_SOUND.rate = r ? r : 44100;
	switch (r) {
	case 22050:
		FDS_SOUND.geta = FDS_SOUND_geta_22050;
		break;
	case 11025:
		FDS_SOUND.geta = FDS_SOUND_geta_11025;
		break;
	case 44100:
	default:
		FDS_SOUND.geta = FDS_SOUND_geta_44100;
	}
}


void FDS_Write (u32 adr, u8 val);
/*
int COUNTER_SHIFT;
u32 val, next, step, max;

void counter_init(u32 m){
	COUNTER_SHIFT = 20;
	val = 0;
	step = 0;

	step = FDS_SOUND.geta;
	max = m << COUNTER_SHIFT;
	val = 0;
}

u8 iup(){
	if (val + step >= max){
		val = val + step - max;
		return TRUE;
	}
	else {
		val += step;
		return FALSE;
	}
}


u32 value(){
	return val >> COUNTER_SHIFT;
}
*/
    	
/*
	void Reset ();
	bool Write (u32 adr, u32 val, u32 id=0);
	bool Read (u32 adr, u32 & val, u32 id=0);
	u32 Render (s32 b[2]);
	void SetRate (double);
	void SetClock (double);
	void SetOption (int, int);
	void SetMask(int m){ mask = m&1; }
	ITrackInfo *GetTrackInfo(int trk);
*/


/*	NES_FDS::~NES_FDS ()
  {
	option[OPT_MOD_PHASE_REFRESH] = true;
	option[OPT_CAR_PHASE_REFRESH] = true;
	option[OPT_USE_PWM] = true;
  }
*/





/*
void SetOption (int id, int val){
	if(id<OPT_END)
		FDS_SOUND.option[id] = val;
}
*/

void FDS_Reset(void){
	int i;

	FDS_SOUND.wavidx = 0;
/*	switch (FDS_SOUND.rate) {
	case 22050:
		FDS_SOUND.geta = FDS_SOUND_geta_22050;
		break;
	case 11025:
		FDS_SOUND.geta = FDS_SOUND_geta_11025;
		break;
	case 44100:
	default:
		FDS_SOUND.geta = FDS_SOUND_geta_44100;
	}*/

//	FDS_SOUND.geta = FDS_SOUND.clock / FDS_SOUND.rate;
//	FDS_SOUND.mask = 0;

	for (i = 0x4040; i < 0x4100; i++)
	  FDS_Write(i, 0);

	FDS_Write(0x408A, 232);

	for (i = 0; i < 0x40; i++)
	{
	  FDS_SOUND.wave[0][i] = 0;
	  FDS_SOUND.wave[1][i] = 0;
	}

	FDS_SOUND.volume = 0;

	for (i = 0; i < 2; i++)
	{
	  FDS_SOUND.pcounter[i] = 0;
	  FDS_SOUND.phase[i] = 0;
	  FDS_SOUND.opout[i] = 0;
	  FDS_SOUND.opval[i] = 0;
	  FDS_SOUND.envelope_output[i] = 0;
	  FDS_SOUND.ecounter[i] = 0;
	}
/*
	FDS_SOUND.option[OPT_MOD_PHASE_REFRESH] = TRUE;
	FDS_SOUND.option[OPT_CAR_PHASE_REFRESH] = FALSE;
	FDS_SOUND.option[OPT_USE_PWM] = FALSE;
*/
}


static void update_envelope(int ch){
	while (FDS_SOUND.ecounter[ch] >= (1<<DOUBLE_SHIFT)){
	  if (FDS_SOUND.envelope_mode[ch] == 0){
		if (FDS_SOUND.envelope_amount[ch])
		  FDS_SOUND.envelope_amount[ch]--;
	  }
	  else if (FDS_SOUND.envelope_mode[ch] == 1){
		if (FDS_SOUND.envelope_amount[ch] < 0x21)
			FDS_SOUND.envelope_amount[ch]++;
	  }
	  FDS_SOUND.ecounter[ch] -= (1<<DOUBLE_SHIFT);
	}
	if(!FDS_SOUND.envelope_disable)
		FDS_SOUND.ecounter[ch] += FDS_SOUND.ecounter_incr[ch];
}

static s32 fds_snd_process(void){
//static s32 calc(){
//	static const u8 vtable[4] = { 72, 48, 34, 22 };
//	static const u8 vtable[4] = { 48, 32, 24, 16 };
//	static const int ctable[4] = { 36, 24, 18, 16 };

	/* Modulator */
	FDS_SOUND.opout[1] =
	  ((FDS_SOUND.opval[1] & 0x40) ? (FDS_SOUND.opval[1] - 128) : FDS_SOUND.opval[1]) * FDS_SOUND.envelope_output[1];

	if (!FDS_SOUND.envelope_disable)
	  FDS_SOUND.envelope_output[1] =
		FDS_SOUND.envelope_amount[1] < 0x21 ? FDS_SOUND.envelope_amount[1] : 0x20;

	while (FDS_SOUND.pcounter[1] >= (1<<DOUBLE_SHIFT))
	{
	  if (FDS_SOUND.wave[1][FDS_SOUND.phase[1]] == -1)
		FDS_SOUND.opval[1] = 0;
	  else
		FDS_SOUND.opval[1] = (FDS_SOUND.opval[1] + FDS_SOUND.wave[1][FDS_SOUND.phase[1]]) & 0x7f;
	  FDS_SOUND.phase[1] = (FDS_SOUND.phase[1] + 1) & 0x3f;
	  FDS_SOUND.pcounter[1] -= (1<<DOUBLE_SHIFT);
	}
	FDS_SOUND.pcounter[1] += FDS_SOUND.incr[1];
	update_envelope (1);

	/* Carrier */
	FDS_SOUND.opout[0] = FDS_SOUND.wave[0][FDS_SOUND.phase[0]];

	while (FDS_SOUND.pcounter[0] >= (1<<DOUBLE_SHIFT))
	{
	  FDS_SOUND.phase[0] = (FDS_SOUND.phase[0] + 1) & 0x3f;
	  if (!FDS_SOUND.phase[0])
		FDS_SOUND.envelope_output[0] =
		  FDS_SOUND.envelope_amount[0] < 0x21 ? FDS_SOUND.envelope_amount[0] : 0x20;
	  FDS_SOUND.pcounter[0] -= (1<<DOUBLE_SHIFT);
	}

	if (!FDS_SOUND.write_enable[0])
	{
	  // 注意)fdssound.txtとは3072〜4095の範囲が異なる。
	  s32 fm =
		(((4096 + 1024 + FDS_SOUND.opout[1]) & 4095) / 16) - 64 + ((FDS_SOUND.opout[1] > 0)
														 && (FDS_SOUND.opout[1] & 0xf) ?
														 2 : 0);
	  FDS_SOUND.pcounter[0] += FDS_SOUND.incr[0] + (FDS_SOUND.incr[0] * fm) / 64;
	}

	update_envelope (0);

/*	if (FDS_SOUND.option[OPT_USE_PWM])
	{
	  iup();
	  if(value() < FDS_SOUND.envelope_output[0])
		return FDS_SOUND.opout[0] << 6;
	  else
		return 0;
	}
	else*/
	  FDS_SOUND.opout[0] *= FDS_SOUND.envelope_output[0];
	
//	return (FDS_SOUND.opout[0] * vtable[FDS_SOUND.volume & 3]) >> 4;
//	return (FDS_SOUND.opout[0] * vtable[FDS_SOUND.volume]) >> 4;
	return (FDS_SOUND.opout[0] * FDS_SOUND.volume) >> 4;
  }


/*
u32 FDS_Render(){
	u32 r = calc ();
//	if(FDS_SOUND.mask)
//		r = 0;
	return r;
}
*/

void FDS_Write (u32 adr, u8 val){
	static const u8 btable[] = { 0, 1, 2, 4, 0xff, 128 - 4, 128 - 2, 128 - 1 };
//	static const u8 vtable[4] = { 48, 32, 24, 16 };
// 	static const u8 vtable[4] = { 60, 40, 30, 20 }; //beta2
	static const u8 vtable[4] = { 72, 48, 34, 22 }; //beta3

	if (0x4040 <= adr && adr < 0x4080)
	{
	  if (FDS_SOUND.write_enable[0])
		FDS_SOUND.wave[0][adr & 0x3f] = (val & 0x3f) - 0x20;
	  return;
	}

	switch (adr)
	{
	case 0x4080:
	  FDS_SOUND.envelope_mode[0] = (val >> 6) & 3;
	  if (FDS_SOUND.envelope_mode[0] & 2)
		FDS_SOUND.envelope_amount[0] = val & 0x3f;
	  else
	  {
		if (FDS_SOUND.envelope_speed)
		  FDS_SOUND.ecounter_incr[0] =
			(FDS_SOUND.geta/8) / (FDS_SOUND.envelope_speed * ((val & 0x3f) + 1));
//		  FDS_SOUND.ecounter_incr[0] =
//			FDS_SOUND.geta / ((8 * FDS_SOUND.envelope_speed) * ((val & 0x3f) + 1));
		else
		  FDS_SOUND.ecounter_incr[0] = 0;
	  }
	  break;

	case 0x4082:
	  FDS_SOUND.freq[0] = (FDS_SOUND.freq[0] & 0xf00) | (val & 0xff);
	  FDS_SOUND.incr[0] = (FDS_SOUND.geta/65536) * FDS_SOUND.freq[0];
	  break;

	case 0x4083:
	  FDS_SOUND.freq[0] = ((val & 0x0f) << 8) | (FDS_SOUND.freq[0] & 0xff);
	  FDS_SOUND.incr[0] = (FDS_SOUND.geta/65536) * FDS_SOUND.freq[0];
	  FDS_SOUND.sound_disable = (val >> 7) & 1;
	  if (FDS_SOUND.sound_disable)
		FDS_SOUND.pcounter[0] = 0;
	  FDS_SOUND.envelope_disable = (val >> 6) & 1;

/*	  if (FDS_SOUND.option[OPT_CAR_PHASE_REFRESH])
	  {
		FDS_SOUND.phase[0] = 0;
		FDS_SOUND.pcounter[0] = 0;
	  }*/
	  break;

	case 0x4084:
	  FDS_SOUND.envelope_mode[1] = (val >> 6) & 3;
	  if (FDS_SOUND.envelope_mode[1] & 2)
		FDS_SOUND.envelope_amount[1] = val & 0x3f;
	  else
	  {
		if (FDS_SOUND.envelope_speed)
		  FDS_SOUND.ecounter_incr[1] =
			(FDS_SOUND.geta/8) / (FDS_SOUND.envelope_speed * ((val & 0x3f) + 1));
//		  FDS_SOUND.ecounter_incr[1] =
//			FDS_SOUND.geta / ((8 * FDS_SOUND.envelope_speed) * ((val & 0x3f) + 1));
		else
		  FDS_SOUND.ecounter_incr[1] = 0;
	  }
	  break;

	case 0x4085:
	  FDS_SOUND.opval[1] = val & 0x7f;

//	  if (FDS_SOUND.option[OPT_MOD_PHASE_REFRESH])
//	  {
		FDS_SOUND.phase[1] = 0;
		FDS_SOUND.pcounter[1] = 0;
//	  }
	  break;

	case 0x4086:
	  FDS_SOUND.freq[1] = (FDS_SOUND.freq[1] & 0xf00) | (val & 0xff);
	  FDS_SOUND.incr[1] = (FDS_SOUND.geta/65536) * FDS_SOUND.freq[1];
	  break;

	case 0x4087:
	  FDS_SOUND.freq[1] = ((val & 0x0f) << 8) | (FDS_SOUND.freq[1] & 0xff);
	  FDS_SOUND.incr[1] = (FDS_SOUND.geta/65536) * FDS_SOUND.freq[1];
	  FDS_SOUND.write_enable[1] = (val >> 7) & 1;
	  break;

	case 0x4088:
	  if (FDS_SOUND.write_enable[1])
	  {
		FDS_SOUND.wave[1][(FDS_SOUND.wavidx++) & 0x3f] = btable[val & 7];
		FDS_SOUND.wave[1][(FDS_SOUND.wavidx++) & 0x3f] = btable[val & 7];
	  }
	  break;

	case 0x4089:
	  FDS_SOUND.write_enable[0] = (val >> 7) & 1;
//	  FDS_SOUND.volume = (val & 3);
	  FDS_SOUND.volume = vtable[(val & 3)];
	  break;

	case 0x408A:
	  FDS_SOUND.envelope_speed = val & 0xff;
	  break;

	case 0x4023:
	  break;

	default:
	  return;
	}
	FDS_SOUND.reg[adr & 0xFF] = val;
	return;
}


u8 FDS_Read(u32 adr){
	u8 val;
	if (0x4040 <= adr && adr < 0x4080){
		val = FDS_SOUND.write_enable ? FDS_SOUND.wave[0][FDS_SOUND.phase[0] & 0x3f] : FDS_SOUND.wave[0][adr & 0x3f];
		return val;
	}
	else if ( 0x4080 <= adr && adr < 0x408B){
		val = FDS_SOUND.reg[adr&0xFF];
		return val;
	}

	switch (adr){
		case 0x4090:
		val = FDS_SOUND.envelope_output[0] | 0x40;
		break;

	case 0x4092:
	  val = FDS_SOUND.envelope_output[1] | 0x40;
	  break;

	default:
	  return 0;
	}
	return val;
}


static int fds_snd_init(void){
//	FDS_SetClock (21477270);
	FDS_SetRate (SAMPLE_RATE);
//	counter_init(36);
//	counter_init(FDS_SOUND.clock, FDS_SOUND.rate, 36);
	FDS_Reset();
	return 1;
}


static void fds_snd_shutdown(void){
}


static void fds_snd_paramschanged(void){
	fds_snd_init();
}

/*
static s32 fds_snd_process(void){
	return FDS_Render();
}
*/

static apu_memwrite fds_memwrite[] =
{
   { 0x4040, 0x40FF, FDS_Write },
   {	 -1,	 -1, NULL }
};


static apu_memwrite fds_memwritesync[] =
{
	{	 -1,	 -1, NULL },
//	{ 0x4040, 0x40FF, 0 },
};


static apu_memread fds_memread[] =
{
   { 0x4040, 0x40FF, FDS_Read},
   {	 -1,	 -1, NULL }
};


apuext_t fds_nesp_ext =
{
   fds_snd_init,
   fds_snd_shutdown,
   FDS_Reset,
   fds_snd_paramschanged,
   fds_snd_process,
   fds_memread,		/* apu_memread	*/
   fds_memwrite,	/* apu_memwrite */
   fds_memwritesync	/* apu_memwrite */
};

