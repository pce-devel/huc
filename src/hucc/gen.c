/*	File gen.c: 2.1 (83/03/20,16:02:06) */
/*% cc -O -c %
 *
 */

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <ctype.h>
#include "defs.h"
#include "data.h"
#include "code.h"
#include "primary.h"
#include "sym.h"
#include "gen.h"
#include "expr.h"
#include "const.h"
#include "error.h"

/*
 *	return next available internal label number
 *
 */
int getlabel (void)
{
	return (nxtlab++);
}

/*
 *	fetch a static memory cell into the primary register
 */
void getmem (SYMBOL *sym)
{
	char *data;

	if ((sym->ident != POINTER) && (sym->type == CCHAR || sym->type == CUCHAR)) {
		int op = I_LDB;
		if (sym->type & CUNSIGNED)
			op = I_LDUB;
		if ((sym->storage & ~WRITTEN) == LSTATIC)
			out_ins(op, T_LABEL, glint(sym));
		else
			out_ins(op, T_SYMBOL, (intptr_t)sym);
	}
	else {
		if ((sym->storage & ~WRITTEN) == LSTATIC)
			out_ins(I_LDW, T_LABEL, glint(sym));
		else if ((sym->storage & ~WRITTEN) == CONST && (data = get_const(sym)))
			out_ins(I_LDWI, T_LITERAL, (intptr_t)data);
		else
			out_ins(I_LDW, T_SYMBOL, (intptr_t)sym);
	}
}

/*
 *	fetch the address of the specified symbol into the primary register
 *
 */
void getloc (SYMBOL *sym)
{
	int value;

	if ((sym->storage & ~WRITTEN) == LSTATIC)
		out_ins(I_LDWI, T_LABEL, glint(sym));
	else {
		value = glint(sym);
		if (norecurse && value < 0) {
			/* XXX: bit of a memory leak, but whatever... */
			SYMBOL * locsym = copysym(sym);
			if (NAMEALLOC <=
				sprintf(locsym->name, "_%s_lend-%ld", current_fn, (long) -value))
				error("norecurse local name too long");
			out_ins(I_LDWI, T_SYMBOL, (intptr_t)locsym);
		}
		else {
			value -= stkp;
			out_ins_sym(X_LEA_S, T_STACK, value, sym);
		}
	}
}

/*
 *	store the primary register into the specified static memory cell
 *
 */
void putmem (SYMBOL *sym)
{
	int code;

	/* XXX: What about 1-byte structs? */
	if ((sym->ident != POINTER) & (sym->type == CCHAR || sym->type == CUCHAR))
		code = I_STB;
	else
		code = I_STW;

	if ((sym->storage & ~WRITTEN) == LSTATIC)
		out_ins(code, T_LABEL, glint(sym));
	else
		out_ins(code, T_SYMBOL, (intptr_t)sym);
}

/*
 *	store the specified object type in the primary register
 *	at the address on the top of the stack
 *
 */
void putstk (char typeobj)
{
	if (typeobj == CCHAR || typeobj == CUCHAR)
		out_ins(I_STBPS, 0, 0);
	else
		out_ins(I_STWPS, 0, 0);
	stkp = stkp + INTSIZE;
}

/*
 *	fetch the specified object type indirect through the primary
 *	register into the primary register
 *
 */
void indirect (char typeobj)
{
	out_ins(I_STW, T_PTR, 0);
	if (typeobj == CCHAR)
		out_ins(I_LDBP, T_PTR, 0);
	else if (typeobj == CUCHAR)
		out_ins(I_LDUBP, T_PTR, 0);
	else
		out_ins(I_LDWP, T_PTR, 0);
}

void farpeek (SYMBOL *ptr)
{
	if (ptr->type == CCHAR)
		out_ins(I_FGETB, T_SYMBOL, (intptr_t)ptr);
	else if (ptr->type == CUCHAR)
		out_ins(I_FGETUB, T_SYMBOL, (intptr_t)ptr);
	else
		out_ins(I_FGETW, T_SYMBOL, (intptr_t)ptr);
}

