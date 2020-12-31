//=============================================================================
// 
// 機種依存部を含まない共通処理部
// 
//=============================================================================
#define COM_DEFINE

#ifdef WIN32
#define PATH_SEPARATOR '\\'
#else
#define PATH_SEPARATOR '/'
#endif

#include "hal.h"
#include "common.h"
#include "cstring.h"

#ifdef WIN32
#include <windows.h>
#endif

static int HAL_Com_Load(char* name,byte* adr,int size);
static int HAL_Com_Save(char* name,byte* adr,int size);


//============================================================================
// 実行時にこの構造体を構築する感じにしたい
//============================================================================
int emu_handler_count=0;
CORE_HANDLER emu_handler[128];

int ext_list_count=1;
int ext_list[130]={EXT_ZIP,EXT_NULL};

//============================================================================
//== MENU CONTROL
//============================================================================
//STATE_HDLR pMenu_Load=0,pMenu_Save=0;
//
//int HAL_SetCb_State(STATE_HDLR pLoadFunc,STATE_HDLR pSaveFunc)
//{
//    pMenu_Load = pLoadFunc;
//    pMenu_Save = pSaveFunc;
//    return 1;
//}

int HAL_Cleanup(void)
{
//    pMenu_Load = 0;
//    pMenu_Save = 0;
    
    return 1;
}




//============================================================================
//== Memory Management
//============================================================================
void* HAL_mem_malloc(u32 sz)
{
	void* ptr = (void*)malloc(sz);
	if(ptr){
		core_memset(ptr,0,sz);
	}
	return ptr;
}

void HAL_mem_free(void* ptr)
{
    free(ptr);
}



//============================================================================
//== DIRECTORY SECTION
//============================================================================
static char boot_path[1024];             // 実行ファイルの存在する場所
static char save_dir[1024];              // SAVEデータを設置するディレクトリ名称

static char roms_path[1024];             // ROMのフルパス
static char roms_name[1024];             // ROMの名前のみ(拡張子なし)

static char save_path[1024];             // STATE SAVE PATH
static char sram_path[1024];             // BACKUP RAM PATH

char* HAL_GetSaveDir(void)
{
    return save_dir;
}

void HAL_SetRomsPath(char* path)
{
    char *pname,*pdot;
    char work[1024];

    core_strcpy(roms_path,path);
    core_strcpy(work,path);

    if( (pdot = core_strrchr(work,'.')) ) {
        *pdot = 0;
    }

    if((pname=core_strrchr(work,PATH_SEPARATOR)) ||
       (pname=core_strrchr(work,PATH_SEPARATOR)) ) {
        pname++;
    }

    /* ROM NAME ONLY */
    core_strcpy(roms_name,pname);
}


void HAL_SetWorkPath(const char* path)
{
    char* p;
	int len;

    core_strcpy(boot_path,path);
    
    if((p=core_strrchr(boot_path,PATH_SEPARATOR))) {
        *(p+1)=0;
    }

    core_strcpy(save_dir,boot_path);
    core_strcat(save_dir,"SAVE" );

	len = core_strlen(save_dir);

	save_dir[len] = PATH_SEPARATOR;
	save_dir[len+1]=0;
    
}

char* HAL_GetWorkPath(void) {    return boot_path;    }
char* HAL_GetRomsPath(void) {    return roms_path;    }

char* HAL_GetSramPath(void)
{
    core_strcpy(sram_path,save_dir);
    core_strcat(sram_path,roms_name);
    core_strcat(sram_path,".srm");
    return sram_path;

}

static char* HAL_GetXxxPath(char* str,char c,int n)
{
    char ext[5];
    
    ext[0]='.';
    ext[1]=c;
    ext[2]='0'+((n/10)%10);
    ext[3]='0'+(n%10);
    ext[4]=0;

    core_strcpy(str,save_dir);
    core_strcat(str,roms_name);
    core_strcat(str,ext);

    return str;
}

char* HAL_GetSavePath(int num)
{
    return HAL_GetXxxPath(save_path,'s',num);
}


