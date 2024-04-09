#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "defs.h"
#include "externs.h"
#include "protos.h"

t_proc *proc_tbl[HASH_COUNT];
t_proc *proc_ptr;
t_proc *proc_first;
t_proc *proc_last;
int proc_nb;
int call_1st;
int call_ptr;
int call_bank;

/* this is set when suppressing the listing output of stripped procedures */
/* n.b. fully compatible with 2-pass assembly because code is still built */
int cloaking_stripped;

/* this is set when not assembling the code within the stripped procedure */
/* n.b. not compatible with 2-pass assembly because symbol addresses will */
/* change because both multi-label and branch tracking counts will change */
int skipping_stripped;

/* this is set to say that skipping is an acceptable alternative to */
/* cloaking, which means that we've decided to do a 3-pass assembly */
int allow_skipping;

/* set this to spew procedure stripping information to the tty */
#define DEBUG_STRIPPING 0

extern int dump_seg;

/* protos */
struct t_proc *proc_look(void);
int            proc_install(void);
void           poke(int addr, int data);
void           proc_sortlist(void);


/* ----
 * do_call()
 * ----
 * call pseudo - this handles both jsr and jmp!
 */

void
do_call(int *ip)
{
	struct t_proc *proc;

	/* define label, unless already defined in classC() instruction flow */
	if (opflg == PSEUDO)
		labldef(LOCATION);

	/* update location counter */
	data_loccnt = loccnt;
	loccnt += 3;

	/* get value */
	if (!evaluate(ip, ';', 0))
		return;

	/* generate code */
	if (pass == LAST_PASS) {
		/* lookup proc table */
		if ((expr_lablcnt == 1) && (complex_expr == 0) && (expr_lablptr != NULL) &&
		    ((proc = expr_lablptr->proc) != NULL) && (proc->label == expr_lablptr)) {

			/* check banks */
			if ((newproc_opt == 0) && (bank == proc->bank)) {
				/* same bank */
				value = proc->org + 0xA000;
			} else {
				/* different bank */
				if (proc->call) {
					value = proc->call;
				} else {
					/* init */
					if (call_bank > max_bank) {
						/* don't increase ROM size until we need a trampoline */
						if (call_bank > bank_limit) {
							fatal_error("There is no target memory left to allocate a bank for .proc trampolines!");
							if (asm_opt[OPT_OPTIMIZE] == 0) {
								printf("Optimized procedure packing is currently disabled, use \"-O\" to enable.\n\n");
							}
							return;
						}
						max_bank = call_bank;
					}

					/* new call */
					if (newproc_opt == 0) {
						/* check that the new trampoline won't overrun the bank */
						if (((call_ptr + 17) & 0xE000) != (call_1st & 0xE000)) {
							error("The .proc trampoline bank is full, there are too many procedures!");
							return;
						}

						/* install HuC trampolines at start of MPR4, map code into MPR5 */
						value = call_ptr;

						poke(call_ptr++, 0xA8);			// tay
						poke(call_ptr++, 0x43);			// tma #5
						poke(call_ptr++, 0x20);
						poke(call_ptr++, 0x48);			// pha
						poke(call_ptr++, 0xA9);			// lda #...
						poke(call_ptr++, proc->label->mprbank);
						poke(call_ptr++, 0x53);			// tam #5
						poke(call_ptr++, 0x20);
						poke(call_ptr++, 0x98);			// tya
						poke(call_ptr++, 0x20);			// jsr ...
						poke(call_ptr++, (proc->org + 0xA000) & 255);
						poke(call_ptr++, (proc->org + 0xA000) >> 8);
						poke(call_ptr++, 0xA8);			// tay
						poke(call_ptr++, 0x68);			// pla
						poke(call_ptr++, 0x53);			// tam #5
						poke(call_ptr++, 0x20);
						poke(call_ptr++, 0x98);			// tya
						poke(call_ptr++, 0x60);			// rts
					} else {
						/* check that the new trampoline won't overrun the bank */
						if (((call_ptr - 9) & 0xE000) != (call_1st & 0xE000)) {
							error("The .proc trampoline bank is full, there are too many procedures!");
							return;
						}

						/* install new trampolines at end of MPR7, map code into MPR6 */
						poke(call_ptr--, (proc->org + 0xC000) >> 8);
						poke(call_ptr--, (proc->org + 0xC000) & 255);
						poke(call_ptr--, 0x4C);			// jmp ...
						poke(call_ptr--, 0x40);
						poke(call_ptr--, 0x53);			// tam #6
						poke(call_ptr--, proc->label->mprbank);
						poke(call_ptr--, 0xA9);			// lda #...
						poke(call_ptr--, 0x48);			// pha
						poke(call_ptr--, 0x40);
						poke(call_ptr--, 0x43);			// tma #6

						value = call_ptr + 1;
					}

					proc->call = value;
				}

				/* special handling for a jmp between procedures */
				if ((newproc_opt != 0) && (optype == 1)) {
					if (proc_ptr) {
						/* don't save tma6 again if already in a procedure */
						if (bank == proc->bank)
							value = proc->org + 0xC000;
						else
							value = value + 3;
					}
				}
			}
		}
		else {
			/* do not reference a procedure in this way! */
			if ((expr_lablptr != NULL) && (expr_lablptr->proc != NULL) && (expr_lablptr->proc->label == expr_lablptr)) {
				error("Illegal entry point to procedure label!");
				return;
			}
		}

		/* opcode */
		if (optype == 0)
			putbyte(data_loccnt, 0x20); /* JSR */
		else
			putbyte(data_loccnt, 0x4C); /* JMP */

		putword(data_loccnt+1, value);

		/* output line */
		println();
	}
}


