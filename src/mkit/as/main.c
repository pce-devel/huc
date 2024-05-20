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
#include <string.h>
#include <ctype.h>
#ifdef _MSC_VER
#include "xgetopt.h"
#else
#include <getopt.h>
#endif
#include "defs.h"
#include "externs.h"
#include "protos.h"
#include "vars.h"
#include "inst.h"
#include "overlay.h"

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
char *section_name[MAX_S + 1] = {
	"NULL",
	"ZP",
	"BSS",
	"CODE",
	"DATA",
	"HOME",
	"XDATA",
	"XINIT",
	"CONST",
	"OSEG",
	"PROC"
};
int newproc_opt;
int strip_opt;
int kickc_opt;
int sdcc_opt;
int dump_seg;
int overlayflag;
int develo_opt;
int trim_opt;
int header_opt;
int padding_opt;
int srec_opt;
int run_opt;
int ipl_opt;
int sgx_opt;
int scd_opt;
int cd_opt;
int sf2_opt;
int mx_opt;
int mlist_opt;		/* macro listing main flag */
int xlist;		/* listing file main flag */
int list_level;		/* output level */
int asm_opt[MAX_OPTS];	/* assembler options */
int zero_need;		/* counter for trailing empty sectors on CDROM */
int rom_used;
int rom_free;

/* current flags for each section */
int section_flags[MAX_S] = {
/* S_NONE  */	S_NO_DATA,
/* S_ZP    */	S_IS_RAM + S_NO_DATA, 
/* S_BSS   */	S_IS_RAM + S_NO_DATA,
/* S_CODE  */	S_IS_ROM + S_IS_CODE,
/* S_DATA  */	S_IS_ROM,
/* S_HOME  */	S_IS_ROM + S_IS_CODE,
/* S_XDATA */	S_IS_RAM + S_NO_DATA,
/* S_XINIT */	S_IS_ROM,
/* S_CONST */	S_IS_ROM,
/* S_OSEG  */	S_IS_RAM
};

/* maximum loccnt limit for each section */
/* N.B. this isn't used, yet! */
int section_limit[MAX_S] = {
/* S_NONE  */	0x0000,
/* S_ZP    */	0x0100,
/* S_BSS   */	0x2000,
/* S_CODE  */	0x2000,
/* S_DATA  */	0x2000,
/* S_HOME  */	0x2000,
/* S_XDATA */	0x2000,
/* S_XINIT */	0x2000,
/* S_CONST */	0x2000,
/* S_OSEG  */	0x0100
};

/* ----
 * atexit callback
 * ----
 */
