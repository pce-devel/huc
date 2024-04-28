#ifndef _SYM_H
#define _SYM_H

intptr_t declglb (intptr_t typ, intptr_t stor, TAG_SYMBOL *mtag, int otag, int is_struct);
void declloc (intptr_t typ, intptr_t stclass, int otag);
intptr_t needsub (void);
SYMBOL *findglb (char *sname);
SYMBOL *findloc (char *sname);
SYMBOL *addglb (char *sname, char id, char typ, intptr_t value, intptr_t stor, SYMBOL *replace);
SYMBOL *addglb_far (char *sname, char typ);
SYMBOL *addloc (char *sname, char id, char typ, intptr_t value, intptr_t stclass, intptr_t size);
intptr_t symname (char *sname);
void illname (void);
void multidef (char *sname);
intptr_t glint (SYMBOL *sym);
SYMBOL *copysym (SYMBOL *sym);
#endif
