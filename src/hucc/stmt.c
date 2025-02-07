/*	File stmt.c: 2.1 (83/03/20,16:02:17) */
/*% cc -O -c %
 *
 */

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include "defs.h"
#include "data.h"
#include "code.h"
#include "enum.h"
#include "error.h"
#include "expr.h"
#include "gen.h"
#include "io.h"
#include "lex.h"
#include "preproc.h"
#include "primary.h"
#include "stmt.h"
#include "sym.h"
#include "while.h"
#include "struct.h"
#include "optimize.h"

/*
 *	statement parser
 *
 *	called whenever syntax requires a statement.  this routine
 *	performs that statement and returns a number telling which one
 *
 *	'func' is true if we require a "function_statement", which
 *	must be compound, and must contain "statement_list" (even if
 *	"declaration_list" is omitted)
 */
int statement (int func)
{
	if ((ch() == 0) & feof(input))
		return (0);

	last_statement = 0;
	if (func) {
		top_level_stkp = 1;	/* uninitialized */
		if (match("{")) {
			compound(YES);
			return (last_statement);
		}
		else
			error("function requires compound statement");
	}
	if (match("{"))
		compound(NO);
	else
		stst();
	return (last_statement);
}

/*
 *	declaration
 */
int stdecl (void)
{
	if (amatch("register", 8))
		doldcls(DEFAUTO);
	else if (amatch("auto", 4))
		doldcls(DEFAUTO);
	else if (amatch("static", 6))
		doldcls(LSTATIC);
	else if (doldcls(AUTO)) ;
	else
		return (NO);

	return (YES);
}

int doldcls (int stclass)
{
	struct type_type t;

	blanks();
	/* we don't do optimizations that would require "volatile" */
	if (match_type(&t, NO, YES)) {
#if ULI_NORECURSE == 0
		if (norecurse && stclass != LSTATIC)
			stclass = LSTATIC | WASAUTO;
#endif
		if (t.type_type == CSTRUCT && t.otag == -1)
			t.otag = define_struct(t.sname, stclass, !!(t.flags & F_STRUCT));
		if (t.type_type == CVOID) {
			blanks();
			if (ch() != '*') {
				error("illegal type \"void\"");
				junk();
				return (0);
			}
		}
		if (t.type_type == CENUM) {
			if (t.otag == -1)
				t.otag = define_enum(t.sname, stclass);
			t.type_type = enum_types[t.otag].base;
		}
		declloc(t.type_type, stclass, t.otag);
	}
	else
		return (0);

	needsemicolon();
	return (1);
}


/*
 *	non-declaration statement
 */
void stst (void)
{
	if (amatch("if", 2)) {
		doif();
		last_statement = STIF;
	}
	else if (amatch("while", 5)) {
		dowhile();
		last_statement = STWHILE;
	}
	else if (amatch("switch", 6)) {
		doswitch();
		last_statement = STSWITCH;
	}
	else if (amatch("do", 2)) {
		dodo();
		needsemicolon();
		last_statement = STDO;
	}
	else if (amatch("for", 3)) {
		dofor();
		last_statement = STFOR;
	}
	else if (amatch("return", 6)) {
		doreturn();
		needsemicolon();
		last_statement = STRETURN;
	}
	else if (amatch("break", 5)) {
		dobreak();
		needsemicolon();
		last_statement = STBREAK;
	}
	else if (amatch("continue", 8)) {
		docont();
		needsemicolon();
		last_statement = STCONT;
	}
	else if (amatch("goto", 4)) {
		dogoto();
		needsemicolon();
		last_statement = STGOTO;
	}
	else if (match(";"))
		;
	else if (amatch("case", 4)) {
		docase();
		last_statement = statement(NO);
	}
	else if (amatch("default", 7)) {
		dodefault();
		last_statement = statement(NO);
	}
	else if (match("#asm")) {
		doasm();
		last_statement = STASM;
	}
	else if (match("{"))
		compound(NO);
	else {
		int slptr = lptr;
		char lbl[NAMESIZE];
		if (symname(lbl) && ch() == ':') {
			gch();
			dolabel(lbl);
			last_statement = statement(NO);
		}
		else {
			/* mark the start of an expression */
			out_ins(I_INFO, T_EXPRESSION, 0);
			lptr = slptr;
			expression(YES);
			needsemicolon();
			last_statement = STEXP;
		}
	}
	gfence();
}

