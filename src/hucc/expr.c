/*	File expr.c: 2.2 (83/06/21,11:24:26) */
/*% cc -O -c %
 *
 */

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "defs.h"
#include "data.h"
#include "code.h"
#include "error.h"
#include "expr.h"
#include "function.h"
#include "gen.h"
#include "io.h"
#include "lex.h"
#include "primary.h"
#include "sym.h"

/*
 *	lval->symbol - symbol table address, else 0 for constant
 *	lval->indirect - type indirect object to fetch, else 0 for static object
 *	lval->ptr_type - type of pointer or array, else 0
 */
void expression (int comma)
{
	LVALUE lval[1] = {{0}};

	expression_ex(lval, comma, NO);
}

int expression_ex (LVALUE *lval, int comma, int norval)
{
	int k;

	do {
		if ((k = heir1(lval, comma)) && !norval)
			rvalue(lval);
		if (!comma)
			return (k);
		blanks();
		if (ch() == ',')
			gfence();
	} while (match(","));

	return (k);
}

/*
 * C11 section 6.3.1.8 "Usual arithmetic conversions"
 *
 * .....   sbyte   sword   ubyte   uword
 * ----------------------------------------
 * sbyte | sbyte   sword   sword   uword
 * sword | sword   sword   sword   uword
 * ubyte | sword   sword   ubyte   uword
 * uword | uword   uword   uword   uword
 */

static int is_unsigned (LVALUE *lval)
{
	/* C promotes operations on an unsigned char
	   to a signed int, not an unsigned int! */

	/* if using a pointer, then the pointer itself is CUINT */
	if (lval->ptr_type)
		return (1);

	/* handle result type or typecast (which overrides symbol) */
	if (lval->val_type)
		return (lval->val_type == CUINT);

	/* handle a symbol reference */
	if (lval->symbol && lval->symbol->sym_type == CUINT)
		return (1);

	return (0);
}

static int is_ptrptr (LVALUE *lval)
{
	SYMBOL *s = lval->symbol;

	return (s && (s->ptr_order > 1 || (s->identity == ARRAY && s->ptr_order > 0)));
}

static void gen_scale_right (LVALUE *lval, LVALUE *lval2)
{
	if (dbltest(lval, lval2)) {
		if (lval->tagsym) {
			TAG_SYMBOL *tag = (TAG_SYMBOL *)(lval->tagsym);
			if (tag->size == 2)
				gaslint();
			else if (tag->size > 1)
				gmult_imm(tag->size);
		}
		else
			gaslint();
	}
}

static void void_value_error (LVALUE *lval)
{
	error("function is declared VOID and does not return a value");
	/* suppress further error messages about this return value */
	lval->val_type = 0;
}

/*
 * assignment operators
 * @param lval
 * @return
 */
int heir1 (LVALUE *lval, int comma)
{
	int k;
	LVALUE lval2[1] = {{0}};
	char fc;

	k = heir1a(lval, comma);
	if (match("=")) {
		if (k == 0) {
			needlval();
			return (0);
		}
		if (lval->indirect)
			gpush();
		if (heir1(lval2, comma))
			rvalue(lval2);
		if (lval2->val_type == CVOID)
			void_value_error(lval2);
		store(lval);
		return (0);
	}
	else {
		fc = ch();
		if (match("-=") ||
		    match("+=") ||
		    match("*=") ||
		    match("/=") ||
		    match("%=") ||
		    match(">>=") ||
		    match("<<=") ||
		    match("&=") ||
		    match("^=") ||
		    match("|=")) {
			if (k == 0) {
				needlval();
				return (0);
			}
			if (lval->indirect)
				gpush();
			rvalue(lval);
			gpush();
			if (heir1(lval2, comma))
				rvalue(lval2);
			if (lval2->val_type == CVOID)
				void_value_error(lval2);
			switch (fc) {
			case '-':       {
				gen_scale_right(lval, lval2);
				gsub();
				result(lval, lval2);
				break;
			}
			case '+':       {
				gen_scale_right(lval, lval2);
				gadd(lval, lval2);
				result(lval, lval2);
				break;
			}
			case '*':       gmult(is_unsigned(lval) || is_unsigned(lval2)); break;
			case '/':       gdiv(is_unsigned(lval) || is_unsigned(lval2)); break;
			case '%':       gmod(is_unsigned(lval) || is_unsigned(lval2)); break;
			case '>':       gasr(is_unsigned(lval)); break;
			case '<':       gasl(); break;
			case '&':       gand(); break;
			case '^':       gxor(); break;
			case '|':       gor(); break;
			}
			store(lval);
			return (0);
		}
		else
			return (k);
	}
}

