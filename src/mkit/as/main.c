/*
 *  MagicKit assembler
 *  ----
 *  This program was originaly a 6502 assembler written by J. H. Van Ornum,
 *  it has been modified and enhanced to support the PC Engine and NES consoles
 *  and the Atari 8-bit home computers.
 *
 *  This program is freeware. You are free to distribute, use and modifiy it
 *  as you wish.
 *
 *  Enjoy!
 *  ----
 *  Original 6502 version by:
 *    J. H. Van Ornum
 *
 *  PC-Engine version by:
 *    David Michel
 *    Dave Shadoff
 *
 *  NES version by:
 *    Charles Doty
 *
 *  ATARI version by:
 *    John Brandwood
 */

#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <ctype.h>
#include "defs.h"
#include "externs.h"
#include "protos.h"
#include "vars.h"
#include "inst.h"
#include "../../../include/overlay.h"

/* defines */
#define STANDARD_CD	1
#define SUPER_CD	2

/* variables */
unsigned char ipl_buffer[4096];
char in_fname[256];	/* file names, input */
char out_fname[256];	/* output */
char bin_fname[256];	/* binary */
char lst_fname[256];	/* listing */
char sym_fname[256];	/* symbol table */
char zeroes[2048];	/* CDROM sector full of zeores */
char *prg_name;		/* program name */
FILE *in_fp;		/* file pointers, input */
FILE *lst_fp;		/* listing */
char section_name[4][8] = {
	"  ZP", " BSS", "CODE", "DATA"
};
int dump_seg;
int overlayflag;
int develo_opt;
int header_opt;
int padding_opt;
int srec_opt;
int run_opt;
int scd_opt;
int cd_opt;
int mx_opt;
int mlist_opt;		/* macro listing main flag */
int xlist;		/* listing file main flag */
int list_level;		/* output level */
int asm_opt[8];		/* assembler options */
int zero_need;		/* counter for trailing empty sectors on CDROM */


/* ----
 * prepare_ipl()
 * ----
 */