/* ----
 * do_leave()
 * ----
 * leave pseudo
 */

void
do_leave(int *ip)
{
	/* define label */
	labldef(LOCATION);

	/* check end of line */
	if (!check_eol(ip))
		return;

	/* update location counter */
	data_loccnt = loccnt;
	loccnt += (newproc_opt != 0) ? 3 : 1;

	/* generate code */
	if (pass == LAST_PASS) {
		if (newproc_opt != 0) {
			/* "jmp" opcode */
			putbyte(data_loccnt, 0x4C);
			putword(data_loccnt+1, call_1st - 4);
		} else {
			/* "rts" opcode */
			putbyte(data_loccnt, 0x60);
		}

		/* output line */
		println();
	}
}


/* ----
 * do_proc()
 * ----
 * .proc pseudo
 */

void
do_proc(int *ip)
{
	struct t_proc *ptr;

	/* special checks for KickC procedures */
	if (optype == P_KICKC) {
		/* reserve "{}" syntax for KickC code */
		if (kickc_mode == 0) {
			fatal_error("Cannot use \"{}\" syntax in .pceas mode!");
			return;
		}

		/* check symbol */
		if (lablptr == NULL) {
			fatal_error("KickC procedure name is missing!");
			return;
		}
	}

	/* do not mix different types of label-scope */
	if (scopeptr) {
		fatal_error("Cannot declare a .proc/.procgroup inside a .struct!");
			return;
	}

	/* check if nesting procs/groups */
	if (proc_ptr) {
		if (optype == P_PGROUP) {
			fatal_error("Cannot declare a .procgroup inside a .proc/.procgroup!");
			return;
		}
		else {
			if (proc_ptr->type != P_PGROUP) {
				fatal_error("Cannot nest procedures!");
				return;
			}
		}
	}

	/* get proc name */
	if (lablptr) {
		/* copy the procedure name from the label */
		strcpy(symbol, lablptr->name);
	}
	else {
		/* skip spaces */
		while (isspace(prlnbuf[*ip]))
			(*ip)++;

		/* extract name */
		if (!colsym(ip, 0)) {
			if (symbol[0])
				return;
			if (optype == P_PROC) {
				fatal_error(".proc name is missing!");
				return;
			}

			/* default name */
			sprintf(&symbol[1], "__group_%d_%d__", input_file[infile_num].file->number, slnum);
			symbol[0] = strlen(&symbol[1]);
		}

		/* lookup symbol table */
		if ((lablptr = stlook(SYM_DEF)) == NULL)
			return;
	}

	/* check symbol */
	if (symbol[1] == '.' || symbol[1] == '@') {
		fatal_error(".proc/.procgroup name cannot be a local label!");
		return;
	}
	if (symbol[1] == '!') {
		fatal_error(".proc/.procgroup name cannot be a multi-label!");
		return;
	}

	/* check end of line */
	if (!check_eol(ip))
		return;

	/* search (or create new) proc */
	if ((ptr = proc_look()))
		proc_ptr = ptr;
	else {
		if (!proc_install())
			return;
	}
	if (pass == FIRST_PASS && proc_ptr->defined) {
		fatal_error(".proc/.procgroup multiply defined!");
		return;
	}

	/* reset location and size in case it changed due to skipping */
	if (pass != LAST_PASS) {
		proc_ptr->org = proc_ptr->base = proc_ptr->group ? loccnt : 0;
		proc_ptr->label->data_size = proc_ptr->size = 0;
	}

	/* can we just not assemble a stripped .proc/.procgroup */
	/* at all instead of assembling it to the STRIPPED_BANK */
	if (proc_ptr->bank == STRIPPED_BANK && allow_skipping && proc_ptr->is_skippable) {
		#if DEBUG_STRIPPING
		printf("Skipping %s \"%s\" with parent \"%s\".\n",
			proc_ptr->type == P_PROC ? ".proc" : ".procgroup",
			proc_ptr->label->name + 1,
			proc_ptr->group == NULL ? "none" : proc_ptr->group->label->name + 1);
		#endif
		proc_ptr = proc_ptr->group;
		skipping_stripped = optype;
		if (pass == LAST_PASS) {
			println();
		}
		return;
	}

	#if DEBUG_STRIPPING
	printf("Assembling %s \"%s\" to bank %d with parent \"%s\".\n",
		proc_ptr->type == P_PROC ? ".proc" : ".procgroup",
		proc_ptr->label->name + 1,
		proc_ptr->bank,
		proc_ptr->group == NULL ? "none" : proc_ptr->group->label->name + 1);
	#endif

	/* increment proc ref counter */
	proc_ptr->defined = 1;
	proc_ptr->is_skippable = if_level;

	/* backup current bank infos */
	bank_glabl[section][bank]  = proc_ptr->old_glablptr = glablptr;
	bank_loccnt[section][bank] = proc_ptr->old_loccnt = loccnt;
	bank_page[section][bank]   = proc_ptr->old_page = page;
	proc_ptr->old_bank = bank;
	proc_nb++;

	/* set new bank infos */
	bank     = proc_ptr->bank;
	page     = (newproc_opt == 0) ? 5 : 6;
	loccnt   = proc_ptr->org;
	glablptr = lablptr;

	/* signal discontiguous change in loccnt */
	discontiguous = 1;

	/* define label */
	labldef(LOCATION);

	/* a KickC procedure also opens a label-scope */
	if (optype == P_KICKC) {
		lablptr->scope = scopeptr;
		scopeptr = lablptr;
	}

	/* output */
	if (pass == LAST_PASS) {
		if (proc_ptr->bank == STRIPPED_BANK) {
			println();
			++cloaking_stripped;
		} else {
			loadlc(loccnt, 0);
			println();
		}
	}
}


