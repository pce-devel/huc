/*	File function.c: 2.1 (83/03/20,16:02:04) */
/*% cc -O -c %
 *
 */

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "defs.h"
#include "data.h"
#include "code.h"
#include "error.h"
#include "expr.h"
#include "fastcall.h"
#include "function.h"
#include "gen.h"
#include "io.h"
#include "lex.h"
#include "optimize.h"
#include "pragma.h"
#include "primary.h"
#include "pseudo.h"
#include "stmt.h"
#include "sym.h"
#include "struct.h"

/* locals */
static INS ins_stack[1024];
static int ins_stack_idx;
static int arg_list[32][2];
static int arg_idx;
static int func_call_stack;

/* globals */
int arg_stack_flag;
int argtop;

/* protos */
void arg_flush (int arg, int adj);
void arg_to_fptr (struct fastcall *fast, int i, int arg, int adj);
void arg_to_dword (struct fastcall *fast, int i, int arg, int adj);

/* function declaration styles */
#define KR 0
#define ANSI 1

/* argument address pointers for ANSI arguments */
short *fixup[32];
char current_fn[NAMESIZE];

static int is_leaf_function;

/*
 *	begin a function
 *
 *	called from "parse", this routine tries to make a function out
 *	of what follows
 *	modified version.  p.l. woods
 *
 */
