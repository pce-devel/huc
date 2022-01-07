#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "defs.h"
#include "externs.h"
#include "protos.h"


/* ----
 * symhash()
 * ----
 * calculate the hash value of a symbol
 */

int
symhash(void)
{
	int i;
	char c;
	int hash = 0;

	/* hash value */
	for (i = 1; i <= symbol[0]; i++) {
		c = symbol[i];
		hash += c;
	}

	/* ok */
	return (hash & 0xFF);
}


/* ----
 * addscope()
 * ----
 * prepend the scope chain to the front of a symbol.
 */

int
addscope(struct t_symbol * curscope, int i)
{
	char * string;

	/* stop at the end of scope chain */
	if (curscope == NULL) {
		return i;
	}

	/* recurse if not at root */
	if (curscope->scope != NULL) {
		i = addscope(curscope->scope, i);
	}

	string = curscope->name + 1;

	while (*string != '\0') {
		if (i < (SBOLSZ - 1)) { symbol[++i] = *string++; }
	}

	if (i < (SBOLSZ - 1)) { symbol[++i] = '.'; }

	return i;
}


/* ----
 * colsym()
 * ----
 * collect a symbol from prlnbuf into symbol[],
 * leaves prlnbuf pointer at first invalid symbol character,
 * returns 0 if no symbol collected
 */

int
colsym(int *ip, int flag)
{
	int err = 0;
	int i = 0;
	int j;
	char c;

	/* prepend the current scope? */
	c = prlnbuf[*ip];
	if ((flag != 0) && (scopeptr != NULL) && (c != '.') && (c != '@') && (c != '!')) {
		i = addscope(scopeptr, i);
	}

	/* remember where the symbol itself starts */
	j = i;

	/* get the symbol */
	for (;;) {
		c = prlnbuf[*ip];
		if (i == j && isdigit(c))
			break;
		if (isalnum(c) || (c == '_') || (c == '.') || (i == j && (c == '@' || c == '!'))) {
			if (i < (SBOLSZ - 1)) { symbol[++i] = c; }
			(*ip)++;
		} else {
			break;
		}
	}

	if (i == j) { i = 0; }

	symbol[0] = i;
	symbol[i + 1] = '\0';

	if (i >= SBOLSZ - 1) {
		char errorstr[512];
		snprintf(errorstr, 512, "Symbol name too long ('%s' is %d chars long, max is %d)", symbol + 1, i, SBOLSZ - 2);
		fatal_error(errorstr);
		return (0);
	}

	/* check if it's a reserved symbol */
	if (i == 1) {
		switch (toupper(symbol[1])) {
		case 'A':
		case 'X':
		case 'Y':
			err = 1;
			break;
		}
	}
	if (check_keyword())
		err = 1;

	/* error */
	if (err) {
		fatal_error("Reserved symbol!");
//		symbol[0] = 0;
//		symbol[1] = '\0';
		return (0);
	}

	/* ok */
	return (i);
}


/* ----
 * stlook()
 * ----
 * symbol table lookup
 * if found, return pointer to symbol
 * else, install symbol as undefined and return pointer
 */

struct t_symbol *
stlook(int type)
{
	struct t_symbol *sym;
	int hash;

	/* local symbol */
	if (symbol[1] == '.' || symbol[1] == '@') {
		if (glablptr) {
			/* search the symbol in the local list */
			sym = glablptr->local;

			while (sym) {
				if (!strcmp(symbol, sym->name))
					break;
				sym = sym->next;
			}

			/* new symbol */
			if ((sym == NULL) && (type != SYM_CHK)) {
				sym = stinstall(0, 1);
			}
		}
		else {
			if (type != SYM_CHK)
				error("Local symbol not allowed here!");
			return (NULL);
		}
	}

	/* global symbol */
	else {
		/* search symbol */
		hash = symhash();
		sym = hash_tbl[hash];
		while (sym) {
			if (!strcmp(symbol, sym->name))
				break;
			sym = sym->next;
		}

		/* new symbol */
		if ((sym == NULL) && (type != SYM_CHK)) {
			sym = stinstall(hash, 0);
		}
	}

	/* increment symbol reference counter */
	if ((sym != NULL) && (type == SYM_REF)) {
		sym->refcnt++;
	}

	/* ok */
	return (sym);
}


