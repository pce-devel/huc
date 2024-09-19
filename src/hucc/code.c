/*	File code.c: 2.2 (84/08/31,10:05:13) */
/*% cc -O -c %
 *
 */

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <time.h>
#include <string.h>

#include "defs.h"
#include "data.h"
#include "code.h"
#include "error.h"
#include "function.h"
#include "io.h"
#include "main.h"
#include "optimize.h"

/* locals */
int segment;

/* externs */
extern int arg_stack_flag;

/*
 *	print all assembler info before any code is generated
 *
 */
void gdata (void)
{
	if (segment == 1) {
		segment = 0;
		ol(".bss");
	}
}

void gtext (void)
{
	if (segment == 0) {
		segment = 1;
		ol(".code");
	}
}

void header (void)
{
	time_t today;

	outstr("; Small-C HuC6280 (1997-Nov-08)\n");
	outstr("; became HuC      (2000-Feb-22)\n");
	outstr("; became HuCC     (2024-May-01)\n");
	outstr(";\n");
	outstr("; This file generated by ");
	outstr(HUC_VERSION);
	outstr("\n");
	outstr("; on ");
	time(&today);
	outstr(ctime(&today));
	outstr(";\n");
	outstr("\n");
	outstr("HUC\t\t=\t1\n");
	outstr("HUCC\t\t=\t1\n");
	/* Reserve space for further global definitions. */
	output_globdef = ftell(output);
	outstr("                                                                           ");
	nl();
}

void asmdefines (void)
{
	outstr(asmdefs);
}

void inc_startup (void)
{
	if (startup_incl == 0) {
		startup_incl = 1;

		nl();
		outstr("\t\tinclude\t\"hucc.asm\"\n");
		outstr("\t\t.data\n");
		outstr("\t\t.bank\tDATA_BANK\n\n");
		gtext();
		nl();
	}
}

/*
 *	print pseudo-op  to define a byte
 *
 */
void defbyte (void)
{
	ot(".db\t\t");
}

/*
 *	print pseudo-op to define storage
 *
 */
void defstorage (void)
{
	ot(".ds\t\t");
}

/*
 *	print pseudo-op to define a word
 *
 */
void defword (void)
{
	ot(".dw\t\t");
}

/*
 *	output instructions
 *
 */
void out_ins (int code, int type, intptr_t data)
{
	INS tmp;

	memset(&tmp, 0, sizeof(INS));

	tmp.code = code;
	tmp.type = type;
	tmp.data = data;
	gen_ins(&tmp);
}

void out_ins_ex (int code, int type, intptr_t data, int imm_type, intptr_t imm_data)
{
	INS tmp;

	memset(&tmp, 0, sizeof(INS));

	tmp.code = code;
	tmp.type = type;
	tmp.data = data;
	tmp.imm_type = imm_type;
	tmp.imm_data = imm_data;
	gen_ins(&tmp);
}

void out_ins_sym (int code, int type, intptr_t data, SYMBOL *sym)
{
	INS tmp;

	memset(&tmp, 0, sizeof(INS));

	tmp.code = code;
	tmp.type = type;
	tmp.data = data;
	tmp.sym = sym;
	gen_ins(&tmp);
}

void gen_ins (INS *tmp)
{
	if (optimize)
		push_ins(tmp);
	else {
		if (arg_stack_flag)
			arg_push_ins(tmp);
		else
			gen_code(tmp);
	}
}

static void out_type (int type, intptr_t data)
{
	switch (type) {
	case T_VALUE:
		outdec((int)data);
		break;
	case T_LABEL:
		outlabel((int)data);
		break;
	case T_SYMBOL:
		outsymbol((SYMBOL *)data);
		break;
	case T_LITERAL:
		outstr((const char *)data);
		break;
	case T_STRING:
		outconst(litlab);
		outbyte('+');
		outdec((int)data);
		break;
	case T_BANK:
		outstr("BANK(");
		outstr((const char *)data);
		outstr(")");
		break;
	case T_VRAM:
		outstr("VRAM(");
		outstr((const char *)data);
		outstr(")");
		break;
	case T_PAL:
		outstr("PAL(");
		outstr((const char *)data);
		outstr(")");
		break;
	}
}

