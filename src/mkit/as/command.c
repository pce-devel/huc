#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "defs.h"
#include "externs.h"
#include "protos.h"

/* pseudo instructions section flag */
char pseudo_flag[] = {
	0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0C, 0x0C, 0x0C, 0x0F,
	0x0C, 0x0C, 0x0C, 0x0C, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F,
	0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F, 0x0F,
	0x0C, 0x0F, 0x0F, 0x0F, 0x0C, 0x0C, 0x0C, 0x0C, 0x0F, 0x0F,
	0x0F, 0x0F, 0x0F, 0x0C, 0x0C, 0x0C, 0x04, 0x0F, 0x04, 0x0F,
	0x04, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0F, 0x0F, 0x0F, 0x0F,
	0x0F, 0x0F, 0x0F, 0x0F, 0x0F
};


/* ----
 * do_pseudo()
 * ----
 * pseudo instruction processor
 */

void
do_pseudo(int *ip)
{
	char str[80];
	int old_bank;
	int size;

	/* check if the directive is allowed in the current section */
	if (!(pseudo_flag[opval] & (1 << section)))
		fatal_error("Directive not allowed in the current section!");

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
			case P_INCMAP:
				if (bank != old_bank) {
					size = ((bank - old_bank - 1) * 8192) + loccnt;
					if (size) {
						sprintf(str, "Warning, bank overflow by %i bytes!\n", size);
						warning(str);
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
 * .db pseudo
 */

void
do_db(int *ip)
{
	unsigned char c;

	/* define label */
	labldef(0, 0, LOCATION);

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
				error("Cannot store non-zero data in .zp or .bss sections!");
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
					case 'r':
						c = '\r';
						break;
					case 'n':
						c = '\n';
						break;
					case 't':
						c = '\t';
						break;
					}
				}
				/* store char on last pass */
				if (pass == LAST_PASS) {
					/* store character */
					putbyte(loccnt, c);
				}

				/* update location counter */
				loccnt++;
			}
			while (isspace(prlnbuf[++(*ip)])) {}
		}
		/* bytes */
		else {
			/* get a byte */
			if (!evaluate(ip, 0))
				return;

			/* update location counter */
			loccnt++;

			/* store byte on last pass */
			if (pass == LAST_PASS) {
				/* check for non-zero value in ZP or BSS sections */
				if ((value != 0) && (section == S_ZP || section == S_BSS)) {
					error("Cannot store non-zero data in .zp or .bss sections!");
					return;
				}

				/* check for overflow */
				if (((value & 0x007FFFFF) > 0xFF) && ((value & 0x007FFFFF) < 0x007FFF80)) {
					error("Overflow error!");
					return;
				}

				/* store byte */
				putbyte(loccnt - 1, value);
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
	if (pass == LAST_PASS)
		println();
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
	labldef(0, 0, LOCATION);

	/* output infos */
	data_loccnt = loccnt;
	data_size = 2;
	data_level = 2;

	/* get data */
	for (;;) {
		/* get a word */
		if (!evaluate(ip, 0))
			return;

		/* update location counter */
		loccnt += 2;

		/* store word on last pass */
		if (pass == LAST_PASS) {
			/* check for non-zero value in ZP or BSS sections */
			if ((value != 0) && (section == S_ZP || section == S_BSS)) {
				error("Cannot store non-zero data in .zp or .bss sections!");
				return;
			}

			/* check for overflow */
			if (((value & 0x007FFFFF) > 0xFFFF) && ((value & 0x007FFFFF) < 0x007F8000)) {
				error("Overflow error!");
				return;
			}

			/* store word */
			putword(loccnt - 2, value);
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
	if (pass == LAST_PASS)
		println();
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
	labldef(0, 0, LOCATION);

	/* output infos */
	data_loccnt = loccnt;
	data_size = 1;
	data_level = 2;

	/* get data */
	for (;;) {
		/* get a word */
		if (!evaluate(ip, 0))
			return;

		/* update location counter */
		loccnt += 1;

		/* store word on last pass */
		if (pass == LAST_PASS) {
			/* check for non-zero value in ZP or BSS sections */
			if ((value != 0) && (section == S_ZP || section == S_BSS)) {
				error("Cannot store non-zero data in .zp or .bss sections!");
				return;
			}

			/* check for overflow */
			if (((value & 0x007FFFFF) > 0xFFFF) && ((value & 0x007FFFFF) < 0x007F8000)) {
				error("Overflow error!");
				return;
			}

			/* store word */
			putbyte(loccnt - 1, (value & 0xff));
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
	if (pass == LAST_PASS)
		println();
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
	labldef(0, 0, LOCATION);

	/* output infos */
	data_loccnt = loccnt;
	data_size = 1;
	data_level = 2;

	/* get data */
	for (;;) {
		/* get a word */
		if (!evaluate(ip, 0))
			return;

		/* update location counter */
		loccnt += 1;

		/* store word on last pass */
		if (pass == LAST_PASS) {
			/* check for non-zero value in ZP or BSS sections */
			if ((value != 0) && (section == S_ZP || section == S_BSS)) {
				error("Cannot store non-zero data in .zp or .bss sections!");
				return;
			}

			/* check for overflow */
			if (((value & 0x007FFFFF) > 0xFFFF) && ((value & 0x007FFFFF) < 0x007F8000)) {
				error("Overflow error!");
				return;
			}

			/* store word */
			putbyte(loccnt - 1, ((value >> 8) & 0xff));
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
	if (pass == LAST_PASS)
		println();
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
	labldef(0, 0, LOCATION);

	/* output infos */
	data_loccnt = loccnt;
	data_size = 4;
	data_level = 2;

	/* get data */
	for (;;) {
		/* get a word */
		if (!evaluate(ip, 0))
			return;

		/* update location counter */
		loccnt += 4;

		/* store dword on last pass */
		if (pass == LAST_PASS) {
			/* check for non-zero value in ZP or BSS sections */
			if ((value != 0) && (section == S_ZP || section == S_BSS)) {
				error("Cannot store non-zero data in .zp or .bss sections!");
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
	if (pass == LAST_PASS)
		println();
}



/* ----
 * do_equ()
 * ----
 * .equ pseudo
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
	if (!evaluate(ip, ';'))
		return;

	/* check for undefined symbol - they are not allowed in .set */
	if ((optype == 1) && (undef != 0)) {
		error("Undefined symbol in operand field!");
		return;
	}

	/* allow ".set" to change a label's value */
	if ((optype == 1) && (lablptr->type == DEFABS)) {
		lablptr->type = DEFABS;
		lablptr->value = value;
	} else {
		/* assign value to the label */
		labldef(value, RESERVED_BANK, CONSTANT);
	}

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
	/* not allowed in procs */
	if (proc_ptr) {
		fatal_error("PAGE can not be changed in procs!");
		return;
	}

	/* define label */
	labldef(0, 0, LOCATION);

	/* get page index */
	if (!evaluate(ip, ';'))
		return;
	if (value > 7) {
		error("Invalid page index!");
		return;
	}
	page = value;

	/* output line */
	if (pass == LAST_PASS) {
		loadlc(value << 13, 1);
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
	/* get the .org value */
	if (!evaluate(ip, ';'))
		return;

	/* check for undefined symbol - they are not allowed in .org */
	if (undef != 0) {
		error("Undefined symbol in operand field!");
		return;
	}

	/* section switch */
	switch (section) {
	case S_ZP:
		/* zero page section */
		if ((value & 0xFFFFFF00) && ((value & 0xFFFFFF00) != machine->ram_base)) {
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
		if (proc_ptr) {
			fatal_error("ORG can not be changed in procs!");
			return;
		}

		/* code and data section */
		if (value & 0xFFFF0000) {
			error("Invalid address!");
			return;
		}
		page = (value >> 13) & 0x07;
		break;
	}

	/* set location counter */
	loccnt = (value & 0x1FFF);

	/* signal discontiguous change in loccnt */
	discontiguous = 1;

	/* set label value if there was one */
	labldef(0, 0, LOCATION);

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

	/* not allowed in procs */
	if (proc_ptr) {
		fatal_error("Bank can not be changed in procs!");
		return;
	}

	/* define label */
	labldef(0, 0, LOCATION);

	/* get bank index */
	if (!evaluate(ip, 0))
		return;
	if (value > bank_limit) {
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
	char fname[128];
	int size;

	/* get file name */
	if (!getstring(ip, fname, 127))
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

	/* define label */
	labldef(0, 0, LOCATION);

	/* output */
	if (pass == LAST_PASS)
		loadlc(loccnt, 0);

	/* open file */
	if ((fp = open_file(fname, "rb")) == NULL) {
		fatal_error("Can not open file!");
		return;
	}

	/* get file size */
	fseek(fp, 0, SEEK_END);
	size = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	/* check if it will fit in the rom */
	if (((bank << 13) + loccnt + size) > rom_limit) {
		fclose(fp);
		error("ROM overflow!");
		return;
	}

	/* load data on last pass */
	if (pass == LAST_PASS) {
		fread(&rom[bank][loccnt], 1, size, fp);
		memset(&map[bank][loccnt], section + (page << 5), size);

		/* output line */
		println();
	}

	/* close file */
	fclose(fp);

	/* update bank and location counters */
	bank += (loccnt + size) >> 13;
	loccnt = (loccnt + size) & 0x1FFF;
	if (bank > max_bank) {
		if (loccnt)
			max_bank = bank;
		else
			max_bank = bank - 1;
	}

	/* size */
	if (lablptr) {
		lablptr->data_type = P_INCBIN;
		lablptr->data_size = size;
	}
	else {
		if (lastlabl) {
			if (lastlabl->data_type == P_INCBIN)
				lastlabl->data_size += size;
		}
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
		fatal_error("Can not open file!");
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
					labldef(0, 0, LOCATION);

					/* output */
					if (pass == LAST_PASS)
						loadlc(loccnt, 0);
				}

				/* copy data */
				if (pass == LAST_PASS) {
					for (i = 0; i < cnt; i++)
						putbyte(loccnt + i, buffer[i]);
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
		labldef(0, 0, LOCATION);

		/* output */
		if (pass == LAST_PASS)
			loadlc(loccnt, 0);
	}

	/* size */
	if (lablptr) {
		lablptr->data_type = P_INCBIN;
		lablptr->data_size = size;
	}
	else {
		if (lastlabl) {
			if (lastlabl->data_type == P_INCBIN)
				lastlabl->data_size += size;
		}
	}

	/* output line */
	if (pass == LAST_PASS)
		println();
}


/* ----
 * forget_included_files()
 * ----
 * keep a list of the .include files during each pass
 */

typedef struct t_filelist {
	struct t_filelist * next;
	int size;
	char name[128];
} t_filelist;

t_filelist * included_files = NULL;

void
forget_included_files(void)
{
	t_filelist * list = included_files;
	while ((list = included_files) != NULL) {
		included_files = list->next;
		free(list);
	}
}


/* ----
 * do_include()
 * ----
 * .include pseudo
 */

void
do_include(int *ip)
{
	char fname[128];
	int fsize;
	int found_include;
	t_filelist * list;

	/* define label */
	labldef(0, 0, LOCATION);

	/* avoid problems */
	if (expand_macro) {
		error("Cannot use INCLUDE inside a macro!");
		return;
	}

	/* get file name */
	if (!getstring(ip, fname, 127))
		return;

	/* have we already included this file on this pass? */
	fsize = strlen(fname);
	found_include = 0;

	for (list = included_files; list != NULL; list = list->next) {
		if ((list->size == fsize) && (strcasecmp(list->name, fname) == 0)) {
			found_include = 1;
			break;
		}
	}

	/* do not include the file a 2nd time on this pass */
	if (!found_include) {
		/* remember include file name */
		if ((list = malloc(sizeof(t_filelist) + fsize - 127)) == NULL) {
			fatal_error("Out of memory!");
			return;
		}

		strcpy(list->name, fname);
		list->size = fsize;
		list->next = included_files;
		included_files = list;

		/* open file */
		if (open_input(fname) == -1) {
			fatal_error("Can not open file!");
			return;
		}
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
	labldef(0, 0, LOCATION);

	/* get value */
	if (!evaluate(ip, ';'))
		return;
	if (value & 0xFFFF0000) {
		error("Address out of range!");
		return;
	}

	/* set 'rs' base */
	rsbase = value;

	/* output line */
	if (pass == LAST_PASS) {
		loadlc(rsbase, 1);
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
	/* define label */
	labldef(rsbase, RESERVED_BANK, CONSTANT);

	/* get the number of bytes to reserve */
	if (!evaluate(ip, ';'))
		return;

	/* ouput line */
	if (pass == LAST_PASS) {
		loadlc(rsbase, 1);
		println();
	}

	/* update 'rs' base */
	rsbase += value;
	if (rsbase & 0xFFFF0000)
		error("Address out of range!");
}


/* ----
 * do_ds()
 * ----
 * .ds pseudo
 */

void
do_ds(int *ip)
{
	int limit = 0;
	int addr;
	unsigned int nbytes;
	unsigned int filler = 0;
	unsigned char c;

	/* define label */
	labldef(0, 0, LOCATION);

	/* output infos */
	data_loccnt = loccnt;
	data_level = 2;

	/* get the number of bytes to reserve */
	if (!evaluate(ip, 0))
		return;

	/* check for undefined symbol - they are not allowed in .ds */
	if (undef != 0) {
		error("Undefined symbol in operand field!");
		return;
	}

	nbytes = value;

	c = prlnbuf[(*ip)++];

	/* check if there's another word */
	if (c == ',') {
		/* get the filler byte */
		if (!evaluate(ip, 0))
			return;

		filler = value & 255;

		c = prlnbuf[(*ip)++];
	}

	/* check error */
	if (c != ';' && c != '\0') {
		error("Syntax error!");
		return;
	}

	/* section switch */
	switch (section) {
	case S_ZP:
		/* zero page section */
		limit = machine->zp_limit;
		break;

	case S_BSS:
		/* ram section */
		limit = machine->ram_limit;
		break;

	case S_CODE:
	case S_DATA:
		/* code and data sections */
		limit = 0x2000;
		break;
	}

	/* check range */
	if ((loccnt + nbytes) > limit) {
		error("Out of range!");
		return;
	}

	/* update max counter for zp and bss sections */
	addr = loccnt + nbytes;

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
		if (filler != 0) {
			if (section == S_ZP)
				error("Cannot fill .ZP section with non-zero data!");
			else
			if (section == S_BSS)
				error("Cannot fill .BSS section with non-zero data!");
		}

		if (section == S_CODE || section == S_DATA) {
			memset(&rom[bank][loccnt], filler, nbytes);
			memset(&map[bank][loccnt], section + (page << 5), nbytes);
			if (bank > max_bank)
				max_bank = bank;
		}
	}

	/* update location counter */
	loccnt += nbytes;

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
 * .zp/.bss/.code/.data pseudo
 */

void
do_section(int *ip)
{
	if (proc_ptr && (scopeptr == NULL)) {
		if (optype == S_DATA) {
			fatal_error("No data segment in procs!");
			return;
		}
	}
	if (section != optype) {
		/* backup current section data */
		section_bank[section] = bank;
		bank_glabl[section][bank] = glablptr;
		bank_loccnt[section][bank] = loccnt;
		bank_page[section][bank] = page;

		/* change section */
		section = optype;

		/* switch to the new section */
		bank = section_bank[section];
		page = bank_page[section][bank];
		loccnt = bank_loccnt[section][bank];
		glablptr = bank_glabl[section][bank];

		/* signal discontiguous change in loccnt */
		discontiguous = 1;
	}

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
 */

void
do_incchr(int *ip)
{
	unsigned char buffer[32];
	unsigned int i, j;
	int x, y, w, h;
	unsigned int tx, ty;
	int total = 0;
	int size;

	/* define label */
	labldef(0, 0, LOCATION);

	/* output */
	if (pass == LAST_PASS)
		loadlc(loccnt, 0);

	/* get args */
	if (!pcx_get_args(ip))
		return;
	if (!pcx_parse_args(0, pcx_nb_args, &x, &y, &w, &h, 8))
		return;

	/* pack data */
	for (i = 0; i < h; i++) {
		for (j = 0; j < w; j++) {
			/* tile coordinates */
			tx = x + (j << 3);
			ty = y + (i << 3);

			/* get tile */
			pcx_pack_8x8_tile(buffer, tx, ty);
			size = (machine->type == MACHINE_PCE) ? 32 : 16;
			total += size;

			/* store tile */
			putbuffer(buffer, size);
		}
	}

	/* size */
	if (lablptr) {
		lablptr->data_type = P_INCCHR;
		lablptr->data_size = total;
	}
	else {
		if (lastlabl) {
			if (lastlabl->data_type == P_INCCHR)
				lastlabl->data_size += total;
		}
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

	/* get the .align value */
	if (!evaluate(ip, ';'))
		return;

	/* check for undefined symbol - they are not allowed in .align */
	if (undef != 0) {
		error("Undefined symbol in operand field!");
		return;
	}

	/* check for power-of-two, 1 bank maximum */
	if ((value > 8192) || (value == 0) || ((value & (value - 1)) != 0)) {
		error(".align value must be a power-of-two, with a maximum of 8192!");
		return;
	}

	/* did the previous instruction fill up the current bank? */
	if (loccnt >= 0x2000) {
		loccnt &= 0x1fff;
		page++;
		bank++;
	}

	/* are we already aligned to the request boundary? */
	if ((offset = loccnt & (value - 1)) != 0) {
		/* update location counter */
		loccnt = (loccnt + value - offset) & 0x1fff;

		if (loccnt == 0) {
			page++;
			bank++;
		}

		/* signal discontiguous change in loccnt */
		discontiguous = 1;
	}

	/* set label value if there was one */
	labldef(0, 0, LOCATION);

	/* output line on last pass */
	if (pass == LAST_PASS) {
		loadlc(loccnt + (page << 13), 1);
		println();
	}
}


/* ----
 * do_kickc()
 * ----
 * .pceas pseudo
 * .kickc pseudo
 */

void
do_kickc(int *ip)
{
	/* define label */
	labldef(0, 0, LOCATION);

	/* check end of line */
	if (!check_eol(ip))
		return;

	/* enable/disable KickC mode */
	kickc_mode = optype;

	/* enable () for indirect addressing in KickC mode */
	asm_opt[OPT_INDPAREN] = optype;

	/* enable auto-detect ZP addressing in KickC mode */
	asm_opt[OPT_ZPDETECT] = optype;

	/* enable long-branch support in KickC mode */
	asm_opt[OPT_LBRANCH] = optype;

	/* output line */
	if (pass == LAST_PASS)
		println();
}


/* ----
 * do_cpu()
 * ----
 * .cpu pseudo (ignored, only for compatibility with KickC)
 */

void
do_cpu(int *ip)
{
	/* define label */
	labldef(0, 0, LOCATION);

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

	/* output line */
	if (pass == LAST_PASS)
		println();
}


/* ----
 * do_segment()
 * ----
 * .segment pseudo (for KickC code)
 */

void
do_segment(int *ip)
{
	/* define label */
	labldef(0, 0, LOCATION);

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
	if (!strcasecmp(&symbol[1], "DATA"))
		optype = S_DATA;
	else {
		fatal_error("Segment can only be CODE, DATA, BSS or ZP!");
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
 * .label & .const pseudo (for KickC code)
 */

void
do_label(int *ip)
{
	/* define label */
	labldef(0, 0, LOCATION);

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
 * do_encoding()
 * ----
 * .encoding pseudo (ignored, only for compatibility with KickC)
 */

void
do_encoding(int *ip)
{
	/* define label */
	labldef(0, 0, LOCATION);

	/* skip spaces */
	while (isspace(prlnbuf[*ip]))
		(*ip)++;

#if 0
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
 * do_struct()
 * ----
 * '.struct' pseudo
 */

void
do_struct(int *ip)
{
	/* the code is written to handle nesting, but try */
	/* this temporarily, while we see if it is needed */
	if (scopeptr != NULL) {
		fatal_error("Cannot nest .struct scopes!");
		return;
	}

	/* do not mix different types of label-scope */
	if (proc_ptr) {
		fatal_error("Cannot declare a .struct inside a .proc/.procgroup!");
			return;
	}

	/* check symbol */
	if (lablptr == NULL) {
		fatal_error("Label name missing from .struct!");
		return;
	}
	if (lablptr->name[1] == '.' || lablptr->name[1] == '@') {
		fatal_error("Cannot open .struct scope on a local label!");
		return;
	}
	if (lablptr->name[1] == '!') {
		fatal_error("Cannot open .struct scope on a multi-label!");
		return;
	}

	/* define label */
	labldef(0, 0, LOCATION);

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
		fatal_error("Unexpected '.ends'!");
		return;
	}

	/* check end of line */
	if (!check_eol(ip))
		return;

	/* restore the scope's original section */
	optype = scopeptr->section;
	do_section(ip);

	/* define label */
	labldef(0, 0, LOCATION);

	/* remember the size of the scope */
	scopeptr->data_type = P_STRUCT;
	scopeptr->data_size = (loccnt + (bank << 13)) - ((scopeptr->value & 0x1FFF) + (scopeptr->bank << 13));

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
	labldef(scopeptr->data_size, RESERVED_BANK, CONSTANT);

	/* restore the previous label */
	lablptr = curlabl;

	/* return to previous scope */
	scopeptr = scopeptr->scope;

//	/* output line */
//	if (pass == LAST_PASS)
//		println();
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