/* ----
 * do_endp()
 * ----
 * .endp pseudo
 */

void
do_endp(int *ip)
{
	/* special checks for KickC procedures */
	if (optype == P_KICKC) {
		/* reserve "{}" syntax for KickC code */
		if (kickc_mode == 0) {
			fatal_error("Cannot use \"{}\" syntax in .pceas mode!");
			return;
		}
	}

	if (proc_ptr == NULL) {
		fatal_error("Unexpected .endp/.endprocgroup!");
		return;
	}
	if (optype != proc_ptr->type) {
		fatal_error("Unexpected .endp/.endprocgroup!");
		return;
	}

	/* check end of line */
	if (!check_eol(ip))
		return;

	/* disable skipping if the procedure starts and ends at different .if nesting */
	if (!(proc_ptr->is_skippable = (proc_ptr->is_skippable == if_level))) {
		if (asm_opt[OPT_WARNING])
			warning("Warning: .proc/.procgroup has mismatched .if nesting!\n");
	}

	/* restore procedure's initial section */
	if (section != proc_ptr->label->section) {
		/* backup current section data */
		section_bank[section] = bank;
		bank_glabl[section][bank] = glablptr;
		bank_loccnt[section][bank] = loccnt;
		bank_page[section][bank] = page;

		/* change section */
		section = proc_ptr->label->section;

		/* switch to the new section */
		bank = section_bank[section];
		page = bank_page[section][bank];
		loccnt = bank_loccnt[section][bank];
		glablptr = bank_glabl[section][bank];

		/* signal discontiguous change in loccnt */
		discontiguous = 1;
	}

	/* define label */
	labldef(LOCATION);

	/* sanity check */
	if (bank != proc_ptr->bank) {
		fatal_error(".endp/.endprocgroup is in a different bank to .proc/,procgroup!");
		return;
	}

	/* record proc size */
	proc_ptr->label->data_type = proc_ptr->type;
	if (pass != LAST_PASS) {
		proc_ptr->label->data_size =
		proc_ptr->size = loccnt - proc_ptr->base;
	}

	/* restore previous bank settings */
	bank = proc_ptr->old_bank;

	if (proc_ptr->group == NULL) {
		page     = bank_page[section][bank]   = proc_ptr->old_page;
		loccnt   = bank_loccnt[section][bank] = proc_ptr->old_loccnt;
		glablptr = bank_glabl[section][bank]  = proc_ptr->old_glablptr;

		/* signal discontiguous change in loccnt */
		discontiguous = 1;
	}

	#if DEBUG_STRIPPING
	printf("Ending %s \"%s\" with parent \"%s\".\n",
		optype == P_PROC ? ".proc" : ".procgroup",
		proc_ptr->label->name + 1,
		proc_ptr->group == NULL ? "none" : proc_ptr->group->label->name + 1);
	#endif

	proc_ptr = proc_ptr->group;

	/* a KickC procedure also closes the label-scope */
	if (optype == P_KICKC) {
		/* return to previous scope */
		if (scopeptr == NULL) {
			fatal_error("Why is scopeptr NULL here?!?!");
			return;
		}
		scopeptr = scopeptr->scope;
	}

	/* output */
	if (pass == LAST_PASS) {
		if (cloaking_stripped)
			--cloaking_stripped;
		println();
	}
}


