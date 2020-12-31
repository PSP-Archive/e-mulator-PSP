/*--------------------------------------------------
   TGB Dual - Gameboy Emulator -
   Copyright (C) 2001  Hii

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2
   of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

//-------------------------------------------------
// GB その他エミュレーション部/外部とのインターフェース
int pad_state;

#include "gb.h"
//#include "../menu.h"
#include "hal.h"

FBFORMAT fb_format;
void renderer_refresh();
void renderer_reset();
void renderer_init();
void render_screen(void *buf);
void gbc_snd_render(short *bufL,short *bufR,int sample);


#define VFRAME_SIZE (SIZE_LINE*256*2)

int now_gb_mode;
struct gb_regs g_regs;
struct gbc_regs cg_regs;

//word dmy[160*5]; // vframe はみ出した時用
//word vframe_mem[SIZE_LINE*(144+112)];

//#ifdef USE_GPU
//word *vframe = (word*)0x040CC000;
//#else
word *vframe = 0;//vframe_mem;
//#endif

struct ext_hook hook_proc;

int skip,skip_buf;
int now_frame;
int re_render;

bool hook_ext;
bool use_gba;

void gb_fill_vframe(word color)
{
	memset(vframe,0,VFRAME_SIZE);
}

void gb_init(void)
{
	lcd_init();
	rom_init();
	apu_init();// ROMより後に作られたし
	mbc_init();
	cpu_init();
	sgb_init();
	cheat_init();
	
	apu_reset();
	mbc_reset();
	//target=NULL;
	
	renderer_init();

	gb_reset();

	hook_ext=false;
	use_gba=false;
}

void set_gb_type();

void gb_reset()
{
	set_gb_type();
	
	g_regs.SC=0;
	g_regs.DIV=0;
	g_regs.TIMA=0;
	g_regs.TMA=0;
	g_regs.TAC=0;
	g_regs.LCDC=0x91;
	g_regs.STAT=0;
	g_regs.SCY=0;
	g_regs.SCX=0;
	g_regs.LY=153;
	g_regs.LYC=0;
	g_regs.BGP=0xFC;
	g_regs.OBP1=0xFF;
	g_regs.OBP2=0xFF;
	g_regs.WY=0;
	g_regs.WX=0;
	g_regs.IF=0;
	g_regs.IE=0;
	
	cpu_irq_check();
	
//	memset(&c_regs,0,sizeof(c_regs));
	
	cpu_reset();
	lcd_reset();
	apu_reset();
	mbc_reset();
	sgb_reset();
	
	renderer_reset();
	
	gb_fill_vframe(0);
	
	now_frame=0;
	skip=skip_buf=0;
	re_render=0;
	
//	char *gb_names[]={"Invalid","Gameboy","SuperGameboy","Gameboy Color","Gameboy Advance"};
//	if (m_rom->get_loaded())
//		renderer_output_log("Current GB Type : %s \n",gb_names[m_rom->get_info()->gb_type]);
}

void gb_hook_extport(struct ext_hook *ext)
{
	hook_proc=*ext;
	hook_ext=true;
}

void gb_unhook_extport()
{
	hook_ext=false;
}

void gb_set_skip(int frame)
{
	skip_buf=frame;
}

bool gb_load_rom(byte *buf,int size,byte *ram,int ram_size)
{
	if (rom_load_rom(buf,size,ram,ram_size)){
		gb_reset();
		return true;
	}
	else
		return false;
}

#if 1 ////////

//#define write_state(in, inlen, out, outbak) \
//{ \
//	if(outbak){ \
//		core_memcpy(out, in, inlen); \
//	} \
//	out += inlen; \
//}

int/*size_t*/ gb_save_state(int fd /*byte *out*/)
{
	const int tbl_ram[]={1,1,1,4,16,8}; // 0と1は保険
//	byte *outbak = out;
	int page,ram_page,dmy=0;
	int halt, mbc_dat;
	int cpu_dat[16],ext_is;
	byte resurved[256],reload=1;
    struct rom_info* pRomInfo = rom_get_info();

//	if (outbak)
//		cheat_decreate_cheat_map();

	HAL_sts_write(fd,&pRomInfo->gb_type,sizeof(int));

	if (pRomInfo->gb_type<=2){ // normal gb & sgb
		HAL_sts_write(fd,cpu_get_ram(),0x2000); // ram
		HAL_sts_write(fd,cpu_get_vram(),0x2000); // vram
		HAL_sts_write(fd,get_sram(),tbl_ram[pRomInfo->ram_size]*0x2000); // sram
		HAL_sts_write(fd,cpu_get_oam(),0xA0);
		HAL_sts_write(fd,cpu_get_stack(),0x80);

		page=(mbc_get_rom()-get_rom())/0x4000;
		ram_page=(mbc_get_sram()-get_sram())/0x2000;

		HAL_sts_write(fd,&page,sizeof(int)); // rom_page
		HAL_sts_write(fd,&ram_page,sizeof(int)); // ram_page

		HAL_sts_write(fd,cpu_get_c_regs(),sizeof(struct cpu_regs)); // cpu_reg
		HAL_sts_write(fd,&g_regs,sizeof(struct gb_regs));//sys_reg
		halt=((*cpu_get_halt())?1:0);
		HAL_sts_write(fd,&halt,sizeof(int));
		HAL_sts_write(fd,&dmy,sizeof(int)); // 元の版ではシリアル通信通信満了までのクロック数
		                                               // (通信の仕様が大幅に変わったためダミーで埋めている)
		mbc_dat=mbc_get_state();
		HAL_sts_write(fd,&mbc_dat,sizeof(int));//MBC

		ext_is=mbc_is_ext_ram()?1:0;
		HAL_sts_write(fd,&ext_is,sizeof(int));

		// ver 1.1 追加
		HAL_sts_write(fd,apu_get_stat_cpu(),sizeof(struct apu_stat));
		HAL_sts_write(fd,apu_get_mem(),0x30);
		HAL_sts_write(fd,apu_get_stat_gen(),sizeof(struct apu_stat));

		memset(resurved,0,256);
		HAL_sts_write(fd,resurved,256);//将来のために確保
		
		// RIN拡張
		if(now_gb_mode==2){
			HAL_sts_write(fd,&sgb_mode, sizeof(int));
			HAL_sts_write(fd,&bit_received, sizeof(int));
			HAL_sts_write(fd,&bits_received, sizeof(int));
			HAL_sts_write(fd,&packets_received, sizeof(int));
			HAL_sts_write(fd,&sgb_state, sizeof(int));
			HAL_sts_write(fd,&sgb_index, sizeof(int));
			HAL_sts_write(fd,&sgb_multiplayer, sizeof(int));
			HAL_sts_write(fd,&sgb_fourplayers, sizeof(int));
			HAL_sts_write(fd,&sgb_nextcontrol, sizeof(int));
			HAL_sts_write(fd,&sgb_readingcontrol, sizeof(int));
			HAL_sts_write(fd,&sgb_mask, sizeof(int));
			
			HAL_sts_write(fd,sgb_palette, sizeof(unsigned short)*8*16);
			HAL_sts_write(fd,sgb_palette_memory, sizeof(unsigned short)*512*4);
			HAL_sts_write(fd,sgb_buffer, 7*16);
			HAL_sts_write(fd,sgb_ATF, 18*20);
			HAL_sts_write(fd,sgb_ATF_list, 45*20*18);
			/*
			sceIoWrite(fd, sgb_border, 2048);
			sceIoWrite(fd, sgb_borderchar, 32*256);
			
			int i, j, n=0;
			for (i=0; i<224; i++){
				for (j=0; j<256; j++){
					if (i>=40 && i<=183 && j==48) j=208;
					border_tmp[n++] = sgb_border_buffer[i*256+j];
				}
			}
			sceIoWrite(fd, border_tmp, sizeof(border_tmp));
			*/
		}
	}
	else if (pRomInfo->gb_type>=3){ // GB Colour / GBA
		HAL_sts_write(fd,cpu_get_ram(),0x2000*4); // ram
		HAL_sts_write(fd,cpu_get_vram(),0x2000*2); // vram
		HAL_sts_write(fd,get_sram(),tbl_ram[pRomInfo->ram_size]*0x2000); // sram
		HAL_sts_write(fd,cpu_get_oam(),0xA0);
		HAL_sts_write(fd,cpu_get_stack(),0x80);

		cpu_save_state(cpu_dat);

		page=(mbc_get_rom()-get_rom())/0x4000;
		ram_page=(mbc_get_sram()-get_sram())/0x2000;

		HAL_sts_write(fd,&page,sizeof(int)); // rom_page
		HAL_sts_write(fd,&ram_page,sizeof(int)); // ram_page
		HAL_sts_write(fd,cpu_dat+0,sizeof(int));//int_page	//color
		HAL_sts_write(fd,cpu_dat+1,sizeof(int));//vram_page	//color


		HAL_sts_write(fd,cpu_get_c_regs(),sizeof(struct cpu_regs)); // cpu_reg
		HAL_sts_write(fd,&g_regs,sizeof(struct gb_regs));//sys_reg
		HAL_sts_write(fd,&cg_regs,sizeof(struct gbc_regs));//col_reg		//color
		HAL_sts_write(fd,lcd_get_pal(0),sizeof(word)*(8*4*2));//palette	//color
		halt=((*cpu_get_halt())?1:0);
		HAL_sts_write(fd,&halt,sizeof(int));
		HAL_sts_write(fd,&dmy,sizeof(int)); // 元の版ではシリアル通信通信満了までのクロック数

		mbc_dat=mbc_get_state();
		HAL_sts_write(fd,&mbc_dat,sizeof(int));//MBC

		ext_is=mbc_is_ext_ram()?1:0;
		HAL_sts_write(fd,&ext_is,sizeof(int));

		//その他諸々
		HAL_sts_write(fd,cpu_dat+2,sizeof(int));	//color
		HAL_sts_write(fd,cpu_dat+3,sizeof(int));	//color
		HAL_sts_write(fd,cpu_dat+4,sizeof(int));	//color
		HAL_sts_write(fd,cpu_dat+5,sizeof(int));	//color
		HAL_sts_write(fd,cpu_dat+6,sizeof(int));	//color
		HAL_sts_write(fd,cpu_dat+7,sizeof(int));	//color

		// ver 1.1 追加
		HAL_sts_write(fd,apu_get_stat_cpu(),sizeof(struct apu_stat));
		HAL_sts_write(fd,apu_get_mem(),0x30);
		HAL_sts_write(fd,apu_get_stat_gen(),sizeof(struct apu_stat));

		memset(resurved,0,256);
//		resurved[0]=1;
		HAL_sts_write(fd,&reload,1);
		HAL_sts_write(fd,resurved,256);//将来のために確保
	}

//	if(outbak)
//		cheat_create_cheat_map();

//	return out-outbak;
	return 1;
}

