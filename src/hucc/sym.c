/*	File sym.c: 2.1 (83/03/20,16:02:19) */
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
#include "const.h"
#include "error.h"
#include "gen.h"
#include "initials.h"
#include "io.h"
#include "lex.h"
#include "primary.h"
#include "pragma.h"
#include "sym.h"
#include "function.h"
#include "struct.h"

/**
 * evaluate one initializer, add data to table
 * @param symbol_name
 * @param type
 * @param identity
 * @param dim
 * @param tag
 * @return
 */
static int init (char *symbol_name, int type, int identity, int *dim, TAG_SYMBOL *tag)
{
	int value;
	int number_of_chars;

	if (identity == POINTER) {
		error("initializing non-const pointers unimplemented");
		kill();
		return (0);
	}

	if (quoted_str(&value)) {
		if ((identity == VARIABLE) || (type != CCHAR && type != CUCHAR))
			error("found string: must assign to char pointer or array");	/* XXX: make this a warning? */
		if (identity == POINTER) {
			/* unimplemented */
			printf("initptr %s value %ld\n", symbol_name, (long) value);
			abort();
			// add_data_initials(symbol_name, CUINT, value, tag);
		}
		else {
			number_of_chars = litptr - value;
			*dim = *dim - number_of_chars;
			while (number_of_chars > 0) {
				add_data_initials(symbol_name, CCHAR, litq[value++], tag);
				number_of_chars = number_of_chars - 1;
			}
		}
	}
	else if (number(&value)) {
		add_data_initials(symbol_name, CINT, value, tag);
		*dim = *dim - 1;
	}
	else if (quoted_str(&value)) {
		add_data_initials(symbol_name, CCHAR, value, tag);
		*dim = *dim - 1;
	}
	else
		return (0);

	return (1);
}

/**
 * initialise structure
 * @param tag
 */
void struct_init (TAG_SYMBOL *tag, char *symbol_name)
{
	int dim;
	int member_idx;

	member_idx = tag->member_idx;
	while (member_idx < tag->member_idx + tag->number_of_members) {
		init(symbol_name, member_table[tag->member_idx + member_idx].sym_type,
		     member_table[tag->member_idx + member_idx].identity, &dim, tag);
		++member_idx;
		if ((match(",") == 0) && (member_idx != (tag->member_idx + tag->number_of_members))) {
			error("struct initialisaton out of data");
			break;
		}
	}
}

/**
 * initialize global objects
 * @param symbol_name
 * @param type char or integer or struct
 * @param identity
 * @param dim
 * @return 1 if variable is initialized
 */
int initials (char *symbol_name, int type, int identity, int dim, int otag)
{
	int dim_unknown = 0;

	if (dim == 0)	// allow for xx[] = {..}; declaration
		dim_unknown = 1;
	if (match("=")) {
		if (type != CCHAR && type != CUCHAR && type != CINT && type != CUINT && type != CSTRUCT)
			error("unsupported storage size");
		// an array or struct
		if (match("{")) {
			// aggregate initialiser
			if ((identity == POINTER || identity == VARIABLE) && type == CSTRUCT) {
				// aggregate is structure or pointer to structure
				dim = 0;
				struct_init(&tag_table[otag], symbol_name);
			}
			else {
				while ((dim > 0) || dim_unknown) {
					if (identity == ARRAY && type == CSTRUCT) {
						// array of struct
						needbracket("{");
						struct_init(&tag_table[otag], symbol_name);
						--dim;
						needbracket("}");
					}
					else {
						if (init(symbol_name, type, identity, &dim, 0))
							dim_unknown++;
					}
					if (match(",") == 0)
						break;
				}
			}
			needbracket("}");
			// single constant
		}
		else
			init(symbol_name, type, identity, &dim, 0);
	}
	return (identity);
}


/*
 *	declare a static variable
 *
 *  David, added support for const arrays and improved error detection
 *
 */
