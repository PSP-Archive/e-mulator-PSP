///////////////////////////////////////////////////////////////////
// 
// CDROM�Ή��ɂ�����h��� PCEP070�̃\�[�X�R�[�h�����p�����Ē����܂����B
// 
//////////////////////////////////////////////////////////////////
#include "hal.h"
#include "pce.h"
#include "cstring.h"

void cd_test_read(char *p,int s, int num);

typedef struct{
    u32   LBA;
	byte  min;
	byte  sec;
	byte  fra;
    byte  type;
}struct_cd_toc;

struct_cd_toc	cd_toc[0x100];
byte nb_max_track = 24;	//(NO MORE BCD!!!!!)


// ******************************************************************************************
// ******************************************************************************************
// ******************************************************************************************
#if 0// *************************************************************************************
// ******************************************************************************************
// ******************************************************************************************
// ******************************************************************************************
#define Uint8 u8
#define Sint8 s8
#define Uint16 u16
#define Sint16 s16
#define Uint32 u32
#define Sint32 s32
#define BOOL u32

/*
	All the Arcade Card I/O code is taken from Hu-Go!
*/
static Uint32		_AcShift;
static Uint32		_AcShiftBits;
static Uint8		_0x1ae5;
static Uint8		_AcRam[0x200000];

typedef struct
{
	Uint8		control;
	Uint32		base;
	Uint16		offset;
	Uint16		increment;
} ACIO;

static ACIO		_Ac[4];



#define BCD(A)	((A / 10) * 16 + (A % 10))		// convert INT --> BCD
#define INT(B)	((B / 16) * 10 + (B % 16))		// convert BCD --> INT


typedef struct
{
	Uint8	minStart;
	Uint8	secStart;
	Uint8	frameStart;

	Uint8	minEnd;
	Uint8	secEnd;
	Uint8	frameEnd;

	Uint8	subqMinTrack;
	Uint8	subqSecTrack;
	Uint8	subqFrmTrack;
	Uint8	subqMinTotal;
	Uint8	subqSecTotal;
	Uint8	subqFrmTotal;
	Uint8	subqTrackNum;

	Sint32	elapsedSec;
	Sint32	playEndSec;
	Sint32	searchMode;
	Sint32	playMode;

	BOOL	bRepeat;
	BOOL	bPlaying;
	BOOL	bPaused;
	BOOL	bSeeking;
	BOOL	bSeekDone;
	BOOL	bInterrupt;
} AudioTrack;


static BOOL			_bBRAMEnabled;

static Uint8		_Port[15];

static Uint8		_ReadBuffer[2048*256];	// 512KB
static Sint32		_ReadBufferIndex;		// �ǂݏo���o�b�t�@�̃C���f�b�N�X 
static Sint32		_ReadByteCount;		// �ǂݏo����o�C�g��  ���F�_�E���J�E���^ 
static Sint32		_CheckCountAfterRead;
static Uint32		_ResetDelayCount = 10;

static Sint32		_CDDAReadBufferSizeSector;
static Sint32		_CDDAReadBufferSizeSector2;
static Sint32		_CDDAReadBufferSize;
static Sint32		_CDDAReadBufferSize2;
static Uint8		_CDDAReadBuffer[2352*128]; //CDDA�ǂݍ��݃o�b�t�@ //Kitao�ǉ�
static Sint32		_CDDAReadBufferIndex = 0; //�ǂݏo���o�b�t�@�̃C���f�b�N�X //Kitao�ǉ�
static Sint32		_CDDAReadByteCount = 0;	//�ǂݏo����o�C�g��  ���F�_�E���J�E���^ //Kitao�ǉ�
static Sint32		_CDDASectorAddr; //�ǂݏo��CDDA�̃Z�N�^�[�A�h���X //Kitao�ǉ�
static Sint32		_CDDASectorEndAddr; //�ǂݏo��CDDA�̍ŏI�Z�N�^�[�A�h���X //Kitao�ǉ�
static Uint8		_ReadCddaFirst = 1; //�e�ȍŏ���CDIF_READCDDA�������Ȃ��Ƃ���1�B�X�e�[�g���[�h����̍Đ��̏ꍇ2�B����ȊO��0�B
static BOOL			_bSeekCddaDone; //Kitao�ǉ�

static BOOL			_bFastCD; //Kitao�ǉ��BCD�A�N�Z�X�ɃE�F�C�g�����Ȃ��Ȃ�TRUE
static Sint32		_CDAccessCount; //Kitao�ǉ��BCD�A�N�Z�X�E�F�C�g�p�J�E���^
static Sint16		_FadeOut1; //Kitao�ǉ��B�m�C�Y�y���̂��߁A�������̓t�F�[�h�A�E�g�ŏ�������B
static Sint16		_FadeOut2; //Kitao�ǉ�

//Kitao�ǉ��BCdFader.c��p�~���Ă�����֓��������B�iCDDA�Đ����̑��x�A�b�v�{�t�F�[�h�󋵂̃X�e�[�g�Z�[�u�̂��߁j
static Sint32		_InitialCdVolume;
static Sint32		_CurrentCdVolume;
static Sint32		_VolumeStep;
static BOOL			_bFadeIn  = FALSE;
static BOOL			_bFadeOut = FALSE;
static double		_CdVolumeEffect = 0.0;//Kitao�ǉ�

static Sint32		_FadeClockCount = 0;
static Sint32		_FadeCycle;
//

// �R�}���h�o�b�t�@�� 
static Sint32		_Command;
static Sint32		_ArgsLeft;
static Uint8		_CmdArgBuffer[10];
static Sint32		_CmdArgBufferIndex;

static BOOL			_bCommandReset = TRUE;
static BOOL			_bCommandReceived = FALSE;
static BOOL			_bCommandDone = FALSE;
static BOOL			_bDriveBusy = FALSE;
static BOOL			_bError = FALSE;		// set when command execution fail

static AudioTrack	_AudioTrack;
static Sint32		_ClockCount = 0;
static BOOL			_bCdromInit = FALSE;

//static DISCINFO		_DiscInfo; //Kitao�ǉ�
static BOOL			_bAutoOverClock = FALSE; //Kitao�ǉ�
static Sint32		_AutoSpriteOver = 0; //Kitao�ǉ�
static Sint32		_AutoNonSpriteOver = 0; //Kitao�ǉ�
static Sint32		_AutoSlowCD = 0; //Kitao�ǉ�
static Sint32		_AutoFastCD = 0; //Kitao�ǉ�

void update_irq_state()
{
}

static void
lba2msf(
	Uint32		lba,
	Uint8*		m,
	Uint8*		s,
	Uint8*		f)
{
	lba += 150; //Kitao�ǉ�v0.53�B150=�P�g���b�N�J�n�܂ł̃X�^�[�g�b���Ԃ�(�v���M���b�v)�𑫂��B
	*m = lba / 75 / 60;
	*s = (lba - *m * 75 * 60) / 75;
	*f = lba - (*m * 75 * 60) - (*s * 75);
}

//Kitao�ǉ�v0.53
static Uint32
msf2lba(
	Uint8		m,
	Uint8		s,
	Uint8		f)
{
	return (m*60 + s)*75 + f - 150; //�P�b�Ԃ�75�t���[���B150=�P�g���b�N�J�n�܂ł̃X�^�[�g�b���Ԃ�(�v���M���b�v)�������B
}

/*-----------------------------------------------------------------------------
	[read_1801]
		CD-ROM ����̃f�[�^��ǂݏo���B
-----------------------------------------------------------------------------*/
static Uint8
read_1801(void)
{
	if (_ReadByteCount > 0)
	{
		Uint8	ret = _ReadBuffer[_ReadBufferIndex++];

		if (--_ReadByteCount > 0)
		{
			_Port[0] = 0xC8;	// data still exist in buffer
		}
		else
		{
			_Port[3] &= ~0x40;	// "data transfer ready" �I�� 
			_Port[3] |= 0x20;	// data transfer done 
			update_irq_state();

			_Port[0] = 0xD8;	// no more data left in buffer
			_ReadBufferIndex = 0;

			// �ǂݏo���������I��������
			// ����������ɏI���������ǂ������m�F���邽�߂�
			// �ǂݏo�����Q��s�Ȃ���B
			_CheckCountAfterRead = 2;
		}
		return ret;
	}
	else	// �o�b�t�@�̃f�[�^���S�ēǂݏo���ꂽ��Ɋm�F�̂��߂� $1801 �ǂݏo�����Q��s�Ȃ���B 
	{
		if (_CheckCountAfterRead == 2)
		{
			--_CheckCountAfterRead;
			_Port[0] = 0xF8;

			if (_bError)
			{
				_bError = FALSE;
				return 1;
			}
		}
		else if (_CheckCountAfterRead == 1)
		{
			--_CheckCountAfterRead;
			_Port[0] &= ~0x80;
		}
	}

	return 0;
}


/*-----------------------------------------------------------------------------
	[read_1808]
		CD-ROM ����̃f�[�^��ǂݏo���B

	[����]
		read_1801 �Ƃ̈Ⴂ�́A�b�c�|�q�n�l�o�b�t�@���̒l��ǂݏI�������
		�Q��̓ǂݏo���̒l��Ԃ��Ȃ��_�B 
-----------------------------------------------------------------------------*/
static Uint8
read_1808(void)
{
	if (_ReadByteCount > 0)
	{
		Uint8	ret = _ReadBuffer[_ReadBufferIndex++];

		if (--_ReadByteCount > 0)
		{
			_Port[0] = 0xC8;	// data still exist in buffer
		}
		else
		{
			_Port[3] &= ~0x40;		// "data transfer ready" �I�� 
			_Port[3] |= 0x20;		// data transfer done 
			update_irq_state();

			_Port[0] = 0xD8;	// no more data left in buffer
			_ReadBufferIndex = 0;

			// �ǂݏo���������I��������
			// ����������ɏI���������ǂ������m�F���邽�߂�
			// �ǂݏo�����Q��s�Ȃ���B
			_CheckCountAfterRead = 2;
		}

		return ret;
	}
	else
	{
		_Port[0] = 0xD8;	// no more data left in buffer
	}

	return 0;
}

static Uint8
get_first_track(void)
{
	if (!_bCdromInit)
		return 0;

#if 1
	return 1;
#else
	return CDIF_GetFirstTrack();
#endif
}


static Uint8
get_last_track(void)
{
	if (!_bCdromInit)
		return 0;

#if 1
	return nb_max_track;	
#else
	return CDIF_GetLastTrack();
#endif
}


static void
get_track_start_position(
	Uint8		track,
	Uint8*		min,
	Uint8*		sec,
	Uint8*		frame,
	Uint8*		type)		// �f�[�^�g���b�N(4)���I�[�f�B�I�g���b�N(0)��
{
#if 1
	Uint32		msft=0;

	*min   = cd_toc[track].min;
	*sec   = cd_toc[track].sec;
	*frame = cd_toc[track].fra;
	*type  = cd_toc[track].type;

#else
	Uint32		msft = CDIF_GetTrackStartPositionMSF(track);
	*min   = (Uint8)(msft >> 24);
	*sec   = (Uint8)(msft >> 16);
	*frame = (Uint8)(msft >> 8);
	*type  = (Uint8)(msft);
#endif
}


static Uint8
get_track_number_by_msf(
	Uint8		m,
	Uint8		s,
	Uint8		f)
{
	int			track = get_first_track();
	int			lastTrack = get_last_track();
	int			msf;

#if 1
*(int*)0=0;
#else
	while (track <= lastTrack+1) //Kitao�X�V�B�ŏI�g���b�N��CD-DA���点��悤��lastTrack+1�Ƃ����BLinda3�Ŕ���
	{
		msf = CDIF_GetTrackStartPositionMSF(track) >> 8;
		if ((m << 16) + (s << 8) + f < msf)
			return track - 1;
		++track;
	}
#endif

	return 0;
}

