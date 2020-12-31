//#define DEBUG_PADDING

/*
** nester - NES emulator
** Copyright (C) 2000  Darren Ranalli
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
** Library General Public License for more details.  To obtain a
** copy of the GNU Library General Public License, write to the Free
** Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
**
** Any permitted reproduction of these routines, in whole or in part,
** must bear this legend.
**
** 圧縮ファイル対応のため、みかみかな によって、Win32 APIの File IO に
** 書きかえられています。
**
** 武田によってFAM,g_ROM.fds形式の読み込みに関するコードが追加されています。
**
** PSP移植のためCPPからCに変更しています。
**
** 他エミュレータとの共通化のためFILE IO関係の処理は削除しまくり。 by e
** 標準的な関数を共通化関数に改名してます。
**
*/

#include "nes.h"
#include "nes_rom.h"
#include "debug.h"
#include "nes_crc32.h"
#include "nes_config.h"
#include "cstring.h"
#include "hal.h"

//
// e[mulator]仕様ではROMはメモリに存在する状態で渡される
//
//u8* pNesRom=0;
//u32 nNesRom=0;

#if 1
NES_ROM* pg_ROM = 0;
#else
//NES_ROM g_ROM;	// ROM
#endif


#define CopyMemIncPtr(o,i,s) \
{\
    core_memcpy(o,i,s);\
	i+=s;\
}

typedef struct nesdb{
    u8  title[64];   /* 64: Game TITLE */
    u32 crc_all;     /*  4: */
    u32 crc_prom;    /*  4: */
    u8  header[2];   /*  2: */
    u8  size_prg;    /*  1: */
    u8  size_chr;    /*  1: */
    u8  country;     /*  1: */
    u8  screen_mode; /*  1: */
#ifdef DEBUG_PADDING // padding for debug
    u8  rsv[50];     /* 50: */
#endif
} NESDB;

#define NESDB_MAX  1200

u32   g_nes_dbNum=0;
NESDB g_nes_db[NESDB_MAX];

NESDB* NES_findDB(NES_ROM* pRom)
{
    int i;

    if(pRom && pRom->crc) {
        for(i=0;i<g_nes_dbNum;i++) {
            if(pRom->crc==g_nes_db[i].crc_prom) {
                return &g_nes_db[i];
            }
        }
    }

    return 0;
}

//NES_openDB("famicom.dat");
//NES_openDB("nesdbase.dat");

