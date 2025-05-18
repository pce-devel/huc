#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include "defs.h"
#include "externs.h"
#include "protos.h"

/* section types mask for pseudo_allowed */
#define IN_NONE		(1 << S_NONE)
#define IN_ZP		(1 << S_ZP)
#define IN_BSS		(1 << S_BSS)
#define IN_CODE		(1 << S_CODE)
#define IN_DATA		(1 << S_DATA)
#define IN_HOME		(1 << S_HOME)
#define IN_XDATA	(1 << S_XDATA)
#define IN_XINIT	(1 << S_XINIT)
#define IN_CONST	(1 << S_CONST)
#define IN_OSEG		(1 << S_OSEG)
#define ANYWHERE	(0xFFFF)

/* pseudo instructions section flag */
unsigned short pseudo_allowed[] = {
/* P_DB          */	IN_CODE + IN_HOME + IN_DATA + IN_ZP + IN_BSS + IN_CONST + IN_XINIT,
/* P_DW          */	IN_CODE + IN_HOME + IN_DATA + IN_ZP + IN_BSS + IN_CONST + IN_XINIT,
/* P_DD          */	IN_CODE + IN_HOME + IN_DATA + IN_ZP + IN_BSS + IN_CONST + IN_XINIT,
/* P_DS          */	IN_CODE + IN_HOME + IN_DATA + IN_ZP + IN_BSS + IN_CONST + IN_XINIT + IN_XDATA + IN_OSEG,
/* P_EQU         */	IN_CODE + IN_HOME + IN_DATA + IN_ZP + IN_BSS,
/* P_ORG         */	IN_CODE + IN_HOME + IN_DATA + IN_ZP + IN_BSS,
/* P_PAGE        */	IN_CODE + IN_HOME + IN_DATA,
/* P_BANK        */	IN_CODE + IN_HOME + IN_DATA,
/* P_INCBIN      */	IN_CODE + IN_HOME + IN_DATA,
/* P_INCLUDE     */	IN_CODE + IN_HOME + IN_DATA + IN_ZP + IN_BSS,
/* P_INCCHR      */	IN_CODE + IN_HOME + IN_DATA,
/* P_INCSPR      */	IN_CODE + IN_HOME + IN_DATA,
/* P_INCPAL      */	IN_CODE + IN_HOME + IN_DATA,
/* P_INCBAT      */	IN_CODE + IN_HOME + IN_DATA,
/* P_MACRO       */	IN_CODE + IN_HOME + IN_DATA + IN_ZP + IN_BSS,
/* P_ENDM        */	IN_CODE + IN_HOME + IN_DATA + IN_ZP + IN_BSS,
/* P_LIST        */	IN_CODE + IN_HOME + IN_DATA + IN_ZP + IN_BSS,
/* P_MLIST       */	IN_CODE + IN_HOME + IN_DATA + IN_ZP + IN_BSS,
/* P_NOLIST      */	IN_CODE + IN_HOME + IN_DATA + IN_ZP + IN_BSS,
/* P_NOMLIST     */	IN_CODE + IN_HOME + IN_DATA + IN_ZP + IN_BSS,
/* P_RSSET       */	IN_CODE + IN_HOME + IN_DATA + IN_ZP + IN_BSS,
/* P_RS          */	IN_CODE + IN_HOME + IN_DATA + IN_ZP + IN_BSS,
/* P_IF          */	IN_CODE + IN_HOME + IN_DATA + IN_ZP + IN_BSS,
/* P_ELSE        */	IN_CODE + IN_HOME + IN_DATA + IN_ZP + IN_BSS,
/* P_ENDIF       */	IN_CODE + IN_HOME + IN_DATA + IN_ZP + IN_BSS,
/* P_FAIL        */	IN_CODE + IN_HOME + IN_DATA + IN_ZP + IN_BSS,
/* P_ZP          */	IN_CODE + IN_HOME + IN_DATA + IN_ZP + IN_BSS,
/* P_BSS         */	IN_CODE + IN_HOME + IN_DATA + IN_ZP + IN_BSS,
/* P_CODE        */	IN_CODE + IN_HOME + IN_DATA + IN_ZP + IN_BSS,
/* P_DATA        */	IN_CODE + IN_HOME + IN_DATA + IN_ZP + IN_BSS,
/* P_DEFCHR      */	IN_CODE + IN_HOME + IN_DATA,
/* P_FUNC        */	IN_CODE + IN_HOME + IN_DATA + IN_ZP + IN_BSS,
/* P_IFDEF       */	IN_CODE + IN_HOME + IN_DATA + IN_ZP + IN_BSS,
/* P_IFNDEF      */	IN_CODE + IN_HOME + IN_DATA + IN_ZP + IN_BSS,
/* P_VRAM        */	IN_CODE + IN_HOME + IN_DATA,
/* P_PAL         */	IN_CODE + IN_HOME + IN_DATA,
/* P_DEFPAL      */	IN_CODE + IN_HOME + IN_DATA,
/* P_DEFSPR      */	IN_CODE + IN_HOME + IN_DATA,
/* P_INESPRG     */	IN_CODE + IN_HOME + IN_DATA + IN_ZP + IN_BSS,
/* P_INESCHR     */	IN_CODE + IN_HOME + IN_DATA + IN_ZP + IN_BSS,
/* P_INESMAP     */	IN_CODE + IN_HOME + IN_DATA + IN_ZP + IN_BSS,
/* P_INESMIR     */	IN_CODE + IN_HOME + IN_DATA + IN_ZP + IN_BSS,
/* P_OPT         */	IN_CODE + IN_HOME + IN_DATA + IN_ZP + IN_BSS,
/* P_INCTILE     */	IN_CODE + IN_HOME + IN_DATA,
/* P_INCBLK      */	IN_CODE + IN_HOME + IN_DATA,
/* P_INCMAP      */	IN_CODE + IN_HOME + IN_DATA,
/* P_MML         */	IN_CODE + IN_HOME + IN_DATA,
/* P_PROC        */	IN_CODE + IN_HOME,
/* P_ENDP        */	IN_CODE + IN_HOME + IN_DATA + IN_ZP + IN_BSS,
/* P_PGROUP      */	IN_CODE + IN_HOME,
/* P_ENDPG       */	IN_CODE + IN_HOME + IN_DATA + IN_ZP + IN_BSS,
/* P_CALL        */	IN_CODE + IN_HOME,
/* P_DWL         */	IN_CODE + IN_HOME + IN_DATA + IN_CONST + IN_XINIT,
/* P_DWH         */	IN_CODE + IN_HOME + IN_DATA + IN_CONST + IN_XINIT,
/* P_INCCHRPAL   */	IN_CODE + IN_HOME + IN_DATA,
/* P_INCSPRPAL   */	IN_CODE + IN_HOME + IN_DATA,
/* P_INCTILEPAL  */	IN_CODE + IN_HOME + IN_DATA,
/* P_CARTRIDGE   */	IN_CODE + IN_HOME + IN_DATA + IN_ZP + IN_BSS,
/* P_ALIGN       */	IN_CODE + IN_HOME + IN_DATA + IN_ZP + IN_BSS,
/* P_KICKC       */	ANYWHERE,
/* P_IGNORE      */	IN_CODE + IN_HOME + IN_DATA + IN_ZP + IN_BSS + IN_CONST + IN_XINIT,
/* P_SEGMENT     */	ANYWHERE,
/* P_LABEL       */	IN_CODE + IN_HOME + IN_DATA + IN_ZP + IN_BSS,
/* P_ENCODING    */	IN_CODE + IN_HOME + IN_DATA + IN_ZP + IN_BSS,
/* P_STRUCT      */	IN_CODE + IN_HOME + IN_DATA + IN_ZP + IN_BSS,
/* P_ENDS        */	IN_CODE + IN_HOME + IN_DATA + IN_ZP + IN_BSS,
/* P_3PASS       */	IN_CODE + IN_HOME + IN_DATA + IN_ZP + IN_BSS,
/* P_ALIAS       */	IN_CODE + IN_HOME + IN_DATA + IN_ZP + IN_BSS,
/* P_REF         */	IN_CODE + IN_HOME + IN_DATA + IN_ZP + IN_BSS,
/* P_PHASE       */	IN_CODE + IN_HOME,
/* P_DEBUG       */	ANYWHERE,
/* P_OUTBIN      */	ANYWHERE,
/* P_OUTPNG      */	ANYWHERE,
/* P_INCMASK     */	IN_CODE + IN_HOME + IN_DATA,
/* P_HALTMAP     */	IN_CODE + IN_HOME + IN_DATA,
/* P_MASKMAP     */	IN_CODE + IN_HOME + IN_DATA,
/* P_FLAGMAP     */	IN_CODE + IN_HOME + IN_DATA
};


/* ----
 * do_pseudo()
 * ----
 * pseudo instruction processor
 */

void
do_pseudo(int *ip)
{
	int old_bank;
	int size;

	/* check if the directive is allowed in the current section */
	if (!(pseudo_allowed[opval] & (1 << section))) {
		fatal_error("Directive not allowed in the %s section!", section_name[section]);
	}

	/* save current location */
	old_bank = bank;

	/* execute directive */
	opproc(ip);

	/* reset last label pointer */
	switch (opval) {
	case P_VRAM:
	case P_PAL:
		break;

	case P_DB:
	case P_DW:
	case P_DD:
	case P_DS:
	case P_DWL:
	case P_DWH:
		if (lastlabl) {
			if (lastlabl->data_type != P_DB)
				lastlabl = NULL;
		}
		break;

	default:
		if (lastlabl) {
			if (lastlabl->data_type != opval)
				lastlabl = NULL;
		}
		break;
	}

	/* bank overflow warning */
	if (pass == LAST_PASS) {
		if (asm_opt[OPT_WARNING]) {
			switch (opval) {
			case P_INCBIN:
			case P_INCCHR:
			case P_INCSPR:
			case P_INCPAL:
			case P_INCBAT:
			case P_INCTILE:
			case P_INCBLK:
			case P_INCMAP:
				if (bank != old_bank) {
					size = ((bank - old_bank - 1) * 8192) + loccnt;
					if (size) {
						warning("Bank overflow by %i bytes!\n", size);
					}
				}
				break;
			}
		}
	}
}