void newfunc (const char *sname, int ret_ptr_order, int ret_type, int ret_otag, int is_fastcall)
{
	char n[NAMESIZE];
	SYMBOL *ptr;
	int nbarg = 0;
	int save_norecurse = norecurse;
	struct fastcall *fc;
	int hash;
	int fc_args;

	is_leaf_function = 1;

	if (sname) {
		/* At the opening parenthesis, declglb() found out that this
		   is a function declaration and called us.  The function
		   name has therefore already been parsed. */
		strcpy(current_fn, sname);
		strcpy(n, sname);
	}
	else {
		/* The function name has not been parsed yet. This can happen
		   in two cases:
		   - No return type: We were called from parse().
		   - fastcall: declglb() found the __fastcall attribute after
		     the type and called us. */
		if (is_fastcall) {
			fc = &ftemp;
			/* fc->nargs is the number of declared arguments;
			   fc_args is the number of arguments passed when
			   calling the function, which can be higher than
			   fc->nargs if types larger than 16 bits are used. */
			fc->nargs = 0;
			fc_args = 0;
			fc->flags = 0;
			if (match("__xsafe"))
				fc->flags |= FASTCALL_XSAFE;
			if (match("__nop"))
				fc->flags |= FASTCALL_NOP;
			else
			if (match("__macro"))
				fc->flags |= FASTCALL_MACRO;
		}
		else {
			/* No explicit return type. */
			ret_type = CINT;
		}
		if (!symname(n)) {
			error("illegal function or declaration");
			kill();
			return;
		}
		strcpy(current_fn, n);

		if (!match("("))
			error("missing open paren");

		if (is_fastcall)
			strcpy(fc->fname, n);
	}

	locptr = STARTLOC;
	argstk = 0;
	argtop = 0;
	nbarg = 0;
	memset(fixup, 0, sizeof(fixup));
	while (!match(")")) {
		/* check if we have an ANSI argument */
		struct type t;
		if (match_type(&t, NO, NO)) {
			int ptr_order;

			if (t.type == CVOID) {
				if (match(")"))
					break;
			}

			if (is_fastcall) {
				if (match("__far") || match("far"))
					fc->argtype[fc_args] = TYPE_FARPTR;
				else {
					if (t.type == CINT || t.type == CUINT)
						fc->argtype[fc_args] = TYPE_WORD;
					else
						fc->argtype[fc_args] = TYPE_BYTE;
					/* We'll change this to TYPE_WORD later
					   if it turns out to be a pointer. */
				}
			}

			ptr_order = getarg(t.type, ANSI, t.otag, is_fastcall);

			if (is_fastcall) {
				if (fc->argtype[fc_args] == TYPE_FARPTR) {
					if (ptr_order < 1) {
						error("far attribute to non-pointer type");
						return;
					}
				}
				else if (ptr_order)
					fc->argtype[fc_args] = TYPE_WORD;

				/* Parse the destination fastcall register
				   in angle brackets. */
				if (!match("<")) {
					error("missing fastcall register");
					kill();
					return;
				}
				if (!symname(fc->argname[fc_args])) {
					error("illegal fastcall register");
					kill();
					return;
				}
				if (fc->argtype[fc_args] == TYPE_FARPTR) {
					/* We have the far pointer bank
					   register, expecting ":". */
					if (!match(":")) {
						error("missing far pointer offset");
						kill();
						return;
					}

					/* Far pointers are handled as two
					   arguments internally. */
					fc_args++;
					if (fc_args >= MAX_FASTCALL_ARGS) {
						error("too many fastcall arguments");
						kill();
						return;
					}

					/* Far pointer offset register. */
					if (!symname(fc->argname[fc_args])) {
						error("illegal far pointer offset register");
						return;
					}
					fc->argtype[fc_args] = TYPE_WORD;
				}
				if (!match(">")) {
					error("missing closing angle bracket");
					return;
				}

				if (!strcmp(fc->argname[fc_args], "acc"))
					fc->argtype[fc_args] =
						(fc->argtype[fc_args] == TYPE_BYTE) ? TYPE_BYTEACC : TYPE_WORDACC;
			}
			nbarg++;
			if (is_fastcall) {
				fc->nargs++;
				fc_args++;
				if (fc_args >= MAX_FASTCALL_ARGS) {
					error("too many fastcall arguments");
					kill();
					return;
				}
			}
		}
		else {
			if (is_fastcall) {
				error("fastcall argument without type");
				kill();
				return;
			}
			/* no valid type, assuming K&R argument */
			if (symname(n)) {
				if (findloc(n))
					multidef(n);
				else {
					addloc(n, 0, 0, argstk, AUTO, INTSIZE);
					argstk = argstk + INTSIZE;
					nbarg++;
				}
			}
			else {
				error("illegal argument name");
				junk();
			}
		}
		blanks();
		if (!streq(line + lptr, ")")) {
			if (!match(","))
				error("expected comma");
		}
		if (endst())
			break;
	}

	for (;;) {
		if (amatch("__norecurse", 14)) {
			norecurse = 1;
			continue;
		}
		else if (amatch("__recurse", 9)) {
			norecurse = 0;
			continue;
		}
		break;
	}

	if (match(";")) {
		/* function prototype */
		ptr = addglb(current_fn, FUNCTION, ret_type, 0, EXTERN, 0);
		norecurse = save_norecurse;

		if (is_fastcall) {
			/* XXX: pragma code sets flag 0x02 if there are
			   memory arguments, but that's not used
			   anywhere... */

			int i;
			for (i = 0; i < fc_args - 1; i++) {
				if (fc->argtype[i] == TYPE_WORDACC || fc->argtype[i] == TYPE_BYTEACC) {
					error("fastcall accumulator argument must come last");
					kill();
					return;
				}
			}

			if (fastcall_look(n, fc->nargs, NULL)) {
				error("fastcall already defined");
				return;
			}

//			printf("Registered __fastcall %s with %d arguments.\n", n, fc->nargs);

			/* insert function into fastcall table */
			fc = (void *)malloc(sizeof(struct fastcall));
			if (!fc) {
				error("out of memory");
				return;
			}
			*fc = ftemp;
			hash = symhash(n);
			fc->next = fastcall_tbl[hash];
			fastcall_tbl[hash] = fc;
		}

		return;
	}
	else if (is_fastcall) {
		error("__fastcall can only be used in prototypes");
		return;
	}

	if (fastcall_look(n, nbarg, NULL)) {
		error("implementation of fastcall functions in C not supported yet");
		return;
	}

	stkp = 0;
	clabel_ptr = 0;
	argtop = argstk;
	while (argstk) {
		/* We only know the final argument offset once we have parsed
		   all of them. That means that for ANSI arguments we have
		   to fix up the addresses for the locations generated in
		   getarg() here. */
		if (fixup[argstk / INTSIZE - 1]) {
			argstk -= INTSIZE;
			*fixup[argstk / INTSIZE] += argtop;
		}
		else {
			struct type t;
			if (match_type(&t, NO, NO)) {
				getarg(t.type, KR, t.otag, is_fastcall);
				ns();
			}
			else {
				error("wrong number args");
				break;
			}
		}
	}

	fexitlab = getlabel();

	if ((ptr = findglb(current_fn))) {
		if (ptr->ident != FUNCTION)
			multidef(current_fn);
		else if (ptr->offset == FUNCTION)
			multidef(current_fn);
		else
			/* XXX: sanity check? */
			ptr->offset = FUNCTION;
	} else {
		ptr = addglb(current_fn, FUNCTION, ret_type, FUNCTION, PUBLIC, 0);
	}
	ptr->ptr_order = ret_ptr_order;
	ptr->tagidx = ret_otag;

	flush_ins();	/* David, .proc directive support */
	gtext();
	comment();
	outstr("*******\n\n");
	ol(".hucc");
	ot(".proc\t\t");
	prefix();
	outstr(current_fn);
	nl();

	/* generate the function prolog */
	out_ins(I_ENTER, T_STRING, (intptr_t)ptr->name);

#if ULI_NORECURSE
	/* When using fixed-address locals, locals_ptr is used to
	   keep track of their memory offset instead of stkp, so
	   we have to reset it before producing code. */
	if (norecurse)
		locals_ptr = 0;
#endif

	/* generate all the code for the function */
	statement(YES);

	/* generate the function epilog */
	gtext();
	gnlabel(fexitlab);
	modstk(nbarg * INTSIZE);
	out_ins(I_LEAVE, T_VALUE, ret_type != CVOID || ret_ptr_order != 0); /* generate the return statement */
	flush_ins();		/* David, optimize.c related */

	ol(".endp");	/* David, .endp directive support */

#if ULI_NORECURSE
	/* Add space for fixed-address locals to .bss section. */
	if (norecurse && locals_ptr < 0) {
		if (is_leaf_function) {
			leaf_functions = realloc(leaf_functions, (leaf_cnt + 1) * sizeof(*leaf_functions));
			leaf_functions[leaf_cnt++] = strdup(current_fn);
			if (-locals_ptr > leaf_size)
				leaf_size = -locals_ptr;
		}
		else {
			ot(".data"); nl();
			ot(".bss"); nl();
			outstr("__"); outstr(current_fn); outstr("_loc:\n\t.ds\t\t");
			outdec(-locals_ptr); nl();
			outstr("__"); outstr(current_fn); outstr("_lend:"); nl();
			ot(".code"); nl();
		}
	}
#endif

	/* Signal that we're not in a function anymore. */
	fexitlab = 0;

	ol(".pceas");
	nl();
	stkp = 0;
	locptr = STARTLOC;
	norecurse = save_norecurse;
}

