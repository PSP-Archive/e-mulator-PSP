#ifdef _NES_MAPPER_CPP_

#include "cstring.h"

extern u8* nes_diskbios;


STATIC void NES_mapper20_Init();
STATIC void NES_mapper20_Reset();
STATIC u8   NES_mapper20_MemoryReadLow(u32 addr);
STATIC void NES_mapper20_MemoryWriteLow(u32 addr, u8 data);
STATIC void NES_mapper20_MemoryWriteSaveRAM(u32 addr, u8 data);
STATIC void NES_mapper20_MemoryWrite(u32 addr, u8 data);
STATIC void NES_mapper20_HSync(u32 scanline);
STATIC void NES_mapper20_VSync();
STATIC u8   NES_mapper20_GetDiskSideNum();
STATIC u8   NES_mapper20_GetDiskSide();
STATIC void  NES_mapper20_SetDiskSide(u8 side);
STATIC u8 *NES_mapper20_GetDiskDatap();
STATIC u8 NES_mapper20_DiskAccessed();


/////////////////////////////////////////////////////////////////////
// Mapper 20
STATIC void NES_mapper20_Init()
{
	g_NESmapper.Reset = NES_mapper20_Reset;
	g_NESmapper.MemoryReadLow = NES_mapper20_MemoryReadLow;
	g_NESmapper.MemoryWriteLow = NES_mapper20_MemoryWriteLow;
	g_NESmapper.MemoryWriteSaveRAM = NES_mapper20_MemoryWriteSaveRAM;
	g_NESmapper.MemoryWrite = NES_mapper20_MemoryWrite;
	g_NESmapper.HSync = NES_mapper20_HSync;
	g_NESmapper.VSync = NES_mapper20_VSync;
	g_NESmapper.GetDiskSideNum = NES_mapper20_GetDiskSideNum;
	g_NESmapper.GetDiskSide = NES_mapper20_GetDiskSide;
	g_NESmapper.SetDiskSide = NES_mapper20_SetDiskSide;
	g_NESmapper.GetDiskDatap = NES_mapper20_GetDiskDatap;
	g_NESmapper.DiskAccessed = NES_mapper20_DiskAccessed;
}