/* ----
 * do_list()
 * ----
 * .list pseudo
 */

void
do_list(int *ip)
{
	/* check end of line */
	if (!check_eol(ip))
		return;

	asm_opt[OPT_LIST] = 1;
	xlist = 1;
}


/* ----
 * do_mlist()
 * ----
 * .mlist pseudo
 */

void
do_mlist(int *ip)
{
	/* check end of line */
	if (!check_eol(ip))
		return;

	asm_opt[OPT_MACRO] = 1;
}


/* ----
 * do_nolist()
 * ----
 * .nolist pseudo
 */

void
do_nolist(int *ip)
{
	/* check end of line */
	if (!check_eol(ip))
		return;

	asm_opt[OPT_LIST] = 0;
}


/* ----
 * do_nomlist()
 * ----
 * .nomlist pseudo
 */

void
do_nomlist(int *ip)
{
	/* check end of line */
	if (!check_eol(ip))
		return;

	asm_opt[OPT_MACRO] = mlist_opt;
}


/* ----
 * do_db()
 * ----
 * .db    pseudo (optype == 0)
 * .text  pseudo (optype == 1)
 * .ascii pseudo (optype == 2)
 */

void
do_db(int *ip)
{
	unsigned char c;
	unsigned char h;

	/* define label */
	labldef(LOCATION);

	/* output infos */
	data_loccnt = loccnt;
	data_level = 2;

	/* get bytes */
	for (;;) {
		/* skip spaces */
		while (isspace(prlnbuf[++(*ip)])) {}

		/* ASCII string */
		if (prlnbuf[*ip] == '\"') {
			/* check for non-zero value in ZP or BSS sections */
			if (section == S_ZP || section == S_BSS) {
				error("Cannot store non-zero data in .ZP or .BSS sections!");
				return;
			}

			for (;;) {
				c = prlnbuf[++(*ip)];
				if (c == '\"')
					break;
				if (c == '\0') {
					error("Unterminated ASCII string!");
					return;
				}
				if (c == '\\') {
					c = prlnbuf[++(*ip)];
					switch (c) {

					case '\\':
						c = '\\';
						break;
					case '\"':
						c = '\"';
						break;
					case '\'':
						c = '\'';
						break;
					case '0':
						c = 0;
						break;
					case 'a':
						c = '\a';
						break;
					case 'b':
						c = '\b';
						break;
					case 'e':
						c = 0x1B;
						break;
					case 'f':
						c = '\f';
						break;
					case 'n':
						c = '\n';
						break;
					case 'r':
						c = '\r';
						break;
					case 't':
						c = '\t';
						break;
					case 'v':
						c = '\v';
						break;
					case 'x':
						c = prlnbuf[++(*ip)];

						if ((c >= '0') && (c <= '8'))
							h = (c - '0');
						else
						if ((c >= 'A') && (c <= 'F'))
							h = (c + 10 - 'A');
						else
						if ((c >= 'a') && (c <= 'f'))
							h = (c + 10 - 'a');
						else {
							error("Illegal character in hex escape sequence!");
							return;
						}

						for (;;) {
							c = prlnbuf[++(*ip)];

							if ((c >= '0') && (c <= '8'))
								h = (h << 4) + (c - '0');
							else
							if ((c >= 'A') && (c <= 'F'))
								h = (h << 4) + (c + 10 - 'A');
							else
							if ((c >= 'a') && (c <= 'f'))
								h = (h << 4) + (c + 10 - 'a');
							else {
								--(*ip);
								break;
							}
						}

						c = h;
						break;
					default:
						error("Illegal character in escape sequence!");
						return;
//						/* just pass it on, breaking the C standard */
//						break;
					}
				}
				/* store char on last pass */
				if (pass == LAST_PASS) {
					/* store character */
					putbyte(loccnt, c, DATA_OUT);
				}

				/* update location counter */
				loccnt++;
			}
			while (isspace(prlnbuf[++(*ip)])) {}
		}
		/* bytes */
		else {
			/* skip SDCC's junk at the start of some .db output */
			if (sdcc_mode && prlnbuf[*ip] == '#')
				(*ip)++;

			/* get a byte */
			if (!evaluate(ip, 0, 0))
				return;

			/* update location counter */
			loccnt++;

			/* store byte on last pass */
			if (pass == LAST_PASS) {
				/* check for non-zero value in ZP or BSS sections */
				if ((value != 0) && (section_flags[section] & S_NO_DATA)) {
					error("Cannot store non-zero data in .ZP or .BSS sections!");
					return;
				}

				/* check for overflow, except in SDCC code (-256..255 are ok) */
				/* SDCC's code generator assumes that the assembler doesn't care */
				if ((sdcc_mode == 0) && (value & ~0xFF) && ((value & ~0xFF) != ~0xFF)) {
					error("Operand too large to fit in a byte!");
					return;
				}

				/* store byte */
				putbyte(loccnt - 1, value, DATA_OUT);
			}
		}

		/* check if there's another byte */
		c = prlnbuf[*ip];

		if (c != ',')
			break;
	}

	/* check error */
	if (c != ';' && c != '\0') {
		error("Syntax error!");
		return;
	}

	/* size */
	if (lablptr) {
		lablptr->data_type = P_DB;
		lablptr->data_size = loccnt - data_loccnt;
	}
	else {
		if (lastlabl) {
			if (lastlabl->data_type == P_DB)
				lastlabl->data_size += loccnt - data_loccnt;
		}
	}

	/* output line */
	if (pass == LAST_PASS) {
		/* just output an address in S_ZP and S_BSS, else show the data */
		if (section_flags[section] & S_NO_DATA) {
			loadlc(data_loccnt, 0);
			data_loccnt = -1;
		}
		println();
	}
}


/* ----
 * do_dw()
 * ----
 * .dw pseudo
 */

void
do_dw(int *ip)
{
	char c;

	/* define label */
	labldef(LOCATION);

	/* output infos */
	data_loccnt = loccnt;
	data_size = 2;
	data_level = 2;

	/* get data */
	for (;;) {
		/* skip spaces */
		while (isspace(prlnbuf[*ip])) { ++(*ip); }

		/* skip SDCC's junk at the start of some .dw output */
		if (sdcc_mode && prlnbuf[*ip] == '#')
			(*ip)++;

		/* get a word */
		if (!evaluate(ip, 0, 0))
			return;

		/* update location counter */
		loccnt += 2;

		/* store word on last pass */
		if (pass == LAST_PASS) {
			/* check for non-zero value in ZP or BSS sections */
			if ((value != 0) && (section_flags[section] & S_NO_DATA)) {
				error("Cannot store non-zero data in .ZP or .BSS sections!");
				return;
			}

			/* check for overflow, except in SDCC code (-65536..65535 are ok) */
			/* SDCC's code generator assumes that the assembler doesn't care */
			if ((sdcc_mode == 0) && (value & ~0xFFFF) && ((value & ~0xFFFF) != ~0xFFFF)) {
				error("Operand too large to fit in a word!");
				return;
			}

			/* store word */
			putword(loccnt - 2, value, DATA_OUT);
		}

		/* check if there's another word */
		c = prlnbuf[(*ip)++];

		if (c != ',')
			break;
	}

	/* check error */
	if (c != ';' && c != '\0') {
		error("Syntax error!");
		return;
	}

	/* size */
	if (lablptr) {
		lablptr->data_type = P_DB;
		lablptr->data_size = loccnt - data_loccnt;
	}
	else {
		if (lastlabl) {
			if (lastlabl->data_type == P_DB)
				lastlabl->data_size += loccnt - data_loccnt;
		}
	}

	/* output line */
	if (pass == LAST_PASS) {
		/* just output an address in S_ZP and S_BSS, else show the data */
		if (section_flags[section] & S_NO_DATA) {
			loadlc(data_loccnt, 0);
			data_loccnt = -1;
		}
		println();
	}
}


/* ----
 * do_dwl()
 * ----
 * .dwl pseudo
 */

void
do_dwl(int *ip)
{
	char c;

	/* define label */
	labldef(LOCATION);

	/* output infos */
	data_loccnt = loccnt;
	data_size = 1;
	data_level = 2;

	/* get data */
	for (;;) {
		/* get a word */
		if (!evaluate(ip, 0, 0))
			return;

		/* update location counter */
		loccnt += 1;

		/* store word on last pass */
		if (pass == LAST_PASS) {
			/* check for non-zero value in ZP or BSS sections */
			if ((value != 0) && (section_flags[section] & S_NO_DATA)) {
				error("Cannot store non-zero data in .ZP or .BSS sections!");
				return;
			}

			/* check for overflow (-65536..65535 are ok) */
			if ((value & ~0xFFFF) && ((value & ~0xFFFF) != ~0xFFFF)) {
				error("Operand too large to fit in a word!");
				return;
			}

			/* store word */
			putbyte(loccnt - 1, (value & 0xff), DATA_OUT);
		}

		/* check if there's another word */
		c = prlnbuf[(*ip)++];

		if (c != ',')
			break;
	}

	/* check error */
	if (c != ';' && c != '\0') {
		error("Syntax error!");
		return;
	}

	/* size */
	if (lablptr) {
		lablptr->data_type = P_DB;
		lablptr->data_size = loccnt - data_loccnt;
	}
	else {
		if (lastlabl) {
			if (lastlabl->data_type == P_DB)
				lastlabl->data_size += loccnt - data_loccnt;
		}
	}

	/* output line */
	if (pass == LAST_PASS) {
		/* just output an address in S_ZP and S_BSS, else show the data */
		if (section_flags[section] & S_NO_DATA) {
			loadlc(data_loccnt, 0);
			data_loccnt = -1;
		}
		println();
	}
}


