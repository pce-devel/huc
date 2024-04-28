#ifndef _INCLUDE_PREPROC_H
#define _INCLUDE_PREPROC_H

void doinclude (void);

void incl_globals (void);

FILE *fixiname (void);

void init_path (void);

void doasmdef (void);

void doasm (void);

void dodefine (void);

void doundef (void);

void preprocess (void);

void doifdef (intptr_t ifdef);

intptr_t ifline (void);

void noiferr (void);

intptr_t cpp (int);

intptr_t keepch (char c);

void defmac (char *s);

void addmac (void);

void delmac (struct macro *mp);

struct macro *findmac (char *sname);

void toggle (char name, intptr_t onoff);

char **include_dirs (void);

#endif