/* ----
 * proc_strip()
 * ----
 *
 */

void
proc_strip(void)
{
	int num_stripped = 0;

	if (proc_nb == 0)
		return;

	if (strip_opt == 0)
		return;

	/* calculate the refthispass for each group */
	proc_ptr = proc_first;

	while (proc_ptr) {
		/* proc within a group */
		if (proc_ptr->group != NULL) {
			proc_ptr->group->label->refthispass += proc_ptr->label->refthispass;
		}
		proc_ptr = proc_ptr->link;
	}

	/* strip out the groups and procs with zero references */
	proc_ptr = proc_first;

	while (proc_ptr) {

		/* group or standalone proc */
		if (proc_ptr->group == NULL) {
			/* strip this .proc or .procgroup? */
			if (proc_ptr->label->refthispass < 1) {
				/* strip this unused code from the ROM */
				#if DEBUG_STRIPPING
				printf("Stripping %s \"%s\" with parent \"%s\".\n",
					proc_ptr->type == P_PROC ? ".proc" : ".procgroup",
					proc_ptr->label->name + 1,
					proc_ptr->group == NULL ? "none" : proc_ptr->group->label->name + 1);
				#endif
				proc_ptr->bank = STRIPPED_BANK;
				--proc_nb;
				++num_stripped;
			}
		}

		/* proc within a group */
		else {
			/* strip this .proc? */
			if ((proc_ptr->group->bank == STRIPPED_BANK) ||
			    (proc_ptr->label->refthispass < 1 && allow_skipping && proc_ptr->is_skippable)) {
				/* strip this unused code from the ROM */
				#if DEBUG_STRIPPING
				printf("Stripping %s \"%s\" with parent \"%s\".\n",
					proc_ptr->type == P_PROC ? ".proc" : ".procgroup",
					proc_ptr->label->name + 1,
					proc_ptr->group == NULL ? "none" : proc_ptr->group->label->name + 1);
				#endif
				proc_ptr->bank = STRIPPED_BANK;
				--proc_nb;
				++num_stripped;
			}
		}

		/* next */
		proc_ptr->defined = 0;
		proc_ptr = proc_ptr->link;
	}
}


