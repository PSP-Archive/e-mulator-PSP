#include "main.h"

extern int MP3_PlayTrack(int track,int bLoop);
extern int MP3_PlayStop(void);

static char filename[512];

//=============================================================================
// HAL for CDDA
//=============================================================================
void HAL_PCE_CD_Play(int track,int option)
{
    MP3_PlayTrack(track,option);
}

void HAL_PCE_CD_Stop(void)
{
    MP3_PlayStop();
}


//
char* cdName(int track)
{
    char *p;
    if(track<0) track=0;
    
    core_strcpy(filename,eConf.pce_toc);
    p=core_strrchr(filename,'/');
    if(p) {
        p[1] = 0x30 + track/10;
        p[2] = 0x30 + track%10;
        p[3] = '.';
        p[4] = 0;
        return filename;
    }
    return 0;
}

char* tocName(void)
{
    return eConf.pce_toc;
}

char *Mp3Name(int track)
{
    char*p = cdName(track);
    if(p) {
        core_strcat(p,"mp3");
        return p;
    }
    return 0;
}

char *IsoName(int track)
{
    char*p = cdName(track);
    if(p) {
        core_strcat(p,"iso");
        return p;
    }
    return 0;
}