//Kitao�ǉ��Bstop_play_track�ƈႢ�APCE���Ɍ��ʂ͕Ԃ��Ȃ��B
void
CDROM_Stop()
{
	_CDDAReadByteCount = 0; //Kitao�X�V�BCDDA���Đ����Ă����ꍇ��~�����B����ɂ��CDDA�f�[�^�ւ̃A�N�Z�X�\����X�g�b�v����̂ŁA�n�[�h�I��stop�����O�ɍs�����Ƃ��d�v�B
	_AudioTrack.bSeekDone = FALSE;
	_AudioTrack.bPaused   = FALSE;
	_AudioTrack.bPlaying  = FALSE;
//
//	//Kitao�ǉ��B�X�v���C�g�I�[�o�[�Č�����K�v������\�t�g�̏ꍇ�ŁA�X�v���C�g�I�[�o�[�Č��̕K�v���Ȃ��Ȃ����猳�ɖ߂�(�Č����Ȃ��ݒ�ɂ���)�B
//	if (_AutoNonSpriteOver == -1)
//		VDC_SetPerformSpOver(FALSE);
//
}



static void
stop_play_track() //Kitao�X�V�BWAV��CD�Đ�����悤�ɂ������߃n�[�h�I�ȃX�g�b�v�͂��Ȃ��BPCE���Ɍ��ʂ�Ԃ��B
{
	CDROM_Stop(); //Kitao�ǉ�

	_Port[0] = 0xd8;
	_ReadByteCount = 0;
	_CheckCountAfterRead = 2;

	_bError = FALSE;
	_bCommandDone = TRUE;
}


void
load_sectors(
	Sint32		sectorAddr,
	Uint8*		pBuffer,
	Sint32		nSector)
{
	CDROM_Stop(); //Kitao�ǉ��BCDDA���t���Ȃ�X�g�b�v����B���t���łȂ��ꍇ�i_AudioTrack.bPlaying=FALSE�j�ł�CDROM_MIX�J�E���^�[���O�łȂ��iCDDA�ǂݍ��ݍŒ��́j���Ƃ����邽�ߌĂԕK�v������B

	//Kitao�ǉ��BCD�A�N�Z�X�̃E�F�C�g�@�\��ǉ��B�Q�[���ɂ���Ă�CD�A�N�Z�X����CPU�p���[���g�����̂�����A�m���E�F�C�g���Ɩ�肪�N����B
	if (_bFastCD) //�������[�h(�f�t�H���g)
		_CDAccessCount = 7159090.0 / 60.0 +1; //��������Ɖ�ʂ�����邱�Ƃ�����̂łP�t���[���Ԃ�̓E�F�C�g����
	else //���@���݂̃E�F�C�g���[�h
		_CDAccessCount = 7159090.0*((double)nSector*2.0/150.0 +0.5); //�P�b=7159090�N���b�N�B�u/150.0�v�͂P�{��CDROM�̕��ϓ]�����x(���b150KB)�B�u+0.5�v�̓w�b�_�ړ��Ԃ�B

#if 1
		cd_test_read(pBuffer,sectorAddr,nSector);
/*
	{
		int i,flg=0;
		for(i=0;i<nb_max_track;i++) {
			if(cd_toc[i+1].LBA<sectorAddr) {
				flg=i+1;
			}
		}

		if(flg){



		}
	}
*/
#else
//	CDIF_ReadSector(pBuffer, sectorAddr, nSector, TRUE); //Kitao�ǋL�B���@�ł͓ǂݏo�������i�V�[�N���j�����邾���Ŏ��ۂ̓ǂݍ��݂͂܂������Ȃ�ꂸ�AREAD1801,1808�̍ۂɃ��[�h�A�N�Z�X�������Ȃ���B
#endif
}


static void
seek_track(
	AudioTrack*		p)
{
	CDROM_Stop(); //Kitao�ǉ��BCDDA���t���Ȃ�X�g�b�v����B���t���łȂ��ꍇ�i_AudioTrack.bPlaying=FALSE�j�ł�CDROM_MIX�J�E���^�[���O�łȂ��iCDDA�ǂݍ��ݍŒ��́j���Ƃ����邽�ߌĂԕK�v������B
#if 1
*(int*)0=0;
#else
	CDIF_Seek(p->minStart, p->secStart, p->frameStart, TRUE);
#endif
}



#if 0
//Kitao�ǉ��BCDDA��WAV�f�[�^�œǂݍ��݁A�_�C���N�g�T�E���h�ōĐ�����悤�ɂ����B
void
CDROM_Mix(
	Sint16*			pDst,				// �o�͐�o�b�t�@ //Kitao�X�V�BCDDA��p�o�b�t�@�ɂ�������Sint16�ɁB
	Uint32			sampleRate,			// �n�[�h�E�F�A�̍Đ����[�g 
	Sint32			nSample)			// �����o���T���v���� 
{
	Sint32	i;
	Sint16	sample1 = 0;
	Sint16	sample2 = 0;
	Sint32	a;
	double 	v;

//	if ((_CDDAReadByteCount == 0)||(_AudioTrack.bPaused))
	if ((!_AudioTrack.bPlaying)||(_AudioTrack.bPaused))
	{
		if ((_FadeOut1 == 0)&&(_FadeOut2 == 0))
			return; //Hu-Card�Ȃ�CD-DA���g��Ȃ��Q�[���ŏ������d���Ȃ�Ȃ��悤�ɁA_FadeOut��0�Ȃ瑦���^�[���B
		
		//�m�C�Y�y���̂��߃t�F�[�h�A�E�g�ŏ�������B
		for (i = 0; i < nSample; i++)
		{
			if (_FadeOut1 > 0)
			{
				_FadeOut1 -= 300; //����������������Ƌt�Ƀm�C�Y���o��(200���Ƒʖ�)�B�傫������(800���Ƒʖ�)�ƌ��ʂȂ��B
				if (_FadeOut1 < 0)
					_FadeOut1 = 0;
			}
			else if (_FadeOut1 < 0)
			{
				_FadeOut1 += 300;
				if (_FadeOut1 > 0)
					_FadeOut1 = 0;
			}
			if (_FadeOut2 > 0)
			{
				_FadeOut2 -= 300;
				if (_FadeOut2 < 0)
					_FadeOut2 = 0;
			}
			else if (_FadeOut2 < 0)
			{
				_FadeOut2 += 300;
				if (_FadeOut2 > 0)
					_FadeOut2 = 0;
			}
			*pDst++ = _FadeOut1;
			*pDst++ = _FadeOut2;
		}
		return;
	}

	if (_CdVolumeEffect == 0.0)
		v = 0.0; //�~���[�g
	else
	 	v = (double)_CurrentCdVolume / 65535.0 / _CdVolumeEffect;//�t�F�[�h�A�E�g�C���p�̊|������_CdVolumeEffect(���ʂ��傫������̂ŉ��ʃ_�E���{���ʒ��ߌ���)
	for (i = 0; i < nSample; i++)	// mixing loop
	{
		sample1  = (Sint32)(_CDDAReadBuffer[_CDDAReadBufferIndex]);
		sample1 += (Sint32)(_CDDAReadBuffer[_CDDAReadBufferIndex++]) <<16;
		sample1 += (Sint32)(_CDDAReadBuffer[_CDDAReadBufferIndex]) << 8;
		sample1 += (Sint32)(_CDDAReadBuffer[_CDDAReadBufferIndex++]) <<24;
		sample2  = (Sint32)(_CDDAReadBuffer[_CDDAReadBufferIndex]);
		sample2 += (Sint32)(_CDDAReadBuffer[_CDDAReadBufferIndex++]) << 16;
		sample2 += (Sint32)(_CDDAReadBuffer[_CDDAReadBufferIndex]) << 8;
		sample2 += (Sint32)(_CDDAReadBuffer[_CDDAReadBufferIndex++]) <<24;
		
		sample1 = (double)sample1 * v;
		sample2 = (double)sample2 * v;
		*pDst++ = sample1; //CDDA��p�̃o�b�t�@�ɂ����̂ŁA�v���X�������ڒl������������B
		*pDst++ = sample2; //�X�e���I2ch�Ԃ񏑂����݊���
		
		_CDDAReadByteCount -= 4; //4�o�C�g�ǂݍ���
		if (_CDDAReadByteCount == 0)
			break;
		
		//�C���f�b�N�X���o�b�t�@�̒��ԂɒB������A�o�b�t�@�̑O���Ƀf�[�^��ǉ��ǂݍ��݂���B
		if (_CDDAReadBufferIndex == _CDDAReadBufferSize2)
		{
			a = _CDDASectorEndAddr - _CDDASectorAddr;
			if (a > _CDDAReadBufferSizeSector2)
				a = _CDDAReadBufferSizeSector2;
#if 1
*(int*)0=0;
#else
			if (a != 0)
				CDIF_ReadCddaSector2(_CDDAReadBuffer, _CDDASectorAddr, a, TRUE);//�o�b�t�@�O�������ɁACDDA�̐擪�Z�N�^�[����WAV�f�[�^��ǂݏo���B
#endif
			//�ǂݏo�������̏ꍇ�Acd_callback�ɂĒʒm�����B���̒i�K�ł͂܂��ǂݍ��݊������Ă��Ȃ��B
		}
		//�C���f�b�N�X���o�b�t�@�̍Ō�ɒB������A�C���f�b�N�X�͂O�ɖ߂��A�o�b�t�@�̌㔼�Ƀf�[�^��ǉ��ǂݍ��݂���B
		if (_CDDAReadBufferIndex == _CDDAReadBufferSize)
		{
			_CDDAReadBufferIndex = 0;
			
			a = _CDDASectorEndAddr - _CDDASectorAddr;
			if (a > _CDDAReadBufferSizeSector2)
				a = _CDDAReadBufferSizeSector2;
#if 1
*(int*)0=0;
#else
			if (a != 0)
				CDIF_ReadCddaSector2(_CDDAReadBuffer + _CDDAReadBufferSize2, _CDDASectorAddr, a, TRUE);//�o�b�t�@�㔼�����ɁACDDA�̐擪�Z�N�^�[����WAV�f�[�^��ǂݏo���B
#endif
			//�ǂݏo�������̏ꍇ�Acd_callback�ɂĒʒm�����B���̒i�K�ł͂܂��ǂݍ��݊������Ă��Ȃ��B
		}
	}
	_FadeOut1 = sample1; //�Ō�̔g�`�l
	_FadeOut2 = sample2; //2ch�Ԃ�
}
#endif