/*
 *	declare argument types
 *
 *	called from "newfunc", this routine add an entry in the local
 *	symbol table for each named argument
 *	completely rewritten version.  p.l. woods
 *
 */
int getarg (int t, int syntax, int otag, int is_fastcall)
{
	int j, legalname, address;
	char n[NAMESIZE];
	SYMBOL *argptr;
	int ptr_order = 0;

/*	char	c; */

	FOREVER {
		if (syntax == KR && argstk == 0)
			return (ptr_order);

		j = VARIABLE;
		while (match("*")) {
			j = POINTER;
			ptr_order++;
		}

		if (t == CSTRUCT && j != POINTER)
			error("passing structures as arguments by value not implemented yet");

		if (t == CVOID) {
			if (j != POINTER)
				error("illegal argument type \"void\"");
			else
				t = CUINT;
		}

		if (!(legalname = symname(n))) {
			if (syntax == ANSI && (ch() == ',' || ch() == ')'))
				sprintf(n, "__anon_%d\n", (int) -argstk);
			else {
				illname();
				junk();
			}
		}
		if (match("[")) {
			while (inbyte() != ']')
				if (endst())
					break;
			j = POINTER;
			ptr_order++;
		}
		if (legalname) {
			if (syntax == ANSI) {
				if (findloc(n))
					multidef(n);
				else {
					addloc(n, 0, 0, argstk, AUTO, INTSIZE);
					argstk = argstk + INTSIZE;
				}
			}
			if ((argptr = findloc(n))) {
				argptr->ident = j;
				argptr->type = t;
				address = argtop - glint(argptr) - 2;
				if ((t == CCHAR || t == CUCHAR) && j == VARIABLE)
					address = address + BYTEOFF;
				argptr->offset = address;
				argptr->tagidx = otag;
				argptr->ptr_order = ptr_order;
				if (syntax == ANSI)
					fixup[argstk / INTSIZE - 1] = &argptr->offset;
			}
			else
				error("expecting argument name");
		}
		if (syntax == KR) {
			argstk = argstk - INTSIZE;
			if (endst())
				return (ptr_order);

			if (!match(","))
				error("expected comma");
		}
		else {
			blanks();
			if (streq(line + lptr, ")") ||
			    streq(line + lptr, ",") ||
			    (is_fastcall && streq(line + lptr, "<"))
			    )
				return (ptr_order);
			else
				error("expected comma, closing bracket, or fastcall register");
		}
	}
	return (ptr_order);
}

