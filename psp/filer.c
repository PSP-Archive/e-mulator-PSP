#include "syscall.h"
#include "filer.h"
#include "pg.h"

#ifndef MAXPATH
#define MAXPATH 512
#endif

#define MAXNAME 256
#define MAX_ENTRY 1024

char LastPath[MAXPATH] = {"\0"};
char FilerMsg[256];
char FileName[MAXPATH];

static struct dirent files[MAX_ENTRY];
static int nfiles;

// 拡張子管理用
const struct {
	char *szExt;
	int nExtId;
} stExtentions[] = {
    {"gb" ,EXT_GB },
    {"gbc",EXT_GBC},
    {"sgb",EXT_SGB},
    
    {"pce",EXT_PCE},
    {"toc",EXT_TOC},

    {"ws", EXT_WS },
    {"wsc",EXT_WSC},

    {"npc",EXT_NPC},
    {"ngp",EXT_NGP},

    {"nes",EXT_NES},

    {"gg" ,EXT_GG },
    {"sms",EXT_SMS},
    
    {"zip",EXT_ZIP},
    {"mp3",EXT_MP3},
    {"txt",EXT_TXT},
    {NULL, EXT_UNKNOWN}
};

int getExtId(const char *szFilePath);

////////////////////////////////////////////////////////////////////////
// クイックソート
#if 0
int cmpFile(struct dirent *a, struct dirent *b)
{
	char ca, cb;
	int i, n, ret;
	
	if(a->type==b->type){
		n=core_strlen(a->name);
		for(i=0; i<=n; i++){
			ca=a->name[i]; cb=b->name[i];
			if(ca>='a' && ca<='z') ca-=0x20;
			if(cb>='a' && cb<='z') cb-=0x20;
			
			ret = ca-cb;
			if(ret!=0) return ret;
		}
		return 0;
	}
	
	if(a->type & TYPE_DIR)	return -1;
	else					return 1;
}
#else
////////////////////////////////////////////////////////////////////////
// クイックソート
// AC add start
void SJISCopy(struct dirent *a, unsigned char *file)
{
	unsigned char ca;
	int i;
	int len=strlen(a->name);
	
	for(i=0;i<=len;i++){
		ca = a->name[i];
		if (((0x81 <= ca)&&(ca <= 0x9f))
		|| ((0xe0 <= ca)&&(ca <= 0xef))){
			file[i++] = ca;
			file[i] = a->name[i];
		}
		else{
			if(ca>='a' && ca<='z') ca-=0x20;
			file[i] = ca;
		}
	}

}
int cmpFile(struct dirent *a, struct dirent *b)
{
    unsigned char file1[0x108];
    unsigned char file2[0x108];
	unsigned char ca, cb;
	int i, n, ret;

	if(a->type==b->type){
		SJISCopy(a, file1);
		SJISCopy(b, file2);
		n=strlen((char*)file1);
		for(i=0; i<=n; i++){
			ca=file1[i]; cb=file2[i];
			ret = ca-cb;
			if(ret!=0) return ret;
		}
		return 0;
	}
	
	if(a->type & TYPE_DIR)	return -1;
	else					return 1;
}
// AC add end
#endif

////////////////////////////////////////////////////////////////////////
void sort(struct dirent *a, int left, int right) {
	struct dirent tmp, pivot;
	int i, p;
	
	if (left < right) {
		pivot = a[left];
		p = left;
		for (i=left+1; i<=right; i++) {
			if (cmpFile(&a[i],&pivot)<0){
				p=p+1;
				tmp=a[p];
				a[p]=a[i];
				a[i]=tmp;
			}
		}
		a[left] = a[p];
		a[p] = pivot;
		sort(a, left, p-1);
		sort(a, p+1, right);
	}
}




////////////////////////////////////////////////////////////////////////
void getDir(const char *path,int *fileExt) {
    int i;
	int fd, b=0;
    int ext;
//	char *p;
	
	nfiles = 0;
	
	if(core_strcmp(path,"ms0:/")){
		core_strcpy(files[nfiles].name,"..");
		files[nfiles].type = TYPE_DIR;
		nfiles++;
		b=1;
	}
	
	fd = sceIoDopen(path);
	while(nfiles<MAX_ENTRY){
		if(sceIoDread(fd, &files[nfiles])<=0) break;
		if(files[nfiles].name[0] == '.') continue;
		if(files[nfiles].type == TYPE_DIR){
			core_strcat(files[nfiles].name, "/");
			nfiles++;
			continue;
		}
        
        ext = getExtId(files[nfiles].name);

        if(fileExt==0 || fileExt[0]==EXT_ALL) {
            nfiles++;
        } else {
#if 9999////////////////////////////////////////////////////////////////
            if(fileExt[0]==EXT_TOC) {
                if(ext==EXT_TOC) {
                    nfiles++;
                }
            }
            else if( HAL_IsSupportExt(ext) ) {
                nfiles++;
            }
#else   ////////////////////////////////////////////////////////////////
            for(i=0;i<sizeof(stExtentions)/sizeof(stExtentions[0]);i++) {
                if(fileExt[i]==EXT_NULL) break;
                if(fileExt[i]==ext) {
                    nfiles++;
                    break;
                }
            }
#endif  ////////////////////////////////////////////////////////////////
        }
	}
	sceIoDclose(fd);
	if(b) sort(files+1, 0, nfiles-2);
	else  sort(files, 0, nfiles-1);
}