unsigned char *
prepare_ipl(unsigned char * dst)
{
	unsigned cmd;
	unsigned len;
	unsigned char * src;
	unsigned char * win;

	static unsigned char disc_header [1047] = {
		0x71,0x36,0x28,0x1B,0x28,0x66,0x29,0xDC,0x29,0x27,0x29,0xE5,0x29,0x23,0x29,0x2A,
		0x28,0x66,0x38,0x32,0x27,0x46,0x26,0x0A,0x28,0x67,0x20,0x3E,0x24,0x04,0x23,0x45,
		0x24,0x7A,0xAA,0x29,0xC4,0x29,0xC2,0x29,0xF6,0x29,0x39,0x28,0x00,0x25,0x20,0x3D,
		0xE6,0x28,0x1F,0x28,0x6E,0x28,0x02,0x28,0x42,0x28,0x76,0x28,0x1D,0x2B,0xE8,0xC3,
		0x19,0xAA,0xC2,0x40,0x22,0x40,0x3F,0x3E,0xE5,0x50,0x17,0x28,0x67,0x3B,0xF9,0xF6,
		0x11,0x5A,0xBA,0x11,0xAA,0xB9,0x06,0xBA,0x70,0x21,0x66,0x25,0x3B,0x3C,0x60,0x28,
		0x63,0x28,0x4C,0x28,0x43,0x21,0x3C,0x23,0x68,0x28,0x62,0x28,0x07,0xAA,0x3F,0x0B,
		0x24,0x60,0x2B,0xEF,0x3F,0x0B,0x3A,0x11,0x2B,0xEF,0x39,0xF7,0x3D,0xDA,0x28,0x1D,
		0x28,0x43,0xA5,0x33,0x6C,0x28,0x67,0x72,0x32,0xAA,0x3C,0xEA,0xCE,0x79,0x02,0x42,
		0x21,0x7C,0x28,0x1C,0x28,0x4D,0x28,0x40,0x7C,0x72,0x2C,0xAA,0x55,0xFA,0xF8,0xE5,
		0xEE,0xFF,0xE9,0xEF,0xF8,0xAA,0x38,0x2C,0x3C,0xD1,0x3A,0xE6,0x22,0x40,0xAA,0xEE,
		0xE3,0xF8,0xEF,0xE9,0xFE,0xE5,0xF8,0xAA,0x3C,0x46,0x3B,0x58,0x25,0x35,0x27,0xE6,
		0xAA,0xE9,0xEE,0x87,0xF8,0xE5,0xE7,0x8A,0xF9,0xE3,0xE7,0xFF,0xE6,0xEB,0xE6,0x70,
		0x11,0xD1,0x3E,0x3D,0x3C,0xEC,0x3F,0xDC,0xAA,0xE8,0xE3,0xE5,0xF9,0x8A,0xE7,0xEB,
		0xE3,0xE4,0x8A,0xE9,0xE5,0xEE,0xEF,0x86,0x8A,0xD6,0x40,0xFA,0xE6,0xEB,0xF3,0xB1,
		0x73,0x01,0x25,0x06,0x3D,0x7B,0x26,0xC2,0x24,0x5D,0xDD,0x70,0x03,0xE9,0xE2,0xEF,
		0xE9,0xE1,0x8A,0xE7,0xE5,0xE4,0xE3,0xC1,0x60,0x86,0x8A,0xF9,0xF8,0xEB,0xE7,0xC9,
		0x30,0xE4,0xEB,0xED,0xD6,0x72,0x0F,0x3E,0x58,0x39,0xC9,0x23,0x41,0x27,0xED,0xAA,
		0xE8,0xEB,0xE9,0xE1,0xFF,0xFA,0x8A,0xE7,0xEF,0xE7,0xE5,0xF8,0xF3,0xAA,0x76,0x0A,
		0xFE,0xEF,0xE4,0xEB,0xE4,0xE9,0xEF,0xAA,0x21,0x4A,0x39,0xC9,0x27,0xEC,0x24,0xC3,
		0xAA,0xA0,0x77,0x22,0x8A,0xF9,0xFF,0xE8,0xAA,0x24,0xE5,0x25,0x49,0x2B,0xEA,0x39,
		0xE4,0xAA,0xFA,0xF9,0xED,0x8A,0xEE,0xF8,0xE3,0xFC,0xEF,0xF8,0xAA,0x20,0x48,0x3F,
		0x09,0x21,0xE7,0x27,0xE1,0xAA,0xED,0xF8,0xEB,0xFA,0xE2,0xE3,0xE9,0xE8,0x6A,0x27,
		0x42,0x26,0xF1,0x3A,0xF4,0xE8,0x04,0xB8,0x7A,0x15,0x21,0xD3,0x3A,0x46,0x27,0x24,
		0x3E,0xFC,0xAA,0x92,0xD2,0x92,0x8A,0xEB,0xE4,0xE1,0x8A,0xEC,0xE5,0xE4,0xFE,0xAA,
		0x25,0x16,0x23,0xF3,0x27,0xF5,0x8B,0x70,0x06,0xEE,0xEF,0xF9,0xE3,0xED,0xE4,0xAA,
		0x22,0x63,0x39,0x0B,0x3A,0xF4,0x24,0x55,0xFE,0xE3,0xFE,0xE6,0xEF,0xEA,0x73,0x0D,
		0x3F,0x3A,0x39,0xC9,0x3A,0xF4,0x3D,0x37,0xAA,0x9B,0x98,0xD2,0x9B,0x98,0x8A,0xE1,
		0xEB,0xE4,0xE0,0xE3,0xB6,0x40,0x20,0xDA,0x38,0xE0,0x58,0x1F,0x6E,0xE6,0x00,0x7F,
		0x01,0x24,0xF8,0x26,0x51,0x28,0x4A,0x28,0x6C,0xE6,0x00,0x7F,0x20,0x23,0x00,0x39,
		0xC9,0x24,0x5F,0x3F,0xDC,0xAA,0xE4,0xEF,0xE9,0x8A,0xE2,0xC5,0xC7,0xCF,0x8A,0xEF,
		0xC6,0xCF,0xC9,0xDE,0xC5,0xD8,0xC5,0xC4,0xC3,0xC9,0xD9,0xAA,0x3D,0x43,0x3C,0x72,
		0x27,0xEC,0x24,0x0B,0xE1,0x05,0x7F,0x01,0x3C,0xDB,0x3C,0x46,0x2B,0xEA,0x27,0xEC,
		0xE1,0x05,0x7F,0x01,0x23,0xDC,0x24,0xDB,0x3B,0x7D,0x25,0x04,0xC2,0x06,0x6F,0x46,
		0x27,0x48,0x32,0xCB,0x3F,0xC2,0x06,0x71,0x92,0x3A,0xFC,0x22,0x4E,0x27,0xED,0x25,
		0x10,0xAA,0x55,0x9B,0x93,0x92,0x92,0x8A,0xF9,0xCF,0xDA,0x84,0x8A,0xFD,0xD8,0xC3,
		0xDE,0xDE,0xCF,0xC4,0x8A,0xC8,0xD3,0x8A,0xFE,0xEB,0xE1,0xEB,0xE1,0xE3,0x8A,0xE1,
		0xE5,0xE8,0xEB,0xF3,0xEB,0xF9,0xE2,0xE3,0xAA,0x68,0x08,0xE0,0x03,0x8A,0x2F,0x50,
		0x03,0x9A,0x2F,0x51,0x03,0x1C,0x2F,0x56,0x03,0x75,0x2F,0x57,0x1B,0x50,0x7B,0x56,
		0x5A,0xAB,0xCA,0x62,0x60,0x7A,0x5F,0x07,0xA7,0x9A,0x7A,0xA8,0x03,0xCA,0x2F,0xAA,
		0x25,0xAA,0xA8,0x2A,0xDC,0x8A,0x33,0x4A,0x07,0xBB,0x9A,0x5A,0xAE,0x63,0x88,0x3A,
		0xA8,0x2A,0xB6,0xD9,0xA4,0x9A,0x56,0x8A,0xA9,0xAA,0xCE,0x55,0xCE,0x50,0x03,0x92,
		0x2F,0x51,0xCE,0x52,0x03,0xA8,0x2F,0x53,0x8A,0xA3,0x4A,0x63,0xAA,0x5A,0xA8,0x2A,
		0xD6,0x03,0xBA,0x27,0x88,0x88,0x27,0x8F,0x88,0x03,0xAA,0x27,0x8A,0x88,0x03,0x92,
		0x27,0x8B,0xF6,0x71,0x16,0x89,0x88,0x03,0x93,0x27,0x8E,0x88,0x03,0xA8,0x27,0xB5,
		0x88,0x07,0xBA,0x9A,0xB2,0xC3,0xAB,0x2F,0x54,0x07,0xA5,0x9A,0xC3,0xAA,0x2F,0x57,
		0x07,0xA4,0xF9,0x75,0x06,0x56,0x03,0x55,0x2F,0x55,0x03,0xAB,0x2F,0x52,0xCE,0x50,
		0xCE,0x51,0xB4,0x12,0x98,0xD5,0x1F,0xA8,0xD5,0x02,0x00,0x71,0x75,0x04,0x2F,0x52,
		0x07,0xB8,0x9A,0x2F,0x50,0x07,0xB9,0x9A,0x2F,0xCE,0x72,0x00,0xAD,0xAA,0xB5,0xAA,
		0xEE,0xD9,0xBE,0x61,0x70,0x02,0x07,0xBD,0x9A,0x5A,0x92,0x63,0x8B,0x1A,0x9E,0xAC,
		0x12,0x55,0xAA,0x10,0x99,0xDC,0x51,0x7A,0x8F,0x45,0xAA,0x88,0xF2,0x20,0xCE,0x52,
		0xDF,0x50,0xA0,0xA0,0xA0,0x7A,0xAE,0xB1,0x73,0x26,0x52,0x2F,0x53,0x07,0xB2,0x9A,
		0x2F,0x55,0x0F,0xAA,0x83,0x2A,0x2F,0x54,0x8A,0x96,0x4A,0x75,0xAA,0xAC,0xA5,0xAA,
		0xA9,0x8A,0x20,0x4A,0x03,0x55,0xF9,0xAB,0x03,0x52,0xF9,0xA8,0x07,0xA2,0x9A,0xB2,
		0xC7,0x5F,0x55,0xF9,0xAE,0x07,0xA3,0xF7,0x33,0xA2,0x07,0xA0,0xF7,0x33,0xBA,0x07,
		0xA1,0xF7,0x33,0x8A,0x07,0xA6,0xF7,0x71,0x17,0xEA,0xD9,0xAA,0x9A,0xAB,0x8A,0xA2,
		0xAA,0x0F,0xAE,0x7A,0xA5,0xB2,0x0F,0xAD,0xC3,0xAA,0x2F,0xAD,0x0F,0xA2,0xC3,0x9A,
		0x2F,0xA2,0x2A,0x99,0xD9,0xAB,0x8A,0x64,0x20,0x03,0xAB,0x94,0x10,0xAE,0x67,0x72,
		0x15,0x53,0x03,0x55,0x6F,0xAF,0x03,0x85,0x4F,0xAC,0x3A,0xA0,0xD9,0x6A,0x86,0x4A,
		0x75,0xBF,0xAA,0xE6,0x4A,0x75,0xD9,0xAF,0x8A,0x50,0x8A,0xA8,0xAA,0x2C,0x7F,0x03,
		0x7A,0xAC,0x08,0x55,0x30,0xC6,0xAD,0x8A,0xCA,0xAA,0xFF,0xEE,0x4A,0x03,0xF5,0x03,
		0xFA,0xE9,0x8A,0xEF,0xC4,0xCD,0xC3,0xC4,0xCF,0x8A,0xBD,0xF8,0xFB,0x17,0xF3,0xF9,
		0xFE,0xEF,0xE7,0xAA,0xE9,0xC5,0xDA,0xD3,0xD8,0xC3,0xCD,0xC2,0xDE,0x8A,0xE2,0xFF,
		0xEE,0xF9,0xE5,0xE4,0x8A,0xF9,0xE5,0xEC,0xFE,0x8A,0x85,0x8A,0x90,0xFA,0x83,0x91,
		0xFA,0x7F,0x00,0x86,0xE6,0xDE,0xCE,0x84,0xAA,0x8A,0xFF,0x03,0x1F,0xAA,0xFF,0xEE,
		0x7F,0x07,0x0F,0x00,0xEE,0x00,0x00
	};

	src = disc_header;

	for (;;)
	{
		cmd = *src++;

		len = (cmd & 112) >> 4;
		if (len == 7) {
			len = *src++ + 7;
			if (len == 256) { len = src[0] + 256 * src[1]; src += 2; }
			else
			if (len == 257) { len = *src++ + 256; }
		}
		while (len--) *dst++ = 0xAAu ^ *src++;
		signed sOff = *src++ - 256;
		if (cmd & 128) {
			sOff	= (sOff ^ 65280) | (256 * *src++);
		}
		win = dst + sOff;
		len = (cmd & 15) + 3;
		if (len == 18) {
			len = *src++ + 18;
			if (len == 256) { len = src[0] + 256 * src[1]; src += 2; if (len == 0) break; }
			else
			if (len == 257) { len = *src++ + 256; }
		}
		while (len--) *dst++ = *win++;
	}

	return(dst);
}