/*
 *	compound statement
 *
 *	allow any number of statements to fall between "{" and "}"
 *
 *	'func' is true if we are in a "function_statement", which
 *	must contain "statement_list"
 */
void compound (int func)
{
	int decls;

	decls = YES;
	ncmp++;
	/* remember stack pointer before entering the first compound
	   statement inside a function */
#if ULI_NORECURSE
	if (!func && top_level_stkp == 1 && !norecurse)
#else
	if (!func && top_level_stkp == 1)
#endif
		top_level_stkp = stkp;
	while (!match("}")) {
		if (feof(input))
			return;

		if (decls) {
			if (!stdecl()) {
				decls = NO;
				gtext();
			}
		}
		else
			stst();
	}
	ncmp--;
}

/*
 *	"if" statement
 */
void doif (void)
{
	int fstkp, flab1, flab2;
	int flev;

	flev = locsym_index;
	fstkp = stkp;
	flab1 = getlabel();
	test(flab1, FALSE);
	statement(NO);
	stkp = modstk(fstkp);
	locsym_index = flev;
	if (!amatch("else", 4)) {
		gnlabel(flab1);
		return;
	}
	jump(flab2 = getlabel());
	gnlabel(flab1);
	statement(NO);
	stkp = modstk(fstkp);
	locsym_index = flev;
	gnlabel(flab2);
}

/*
 *	"while" statement
 */
void dowhile (void)
{
	int ws[WS_COUNT];

	ws[WS_LOCSYM_INDEX] = locsym_index;
	ws[WS_STACK_OFFSET] = stkp;
	ws[WS_TYPE] = WS_WHILE;
	ws[WS_TEST_LABEL] = getlabel();
	ws[WS_EXIT_LABEL] = getlabel();
	addwhile(ws);
	gnlabel(ws[WS_TEST_LABEL]);
	test(ws[WS_EXIT_LABEL], FALSE);
	statement(NO);
	jump(ws[WS_TEST_LABEL]);
	gnlabel(ws[WS_EXIT_LABEL]);
	locsym_index = ws[WS_LOCSYM_INDEX];
	stkp = modstk(ws[WS_STACK_OFFSET]);
	delwhile();
}

/*
 *	"do" statement
 */
void dodo (void)
{
	int ws[WS_COUNT];

	ws[WS_LOCSYM_INDEX] = locsym_index;
	ws[WS_STACK_OFFSET] = stkp;
	ws[WS_TYPE] = WS_DO;
	ws[WS_BODY_LABEL] = getlabel();
	ws[WS_TEST_LABEL] = getlabel();
	ws[WS_EXIT_LABEL] = getlabel();
	addwhile(ws);
	gnlabel(ws[WS_BODY_LABEL]);
	statement(NO);
	if (!match("while")) {
		error("missing while");
		return;
	}
	gnlabel(ws[WS_TEST_LABEL]);
	test(ws[WS_BODY_LABEL], TRUE);
	gnlabel(ws[WS_EXIT_LABEL]);
	locsym_index = ws[WS_LOCSYM_INDEX];
	stkp = modstk(ws[WS_STACK_OFFSET]);
	delwhile();
}

/*
 *	"for" statement
 */
void dofor (void)
{
	int ws[WS_COUNT], *pws;

	ws[WS_LOCSYM_INDEX] = locsym_index;
	ws[WS_STACK_OFFSET] = stkp;
	ws[WS_TYPE] = WS_FOR;
	ws[WS_TEST_LABEL] = getlabel();
	ws[WS_INCR_LABEL] = getlabel();
	ws[WS_BODY_LABEL] = getlabel();
	ws[WS_EXIT_LABEL] = getlabel();
	addwhile(ws);
	pws = readwhile();
	needbracket("(");
	if (!match(";")) {
		expression(YES);
		needsemicolon();
		gfence();
	}
	gnlabel((int)pws[WS_TEST_LABEL]);
	if (!match(";")) {
		expression(YES);
		testjump(pws[WS_BODY_LABEL], TRUE);
		jump(pws[WS_EXIT_LABEL]);
		needsemicolon();
	}
	else
		pws[WS_TEST_LABEL] = pws[WS_BODY_LABEL];
	gnlabel(pws[WS_INCR_LABEL]);
	if (!match(")")) {
		expression(YES);
		needbracket(")");
		gfence();
		jump(pws[WS_TEST_LABEL]);
	}
	else
		pws[WS_INCR_LABEL] = pws[WS_TEST_LABEL];
	gnlabel(pws[WS_BODY_LABEL]);
	statement(NO);
	stkp = modstk(pws[WS_STACK_OFFSET]);
	jump(pws[WS_INCR_LABEL]);
	gnlabel(pws[WS_EXIT_LABEL]);
	locsym_index = pws[WS_LOCSYM_INDEX];
	delwhile();
}