/* ----
 * stinstall()
 * ----
 * install symbol into symbol hash table
 */

struct t_symbol *
stinstall(int hash, int type)
{
	struct t_symbol *sym;

	/* allocate symbol structure */
	if ((sym = (void *)malloc(sizeof(struct t_symbol))) == NULL) {
		fatal_error("Out of memory!");
		return (NULL);
	}

	/* init the symbol struct */
	sym->type = if_expr ? IFUNDEF : UNDEF;
	sym->value = 0;
	sym->local = NULL;
	sym->scope = NULL;
	sym->proc = NULL;
	sym->section = -1;
	sym->bank = RESERVED_BANK;
	sym->nb = 0;
	sym->size = 0;
	sym->page = -1;
	sym->vram = -1;
	sym->pal = -1;
	sym->defcnt = 0;
	sym->refcnt = 0;
	sym->reserved = 0;
	sym->data_type = -1;
	sym->data_size = 0;
	strcpy(sym->name, symbol);

	/* add the symbol to the hash table */
	if (type) {
		/* local */
		sym->next = glablptr->local;
		glablptr->local = sym;
	}
	else {
		/* global */
		sym->next = hash_tbl[hash];
		hash_tbl[hash] = sym;
	}

	/* ok */
	return (sym);
}


/* ----
 * labldef()
 * ----
 * assign <lval> to label pointed to by lablptr,
 * checking for valid definition, etc.
 */

int
labldef(int lval, int lbnk, int lsrc)
{
	char c;

	/* check for NULL ptr */
	if (lablptr == NULL)
		return (0);

	/* adjust symbol address */
	if (lsrc == LOCATION) {
		/* is this a multi-label? */
		if (lablptr->name[1] == '!') {
			char tail [10];

			/* sanity check */
			if (lablptr->type != UNDEF) {
				fatal_error("How did this multi-label get defined!");
				return (-1);
			}

			/* define the next multi-label instance */
			strcpy(symbol, lablptr->name);
			sprintf(tail, "!%d", 0x7FFFF & ++(lablptr->defcnt));
			strncat(symbol, tail, SBOLSZ - 1 - strlen(symbol));
			symbol[0] = strlen(&symbol[1]);
			if ((lablptr = stlook(SYM_DEF)) == NULL) {
				fatal_error("Out of memory!");
				return (-1);
			}
		}

		/* fix location after crossing bank */
		if (loccnt >= 0x2000) {
			loccnt &= 0x1FFF;
			page++;
			bank++;
		}

		lval = ((loccnt + (page << 13)) & 0xFFFF);

		if (bank >= RESERVED_BANK)
			lbnk = bank;
		else
			lbnk = bank_base + bank;

		/* KickC can't call bank(), so put it in the label */
		if (kickc_mode)
			lval += lbnk << 23;
	}

	/* record definition */
	lablptr->defcnt = 1;

	/* first pass */
	if (pass == FIRST_PASS) {
		switch (lablptr->type) {
		/* undefined */
		case UNDEF:
		case IFUNDEF:
			lablptr->type = DEFABS;
			lablptr->bank = lbnk;
			lablptr->value = lval;
			break;

		/* already defined - error */
		case MACRO:
			error("Symbol already used by a macro!");
			return (-1);

		case FUNC:
			error("Symbol already used by a function!");
			return (-1);

		default:
			/* reserved label */
			if (lablptr->reserved) {
				fatal_error("Reserved symbol!");
				return (-1);
			}

			/* compare the values */
			if (lablptr->value == lval)
				break;

			/* normal label */
			lablptr->type = MDEF;
			lablptr->value = 0;
			error("Label multiply defined!");
			return (-1);
		}
	}

	/* branch pass */
	else if (pass != LAST_PASS) {
		if (lablptr->type == DEFABS) {
			lablptr->bank = lbnk;
			lablptr->value = lval;
		}
	}

	/* last pass */
	else {
		if ((lablptr->value != lval) ||
		    ((lsrc == LOCATION) && (bank < bank_limit) && (lablptr->bank != bank_base + bank))) {
			fatal_error("Symbol's bank or address changed in final pass!");
			return (-1);
		}
	}

	/* update symbol data */
	if (lsrc == LOCATION) {
		lablptr->section = section;

		if (section == S_CODE)
			lablptr->proc = proc_ptr;

		lablptr->page = page;

		/* check if it's a local or global symbol */
		c = lablptr->name[1];
		if (c == '.' || c == '@' || c == '!') {
			/* local */
			lastlabl = NULL;
		}
		else {
			/* global */
			glablptr = lablptr;
			lastlabl = lablptr;
		}
	}

	/* ok */
	return (0);
}