/* ----
 * main()
 * ----
 */

int
main(int argc, char **argv)
{
	FILE *fp;
	char *p;
	char cmd[256];
	int i, j;
	int file;
	int ram_bank;

	/* get program name */
	if ((prg_name = strrchr(argv[0], '/')) != NULL)
		prg_name++;
	else {
		if ((prg_name = strrchr(argv[0], '\\')) == NULL)
			prg_name = argv[0];
		else
			prg_name++;
	}

	/* remove extension */
	if ((p = strrchr(prg_name, '.')) != NULL)
		*p = '\0';

	/* machine detection */
	if (!strncasecmp(prg_name, "PCE", 3))
		machine = &pce;
	else
	if (!strncasecmp(prg_name, "NES", 3))
		machine = &nes;
	else
		machine = &fuji;

	/* init assembler options */
	list_level = 2;
	header_opt = 1;
	padding_opt = 0;
	overlayflag = 0;
	develo_opt = 0;
	mlist_opt = 0;
	srec_opt = 0;
	run_opt = 0;
	scd_opt = 0;
	cd_opt = 0;
	mx_opt = 0;
	file = 0;

	/* display assembler version message */
	printf("%s\n\n", machine->asm_title);

	/* parse command line */
	if (argc > 1) {
		for (i = 1; i < argc; i++) {
			if (argv[i][0] == '-') {
				/* segment usage */
				if (!strcmp(argv[i], "-s"))
					dump_seg = 1;
				else if (!strcmp(argv[i], "-S"))
					dump_seg = 2;

				/* forces macros expansion */
				else if (!strcmp(argv[i], "-m"))
					mlist_opt = 1;

				/* no header */
				else if (!strcmp(argv[i], "-raw"))
					header_opt = 0;

				/* pad to power-of-two */
				else if (!strcmp(argv[i], "-pad"))
					padding_opt = 1;

				/* output s-record file */
				else if (!strcmp(argv[i], "-srec"))
					srec_opt = 1;

				/* output level */
				else if (!strncmp(argv[i], "-l", 2)) {
					/* get level */
					if (strlen(argv[i]) == 2)
						list_level = atol(argv[++i]);
					else
						list_level = atol(&argv[i][2]);

					/* check range */
					if (list_level < 0 || list_level > 3)
						list_level = 2;
				}

				/* help */
				else if (!strcmp(argv[i], "-?")) {
					help();
					return (0);
				}

				else {
					/* PCE specific functions */
					if (machine->type == MACHINE_PCE) {
						/* cd-rom */
						if (!strcmp(argv[i], "-cd")) {
							cd_opt = STANDARD_CD;
							scd_opt = 0;
						}

						/* super cd-rom */
						else if (!strcmp(argv[i], "-scd")) {
							scd_opt = SUPER_CD;
							cd_opt = 0;
						}

						/* cd-rom overlay */
						else if (!strcmp(argv[i], "-over"))
							overlayflag = 1;
						else if (!strcmp(argv[i], "-overlay"))
							overlayflag = 1;

						/* develo auto-run */
						else if (!strcmp(argv[i], "-develo"))
							develo_opt = 1;
						else if (!strcmp(argv[i], "-dev"))
							develo_opt = 1;

						/* output mx file */
						else if (!strcmp(argv[i], "-mx"))
							mx_opt = 1;
					}
				}
			}
			else {
				strcpy(in_fname, argv[i]);
				file++;
			}
		}
	}

	if ((overlayflag == 1) &&
	    ((scd_opt == 0) && (cd_opt == 0))) {
		printf("Overlay option only valid for CD or SCD programs\n\n");
		help();
		return (0);
	}

	if (!file) {
		help();
		return (0);
	}

	/* search file extension */
	if ((p = strrchr(in_fname, '.')) != NULL) {
		if (!strchr(p, PATH_SEPARATOR))
			*p = '\0';
		else
			p = NULL;
	}

	/* auto-add file extensions */
	strcpy(out_fname, in_fname);
	strcpy(bin_fname, in_fname);
	strcpy(lst_fname, in_fname);
	strcpy(sym_fname, in_fname);
	strcat(lst_fname, ".lst");
	strcat(sym_fname, ".sym");

	if (overlayflag == 1)
		strcat(bin_fname, ".ovl");
	else if (cd_opt || scd_opt)
		strcat(bin_fname, ".iso");
	else
		strcat(bin_fname, machine->rom_ext);

	if (p)
		*p = '.';
	else
		strcat(in_fname, ".asm");

	/* init include path */
	init_path();

	/* init crc functions */
	crc_init();

	/* open the input file */
	if (open_input(in_fname)) {
		printf("Can not open input file '%s'!\n", in_fname);
		exit(1);
	}

	/* clear the ROM array */
	memset(rom, 0xFF, 8192 * 128);
	memset(map, 0xFF, 8192 * 128);

	/* clear symbol hash tables */
	for (i = 0; i < 256; i++) {
		hash_tbl[i] = NULL;
		macro_tbl[i] = NULL;
		func_tbl[i] = NULL;
		inst_tbl[i] = NULL;
	}

	/* fill the instruction hash table */

	addinst(base_pseudo);

	/* add machine specific instructions and pseudos */
	addinst(machine->base_inst);
	if (machine->plus_inst)
		addinst(machine->plus_inst);
	addinst(machine->pseudo_inst);

	/* predefined symbols */
	lablset("MAGICKIT", 1);
	lablset("DEVELO", develo_opt | mx_opt);
	lablset("CDROM", cd_opt | scd_opt);
	lablset("_bss_end", 0);
	lablset("_bank_base", 0);
	lablset("_nb_bank", 1);
	lablset("_call_bank", 0);

	/* init global variables */
	max_zp = 0x01;
	max_bss = 0x0201;
	max_bank = 0;
	rom_limit = 0x100000;		/* 1MB */
	bank_limit = 0x7F;
	bank_base = 0;
	errcnt = 0;

	if (cd_opt) {
		rom_limit = 0x10000;	/* 64KB */
		bank_limit = 0x07;
	}
	else if (scd_opt) {
		rom_limit = 0x40000;	/* 256KB */
		bank_limit = 0x1F;
	}
	else if (develo_opt || mx_opt) {
		rom_limit = 0x30000;	/* 192KB */
		bank_limit = 0x17;
	}

	/* assemble */
	for (pass = FIRST_PASS; pass <= LAST_PASS; pass++) {
		infile_error = -1;
		page = 7;
		bank = 0;
		loccnt = 0;
		slnum = 0;
		mcounter = 0;
		mcntmax = 0;
		xlist = 0;
		glablptr = NULL;
		skip_lines = 0;
		rsbase = 0;
		proc_nb = 0;

		/* reset assembler options */
		asm_opt[OPT_LIST] = 0;
		asm_opt[OPT_MACRO] = mlist_opt;
		asm_opt[OPT_WARNING] = 0;
		asm_opt[OPT_OPTIMIZE] = 0;

		/* reset bank arrays */
		for (i = 0; i < 4; i++) {
			for (j = 0; j < 256; j++) {
				bank_loccnt[i][j] = 0;
				bank_glabl[i][j] = NULL;
				bank_page[i][j] = 0;
			}
		}

		/* reset sections */
		ram_bank = machine->ram_bank;
		section = S_CODE;

		/* .zp */
		section_bank[S_ZP] = ram_bank;
		bank_page[S_ZP][ram_bank] = machine->ram_page;
		bank_loccnt[S_ZP][ram_bank] = 0x0000;

		/* .bss */
		section_bank[S_BSS] = ram_bank;
		bank_page[S_BSS][ram_bank] = machine->ram_page;
		bank_loccnt[S_BSS][ram_bank] = 0x0200;

		/* .code */
		section_bank[S_CODE] = 0x00;
		bank_page[S_CODE][0x00] = 0x07;
		bank_loccnt[S_CODE][0x00] = 0x0000;

		/* .data */
		section_bank[S_DATA] = 0x00;
		bank_page[S_DATA][0x00] = 0x07;
		bank_loccnt[S_DATA][0x00] = 0x0000;

		/* pass message */
		printf("pass %i\n", pass + 1);

		/* assemble */
		while (readline() != -1) {
			assemble(0);
			if (loccnt > 0x2000) {
				loccnt &= 0x1fff;
				page++;
				bank++;
				if (pass == FIRST_PASS)
					printf("   (Warning. Opcode crossing page boundary $%04X, bank $%02X)\n", (page * 0x2000), bank);
			}
			if (stop_pass)
				break;
		}

		/* relocate procs */
		if (pass == FIRST_PASS)
			proc_reloc();

		/* abord pass on errors */
		if (errcnt) {
			printf("# %d error(s)\n", errcnt);
			exit(1);
			// break;
		}

		/* adjust bank base */
		if (pass == FIRST_PASS)
			bank_base = calc_bank_base();

		/* update predefined symbols */
		if (pass == FIRST_PASS) {
			lablset("_bss_end", machine->ram_base + max_bss);
			lablset("_bank_base", bank_base);
			lablset("_call_bank", bank_base + max_bank + 1);
			lablset("_nb_bank", max_bank + 2);
		}

		/* adjust the symbol table for the develo or for cd-roms */
		if (pass == FIRST_PASS) {
			if (develo_opt || mx_opt || cd_opt || scd_opt)
				lablremap();
		}

		/* rewind input file */
		rewind(in_fp);

		/* open the listing file */
		if (pass == FIRST_PASS) {
			if (xlist && list_level) {
				if ((lst_fp = fopen(lst_fname, "w")) == NULL) {
					printf("Can not open listing file '%s'!\n", lst_fname);
					exit(1);
				}
				fprintf(lst_fp, "#[1]   %s\n", input_file[1].name);
			}
		}
	}

	/* rom */
	if (errcnt == 0) {
		/* cd-rom */
		if (cd_opt || scd_opt) {
			/* open output file */
			if ((fp = fopen(bin_fname, "wb")) == NULL) {
				printf("Can not open output file '%s'!\n", bin_fname);
				exit(1);
			}

			/* boot code */
			if ((header_opt) && (overlayflag == 0)) {
				/* initialize the ipl */
				prepare_ipl(ipl_buffer);

				memset(&ipl_buffer[0x800], 0, 32);
				/* prg sector base */
				ipl_buffer[0x802] = 2;
				/* nb sectors */
				ipl_buffer[0x803] = 16;
				/* loading address */
				ipl_buffer[0x804] = 0x00;
				ipl_buffer[0x805] = 0x40;
				/* starting address */
				ipl_buffer[0x806] = BOOT_ENTRY_POINT & 0xFF;
				ipl_buffer[0x807] = (BOOT_ENTRY_POINT >> 8) & 0xFF;
				/* mpr registers */
				ipl_buffer[0x808] = 0x00;
				ipl_buffer[0x809] = 0x01;
				ipl_buffer[0x80A] = 0x02;
				ipl_buffer[0x80B] = 0x03;
				ipl_buffer[0x80C] = 0x00;	/* boot loader @ $C000 */
				/* load mode */
				ipl_buffer[0x80D] = 0x60;

				/* write boot code */
				fwrite(ipl_buffer, 1, 4096, fp);
			}

			/* write rom */
			fwrite(rom, 8192, (max_bank + 1), fp);

			/* write trailing zeroes to fill */
			/* at least 4 seconds of CDROM */
			if (overlayflag == 0) {
				memset(zeroes, 0, 2048);

				/* calculate number of trailing zero sectors      */
				/* rule 1: track must be at least 6 seconds total */
				zero_need = (6 * 75) - 2 - (4 * (max_bank + 1));

				/* rule 2: track should have at least 2 seconds     */
				/*         of trailing zeroes before an audio track */
				if (zero_need < (2 * 75))
					zero_need = (2 * 75);

				while (zero_need > 0) {
					fwrite(zeroes, 1, 2048, fp);
					zero_need--;
				}
			}

			fclose(fp);
		}

		/* develo box */
		else if (develo_opt || mx_opt) {
			page = (map[0][0] >> 5);

			/* save mx file */
			if ((page + max_bank) < 7)
				/* old format */
				write_srec(out_fname, "mx", page << 13);
			else
				/* new format */
				write_srec(out_fname, "mx", 0xD0000);

			/* execute */
			if (develo_opt) {
				snprintf(cmd, sizeof(cmd), "perun %s", out_fname);
				system(cmd);
			}
		}

		/* save */
		else {
			/* s-record file */
			if (srec_opt)
				write_srec(out_fname, "s28", 0);

			/* binary file */
			else {
				/* pad rom to power-of-two? */
				int num_banks = max_bank + 1;
				if (padding_opt) {
					num_banks = 1;
					while (num_banks <= max_bank) num_banks <<= 1;
				}

				/* open file */
				if ((fp = fopen(bin_fname, "wb")) == NULL) {
					printf("Can not open binary file '%s'!\n", bin_fname);
					exit(1);
				}

				/* write header */
				if (header_opt)
					machine->write_header(fp, num_banks);

				/* write rom */
				fwrite(rom, 8192, num_banks, fp);
				fclose(fp);
			}
		}
	}

	/* close listing file */
	if (xlist && list_level)
		fclose(lst_fp);

	/* close input file */
	fclose(in_fp);

	/* dump the symbol table */
	if ((fp = fopen(sym_fname, "w")) != NULL) {
		labldump(fp);
		fclose(fp);
	}

	/* dump the bank table */
	if (dump_seg)
		show_seg_usage();

	/* warn about 384KB hucard rom size */
	if (cd_opt == 0 && scd_opt == 0 && padding_opt == 0) {
		if (max_bank == 0x2F) {
			printf(
				"\n!!!WARNING!!!\n"
				"Most emulators expect a 384KB HuCard ROM to be in a split-image layout.\n\n"
				"Unless you are patching an existing 384KB HuCard game image, you are\n"
				"almost-certainly not using that layout, and your ROM will crash.\n\n"
				"To avoid problems, add or remove enough data to avoid the 384KB size.\n"
				"!!!WARNING!!!\n\n");
		}
	}

	/* ok */
	return (0);
}