//Kitao�X�V�BCDROM�̃Z�N�^�[���璼��CDDA��WAV�f�[�^��ǂݏo���čĐ�����悤�ɂ����B
static void
play_track(
	AudioTrack*		p)
{
	int		trackNo;//Kitao�ǉ�
	Sint32	a;

	CDROM_Stop(); //Kitao�ǉ��BCDDA���t���Ȃ�X�g�b�v����B���t���łȂ��ꍇ�i_AudioTrack.bPlaying=FALSE�j�ł�CDROM_MIX�J�E���^�[���O�łȂ����Ƃ����邽�ߌĂԕK�v������B

	_CDDASectorAddr = msf2lba(p->minStart, p->secStart, p->frameStart);
	_CDDASectorEndAddr = msf2lba(p->minEnd, p->secEnd, p->frameEnd);
	a = _CDDASectorEndAddr - _CDDASectorAddr;
	if (a > _CDDAReadBufferSizeSector)
		a = _CDDAReadBufferSizeSector;
	_ReadCddaFirst = 1;//����Đ��̍��}

#if 1
*(int*)0=0;
#else
	CDIF_ReadCddaSector(_CDDAReadBuffer, _CDDASectorAddr, a, TRUE);//CDDA�̐擪�Z�N�^�[����WAV�f�[�^��ǂݏo���B����Ȃ̂Ńo�b�t�@�S�̂Ԃ�ǂݏo���B
#endif

	//�ǂݏo�������̏ꍇ�Acd_callback�ɂĒʒm�����B���̒i�K�ł͂܂��ǂݍ��݊������Ă��Ȃ��B

	//Kitao�ǉ��B�g���b�N�i���o�[���擾
 	trackNo = get_track_number_by_msf(p->minStart, p->secStart, p->frameStart);

	//Kitao�ǉ��BCD�A�N�Z�X�̑��x�����@���ɂ���K�v������\�t�g�̏ꍇ�A����g���b�N�̋Ȃ�����������A��������}�ɂ����Œx������B
	if (trackNo == _AutoSlowCD)
		_bFastCD = FALSE;
	if (trackNo == _AutoFastCD) //�x������K�v���Ȃ��Ȃ����猳�ɖ߂�(��������)
		_bFastCD = TRUE;

//
//	//Kitao�ǉ��B�X�v���C�g�I�[�o�[�Č�����K�v������\�t�g�̏ꍇ�A����g���b�N�̋Ȃ�����������A��������}�ɂ����ŃX�v���C�g�I�[�o�[�Č��ݒ�ɕύX����B
//	if (trackNo == _AutoSpriteOver)
//		VDC_SetPerformSpOver(TRUE);
//

//Kitao�e�X�g�p
/*
//if (WINMAIN_GetBreakTrap())
{
	char s[100];
	sprintf(s,"TrackNo=%d",(int)trackNo);
	int ok;
	ok = MessageBox(WINMAIN_GetHwnd(),
			s,
			"Test",
			MB_YESNO); //Kitao�e�X�g
	if (ok != IDYES)
		WINMAIN_SetBreakTrap(FALSE);
}
*/
}

//Kitao�ǉ��B�X�e�[�g���[�h����ĊJ�p��play_track�B
static void
play_track2(
	AudioTrack*		p)
{
	_ReadCddaFirst = 0; //����Đ����I���Ă���̂�0�ɃZ�b�g

	//�����ɃA�N�Z�X�ł���悤�A�Ȃ̍Đ��ʒu��O�ɃV�[�N����B
	_bSeekCddaDone = FALSE;

#if 1
*(int*)0=0;
#else
	CDIF_SeekCdda(p->minStart + p->elapsedSec/60,
				  p->secStart + p->elapsedSec%60,
				  p->frameStart + ((double)_ClockCount/7159090.0)*75.0,
				  TRUE);
#endif

	//�V�[�N�����̏ꍇ�Acd_callback�ɂĒʒm�����B���̒i�K�ł͂܂��ǂݍ��݊������Ă��Ȃ��B
	while (!_bSeekCddaDone)
		Sleep(1); //�V�[�N�����܂ő҂�
}


static void
pause_track(
	BOOL		bPause)
{
	_AudioTrack.bPaused = bPause; //Kitao�ǉ�

#if 1
*(int*)0=0;
#else
	if (!CDIF_PauseAudioTrack(bPause, TRUE))
	{
		/* play ���Ă��Ȃ��Ƃ��� pause ���ꂽ�ꍇ��(�Ƃ肠������)�Ώ� */
		/* �{���̓G���[��Ԃ��悤�ɂ��ׂ� */
		//_bError = TRUE;
		_Port[0] = 0xd8;
		_ReadByteCount = 0;
		_CheckCountAfterRead = 2;

		_bError = FALSE;
		_bCommandDone = TRUE;
	}
#endif
}



//�e�X�g�p
void
show_command_string(
	const char*		pCmdName,
	const Uint8*	pCmdString)
{
/*
	// This function only used for hardware-level logging
	//
	PRINTF("%s %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X\n",
			pCmdName,
			pCmdString[0],pCmdString[1],pCmdString[2],pCmdString[3],pCmdString[4],
			pCmdString[5],pCmdString[6],pCmdString[7],pCmdString[8],pCmdString[9]);
*/
}

static void
execute_read_sector()
{
	Uint32		sectorAddr;

	show_command_string("[READ SECTOR]", _CmdArgBuffer);

	sectorAddr = _CmdArgBuffer[1]*65536 + _CmdArgBuffer[2]*256 + _CmdArgBuffer[3];
	load_sectors(sectorAddr, _ReadBuffer, _CmdArgBuffer[4]);
	_ReadByteCount = 2048 * _CmdArgBuffer[4];
}

static void
execute_cd_playback_start_position()
{
	Uint8	not_used;

	show_command_string("[PLAY AUDIO1]", _CmdArgBuffer);
	memset(&_AudioTrack, 0, sizeof(AudioTrack));

	/* �������S�ă[���̂Ƃ��͒�~������ */
	if ((_CmdArgBuffer[2] | _CmdArgBuffer[3] | _CmdArgBuffer[4]) == 0)
	{
		stop_play_track();
		_Port[0] = 0xD8;
		_ReadByteCount = 0;
		_CheckCountAfterRead = 2;

		_AudioTrack.bSeekDone = FALSE;
		_AudioTrack.bPaused   = FALSE;
		_AudioTrack.bPlaying  = FALSE;

		_Port[3] |= 0x20;
		update_irq_state();

		_bError = FALSE;
		return;
	}

	switch (_CmdArgBuffer[9] & 0xC0)
	{
		case 0x00:	// LBA�w�胂�[�h
			lba2msf((_CmdArgBuffer[2]<<16)|(_CmdArgBuffer[3]<<8)|_CmdArgBuffer[4],
					&_AudioTrack.minStart,
					&_AudioTrack.secStart,
					&_AudioTrack.frameStart);
			break;

		case 0x40:	// MSF�w�胂�[�h
			_AudioTrack.minStart   = INT(_CmdArgBuffer[2]);
			_AudioTrack.secStart   = INT(_CmdArgBuffer[3]);
			_AudioTrack.frameStart = INT(_CmdArgBuffer[4]);
			break;

		case 0x80:	// �g���b�N�ԍ��w�胂�[�h
			get_track_start_position(	INT(_CmdArgBuffer[2]),
										&_AudioTrack.minStart,
										&_AudioTrack.secStart,
										&_AudioTrack.frameStart,
										&not_used);
			break;

		case 0xc0:	// ???
			//PRINTF("unknown mode");
			break;
	}
	_AudioTrack.searchMode = _CmdArgBuffer[1];

	//Kitao�ǉ��B�T�[�`���[�h���I���̂Ƃ��̂��߂ɁA�Ȃ̃G���h�|�W�V�������ŏI�g���b�N�̏I���ɐݒ肵�Ă����B
	get_track_start_position(	get_last_track()+1,
								&_AudioTrack.minEnd,
								&_AudioTrack.secEnd,
								&_AudioTrack.frameEnd,
								&not_used);

	//Kitao�ǉ��B�T�[�`���[�h�̒l���ݒ肳��Ă���΂����ōĐ����J�n�B
	switch (_CmdArgBuffer[1])
	{
		case 0x00:		// Kitao�X�V�Bseek only
			seek_track(&_AudioTrack);
			break;

		case 0x01:		// repeat play
			_AudioTrack.bRepeat = TRUE;
			_AudioTrack.playMode = 1;
			play_track(&_AudioTrack);
			break;

		case 0x02:		// play, IRQ2 when finished ??
			_AudioTrack.bInterrupt = TRUE;
			_AudioTrack.playMode = 2;
			play_track(&_AudioTrack);
			break;

		case 0x03:		// play without repeat
			_AudioTrack.playMode = 3;
			if (_AudioTrack.bPaused)
				pause_track(FALSE);
			else
				play_track(&_AudioTrack);
			break;
	}
}


static void
execute_cd_playback_end_position()
{
	Uint8		not_used;

	show_command_string("[PLAY AUDIO2]", _CmdArgBuffer);
	switch (_CmdArgBuffer[9] & 0xc0)
	{
		case 0x00:	// LBA�w�胂�[�h
			lba2msf((_CmdArgBuffer[2]<<16)|(_CmdArgBuffer[3]<<8)|_CmdArgBuffer[4],
					&_AudioTrack.minEnd,
					&_AudioTrack.secEnd,
					&_AudioTrack.frameEnd);
			break;

		case 0x40:	// MSF�w�胂�[�h
			_AudioTrack.minEnd   = INT(_CmdArgBuffer[2]);
			_AudioTrack.secEnd   = INT(_CmdArgBuffer[3]);
			_AudioTrack.frameEnd = INT(_CmdArgBuffer[4]);
			break;

		case 0x80:	// �g���b�N�ԍ��w�胂�[�h
			get_track_start_position(	INT(_CmdArgBuffer[2]),
										&_AudioTrack.minEnd,
										&_AudioTrack.secEnd,
										&_AudioTrack.frameEnd,
										&not_used);
			break;

		case 0xc0:	// �g���b�N�I���ʒu?? 
			break;
	}

	switch (_CmdArgBuffer[1])
	{
		case 0x00:		// no operation ??
			_AudioTrack.playMode = 0;
			stop_play_track();
			_Port[0] = 0xD8;
			_ReadByteCount = 0;
			_CheckCountAfterRead = 2;
			_bCommandDone = TRUE;
			break;

		case 0x01:		// repeat play
			_AudioTrack.bRepeat = TRUE;
			_AudioTrack.playMode = 1;
			play_track(&_AudioTrack);
			break;

		case 0x02:		// play, IRQ2 when finished ??
			_AudioTrack.bInterrupt = TRUE;
			_AudioTrack.playMode = 2;
			play_track(&_AudioTrack);
			break;

		case 0x03:		// play without repeat
			_AudioTrack.playMode = 3;
			if (_AudioTrack.bPaused)
				pause_track(FALSE);
			else
				play_track(&_AudioTrack);
			break;
	}
}


static void
execute_pause_cd_playback()
{
	show_command_string("[PAUSE AUDIO]", _CmdArgBuffer);
	pause_track(TRUE);
}


static void
execute_read_subchannel_q()
{
	_ReadBuffer[2] = BCD(_AudioTrack.subqTrackNum);
	_ReadBuffer[4] = BCD(_AudioTrack.subqMinTrack);
	_ReadBuffer[5] = BCD(_AudioTrack.subqSecTrack);
	_ReadBuffer[6] = BCD(_AudioTrack.subqFrmTrack);
	_ReadBuffer[7] = BCD(_AudioTrack.subqMinTotal);
	_ReadBuffer[8] = BCD(_AudioTrack.subqSecTotal);
	_ReadBuffer[9] = BCD(_AudioTrack.subqFrmTotal);

	if (_AudioTrack.bPlaying)                              _ReadBuffer[0] = 0;
	else if (_AudioTrack.bPaused || _AudioTrack.bSeekDone) _ReadBuffer[0] = 2;
	else                                                   _ReadBuffer[0] = 3;
/*
	PRINTF("[Q SUB CHANN] %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X\n",
			_ReadBuffer[0],_ReadBuffer[1],_ReadBuffer[2],_ReadBuffer[3],_ReadBuffer[4],
			_ReadBuffer[5],_ReadBuffer[6],_ReadBuffer[7],_ReadBuffer[8],_ReadBuffer[9]);
*/
	_ReadByteCount = 10;
	_Port[0] = 0xc8;
	_bCommandDone = TRUE;
}


