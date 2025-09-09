/*	File main.c: 2.7 (84/11/28,10:14:56)
 *
 * PC Engine C Compiler
 * based on Small-C Compiler by Ron Cain, James E. Hendrix, and others
 * hacked to work on PC Engine by David Michel
 * work resumed by Zeograd
 * work resumed again by Ulrich Hecht
 * work resumed again by John Brandwood
 *
 * 00/02/22 : added oldargv variable to show real exe name in usage function
 */
/*% cc -O -c %
 *
 */

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>
#include <errno.h>
#include "defs.h"
#include "data.h"
#include "code.h"
#include "const.h"
#include "enum.h"
#include "error.h"
#include "function.h"
#include "gen.h"
#include "initials.h"
#include "io.h"
#include "lex.h"
#include "main.h"
#include "optimize.h"
#include "pragma.h"
#include "preproc.h"
#include "primary.h"
#include "pseudo.h"
#include "sym.h"
#include "struct.h"

static char **link_libs = 0;
static int link_lib_ptr;
static char **infiles = 0;
static int infile_ptr;

static int user_norecurse = 0;

#if 0
#if !((__STDC_VERSION__ >= 201112L) || (_MSC_VER >= 1910))
#   if !defined(HAVE_STRCAT_S) 
static int strcat_s(char* dst, size_t len, const char* src) {
	size_t i;
	if (!dst || !len) {
		return EINVAL;
	}
	if (src) {
		for (i = 0; i < len; i++) {
			if (dst[i] == '\0') {
				size_t j;
				for (j = 0; (i+j) < len; j++) {
					if ((dst[i+j] = src[j]) == '\0') {
						return 0;
					}
				}
			}
		}
	}
	dst[0] = '\0';
	return EINVAL;
}
#   endif // !HAVE_STRCAT_S

#   if !defined(HAVE_STRCPY_S)
static int strcpy_s(char* dst, size_t len, const char* src) {
	if (!dst || !len) {
		return EINVAL;
	}
	if (src) {
		size_t i;
		for (i = 0; i < len; i++) {
			if ((dst[i] = src[i]) == '\0') {
				return 0;
			}
		}
	}
	dst[0] = '\0';
	return EINVAL;
}
#   endif // !HAVE_STRCPY_S
#endif 
#endif

static char *lib_to_file (char *lib)
{
	int i;
	static char libfile[FILENAMESIZE];
	char **incd = include_dirs();

	for (i = 0; incd[i]; i++) {
		struct stat st;
		sprintf(libfile, "%s/%s.c", incd[i], lib);
		if (!stat(libfile, &st))
			return (libfile);
	}
	return (0);
}
static void dumpfinal (void);

