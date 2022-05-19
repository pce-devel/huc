/*	File data.c: 2.2 (84/11/27,16:26:13) */
/*% cc -O -c %
 *
 */

#include <stdint.h>
#include <stdio.h>
#include "defs.h"

/* constant arrays storage */

struct const_array *const_ptr;
struct const_array const_var[MAX_CONST];
intptr_t const_val[MAX_CONST_VALUE];
char const_data[MAX_CONST_DATA];
intptr_t const_val_start;
intptr_t const_val_idx;
intptr_t const_data_start;
intptr_t const_data_idx;
intptr_t const_size;
intptr_t const_nb;

/* storage words */

SYMBOL symtab[SYMTBSZ];
SYMBOL *glbptr, *rglbptr, *locptr;
intptr_t ws[WSTABSZ];
intptr_t *wsptr;
intptr_t swstcase[SWSTSZ];
intptr_t swstlab[SWSTSZ];
intptr_t swstp;
char litq[LITABSZ];
char litq2[LITABSZ];
intptr_t litptr;
struct macro macq[MACQSIZE];
intptr_t macptr;
char line[LINESIZE];
char mline[LINESIZE];
intptr_t lptr, mptr;

TAG_SYMBOL tag_table[NUMTAG];	// start of structure tag table
int tag_table_index;		// ptr to next entry

SYMBOL member_table[NUMMEMB];	// structure member table
int member_table_index;		// ptr to next member

char asmdefs[LITABSZ];

/* miscellaneous storage */

intptr_t nxtlab,
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

intptr_t top_level_stkp;

FILE *input, *input2, *output;
FILE *inclstk[INCLSIZ];

char inclstk_name[INCLSIZ][FILENAMESIZE];
char fname_copy[FILENAMESIZE];
char user_outfile[FILENAMESIZE];
intptr_t inclstk_line[INCLSIZ];
intptr_t line_number;

intptr_t inclsp;
char fname[FILENAMESIZE];

char quote[2];
char *cptr;
intptr_t *iptr;
intptr_t fexitlab;
intptr_t iflevel, skiplevel;
intptr_t errfile;
intptr_t sflag;
intptr_t cdflag;
intptr_t verboseflag;
intptr_t startup_incl;
intptr_t errs;

int norecurse = 0;
intptr_t locals_ptr;

struct type *typedefs;
int typedef_ptr = 0;

struct clabel *clabels;
int clabel_ptr = 0;

struct enum_s *enums;
int enum_ptr = 0;
struct enum_type *enum_types;
int enum_type_ptr = 0;

int user_short_enums = 1;
int user_signed_char = 0;

intptr_t output_globdef;
int have_irq_handler;
int have_sirq_handler;

int need_map_call_bank;

char **leaf_functions = 0;
int leaf_cnt = 0;
int leaf_size = 0;

INITIALS initials_table[NUMGLBS];
char initials_data_table[INITIALS_SIZE];	// 5kB space for initialisation data
int initials_idx = 0, initials_data_idx = 0;