STATIC void NES_mapper20_Reset()
{
    nes6502_context context;

  // Init ExSound
	NES_APU_SelectExSound(4);

//	hFile = NULL;

	g_NESmapper.Mapper20.wram = g_NES.mapper_extram;
	g_NES.mapper_extramsize = 0x8000;
	core_memset(g_NESmapper.Mapper20.wram, 0, 0x8000);

    if( nes_diskbios ) {
        core_memcpy(g_NESmapper.Mapper20.bios,nes_diskbios,NESDISKROM_SIZE);
    }
  
//#if 1
//    if(nes_diskbios) {
//        core_memcpy(g_NESmapper.Mapper20.bios,nes_diskbios,sizeof(nes_diskbios));
//    }
//#else
//    // DISKSYS.ROM�̃p�X�𐶐�
//	GetModulePath(fn, sizeof(fn));
//	_strcat(fn, "DISKSYS.ROM");
//
//	if((hFile = NES_fopen(fn, FILE_MODE_READ)) >= 0)
//	{
//		u8 head1 = NES_fgetc(hFile);
//		u8 head2 = NES_fgetc(hFile);
//		u8 head3 = NES_fgetc(hFile);
//		if(head1 == 'N' && head2 == 'E' && head3 == 'S')
//		{
//			NES_fseek(hFile, 0x6010, FILE_SEEK_SET);
//		}
//		else
//		{
//			NES_fseek(hFile, 0, FILE_SEEK_SET);
//		}
//#if 0
//		for(u32 i = 0; i < 0x2000; i++)
//		{
//			g_NESmapper.Mapper20.bios[i] = NES_fgetc(hFile);
//		}
//#else
//		NES_fread(g_NESmapper.Mapper20.bios, 1, 0x2000, hFile);
//#endif
//		NES_fclose(hFile);
//	}
//	else
//	{
//		// �Ƃ肠�����X���[�����Ⴈ��
//		//throw "DISKSYS.ROM not found.";
//		return;
//	}
//#endif
  
	// cancel license screen
	//g_NESmapper.Mapper20.bios[0xfb0] = 0x00;

	NES6502_GetContext(&context);
	context.mem_page[3] = g_NESmapper.Mapper20.wram + 0x0000;
	context.mem_page[4] = g_NESmapper.Mapper20.wram + 0x2000;
	context.mem_page[5] = g_NESmapper.Mapper20.wram + 0x4000;
	context.mem_page[6] = g_NESmapper.Mapper20.wram + 0x6000;
	context.mem_page[7] = g_NESmapper.Mapper20.bios;
	NES6502_SetContext(&context);

	// read FDS g_NESmapper.Mapper20.disk image
	if(g_NESmapper.ROM_banks[0] == 'F' && g_NESmapper.ROM_banks[1] == 'D' && g_NESmapper.ROM_banks[2] == 'S')
	{
#if 0
		for(u32 i = 0; i < g_NESmapper.ROM_banks[4]; i++)
		{
			for(u32 j = 0; j < 65500; j++)
			{
				g_NESmapper.Mapper20.disk[0x10000*i+j] = g_NESmapper.ROM_banks[16+65500*i+j];
			}
		}
#else
//		for(u32 i = 0; i < g_NESmapper.ROM_banks[4]; i++){
//			core_memcpy(&g_NESmapper.Mapper20.disk[65500*i], &g_NESmapper.ROM_banks[16+65500*i], 65500);
//		}
		core_memcpy(g_NESmapper.Mapper20.disk, &g_NESmapper.ROM_banks[16], g_NESmapper.ROM_banks[4]*65500);
#endif
	}
	else
	{
		return;
		//throw "Invalid g_NESmapper.Mapper20.disk image.";
	}

	g_NESmapper.Mapper20.patch = 0;
	if(NES_fds_id() == 0xc7525744) // Reflect World
	{
		g_NES.frame_irq_disenabled = 1;
	}
	else if(NES_fds_id() == 0x01534d42) // SMB2
	{
		g_NESmapper.Mapper20.patch = 1;
	}

	g_NESmapper.Mapper20.irq_enabled = 0;
	g_NESmapper.Mapper20.irq_counter = 0;
	g_NESmapper.Mapper20.irq_latch = 0;
	g_NESmapper.Mapper20.irq_wait = 0;

	g_NESmapper.Mapper20.access_flag = 0;
	g_NESmapper.Mapper20.disk_enabled = 1;
	g_NESmapper.Mapper20.head_position = 0;
	g_NESmapper.Mapper20.write_skip = 0;
	g_NESmapper.Mapper20.disk_status = 0;
	g_NESmapper.Mapper20.write_reg = 0;
	g_NESmapper.Mapper20.current_side = 1;
	g_NESmapper.Mapper20.last_side = 1;
	g_NESmapper.Mapper20.insert_wait = 180;
}