/*
 *	print partial instruction to get an immediate value into
 *	the primary register
 *
 */
void immed (int type, intptr_t data)
{
	if (type == T_VALUE && (data < -32768 || data > 65535))
		warning(W_GENERAL, "large integer truncated");
	out_ins(I_LDWI, type, data);
}

/*
 *	push the primary register onto the stack
 *
 */
void gpush (void)
{
	out_ins(I_PUSHW, T_VALUE, INTSIZE);
	stkp = stkp - INTSIZE;
}

/*
 *	push the primary register onto the stack
 *
 */
void gpusharg (int size)
{
	out_ins(I_PUSHW, T_SIZE, size);
	stkp = stkp - size;
}

/*
 *	pop the top of the stack into the secondary register
 *
 */
void gpop (void)
{
	out_ins(I_POPW, 0, 0);
	stkp = stkp + INTSIZE;
}

/*
 *	call the specified subroutine name
 *
 */
void gcall (char *sname, int nargs)
{
	out_ins_ex(I_CALL, T_LITERAL, (intptr_t)sname, T_VALUE, nargs);
}

/*
 *	call the specified macro name
 *
 */
void gmacro (char *sname, int nargs)
{
	out_ins_ex(I_MACRO, T_LITERAL, (intptr_t)sname, T_VALUE, nargs);
}

/*
 *         generate a bank pseudo instruction
 *
 */
void gbank (unsigned char bank, unsigned short offset)
{
	out_ins(I_BANK, T_VALUE, bank);
	out_ins(I_OFFSET, T_VALUE, offset);
}

/*
 *	jump to specified internal label number
 *
 */
void jump (int label)
{
	out_ins(I_BRA, T_LABEL, label);
}

/*
 *	test the primary register and jump if false to label
 *
 */
void testjump (int label, int ft)
{
	out_ins(I_TSTW, 0, 0);
	if (ft)
		out_ins(I_BTRUE, T_LABEL, label);
	else
		out_ins(I_BFALSE, T_LABEL, label);
}

/*
 *	modify the stack pointer to the new value indicated
 *      Is it there that we decrease the value of the stack to add local vars ?
 */
int modstk (int newstkp)
{
	int k;

//	k = galign(newstkp - stkp);
	k = newstkp - stkp;
	if (k) {
		gtext();
		out_ins(I_MODSP, T_STACK, k);
	}
	return (newstkp);
}

/*
 *	multiply the primary register by INTSIZE
 */
void gaslint (void)
{
	out_ins(I_ASLW, 0, 0);
}

/*
 *	divide the primary register by INTSIZE
 */
void gasrint (void)
{
	out_ins(I_ASRW, 0, 0);
}

/*
 *	Case jump instruction
 */
void gjcase (void)
{
	out_ins(I_JMP, T_LIB, (intptr_t)"__case");
}

/*
 *	add the primary and secondary registers
 *	if lval2 is int pointer and lval is int, scale lval
 */
void gadd (LVALUE *lval, LVALUE *lval2)
{
	/* XXX: isn't this done in expr.c already? */
	if (dbltest(lval2, lval))
		out_ins(I_ASLWS, 0, 0);
	if (lval && lval2 && is_byte(lval) && is_byte(lval2))
		out_ins(I_ADDBS, 0, 0);
	else
		out_ins(I_ADDWS, 0, 0);
	stkp = stkp + INTSIZE;
}

/*
 *	subtract the primary register from the secondary
 *
 */
void gsub (void)
{
	out_ins(I_SUBWS, 0, 0);
	stkp = stkp + INTSIZE;
}

/*
 *	multiply the primary and secondary registers
 *	(result in primary)
 *
 */
void gmult (int is_unsigned)
{
	if (is_unsigned)
		out_ins(I_JSR, T_LIB, (intptr_t)"umul");
	else
		out_ins(I_JSR, T_LIB, (intptr_t)"smul");
	stkp = stkp + INTSIZE;
}

void gmult_imm (int value)
{
	out_ins(I_MULWI, T_VALUE, (intptr_t)value);
}

/*
 *	divide the secondary register by the primary
 *	(quotient in primary, remainder in secondary)
 *
 */
