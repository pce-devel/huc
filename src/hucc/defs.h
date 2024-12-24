/*	File defs.h: 2.1 (83/03/21,02:07:20) */

#ifndef INCLUDE_DEFS_H
#define INCLUDE_DEFS_H

#define ULI_NORECURSE 1

/*
 * i-code pseudo instructions
 *
 * N.B. this i-code enum list MUST be kept updated and in the same order
 * as the table of i-code flag information in optimize.c
 */
enum ICODE {
	/* i-code to mark an instruction as retired */

	I_RETIRED = 0,

	/* i-code for internal compiler information */

	I_INFO,

	/* i-code that retires the primary register contents */

	I_FENCE,

	/* i-code that declares a byte sized primary register */

	I_SHORT,

	/* i-codes for handling farptr */

	I_FARPTR,
	I_FARPTR_I,
	I_FARPTR_GET,
	I_FGETW,
	I_FGETB,
	I_FGETUB,

	/* i-codes for interrupts */

	I_SEI,
	I_CLI,

	/* i-codes for calling functions */

	I_MACRO,
	I_CALL,
	I_FUNCP_WR,
	I_CALLP,

	/* i-codes for C functions and the C parameter stack */

	I_ENTER,
	I_RETURN,
	I_MODSP,
	I_PUSHARG_WR,
	I_PUSH_WR,
	I_POP_WR,
	I_SPUSH_WR,	/* push and pop on the hw-stack */
	I_SPUSH_UR,	/* to temporarily save data */
	I_SPOP_WR,
	I_SPOP_UR,

	/* i-codes for handling boolean tests and branching */

	I_SWITCH_WR,
	I_SWITCH_UR,
	I_DEFAULT,
	I_CASE,
	I_ENDCASE,
	I_LABEL,
	I_ALIAS,
	I_BRA,
	I_BFALSE,
	I_BTRUE,
	I_DEF,

	I_CMP_WT,
	X_CMP_WI,
	X_CMP_WM,
	X_CMP_UM,
	X_CMP_WS,
	X_CMP_US,

	X_CMP_UIQ,
	X_CMP_UMQ,
	X_CMP_USQ,

	I_NOT_WR,
	X_NOT_WP,
	X_NOT_WM,
	X_NOT_WS,
	X_NOT_WAR,
	X_NOT_UP,
	X_NOT_UM,
	X_NOT_US,
	X_NOT_UAR,
	X_NOT_UAY,

	I_TST_WR,
	X_TST_WP,
	X_TST_WM,
	X_TST_WS,
	X_TST_WAR,
	X_TST_UP,
	X_TST_UM,
	X_TST_US,
	X_TST_UAR,
	X_TST_UAY,

	X_NAND_WI,
	X_TAND_WI,

	X_NOT_CF,

	I_BOOLEAN,
	X_BOOLNOT_WR,
	X_BOOLNOT_WP,
	X_BOOLNOT_WM,
	X_BOOLNOT_WS,
	X_BOOLNOT_WAR,
	X_BOOLNOT_UP,
	X_BOOLNOT_UM,
	X_BOOLNOT_US,
	X_BOOLNOT_UAR,
	X_BOOLNOT_UAY,

	/* i-codes for loading the primary register */

	I_LD_WI,
	X_LD_UIQ,
	I_LEA_S,

	I_LD_WM,
	I_LD_BM,
	I_LD_UM,

	I_LD_WMQ,
	I_LD_BMQ,
	I_LD_UMQ,

	I_LDY_WMQ,
	I_LDY_BMQ,
	I_LDY_UMQ,

	I_LD_WP,
	I_LD_BP,
	I_LD_UP,

	X_LD_WAR,
	X_LD_BAR,
	X_LD_UAR,

	X_LD_BAY,
	X_LD_UAY,

	X_LD_WS,
	X_LD_BS,
	X_LD_US,

	X_LD_WSQ,
	X_LD_BSQ,
	X_LD_USQ,

	X_LDY_WSQ,
	X_LDY_BSQ,
	X_LDY_USQ,

	X_LDP_WAR,
	X_LDP_BAR,
	X_LDP_UAR,

	X_LDP_BAY,
	X_LDP_UAY,

	/* i-codes for pre- and post- increment and decrement */

	X_INCLD_WM,
	X_INCLD_BM,
	X_INCLD_UM,

	X_DECLD_WM,
	X_DECLD_BM,
	X_DECLD_UM,

