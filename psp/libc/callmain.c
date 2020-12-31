//*****************************************************************************
//                                                                             
// Startup Library                                                             
//                                                                             
//*****************************************************************************

extern int main(int argc, char** argv);
extern int init_malloc(int ispsp);

//------------------------------------------------------------------------
// malloc���C�u�����̓R�m��`���g���ă������Ǘ�����B
// �������Ǘ���z��ł�肽���ꍇ�̓R�R�̒�`�����������Ă�
// PSP�̃������Ǘ����g���ꍇ�̓|�C���^�ƃT�C�Y��ݒ肵�Ă�
//------------------------------------------------------------------------
#define MALLOC_MEMSIZE()   (16*1024*1024 + 512*1024)

char malloc_mem[ MALLOC_MEMSIZE() ];
int  malloc_size = sizeof(malloc_mem);

//=========================================================
// 
// �������R���o�[�g����ړI�Ŏg���Ă���
// argc��0��PSPE�œ��삵�Ă鎖�ɂ���
// 
//=========================================================
int xmain(int argc0, char* argv0)
{
	int argc = argc0;
    char * argv[2]={argv0,0},*a=argv0;

    // PSPE�œ��삷��ꍇ��argv0=0
    // PSP�œ��삷��ꍇ�� ms0:/PSP/GAME/XXXX 
    if(*a++=='m' && *a++=='s' && *a++=='0' && *a++==':' && *a++=='/' ) {
        argc = 1;
        argv[0] = argv0;
    } else {
        argc = 0;
        argv[0] = "ms0:/PSP/GAME/hue/EBOOT.pbp";
    }
    
    // memory initialize
    if(!init_malloc(argc0)) {
        return 0;
    }

    // call main function
    main(argc,argv);

    return 0;
}