STATIC u8 NES_mapper20_MemoryReadLow(u32 addr)
{
	u8 val = 0x00;

	switch (addr)
	{
	case 0x4030:
		{
			val = g_NESmapper.Mapper20.disk_status;
		}
		break;

	case 0x4031:
		{
			if((g_NESmapper.Mapper20.current_side != 0) && (g_NESmapper.Mapper20.current_side == g_NESmapper.Mapper20.last_side))
			{
				val = g_NESmapper.Mapper20.disk[(g_NESmapper.Mapper20.current_side-1)*65500+g_NESmapper.Mapper20.head_position];
				if(g_NESmapper.Mapper20.write_reg & 0x01)
				{
					g_NESmapper.Mapper20.head_position += (g_NESmapper.Mapper20.head_position < 64999) ? 1 : 0;
					g_NESmapper.Mapper20.irq_wait = 2;
				}
				g_NESmapper.Mapper20.access_flag = 1;

			}
			else
			{
				val = 0xff;
			}
		}
		break;

	case 0x4032:
		{
			u8 eject = ((g_NESmapper.Mapper20.current_side != 0) && (g_NESmapper.Mapper20.current_side == g_NESmapper.Mapper20.last_side)) ? 0 : 1;
			val = 0x40;
			val |= eject ? 1 : 0;
			val |= eject ? 4 : 0;
			val |= (!eject && (g_NESmapper.Mapper20.write_reg & 0x01) && !(g_NESmapper.Mapper20.write_reg & 0x02)) ? 0 : 2;

			if(g_NESmapper.Mapper20.last_side != g_NESmapper.Mapper20.current_side)
			{
				// wait 2.0 sec for change g_NESmapper.Mapper20.disk
				if(g_NESmapper.Mapper20.insert_wait > 120)
				{
					g_NESmapper.Mapper20.last_side = g_NESmapper.Mapper20.current_side;
				}
			}
		}
		break;

	case 0x4033:
		{
			val = 0x80;
		}
		break;

	default:
		{
			val = NES_APU_ExRead(addr);
		}
		break;
	}

	return val;
}

STATIC void NES_mapper20_MemoryWriteLow(u32 addr, u8 data)
{
	switch (addr)
	{
	case 0x4020:
		{
			g_NESmapper.Mapper20.irq_latch = (g_NESmapper.Mapper20.irq_latch & 0xFF00) | data;
		}
		break;

	case 0x4021:
		{
			g_NESmapper.Mapper20.irq_latch = (g_NESmapper.Mapper20.irq_latch & 0x00FF) | ((u32)data << 8);
		}
		break;

	case 0x4022:
		{
			g_NESmapper.Mapper20.irq_counter = g_NESmapper.Mapper20.irq_latch;
			g_NESmapper.Mapper20.irq_enabled = data & 0x03;
		}
		break;

	case 0x4023:
		{
			g_NESmapper.Mapper20.disk_enabled = data & 0x01;
		}
		break;

	case 0x4024:
		{
			if((g_NESmapper.Mapper20.current_side != 0) && (g_NESmapper.Mapper20.current_side == g_NESmapper.Mapper20.last_side))
			{
				if(g_NESmapper.Mapper20.disk_enabled && !(g_NESmapper.Mapper20.write_reg & 0x04) && g_NESmapper.Mapper20.head_position < 65000)
				{
					if(g_NESmapper.Mapper20.write_skip)
					{
						g_NESmapper.Mapper20.write_skip--;
					}
					else if(g_NESmapper.Mapper20.head_position >= 2)
					{
						g_NESmapper.Mapper20.disk[(g_NESmapper.Mapper20.current_side-1)*65500+g_NESmapper.Mapper20.head_position-2] = data;
						g_NESmapper.Mapper20.access_flag = 1;
					}
				}
			}
		}
		break;

	case 0x4025:
		{
			if(data & 0x08)
			{
				g_NESmapper.set_mirroring2(NES_PPU_MIRROR_HORIZ);
			}
			else
			{
				g_NESmapper.set_mirroring2(NES_PPU_MIRROR_VERT);
			}
			if((g_NESmapper.Mapper20.current_side != 0) && (g_NESmapper.Mapper20.current_side == g_NESmapper.Mapper20.last_side))
			{
				if((g_NESmapper.Mapper20.write_reg & 0x40) && !(data & 0x10) && !(data & 0x40))
				{
					g_NESmapper.Mapper20.head_position = (g_NESmapper.Mapper20.head_position < 2) ? 0 : g_NESmapper.Mapper20.head_position - 2;
				}
				if(!(data & 0x04))
				{
					g_NESmapper.Mapper20.write_skip = 2;
				}
				if(data & 0x02)
				{
					g_NESmapper.Mapper20.head_position = 0;
					g_NESmapper.Mapper20.irq_wait = 2;
				}
				if(data & 0x80)
				{
					g_NESmapper.Mapper20.irq_wait = 2;
				}
			}
			g_NESmapper.Mapper20.write_reg = data;
		}
		break;

	default:
		{
			if(addr >= 0x4040 && addr < 0x4100)
				NES_APU_ExWrite(addr, data);
		}
		break;
	}
}

