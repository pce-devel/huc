/*	File stmt.c: 2.1 (83/03/20,16:02:17) */
/*% cc -O -c %
 *
 */


#ifndef _STMT_H
#define _STMT_H

intptr_t statement (intptr_t func);
intptr_t stdecl (void);
intptr_t doldcls (intptr_t stclass);
void stst (void);
void compound (intptr_t func);
void doif (void);
void dowhile (void);
void dodo (void);
void dofor (void);
void doswitch (void);
void docase (void);
void dodefault (void);
void doreturn (void);
void dobreak (void);
void docont (void);
void dolabel (char *name);
void dogoto (void);
void dumpsw (intptr_t *ws);
void test (intptr_t label, intptr_t ft);

#endif