int main (int argc, char *argv[])
{
	char *p, *pp, *bp;
	char **oldargv = argv;
	char **link_lib;
	int smacptr;
	int first = 1;
	char *asmdefs_global_end;

	macptr = 0;
	ctext = 1;
	argc--; argv++;
	errs = 0;
	sflag = 0;
	cdflag = 0;
	verboseflag = 0;
	startup_incl = 0;
	optimize = 2;	/* -O2 by default */
	overlayflag = 0;
	asmdefs[0] = '\0';

	while ((p = *argv++)) {
		if (*p == '-') {
			/* Allow GNU style "--" long options */
			if (p[1] == '-') ++p;
			while (*++p) switch (*p) {
				/* defines to pass to assembler */
				case 'a':
					if (strncmp(p, "acd", 3) == 0) {
						cdflag = 2;	/* pass '--scd' to assembler */
						acflag = 1;
						strcat(asmdefs, "_ACD\t\t=\t1\n");
						p += 2;
						break;
					}
				/* fallthrough */
				case 'A':
					bp = ++p;
					if (!*p) usage(oldargv[0]);
					while (*p && *p != '=') p++;
					strncat(asmdefs, bp, (p - bp));
/*					if (*p == '=') *p = '\t'; */
					bp = ++p;
					strcat(asmdefs, "\t= ");
					if (*bp == '\0')
						strcat(asmdefs, "1\n");
					else {
						strcat(asmdefs, bp);
						strcat(asmdefs, "\n");
					}
					break;

				case 'c':
					if (*(p + 1) == 'd') {
						cdflag = 1;	/* pass '-cd' to assembler */
						p++;
						break;
					}
					else {
						usage(oldargv[0]);
						break;
					}

				case 'd':
				case 'D':
					bp = ++p;
					if (!*p) usage(oldargv[0]);
					while (*p && *p != '=') p++;
					if (*p == '=') *p = '\t';
					while (*p) p++;
					p--;
					defmac(bp);
					break;

				case 'f':
					p++;
					if (!strcmp(p, "no-recursive")) {
						user_norecurse = 1;
						p += 11;
					}
					else if (!strcmp(p, "recursive")) {
						user_norecurse = 0;
						p += 8;
					}
					else if (!strcmp(p, "no-short-enums")) {
						user_short_enums = 0;
						p += 13;
					}
					else if (!strcmp(p, "short-enums")) {
						user_short_enums = 1;
						p += 10;
					}
					else if (!strcmp(p, "signed-char")) {
						user_signed_char = 1;
						p += 10;
					}
					else if (!strcmp(p, "unsigned-char")) {
						user_signed_char = 0;
						p += 12;
					}
					else if (!strcmp(p, "no-far-arrays")) {
						user_far_arrays = 0;
						p += 12;
					}
					else if (!strcmp(p, "far-arrays")) {
						user_far_arrays = 1;
						p += 9;
					}
					else
						goto unknown_option;
					break;

				case 'g':
//					debug = 1;
//					strcat(asmdefs, "_DEBUG\t\t=\t1\n");
					debug_info = toupper(*(p + 1));
					if (debug_info != 'C' && debug_info != 'A' && debug_info != 'L')
						goto unknown_option;
					p++;
					if (debug_info == 'L')
						verboseflag = 1;
					break;

				case 'l':
					bp = ++p;
					while (*p && *p != ' ' && *p != '\t')
						p++;
					link_libs = realloc(link_libs, (link_lib_ptr + 2) * sizeof(*link_libs));
					link_libs[link_lib_ptr] = malloc(p - bp + 1);
					memcpy(link_libs[link_lib_ptr], bp, p - bp);
					link_libs[link_lib_ptr][p - bp] = 0;
					strcat(asmdefs, "LINK_");
					strcat(asmdefs, link_libs[link_lib_ptr]);
					strcat(asmdefs, "\t= 1\n");
					link_libs[++link_lib_ptr] = 0;
					p--;
					break;

				case 'm':
					if (!strcmp(p + 1, "small")) {
						/* accept this but ignore it */
						p += 5;
					}
					else {
unknown_option:
						fprintf(stderr, "unknown option %s\n", p);
						exit(1);
					}
					break;

				case 'o':
					if (strncmp(p, "over", 4) == 0) {
						overlayflag = 1;
						if (strncmp(p, "overlay", 7) == 0)
							p += 6;
						else
							p += 3;
					}
					else {
						bp = ++p;
						while (*p && *p != ' ' && *p != '\t')
							p++;
						memcpy(user_outfile, bp, p - bp);
						user_outfile[p - bp] = 0;
						p--;
					}
					break;
				case 'O':
					/* David, made -O equal to -O2
					 * I'm too lazy to tape -O2 each time :)
					 */
					if (!p[1]) optimize = 2;
					else optimize = atoi(++p);
					break;

				case 's':
					if (strncmp(p, "scd", 3) == 0) {
						cdflag = 2;	/* pass '-scd' to assembler */
						p += 2;
						break;
					}
					else if (strncmp(p, "sgx", 3) == 0) {
						sgflag = 1;
						strcat(asmdefs, "_SGX\t\t=\t1\n");
						p += 2;
						break;
					}
				/* fallthrough */
				case 'S':
					sflag = 1;
					break;

				case 't':
					if (strncmp(p, "ted2", 4) == 0) {
						ted2flag = 1;
						strcat(asmdefs, "_TED2\t\t=\t1\n");
						p += 3;
						break;
					}
				/* fallthrough */
				case 'T':
					/* accept this but ignore it */
//					ctext = 1;
					break;

				case 'v':
					verboseflag = 1;
					break;

				default:
					usage(oldargv[0]);
				}
		}
		else {
			infiles = realloc(infiles, (infile_ptr + 2) * sizeof(*infiles));
			infiles[infile_ptr++] = p;
			infiles[infile_ptr] = 0;
		}
	}

	smacptr = macptr;
	if (!infiles)
		usage(oldargv[0]);
	printf(HUC_VERSION);
	printf("\n");
	if (!init_path())
		exit(1);

	/* Remember the first file, it will be used as the base for the
	   output file name unless there is a user-specified outfile. */
	p = pp = infiles[0];
	/* Labels count is not reset for each file because labels are
	   global and conflicts would arise. */
	nxtlab = 1;
	link_lib = link_libs;
	infile_ptr = 1;
	/* Remember where the global assembler defines end so we can
	   reset to that point for each file. */
	/* XXX: Even if we don't repeat the local asm defines, they
	   are still defined because we compile everything into one
	   assembly file. */
	asmdefs_global_end = asmdefs + strlen(asmdefs);
	while (p) {
		errfile = 0;
		/* Truncate asm defines to the point where global
		   defines end. */
		asmdefs_global_end[0] = 0;
		if (extension(p) == 'c' || extension(p) == 'C') {
			glbsym_index = STARTGLB;
			locsym_index = STARTLOC;
			wsptr = ws;
			inclsp =
			iflevel =
			skiplevel =
			swstp =
			litptr =
			stkp =
			errcnt =
			ncmp =
			last_statement =
			quote[1] =
			const_nb =
			line_number = 0;
			macptr = smacptr;
			input2 = NULL;
			quote[0] = '"';
			cmode = 1;
			glbflag = 1;
			litlab = getlabel();
			member_table_index = 0;
			memset(member_table, 0, sizeof(member_table));
			tag_table_index = 0;
			norecurse = user_norecurse;
			typedef_ptr = 0;
			enum_ptr = 0;
			enum_type_ptr = 0;
			memset(fastcall_tbl, 0, sizeof(fastcall_tbl));
			defpragma();

			/* Macros and globals have to be reset for each
			   file, so we have to define the defaults all over
			   each time. */
			defmac("__end\t__memory");
			addglb("__memory", ARRAY, CCHAR, 0, EXTERN, 0);
			addglb("stack", ARRAY, CCHAR, 0, EXTERN, 0);
			rglbsym_index = glbsym_index;
			addglb("etext", ARRAY, CCHAR, 0, EXTERN, 0);
			addglb("edata", ARRAY, CCHAR, 0, EXTERN, 0);
			/* PCE specific externs */
			addglb("font_base", VARIABLE, CINT, 0, EXTERN, 0);
			/* end specific externs */

			/* deprecated ident macros */
			defmac("huc6280\t1");
			defmac("huc\t1");
			/* modern ident macros */
			defmac("__HUC__\t1");
			defmac("__HUCC__\t1");

			if (debug)
				defmac("_DEBUG\t1");

			if (cdflag == 1)
				defmac("_CD\t1");
			else if (cdflag == 2)
				defmac("_SCD\t1");
			else
				defmac("_ROM\t1");

			if (overlayflag == 1)
				defmac("_OVERLAY\t1");

			if (acflag)
				defmac("_ACD\t1");
			if (sgflag)
				defmac("_SGX\t1");
			if (ted2flag)
				defmac("_TED2\t1");

//			initmac();
			/*
			 *	compiler body
			 */
			if (!openin(p))
				exit(1);
			if (first && !openout())
				exit(1);
			if (first)
				header();
			asmdefines();
//			gtext ();
			parse();
			fclose(input);
			ol(".dbg\tclear");
//			gdata ();
			dumplits();
			dumpglbs();
			errorsummary();
//			trailer ();
			pl("");
			errs = errs || errfile;
		}
		else {
			fputs("Don't understand file ", stderr);
			fputs(p, stderr);
			fputc('\n', stderr);
			exit(1);
		}
		p = infiles[infile_ptr];
		if (!p && link_lib && *link_lib) {
			/* No more command-line files, continue with
			   libraries. */
			p = lib_to_file(*link_lib);
			if (!p) {
				fprintf(stderr, "cannot find library %s\n", *link_lib);
				exit(1);
			}
			link_lib++;
		}
		else
			infile_ptr++;
		first = 0;
	}
	dumpfinal();
	fclose(output);
	if (!errs && !sflag) {
		if (user_outfile[0])
			errs = errs || assemble(user_outfile);
		else
			errs = errs || assemble(pp);
	}
	exit(errs != 0);
}

