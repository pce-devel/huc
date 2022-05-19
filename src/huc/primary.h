/*	File primary.c: 2.4 (84/11/27,16:26:07) */
/*% cc -O -c %
 *
 */

#ifndef _PRIMARY_H
#define _PRIMARY_H

intptr_t primary (LVALUE *lval, int comma);
intptr_t dbltest (LVALUE val1[], LVALUE val2[]);
void result (LVALUE lval[], LVALUE lval2[]);
intptr_t constant (intptr_t val[]);
intptr_t number (intptr_t val[]);
int const_expr (intptr_t *, char *, char *);
intptr_t pstr (intptr_t val[]);
intptr_t qstr (intptr_t val[]);
intptr_t const_str (intptr_t val[], const char *);
intptr_t readqstr (void);
intptr_t readstr (void);
intptr_t spechar (void);

int match_type (struct type *, int do_ptr, int allow_unk_compound);

#endif