/*
 * processes ? : expression
 * @param lval
 * @return 0 or 1, fetch or no fetch
 */
int heir1a (LVALUE *lval, int comma)
{
	int k, lab1, lab2;

	k = heir1b(lval, comma);
	blanks();
	if (ch() != '?')
		return (k);

	if (k)
		rvalue(lval);
	if (lval->val_type == CVOID)
		void_value_error(lval);
	FOREVER
	if (match("?")) {
		testjump(lab1 = getlabel(), FALSE);
		if (heir1b(lval, comma))
			rvalue(lval);
		if (lval->val_type == CVOID)
			void_value_error(lval);
		jump(lab2 = getlabel());
		gnlabel(lab1);
		blanks();
		if (!match(":")) {
			error("missing colon");
			return (0);
		}
		if (heir1b(lval, comma))
			rvalue(lval);
		if (lval->val_type == CVOID)
			void_value_error(lval);
		gnlabel(lab2);
	}
	else
		return (0);
}

/*
 * processes logical or ||
 * @param lval
 * @return 0 or 1, fetch or no fetch
 */
int heir1b (LVALUE *lval, int comma)
{
	int k, lab;
	LVALUE lval2[1] = {{0}};

	k = heir1c(lval, comma);
	blanks();
	if (!sstreq("||"))
		return (k);

	if (k)
		rvalue(lval);
	if (lval->val_type == CVOID)
		void_value_error(lval);
	FOREVER
	if (match("||")) {
		testjump(lab = getlabel(), TRUE);
		if (heir1c(lval2, comma))
			rvalue(lval2);
		if (lval2->val_type == CVOID)
			void_value_error(lval2);
		gtest();
		gnlabel(lab);
		gbool();
	}
	else
		return (0);
}

/*
 * processes logical and &&
 * @param lval
 * @return 0 or 1, fetch or no fetch
 */
int heir1c (LVALUE *lval, int comma)
{
	int k, lab;
	LVALUE lval2[1] = {{0}};

	k = heir2(lval, comma);
	blanks();
	if (!sstreq("&&"))
		return (k);

	if (k)
		rvalue(lval);
	if (lval->val_type == CVOID)
		void_value_error(lval);
	FOREVER
	if (match("&&")) {
		testjump(lab = getlabel(), FALSE);
		if (heir2(lval2, comma))
			rvalue(lval2);
		if (lval2->val_type == CVOID)
			void_value_error(lval2);
		gtest();
		gnlabel(lab);
		gbool();
	}
	else
		return (0);
}

/*
 * processes bitwise or |
 * @param lval
 * @return 0 or 1, fetch or no fetch
 */
int heir2 (LVALUE *lval, int comma)
{
	int k;
	LVALUE lval2[1] = {{0}};

	k = heir3(lval, comma);
	blanks();
	if ((ch() != '|') || (nch() == '|') || (nch() == '='))
		return (k);

	if (k)
		rvalue(lval);
	if (lval->val_type == CVOID)
		void_value_error(lval);
	FOREVER {
		if ((ch() == '|') && (nch() != '|') && (nch() != '=')) {
			inbyte();
			gpush();
			if (heir3(lval2, comma))
				rvalue(lval2);
			if (lval2->val_type == CVOID)
				void_value_error(lval2);
			gor();
			blanks();
		}
		else
			return (0);
	}
}

/*
 * processes bitwise exclusive or
 * @param lval
 * @return 0 or 1, fetch or no fetch
 */
