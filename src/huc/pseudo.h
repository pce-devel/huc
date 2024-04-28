/*
 * pseudo.h
 */

#ifndef _PSEUDO_H
#define _PSEUDO_H

void dopsdinc (void);
void dopsddef (void);
intptr_t outcomma (void);
intptr_t outnameunderline (void);
intptr_t outconst (void);
void doset_bgpalstatement (void);
void doset_sprpalstatement (void);
void doload_spritesstatement (void);
void doload_backgroundstatement (void);
void do_asm_func (intptr_t type);
char *new_string (intptr_t und, char *a);

#endif