//------------------------------------------------------------------------------
// 
//------------------------------------------------------------------------------
int HAL_SetupExt(int   ext,   char* szExt,
                 void* pInit, void* pLoop, void* pExit, void* pReset,
                 void* pLoad, void* pSave)
{
    int n=emu_handler_count;
    
    emu_handler[n].ext = ext;
    emu_handler[n].pINIT = pInit;
    emu_handler[n].pLOOP = pLoop;
    emu_handler[n].pEXIT = pExit;
    emu_handler[n].pLOAD = pLoad;
    emu_handler[n].pSAVE = pSave;
    emu_handler[n].pRESET= pReset;
    
    core_strcpy(emu_handler[n].szExt,szExt);
    
    emu_handler_count++;

    n = ext_list_count;
    ext_list[n]  = ext;
    ext_list[n+1]= EXT_NULL;
    ext_list_count++;

    return 1;
}

//------------------------------------------------------------------------------
// 
//------------------------------------------------------------------------------
int HAL_GetSupportExt(int num)
{
    if(num>=ext_list_count) {
        return -1;
    }

    return ext_list[num];
}

//------------------------------------------------------------------------------
// 
//------------------------------------------------------------------------------
int HAL_IsSupportExt(int ext)
{
    int i;

    for(i=0;i<ext_list_count;i++) {
        if(ext_list[i]==ext) {
            return 1;
        }
    }

    return 0;
}


//int do_compress_m2m(char *pIn,int nInSize,char *pOut,int nOutSize,int level);
//int do_decompress_m2m(char *pIn,int nInSize,char *pOut,int nOutSize);
//
////
//// (0) 圧縮前のサイズ(4byte)
//// (1) ディスク上のサイズ(4byte)
//// (2) データ
////
//int HAL_cmp_write(int fp,void* ptr,int length)
//{
//    int len,szc;
//    char* mem = HAL_mem_malloc(length);
//
//    if(mem==0) { return 0; }
//
//    len = HAL_fd_write(fp,&length,sizeof(length));
//    szc = do_compress_m2m(ptr,length,mem,length,1);
//    len+= HAL_fd_write(fp,&szc,sizeof(szc));
//    len+= HAL_fd_write(fp,mem,szc);
//    HAL_mem_free(mem);
//    return length;
//}
//
//int HAL_cmp_read(int fp,void* ptr,int length)
//{
//    int len,szc,rl;
//    char* mem;
//
//    rl = HAL_fd_read(fp,&len,sizeof(len));
//
//    if(len==length) {
//        rl = HAL_fd_read(fp,&szc,sizeof(szc));
//        
//        mem = HAL_mem_malloc(szc);
//        rl = HAL_fd_read(fp,mem,szc);
//        
//        rl = do_decompress_m2m(mem,szc,ptr,length);
//        HAL_mem_free(mem);
//    }
//    return length;
//}

//
int HAL_Cfg_Load(char* name,byte* adr,int size)
{
    char work[512];
    core_strcpy(work,HAL_GetWorkPath());
    core_strcat(work,name);
    return HAL_Com_Load(work,adr,size);
}

//
int HAL_Cfg_Save(char* name,byte* adr,int size)
{
    char work[512];
    core_strcpy(work,HAL_GetWorkPath());
    core_strcat(work,name);
    return HAL_Com_Save(work,adr,size);
}


//
int HAL_Mem_Load(byte* adr, int size)
{
    char* name = HAL_GetSramPath();
    return HAL_Com_Load(name,adr,size);
}

//
int HAL_Mem_Save(byte* adr, int size)
{
    char* name = HAL_GetSramPath();
    return HAL_Com_Save(name,adr,size);
}



//
static int HAL_Com_Load(char* name,byte* adr,int size)
{
    int fd,len=0;

    if(name && adr && size) {
        if((fd = HAL_fd_open(name,HAL_MODE_READ))>=0) {
            len = HAL_fd_read(fd,adr,size);
            HAL_fd_close(fd);
        }
    }
    return len;
}


//
static int HAL_Com_Save(char* name,byte* adr,int size)
{
    int fd,len=size+1;
    
    if(name) {
        if((fd=HAL_fd_open(name,HAL_MODE_WRITE))) {
//          len=HAL_cmp_write(fd,adr,size);
            len=HAL_fd_write(fd,adr,size);
            HAL_fd_close(fd);
        }
    }
    return (len==size);
}