int declglb (char typ, char stor, TAG_SYMBOL *mtag, int otag, int is_struct)
{
	int k, id;
	char sname[NAMESIZE];
	char typ_now;
	int ptr_order;
	int funcptr_type;
	int funcptr_order;
	int arg_count;
	SYMBOL *s;

	for (;;) {
		for (;;) {
			typ_now = typ;
			ptr_order = 0;
			funcptr_type = 0;
			funcptr_order = 0;
			arg_count = -1;
			if (endst())
				return (0);

			k = 1;
			id = VARIABLE;
			while (match("*")) {
				id = POINTER;
				ptr_order++;
			}
			if (amatch("__fastcall", 10)) {
				newfunc(NULL, ptr_order, typ, otag, 1);
				return (2);
			}
			if (match("(")) {
				if (typ == 0) {
					error("syntax error, a function pointer must declare its return type");
					typ_now = CINT;
				}
				funcptr_type = typ_now;
				funcptr_order = ptr_order;
				typ_now = CUINT;
				ptr_order = 0;
				while (match("*")) {
					id = POINTER;
					ptr_order++;
				}
				if (ptr_order == 0) {
					error("syntax error, expected function pointer declaration");
					return (1);
				}
				id = (--ptr_order) ? POINTER : VARIABLE;

//				if (ptr_order >= 2) {
//					error("syntax error, no pointer to a function pointer in HuCC");
//					return (1);
//				}
			}

			if (!symname(sname))
				illname();
			if (match("(")) {
				if (funcptr_type) {
					error("syntax error, expected function pointer declaration");
					return (1);
				}
				newfunc(sname, ptr_order, typ, otag, 0);
				return (2);
			}
			if ((s = findglb(sname))) {
				if ((s->storage & STORAGE) != EXTERN && !mtag)
					multidef(sname);
			}
			if (mtag && find_member(mtag, sname))
				multidef(sname);

			if (match("[")) {
				k = needsub();

				if (funcptr_type) {
					arg_count = needarguments();
					if (arg_count < 0) {
						error("syntax error, expected function pointer declaration");
						junk();
						return (1);
					}
				}
				if (stor == CONST)
					k = array_initializer(typ_now, id, stor, k);
				if (k == -1)
					return (1);

				/* XXX: This doesn't really belong here, but I
				   can't think of a better place right now. */
				if (id == POINTER && (typ_now == CCHAR || typ_now == CUCHAR || typ_now == CVOID))
					k *= INTSIZE;
				if (k || (stor == EXTERN))
					id = ARRAY;
				else {
					if (stor == CONST) {
						error("empty const array");
						id = ARRAY;
					}
					else if (id == POINTER)
						id = ARRAY;
					else {
						id = POINTER;
						ptr_order++;
					}
				}
			}
			else {
				if (funcptr_type) {
					arg_count = needarguments();
					if (arg_count < 0) {
						error("syntax error, expected function pointer declaration");
						junk();
						return (1);
					}
				}
				if (stor == CONST) {
					/* stor  = PUBLIC; XXX: What is this for? */
					scalar_initializer(typ_now, id, stor);
				}
			}

			if (mtag == 0) {
				if (typ_now == CSTRUCT) {
					if (id == VARIABLE)
						k = tag_table[otag].size;
					else if (id == POINTER)
						k = INTSIZE;
					else if (id == ARRAY)
						k *= tag_table[otag].size;
				}
				if (stor != CONST) {
					id = initials(sname, typ_now, id, k, otag);
					SYMBOL *c = addglb(sname, id, typ_now, k, stor, s);
					if (typ_now == CSTRUCT)
						c->tagidx = otag;
					c->ptr_order = ptr_order;
					c->funcptr_type = funcptr_type;
					c->funcptr_order = funcptr_order;
					c->arg_count = arg_count;
				}
				else {
					SYMBOL *c = addglb(sname, id, typ_now, k, CONST, s);
					if (c) {
						add_const(typ_now);
						if (typ_now == CSTRUCT)
							c->tagidx = otag;
					}
					c->ptr_order = ptr_order;
					c->funcptr_type = funcptr_type;
					c->funcptr_order = funcptr_order;
					c->arg_count = arg_count;
				}
			}
			else if (is_struct) {
				add_member(sname, id, typ_now, mtag->size, stor, otag, ptr_order, funcptr_type, funcptr_order, arg_count);
				if (id == POINTER)
					typ_now = CUINT;
				scale_const(typ_now, otag, &k);
				mtag->size += k;
			}
			else {
				add_member(sname, id, typ_now, 0, stor, otag, ptr_order, funcptr_type, funcptr_order, arg_count);
				if (id == POINTER)
					typ_now = CUINT;
				scale_const(typ_now, otag, &k);
				if (mtag->size < k)
					mtag->size = k;
			}
			break;
		}
		if (endst())
			return (0);

		if (!match(",")) {
			error("syntax error");
			return (1);
		}
	}
	return (0);
}