/*
 *	perform a function call
 *
 *	called from "heir11", this routine will either call the named
 *	function, or if the supplied ptr is zero, will call the contents
 *	of HL
 *
 */
#define SPILLB(a) { \
		spilled_args[sparg_idx] = (a); \
		spilled_arg_sizes[sparg_idx++] = 1; \
		out_ins(I_SPUSH_UR, 0, 0); \
}

#define SPILLW(a) { \
		spilled_args[sparg_idx] = (a); \
		spilled_arg_sizes[sparg_idx++] = 2; \
		out_ins(I_SPUSH_WR, 0, 0); \
}

void callfunction (char *ptr)
{
	extern char *new_string (int, char *);
	struct fastcall *fast = NULL;
	SYMBOL *func = NULL;
	int is_fc = 0;
	int argcnt = 0;
	int argsiz = 0;
	int call_stack_ref;
	int i, j;
	int adj;
	int max_fc_arg = 0;	/* highest arg with a fastcall inside */
	/* args spilled to the native stack */
	const char *spilled_args[MAX_FASTCALL_ARGS];
	/* byte sizes of spilled args */
	int spilled_arg_sizes[MAX_FASTCALL_ARGS];
	int sparg_idx = 0;	/* index into spilled_args[] */
	int uses_acc = 0;	/* does callee use acc? */

	is_leaf_function = 0;

	call_stack_ref = ++func_call_stack;

	/* skip blanks */
	blanks();

	/* check if it's a special function,
	 * if yes handle it externaly
	 */
	if (ptr) {
		if (!strcmp(ptr, "bank")) {
			do_asm_func(T_BANK); return;
		}
		else if (!strcmp(ptr, "vram")) {
			do_asm_func(T_VRAM); return;
		}
		else if (!strcmp(ptr, "pal")) {
			do_asm_func(T_PAL);  return;
		}
	}

//	flush_ins();
//	ot("__calling\n");

	if (ptr == NULL) {
		/* save indirect call function-ptr on the hardware-stack */
		out_ins(I_SPUSH_WR, 0, 0);
	} else {
		/* fastcall check, but don't know how many parameters */
		if ((is_fc = fastcall_look(ptr, -1, NULL)) == 0) {
			/* N.B. functions are not required to be pre-declared! */
			func = findglb(ptr);
		}
	}

	/* calling regular functions in fastcall arguments is OK */
	if (is_fc)
		flush_ins();
	else
		--func_call_stack;

	/* get args */
	while (!streq(line + lptr, ")")) {
		if (endst())
			break;
		/* fastcall func */
		if (is_fc) {
			int nfc = func_call_stack;

			arg_stack(arg_idx++);
			expression(NO);
			flush_ins();
			stkp = stkp - INTSIZE;

			/* Check if we had a fastcall in our argument. */
			if (nfc < func_call_stack) {
				/* Remember the last argument with an FC. */
				if (max_fc_arg < arg_idx - 1)
					max_fc_arg = arg_idx - 1;
			}
		}
		/* standard func */
		else {
			expression(NO);
			gpusharg(INTSIZE);
			gfence();
		}
		argsiz = argsiz + INTSIZE;
		argcnt++;
		if (!match(","))
			break;
	}

	/* adjust arg stack */
	if (is_fc) {
		if (argcnt) {
			arg_list[arg_idx - 1][1] = ins_stack_idx;
			arg_idx -= argcnt;
		}
		if (argcnt && arg_idx)
			ins_stack_idx = arg_list[arg_idx - 1][1];
		else {
			ins_stack_idx = 0;
			arg_stack_flag = 0;
		}
	}

	/* fastcall func */
	if (is_fc) {
		is_fc = fastcall_look(ptr, argcnt, &fast);

		/* flush arg instruction stacks */
		if (is_fc) {
			/* fastcall */
			for (i = 0, j = 0, adj = 0; i < argcnt; i++) {
				/* flush arg stack (except for farptr and dword args) */
				if ((fast->argtype[j] != TYPE_FARPTR) &&
				    (fast->argtype[j] != TYPE_DWORD))
					arg_flush(arg_idx + i, adj);

				/* Either store the argument in its designated
				   location, or save it on the hardware-stack
				   if there is another fastcall ahead. */
				switch (fast->argtype[j]) {
				case TYPE_BYTE:
					if (i < max_fc_arg)
						SPILLB(fast->argname[j])
					else {
						out_ins(I_ST_UM, T_LITERAL, (intptr_t)fast->argname[j]);
						gfence();
					}
					break;
				case TYPE_WORD:
					if (i < max_fc_arg)
						SPILLW(fast->argname[j])
					else {
						out_ins(I_ST_WM, T_LITERAL, (intptr_t)fast->argname[j]);
						gfence();
					}
					break;
				case TYPE_FARPTR:
					arg_to_fptr(fast, j, arg_idx + i, adj);
					if (i < max_fc_arg) {
						out_ins(I_LD_UM, T_LITERAL, (intptr_t)fast->argname[j]);
						SPILLB(fast->argname[j])
						out_ins(I_LD_WM, T_LITERAL, (intptr_t)fast->argname[j + 1]);
						SPILLW(fast->argname[j + 1])
					}
					j += 1;
					break;
				case TYPE_DWORD:
					arg_to_dword(fast, j, arg_idx + i, adj);
					if (i < max_fc_arg) {
						out_ins(I_LD_WM, T_LITERAL, (intptr_t)fast->argname[j]);
						SPILLW(fast->argname[j])
						out_ins(I_LD_WM, T_LITERAL, (intptr_t)fast->argname[j + 1]);
						SPILLW(fast->argname[j + 1])
						out_ins(I_LD_WM, T_LITERAL, (intptr_t)fast->argname[j + 2]);
						SPILLW(fast->argname[j + 2])
					}
					j += 2;
					break;
				case TYPE_WORDACC:
					if (i < max_fc_arg)
						SPILLW(0)
					uses_acc = 1;
					break;
				case TYPE_BYTEACC:
					if (i < max_fc_arg)
						SPILLW(0)
					else
						gshort();
					uses_acc = 1;
					break;
				default:
					error("fastcall internal error");
					break;
				}

				/* next */
				adj += INTSIZE;
				j++;
			}
		}
		else {
			/* not really a fastcall after all! */
			for (i = 0; i < argcnt; i++) {
				arg_flush(arg_idx + i, 0);
				gpusharg(0);
				gfence();
			}
		}
	}

	/* reset func call stack */
	if (call_stack_ref == 1)
		func_call_stack = 0;

	/* close */
	needbrack(")");

	/* call function */
	/* Reload fastcall arguments spilled to the hardware-stack. */
	if (sparg_idx) {
		/* Reloading corrupts acc, so we need to save it if it
		   is used by the callee. */
		if (uses_acc) {
			out_ins(I_ST_WM, T_LITERAL, (intptr_t)"__temp");
			gfence();
		}

		for (i = sparg_idx - 1; i > -1; i--) {
			if (spilled_arg_sizes[i] == 1) {
				out_ins(I_SPOP_UR, 0, 0);
				if (spilled_args[i]) {
					out_ins(I_ST_UM, T_LITERAL, (intptr_t)spilled_args[i]);
					gfence();
				}
			}
			else {
				out_ins(I_SPOP_WR, 0, 0);
				if (spilled_args[i]) {
					out_ins(I_ST_WM, T_LITERAL, (intptr_t)spilled_args[i]);
					gfence();
				}
			}
		}

		if (uses_acc)
			out_ins(I_LD_WM, T_LITERAL, (intptr_t)"__temp");
	}

	if (ptr == NULL) {
		/* restore indirect call function-ptr from the hardware-stack */
		out_ins(I_SPOP_WR, 0, 0);
		out_ins(I_CALLP, 0, 0);
	} else {
		if (fast && !(fast->flags & FASTCALL_XSAFE))
			out_ins(I_SAVESP, 0, 0);
		if (fast && (fast->flags & (FASTCALL_NOP | FASTCALL_MACRO))) {

			// Only macro fastcalls get a name generated
			if (fast->flags & FASTCALL_MACRO) {
				if (is_fc)
					gmacro(ptr, argcnt);
				else
					gmacro(ptr, 0);
			}
		}
		// Else not a NOP or MACRO fastcall
		else if (is_fc)
			gcall(ptr, argcnt);
		else
			gcall(ptr, 0);
		if (fast && !(fast->flags & FASTCALL_XSAFE))
			out_ins(I_LOADSP, 0, 0);
	}

//	flush_ins();
//	ot("__called\n");

	/* adjust stack */
	if (argsiz) {
		stkp = stkp + argsiz;
		/* this is put into the instruction stream to balance the stack */
		/* calculations during the peephole optimization phase, but the */
		/* T_NOP stops the code-output from writing it to the .S file.  */
		/* The function that is called is actually the one responsible  */
		/* for removing the arguments from the stack. */
		out_ins(I_MODSP, T_NOP, argsiz);
	}

	/* load acc.l if this a standard func returning a value */
	if (!is_fc) {
		if (func == NULL || func->type != CVOID || func->ptr_order != 0)
			out_ins(I_GETACC, 0, 0);
	}
}

