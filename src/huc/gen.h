#ifndef _GEN_H
#define _GEN_H

intptr_t getlabel (void);
void getmem (SYMBOL *sym);
void getio (SYMBOL *sym);
void getvram (SYMBOL *sym);
void getloc (SYMBOL *sym);
void putmem (SYMBOL *sym);
void putstk (char typeobj);
void putio (SYMBOL *sym);
void putvram (SYMBOL *sym);
void indirect (char typeobj);
void farpeek (SYMBOL *ptr);
void immed (intptr_t type, intptr_t data);
void gpush (void);
void gpusharg (intptr_t size);
void gpop (void);
void swapstk (void);
void gcall (char *sname, intptr_t nargs);
void gmacro (char *sname, intptr_t nargs);
void gbank (unsigned char bank, unsigned short offset);
void gret (void);
void callstk (intptr_t nargs);
void jump (intptr_t label);
void testjump (intptr_t label, intptr_t ft);
intptr_t modstk (intptr_t newstkp);
void gaslint (void);
void gasrint (void);
void gjcase (void);
void gadd (LVALUE *lval, LVALUE *lval2);
void gsub (void);
void gmult (int is_unsigned);
void gmult_imm (int value);
void gdiv (int is_unsigned);
void gdiv_imm (int value);
void gmod (int is_unsigned);
void gor (void);
void gxor (void);
void gand (void);
void gasr (int is_unsigned);
void gasl (void);
void gneg (void);
void gcom (void);
void gbool (void);
void glneg (void);
void ginc (LVALUE *lval);
void gdec (LVALUE *lval);
void geq (int is_byte);
void gne (int is_byte);
void glt (int is_byte);
void gle (int is_byte);
void ggt (int is_byte);
void gge (int is_byte);
void gult (int is_byte);
void gule (int is_byte);
void gugt (int is_byte);
void guge (int is_byte);
void gcast (int type);
void gsei (void);
void gcli (void);

void scale_const (int type, int otag, intptr_t *size);

#endif
