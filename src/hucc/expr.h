/*	File expr.c: 2.2 (83/06/21,11:24:26) */
/*% cc -O -c %
 *
 */

#ifndef _EXPR_H
#define _EXPR_H

void expression (int comma);
int expression_ex (LVALUE *lval, int comma, int norval);
int heir1 (LVALUE *lval, int comma);
int heir1a (LVALUE *lval, int comma);
int heir1b (LVALUE *lval, int comma);
int heir1c (LVALUE *lval, int comma);
int heir2 (LVALUE *lval, int comma);
int heir3 (LVALUE *lval, int comma);
int heir4 (LVALUE *lval, int comma);
int heir5 (LVALUE *lval, int comma);
int heir6 (LVALUE *lval, int comma);
int heir7 (LVALUE *lval, int comma);
int heir8 (LVALUE *lval, int comma);
int heir9 (LVALUE *lval, int comma);
int heir10 (LVALUE *lval, int comma);
int heir11 (LVALUE *lval, int comma);
void store (LVALUE *lval);
void rvalue (LVALUE *lval);
void needlval (void);

int is_byte (LVALUE *lval);

#endif