int heir3 (LVALUE *lval, int comma)
{
	int k;
	LVALUE lval2[1] = {{0}};

	k = heir4(lval, comma);
	blanks();
	if ((ch() != '^') || (nch() == '='))
		return (k);

	if (k)
		rvalue(lval);
	if (lval->val_type == CVOID)
		void_value_error(lval);
	FOREVER {
		if ((ch() == '^') && (nch() != '=')) {
			inbyte();
			gpush();
			if (heir4(lval2, comma))
				rvalue(lval2);
			if (lval2->val_type == CVOID)
				void_value_error(lval2);
			gxor();
			blanks();
		}
		else
			return (0);
	}
}

/*
 * processes bitwise and &
 * @param lval
 * @return 0 or 1, fetch or no fetch
 */
int heir4 (LVALUE *lval, int comma)
{
	int k;
	LVALUE lval2[1] = {{0}};

	k = heir5(lval, comma);
	blanks();
	if ((ch() != '&') || (nch() == '|') || (nch() == '='))
		return (k);

	if (k)
		rvalue(lval);
	if (lval->val_type == CVOID)
		void_value_error(lval);
	FOREVER {
		if ((ch() == '&') && (nch() != '&') && (nch() != '=')) {
			inbyte();
			gpush();
			if (heir5(lval2, comma))
				rvalue(lval2);
			if (lval2->val_type == CVOID)
				void_value_error(lval2);
			gand();
			blanks();
		}
		else
			return (0);
	}
}

/*
 * processes equal and not equal operators
 * @param lval
 * @return 0 or 1, fetch or no fetch
 */
int heir5 (LVALUE *lval, int comma)
{
	int k;
	LVALUE lval2[1] = {{0}};

	k = heir6(lval, comma);
	blanks();
	if ((!sstreq("==")) &&
	    (!sstreq("!=")))
		return (k);

	if (k)
		rvalue(lval);
	if (lval->val_type == CVOID)
		void_value_error(lval);
	FOREVER {
		if (match("==")) {
			gpush();
			if (heir6(lval2, comma))
				rvalue(lval2);
			if (lval2->val_type == CVOID)
				void_value_error(lval2);
			geq();
		}
		else if (match("!=")) {
			gpush();
			if (heir6(lval2, comma))
				rvalue(lval2);
			if (lval2->val_type == CVOID)
				void_value_error(lval2);
			gne();
		}
		else
			return (0);
	}
}

/*
 * comparison operators
 * @param lval
 * @return 0 or 1, fetch or no fetch
 */
int heir6 (LVALUE *lval, int comma)
{
	int k;
	LVALUE lval2[1] = {{0}};

	k = heir7(lval, comma);
	blanks();
	if (!sstreq("<") &&
	    !sstreq("<=") &&
	    !sstreq(">=") &&
	    !sstreq(">"))
		return (k);

	if (sstreq("<<") || sstreq(">>"))
		return (k);

	if (k)
		rvalue(lval);
	if (lval->val_type == CVOID)
		void_value_error(lval);
	FOREVER {
		if (match("<=")) {
			gpush();
			if (heir7(lval2, comma))
				rvalue(lval2);
			if (lval2->val_type == CVOID)
				void_value_error(lval2);
			if (lval->ptr_type || lval2->ptr_type ||
			    is_unsigned(lval) ||
			    is_unsigned(lval2)
			    ) {
				gule();
				continue;
			}
			gle();
		}
		else if (match(">=")) {
			gpush();
			if (heir7(lval2, comma))
				rvalue(lval2);
			if (lval2->val_type == CVOID)
				void_value_error(lval2);
			if (lval->ptr_type || lval2->ptr_type ||
			    is_unsigned(lval) ||
			    is_unsigned(lval2)
			    ) {
				guge();
				continue;
			}
			gge();
		}
		else if ((sstreq("<")) &&
			 !sstreq("<<")) {
			inbyte();
			gpush();
			if (heir7(lval2, comma))
				rvalue(lval2);
			if (lval2->val_type == CVOID)
				void_value_error(lval2);
			if (lval->ptr_type || lval2->ptr_type ||
			    is_unsigned(lval) ||
			    is_unsigned(lval2)
			    ) {
				gult();
				continue;
			}
			glt();
		}
		else if ((sstreq(">")) &&
			 !sstreq(">>")) {
			inbyte();
			gpush();
			if (heir7(lval2, comma))
				rvalue(lval2);
			if (lval2->val_type == CVOID)
				void_value_error(lval2);
			if (lval->ptr_type || lval2->ptr_type ||
			    is_unsigned(lval) ||
			    is_unsigned(lval2)
			    ) {
				gugt();
				continue;
			}
			ggt();
		}
		else
			return (0);

		blanks();
	}
}