/* ----
 * calc_bank_base()
 * ----
 * calculate rom bank base
 */

int
calc_bank_base(void)
{
	int base;

	/* cd */
	if (cd_opt)
		base = 0x80;

	/* super cd */
	else if (scd_opt)
		base = 0x68;

	/* develo */
	else if (develo_opt || mx_opt) {
		if (max_bank < 4)
			base = 0x84;
		else
			base = 0x68;
	}

	/* default */
	else {
		base = 0;
	}

	return (base);
}


/* ----
 * help()
 * ----
 * show assembler usage
 */

void
help(void)
{
	/* check program name */
	if (strlen(prg_name) == 0)
		prg_name = machine->asm_name;

	/* display help */
	printf("%s [-options] [-? (for help)] infile\n\n", prg_name);
	printf("-s/S       : show segment usage\n");
	printf("-l #       : listing file output level (0-3)\n");
	printf("-m         : force macro expansion in listing\n");
	printf("-raw       : prevent adding a ROM header\n");
	printf("-pad       : pad ROM size to power-of-two\n");
	if (machine->type == MACHINE_PCE) {
		printf("-cd        : create a CD-ROM track image\n");
		printf("-scd       : create a Super CD-ROM track image\n");
		printf("-over(lay) : create an executable 'overlay' program segment\n");
		printf("-dev       : assemble and run on the Develo Box\n");
		printf("-mx        : create a Develo MX file\n");
	}
	printf("-srec  : create a Motorola S-record file\n");
	printf("infile : file to be assembled\n");
	printf("\n");
}


