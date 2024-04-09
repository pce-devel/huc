#ifndef DEFS_H
#define DEFS_H

#include "version.h"

#define NES_ASM_VERSION ("NES Assembler (" GIT_VERSION ", " GIT_DATE ")")
#define PCE_ASM_VERSION ("PC Engine Assembler (" GIT_VERSION ", " GIT_DATE ")")
#define FUJI_ASM_VERSION ("Fuji Assembler for Atari (" GIT_VERSION ", " GIT_DATE ")")

/* path separator */
#if defined(_WIN32)
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

/* maximum user rom size */
#define ROM_BANKS	1024

/* reserved bank index */
#define UNDEFINED_BANK	(ROM_BANKS + 0)
#define PROC_BANK	(ROM_BANKS + 1)
#define GROUP_BANK	(ROM_BANKS + 2)
#define STRIPPED_BANK	(ROM_BANKS + 3)

/* number of reserved banks used beyond ROM_BANKS */
#define RESERVED_BANKS	4

/* total number of banks to allocate for workspace */
#define MAX_BANKS	(ROM_BANKS + RESERVED_BANKS)

/* tile format for encoder */
#define CHUNKY_TILE	1
#define PACKED_TILE	2

/* max nesting of include files */
#define MAX_NESTING	31

/* line buffer length */
#define LAST_CH_POS	(32768 - 4)
#define SFIELD		30

/* symbol name size, including length byte and '\0' */
/* must be <= 129 if "char" is signed */
#define SBOLSZ		128

/* file name size, including the '\0' */
#define PATHSZ		260

/* macro argument types */
#define NO_ARG		0
#define ARG_REG		1
#define ARG_IMM		2
#define ARG_ABS		3
#define ARG_INDIRECT	4
#define ARG_STRING	5
#define ARG_LABEL	6

/* section types */
/* update pseudo_allowed when adding or changing! */
/* update section_name  when adding or changing! */
/* update section_flags when adding or changing! */
/* update section_limit when adding or changing! */
#define S_NONE		0 /* SDCC: sections that should always be empty! */
#define S_ZP		1
#define S_BSS		2
#define S_CODE		3
#define S_DATA		4
#define S_HOME		5 /* SDCC: permanently-mapped code */
#define S_XDATA		6 /* SDCC: BSS copy of initialized variables */
#define S_XINIT		7 /* SDCC: ROM copy of initialized variables */
#define S_CONST		8 /* SDCC: permanent const data */
#define S_OSEG		9 /* SDCC: overlayed variables in ZP */
#define MAX_S		10 /* selectable section types */
#define S_PROC		10 /* info only, trampolines for .proc */

/* section flag mask */
#define S_IS_RAM	1
#define S_IS_ROM	2
#define S_IS_SF2	4
#define S_IS_CODE	8
#define S_NO_DATA       16

/* assembler options */
#define OPT_LIST	0
#define OPT_MACRO	1
#define OPT_WARNING	2
#define OPT_OPTIMIZE	3
#define OPT_CCOMMENT	4
#define OPT_INDPAREN	5
#define OPT_ZPDETECT	6
#define OPT_LBRANCH	7
#define OPT_DATAPAGE	8
#define OPT_FORWARD	9
#define MAX_OPTS	10

/* assembler directives */
/* update pseudo_allowed when adding or changing! */
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
#define P_KICKC		58	// .kickc .r6502 .r65c02
#define P_IGNORE	59	// .cpu .optsdcc .globl
#define P_SEGMENT	60	// .segment .area
#define P_LABEL		61	// .label .const
#define P_ENCODING	62	// .encoding
#define P_STRUCT	63	// .struct
#define P_ENDS		64	// .ends
#define P_3PASS		65	// .3pass

/* symbol flags */
#define UNDEF	1	/* undefined - may be zero page */
#define IFUNDEF 2	/* declared in a .if expression */
#define MDEF	3	/* multiply defined */
#define DEFABS	4	/* defined - two byte address */
#define MACRO	5	/* used for a macro name */
#define FUNC	6	/* used for a function */

/* symbol lookup flags */
#define SYM_CHK	0	/* does it exist? */
#define SYM_DEF 1	/* symbol definition */
#define SYM_REF	2	/* symbol reference */

/* symbol definition source */
#define LOCATION 0	/* location (current PC) */
#define CONSTANT 1	/* constant value */
#define VARIABLE 2	/* variable value */

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

/* pass type flags */
#define FIRST_PASS	0
#define EXTRA_PASS	1
#define LAST_PASS	2

/* size of various hashing tables */
#define HASH_COUNT	256

/* size of remembered filename strings */
#define FILE_NAMES_SIZE 65536

/* structs */
typedef struct t_opcode {
	struct t_opcode *next;
	char *name;
	void (*proc)(int *);
	int flag;
	int value;
	int type_idx;
} t_opcode;

typedef struct t_file_names {
	struct t_file_names *next;
	int remain;
	char buffer [FILE_NAMES_SIZE];
} t_file_names;

typedef struct t_file {
	struct t_file *next;
	int number;
	int included;
	char *name;
} t_file;

typedef struct t_input {
	struct t_file *file;
	FILE *fp;
	int lnum;
	int if_level;
} t_input;

typedef struct t_proc {
	struct t_proc *next;
	struct t_proc *link;
	struct t_proc *group;
	struct t_symbol *label;
	struct t_symbol *old_glablptr;
	int old_bank;
	int old_page;
	int old_loccnt;
	int bank;
	int org;
	int base;
	int size;
	int call;
	int type;
	int kickc;
	int defined;
	int is_skippable;
} t_proc;

/* update pc_symbol when adding or changing! */
typedef struct t_symbol {
	struct t_symbol *next;
	struct t_symbol *local;
	struct t_symbol *scope;
	struct t_proc *proc;
	int reason;
	int type;
	int value;
	int section;
	int overlay;
	int mprbank;
	int rombank;
	int page;
	int nb;
	int size;
	int vram;
	int pal;
	int reserved;
	int data_type;
	int data_size;
	int deflastpass;
	int reflastpass;
	int defthispass;
	int refthispass;
	char name[SBOLSZ];
} t_symbol;

typedef struct t_branch {
	struct t_branch *next;
	struct t_symbol *label;
	int  addr;
	char checked;
	char convert;
} t_branch;

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
} t_machine;

#endif // DEFS_H
