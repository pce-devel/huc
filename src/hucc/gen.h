#ifndef _GEN_H
#define _GEN_H

int getlabel (void);
void getmem (SYMBOL *sym);
void getloc (SYMBOL *sym);
void putmem (SYMBOL *sym);
void putstk (char typeobj);
void indirect (char typeobj);
void farpeek (SYMBOL *ptr);
void immed (int type, intptr_t data);
void gpush (void);
void gpusharg (int size);
void gpop (void);
void gcall (char *sname, int nargs);
void gmacro (char *sname, int nargs);
void jump (int label);
void testjump (int label, int ft);
int modstk (int newstkp);
void gaslint (void);
void gasrint (void);
void gswitch (int nlab);
void gcase (int nlab, int value);
void gdefault (int nlab);
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
void gtest (void);
void gnot (void);
void ginc (LVALUE *lval);
void gdec (LVALUE *lval);
void geq (void);
void gne (void);
void glt (void);
void gle (void);
void ggt (void);
void gge (void);
void gult (void);
void gule (void);
void gugt (void);
void guge (void);
void gcast (int type);
void gsei (void);
void gcli (void);
void gfence (void);
void gshort (void);

void scale_const (int type, int otag, int *size);

#endif
