/*	File defs.h: 2.1 (83/03/21,02:07:20) */

#ifndef INCLUDE_DEFS_H
#define INCLUDE_DEFS_H

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
	I_PLACEHOLDER,
	I_LDB,
	I_LDBP,
	I_LDW,
	I_LDWI,
	I_LDWP,
	I_STB,
	I_STBPS,
	I_STW,
	I_STWPS,
	I_ADDWI,
	I_ADDWS,
	I_MODSP,
	I_SUBWI,
	I_SUBWS,
	I_ORWS,
	I_EORWS,
	I_ANDWS,
	I_ASLW,
	I_ASLWS,
	I_ASRW,
	I_COMW,
	I_NEGW,
	I_EXTW,
	I_NOTW,
	I_TSTW,
	I_BFALSE,
	I_BTRUE,
	I_JMP,
	I_JSR,
	I_ENTER,
	I_LEAVE,
	I_CALL,
	I_MACRO,
	I_CALLP,
	I_PUSHW,
	I_POPW,
	I_BRA,
	I_BANK,
	I_OFFSET,
	I_FARPTR,
	I_FARPTR_I,
	I_FARPTR_GET,
	I_FGETB,
	I_FGETW,
	I_INCW,
	I_ANDWI,
	I_ORWI,
	I_ADDW,
	I_SUBW,
	I_LDUB,
	I_LDUBP,
	I_FGETUB,
	I_ADDBS,
	I_ADDBI,
	I_LABEL,
	I_STWIP,
	I_STBIP,
	I_MULWI,
	I_STWI,
	I_STBI,
	I_ASLWI,
	I_LSRWI,
	I_ASRWI,
	I_ADDB,
	I_ADDUB,
	I_ADDBI_P,
	I_INCB,
	I_STWZ,
	I_STBZ,
	I_CMPWI_EQ,
	I_CMPWI_NE,
	I_STBP,
	I_STWP,
	I_EORWI,
	I_SPUSHW,	/* push and pop on the hw-stack */
	I_SPUSHB,	/* to temporarily save data */
	I_SPOPW,
	I_SPOPB,
	I_EXTUW,
	I_DEF,
	I_SEI,
	I_CLI,
	I_SAVESP,
	I_LOADSP,
	I_GETACC,
	I_CMPW,
	I_CMPB,
	X_LDB,
	X_LDB_S,
	X_LDW_S,
	X_LDD_I,
	X_LDD_B,
	X_LDD_W,
	X_LDD_S_B,
	X_LDD_S_W,
	X_LEA_S,
	X_PEA,
	X_PEA_S,
	X_STB_S,
	X_STW_S,
	X_STBI_S,
	X_STWI_S,
	X_ADDW_S,
	X_INCW_S,
	X_DECW_S,
	X_LDUB_S,	/* Uli added these ... */
	X_ADDB_S,
	X_ADDUB_S,
	X_INCB_S
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