//#define read_state(fd, out, len) \
//{ \
//	if(buf){ \
//		core_memcpy(out, buf, len); \
//		buf += len; \
//	}else{ \
//		gzread(fd, out, len); \
//	} \
//}

void mbc_set_ext_is(bool ext);

int gb_restore_state(/*gzFile*/int fd /*, const byte *buf*/)
{
	const int tbl_ram[]={1,1,1,4,16,8}; // 0と1は保険
	int gb_type,dmy;
	byte resurved[256];
	int page,ram_page;
	int halt;
	int mbc_dat;
	int ext_is;
	int cpu_dat[16];
    struct rom_info* pRomInfo = rom_get_info();
	
	HAL_sts_read(fd, &gb_type, sizeof(int));
	
	pRomInfo->gb_type=gb_type;
	
	if (gb_type<=2){
		HAL_sts_read(fd, cpu_get_ram(),0x2000); // ram
		HAL_sts_read(fd, cpu_get_vram(),0x2000); // vram
		HAL_sts_read(fd, get_sram(),tbl_ram[pRomInfo->ram_size]*0x2000); // sram
		HAL_sts_read(fd, cpu_get_oam(),0xA0);
		HAL_sts_read(fd, cpu_get_stack(),0x80);

		HAL_sts_read(fd, &page, sizeof(int)); // rom_page
		HAL_sts_read(fd, &ram_page, sizeof(int)); // ram_page
		mbc_set_page(page,ram_page);

		HAL_sts_read(fd, cpu_get_c_regs(),sizeof(struct cpu_regs)); // cpu_reg
		cpu_set_c_regs();
		HAL_sts_read(fd, (void *)&g_regs,sizeof(struct gb_regs)); // sys_reg
		HAL_sts_read(fd, &halt,sizeof(int));
		*cpu_get_halt()=((halt)?true:false);
		HAL_sts_read(fd, &dmy,sizeof(int));

		HAL_sts_read(fd, &mbc_dat,sizeof(int)); // MBC
		mbc_set_state(mbc_dat);
		HAL_sts_read(fd, &ext_is,sizeof(int));
		mbc_set_ext_is(ext_is?true:false);

		// ver 1.1 追加
//		byte tmp[256],tester[100];
//		HAL_sts_read(fd, tmp, 100); // とりあえず調べてみる
//		_memset(tester,0,100);
//		if (_memcmp(tmp,tester,100)!=0){
			// apu 部分
//			sceIoLseek(fd, -100, 1);
			HAL_sts_read(fd, apu_get_stat_cpu(),sizeof(struct apu_stat));
			HAL_sts_read(fd, apu_get_mem(),0x30);
			HAL_sts_read(fd, apu_get_stat_gen(),sizeof(struct apu_stat));
//		}

		HAL_sts_read(fd, resurved, 256);//将来のために確保
		
		// RIN拡張
		if(gb_type==2 && sgb_mode){
			HAL_sts_read(fd, &dmy, sizeof(int));

			HAL_sts_read(fd, &bit_received, sizeof(int));
			HAL_sts_read(fd, &bits_received, sizeof(int));
			HAL_sts_read(fd, &packets_received, sizeof(int));
			HAL_sts_read(fd, &sgb_state, sizeof(int));
			HAL_sts_read(fd, &sgb_index, sizeof(int));
			HAL_sts_read(fd, &sgb_multiplayer, sizeof(int));
			HAL_sts_read(fd, &sgb_fourplayers, sizeof(int));
			HAL_sts_read(fd, &sgb_nextcontrol, sizeof(int));
			HAL_sts_read(fd, &sgb_readingcontrol, sizeof(int));
			HAL_sts_read(fd, &sgb_mask, sizeof(int));
			
			HAL_sts_read(fd, sgb_palette, sizeof(unsigned short)*8*16);
			HAL_sts_read(fd, sgb_palette_memory, sizeof(unsigned short)*512*4);
			HAL_sts_read(fd, sgb_buffer, 7*16);
			HAL_sts_read(fd, sgb_ATF, 18*20);
			HAL_sts_read(fd, sgb_ATF_list, 45*20*18);
			/*
			HAL_sts_read(fd, sgb_border, 2048);
			HAL_sts_read(fd, sgb_borderchar, 32*256);
			HAL_sts_read(fd, border_tmp, sizeof(border_tmp));
			int i, j, n=0;
			for (i=0; i<224; i++){
				for (j=0; j<256; j++){
					if (i>=40 && i<=183 && j==48) j=208;
					sgb_border_buffer[i*256+j] = border_tmp[n++];
				}
			}
			*/
		}
	}
	else if (gb_type>=3){ // GB Colour / GBA
		HAL_sts_read(fd, cpu_get_ram(),0x2000*4); // ram
		HAL_sts_read(fd, cpu_get_vram(),0x2000*2); // vram
		HAL_sts_read(fd, get_sram(),tbl_ram[pRomInfo->ram_size]*0x2000); // sram
		HAL_sts_read(fd, cpu_get_oam(),0xA0);
		HAL_sts_read(fd, cpu_get_stack(),0x80);

		HAL_sts_read(fd, &page, sizeof(int)); // rom_page
		HAL_sts_read(fd, &ram_page, sizeof(int)); // ram_page
		mbc_set_page(page,ram_page);
		page=(mbc_get_rom()-get_rom())/0x4000;
		ram_page=(mbc_get_sram()-get_sram())/0x2000;

		HAL_sts_read(fd, cpu_dat+0,sizeof(int));//int_page
		HAL_sts_read(fd, cpu_dat+1,sizeof(int));//vram_page

		HAL_sts_read(fd, cpu_get_c_regs(),sizeof(struct cpu_regs)); // cpu_reg
		cpu_set_c_regs();
		HAL_sts_read(fd, &g_regs,sizeof(struct gb_regs));//sys_reg
		HAL_sts_read(fd, &cg_regs,sizeof(struct gbc_regs));//col_reg
		HAL_sts_read(fd, lcd_get_pal(0),sizeof(word)*(8*4*2));//palette
		HAL_sts_read(fd, &halt,sizeof(int));
		*cpu_get_halt()=(halt?true:false);
		HAL_sts_read(fd, &dmy,sizeof(int)); // 元の版ではシリアル通信通信満了までのクロック数

		HAL_sts_read(fd, &mbc_dat,sizeof(int)); // MBC
		mbc_set_state(mbc_dat);
		HAL_sts_read(fd, &ext_is,sizeof(int));
		mbc_set_ext_is(ext_is?true:false);

		//その他諸々
		HAL_sts_read(fd, cpu_dat+2,sizeof(int));
		HAL_sts_read(fd, cpu_dat+3,sizeof(int));
		HAL_sts_read(fd, cpu_dat+4,sizeof(int));
		HAL_sts_read(fd, cpu_dat+5,sizeof(int));
		HAL_sts_read(fd, cpu_dat+6,sizeof(int));
		HAL_sts_read(fd, cpu_dat+7,sizeof(int));
		cpu_restore_state(cpu_dat);

		// ver 1.1 追加
//		byte tmp[256],tester[100];
//		HAL_sts_read(fd, tmp,100); // とりあえず調べてみる
//		_memset(tester,0,100);
//		if (_memcmp(tmp,tester,100)!=0){
			// apu 部分
//			sceIoLseek(fd, -100, 1);
			HAL_sts_read(fd, apu_get_stat_cpu(),sizeof(struct apu_stat));
			HAL_sts_read(fd, apu_get_mem(),0x30);
			HAL_sts_read(fd, apu_get_stat_gen(),sizeof(struct apu_stat));

//			HAL_sts_read(fd, tmp,1);
			/* renderer_map_colorからlcd_get_mapped_palへの変更は不要になりました。 ruka
			int i;
			if (tmp[0])
				for (i=0;i<64;i++)
					lcd_get_mapped_pal(i>>2)[i&3]=renderer_map_color(lcd_get_pal(i>>2)[i&3]);
			else{
				for (i=0;i<64;i++)
					lcd_get_pal(i>>2)[i&3]=renderer_unmap_color(lcd_get_pal(i>>2)[i&3]);
				for (i=0;i<64;i++)
					lcd_get_mapped_pal(i>>2)[i&3]=renderer_map_color(lcd_get_pal(i>>2)[i&3]);
			}*/
//		}
		HAL_sts_read(fd, resurved,256);//将来のために確保
	}

	cheat_create_cheat_map();

    return 1;
}
#endif////////