/* ----
 * show_seg_usage()
 * ----
 */

void
show_seg_usage(void)
{
	int i, j;
	int addr, start, stop, nb;
	int rom_used;
	int rom_free;
	int ram_base = machine->ram_base;

	printf("segment usage:\n");
	printf("\n");

	if (max_bank)
		printf("\t\t\t\t    USED/FREE\n");

	/* zp usage */
	if (max_zp <= 1)
		printf("      ZP    -\n");
	else {
		start = ram_base;
		stop = ram_base + (max_zp - 1);
		printf("      ZP    $%04X-$%04X  [%4i]     %4i/%4i\n", start, stop, stop - start + 1, stop - start + 1, 256 - (stop - start + 1));
	}

	/* bss usage */
	if (max_bss <= 0x201)
		printf("     BSS    -\n");
	else {
		start = ram_base + 0x200;
		stop = ram_base + (max_bss - 1);
		printf("     BSS    $%04X-$%04X  [%4i]     %4i/%4i\n\n", start, stop, stop - start + 1, stop - start + 1, 8192 - (stop - start + 1));
	}

	/* bank usage */
	rom_used = 0;
	rom_free = 0;

	/* scan banks */
	for (i = 0; i <= max_bank; i++) {
		start = 0;
		addr = 0;
		nb = 0;

		/* count used and free bytes */
		for (j = 0; j < 8192; j++)
			if (map[i][j] != 0xFF)
				nb++;

		/* update used/free counters */
		rom_used += nb;
		rom_free += 8192 - nb;

		/* display bank infos */
		if (nb)
			printf("BANK %2X  %-23s    %4i/%4i\n",
			       i, bank_name[i], nb, 8192 - nb);
		else {
			printf("BANK %2X  %-23s       0/8192\n", i, bank_name[i]);
			continue;
		}

		/* scan */
		if (dump_seg == 1)
			continue;

		for (;;) {
			/* search section start */
			for (; addr < 8192; addr++)
				if (map[i][addr] != 0xFF)
					break;

			/* check for end of bank */
			if (addr > 8191)
				break;

			/* get section type */
			section = map[i][addr] & 0x0F;
			page = (map[i][addr] & 0xE0) << 8;
			start = addr;

			/* search section end */
			for (; addr < 8192; addr++)
				if ((map[i][addr] & 0x0F) != section)
					break;

			/* display section infos */
			printf("    %s    $%04X-$%04X  [%4i]\n",
			       section_name[section],		/* section name */
			       start + page,			/* starting address */
			       addr + page - 1,			/* end address */
			       addr - start);			/* size */
		}
	}

	/* total */
	rom_used = (rom_used + 1023) >> 10;
	rom_free = (rom_free) >> 10;
	printf("\t\t\t\t    ---- ----\n");
	printf("\t\t\t\t    %4iK%4iK\n", rom_used, rom_free);
	printf("\n\t\t\tTOTAL SIZE =     %4iK\n", (rom_used + rom_free));
}