void gdiv (int is_unsigned)
{
	if (is_unsigned)
		out_ins(I_JSR, T_LIB, (intptr_t)"udiv");
	else
		out_ins(I_JSR, T_LIB, (intptr_t)"sdiv");
	stkp = stkp + INTSIZE;
}

void gdiv_imm (int value)
{
	gpush();
	immed(T_VALUE, value);
	gdiv(1);
}

/*
 *	compute the remainder (mod) of the secondary register
 *	divided by the primary register
 *	(remainder in primary, quotient in secondary)
 *
 */
void gmod (int is_unsigned)
{
	if (is_unsigned)
		out_ins(I_JSR, T_LIB, (intptr_t)"umod");
	else
		out_ins(I_JSR, T_LIB, (intptr_t)"smod");
	stkp = stkp + INTSIZE;
}

/*
 *	inclusive 'or' the primary and secondary registers
 *
 */
void gor (void)
{
	out_ins(I_ORWS, 0, 0);
	stkp = stkp + INTSIZE;
}

/*
 *	exclusive 'or' the primary and secondary registers
 *
 */
void gxor (void)
{
	out_ins(I_EORWS, 0, 0);
	stkp = stkp + INTSIZE;
}

/*
 *	'and' the primary and secondary registers
 *
 */
void gand (void)
{
	out_ins(I_ANDWS, 0, 0);
	stkp = stkp + INTSIZE;
}

/*
 *	arithmetic shift right the secondary register the number of
 *	times in the primary register
 *	(results in primary register)
 *
 */
void gasr (int is_unsigned)
{
	if (is_unsigned)
		out_ins(I_JSR, T_LIB, (intptr_t)"lsrw");
	else
		out_ins(I_JSR, T_LIB, (intptr_t)"asrw");
	stkp = stkp + INTSIZE;
}

/*
 *	arithmetic shift left the secondary register the number of
 *	times in the primary register
 *	(results in primary register)
 *
 */
void gasl (void)
{
	out_ins(I_JSR, T_LIB, (intptr_t)"aslw");
	stkp = stkp + INTSIZE;
}

/*
 *	two's complement of primary register
 *
 */
void gneg (void)
{
	out_ins(I_NEGW, 0, 0);
}

/*
 *	one's complement of primary register
 *
 */
void gcom (void)
{
	out_ins(I_COMW, 0, 0);
}

/*
 *	convert primary register into logical value
 *
 */
void gbool (void)
{
	out_ins(I_TSTW, 0, 0);
}

/*
 *	logical complement of primary register
 *
 */
void glneg (void)
{
	out_ins(I_NOTW, 0, 0);
}

/*
 *	increment the primary register by 1 if char, INTSIZE if int
 */
void ginc (LVALUE *lval)
{
	SYMBOL *sym = lval->symbol;

	if (lval->ptr_type == CINT || lval->ptr_type == CUINT ||
	    (sym && (sym->ptr_order > 1 || (sym->ident == ARRAY && sym->ptr_order > 0))))
		out_ins(I_ADDWI, T_VALUE, 2);
	else if (lval->ptr_type == CSTRUCT)
		out_ins(I_ADDWI, T_VALUE, lval->tagsym->size);
	else
		out_ins(I_ADDWI, T_VALUE, 1);
}

/*
 *	decrement the primary register by one if char, INTSIZE if int
 */
void gdec (LVALUE *lval)
{
	SYMBOL *sym = lval->symbol;

	if (lval->ptr_type == CINT || lval->ptr_type == CUINT ||
	    (sym && (sym->ptr_order > 1 || (sym->ident == ARRAY && sym->ptr_order > 0))))
		out_ins(I_SUBWI, T_VALUE, 2);
	else if (lval->ptr_type == CSTRUCT)
		out_ins(I_SUBWI, T_VALUE, lval->tagsym->size);
	else
		out_ins(I_SUBWI, T_VALUE, 1);
}

/*
 *	following are the conditional operators.
 *	they compare the secondary register against the primary register
 *	and put a literl 1 in the primary if the condition is true,
 *	otherwise they clear the primary register
 *
 */

