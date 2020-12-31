//*****************************************************************************
//                                                                             
// Startup Library                                                             
//                                                                             
//*****************************************************************************

extern int main(int argc, char** argv);
extern int init_malloc(int ispsp);

//------------------------------------------------------------------------
// mallocライブラリはコノ定義を使ってメモリ管理する。
// メモリ管理を配列でやりたい場合はココの定義を小さくしてね
// PSPのメモリ管理を使う場合はポインタとサイズを設定してね
//------------------------------------------------------------------------
#define MALLOC_MEMSIZE()   (16*1024*1024 + 512*1024)

char malloc_mem[ MALLOC_MEMSIZE() ];
int  malloc_size = sizeof(malloc_mem);

//=========================================================
// 
// 引数をコンバートする目的で使ってる状態
// argcを0でPSPEで動作してる事にする
// 
//=========================================================
int xmain(int argc0, char* argv0)
{
	int argc = argc0;
    char * argv[2]={argv0,0},*a=argv0;

    // PSPEで動作する場合はargv0=0
    // PSPで動作する場合は ms0:/PSP/GAME/XXXX 
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


