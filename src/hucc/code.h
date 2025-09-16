/*      File code.h: 2.2 (84/11/27,16:26:11) */
/*% cc -O -c %
 *
 */

#ifndef _CODE_H
#define _CODE_H

#include "defs.h"

extern int segment;

void gdata (void);
void gtext (void);
void gzp (void);
void header (void);
void inc_startup (void);
void asmdefines (void);
void defbyte (void);
void defstorage (void);
void defword (void);
void out_ins (int code, int type, intptr_t data);
void out_ins_ex (int code, int type, intptr_t data, int imm_type, intptr_t imm_data);
void out_ins_ex_arg (int code, int type, intptr_t data, int imm_type, intptr_t imm_data, char * string);
void out_ins_sym (int code, int type, intptr_t data, SYMBOL *sym);
void out_ins_cmp (int code, int type);
void gen_ins (INS *tmp);
char gen_code (INS *tmp);

void dump_ins (INS *tmp);

#endif