/*
 *  declare local variables
 *
 *  works just like "declglb", but modifies machine stack and adds
 *  symbol table entry with appropriate stack offset to find it again
 *
 *  zeo : added "totalk" stuff and global stack modification (00/04/12)
 */
void declloc (char typ, char stclass, int otag)
{
	int k = 0, j;
	int elements = 0;
	char sname[NAMESIZE];
	int totalk = 0;

	for (;;) {
		SYMBOL * sym = NULL;
		for (;;) {
			int ptr_order = 0;
			if (endst()) {
				if (!norecurse)
					stkp = modstk(stkp - totalk);
				return;
			}
			j = VARIABLE;
			while (match("*")) {
				j = POINTER;
				ptr_order++;
			}
			if (!symname(sname))
				illname();
			if (findloc(sname))
				multidef(sname);
			if (match("[")) {
				elements = k = needsub();
				if (k) {
					if (typ == CINT || typ == CUINT || j == POINTER)
						k = k * INTSIZE;
					else if (typ == CSTRUCT)
						k *= tag_table[otag].size;
					j = ARRAY;
				}
				else {
					j = POINTER;
					ptr_order++;
					k = INTSIZE;
					elements = 1;
				}
			}
			else {
				elements = 1;
				if ((typ == CCHAR || typ == CUCHAR) & (j != POINTER))
					k = 1;
				else if (typ == CSTRUCT) {
					if (j == VARIABLE)
						k = tag_table[otag].size;
					else if (j == POINTER)
						k = INTSIZE;
				}
				else
					k = INTSIZE;
			}
			if ((stclass & STORAGE) == LSTATIC) {
				/* Local statics are identified in two
				   different ways: The human-readable
				   identifier as given in the source text,
				   and the internal label that is used in
				   the assembly output.

				   The initializer code wants the label, and
				   it is also used to add a global to make
				   sure the right amount of space is
				   reserved in .bss and the initialized data
				   is dumped eventually.

				   addloc(), OTOH, wants the identifier so
				   it can be found when referenced further
				   down in the source text.  */
				SYMBOL *loc;
				char lsname[NAMESIZE];
				int label = getlabel();
				sprintf(lsname, "SL%d", label);

				/* do static initialization unless from norecurse */
				if ((stclass & WASAUTO) == 0)
					j = initials(lsname, typ, j, k, otag);

				/* NB: addglb() expects the number of
				   elements, not a byte size.  Unless, of
				   course, we have a CSTRUCT. *sigh* */
				if (typ == CSTRUCT)
					sym = addglb(lsname, j, typ, k, stclass, 0);
				else
					sym = addglb(lsname, j, typ, elements, stclass, 0);

				loc = addloc(sname, j, typ, label, stclass, k);
				if (typ == CSTRUCT)
					loc->tagidx = otag;
				loc->ptr_order = ptr_order;

				/* link the local and global symbols together */
				sym->linked = loc;
				loc->linked = sym;
			}
			else {
//				k = galign(k);
				totalk += k;
				// stkp = modstk (stkp - k);
				// addloc (sname, j, typ, stkp, AUTO);
#if ULI_NORECURSE
				if (!norecurse)
					sym = addloc(sname, j, typ, stkp - totalk, AUTO, k);
				else
					sym = addloc(sname, j, typ, local_offset - totalk, AUTO, k);
#else
				sym = addloc(sname, j, typ, stkp - totalk, AUTO, k);
#endif
				if (typ == CSTRUCT)
					sym->tagidx = otag;
				sym->ptr_order = ptr_order;
			}
			break;
		}
		if (match("=")) {
			int num;

			/* this should have been done in initials() above */
			if (stclass == LSTATIC)
				error("initialization of static local variables unimplemented");

#if ULI_NORECURSE
			if (!norecurse)
				stkp = modstk(stkp - totalk);
			else
				local_offset -= totalk;
			totalk -= k;
#else
			if (!norecurse) {
				stkp = modstk(stkp - totalk);
				totalk -= k;
			}
#endif

			if (const_expr(&num, ",", ";")) {
				gtext();
				if (k == 1) {
#if ULI_NORECURSE
					if (norecurse) {
						/* XXX: bit of a memory leak, but whatever... */
						SYMBOL * locsym = copysym(sym);
						sprintf(locsym->name, "_%s_end - %d", current_fn, -local_offset);
						locsym->linked = sym;
						out_ins_ex(I_ST_UMIQ, T_SYMBOL, (intptr_t)locsym, T_VALUE, num);
					}
					else
#else
					if (stclass == (LSTATIC | WASAUTO)) {
						out_ins_ex(I_ST_UMIQ, T_SYMBOL, (intptr_t)ssym, T_VALUE, num);
					}
					else
#endif
						out_ins_ex(X_ST_USIQ, T_VALUE, 0, T_VALUE, num);
				}
				else if (k == 2) {
#if ULI_NORECURSE
					if (norecurse) {
						/* XXX: bit of a memory leak, but whatever... */
						SYMBOL * locsym = copysym(sym);
						sprintf(locsym->name, "_%s_end - %d", current_fn, -local_offset);
						locsym->linked = sym;
						out_ins_ex(I_ST_WMIQ, T_SYMBOL, (intptr_t)locsym, T_VALUE, num);
					}
					else
#else
					if (stclass == (LSTATIC | WASAUTO)) {
						out_ins_ex(I_ST_WMIQ, T_SYMBOL, (intptr_t)ssym, T_VALUE, num);
					}
					else
#endif
						out_ins_ex(X_ST_WSIQ, T_VALUE, 0, T_VALUE, num);
				}
				else
					error("complex type initialization not implemented");
			}
			else
				error("cannot parse initializer");
		}
		if (!match(",")) {
			if (!norecurse)
				stkp = modstk(stkp - totalk);
#if ULI_NORECURSE
			else
				local_offset -= totalk;
#endif
			return;
		}
	}
}

