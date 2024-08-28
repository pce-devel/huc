/*	File defs.h: 2.1 (83/03/21,02:07:20) */

#ifndef INCLUDE_DEFS_H
#define INCLUDE_DEFS_H

#define ULI_NORECURSE 1

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
#define T_LIB            7
#define T_SIZE           8
#define T_BANK           9
#define T_VRAM          10
#define T_PAL           11
#define T_LITERAL       12

/* basic pseudo instructions */
enum ICODE {
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
	I_CALLP,
	I_JSR,

	/* i-codes for C functions and the C parameter stack */

	I_ENTER,
	I_LEAVE,
	I_GETACC,
	I_SAVESP,
	I_LOADSP,
	I_MODSP,
	I_PUSHW,
	I_POPW,
	I_SPUSHW,	/* push and pop on the hw-stack */
	I_SPUSHB,	/* to temporarily save data */
	I_SPOPW,
	I_SPOPB,

	/* i-codes for handling boolean tests and branching */

	I_SWITCHW,
	I_SWITCHB,
	I_CASE,
	I_ENDCASE,
	I_LABEL,
	I_BRA,
	I_DEF,
	I_CMPW,
	I_CMPB,
	X_CMPWI_EQ,
	X_CMPWI_NE,
	I_NOTW,
	I_TSTW,
	I_BFALSE,
	I_BTRUE,
	X_TZB,
	X_TZBP,
	X_TZB_S,
	X_TZW,
	X_TZWP,
	X_TZW_S,
	X_TNZB,
	X_TNZBP,
	X_TNZB_S,
	X_TNZW,
	X_TNZWP,
	X_TNZW_S,

	/* i-codes for loading the primary register */

	I_LDWI,
	I_LDW,
	I_LDB,
	I_LDUB,
	I_LDWP,
	I_LDBP,
	I_LDUBP,

	/* i-codes for saving the primary register */

	I_STWZ,
	I_STBZ,
	I_STWI,
	I_STBI,
	I_STWIP,
	I_STBIP,
	I_STW,
	I_STB,
	I_STWP,
	I_STBP,
	I_STWPS,
	I_STBPS,

	/* i-codes for extending the primary register */

	I_EXTW,
	I_EXTUW,

	/* i-codes for math with the primary register  */

	I_COMW,
	I_NEGW,

	I_INCW,
	I_INCB,

	I_ADDWI,
	I_ADDBI,
	I_ADDWS,
	I_ADDBS,
	I_ADDW,
	I_ADDB,
	I_ADDUB,

	I_ADDBI_P,

	I_SUBWI,
	I_SUBWS,
	I_SUBW,

	I_ANDWI,
	I_ANDWS,

	I_EORWI,
	I_EORWS,

	I_ORWI,
	I_ORWS,

	I_ASLWI,
	I_ASLWS,
	I_ASLW,

	I_ASRWI,
	I_ASRW,

	I_LSRWI,

	I_MULWI,

	/* optimized i-codes for local variables on the C stack */

	X_LEA_S,
	X_PEA_S,

	X_LDW_S,
	X_LDB_S,
	X_LDUB_S,

	X_STWI_S,
	X_STBI_S,
	X_STW_S,
	X_STB_S,

	X_ADDW_S,
	X_ADDB_S,
	X_ADDUB_S,

	X_INCW_S,
	X_INCB_S,

	/* i-codes for 32-bit longs */

	X_LDD_I,
	X_LDD_W,
	X_LDD_B,
	X_LDD_S_W,
	X_LDD_S_B
};

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

#define FILENAMESIZE    256

/* symbol table parameters */

/* old values, too restrictive
 * #define	SYMSIZ	14
 * #define	SYMTBSZ	32768
 * #define	NUMGLBS	1500
 */
#define SYMTBSZ 4096
#define NUMGLBS 2048

#define STARTGLB        symtab
#define ENDGLB  (STARTGLB + NUMGLBS)
#define STARTLOC        (ENDGLB + 1)
#define ENDLOC  (symtab + SYMTBSZ - 1)

/* symbol table entry format */
/* N.B. nasty hack to allow space beyond NAMEMAX (see "copysym") */

#define NAMESIZE	48
#define NAMEMAX		47
#define NAMEALLOC	64

struct symbol {
	char name[NAMEALLOC];
	char ident;
	char type;
	char storage;
	char far;
	short offset;
	short tagidx;
	int size;
	int ptr_order;
};

typedef struct symbol SYMBOL;

#define NUMTAG  64

struct tag_symbol {
	char name[NAMESIZE];	// structure tag name
	int size;		// size of struct in bytes
	int member_idx;		// index of first member
	int number_of_members;	// number of tag members
};
#define TAG_SYMBOL struct tag_symbol

#define NULL_TAG 0

// Define the structure member table parameters
#define NUMMEMB         256

/* possible entries for "ident" */

#define VARIABLE        1
#define ARRAY   2
#define POINTER 3
#define FUNCTION        4

/* possible entries for "type" */

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

#define WSSIZ   7
#define WSTABSZ (WSSIZ * 16)
#define WSMAX   ws + WSTABSZ - WSSIZ

/* entry offsets in "do"/"for"/"while"/"switch" stack */

#define WSSYM   0
#define WSSP    1
#define WSTYP   2
#define WSCASEP 3
#define WSTEST  3
#define WSINCR  4
#define WSDEF   4
#define WSBODY  5
#define WSTAB   5
#define WSEXIT  6

/* possible entries for "wstyp" */

#define WSWHILE 0
#define WSFOR   1
#define WSDO    2
#define WSSWITCH        3

/* "switch" label stack */

#define SWSTSZ  256

/* literal pool */

#define LITABSZ 8192
#define LITMAX  LITABSZ - 1

/* input string */

#define LITMAX2 LITABSZ - 1

/* input line */

#define LINESIZE        384
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
	int code;
	int type;
	intptr_t data;
	int imm_type;
	intptr_t imm_data;
	char *arg[3];
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
#define FASTCALL_XSAFE    0x02  // bitmask values
#define FASTCALL_NOP      0x04  // bitmask values
#define FASTCALL_MACRO    0x08  // bitmask values

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
	char name[NAMESIZE];	// symbol name
	int type;		// type
	int dim;		// length of data (possibly an array)
	int data_len;		// index of tag or zero
};
#define INITIALS struct initials_table

SYMBOL *find_member (TAG_SYMBOL *tag, char *sname);

struct lvalue {
	SYMBOL *symbol;
	int indirect;
	int ptr_type;
	SYMBOL *symbol2;
	int value;
	TAG_SYMBOL *tagsym;
	int ptr_order;
	int type;
};
#define LVALUE struct lvalue

#define W_GENERAL 1

struct type {
	int type;
	int ident;
	int ptr_order;
	int otag;
	int flags;
	char sname[NAMESIZE];
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
	int value;
};
struct enum_type {
	char name[NAMESIZE];
	int start;
	int base;
};

#endif
