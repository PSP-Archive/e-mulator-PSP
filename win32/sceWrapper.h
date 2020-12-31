//--------------------------------------------------------
// PSP�A�v����Windows�ō�邽�߂̃��b�p�[���C�u����
// ........................................�̂ւ����ӂ�����
//--------------------------------------------------------
#define SAMPLING_MIN  512   // audio minimum sampling size

// ���[�U�[�A�v���Ŏw�肵��ms0:��u�������镶��
//#define MS_BASE "D:\\xpcewin\\pspe\\ms0"
#define MS_BASE "D:\\xpcewin\\pspe\\ms0"

typedef struct //_ctrl_data 
{ 
   u32 frame; 
   u32 buttons; 
   u8  analog[4]; 
   u32 unused; 
} ctrl_data_t; 


struct dirent { 
    u32 unk0; 
    u32 type; 
    u32 size; 
    u32 unk[19]; 
    char name[0x108]; 
}; 

enum { 
    TYPE_DIR=0x10, 
    TYPE_FILE=0x20 
}; 