/*
 *	get required array size
 */
int needsub (void)
{
	int num[1];

	if (match("]"))
		return (0);

	if (!const_expr(num, "]", NULL)) {
		error("must be constant");
		num[0] = 1;
	}
	if (!match("]"))
		error("internal error");
	if (num[0] < 0) {
		error("negative size illegal");
		num[0] = (-num[0]);
	}
	return (num[0]);
}

/*
 *	get definition of function pointer arguments
 */
int needarguments (void)
{
	struct type_type t;
	int arg_count = 0;

	if (!match(")"))
		return (-1);
	if (!match("("))
		return (-1);
	if (match(")"))
		return (0);

	do {
		if (!match_type(&t, YES, NO))
			return (-1);
		if (t.type_type == CVOID && t.ptr_order == 0) {
			if (arg_count > 0)
				return (-1);
			break;
		}
		++arg_count;
	} while (match(","));

	if (!match(")"))
		return (-1);

	return arg_count;
}

SYMBOL *findglb (char *sname)
{
	SYMBOL *ptr;

	ptr = symtab + STARTGLB;
	while (ptr != (symtab + glbsym_index)) {
		if (astreq(sname, ptr->name, NAMEMAX))
			return (ptr);

		ptr++;
	}
	return (NULL);
}

SYMBOL *findloc (char *sname)
{
	SYMBOL *ptr;

	ptr = symtab + locsym_index;
	while (ptr != (symtab + STARTLOC)) {
		ptr--;
		if (astreq(sname, ptr->name, NAMEMAX))
			return (ptr);
	}
	return (NULL);
}