void FEvers (void)
{
	outstr("\tFront End (2.7,84/11/28)");
}

void usage (char *exename)
{
	fprintf(stderr, HUC_VERSION);
	fprintf(stderr, "\n\n");
	fprintf(stderr, "USAGE: %s [-options] infile\n", exename);
	fprintf(stderr, "\nCompiler options:\n");
	fprintf(stderr, "-Dsym[=val]       Define symbol 'sym' when compiling\n");
	fprintf(stderr, "-O[val]           Invoke optimization (level <value>)\n");
	fprintf(stderr, "-fno-far-arrays   Disable deprecated __far array syntax\n");
	fprintf(stderr, "-fno-recursive    Optimize assuming non-recursive code\n");
	fprintf(stderr, "-fno-short-enums  Always use signed int for enums\n");
	fprintf(stderr, "-funsigned-char   Make \"char\" unsigned (the default)\n");
	fprintf(stderr, "-fsigned-char     Make \"char\" signed\n");
	fprintf(stderr, "\nOutput options:\n");
	fprintf(stderr, "-s/-S             Create asm output only (do not invoke assembler)\n");
	fprintf(stderr, "\nLinker options:\n");
	fprintf(stderr, "-lname            Add library 'name.c' from include path\n");
	fprintf(stderr, "--cd              Create CD-ROM output\n");
	fprintf(stderr, "--scd             Create Super CD-ROM output\n");
	fprintf(stderr, "--acd             Create Arcade Card CD output\n");
	fprintf(stderr, "--sgx             Enable SuperGrafx support\n");
	fprintf(stderr, "--ted2            Enable Turbo Everdrive2 support\n");
	fprintf(stderr, "--over(lay)       Create CD-ROM overlay section\n");
	fprintf(stderr, "\nAssembler options:\n");
	fprintf(stderr, "-Asym[=val]       Define symbol 'sym' to assembler\n");
	fprintf(stderr, "\nDebugging options:\n");
	fprintf(stderr, "-gC               Output .SYM for mesen2 C source debugging\n");
	fprintf(stderr, "-gA               Output .SYM for mesen2 ASM source debugging\n");
	fprintf(stderr, "-gL               Output .SYM for mesen2 LST file debugging (the default)\n");
//	fprintf(stderr, "-g                Enable extra debugging checks in output code\n");
	fprintf(stderr, "-v/-V             Increase verbosity of output files\n\n");
	exit(1);
}

