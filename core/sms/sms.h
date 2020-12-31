
#ifndef _SMS_H_
#define _SMS_H_

#define TYPE_OVERSEAS   (0)
#define TYPE_DOMESTIC   (1)

/* SMS context */
typedef struct
{
    u8 dummy[0x2000];
    u8 ram[0x2000];
    u8 sram[0x8000];
    u8 fcr[4];
    u8 paused;
    u8 save;
    u8 country;
    u8 port_3F;
    u8 port_F2;
    u8 use_fm;
    u8 irq;
    u8 psg_mask;
}t_sms;

/* Global data */
extern t_sms sms;

/* Function prototypes */
void sms_frame(int skip_render);
void sms_init(void);
void sms_reset(void);
int  sms_irq_callback(int param);
void sms_mapper_w(int address, int data);
void z80cpu_reset(void);

#endif /* _SMS_H_ */
