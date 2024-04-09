#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "defs.h"
#include "externs.h"
#include "protos.h"

int in_if;		/* set when we are in an .if statement */
int if_expr;		/* set when parsing an .if expression */
int if_level;		/* level of nested .if's */
int if_state[256];	/* status when entering the .if */
int if_flag[256];	/* .if/.else status */
int if_line[256];	/* .if line number */
char if_txt[256][64];	/* .if condition text */
int skip_lines;		/* set when lines must be skipped */
int continued_line;	/* set when a line is the continuation of another line */


/* ----
 * assemble()
 * ----
 * translate source line to machine language
 */

void
assemble(int do_label)
{
	struct t_line *ptr;
	char *buf;
	char c;
	int flag;
	int ip, i, j;			/* prlnbuf pointer */

	/* init variables */
	lablptr = NULL;
	continued_line = 0;
	data_loccnt = -1;
	data_size = 4;
	data_level = 1;

	/* macro definition */
	if (in_macro) {
		i = preproc_sfield;
		if (do_label && colsym(&i, 0))
			if (prlnbuf[i] == ':')
				i++;
		while (isspace(prlnbuf[i]))
			i++;
		if (pass == LAST_PASS)
			println();
		if (oplook(&i) >= 0) {
			if (opflg == PSEUDO) {
				if (opval == P_MACRO) {
					error("Can not nest macro definitions!");
					return;
				}
				if (opval == P_ENDM) {
					if (!check_eol(&i))
						return;
					in_macro = 0;
					return;
				}
			}
		}
		if (pass == FIRST_PASS) {
			if (preproc_modidx != 0) {
				/* Remove C-style comments within a macro */
				prlnbuf[preproc_modidx] = '\0';
			}
			ptr = (void *)malloc(sizeof(struct t_line));
			buf = (void *)malloc(strlen(&prlnbuf[preproc_sfield]) + 1);
			if ((ptr == NULL) || (buf == NULL)) {
				error("Out of memory!");
				return;
			}
			strcpy(buf, &prlnbuf[preproc_sfield]);
			ptr->next = NULL;
			ptr->data = buf;
			if (mlptr)
				mlptr->next = ptr;
			else
				mptr->line = ptr;
			mlptr = ptr;
		}
		return;
	}

	/* unused PROC/PROCGROUP that has been stripped out;
	 * check for a '.endp' or '.endprocgroup'
	 * to toggle state
	 */
	if (skipping_stripped) {
		i = preproc_sfield;
		while (isspace(prlnbuf[i])) { i++; }
		if ((oplook(&i) >= 0) && (opflg == PSEUDO)) {
			switch (opval) {

			case P_ENDP:		// .endp
			case P_ENDPG:		// .endprocgroup
				if (optype != skipping_stripped)
					break;
				skipping_stripped = 0;
				if (pass == LAST_PASS)
					println();
			}
		}
		return;
	}

	/* IF/ELSE section;
	 * check for a '.else' or '.endif'
	 * to toggle state
	 */
	if (in_if) {
		i = preproc_sfield;
		while (isspace(prlnbuf[i])) { i++; }
		if (oplook(&i) >= 0) {
			if (opflg == PSEUDO) {

				switch (opval) {
				case P_IF:		// .if
				case P_IFDEF:		// .ifdef
				case P_IFNDEF:		// .ifndef
					save_if_expr(&i);
					if (skip_lines) {
						if_level++;
						if_state[if_level] = 0;
					}
					break;

				case P_ELSE:		// .else
					while (isspace(prlnbuf[i])) { i++; }
					if ((prlnbuf[i] != ';') && (prlnbuf[i] != '\0')) {
						/* check that expression matches if_level */
						save_if_expr(&i);
						if (strcmp(if_txt[if_level], if_txt[if_level-1]) != 0) {
							char message [128];
							sprintf(message, "Condition does not match \".if %s\" at line %d!", if_txt[if_level-1], if_line[if_level-1]);
							fatal_error(message);
							return;
						}
					}
					if (if_state[if_level]) {
						skip_lines = !if_flag[if_level];
						if (pass == LAST_PASS)
							println();
					}
					return;

				case P_ENDIF:		// .endif
					while (isspace(prlnbuf[i])) { i++; }
					if ((prlnbuf[i] != ';') && (prlnbuf[i] != '\0')) {
						/* check that expression matches if_level */
						save_if_expr(&i);
						if (strcmp(if_txt[if_level], if_txt[if_level-1]) != 0) {
							char message [128];
							sprintf(message, "Condition does not match \".if %s\" at line %d!", if_txt[if_level-1], if_line[if_level-1]);
							fatal_error(message);
							return;
						}
					}
					if (if_state[if_level] && (pass == LAST_PASS))
						println();
					skip_lines = !if_state[if_level];
					if_level--;
					if (if_level == 0)
						in_if = 0;
					return;
				}
			}
		}
	}

	if (skip_lines)
		return;

	/* comment line */
	c = prlnbuf[preproc_sfield];
	if (c == ';' || c == '\0') {
		lastlabl = NULL;
		if (pass == LAST_PASS)
			println();
		return;
	}

	/* search for a symbol, either a label or an instruction */
	i = preproc_sfield;
	j = 0;
	while (isspace(prlnbuf[i]))
		i++;

	c = prlnbuf[i];

	if (isdigit(c)) {
		/* check for an SDCC local-symbol */
		if (sdcc_mode && (prlnbuf[i+5] == '$') && (prlnbuf[i+6] == ':')
			&& isdigit(prlnbuf[i+1]) && isdigit(prlnbuf[i+2])
			&& isdigit(prlnbuf[i+3]) && isdigit(prlnbuf[i+4])) {
			j = 6; c = ':';
		}
	} else
	while (isalnum(c) || (c == '_') || (c == '.') || (j == 0 && (c == '@' || c == '!'))) {
		c = prlnbuf[i + (++j)];
	}

	if ((j == 0) || ((i != preproc_sfield) && (c != ':'))) {
		/* either it is not a symbol, or it is a symbol that is */
		/* not in 1st column and that does not end with a ':' */
		i = preproc_sfield;
	}
	else {
		/* a symbol, either in the 1st column or ending with a ':' */
		j = i;
		if (!colsym(&i, 0))
			return;
		if ((do_label == 0) && (prlnbuf[i] != ':') && (stlook(SYM_CHK) == NULL)) {
			/* it doesn't end with a ':', and it isn't a symbol */
			/* that is recognized, so attempt to process it as  */
			/* an opcode (unless forced), so that both opcodes  */
			/* and pseudo-ops work properly in the 1st column   */
			i = preproc_sfield;
		} else {
			/* it either ends with a ':', it is already known as */
			/* a label or macro, or it failed the attempt to see */
			/* if it was a pseudo-op ... so it MUST be a label!  */
			c = symbol[1];
			if ((scopeptr != NULL) && (c != '.') && (c != '@') && (c != '!')) {
				/* adjust name to include full label-scope */
				i = j;
				if (!colsym(&i, 1))
					return;
			}
			if (prlnbuf[i] == ':')
				i++;
			if ((lablptr = stlook(SYM_DEF)) == NULL)
				return;
		}
	}

	/* skip spaces */
	while (isspace(prlnbuf[i]))
		i++;

	/* is it a macro? */
	ip = i;
	mptr = macro_look(&ip);
	if (mptr) {
		/* define label */
		labldef(LOCATION);

		/* output location counter */
		if (pass == LAST_PASS) {
			if (!asm_opt[OPT_MACRO])
				loadlc(loccnt + (page << 13), 0);
		}

		/* get macro args */
		if (!macro_getargs(ip))
			return;

		/* output line */
		if (pass == LAST_PASS)
			println();

		/* ok */
		mcntmax++;
		mcounter = mcntmax;
		expand_macro = 1;
		mlptr = mptr->line;
		return;
	}

	/* an instruction then */
	ip = i;
	flag = oplook(&ip);
	if (flag < 0) {
		if (flag == -1 && !do_label) {
			/* try again, maybe it's a label */
			assemble(1);
			return;
		}

		labldef(LOCATION);
		if (flag == -1)
			error("Unknown instruction!");
		if ((flag == -2) && (pass == LAST_PASS)) {
			if (lablptr)
				loadlc(loccnt, 0);
			println();
		}
		lastlabl = NULL;
		return;
	}

	/* generate code */
	if (opflg == PSEUDO)
		do_pseudo(&ip);
	else if (labldef(LOCATION) == -1)
		return;
	else {
		/* output infos */
		data_loccnt = loccnt;

		/* check if we are in the CODE section */
		if ((section_flags[section] & S_IS_CODE) == 0)
			fatal_error("Instructions not allowed in this section!");

		/* generate code */
		opproc(&ip);

		/* reset last label pointer */
		lastlabl = NULL;
	}
}


