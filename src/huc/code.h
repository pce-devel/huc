/*      File code.h: 2.2 (84/11/27,16:26:11) */
/*% cc -O -c %
 *
 */

#ifndef _CODE_H
#define _CODE_H

#include "defs.h"

extern intptr_t segment;

void gdata (void);
void gtext (void);
void header (void);
void inc_startup (void);
void asmdefines (void);
void defbyte (void);
void defstorage (void);
void defword (void);
void out_ins (intptr_t code, intptr_t type, intptr_t data);
void out_ins_ex (intptr_t code, intptr_t type, intptr_t data, intptr_t imm_type, intptr_t imm_data);
void out_ins_sym (intptr_t code, intptr_t type, intptr_t data, SYMBOL *sym);
void gen_ins (INS *tmp);
void gen_code (INS *tmp);

void dump_ins (INS *tmp);

#endif