/*
 *	process all input text
 *
 *	at this level, only static declarations, defines, includes,
 *	and function definitions are legal.
 *
 */
void parse (void)
{
	if (!startup_incl) {
		inc_startup();
		incl_huc_h();
		incl_globals_h();
	}

	while (1) {
		blanks();
		if (feof(input))
			break;
// Note:
// At beginning of 'parse' call, the header has been output to '.s'
// file, as well as all the -Asym=val operands from command line.
//
// But initial '#include "startup.asm"' statement was not yet output
// (allowing for additional asm define's to be parsed and output first.
//
// We can parse some trivial tokens first (including the asmdef...),
// but the #include "startup.asm" line must be output before actual code
// (And only once...)
//
		if (amatch("#asmdef", 7)) {
			doasmdef();
			continue;
		}

		if (amatch("extern", 6))
			dodcls(EXTERN, NULL_TAG, 0);
		else if (amatch("static", 6)) {
			if (amatch("const", 5)) {
				/* XXX: what about the static part? */
				dodcls(CONST, NULL_TAG, 0);
			}
			else
				dodcls(STATIC, NULL_TAG, 0);
		}
		else if (amatch("const", 5))
			dodcls(CONST, NULL_TAG, 0);
		else if (amatch("typedef", 7))
			dotypedef();
		else if (dodcls(PUBLIC, NULL_TAG, 0)) ;
		else if (match("#asm"))
			doasm();
		else if (match("#include"))
			doinclude();
		else if (match("#inc"))
			dopsdinc();
		else if (match("#def"))
			dopsddef();
		else
			newfunc(NULL, 0, 0, 0, 0);
	}
	if (optimize)
		flush_ins();
}