int NES_openDB(char* dbname)
{
    int z,fd,i;
    char fn2[512],buf[512],buf2[16];
    char *pReadPtr;
    u32   nReadSiz,rdp,nB;
    u8    rd;
    NESDB* pNesDB = g_nes_db;
    u32  pt;

    g_nes_dbNum = 0;
    core_memset(g_nes_db,0,sizeof(g_nes_db));

    core_strcpy(fn2,HAL_GetWorkPath());
    core_strcat(fn2,dbname);

    if((fd = HAL_fd_open(fn2,HAL_MODE_READ))>=0){
        nReadSiz = HAL_fd_size(fd);
        pReadPtr = HAL_mem_malloc(nReadSiz);
        HAL_fd_read(fd,pReadPtr,nReadSiz);
        HAL_fd_close(fd);

        nB=0;
        
        for(rdp=0;rdp<nReadSiz;rdp++) {
            
            buf[nB++] = rd = pReadPtr[rdp];
            
            /* Search Line termination */
            if(rd=='\r' || rd=='\n' || rd==0) {
                buf[nB]=0;
                nB=0;

                pNesDB = &g_nes_db[g_nes_dbNum];

                //-------------------------------------------------
                // ALL-CRCに適合しない文字ならコメントと判断
                //-------------------------------------------------
                if(!(('0'<=buf[0] && buf[0]<='9') ||
                     ('a'<=buf[0] && buf[0]<='f') ||
                     ('A'<=buf[0] && buf[0]<='F')) ) {
                    continue;
                }

                //-------------------------------------------------
                // CRC ALL+PROM
                //-------------------------------------------------
                for(pt=z=0;z<2;z++) {
                    for(i=0;i<8 && buf[pt]!=';' && buf[pt]; i++, pt++){
                        buf2[i] = buf[pt];
                    }
                    if(buf[pt]=='\0')  break;
                    ++pt;
                    buf2[i] = 0;
                    
                    if(z==0) pNesDB->crc_all = core_atoh(buf2);
                    else     pNesDB->crc_prom= core_atoh(buf2);
                }
                
                // error check
                if(buf[pt]==0) continue;
                
                //-------------------------------------------------
                // Title
                //-------------------------------------------------
                for(i=0; buf[pt] != ';' && buf[pt]!='\0'; ++i, ++pt){
                    if(pt>62) { pNesDB->title[63]=0;       }
                    else      { pNesDB->title[i] =buf[pt]; }
                }
                pt++;
                
                //-------------------------------------------------
                // HEADER : 0 & 1
                //-------------------------------------------------
                for(z=0;z<2;z++) {
                    i=0;
                    while(buf[pt] != ';'){ buf2[i++] = buf[pt++]; }  pt++;
                    buf2[i] = '\0';
                    pNesDB->header[z] = core_atoi(buf2);
                }
                
                //-------------------------------------------------
                // PROM Size
                //-------------------------------------------------
                while(buf[pt]!=';'){ pt++; } pt++;
                //-------------------------------------------------
                // CROM Size
                //-------------------------------------------------
                while(buf[pt]!=';'){ pt++; } pt++;
                
                //-------------------------------------------------
                // Country
                //-------------------------------------------------
                if(/*buf[pt] == 'A' ||*/
                   buf[pt] == 'E' ||
                   (buf[pt] == 'P' && buf[pt+1] == 'D') ||
                   buf[pt] == 'S') {
                    pNesDB->screen_mode = 2;// Asia, Europe, PD, Swedish
                }
                else if(buf[pt] == 'J' || buf[pt] == 'U' || buf[pt] == 'V'){
                    pNesDB->screen_mode = 1;
                }
                
                g_nes_dbNum++;
                
                if(g_nes_dbNum==NESDB_MAX) {
                    break;
                }
            }
        }

        HAL_mem_free(pReadPtr);
        fd = 1;
    } else {
        fd = 0;
    }

#ifdef DEBUG_PADDING /* DEBUG */
    {
        int fd = HAL_fd_open("ms0:/db.dat",HAL_MODE_WRITE);

        if(fd>=0) {
            HAL_fd_write(fd,g_nes_db,sizeof(g_nes_db));
            HAL_fd_close(fd);
        }
    }
#endif/*DEBUG*/

    return fd;
}

int NES_closeDB(void)
{
    return 1;
}

