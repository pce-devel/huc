/*	File pragma.c: 2.1 (00/08/09,04:59:24) */
/*% cc -O -c %
 *
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "defs.h"
#include "data.h"
#include "error.h"
#include "io.h"
#include "lex.h"
#include "pragma.h"
#include "sym.h"
#include "fastcall.h"

/* locals */
struct fastcall ftemp;
struct fastcall *fastcall_tbl[256];
static char cmd[LINESIZE];
static char *cmdptr;

/* default pragma's */
static char *pragma_init[] = {
	/* far pointer support funcs */
	"fastcall farpeekb(farptr __fbank:__fptr)",
	"fastcall farpeekw(farptr __fbank:__fptr)",
	"fastcall farmemget(word __bx, farptr __fbank:__fptr, word acc)",
	/* asm-lib wrappers */
	"fastcall load_palette(byte __al, farptr __bl:__si, byte __cl)",
	"fastcall load_bat(word __di, farptr __bl:__si, byte __cl, byte __ch)",
	"fastcall load_vram(word __di, farptr __bl:__si, word __cx)",
	"fastcall load_vram2(word __di, word __si, byte __bl, word __cx)",
	"fastcall snd_trkreg(byte __al, farptr __bl:__si)",
	/* text funcs */
	"fastcall cls(word __dx)",
	"fastcall set_xres(word __ax)",
	"fastcall set_xres(word __ax, byte __cl)",
	"fastcall set_font_color(byte __al, byte acc)",
	"fastcall load_font(farptr __bl:__si, byte __cl)",
	"fastcall load_font(farptr __bl:__si, byte __cl, word __di)",
	"fastcall load_default_font(byte __dl)",
	"fastcall load_default_font(byte __dl, word __di)",
	"fastcall put_digit(byte __dl, word acc)",
	"fastcall put_digit(byte __dl, byte __cl, byte acc)",
	"fastcall put_char(byte __dl, word acc)",
	"fastcall put_char(byte __dl, byte __cl, byte acc)",
	"fastcall put_raw(word __dx, word acc)",
	"fastcall put_raw(word __dx, byte __cl, byte acc)",
	"fastcall put_number(word __dx, byte __cl, word acc)",
	"fastcall put_number(word __dx, byte __cl, byte __bl, byte acc)",
	"fastcall put_hex(word __dx, byte __cl, word acc)",
	"fastcall put_hex(word __dx, byte __cl, byte __bl, byte acc)",
	"fastcall put_string(word __si, word acc)",
	"fastcall put_string(word __si, byte __bl, byte acc)",
	/* gfx lib funcs */
	"fastcall gfx_plot(word __bx, word __cx, word acc)",
	"fastcall gfx_point(word __bx, word __cx)",
	"fastcall gfx_line(word __bx, word __cx, word __si, word __bp, word acc)",

	"fastcall vram_addr(byte __al, byte acc)",
	"fastcall spr_ctrl(byte __al, byte acc)",
	"fastcall get_color(word color_reg)",
	"fastcall set_color(word color_reg, word color_data) nop",
	"fastcall set_color_rgb(word color_reg, byte __al, byte __ah, byte acc)",
	"fastcall fade_color(word __ax, byte acc)",
	"fastcall fade_color(word color_reg, word __ax, byte acc)",
	/* map lib funcs */
	"fastcall scan_map_table(word __si, word __ax, word __cx)",
	"fastcall load_map(byte __al, byte __ah, word __di, word __bx, byte __dl, byte __dh)",
	"fastcall set_map_data(word acc)",
	"fastcall set_map_data(farptr __bl:__si, word __ax, word acc)",
	"fastcall set_map_data(farptr __bl:__si, word __ax, word __dx, byte acc)",
	"fastcall set_tile_data(word __di)",
	"fastcall set_tile_data(farptr __bl:__si, word __cx, farptr __al:__dx, byte __ah)",
	"fastcall put_tile(word __dx, word acc)",
	"fastcall put_tile(word __dx, byte __al, byte acc)",
	"fastcall map_get_tile(byte __dl, byte acc)",
	"fastcall map_put_tile(byte __dl, byte __dh, byte acc)",
	/* misc funcs */
	"fastcall get_joy_events(byte acc)",
	"fastcall get_joy_events(byte __al, byte acc)",
	"fastcall set_joy_callback(byte __dl, byte __al, byte __ah, farptr __bl:__si)",
	"fastcall poke(word __bx, word acc)",
	"fastcall pokew(word __bx, word acc)",
	"fastcall srand32(word __dx, word __cx)",
	/* 32-bit math funcs */
	"fastcall mov32(word __di, dword acc:__ax|__bx)",
	"fastcall add32(word __di, dword acc:__ax|__bx)",
	"fastcall sub32(word __di, dword acc:__ax|__bx)",
	"fastcall mul32(word __bp, dword acc:__ax|__bx)",
	"fastcall div32(word __bp, dword acc:__ax|__bx)",
	"fastcall cmp32(word __di, dword acc:__ax|__bx)",
	"fastcall com32(word __di)",
	/* bcd math funcs */
	"fastcall bcd_init(word __bx, word acc)",
	"fastcall bcd_set(word __bx, word acc)",
	"fastcall bcd_mov(word __bx, word acc)",
	"fastcall bcd_add(word __di, word acc)",
	/* bram funcs */
	"fastcall bm_rawwrite(word __bx, word acc)",
	"fastcall bm_read(word __di, word __bx, word __bp, word acc)",
	"fastcall bm_write(word __di, word __bx, word __bp, word acc)",
	"fastcall bm_create(word __bx, word acc)",
	"fastcall bm_getptr(word __bp, word acc)",
	/* string funcs */
	"fastcall strcpy(word __di, word __si)",
	"fastcall strncpy(word __di, word __si, word acc)",
	"fastcall strcat(word __di, word __si)",
	"fastcall strncat(word __di, word __si, word acc)",
	"fastcall strcmp(word __di, word __si)",
	"fastcall strncmp(word __di, word __si, word acc)",
	"fastcall strlen(word __si)",
	"fastcall memcpy(word __di, word __si, word acc)",
	"fastcall mempcpy(word __di, word __si, word acc)",
	"fastcall memcmp(word __di, word __si, word acc)",
	"fastcall memset(word __di, word __bx, word acc)",
	/* CDROM funcs */
	"fastcall cd_trkinfo(word __ax, word __cx, word __dx, word __bp)",
	"fastcall cd_playtrk(word __bx, word __cx, word acc)",
	"fastcall cd_playmsf(byte __al, byte __ah, byte __bl, byte __cl, byte __ch, byte __dl, word acc)",
	"fastcall cd_loadvram(word __di, word __si, word __bx, word acc)",
	"fastcall cd_loaddata(word __di, word __si, farptr __bl:__bp, word acc)",
	/* ADPCM funcs */
	"fastcall ad_trans(word __di, word __si, byte __al, word __bx)",
	"fastcall ad_read(word __cx, byte __dh, word __bx, word __ax)",
	"fastcall ad_write(word __cx, byte __dh, word __bx, word __ax)",
	"fastcall ad_play(word __bx, word __ax, byte __dh, byte __dl)",

	"fastcall __builtin_ffs(word acc)",
	NULL
};

