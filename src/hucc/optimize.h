/*	File opt.c: 2.1 (83/03/20,16:02:09) */
/*% cc -O -c %
 *
 */

#ifndef _OPTIMIZE_H
#define _OPTIMIZE_H

/* bit-flag definitions for the i-code instructions */
#define IS_SPREL	1
#define IS_UBYTE	2
#define IS_SBYTE	4
#define IS_USEPR	8
#define IS_STORE	16
#define IS_SHORT	32

/* flag information for each of the i-code instructions */
extern unsigned char icode_flags[];

void push_ins (INS *ins);
void flush_ins (void);
void flush_ins_label (int nextlabel);
void gen_asm (INS *inst);

#endif