NES_ROM *NES_ROM_LoadRom(int nSize,u8* pRomAddr)
{
	u8 *p = NULL;
	u8 *buf = 0; // g_ReadRomImage;
	u32 filesize;
	u8 image_type;

    core_memset(g_ROM.trainer, 0x00, sizeof(g_ROM.trainer));

    g_ROM.crc = 0;
    g_ROM.crc_all = 0;
    g_ROM.fds = 0;
    g_ROM.unif_mapper=0;
    g_ROM.GameTitle[0] =0;
    g_ROM.dbcorrect[0]=g_ROM.dbcorrect[1]=g_ROM.dbcorrect[2]=0;
    
	image_type = 0;

    // ファイルは外部で読み込み済
    filesize = nSize; //nNesRom;
    buf = pRomAddr;   //pNesRom;
    
	// Header check...
	p = buf;
	CopyMemIncPtr( &g_ROM.header, p, sizeof(NES_header) );
    
    if( ( !core_memcmp( (char*)g_ROM.header.id, "NES", 3) && ( g_ROM.header.ctrl_z == 0x1A) ) ||
        ( !core_memcmp( (char*)g_ROM.header.id, "NEZ", 3) && ( g_ROM.header.ctrl_z == 0x1A) ) ||
        ( !core_memcmp( (char*)g_ROM.header.id, "FDS", 3) && ( g_ROM.header.ctrl_z == 0x1A) ) ||
        ( g_ROM.header.id[0] <= 0x1A && g_ROM.header.id[1] == 0x00 && (g_ROM.header.id[2] || g_ROM.header.id[3] || g_ROM.header.ctrl_z)&& g_ROM.header.num_8k_vrom_banks == 0x00 ) ||
        ( (!core_memcmp( (char*)g_ROM.header.id, "NES", 3) && (g_ROM.header.ctrl_z == 'M'  )) ||
          ( !core_memcmp( (char*)g_ROM.header.id, "UNIF", 4)))
	  ) {
		p=buf+sizeof(NES_header);
	}
	else
	{
		DEBUG("UNKNOWN FORMAT!");
		return NULL;
	}

	// internal check...
    if( (g_ROM.header.ctrl_z==0x1A) &&
        (g_ROM.header.id[0] == 'N') && (g_ROM.header.id[1] == 'E') &&
        (g_ROM.header.id[2] == 'S' || g_ROM.header.id[2] == 'Z') ) {
        
        // load g_ROM.trainer if present
		if( NES_ROM_has_trainer() ) {
            if( p+TRAINER_LEN-buf > filesize ){
                DEBUG("Error reading Trainer");
                return NULL;
            }
            CopyMemIncPtr( g_ROM.trainer, p, TRAINER_LEN );
            g_ROM.crc = CrcCalc(g_ROM.trainer, TRAINER_LEN);
        }
        
        if( p + (16*1024) * g_ROM.header.num_16k_rom_banks - buf > filesize ){
            DEBUG("Error reading ROM banks");
            return NULL;
        }
        CopyMemIncPtr( g_ROM.ROM_banks, p, (16*1024) * g_ROM.header.num_16k_rom_banks );
        
        if( p + (8*1024) * g_ROM.header.num_8k_vrom_banks - buf > filesize ){
            DEBUG("Error reading VROM banks");
            return NULL;
        }
        
        CopyMemIncPtr( g_ROM.VROM_banks, p, (8*1024) * g_ROM.header.num_8k_vrom_banks );

        if(((g_ROM.header.flags_1 >> 4) | (g_ROM.header.flags_2 & 0xF0)) == 20) {
            u32 i;
            u8 disk_num;
            u8 disk[0x10000];
            u8 disk_header[15] = {
                0x01,0x2A,0x4E,0x49,0x4E,0x54,0x45,0x4E,0x44,0x4F,0x2D,0x48,0x56,0x43,0x2A
              };
            image_type = 1;
            // convert NES disk image
            disk_num = g_ROM.header.num_16k_rom_banks >> 2;

            if(disk_num > 4) {
                disk_num = 4;
            }
            
            for (i = 0; i < disk_num; i++) {
                int rpos, wpos;
                wpos = i*65500+16;
                rpos = i*0x10000;
                
                core_memcpy(disk, &g_ROM.ROM_banks[rpos], 65500);
                core_memcpy(&g_ROM.ROM_banks[wpos], disk_header, 15);
                wpos+=15;
                core_memcpy(&g_ROM.ROM_banks[wpos], disk, 65500-15);
            }

			//g_ROM.header.id[0] = 'N';
			//g_ROM.header.id[1] = 'E';
			//g_ROM.header.id[2] = 'S';
			//g_ROM.header.num_16k_rom_banks = disk_num*4;
			//g_ROM.header.num_8k_vrom_banks = 0;
			//g_ROM.header.flags_1 = 0x40;
			//g_ROM.header.flags_2 = 0x10;
			//g_ROM.header.reserved[0] = g_ROM.header.reserved[1] = g_ROM.header.reserved[2] = g_ROM.header.reserved[3] = 0;
			//g_ROM.header.reserved[4] = g_ROM.header.reserved[5] = g_ROM.header.reserved[6] = g_ROM.header.reserved[7] = 0;
			g_ROM.ROM_banks[0] = 'F';
			g_ROM.ROM_banks[1] = 'D';
			g_ROM.ROM_banks[2] = 'S';
			g_ROM.ROM_banks[3] = 0x1A;
			g_ROM.ROM_banks[4] = disk_num;
		}
	}
	else if( !core_memcmp( (char*)g_ROM.header.id, "FDS", 3) && ( g_ROM.header.ctrl_z == 0x1A) )
	{
//		if(!g_NESConfig.preferences.DisableIPSPatch)
//			LoadIPSPatch((unsigned char *)p, (char *)fn);		//IPS PATCH
		u8 disk_num;

		image_type = 1;
		disk_num = g_ROM.header.num_16k_rom_banks;

		g_ROM.header.id[0] = 'N';
		g_ROM.header.id[1] = 'E';
		g_ROM.header.id[2] = 'S';
		g_ROM.header.num_16k_rom_banks *= 4;
		g_ROM.header.num_8k_vrom_banks = 0;
		g_ROM.header.flags_1 = 0x40;
		g_ROM.header.flags_2 = 0x10;
		g_ROM.header.reserved[0] = g_ROM.header.reserved[1] = g_ROM.header.reserved[2] = g_ROM.header.reserved[3] = 0;
		g_ROM.header.reserved[4] = g_ROM.header.reserved[5] = g_ROM.header.reserved[6] = g_ROM.header.reserved[7] = 0;

		if( p + 65500 * disk_num - buf > filesize ){
			DEBUG("Error reading FDS Image");
			return NULL;
		}
		CopyMemIncPtr( g_ROM.ROM_banks + 16, p, 65500 * disk_num );

		g_ROM.ROM_banks[0] = 'F';
		g_ROM.ROM_banks[1] = 'D';
		g_ROM.ROM_banks[2] = 'S';
		g_ROM.ROM_banks[3] = 0x1A;
		g_ROM.ROM_banks[4] = disk_num;
	}
	else if(g_ROM.header.id[0] <= 0x1A && g_ROM.header.id[1] == 0x00 && g_ROM.header.num_8k_vrom_banks == 0x00)
	{
		u8 fam[6];
		u8 disk_num;
		image_type = 1;
		fam[0] = g_ROM.header.id[0];
		fam[1] = g_ROM.header.id[1];
		fam[2] = g_ROM.header.id[2];
		fam[3] = g_ROM.header.ctrl_z;
		fam[4] = g_ROM.header.num_16k_rom_banks;
		fam[5] = g_ROM.header.num_8k_vrom_banks;

		p = 6 + buf;

		while(!((fam[0] == 0x13 || fam[0] == 0x1A) && fam[1] == 0x00))
		{
			if(p + (u32)fam[2]+((u32)fam[3]<<8)+((u32)fam[4]<<16)-6 - buf > filesize) {
				DEBUG("Error reading FAM image");
				return NULL;
			}
			p += (u32)fam[2]+((u32)fam[3]<<8)+((u32)fam[4]<<16)-6;
			if(p + 6 - buf > filesize) {
				DEBUG("Error reading FAM image");
				return NULL;
			}
			CopyMemIncPtr( fam, p, 6 );
		}

		disk_num = (u8)(((u32)fam[2]+((u32)fam[3]<<8)+((u32)fam[4]<<16))/65500);

		g_ROM.header.id[0] = 'N';
		g_ROM.header.id[1] = 'E';
		g_ROM.header.id[2] = 'S';
		g_ROM.header.num_16k_rom_banks = disk_num*4;
		g_ROM.header.num_8k_vrom_banks = 0;
		g_ROM.header.flags_1 = 0x40;
		g_ROM.header.flags_2 = 0x10;
		g_ROM.header.reserved[0] = g_ROM.header.reserved[1] = g_ROM.header.reserved[2] = g_ROM.header.reserved[3] = 0;
		g_ROM.header.reserved[4] = g_ROM.header.reserved[5] = g_ROM.header.reserved[6] = g_ROM.header.reserved[7] = 0;

		if(fam[0] == 0x1A){
			if( p + 16 - buf > filesize ) {
				DEBUG("Error reading FAM image");
				return NULL;
			}
			p += 16;
		}

		if( p + 65500 * disk_num - buf > filesize ) {
			DEBUG("Error reading FAM image");
			return NULL;
		}
		CopyMemIncPtr( g_ROM.ROM_banks + 16, p, 65500 * disk_num );

		g_ROM.ROM_banks[0] = 'F';
		g_ROM.ROM_banks[1] = 'D';
		g_ROM.ROM_banks[2] = 'S';
		g_ROM.ROM_banks[3] = 0x1A;
		g_ROM.ROM_banks[4] = disk_num;
	}
	else if(!core_memcmp((const char*)g_ROM.header.id, "NES", 3) && (g_ROM.header.ctrl_z == 'M'))
	{
		image_type = 2;

		if( filesize > 0x40000 ){
			DEBUG("NSF file is over 256 KB");
			return NULL;
		}
		CopyMemIncPtr( g_ROM.ROM_banks + 0x10, p, filesize - ( p - buf ) );

		g_ROM.ROM_banks[0x0] = filesize & 0xFF;
		g_ROM.ROM_banks[0x1] = (filesize & 0xFF00) >> 8;
		g_ROM.ROM_banks[0x2] = (filesize & 0xFF0000) >> 16;
		g_ROM.ROM_banks[0x3] = filesize >> 24;
		g_ROM.ROM_banks[0x4] = g_ROM.header.num_16k_rom_banks;
		g_ROM.ROM_banks[0x5] = g_ROM.header.num_8k_vrom_banks;
		g_ROM.ROM_banks[0x6] = g_ROM.header.flags_1;
		g_ROM.ROM_banks[0x7] = g_ROM.header.flags_2;
		g_ROM.ROM_banks[0x8] = g_ROM.header.reserved[0];
		g_ROM.ROM_banks[0x9] = g_ROM.header.reserved[1];
		g_ROM.ROM_banks[0xA] = g_ROM.header.reserved[2];
		g_ROM.ROM_banks[0xB] = g_ROM.header.reserved[3];
		g_ROM.ROM_banks[0xC] = g_ROM.header.reserved[4];
		g_ROM.ROM_banks[0xD] = g_ROM.header.reserved[5];
		g_ROM.ROM_banks[0xE] = g_ROM.header.reserved[6];
		g_ROM.ROM_banks[0xF] = g_ROM.header.reserved[7];

		g_ROM.header.id[0] = 'N';
		g_ROM.header.id[1] = 'E';
		g_ROM.header.id[2] = 'S';
		g_ROM.header.ctrl_z = 0x1A;
		g_ROM.header.num_16k_rom_banks = 1;
		g_ROM.header.num_8k_vrom_banks = 0;
		g_ROM.header.flags_1 = 0xC0;
		g_ROM.header.flags_2 = 0x00;
		g_ROM.header.reserved[0] = g_ROM.header.reserved[1] = g_ROM.header.reserved[2] = g_ROM.header.reserved[3] = 0;
		g_ROM.header.reserved[4] = g_ROM.header.reserved[5] = g_ROM.header.reserved[6] = g_ROM.header.reserved[7] = 0;
	}
	else if(!core_memcmp((char*)g_ROM.header.id, "UNIF", 4)){		// UNIF
		int /*promcn=0, cromcn=0,*/ promtsize=0, cromtsize=0, unif_pos=0x10;
		unsigned char *prommp[16], *crommp[16];
		unsigned int prommsize[16], crommsize[16], promcsize[16], cromcsize[16];
		const char *mapperstr[4]={ "BMC-NovelDiamond9999999" ,"BTL-MARIO1-MALEE" , "BMC-Supervision16in1","BMC-Super24in1"};
		g_ROM.header.num_16k_rom_banks = g_ROM.header.num_8k_vrom_banks = 0;
		g_ROM.header.flags_1 = g_ROM.header.flags_2 = 0;
		g_ROM.unif_psize_16k = g_ROM.unif_csize_8k =0;
		core_memset(prommp, 0, sizeof(prommp));
		core_memset(crommp, 0, sizeof(crommp));

		image_type = 3;
		g_ROM.unif_mapper = 0xFF;
		while(unif_pos < (filesize-0x10)){
			u32 ch = *((u32 *)&p[unif_pos]);
			u32 chunksize = *((u32 *)&p[unif_pos+4]);
			if(ch==0x5250414D){						//MAPR
				if(chunksize>=23 && !core_memcmp(&p[unif_pos+8], mapperstr[0], 23)){
					g_ROM.unif_mapper = 1;
					core_strcpy(g_ROM.GameTitle, mapperstr[0]);
				}
				else if(chunksize>=16 && !core_memcmp(&p[unif_pos+8], mapperstr[1], 16)){
					g_ROM.unif_mapper = 2;
					core_strcpy(g_ROM.GameTitle, mapperstr[1]);
				}
				else if(chunksize>=20 && !core_memcmp(&p[unif_pos+8], mapperstr[2], 20)){
					g_ROM.unif_mapper = 3;
					core_strcpy(g_ROM.GameTitle, mapperstr[2]);
				}
				else if(chunksize>=14 && !core_memcmp(&p[unif_pos+8], mapperstr[3], 14)){
					g_ROM.unif_mapper = 4;
					core_strcpy(g_ROM.GameTitle, mapperstr[3]);
				}
			}
			else if(ch==0x5252494D){				//MIRR
			}
			else if(ch==0x52544142){				//BATR
			}
			else if((ch&0x00ffffff)==0x00475250){	//PRG?
				int msize, pagen;
				pagen = (ch>>24)-'0';
				if(chunksize<0x4000){
					msize=0x4000;
				}
				else{
					msize=chunksize&0xFFFFC000;
					if(chunksize&0x3FFF)
						msize+=0x4000;
				}
				g_ROM.unif_psize_16k += (unsigned char)(msize/0x4000);
				prommp[pagen] = &p[unif_pos+8];
				promcsize[pagen] = chunksize;
				prommsize[pagen] = msize;
				promtsize += msize;
			}
			else if((ch&0x00ffffff)==0x00524843){	//CHR?
				int msize, pagen;
				pagen = (ch>>24)-'0';
				if(chunksize<0x2000){
					msize=0x2000;
				}
				else{
					msize=chunksize&0xFFFFE000;
					if(chunksize&0x1FFF)
						msize+=0x2000;
				}
				g_ROM.unif_csize_8k += (unsigned char)(msize/0x2000);
				crommp[pagen] = &p[unif_pos+8];
				cromcsize[pagen] = chunksize;
				crommsize[pagen] = msize;
				cromtsize += msize;
			}
			unif_pos += (chunksize + 8);
		}
		{
			int i, tsize;

			for(tsize=0, i=0; prommp[i]&&i<16; i++){
				core_memcpy(&g_ROM.ROM_banks[tsize], prommp[i], promcsize[i]);
				tsize+=prommsize[i];
			}
			for(tsize=0, i=0; crommp[i]&&i<16; i++){
				core_memcpy(&g_ROM.VROM_banks[tsize], crommp[i], cromcsize[i]);
				tsize+=crommsize[i];
			}
		}
	}
	else
	{
		DEBUG("Unsupported File");
		return NULL;
	}

	//	u32 j;
	g_ROM.screen_mode = 1;

	// figure out g_ROM.mapper number
	g_ROM.mapper = ( g_ROM.header.flags_1 >> 4);

	// if there is anything in the reserved bytes,
	// don't trust the high nybble of the mapper number
	//	for( i = 0; i < sizeof(g_ROM.header.reserved); i++ )
	//	{
	//		if(g_ROM.header.reserved[i] != 0x00) throw "Invalid NES g_ROM.header ($8-$F)";
	//	}
	g_ROM.mapper |= ( g_ROM.header.flags_2 & 0xF0 );

	g_ROM.dbcorrect[0]=0;
//	if(g_ROM.unif_mapper)
//		return;

	if(image_type == 1)
	{
		int i;
		//	g_ROM.screen_mode = 1;
		g_ROM.mapper = 20;

		g_ROM.fds = (g_ROM.ROM_banks[0x1f] << 24) | (g_ROM.ROM_banks[0x20] << 16) |
		      (g_ROM.ROM_banks[0x21] <<  8) | (g_ROM.ROM_banks[0x22] <<  0);
		for(i = 0; i < g_ROM.ROM_banks[4]; i++)
		{
			u8 file_num = 0;
			u32 pt = 16+65500*i+0x3a;
			while(g_ROM.ROM_banks[pt] == 0x03)
			{
				pt += 0x0d;
				pt += g_ROM.ROM_banks[pt] + g_ROM.ROM_banks[pt+1] * 256 + 4;
				file_num++;
			}
			g_ROM.ROM_banks[16+65500*i+0x39] = file_num;
		}
	}
	else if(image_type == 2)
	{
		//    g_ROM.screen_mode = 1;
		g_ROM.mapper = 12; // 12 is private g_ROM.mapper number
	}
	else if(image_type == 0)
	{
        NESDB* pNesDB;
        
//		g_ROM.crc = CrcCalc(g_ROM.ROM_banks, g_ROM.header.num_16k_rom_banks * 0x4000);
        g_ROM.crc = CrcCalca(g_ROM.ROM_banks, g_ROM.header.num_16k_rom_banks * 0x4000, g_ROM.crc);
		g_ROM.crc_all = CrcCalca(g_ROM.VROM_banks, g_ROM.header.num_8k_vrom_banks * 0x2000, g_ROM.crc);

        if( (pNesDB = NES_findDB(&g_ROM)) ) {
            g_ROM.screen_mode = pNesDB->screen_mode;
            g_ROM.dbcorrect[0]= 1;
            g_ROM.dbcorrect[1]= g_ROM.header.flags_1;
            g_ROM.dbcorrect[2]= g_ROM.header.flags_2;
            g_ROM.header.flags_1 = pNesDB->header[0];
            g_ROM.header.flags_2 = pNesDB->header[1];
            g_ROM.mapper = (g_ROM.header.flags_1 >> 4) | (g_ROM.header.flags_2 & 0xF0);
        }
	}
	#include "nes_rom_correct.cpp"
	return &g_ROM;
}