/* ----
 * do_dwh()
 * ----
 * .dwh pseudo
 */

void
do_dwh(int *ip)
{
	char c;

	/* define label */
	labldef(LOCATION);

	/* output infos */
	data_loccnt = loccnt;
	data_size = 1;
	data_level = 2;

	/* get data */
	for (;;) {
		/* get a word */
		if (!evaluate(ip, 0, 0))
			return;

		/* update location counter */
		loccnt += 1;

		/* store word on last pass */
		if (pass == LAST_PASS) {
			/* check for non-zero value in ZP or BSS sections */
			if ((value != 0) && (section_flags[section] & S_NO_DATA)) {
				error("Cannot store non-zero data in .ZP or .BSS sections!");
				return;
			}

			/* check for overflow (-65536..65535 are ok) */
			if ((value & ~0xFFFF) && ((value & ~0xFFFF) != ~0xFFFF)) {
				error("Operand too large to fit in a word!");
				return;
			}

			/* store word */
			putbyte(loccnt - 1, ((value >> 8) & 0xff), DATA_OUT);
		}

		/* check if there's another word */
		c = prlnbuf[(*ip)++];

		if (c != ',')
			break;
	}

	/* check error */
	if (c != ';' && c != '\0') {
		error("Syntax error!");
		return;
	}

	/* size */
	if (lablptr) {
		lablptr->data_type = P_DB;
		lablptr->data_size = loccnt - data_loccnt;
	}
	else {
		if (lastlabl) {
			if (lastlabl->data_type == P_DB)
				lastlabl->data_size += loccnt - data_loccnt;
		}
	}

	/* output line */
	if (pass == LAST_PASS) {
		/* just output an address in S_ZP and S_BSS, else show the data */
		if (section_flags[section] & S_NO_DATA) {
			loadlc(data_loccnt, 0);
			data_loccnt = -1;
		}
		println();
	}
}


/* ----
 * do_dd()
 * ----
 * .dd pseudo
 */

void
do_dd(int *ip)
{
	char c;

	/* define label */
	labldef(LOCATION);

	/* output infos */
	data_loccnt = loccnt;
	data_size = 4;
	data_level = 2;

	/* get data */
	for (;;) {
		/* get a word */
		if (!evaluate(ip, 0, 0))
			return;

		/* update location counter */
		loccnt += 4;

		/* store dword on last pass */
		if (pass == LAST_PASS) {
			/* check for non-zero value in ZP or BSS sections */
			if ((value != 0) && (section_flags[section] & S_NO_DATA)) {
				error("Cannot store non-zero data in .ZP or .BSS sections!");
				return;
			}

			/* store word */
			putdword(loccnt - 4, value);
		}

		/* check if there's another word */
		c = prlnbuf[(*ip)++];

		if (c != ',')
			break;
	}

	/* check error */
	if (c != ';' && c != '\0') {
		error("Syntax error!");
		return;
	}

	/* size */
	if (lablptr) {
		lablptr->data_type = P_DB;
		lablptr->data_size = loccnt - data_loccnt;
	}
	else {
		if (lastlabl) {
			if (lastlabl->data_type == P_DB)
				lastlabl->data_size += loccnt - data_loccnt;
		}
	}

	/* output line */
	if (pass == LAST_PASS) {
		/* just output an address in S_ZP and S_BSS, else show the data */
		if (section_flags[section] & S_NO_DATA) {
			loadlc(data_loccnt, 0);
			data_loccnt = -1;
		}
		println();
	}
}



/* ----
 * do_equ()
 * ----
 * .equ pseudo (optype == 0)
 * .set pseudo (optype == 1)
 */

void
do_equ(int *ip)
{
	/* check symbol */
	if (lablptr == NULL) {
		fatal_error("Label name missing from equate!");
		return;
	}
	if (lablptr->name[1] == '!') {
		fatal_error("A multi-label must be a location, not an equate!");
		return;
	}

	/* get value */
	if (!evaluate(ip, 0, 1))
		return;

	/* check for undefined symbols - they are only allowed in the FIRST_PASS */
	if (undef != 0) {
		if ((asm_opt[OPT_FORWARD] == 0) || (pass != FIRST_PASS))
			error("Undefined symbol in operand field!");
		else
			need_another_pass = 1;
		return;
	}

	/* assign value to the label */
	labldef(CONSTANT + optype); /* or VARIABLE */

	/* check for a 2nd parameter */
	if (prlnbuf[*ip] == ',') {
		/* skip spaces */
		while (isspace(prlnbuf[++(*ip)])) {}

		if (strncmp(&prlnbuf[*ip], "CODE", 4) == 0 ||
		    strncmp(&prlnbuf[*ip], "code", 4) == 0) {
			*ip += 4;
			lablptr->data_type = -1;
			lablptr->data_size = 0;
			lablptr->flags |= FLG_CODE;
		}
		else
		if (strncmp(&prlnbuf[*ip], "FUNC", 4) == 0 ||
		    strncmp(&prlnbuf[*ip], "func", 4) == 0) {
			*ip += 4;
			lablptr->data_type = -1;
			lablptr->data_size = 0;
			lablptr->flags |= FLG_CODE + FLG_FUNC;
		}
		else {
			if (!evaluate(ip, 0, 1))
				return;
			lablptr->data_type = P_DB;
			lablptr->data_size = value;
		}
	}

	/* check end of line */
	if (!check_eol(ip))
		return;

	/* output line */
	if (pass == LAST_PASS) {
		loadlc(value, 1);
		println();
	}
}


/* ----
 * do_page()
 * ----
 * .page pseudo
 */

void
do_page(int *ip)
{
	/* not allowed while .phase is active */
	if (phase_offset) {
		fatal_error(".PAGE cannot be changed within a .PHASE'd chunk of code!");
		return;
	}

	/* not allowed in procs */
	if (proc_ptr && (section_flags[section] & S_IS_CODE)) {
		fatal_error("Code .PAGE cannot be changed within a .PROC!");
		return;
	}

	/* define label */
	labldef(LOCATION);

	/* get page index */
	if (!evaluate(ip, ';', 0))
		return;
	if (value > 7) {
		error("Invalid page index!");
		return;
	}
	page = value;

	/* output line */
	if (pass == LAST_PASS) {
		loadlc(loccnt + (page << 13), 1);
		println();
	}
}


/* ----
 * do_org()
 * ----
 * .org pseudo
 */

void
do_org(int *ip)
{
	/* not allowed while .phase is active */
	if (phase_offset) {
		fatal_error(".ORG cannot be changed within a .PHASE'd chunk of code!");
		return;
	}

	/* get the .org value */
	if (!evaluate(ip, ';', 0))
		return;

	/* check for undefined symbol - they are not allowed in .org */
	if (undef != 0) {
		error("Undefined symbol in operand field!");
		return;
	}

	/* section switch */
	switch (section) {
	case S_ZP:
		/* zero page section (accept traditional 6502 zero-page) */
		if ((value & ~0xFF) && ((value & ~0xFF) != machine->ram_base)) {
			error("Invalid address!");
			return;
		}
		break;

	case S_BSS:
		/* ram section */
		if ((value < machine->ram_base) || (value >= (machine->ram_base + machine->ram_limit))) {
			error("Invalid address!");
			return;
		}
		break;

	case S_CODE:
	case S_DATA:
		/* not allowed in procs */
		if (proc_ptr && (section_flags[section] & S_IS_CODE)) {
			fatal_error("Code .ORG cannot be changed within a .PROC!");
			return;
		}

		/* code and data section */
		if (value & ~0xFFFF) {
			error("Invalid address!");
			return;
		}
		page = (value >> 13) & 7;
		break;
	}

	/* set location counter */
	loccnt = (value & 0x1FFF);

	/* signal discontiguous change in loccnt */
	discontiguous = 1;

	/* set label value if there was one */
	labldef(LOCATION);

	/* output line on last pass */
	if (pass == LAST_PASS) {
		loadlc(value, 1);
		println();
	}
}


/* ----
 * do_bank()
 * ----
 * .bank pseudo
 */

void
do_bank(int *ip)
{
	char name[128];

	/* not allowed while .phase is active */
	if (phase_offset) {
		fatal_error(".BANK cannot be changed within a .PHASE'd chunk of code!");
		return;
	}

	/* not allowed in procs */
	if (proc_ptr && (section_flags[section] & S_IS_CODE)) {
		fatal_error("Code .BANK cannot be changed within a .PROC!");
		return;
	}

	/* define label */
	labldef(LOCATION);

	/* get bank index */
	if (!evaluate(ip, 0, 0))
		return;

	/* check for undefined symbol - they are not allowed in .bank */
	if (undef != 0) {
		error("Undefined symbol in operand field!");
		return;
	}

	if (value > (unsigned)bank_limit) {
		error("Bank index out of range!");
		return;
	}

	/* check if there's a bank name */
	switch (prlnbuf[*ip]) {
	case ';':
	case '\0':
		break;

	case ',':
		/* get name */
		(*ip)++;
		if (!getstring(ip, name, 63))
			return;

		/* check name validity */
		if (strlen(bank_name[value])) {
			if (strcasecmp(bank_name[value], name)) {
				error("Different bank names not allowed!");
				return;
			}
		}

		/* copy name */
		strcpy(bank_name[value], name);

		/* check end of line */
		if (!check_eol(ip))
			return;

		/* ok */
		break;

	default:
		error("Syntax error!");
		return;
	}

	/* backup current bank infos */
	bank_glabl[section][bank] = glablptr;
	bank_loccnt[section][bank] = loccnt;
	bank_page[section][bank] = page;

	/* get new bank infos */
	bank = value;
	page = bank_page[section][bank];
	loccnt = bank_loccnt[section][bank];
	glablptr = bank_glabl[section][bank];

	/* signal discontiguous change in loccnt */
	discontiguous = 1;

	/* update the max bank counter */
	if (max_bank < bank)
		max_bank = bank;

	/* output on last pass */
	if (pass == LAST_PASS) {
		loadlc(bank, 1);
		println();
	}
}