////////////////////////////////////////////////////////////////////////
int getExtId(const char *szFilePath) {
	char *pszExt;
	int i;
	if((pszExt = core_strrchr(szFilePath, '.'))) {
		pszExt++;

        if( !core_stricmp("zip",pszExt) ) {
            return EXT_ZIP;
        }

        if( !core_stricmp("toc",pszExt) ) {
            return EXT_TOC;
        }
        
		for (i=0; i<emu_handler_count; i++) {
			if (!core_stricmp(emu_handler[i].szExt,pszExt)) {
				return emu_handler[i].ext;
			}
		}
	}
	return EXT_UNKNOWN;
}




CORE_HANDLER* GetCoreHandler(char *szFilePath)
{
	char *pszExt;
	int i;

	if((pszExt = core_strrchr(szFilePath, '.'))) {
		pszExt++;

		if( !core_stricmp("zip",pszExt) ) { return 0; }
	    if( !core_stricmp("toc",pszExt) ) { return 0; }

		for (i=0; i<emu_handler_count; i++) {
			if (!core_stricmp(emu_handler[i].szExt,pszExt)) {
				return &emu_handler[i];
			}
		}
	}

	return 0;
}

CORE_HANDLER* GetCoreHandlerFromType(int type)
{
	char *pszExt;
	int i;

	for (i=0; i<emu_handler_count; i++) {
		if(emu_handler[i].ext==type) {
			return &emu_handler[i];
		}
	}

	return 0;
}



/* HOOK用メモリ */
int memsts_bufsiz=0;

#ifdef WIN32
u8* memsts_buffer[1024*1024];
#else
u8* memsts_buffer=0;
#endif

int memsts_seek  =0;
int memsts_length=0;


int HAL_sts_open(char* name,int mode,u8 *pBuffer,u32 nSize)
{
	memsts_bufsiz = 1024*1024;
	memsts_seek = 0;
	memsts_length = 0;
    return 0;
}

int HAL_sts_close(int fd)
{
    return 0;
}


int HAL_sts_write(int fd,void* ptr,int length)
{
    int len=0;
    
    if( fd==HAL_FP_MEM ) {
        if(memsts_buffer){
            if(memsts_bufsiz>=(length+memsts_seek)) {
                core_memcpy(&memsts_buffer[memsts_seek],ptr,length);
                memsts_seek += length;

                // 少し問題があるかもしれない。
                if(memsts_seek>memsts_length) {
                    memsts_length = memsts_seek;
                }
                
                len=length;
            } else {
                //*(int*)0x99 = 1;
            }
        } else {
            //*(int*)0x99 = 2;
        }
    } else {
        len = HAL_fd_write(fd,ptr,length);
//        if( fd & HAL_STS_COMPRESS ) {
//            fd &= ~HAL_STS_COMPRESS;
//            len = HAL_cmp_write(fd,ptr,length);
//        } else {
//            len = HAL_fd_write(fd,ptr,length);
//        }
    }
    
    return len;
}

int HAL_sts_read(int fd,void* ptr,int length)
{
    int len=0;

    if( fd==HAL_FP_MEM ) {
        if(memsts_buffer && memsts_bufsiz>=(length+memsts_seek)) {
            core_memcpy(ptr,&memsts_buffer[memsts_seek],length);
            memsts_seek += length;
            len=length;
        }
    } else {
        len = HAL_fd_read(fd,ptr,length);
//        if( fd & HAL_STS_COMPRESS ) {
//            fd &= ~HAL_STS_COMPRESS;
//            len = HAL_cmp_read(fd,ptr,length);
//        } else {
//            len = HAL_fd_read(fd,ptr,length);
//        }
    }

    return len;
}

int HAL_sts_seek(int fd,int offset,int origin)
{
    if( fd==HAL_FP_MEM ) {
	    switch(origin) {
		  case HAL_SEEK_CUR:
			memsts_seek += offset;
			break;

		  case HAL_SEEK_END:
			memsts_seek = memsts_length - offset;
			break;

		  case HAL_SEEK_SET:
			memsts_seek = offset;
			break;
		}

		if(memsts_seek<0) {
			memsts_seek=0;
		}
		else if(memsts_seek>memsts_length) {
			memsts_seek=memsts_length-1;
		}

		return memsts_seek;
	}

	return HAL_fd_seek(fd,offset,origin);
}