/*
 *		parse top level declarations
 */
int dodcls (int stclass, TAG_SYMBOL *mtag, int is_struct)
{
	int err;
	struct type_type t;

	if (amatch("__zp", 4)) {
		if ((stclass & STORAGE) == CONST || mtag != NULL_TAG)
			error("syntax error, __zp can only be used for variables");
		else
			stclass |= ZEROPAGE;
	}
	blanks();

	if (match_type(&t, NO, YES)) {
		if (t.type_type == CSTRUCT && t.otag == -1)
			t.otag = define_struct(t.sname, stclass & ~ZEROPAGE, !!(t.flags & F_STRUCT));
		else if (t.type_type == CENUM) {
			if (t.otag == -1)
				t.otag = define_enum(t.sname, stclass & ~ZEROPAGE);
			t.type_type = enum_types[t.otag].base;
		}
		err = declglb(t.type_type, stclass, mtag, t.otag, is_struct);
	}
	else if (stclass == PUBLIC)
		return (0);
	else
		err = declglb(CINT, stclass, mtag, NULL_TAG, is_struct);

	if (err == 2)	/* function */
		return (1);
	else if (err) {
		kill();
		return (0);
	}
	else
		needsemicolon();
	return (1);
}

void dotypedef (void)
{
	struct type_type t;

	if (!match_type(&t, YES, NO)) {
		error("unknown type");
		kill();
		return;
	}
	if (t.type_type == CENUM) {
		if (t.otag == -1) {
			if (user_short_enums)
				warning(W_GENERAL,
					"typedef to undefined enum; "
					"assuming base type int");
			t.type_type = CINT;
		}
		else
			t.type_type = enum_types[t.otag].base;
	}
	if (!symname(t.sname)) {
		error("invalid type name");
		kill();
		return;
	}
	typedefs = realloc(typedefs, (typedef_ptr + 1) * sizeof(struct type_type));
	typedefs[typedef_ptr++] = t;
	needsemicolon();
}

/*
 *	dump the literal pool
 */
void dumplits (void)
{
	int j, k;

	if ((litptr == 0) && (const_nb == 0))
		return;

	outstr("\t.rodata\n");
	if (litptr) {
		outconst(litlab);
		col();
		k = 0;
		while (k < litptr) {
			defbyte();
			j = 8;
			while (j--) {
				outdec(litq[k++] & 0xFF);
				if ((j == 0) | (k >= litptr)) {
					nl();
					break;
				}
				outbyte(',');
			}
		}
	}
	if (const_nb)
		dump_const();
}

/**
 * dump struct data
 * @param symbol struct variable
 * @param position position of the struct in the array, or zero
 */
int dump_struct (SYMBOL *symbol, int position)
{
	int dumped_bytes = 0;
	int i, number_of_members, value;

	number_of_members = tag_table[symbol->tagidx].number_of_members;
	for (i = 0; i < number_of_members; i++) {
		// i is the index of current member, get type
		int member_type = member_table[tag_table[symbol->tagidx].member_idx + i].sym_type;
		if (member_type == CCHAR || member_type == CUCHAR) {
			defbyte();
			dumped_bytes += 1;
		}
		else {
			/* XXX: compound types? */
			defword();
			dumped_bytes += 2;
		}
		if (position < get_size(symbol->name)) {
			// dump data
			value = get_item_at(symbol->name, position * number_of_members + i, &tag_table[symbol->tagidx]);
			outdec(value);
		}
		else {
			// dump zero, no more data available
			outdec(0);
		}
		nl();
	}
	return (dumped_bytes);
}