SYMBOL *addglb (char *sname, char id, char typ, int value, char stor, SYMBOL *replace)
{
	char *ptr;

	if (!replace) {
		cptr = findglb(sname);
		if (cptr)
			return (cptr);

		if (glbsym_index >= ENDGLB) {
			/* the compiler can't recover from this */
			/* and will write to NULL* if we return */
			errcnt = 0;
			error("too many global C symbols, aborting");
			error("fix SYMTBSZ and NUMGLBS then rebuild the compiler");
			exit(1);
		}
		cptr = symtab + glbsym_index;
		glbsym_index++;
	}
	else
		cptr = replace;
	memset(cptr, 0, sizeof(SYMBOL));

	ptr = cptr->name;
	while (alphanum(*ptr++ = *sname++)) ;

	cptr->identity = id;
	cptr->sym_type = typ;
	cptr->storage = stor;
	cptr->offset = value;
	cptr->alloc_size = value;
	cptr->far = 0;
	cptr->linked = NULL;
	cptr->arg_count = -1;
	cptr->ptr_order = 0;
	cptr->funcptr_order = 0;

	if (id == FUNCTION)
		cptr->alloc_size = 0;
	else if (id == POINTER)
		cptr->alloc_size = INTSIZE;
	else if (typ == CINT || typ == CUINT)
		cptr->alloc_size *= 2;
	return (cptr);
}

SYMBOL *addglb_far (char *sname, char typ)
{
	SYMBOL *ptr;

	ptr = addglb(sname, ARRAY, typ, 0, EXTERN, 0);
	if (ptr)
		ptr->far = 1;
	return (ptr);
}


SYMBOL *addloc (char *sname, char id, char typ, int value, char stclass, int size)
{
	char *ptr;

	cptr = findloc(sname);
	if (cptr)
		return (cptr);

	if (locsym_index >= ENDLOC) {
		/* the compiler can't recover from this */
		/* and will write to NULL* if we return */
		errcnt = 0;
		error("too many local C symbols, aborting");
		error("fix SYMTBSZ and NUMGLBS then rebuild the compiler");
		exit(1);
	}
	cptr = symtab + locsym_index;
	memset(cptr, 0, sizeof(SYMBOL));

	ptr = symtab[locsym_index].name;
	while (alphanum(*ptr++ = *sname++)) ;

	cptr->identity = id;
	cptr->sym_type = typ;
	cptr->storage = stclass;
	cptr->offset = value;
	cptr->alloc_size = size;
	cptr->linked = NULL;
	cptr->arg_count = -1;
	locsym_index++;
	return (cptr);
}

/*
 *	test if next input string is legal symbol name
 *
 */
bool symname (char *sname)
{
	int k;

/*	char	c; */

	blanks();
	if (!alpha(ch()))
		return (false);

	k = 0;
	while (alphanum(ch()))
		sname[k++] = gch();
	sname[k] = 0;
	return (true);
}

void illname (void)
{
	error("illegal symbol name");
}

void multidef (char *sname)
{
	error("already defined");
	comment();
	outstr(sname);
	nl();
}

SYMBOL *copysym (SYMBOL *sym)
{
	SYMBOL *ptr = malloc(sizeof(SYMBOL));
	if (ptr == NULL)
		error("out of memory to copy symbol");
	else
		memcpy(ptr, sym, sizeof(SYMBOL));
	return (ptr);
}