/* protos */
intptr_t fastcall_look (const char *fname, intptr_t nargs, struct fastcall **p);


/* ----
 * dopragma()
 * ----
 * handle pragma directive
 *
 */
void dopragma (void)
{
	intptr_t i;

	/* make a local copy of the pragma command line */
	for (i = 0;; i++) {
		if (ch() == 0)
			break;
		cmd[i] = gch();
	}
	cmd[i] = 0;

	/* parse */
	parse_pragma();
}


/* ----
 * defpragma()
 * ----
 * default pragmas
 *
 */
void defpragma (void)
{
	intptr_t i;

	for (i = 0;; i++) {
		if (pragma_init[i] == NULL)
			break;
		strcpy(cmd, pragma_init[i]);
		parse_pragma();
	}
}


/* ----
 * parse_pragma()
 * ----
 * parse pragma command line
 *
 */
void parse_pragma (void)
{
	char sname[NAMESIZE];

	/* get command */
	cmdptr = cmd;

	if (!symget(sname)) {
		error("illegal symbol name");
		return;
	}

	/* fastcall */
	if (strcmp(sname, "fastcall") == 0)
		new_fastcall();
/*		new_fastcall(sname); */
	else if (!strcmp(sname, "no_recursive"))
		norecurse = 1;
	else if (!strcmp(sname, "recursive"))
		norecurse = 0;
	/* others */
	else
		error("unknown pragma");
}