/* ----
 * do_incbin()
 * ----
 * .incbin pseudo
 */

void
do_incbin(int *ip)
{
	FILE *fp;
	char *p;
	char fname[PATHSZ];
	int size;
	int step;
	int offset =  0;
	int length = -1;

	/* get file name */
	if (!getstring(ip, fname, PATHSZ - 1))
		return;

	/* get file extension */
	if ((p = strrchr(fname, '.')) != NULL) {
		if (!strchr(p, PATH_SEPARATOR)) {
			/* check if it's a mx file */
			if (!strcasecmp(p, ".mx")) {
				do_mx(fname);
				return;
			}
			/* check if it's a map file */
			if (!strcasecmp(p, ".fmp")) {
				if (pce_load_map(fname, 0))
					return;
			}
			/* check if it's a stm file */
			if (!strcasecmp(p, ".stm")) {
				if (pce_load_stm(fname, 0))
					return;
			}
		}
	}

	/* get the optional offset and length */
	if (prlnbuf[*ip] == ',') {
		/* get the offset */
		++(*ip);
		if (!evaluate(ip, 0, 0))
			return;

		if (undef != 0) {
			error("Undefined symbol in offset field!");
			return;
		}

		if (0 > (int)value) {
			error(".INCBIN offset cannot be negative!");
			return;
		}
		offset = value;

		if (prlnbuf[*ip] == ',') {
			/* get a byte */
			++(*ip);
			if (!evaluate(ip, 0, 0))
				return;

			if (undef != 0) {
				error("Undefined symbol in length field!");
				return;
			}

			if (0 > (int)value) {
				error(".INCBIN length cannot be negative!");
				return;
			}
			length = value;
		}
	}

	/* check end of line */
	if (!check_eol(ip))
		return;

	/* define label */
	labldef(LOCATION);

	/* output */
	if (pass == LAST_PASS)
		loadlc(loccnt, 0);

	/* open file */
	if ((fp = open_file(fname, "rb")) == NULL) {
		fatal_error("Unable to open file!");
		return;
	}

	/* get file size */
	fseek(fp, 0, SEEK_END);
	size = ftell(fp) - offset;

	if (size < 0) {
		fclose(fp);
		error(".INCBIN offset is greater than the file's length!");
		return;
	}

	if (length >= 0) {
		if (length > size) {
			fclose(fp);
			error(".INCBIN length is greater than the file's length!");
			return;
			}
		size = length;
	}

	/* seek to the file offset */
	fseek(fp, offset, SEEK_SET);

	/* check if it will fit in the rom */
	if ((section_flags[section] & S_IS_ROM) && (bank < UNDEFINED_BANK)) {
		/* check if it will fit in the rom */
		if (((bank << 13) + loccnt + size) > rom_limit) {
			fclose(fp);
			error("ROM overflow!");
			return;
		}

		/* load data on last pass */
		if (pass == LAST_PASS) {
			uint32_t info, *fill_a;
			uint8_t *fill_b;

			fread(&rom[bank][loccnt], 1, size, fp);

			if (section == S_DATA && asm_opt[OPT_DATAPAGE] != 0)
				memset(&map[bank][loccnt], section + (page << 5), size);
			else {
				int addr = (bank << 13) + loccnt;
				int temp = (page + (loccnt >> 13)) & 7;
				int left = size;

				while (left != 0) {
					step = 0x2000 - (addr & 0x1FFF);
					if (step > left) { step = left; }
					memset(&map[0][0] + addr, section + (temp << 5), step);
					temp = (temp + 1) & 7;
					addr += step;
					left -= step;
				}
			}

			info = debug_info(DATA_OUT);
			fill_a = &dbg_info[bank][loccnt];
			fill_b = &dbg_column[bank][loccnt];
			step = size;
			while (step--) {
				*fill_a++ = info;
				*fill_b++ = debug_column;
			}
			/* output line */
			println();
		}
	} else {
		if ((loccnt + size) > section_limit[section]) {
			fclose(fp);
			fatal_error("Too large to fit in the current section!");
			return;
		}
	}

	/* close file */
	fclose(fp);

	/* update bank and location counters */
	step = (loccnt + size) >> 13;
	bank = (bank + step);
	if (section != S_DATA || asm_opt[OPT_DATAPAGE] == 0)
		page = (page + step) & 7;

	loccnt = (loccnt + size) & 0x1FFF;

	if (loccnt == 0 && step != 0) {
		loccnt = 0x2000;
		bank = (bank - 1);
		if (section != S_DATA || asm_opt[OPT_DATAPAGE] == 0)
			page = (page - 1) & 7;
	}

	/* update rom size */
	if ((section_flags[section] & S_IS_ROM) && (bank < UNDEFINED_BANK)) {
		if (bank > max_bank) {
			if (loccnt)
				max_bank = bank;
			else
				max_bank = bank - 1;
		}
	}

	/* attach the size of the data the label */
	if (lablptr) {
		lablptr->data_type = P_INCBIN;
		lablptr->data_size = size;
		lablptr->data_count = 1;
	}
	else
	if (lastlabl && lastlabl->data_type == P_INCBIN) {
		lastlabl->data_size += size;
		lastlabl->data_count += 1;
	}

	/* set the size after conversion in the last pass */
	if (lastlabl && lastlabl->data_type == P_INCBIN) {
		if (pass == LAST_PASS)
			lastlabl->size = 1;
	}
}


/* ----
 * do_mx()
 * ----
 * load a mx file
 */

void
do_mx(char *fname)
{
	FILE *fp;
	char *ptr;
	char type;
	char line[256];
	unsigned char buffer[128];
	int data;
	int flag = 0;
	int size = 0;
	int cnt, addr, chksum;
	int i;

	/* open the file */
	if ((fp = open_file(fname, "r")) == NULL) {
		fatal_error("Unable to open file!");
		return;
	}

	/* read loop */
	while (fgets(line, 254, fp) != NULL) {
		if (line[0] == 'S') {
			/* get record type */
			type = line[1];

			/* error on unsupported records */
			if ((type != '2') && (type != '8')) {
				error("Unsupported S-record type!");
				return;
			}

			/* get count and address */
			cnt = htoi(&line[2], 2);
			addr = htoi(&line[4], 6);

			if ((strlen(line) < 12) || (cnt < 4) || (addr == -1)) {
				error("Incorrect S-record line!");
				return;
			}

			/* adjust count */
			cnt -= 4;

			/* checksum */
			chksum = cnt + ((addr >> 16) & 0xFF) +
				 ((addr >> 8) & 0xFF) +
				 ((addr) & 0xFF) + 4;

			/* get data */
			ptr = &line[10];

			for (i = 0; i < cnt; i++) {
				data = htoi(ptr, 2);
				buffer[i] = data;
				chksum += data;
				ptr += 2;

				if (data == -1) {
					error("Syntax error in a S-record line!");
					return;
				}
			}

			/* checksum test */
			data = htoi(ptr, 2);
			chksum = (~chksum) & 0xFF;

			if (data != chksum) {
				error("Checksum error!");
				return;
			}

			/* end record */
			if (type == '8')
				break;

			/* data record */
			if (type == '2') {
				/* set the location counter */
				if (addr & 0xFFFF0000) {
					error("Invalid address!");
					return;
				}
				page = (addr >> 13) & 0x07;
				loccnt = (addr & 0x1FFF);

				/* define label */
				if (flag == 0) {
					flag = 1;
					labldef(LOCATION);

					/* output */
					if (pass == LAST_PASS)
						loadlc(loccnt, 0);
				}

				/* copy data */
				if (pass == LAST_PASS) {
					for (i = 0; i < cnt; i++)
						putbyte(loccnt + i, buffer[i], DATA_OUT);
				}

				/* update location counter */
				loccnt += cnt;
				size += cnt;
			}
		}
	}

	/* close file */
	fclose(fp);

	/* define label */
	if (flag == 0) {
		labldef(LOCATION);

		/* output */
		if (pass == LAST_PASS)
			loadlc(loccnt, 0);
	}

	/* size */
	if (lablptr) {
		lablptr->data_type = P_INCBIN;
		lablptr->data_size = size;
	}
	else
	if (lastlabl && lastlabl->data_type == P_INCBIN) {
		lastlabl->data_size += size;
	}

	/* output line */
	if (pass == LAST_PASS)
		println();
}


/* ----
 * do_include()
 * ----
 * .include pseudo
 */