void gb_refresh_pal()
{
	/* renderer_map_colorからlcd_get_mapped_palへの変更は不要になりました。 ruka
	int i;
	for (i=0;i<64;i++)
		lcd_get_mapped_pal(i>>2)[i&3]=renderer_map_color(lcd_get_pal(i>>2)[i&3]);
	*/
}

void gb_run(int bDraw)
{
    int sline; 

#if 9999
	lcd_clear_win_count();

	if (!(g_regs.LCDC&0x80)){ // LCDC 起動時
		memset(vframe,0xff,(160+16)*144*2);
	}
#endif//9999

    for(sline=0;sline<154;sline++) {//if (rom_get_loaded()){

#if 9999
		g_regs.LY = sline;
#endif//999
		
		if (g_regs.LCDC&0x80){ // LCDC 起動時

#if !9999
//			g_regs.LY=(g_regs.LY+1)%154;
#endif//9999
			g_regs.STAT&=0xF8;
			if (g_regs.LYC==g_regs.LY){
				g_regs.STAT|=4;
				if (g_regs.STAT&0x40)
					cpu_irq(INT_LCDC);
			}
#if !9999
//			if (g_regs.LY==0){
//				renderer_refresh();
//				if (now_frame>=skip){
//					render_screen(vframe);
//					now_frame=0;
//				}
//				else
//					now_frame++;
//				lcd_clear_win_count();
////				skip=skip_buf;
//			}
#endif//9999
			if (g_regs.LY>=144){ // VBlank 期間中
				g_regs.STAT|=1;
				if (g_regs.LY==144){
					cpu_exec(72);
					cpu_irq(INT_VBLANK);
					if (g_regs.STAT&0x10)
                      cpu_irq(INT_LCDC);
					cpu_exec(456-80);
				}
				else if (g_regs.LY==153){
					cpu_exec(80);
					g_regs.LY=0;
					cpu_exec(456-80); // 前のラインのかなり早目から0になるようだ。
					g_regs.LY=153;
				}
				else
					cpu_exec(456);
			}
			else{ // VBlank 期間外
				g_regs.STAT|=2;
				if (g_regs.STAT&0x20) cpu_irq(INT_LCDC);
				cpu_exec(80); // state=2
				g_regs.STAT|=3;
				cpu_exec(169); // state=3

				if (dma_executing){ // HBlank DMA
					if (b_dma_first){
						dma_dest_bank=vram_bank;
						if (dma_src<0x4000)      dma_src_bank=get_rom();
						else if (dma_src<0x8000) dma_src_bank=mbc_get_rom();
#if 1
						else if (dma_src<0xA000) dma_src_bank=NULL;
						else if (dma_src<0xC000) dma_src_bank=mbc_get_sram()-0xA000;
						else if (dma_src<0xD000) dma_src_bank=ram-0xC000;
						else if (dma_src<0xE000) dma_src_bank=ram_bank-0xD000;
						else                     dma_src_bank=NULL;
#else
//						else if (dma_src>=0xA000&&dma_src<0xC000) dma_src_bank=mbc_get_sram()-0xA000;
//						else if (dma_src>=0xC000&&dma_src<0xD000) dma_src_bank=ram-0xC000;
//						else if (dma_src>=0xD000&&dma_src<0xE000) dma_src_bank=ram_bank-0xD000;
//						else                                      dma_src_bank=NULL;
#endif
						b_dma_first=false;
					}
					core_memcpy(dma_dest_bank+(dma_dest&0x1ff0),dma_src_bank+dma_src,16);
//					fprintf(cpu_file,"%03d : dma exec %04X -> %04X rest %d\n",g_regs.LY,cpu_dma_src,cpu_dma_dest,cpu_dma_rest);

					dma_src+=16;   dma_src&=0xfff0;
					dma_dest+=16;  dma_dest&=0xfff0;
					dma_rest--;
					if (!dma_rest)
						dma_executing=false;

//					cpu_total_clock+=207*(cpu_speed?2:1);
//					cpu_sys_clock+=207*(cpu_speed?2:1);
//					cpu_div_clock+=207*(cpu_speed?2:1);
//					g_regs.STAT|=3;

					if (bDraw/*now_frame>=skip*/ && !sgb_mask)
						lcd_render(vframe,g_regs.LY);

					g_regs.STAT&=0xfc;
					cpu_exec(207); // state=3
				}
				else{
/*					if (lcd_get_sprite_count()){
						if (lcd_get_sprite_count()>=10){
							cpu_exec(129);
							if ((g_regs.STAT&0x08))
								cpu_irq(INT_LCDC);
							g_regs.STAT&=0xfc;
							if (now_frame>=skip)
								lcd_render(vframe,g_regs.LY);
							cpu_exec(78); // state=0
						}
						else{
							cpu_exec(129*lcd_get_sprite_count()/10);
							if ((g_regs.STAT&0x08))
								cpu_irq(INT_LCDC);
							g_regs.STAT&=0xfc;
							if (now_frame>=skip)
								lcd_render(vframe,g_regs.LY);
							cpu_exec(207-(129*lcd_get_sprite_count()/10)); // state=0
						}
					}
					else{
*/						g_regs.STAT&=0xfc;
						if (bDraw/*now_frame>=skip*/ && !sgb_mask)
							lcd_render(vframe,g_regs.LY);
						if ((g_regs.STAT&0x08))
							cpu_irq(INT_LCDC);
						cpu_exec(207); // state=0
//					}
				}
			}
		}
		else{ // LCDC 停止時
//#if !9999
//			g_regs.LY=0; // g_regs.LY=(g_regs.LY+1)%154;
//#endif
//			re_render++;
//			if (re_render>=154){
//				memset(vframe,0xff,(160+16)*144*2);
//#if !9999
//				renderer_refresh();
//				if (now_frame>=skip){
//					render_screen(vframe);
//					now_frame=0;
//				}
//				else
//					now_frame++;
//				lcd_clear_win_count();
//#endif//9999
//				re_render=0;
//			}
			g_regs.STAT&=0xF8;
			cpu_exec(456);
		}
    }

#if 9999
	renderer_refresh();
    //	renderer_update_pad();
    //	renderer_update_sound();
    
    if( HAL_Sound() ){
        // build sound data
        gbc_snd_render(halSnd.L16,halSnd.R16,736);
        HAL_Sound_Proc16(halSnd.R16,halSnd.L16,736);
    }

#if 9999
    if(bDraw) {
        render_screen(vframe);
    }
#else//999
//	if (now_frame>=skip){
//		render_screen(vframe);
//		now_frame=0;
//	}
//	else{
//		now_frame++;
//	}
#endif//!9999
    //	lcd_clear_win_count();
//	skip=skip_buf;
#endif //9999
}