static int have_init_data = 0;
/* Initialized data must be kept in one contiguous block; pceas does not
   provide segments for that, so we keep the definitions and data in
   separate buffers and dump them all together after the last input file.
 */
#define DATABUFSIZE 65536
char data_buffer[DATABUFSIZE+256];
int  data_offset = 0;
char rodata_buffer[DATABUFSIZE+256];
int  rodata_offset = 0;

char *current_buffer = NULL;
int   current_offset = 0;

/*
 *	buffered print symbol prefix character
 *
 */
void prefixBuffer(void)
{
	current_buffer[current_offset++] = '_';
}

/*
 *	buffered print string
 *
 */
void outstrBuffer (char *ptr)
{
	int i = 0;

	if (current_offset >=	DATABUFSIZE) {
		printf("HuC compiler overrun detected, DATABUFSIZE is too small!\n");
		exit(-1);
	}

	while (ptr[i])
		current_buffer[current_offset++] = ptr[i++];
}

/*
 *	buffered print character
 *
 */
char outbyteBuffer (char c)
{
	if (c == 0)
		return (0);

	current_buffer[current_offset++] = c;

	return (c);
}

/*
 *	buffered print decimal number
 *
 */
void outdecBuffer (int number)
{
	if (current_offset >=	DATABUFSIZE) {
		printf("HuC compiler overrun detected, DATABUFSIZE is too small!\n");
		exit(-1);
	}

	current_offset += sprintf(current_buffer + current_offset, "%ld", (long) number);
}

/*
 *	buffered print pseudo-op to define a byte
 *
 */
void nlBuffer (void)
{
	current_buffer[current_offset++] = EOL;
}

/*
 *	buffered print pseudo-op to define a byte
 *
 */
void defbyteBuffer (void)
{
	outstrBuffer("\t\tdb\t");
}

/*
 *	buffered print pseudo-op to define storage
 *
 */
void defstorageBuffer (void)
{
	outstrBuffer("\t\tds\t");
}

/*
 *	buffered print pseudo-op to define a word
 *
 */
void defwordBuffer (void)
{
	outstrBuffer("\t\tdw\t");
}

/**
 * buffered dump struct data
 * @param symbol struct variable
 * @param position position of the struct in the array, or zero
 */
int dump_structBuffer (SYMBOL *symbol, int position)
{
	int dumped_bytes = 0;
	int i, number_of_members, value;

	number_of_members = tag_table[symbol->tagidx].number_of_members;
	for (i = 0; i < number_of_members; i++) {
		// i is the index of current member, get type
		int member_type = member_table[tag_table[symbol->tagidx].member_idx + i].sym_type;
		if (member_type == CCHAR || member_type == CUCHAR) {
			defbyteBuffer();
			dumped_bytes += 1;
		}
		else {
			/* XXX: compound types? */
			defwordBuffer();
			dumped_bytes += 2;
		}
		if (position < get_size(symbol->name)) {
			// dump data
			value = get_item_at(symbol->name, position * number_of_members + i, &tag_table[symbol->tagidx]);
			outdecBuffer(value);
		}
		else {
			// dump zero, no more data available
			outdecBuffer(0);
		}
		nlBuffer();
	}
	return (dumped_bytes);
}

/*
 *	dump all static variables
 */
