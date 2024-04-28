/*	File pragma.c: 2.1 (00/08/09,04:59:24) */
/*% cc -O -c %
 *
 */

#ifndef _PRAGMA_H
#define _PRAGMA_H

void dopragma (void);
void defpragma (void);
void parse_pragma (void);
void new_fastcall (void);
intptr_t fastcall_look (const char *fname, intptr_t nargs, struct fastcall **p);
intptr_t symhash (const char *sym);
intptr_t symget (char *sname);
intptr_t strmatch (char *lit);
void skip_blanks (void);

extern struct fastcall *fastcall_tbl[256];

#endif
