/*
 * pseudo.c
 */

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "data.h"
#include "defs.h"
#include "code.h"
#include "const.h"
#include "error.h"
#include "io.h"
#include "lex.h"
#include "optimize.h"
#include "primary.h"
#include "pseudo.h"
#include "sym.h"


/* local array to store internal strings */
static char str_buf[0x10000];
static int str_idx;

/* protos */
char *new_string (int und, char *a);
void  do_inc_ex (int type);

/*
 * This source file includes all kind of stuff used to 'simulate' pseudo code
 * of the magic kit and so using itself pseudo C functions...
 */
void dopsdinc (void)
{
	int dummy;		/* Used in the qstr function, I don't know its utility yet */
	int numericarg = 0;	/* Number of numeric arg to test validity */

	if (amatch("pal", 3)) {
		if (!match("("))
			error("missing (");

		ol(".data");

		readstr();	/* read the label name */
		prefix();
		outstr(litq2);
		outstr(":\n");
		addglb_far(litq2, CINT);

		if (!match(",")) {
			error("missing ,");
			kill();
			return;
		}

		ot(".incpal \"");

		if (readqstr() == 0) {
			/* read the filename */
			error("bad filename in incpal");
			kill();
			return;
		}

		outstr(litq2);
		outstr("\"");

		if (match(","))
			outstr(",");

		numericarg = 0;

		while (!match(")")) {
			numericarg++;

			number(&dummy);
			outdec(dummy);
			if (match(","))
				outstr(",");
		}

		nl();
		ol(".code");

		if (numericarg > 2)
			error("Maximum 2 numeric arg for incpal(name,\"filename\" [,start_pal] [,nb_pal])");

		kill();
	}
	else
	if (amatch("bin", 3)) {
		if (!match("("))
			error("missing (");

		ol(".data");

		readstr();	/* read the label name */
		prefix();
		outstr(litq2);
		outstr(":\n");
		addglb_far(litq2, CUCHAR);

		if (!match(",")) {
			error("missing ,");
			kill();
			return;
		}

		ot(".incbin \"");
		if (readqstr() == 0) {
			/* read the filename */
			error("bad filename in incbin");
			kill();
			return;
		}
		outstr(litq2);
		outstr("\"\n");

		if (!match(")"))
			error("missing )");

		nl();
		ol(".code");
		kill();
	}
	else
	if (amatch("bat", 3)) {
		if (!match("("))
			error("missing (");

		ol(".data");

		readstr();	/* read the label name */
		prefix();
		outstr(litq2);
		outstr(":\n");
		addglb_far(litq2, CINT);

		if (!match(",")) {
			error("missing ,");
			kill();
			return;
		}

		ot(".incbat \"");

		if (readqstr() == 0) {
			error("bad filename in incbat");
			kill();
			return;
		}

		outstr(litq2);
		outstr("\"");

		if (match(","))
			outstr(",");

		numericarg = 0;

		while (!match(")")) {
			numericarg++;

			number(&dummy);
			outdec(dummy);
			if (match(","))
				outstr(",");
		}

		nl();
		ol(".code");

		if ((numericarg != 1) &&
		    (numericarg != 3) &&
		    (numericarg != 5))
			error("Either 1,3 or 5 numeric arguments are needed for incbat statement");

		kill();
	}
	else
	if (amatch("spr", 3)) {
		if (!match("("))
			error("missing (");

		ol(".data");

		readstr();	/* read the label name */
		prefix();
		outstr(litq2);
		outstr(":\n");
		addglb_far(litq2, CINT);

		if (!match(",")) {
			error("missing ,");
			kill();
			return;
		}

		ot(".incspr \"");

		if (readqstr() == 0) {
			error("bad filename in incspr");
			kill();
			return;
		}

		outstr(litq2);
		outstr("\"");

		if (match(","))
			outstr(",");

		numericarg = 0;

		while (!match(")")) {
			numericarg++;

			number(&dummy);
			outdec(dummy);
			if (match(","))
				outstr(",");
		}

		nl();
		ol(".code");

		if ((numericarg != 0) &&
		    (numericarg != 2) &&
		    (numericarg != 4))
			error("Either 0,2 or 4 numeric arguments are needed for incspr statement");

		kill();
	}
	else
	if (amatch("sprpal", 6)) {
		if (!match("("))
			error("missing (");

		ol(".data");

		readstr();	/* read the label name */
		prefix();
		outstr(litq2);
		outstr(":\n");
		addglb_far(litq2, CINT);

		if (!match(",")) {
			error("missing ,");
			kill();
			return;
		}

		ot(".incsprpal \"");

		if (readqstr() == 0) {
			error("bad filename in incsprpal");
			kill();
			return;
		}

		outstr(litq2);
		outstr("\"");

		if (match(","))
			outstr(",");

		numericarg = 0;

		while (!match(")")) {
			numericarg++;

			number(&dummy);
			outdec(dummy);
			if (match(","))
				outstr(",");
		}

		nl();
		ol(".code");

		if ((numericarg != 0) &&
		    (numericarg != 2) &&
		    (numericarg != 4))
			error("Either 0,2 or 4 numeric arguments are needed for incsprpal statement");

		kill();
	}
	else
	if (amatch("chr", 3)) {
		if (!match("("))
			error("missing (");

		ol(".data");

		readstr();	/* read the label name */
		prefix();
		outstr(litq2);
		outstr(":\n");
		addglb_far(litq2, CINT);

		if (!match(",")) {
			error("missing ,");
			kill();
			return;
		}

		ot(".incchr \"");

		if (readqstr() == 0) {
			error("bad filename in incchr");
			kill();
			return;
		}

		outstr(litq2);
		outstr("\"");

		if (match(","))
			outstr(",");

		numericarg = 0;

		while (!match(")")) {
			numericarg++;

			number(&dummy);
			outdec(dummy);
			if (match(","))
				outstr(",");
		}

		nl();
		ol(".code");

		if ((numericarg != 0) &&
		    (numericarg != 2) &&
		    (numericarg != 4))
			error("Either 0,2 or 4 numeric arguments are needed for incchr statement");

		kill();
	}
	else
	if (amatch("chrpal", 6)) {
		if (!match("("))
			error("missing (");

		ol(".data");

		readstr();	/* read the label name */
		prefix();
		outstr(litq2);
		outstr(":\n");
		addglb_far(litq2, CINT);

		if (!match(",")) {
			error("missing ,");
			kill();
			return;
		}

		ot(".incchrpal \"");

		if (readqstr() == 0) {
			error("bad filename in incchrpal");
			kill();
			return;
		}

		outstr(litq2);
		outstr("\"");

		if (match(","))
			outstr(",");

		numericarg = 0;

		while (!match(")")) {
			numericarg++;

			number(&dummy);
			outdec(dummy);
			if (match(","))
				outstr(",");
		}

		nl();
		ol(".code");

		if ((numericarg != 0) &&
		    (numericarg != 2) &&
		    (numericarg != 4))
			error("Either 0,2 or 4 numeric arguments are needed for incchrpal statement");

		kill();
	}
	else
	if (amatch("chr_ex", 6))
		do_inc_ex(8);
	else
	if (amatch("tile", 4)) {
		if (!match("("))
			error("missing (");

		ol(".data");

		readstr();	/* read the label name */
		prefix();
		outstr(litq2);
		outstr(":\n");
		addglb_far(litq2, CINT);

		if (!match(",")) {
			error("missing ,");
			kill();
			return;
		}

		ot(".inctile \"");

		if (readqstr() == 0) {
			error("bad filename in inctile");
			kill();
			return;
		}

		outstr(litq2);
		outstr("\"");

		if (match(","))
			outstr(",");

		numericarg = 0;

		while (!match(")")) {
			numericarg++;

			number(&dummy);
			outdec(dummy);
			if (match(","))
				outstr(",");
		}

		nl();
		ol(".code");

		if ((numericarg != 0) &&
		    (numericarg != 2) &&
		    (numericarg != 4))
			error("Either 0,2 or 4 numeric arguments are needed for inctile statement");

		kill();
	}
	else
	if (amatch("tilepal", 7)) {
		if (!match("("))
			error("missing (");

		ol(".data");
		ol(".if\t(* & $1FFF) > $1F00");
		ol(".align\t$2000");
		ol(".endif");

		readstr();	/* read the label name */
		prefix();
		outstr(litq2);
		outstr(":\n");
		addglb_far(litq2, CINT);

		if (!match(",")) {
			error("missing ,");
			kill();
			return;
		}

		ot(".inctilepal \"");

		if (readqstr() == 0) {
			error("bad filename in inctilepal");
			kill();
			return;
		}

		outstr(litq2);
		outstr("\"");

		if (match(","))
			outstr(",");

		numericarg = 0;

		while (!match(")")) {
			numericarg++;

			number(&dummy);
			outdec(dummy);
			if (match(","))
				outstr(",");
		}

		nl();
		ol(".code");

		if ((numericarg != 0) &&
		    (numericarg != 2) &&
		    (numericarg != 4))
			error("Either 0,2 or 4 numeric arguments are needed for inctilepal statement");

		kill();
	}
	else
	if (amatch("tile_ex", 7))
		do_inc_ex(16);
	else
	if (amatch("asmlabel", 8)) {
		if (!match("("))
			error("missing (");

		// .data first!
		nl();
		ol(".code");

		// Get the label, but save it for later.
		readstr();
		strcpy(str_buf, litq2);
		addglb_far(litq2, CUCHAR);

		if (!match(",")) {
			error("asmlabel missing ,");
			kill();
			return;
		}

		// Get the file name
		if (readqstr() == 0) {
			error("bad filename in incasm");
			kill();
			return;
		}

		// If page argument, then get it. Else default it.
		if (match(",")) {
			if (number(&dummy) != 0) {
				ot(".page\t");
				if (dummy > 8)
					outdec(dummy / 0x2000);
				else
					outdec(dummy);
				nl();
			}
			else {
				error("missing page number/address");
				kill();
				return;
			}
		}
		else {
			ol(".page\t2");
		}

		// Output the label name:
		prefix();
		outstr(str_buf);
		outstr(":\n");

		ot("include\t\"");
		outstr(litq2);
		outstr("\"");
		nl();

		ol(".page\t7");	/* assumes code bank is mapped at 0xE000 */

		if (!match(")"))
			error("missing )");
		nl();
		kill();
	}
	else
	if (amatch("asm", 3)) {
		if (!match("("))
			error("missing (");

		ol(".data");

		if (readqstr() == 0) {
			error("bad filename in incasm");
			kill();
			return;
		}

		if (match(",")) {
			if (number(&dummy) != 0) {
				ot(".page ");
				if (dummy > 8)
					outdec(dummy / 0x2000);
				else
					outdec(dummy);
				nl();
			}
			else {
				error("missing page number/address");
				kill();
				return;
			}
		}

		ot("include\t\"");
		outstr(litq2);
		outstr("\"");
		nl();

		ol(".page\t3");	/* assumes data bank is mapped at 0x6000 */

		if (!match(")"))
			error("missing )");
		ol(".code");
		kill();
	}
	else {
		error("Unknown include directive");
		kill();
	}
	return;
}