static void
execute_get_dir_info()
{
	switch (_CmdArgBuffer[1])
	{
		case 0:	// get first and last track number
			_ReadBuffer[0] = BCD(get_first_track());
			_ReadBuffer[1] = BCD(get_last_track());
			_ReadByteCount = 2;
			break;

		case 1:	// get total running time of disc
			_ReadBuffer[0] = BCD(72);
			_ReadBuffer[1] = BCD(26);
			_ReadBuffer[2] = BCD(29);
			_ReadByteCount = 3;
			break;

		case 2:	// get track starting position and mode
		{
			Uint8		min;
			Uint8		sec;
			Uint8		frame;
			Uint8		type;

			get_track_start_position(_CmdArgBuffer[2], &min, &sec, &frame, &type);

			_ReadBuffer[0] = BCD(min);
			_ReadBuffer[1] = BCD(sec);
			_ReadBuffer[2] = BCD(frame);
			_ReadBuffer[3] = type;
			_ReadByteCount = 4;
			break;
		}
	}
	_Port[0] = 0xc8;
	_bCommandDone = TRUE;
}


static void
execute_command(void)
{
	switch (_CmdArgBuffer[0])
	{
		case 0x08:	// read sector
			execute_read_sector();
			break;

		case 0xD8:	// set audio playback start position
			execute_cd_playback_start_position();
			break;

		case 0xD9:	// set audio playback end position and start playing
			execute_cd_playback_end_position();
			break;

		case 0xDA:	// pause audio
			execute_pause_cd_playback();
			break;

		case 0xDD:	// read Q sub-channel
			execute_read_subchannel_q();
			break;

		case 0xDE:	// get dir info
			execute_get_dir_info();
			break;
	}
}


static
void
receive_command(
	Uint8		data)
{
	if (_bCommandReset)
	{
		_bCommandDone = FALSE;
		_Command = data;

		// �R�}���h�n�ϐ������Z�b�g����B 
		_bCommandReset = FALSE;
		_bCommandReceived = FALSE;
		_CmdArgBufferIndex = 0;
		_ArgsLeft = 0;
		_ReadBufferIndex = 0;
		_ReadByteCount = 0;

		// �R�}���h���󂯕t����B 
		switch (_Command)
		{
			case 0x00:	// TEST UNIT READY 
				_Port[0] = 0xD8;	// no more data needed
				_ArgsLeft = 0;
				_ReadByteCount = 0;
				_CheckCountAfterRead = 2;
				break;

			case 0x03:	// REQUEST SENSE
				break;

			case 0x08:	// read sector
				_CmdArgBuffer[_CmdArgBufferIndex++] = 0x08;
				_ArgsLeft = 5;
				break;

			case 0xD8:	// play audio (start position)
				_CmdArgBuffer[_CmdArgBufferIndex++] = 0xD8;
				_ArgsLeft = 9;
				break;

			case 0xD9:	// play audio (end position)
				_CmdArgBuffer[_CmdArgBufferIndex++] = 0xD9;
				_ArgsLeft = 9;
				break;

			case 0xDA:	// pause audio 
				_CmdArgBuffer[_CmdArgBufferIndex++] = 0xDA;
				_ArgsLeft = 9;
				_ReadByteCount = 0;
				_CheckCountAfterRead = 2;
				break;

			case 0xDD:
				_CmdArgBuffer[_CmdArgBufferIndex++] = 0xDD;
				_ArgsLeft = 9;
				_ReadByteCount = 10;
				break;

			case 0xDE:	// get CD directory info
				_CmdArgBuffer[_CmdArgBufferIndex++] = 0xDE;
				_ArgsLeft = 9;
				_ReadByteCount = 4;
				break;

			default:
				break;
		}
	}
	else
	{
		// �������󂯕t���� 
		_CmdArgBuffer[_CmdArgBufferIndex++] = data;

		if (--_ArgsLeft > 0)
		{
			_Port[0] = 0xd0;		// 0xd0: need more data
		}
		else
		{
			execute_command();
			_bCommandReceived = TRUE;
		}
	}
}

/*
	[�t�F�[�h�A�E�g]
	S �b�Ńt�F�[�h�A�E�g����Ƃ��A
	�N���b�N�� C [Hz]�ŗ���Ƃ���ƁA
		S * C [cycles]
	�Ŋ��S�ɉ��ʂ��[���ɂȂ�΂悢�B
	���݂̉��ʂ� V ���Ƃ���ƁA
	S * C / V [cycles]
	���Ƃ� V ���P�������Ă䂯�΂悢�B
	�Ȃ� V ���P���������ɁAD��������ꍇ��
		S * C / (V / D) [cycles]
	���Ƃ� V ��������B
*/
static void CDROM_FadeOut(Sint32 ms)
{
	if (ms == 0){
		_CurrentCdVolume = 0;
		_bFadeOut = FALSE;
		_bFadeIn  = FALSE;
		_FadeCycle = 0;
	}
	else if(_CurrentCdVolume > 0){
		_FadeCycle = ((7159090.0 / ((double)_CurrentCdVolume / (double)_VolumeStep)) * (double)ms) / 1000.0;
		_bFadeOut	= TRUE;
		_bFadeIn	= FALSE;
	}
	else {
		_bFadeOut = FALSE;
		_bFadeIn  = FALSE;
		_FadeCycle = 0;
	}
}


static void
CDROM_FadeIn(
	Sint32			ms)
{
	if (ms == 0)
	{
		_CurrentCdVolume = _InitialCdVolume;
		_bFadeOut = FALSE;
		_bFadeIn  = FALSE;
		_FadeCycle = 0;
	}
	else if (_InitialCdVolume - _CurrentCdVolume > 0)
	{
		_FadeCycle = ((7159090.0 / (((double)_InitialCdVolume - (double)_CurrentCdVolume) / (double)_VolumeStep)) * (double)ms) / 1000.0;
		_bFadeOut = FALSE;
		_bFadeIn  = TRUE;
	}
	else
	{
		_bFadeOut = FALSE;
		_bFadeIn  = FALSE;
		_FadeCycle = 0;
	}
}



//=============================================================================
// 
// Cd write access
// 
//=============================================================================
void CD_write(word addr,byte data)
{
    switch(addr & 0x0f) {
      case 0x0:		// $1800 write: resets the command input
        _Port[0] = 0xD0;	// = need more data
        _bCommandReset = TRUE;
        _bCommandReceived = FALSE;
        _bCommandDone = FALSE;
        _bDriveBusy = FALSE;
        _ResetDelayCount = 10;
        //			PRINTF("$1800 <-- $81: command reset?? $1800 = 0xd0");

        /* reset irq status */
        _Port[3] = 0;
        update_irq_state();
        return;
        
      case 0x1:		// $1801
        _Port[1] = data;
        if (data == 0x81) {
            // _ArgsLeft > 0 �܂�R�}���h��������t���̂Ƃ���
            // ���Z�b�g���Ȃ��B
            if (_ArgsLeft == 0) {
                //					PRINTF("$1801 <-- $81: cd reset?? $1800 = 0x00");
                _bCommandReset = TRUE;
                _bCommandReceived = FALSE;
                _bCommandDone = FALSE;
                _Port[0] = 0x00;
                
                /* reset irq status */
                _Port[3] = 0;
                update_irq_state();
                return;
            }
        }
        receive_command(data);
        return;
        
      case 0x2:		// $1802
        _Port[2] = data;
        update_irq_state();
        return;
        
        //		case 0x3:		// read only
        //			return;
        
      case 0x4:
        if (data & 2) {
            // cd reset
            _bCommandReset = TRUE;
            _bCommandReceived = FALSE;
            _bCommandDone = FALSE;
            _bDriveBusy = FALSE;
            _ResetDelayCount = 10;
            stop_play_track();
            
            /* reset irq status */
            _Port[3] = 0;
            update_irq_state();
        }
        _Port[4] = data;
        return;

      case 0x7:	// $1807: D7=1 enables backup ram
        if (data & 0x80)
          _bBRAMEnabled = TRUE;
        return;

      case 0x8:
//        ADPCM_SetAddrLo(data);
        return;

      case 0x9:
//        ADPCM_SetAddrHi(data);
        return;

      case 0xA:
//        ADPCM_WriteBuffer(data);
        return;

      case 0xB:	// adpcm dma
        if (data & 0x03)
          {
//              while (_ReadByteCount > 0)
//                ADPCM_WriteBuffer(read_1801());
          }

        /*	$180C �� D2 �� ADPCM �o�b�t�@�ɏ������݂��s�Ȃ��Ă���
				���΂炭�̊ԃZ�b�g�����悤���B */
        if (_ReadByteCount == 0)
          _Port[0xC] &= ~4;		// busy writing data
        else
          _Port[0xC] |= 4;

        _Port[0xB] = data;
        return;

        //		case 0xC:		// read-only
        //			return;

      case 0xD:
//        if (data & 0x80)
//          ADPCM_Reset();

        // D5 �� D6 ���Z�b�g����ƍĐ��J�n�H
//        ADPCM_Repeat((data & 0x20) == 0x20);
//        ADPCM_Play((data & 0x40) == 0x40); //Kitao�X�V�BBit6�����Z�b�g���ꂽ�ꍇ���Đ�����悤�ɂ����B�i�X�`�[���n�[�c�j

//        if (data & 0x10)
//          ADPCM_SetLength();

//        if (data & 0x08)
//          ADPCM_SetReadAddr();

//        if (data & 0x03) //Kitao�X�Vv0.52�Bdata��0x02�̂Ƃ����Z�b�g����悤�ɂ����B�V�O����ZIRIA�ɂāB���̎����ł��܂��ʖځBdata��0x03�̂Ƃ���0x02(ZIRAIA)�ŉ�������ɈႢ������͂��B
//          ADPCM_SetWriteAddr();

        _Port[0xd] = data;
        return;

      case 0xE:		// Set ADPCM playback rate
//        ADPCM_SetFreq(32 * 1000 / (16 - (data & 15)));
        return;

      case 0xF:
        switch (data & 0xF)
          {
            case 0:	// �t�F�[�h�A�E�g����
              CDROM_FadeIn(0);
//              ADPCM_FadeIn(0);
              break;

            case 8:	// fade out CD (6[s])
            case 9:
              CDROM_FadeOut(6000);
              break;

            case 0xA: // fade out ADPCM (6[s])
              //PRINTF("ADPCM fade (6[s])");
//              ADPCM_FadeOut(6000);
              break;

            case 0xC:
            case 0xD:
              CDROM_FadeOut(2500);
              break;

            case 0xE: // fade out ADPCM (2.5[s])
              //PRINTF("ADPCM fade (2.5[s])");
//              ADPCM_FadeOut(2500);
              break;
          }
        return;
    }
}

void ACD_init(void)
{

}


void CD_init(void)
{
	_bCdromInit = FALSE;

//	CDROM_SetCdVolume(APP_GetCdVolume());//Kitao�ǉ�
	_bFadeOut = FALSE;
	_bFadeIn  = FALSE;
	_bFastCD = TRUE; //Kitao�ǉ�
	_FadeOut1 = 0; //Kitao�ǉ�
	_FadeOut2 = 0; //Kitao�ǉ�

//	if (!CDIF_Init(cd_callback))
//		return -1;

//	if (!check_cdrom2_disc())
//		return -1;

//	ADPCM_SetNotificationFunction(adpcm_state_notification_callback_function);

	_bCdromInit = TRUE;
	_bError = FALSE;

	memset(_ReadBuffer,0,sizeof(_ReadBuffer));

//	return 0;
}



