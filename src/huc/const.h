/*	File const.h: 2.1 (00/07/17,16:02:19) */
/*% cc -O -c %
 *
 */

#ifndef _CONST_H
#define _CONST_H

void new_const (void);
void add_const (intptr_t typ);
intptr_t array_initializer (intptr_t typ, intptr_t id, intptr_t stor);
intptr_t scalar_initializer (intptr_t typ, intptr_t id, intptr_t stor);
intptr_t get_string_ptr (intptr_t typ);
intptr_t get_raw_value (char sep);
int add_buffer (char *p, char c, int is_address);
void dump_const (void);
char *get_const (SYMBOL *);

#endif
