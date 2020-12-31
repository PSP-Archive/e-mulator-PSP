//=============================================================================
// FILE I/O HAL
//=============================================================================
#include <windows.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <io.h>
#include <stdio.h>
#include "hal.h"

void HAL_fd_delete(char* name)
{
	//rm(name);
}

int HAL_fd_open(char* name,int mode)
{
    int flag = 1;
    int fp;

    switch(mode) {
      case HAL_MODE_READ:
        flag = 1;
		fp = _open(name,O_RDONLY|O_BINARY);
        break;

      case HAL_MODE_WRITE:
        fp = _open(name,O_WRONLY|O_BINARY|O_CREAT|O_TRUNC,0666);
        break;

      case HAL_MODE_APPEND:
        fp = _open(name,O_APPEND|O_BINARY);
        break;

      default:
        while(1);
        break;
    }
    return fp;
}

void HAL_fd_close(int fp)
{
    _close(fp);
}

int HAL_fd_seek(int fp,int pos,int whence)
{
    switch(whence) {
      case HAL_SEEK_SET:
        return _lseek(fp,(long)pos,0); 
        break;
        
      case HAL_SEEK_CUR:
        return _lseek(fp,(long)pos,1); 
        break;
        
      case HAL_SEEK_END:
        return _lseek(fp,(long)pos,2); 
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
    int len = _read(fp,ptr,length);
    return len;
}

int HAL_fd_write(int fp,void* ptr,int length)
{
    int len = _write(fp,ptr,length);
    return len;
}
