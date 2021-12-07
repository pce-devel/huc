#ifndef DEFS_H
#define DEFS_H

#include "version.h"

#define NES_ASM_VERSION ("NES Assembler (v 3.25-" GIT_VERSION " Beta, " GIT_DATE ")")
#define PCE_ASM_VERSION ("PC Engine Assembler (v 3.25-" GIT_VERSION ", " GIT_DATE ")")
#define FUJI_ASM_VERSION ("Fuji Assembler for Atari (v 3.25-" GIT_VERSION " Beta, " GIT_DATE ")")

/* path separator */
#if defined(WIN32)
#define PATH_SEPARATOR		'\\'
#define PATH_SEPARATOR_STRING	"\\"
#else
#define PATH_SEPARATOR		'/'
#define PATH_SEPARATOR_STRING	"/"
#endif

/* machine */
#define MACHINE_PCE	0
#define MACHINE_NES	1
#define MACHINE_FUJI	2

/* reserved bank index */
#define RESERVED_BANK	0xF0
#define PROC_BANK	0xF1
#define GROUP_BANK	0xF2
#define STRIPPED_BANK	0xF3

/* tile format for encoder */
#define CHUNKY_TILE	1
#define PACKED_TILE	2

/* line buffer length */
#define LAST_CH_POS	158
#define SFIELD		29
#define SBOLSZ		64

/* macro argument types */
#define NO_ARG		0
#define ARG_REG		1
#define ARG_IMM		2
#define ARG_ABS		3
#define ARG_INDIRECT	4
#define ARG_STRING	5
#define ARG_LABEL	6

/* section types */
#define S_ZP	0
#define S_BSS	1
#define S_CODE	2
#define S_DATA	3
#define S_PROC	4	/* trampolines for .proc */

/* assembler options */
#define OPT_LIST	0
#define OPT_MACRO	1
#define OPT_WARNING	2
#define OPT_OPTIMIZE	3

/* assembler directives */
#define P_DB		0	// .db
#define P_DW		1	// .dw
#define P_DD		2	// .dd
#define P_DS		3	// .ds
#define P_EQU		4	// .equ
#define P_ORG		5	// .org
#define P_PAGE		6	// .page
#define P_BANK		7	// .bank
#define P_INCBIN	8	// .incbin
#define P_INCLUDE	9	// .include
#define P_INCCHR	10	// .incchr
#define P_INCSPR	11	// .incspr
#define P_INCPAL	12	// .incpal
#define P_INCBAT	13	// .incbat
#define P_MACRO		14	// .macro
#define P_ENDM		15	// .endm
#define P_LIST		16	// .list
#define P_MLIST		17	// .mlist
#define P_NOLIST	18	// .nolist
#define P_NOMLIST	19	// .nomlist
#define P_RSSET		20	// .rsset
#define P_RS		21	// .rs
#define P_IF		22	// .if
#define P_ELSE		23	// .else
#define P_ENDIF		24	// .endif
#define P_FAIL		25	// .fail
#define P_ZP		26	// .zp
#define P_BSS		27	// .bss
#define P_CODE		28	// .code
#define P_DATA		29	// .data
#define P_DEFCHR	30	// .defchr
#define P_FUNC		31	// .func
#define P_IFDEF		32	// .ifdef
#define P_IFNDEF	33	// .ifndef
#define P_VRAM		34	// .vram
#define P_PAL		35	// .pal
#define P_DEFPAL	36	// .defpal
#define P_DEFSPR	37	// .defspr
#define P_INESPRG	38	// .inesprg
#define P_INESCHR	39	// .ineschr
#define P_INESMAP	40	// .inesmap
#define P_INESMIR	41	// .inesmir
#define P_OPT		42	// .opt
#define P_INCTILE	43	// .inctile
#define P_INCMAP	44	// .incmap
#define P_MML		45	// .mml
#define P_PROC		46	// .proc
#define P_ENDP		47	// .endp
#define P_PGROUP	48	// .procgroup
#define P_ENDPG		49	// .endprocgroup
#define P_CALL		50	// .call
#define P_DWL		51	// lsb of a WORD
#define P_DWH		52	// lsb of a WORD
#define P_INCCHRPAL	53	// .incchrpal
#define P_INCSPRPAL	54	// .incsprpal
#define P_INCTILEPAL	55	// .inctilepal
#define P_CARTRIDGE	56	// .cartridge
#define P_ALIGN		57	// .align
#define P_KICKC		58	// .kickc
#define P_CPU		59	// .cpu
#define P_SEGMENT	60	// .segment
#define P_LABEL		61	// .label or .const
#define P_ENCODING	62	// .encoding
#define P_SCOPE		63	// '{'
#define P_ENDS		64	// '}'