/* ----
 * proc_reloc()
 * ----
 *
 */

void
proc_reloc(void)
{
	struct t_symbol *sym;
	struct t_symbol *local;
	struct t_proc   *group;
	int num_relocated = 0;
	int i;
	int *bank_free = NULL;
	int new_bank = 0;

	if (proc_nb == 0)
		return;

	/* init */
	bank_free = (int*) malloc(sizeof(int) * (bank_limit+1));
	if (!bank_free) {
		fatal_error("Not enough RAM to allocate banks!");
		return;
	}

	for (i = 0; i <= bank_limit; i++) {
		bank_free[i] = 0x2000 - bank_maxloc[i];
	}

	/* sort procedures by size (largest first) for better packing */
	if (asm_opt[OPT_OPTIMIZE])
		proc_sortlist();
	else {
		bank_free[max_bank] = 0;
		new_bank = max_bank + 1;
	}

	/* alloc memory */
	proc_ptr = proc_first;

	while (proc_ptr) {

		/* group or standalone proc */
		if (proc_ptr->group == NULL) {

			/* relocate if not stripped */
			if (proc_ptr->bank != STRIPPED_BANK) {
				/* relocate code to any unused bank in ROM */
				int reloc_bank = -1;
				int check_bank = 0;
				int check_last = 0;
				int smallest = 0x2000;

				while (reloc_bank == -1)
				{
					check_bank = (asm_opt[OPT_OPTIMIZE]) ? 0 : max_bank;

					/* limit procs to the first 64 banks if using the SF2 mapper */
					check_last = (section_flags[S_DATA] & S_IS_SF2) ? 63 : max_bank;

					for (; check_bank <= check_last; check_bank++)
					{
						/* don't use a full bank, even if proc_ptr->size==0 */
						if ((bank_free[check_bank] != 0) && (bank_free[check_bank] >= proc_ptr->size))
						{
							if (smallest > (bank_free[check_bank] - proc_ptr->size))
							{
								smallest = bank_free[check_bank] - proc_ptr->size;
								reloc_bank = check_bank;
							}
						}
					}

					if (reloc_bank == -1)
					{
						/* new bank needed, is there space below 1MB? */
						if ((max_bank >= 127) || (max_bank >= bank_limit)) {
							int total = 0, totfree = 0;
							struct t_proc *current = NULL;

							current = proc_ptr;

							fatal_error("\nThere is not enough free target memory for all the procedures!\n");

							if (asm_opt[OPT_OPTIMIZE] == 0) {
								printf("Optimized procedure packing is currently disabled, use \"-O\" to enable.\n\n");
							}

							for (i = new_bank; i <= max_bank; i++) {
								printf("BANK %3X: %d bytes free\n", i, bank_free[i]);
								totfree += bank_free[i];
							}
							printf("\nTotal free space in all banks %d.\n\n", totfree);

							total = 0;
							proc_ptr = proc_first;
							while (proc_ptr) {
								if (proc_ptr->bank == PROC_BANK) {
									printf("Proc: %s Bank: 0x%02X Size: %4d %s\n",
										proc_ptr->label->name + 1, proc_ptr->bank == PROC_BANK ? 0 : proc_ptr->bank, proc_ptr->size,
										proc_ptr->bank == PROC_BANK && proc_ptr == current ? " ** too big **" : proc_ptr->bank == PROC_BANK ? "** unassigned **" : "");
									total += proc_ptr->size;
								}
								proc_ptr = proc_ptr->link;
							}
							printf("\nTotal bytes that didn't fit in ROM: %d\n\n", total);
							if (totfree > total && current)
								printf("Try splitting the \"%s\" procedure into smaller chunks.\n\n", current->label->name + 1);
							else
								printf("There are %d bytes that won't fit into the currently available BANK space\n\n", total - totfree);
							errcnt++;

							return;
						}
						reloc_bank = ++max_bank;
					}
				}

				proc_ptr->bank = reloc_bank;
				proc_ptr->org = 0x2000 - bank_free[reloc_bank];

				bank_free[reloc_bank] -= proc_ptr->size;

				++num_relocated;
			}
		}

		/* proc within a group */
		else {
			/* reloc proc */
			group = proc_ptr->group;
			if (proc_ptr->bank != STRIPPED_BANK)
				proc_ptr->bank = group->bank;
			proc_ptr->org += (group->org - group->base);
		}

		/* next */
		proc_ptr->defined = 0;
		proc_ptr = proc_ptr->link;
	}

	free(bank_free);
	bank_free = NULL;

	/* remap proc symbols */
	for (i = 0; i < HASH_COUNT; i++) {
		sym = hash_tbl[i];

		while (sym) {
			proc_ptr = sym->proc;

			/* remap addr */
			if (sym->proc) {
				if (proc_ptr->bank == STRIPPED_BANK) {
					sym->rombank =
					sym->mprbank = STRIPPED_BANK;
					sym->overlay = 0;
				} else {
					sym->rombank = proc_ptr->bank;
					sym->mprbank = bank2mprbank(sym->rombank, sym->section);
					sym->overlay = bank2overlay(sym->rombank, sym->section);
				}

				sym->value += (proc_ptr->org - proc_ptr->base);

				/* local symbols */
				if (sym->local) {
					local = sym->local;

					while (local) {
						proc_ptr = local->proc;

						/* remap addr */
						if (local->proc) {
							if (proc_ptr->bank == STRIPPED_BANK) {
								local->rombank =
								local->mprbank = STRIPPED_BANK;
								local->overlay = 0;
							} else {
								local->rombank = proc_ptr->bank;
								local->mprbank = bank2mprbank(local->rombank, local->section);
								local->overlay = bank2overlay(local->rombank, local->section);
							}

							local->value += (proc_ptr->org - proc_ptr->base);
						}

						/* next */
						local = local->next;
					}
				}
			}

			/* next */
			sym = sym->next;
		}
	}

	/* reset */
	proc_ptr = NULL;

	/* initialize trampoline bank/addr after relocation */
	if (newproc_opt) {
		call_bank = 0;
		call_ptr  = 0xFFF5;
	} else {
		call_bank = max_bank + 1;
		call_ptr  = 0x8000;
	}

	if (lablexists("__trampolinebnk")) {
		call_bank = lablptr->value & 0x7F;
	}
	if (lablexists("__trampolineptr")) {
		call_ptr  = lablptr->value & 0xFFFF;
	}

	call_1st = call_ptr;

	/* initialize the "leave_proc" routine for exiting a procedure */
	if ((newproc_opt != 0) && (num_relocated != 0)) {
		/* install code for leaving .proc */
		/* fix do_proc() if this changes! */
		poke(call_ptr--, 0x60);			// rts
		poke(call_ptr--, 0x98);			// tya
		poke(call_ptr--, 0x40);
		poke(call_ptr--, 0x53);			// tam #6
		poke(call_ptr--, 0x68);			// pla

		lablset("leave_proc", call_ptr + 1);
	}
}