void
do_include(int *ip)
{
	char fname[PATHSZ];

	/* define label */
	labldef(LOCATION);

#if 0 // This breaks @turboxray's code, so disable it for now.
	/* avoid problems */
	if (expand_macro) {
		error("Cannot use .INCLUDE inside a macro!");
		return;
	}
#endif

	/* if this is an SDCC .module pseudo-op then auto-include sdcc.asm */
	if (optype == 1) {
		/* ignore the module name and everything else on the line */
		strcpy(fname, "sdcc.asm");
	} else {
		/* get file name */
		if (!getstring(ip, fname, PATHSZ - 1))
			return;

		/* check end of line */
		if (!check_eol(ip))
			return;
	}

	/* open file */
	if (open_input(fname) == -1) {
		fatal_error("Unable to open file!");
		return;
	}

	/* output line */
	if (pass == LAST_PASS)
		println();
}


/* ----
 * do_rsset()
 * ----
 * .rsset pseudo
 */

void
do_rsset(int *ip)
{
	/* define label */
	labldef(LOCATION);

	/* get value */
	if (!evaluate(ip, ';', 1))
		return;
	if (value & ~0xFFFF) {
		error("Address out of range!");
		return;
	}

	/* set 'rs' base and bank */
	rs_base = value;
	rs_mprbank = expr_mprbank;
	rs_overlay = expr_overlay;

	/* output line */
	if (pass == LAST_PASS) {
		loadlc(rs_base, 1);
		println();
	}
}


/* ----
 * do_rs()
 * ----
 * .rs pseudo
 */

void
do_rs(int *ip)
{
	int old_rs = rs_base;

	/* define label */
	value = rs_base;
	expr_mprbank = rs_mprbank;
	expr_overlay = rs_overlay;
	labldef(CONSTANT);

	/* get the number of bytes to reserve */
	if (!evaluate(ip, ';', 0))
		return;

	/* ouput line */
	if (pass == LAST_PASS) {
		loadlc(rs_base, 1);
		println();
	}

	/* update 'rs' base */
	rs_base += value;
	if (rs_base & ~0xFFFF)
		error("Address out of range!");

	/* update 'rs' bank */
	if (rs_mprbank != UNDEFINED_BANK) {
		while ((old_rs & 0xE000) != (rs_base & 0xE000)) {
			old_rs += 0x2000;
			++rs_mprbank;
		}
	}
}


/* ----
 * do_ds()
 * ----
 * .ds pseudo
 */

void
do_ds(int *ip)
{
	int addr;
	int step;
	int nbytes;
	unsigned int filler = 0;
	unsigned char c;

	/* define label */
	labldef(LOCATION);

	/* output infos */
	data_loccnt = loccnt;
	data_level = 2;

	/* get the number of bytes to reserve */
	if (!evaluate(ip, 0, 0))
		return;

	/* check for undefined symbol - they are not allowed in .ds */
	if (undef != 0) {
		if ((asm_opt[OPT_FORWARD] == 0) || (pass != FIRST_PASS))
			error("Undefined symbol in operand field!");
		else
			need_another_pass = 1;
		return;
	}

	/* check for negative value */
	if (value > INT_MAX) {
		error("Negative value in operand field!");
		return;
	}

	nbytes = value;

	c = prlnbuf[(*ip)++];

	/* check if there's another word */
	if (c == ',') {
		/* get the filler byte */
		if (!evaluate(ip, 0, 0))
			return;

		filler = value & 255;

		c = prlnbuf[(*ip)++];
	}

	/* check error */
	if (c != ';' && c != '\0') {
		error("Syntax error!");
		return;
	}

	/* check range */
	addr = loccnt + nbytes;

	if ((section_flags[section] & S_IS_ROM) && (bank < UNDEFINED_BANK)) {
		if (((bank << 13) + addr) > rom_limit) {
			error("ROM overflow!");
			return;
		}
	}
	else
	if (addr > section_limit[section]) {
		error("The .DS is too large for the current bank or section!");
		return;
	}

	/* update max counter for zp and bss sections */
	switch (section) {
	case S_ZP:
		/* zero page */
		if (addr > max_zp)
			max_zp = addr;
		break;

	case S_BSS:
		/* ram page */
		if (addr > max_bss)
			max_bss = addr;
		break;
	}

	/* output line on last pass */
	if (pass == LAST_PASS) {
		uint32_t info, *fill_a;
		uint8_t *fill_b;

		if (filler != 0) {
			if (section == S_ZP)
				error("Cannot fill .ZP section with non-zero data!");
			else
			if (section == S_BSS)
				error("Cannot fill .BSS section with non-zero data!");
		}

		if (section_flags[section] & S_IS_ROM) {
			memset(&rom[bank][loccnt], filler, nbytes);

			if (section == S_DATA && asm_opt[OPT_DATAPAGE] != 0)
				memset(&map[bank][loccnt], section + (page << 5), nbytes);
			else {
				int addr = (bank << 13) + loccnt;
				int temp = (page + (loccnt >> 13)) & 7;
				int left = nbytes;

				while (left != 0) {
					step = 0x2000 - (addr & 0x1FFF);
					if (step > left) { step = left; }
					memset(&map[0][0] + addr, section + (temp << 5), step);
					temp = (temp + 1) & 7;
					addr += step;
					left -= step;
				}
			}

			info = debug_info(DATA_OUT);
			fill_a = &dbg_info[bank][loccnt];
			fill_b = &dbg_column[bank][loccnt];
			step = nbytes;
			while (step--) {
				*fill_a++ = info;
				*fill_b++ = debug_column;
			}
		}
	}

	/* update location counter */
	step = (loccnt + nbytes) >> 13;
	bank = (bank + step);
	if (section != S_DATA || asm_opt[OPT_DATAPAGE] == 0)
		page = (page + step) & 7;

	loccnt = (loccnt + nbytes) & 0x1FFF;

	if (loccnt == 0 && step != 0) {
		loccnt = 0x2000;
		bank = (bank - 1);
		if (section != S_DATA || asm_opt[OPT_DATAPAGE] == 0)
			page = (page - 1) & 7;
	}

	/* update rom size */
	if ((section_flags[section] & S_IS_ROM) && (bank < UNDEFINED_BANK)) {
		if (bank > max_bank) {
			if (loccnt)
				max_bank = bank;
			else
				max_bank = bank - 1;
		}
	}

	/* size */
	if (lablptr) {
		lablptr->data_type = P_DB;
		lablptr->data_size = loccnt - data_loccnt;
	}
	else
	if (lastlabl && lastlabl->data_type == P_DB) {
		lastlabl->data_size += loccnt - data_loccnt;
	}

	/* output line */
	if (pass == LAST_PASS) {
		/* just output an address in S_ZP and S_BSS, else show the data */
		if (section == S_ZP || section == S_BSS) {
			loadlc(data_loccnt, 0);
			data_loccnt = -1;
		}
		println();
	}
}


/* ----
 * do_fail()
 * ----
 * .fail pseudo
 */

void
do_fail(int *ip)
{
	fatal_error("Compilation failed!");
}


/* ----
 * do_section()
 * ----
 * .zp/.bss/.home/.code/.data pseudo
 */

void
do_section(int *ip)
{
	set_section(optype);

	/* output line */
	if (pass == LAST_PASS) {
		loadlc(loccnt + (page << 13), 1);
		println();
	}
}


/* ----
 * do_incchr()
 * ----
 * .inchr pseudo - convert a PCX to 8x8 character tiles
 *
 * .incchr "filename" [, x, y [, w, h ]] [, optimize]
 */

void
do_incchr(int *ip)
{
	int i, j;
	int x, y, w, h;
	int tx, ty;
	int nb_tile = 0;
	int total = 0;
	int size = (machine->type == MACHINE_PCE) ? 32 : 16;
	unsigned char optimize;
	unsigned int crc;
	unsigned int hash;
	unsigned char tile_data[32];

	/* define label */
	labldef(LOCATION);

	/* allocate memory for the symbol's tags */
	if (lablptr && lablptr->tags == NULL) {
		if ((lablptr->tags = calloc(1, sizeof(t_tags))) == NULL) {
			error("Cannot allocate memory for tags!");
			return;
		}
	}

	/* output */
	if (pass == LAST_PASS)
		loadlc(loccnt, 0);

	/* get args */
	if (!pcx_get_args(ip))
		return;

	/* odd number of args after filename if there is an "optimize" flag */
	optimize = 0;
	if ((pcx_nb_args & 1) != 0) {
		optimize = (pcx_arg[pcx_nb_args - 1] != 0);
		--pcx_nb_args;
	}

	/* set up x, y, w, h from the args */
	if (!pcx_parse_args(0, pcx_nb_args, &x, &y, &w, &h, 8))
		return;

	/* shortcut if we already know how much data is produced */
	if (pass == EXTRA_PASS && lastlabl && lastlabl->data_type == P_INCCHR) {
		/* output the cumulative size from the first .incchr of a set */
		if (lastlabl && lastlabl == lablptr)
			putbuffer(workspace, lablptr->data_size);
		return;
	}

	/* are we expanding a set of characters that was just created? */
	if (lablptr == NULL && lastlabl != NULL && lastlabl->data_type == P_INCCHR) {
		nb_tile = lastlabl->data_count;
	} else {
		tile_lablptr = lablptr;
		if (lablptr)
			tile_offset = lablptr->value;
		memset(tile_tbl, 0, sizeof(tile_tbl));
	}

	/* pack data */
	for (i = 0; i < h; i++) {
		for (j = 0; j < w; j++) {
			/* tile coordinates */
			tx = x + (j << 3);
			ty = y + (i << 3);

			/* get tile */
			pcx_pack_8x8_tile(tile_data, tx, ty);

			/* calculate tile crc */
			crc = crc_calc(tile_data, size);
			hash = crc & (HASH_COUNT - 1);

			if (optimize) {
				/* search tile */
				t_tile *test_tile = tile_tbl[hash];
				while (test_tile) {
					if (test_tile->crc == crc &&
					    memcmp(test_tile->data, tile_data, size) == 0)
						break;
					test_tile = test_tile->next;
				}

				/* ignore repeated tiles */
				if (test_tile) {
					continue;
				}
			}

			/* insert the new tile in the tile table */
			if (nb_tile == (sizeof(tile) / sizeof(struct t_tile)))
				continue;

			tile[nb_tile].next = tile_tbl[hash];
			tile[nb_tile].index = nb_tile;
			tile[nb_tile].data = &rom[bank][loccnt];
			tile[nb_tile].crc = crc;
			tile_tbl[hash] = &tile[nb_tile];

			/* putbuffer only copies on the LAST_PASS and we really need it */
			if (pass != LAST_PASS && !stop_pass) {
				memcpy(tile[nb_tile].data, tile_data, size);
			}

			putbuffer(tile_data, size);

			/* store tile */
			nb_tile += 1;
			total += size;
		}
	}

	/* attach the number of loaded characters to the label */
	if (lablptr) {
		lablptr->data_type = P_INCCHR;
		lablptr->data_size = total;
		lablptr->data_count = nb_tile;
	}
	else
	if (lastlabl && lastlabl->data_type == P_INCCHR) {
		lastlabl->data_size += total;
		lastlabl->data_count = nb_tile;
	}

	/* set the size after conversion in the last pass */
	if (lastlabl && lastlabl->data_type == P_INCCHR) {
		if (pass == LAST_PASS)
			lastlabl->size = (machine->type == MACHINE_PCE) ? 32 : 16;
	}

	/* output */
	if (pass == LAST_PASS)
		println();
}