////////////////////////////////////////////////////////////////////////
int getFilePath(char *out,char *pDefPath,int * fileExt)
{
	word color = RGB_WHITE;
	int sel=0, rows=21, top=0, x, y, h, i, /*len,*/ bMsg=0, up=0;
	char path[MAXPATH], oldDir[MAXNAME], *p;
    int new_pad=0;

    if(core_strlen(LastPath)==0) core_strcpy(path,pDefPath);
    else                         core_strcpy(path,LastPath);
    
    if(FilerMsg[0])
		bMsg=1;
	
	getDir(path,fileExt);

    for(;;){
        print_menu_frame(path,0,0);
          
        if((new_pad = readpad_new())) {
            bMsg=0;
        }
        
		if(new_pad & CTRL_CIRCLE){
			if(files[sel].type == TYPE_DIR){
				if(!core_strcmp(files[sel].name,"..")){  up=1; }
                else{
					core_strcat(path,files[sel].name);
					getDir(path,fileExt);
					sel=0;
				}
			}else{
				core_strcpy(out, path);
				core_strcat(out, files[sel].name);
				core_strcpy(LastPath,path);
				return 1;
			}
		}
        else if(new_pad & CTRL_CROSS)   { return 0; }
        else if(new_pad & CTRL_TRIANGLE){ up=1;     }
        else if(new_pad & CTRL_UP)      { sel--;    }
        else if(new_pad & CTRL_DOWN)    { sel++;    }
        else if(new_pad & CTRL_LEFT)    { sel-=10;  }
        else if(new_pad & CTRL_RIGHT)   { sel+=10;  }
        // 左か右のトリガーボタンを押すと名前が違うファイルまで移動するっぽ
        else {
            int findp=0;
            if(new_pad & CTRL_LTRIGGER){ findp=-1; }
            if(new_pad & CTRL_RTRIGGER){ findp=+1;  }
            
            if(findp!=0) {
                int i;
                char selc = files[sel].name[0];
                for(i=sel+findp;1;i+=findp) {
                    if(i<=0) { sel=0; break; }
                    if(i>=nfiles-1) { sel=nfiles-1; break; }
                    if(files[i].name[0]!=selc) {
                        sel=i;
                        break;
                    }
                }
            }
        }

        
		if(up){
			if(core_strcmp(path,"ms0:/")){
				p=core_strrchr(path,'/');
				*p=0;
				p=core_strrchr(path,'/');
				p++;
				core_strcpy(oldDir,p);
				core_strcat(oldDir,"/");
				*p=0;
				getDir(path,fileExt);
				sel=0;
				for(i=0; i<nfiles; i++) {
					if(!core_strcmp(oldDir, files[i].name)) {
						sel=i;
						top=sel-3;
						break;
					}
				}
			}
			up=0;
		}
		
		if(top > nfiles-rows)	top=nfiles-rows;
		if(top < 0)				top=0;
		if(sel >= nfiles)		sel=nfiles-1;
		if(sel < 0)				sel=0;
		if(sel >= top+rows)		top=sel-rows+1;
		if(sel < top)			top=sel;
		
		// スクロールバー
		if(nfiles > rows){
			h = 219;
#if 1
			pgFillBox(467, h*top/nfiles + 27, 472, h*(top+rows)/nfiles + 23,RGB_YELLOW);
#else
            pgFillBox(445,25,446,245,RGB_YELLOW);
			pgFillBox(448, h*top/nfiles + 27, 460, h*(top+rows)/nfiles + 23,RGB_BLUE);
			pgFillBox(463,25,464,245,RGB_YELLOW);
#endif
		}
		
		x=28; y=32;
		for(i=0; i<rows; i++){
			if(top+i >= nfiles) break;
			if(top+i == sel) color = RGB_RED;
			else			 color = RGB_WHITE;
			mh_print(x, y, files[top+i].name, color);
			y+=10;
		}

        pgScreenFlipV();
        
        if(PSP_IsEsc()) {
            return -1;
        }
	}
}

/*
int InitFiler(char*msg,char*path)
{
    strcpy(FilerMsg,msg);
    strcpy(LastPath,path);
    memset(files,0,sizeof(files));
    return 1;
}
*/
