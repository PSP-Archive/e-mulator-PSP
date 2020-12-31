#pragma comment(lib, "SDL.lib")
#pragma comment(lib, "SDLmain.lib")

#include "stdafx.h"

#include "SDL.h"
#include "SDL_thread.h"
#include "SDL_audio.h"
#include "sceWrapper.h"
#include "hal.h"
#include "lib\comctl.h"
#include "cstring.h"

int g_EmuFlag=0;

void sdl_graph_init(void);
int  sdl_thread_close(void);
void sdl_graph_close(void);
extern int CartLoad(char *name,void** pRomAddr,int* pRomSize,int* pState);
extern void CartFree(void* pRom);

char *gui_getSavePath(HWND hWnd,int bSave);


char* tocName(void)
{
	return "D:\\pspe\\ms0\\roms\\ys\\ys.toc";
}


int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow )
{
 	// TODO: この位置にコードを記述してください。
	int loop=1;
	CORE_HANDLER *pCoreHdl=0;
	char cpath[4096];

	sdl_graph_init();
	HAL_Sound_Init();

	getcwd(cpath,sizeof(cpath));
	core_strcat(cpath,"\\exe");

	HAL_SetWorkPath(cpath);

	{
		int nSize;// = 32*1024*1024;
		void* pRom;// = malloc(nSize);
		char *name=0;
		int type,rsize;

		GBC_Setup();
		NGP_Setup();
		SWAN_Setup();
		PCE_Setup();
		NES_Setup();
		SMS_Setup();
		LNX_Setup();

CHANGE_ROM:
		name = gui_getRomPath(0);

		if(name) {
			type = CartLoad(name,&pRom,&nSize,&rsize);
			if( (pCoreHdl=GetCoreHandlerFromType(type)) ){
				HAL_SetRomsPath(name);

				if( pCoreHdl->pINIT(nSize,pRom) && 
					pCoreHdl->pRESET() ){
					while(1) {
						if( pCoreHdl->pLOOP() ) {
							break;
						}
						
						if(g_EmuFlag) {
							char* sf=0;
							int fd;

							if(g_EmuFlag&(1<<0)) {
								sf = gui_getSavePath(0,0); 
								if(sf && (fd = HAL_fd_open(sf,HAL_MODE_READ))){
									pCoreHdl->pLOAD(fd);
									HAL_fd_close(fd);
								}
							} 
							else if(g_EmuFlag&(1<<1)) {
								sf = gui_getSavePath(0,1); 
								if(sf && (fd = HAL_fd_open(sf,HAL_MODE_WRITE))){
									pCoreHdl->pSAVE(fd);
									HAL_fd_close(fd);
								}
							} 
							else if(g_EmuFlag & (1<<31)) {
								break;
							}
							g_EmuFlag=0;
						}
					}
				}

				pCoreHdl->pEXIT();

				CartFree(pRom);
				pRom=0;

				if(g_EmuFlag & (1<<31)) {
					g_EmuFlag=0;
					goto CHANGE_ROM;
				}
			}
		}
	}

	sdl_thread_close();

	//sdl_audio_close();
	sdl_graph_close();


	return 0;
}



void swan_port_debug(int after,byte port,byte value)
{
	if(after) {
		switch(port) {
		case 0x04:
			port = port;
			break;
		}
	}
}

