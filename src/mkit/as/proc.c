#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "defs.h"
#include "externs.h"
#include "protos.h"

struct t_proc *proc_tbl[256];
struct t_proc *proc_ptr;
struct t_proc *proc_first;
struct t_proc *proc_last;
int proc_nb;
int call_1st;
int call_ptr;
int call_bank;

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
		labldef(0, 0, LOCATION);

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
							fatal_error("Not enough ROM space for proc trampolines!");
							if (asm_opt[OPT_OPTIMIZE] == 0) {
								printf("Procedure optimization is currently disabled, use \"-O\" to enable.\n\n");
							}
							return;
						}
						max_bank = call_bank;
					}

					/* new call */
					if (newproc_opt == 0) {
						/* check that the new trampoline won't overrun the bank */
						if (((call_ptr + 17) & 0xE000) != (call_1st & 0xE000)) {
							error("No more space in bank for .proc trampoline!");
							if (asm_opt[OPT_OPTIMIZE] == 0) {
								printf("Procedure optimization is currently disabled, use \"-O\" to enable.\n\n");
							}
							return;
						}

						/* install HuC trampolines at start of MPR4, map code into MPR5 */
						value = call_ptr;

						poke(call_ptr++, 0xA8);			// tay
						poke(call_ptr++, 0x43);			// tma #5
						poke(call_ptr++, 0x20);
						poke(call_ptr++, 0x48);			// pha
						poke(call_ptr++, 0xA9);			// lda #...
						poke(call_ptr++, proc->bank + bank_base);
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
							error("No more space in bank for .proc trampoline!");
							return;
						}

						/* install new trampolines at end of MPR7, map code into MPR6 */
						poke(call_ptr--, (proc->org + 0xC000) >> 8);
						poke(call_ptr--, (proc->org + 0xC000) & 255);
						poke(call_ptr--, 0x4C);			// jmp ...
						poke(call_ptr--, 0x40);
						poke(call_ptr--, 0x53);			// tam #6
						poke(call_ptr--, proc->bank + bank_base);
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
	labldef(0, 0, LOCATION);

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
			sprintf(&symbol[1], "__group_%i__", proc_nb + 1);
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
	if((ptr = proc_look()))
		proc_ptr = ptr;
	else {
		if (!proc_install())
			return;
	}
	if (pass == FIRST_PASS && proc_ptr->defined) {
		fatal_error(".proc/.procgroup multiply defined!");
		return;
	}

	/* increment proc ref counter */
	proc_ptr->defined = 1;

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
	labldef(0, 0, LOCATION);

	/* a KickC procedure also opens a label-scope */
	if (optype == P_KICKC) {
		lablptr->scope = scopeptr;
		scopeptr = lablptr;
	}

	/* output */
	if (pass == LAST_PASS) {
		loadlc(loccnt, 0);
		println();
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
	labldef(0, 0, LOCATION);

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
	if (pass == LAST_PASS)
		println();
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

	proc_ptr = proc_first;

	/* sum up each group's refcnt */
	while (proc_ptr) {
		/* proc within a group */
		if (proc_ptr->group != NULL) {
			proc_ptr->group->label->refcnt += proc_ptr->label->refcnt;
		}
		proc_ptr = proc_ptr->link;
	}

	proc_ptr = proc_first;

	/* alloc memory */
	while (proc_ptr) {

		/* group or standalone proc */
		if (proc_ptr->group == NULL) {

			/* relocate or strip? */
			if ((strip_opt != 0) && (proc_ptr->label->refcnt < 1)) {
				/* strip this unused code from the ROM */
				proc_ptr->bank = STRIPPED_BANK;
			} else {
				/* relocate code to any unused bank in ROM */
				int reloc_bank = -1;
				int check_bank = 0;
				int smallest = 0x2000;

				while (reloc_bank == -1)
				{
					check_bank = (asm_opt[OPT_OPTIMIZE]) ? 0 : max_bank;

					for (; check_bank <= max_bank; check_bank++)
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
						/* need new bank */
						if (++max_bank > bank_limit) {
							int total = 0, totfree = 0;
							struct t_proc *current = NULL;

							current = proc_ptr;

							fatal_error("\nThere is not enough free memory for all the procedures!\n");

							if (asm_opt[OPT_OPTIMIZE] == 0) {
								printf("Procedure optimization is currently disabled, use \"-O\" to enable.\n\n");
							}

							for (i = new_bank; i < max_bank; i++) {
								printf("BANK %02X: %d bytes free\n", i, bank_free[i]);
								totfree += bank_free[i];
							}
							printf("\nTotal free space in all banks %d.\n\n", totfree);

							total = 0;
							proc_ptr = proc_first;
							while (proc_ptr) {
								if (proc_ptr->bank == PROC_BANK) {
									printf("Proc: %s Bank: 0x%02X Size: %4d %s\n",
										proc_ptr->name, proc_ptr->bank == PROC_BANK ? 0 : proc_ptr->bank, proc_ptr->size,
										proc_ptr->bank == PROC_BANK && proc_ptr == current ? " ** too big **" : proc_ptr->bank == PROC_BANK ? "** unassigned **" : "");
									total += proc_ptr->size;
								}
								proc_ptr = proc_ptr->link;
							}
							printf("\nTotal bytes that didn't fit in ROM: %d\n\n", total);
							if (totfree > total && current)
								printf("Try splitting the \"%s\" procedure into smaller chunks.\n\n", current->name);
							else
								printf("There are %d bytes that won't fit into the currently available BANK space\n\n", total - totfree);
							errcnt++;

							return;
						}
						reloc_bank = max_bank;
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
	for (i = 0; i < 256; i++) {
		sym = hash_tbl[i];

		while (sym) {
			proc_ptr = sym->proc;

			/* remap addr */
			if (sym->proc) {
				if (proc_ptr->bank == STRIPPED_BANK)
					sym->bank = STRIPPED_BANK;
				else
					sym->bank = proc_ptr->bank + bank_base;

				sym->value = (sym->value & 0x007FFFFF);
				sym->value += (proc_ptr->org - proc_ptr->base);

				/* local symbols */
				if (sym->local) {
					local = sym->local;

					while (local) {
						proc_ptr = local->proc;

						/* remap addr */
						if (local->proc) {
							if (proc_ptr->bank == STRIPPED_BANK)
								local->bank = STRIPPED_BANK;
							else
								local->bank = proc_ptr->bank + bank_base;

							local->value = (local->value & 0x007FFFFF);
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
	proc_nb = 0;

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
		if (!strcmp(&symbol[1], ptr->name))
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
	strcpy(ptr->name, &symbol[1]);
	hash = symhash();
	ptr->bank = (optype == P_PGROUP)  ? GROUP_BANK : PROC_BANK;
	ptr->base = proc_ptr ? loccnt : 0;
	ptr->org = ptr->base;
	ptr->size = 0;
	ptr->call = 0;
	ptr->kickc = kickc_mode;
	ptr->defined = 0;
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
				if(list->size < proc_ptr->size)
				{
					if(!previous)
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

			if(!inserted)
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
			if ((proc_ptr->group == NULL) && (proc_ptr->bank < RESERVED_BANK)) {
				if (fprintf( lst_fp, "Size: $%04X, Addr: $%02X:%04X, %s %s\n", proc_ptr->size, proc_ptr->bank, proc_ptr->label->value,
					(proc_ptr->type == P_PGROUP) ? ".procgroup" : "     .proc" , proc_ptr->name) < 0)
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
		show_bnk_usage(call_bank);
		printf("\n");
		return (1);
	}
	return (0);
}