static void out_addr (int type, intptr_t data)
{
	switch (type) {
	case T_LABEL:
		outlabel((int)data);
		break;
	case T_SYMBOL:
		outsymbol((SYMBOL *)data);
		break;
	case T_LITERAL:
		outstr((const char *)data);
		break;
	case T_PTR:
		outstr("__ptr");
		break;
	case T_VALUE:
		outdec((int)data);
		break;
	case T_STACK:
		outstr("__stack");
		break;
	}
}

void dump_ins (INS *tmp)
{
	FILE *save = output;

	output = stdout;
	gen_code(tmp);
	output = save;
}

/*
 *	gen assembly code
 *
 */
void gen_code (INS *tmp)
{
	enum ICODE code;
	int type;
	intptr_t data;
	int imm_type;
	intptr_t imm_data;

	code = tmp->code;
	type = tmp->type;
	data = tmp->data;
	imm_type = tmp->imm_type;
	imm_data = tmp->imm_data;

	if (type == T_NOP)
		return;

	switch (code) {

	/* i-code that retires the primary register contents */

	case I_FENCE:
		ot("__fence");
		nl();
		break;

	/* i-code that declares a byte sized primary register */

	case I_SHORT:
		ot("__short");
		nl();
		break;

	/* i-codes for handling farptr */

	case I_FARPTR:
		ot("__farptr\t");

		switch (type) {
		case T_LABEL:
			outlabel((int)data);
			break;
		case T_SYMBOL:
			outsymbol((SYMBOL *)data);
			break;
		case T_STRING:
			outconst(litlab);
			outbyte('+');
			outdec((int)data);
			break;
		}
		outstr(", ");
		outstr(tmp->arg[0]);
		outstr(", ");
		outstr(tmp->arg[1]);
		nl();
		break;

	case I_FARPTR_I:
		ot("__farptr_i\t");
		outsymbol((SYMBOL *)data);
		outstr(", ");
		outstr(tmp->arg[0]);
		outstr(", ");
		outstr(tmp->arg[1]);
		nl();
		break;

	case I_FARPTR_GET:
		ot("__farptr_get\t");
		outstr(tmp->arg[0]);
		outstr(", ");
		outstr(tmp->arg[1]);
		nl();
		break;

	case I_FGETW:
		ot("__farptr_i\t");
		outsymbol((SYMBOL *)data);
		nl();
		ol("  jsr\t_farpeekw.fast");
		break;

	case I_FGETB:
		ot("__farptr_i\t");
		outsymbol((SYMBOL *)data);
		nl();
		ol("__fgetb");
		break;

	case I_FGETUB:
		ot("__farptr_i\t");
		outsymbol((SYMBOL *)data);
		nl();
		ol("__fgetub");
		break;

	/* i-codes for interrupts */

	case I_SEI:
		ol("sei");
		break;

	case I_CLI:
		ol("cli");
		break;

	/* i-codes for calling functions */

	case I_MACRO:
		/* because functions don't need to be pre-declared
		   in HuC we get a string and not a symbol */
		switch (type) {
		case T_LITERAL:
			ot(" ");
			prefix();
			outstr((const char *)data);
			if (imm_data) {
				outstr(".");
				outdec((int)imm_data);
			}
			break;
		}
		nl();
		break;

	case I_CALL:
		/* because functions don't need to be pre-declared
		   in HuC we get a string and not a symbol */
		switch (type) {
		case T_LITERAL:
			ot("  call\t\t");
			prefix();
			outstr((const char *)data);
			if (imm_data) {
				outstr(".");
				outdec((int)imm_data);
			}
			break;
		}
		nl();
		break;

	case I_CALLP:
		ol("__callp");
		break;

	case I_JSR:
		ot("  jsr\t\t");

		switch (type) {
		case T_SYMBOL:
			outsymbol((SYMBOL *)data);
			break;
		case T_LIB:
			outstr((const char *)data);
			break;
		}
		nl();
		break;

	/* i-codes for C functions and the C parameter stack */

	case I_ENTER:
		ot("__enter\t\t");
		outsymbol((SYMBOL *)data);
		nl();
		break;

	case I_LEAVE:
		ot("__leave\t\t");
		outdec((int)data);
		nl();
		break;

	case I_GETACC:
		ol("__getacc");
		break;

	case I_SAVESP:
		ol("__savesp");
		break;

	case I_LOADSP:
		ol("__loadsp");
		break;

	case I_MODSP:
		ot("__modsp");
		if (type == T_LITERAL) {
			outstr("_sym\t");
			outstr((const char *)data);
		}
		else {
			outstr("\t\t");
			outdec((int)data);
		}
		nl();
		break;

	case I_PUSH_WR:
		ol("__push.wr");
		break;

	case I_POP_WR:
		ol("__pop.wr");
		break;

	case I_SPUSH_WR:
		ol("__spush.wr");
		break;

	case I_SPOP_WR:
		ol("__spop.wr");
		break;

	case I_SPUSH_UR:
		ol("__spush.ur");
		break;

	case I_SPOP_UR:
		ol("__spop.ur");
		break;

	/* i-codes for handling boolean tests and branching */

	case I_SWITCH_WR:
		ot("__switch.wr\t");
		outlabel((int)data);
		nl();
		break;

	case I_SWITCH_UR:
		ot("__switch.ur\t");
		outlabel((int)data);
		nl();
		break;

	case I_CASE:
		ot("__case\t\t");
		if (type == T_VALUE)
			outdec((int)data);
		nl();
		break;

	case I_ENDCASE:
		ot("__endcase");
		nl();
		break;

	case I_LABEL:
		outlabel((int)data);
		col();
		break;

	case I_ALIAS:
		outlabel((int)data);
		ot(".alias\t\t");
		outlabel((int)imm_data);
		nl();
		break;

	case I_BRA:
		ot("__bra\t\t");
		outlabel((int)data);
		nl();
		break;

	case I_DEF:
		outstr((const char *)data);
		outstr(" .equ ");
		outdec((int)imm_data);
		nl();
		break;

	case I_CMP_WT:
		ot("__cmp.wt\t");

		switch (type) {
		case T_SYMBOL:
			outsymbol((SYMBOL *)data);
			break;
		case T_LIB:
			outstr((const char *)data);
			break;
		}
		nl();
		break;

	case I_CMP_UT:
		ot("__cmp.ut\t");

		switch (type) {
		case T_SYMBOL:
			outsymbol((SYMBOL *)data);
			break;
		case T_LIB:
			outstr((const char *)data);
			break;
		}
		nl();
		break;

	case X_EQU_WI:
		ot("__equ.wi\t");
		out_type(type, data);
		nl();
		break;

	case X_NEQ_WI:
		ot("__neq.wi\t");
		out_type(type, data);
		nl();
		break;

	case I_TST_WR:
		ol("__tst.wr");
		break;

	case I_NOT_WR:
		ol("__not.wr");
		break;

	case I_BFALSE:
		ot("__bfalse\t");
		outlabel((int)data);
		nl();
		nl();
		break;

	case I_BTRUE:
		ot("__btrue\t\t");
		outlabel((int)data);
		nl();
		nl();
		break;

	case X_NOT_WP:
		ot("__not.wp\t");
		out_addr(type, data);
		nl();
		break;

	case X_NOT_WM:
		ot("__not.wm\t");
		out_addr(type, data);
		nl();
		break;

	case X_NOT_WS:
		ot("__not.ws\t");
		outdec((int)data);
		nl();
		break;

	case X_NOT_WAR:
		ot("__not.war\t");
		out_addr(type, data);
		nl();
		break;

	case X_NOT_UP:
		ot("__not.up\t");
		out_addr(type, data);
		nl();
		break;

	case X_NOT_UM:
		ot("__not.um\t");
		out_addr(type, data);
		nl();
		break;

	case X_NOT_US:
		ot("__not.us\t");
		outdec((int)data);
		nl();
		break;

	case X_NOT_UAR:
		ot("__not.uar\t");
		out_addr(type, data);
		nl();
		break;

	case X_NOT_UAY:
		ot("__not.uay\t");
		out_addr(type, data);
		nl();
		break;

	case X_TST_WP:
		ot("__tst.wp\t");
		out_addr(type, data);
		nl();
		break;

	case X_TST_WM:
		ot("__tst.wm\t");
		out_addr(type, data);
		nl();
		break;

	case X_TST_WS:
		ot("__tst.ws\t");
		outdec((int)data);
		nl();
		break;

	case X_TST_WAR:
		ot("__tst.war\t");
		out_addr(type, data);
		nl();
		break;

	case X_TST_UP:
		ot("__tst.up\t");
		out_addr(type, data);
		nl();
		break;

	case X_TST_UM:
		ot("__tst.um\t");
		out_addr(type, data);
		nl();
		break;

	case X_TST_US:
		ot("__tst.us\t");
		outdec((int)data);
		nl();
		break;

	case X_TST_UAR:
		ot("__tst.uar\t");
		out_addr(type, data);
		nl();
		break;

	case X_TST_UAY:
		ot("__tst.uay\t");
		out_addr(type, data);
		nl();
		break;

	case X_NAND_WI:
		ot("__nand.wi\t");
		out_type(type, data);
		nl();
		break;

	case X_TAND_WI:
		ot("__tand.wi\t");
		out_type(type, data);
		nl();
		break;

	/* i-codes for loading the primary register */

	case I_LD_WI:
		ot("__ld.wi\t\t");
		out_type(type, data);
		nl();
		break;

	case X_LD_UIQ:
		ot("__ld.uiq\t");
		out_type(type, data);
		nl();
		break;

	case I_LEA_S:
		ot("__lea.s\t\t");
		outdec((int)data);
		outlocal(tmp->sym);
		nl();
		break;

	case I_LD_WM:
		ot("__ld.wm\t\t");
		out_addr(type, data);
		nl();
		break;

	case I_LD_BM:
		ot("__ld.bm\t\t");
		out_type(type, data);
		nl();
		break;

	case I_LD_UM:
		ot("__ld.um\t\t");
		out_type(type, data);
		nl();
		break;

	case I_LD_WMQ:
		ot("__ld.wmq\t");
		out_addr(type, data);
		nl();
		break;

	case I_LD_BMQ:
		ot("__ld.bmq\t");
		out_type(type, data);
		nl();
		break;

	case I_LD_UMQ:
		ot("__ld.umq\t");
		out_type(type, data);
		nl();
		break;

	case I_LDY_WMQ:
		ot("__ldy.wmq\t");
		out_addr(type, data);
		nl();
		break;

	case I_LDY_BMQ:
		ot("__ldy.bmq\t");
		out_type(type, data);
		nl();
		break;

	case I_LDY_UMQ:
		ot("__ldy.umq\t");
		out_type(type, data);
		nl();
		break;

	case I_LD_WP:
		ot("__ld.wp\t\t");
		out_addr(type, data);
		nl();
		break;

	case I_LD_BP:
		ot("__ld.bp\t\t");
		out_addr(type, data);
		nl();
		break;

	case I_LD_UP:
		ot("__ld.up\t\t");
		out_addr(type, data);
		nl();
		break;

	case X_LD_WAR:
		ot("__ld.war\t");
		out_type(type, data);
		nl();
		break;

	case X_LD_BAR:
		ot("__ld.bar\t");
		out_type(type, data);
		nl();
		break;

	case X_LD_UAR:
		ot("__ld.uar\t");
		out_type(type, data);
		nl();
		break;

	case X_LD_BAY:
		ot("__ld.bay\t");
		out_type(type, data);
		nl();
		break;

	case X_LD_UAY:
		ot("__ld.uay\t");
		out_type(type, data);
		nl();
		break;

	case X_LD_WS:
		ot("__ld.ws\t\t");
		outdec((int)data);
		outlocal(tmp->sym);
		nl();
		break;

	case X_LD_BS:
		ot("__ld.bs\t\t");
		outdec((int)data);
		outlocal(tmp->sym);
		nl();
		break;

	case X_LD_US:
		ot("__ld.us\t\t");
		outdec((int)data);
		outlocal(tmp->sym);
		nl();
		break;

	case X_LD_WSQ:
		ot("__ld.wsq\t");
		outdec((int)data);
		outlocal(tmp->sym);
		nl();
		break;

	case X_LD_BSQ:
		ot("__ld.bsq\t");
		outdec((int)data);
		outlocal(tmp->sym);
		nl();
		break;

	case X_LD_USQ:
		ot("__ld.usq\t");
		outdec((int)data);
		outlocal(tmp->sym);
		nl();
		break;

	case X_LDY_WSQ:
		ot("__ldy.wsq\t");
		outdec((int)data);
		outlocal(tmp->sym);
		nl();
		break;

	case X_LDY_BSQ:
		ot("__ldy.bsq\t");
		outdec((int)data);
		outlocal(tmp->sym);
		nl();
		break;

	case X_LDY_USQ:
		ot("__ldy.usq\t");
		outdec((int)data);
		outlocal(tmp->sym);
		nl();
		break;

	case X_LDP_WAR:
		ot("__ldp.war\t");
		out_type(type, data);
		nl();
		break;

	case X_LDP_BAR:
		ot("__ldp.bar\t");
		out_type(type, data);
		nl();
		break;

	case X_LDP_UAR:
		ot("__ldp.uar\t");
		out_type(type, data);
		nl();
		break;

	case X_LDP_BAY:
		ot("__ldp.bay\t");
		out_type(type, data);
		nl();
		break;

	case X_LDP_UAY:
		ot("__ldp.uay\t");
		out_type(type, data);
		nl();
		break;

//	case X_LDWA_S:
//		ot("__ld.was\t");
//		outsymbol((SYMBOL *)imm_data);
//		outstr(", ");
//		outdec((int)data);
//		outlocal(tmp->sym);
//		nl();
//		break;
//
//	case X_LDBA_S:
//		ot("__ld.bas\t");
//		outsymbol((SYMBOL *)imm_data);
//		outstr(", ");
//		outdec((int)data);
//		outlocal(tmp->sym);
//		nl();
//		break;
//
//	case X_LDUBA_S:
//		ot("__ld.uas\t");
//		outsymbol((SYMBOL *)imm_data);
//		outstr(", ");
//		outdec((int)data);
//		outlocal(tmp->sym);
//		nl();
//		break;

	/* i-codes for pre- and post- increment and decrement */

	case X_INCLD_WM:
		ot("__incld.wm\t");
		out_type(type, data);
		nl();
		break;

	case X_INCLD_BM:
		ot("__incld.bm\t");
		out_type(type, data);
		nl();
		break;

	case X_INCLD_UM:
		ot("__incld.um\t");
		out_type(type, data);
		nl();
		break;

	case X_DECLD_WM:
		ot("__decld.wm\t");
		out_type(type, data);
		nl();
		break;

	case X_DECLD_BM:
		ot("__decld.bm\t");
		out_type(type, data);
		nl();
		break;

	case X_DECLD_UM:
		ot("__decld.um\t");
		out_type(type, data);
		nl();
		break;

	case X_LDINC_WM:
		ot("__ldinc.wm\t");
		out_type(type, data);
		nl();
		break;

	case X_LDINC_BM:
		ot("__ldinc.bm\t");
		out_type(type, data);
		nl();
		break;

	case X_LDINC_UM:
		ot("__ldinc.um\t");
		out_type(type, data);
		nl();
		break;

	case X_LDDEC_WM:
		ot("__lddec.wm\t");
		out_type(type, data);
		nl();
		break;

	case X_LDDEC_BM:
		ot("__lddec.bm\t");
		out_type(type, data);
		nl();
		break;

	case X_LDDEC_UM:
		ot("__lddec.um\t");
		out_type(type, data);
		nl();
		break;

	case X_INC_WMQ:
		ot("__inc.wmq\t");
		out_type(type, data);
		nl();
		break;

	case X_INC_UMQ:
		ot("__inc.umq\t");
		out_type(type, data);
		nl();
		break;

	case X_DEC_WMQ:
		ot("__dec.wmq\t");
		out_type(type, data);
		nl();
		break;

	case X_DEC_UMQ:
		ot("__dec.umq\t");
		out_type(type, data);
		nl();
		break;

	case X_INCLD_WS:
		ot("__incld.ws\t");
		outdec((int)data);
		outlocal(tmp->sym);
		nl();
		break;

	case X_INCLD_BS:
		ot("__incld.bs\t");
		outdec((int)data);
		outlocal(tmp->sym);
		nl();
		break;

	case X_INCLD_US:
		ot("__incld.us\t");
		outdec((int)data);
		outlocal(tmp->sym);
		nl();
		break;

	case X_DECLD_WS:
		ot("__decld.ws\t");
		outdec((int)data);
		outlocal(tmp->sym);
		nl();
		break;

	case X_DECLD_BS:
		ot("__decld.bs\t");
		outdec((int)data);
		outlocal(tmp->sym);
		nl();
		break;

	case X_DECLD_US:
		ot("__decld.us\t");
		outdec((int)data);
		outlocal(tmp->sym);
		nl();
		break;

	case X_LDINC_WS:
		ot("__ldinc.ws\t");
		outdec((int)data);
		outlocal(tmp->sym);
		nl();
		break;

	case X_LDINC_BS:
		ot("__ldinc.bs\t");
		outdec((int)data);
		outlocal(tmp->sym);
		nl();
		break;

	case X_LDINC_US:
		ot("__ldinc.us\t");
		outdec((int)data);
		outlocal(tmp->sym);
		nl();
		break;

	case X_LDDEC_WS:
		ot("__lddec.ws\t");
		outdec((int)data);
		outlocal(tmp->sym);
		nl();
		break;

	case X_LDDEC_BS:
		ot("__lddec.bs\t");
		outdec((int)data);
		outlocal(tmp->sym);
		nl();
		break;

	case X_LDDEC_US:
		ot("__lddec.us\t");
		outdec((int)data);
		outlocal(tmp->sym);
		nl();
		break;

	case X_INC_WSQ:
		ot("__inc.wsq\t");
		outdec((int)data);
		outlocal(tmp->sym);
		nl();
		break;

	case X_INC_USQ:
		ot("__inc.usq\t");
		outdec((int)data);
		outlocal(tmp->sym);
		nl();
		break;

	case X_DEC_WSQ:
		ot("__dec.wsq\t");
		outdec((int)data);
		outlocal(tmp->sym);
		nl();
		break;

	case X_DEC_USQ:
		ot("__dec.usq\t");
		outdec((int)data);
		outlocal(tmp->sym);
		nl();
		break;

	case X_INCLD_WAR:
		ot("__incld.war\t");
		out_type(type, data);
		nl();
		break;

	case X_LDINC_WAR:
		ot("__ldinc.war\t");
		out_type(type, data);
		nl();
		break;

	case X_DECLD_WAR:
		ot("__decld.war\t");
		out_type(type, data);
		nl();
		break;

	case X_LDDEC_WAR:
		ot("__lddec.war\t");
		out_type(type, data);
		nl();
		break;

	case X_INCLD_BAR:
		ot("__incld.bar\t");
		out_type(type, data);
		nl();
		break;

	case X_INCLD_UAR:
		ot("__incld.uar\t");
		out_type(type, data);
		nl();
		break;

	case X_LDINC_BAR:
		ot("__ldinc.bar\t");
		out_type(type, data);
		nl();
		break;

	case X_LDINC_UAR:
		ot("__ldinc.uar\t");
		out_type(type, data);
		nl();
		break;

	case X_DECLD_BAR:
		ot("__decld.bar\t");
		out_type(type, data);
		nl();
		break;

	case X_DECLD_UAR:
		ot("__decld.uar\t");
		out_type(type, data);
		nl();
		break;

	case X_LDDEC_BAR:
		ot("__lddec.bar\t");
		out_type(type, data);
		nl();
		break;

	case X_LDDEC_UAR:
		ot("__lddec.uar\t");
		out_type(type, data);
		nl();
		break;

	case X_INCLD_BAY:
		ot("__incld.bay\t");
		out_type(type, data);
		nl();
		break;

	case X_INCLD_UAY:
		ot("__incld.uay\t");
		out_type(type, data);
		nl();
		break;

	case X_LDINC_BAY:
		ot("__ldinc.bay\t");
		out_type(type, data);
		nl();
		break;

	case X_LDINC_UAY:
		ot("__ldinc.uay\t");
		out_type(type, data);
		nl();
		break;

	case X_DECLD_BAY:
		ot("__decld.bay\t");
		out_type(type, data);
		nl();
		break;

	case X_DECLD_UAY:
		ot("__decld.uay\t");
		out_type(type, data);
		nl();
		break;

	case X_LDDEC_BAY:
		ot("__lddec.bar\t");
		out_type(type, data);
		nl();
		break;

	case X_LDDEC_UAY:
		ot("__lddec.uar\t");
		out_type(type, data);
		nl();
		break;

	case X_INC_WARQ:
		ot("__inc.warq\t");
		out_type(type, data);
		nl();
		break;

	case X_INC_UARQ:
		ot("__inc.uarq\t");
		out_type(type, data);
		nl();
		break;

	case X_INC_UAYQ:
		ot("__inc.uayq\t");
		out_type(type, data);
		nl();
		break;

	case X_DEC_WARQ:
		ot("__dec.warq\t");
		out_type(type, data);
		nl();
		break;

	case X_DEC_UARQ:
		ot("__dec.uarq\t");
		out_type(type, data);
		nl();
		break;

	case X_DEC_UAYQ:
		ot("__dec.uayq\t");
		out_type(type, data);
		nl();
		break;

	/* i-codes for saving the primary register */

	case I_ST_WMIQ:
		ot("__st.wmiq\t");
		out_type(imm_type, imm_data);
		outstr(", ");
		out_type(type, data);
		nl();
		break;

	case I_ST_UMIQ:
		ot("__st.umiq\t");
		out_type(imm_type, imm_data);
		outstr(", ");
		out_type(type, data);
		nl();
		break;

	case I_ST_WPI:
		ot("__st.wpi\t\t");
		outdec((int)data);
		nl();
		break;

	case I_ST_UPI:
		ot("__st.upi\t\t");
		outdec((int)data);
		nl();
		break;

	case I_ST_WM:
		ot("__st.wm\t\t");
		out_addr(type, data);
		nl();
		break;

	case I_ST_UM:
		ot("__st.um\t\t");
		out_addr(type, data);
		nl();
		break;

	case I_ST_WP:
		ot("__st.wp\t\t");
		out_addr(type, data);
		nl();
		break;

	case I_ST_UP:
		ot("__st.up\t\t");
		out_addr(type, data);
		nl();
		break;

	case I_ST_WPT:
		ol("__st.wpt");
		break;

	case I_ST_UPT:
		ol("__st.upt");
		break;

	case X_ST_WSIQ:
		ot("__st.wsiq\t");
		outdec((int)imm_data);
		outstr(", ");
		outdec((int)data);
		outlocal(tmp->sym);
		nl();
		break;

	case X_ST_USIQ:
		ot("__st.usiq\t");
		outdec((int)imm_data);
		outstr(", ");
		outdec((int)data);
		outlocal(tmp->sym);
		nl();
		break;

	case X_ST_WS:
		ot("__st.ws\t\t");
		outdec((int)data);
		outlocal(tmp->sym);
		nl();
		break;

	case X_ST_US:
		ot("__st.us\t\t");
		outdec((int)data);
		outlocal(tmp->sym);
		nl();
		break;

	case X_INDEX_WR:
		ot("__index.wr\t");
		out_type(type, data);
		nl();
		break;

	case X_INDEX_UR:
		ot("__index.ur\t");
		out_type(type, data);
		nl();
		break;

	case X_ST_WAT:
		ot("__st.wat\t");
		out_type(type, data);
		nl();
		break;

	case X_ST_UAT:
		ot("__st.uat\t");
		out_type(type, data);
		nl();
		break;

//	case X_STWA_S:
//		ot("__stwa_s\t");
//		outsymbol((SYMBOL *)imm_data);
//		outstr(", ");
//		outdec((int)data);
//		nl();
//		break;
//
//	case X_STBA_S:
//		ot("__stba_s\t");
//		outsymbol((SYMBOL *)imm_data);
//		outstr(", ");
//		outdec((int)data);
//		nl();
//		break;

	/* i-codes for extending a byte to a word */

	case I_EXT_BR:
		ol("__ext.br");
		break;

	case I_EXT_UR:
		ol("__ext.ur");
		break;

	/* i-codes for math with the primary register  */

	case I_COM_WR:
		ol("__com.wr");
		break;

	case I_NEG_WR:
		ol("__neg.wr");
		break;

	case I_ADD_WT:
		ol("__add.wt");
		break;

	case I_ADD_WI:
		ot("__add.wi\t");
		out_type(type, data);
		nl();
		break;

	case I_ADD_WM:
		ot("__add.wm\t");
		out_addr(type, data);
		nl();
		break;

	case I_ADD_UM:
		ot("__add.um\t");
		out_addr(type, data);
		nl();
		break;

	case X_ADD_WS:
		ot("__add.ws\t");
		outdec((int)data);
		outlocal(tmp->sym);
		nl();
		break;

	case X_ADD_US:
		ot("__add.us\t");
		outdec((int)data);
		outlocal(tmp->sym);
		nl();
		break;

	case I_ADDBI_P:
		ot("__addbi_p\t");
		out_type(type, data);
		nl();
		break;

	case I_SUB_WT:
		ol("__sub.wt");
		break;

	case I_SUB_WI:
		ot("__sub.wi\t");
		out_type(type, data);
		nl();
		break;

	case I_SUB_WM:
		ot("__sub.wm\t");
		out_addr(type, data);
		nl();
		break;

	case I_SUB_UM:
		ot("__sub.um\t");
		out_addr(type, data);
		nl();
		break;

	case I_ISUB_WI:
		ot("__isub.wi\t");
		out_type(type, data);
		nl();
		break;

	case I_AND_WT:
		ol("__and.wt");
		break;

	case I_AND_WI:
		ot("__and.wi\t");
		out_type(type, data);
		nl();
		break;

	case I_AND_UIQ:
		ot("__and.uiq\t");
		out_type(type, data);
		nl();
		break;

	case I_AND_WM:
		ot("__and.wm\t");
		out_addr(type, data);
		nl();
		break;

	case I_AND_UM:
		ot("__and.um\t");
		out_addr(type, data);
		nl();
		break;

	case I_EOR_WT:
		ol("__eor.wt");
		break;

	case I_EOR_WI:
		ot("__eor.wi\t");
		out_type(type, data);
		nl();
		break;

	case I_EOR_WM:
		ot("__eor.wm\t");
		out_addr(type, data);
		nl();
		break;

	case I_EOR_UM:
		ot("__eor.um\t");
		out_addr(type, data);
		nl();
		break;

	case I_OR_WT:
		ol("__or.wt");
		break;

	case I_OR_WI:
		ot("__or.wi\t\t");
		out_type(type, data);
		nl();
		break;

	case I_OR_WM:
		ot("__or.wm\t");
		out_addr(type, data);
		nl();
		break;

	case I_OR_UM:
		ot("__or.um\t");
		out_addr(type, data);
		nl();
		break;

	case I_ASL_WT:
		ol("__asl.wt");
		break;

	case I_ASL_WI:
		ot("__asl.wi\t");
		out_type(type, data);
		nl();
		break;

	case I_ASL_WR:
		ol("__asl.wr");
		break;

	case I_ASR_WI:
		ot("__asr.wi\t");
		out_type(type, data);
		nl();
		break;

	case I_LSR_WI:
		ot("__lsr.wi\t");
		out_type(type, data);
		nl();
		break;

	case I_LSR_UIQ:
		ot("__lsr.uiq\t");
		out_type(type, data);
		nl();
		break;

	case I_MUL_WI:
		ot("__mul.wi\t");
		outdec((int)data);
		nl();
		break;

	default:
		gen_asm(tmp);
		break;
	}
}

