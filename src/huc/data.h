/*	File data.h: 2.2 (84/11/27,16:26:11) */

#include <stdint.h>
#include <stdio.h>

#include "defs.h"

/* constant arrays storage */

extern struct const_array *const_ptr;
extern struct const_array const_var[MAX_CONST];
extern intptr_t const_val[MAX_CONST_VALUE];
extern char const_data[MAX_CONST_DATA];
extern intptr_t const_val_start;
extern intptr_t const_val_idx;
extern intptr_t const_data_start;
extern intptr_t const_data_idx;
extern intptr_t const_size;
extern intptr_t const_nb;

/* storage words */

extern SYMBOL symtab[];
extern SYMBOL *glbptr, *rglbptr, *locptr;
extern intptr_t ws[];
extern intptr_t *wsptr;
extern intptr_t swstcase[];
extern intptr_t swstlab[];
extern intptr_t swstp;
extern char litq[];
extern char litq2[];
extern intptr_t litptr;
extern struct macro macq[];
extern intptr_t macptr;
extern char line[];
extern char mline[];
extern intptr_t lptr, mptr;

extern TAG_SYMBOL tag_table[NUMTAG];	// start of structure tag table
extern int tag_table_index;		// ptr to next entry

extern SYMBOL member_table[NUMMEMB];	// structure member table
extern int member_table_index;		// ptr to next member<

extern char asmdefs[];

/* miscellaneous storage */

extern intptr_t nxtlab,
	    litlab,
	    stkp,
	    zpstkp,
	    argstk,
	    ncmp,
	    errcnt,
	    glbflag,
	    indflg,
	    ctext,
	    cmode,
	    lastst,
	    overlayflag,
	    optimize,
	    globals_h_in_process;

extern FILE *input, *input2, *output;
extern FILE *inclstk[];

extern char inclstk_name[INCLSIZ][FILENAMESIZE];
extern intptr_t inclstk_line[];
extern char fname_copy[FILENAMESIZE];
extern char user_outfile[FILENAMESIZE];
extern intptr_t line_number;

extern intptr_t inclsp;
extern char fname[];

extern char quote[];
extern SYMBOL *cptr;
extern intptr_t *iptr;
extern intptr_t fexitlab;
extern intptr_t iflevel, skiplevel;
extern intptr_t errfile;
extern intptr_t sflag;
extern intptr_t cdflag;
extern intptr_t verboseflag;
extern intptr_t startup_incl;
extern intptr_t errs;

extern intptr_t top_level_stkp;
extern int norecurse;
extern intptr_t locals_ptr;
extern char current_fn[];

extern struct type *typedefs;
extern int typedef_ptr;

extern struct clabel *clabels;
extern int clabel_ptr;

extern struct enum_s *enums;
extern int enum_ptr;
extern struct enum_type *enum_types;
extern int enum_type_ptr;

extern int user_short_enums;
extern int user_signed_char;

extern intptr_t output_globdef;
extern int have_irq_handler;
extern int have_sirq_handler;

extern int need_map_call_bank;

extern char **leaf_functions;
extern int leaf_cnt;
extern int leaf_size;

extern INITIALS initials_table[NUMGLBS];
extern char initials_data_table[INITIALS_SIZE];		// 5kB space for initialisation data
extern int initials_idx, initials_data_idx;