	X_LDINC_WM,
	X_LDINC_BM,
	X_LDINC_UM,

	X_LDDEC_WM,
	X_LDDEC_BM,
	X_LDDEC_UM,

	X_INC_WMQ,
	X_INC_UMQ,

	X_DEC_WMQ,
	X_DEC_UMQ,

	X_INCLD_WS,
	X_INCLD_BS,
	X_INCLD_US,

	X_DECLD_WS,
	X_DECLD_BS,
	X_DECLD_US,

	X_LDINC_WS,
	X_LDINC_BS,
	X_LDINC_US,

	X_LDDEC_WS,
	X_LDDEC_BS,
	X_LDDEC_US,

	X_INC_WSQ,
	X_INC_USQ,

	X_DEC_WSQ,
	X_DEC_USQ,

	X_INCLD_WAR,
	X_LDINC_WAR,
	X_DECLD_WAR,
	X_LDDEC_WAR,

	X_INCLD_BAR,
	X_INCLD_UAR,
	X_LDINC_BAR,
	X_LDINC_UAR,
	X_DECLD_BAR,
	X_DECLD_UAR,
	X_LDDEC_BAR,
	X_LDDEC_UAR,

	X_INCLD_BAY,
	X_INCLD_UAY,
	X_LDINC_BAY,
	X_LDINC_UAY,
	X_DECLD_BAY,
	X_DECLD_UAY,
	X_LDDEC_BAY,
	X_LDDEC_UAY,

	X_INC_WARQ,
	X_INC_UARQ,
	X_INC_UAYQ,

	X_DEC_WARQ,
	X_DEC_UARQ,
	X_DEC_UAYQ,

	/* i-codes for saving the primary register */

	I_ST_WMIQ,
	I_ST_UMIQ,
	I_ST_WPI,
	I_ST_UPI,
	I_ST_WM,
	I_ST_UM,
	I_ST_WP,
	I_ST_UP,
	I_ST_WPT,
	I_ST_UPT,
	X_ST_WSIQ,
	X_ST_USIQ,
	X_ST_WS,
	X_ST_US,

	X_INDEX_WR,
	X_INDEX_UR,

	X_ST_WAT,
	X_ST_UAT,

	/* i-codes for extending the primary register */

	I_EXT_BR,
	I_EXT_UR,

	/* i-codes for math with the primary register  */

	I_COM_WR,
	I_NEG_WR,

	I_ADD_WT,
	I_ADD_WI,
	I_ADD_WM,
	I_ADD_UM,
	X_ADD_WS,
	X_ADD_US,

	I_SUB_WT,
	I_SUB_WI,
	I_SUB_WM,
	I_SUB_UM,
	X_SUB_WS,
	X_SUB_US,

	X_ISUB_WT,
	X_ISUB_WI,
	X_ISUB_WM,
	X_ISUB_UM,
	X_ISUB_WS,
	X_ISUB_US,

	I_AND_WT,
	I_AND_WI,
	I_AND_UIQ,
	I_AND_WM,
	I_AND_UM,
	X_AND_WS,
	X_AND_US,

	I_EOR_WT,
	I_EOR_WI,
	I_EOR_WM,
	I_EOR_UM,
	X_EOR_WS,
	X_EOR_US,

	I_OR_WT,
	I_OR_WI,
	I_OR_WM,
	I_OR_UM,
	X_OR_WS,
	X_OR_US,

	I_ASL_WT,
	I_ASL_WI,
	I_ASL_WR,

	I_ASR_WT,
	I_ASR_WI,

	I_LSR_WT,
	I_LSR_WI,
	I_LSR_UIQ,

	I_MUL_WT,
	I_MUL_WI,

	I_SDIV_WT,
	I_SDIV_WI,

	I_UDIV_WT,
	I_UDIV_WI,
	I_UDIV_UI,

	I_SMOD_WT,
	I_SMOD_WI,

	I_UMOD_WT,
	I_UMOD_WI,
	I_UMOD_UI,

	I_DOUBLE_WT,

	/* i-codes for 32-bit longs */

	X_LDD_I,
	X_LDD_W,
	X_LDD_B,
	X_LDD_S_W,
	X_LDD_S_B
};

/*
 * boolean comparison operations
 */
enum ICOMPARE {
	CMP_EQU,
	CMP_NEQ,
	CMP_SLT,
	CMP_SLE,
	CMP_SGT,
	CMP_SGE,
	CMP_ULT,
	CMP_ULE,
	CMP_UGT,
	CMP_UGE
};