void dumpglbs (void)
{
	int i = 1;
	int dim, list_size, line_count;
	int j, pass;
	FILE *save = output;

	if (glbflag == 0) goto finished;

	/* This is done in several passes:
	   Pass 0: Dump initialization data into const bank.
	   Pass 1: Define space for zp data.
	   Pass 2: Define space for uninitialized data.
	   Pass 3: Define space for initialized data.
	 */
	for (pass = 0; pass < 4; ++pass) {
		i = 1;
		for (cptr = (symtab + rglbsym_index); cptr < (symtab + glbsym_index); cptr++) {
			if (cptr->identity == FUNCTION)
				continue;
			if ((cptr->storage & WRITTEN) != 0 ||
			    (cptr->storage & STORAGE) == EXTERN)
				continue;

			dim = cptr->offset;
			if (find_symbol_initials(cptr->name)) {
				/* symbol has initialized data */
				if (pass == 3) {
					/* define space for initialized data */
					current_buffer = data_buffer;
					current_offset = data_offset;
					if ((cptr->storage & STORAGE) != LSTATIC)
						prefixBuffer();
					outstrBuffer(cptr->name);
					outstrBuffer(":\n");
					defstorageBuffer();
					outdecBuffer(cptr->alloc_size);
					nlBuffer();
					cptr->storage |= WRITTEN;
					data_offset = current_offset;
					current_buffer = NULL;
					current_offset = 0;
					continue;
				}
				if (pass != 0)
					continue;
				/* output initialization data into const bank */
				current_buffer = rodata_buffer;
				current_offset = rodata_offset;
				have_init_data = 1;
				list_size = 0;
				line_count = 0;
				list_size = get_size(cptr->name);
				if (cptr->sym_type == CSTRUCT)
					list_size /= tag_table[cptr->tagidx].number_of_members;
				if (dim == -1)
					dim = list_size;
				int item;
				/* dim is an item count for non-compound types and a byte size
				   for compound types; dump_struct() wants an item number, so
				   we have to count both to get the right members out. */
				for (j = item = 0; j < dim; j++, item++) {
					if (cptr->sym_type == CSTRUCT)
						j += dump_structBuffer(cptr, item) - 1;
					else {
						if (line_count % 10 == 0) {
							nlBuffer();
							if (cptr->sym_type == CCHAR || cptr->sym_type == CUCHAR)
								defbyteBuffer();
							else
								defwordBuffer();
						}
						if (j < list_size) {
							// dump data
							int value = get_item_at(cptr->name, j, &tag_table[cptr->tagidx]);
							outdecBuffer(value);
						}
						else {
							// dump zero, no more data available
							outdecBuffer(0);
						}
						line_count++;
						if (line_count % 10 == 0)
							line_count = 0;
						else {
							if (j < dim - 1)
								outbyteBuffer(',');
						}
					}
				}
				nlBuffer();
				rodata_offset = current_offset;
				current_buffer = NULL;
				current_offset = 0;
				continue;
			}
			else {
				/* symbol is uninitialized */
				if (pass == 0)
					continue;
				if (pass == 1 && !(cptr->storage & ZEROPAGE))
					continue;
				/* define space in bss */
				if (i) {
					i = 0;
					nl();
					if (pass == 1)
						gzp();
					else
						gdata();
				}
				if ((cptr->storage & STORAGE) != LSTATIC)
					prefix();
				outstr(cptr->name);
				outstr(":\n");
				defstorage();
				outdec(cptr->alloc_size);
				nl();
				cptr->storage |= WRITTEN;
				continue;
			}
		}
	}

finished:
	if (i) {
		nl();
		gdata();
	}
	output = save;
}

static void dumpfinal (void)
{
	int i;

	if (leaf_cnt) {
		outstr("leaf_loc:\n\t\tds\t");
		outdec(leaf_size);
		nl();
		for (i = 0; i < leaf_cnt; i++) {
			outstr("__");
			outstr(leaf_functions[i]);
			outstr("_end:\n");
		}
	}
	outstr("\n__bss_init:\n\n");
	if (data_offset != 0) {
		data_buffer[data_offset] = '\0';
		outstr(data_buffer);
	}
	outstr("__heap_start:\n");
	if (rodata_offset != 0) {
		rodata_buffer[rodata_offset] = '\0';
		nl();
		ol(".rodata");
		outstr("__rom_init:\n");
		outstr(rodata_buffer);
	}
}

/*
 *	report errors
 */