void dopsddef (void)
{
	int numericarg = 0;
	int dummy;
	int dummy_array[16];
	int i;

	if (amatch("pal", 3)) {
		if (!match("("))
			error("missing (");

		readstr();	/* read the label name */
		addglb_far(litq2, CINT);

		if (!match(",")) {
			error("missing ',' in #defpal");
			kill();
			return;
		}

		numericarg = 0;

		while (!match(")")) {
			number(&dummy_array[numericarg]);
			numericarg++;

			if (numericarg > 16)
				error("No more than 16 colors can be defined at once");

			match(",");
		}

		ol(".data");
		prefix();
		outstr(litq2);
		outstr(":");
		ot(".defpal ");

		for (i = 0; i < numericarg; i++) {
			outhexfix(dummy_array[i], 3);

			if (i < numericarg - 1) {
				outstr(",");
				if (i == 7) {
					outstr(" \\\n");
					ot("\t");
				}
			}
		}

		nl();
		ol(".code");

		kill();
	}
	else
	if (amatch("chr", 3)) {
		if (!match("("))
			error("missing (");

		ol(".data");

		readstr();	/* read the label name */
		prefix();
		outstr(litq2);
		outstr(":");
		addglb_far(litq2, CINT);

		if (!match(",")) {
			error("missing ,");
			kill();
			return;
		}

		ot(".defchr ");

		numericarg = 0;

		while (!match(")")) {
			numericarg++;

			number(&dummy);

			switch (numericarg) {
			case 1:
				outhexfix(dummy, 4);
				outstr(",");
				break;
			case 2:
				outdec(dummy);
				outstr(",\\");
				nl();
				break;
			case 10:
				outhexfix(dummy, 8);
				break;
			default:
				outhexfix(dummy, 8);
				outstr(",\\");
				nl();
			}

			match(",");
		}

		nl();
		ol(".code");

		if (numericarg != 10)
			error("You must enter the VRAM address, the default palette and 8 values for pattern");

		kill();
	}
	else
	if (amatch("spr", 3)) {
		if (!match("("))
			error("missing (");

		ol(".data");

		readstr();	/* read the label name */
		prefix();
		outstr(litq2);
		outstr(":");
		addglb_far(litq2, CINT);

		if (!match(",")) {
			error("missing ,");
			kill();
			return;
		}

		ot(".defspr ");

		numericarg = 0;

		while (!match(")")) {
			numericarg++;

			number(&dummy);

			switch (numericarg) {
			case 1:
				outhexfix(dummy, 4);
				outstr(",");
				break;
			case 2:
				outdec(dummy);
				outstr(",\\");
				nl();
				break;
			case 34:
				outhexfix(dummy, 8);
				break;
			default:
				outhexfix(dummy, 8);
				outstr(",");

				if (!(numericarg & 1)) {
					outstr("\\");
					nl();
				}
			}

			match(",");
		}

		nl();
		ol(".code");

		if (numericarg != 34)
			error("You must enter the VRAM address, the default palette and 32 values for pattern");

		kill();
	}
	else {
		error("Unknown define directive");
		kill();
	}
	return;
}