/* ----
 * lablset()
 * ----
 * create/update a reserved symbol
 */

void
lablset(char *name, int val)
{
	int len;

	len = strlen(name);
	lablptr = NULL;

	if (len) {
		symbol[0] = len;
		strcpy(&symbol[1], name);
		lablptr = stlook(SYM_DEF);

		if (lablptr) {
			lablptr->type = DEFABS;
			lablptr->value = val;
			lablptr->defcnt = 1;
			lablptr->reserved = 1;
		}
	}

	/* ok */
	return;
}


/* ----
 * lablexists()
 * ----
 * Determine if label exists
 */

int
lablexists(char *name)
{
	int len;

	len = strlen(name);
	lablptr = NULL;

	if (len) {
		symbol[0] = len;
		strcpy(&symbol[1], name);
		lablptr = stlook(SYM_CHK);

		if (lablptr) {
			return (1);
		}
	}
	return (0);
}


#if 0
/* ----
 * lablremap()
 * ----
 * remap all the labels
 */

void
lablremap(void)
{
	struct t_symbol *sym;
	struct t_symbol *local;
	int i;

	/* browse the symbol table */
	for (i = 0; i < 256; i++) {
		sym = hash_tbl[i];
		while (sym) {
			/* remap the bank */
			if (sym->bank <= bank_limit)
				sym->bank += bank_base;

			/* local symbols */
			if (sym->local) {
				local = sym->local;

				while (local) {
					if (local->bank <= bank_limit)
						local->bank += bank_base;

					/* next */
					local = local->next;
				}
			}

			/* next */
			sym = sym->next;
		}
	}
}
#endif


/* ----
 * dumplabl()
 * ----
 * dump all label values
 */

void
labldump(FILE *fp)
{
	struct t_symbol *sym;
	struct t_symbol *local;
	int i;

	fprintf(fp, "Bank\tAddr\tLabel\n");
	fprintf(fp, "----\t----\t-----\n");

	/* browse the symbol table */
	for (i = 0; i < 256; i++) {
		for (sym = hash_tbl[i]; sym != NULL; sym = sym->next) {
			/* skip undefined symbols and stripped symbols */
			if ((sym->type != DEFABS) || (sym->bank == STRIPPED_BANK) || (sym->name[1] == '!'))
				continue;

			/* dump the label */
			fprintf(fp, "%2.2x\t%4.4x\t", sym->bank, sym->value & 0xFFFF);
			fprintf(fp, "%s\t", &(sym->name[1]));
			if (strlen(&(sym->name[1])) < 8)
				fprintf(fp, "\t");
			if (strlen(&(sym->name[1])) < 16)
				fprintf(fp, "\t");
			if (strlen(&(sym->name[1])) < 24)
				fprintf(fp, "\t");
			fprintf(fp, "\n");

			/* local symbols */
			if (sym->local) {
				local = sym->local;

				while (local) {
					fprintf(fp, "%2.2x\t%4.4x\t", local->bank, local->value & 0xFFFF);
					fprintf(fp, "\t%s\t", &(local->name[1]));
					if (strlen(&(local->name[1])) < 8)
						fprintf(fp, "\t");
					if (strlen(&(local->name[1])) < 16)
						fprintf(fp, "\t");
					fprintf(fp, "\n");

					/* next */
					local = local->next;
				}
			}
		}
	}
}


/* ----
 * lablresetdefcnt
 * ----
 * clear the defcnt on all the multi-labels
 */

void
lablresetdefcnt(void)
{
	struct t_symbol *sym;
	int i;

	/* browse the symbol table */
	for (i = 0; i < 256; i++) {
		sym = hash_tbl[i];
		while (sym) {
			sym->defcnt = 0;

			/* local symbols */
			if (sym->local) {
				struct t_symbol * local = sym->local;

				while (local) {
					local->defcnt = 0;

					/* next */
					local = local->next;
				}
			}

			/* next */
			sym = sym->next;
		}
	}
}