void errorsummary (void)
{
	if (ncmp)
		error("missing closing bracket");
	nl();
	comment();
	outdec(errcnt);
	if (errcnt) errfile = YES;
	outstr(" error(s) in compilation");
	nl();
	comment();
	ot("literal pool:");
	outdec(litptr);
	nl();
	comment();
	ot("constant pool:");
	outdec(const_size);
	nl();
	comment();
	ot("global pool:");
	outdec(glbsym_index - rglbsym_index);
	nl();
	comment();
	ot("Macro pool:");
	outdec(macptr);
	nl();
	pl(errcnt ? "Error(s)" : "No errors");
}

char extension (char *s)
{
	s += strlen(s) - 2;
	if (*s == '.')
		return (*(s + 1));

	return (' ');
}

#ifndef _WIN32
#include <unistd.h>
#endif

int assemble (char *s)
{
#if defined(_WIN32)

	char *exe;
	char buf[512];
	char* p;

	exe = getenv("PCE_PCEAS");
	if (!exe) {
		exe = "pceas.exe";
	}

	strcpy_s(buf, sizeof(buf), exe);
	strcat_s(buf, sizeof(buf), " ");
	for (p = buf; (p = strchr(p, '/')) != NULL; *p++ = '\\');

	strcat_s(buf, sizeof(buf), "--hucc ");

	if (sgflag)
		strcat_s(buf, sizeof(buf), "--sgx ");

	switch (cdflag) {
	case 1:
		strcat_s(buf, sizeof(buf), "--cd ");
		break;

	case 2:
		strcat_s(buf, sizeof(buf), "--scd ");
		break;

	default:
		strcat_s(buf, sizeof(buf), "--raw --pad ");
		break;
	}

	if (overlayflag)
		strcat_s(buf, sizeof(buf), "--over ");

	if (verboseflag) {
		strcat_s(buf, sizeof(buf), "-S -l 3 -m ");
	}

	if (debug_info) {
		const char * debug_opt;
		switch (debug_info) {
		case 'C': debug_opt = "-gC "; break;
		case 'A': debug_opt = "-gA "; break;
		default:  debug_opt = "-gL "; break;
		}
		strcat_s(buf, sizeof(buf), debug_opt);
	}

	strcat_s(buf, sizeof(buf), "\"");
	strcat_s(buf, sizeof(buf), s);
	buf[strlen(buf) - 1] = 's';
	strcat_s(buf, sizeof(buf), "\"");

// Comment this out later...
//	printf("invoking pceas:\n");
//	printf("%s\n", buf);

	return (system(buf));

#elif defined(__unix__) || defined(__APPLE__)

	char *exe;
	char buf[512];
	char *opts[16];
	int i = 0;

	exe = getenv("PCE_PCEAS");
	if (!exe) {
		exe = "pceas";
	}
	opts[i++] = exe;
	opts[i++] = "--hucc";		/* --newproc --strip -O and more! */

	if (sgflag)
		opts[i++] = "--sgx";

	switch (cdflag) {
	case 1:
		opts[i++] = "--cd";
		break;

	case 2:
		opts[i++] = "--scd";
		break;

	default:
		opts[i++] = "--raw";
		opts[i++] = "--pad";
		break;
	}

	if (overlayflag)
		opts[i++] = "--over";	/* compile as overlay */


	if (verboseflag) {
		opts[i++] = "-S";	/* asm: display full segment map */
		opts[i++] = "-l 3";	/* top listing output */
		opts[i++] = "-m";	/* force macros also */
	}
	else
		opts[i++] = "-l 0";

	if (debug_info) {
		switch (debug_info) {
		case 'C': opts[i++] = "-gC"; break;
		case 'A': opts[i++] = "-gA"; break;
		default:  opts[i++] = "-gL"; break;
		}
	}

	strcpy(buf, s);
	buf[strlen(buf) - 1] = 's';
	opts[i++] = buf;

	opts[i++] = 0;

// Comment this out later...
//	{
//		int j;
//		printf("invoking pceas:\n");
//		for (j = 0; j < i; j++)
//			printf("arg[%d] = %s\n", j, opts[j]);
//	}

	return (execvp(exe, (char *const *)opts));

#else
#error Add calling sequence depending on your OS
#endif
}