Uint8 CD_read( u16 physAddr )
{
	if ((physAddr & 0x18c0) == 0x18c0) {
		switch (physAddr & 0x18cf){
			case 0x18c1: return 0xaa;
			case 0x18c2: return 0x55;
			case 0x18c3: return 0;
			case 0x18c5: return 0xaa;
			case 0x18c6: return 0x55;
			case 0x18c7: return 0x03;
		}
	}

	switch (physAddr & 0xf)
	{
		case 0x0:
//			PRINTF("$1800 = %02x\n", _Port[0]);
			if (_Port[2] & 0x80)
			{
				if (_CheckCountAfterRead == 0)
				{
					_Port[3] &= ~0x20;
					_bDriveBusy = FALSE;
					update_irq_state();
				}
				return _Port[0] & ~0x40;
			}
			else if (_bCommandReceived && !_bCommandDone)
			{
				return _Port[0] & ~0x40;
			}
			else if (_bDriveBusy)
			{
				return _Port[0] | 0x80;
			}
			else if (_ResetDelayCount > 0)
			{
				--_ResetDelayCount;
				return _Port[0] & ~0x40;
			}
			return _Port[0] | 0x40;

		case 0x1:
			return read_1801();

		case 0x2: // read/write port (control port)
			return _Port[2];

		case 0x3:	// �o�b�N�A�b�v���������֎~����B (read only)
					// status-read port
			_bBRAMEnabled = FALSE;
			/* switch left/right of digitized cd playback */
			_Port[3] ^= 2;
			return _Port[3] | 0x10;

		case 0x4:
			return _Port[4];

		case 0x5:
			return ++_Port[5];

		case 0x6:
			return ++_Port[6];

		case 0x7:
			// CD subchannel read
			return _Port[7];

		case 0x8:	// CD-ROM ����Z�N�^��ǂݏo���B
			return read_1808();

		case 0xa:
//			return ADPCM_ReadBuffer();
			return 0;

		case 0xb:
			return _Port[0xb] & ~1;

		case 0xc:
			// D0: �����_�� ADPCM �̍Đ����I���܂��͒�~����Ă���ꍇ�͂P 
			// D2: CD --> DMA �]�����̓[���H (1: busy prepareing ADPCM data)
			// D3: ADPCM �Đ����͂P
			// D7: �O��� $180A �̓ǂݏo�������ła�t�r�x�̏ꍇ�́AD7 = 1 �ƂȂ�B
			if (! 1/*ADPCM_IsPlaying()*/)
			{
				_Port[0xc] |= 1;
				_Port[0xc] &= ~8;
			}
			else
			{
				_Port[0xc] &= ~1;
				_Port[0xc] |= 8;
			}
			return _Port[0xc];

		case 0xd:
			return _Port[0xd];
	}

	return 0;
}


static void increment_acaddr( ACIO* port )
{
	if (port->control & 1)		// CONFIRMED:  D0 enables base / offset increment
	{
		if (port->control & 0x10)	// CONFIRMED: D4 selects base / offset to be incremented
		{
			port->base += port->increment;
			port->base &= 0xffffff;
		}
		else
		{
			port->offset += port->increment;
		}
	}
}


Uint8 ACD_read( Uint16 physAddr)
{
	ACIO		*port = &_Ac[(physAddr >> 4) & 3];
	Uint8		ret;

	if ((physAddr & 0x1ae0) == 0x1ae0)
	{
		switch (physAddr & 0x1aef)
		{
			case 0x1ae0:
				return (Uint8)_AcShift;
			case 0x1ae1:
				return (Uint8)(_AcShift >> 8);
			case 0x1ae2:
				return (Uint8)(_AcShift >> 16);
			case 0x1ae3:
				return (Uint8)(_AcShift >> 24);
			case 0x1ae4:
				return (Uint8)(_AcShiftBits);
			case 0x1ae5:
				return _0x1ae5;
			case 0x1aee:
				return 0x10;
			case 0x1aef:
				return 0x51;
		}
		return 0xff;
	}

	switch (physAddr & 0xf)
	{
		case 0x0:
		case 0x1:
			if (port->control & 2)
				ret = _AcRam[(port->base + port->offset) & 0x1fffff];
			else
				ret = _AcRam[port->base & 0x1fffff];
			increment_acaddr(port);
			return ret;

		case 0x2:	return (Uint8)(port->base);
		case 0x3:	return (Uint8)(port->base >> 8);
		case 0x4:	return (Uint8)(port->base >> 16);
		case 0x5:	return (Uint8)(port->offset);
		case 0x6:	return (Uint8)(port->offset >> 8);
		case 0x7:	return (Uint8)(port->increment);
		case 0x8:	return (Uint8)(port->increment >> 8);
		case 0x9:	return port->control;
		case 0xa:	return 0;
		default:
			break;
	}
	return 0xff;
}


void ACD_write( Uint16 physAddr, Uint8 data)
{
//	_bAcUse = TRUE;//Kitao�ǉ��B�A�[�P�[�h�J�[�h�𗘗p������B�X�e�[�g�Z�[�u���ɃA�[�P�[�h�J�[�h�֘A���Z�[�u����B

	if ((physAddr & 0x1ae0) == 0x1ae0) {
		switch (physAddr & 0xf) {
			case 0: _AcShift = (_AcShift & ~0x000000ff) | data;         return;
			case 1: _AcShift = (_AcShift & ~0x0000ff00) | (data << 8);  return;
			case 2: _AcShift = (_AcShift & ~0x00ff0000) | (data << 16); return;
			case 3:	_AcShift = (_AcShift & ~0xff000000) | (data << 24); return;
			case 4:
				if ((_AcShiftBits = data & 0xf) != 0) {
					if (_AcShiftBits < 8) _AcShift <<= _AcShiftBits;
					else                  _AcShift >>= 16 - _AcShiftBits;
				}
				return;
			case 5:
				_0x1ae5 = data;
				return;
		}
	}
	else
	{
		ACIO		*port = &_Ac[(physAddr >> 4) & 3];

		switch (physAddr & 0xf)
		{
			case 0x0:
			case 0x1:
				if (port->control & 2)
					_AcRam[(port->base + port->offset) & 0x1fffff] = data;
				else
					_AcRam[port->base & 0x1fffff] = data;
				increment_acaddr(port);
				return;

			case 0x2:
				port->base = (port->base & ~0xff) | data;
				return;
			case 0x3:
				port->base = (port->base & ~0xff00) | (data << 8);
				return;
			case 0x4:
				port->base = (port->base & ~0xff0000) | (data << 16);
				return;
			case 0x5:
				port->offset = (port->offset & ~0xff) | data;
				return;
			case 0x6:
				port->offset = (port->offset & ~0xff00) | (data << 8);
				if ((port->control & 0x60) == 0x40)
				{
					if (port->control & 0x08)
						port->base += port->offset + 0xff0000;
					else
						port->base += port->offset;
					port->base &= 0xffffff;
				}
				return;
			case 0x7:
				port->increment = (port->increment & ~0xff) | data;
				return;
			case 0x8:
				port->increment = (port->increment & ~0xff00) | (data << 8);
				return;
			case 0x9:
				port->control = data & 0x7f;		// D7 is not used
				return;
			case 0xa:
				// value written is not used 
				if ((port->control & 0x60) == 0x60)
				{
					port->base += port->offset;
					port->base &= 0xffffff;
				}
				return;
		}
	}
}




// ******************************************************************************************
// ******************************************************************************************
// ******************************************************************************************
#else// *************************************************************************************
// ******************************************************************************************
// ******************************************************************************************
// ******************************************************************************************

// =-=-=- For CDROM^2
#define CD_FRAMES 75
#define CD_SECS 60
#define CD_BUF_LENGTH 8


#define AC_ENABLE_OFFSET_BASE_6 0x40
#define AC_ENABLE_OFFSET_BASE_A 0x20
#define AC_INCREMENT_BASE 0x10
#define AC_USE_OFFSET 0x02
#define AC_ENABLE_INC 0x01

#define CD_MSF_OFFSET 150

//void cd_test_read(char *p,int s, int num);

char cd_last_read[1024]={"test"};

byte cd_fade;
DWORD  msf2nb_sect(byte min, byte sec, byte fra);
void    nb_sect2msf(DWORD lsn,byte *min, byte *sec, byte *frm);
void fill_cd_info();

Track CD_track[0x100];

extern u8 binbcd[0x100];
extern u8 bcdbin[0x100];

byte cd_port_1800 = 0;
byte cd_port_1801 = 0;
byte cd_port_1802 = 0;
byte cd_port_1804 = 0;
byte cd_port_180b = 0;
byte pce_cd_adpcm_trans_done = 0;
byte pce_cd_curcmd;
byte pce_cd_cmdcnt;
byte cd_sectorcnt;

DWORD packed_iso_filesize = 0;
byte cd_sector_buffer[0x2000];	// contain really data
// DWORD pce_cd_read_datacnt;
DWORD pce_cd_sectoraddy;
DWORD pce_cd_read_datacnt;
byte pce_cd_sectoraddress[3];
byte pce_cd_temp_dirinfo[4];
byte pce_cd_temp_play[4];
byte pce_cd_temp_stop[4];
byte *cd_read_buffer;
byte pce_cd_dirinfo[4];
// struct cdrom_tocentry pce_cd_tocentry;
//char cdsystem_path[256];
//extern char   *pCartName;
//extern char snd_bSound;
// Pre declaration of reading function routines


// internal function
//int cd_toc_read(void);


void issue_ADPCM_dma(void);
void pce_cd_handle_command(void);

//=============================================================================
//
//
//
//=============================================================================
static u8 CD_read_1801(void)
{

}

//=============================================================================
//
//
//
//=============================================================================
static u8 CD_read_1808(void)
{


}



//=============================================================================
//
//
//
//=============================================================================
void CD_init(void)
{
//    CD_emulation = 0;
}

