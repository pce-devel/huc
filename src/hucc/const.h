/*	File const.h: 2.1 (00/07/17,16:02:19) */
/*% cc -O -c %
 *
 */

#ifndef _CONST_H
#define _CONST_H

void new_const (void);
void add_const (char typ);
int array_initializer (char typ, char id, char stor);
int scalar_initializer (char typ, char id, char stor);
int get_string_ptr (char typ);
int get_raw_value (char sep);
int add_buffer (char *p, char c, int is_address);
void dump_const (void);
char *get_const (SYMBOL *);

#endif