/* ----
 * new_fastcall()
 * ----
 * setup a new fastcall
 *
 * ie. #pragma fastcall func(word __dx, byte __al, byte __ah)
 *
 */
void new_fastcall (void)
{
	struct fastcall *ptr;
	char fname[NAMESIZE];
	char sname[NAMESIZE];
	intptr_t hash;
	intptr_t cnt;
	intptr_t i;

	ptr = &ftemp;
	cnt = 0;
	ptr->nargs = 0;
	ptr->flags = 0;

	/* get func name */
	if (!symget(fname)) {
		error("illegal symbol name");
		return;
	}

	/* open */
	if (!strmatch("(")) {
		error("missing bracket");
		return;
	}

	/* extract args (max. 8) */
	for (i = 0; i < 8; i++) {
		/* get type */
		if (!symget(sname)) {
			if (*cmdptr == ')')
				break;
			error("syntax error");
			return;
		}
		if (strcmp(sname, "byte") == 0)
			ptr->argtype[i] = TYPE_BYTE;
		else if (strcmp(sname, "word") == 0)
			ptr->argtype[i] = TYPE_WORD;
		else if (strcmp(sname, "farptr") == 0)
			ptr->argtype[i] = TYPE_FARPTR;
		else if (strcmp(sname, "dword") == 0)
			ptr->argtype[i] = TYPE_DWORD;
		else {
			error("fastcall unknown type");
			return;
		}

		/* get name */
		if (!symget(sname)) {
			/* auto */
			if (*cmdptr != ',')
				ptr->argtype[i] = TYPE_ACC;
			else {
				error("fastcall register missing");
				return;
			}
		}
		else {
			/* dword */
			if (ptr->argtype[i] == TYPE_DWORD) {
				/* ptr */
				if (strcmp(sname, "acc") == 0)
					strcpy(ptr->argname[i++], "#acc");
				else
					strcpy(ptr->argname[i++], sname);

				/* low word */
				if (*cmdptr++ != ':') {
					error("syntax error");
					return;
				}
				if (!symget(sname)) {
					error("illegal symbol name");
					return;
				}

				/* copy */
				strcpy(ptr->argname[i++], sname);

				/* high word */
				if (*cmdptr++ != '|') {
					error("syntax error");
					return;
				}
				if (!symget(sname)) {
					error("illegal symbol name");
					return;
				}

				/* copy */
				strcpy(ptr->argname[i], sname);
				cnt++;
			}

			/* far ptr */
			else if (ptr->argtype[i] == TYPE_FARPTR) {
				/* bank */
				strcpy(ptr->argname[i++], sname);
				ptr->argtype[i] = TYPE_WORD;

				/* addr */
				if (*cmdptr++ != ':') {
					error("syntax error");
					return;
				}
				if (!symget(sname)) {
					error("illegal symbol name");
					return;
				}

				/* copy */
				strcpy(ptr->argname[i], sname);
				cnt++;
			}

			/* other */
			else {
				if (strcmp(sname, "acc") == 0) {
					/* accumulator */
					ptr->argtype[i] = TYPE_ACC;
				}
				else {
					/* variable */
					strcpy(ptr->argname[i], sname);
					cnt++;
				}
			}
		}

		/* increment arg counter */
		ptr->nargs++;

		/* next */
		if (!strmatch(","))
			break;
	}

	/* close */
	if (!strmatch(")")) {
		error("missing bracket");
		return;
	}

	/* extra infos */
	if (cnt) {
		if (ptr->nargs > 1)
			ptr->flags |= 0x02;
	}
	if (symget(sname)) {
		if (strcmp(sname, "nop") == 0)
			ptr->flags |= FASTCALL_NOP;
		if (strcmp(sname, "macro") == 0)
			ptr->flags |= FASTCALL_MACRO;
	}

	/* check arg number */
	if (ptr->nargs == 0)
		return;

	/* copy func name */
	strcpy(ptr->fname, fname);

	/* search for multi-decl */
	if (fastcall_look(fname, ptr->nargs, NULL)) {
		error("already defined");
		return;
	}

	/* ok */
	ptr = (void *)malloc(sizeof(struct fastcall));

	if (ptr == NULL)
		error("out of memory");
	else {
		/* dup struct */
		*ptr = ftemp;

		/* add to hash table */
		hash = symhash(fname);
		ptr->next = fastcall_tbl[hash];
		fastcall_tbl[hash] = ptr;
	}
}