void CD_write(word A,byte V)
{
    switch(A&15){
      case 7: cd.backup = ENABLE; return;
        /*	case 8: io.adpcm_ptr.B.l = V; return;
		case 9: io.adpcm_ptr.B.h = V; return;
		case 0xa: PCM[io.adpcm_wptr++] = V; return;
		case 0xd:
			if (V&4) io.adpcm_wptr = io.adpcm_ptr.W;
			else { io.adpcm_rptr = io.adpcm_ptr.W; io.adpcm_firstread = 1; }
			return;
         */
      case 0: if (V == 0x81) cd_port_1800 = 0xD0; return;

      case 1:
        cd_port_1801 = V;
        if (!pce_cd_cmdcnt) {
            switch (V) {
              case 0x81:	// Another Reset?
                cd_port_1800 = 0x40;
                return;
              case 0:		// RESET?
              case 3:		// Get System Status?
              case 8:		// Read Sector
              case 0xD8:	// Play Audio?
              case 0xD9:	// Play Audio?
              case 0xDA:	// Pause Audio?
              case 0xDD:	// Read Q Channel?
              case 0xDE:	// Get Directory Info?
              default:
                return;
            }
        }
        return;

      case 2:
        if ((!(cd_port_1802 & 0x80)) && (V & 0x80)) {
            cd_port_1800 &= ~0x40;
        } else if ((cd_port_1802 & 0x80) && (!(V & 0x80))) {
            cd_port_1800 |= 0x40;
            if (pce_cd_adpcm_trans_done) {
                cd_port_1800 |= 0x10;
                pce_cd_curcmd = 0x00;
                pce_cd_adpcm_trans_done = 0;
            }

            if (cd_port_1800 & 0x08) {
                if (cd_port_1800 & 0x20) {
                    cd_port_1800 &= ~0x80;
                } else if (!pce_cd_read_datacnt) {
                    if (pce_cd_curcmd == 0x08) {
                        if (!--cd_sectorcnt) {
                            cd_port_1800 |= 0x10;	/* wrong */
                            pce_cd_curcmd = 0x00;
                        } else {
                            pce_cd_read_sector();
                        }
                    } else {
                        if (cd_port_1800 & 0x10) {
                            cd_port_1800 |= 0x20;
                        } else {
                            cd_port_1800 |= 0x10;
                        }
                    }
                } else {
                    pce_cd_read_datacnt--;
                }
            } else {
                pce_cd_handle_command();
            }
        }

        cd_port_1802 = V;
        return;

      case 4:
        if (V & 2) {
            // Reset asked
            // do nothing for now
            //            CD_emulation=1;
            //            cd_toc_read();
            fill_cd_info();

            _Wr6502(0x222D, 1);
            // This byte is set to 1 if a disc if present
            //cd_port_1800 &= ~0x40;
            cd_port_1804 = V;
        } else {
            // Normal utilisation
            cd_port_1804 = V;
            // cd_port_1800 |= 0x40; // Maybe the previous reset is enough
            // cd_port_1800 |= 0xD0;
            // Indicates that the Hardware is ready after such a reset
        }
        return;

      case 8:
        cd.adpcm_ptr.B.l = V;
        return;

      case 9:
        cd.adpcm_ptr.B.h = V;
        return;

      case 0x0A:
        cd.PCM[cd.adpcm_wptr++] = V;
        return;

      case 0x0B:		// DMA enable ?
        if ((V & 2) && (!(cd_port_180b & 2))) {
            issue_ADPCM_dma ();
            cd_port_180b = V;
            return;
        }
        /* TEST */
        if (!V) {
            cd_port_1800 &= ~0xF8;
            cd_port_1800 |= 0xD8;
        }
        cd_port_180b = V;
        return;

      case 0x0C:		/* TEST, not nyef code */
        // well, do nothing
        return;

      case 0x0D:
        if ((V & 0x03) == 0x03) {
            cd.adpcm_dmaptr = cd.adpcm_ptr.W;	// set DMA pointer
        }

        if (V & 0x04) {
            cd.adpcm_wptr = cd.adpcm_ptr.W;	// set write pointer
        }

        if (V & 0x08) {		// set read pointer
            cd.adpcm_rptr = cd.adpcm_ptr.W;
            cd.adpcm_firstread = 2;
        }

        if (V & 0x80) {			// ADPCM reset
        } else {			// Normal ADPCM utilisation
        }
        return;

      case 0xe:		// Set ADPCM playback rate
        cd.adpcm_rate = 32 / (16 - (V & 15));
        return;

      case 0xf:		// don't know how to use it
        cd_fade = V;
        return;
    }			// A&15 switch, i.e. CD ports
}


//=============================================================================
// 
// CD read access
// 
//=============================================================================
byte CD_read(word A)
{
    if((A&0x18c0)==0x18c0) {
        switch (A & 15) {
          case 5:
          case 1:  return 0xAA;
          case 2:
          case 6:  return 0x55;
          case 3:
          case 7:  return 0x03;
            // case 15: // ACD support ?
            //  return 0x51;
        }
        return 0xff;
    }
    
    switch(A&15){
      case 0:
        return cd_port_1800;	// return 0x40; // ready ?
        break;
      case 1: {
          byte retval;
          
          if(cd_read_buffer) {
              retval = *cd_read_buffer++;
              if (pce_cd_read_datacnt == 2048) {
                  pce_cd_read_datacnt--;
              }
              if (!pce_cd_read_datacnt)
                cd_read_buffer = 0;
          } else
            retval = 0;
          return retval;
      }
        
      case 2: return cd_port_1802;	// Test
        //	case 3: return io.backup = DISABLE;
      case 3: {
          static byte tmp_res = 0x02;
          tmp_res = 0x02 - tmp_res;
          cd.backup = DISABLE;
          /* TEST */// return 0x20;
          return tmp_res | 0x20;
      }
      case 4: return cd_port_1804;	// Test
      case 5: return 0x50;			// Test
      case 6: return 0x05;			// Test
      case 0x0A:
        if (!cd.adpcm_firstread)
          return cd.PCM[cd.adpcm_rptr++];
        else {
            cd.adpcm_firstread--;
            return NODATA;
        }
        
      case 0x0B: return 0x00;			// Test
      case 0x0C: return 0x01;			// Test
      case 0x0D: return 0x00;			// Test
      case 8:
        if (pce_cd_read_datacnt) {
            byte retval;
            if (cd_read_buffer) {
                retval = *cd_read_buffer++;
            } else
              retval = 0;
            
            if (!--pce_cd_read_datacnt) {
                cd_read_buffer = 0;
                if (!--cd_sectorcnt) {
                    cd_port_1800 |= 0x10;
                    pce_cd_curcmd = 0;
                } else {
                    pce_cd_read_sector();
                }
            }
            return retval;
        }
        break;
    }
	return 0xff;
}



//=============================================================================
//
//
//
//=============================================================================
void fill_cd_info()
{
    byte Min, Sec, Fra;
    byte current_track;

    // Track 1 is almost always a audio avertising track
    // 30 sec. seems usual
    
    CD_track[1].beg_min = binbcd[00];
    CD_track[1].beg_sec = binbcd[02];
    CD_track[1].beg_fra = binbcd[00];
    
    CD_track[1].type = 0;
    CD_track[1].beg_lsn = 0;	// Number of sector since the
    // beginning of track 1
    
    CD_track[1].length = 47 * CD_FRAMES + 65;
    
    // CD_track[0x01].length=53 * CD_FRAMES + 65;
    
    // CD_track[0x01].length=0 * CD_FRAMES + 16;
    
    nb_sect2msf (CD_track[1].length, &Min, &Sec, &Fra);
    
    // Fra = CD_track[0x01].length % CD_FRAMES;
    // Sec = (CD_track[0x01].length) % (CD_FRAMES * CD_SECS) / CD_SECS;
    // Min = (CD_track[0x01].length) (CD_FRAMES * CD_SECS);
    
    // Second track is the main code track
    
    CD_track[2].beg_min = binbcd[bcdbin[CD_track[1].beg_min] + Min];
    CD_track[2].beg_sec = binbcd[bcdbin[CD_track[1].beg_sec] + Sec];
    CD_track[2].beg_fra = binbcd[bcdbin[CD_track[1].beg_fra] + Fra];
    
    CD_track[2].type = 4;
    CD_track[2].beg_lsn =
      msf2nb_sect (bcdbin[CD_track[2].beg_min] - bcdbin[CD_track[1].beg_min],
                   bcdbin[CD_track[2].beg_sec] - bcdbin[CD_track[1].beg_sec],
                   bcdbin[CD_track[2].beg_fra] - bcdbin[CD_track[1].beg_fra]);

    CD_track[0x02].length = 140000;
    
    // Now most track are audio
    for (current_track = 3; current_track < bcdbin[nb_max_track];current_track++) {
        
        Fra = (byte) (CD_track[current_track - 1].length % CD_FRAMES);
        Sec = (byte) ((CD_track[current_track - 1].length / CD_FRAMES) % CD_SECS);
        Min = (byte) ((CD_track[current_track - 1].length / CD_FRAMES) / CD_SECS);

        CD_track[current_track].beg_min = binbcd[bcdbin[CD_track[current_track - 1].beg_min] + Min];
        CD_track[current_track].beg_sec = binbcd[bcdbin[CD_track[current_track - 1].beg_sec] + Sec];
        CD_track[current_track].beg_fra = binbcd[bcdbin[CD_track[current_track - 1].beg_fra] + Fra];
        
        CD_track[current_track].type = 0;
        CD_track[current_track].beg_lsn =
          msf2nb_sect (bcdbin[CD_track[current_track].beg_min] -
                       bcdbin[CD_track[1].beg_min],
                       bcdbin[CD_track[current_track].beg_sec] -
                       bcdbin[CD_track[1].beg_sec],
                       bcdbin[CD_track[current_track].beg_fra] -
                       bcdbin[CD_track[1].beg_fra]);
        // 1 min for all
        CD_track[current_track].length = 1 * CD_SECS * CD_FRAMES;
    }
    
    // And the last one is generally also code

    Fra = (byte) (CD_track[nb_max_track - 1].length % CD_FRAMES);
    Sec = (byte) ((CD_track[nb_max_track - 1].length / CD_FRAMES) % CD_SECS);
    Min = (byte) ((CD_track[nb_max_track - 1].length / CD_FRAMES) / CD_SECS);
    
    CD_track[nb_max_track].beg_min = binbcd[bcdbin[CD_track[nb_max_track - 1].beg_min] + Min];
    CD_track[nb_max_track].beg_sec = binbcd[bcdbin[CD_track[nb_max_track - 1].beg_sec] + Sec];
    CD_track[nb_max_track].beg_fra = binbcd[bcdbin[CD_track[nb_max_track - 1].beg_fra] + Fra];
    
    CD_track[nb_max_track].type = 4;
    CD_track[nb_max_track].beg_lsn =
      msf2nb_sect (bcdbin[CD_track[nb_max_track].beg_min] - bcdbin[CD_track[1].beg_min],
                   bcdbin[CD_track[nb_max_track].beg_sec] - bcdbin[CD_track[1].beg_sec],
                   bcdbin[CD_track[nb_max_track].beg_fra] - bcdbin[CD_track[1].beg_fra]);

    CD_track[nb_max_track].length = 14000;

    return;
}



/*
//=============================================================================
//
//
//
//=============================================================================
void cd_test_read(char *p,DWORD s, int num)
{
	char i,fd,n[1024];
	char tn[3];
    char *wp;

    for(i=1;i<0x100;i++){
        if( cd_toc[i].LBA>s )break;
    }
    
    i--;
    tn[0]='0'+(i/10);
    tn[1]='0'+(i%10);
    tn[2]=0;

	core_strcpy( n, HAL_GetRomsPath() );
//    core_strcpy( n, tocName() );

    wp=core_strrchr( n, '/' );
	if(!wp) wp=core_strrchr( n, '\\' );

	wp[1]=0;
	
	core_strcat( n, tn);
    core_strcat( n, ".iso");
    //Error_mes(n);

    core_strcpy(cd_last_read,n);

    //mh_print(400,0,n,RGB_WHITE);

    if( (fd = HAL_fd_open(n,HAL_MODE_READ))>=0 ) {
        int a;
        DWORD w;
        a = (s-cd_toc[i].LBA)*2048;
        w= HAL_fd_seek(fd,0, HAL_SEEK_END);
        
        if( a<0 || w<a+2048 ){
			a = a;
            //Error_mes("CD�V�[�N�G���[");
            //mh_print(400,10,"cd seek error",-1);
            //mh_print(400,20,n,-1);
            //mh_print_hex8(400,20,w,-1);
            //mh_print_hex8(400,30,a,-1);
            //mh_print_hex8(400,40,i,-1);
            //mh_print_hex8(400,50,s,-1);
            //mh_print_hex8(400,60,cd_toc[i].LBA,-1);
        }else{
            int rd;
            w = HAL_fd_seek( fd, a, HAL_SEEK_SET);
            rd = HAL_fd_read(fd, p, 2048*num);
            
            //mh_print     (400,10,"Read Status",-1);
            //mh_print_hex8(400,20,w,-1);
            //mh_print_hex8(400,30,a,-1);
            //mh_print_hex8(400,40,rd,-1);
            //mh_print_hex8(400,50,s,-1);
            //mh_print_hex8(400,60,cd_toc[i].LBA,-1);
        }
        HAL_fd_close(fd);
    }else{
		fd = fd;
//        mh_print(0,0,"cd open error",-1);
//        mh_print(0,10,n,-1);
//        pgScreenFlipV();
	}
}*/


