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
int call_ptr;
int call_bank;

extern int strip_opt;
extern int newproc_opt;

/* protos */
struct t_proc *proc_look(void);
int            proc_install(void);
void           poke(int addr, int data);
void           proc_sortlist(void);


/* ----
 * do_call()
 * ----
 * call pseudo
 */

void
do_call(int *ip)
{
	struct t_proc *ptr;
	int value;

	/* define label */
	labldef(loccnt, 1);

	/* update location counter */
	data_loccnt = loccnt;
	loccnt += 3;

	/* skip spaces */
	while (isspace(prlnbuf[*ip]))
		(*ip)++;

	/* extract name */
	if (!colsym(ip)) {
		if (symbol[0] == 0)
			fatal_error("Syntax error!");
		return;
	}

	/* check end of line */
	check_eol(ip);

	/* increment procedure refcnt */
	if (stlook(1) == NULL)
		return;

	/* generate code */
	if (pass == LAST_PASS) {
		/* lookup proc table */
		if((ptr = proc_look())) {
			/* check banks */
			if ((newproc_opt == 0) && (bank == ptr->bank)) {
				/* different */
				value = ptr->org + 0xA000;
			} else {
				/* different */
				if (ptr->call) {
					value = ptr->call;
				} else {
					/* init */
					if (call_bank > max_bank) {
						/* don't increase ROM size until we need a trampoline */
						if (call_bank > bank_limit) {
							fatal_error("Not enough ROM space for proc trampolines!");
							return;
						}
						max_bank = call_bank;
					}

					/* new call */
					if (newproc_opt == 0) {
						/* check that the new trampoline won't overwrite existing data */
						if ((map[call_bank][(call_ptr + 17) & 0x1FFF] & 0x0F) != 0x0F)	{
							fatal_error("No more space in bank for .proc trampolines!");
							return;
						}

						/* install HuC trampolines at start of MPR4, map code into MPR5 */
						value = call_ptr;

						poke(call_ptr++, 0xA8);			// tay
						poke(call_ptr++, 0x43);			// tma #5
						poke(call_ptr++, 0x20);
						poke(call_ptr++, 0x48);			// pha
						poke(call_ptr++, 0xA9);			// lda #...
						poke(call_ptr++, ptr->bank + bank_base);
						poke(call_ptr++, 0x53);			// tam #5
						poke(call_ptr++, 0x20);
						poke(call_ptr++, 0x98);			// tya
						poke(call_ptr++, 0x20);			// jsr ...
						poke(call_ptr++, (ptr->org + 0xA000) & 255);
						poke(call_ptr++, (ptr->org + 0xA000) >> 8);
						poke(call_ptr++, 0xA8);			// tay
						poke(call_ptr++, 0x68);			// pla
						poke(call_ptr++, 0x53);			// tam #5
						poke(call_ptr++, 0x20);
						poke(call_ptr++, 0x98);			// tya
						poke(call_ptr++, 0x60);			// rts
					} else {
						/* check that the new trampoline won't overwrite existing data */
						if ((map[call_bank][(call_ptr - 10) & 0x1FFF] & 0x0F) != 0x0F)	{
							fatal_error("No more space in bank for .proc trampolines!");
							return;
						}

						/* install new trampolines at end of MPR7, map code into MPR6 */
						poke(--call_ptr, (ptr->org + 0xC000) >> 8);
						poke(--call_ptr, (ptr->org + 0xC000) & 255);
						poke(--call_ptr, 0x4C);			// jmp ...
						poke(--call_ptr, 0x40);
						poke(--call_ptr, 0x53);			// tam #6
						poke(--call_ptr, ptr->bank + bank_base);
						poke(--call_ptr, 0xA9);			// lda #...
						poke(--call_ptr, 0x48);			// pha
						poke(--call_ptr, 0x40);
						poke(--call_ptr, 0x43);			// tma #6

						value = call_ptr;
					}

					ptr->call = value;
				}
			}
		}
		else {
			/* lookup symbol table */
			if ((lablptr = stlook(0)) == NULL) {
				fatal_error("Undefined destination!");
				return;
			}

			/* get symbol value */
			value = lablptr->value;
		}

		/* opcode */
		putbyte(data_loccnt, 0x20);
		putword(data_loccnt+1, value);

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

	/* check if nesting procs/groups */
	if (proc_ptr) {
		if (optype == P_PGROUP) {
			fatal_error("Can not declare a group inside a proc/group!");
			return;
		}
		else {
			if (proc_ptr->type == P_PROC) {
				fatal_error("Can not nest procs!");
				return;
			}
		}
	}

	/* get proc name */
	if (lablptr) {
		strcpy(&symbol[1], &lablptr->name[1]);
		symbol[0] = strlen(&symbol[1]);

		/* hack to fix double-counted reference when label is */
		/* used in code and then defined on line before .proc */
		if (lablptr->refcnt)
			lablptr->refcnt--;
	}
	else {
		/* skip spaces */
		while (isspace(prlnbuf[*ip]))
			(*ip)++;

		/* extract name */
		if (!colsym(ip)) {
			if (symbol[0])
				return;
			if (optype == P_PROC) {
				fatal_error("Proc name is missing!");
				return;
			}

			/* default name */
			sprintf(&symbol[1], "__group_%i__", proc_nb + 1);
			symbol[0] = strlen(&symbol[1]);
		}

		/* lookup symbol table */
		if ((lablptr = stlook(1)) == NULL)
			return;
	}

	/* check symbol */
	if (symbol[1] == '.' || symbol[1] == '@') {
		fatal_error("Proc/group name can not be local!");
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
	if (proc_ptr->refcnt) {
		fatal_error("Proc/group multiply defined!");
		return;
	}

	/* increment proc ref counter */
	proc_ptr->refcnt++;

	/* backup current bank infos */
	bank_glabl[section][bank]  = glablptr;
	bank_loccnt[section][bank] = loccnt;
	bank_page[section][bank]   = page;
	proc_ptr->old_bank = bank;
	proc_nb++;

	/* set new bank infos */
	bank     = proc_ptr->bank;
	page     = (newproc_opt == 0) ? 5 : 6;
	loccnt   = proc_ptr->org;
	glablptr = lablptr;

	/* define label */
	labldef(loccnt, 1);

	/* output */
	if (pass == LAST_PASS) {
		loadlc((page << 13) + loccnt, 0);
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
	if (proc_ptr == NULL) {
		fatal_error("Unexpected ENDP/ENDPROCGROUP!");
		return;
	}
	if (optype != proc_ptr->type) {
		fatal_error("Unexpected ENDP/ENDPROCGROUP!");
		return;
	}

	/* check end of line */
	if (!check_eol(ip))
		return;

	/* record proc size */
	bank = proc_ptr->old_bank;
	proc_ptr->size = loccnt - proc_ptr->base;
	proc_ptr = proc_ptr->group;

	/* restore previous bank settings */
	if (proc_ptr == NULL) {
		page     = bank_page[section][bank];
		loccnt   = bank_loccnt[section][bank];
		glablptr = bank_glabl[section][bank];
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
	int i;
	int *bank_free = NULL;
	int new_bank = 0;

	if (proc_nb == 0)
		return;

	/* sort procedures by size (largest first) for better packing */
	proc_sortlist();

	/* init */
	bank_free = (int*) malloc(sizeof(int) * (bank_limit+1));
	if (!bank_free) {
		fatal_error("Not enough RAM to allocate banks!");
		return;
	}

	for (i = 0; i <= bank_limit; i++) {
		if (i > max_bank)
			bank_free[i] = 0x2000;
		else
			bank_free[i] = 0;
	}

	new_bank = max_bank + 1;

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
					for (check_bank = new_bank; check_bank <= max_bank; check_bank++)
					{
						if (bank_free[check_bank] >= proc_ptr->size)
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

							fatal_error("Not enough ROM space for procs!");

							for (i = new_bank; i < max_bank; i++) {
								printf("BANK %02X: %d bytes free\n", i, bank_free[i]);
								totfree += bank_free[i];
							}
							printf("Total free space in all banks %d\n", totfree);

							total = 0;
							proc_ptr = proc_first;
							while (proc_ptr) {
								printf("Proc: %s Bank: 0x%02X Size: %d %s\n",
									proc_ptr->name, proc_ptr->bank == PROC_BANK ? 0 : proc_ptr->bank, proc_ptr->size,
									proc_ptr->bank == PROC_BANK && proc_ptr == current ? " ** too big **" : proc_ptr->bank == PROC_BANK ? "** unassigned **" : "");
								if (proc_ptr->bank == PROC_BANK)
									total += proc_ptr->size;
								proc_ptr = proc_ptr->link;
							}
							printf("\nTotal bytes that didn't fit in ROM: %d\n", total);
							if (totfree > total && current)
								printf("Try segmenting the %s procedure into smaller chunks\n", current->name);
							else
								printf("There are %d bytes that won't fit into the currently available BANK space\n", total - totfree);
							errcnt++;
							return;
						}
						reloc_bank = max_bank;
					}
				}

				proc_ptr->bank = reloc_bank;
				proc_ptr->org = 0x2000 - bank_free[reloc_bank];

				bank_free[reloc_bank] -= proc_ptr->size;
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
		proc_ptr->refcnt = 0;
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
				sym->bank   =  proc_ptr->bank + bank_base;
				sym->value += (proc_ptr->org - proc_ptr->base);

				/* local symbols */
				if (sym->local) {
					local = sym->local;

					while (local) {
						proc_ptr = local->proc;

						/* remap addr */
						if (local->proc) {
							local->bank   =  proc_ptr->bank + bank_base;
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
		call_ptr  = 0xFFD0;
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
	ptr->refcnt = 0;
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
	rom[call_bank][(addr & 0x1FFF)] = data;
	map[call_bank][(addr & 0x1FFF)] = S_PROC + ((call_ptr & 0xE000) >> 8);
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