/*
 *	equal
 *
 */
void geq (int is_byte)
{
	if (is_byte)
		out_ins(I_CMPW, T_LIB, (intptr_t)"eq_b");
	else
		out_ins(I_CMPW, T_LIB, (intptr_t)"eq_w");
	stkp = stkp + INTSIZE;
}

/*
 *	not equal
 *
 */
void gne (int is_byte)
{
	if (is_byte)
		out_ins(I_CMPW, T_LIB, (intptr_t)"ne_b");
	else
		out_ins(I_CMPW, T_LIB, (intptr_t)"ne_w");
	stkp = stkp + INTSIZE;
}

/*
 *	less than (signed)
 *
 */
void glt (int is_byte)
{
	if (is_byte)
		out_ins(I_CMPW, T_LIB, (intptr_t)"lt_sb");
	else
		out_ins(I_CMPW, T_LIB, (intptr_t)"lt_sw");
	stkp = stkp + INTSIZE;
}

/*
 *	less than or equal (signed)
 *
 */
void gle (int is_byte)
{
	if (is_byte)
		out_ins(I_CMPW, T_LIB, (intptr_t)"le_sb");
	else
		out_ins(I_CMPW, T_LIB, (intptr_t)"le_sw");
	stkp = stkp + INTSIZE;
}

/*
 *	greater than (signed)
 *
 */
void ggt (int is_byte)
{
	if (is_byte)
		out_ins(I_CMPW, T_LIB, (intptr_t)"gt_sb");
	else
		out_ins(I_CMPW, T_LIB, (intptr_t)"gt_sw");
	stkp = stkp + INTSIZE;
}

/*
 *	greater than or equal (signed)
 *
 */
void gge (int is_byte)
{
	if (is_byte)
		out_ins(I_CMPW, T_LIB, (intptr_t)"ge_sb");
	else
		out_ins(I_CMPW, T_LIB, (intptr_t)"ge_sw");
	stkp = stkp + INTSIZE;
}

/*
 *	less than (unsigned)
 *
 */
void gult (int is_byte)
{
	if (is_byte)
		out_ins(I_CMPW, T_LIB, (intptr_t)"lt_ub");
	else
		out_ins(I_CMPW, T_LIB, (intptr_t)"lt_uw");
	stkp = stkp + INTSIZE;
}

/*
 *	less than or equal (unsigned)
 *
 */
void gule (int is_byte)
{
	if (is_byte)
		out_ins(I_CMPW, T_LIB, (intptr_t)"le_ub");
	else
		out_ins(I_CMPW, T_LIB, (intptr_t)"le_uw");
	stkp = stkp + INTSIZE;
}

/*
 *	greater than (unsigned)
 *
 */
void gugt (int is_byte)
{
	if (is_byte)
		out_ins(I_CMPW, T_LIB, (intptr_t)"gt_ub");
	else
		out_ins(I_CMPW, T_LIB, (intptr_t)"gt_uw");
	stkp = stkp + INTSIZE;
}

/*
 *	greater than or equal (unsigned)
 *
 */
void guge (int is_byte)
{
	if (is_byte)
		out_ins(I_CMPW, T_LIB, (intptr_t)"ge_ub");
	else
		out_ins(I_CMPW, T_LIB, (intptr_t)"ge_uw");
	stkp = stkp + INTSIZE;
}

void scale_const (int type, int otag, int *size)
{
	switch (type) {
	case CINT:
	case CUINT:
		*size += *size;
		break;
	case CSTRUCT:
		*size *= tag_table[otag].size;
		break;
	default:
		break;
	}
}

void gcast (int type)
{
	switch (type) {
	case CCHAR:
		out_ins(I_EXTW, 0, 0);
		break;
	case CUCHAR:
		out_ins(I_EXTUW, 0, 0);
		break;
	case CINT:
	case CUINT:
	case CVOID:
		break;
	default:
		abort();
	}
	;
}

void gsei (void)
{
	out_ins(I_SEI, 0, 0);
}
void gcli (void)
{
	out_ins(I_CLI, 0, 0);
}