/*
 * bitwise left, right shift
 * @param lval
 * @return 0 or 1, fetch or no fetch
 */
int heir7 (LVALUE *lval, int comma)
{
	int k;
	LVALUE lval2[1] = {{0}};

	k = heir8(lval, comma);
	blanks();
	if ((!sstreq(">>") && !sstreq("<<")) ||
	    sstreq(">>=") || sstreq("<<="))
		return (k);

	if (k)
		rvalue(lval);
	if (lval->val_type == CVOID)
		void_value_error(lval);
	FOREVER {
		if (sstreq(">>") && !sstreq(">>=")) {
			inbyte(); inbyte();
			gpush();
			if (heir8(lval2, comma))
				rvalue(lval2);
			if (lval2->val_type == CVOID)
				void_value_error(lval2);
			gasr(is_unsigned(lval));
		}
		else if (sstreq("<<") && !sstreq("<<=")) {
			inbyte(); inbyte();
			gpush();
			if (heir8(lval2, comma))
				rvalue(lval2);
			if (lval2->val_type == CVOID)
				void_value_error(lval2);
			gasl();
		}
		else
			return (0);

		blanks();
	}
}

/*
 * addition, subtraction
 * @param lval
 * @return 0 or 1, fetch or no fetch
 */
int heir8 (LVALUE *lval, int comma)
{
	int k;
	LVALUE lval2[1] = {{0}};

	k = heir9(lval, comma);
	blanks();
	if (((ch() != '+') && (ch() != '-')) || (nch() == '='))
		return (k);

	if (k)
		rvalue(lval);
	if (lval->val_type == CVOID)
		void_value_error(lval);
	FOREVER {
		if (match("+")) {
			gpush();
			if (heir9(lval2, comma))
				rvalue(lval2);
			if (lval2->val_type == CVOID)
				void_value_error(lval2);
			/* if left is pointer and right is int, scale right */
			gen_scale_right(lval, lval2);
			/* will scale left if right int pointer and left int */
			gadd(lval, lval2);
			result(lval, lval2);
		}
		else if (match("-")) {
			gpush();
			if (heir9(lval2, comma))
				rvalue(lval2);
			if (lval2->val_type == CVOID)
				void_value_error(lval2);
			/* if dbl, can only be: pointer - int, or
			                        pointer - pointer, thus,
			        in first case, int is scaled up,
			        in second, result is scaled down. */
			gen_scale_right(lval, lval2);
			gsub();
			/* if both pointers, scale result */
			if ((lval->ptr_type == CINT || lval->ptr_type == CUINT || is_ptrptr(lval)) &&
			    (lval2->ptr_type == CINT || lval2->ptr_type == CUINT || is_ptrptr(lval2)))
				gasrint();	/* divide by intsize */
			else if (lval->ptr_type == CSTRUCT && lval2->ptr_type == CSTRUCT) {
				TAG_SYMBOL *tag = lval->tagsym;
				if (tag->size == 2)
					gasrint();
				else if (tag->size > 1)
					gdiv_imm(tag->size);
			}
			result(lval, lval2);
		}
		else
			return (0);
	}
}

/*
 * multiplication, division, modulus
 * @param lval
 * @return 0 or 1, fetch or no fetch
 */
int heir9 (LVALUE *lval, int comma)
{
	int k;
	LVALUE lval2[1] = {{0}};

	k = heir10(lval, comma);
	blanks();
	if (((ch() != '*') && (ch() != '/') &&
	     (ch() != '%')) || (nch() == '='))
		return (k);

	if (k)
		rvalue(lval);
	if (lval->val_type == CVOID)
		void_value_error(lval);
	FOREVER {
		if (match("*")) {
			gpush();
			if (heir10(lval2, comma))
				rvalue(lval2);
			if (lval2->val_type == CVOID)
				void_value_error(lval2);
			gmult(is_unsigned(lval) || is_unsigned(lval2));
		}
		else if (match("/")) {
			gpush();
			if (heir10(lval2, comma))
				rvalue(lval2);
			if (lval2->val_type == CVOID)
				void_value_error(lval2);
			gdiv(is_unsigned(lval) || is_unsigned(lval2));
		}
		else if (match("%")) {
			gpush();
			if (heir10(lval2, comma))
				rvalue(lval2);
			if (lval2->val_type == CVOID)
				void_value_error(lval2);
			gmod(is_unsigned(lval) || is_unsigned(lval2));
		}
		else
			return (0);
	}
}