mirroring_type NES_ROM_get_mirroring()
{
    if(g_ROM.header.flags_1 & MASK_4SCREEN_MIRRORING) {
        return NES_PPU_MIRROR_FOUR_SCREEN;
    }
    else if(g_ROM.header.flags_1 & MASK_VERTICAL_MIRRORING) {
        return NES_PPU_MIRROR_VERT;
    }
    else {
        return NES_PPU_MIRROR_HORIZ;
    }
}


u8 NES_ROM_get_mapper_num() { return g_ROM.mapper; }
u8 NES_ROM_get_unifmapper_num() { return g_ROM.unif_mapper; }

u8  NES_ROM_has_save_RAM()   { return g_ROM.header.flags_1 & MASK_HAS_SAVE_RAM; }
u8  NES_ROM_has_trainer()    { return g_ROM.header.flags_1 & MASK_HAS_TRAINER;  }
u8  NES_ROM_is_VSUnisystem() { return g_ROM.header.flags_2 & 0x01;              }

u8 NES_ROM_get_num_16k_ROM_banks() { return g_ROM.header.num_16k_rom_banks; }
u8 NES_ROM_get_num_8k_VROM_banks() { return g_ROM.header.num_8k_vrom_banks; }

u8* NES_ROM_get_trainer()    { return g_ROM.trainer;     }
u8* NES_ROM_get_ROM_banks()  { return g_ROM.ROM_banks;   }
u8* NES_ROM_get_VROM_banks() { return g_ROM.VROM_banks;  }