/*
 *	INTSIZE is the size of an integer in the target machine
 *	BYTEOFF is the offset of an byte within an integer on the
 *		target machine. (ie: 8080,pdp11 = 0, 6809 = 1,
 *		360 = 3)
 *	This compiler assumes that an integer is the SAME length as
 *	a pointer - in fact, the compiler uses INTSIZE for both.
 */

#define INTSIZE 2
#define BYTEOFF 0

/* pseudo instruction arg types */
#define T_NOP           -1
#define T_VALUE          1
#define T_LABEL          2
#define T_SYMBOL         3
#define T_PTR            4
#define T_STACK          5
#define T_STRING         6
#define T_SIZE           7
#define T_BANK           8
#define T_VRAM           9
#define T_PAL           10
#define T_LITERAL       11
/* pseudo instruction arg types for compiler I_INFO */
#define T_SOURCE_LINE   12
#define T_CLEAR_LINE    13
#define T_MARKER        14

#define FOREVER for (;;)
#define FALSE   0
#define TRUE    1
#define NO      0
#define YES     1

/* miscellaneous */

#define EOS     0
#define EOL     10
#define BKSP    8
#define CR      13
#define FFEED   12
#define TAB     9

#ifdef _WIN32
#define FILENAMESIZE 260
#else
#define FILENAMESIZE 256
#endif

/* symbol table parameters (locals are reset for every function) */
#define SYMTBSZ 4096
#define NUMGLBS (SYMTBSZ - 512)

#define STARTGLB 0
#define ENDGLB   (NUMGLBS - 1)
#define STARTLOC (NUMGLBS)
#define ENDLOC   (SYMTBSZ - 1)

/* symbol table entry format */
/* N.B. nasty hack to allow space beyond NAMEMAX (see "copysym") */

#define NAMESIZE	48
#define NAMEMAX		47
#define NAMEALLOC	64

typedef struct symbol {
	char name[NAMEALLOC];	/* symbol name */
	struct symbol *linked;	/* HuC: linked local and global symbols */
	int alloc_size;
	char identity;		/* variable, array, pointer, function */
	char sym_type;		/* char, int, uchar, unit */
	char storage;		/* public, auto, extern, static, lstatic, defauto*/
	char far;		/* HuC: 1 if array of data in far memory */
	char ptr_order;		/* HuC: 1 if array of data in far memory */
	char funcptr_type;	/* HuC: return type if function pointer */
	char funcptr_order;	/* HuC: return order if function pointer */
	char arg_count;		/* HuC: #arguments for function or function pointer */
	short offset;		/* offset*/
	short tagidx;		/* index of struct in tag table*/
} SYMBOL;

/* Define the structure tag table parameters */

#define NUMTAG  64

struct tag_symbol {
	char name[NAMESIZE];	/* structure tag name */
	int size;		/* size of struct in bytes */
	int member_idx;		/* index of first member */
	int number_of_members;	/* number of tag members */
};
#define TAG_SYMBOL struct tag_symbol

#define NULL_TAG 0

/* Define the structure member table parameters */

#define NUMMEMB         256

/* possible entries for "ident" */

#define VARIABLE 1
#define ARRAY    2
#define POINTER  3
#define FUNCTION 4

/* possible entries for "type" for sym_type, val_type, type_type, init_type */

#define CCHAR   1
#define CINT    2
#define CVOID   3
#define CSTRUCT 4
#define CENUM   5
#define CSIGNED 0
#define CUNSIGNED 8
#define CUINT (CINT | CUNSIGNED)
#define CUCHAR (CCHAR | CUNSIGNED)

/* possible entries for storage */

#define PUBLIC  1
#define AUTO    2
#define EXTERN  3
#define STATIC  4
#define LSTATIC 5
#define DEFAUTO 6
#define CONST   7

#define STORAGE 15 /* bitmask for the storage type */

#define WASAUTO 64
#define WRITTEN 128

/* "do"/"for"/"while"/"switch" statement stack */

#define WS_COUNT 7 /* number of ints per "while" entry */
#define WS_TOTAL (20 * WS_COUNT)
#define WS_LIMIT (ws + WS_TOTAL - WS_COUNT)

/* entry offsets in "do"/"for"/"while"/"switch" stack */