/*
 * start arg instruction stacking
 *
 */
void arg_stack (int arg)
{
	if (arg > 31)
		error("too many args");
	else {
		/* close previous stack */
		if (arg)
			arg_list[arg - 1][1] = ins_stack_idx;

		/* init new stack */
		ins_stack_idx += 4;
		arg_list[arg][0] = ins_stack_idx;
		arg_list[arg][1] = -1;
		arg_stack_flag = 1;
	}
}

/*
 * put instructions in a temporary stack (one for each func arg)
 *
 */
void arg_push_ins (INS *ptr)
{
	if (ins_stack_idx < 1024)
		ins_stack[ins_stack_idx++] = *ptr;
	else {
		if (ins_stack_idx < 1025) {
			ins_stack_idx++;
			error("arg stack full");
		}
	}
}

/*
 * flush arg instruction stacks
 *
 */
void arg_flush (int arg, int adj)
{
	INS *ins;
	int idx;
	int nb;
	int i;

	if (arg > 31)
		return;

	idx = arg_list[arg][0];
	nb = arg_list[arg][1] - arg_list[arg][0];

	for (i = 0; i < nb;) {
		/* adjust stack refs */
		i++;
		ins = &ins_stack[idx];

		if ((ins->type == T_STACK) && (ins->code == I_LD_WM)) {
printf("Can this ever occur?\n");
abort();
			if (i < nb) {
				ins = &ins_stack[idx + 1];
				if ((ins->code == I_ADD_WI) && (ins->type == T_VALUE))
					ins->data -= adj;
			}
		}
		else {
			if (icode_flags[ins->code] & IS_SPREL)
				ins->data -= adj;
		}

		/* flush */
		gen_ins(&ins_stack[idx++]);
	}
}

