#include "fileio.h"
#include "debug.h"
#include "hal.h"
#include "cstring.h"

#if 0 //-----------------------------------------------------------------------
//HANDLE NES_fopen(const char *pPath, int mode)
//{
//    int hFile = -1;
//    
//    DEBUG(pPath);
//    switch (mode) {
//      case FILE_MODE_READ:
//        DEBUG("NES_fopen(READ)");
//        hFile = HAL_fd_open((char*)pPath,HAL_MODE_READ);
//        break;
//      case FILE_MODE_WRITE:
//        DEBUG("NES_fopen(WRITE)");
//        hFile = HAL_fd_open((char*)pPath,HAL_MODE_WRITE);
//        break;
//      case FILE_MODE_APPEND:
//        DEBUG("NES_fopen(APPEND)");
//        hFile = HAL_fd_open((char*)pPath,HAL_MODE_APPEND);
//        break;
//    }
//    
//    if (hFile < 0) {
//        DEBUG("NES_fopen failed");
//        DEBUG(pPath);
//        return hFile;
//    }
//	DEBUG("NES_fopen ");
//	DEBUG(pPath);
//	return (HANDLE)hFile;
//}
//
//int NES_fclose(HANDLE fh)
//{
//    DEBUG("NES_fclose hFile");
//    HAL_fd_close((int)fh);
//    return TRUE;
//}
//
//int NES_fgetc(HANDLE fh)
//{
//	unsigned char ch = 0;
//	DEBUG(" NES_fgetc");
//    
//    if(HAL_fd_read((int)fh,&ch,1)) {
//        return ch;
//	} else {
//        return -1;
//    }
//}
//
//int NES_fputc(int chVal, HANDLE fh)
//{
//	char ch = chVal;
//	DEBUG(" NES_fputc");
//    return HAL_fd_write((int)fh, &ch, 1);
//}
//
//size_t NES_fread(void *buf, size_t size, size_t count, HANDLE fh)
//{
//	DEBUG(" NES_fread");
//	return HAL_fd_read(fh, buf, size*count);
//}
//
//size_t NES_fwrite(void *buf, size_t size, size_t count, HANDLE fh)
//{
//	DEBUGVALUE(" NES_fwrite hFile", fh);
//	return HAL_fd_write(fh, buf, size*count);
//}
//
//void NES_DeleteFile(const char *pFile)
//{
//    HAL_fd_delete((char*)pFile);
//}
//
//int NES_fseek(HANDLE fh, long offset, int origin)
//{
//    DEBUG("NES_fseek");
//
//    switch (origin) {
//      case FILE_SEEK_CUR:
//        return HAL_fd_seek(fh,offset, HAL_SEEK_CUR);
//        
//      case FILE_SEEK_END:
//        return HAL_fd_seek(fh, offset, HAL_SEEK_END);
//        
//      case FILE_SEEK_SET:
//        return HAL_fd_seek(fh, offset, HAL_SEEK_SET);
//    }
//    return -1;
//}
#endif//-----------------------------------------------------------------------

