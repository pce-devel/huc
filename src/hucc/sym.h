#ifndef _SYM_H
#define _SYM_H

int declglb (char typ, char stor, TAG_SYMBOL *mtag, int otag, int is_struct);
void declloc (char typ, char stclass, int otag);
int needsub (void);
int needarguments (void);
SYMBOL *findglb (char *sname);
SYMBOL *findloc (char *sname);
SYMBOL *addglb (char *sname, char id, char typ, int value, char stor, SYMBOL *replace);
SYMBOL *addglb_far (char *sname, char typ);
SYMBOL *addloc (char *sname, char id, char typ, int value, char stclass, int size);
bool symname (char *sname);
void illname (void);
void multidef (char *sname);
int glint (SYMBOL *sym);
SYMBOL *copysym (SYMBOL *sym);
#endif
