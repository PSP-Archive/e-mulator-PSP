/* ASG 971222 -- rewrote this interface */
#ifndef __NEC_H_
#define __NEC_H_



enum {
	NEC_IP=1, NEC_AW, NEC_CW, NEC_DW, NEC_BW, NEC_SP, NEC_BP, NEC_IX, NEC_IY,
	NEC_FLAGS, NEC_ES, NEC_CS, NEC_SS, NEC_DS,
	NEC_VECTOR, NEC_PENDING, NEC_NMI_STATE, NEC_IRQ_STATE };

/* Public variables */
//extern int nec_ICount;

/* Public functions */
void nec_set_reg(int,unsigned);
void nec_execute(int cycles);	
unsigned nec_get_reg(int regnum);
void nec_reset (void *param);
int nec_int(u32 wektor);

#endif