//	2�g���b�N�� 3590
DWORD first_sector = 0;

//=============================================================================
//
//
//
//=============================================================================
void read_sector_CD(unsigned char *p, DWORD sector)
{
    int i;

    HAL_PCE_CD_Stop();
    
    if ((sector >= first_sector) && (sector <= first_sector + CD_BUF_LENGTH - 1)) {
        core_memcpy(p, cd.cd_buf + 2048 * (sector - first_sector), 2048);
        return;
    }
    else {
#if 1
		cd_test_read(cd.cd_buf, sector, CD_BUF_LENGTH);
#else
        for(i=0;i<CD_BUF_LENGTH/*8*/;i++) {
			cd_test_read(cd.cd_buf + 2048 * i, sector + i);
		}
#endif
        first_sector = sector;
        core_memcpy(p, cd.cd_buf, 2048);
    }
} 


//=============================================================================
//
//
//
//=============================================================================
void pce_cd_read_sector(void)
{
    read_sector_CD( cd_sector_buffer, pce_cd_sectoraddy );
    /* Avoid sound jiggling when accessing some sectors */
    pce_cd_sectoraddy++;
    pce_cd_read_datacnt = 2048;
    cd_read_buffer = cd_sector_buffer;
    /* restore sound volume */
}


//=============================================================================
//
//
//
//=============================================================================
void lba2msf (int lba, unsigned char *msf)
{
    lba += CD_MSF_OFFSET;
    msf[0] = binbcd[lba / (CD_SECS * CD_FRAMES)];
    lba %= CD_SECS * CD_FRAMES;
    msf[1] = binbcd[lba / CD_FRAMES];
    msf[2] = binbcd[lba % CD_FRAMES];
}


//=============================================================================
//
//
//
//=============================================================================
DWORD msf2nb_sect (byte min, byte sec, byte frm)
{
    DWORD result = frm;
    result += sec * CD_FRAMES;
    result += min * CD_FRAMES * CD_SECS;
    return result;
}

//=============================================================================
//
//
//
//=============================================================================
void nb_sect2msf (DWORD lsn, byte * min, byte * sec, byte * frm)
{
    (*frm) = (byte) (lsn % CD_FRAMES);
    lsn /= CD_FRAMES;
    (*sec) = (byte) (lsn % CD_SECS);
    (*min) = (byte) (lsn / CD_SECS);
    return;
}

void pce_cd_set_sector_address(void);


//=============================================================================
//
//
//
//=============================================================================
void pce_cd_handle_command(void)
{
    if (pce_cd_cmdcnt) {
        if (--pce_cd_cmdcnt)
          cd_port_1800 = 0xd0;
        else
          cd_port_1800 = 0xc8;
        
        switch (pce_cd_curcmd) {
          case 0x08:
            if (!pce_cd_cmdcnt) {
                cd_sectorcnt = cd_port_1801;
                pce_cd_set_sector_address();
                pce_cd_read_sector();
                
                /* TEST */
                // cd_port_1800 = 0xD0; // Xanadu 2 doesn't block but still crash
                /* TEST */
                
                /* TEST ZEO
                  if (Rd6502(0x20ff)==0xfe)
                    cd_port_1800 = 0x98;
                  else
                    cd_port_1800 = 0xc8;
                 ******** */
            } else
              pce_cd_sectoraddress[3 - pce_cd_cmdcnt] = cd_port_1801;
            break;
            
          case 0xd8:
            
            pce_cd_temp_play[pce_cd_cmdcnt] = cd_port_1801;
            
            if (!pce_cd_cmdcnt) {
                cd_port_1800 = 0xd8;
            }
            break;
            
          case 0xd9:
            pce_cd_temp_stop[pce_cd_cmdcnt] = cd_port_1801;
            if (!pce_cd_cmdcnt) {
                cd_port_1800 = 0xd8;

                HAL_PCE_CD_Play(bcdbin[pce_cd_temp_play[2]],1);
                
                //cd_PlayTrack(bcdbin[pce_cd_temp_play[2]]);
                
                /*
               if (pce_cd_temp_stop[3] == 1)
                 osd_cd_play_audio_track(bcdbin[pce_cd_temp_play[2]]);
               else
                 */
                if ((pce_cd_temp_play[0] | pce_cd_temp_play[1] | pce_cd_temp_stop[0] | pce_cd_temp_stop[1]) == 0) {
                    //osd_cd_play_audio_track(bcdbin[pce_cd_temp_play[2]]);
                    //cd_PlayTrack(bcdbin[pce_cd_temp_play[2]]);
                } else {
                    //osd_cd_play_audio_range(bcdbin[pce_cd_temp_play[2]], bcdbin[pce_cd_temp_play[1]], bcdbin[pce_cd_temp_play[0]], bcdbin[pce_cd_temp_stop[2]], bcdbin[pce_cd_temp_stop[1]], bcdbin[pce_cd_temp_stop[0]]);

                    // ��������
                    //bcdbin[pce_cd_temp_play[2]], bcdbin[pce_cd_temp_play[1]], bcdbin[pce_cd_temp_play[0]];
                    // �����܂ł͈̔͂�
                    //bcdbin[pce_cd_temp_stop[2]], bcdbin[pce_cd_temp_stop[1]], bcdbin[pce_cd_temp_stop[0]];
                    // CD�Đ����ė~�����炵���ƌ����Ă��ł���E�E�E�g���b�N�ԍ��͂ǂ��Ȃ�́H�����H
                    //cd_PlayWithRange(bcdbin[pce_cd_temp_play[2]], bcdbin[pce_cd_temp_play[1]], bcdbin[pce_cd_temp_play[0]],
                    //                 bcdbin[pce_cd_temp_stop[2]], bcdbin[pce_cd_temp_stop[1]], bcdbin[pce_cd_temp_stop[0]]);
                }
            }
            break;
            
          case 0xde:
            if (pce_cd_cmdcnt)
              pce_cd_temp_dirinfo[pce_cd_cmdcnt] = cd_port_1801;
            else {
                // We have received two arguments in pce_cd_temp_dirinfo
                // We can use only one
                // There's an argument indicating the kind of info we want
                // and an optional argument for track number
                pce_cd_temp_dirinfo[0] = cd_port_1801;
                
				switch (pce_cd_temp_dirinfo[1]) {
                  case 0:
                    // We want info on number of first and last track
/*                    
                    switch (CD_emulation) {
                      case 2:
                      case 3:
                        pce_cd_dirinfo[0] = binbcd[01];	// Number of first track  (BCD)
                        pce_cd_dirinfo[1] = binbcd[nb_max_track];	// Number of last track (BCD)
                        break;
                      case 1: {
                          int first_track, last_track;
                          // ������
                          //osd_cd_nb_tracks (&first_track, &last_track);
                          cd_nb_tracks(&first_track,&last_track);
                          pce_cd_dirinfo[0] = binbcd[first_track];
                          pce_cd_dirinfo[1] = binbcd[last_track];
                      }
                        break;
                    }// switch CD emulation
*/
					pce_cd_dirinfo[0] = binbcd[1];
					pce_cd_dirinfo[1] = binbcd[nb_max_track];
  
                    cd_read_buffer = pce_cd_dirinfo;
                    pce_cd_read_datacnt = 2;
                    break;
                    
                  case 2:
                    // We want info on the track whose number is pce_cd_temp_dirinfo[0]
/*                    switch (CD_emulation) {
                      case 2:
                      case 3:
                        pce_cd_dirinfo[0] = CD_track[bcdbin[pce_cd_temp_dirinfo[0]]].beg_min;
                        pce_cd_dirinfo[1] = CD_track[bcdbin[pce_cd_temp_dirinfo[0]]].beg_sec;
                        pce_cd_dirinfo[2] = CD_track[bcdbin[pce_cd_temp_dirinfo[0]]].beg_fra;
                        pce_cd_dirinfo[3] = CD_track[bcdbin[pce_cd_temp_dirinfo[0]]].type;
                        break;
                      case 1: {
                          int Min, Sec, Fra, Ctrl;
                          //byte *buffer = (byte *) alloca (7);
                          // ������
                          //osd_cd_track_info (bcdbin[pce_cd_temp_dirinfo[0]], &Min, &Sec, &Fra, &Ctrl);
                          cd_track_info(bcdbin[pce_cd_temp_dirinfo[0]], &Min, &Sec, &Fra, &Ctrl);
                          pce_cd_dirinfo[0] = binbcd[Min];
                          pce_cd_dirinfo[1] = binbcd[Sec];
                          pce_cd_dirinfo[2] = binbcd[Fra];
                          pce_cd_dirinfo[3] = Ctrl;
#ifdef WIN32
                          LogDump("The control byte of the audio track #%d is 0x%02X\n", bcdbin[pce_cd_temp_dirinfo[0]], pce_cd_dirinfo[3]);
#endif//WIN32
                          break;
                      }		// case CD emulation = 1
                    }		// switch CD emulation
*/
                    pce_cd_dirinfo[0] = binbcd[cd_toc[bcdbin[pce_cd_temp_dirinfo[0]]].min];
                    pce_cd_dirinfo[1] = binbcd[cd_toc[bcdbin[pce_cd_temp_dirinfo[0]]].sec];
                    pce_cd_dirinfo[2] = binbcd[cd_toc[bcdbin[pce_cd_temp_dirinfo[0]]].fra];
                    pce_cd_dirinfo[3] = binbcd[cd_toc[bcdbin[pce_cd_temp_dirinfo[0]]].type];
                    
                    pce_cd_read_datacnt = 3;
                    cd_read_buffer = pce_cd_dirinfo;
                    break;
                    
                  case 1:
                    pce_cd_dirinfo[0] = cd_toc[nb_max_track].min;//0x25;
                    pce_cd_dirinfo[1] = cd_toc[nb_max_track].sec;//0x06;
                    pce_cd_dirinfo[2] = cd_toc[nb_max_track].fra;//0x00;
                    pce_cd_read_datacnt = 3;
                    cd_read_buffer = pce_cd_dirinfo;
                    break;
                }		// switch command of request 0xde
            }			// end if of request 0xde (receiving command or executing them)
        }			// switch of request
    }				// end if of command arg or new request
    else {
        // it's an command ID we're receiving
        switch (cd_port_1801) {
          case 0x00:
            cd_port_1800 = 0xD8;
            break;
          case 0x08:
            pce_cd_curcmd = cd_port_1801;
            pce_cd_cmdcnt = 4;
            break;
          case 0xD8:
            pce_cd_curcmd = cd_port_1801;
            pce_cd_cmdcnt = 4;
            break;
          case 0xD9:
            pce_cd_curcmd = cd_port_1801;
            pce_cd_cmdcnt = 4;
            break;
          case 0xDA:
            pce_cd_curcmd = cd_port_1801;
            pce_cd_cmdcnt = 0;

            HAL_PCE_CD_Stop();

            //if (CD_emulation == 1)
				//cd_PlayStop();    //osd_cd_stop_audio ();
            break;
          case 0xDE:
            /* Get CD directory info */
            /* First arg is command? */
            /* Second arg is track? */
            cd_port_1800 = 0xd0;
            pce_cd_cmdcnt = 2;
            pce_cd_read_datacnt = 3;	/* 4 bytes */
            pce_cd_curcmd = cd_port_1801;
            break;
        }
        
        /*
        if (cd_port_1801 == 0x00) {
            cd_port_1800 = 0xd8;
        } else if (cd_port_1801 == 0x08) {
            pce_cd_curcmd = cd_port_1801;
            pce_cd_cmdcnt = 4;
        } else if (cd_port_1801 == 0xd8) {
            pce_cd_cmdcnt = 4;
            pce_cd_curcmd = cd_port_1801;
        } else if (cd_port_1801 == 0xd9) {
            pce_cd_cmdcnt = 4;
            pce_cd_curcmd = cd_port_1801;
        } else if (cd_port_1801 == 0xde) {
            // Get CD directory info
            // First arg is command?
            // Second arg is track?
            cd_port_1800 = 0xd0;
            pce_cd_cmdcnt = 2;
            pce_cd_read_datacnt = 3; // 4 bytes
            pce_cd_curcmd = cd_port_1801;
        }
         */
    }
}



