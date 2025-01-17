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
	bool auto_default = false;

	ws[WS_LOCSYM_INDEX] = locsym_index;
	ws[WS_STACK_OFFSET] = stkp;
	ws[WS_TYPE] = WS_SWITCH;
	ws[WS_CASE_INDEX] = swstp;
	ws[WS_SWITCH_LABEL] = getlabel();
	ws[WS_DEFAULT_LABEL] = ws[WS_EXIT_LABEL] = getlabel();
	addwhile(ws);
	needbracket("(");
	expression(YES);
	needbracket(")");
	jump(ws[WS_SWITCH_LABEL]);
	statement(NO);
	ptr = readswitch();
	jump(ptr[WS_EXIT_LABEL]);

	if (ptr[WS_DEFAULT_LABEL] == ptr[WS_EXIT_LABEL]) {
		ptr[WS_DEFAULT_LABEL] = getlabel();
		auto_default = true;
	}
	gnlabel(ptr[WS_SWITCH_LABEL]);

	dumpswitch(ptr);

	if (auto_default) {
		gnlabel((int)ptr[WS_DEFAULT_LABEL]);
		out_ins(I_DEFAULT, 0, 0);
	}
	gnlabel(ptr[WS_EXIT_LABEL]);

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
	int val = 0;
	if (readswitch()) {
		if (!const_expr(&val, ":", NULL)) {
			error("case label must be constant");
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
 *	dump switch table
 *
 *	case_table:
 *	+  0		db	6		; #bytes of case values.
 *	+ 12		dw	val1
 *	+ 34		dw	val2
 *	+ 56		dw	val3
 *	+ 78		dw	jmpdefault
 *	+ 9A		dw	jmp1
 *	+ BC		dw	jmp2
 *	+ DE		dw	jmp3
 */
void dumpswitch (int *ws)
{
	int i, j, column;

	i = getlabel();
	gswitch(i);
	gnlabel(i);
	flush_ins();

	i = ws[WS_CASE_INDEX];

	if ((swstp - i) > 63) {
		error("too many case statements in switch(), there must be less than 64");
		return;
	}

	defbyte();
	outdec((swstp - i) << 1);
	nl();

	j = swstp;
	while (j > i) {
		defbyte();
		column = 8;
		while (column--) {
			outbyte('>');
			outdec(swstcase[--j]);
			outbyte(',');
			outbyte('<');
			outdec(swstcase[j]);
			if ((column == 0) | (j == i)) {
				nl();
				break;
			}
			outbyte(',');
		}
	}

	defword();
	outlabel(ws[WS_DEFAULT_LABEL]);
	nl();

	j = swstp;
	while (j > i) {
		defword();
		column = 8;
		while (column--) {
			outlabel(swstlabel[--j]);
			if ((column == 0) | (j == i)) {
				nl();
				break;
			}
			outbyte(',');
		}
	}
	nl();
}

void test (int label, int ft)
{
	needbracket("(");
	expression(YES);
	needbracket(")");
	testjump(label, ft);
}