/*
 *	"switch" statement
 */
void doswitch (void)
{
	int ws[WS_COUNT];
	int *ptr;

	ws[WS_LOCSYM_INDEX] = locsym_index;
	ws[WS_STACK_OFFSET] = stkp;
	ws[WS_TYPE] = WS_SWITCH;
	ws[WS_CASE_INDEX] = swstp;
	ws[WS_SWITCH_LABEL] = getlabel();
	ws[WS_DEFAULT_LABEL] = ws[WS_EXIT_LABEL] = getlabel();
	ws[WS_SWITCH_TYPE] = CINT;
	addwhile(ws);
	needbracket("(");
	expression(YES);
	needbracket(")");

	/* peek at the output to see the switch variable's type */
	ptr = readswitch();
	if (optimize && q_nb) {
		/* these are all only optimizations, the generated code will work without them */
		switch (q_ins[q_wr].ins_code) {
		case I_LD_UM:  q_ins[q_wr].ins_code = X_LD_UMQ;  break;
		case I_LD_UP:  q_ins[q_wr].ins_code = X_LD_UPQ;  break;
		case X_LD_US:  q_ins[q_wr].ins_code = X_LD_USQ;  break;
		case X_LD_UAR: q_ins[q_wr].ins_code = X_LD_UARQ; break;
		case X_LD_UAX: q_ins[q_wr].ins_code = X_LD_UAXQ; break;
		default: break;
		}
		if (icode_flags[(q_ins[q_wr].ins_code)] & IS_SBYTE)
			ptr[WS_SWITCH_TYPE] = CCHAR;
		else
		if (icode_flags[(q_ins[q_wr].ins_code)] & IS_UBYTE)
			ptr[WS_SWITCH_TYPE] = CUCHAR;
	}

	jump(ws[WS_SWITCH_LABEL]);

	statement(NO);

	ptr = readswitch();
	if (swstp != ptr[WS_CASE_INDEX]) {
		/* there is at-least one case */
		jump(ptr[WS_EXIT_LABEL]);
		gnlabel(ptr[WS_SWITCH_LABEL]);
		dumpswitch(ptr);
	}
	gnlabel(ptr[WS_EXIT_LABEL]);
	if (swstp == ptr[WS_CASE_INDEX]) {
		/* there are no cases, so alias the switch label */
		/* done after writing the exit label so the optimizer can remove */
		/* the "jump(ptr[WS_EXIT_LABEL])" created by the final "break;" */
		out_ins_ex(I_ALIAS, T_LABEL, ptr[WS_SWITCH_LABEL], T_LABEL, ptr[WS_DEFAULT_LABEL]);
	}

	locsym_index = ptr[WS_LOCSYM_INDEX];
	stkp = modstk(ptr[WS_STACK_OFFSET]);
	swstp = ptr[WS_CASE_INDEX];
	delwhile();
}

/*
 *	"case" label
 */
void docase (void)
{
	int *ptr;
	int val = 0;

	ptr = readswitch();
	if (ptr) {
		if (!const_expr(&val, ":", NULL)) {
			error("case label must be constant");
		}
		if (ptr[WS_SWITCH_TYPE] == CCHAR) {
			if (val < -128)
				error("case label value is less than minimum value for signed char");
			if (val >  127)
				error("case label value exceeds maximum value for signed char");
		} else
		if (ptr[WS_SWITCH_TYPE] == CUCHAR) {
			if (val < 0)
				error("case label value is less than minimum value for unsigned char");
			if (val > 255)
				error("case label value exceeds maximum value for unsigned char");
		} else {
			if (val < -32768)
				error("case label value is less than minimum value for signed int");
			if (val >  65535)
				error("case label value exceeds maximum value for unsigned int");
		}
		addcase(val);
		if (!match(":"))
			error("missing colon");
	}
	else
		error("no active switch");
}

