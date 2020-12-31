#define ZIP_SUPPORT

#include "main.h"

#include <stdio.h>
#include <string.h>
#include "deflateInterface.h"
#include "zlibInterface.h"
#include "filer.h"
#include "hal.h"
#include "cstring.h"

#define STATE_MAX    10

#ifdef ZIP_SUPPORT /* ZIP SUPPORT  */

#include "zlibInterface.h"

typedef struct {
    byte* pRomAddr;
    int   nRomSize;
    int   nRomExt;
    char  pRomName[512];
} ZIPROM_INFO;

// 宣言
int funcUnzipCallback(int nCallbackId,
                      unsigned long ulExtractSize,
                      unsigned long ulCurrentPosition,
                      const void *pData,
                      unsigned long ulDataSize,
                      unsigned long ulUserData)
{
    const char *pszFileName;
    const unsigned char *pbData;
    ZIPROM_INFO* pZipInfo = (ZIPROM_INFO*)ulUserData;

    switch(nCallbackId) {
      case UZCB_FIND_FILE: {
          int extid;
          pszFileName = (const char *)pData;
          extid = getExtId(pszFileName);

          if( HAL_IsSupportExt(extid) ) {

              pZipInfo->pRomAddr = (byte*)HAL_mem_malloc(ulExtractSize);

              if(pZipInfo->pRomAddr) {
                  pZipInfo->nRomSize = ulExtractSize;
                  pZipInfo->nRomExt  = extid;
                  core_strcpy(pZipInfo->pRomName,pszFileName);
                  return UZCBR_OK;
              }

              return UZCBR_PASS;
          }
      }
        return UZCBR_PASS;
        break;

      case UZCB_EXTRACT_PROGRESS:
        if( pZipInfo->pRomAddr ) {
            pbData = (const unsigned char *)pData;
            core_memcpy(&pZipInfo->pRomAddr[ulCurrentPosition],pbData,ulDataSize);
            return UZCBR_OK;
        }
        break;

      default: // unknown...
        /*
         現状のバージョンではここには絶対にこない(と思う)が、FAILSAFEのために何か
         デバッグコードをいれておくと良いかも…
         */
        break;
    }
    return UZCBR_CANCEL;
}
#endif /* ZIP_SUPPORT */


//--------------------------------------------------------------------------
// 
//--------------------------------------------------------------------------
int CartLoad(char *name,void** pRomAddr,int* pRomSize,int* pState)
{
    int fd;
    int ret_ext = EXT_NULL;
    int file_ext;
    int   nSize=0;
    BYTE* pAddr=0;

    file_ext = getExtId(name);

    if(HAL_IsSupportExt(file_ext)) {
        if( file_ext == EXT_ZIP ) {
            int extract;
            ZIPROM_INFO info;
            
            core_memset(&info,0,sizeof(info));
            
            Unzip_setCallback(funcUnzipCallback);
            extract = Unzip_execExtract(name,&info);
            
            if(extract==UZEXR_OK) {
                nSize = info.nRomSize;
                pAddr = info.pRomAddr;
                ret_ext=info.nRomExt;
                HAL_SetRomsPath(name); // N:/XXXX/YYY/ZZ.zip
            } else {
                nSize = 0;
                pAddr = 0;
                ret_ext = EXT_NULL;
            }
        }
        else {
            if((fd=HAL_fd_open(name,HAL_MODE_READ))) {
                nSize = HAL_fd_size(fd);
                
                if((pAddr = (byte*)HAL_mem_malloc(nSize))) {
                    nSize = HAL_fd_read(fd,pAddr,nSize);
                    HAL_SetRomsPath(name);
                    ret_ext = file_ext;
                } else {
                    nSize = 0;
                    pAddr = 0;
                    HAL_SetRomsPath("noname");
                    ret_ext=EXT_NULL;
                }
                HAL_fd_close(fd);
            }
        }

        if( pRomAddr ) {
            *pRomAddr = pAddr;
            *pRomSize = nSize;
            *pState = CheckState();
            return ret_ext;
        }
        
        *pState=0;
    }


    return EXT_NULL;
}

//--------------------------------------------------------------------------
// 
//--------------------------------------------------------------------------
void CartFree(void* pRom)
{
    if(pRom) {
        HAL_mem_free(pRom);
    }
}

//--------------------------------------------------------------------------
// 
//--------------------------------------------------------------------------
int CheckState(void)
{
    int i,ret=0,fd;
    char* pName=0;
    
    for(i=0;i<STATE_MAX;i++) {
        if((pName = HAL_GetSavePath(i))) {
            if((fd=HAL_fd_open(pName,HAL_MODE_READ))>=0) {
                HAL_fd_close(fd);
                ret |= 1<<i;
            }
        }
    }
    
    return ret;
}

