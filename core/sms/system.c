/*
    Copyright (C) 1998, 1999, 2000  Charles Mac Donald

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "shared.h"
#include "hal.h"
#include "cstring.h"


t_bitmap bitmap;
t_cart cart;                
t_snd snd;
t_input input;

#ifdef SMS_SOUND
OPLL *opll;
#endif

struct {
	char reg[64];
} ym2413;


void system_init(int rate)
{
	/* Initialize the VDP emulation */
	vdp_init();

	/* Initialize the SMS emulation */
	sms_init();

	/* Initialize the look-up tables and related data */
	render_init();

	/* Enable sound emulation if the sample rate was specified */
	audio_init(rate);

	/* Don't save SRAM by default */
	sms.save = 0;

	/* Clear emulated button state */
	core_memset(&input, 0, sizeof(input));
}

void audio_init(int rate)
{
    /* Clear sound context */
    core_memset(&snd, 0, sizeof(t_snd));

    /* Reset logging data */
    snd.log = 0;
    snd.callback = NULL;

    /* Oops.. sound is disabled */
    if(!rate) return;

    /* Calculate buffer size in samples */
    snd.bufsize = (rate / 60);

    /* Sound output */
    snd.buffer[0] = (signed short int *)HAL_mem_malloc(snd.bufsize * 2);
    snd.buffer[1] = (signed short int *)HAL_mem_malloc(snd.bufsize * 2);

    if(!snd.buffer[0] || !snd.buffer[1]) return;
    core_memset(snd.buffer[0], 0, snd.bufsize * 2);
    core_memset(snd.buffer[1], 0, snd.bufsize * 2);

    /* YM2413 sound stream */
    snd.fm_buffer = (signed short int *)HAL_mem_malloc(snd.bufsize * 2);
    if(!snd.fm_buffer) return;
    core_memset(snd.fm_buffer, 0, snd.bufsize * 2);

    /* SN76489 sound stream */
    snd.psg_buffer[0] = (signed short int *)HAL_mem_malloc(snd.bufsize * 2);
    snd.psg_buffer[1] = (signed short int *)HAL_mem_malloc(snd.bufsize * 2);
    if(!snd.psg_buffer[0] || !snd.psg_buffer[1]) return;
    core_memset(snd.psg_buffer[0], 0, snd.bufsize * 2);
    core_memset(snd.psg_buffer[1], 0, snd.bufsize * 2);

#ifdef SMS_SOUND
    /* Set up SN76489 emulation */
    SN76496_init(0, MASTER_CLOCK, 255, rate);

    /* Set up YM2413 emulation */
    OPLL_init(3579545, rate) ;
    opll = OPLL_new() ;
    OPLL_reset(opll) ;
    OPLL_reset_patch(opll,0) ;            /* if use default voice data. */ 
#endif
    /* Inform other functions that we can use sound */
    snd.enabled = 1;
}


void system_shutdown(void)
{
	if(snd.enabled) {
#ifdef SMS_SOUND
		OPLL_delete(opll);
		OPLL_close();
#endif
	}
}


void system_reset(void)
{
    z80cpu_reset();
    vdp_reset();
    sms_reset();
    render_reset();

    // system_load_sram(); �v����

    if(snd.enabled) {
#ifdef SMS_SOUND
        OPLL_reset(opll) ;
        OPLL_reset_patch(opll,0) ;            /* if use default voice data. */
#endif
    }
}


void system_save_state(int fd)
{
    /* Save VDP context */
    HAL_sts_write(fd,&vdp,sizeof(t_vdp));
    //fwrite(&vdp, sizeof(t_vdp), 1, fd);

    /* Save SMS context */
    HAL_sts_write(fd,&sms,sizeof(t_sms));
    //fwrite(&sms, sizeof(t_sms), 1, fd);

    /* Save Z80 context */
    HAL_sts_write(fd,Z80_Context, sizeof(Z80_Regs));
    HAL_sts_write(fd,&after_EI, sizeof(int));
    //fwrite(Z80_Context, sizeof(Z80_Regs), 1, fd);
    //fwrite(&after_EI, sizeof(int), 1, fd);

#ifdef SMS_SOUND
    /* Save YM2413 registers */
    HAL_sts_write(fd,&ym2413.reg[0], 0x40);
    //fwrite(&ym2413.reg[0], 0x40, 1, fd);

    /* Save SN76489 context */
    HAL_sts_write(fd,&sn[0], sizeof(t_SN76496));
    //fwrite(&sn[0], sizeof(t_SN76496), 1, fd);
#endif
}