/*
 *	"default" label
 */
void dodefault (void)
{
	int *ptr;
	int lab;

	ptr = readswitch();
	if (ptr) {
		ptr[WS_DEFAULT_LABEL] = lab = getlabel();
		gdefault(lab);
		if (!match(":"))
			error("missing colon");
	}
	else
		error("no active switch");
}

/*
 *	"return" statement
 */
void doreturn (void)
{
	/* Jumping to fexitlab will clean up the top-level stack,
	   but we may have added additional stack variables in
	   an inner compound statement, and if so we have to
	   clean those up as well. */
	if (top_level_stkp != 1 && stkp != top_level_stkp)
		modstk(top_level_stkp);

	if (endst() == 0)
		expression(YES);

	jump(fexitlab);
}

/*
 *	"break" statement
 */
void dobreak (void)
{
	int *ptr;

	if ((ptr = readwhile()) == 0)
		return;

	modstk(ptr[WS_STACK_OFFSET]);
	jump(ptr[WS_EXIT_LABEL]);
}

/*
 *	"continue" statement
 */
void docont (void)
{
	int *ptr;

	if ((ptr = findwhile()) == 0)
		return;

	modstk(ptr[WS_STACK_OFFSET]);
	if (ptr[WS_TYPE] == WS_FOR)
		jump(ptr[WS_INCR_LABEL]);
	else
		jump(ptr[WS_TEST_LABEL]);
}

/*
 *
 */
void dolabel (char *name)
{
	int i;

	for (i = 0; i < clabel_ptr; i++) {
		if (!strcmp(clabels[i].name, name)) {
			/* This label has been goto'd to before.
			   We have to create a stack pointer offset EQU
			   that describes the stack pointer difference from
			   the goto to here. */
			sprintf(name, "LL%d_stkp", clabels[i].label);
			/* XXX: memleak */
			out_ins_ex(I_DEF, T_LITERAL, (intptr_t)strdup(name),
				   T_VALUE, stkp - clabels[i].stkp);
			/* From now on, clabel::stkp contains the relative
			   stack pointer at the location of the label. */
			clabels[i].stkp = stkp;
			gnlabel(clabels[i].label);
//			printf("old label %s stkp %ld\n", clabels[i].name, (long) stkp);
			return;
		}
	}
	/* This label has not been referenced before, we need to create a
	   new entry. */
	clabels = realloc(clabels, (clabel_ptr + 1) * sizeof(struct clabel));
	strcpy(clabels[clabel_ptr].name, name);
	clabels[clabel_ptr].stkp = stkp;
	clabels[clabel_ptr].label = getlabel();
//	printf("new label %s id %d stkp %ld\n", name, clabels[clabel_ptr].label, (long) stkp);
	gnlabel(clabels[clabel_ptr].label);
	clabel_ptr++;
}

void dogoto (void)
{
	int i;
	char sname[NAMESIZE];

	if (!symname(sname)) {
		error("invalid label name");
		return;
	}
	for (i = 0; i < clabel_ptr; i++) {
		if (!strcmp(clabels[i].name, sname)) {
			/* This label has already been defined. All we have
			   to do is to adjust the stack pointer and jump. */
			printf("goto found label %s id %d stkp %d\n", sname, clabels[i].label, clabels[i].stkp);
			modstk(clabels[i].stkp);
			jump(clabels[i].label);
			return;
		}
	}

	/* This label has not been defined yet. We have to create a new
	   entry in the label array. We also don't know yet what the relative
	   SP will be at the label, so we have to emit a stack pointer
	   operation with a symbolic operand that will be defined to the
	   appropriate value at the label definition. */
	clabels = realloc(clabels, (i + 1) * sizeof(*clabels));
	strcpy(clabels[i].name, sname);
	/* Save our relative SP in the label entry; this will be corrected
	   to the label's relative SP at the time of definition. */
	clabels[i].stkp = stkp;
	clabels[i].label = getlabel();
	sprintf(sname, "LL%d_stkp", clabels[i].label);
	/* XXX: memleak */
	out_ins(I_MODSP, T_LITERAL, (intptr_t)strdup(sname));
	jump(clabels[i].label);
	clabel_ptr++;
}