/* ----
 * proc_look()
 * ----
 *
 */

struct t_proc *
proc_look(void)
{
	struct t_proc *ptr;
	int hash;

	/* search the procedure in the hash table */
	hash = symhash();
	ptr = proc_tbl[hash];
	while (ptr) {
		if (!strcmp(symbol, ptr->label->name))
			break;
		ptr = ptr->next;
	}

	/* ok */
	return (ptr);
}


/* ----
 * proc_install()
 * ----
 * install a procedure in the hash table
 *
 */

int
proc_install(void)
{
	struct t_proc *ptr;
	int hash;

	/* allocate a new proc struct */
	if ((ptr = (void *)malloc(sizeof(struct t_proc))) == NULL) {
		error("Out of memory!");
		return (0);
	}

	/* initialize it */
	hash = symhash();
	ptr->bank = (optype == P_PGROUP)  ? GROUP_BANK : PROC_BANK;
	ptr->base = proc_ptr ? loccnt : 0;
	ptr->org = ptr->base;
	ptr->size = 0;
	ptr->call = 0;
	ptr->kickc = kickc_mode;
	ptr->defined = 0;
	ptr->is_skippable = 0;
	ptr->link = NULL;
	ptr->next = proc_tbl[hash];
	ptr->group = proc_ptr;
	ptr->label = lablptr;
	ptr->type = optype;
	proc_ptr = ptr;
	proc_tbl[hash] = proc_ptr;

	/* link it */
	if (proc_first == NULL) {
		proc_first = proc_ptr;
		proc_last  = proc_ptr;
	}
	else {
		proc_last->link = proc_ptr;
		proc_last = proc_ptr;
	}

	/* ok */
	return (1);
}