/*
 * increment, decrement, unary operators and pointer dereferencing
 * @param lval
 * @return 0 or 1, fetch or no fetch
 */
int heir10 (LVALUE *lval, int comma)
{
	int k;
	SYMBOL *ptr;

	if (match("++")) {
		indflg = 0;
		if ((k = heir10(lval, comma)) == 0) {
			if (lval->val_type == CVOID)
				void_value_error(lval);
			else
				needlval();
			return (0);
		}
		if (lval->indirect)
			gpush();
		rvalue(lval);
		ginc(lval);
		store(lval);
		return (0);
	}
	else if (match("--")) {
		indflg = 0;
		if ((k = heir10(lval, comma)) == 0) {
			if (lval->val_type == CVOID)
				void_value_error(lval);
			else
				needlval();
			return (0);
		}
		if (lval->indirect)
			gpush();
		rvalue(lval);
		gdec(lval);
		store(lval);
		return (0);
	}
	else if (match("-")) {
		indflg = 0;
		k = heir10(lval, comma);
		if (k)
			rvalue(lval);
		if (lval->val_type == CVOID)
			void_value_error(lval);
		gneg();
		return (0);
	}
	else if (match("~")) {
		indflg = 0;
		k = heir10(lval, comma);
		if (k)
			rvalue(lval);
		if (lval->val_type == CVOID)
			void_value_error(lval);
		gcom();
		return (0);
	}
	else if (match("!")) {
		indflg = 0;
		k = heir10(lval, comma);
		if (k)
			rvalue(lval);
		if (lval->val_type == CVOID)
			void_value_error(lval);
		gnot();
		return (0);
	}
	else if (ch() == '*' && nch() != '=') {
		inbyte();
		indflg = 1;
		k = heir10(lval, comma);
		indflg = 0;
		ptr = lval->symbol;
		if (ptr && ptr->funcptr_type && lval->ptr_order == 0) {
			/* ignore optional dereference of a function pointer */
			return (k);
		}
		if (k)
			rvalue(lval);
		if (lval->val_type == CVOID)
			void_value_error(lval);
		if (lval->ptr_order > 1) {
			lval->indirect = CUINT;
			lval->ptr_order--;
		} else {
			if (lval->ptr_type == 0)
				error("not a pointer");
			lval->indirect = lval->ptr_type;
			lval->ptr_type = 0; /* flag as not pointer or array */
			lval->ptr_order = 0;
		}
		return (1);
	}
	else if (ch() == '&' && nch() != '&' && nch() != '=') {
		indflg = 0;
		inbyte();
		k = heir10(lval, comma);
		if (lval->val_type == CVOID) {
			void_value_error(lval);
			return (0);
		}
		if (k == 0) {
			/* allow "&function" in function pointer assignment */
			if (lval->symbol == NULL || lval->symbol->identity != FUNCTION)
				error("illegal address");
			return (0);
		}
		if (lval->symbol) {
			ptr = lval->symbol;
			lval->ptr_type = ptr->sym_type;
			lval->ptr_order = ptr->ptr_order;
		}
		lval->ptr_order++;
		if (lval->indirect)
			return (0);

		/* global and non-array */
		ptr = lval->symbol;
		immed(T_SYMBOL, (intptr_t)ptr);
		lval->indirect = ptr->sym_type;
		return (0);
	}
	else {
		k = heir11(lval, comma);
		ptr = lval->symbol;
		if (match("++")) {
			if (k == 0) {
				if (lval->val_type == CVOID)
					void_value_error(lval);
				else
					needlval();
				return (0);
			}
			if (lval->indirect)
				gpush();
			rvalue(lval);
			ginc(lval);
			store(lval);
			gdec(lval);
			return (0);
		}
		else if (match("--")) {
			if (k == 0) {
				if (lval->val_type == CVOID)
					void_value_error(lval);
				else
					needlval();
				return (0);
			}
			if (lval->indirect)
				gpush();
			rvalue(lval);
			gdec(lval);
			store(lval);
			ginc(lval);
			return (0);
		}
		else
			return (k);
	}
}

