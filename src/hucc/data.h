/*	File data.h: 2.2 (84/11/27,16:26:11) */

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#include "defs.h"

/* constant arrays storage */

extern struct const_array *const_ptr;
extern struct const_array const_var[MAX_CONST];
extern int const_val[MAX_CONST_VALUE];
extern char const_data[MAX_CONST_DATA];
extern int const_val_start;
extern int const_val_idx;
extern int const_data_start;
extern int const_data_idx;
extern int const_size;
extern int const_nb;

/* storage words */

extern SYMBOL symtab[];
extern int glbsym_index, rglbsym_index, locsym_index;
extern int ws[];
extern int *wsptr;
extern int swstcase[];
extern int swstlabel[];
extern int swstp;
extern char litq[];
extern char litq2[];
extern int litptr;
extern struct macro macq[];
extern int macptr;
extern char line[];
extern char mline[];
extern int lptr, mptr;

extern TAG_SYMBOL tag_table[NUMTAG];	// start of structure tag table
extern int tag_table_index;		// ptr to next entry

extern SYMBOL member_table[NUMMEMB];	// structure member table
extern int member_table_index;		// ptr to next member<

extern char asmdefs[];

/* miscellaneous storage */

extern int nxtlab,
	    litlab,
	    stkp,
	    argstk,
	    ncmp,
	    errcnt,
	    glbflag,
	    indflg,
	    ctext,
	    cmode,
	    last_statement,
	    overlayflag,
	    optimize,
	    globals_h_in_process;

extern FILE *input, *input2, *output;
extern FILE *inclstk[];

extern char inclstk_name[INCLSIZ][FILENAMESIZE];
extern int inclstk_line[];
extern char fname_copy[FILENAMESIZE];
extern char user_outfile[FILENAMESIZE];
extern int line_number;

extern int inclsp;
extern char fname[];

extern char quote[];
extern SYMBOL *cptr;
extern int *iptr;
extern int fexitlab;
extern int iflevel, skiplevel;
extern int errfile;
extern int debug;
extern int sflag;
extern int cdflag;
extern int acflag;
extern int sgflag;
extern int ted2flag;
extern int verboseflag;
extern int startup_incl;
extern int errs;
extern char debug_info;

extern int top_level_stkp;
extern int norecurse;
#if ULI_NORECURSE
extern int local_offset;
#endif
extern char current_fn[];

extern struct type_type *typedefs;
extern int typedef_ptr;

extern struct clabel *clabels;
extern int clabel_ptr;

extern struct enum_s *enums;
extern int enum_ptr;
extern struct enum_type *enum_types;
extern int enum_type_ptr;

extern int user_short_enums;
extern int user_signed_char;

extern int output_globdef;

extern char **leaf_functions;
extern int leaf_cnt;
extern int leaf_size;

extern INITIALS initials_table[NUMGLBS];
extern char initials_data_table[INITIALS_SIZE];		// 5kB space for initialisation data
extern int initials_idx, initials_data_idx;