/*
 *	dump switch table (only if at least one case)
 *
 * Table format for 16-bit comparisons:
 *
 * !table:	dw	val3		; +01 x=2
 *		dw	val2		; +23 x=4
 *		dw	val1		; +45 x=6
 *		dw	jmpdefault	; +67 x=0
 *		dw	jmp3		; +89 x=2
 *		dw	jmp2		; +AB x=4
 *		dw	jmp1		; +CD x=6
 *
 * Table format for 8-bit comparisons:
 *
 * !table:	db	val3		; +0  x=1
 *		db	val2            ; +1  x=2
 *		db	val1            ; +2  x=3
 *		dw	jmpdefault      ; +34 x=0
 *		dw	jmp3            ; +56 x=1
 *		dw	jmp2            ; +78 x=2
 *		dw	jmp1            ; +9A x=3
 *
 * Table format for min-max ranges:
 *
 * !table:	dw	jmp1		; +01 x=\1
 *		dw	jmp2		; +23 x=\1+1
 *		dw	jmp3		; +45 x=\1+2
 *		dw	jmp4		; +67 x=\2
 *		dw	jmpdefault	; +89 x=\2+1
 */
void dumpswitch (int *ws)
{
	int i, j, column;
	int mincase = 131072;
	int maxcase = -65536;
	int numcases;
	bool do_range;
	bool is_byte;
	int label[128];

	is_byte = (ws[WS_SWITCH_TYPE] == CCHAR || ws[WS_SWITCH_TYPE] == CUCHAR);

	i = ws[WS_CASE_INDEX];
	j = swstp;
	numcases = j - i;
	while (j-- > i) {
		if (mincase > swstcase[j])
			mincase = swstcase[j];
		if (maxcase < swstcase[j])
			maxcase = swstcase[j];
	}
	if (numcases == 0) {
		error("no case statements in switch()!");
		return;
	}
	if (numcases > 127) {
		error("too many case statements in switch(), there must be less than 127");
		return;
	}

	/* is it beneficial to encode this switch() as a range? */
	do_range = (maxcase + 1 - mincase) <= (numcases * 2);

	/* create the array of labels for each entry in the range */
	if (do_range) {
		numcases = (maxcase - mincase) + 1;
		if ((mincase == 2) && (numcases < 126)) {
			/* this makes the code a bit smaller and faster */
			mincase = 0; numcases += 2;
		}
		if ((mincase == 1) && (numcases < 127)) {
			/* this makes the code a bit smaller and faster */
			mincase = 0; numcases += 1;
		}

		out_ins_ex(is_byte ? I_SWITCH_R_UR : I_SWITCH_R_WR, T_VALUE, mincase, T_VALUE, maxcase);
		flush_ins();
		outstr("\n!table:");

		for (j = 0; j < numcases; ++j)
			label[j] = ws[WS_DEFAULT_LABEL];
		j = swstp;
		while (j-- > ws[WS_CASE_INDEX])
			label[(swstcase[j] - mincase)] = swstlabel[j];

		j = 0;
		while (j < numcases) {
			defword();
			column = 8;
			while (column--) {
				outlabel(label[j++]);
				if ((column == 0) || (j == numcases)) {
					nl();
					break;
				}
				outstr(", ");
			}
		}

		defword();
		outlabel(ws[WS_DEFAULT_LABEL]);
		nl();
	} else {
		out_ins(is_byte ? I_SWITCH_C_UR : I_SWITCH_C_WR, T_VALUE, numcases);
		flush_ins();
		outstr("\n!table:");

		j = swstp;
		while (j > ws[WS_CASE_INDEX]) {
			if (is_byte)
				defbyte();
			else
				defword();
			column = 8;
			while (column--) {
				outdec(swstcase[--j]);
				if ((column == 0) || (j == ws[WS_CASE_INDEX])) {
					nl();
					break;
				}
				outstr(", ");
			}
		}

		defword();
		outlabel(ws[WS_DEFAULT_LABEL]);
		nl();

		j = swstp;
		while (j > ws[WS_CASE_INDEX]) {
			defword();
			column = 8;
			while (column--) {
				outlabel(swstlabel[--j]);
				if ((column == 0) | (j == ws[WS_CASE_INDEX])) {
					nl();
					break;
				}
				outstr(", ");
			}
		}
		nl();
	}
}

/*
 *
 */
void test (int label, int ft)
{
	needbracket("(");
	expression(YES);
	needbracket(")");
	testjump(label, ft);
}