/* ----
 * do_opt()
 * ----
 * .opt pseudo - compilation options
 */

void
do_opt(int *ip)
{
	char c;
	char flag;
	char name[32];
	int i;

	for (;;) {
		/* skip spaces */
		while (isspace(prlnbuf[*ip]))
			(*ip)++;

		/* get char */
		c = prlnbuf[(*ip)];

		/* extract option */
		i = 0;
		while (!isspace(c) && (c != ',') && (c != ';') && (c != '\0')) {
			if (i > 31) {
				error("Syntax error!");
				return;
			}
			name[i++] = c;
			c = prlnbuf[++(*ip)];
		}

		/* get option flag */
		flag = (i != 0) ? name[--i] : '\0';
		name[i] = '\0';

		/* set option */
		if (flag == '+')
			i = 1;
		else if (flag == '-')
			i = 0;
		else {
			error("Syntax error!");
			return;
		}

		/* search option */
		if (!strcasecmp(name, "l"))
			asm_opt[OPT_LIST] = i;
		else if (!strcasecmp(name, "m"))
			asm_opt[OPT_MACRO] = i;
		else if (!strcasecmp(name, "w"))
			asm_opt[OPT_WARNING] = i;
		else if (!strcasecmp(name, "o"))
			asm_opt[OPT_OPTIMIZE] = i;
		else if (!strcasecmp(name, "c"))
			asm_opt[OPT_CCOMMENT] = i;
		else if (!strcasecmp(name, "i"))
			asm_opt[OPT_INDPAREN] = i;
		else if (!strcasecmp(name, "a"))
			asm_opt[OPT_ZPDETECT] = i;
		else if (!strcasecmp(name, "b"))
			asm_opt[OPT_LBRANCH] = i;
		else if (!strcasecmp(name, "d"))
			asm_opt[OPT_DATAPAGE] = i;
		else if (!strcasecmp(name, "f"))
			asm_opt[OPT_FORWARD] = i;
		else {
			error("Unknown option!");
			return;
		}

		/* skip spaces */
		while (isspace(prlnbuf[*ip]))
			(*ip)++;

		/* get char */
		c = prlnbuf[(*ip)++];

		/* end of line */
		if (c == ';' || c == '\0')
			break;

		/* skip comma */
		if (c != ',') {
			error("Syntax error!");
			return;
		}
	}

	/* output */
	if (pass == LAST_PASS)
		println();
}


/* ----
 * do_align()
 * ----
 * .align pseudo
 */

void
do_align(int *ip)
{
	int offset;

	/* not allowed while .phase is active - but could be implemented if needed */
	if (phase_offset) {
		fatal_error("Cannot .ALIGN within a .PHASE'd chunk of code!");
		return;
	}

	/* get the .align value */
	if (!evaluate(ip, ';', 0))
		return;

	/* check for undefined symbol - they are not allowed in .align */
	if (undef != 0) {
		error("Undefined symbol in operand field!");
		return;
	}

	/* check for power-of-two, 1 bank maximum */
	if ((value > 8192) || (value == 0) || ((value & (value - 1)) != 0)) {
		error(".ALIGN value must be a power-of-two, with a maximum of 8192!");
		return;
	}

	/* did the previous instruction fill up the current bank? */
	if (loccnt >= 0x2000) {
		loccnt &= 0x1FFF;
		bank = (bank + 1);
		if (section != S_DATA || asm_opt[OPT_DATAPAGE] == 0)
			page = (page + 1) & 7;
	}

	/* are we already aligned to the request boundary? */
	if ((offset = loccnt & (value - 1)) != 0) {
		/* update location counter */
		int oldloc = loccnt;
		loccnt = (loccnt + value - offset) & 0x1fff;

		if (loccnt == 0) {
			/* signal discontiguous change in loccnt */
			discontiguous = 1;
			bank = (bank + 1);
			if (section != S_DATA || asm_opt[OPT_DATAPAGE] == 0)
				page = (page + 1) & 7;
		} else {
			if ((section_flags[section] & S_IS_ROM) && (bank < UNDEFINED_BANK)) {
				uint32_t info, *fill_a;
				uint8_t *fill_b;

				memset(&rom[bank][oldloc], 0, loccnt - oldloc);
				memset(&map[bank][oldloc], section + (page << 5), loccnt - oldloc);

				info = debug_info(DATA_OUT);
				fill_a = &dbg_info[bank][oldloc];
				fill_b = &dbg_column[bank][oldloc];
				offset = loccnt - oldloc;
				while (offset--) {
					*fill_a++ = info;
					*fill_b++ = debug_column;
				}
			}
		}
	}

	/* set label value if there was one */
	labldef(LOCATION);

	/* output line on last pass */
	if (pass == LAST_PASS) {
		loadlc(loccnt + (page << 13), 1);
		println();
	}
}


/* ----
 * do_3pass()
 * ----
 * .3pass  pseudo (optype == 0)
 */

void
do_3pass(int *ip)
{
	/* define label */
	labldef(LOCATION);

	/* check end of line */
	if (optype == 0 && !check_eol(ip))
		return;

	/* signal that an extra pass is wanted */
	if (pass == FIRST_PASS)
		need_another_pass = 1;

	/* enable forward-reference support in 3pass mode */
	asm_opt[OPT_FORWARD] = 1;

	/* output line */
	if (pass == LAST_PASS)
		println();
}


/* ----
 * do_kickc()
 * ----
 * .pceas  pseudo (optype == 0)
 * .kickc  pseudo (optype == 1) (for KickC code)
 * .r6502  pseudo (optype == 2) (for compatibility with SDCC)
 * .r65c02 pseudo (optype == 2) (for compatibility with SDCC)
 */

void
do_kickc(int *ip)
{
	/* define label */
	labldef(LOCATION);

	/* check end of line */
	if (!check_eol(ip))
		return;

	/* enable/disable KickC, SDCC or HuCC mode */
	kickc_mode = (optype & 1) >> 0;
	sdcc_mode  = (optype & 2) >> 1;
	hucc_mode  = (optype & 4) >> 2;

	/* signal to include final.asm, but not if already inside final.asm */
	if (!in_final) {
		kickc_final |= kickc_mode;
		hucc_final  |= (sdcc_mode | hucc_mode);
	}

	/* enable () for indirect addressing during KickC code */
	asm_opt[OPT_INDPAREN] = kickc_mode;

	/* enable auto-detect ZP addressing during KickC and SDCC code */
	/* for SDCC, this is needed to assemble library function params into ZP */
	asm_opt[OPT_ZPDETECT] = (kickc_mode | sdcc_mode | hucc_mode);

	/* enable long-branch support when building KickC code */
	asm_opt[OPT_LBRANCH] |= (kickc_mode | hucc_mode);

	/* enable forward-references when building KickC code */
	asm_opt[OPT_FORWARD] |= kickc_mode;

	/* disable C comments in HuCC code, it breaks HuCC's macro comments */
	if (hucc_mode)
		asm_opt[OPT_CCOMMENT] = 0;

	/* output line */
	if (pass == LAST_PASS)
		println();
}


/* ----
 * do_ignore()
 * ----
 * .cpu      pseudo (ignored, for compatibility with KickC)
 * .encoding pseudo (ignored, for compatibility with KickC)
 * .optsdcc  pseudo (ignored, for compatibility with SDCC)
 * .globl    pseudo (ignored, for compatibility with SDCC)
 */

void
do_ignore(int *ip)
{
	/* define label */
	labldef(LOCATION);

#if 0
	/* skip spaces */
	while (isspace(prlnbuf[*ip]))
		(*ip)++;

	/* extract name */
	if (!colsym(ip, 0)) {
		if (symbol[0] == 0)
			fatal_error("Syntax error!");
		return;
	}

	/* check end of line */
	if (!check_eol(ip))
		return;
#endif

	/* output line */
	if (pass == LAST_PASS)
		println();
}


/* ----
 * do_segment()
 * ----
 * .segment pseudo (optype == 0) (for compatibility with KickC)
 * .area    pseudo (optype == 1) (for compatibility with SDCC)
 */