/* symbol flags */
#define UNDEF	1	/* undefined - may be zero page */
#define IFUNDEF 2	/* declared in a .if expression */
#define MDEF	3	/* multiply defined */
#define DEFABS	4	/* defined - two byte address */
#define MACRO	5	/* used for a macro name */
#define FUNC	6	/* used for a function */

/* operation code flags */
#define PSEUDO		0x0008000
#define CLASS1		0x0010000
#define CLASS2		0x0020000
#define CLASS3		0x0040000
#define CLASS5		0x0080000
#define CLASS6		0x0100000
#define CLASS7		0x0200000
#define CLASS8		0x0400000
#define CLASS9		0x0800000
#define CLASS10		0x1000000
#define ACC		0x0000001
#define IMM		0x0000002
#define ZP		0x0000004
#define ZP_X		0x0000008
#define ZP_Y		0x0000010
#define ZP_IND		0x0000020
#define ZP_IND_X	0x0000040
#define ZP_IND_Y	0x0000080
#define ABS		0x0000100
#define ABS_X		0x0000200
#define ABS_Y		0x0000400
#define ABS_IND		0x0000800
#define ABS_IND_X	0x0001000

/* pass flags */
#define FIRST_PASS	0
#define LAST_PASS	1

/* structs */
typedef struct t_opcode {
	struct t_opcode *next;
	char *name;
	void (*proc)(int *);
	int flag;
	int value;
	int type_idx;
} t_opcode;

typedef struct t_input_info {
	FILE *fp;
	int lnum;
	int if_level;
	char name[116];
} t_input_info;

typedef struct t_proc {
	struct t_proc *next;
	struct t_proc *link;
	struct t_proc *group;
	struct t_symbol *label;
	int old_bank;
	int bank;
	int org;
	int base;
	int size;
	int call;
	int type;
	int refcnt;
	char name[SBOLSZ];
} t_proc;

typedef struct t_symbol {
	struct t_symbol *next;
	struct t_symbol *local;
	struct t_symbol *scope;
	struct t_proc *proc;
	int type;
	int value;
	int bank;
	int page;
	int nb;
	int size;
	int vram;
	int pal;
	int defcnt;
	int refcnt;
	int reserved;
	int data_type;
	int data_size;
	char name[SBOLSZ];
} t_symbol;

typedef struct t_line {
	struct t_line *next;
	char *data;
} t_line;

typedef struct t_macro {
	struct t_macro *next;
	struct t_line *line;
	char name[SBOLSZ];
} t_macro;

typedef struct t_func {
	struct t_func *next;
	char line[128];
	char name[SBOLSZ];
} t_func;

typedef struct t_tile {
	struct t_tile *next;
	unsigned char *data;
	unsigned int crc;
	int index;
} t_tile;

typedef struct t_machine {
	int type;
	char *asm_name;
	char *asm_title;
	char *rom_ext;
	char *include_env;
	const char *default_dir;
	unsigned int zp_limit;
	unsigned int ram_limit;
	unsigned int ram_base;
	unsigned int ram_page;
	unsigned int ram_bank;
	struct t_opcode *base_inst;
	struct t_opcode *plus_inst;
	struct t_opcode *pseudo_inst;
	int (*pack_8x8_tile)(unsigned char *, void *, int, int);
	int (*pack_16x16_tile)(unsigned char *, void *, int, int);
	int (*pack_16x16_sprite)(unsigned char *, void *, int, int);
	void (*write_header)(FILE *, int);
} MACHINE;

#endif // DEFS_H