/*
 * array subscripting
 * function calls
 * structure members
 * @param lval
 * @return 0 or 1, fetch or no fetch
 */
int heir11 (LVALUE *lval, int comma)
{
	int direct, k;
	SYMBOL *ptr;
	bool deferred = false;
	char sname[NAMESIZE];

	k = primary(lval, comma, &deferred);
	ptr = lval->symbol;
	blanks();
	for (;;) {
		if (match("[")) {
			if (lval->val_type == CVOID)
				void_value_error(lval);
			if (ptr == 0) {
				if (lval->ptr_type && lval->ptr_order == 1) {
					/* subscription of anonymous array which is currently
					   only supported for a string literal, primarily for
					   the testsuite, such as "921218-1.c". */
					if (lval->ptr_type != ((user_signed_char) ? CCHAR : CUCHAR))
						error("internal error: cannot subscript non-character literals");
					/* Primary contains literal pointer, add subscript. */
					gpush();
					expression(YES);
					needbracket("]");
					gadd(NULL, NULL);
					/* Dereference final pointer. */
					lval->symbol = lval->symbol2 = 0;
					lval->indirect = lval->ptr_type;
					if (lval->ptr_order > 1)
						lval->ptr_order--;
					else {
						lval->ptr_type = 0;
						lval->ptr_order = 0;
					}
					k = 1;
					continue;
				}
				else {
					error("can't subscript");
					junk();
					needbracket("]");
					return (0);
				}
			}
			else if (ptr->identity != POINTER && ptr->identity != ARRAY) {
				error("can't subscript");
				k = 0;
			}
			if (k)
				rvalue(lval);
			if (!deferred && !ptr->far)
				gpush();
			expression(YES);
			needbracket("]");
			if (ptr->sym_type == CINT || ptr->sym_type == CUINT ||
			    lval->ptr_order > ((ptr->identity == ARRAY) ? 0 : 1))
				gaslint();
			else if (ptr->sym_type == CSTRUCT) {
				int size = tag_table[ptr->tagidx].size;
				if (size == 2)
					gaslint();
				else if (size > 1)
					gmult_imm(size);
			}
			if (deferred) {
#if ULI_NORECURSE
				if ((ptr->storage & STORAGE) == AUTO && norecurse && ptr->offset < 0) {
					/* XXX: bit of a memory leak, but whatever... */
					SYMBOL * locsym = copysym(ptr);
					if (NAMEALLOC <=
						sprintf(locsym->name, "_%s_end - %d", current_fn, -ptr->offset))
						error("norecurse local name too long");
					locsym->linked = ptr;
					out_ins(I_ADD_WI, T_SYMBOL, (intptr_t)locsym);
				}
				else
#endif
				if ((ptr->storage & STORAGE) == LSTATIC)
					out_ins(I_ADD_WI, T_SYMBOL, (intptr_t)(ptr->linked));
				else
					out_ins(I_ADD_WI, T_SYMBOL, (intptr_t)ptr);
				deferred = false;
			}
			else
			if (!ptr->far)
				gadd(NULL, NULL);
			if (lval->ptr_order > ((ptr->identity == ARRAY) ? 0 : 1)) {
				lval->indirect = CUINT;
				lval->ptr_order--;
			}
			else {
				lval->indirect = lval->ptr_type;
				lval->ptr_type = 0;
				lval->ptr_order = 0;
				blanks();
				if (ch() == '[') {
					/* force an error in the next subscript attempt */
					lval->symbol = NULL;
				}
			}
			lval->symbol2 = ptr->far ? ptr : NULL;
			k = 1;
		}
		else if (match("(")) {
			if (lval->val_type == CVOID)
				void_value_error(lval);
			if (ptr == 0) {
				error("invalid or unsupported function call");
				junk();
				return (0);
			}
			else
			if (ptr->identity == FUNCTION) {
				callfunction(ptr);
				if (ptr->ptr_order) {
					lval->val_type = 0;
					lval->ptr_type = ptr->sym_type;
					lval->ptr_order = ptr->ptr_order;
				} else {
					lval->val_type = ptr->sym_type;
					lval->ptr_type = 0;
					lval->ptr_order = 0;
				}
				if (ptr->sym_type == CSTRUCT)
					lval->tagsym = &tag_table[ptr->tagidx];
			}
			else {
				if (ptr->funcptr_type == 0 || lval->ptr_order != 0)
					error("not a function pointer");
				if (k)
					rvalue(lval);
				callfunction(ptr);
				if (ptr->funcptr_order) {
					lval->val_type = 0;
					lval->ptr_type = ptr->funcptr_type;
					lval->ptr_order = ptr->funcptr_order;
				} else {
					lval->val_type = ptr->funcptr_type;
					lval->ptr_type = 0;
					lval->ptr_order = 0;
				}
				if (ptr->funcptr_type == CSTRUCT)
					lval->tagsym = &tag_table[ptr->tagidx];
			}
			k = 0;
			lval->symbol = 0;
		}
		else if ((direct = match(".")) || match("->")) {
			if (lval->val_type == CVOID)
				void_value_error(lval);
			if (lval->tagsym == 0) {
				error("can't take member");
				junk();
				return (0);
			}
			if (symname(sname) == 0 ||
			    ((ptr = find_member(lval->tagsym, sname)) == 0)) {
				error("unknown member");
				junk();
				return (0);
			}
			if (k && direct == 0)
				rvalue(lval);
			if (ptr->offset)
				out_ins(I_ADD_WI, T_VALUE, ptr->offset);	// move pointer from struct begin to struct member
			lval->symbol = ptr;
			lval->indirect = ptr->sym_type;		// lval->indirect = lval->val_type = ptr->sym_type
			lval->ptr_type = 0;
			lval->ptr_order = 0;
			lval->tagsym = NULL_TAG;
			if (ptr->sym_type == CSTRUCT)
				lval->tagsym = &tag_table[ptr->tagidx];
			if (ptr->identity == POINTER) {
				lval->indirect = CUINT;
				lval->ptr_type = ptr->sym_type;
				lval->ptr_order = ptr->ptr_order;
				// lval->val_type = CINT;
			}
			if (ptr->identity == ARRAY ||
			    (ptr->sym_type == CSTRUCT && ptr->identity == VARIABLE)) {
				// array or struct
				lval->ptr_type = ptr->sym_type;
				lval->ptr_order = ptr->ptr_order;
				// lval->val_type = CINT;
				k = 0;
			}
			else k = 1;
		}
		else
			return (k);
	}
	if (ptr == 0)
		return (k);

	if (ptr->identity == FUNCTION) {
		immed(T_SYMBOL, (intptr_t)ptr);
		return (0);
	}
	return (k);
}

void store (LVALUE *lval)
{
	if (lval->symbol2) {
		/* far arrays */
		error("const arrays can't be written");
		gpop();
	}
	else {
		/* other */
		if (lval->indirect != 0) {
			if (lval->indirect == CVOID)
				error("cannot dereference a VOID pointer");
			putstk(lval->indirect);
		}
		else {
			if (lval->symbol) {
				putmem(lval->symbol);
			}
			else {
//				/* write to a memory addresses given as an immediate value */
//				out_ins(I_ST_WM, T_VALUE, lval->value);
				error("cannot write to a string constant or literal value");
			}
		}
	}
}

void rvalue (LVALUE *lval)
{
	if ((lval->symbol != 0) && (lval->indirect == 0)) {
		getmem(lval->symbol);
	}
	else {
		if (lval->symbol2 == 0) {
			if (lval->indirect == CVOID)
				error("cannot dereference a VOID pointer");
			indirect(lval->indirect);
		}
		else {
			/* far arrays */
			farpeek(lval->symbol2);
		}
	}
}

void needlval (void)
{
	error("must be lvalue");
}