#define WS_LOCSYM_INDEX   0	/* locsym_index to restore at the end of the compound statement */
#define WS_STACK_OFFSET   1	/* stack offset to restore at the end of the compound statement */
#define WS_TYPE           2
#define WS_CASE_INDEX     3	/* switch label stack index of first "case" */
#define WS_TEST_LABEL     3
#define WS_INCR_LABEL     4
#define WS_DEFAULT_LABEL  4
#define WS_BODY_LABEL     5
#define WS_SWITCH_LABEL   5
#define WS_EXIT_LABEL     6

/* possible entries for "WS_TYPE" */

#define WS_WHILE  0
#define WS_FOR    1
#define WS_DO     2
#define WS_SWITCH 3

/* "switch" label stack size */

#define SWST_COUNT  256

/* literal pool */

#define LITABSZ 8192
#define LITMAX  LITABSZ - 1

/* input string */

#define LITMAX2 LITABSZ - 1

/* input line */

#define LINESIZE        512
#define LINEMAX (LINESIZE - 1)
#define MPMAX   LINEMAX

/* macro (define) pool */

#define MACQSIZE        16384
#define MACMAX  (MACQSIZE - 1)

struct macro {
	char *name;
	char **args;
	int argc;
	struct {
		int arg;
		int pos;
	} *argpos;
	char *def;
};

/* "include" stack */

#define INCLSIZ 3

/* statement types (tokens) */

#define STIF    1
#define STWHILE 2
#define STRETURN        3
#define STBREAK 4
#define STCONT  5
#define STASM   6
#define STEXP   7
#define STDO    8
#define STFOR   9
#define STSWITCH        10
#define STGOTO  11

/* pseudo instruction structure */

typedef struct {
	unsigned sequence;
	enum ICODE ins_code;
	enum ICOMPARE cmp_type;
	int ins_type;
	intptr_t ins_data;
	int imm_type;
	intptr_t imm_data;
	const char *arg[3];
	SYMBOL *sym;
} INS;

/* constant array struct */

#define MAX_CONST        1024
#define MAX_CONST_VALUE  8192
#define MAX_CONST_DATA  65536

struct const_array {
	SYMBOL *sym;
	int typ;
	int size;
	int data;
};

/* fastcall func struct */

#define MAX_FASTCALL_ARGS 16
#define FASTCALL_EXTRA    0x01  // bitmask values
#define FASTCALL_NOP      0x02  // bitmask values
#define FASTCALL_MACRO    0x04  // bitmask values

struct fastcall {
	struct fastcall *next;
	char fname[NAMESIZE];
	int nargs;
	int flags;
	char argtype[MAX_FASTCALL_ARGS];
	char argname[MAX_FASTCALL_ARGS][NAMESIZE];
};

// initialisation of global variables

#define INIT_TYPE    NAMESIZE
#define INIT_LENGTH  NAMESIZE + 1
#define INITIALS_SIZE 5 * 1024

struct initials_table {
	char name[NAMESIZE];	/* symbol name */
	int init_type;		/* type */
	int dim;		/* length of data (possibly an array) */
	int data_len;		/* index of tag or zero */
};
#define INITIALS struct initials_table

SYMBOL *find_member (TAG_SYMBOL *tag, char *sname);

struct lvalue {
	SYMBOL *symbol;		/* symbol table address, or 0 for constant */
	int indirect;		/* type of object pointed to by primary register, 0 for static object */
	int ptr_type;		/* type of pointer or array, 0 for other idents */
	TAG_SYMBOL *tagsym;	/* tag symbol address, 0 if not struct */
	SYMBOL *symbol2;	/* HuC: */
	int ptr_order;		/* HuC: allows for more than 1 level of indirection */
	int val_type;		/* HuC: type of current value, if known */
};
#define LVALUE struct lvalue

#define W_GENERAL 1

/* typedef struct */

struct type_type {
	int type_type;		/* char, int, uchar, unit */
	int type_ident;		/* variable, array, pointer */
	int ptr_order;
	int otag;
	int flags;
	char sname[NAMESIZE];	/* type name */
};

#define F_REGISTER 1
#define F_CONST 2
#define F_VOLATILE 4
#define F_STRUCT 8	/* set if CSTRUCT is struct, not union */

struct clabel {
	char name[NAMESIZE];
	int stkp;
	int label;
};

struct enum_s {
	char name[NAMESIZE];
	int enum_value;
};

/* enum struct */

struct enum_type {
	char name[NAMESIZE];
	int start;
	int base;
};

#endif