void
do_segment(int *ip)
{
	/* define label */
	labldef(LOCATION);

	/* skip spaces */
	while (isspace(prlnbuf[*ip]))
		(*ip)++;

	/* extract name */
	if (!colsym(ip, 0)) {
		if (symbol[0] == 0)
			fatal_error("Syntax error!");
		return;
	}

	/* check end of line */
	if (sdcc_mode == 0 && !check_eol(ip))
		return;

	/* which segment? */
	if (!strcasecmp(&symbol[1], "ZP"))
		optype = S_ZP;
	else
	if (!strcasecmp(&symbol[1], "BSS"))
		optype = S_BSS;
	else
	if (!strcasecmp(&symbol[1], "CODE"))
		optype = S_CODE;
	else
	if (!strcasecmp(&symbol[1], "DATA")) {
		if (sdcc_mode)
			optype = S_XDATA;
		else
			optype = S_DATA;
	}
	else
	if (!strcasecmp(&symbol[1], "XDATA"))
		optype = S_XDATA;
	else
	if (!strcasecmp(&symbol[1], "XINIT"))
		optype = S_XINIT;
	else
	if (!strcasecmp(&symbol[1], "CONST"))
		optype = S_CONST;
	else
	if (!strcasecmp(&symbol[1], "RODATA"))
		optype = S_CONST;
	else
	if (!strcasecmp(&symbol[1], "OSEG"))
		optype = S_OSEG;
	else
	if (!strcasecmp(&symbol[1], "_CODE"))
		optype = S_HOME;
	else
	if (!strcasecmp(&symbol[1], "_DATA")) {
		if (sdcc_mode)
			optype = S_DATA;
		else
			optype = S_NONE;
	}
	else
	if (!strcasecmp(&symbol[1], "CABS"))
		optype = S_NONE;
	else
	if (!strcasecmp(&symbol[1], "DABS"))
		optype = S_NONE;
	else
	if (!strcasecmp(&symbol[1], "GSINIT"))
		optype = S_NONE;
	else
	if (!strcasecmp(&symbol[1], "GSFINAL"))
		optype = S_NONE;
	else {
		fatal_error("Unknown %s name!", (optype) ? ".AREA" : ".SEGMENT");
		return;
	}

	/* handle this as a PCEAS section type */
	do_section(ip);
}


/* ----
 * do_star()
 * ----
 * '*' pseudo (for KickC code)
 */

void
do_star(int *ip)
{
	/* skip spaces */
	while (isspace(prlnbuf[*ip]))
		(*ip)++;

	if (prlnbuf[(*ip)++] != '=') {
		fatal_error("Syntax error!");
		return;
	}

	/* handle the rest of this as a PCEAS ".org" */
	do_org(ip);
}


/* ----
 * do_label()
 * ----
 * .label & .const pseudo (for compatibility with KickC)
 */

void
do_label(int *ip)
{
	/* define label */
	labldef(LOCATION);

	/* skip spaces */
	while (isspace(prlnbuf[*ip]))
		(*ip)++;

	/* extract name */
	if (!colsym(ip, 1)) {
		if (symbol[0] == 0)
			fatal_error("Syntax error!");
		return;
	}

	/* skip spaces */
	while (isspace(prlnbuf[*ip]))
		(*ip)++;

	if (prlnbuf[(*ip)++] != '=') {
		fatal_error("Syntax error!");
		return;
	}

	/* create the symbol */
	if ((lablptr = stlook(SYM_DEF)) == NULL)
		return;

	/* handle the rest of this as a PCEAS ".equ" */
	do_equ(ip);
}


/* ----
 * do_struct()
 * ----
 * '.struct' pseudo
 */

void
do_struct(int *ip)
{
	/* not allowed while .phase is active */
	if (phase_offset) {
		fatal_error("Cannot declare a .STRUCT within a .PHASE'd chunk of code!");
		return;
	}

	/* the code is written to handle nesting, but try */
	/* this temporarily, while we see if it is needed */
	if (scopeptr != NULL) {
		fatal_error("Cannot nest .STRUCT scopes!");
		return;
	}

	/* do not mix different types of label-scope */
	if (proc_ptr) {
		fatal_error("Cannot declare a .STRUCT inside a .PROC/.PROCGROUP!");
			return;
	}

	/* check symbol */
	if (lablptr == NULL) {
		fatal_error("Label name missing from .STRUCT!");
		return;
	}
	if (lablptr->name[1] == '.' || lablptr->name[1] == '@') {
		fatal_error("Cannot open .STRUCT scope on a local label!");
		return;
	}
	if (lablptr->name[1] == '!') {
		fatal_error("Cannot open .STRUCT scope on a multi-label!");
		return;
	}

	/* define label */
	labldef(LOCATION);

	/* check end of line */
	if (!check_eol(ip))
		return;

	lablptr->scope = scopeptr;
	scopeptr = lablptr;

	/* output line */
	if (pass == LAST_PASS)
		println();
}


/* ----
 * do_ends()
 * ----
 * '.ends' pseudo
 */

void
do_ends(int *ip)
{
	/* remember the current label */
	struct t_symbol *curlabl = lablptr;
	int i;

	/* sanity check */
	if (scopeptr == NULL) {
		fatal_error("Unexpected .ENDS!");
		return;
	}

	/* check end of line */
	if (!check_eol(ip))
		return;

	/* restore the scope's original section */
	optype = scopeptr->section;
	do_section(ip);

	/* define label */
	labldef(LOCATION);

	/* remember the size of the scope */
	scopeptr->data_type = P_STRUCT;

	// fixme:
	fatal_error("This needs to be fixed, but not today!");
	return;
	// end of fixme:

	scopeptr->data_size = (loccnt + (bank << 13)) - ((scopeptr->value & 0x1FFF) + (scopeptr->mprbank << 13));

	/* add a label with the scope size */
	i = addscope(scopeptr, 0);
	symbol[++i] = '\0';

	if (i > (SBOLSZ - 1 - 7)) {
		fatal_error("Struct name too long to create \"_sizeof\" label!");
		return;
	}
	strncat(&symbol[i], "_sizeof", SBOLSZ - 1 - i);

	/* create the "_sizeof" label */
	if ((lablptr = stlook(SYM_DEF)) == NULL)
		return;

	/* assign value to the label */
//	labldef(scopeptr->data_size, UNDEFINED_BANK, 0, CONSTANT);

	/* restore the previous label */
	lablptr = curlabl;

	/* return to previous scope */
	scopeptr = scopeptr->scope;

//	/* output line */
//	if (pass == LAST_PASS)
//		println();
}


/* ----
 * do_alias()
 * ----
 * .alias pseudo
 */

void
do_alias(int *ip)
{
	struct t_symbol *alias;
	int i;


	/* get alias name */
	if (lablptr) {
		/* go back to the unaliased label */
		if (lablptr != unaliased) {
			lablptr = unaliased;
		}

		/* copy the procedure name from the label */
		strcpy(symbol, lablptr->name);
	}
	else {
		/* skip spaces */
		while (isspace(prlnbuf[*ip]))
			(*ip)++;

		/* extract name */
		if (!colsym(ip, 0)) {
			/* was there a bad symbol */
			if (symbol[0])
				return;
			/* or just no symbol at all */
			fatal_error(".ALIAS name is missing!");
			return;
		}

		/* lookup symbol table */
		if ((lablptr = stlook(SYM_DEF)) == NULL)
			return;

		/* go back to the unaliased label */
		if (lablptr != unaliased) {
			lablptr = unaliased;
		}

		/* skip spaces */
		while (isspace(prlnbuf[*ip]))
			(*ip)++;

		if (prlnbuf[(*ip)++] != '=') {
			fatal_error("Syntax error!");
			return;
		}
	}

	/* check symbol */
	if (symbol[1] == '.' || symbol[1] == '@') {
		error(".ALIAS name cannot be a local label!");
		return;
	}
	if (symbol[1] == '!') {
		error(".ALIAS name cannot be a multi-label!");
		return;
	}

	/* is the symbol already used for somthing else */
	if (lablptr->type == MACRO) {
		error("Symbol already used by a macro!");
		return;
	}
	if (lablptr->type == FUNC) {
		error("Symbol already used by a function!");
		return;
	}
	if (lablptr->type == DEFABS || lablptr->type == MDEF ) {
		error("Symbol already used by a label!");
		return;
	}
	if (lablptr->flags & FLG_RESERVED) {
		error("Reserved symbol!");
		return;
	}

	/* skip spaces */
	while (isspace(prlnbuf[*ip]))
		(*ip)++;

	/* make sure that it's not an instruction or pseudo op, N.B. "i" is altered by oplook() */
	i = *ip;
	if (oplook(&i) >= 0) {
		fatal_error("Cannot create a .ALIAS to an instruction name!");
		return;
	}

	/* extract symbol name of .ALIAS target */
	if (!colsym(ip, 0)) {
		/* was there a bad symbol */
		if (symbol[0] == 0)
			return;
		/* or just no symbol at all */
		error(".ALIAS target symbol name is missing!");
		return;
	}

	/* check end of line */
	if (!check_eol(ip))
		return;

	/* check symbol */
	if (symbol[1] == '.' || symbol[1] == '@') {
		fatal_error("Cannot create a .ALIAS to a local label!");
		return;
	}
	if (symbol[1] == '!') {
		fatal_error("Cannot create a .ALIAS to a multi-label!");
		return;
	}

	/* check for redefinition */
	if (lablptr->type == ALIAS) {
		alias = lablptr->local;
		if (strcmp(alias->name, symbol) != 0) {
			error(".ALIAS was already defined differently!");
			return;
		}
	} else {
		/* lookup or create the target symbol name */
		if ((alias = stlook(SYM_DEF)) == NULL)
			return;

		/* is the symbol already used for somthing else */
		if (alias->type == MACRO) {
			error("Cannot create a .ALIAS to a macro!");
			return;
		}
		if (alias->type == FUNC) {
			error("Cannot create a .ALIAS to a function!");
			return;
		}
		if (alias->flags & FLG_RESERVED) {
			error("Cannot create a .ALIAS to a reserved symbol!");
			return;
		}

		/* define alias */
		lablptr->type = ALIAS;
		lablptr->local = alias;

		/* remember where this was defined */
		lablptr->fileinfo = input_file[infile_num].file;
		lablptr->fileline = slnum;
		lablptr->filecolumn = 0;

		/* the alias needs to inherit any previous references to the label */
		alias->refthispass += lablptr->refthispass;
	}

	/* check for circular definition */
	if (strcmp(lablptr->name, alias->name) == 0) {
		error(".ALIAS cannot refer to itself!");
		return;
	}

	/* record definition */
	lablptr->defthispass = 1;

	/* output */
	if (pass == LAST_PASS) {
		loadlc(loccnt, 0);
		println();
	}
}


