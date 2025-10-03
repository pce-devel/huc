/*	File pragma.c: 2.1 (00/08/09,04:59:24) */
/*% cc -O -c %
 *
 */

#ifndef _PRAGMA_H
#define _PRAGMA_H

void dopragma (void);
void parse_pragma (void);
void add_fastcall (const char *func);
void new_fastcall (void);
int fastcall_look (const char *fname, int nargs, struct fastcall **p);
int symhash (const char *sym);
int symget (char *sname);
int strmatch (char *lit);
void skip_blanks (void);

extern struct fastcall *fastcall_tbl[256];

#endif
