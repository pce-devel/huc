/*	File function.c: 2.1 (83/03/20,16:02:04) */
/*% cc -O -c %
 *
 */

#ifndef _FUNCTION_H
#define _FUNCTION_H

void newfunc (const char *sname, int ret_ptr_order, int ret_type, int ret_otag, int is_fastcall);
int getarg (intptr_t t, int syntax, int otag, int is_fastcall);
void callfunction (char *ptr);
void arg_stack (intptr_t arg);
void arg_push_ins (INS *ptr);
void arg_flush (intptr_t arg, intptr_t adj);
void arg_to_fptr (struct fastcall *fast, intptr_t i, intptr_t arg, intptr_t adj);
void arg_to_dword (struct fastcall *fast, intptr_t i, intptr_t arg, intptr_t adj);

#endif