/*
 * convert a function argument into a farptr
 *
 */
void arg_to_fptr (struct fastcall *fast, int i, int arg, int adj)
{
	INS *ins, tmp;
	SYMBOL *sym;
	int idx;
	int err;
	int nb;

	if (arg > 31)
		return;

	idx = arg_list[arg][0];
	nb = arg_list[arg][1] - arg_list[arg][0];
	ins = &ins_stack[idx];
	err = 0;

	/* check arg */
	/* this code can be fooled, but it should catch most common errors */
	err = 1;
	if (nb == 1) {
		if (ins->type == T_SYMBOL) {
			/* allow either "function", "array", or "array+const" */
			sym = (SYMBOL *)ins->data;
			switch (sym->ident) {
				case FUNCTION:
				case ARRAY:
					if (ins->code == I_LD_WI)
						err = 0;
					break;
				case POINTER:
					if (ins->code == I_LD_WM)
						err = 0;
					break;
			}
		}
	}
	else if (nb > 1) {
		/* search the complex expression for the base symbol */
		for (idx = 0; idx < nb; ++idx, ++ins) {
			if (ins->type == T_SYMBOL) {
				sym = (SYMBOL *)ins->data;
				switch (sym->ident) {
					/* for the first "array", or "array+const" */
					case ARRAY:
						if ((ins->code == I_LD_WI) ||
							(ins->code == I_ADD_WI))
							err = 0;
						break;
					/* or the first pointer */
					case POINTER:
						if (ins->code == I_LD_WM)
							err = 0;
						break;
				}
			}
			/* break when a qualifying symbol is found */
			if (err == 0) break;
		}

		/* check if last instruction is a pointer dereference */
		switch (ins_stack[ arg_list[arg][0] + nb - 1 ].code) {
			case I_LD_UP:
			case I_LD_WP:
				err = 1;
				break;
			default:
				break;
		}
	}

	if (err) {
		error("can't get farptr");
		return;
	}

	/* ok */
	if (nb == 1) {
		ins->code = I_FARPTR;
		ins->arg[0] = fast->argname[i];
		ins->arg[1] = fast->argname[i + 1];
		gen_ins(ins);
	}
	else {
		sym = (SYMBOL *)ins->data;

		/* check symbol type */
		if (sym->far) {
			tmp.code = I_FARPTR_I;
			tmp.type = T_SYMBOL;
			tmp.data = ins->data;
			tmp.arg[0] = fast->argname[i];
			tmp.arg[1] = fast->argname[i + 1];
			/* this nukes the symbol from the I_LD_WI or I_ADD_WI */
			ins->type = T_VALUE;
			ins->data = 0;
		}
		else {
			/* a pointer or an array of pointers */
			if (((sym->ident == ARRAY) ||
			     (sym->ident == POINTER)) &&
			    (sym->type == CINT || sym->type == CUINT)) {
				tmp.code = I_FARPTR_GET;
				tmp.type = 0;
				tmp.data = 0;
				tmp.arg[0] = fast->argname[i];
				tmp.arg[1] = fast->argname[i + 1];
			}
			else {
				error("can't get farptr");
				return;
			}
		}
		arg_flush(arg, adj);
		gen_ins(&tmp);
	}
}

