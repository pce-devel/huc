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
	const char * string;

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

	/* convert an SDCC local-symbol into a PCEAS local-symbol */
	if (sdcc_mode && (prlnbuf[(*ip)+5] == '$') &&
		isdigit((symbol[2] = prlnbuf[(*ip)+0])) &&
		isdigit((symbol[3] = prlnbuf[(*ip)+1])) &&
		isdigit((symbol[4] = prlnbuf[(*ip)+2])) &&
		isdigit((symbol[5] = prlnbuf[(*ip)+3])) &&
		isdigit((symbol[6] = prlnbuf[(*ip)+4]))) {
		symbol[0] = 6;
		symbol[1] = '@';
		symbol[7] = '\0';
		(*ip) += 6;
		return (6);
	}

	/* prepend the current scope? */
	c = prlnbuf[*ip];
	if ((flag != 0) && (scopeptr != NULL) && (c != '.') && (c != '@') && (c != '!')) {
		i = addscope(scopeptr, i);
	}

	/* remember where the symbol itself starts after the scope */
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

	if (i == SBOLSZ - 1) {
		symbol[SBOLSZ - 1] = '\0';
		fatal_error("Symbol name too long, maximum is %d characters.", SBOLSZ - 2);
		return (0);
	}

	symbol[i + 1] = '\0';

	/* skip passed the first ':' if there are two in SDCC code */
	if (sdcc_mode && (prlnbuf[*ip] == ':') && (prlnbuf[(*ip)+1] == ':'))
		(*ip)++;

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
	if (check_keyword(symbol))
		err = 1;

	/* error */
	if (err) {
		fatal_error("Reserved symbol!");
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

	/* resolve symbol alias */
	unaliased = sym;
	while ((sym != NULL) && (sym->type == ALIAS)) {
		sym = sym->local;
	}

	/* increment symbol reference counter */
	if ((sym != NULL) && (type == SYM_REF) && (if_expr == 0)) {
		sym->refthispass++;
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
	sym->local = NULL;
	sym->scope = NULL;
	sym->proc = NULL;
	sym->reason = -1;
	sym->type = if_expr ? IFUNDEF : UNDEF;
	sym->value = 0;
	sym->phase = 0;
	sym->section = S_NONE;
	sym->overlay = 0;
	sym->mprbank = UNDEFINED_BANK;
	sym->rombank = UNDEFINED_BANK;
	sym->page = -1;
	sym->nb = 0;
	sym->size = 0;
	sym->vram = -1;
	sym->pal = -1;
	sym->reserved = 0;
	sym->data_type = -1;
	sym->data_size = 0;
	sym->deflastpass = 0;
	sym->reflastpass = 1; /* so that .ifref triggers in 1st pass */
	sym->defthispass = 0;
	sym->refthispass = 0;
	sym->name = remember_string(symbol, (size_t)symbol[0] + 2);

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
labldef(int reason)
{
	char c;
	int labl_value, labl_rombank, labl_mprbank, labl_overlay, labl_section;

	/* check for NULL ptr */
	if (lablptr == NULL)
		return (0);

	/* is the symbol already used for somthing else */
	if (lablptr->type == ALIAS) {
		error("Symbol already used by an alias!");
		return (-1);
	}
	if (lablptr->type == MACRO) {
		error("Symbol already used by a macro!");
		return (-1);
	}
	if (lablptr->type == FUNC) {
		error("Symbol already used by a function!");
		return (-1);
	}

	if (lablptr->reserved) {
		fatal_error("Reserved symbol!");
		return (-1);
	}

	/* remember where this was defined */
	lablptr->fileinfo = input_file[infile_num].file;
	lablptr->fileline = slnum;

	if (reason == LOCATION) {
		/* label is set from the current LOCATION */

		/* is this a multi-label? */
		if (lablptr->name[1] == '!') {
			char tail [10];

			/* sanity check */
			if (lablptr->type != UNDEF) {
				fatal_error("How did this base multi-label get defined, that should never happen!");
				return (-1);
			}

			/* define the next multi-label instance */
			strcpy(symbol, lablptr->name);
			sprintf(tail, "!%d", 0x7FFFF & ++(lablptr->defthispass));
			strncat(symbol, tail, SBOLSZ - 1 - strlen(symbol));
			symbol[0] = (char)strlen(&symbol[1]);
			if ((lablptr = stlook(SYM_DEF)) == NULL) {
				fatal_error("Out of memory!");
				return (-1);
			}
		}

		/* fix location after crossing bank */
		if (loccnt >= 0x2000) {
			loccnt &= 0x1FFF;
			bank = (bank + 1);
			if (section != S_DATA || asm_opt[OPT_DATAPAGE] == 0)
				page = (page + 1) & 7;
		}

		labl_value = (loccnt + (page << 13) + phase_offset) & 0xFFFF;
		labl_rombank = bank;
		labl_mprbank = bank2mprbank(bank, section);
		labl_overlay = bank2overlay(bank, section);
		labl_section = section;
	} else {
		/* label is a CONSTANT or VARIABLE set from the current expression */

		/* is this a multi-label? */
		if (lablptr->name[1] == '!') {
			/* sanity check */
			fatal_error("A multi-label can only be a location!");
			return (-1);
		}

		labl_value = value;
		labl_mprbank = expr_mprbank;
		labl_overlay = expr_overlay;
		labl_rombank = mprbank2bank(expr_mprbank, expr_overlay);
		labl_section = (labl_rombank < UNDEFINED_BANK) ? S_DATA : S_NONE;
	}

//	/* needed for forward-references (in KickC and elsewhere) */
//	if ((pass != FIRST_PASS) && (lablptr->type == UNDEF)) {
//		if (pass_count < 3)
//			need_another_pass = 1;
//	}

	/* allow ".set" to change a label's value at any time */
	if ((reason == VARIABLE) && (lablptr->reason == VARIABLE))
		lablptr->type = UNDEF;

	/* is the symbol currently undefined? */
	if ((lablptr->type == UNDEF) || (lablptr->type == IFUNDEF)) {
		/* allow the definition */
	}
	/* don't allow the reason, or the value, to change during a single pass */
	else if ((lablptr->reason != reason) ||
		 (lablptr->defthispass && lablptr->value != labl_value)) {
		/* normal label */
		lablptr->type = MDEF;
		lablptr->value = 0;
		error("Label was already defined differently!");
		return (-1);
	}
	/* make sure that nothing changes at all in the last pass */
	else if (pass == LAST_PASS) {
		if ((lablptr->value != labl_value) || (lablptr->overlay != labl_overlay) ||
		    ((reason == LOCATION) && (labl_mprbank < UNDEFINED_BANK) && (lablptr->mprbank != labl_mprbank))) {
			fatal_error("Symbol's bank or address changed in final pass!");
			#if 0
			fprintf(ERROUT, "lablptr->value = $%04x, labl_value = $%04x\n", lablptr->value, labl_value);
			fprintf(ERROUT, "lablptr->mprbank = $%02x, labl_mprbank = $%02x\n", lablptr->mprbank, labl_mprbank);
			fprintf(ERROUT, "lablptr->rombank = $%02x, labl_rombank = $%02x\n", lablptr->rombank, labl_rombank);
			#endif
			return (-1);
		}
	}

	/* record definition */
	lablptr->defthispass = 1;

	/* update symbol data */
	lablptr->reason  = reason;
	lablptr->type    = DEFABS;
	lablptr->value   = labl_value;
//	if (labl_rombank < UNDEFINED_BANK) /* Don't overwrite with undefined */
		lablptr->rombank = labl_rombank;
//	if (labl_mprbank < UNDEFINED_BANK) /* Don't overwrite with undefined */
		lablptr->mprbank = labl_mprbank;
	lablptr->overlay = labl_overlay;
	lablptr->section = labl_section;

	if (reason == LOCATION) {
		if (section_flags[section] & S_IS_CODE)
			lablptr->proc = proc_ptr;

		lablptr->phase = phase_offset;
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

	len = (int)strlen(name);
	lablptr = NULL;

	if (len) {
		symbol[0] = len;
		strcpy(&symbol[1], name);
		lablptr = stlook(SYM_DEF);

		if (lablptr) {
			lablptr->type = DEFABS;
			lablptr->value = val;
			lablptr->defthispass = 1;
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

	len = (int)strlen(name);
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
	for (i = 0; i < HASH_COUNT; i++) {
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
	for (i = 0; i < HASH_COUNT; i++) {
		for (sym = hash_tbl[i]; sym != NULL; sym = sym->next) {
			/* skip undefined symbols and stripped symbols */
			if ((sym->type != DEFABS) || (sym->mprbank == STRIPPED_BANK) || (sym->name[1] == '!'))
				continue;

			/* dump the label */
			if (sym->mprbank >= UNDEFINED_BANK)
				fprintf(fp, "   -");
			else if (sym->overlay == 0)
				fprintf(fp, "  %2.2x", sym->mprbank);
			else
				fprintf(fp, "%1.1x:%2.2x", sym->overlay, sym->mprbank);

			fprintf(fp, "\t%4.4x\t", sym->value & 0xFFFF);

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
					if (local->mprbank >= UNDEFINED_BANK)
						fprintf(fp, "   -");
					else if (sym->overlay == 0)
						fprintf(fp, "  %2.2x", local->mprbank);
					else
						fprintf(fp, "%1.1x:%2.2x", local->overlay, local->mprbank);

					fprintf(fp, "\t%4.4x\t", local->value & 0xFFFF);
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
 * lablstartpass
 * ----
 * reset symbol definition and reference tracking
 */

void
lablstartpass(void)
{
	struct t_symbol *sym;
	int i;

	/* browse the symbol table */
	for (i = 0; i < HASH_COUNT; i++) {
		sym = hash_tbl[i];
		while (sym) {
			sym->deflastpass = sym->defthispass;
			sym->defthispass = 0;
			sym->reflastpass = sym->refthispass;
			sym->refthispass = 0;

			/* local symbols */
			if (sym->local) {
				struct t_symbol * local = sym->local;

				while (local) {
					local->deflastpass = local->defthispass;
					local->defthispass = 0;
					local->reflastpass = local->refthispass;
					local->refthispass = 0;

					/* next */
					local = local->next;
				}
			}

			/* next */
			sym = sym->next;
		}
	}
}


/* ----
 * bank2mprbank
 * ----
 * convert a bank number into a value to put into an MPR register
 */

int bank2mprbank (int what_bank, int what_section)
{
	if ((section_flags[what_section] & S_IS_ROM) && (what_bank < UNDEFINED_BANK)) {
		if ((section_flags[what_section] & S_IS_SF2) && (what_bank > 0x7F)) {
			/* for StreetFighterII banks in ROM */
			what_bank = 0x40 + (what_bank & 0x3F);
		} else {
			/* for all non-SF2, CD, SCD code and data banks */
			what_bank = what_bank + bank_base;
		}
	}
	return (what_bank);
}


/* ----
 * bank2overlay
 * ----
 * convert a bank number into an overlay number
 */

int bank2overlay (int what_bank, int what_section)
{
	if ((section_flags[what_section] & S_IS_ROM) && (what_bank < UNDEFINED_BANK)) {
		if ((section_flags[what_section] & S_IS_SF2) && (what_bank > 0x7F)) {
			/* for StreetFighterII banks in ROM */
			return (what_bank / 0x40) - 1;
		}
	}
	return (0);
}


/* ----
 * mprbank2bank
 * ----
 * convert an overlay and MPR register value into a bank number
 */

int mprbank2bank (int what_bank, int what_overlay)
{
	if (what_overlay != 0) {
		if ((section_flags[S_DATA] & S_IS_SF2) == 0) {
			error("You can only use overlays with the StreetFighterII mapper!");
			return (UNDEFINED_BANK);
		}
		if ((what_bank < 0x40) || (what_bank > 0x7F)) {
			error("Invalid bank and overlay for the StreetFighterII mapper!");
			return (UNDEFINED_BANK);
		}
		return (what_bank + what_overlay * 0x40);
	}
	what_bank = what_bank - bank_base;
	return (what_bank <= bank_limit) ? what_bank : UNDEFINED_BANK;
}