void do_asm_func (int type)
{
	/* syntax is
	   name of the data : identifier
	 */
	char n[NAMESIZE];
	char *ptr;

	/* get identifier */
	if (!symname(n)) {
		error("invalid identifier");
		kill();
		return;
	}

	/* close function */
	needbrack(")");

	/* duplicate identifier */
	ptr = new_string(1, n);

	/* gen code */
	if (ptr)
		out_ins(I_LDWI, type, (intptr_t)ptr);
	else
		error("out of memory");
}

char *new_string (int und, char *a)
{
	int len;
	int tmp;

	if (a == NULL)
		return (NULL);

	len = (int)strlen(a);
	tmp = str_idx;
	if ((len + str_idx) > 0xFFF0)
		return (NULL);

	if (und)
		str_buf[str_idx++] = '_';
	strcpy(&str_buf[str_idx], a);
	str_idx += len;
	str_buf[str_idx++] = '\0';
	return (&str_buf[tmp]);
}

void do_inc_ex (int type)
{
	int end;
	int i;
	int j;
	int num;
	int nb_tile;
	char label[NAMESIZE];
	char label2[NAMESIZE];
	char str[NAMESIZE + 32];

	struct {
		char fname[FILENAMESIZE];
		int arg[5];
	} tiles[16];

	if (!match("(")) {
		error("missing '('");
		kill();
		return;
	}

	readstr();	/* read the label name */
	strcpy(label, litq2);
	strcpy(label2, litq2);
	strcpy(str, "__data__");
	strcat(label2, str);
	addglb(label2, ARRAY, CINT, 0, EXTERN, 0);
	addglb(label, ARRAY, CINT, 0, EXTERN, 0);

	if (!match(",")) {
		error("comma missing");
		kill();
		return;
	}

	end = 0;
	num = 0;
	nb_tile = 0;
	while (!end) {
		if (match("\\")) {};
		if (!readqstr()) {
			error("not a file name");
			kill();
			return;
		}
		if (!match(",")) {
			error("comma missing");
			kill();
			return;
		}
		strcpy(tiles[num].fname, litq2);

		for (i = 0; i < 5; i++) {
			if (match("\\")) {};
			if (!number(&tiles[num].arg[i])) {
				error("not a number");
				kill();
				return;
			}
			if (match(")")) {
				if (i == 4) {
					kill();
					end = 1;
					break;
				}
				else {
					error("arg missing");
					kill();
					return;
				}
			}
			if (!match(",")) {
				error("comma missing");
				kill();
				return;
			}
			while ((ch() == ' ') || (ch() == '\t'))
				gch();
			if (ch() == '\0') {
				error("arg missing");
				kill();
				return;
			}
		}
		nb_tile += tiles[num].arg[2] * tiles[num].arg[3];
		num++;
		if (num == 16) {
			if (!end) {
				error("too many args (max 16 files)");
				kill();
				return;
			}
		}
	}

	/* create const array to hold extra infos */
	new_const();
	const_val[const_val_idx++] = const_data_idx;	/* number of tile */
	sprintf(str, "%i", nb_tile);
	add_buffer(str, '(', 1);
	const_data[const_data_idx++] = '\0';
	const_val[const_val_idx++] = const_data_idx;	/* tile size */
	sprintf(str, "%i", (int)type);
	add_buffer(str, '(', 1);
	const_data[const_data_idx++] = '\0';
	const_val[const_val_idx++] = const_data_idx;	/* tile bank */
	sprintf(str, "BANK(_%s)", label2);
	add_buffer(str, '(', 1);
	const_data[const_data_idx++] = '\0';
	const_val[const_val_idx++] = const_data_idx;	/* tile addr */
	sprintf(str, "     _%s", label2);
	add_buffer(str, '(', 1);
	const_data[const_data_idx++] = '\0';
	const_val[const_val_idx++] = -(litptr + 1024);	/* pal idx table addr */
	add_const(CINT);

	/* create pal idx table */
	for (i = 0; i < num; i++) {
		j = tiles[i].arg[2] * tiles[i].arg[3];
		while (j) {
			j--;
			if (litptr < LITMAX)
				litq[litptr++] = (tiles[i].arg[4] << 4);
		}
	}

	/* dump incchr/tile cmds */
	ol(".data");
	prefix();
	outstr(label2);
	outstr(":\n");
	for (i = 0; i < num; i++) {
		if (type == 8)
			ot(".incchr \"");
		else
			ot(".inctile \"");
		outstr(tiles[i].fname);
		outstr("\"");
		for (j = 0; j < 4; j++) {
			outstr(",");
			outdec(tiles[i].arg[j]);
		}
		nl();
	}
	ol(".code");
	kill();
}