/*
 *
 */
void arg_to_dword (struct fastcall *fast, int i, int arg, int adj)
{
	INS *ins, *ptr, tmp;
	SYMBOL *sym;
	int idx;
	int gen;
	int err;
	int nb;

	if (arg > 31)
		return;

	idx = arg_list[arg][0];
	nb = arg_list[arg][1] - arg_list[arg][0];
	ins = &ins_stack[idx];
	gen = 0;
	err = 1;

	/* check arg */
	if (nb == 1) {
		/* immediate value */
		if ((ins->code == I_LD_WI) && (ins->type == T_VALUE)) {
			ins->code = X_LDD_I;
			ins->arg[0] = fast->argname[i + 1];
			ins->arg[1] = fast->argname[i + 2];
			gen = 1;
		}

		/* var/ptr */
		else if ((((ins->code == I_LD_WM) || (ins->code == I_LD_BM) || (ins->code == I_LD_UM))
			  && (ins->type == T_SYMBOL)) || (ins->type == T_LABEL)) {
			/* check special cases */
			if (ins->type == T_LABEL) {
				error("dword arg can't be a static var");
				return;
			}

			/* get symbol */
			sym = (SYMBOL *)ins->data;

			/* check type */
			if (sym->ident == POINTER)
				gen = 1;
			else if (sym->ident == VARIABLE) {
				if (ins->code == I_LD_WM)
					ins->code = X_LDD_W;
				else
					ins->code = X_LDD_B;

				ins->type = T_SYMBOL;
				ins->data = (intptr_t)sym;
				ins->arg[0] = fast->argname[i + 1];
				ins->arg[1] = fast->argname[i + 2];
				gen = 1;
			}
		}

		/* var/ptr */
		else if ((ins->code == X_LD_WS) || (ins->code == X_LD_BS) || ins->code == X_LD_US) {
			/* get symbol */
			sym = ins->sym;

			/* check type */
			if (sym) {
				if (sym->ident == POINTER)
					gen = 1;
				else if (sym->ident == VARIABLE) {
					if (ins->code == X_LD_WS)
						ins->code = X_LDD_S_W;
					else
						ins->code = X_LDD_S_B;

					ins->data -= adj;
					ins->arg[0] = fast->argname[i + 1];
					ins->arg[1] = fast->argname[i + 2];
					gen = 1;
				}
			}
		}

		/* array */
		else if (ins->code == I_LEA_S) {
			sym = ins->sym;

			if (sym && (sym->ident == ARRAY)) {
				ins->data -= adj;
				gen = 1;
			}
		}

		/* array */
		else if ((ins->code == I_LD_WI) && (ins->type == T_SYMBOL)) {
			/* get symbol */
			sym = (SYMBOL *)ins->data;

			/* check type */
			if (sym->ident == ARRAY)
				gen = 1;
		}
	}
	else if (nb == 2) {
		/* array */
		if ((ins->code == I_LD_WI) && (ins->type == T_SYMBOL)) {
			/* get symbol */
			sym = (SYMBOL *)ins->data;

			/* check type */
			if (sym->ident == ARRAY) {
				ptr = ins;
				ins = &ins_stack[idx + 1];

				if ((ins->code == I_ADD_WI) && (ins->type == T_VALUE)) {
					gen_ins(ptr);
					gen = 1;
				}
			}
		}
	}

	/* gen code */
	if (gen) {
		gen_ins(ins);
		err = 0;

		if (strcmp(fast->argname[i], "#acc") != 0) {
			tmp.code = I_ST_WM;
			tmp.type = T_SYMBOL;
			tmp.data = (intptr_t)fast->argname[i];
			gen_ins(&tmp);
		}
	}

	/* errors */
	if (err) {
		if (optimize < 1)
			error("dword arg support works only with optimization enabled");
		else
			error("invalid or too complex dword arg syntax");
	}
}
