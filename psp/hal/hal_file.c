//=============================================================================
// FILE I/O HAL
//=============================================================================
#include "pg.h"
#include "hal.h"

void HAL_fd_mkdir(char* name)
{
    sceIoMkdir((const char*)name,0777);
}

void HAL_fd_delete(char* name)
{
    sceIoRemove(name);
}

int HAL_fd_open(char* name,int mode)
{
    int flag = 1;
    int fp;

    switch(mode) {
      case HAL_MODE_READ:
        flag = 1;
        fp = sceIoOpen(name,flag,0777);
        break;

      case HAL_MODE_WRITE:
        flag = (PSP_Is()?0x0602:0x0603);
        fp = sceIoOpen(name,flag,0777);
        break;

      case HAL_MODE_APPEND:
        flag = 0x0100 | 0x0003;
        fp = sceIoOpen(name,flag,0777);
        break;

      default:
        while(1);
        break;
    }
    return fp;
}

void HAL_fd_close(int fp)
{
    sceIoClose(fp);
}

int HAL_fd_seek(int fp,int pos,int whence)
{
    switch(whence) {
      case HAL_SEEK_SET:
        return sceIoLseek(fp,(long long)pos,0); 
        break;
        
      case HAL_SEEK_CUR:
        return sceIoLseek(fp,(long long)pos,1); 
        break;
        
      case HAL_SEEK_END:
        return sceIoLseek(fp,(long long)pos,2); 
        break;

      default:
        while(1);
        break;
    }
}

int HAL_fd_size(int fp)
{
    int pos = HAL_fd_seek(fp,0,HAL_SEEK_CUR);
    int siz = HAL_fd_seek(fp,0,HAL_SEEK_END);
    HAL_fd_seek(fp,pos,HAL_SEEK_SET);
    return siz;
}

int HAL_fd_read(int fp,void* ptr,int length)
{
    int len = sceIoRead(fp,ptr,length);
    return len;
}

int HAL_fd_write(int fp,void* ptr,int length)
{
    int len = sceIoWrite(fp,ptr,length);
    return len;
}
