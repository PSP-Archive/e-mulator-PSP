/*
** Nofrendo (c) 1998-2000 Matthew Conte (matt@conte.com)
**
**
** This program is free software; you can redistribute it and/or
** modify it under the terms of version 2 of the GNU Library General
** Public License as published by the Free Software Foundation.
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
**
** nes6502.h
**
** NES custom 6502 CPU definitions / prototypes
** $Id: nes6502.h,v 1.12 2000/09/08 11:54:48 matt Exp $
*/

/* NOTE: 16-bit addresses avoided like the plague: use 32-bit values
**       wherever humanly possible
*/
#ifndef _NES6502_H_
#define _NES6502_H_

/* Define this to enable decimal mode in ADC / SBC (not needed in NES) */
/*#define  NES6502_DECIMAL*/

/* number of bank pointers the CPU emulation core handles */
#ifdef NSF_PLAYER
#define  NES6502_4KBANKS
#endif

#ifdef NES6502_4KBANKS
#define  NES6502_NUMBANKS  16
#define  NES6502_BANKSHIFT 12
#else
#define  NES6502_NUMBANKS  8
#define  NES6502_BANKSHIFT 13
#endif

#define  NES6502_BANKSIZE  (0x10000 / NES6502_NUMBANKS)
#define  NES6502_BANKMASK  (NES6502_BANKSIZE - 1)

/* P (flag) register bitmasks */
#define  N_FLAG         0x80
#define  V_FLAG         0x40
#define  R_FLAG         0x20  /* Reserved, always 1 */
#define  B_FLAG         0x10
#define  D_FLAG         0x08
#define  I_FLAG         0x04
#define  Z_FLAG         0x02
#define  C_FLAG         0x01

/* Vector addresses */
#define  NMI_VECTOR     0xFFFA
#define  RESET_VECTOR   0xFFFC
#define  IRQ_VECTOR     0xFFFE

/* cycle counts for interrupts */
#define  INT_CYCLES     7
#define  RESET_CYCLES   6

#define  NMI_MASK       0x01
#define  IRQ_MASK       0x02

/* Stack is located on 6502 page 1 */
#define  STACK_OFFSET   0x0100
/*
typedef struct
{
   u32 min_range, max_range;
   u8 (*read_func)(u32 address);
} nes6502_memread;

typedef struct
{
   u32 min_range, max_range;
   void (*write_func)(u32 address, u8 value);
} nes6502_memwrite;
*/
typedef struct
{
   u8 *mem_page[NES6502_NUMBANKS];  /* memory page pointers */

//   nes6502_memread *read_handler;
//   nes6502_memwrite *write_handler;

   u32 pc_reg;

   u8 a_reg, p_reg;
   u8 x_reg, y_reg;
   u8 s_reg, __padding;
   u8 int_pending, jammed;

   s32 total_cycles, burn_cycles;
} nes6502_context;

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Functions which govern the 6502's execution */
extern void nes6502_reset(void);
extern int nes6502_execute(int total_cycles);
extern void nes6502_nmi(void);
extern void nes6502_irq(void);
extern void nes6502_pending_irq(void);
extern u8 nes6502_getbyte(u32 address);
extern u32 nes6502_getcycles(u8 reset_flag);
extern void nes6502_burn(int cycles);

/* Context get/set */
extern void nes6502_setcontext(nes6502_context *cpu);
extern void nes6502_getcontext(nes6502_context *cpu);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _NES6502_H_ */

/*
** $Log: nes6502.h,v $
** Revision 1.12  2000/09/08 11:54:48  matt
** optimize
**
** Revision 1.11  2000/09/07 21:58:18  matt
** api change for nes6502_burn, optimized core
**
** Revision 1.10  2000/09/07 01:34:55  matt
** nes6502_init deprecated, moved flag regs to separate vars
**
** Revision 1.9  2000/08/28 12:53:44  matt
** fixes for disassembler
**
** Revision 1.8  2000/08/28 01:46:15  matt
** moved some of them defines around, cleaned up jamming code
**
** Revision 1.7  2000/08/16 04:56:37  matt
** accurate CPU jamming, added dead page emulation
**
** Revision 1.6  2000/07/30 04:32:00  matt
** now emulates the NES frame IRQ
**
** Revision 1.5  2000/07/17 01:52:28  matt
** made sure last line of all source files is a newline
**
** Revision 1.4  2000/06/09 15:12:25  matt
** initial revision
**
*/