/* ----
 * do_ref()
 * ----
 * .ref pseudo
 */

void
do_ref(int *ip)
{
	/* define label */
	labldef(LOCATION);

	/* evaluate expression to increment a label's reference count */
	if (!evaluate(ip, ';', 0))
		return;

	/* output line */
	if (pass == LAST_PASS) {
		loadlc(loccnt, 0);
		println();
	}
}


/* ----
 * do_phase()
 * ----
 * .phase   pseudo (optype == 0)
 * .dephase pseudo (optype == 1)
 */

void
do_phase(int *ip)
{
	/* set label value if there was one */
	labldef(LOCATION);

	if (optype == 0) {
		/* not allowed while .phase is active */
		if (phase_offset) {
			error("Already in a .PHASE'd chunk of code!");
			return;
		}

		/* get the .phase value */
		if (!evaluate(ip, ';', 1))
			return;

		/* check for undefined symbols - they are only allowed in the FIRST_PASS */
		if (undef != 0) {
			phase_offset = 1;

			if ((asm_opt[OPT_FORWARD] == 0) || (pass != FIRST_PASS))
				error("Undefined symbol in operand field!");
			else
				need_another_pass = 1;
			return;
		}

		/* check for address out of range */
		if (value > 0xFFFF) {
			error(".PHASE value must be an address $0000..$FFFF!");
			return;
		}

	} else {
		/* not allowed while .phase is inactive */
		if (!phase_offset) {
			error("Not in a .PHASE'd chunk of code!");
			return;
		}

		/* check end of line */
		if (!check_eol(ip))
			return;

		value = (loccnt + (page << 13));
	}

	/* output line on last pass */
	if (pass == LAST_PASS) {
		loadlc(loccnt, 0);
		println();
	}

	/* set the phase_offset to add to subsequent location labels */
	phase_offset = value - (loccnt + (page << 13));
	if (expr_mprbank == UNDEFINED_BANK)
		phase_bank = bank2mprbank(bank, section);
	else
		phase_bank = expr_mprbank;
}


/* ----
 * do_debug()
 * ----
 * .dbg pseudo for C source code debugging
 */

void
do_debug(int *ip)
{
	char fname[PATHSZ];

	/* set label value if there was one */
	labldef(LOCATION);

	/* skip spaces */
	while (isspace(prlnbuf[*ip]))
		(*ip)++;

	/* extract type */
	if (!colsym(ip, 0)) {
		if (symbol[0] == 0)
			fatal_error("Syntax error!");
		return;
	}

	if (!strcasecmp(&symbol[1], "line")) {
		debug_file = NULL;
		debug_line = 0;
		debug_column = 0;

		/* get file name */
		if (prlnbuf[*ip] != ',') {
			error(".DBG line filename missing!");
			return;
		}
		++(*ip);
		if (!getstring(ip, fname, PATHSZ - 1))
			return;

		/* get file line */
		if (prlnbuf[*ip] != ',') {
			error(".DBG line number missing!");
			return;
		}
		++(*ip);
		if (!evaluate(ip, 0, 0))
			return;
		if (0 > (int)value) {
			error(".DBG line number cannot be negative!");
			return;
		}
		debug_line = value;

		/* get file column (optional) */
		debug_column = 1;
		if (prlnbuf[*ip] == ',') {
			++(*ip);
			if (!evaluate(ip, 0, 0))
				return;
			if (0 > (int)value) {
				error(".DBG column number cannot be negative!");
				return;
			}
			debug_column = value;
		}

		debug_file = lookup_file(fname);
		if (debug_file == NULL)
			return;

		/* check end of line */
		if (!check_eol(ip))
			return;
	}
	else
	if (!strcasecmp(&symbol[1], "clear")) {
		debug_file = NULL;
		debug_line = 0;
		debug_column = 0;

		/* check end of line */
		if (!check_eol(ip))
			return;
	}
	else {
		error("Unknown .DBG type \"%s\"!", &symbol[1]);
		return;
	}

	/* output line on last pass */
	if (pass == LAST_PASS) {
		println();
	}
}


/* ----
 * htoi()
 * ----
 */

int
htoi(char *str, int nb)
{
	char c;
	int val;
	int i;

	val = 0;

	for (i = 0; i < nb; i++) {
		c = toupper(str[i]);

		if ((c >= '0') && (c <= '9'))
			val = (val << 4) + (c - '0');
		else if ((c >= 'A') && (c <= 'F'))
			val = (val << 4) + (c - 'A' + 10);
		else
			return (-1);
	}

	/* ok */
	return (val);
}


/* ----
 * set_section(unsigned char new_section)
 * ----
 */

void
set_section(unsigned char new_section)
{
	if (section != new_section) {
		/* backup current section data */
		section_bank[section] = bank;
		bank_glabl[section][bank] = glablptr;
		bank_loccnt[section][bank] = loccnt;
		bank_page[section][bank] = page;
		section_phase[section] = phase_offset;

		/* change section */
		section = new_section;

		/* switch to the new section */
		bank = section_bank[section];
		page = bank_page[section][bank];
		loccnt = bank_loccnt[section][bank];
		glablptr = bank_glabl[section][bank];
		phase_offset = section_phase[section];

		/* signal discontiguous change in loccnt */
		discontiguous = 1;
	}
}


/* ----
 * do_outbin()
 * ----
 * .outbin pseudo (optype == 0)
 * .outzx0 pseudo (optype == 1)
 */

#include "format.h"
#include "shrink.h"

void
do_outbin(int *ip)
{
	unsigned char *addr = NULL;
	unsigned offset = 0;
	unsigned length = 0;
	unsigned window = 0;
	unsigned need = 0;
	char fname[PATHSZ];

	/* ignore this until the last pass */
	if (pass != LAST_PASS)
		return;

	/* disable ROM output when using this */
	no_rom_file = 1;

	/* get the output offset (linear rom address) */
	if (!evaluate(ip, 0, 0))
		return;
	if (undef != 0) {
		error("Undefined symbol in OFFSET field!");
		return;
	}
	offset = value;
	if (offset >= (ROM_BANKS * 8192)) {
		error("OFFSET must be >= 0 and <= 8MB!");
		return;
	}

	/* get the output length */
	if (prlnbuf[*ip] != ',') {
		error("LENGTH value is missing!");
		return;
	}
	++(*ip);

	if (!evaluate(ip, 0, 0))
		return;
	if (undef != 0) {
		error("Undefined symbol in LENGTH field!");
		return;
	}
	length = value;
	if (length >= (ROM_BANKS * 8192)) {
		error("LENGTH must be >= 0 and <= 8MB!");
		return;
	}
	if ((offset + length) >= (ROM_BANKS * 8192)) {
		error("OFFSET+LENGTH must be >= 0 and <= 8MB!");
		return;
	}

	/* get the window length */
	if (optype == 1) {
		if (prlnbuf[*ip] != ',') {
			error("WINDOW value is missing!");
			return;
		}
		++(*ip);

		if (!evaluate(ip, 0, 0))
			return;
		if (undef != 0) {
			error("Undefined symbol in WINDOW field!");
			return;
		}
		window = value;
		if ((window & (window - 1)) || (window > 8192)) {
			error("WINDOW must be 0 or a power-of-2 <= 8192!");
			return;
		}
	}

	/* get the optional file name */
	if (prlnbuf[*ip] == ',') {
		++(*ip);

		/* close current output file */
		if (out_fp) {
			fclose(out_fp);
			out_fp = NULL;
		}

		/* get file name */
		if (!getstring(ip, fname, PATHSZ - 1))
			return;

		/* open new output file */
		if ((out_fp = open_file(fname, "wb")) == NULL) {
			fatal_error("Unable to open file!");
			return;
		}
	}

	/* check end of line */
	if (!check_eol(ip))
		return;

	/* check end of line */
	if (length) {
		/* locate the data */
		addr = &rom[0][0] + offset;

		/* compress the data */
		if (optype == 1) {
			need = salvador_get_max_compressed_size(length);
			addr = calloc(need, 1);
			length = salvador_compress(&rom[0][0] + offset, addr, length, need, 0, window, 0, NULL, NULL);
			if (length > need) {
				error("Error while compressing the data!");
				free(addr);
				return;
			}
		}

		/* write the data */
		if (fwrite(addr, 1, length, out_fp) != length) {
			fatal_error("Unable to write data to the current output file!");
		}

		/* do we want to compress the data */
		if (need) {
			free(addr);
		}
	}

	/* assign the output length to the label (if there is one) */
	value = length;
	expr_mprbank = UNDEFINED_BANK;
	expr_overlay = 0;
	labldef(CONSTANT);

	/* output line */
	if (pass == LAST_PASS) {
		loadlc(value, 1);
		println();
	}
}