/* ----
 * oplook()
 * ----
 * operation code table lookup
 * return symbol length if found
 * return -1 on syntax error
 * return -2 if no symbol
 */

int
oplook(int *idx)
{
	struct t_opcode *ptr;
	char name[16];
	char c;
	int flag;
	int hash;
	int i;

	/* get instruction name */
	i = 0;
	opext = 0;
	flag = 0;
	hash = 0;

	for (;;) {
		c = toupper(prlnbuf[*idx]);
		if (c == ' ' || c == '\t' || c == '\0' || c == ';')
			break;
		if ((!isalnum(c)) && (c != '.') && (c != '*') && (c != '=') && (c != '{') && (c != '}'))
			return (-1);
		if (i == 15)
			return (-1);

		/* handle instruction extension */
		if ((c == '.') && (i != 0)) {
			if (flag)
				return (-1);
			flag = 1;
			(*idx)++;
			continue;
		}
		if (flag) {
			if (opext)
				return (-1);
			opext = c;
			(*idx)++;
			continue;
		}

		/* store char */
		name[i++] = c;
		hash += c;
		(*idx)++;

		/* break if single-character directive */
		if ((c == '*') || (c == '=') || (c == '{') || (c == '}'))
			break;
	}

	/* check extension */
	if (flag) {
		if ((opext != 'A') && (opext != 'H') && (opext != 'L') && (opext != 'Z'))
			return (-1);
	}

	/* end name string */
	name[i] = '\0';

	/* return if no instruction */
	if (i == 0)
		return (-2);

	/* search the instruction in the hash table */
	ptr = inst_tbl[hash & 0xFF];

	while (ptr) {
		if (name[0] == ptr->name[0] && !strcmp(name, ptr->name)) {
			opproc = ptr->proc;
			opflg = ptr->flag;
			opval = ptr->value;
			optype = ptr->type_idx;

			if (opext) {
				/* no extension for pseudos */
				if (opflg == PSEUDO)
					return (-1);
				/* extension valid only for these addressing modes */
				if (!(opflg & (IMM | ZP | ZP_X | ZP_Y | ABS | ABS_X | ABS_Y)))
					return (-1);
			}
			return (i);
		}
		ptr = ptr->next;
	}

	/* didn't find this instruction */
	return (-1);
}