/* ----
 * fastcall_look()
 * ----
 * search a fastcall function
 *
 */
intptr_t fastcall_look (const char *fname, intptr_t nargs, struct fastcall **p)
{
	struct fastcall *ptr;
	struct fastcall *ref;
	intptr_t hash;
	intptr_t nb;

	/* search */
	hash = symhash(fname);
	ptr = fastcall_tbl[hash];
	ref = NULL;
	nb = 0;
	while (ptr) {
		if (strcmp(ptr->fname, fname) == 0) {
			nb++;
			if (nargs != -1) {
				if (ptr->nargs == nargs)
					ref = ptr;
			}
		}
		ptr = ptr->next;
	}
	if (nargs != -1) {
		if (!ref)
			nb = 0;
	}

	/* return result */
	if (p)
		*p = ref;
	return (nb);
}


/* ----
 * symhash()
 * ----
 * calculate the hash value of a symbol
 *
 */
intptr_t symhash (const char *sym)
{
	intptr_t i;
	char c;
	intptr_t hash = 0;

	/* calc hash value */
	for (i = 0;; i++) {
		c = sym[i];
		if (c == 0)
			break;
		hash += c;
		hash = (hash << 3) + (hash >> 5) + c;
	}

	/* ok */
	return (hash & 0xFF);
}


/* ----
 * symget()
 * ----
 * extract a symbol name
 *
 */
intptr_t symget (char *sname)
{
	intptr_t i;

	skip_blanks();

	/* first char must be alpha */
	if (!alpha(*cmdptr))
		return (0);

	/* extract symbol name (stops at first non-alphanum char) */
	for (i = 0;; i++) {
		if (!an(*cmdptr))
			break;
		sname[i] = *cmdptr++;
	}
	sname[i] = 0;

	/* ok */
	return (1);
}


/* ----
 * strmatch()
 * ----
 * test if next input string is legal symbol name
 *
 */
intptr_t strmatch (char *lit)
{
	intptr_t i;

	skip_blanks();

	/* compare */
	i = streq(cmdptr, lit);

	if (i) {
		/* match */
		cmdptr += i;
		return (1);
	}

	/* different */
	return (0);
}


/* ----
 * skip_blanks()
 * ----
 * skips blank chars (stops at end of input line)
 *
 */
void skip_blanks (void)
{
	while ((*cmdptr == ' ') || (*cmdptr == '\t'))
		cmdptr++;
}