void
cleanup(void)
{
	cleanup_path();
}

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
	int i, j, opt;
	int ram_bank;
	static t_file *extra_source = NULL;
	static t_file *final_source = NULL;

	static int cd_type;
	static const char *cmd_line_options = "I:OShl:mo:s";
	static const struct option cmd_line_long_options[] = {
		{"include",     required_argument, 0,           'I'},
		{"pack",        no_argument,       0,           'O'},
		{"fullsegment", no_argument,       0,           'S'},
		{"help",        no_argument,       0,           'h'},
		{"listing",     required_argument, 0,           'l'},
		{"macro",       no_argument,       0,           'm'},
		{"output",      required_argument, 0,           'o'},
		{"segment",     no_argument,       0,           's'},

		{"cd",          no_argument,       &cd_type,     1 },
		{"develo",      no_argument,       &develo_opt,  1 },
		{"ipl",         no_argument,       &ipl_opt,     1 },
		{"kc",          no_argument,       &kickc_opt,   1 },
		{"mx",          no_argument,       &mx_opt,      1 },
		{"overlay",     no_argument,       &overlayflag, 1 },
		{"newproc",     no_argument,       &newproc_opt, 1 },
		{"pad",         no_argument,       &padding_opt, 1 },
		{"raw",         no_argument,       &header_opt,  0 },
		{"scd",         no_argument,       &cd_type,     2 },
		{"sdcc",        no_argument,       &sdcc_opt,    1 },
		{"sf2",         no_argument,       &sf2_opt,     1 },
		{"sgx",         no_argument,       &sgx_opt,     1 },
		{"srec",        no_argument,       &srec_opt,    1 },
		{"strip",       no_argument,       &strip_opt,   1 },
		{"trim",        no_argument,       &trim_opt,    1 },

		{0,		no_argument,       0,		 0 }
	};

	/* register atexit callback */
	atexit(cleanup);

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
	if (
		((prg_name[0] == 'P') || (prg_name[0] == 'p')) &&
		((prg_name[1] == 'C') || (prg_name[1] == 'c')) &&
		((prg_name[2] == 'E') || (prg_name[2] == 'e'))
		) {
		machine = &pce;
	}
	else if (
			((prg_name[0] == 'N') || (prg_name[0] == 'n')) &&
			((prg_name[1] == 'E') || (prg_name[1] == 'e')) &&
			((prg_name[2] == 'S') || (prg_name[2] == 's'))
			) {
		machine = &nes;
	}
	else {
		machine = &fuji;
	}

	/* init assembler options */
	list_level = 2;
	trim_opt = 0;
	header_opt = 1;
	padding_opt = 0;
	overlayflag = 0;
	develo_opt = 0;
	mlist_opt = 0;
	srec_opt = 0;
	run_opt = 0;
	ipl_opt = 0;
	sgx_opt = 0;
	sf2_opt = 0;
	scd_opt = 0;
	cd_opt = 0;
	mx_opt = 0;
	cd_type = 0;
	strip_opt = 0;
	kickc_opt = 0;
	newproc_opt = 0;

	/* display assembler version message */
	printf("%s\n\n", machine->asm_title);

	/* process the command line options */
	while ((opt = getopt_long_only (argc, argv, cmd_line_options, cmd_line_long_options, NULL)) != -1)
	{
		switch(opt)
		{
			case 'I':
				/* optarg can have a leading space on linux/mac */
				while (*optarg == ' ') { ++optarg; }

				if (*optarg == '-') {
					fprintf(stderr, "%s: include path missing after \"-I\"!\n", argv[0]);
					return (1);
				}
				if (!add_path(optarg, (int)strlen(optarg)+1)) {
					fprintf(stderr, "%s: could not add path to list of include directories\n", argv[0]);
					return (1);
				}
				break;

			case 'O':
				asm_opt[OPT_OPTIMIZE] = 1;
				break;

			case 'S':
				dump_seg = 2;
				break;

			case 'h':
				help();
				return (0);

			case 'l':
				/* optarg can have a leading space on linux/mac */
				while (*optarg == ' ') { ++optarg; }

				if ((isdigit(*optarg) == 0) || (optarg[1] != '\0')) {
					fprintf(stderr, "%s: \"-l\" option must be followed by a single digit\n", argv[0]);
					return (1);
				}

				/* get level */
				list_level = *optarg - '0';

				/* check range */
				if (list_level < 0 || list_level > 3)
					list_level = 2;
				break;

			case 'm':
				mlist_opt = 1;
				break;

			case 'o':
				/* optarg can have a leading space on linux/mac */
				while (*optarg == ' ') { ++optarg; }

				if (*optarg == '-') {
					fprintf(stderr, "%s: output filename missing after \"-o\"\n", argv[0]);
					return (1);
				}
				if (strlen(optarg) >= PATHSZ) {
					fprintf(stderr, "%s: output filename too long, maximum %d characters\n", argv[0], PATHSZ - 1);
					return (1);
				}
				strcpy(out_fname, optarg);
				break;

			case 's':
				dump_seg = 1;
				break;

			/* when a long-option has been processed */
			case 0:
				break;

			/* unknown option */
			default:
				help();
				return (1);
		}
	}

	/* no exclusive options with getopt_long_only(), sanitize ipl_opt */
	if (ipl_opt)
	{
		trim_opt = 1;
	}

	/* no exclusive options with getopt_long_only(), sanitize trim_opt */
	if (trim_opt)
	{
		header_opt = 0;
		padding_opt = 0;
		overlayflag = 0;
	}

	/* enable optimized procedure packing if stripping */
	asm_opt[OPT_OPTIMIZE] |= strip_opt;

	if (machine->type == MACHINE_PCE) {
		/* Adjust cdrom type values ... */
		switch(cd_type) {
			case 1:
				/* cdrom */
				cd_opt  = STANDARD_CD;
				scd_opt = 0;
				break;

			case 2:
				/* super cdrom */
				scd_opt = SUPER_CD;
				cd_opt  = 0;
				break;
		}

		if ((overlayflag == 1) &&
		    ((scd_opt == 0) && (cd_opt == 0))) {
			fprintf(ERROUT, "Error: Overlay option only valid for CD or SCD programs.\n\n");
			help();
			return (1);
		}
	} else {
		/* Force ipl_opt off if not PCEAS */
		ipl_opt = 0;
	}

	/* check for input file name missing */
	if (optind == argc)
	{
		fprintf(ERROUT, "Error: Input file name missing!\n");
		return (1);
	}

	/* check for input file name too long (including room to add an extension) */
	if (strlen(argv[optind]) > (PATHSZ - 5)) {
		fprintf(ERROUT, "Error: Input file name too long!\n");
		return (1);
	}

	/* get the input file name */
	strcpy(in_fname, argv[optind++]);

	/* search file extension */
	if ((p = strrchr(in_fname, '.')) != NULL) {
		if (!strchr(p, PATH_SEPARATOR))
			*p = '\0';
		else
			p = NULL;
	}

	/* auto-add file extensions */
	strcpy(bin_fname, in_fname);
	strcpy(lst_fname, in_fname);
	strcpy(sym_fname, in_fname);
	strcat(lst_fname, ".lst");
	strcat(sym_fname, ".sym");

	if (out_fname[0]) {
		strcpy(bin_fname, out_fname);
	} else {
		strcpy(bin_fname, in_fname);
		if (trim_opt == 1)
			strcat(bin_fname, ".bin");
		else if (overlayflag == 1)
			strcat(bin_fname, ".ovl");
		else if (cd_opt || scd_opt)
			strcat(bin_fname, ".iso");
		else if (sgx_opt)
			strcat(bin_fname, ".sgx");
		else
			strcat(bin_fname, machine->rom_ext);
	}

	if (p)
		*p = '.';
	else
		strcat(in_fname, ".asm");

	/* get any additional file names */
	while (optind < argc) {
		t_file *file = malloc(sizeof(t_file));
		if (file == NULL) {
			fprintf(ERROUT, "Error: Not enough memory!\n");
			exit(1);
		}
		file->next = NULL;
		file->name = argv[optind++];

		if (extra_source == NULL)
			extra_source = file;
		if (final_source != NULL)
			final_source->next = file;
		final_source = file;
	}

	/* init include path */
	init_path();

	/* open the input file */
	if (open_input(in_fname)) {
		fprintf(ERROUT, "Error: Cannot open input file \"%s\"!\n", in_fname);
		exit(1);
	}

	/* clear the ROM array */
	memset(rom, 0xFF, MAX_BANKS * 8192);
	memset(map, 0xFF, MAX_BANKS * 8192);

	/* are we creating a custom PCE CDROM IPL? */
	if (ipl_opt) {
		/* initialize the ipl */
		prepare_ipl(&rom[0][2048]);
		memset(&map[0][2048], S_DATA + (1 << 5), 4096);
	}

	/* clear symbol hash tables */
	for (i = 0; i < HASH_COUNT; i++) {
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

	/* init global variables */
	branchlst = NULL;
	max_zp = 0x01;
	max_bss = 0x0201;
	max_bank = 0;
	errcnt = 0;

	rom_limit  = 0x100000;		/* 1MB */
	bank_base  = 0x00;
	bank_limit = 0x7F;

	/* fixme: these should be exclusive! */
	/* process -sf2 first, because overlays are incompatible with the other modes */
	if (sf2_opt) {
		rom_limit  = ROM_BANKS * 8192;	/* 8MB */
		bank_base  = 0x00;
		bank_limit = ROM_BANKS - 1;
		section_flags[S_HOME] |= S_IS_SF2;
		section_flags[S_CODE] |= S_IS_SF2;
		section_flags[S_DATA] |= S_IS_SF2;
	}
	else if (ipl_opt) {
		rom_limit  = 0x01800;	/* 4KB */
		bank_base  = 0x00;
		bank_limit = 0x00;
	}
	else if (cd_opt) {
		rom_limit  = 0x10000;	/* 64KB */
		bank_base  = 0x80;
		bank_limit = 0x07;
	}
	else if (scd_opt) {
		rom_limit  = 0x40000;	/* 256KB */
		bank_base  = 0x68;
		bank_limit = 0x1F;
	}
	else if (develo_opt || mx_opt) {
		rom_limit  = 0x30000;	/* 192KB */
		bank_base  = 0x68;	/* or 0x84, if small */
		bank_limit = 0x17;
	}

	/* predefined symbols */
	lablset("MAGICKIT", 1);
	lablset("DEVELO", develo_opt | mx_opt);
	lablset("CDROM", cd_opt | scd_opt);
	lablset("USING_NEWPROC", newproc_opt);
	lablset("_bss_end", 0);
	lablset("_bank_base", bank_base);
	lablset("_nb_bank", 1);
	lablset("_call_bank", 0);

	/* assemble */
	for (pass = FIRST_PASS; pass <= LAST_PASS; pass++) {
		extra_file = extra_source;
		infile_error = -1;
		page = 7;
		bank = 0;
		loccnt = 0;
		slnum = 0;
		mcounter = 0;
		mcntmax = 0;
		xlist = 0;
		glablptr = NULL;
		scopeptr = NULL;
		branchptr = branchlst;
		branches_changed = 0;
		need_another_pass = 0;
		skip_lines = 0;
		rs_base = 0;
		rs_mprbank = UNDEFINED_BANK;
		rs_overlay = 0;
		proc_ptr = NULL;
		proc_nb = 0;
		kickc_mode = 0;
		sdcc_mode = 0;
		kickc_final = 0;
		hucc_final = 0;
		in_final = 0;

		/* reset assembler options */
		asm_opt[OPT_LIST] = 0;
		asm_opt[OPT_MACRO] = mlist_opt;
		asm_opt[OPT_WARNING] = 0;
		asm_opt[OPT_CCOMMENT] = kickc_opt;
		asm_opt[OPT_INDPAREN] = 0;
		asm_opt[OPT_ZPDETECT] = 0;
		asm_opt[OPT_LBRANCH] = 0;
		asm_opt[OPT_DATAPAGE] = 0;
		asm_opt[OPT_FORWARD] = 1;

		/* reset bank arrays */
		for (i = 0; i < MAX_S; i++) {
			for (j = 0; j < MAX_BANKS; j++) {
				bank_maxloc[j] = 0;
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

		/* reset symbol table and include files */
		if (pass != FIRST_PASS) {
			/* reset symbol definition and reference tracking */
			lablstartpass();

			/* clear the list of included files */
			clear_included();
		}

		/* reset max_bank */
		if (pass != LAST_PASS) {
			max_bank = 0;
		}

		/* pass message */
		printf("pass %i\n", ++pass_count);

		/* assemble */
		while (readline() != -1) {
			int old_bank = bank;
			discontiguous = 0;

			assemble(0);

			if (!discontiguous) {
				/* N.B. $2000 is a legal loccnt that says that the bank is full! */
				if (loccnt > 0x2000) {
					if (proc_ptr) {
						fatal_error(".PROC/.PROGROUP \"%s\" is larger than 8192 bytes!\n", proc_ptr->label->name + 1);
						break;
					}

					loccnt &= 0x1FFF;
					bank = (bank + 1);
					if (bank > max_bank) {
						max_bank = bank;
					}
					if (section != S_DATA || asm_opt[OPT_DATAPAGE] == 0)
						page = (page + 1) & 7;

					if ((section_flags[section] & S_IS_CODE) && page == 0) {
						error(".CODE or .HOME section wrapped from MPR7 to MPR0!");
					}

					if (asm_opt[OPT_WARNING] && pass == LAST_PASS) {
						warning("Opcode crossing page boundary $%04X, bank $%02X", (page * 0x2000), bank);
					}
				}
				while (old_bank != bank) {
					bank_maxloc[old_bank++] = 0x2000;
				}
				if (bank_maxloc[bank] < loccnt) {
					bank_maxloc[bank] = loccnt;
				}
			}

			/* sanity check */
			if (max_bank > bank_limit) {
				fatal_error("Assembler bug ... max_bank (0x%04X) > bank_limit (0x%04X)!\n", max_bank, bank_limit);
			}

			if (stop_pass)
				break;
		}

		/* abort pass on errors during the pass */
		if (errcnt) {
			fprintf(ERROUT, "# %d error(s)\n", errcnt);
			exit(1);
		}

		/* check which procedures have been referenced */
		if (pass == FIRST_PASS) {
//			/* force a 3rd pass for testing */
//			need_another_pass = 1;

			/* only skip stripped procedures if we're going to
			** run a 3rd pass anyway, just hide them if not */
			allow_skipping = need_another_pass;

			/* strip unreferenced procedures */
			proc_strip();
		}

		/* set pass to FIRST_PASS to run EXTRA_PASS next */
		/* or set it to EXTRA_PASS to run LAST_PASS next */
		if (pass != LAST_PASS) {
			/* fix out-of-range short-branches, return number fixed */
			if ((branchopt() != 0) || (need_another_pass != 0))
				pass = FIRST_PASS;
			else
				pass = EXTRA_PASS;
		}

		/* do this just before the last pass */
		if (pass == EXTRA_PASS) {
			/* open the listing file */
			if (lst_fp == NULL && xlist && list_level) {
				if ((lst_fp = fopen(lst_fname, "w")) == NULL) {
					fprintf(ERROUT, "Error: Cannot open listing file \"%s\"!\n", lst_fname);
					exit(1);
				}
				fprintf(lst_fp, "#[1]   \"%s\"\n", input_file[1].file->name);
			}

			/* relocate procs */
			proc_reloc();

			/* update predefined symbols */
			lablset("_bss_end", machine->ram_base + max_bss);
			lablset("_call_bank", bank_base + call_bank);
			if (call_bank > max_bank) {
				lablset("_nb_bank", call_bank + 1);
			} else {
				lablset("_nb_bank", max_bank + 1);
			}
		}

		/* abort pass on errors after the pass */
		if (errcnt) {
			fprintf(ERROUT, "# %d error(s)\n", errcnt);
			exit(1);
		}

		/* rewind input file */
		rewind(in_fp);
	}

	/* close listing file */
	if (lst_fp) {
		if ((list_level >= 2) && (errcnt == 0)) {
			list_procs();
		}
		fclose(lst_fp);
	}

	/* close input file */
	fclose(in_fp);

	/* dump the rom */
	if (errcnt == 0) {
		/* cd-rom */
		if ((cd_opt || scd_opt) && !trim_opt) {
			/* open output file */
			if ((fp = fopen(bin_fname, "wb")) == NULL) {
				fprintf(ERROUT, "Error: Cannot open output file \"%s\"!\n", bin_fname);
				exit(1);
			}

			/* boot code */
			if ((header_opt) && (overlayflag == 0)) {
				/* initialize the ipl */
				prepare_ipl(ipl_buffer);

				/* prg sector base */
				ipl_buffer[0x802] = 2;

				/* nb sectors */
				if (lablexists("HUC")) {
					ipl_buffer[0x803] = 16; /* 4 banks */
				} else {
					ipl_buffer[0x803] = (max_bank + 1) * 8192 / 2048;
				}

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
				ipl_buffer[0x80C] = 0x04;

				/* load mode */
				ipl_buffer[0x80D] = 0x60;

				/* add a SuperGRAFX signature to the IPL Information Block */
				if (sgx_opt)
					memcpy(ipl_buffer + 0x880, "(for SuperGRAFX)", 16);

				/* directory with only 2 files, the IPL and this program */
				ipl_buffer[0xE00] = (0);
				ipl_buffer[0xF00] = (0) >> 8;
				ipl_buffer[0xE01] = (2);
				ipl_buffer[0xF01] = (2) >> 8;
				ipl_buffer[0xE02] = (2 + ((max_bank + 1) * 8192 / 2048));
				ipl_buffer[0xF02] = (2 + ((max_bank + 1) * 8192 / 2048)) >> 8;

				/* store which is the cd error overlay file */
				ipl_buffer[0xDFF] = 0;

				/* store the count of directory entries */
				ipl_buffer[0xE00] = 2;

				/* store which is the first file beyond the 128Mbyte ISO boundary */
				ipl_buffer[0xF00] = 255;

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
				char cmd[PATHSZ+6];
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
				int num_banks = max_bank + 1;

				/* make StreetFighterII roms compatible with emulators which detect size */
				if (sf2_opt) {
					/* Pad a StreetFighterII HuCARD to a minimum of the 2MB */
					if (num_banks < (64 * 4))
						num_banks = (64 * 4);

					/* Also pad it to a multiple of the 512KB mapper size */
					if (num_banks & 63)
						num_banks = num_banks + 64 - (num_banks & 63);
				}

				/* pad rom to power-of-two? */
				if (padding_opt) {
					num_banks = 1;
					while (num_banks <= max_bank) num_banks <<= 1;
				}

				/* open file */
				if ((fp = fopen(bin_fname, "wb")) == NULL) {
					fprintf(ERROUT, "Error: Cannot open binary file \"%s\"!\n", bin_fname);
					exit(1);
				}

				/* write header */
				if (header_opt)
					machine->write_header(fp, num_banks);

				/* write rom */
				if (trim_opt) {
					/* only write from start to end of used data */
					int bank0, index0;
					int bank1, index1;

					/* find first used byte of data */
					for (bank0 = 0; bank0 <= max_bank; ++bank0) {
						for (index0 = 0; index0 < 8192; ++index0) {
							if (map[bank0][index0] != 0xFF) break;
						}
						if (index0 < 8192) break;
					}

					if (bank0 <= max_bank) {
						/* find last used byte of data */
						for (bank1 = max_bank; bank1 >= 0; --bank1) {
							for (index1 = 8191; index1 >= 0; --index1) {
								if (map[bank1][index1] != 0xFF) break;
							}
							if (index1 >= 0) break;
						}

						/* output all of the used bytes between the first and last */
						if (bank0 == bank1) {
							fwrite(&rom[bank0][index0], 1, (index1 + 1) - index0, fp);
						} else {
							fwrite(&rom[bank0][index0], 1, 8192 - index0, fp);
							while (++bank0 != bank1) {
								fwrite(&rom[bank0][0], 1, 8192, fp);
							}
							fwrite(&rom[bank1][0], 1, (index1 + 1), fp);
						}
					}
				} else {
					/* write the whole rom in one go */
					fwrite(rom, 8192, num_banks, fp);
				}

				fclose(fp);
			}
		}
	}

	/* dump the symbol table */
	if ((fp = fopen(sym_fname, "w")) != NULL) {
		labldump(fp);
		fclose(fp);
	}

	/* dump the bank table */
	if (dump_seg)
		show_seg_usage(stdout);

	/* check for corrupted thunks */
	if (check_thunks()) {
		exit(1);
	}

	/* warn about 384KB hucard rom size */
	if (cd_opt == 0 && scd_opt == 0 && padding_opt == 0) {
		if (max_bank == 0x2F) {
			warning("A 384KB ROM size may not work with emulators!\n\n"
				"Most emulators expect a 384KB HuCard ROM to be in a split-image layout.\n\n"
				"Unless you are patching an existing 384KB HuCard game image, you are\n"
				"almost-certainly not using that layout, and your ROM will crash.\n\n"
				"To avoid problems, add or remove enough data to avoid the 384KB size.\n\n");
		}
	}

	/* ok */
	return (0);
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
	printf("%s [options] [-h (for help)] [-o outfile] infiles\n\n", prg_name);
	printf("-s         : show segment usage\n");
	printf("-S         : show segment usage and contents\n");
	printf("-l <0..3>  : listing file output level (0-3), default is 2\n");
	printf("-m         : force macro expansion in listing\n");
	printf("--raw      : prevent adding a ROM header\n");
	printf("--pad      : pad ROM size to power-of-two\n");
	printf("--trim     : strip unused head and tail from ROM\n");
	printf("-I         : add include path\n");
	if (machine->type == MACHINE_PCE) {
		printf("--sf2      : create a StreetFighterII HuCARD\n");
		printf("--cd       : create a CD-ROM track image\n");
		printf("--scd      : create a Super CD-ROM track image\n");
		printf("--sgx      : add a SuperGRAFX signature to the CD-ROM\n");
		printf("--overlay  : create an executable 'overlay' program segment\n");
		printf("--ipl      : create a custom CD-ROM IPL file\n");
		printf("--develo   : assemble and run on the Develo Box\n");
		printf("--mx       : create a Develo MX file\n");
		printf("-O         : optimize .proc packing (compared to HuC v3.21)\n");
		printf("--strip    : strip unused .proc & .procgroup\n");
		printf("--newproc  : run .proc code in MPR6, instead of MPR5\n");
	}
	printf("--kc       : assemble code generated by the KickC compiler\n");
	printf("--srec     : create a Motorola S-record file\n");
	printf("infiles    : one or more files to be assembled\n");
	printf("\n");
}


/* ----
 * show_bank_usage()
 * ----
 */

void
show_bank_usage(FILE *fp, int which_bank)
{
	int addr, start, nb;

	/* scan bank */
	nb = 0;

	/* count used and free bytes */
	for (addr = 0; addr < 8192; addr++)
		if (map[which_bank][addr] != 0xFF)
			nb++;

	/* update used/free counters */
	rom_used += nb;
	rom_free += 8192 - nb;

	/* display bank infos */
	if (nb)
		fprintf(fp, "BANK $%-3X  %-23s    %4i / %4i\n",
		       which_bank, bank_name[which_bank], nb, 8192 - nb);
	else {
		fprintf(fp, "BANK $%-3X  %-23s       0 / 8192\n", which_bank, bank_name[which_bank]);
		return;
	}

	/* scan */
	if (dump_seg == 1)
		return;

	for (addr = 0;;) {
		/* search section start */
		for (; addr < 8192; addr++)
			if (map[which_bank][addr] != 0xFF)
				break;

		/* check for end of bank */
		if (addr > 8191)
			break;

		/* get section type */
		section = map[which_bank][addr] & 0x0F;
		page = (map[which_bank][addr] & 0xE0) << 8;
		start = addr;

		/* search section end */
		for (; addr < 8192; addr++)
			if (map[which_bank][addr] != map[which_bank][start])
				break;

		/* display section infos */
		fprintf(fp, "    %5s    $%04X-$%04X  [%4i]\n",
			section_name[section],		/* section name */
			start + page,			/* starting address */
			addr + page - 1,		/* end address */
			addr - start);			/* size */
	}
}


/* ----
 * show_seg_usage()
 * ----
 */

void
show_seg_usage(FILE *fp)
{
	int i;
	int start, stop;
	int ram_base = machine->ram_base;

	fprintf(fp, "\nSegment Usage:\n");

	fprintf(fp, "%37c USED / FREE\n", ' ');

	/* zp usage */
	if (max_zp <= 1)
		fprintf(fp, "       ZP    -\n");
	else {
		start = ram_base;
		stop = ram_base + (max_zp - 1);
		fprintf(fp, "       ZP    $%04X-$%04X  [%4i]      %4i / %4i\n", start, stop, stop - start + 1, stop - start + 1, 256 - (stop - start + 1));
	}

	/* bss usage */
	if (max_bss <= 0x201)
		fprintf(fp, "      BSS    -\n\n");
	else {
		start = ram_base + 0x200;
		stop = ram_base + (max_bss - 1);
		fprintf(fp, "      BSS    $%04X-$%04X  [%4i]      %4i / %4i\n\n", start, stop, stop - start + 1, stop - start + 1, 8192 - (stop - start + 1));
	}

	/* bank usage */
	rom_used = 0;
	rom_free = 0;

	/* scan banks */
	for (i = 0; i <= max_bank; i++) {
		show_bank_usage(fp, i);
	}

	/* total */
	rom_used = (rom_used + 1023) >> 10;
	rom_free = (rom_free) >> 10;
	fprintf(fp, "%36c -----  -----\n", ' ');
	fprintf(fp, "%36c %4iK  %4iK\n", ' ', rom_used, rom_free);
	fprintf(fp, "\n%24c TOTAL SIZE =       %4iK\n\n", ' ', (rom_used + rom_free));
}