STATIC void NES_mapper20_MemoryWriteSaveRAM(u32 addr, u8 data)
{
	g_NESmapper.Mapper20.wram[addr - 0x6000] = data;
}

STATIC void NES_mapper20_MemoryWrite(u32 addr, u8 data)
{
	if(addr < 0xE000)
	{
		g_NESmapper.Mapper20.wram[addr - 0x6000] = data;
	}
}

STATIC void NES_mapper20_HSync(u32 scanline)
{
	g_NESmapper.Mapper20.disk_status &= 0xfc;

	if(g_NESmapper.Mapper20.irq_enabled)
	{
		if(g_NESmapper.Mapper20.irq_counter < (unsigned int)((g_NESmapper.Mapper20.patch)?112:113)) //113)
		{
			g_NESmapper.Mapper20.irq_enabled &= 0x01;
			g_NESmapper.Mapper20.irq_counter = g_NESmapper.Mapper20.irq_latch;
			g_NESmapper.Mapper20.disk_status |= 0x01;
			NES6502_DoIRQ();
		}
		else
		{
			g_NESmapper.Mapper20.irq_counter -= ((g_NESmapper.Mapper20.patch)?112:113); //113;
		}
	}
	else if(g_NESmapper.Mapper20.irq_wait)
	{
		g_NESmapper.Mapper20.irq_wait--;
		if(!g_NESmapper.Mapper20.irq_wait && (g_NESmapper.Mapper20.write_reg & 0x80))
		{
			g_NESmapper.Mapper20.disk_status |= 0x02;
			NES6502_DoIRQ();
		}
	}
}

STATIC void NES_mapper20_VSync()
{
	// count MAX 3sec
	g_NESmapper.Mapper20.insert_wait += (g_NESmapper.Mapper20.insert_wait < 180) ? 1 : 0;
}

void NES_mapper20_SNSS_fixup()
{
	nes6502_context context;
	NES6502_GetContext(&context);
	context.mem_page[3] = g_NESmapper.Mapper20.wram + 0x0000;
	context.mem_page[4] = g_NESmapper.Mapper20.wram + 0x2000;
	context.mem_page[5] = g_NESmapper.Mapper20.wram + 0x4000;
	context.mem_page[6] = g_NESmapper.Mapper20.wram + 0x6000;
	context.mem_page[7] = g_NESmapper.Mapper20.bios;
	NES6502_SetContext(&context);
}

STATIC u8 NES_mapper20_GetDiskSideNum()
{
	return g_NESmapper.ROM_banks[4];
}

STATIC u8 NES_mapper20_GetDiskSide()
{
	return g_NESmapper.Mapper20.current_side;
}

STATIC void  NES_mapper20_SetDiskSide(u8 side)
{
	if(side <= g_NESmapper.ROM_banks[4]) {
		g_NESmapper.Mapper20.current_side = side;
		g_NESmapper.Mapper20.insert_wait = 0;
	}
}

STATIC u8 *NES_mapper20_GetDiskDatap()
{
	return g_NESmapper.Mapper20.disk;
}

STATIC u8 NES_mapper20_DiskAccessed()
{
	u8 retval = g_NESmapper.Mapper20.access_flag;
	g_NESmapper.Mapper20.access_flag = 0;
	return retval;
}


/////////////////////////////////////////////////////////////////////

#endif