//const char* NES_ROM_GetRomName() { return g_ROM.rom_name; }
//const char* NES_ROM_GetRomPath() { return g_ROM.rom_path; }

u8 NES_ROM_get_screen_mode() { return g_ROM.screen_mode; }
char *NES_ROM_get_GameTitleName(){return g_ROM.GameTitle; }

u32 NES_ROM_crc32()       { return g_ROM.crc;  }
u32 NES_ROM_crc32_all()       { return g_ROM.crc_all;  }
u32 NES_ROM_fds_id()	{ return g_ROM.fds; }

// for Best Play - Pro Yakyuu Special
u32 NES_ROM_get_size_SaveRAM() { return g_ROM.size_SaveRAM; }

u32 NES_ROM_get_UNIF_psize_16k() { return g_ROM.unif_psize_16k;  }
u32 NES_ROM_get_UNIF_csize_8k() { return g_ROM.unif_csize_8k;  }


void NES_ROM_GetROMInfoStr(char *h){
	char headerflag[5],headerflag2[5];
	int i, j;
	unsigned char *th = (unsigned char *)&g_ROM.header;
//	h[0x10]= g_ROM.dbcorrect[0], h[0x11]= g_ROM.dbcorrect[1], h[0x12]= g_ROM.dbcorrect[2];
	for(i=0,j=1; i<4; ++i, j<<=1){
		if(th[6] & j)
			headerflag[i]='1';
		else
			headerflag[i]='0';
		if(th[0x11] & j)
			headerflag2[i]='1';
		else
			headerflag2[i]='0';
	}
	headerflag[4]=0, headerflag2[4]=0;
/*	if(g_ROM.dbcorrect[0]){
		wsprintf(h, " Mapper [ %u -> %u ], PROM %uKB, CROM %uKB, FLAG[ %s -> %s ], PROM g_ROM.crc 0x%08X, ROM g_ROM.crc 0x%08X", (g_ROM.dbcorrect[1] >> 4)|(g_ROM.dbcorrect[2] & 0xF0),(th[6] >> 4)|(th[7] & 0xF0), th[4]*16, th[5]*8, headerflag2, headerflag, g_ROM.crc, g_ROM.crc_all);
	}
	else{
		wsprintf(h, " Mapper [ %u ], PROM %uKB, CROM %uKB, FLAG %s, PROM g_ROM.crc 0x%08X, ROM g_ROM.crc 0x%08X", (th[6] >> 4)|(th[7] & 0xF0), th[4]*16, th[5]*8, headerflag, g_ROM.crc, g_ROM.crc_all);
	}*/
}