void system_load_state(int fd)
{
    int i;
    u8 reg[0x40];

    /* Initialize everything */
    z80cpu_reset();
    system_reset();

    /* Load VDP context */
    HAL_sts_read(fd,&vdp, sizeof(t_vdp));
    //fread(&vdp, sizeof(t_vdp), 1, fd);

    /* Load SMS context */
    HAL_sts_read(fd,&sms, sizeof(t_sms));
    //fread(&sms, sizeof(t_sms), 1, fd);

    /* Load Z80 context */
    HAL_sts_read(fd,Z80_Context, sizeof(Z80_Regs));
    HAL_sts_read(fd,&after_EI, sizeof(int));
    //fread(Z80_Context, sizeof(Z80_Regs), 1, fd);
    //fread(&after_EI, sizeof(int), 1, fd);

    /* Load YM2413 registers */
    HAL_sts_read(fd,reg, 0x40);
    //fread(reg, 0x40, 1, fd);

#ifdef SMS_SOUND
    /* Load SN76489 context */
    HAL_sts_read(fd,&sn[0], sizeof(t_SN76496));
    //fread(&sn[0], sizeof(t_SN76496), 1, fd);
#endif

    /* Restore callbacks */
    z80_set_irq_callback(sms_irq_callback);

    cpu_readmap[0] = cart.rom + 0x0000; /* 0000-3FFF */
    cpu_readmap[1] = cart.rom + 0x2000;
    cpu_readmap[2] = cart.rom + 0x4000; /* 4000-7FFF */
    cpu_readmap[3] = cart.rom + 0x6000;
    cpu_readmap[4] = cart.rom + 0x0000; /* 0000-3FFF */
    cpu_readmap[5] = cart.rom + 0x2000;
    cpu_readmap[6] = sms.ram;
    cpu_readmap[7] = sms.ram;

    cpu_writemap[0] = sms.dummy;
    cpu_writemap[1] = sms.dummy;
    cpu_writemap[2] = sms.dummy;         
    cpu_writemap[3] = sms.dummy;
    cpu_writemap[4] = sms.dummy;         
    cpu_writemap[5] = sms.dummy;
    cpu_writemap[6] = sms.ram;           
    cpu_writemap[7] = sms.ram;

    sms_mapper_w(3, sms.fcr[3]);
    sms_mapper_w(2, sms.fcr[2]);
    sms_mapper_w(1, sms.fcr[1]);
    sms_mapper_w(0, sms.fcr[0]);

    /* Force full pattern cache update */
    is_vram_dirty = 1;
    core_memset(vram_dirty, 1, 0x200);

    /* Restore palette */
	for(i = 0; i < PALETTE_SIZE; i += 1){
		palette_sync(i);
	}

    /* Restore sound state */
    if(snd.enabled) {
#ifdef SMS_SOUND
        /* Clear YM2413 context */
        OPLL_reset(opll) ;
        OPLL_reset_patch(opll,0) ;            /* if use default voice data. */ 

        /* Restore rhythm enable first */
        ym2413_write(0, 0, 0x0E);
        ym2413_write(0, 1, reg[0x0E]);

        /* User instrument settings */
        for(i = 0x00; i <= 0x07; i += 1) {
            ym2413_write(0, 0, i);
            ym2413_write(0, 1, reg[i]);
        }

        /* Channel frequency */
        for(i = 0x10; i <= 0x18; i += 1){
            ym2413_write(0, 0, i);
            ym2413_write(0, 1, reg[i]);
        }

        /* Channel frequency + ctrl. */
        for(i = 0x20; i <= 0x28; i += 1){
            ym2413_write(0, 0, i);
            ym2413_write(0, 1, reg[i]);
        }

        /* Instrument and volume settings  */
        for(i = 0x30; i <= 0x38; i += 1){
            ym2413_write(0, 0, i);
            ym2413_write(0, 1, reg[i]);
        }
#endif
    }
}

void ym2413_write(int chip, int offset, int data)
{
    static u8 latch = 0;

    if(offset & 1) {
#ifdef SMS_SOUND 
        OPLL_writeReg(opll, latch, data);
#endif
    } else {
        latch = data;
    }
}







