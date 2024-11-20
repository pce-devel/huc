/*	File primary.c: 2.4 (84/11/27,16:26:07) */
/*% cc -O -c %
 *
 */

#ifndef _PRIMARY_H
#define _PRIMARY_H

int primary (LVALUE *lval, int comma, bool *deferred);
bool dbltest (LVALUE val1[], LVALUE val2[]);
void result (LVALUE lval[], LVALUE lval2[]);
int constant (int *val);
bool number (int *val);
bool const_expr (int *num, char *, char *);
bool quoted_chr (int *val);
bool quoted_str (int *val);
bool const_str (int *val, const char *str);
bool readqstr (void);
bool readstr (void);
char spechar (void);

int match_type (struct type_type *, int do_ptr, int allow_unk_compound);

#endif
