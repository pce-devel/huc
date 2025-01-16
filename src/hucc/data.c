/*	File data.c: 2.2 (84/11/27,16:26:13) */
/*% cc -O -c %
 *
 */

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "defs.h"

/* constant arrays storage */

struct const_array *const_ptr;
struct const_array const_var[MAX_CONST];
int const_val[MAX_CONST_VALUE];
char const_data[MAX_CONST_DATA];
int const_val_start;
int const_val_idx;
int const_data_start;
int const_data_idx;
int const_size;
int const_nb;

/* storage words */

SYMBOL symtab[SYMTBSZ];
int glbsym_index, rglbsym_index, locsym_index;

int ws[WS_TOTAL];
int *wsptr;

int swstcase[SWST_COUNT];
int swstlabel[SWST_COUNT];
int swstp;
char litq[LITABSZ];
char litq2[LITABSZ];
int litptr;
struct macro macq[MACQSIZE];
int macptr;
char line[LINESIZE];
char mline[LINESIZE];
int lptr, mptr;

TAG_SYMBOL tag_table[NUMTAG];	// start of structure tag table
int tag_table_index;		// ptr to next entry

SYMBOL member_table[NUMMEMB];	// structure member table
int member_table_index;		// ptr to next member

char asmdefs[LITABSZ];

/* miscellaneous storage */

int nxtlab,
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

int top_level_stkp;

FILE *input, *input2, *output;
FILE *inclstk[INCLSIZ];

char inclstk_name[INCLSIZ][FILENAMESIZE];
char fname_copy[FILENAMESIZE];
char user_outfile[FILENAMESIZE];
int inclstk_line[INCLSIZ];
int line_number;

int inclsp;
char fname[FILENAMESIZE];

char quote[2];
SYMBOL *cptr;
int *iptr;
int fexitlab;
int iflevel, skiplevel;
int errfile;
int debug;
int sflag;
int cdflag;
int acflag;
int sgflag;
int ted2flag;
int verboseflag;
int startup_incl;
int errs;
char debug_info;

int norecurse = 0;
#if ULI_NORECURSE
int local_offset;
#endif

struct type_type *typedefs;
int typedef_ptr = 0;

struct clabel *clabels;
int clabel_ptr = 0;

struct enum_s *enums;
int enum_ptr = 0;
struct enum_type *enum_types;
int enum_type_ptr = 0;

int user_short_enums = 1;
int user_signed_char = 0;

int output_globdef;

char **leaf_functions = 0;
int leaf_cnt = 0;
int leaf_size = 0;

INITIALS initials_table[NUMGLBS];
char initials_data_table[INITIALS_SIZE];	// 5kB space for initialisation data
int initials_idx = 0, initials_data_idx = 0;