/* ----
 * addinst()
 * ----
 * add a list of instructions to the instruction
 * hash table
 */

void
addinst(struct t_opcode *optbl)
{
	int hash;
	int len;
	int i;
	char *ptr;
	char c;

	if (optbl == NULL)
		return;

	/* parse list */
	while (optbl->name) {
		/* calculate instruction hash value */
		hash = 0;
		len = strlen(optbl->name);
		ptr = optbl->name;

		for (i = 0; i < len; i++) {
			c = *ptr++;
			hash += c;
		}

		hash &= 0xFF;

		/* insert the instruction in the hash table */
		optbl->next = inst_tbl[hash];
		inst_tbl[hash] = optbl;

		/* next instruction */
		optbl++;
	}
}


/* ----
 * check_eol()
 * ----
 * check the end of line for garbage
 */

int
check_eol(int *ip)
{
	while (isspace(prlnbuf[*ip]))
		(*ip)++;
	if (prlnbuf[*ip] == ';' || prlnbuf[*ip] == '\0')
		return (1);
	else {
		error("Syntax error!");
		return (0);
	}
}


/* ----
 * save_if_expr()
 * ----
 * store .if expression to check for .else/.endif mismatch
 */

void
save_if_expr(int *ip)
{
	int i, j;
	if_line[if_level] = slnum;
	if_txt[if_level][0] = '\0';
	i = *ip;
	while (isspace(prlnbuf[i])) { i++; }
	j = 0;
	while ((prlnbuf[i] != ';') && (prlnbuf[i] != '\0') && (j < 63)) {
		if_txt[if_level][j++] = prlnbuf[i++];
	}
	if (j != 0) {
		while (isspace(if_txt[if_level][--j])) {}
		if_txt[if_level][++j] = '\0';
	}
}


/* .if pseudo */

void
do_if(int *ip)
{
	labldef(LOCATION);

	/* save condition text */
	save_if_expr(ip);

	/* get expression */
	if_expr = 1;
	if (!evaluate(ip, ';', 0)) {
		if_expr = 0;
		return;
	}
	if_expr = 0;

	/* check for '.if' stack overflow */
	if (if_level == 255) {
		fatal_error("Too many nested IF/ENDIF!");
		return;
	}
	in_if = 1;
	if_level++;
	if_state[if_level] = !skip_lines;
	if (!skip_lines)
		skip_lines = if_flag[if_level] = value ? 0 : 1;

	if (pass == LAST_PASS) {
		loadlc(value, 1);
		println();
	}
}

/* .else pseudo */

void
do_else(int *ip)
{
	if (!in_if)
		fatal_error("Unexpected ELSE!");
}

/* .endif pseudo */

void
do_endif(int *ip)
{
	if (!in_if)
		fatal_error("Unexpected ENDIF!");
}

/* .ifdef/.ifndef pseudo */

void
do_ifdef(int *ip)
{
	labldef(LOCATION);

	/* skip spaces */
	while (isspace(prlnbuf[*ip]))
		(*ip)++;

	/* save condition text */
	save_if_expr(ip);

	/* get symbol */
	if (!colsym(ip, 0)) {
		error("Syntax error!");
		return;
	}
	if (!check_eol(ip))
		return;

	/* this check does not count as a refererence! */
	lablptr = stlook(SYM_CHK);

	/* is it undefined */
	if ((lablptr != NULL) && ((lablptr->type == IFUNDEF) || (lablptr->type == UNDEF)))
		lablptr = NULL;

	/* check for '.if' stack overflow */
	if (if_level == 255) {
		fatal_error("Too many nested IF/ENDIF!");
		return;
	}
	in_if = 1;
	if_level++;
	if_state[if_level] = !skip_lines;
	if (!skip_lines) {
		if (optype) {
			/* .ifdef */
			skip_lines = if_flag[if_level] = (lablptr == NULL) ? 1 : 0;
		}
		else {
			/* .ifndef */
			skip_lines = if_flag[if_level] = (lablptr == NULL) ? 0 : 1;
		}
	}

	if (pass == LAST_PASS) {
		loadlc(!skip_lines, 1);
		println();
	}
}