/* ----
 * gen_asm()
 * ----
 * generate optimizer asm code
 *
 */
void gen_asm (INS *inst)
{
//	int type = inst->type;
//	intptr_t data = inst->data;

	/* i-codes for 32-bit longs */

	switch (inst->code) {

	case X_LDD_I:
		ot("__ldd_i\t\t");
		outdec((int)inst->data);
		outstr(", ");
		prefix();
		outstr(inst->arg[0]);
		outstr(", ");
		prefix();
		outstr(inst->arg[1]);
		nl();
		break;

	case X_LDD_W:
		ot("__ldd_w\t\t");
		outsymbol((SYMBOL *)inst->data);
		outstr(", ");
		prefix();
		outstr(inst->arg[0]);
		outstr(", ");
		prefix();
		outstr(inst->arg[1]);
		nl();
		break;

	case X_LDD_B:
		ot("__ldd_b\t\t");
		outsymbol((SYMBOL *)inst->data);
		outstr(", ");
		prefix();
		outstr(inst->arg[0]);
		outstr(", ");
		prefix();
		outstr(inst->arg[1]);
		nl();
		break;

	case X_LDD_S_W:
		ot("__ldd_s_w\t");
		outdec((int)inst->data);
		outstr(", ");
		prefix();
		outstr(inst->arg[0]);
		outstr(", ");
		prefix();
		outstr(inst->arg[1]);
		nl();
		break;

	case X_LDD_S_B:
		ot("__ldd_s_b\t");
		outdec((int)inst->data);
		outstr(", ");
		prefix();
		outstr(inst->arg[0]);
		outstr(", ");
		prefix();
		outstr(inst->arg[1]);
		nl();
		break;

	default:
		error("internal error: invalid instruction");
		break;
	}
}