//=============================================================================
//
//
//
//=============================================================================
void pce_cd_set_sector_address(void)
{
    pce_cd_sectoraddy = pce_cd_sectoraddress[0] << 16;
    pce_cd_sectoraddy += pce_cd_sectoraddress[1] << 8;
    pce_cd_sectoraddy += pce_cd_sectoraddress[2];
}

//=============================================================================
//
//
//
//=============================================================================
void issue_ADPCM_dma (void)
{
    while (cd_sectorcnt--) {
        core_memcpy(cd.PCM + cd.adpcm_dmaptr, cd_read_buffer, pce_cd_read_datacnt);
        cd_read_buffer = NULL;
        cd.adpcm_dmaptr += (unsigned short) pce_cd_read_datacnt;
        pce_cd_read_datacnt = 0;
        pce_cd_read_sector ();
    }
    pce_cd_read_datacnt = 0;
    pce_cd_adpcm_trans_done = 1;
    cd_read_buffer = NULL;
}



//=============================================================================
//
//
//
//=============================================================================
void ACD_init(void)
{
    core_memset(&acd,0,sizeof(acd));
}


//=============================================================================
//
//
//
//=============================================================================
void ACD_write(word adr,byte V)
{
  if ((adr&0x1AE0)==0x1AE0) {
        switch(adr & 15) {
          case 0: acd.ac_shift = (acd.ac_shift & 0xffffff00) | V;          break;
          case 1: acd.ac_shift = (acd.ac_shift & 0xffff00ff) | (V << 8);   break;
          case 2: acd.ac_shift = (acd.ac_shift & 0xff00ffff) | (V << 16);  break;
          case 3: acd.ac_shift = (acd.ac_shift & 0x00ffffff) | (V << 24);  break;
          case 4: acd.ac_shiftbits = V & 0x0f;
                  if(acd.ac_shiftbits != 0) {
                    if(acd.ac_shiftbits < 8) {
                      acd.ac_shift <<= acd.ac_shiftbits;
                    }
                    else {
                      acd.ac_shift >>= (16 - acd.ac_shiftbits);
                    }
                  }
				  break;
          case 5:
            acd.ac_unknown4 = V;
          default: break;
        }
        return;
    }
    else {
        byte ac_port = (adr >> 4) & 3;
        switch (adr & 15) {
          case 0:
          case 1:
            if (acd.ac_control[ac_port] & AC_USE_OFFSET) {
                acd.ac_extra_mem[((acd.ac_base[ac_port] + acd.ac_offset[ac_port]) & 0x1fffff)] = V;
            }
            else {
                acd.ac_extra_mem[((acd.ac_base[ac_port]) & 0x1fffff)] = V;
            }
            
            if (acd.ac_control[ac_port] & AC_ENABLE_INC) {
                if (acd.ac_control[ac_port] & AC_INCREMENT_BASE) {
					acd.ac_base[ac_port] += acd.ac_incr[ac_port];
					acd.ac_base[ac_port] &= 0xffffff;
				}
                else {
					acd.ac_offset[ac_port] += acd.ac_incr[ac_port];
					acd.ac_offset[ac_port] &= 0xffffff;
				}
            }
            
            //diffdiff
            return ;
            //diffdiff
          case  2:  acd.ac_base[ac_port] = (acd.ac_base[ac_port] & 0xffff00) | V;          return;
          case  3:  acd.ac_base[ac_port] = (acd.ac_base[ac_port] & 0xff00ff) | (V << 8);   return;
          case  4:  acd.ac_base[ac_port] = (acd.ac_base[ac_port] & 0x00ffff) | (V << 16);  return;
          case  5:  acd.ac_offset[ac_port] = (acd.ac_offset[ac_port] & 0xff00) | V;        return;
          case  6:  acd.ac_offset[ac_port] = (acd.ac_offset[ac_port] & 0x00ff) | (V << 8);
                    if(acd.ac_control[ac_port] & (AC_ENABLE_OFFSET_BASE_6))
                       acd.ac_base[ac_port] = (acd.ac_base[ac_port] + acd.ac_offset[ac_port]) & 0xffffff;
                    return;
          case  7:  acd.ac_incr[ac_port] = (acd.ac_incr[ac_port] & 0xff00) | V;            return;
          case  8:  acd.ac_incr[ac_port] = (acd.ac_incr[ac_port] & 0x00ff) | (V << 8);     return;
          case  9:  acd.ac_control[ac_port] = V;                                          return;
          case 10:  if (acd.ac_control[ac_port] & (AC_ENABLE_OFFSET_BASE_A | AC_ENABLE_OFFSET_BASE_6) == (AC_ENABLE_OFFSET_BASE_A | AC_ENABLE_OFFSET_BASE_6))
                        acd.ac_base[ac_port] = (acd.ac_base[ac_port] + acd.ac_offset[ac_port]) & 0xffffff;
            return;
          default:
            //Log ("\nUnknown AC write %d into 0x%04X\n", V, A);
			  ;
        }
        
    }
}


//=============================================================================
//
//
//
//=============================================================================
byte ACD_read(word adr)
{
	if((adr&0x1AE0)==0x1AE0) {

        switch(adr & 0x1aef) {
		  case 0x1ae0: return (byte) (acd.ac_shift);
	 	  case 0x1ae1: return (byte) (acd.ac_shift >> 8);
		  case 0x1ae2: return (byte) (acd.ac_shift >> 16);
	      case 0x1ae3: return (byte) (acd.ac_shift >> 24);
		  case 0x1ae4: return acd.ac_shiftbits;
		  case 0x1ae5: return acd.ac_unknown4;
		  case 0x1aee: return 0x10;
		  case 0x1aef: return 0x51;
		}
		return 0xff;
	}

	{
        byte ac_port = (adr >> 4) & 3;
        byte ret = 0;

        switch (adr & 15) {
          case 0x00:
          case 0x01:
			if(acd.ac_control[ac_port] & AC_USE_OFFSET) {
				ret = acd.ac_extra_mem[(acd.ac_base[ac_port] + acd.ac_offset[ac_port]) & 0x1fffff];
			}
            else {
				ret = acd.ac_extra_mem[(acd.ac_base[ac_port]) & 0x1fffff];
			}
            
            if(acd.ac_control[ac_port] & AC_ENABLE_INC) {
                if (acd.ac_control[ac_port] & AC_INCREMENT_BASE) {
					acd.ac_base[ac_port] += acd.ac_incr[ac_port];
					acd.ac_base[ac_port] &= 0x1fffff;
				}
                else {
					acd.ac_offset[ac_port] += acd.ac_incr[ac_port];
					acd.ac_offset[ac_port] &= 0x1fffff;
				}
            }
            return ret;
          case 0x02: return (byte) (acd.ac_base[ac_port]);
          case 0x03: return (byte) (acd.ac_base[ac_port] >> 8);
          case 0x04: return (byte) (acd.ac_base[ac_port] >> 16);
          case 0x05: return (byte) (acd.ac_offset[ac_port]);
          case 0x06: return (byte) (acd.ac_offset[ac_port] >> 8);
          case 0x07: return (byte) (acd.ac_incr[ac_port]);
          case 0x08: return (byte) (acd.ac_incr[ac_port] >> 8);
          case 0x09: return acd.ac_control[ac_port];
          case 0x0a: return 0;
          default: return 0xff;
		}
    }

	return 0xff;
}



// ******************************************************************************************
// ******************************************************************************************
// ******************************************************************************************
#endif// ************************************************************************************
// ******************************************************************************************
// ******************************************************************************************
// ******************************************************************************************

//=============================================================================
//
// CD����TOC����ǂ݂܂�
//
//=============================================================================
int cd_toc_read(int nsize,char* toc_buf)
{
	char *p;
	int	n=0,f=0,c=0,s=0;

	core_memset( cd_toc, 0, sizeof(cd_toc) );
	
	p=toc_buf;

	while(nsize>0){
		if( *p>='0' && *p<='9' ){
			s*=10;
			s+=*p-'0';
			f=1;
		}else{
			if( f ){
				switch(c){
				case	0:				n = s;	c++;	break;
				case	1:	cd_toc[n].min = s;	c++;	break;
				case	2:	cd_toc[n].sec = s;	c++;	break;
				case	3:	cd_toc[n].fra = s;	c++;	break;
				case	4:	cd_toc[n].LBA = s;	c=0;	
					n++;
					if( n>=0x100 ) nsize=0;
					break;
				}
			}
			if( !core_memcmp( p, "Data",  4 ) ) cd_toc[n].type=4;
			if( !core_memcmp( p, "Audio", 5 ) ) cd_toc[n].type=0;
			f=0;
			s=0;
		}
		p++;
		nsize--;
	}

	nb_max_track=n-1;
	
	return 0;
}


//=============================================================================
//
// CD����TOC����ǂ݂܂�
//
//=============================================================================
int cd_track_search(int m,int s,int f)
{
	int i;
	for(i=0;i<0x100;i++){
		if(	cd_toc[i].min==m &&
			cd_toc[i].sec==s &&
			cd_toc[i].fra==f ) {
            return	i;
        }
    }
    return 0;
}


//=============================================================================
//
//
//
//=============================================================================
void cd_test_read(char *p,int s, int num)
{
	char i,fd,n[1024];
	char tn[3];
    char *wp;

    for(i=1;i<0x100;i++){
        if( cd_toc[i].LBA>s )break;
    }
    
    i--;
    tn[0]='0'+(i/10);
    tn[1]='0'+(i%10);
    tn[2]=0;

	core_strcpy( n, HAL_GetRomsPath() );
//    core_strcpy( n, tocName() );

    wp=core_strrchr( n, '/' );
	if(!wp) wp=core_strrchr( n, '\\' );

	wp[1]=0;
	
	core_strcat( n, tn);
    core_strcat( n, ".iso");
    //Error_mes(n);

//    core_strcpy(cd_last_read,n);

    //mh_print(400,0,n,RGB_WHITE);

    if( (fd = HAL_fd_open(n,HAL_MODE_READ))>=0 ) {
        int a;
        DWORD w;
        a = (s-cd_toc[i].LBA)*2048;
        w= HAL_fd_seek(fd,0, HAL_SEEK_END);
        
        if( a<0 || w<a+2048 ){
			a = a;
        }else{
            int rd;
            w = HAL_fd_seek( fd, a, HAL_SEEK_SET);
            rd = HAL_fd_read(fd, p, 2048*num);
        }
        HAL_fd_close(fd);
    }else{
		fd = fd;
	}
}