/*static inline*/ struct rom_info *rom_get_info() { return &info; }
/*static inline*/ byte *get_rom()                 { return first_page; }
/*static inline*/ byte *get_sram()                { return sram; }
/*static inline*/ byte *mbc_get_rom()             { return rom_page; }
/*static inline*/ byte *mbc_get_sram()            { return sram_page; }
/*static inline*/ bool mbc_is_ext_ram()           { return ext_is_ram; }
/*static inline*/ void mbc_set_ext_is(bool ext)   { ext_is_ram=ext; }


/*inline*/ void _memcpy4x(void *d, void *s, unsigned long c)
{
	//for (; c>0; --c) *(((unsigned long *)d)++)=*(((unsigned long *)s)++);
	unsigned long *dd=d,*ss=s;
	for (; c>0; --c) *dd++ = *ss++;
}


/*inline*/ void _memcpy40(void *d, void *s, unsigned long c)
{
	unsigned long *dd=d,*ss=s;
	int i;
	for(i=0; i<5; i++){
		dd[0] = ss[0];
		dd[1] = ss[1];
		dd[2] = ss[2];
		dd[3] = ss[3];
		dd[4] = ss[4];
		dd[5] = ss[5];
		dd[6] = ss[6];
		dd[7] = ss[7];
		dd+=8;
		ss+=8;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
int now_sensor_x,now_sensor_y;

void set_gb_type()
{
    switch(0/*setting.gb_type*/){
      case 0:
        if(org_gbtype==1){
            rom_get_info()->gb_type = 2;
            lcd_set_mpal(0/*setting.gb_palette*/);
        }else if(org_gbtype == 2){
            rom_get_info()->gb_type = 2;
            lcd_set_mpal(PAL_SGB);
        }else if(org_gbtype == 3){
            rom_get_info()->gb_type = 3;
            lcd_set_mpal(PAL_SGB);
        }
        break;
      case 1:
        rom_get_info()->gb_type = 1;
        lcd_set_mpal(PAL_MONOCHROME);
        break;
      case 2:
        rom_get_info()->gb_type = 2;
        if(sgb_mode)
          lcd_set_mpal(PAL_SGB);
        else
          lcd_set_mpal(0/*setting.gb_palette*/);
        break;
      case 3:
        rom_get_info()->gb_type = 3;
        lcd_set_mpal(0/*setting.gb_palette*/);
        break;
      case 4:
        rom_get_info()->gb_type = 4;
        lcd_set_mpal(0/*setting.gb_palette*/);
        break;
    }

    if(rom_get_info()->gb_type>=3 && org_gbtype==3)
      now_gb_mode = 3;
    else if(rom_get_info()->gb_type==2 && sgb_mode)
      now_gb_mode = 2;
    else
      now_gb_mode = 1;
}

byte renderer_get_time(int type)
{
/*	unsigned long now=time(NULL)-cur_time;

	switch(type){
	case 8: // 秒
		return (byte)(now%60);
	case 9: // 分
		return (byte)((now/60)%60);
	case 10: // 時
		return (byte)((now/(60*60))%24);
	case 11: // 日(L)
		return (byte)((now/(24*60*60))&0xff);
	case 12: // 日(H)
		return (byte)((now/(256*24*60*60))&1);
	}*/
	return 0;
}

void renderer_set_time(int type,byte dat)
{/*
	unsigned long now=time(NULL);
	unsigned long adj=now-cur_time;

	switch(type){
	case 8: // 秒
		adj=(adj/60)*60+(dat%60);
		break;
	case 9: // 分
		adj=(adj/(60*60))*60*60+(dat%60)*60+(adj%60);
		break;
	case 10: // 時
		adj=(adj/(24*60*60))*24*60*60+(dat%24)*60*60+(adj%(60*60));
		break;
	case 11: // 日(L)
		adj=(adj/(256*24*60*60))*256*24*60*60+(dat*24*60*60)+(adj%(24*60*60));
		break;
	case 12: // 日(H)
		adj=(dat&1)*256*24*60*60+(adj%(256*24*60*60));
		break;
	}
	cur_time=now-adj;*/
}

word renderer_get_sensor(bool x_y)
{
	return (x_y?(now_sensor_x&0x0fff):(now_sensor_y&0x0fff));
}

int renderer_get_timer_state()
{
	return 0;//cur_time;
}

void renderer_set_timer_state(int timer)
{
	//cur_time=timer;
}

void renderer_refresh()
{
//	renderer_update_pad();
//	renderer_update_sound();
}

void renderer_reset()
{
    pad_state=0;
//	now_sensor_x=now_sensor_y=2047;
}

void renderer_init()
{
//	cur_time=0;
//	render_msg[0]=0;
//	render_msg_mode=0;
	
	renderer_reset();
}

void render_screen(void *buf)
{
	HAL_fb2_bitblt(&fb_format);

    vframe = fb_format.fb;
}

extern void malloc_cheatmap(void);
extern void free_cheatmap(void);

static u32 gbe_init(int nRomSize,byte* pRomAddr)
{
    malloc_cheatmap();
    
    HAL_fb2_init(256,256,&fb_format,HW_GBC);
    
	fb_format.bpp = 2;
	fb_format.width = 256;
	fb_format.height = 256;
	fb_format.pic_h = 144;
	fb_format.pic_w = 160;
	fb_format.pic_x = GUARD_LINE;
	fb_format.pic_y = 0;
    fb_format.rotate = 0;

    vframe = fb_format.fb;
    
//    sram_space = HAL_mem_malloc(16*0x2000+4);
    
    gb_init();

    // 必要ならSRAMファイルを読む
    gb_load_rom(pRomAddr,nRomSize,0,0);

	return 1;
}


//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
static u32 GBC_INIT(int nRomSize,byte* pRomAddr)
{
	gbe_init(nRomSize,pRomAddr);

    return 1;
}

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
static u32 SGB_INIT(int nRomSize,byte* pRomAddr)
{
	gbe_init(nRomSize,pRomAddr);

    return 1;
}


//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
static u32 GBC_LOOP(void)
{
    u32 key32;
    u32 skip = HAL_fps(60);
    
    gb_run( !skip );

    key32 = HAL_Input(0,HW_GBC);
    pad_state = key32;
    
    return (key32 & (1<<31));
}

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
static u32 GBC_EXIT(void)
{
//    if(sram_space) {
//        HAL_mem_free(sram_space);
//        sram_space=0;
//    }
    
    free_cheatmap();
    return 1;
}

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
static void GBC_RESET(void)
{
    gb_reset();
}


//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
int GBC_Setup(void)
{
    HAL_SetupExt(EXT_SGB ,"sgb" ,
                 SGB_INIT , GBC_LOOP , GBC_EXIT, GBC_RESET,
                 gb_restore_state, gb_save_state
                 );

    HAL_SetupExt(EXT_GB ,"gb" ,
                 GBC_INIT , GBC_LOOP , GBC_EXIT, GBC_RESET,
                 gb_restore_state, gb_save_state
                 );
    
    HAL_SetupExt(EXT_GBC,"gbc",
                 GBC_INIT , GBC_LOOP , GBC_EXIT, GBC_RESET,
                 gb_restore_state, gb_save_state
                 );
    return 1;
}