/* ----
 * poke()
 * ----
 *
 */

void
poke(int addr, int data)
{
	/* do not overwrite existing data! check_trampolines() will report */
	/* this error later on and show a segment dump to provide help */
	if ((map[call_bank][(addr & 0x1FFF)] & 0x0F) == 0x0F) {
		rom[call_bank][(addr & 0x1FFF)] = data;
		map[call_bank][(addr & 0x1FFF)] = S_PROC + ((addr & 0xE000) >> 8);
	}
}


/* ----
 * proc_sortlist()
 * ----
 *
 */

void
proc_sortlist(void)
{
	struct t_proc *unsorted_list = proc_first;
	struct t_proc *sorted_list = NULL;

	while(unsorted_list)
	{
		proc_ptr = unsorted_list;
		unsorted_list = unsorted_list->link;
		proc_ptr->link = NULL;

		/* link it */
		if (sorted_list == NULL)
		{
			sorted_list = proc_ptr;
		}
		else
		{
			int inserted = 0;
			struct t_proc *list = sorted_list;
			struct t_proc *previous = NULL;
			while(!inserted && list)
			{
				if (list->size < proc_ptr->size)
				{
					if (!previous)
						sorted_list = proc_ptr;
					else
						previous->link = proc_ptr;
					proc_ptr->link = list;
					inserted = 1;
				}
				else
				{
					previous = list;
					list = list->link;
				}
			}

			if (!inserted)
				previous->link = proc_ptr;
		}
	}
	proc_first = sorted_list;
	return;
}


/* ----
 * list_procs()
 * ----
 * dump the procedure list to the listing file
 */

void
list_procs(void)
{
	struct t_proc *proc_ptr = proc_first;

	if ((lst_fp != NULL) && (proc_ptr != NULL) && (fprintf(lst_fp, "\nPROCEDURE LIST (in order of size):\n\n") > 0)) {
		while (proc_ptr) {
			if ((proc_ptr->group == NULL) && (proc_ptr->bank < UNDEFINED_BANK)) {
				if (fprintf( lst_fp, "Size: $%04X, Addr: $%02X:%04X, %s %s\n", proc_ptr->size, proc_ptr->bank, proc_ptr->label->value,
					(proc_ptr->type == P_PGROUP) ? ".procgroup" : "     .proc" , proc_ptr->label->name + 1) < 0)
					break;
			}
			proc_ptr = proc_ptr->link;
		}
	}
}


/* ----
 * check_trampolines()
 * ----
 * were they overwritten by other code/data?
 */

int
check_trampolines(void)
{
	int first_bad = -1;
	int final_bad = -1;
	int first_call;
	int final_call;

	if (newproc_opt == 0) {
		first_call = call_1st;
		final_call = call_ptr - 1;
	} else {
		first_call = call_ptr + 1;
		final_call = call_1st;
	}

	for (; first_call <= final_call; ++first_call) {
		if ((map[call_bank][first_call & 0x1FFF] & 0x1F) != S_PROC) {
			if (first_bad < 0)
				first_bad = first_call;
			final_bad = first_call;
		}
	}

	if (first_bad >= 0) {
		printf("Error: .proc trampolines between $%04X-$%04X are overwritten by code or data!\n\nTrampoline Bank ...\n",
			first_bad, final_bad);
		dump_seg = 2;
		show_bank_usage(call_bank);
		printf("\n");
		return (1);
	}
	return (0);
}
