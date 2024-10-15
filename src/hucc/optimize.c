/*	File opt.c: 2.1 (83/03/20,16:02:09) */
/*% cc -O -c %
 *
 */

// #define DEBUG_OPTIMIZER

#define OPT_ARRAY_RD	1
#define OPT_ARRAY_WR	1

#ifdef DEBUG_OPTIMIZER
#define ODEBUG(...) printf( __VA_ARGS__ )
#else
#define ODEBUG(...)
#endif

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "defs.h"
#include "data.h"
#include "sym.h"
#include "code.h"
#include "function.h"
#include "optimize.h"
#include "io.h"
#include "error.h"

#ifdef _MSC_VER
 #include <intrin.h>
 #define __builtin_popcount __popcnt
 #define __builtin_ctz _tzcnt_u32
#endif

#ifdef __GNUC__
 /* GCC doesn't like "strcmp((char *)p[0]->ins_data,"! */
 #pragma GCC diagnostic ignored "-Wstringop-overread"
#endif

/* flag information for each of the i-code instructions */
/*
 * N.B. this table MUST be kept updated and in the same order as the i-code
 * enum list in defs.h
 */
unsigned char icode_flags[] = {
	0,

	// i-code that retires the primary register contents

	/* I_FENCE              */	0,

	/* i-code that declares a byte sized primary register */

	/* I_SHORT              */	0,

	// i-codes for handling farptr

	/* I_FARPTR             */	0,
	/* I_FARPTR_I           */	0,
	/* I_FARPTR_GET         */	0,
	/* I_FGETW              */	0,
	/* I_FGETB              */	0,
	/* I_FGETUB             */	0,

	// i-codes for interrupts

	/* I_SEI                */	0,
	/* I_CLI                */	0,

	// i-codes for calling functions

	/* I_MACRO              */	0,
	/* I_CALL               */	0,
	/* I_CALLP              */	0,

	// i-codes for C functions and the C parameter stack

	/* I_ENTER              */	0,
	/* I_LEAVE              */	0,
	/* I_GETACC             */	0,
	/* I_SAVESP             */	0,
	/* I_LOADSP             */	0,
	/* I_MODSP              */	0,
	/* I_PUSH_WR            */	0,
	/* I_POP_WR             */	0,
	/* I_SPUSH_WR           */	0,
	/* I_SPUSH_UR           */	0,
	/* I_SPOP_WR            */	0,
	/* I_SPOP_UR            */	0,

	// i-codes for handling boolean tests and branching

	/* I_SWITCH_WR          */	0,
	/* I_SWITCH_UR          */	0,
	/* I_CASE               */	0,
	/* I_ENDCASE            */	0,
	/* I_LABEL              */	0,
	/* I_ALIAS              */	0,
	/* I_BRA                */	0,
	/* I_BFALSE             */	0,
	/* I_BTRUE              */	0,
	/* I_DEF                */	0,

	/* I_CMP_WT             */	0,
	/* X_CMP_WI             */	0,
	/* X_CMP_WM             */	0,
	/* X_CMP_WS             */	IS_SPREL,

	/* X_CMP_UIQ            */	IS_UBYTE,
	/* X_CMP_UMQ            */	IS_UBYTE,
	/* X_CMP_USQ            */	IS_SPREL + IS_UBYTE,

	/* I_NOT_WR             */	0,
	/* X_NOT_WP             */	0,
	/* X_NOT_WM             */	0,
	/* X_NOT_WS             */	IS_SPREL,
	/* X_NOT_WAR            */	0,
	/* X_NOT_UP             */	0,
	/* X_NOT_UM             */	0,
	/* X_NOT_US             */	IS_SPREL,
	/* X_NOT_UAR            */	0,
	/* X_NOT_UAY            */	0,

	/* I_TST_WR             */	0,
	/* X_TST_WP             */	0,
	/* X_TST_WM             */	0,
	/* X_TST_WS             */	IS_SPREL,
	/* X_TST_WAR            */	0,
	/* X_TST_UP             */	0,
	/* X_TST_UM             */	0,
	/* X_TST_US             */	IS_SPREL,
	/* X_TST_UAR            */	0,
	/* X_TST_UAY            */	0,

	/* X_NAND_WI            */	0,
	/* X_TAND_WI            */	0,

	/* I_BOOLEAN            */	0,
	/* X_NOT_CF             */	0,

	// i-codes for loading the primary register

	/* I_LD_WI              */	0,
	/* X_LD_UIQ             */	0,
	/* I_LEA_S              */	IS_SPREL,

	/* I_LD_WM              */	0,
	/* I_LD_BM              */	0,
	/* I_LD_UM              */	IS_UBYTE,

	/* I_LD_WMQ             */	0,
	/* I_LD_BMQ             */	0,
	/* I_LD_UMQ             */	IS_UBYTE,

	/* I_LDY_WMQ            */	0,
	/* I_LDY_BMQ            */	0,
	/* I_LDY_UMQ            */	IS_UBYTE,

	/* I_LD_WP              */	0,
	/* I_LD_BP              */	0,
	/* I_LD_UP              */	IS_UBYTE,

	/* X_LD_WAR             */	0,
	/* X_LD_BAR             */	0,
	/* X_LD_UAR             */	IS_UBYTE,

	/* X_LD_BAY             */	0,
	/* X_LD_UAY             */	IS_UBYTE,

	/* X_LD_WS              */	IS_SPREL,
	/* X_LD_BS              */	IS_SPREL,
	/* X_LD_US              */	IS_SPREL + IS_UBYTE,

	/* X_LD_WSQ             */	IS_SPREL,
	/* X_LD_BSQ             */	IS_SPREL,
	/* X_LD_USQ             */	IS_SPREL + IS_UBYTE,

	/* X_LDY_WSQ            */	IS_SPREL,
	/* X_LDY_BSQ            */	IS_SPREL,
	/* X_LDY_USQ            */	IS_SPREL + IS_UBYTE,

	/* X_LDP_WAR            */	0,
	/* X_LDP_BAR            */	0,
	/* X_LDP_UAR            */	IS_UBYTE,

	/* X_LDP_BAY            */	0,
	/* X_LDP_UAY            */	IS_UBYTE,

	// i-codes for pre- and post- increment and decrement

	/* X_INCLD_WM           */	0,
	/* X_INCLD_BM           */	0,
	/* X_INCLD_UM           */	IS_UBYTE,

	/* X_DECLD_WM           */	0,
	/* X_DECLD_BM           */	0,
	/* X_DECLD_UM           */	IS_UBYTE,

	/* X_LDINC_WM           */	0,
	/* X_LDINC_BM           */	0,
	/* X_LDINC_UM           */	IS_UBYTE,

	/* X_LDDEC_WM           */	0,
	/* X_LDDEC_BM           */	0,
	/* X_LDDEC_UM           */	IS_UBYTE,

	/* X_INC_WMQ            */	0,
	/* X_INC_UMQ            */	0,

	/* X_DEC_WMQ            */	0,
	/* X_DEC_UMQ            */	IS_UBYTE,

	/* X_INCLD_WS           */	IS_SPREL,
	/* X_INCLD_BS           */	IS_SPREL,
	/* X_INCLD_US           */	IS_SPREL + IS_UBYTE,

	/* X_DECLD_WS           */	IS_SPREL,
	/* X_DECLD_BS           */	IS_SPREL,
	/* X_DECLD_US           */	IS_SPREL + IS_UBYTE,

	/* X_LDINC_WS           */	IS_SPREL,
	/* X_LDINC_BS           */	IS_SPREL,
	/* X_LDINC_US           */	IS_SPREL + IS_UBYTE,

	/* X_LDDEC_WS           */	IS_SPREL,
	/* X_LDDEC_BS           */	IS_SPREL,
	/* X_LDDEC_US           */	IS_SPREL + IS_UBYTE,

	/* X_INC_WSQ            */	IS_SPREL,
	/* X_INC_USQ            */	IS_SPREL,

	/* X_DEC_WSQ            */	IS_SPREL,
	/* X_DEC_USQ            */	IS_SPREL,

	/* X_INCLD_WAR          */	0,
	/* X_LDINC_WAR          */	0,
	/* X_DECLD_WAR          */	0,
	/* X_LDDEC_WAR          */	0,

	/* X_INCLD_BAR          */	0,
	/* X_INCLD_UAR          */	IS_UBYTE,
	/* X_LDINC_BAR          */	0,
	/* X_LDINC_UAR          */	IS_UBYTE,
	/* X_DECLD_BAR          */	0,
	/* X_DECLD_UAR          */	IS_UBYTE,
	/* X_LDDEC_BAR          */	0,
	/* X_LDDEC_UAR          */	IS_UBYTE,

	/* X_INCLD_BAY          */	0,
	/* X_INCLD_UAY          */	IS_UBYTE,
	/* X_LDINC_BAY          */	0,
	/* X_LDINC_UAY          */	IS_UBYTE,
	/* X_DECLD_BAY          */	0,
	/* X_DECLD_UAY          */	IS_UBYTE,
	/* X_LDDEC_BAY          */	0,
	/* X_LDDEC_UAY          */	IS_UBYTE,

	/* X_INC_WARQ           */	0,
	/* X_INC_UARQ           */	0,
	/* X_INC_UAYQ           */	0,

	/* X_DEC_WARQ           */	0,
	/* X_DEC_UARQ           */	0,
	/* X_DEC_UAYQ           */	0,

	// i-codes for saving the primary register

	/* I_ST_WMIQ            */	0,
	/* I_ST_UMIQ            */	0,
	/* I_ST_WPI             */	0,
	/* I_ST_UPI             */	0,
	/* I_ST_WM              */	0,
	/* I_ST_UM              */	0,
	/* I_ST_WP              */	0,
	/* I_ST_UP              */	0,
	/* I_ST_WPT             */	0,
	/* I_ST_UPT             */	0,
	/* X_ST_WSIQ            */	IS_SPREL,
	/* X_ST_USIQ            */	IS_SPREL,
	/* X_ST_WS              */	IS_SPREL,
	/* X_ST_US              */	IS_SPREL,

	/* X_INDEX_WR           */	0,
	/* X_INDEX_UR           */	0,

	/* X_ST_WAT             */	0,
	/* X_ST_UAT             */	0,

	// i-codes for extending the primary register

	/* I_EXT_BR             */	0,
	/* I_EXT_UR             */	0,

	// i-codes for math with the primary register

	/* I_COM_WR             */	0,
	/* I_NEG_WR             */	0,

	/* I_ADD_WT             */	0,
	/* I_ADD_WI             */	0,
	/* I_ADD_WM             */	0,
	/* I_ADD_UM             */	0,
	/* X_ADD_WS             */	IS_SPREL,
	/* X_ADD_US             */	IS_SPREL,

	/* I_SUB_WT             */	0,
	/* I_SUB_WI             */	0,
	/* I_SUB_WM             */	0,
	/* I_SUB_UM             */	0,

	/* I_ISUB_WI            */	0,

	/* I_AND_WT             */	0,
	/* I_AND_WI             */	0,
	/* I_AND_UIQ            */	IS_UBYTE,
	/* I_AND_WM             */	0,
	/* I_AND_UM             */	0,

	/* I_EOR_WT             */	0,
	/* I_EOR_WI             */	0,
	/* I_EOR_WM             */	0,
	/* I_EOR_UM             */	0,

	/* I_OR_WT              */	0,
	/* I_OR_WI              */	0,
	/* I_OR_WM              */	0,
	/* I_OR_UM              */	0,

	/* I_ASL_WT             */	0,
	/* I_ASL_WI             */	0,
	/* I_ASL_WR             */	0,

	/* I_ASR_WT             */	0,
	/* I_ASR_WI             */	0,

	/* I_LSR_WT             */	0,
	/* I_LSR_WI             */	0,
	/* I_LSR_UIQ            */	IS_UBYTE,

	/* I_MUL_WT             */	0,
	/* I_MUL_WI             */	0,

	/* I_SDIV_WT            */	0,
	/* I_SDIV_WI            */	0,

	/* I_UDIV_WT            */	0,
	/* I_UDIV_WI            */	0,
	/* I_UDIV_UI            */	IS_UBYTE,

	/* I_SMOD_WT            */	0,
	/* I_SMOD_WI            */	0,

	/* I_UMOD_WT            */	0,
	/* I_UMOD_WI            */	0,
	/* I_UMOD_UI            */	IS_UBYTE,

	/* I_DOUBLE             */	0,

	// i-codes for 32-bit longs

	/* X_LDD_I              */	0,
	/* X_LDD_W              */	0,
	/* X_LDD_B              */	0,
	/* X_LDD_S_W            */	IS_SPREL,
	/* X_LDD_S_B            */	IS_SPREL
};

/* invert comparison operation */
int compare2not [] = {
	CMP_NEQ,	// CMP_EQU
	CMP_EQU,	// CMP_NEQ
	CMP_SGE,	// CMP_SLT
	CMP_SGT,	// CMP_SLE
	CMP_SLE,	// CMP_SGT
	CMP_SLT,	// CMP_SGE
	CMP_UGE,	// CMP_ULT
	CMP_UGT,	// CMP_ULE
	CMP_ULE,	// CMP_UGT
	CMP_ULT		// CMP_UGE
};

/* optimize comparison between unsigned chars */
/* C promotes an unsigned char to a signed int for comparison */
int compare2uchar [] = {
	CMP_EQU,	// CMP_EQU
	CMP_NEQ,	// CMP_NEQ
	CMP_ULT,	// CMP_SLT
	CMP_ULE,	// CMP_SLE
	CMP_UGT,	// CMP_SGT
	CMP_UGE,	// CMP_SGE
	CMP_ULT,	// CMP_ULT
	CMP_ULE,	// CMP_ULE
	CMP_UGT,	// CMP_UGT
	CMP_UGE		// CMP_UGE
};

/* defines */
#define Q_SIZE		16

/* locals */
static INS q_ins[Q_SIZE];
static int q_rd;
static int q_wr;
static int q_nb;

/* externs */
extern int arg_stack_flag;


int cmp_operands (INS *p1, INS *p2)
{
#ifdef DEBUG_OPTIMIZER
	printf("cmp"); dump_ins(p1); dump_ins(p2);
#endif
	if (p1->ins_type != p2->ins_type)
		return (0);

	if (p1->ins_type == T_SYMBOL) {
		if (strcmp(((SYMBOL *)p1->ins_data)->name, ((SYMBOL *)p2->ins_data)->name) != 0)
			return (0);
	}
	else {
		if (p1->ins_data != p2->ins_data)
			return (0);
	}
	return (1);
}

inline bool is_sprel (INS *i)
{
	return (icode_flags[i->ins_code] & IS_SPREL);
}

inline bool is_ubyte (INS *i)
{
	return (icode_flags[i->ins_code] & IS_UBYTE);
}

inline bool is_small_array (SYMBOL *sym)
{
	return (sym->identity == ARRAY && sym->alloc_size > 0 && sym->alloc_size <= 256);
}

/* ----
 * push_ins()
 * ----
 *
 */
void push_ins (INS *ins)
{
#ifdef DEBUG_OPTIMIZER
	printf("\npush "); dump_ins(ins);
#endif
	/* check queue size */
	if (q_nb == Q_SIZE) {
		/* queue is full - flush the last instruction */
		if (arg_stack_flag)
			arg_push_ins(&q_ins[q_rd]);
		else
			gen_code(&q_ins[q_rd]);

		/* advance queue read pointer */
		q_rd++;
		q_nb--;

		if (q_rd == Q_SIZE)
			q_rd = 0;
	}

	/* push new instruction */
	q_wr++;
	q_nb++;

	if (q_wr == Q_SIZE)
		q_wr = 0;

	q_ins[q_wr] = *ins;

	/* optimization level 1 - simple peephole optimizer,
	 * replace known instruction patterns by highly
	 * optimized asm code
	 */
	if (optimize >= 1) {
		INS *p[Q_SIZE];
		int i, j;
		int nb;

lv1_loop:


		/* precalculate pointers to instructions */
		if (q_nb > 10)
			nb = 10;
		else
			nb = q_nb;
#ifdef DEBUG_OPTIMIZER
		printf("\nlv1_loop:\n");
#endif
		i = 0;
		j = q_wr;
		while (i < nb) {
			/* save pointer */
			p[i] = &q_ins[j];
#ifdef DEBUG_OPTIMIZER
			printf("%d ", i); dump_ins(p[i]);
#endif
			/* next */
			j -= 1;
			if (j < 0)
				j += Q_SIZE;

			++i;
		}

		/* LEVEL 1 - FUN STUFF STARTS HERE */
		nb = 0;

		/* ********************************************************* */
		/* ********************************************************* */

		/* first check for I_FENCE, and remove it ASAP */
		if (q_nb >= 1 && p[0]->ins_code == I_FENCE) {
			/* remove I_FENCE after it has been checked */
			nb = 1;

			/*
			 *  __ld.wi		i	-->	__st.{w/u}miq	symbol, i
			 *  __st.{w/u}m		symbol
			 *  __fence
			 *
			 *  __ld.wi		i	-->	__st.{w/u}siq	n, 1
			 *  __st.{w/u}s		n
			 *  __fence
			 */
			if
			((q_nb >= 3) &&
			 (p[1]->ins_code == I_ST_WM ||
			  p[1]->ins_code == I_ST_UM ||
			  p[1]->ins_code == X_ST_WS ||
			  p[1]->ins_code == X_ST_US) &&
			 (p[2]->ins_code == I_LD_WI) &&
			 (p[2]->ins_type == T_VALUE)
			) {
				/* replace code */
				intptr_t data = p[2]->ins_data;
				*p[2] = *p[1];
				switch (p[1]->ins_code) {
				case I_ST_WM: p[2]->ins_code = I_ST_WMIQ; break;
				case I_ST_UM: p[2]->ins_code = I_ST_UMIQ; break;
				case X_ST_WS: p[2]->ins_code = X_ST_WSIQ; break;
				case X_ST_US: p[2]->ins_code = X_ST_USIQ; break;
				default: abort();
				}
				p[2]->imm_type = T_VALUE;
				p[2]->imm_data = data;
				nb = 2;
			}

			/*
			 *  __getacc			-->
			 *  __fence
			 *
			 *  __add.wi / __sub.wi		-->
			 *  __fence
			 */
			else if
			((q_nb >= 2) &&
			 (p[1]->ins_code == I_ADD_WI ||
			  p[1]->ins_code == I_SUB_WI ||
			  p[1]->ins_code == I_GETACC)
			) {
				nb = 2;
			}

			/*
			 *  __ldinc.wm / __incld.wm	-->	__inc.wm
			 *  __fence
			 *
			 *  __lddec.wm / __decld.wm	-->	__dec.wm
			 *  __fence
			 *
			 *  unused pre-increment or post-increment value
			 *  unused pre-decrement or post-decrement value
			 */
			else if
			((q_nb >= 2) &&
			 (p[1]->ins_code == X_INCLD_WM ||
			  p[1]->ins_code == X_INCLD_BM ||
			  p[1]->ins_code == X_INCLD_UM ||
			  p[1]->ins_code == X_LDINC_WM ||
			  p[1]->ins_code == X_LDINC_BM ||
			  p[1]->ins_code == X_LDINC_UM ||
			  p[1]->ins_code == X_DECLD_WM ||
			  p[1]->ins_code == X_DECLD_BM ||
			  p[1]->ins_code == X_DECLD_UM ||
			  p[1]->ins_code == X_LDDEC_WM ||
			  p[1]->ins_code == X_LDDEC_BM ||
			  p[1]->ins_code == X_LDDEC_UM ||
			  p[1]->ins_code == X_INCLD_WS ||
			  p[1]->ins_code == X_INCLD_BS ||
			  p[1]->ins_code == X_INCLD_US ||
			  p[1]->ins_code == X_LDINC_WS ||
			  p[1]->ins_code == X_LDINC_BS ||
			  p[1]->ins_code == X_LDINC_US ||
			  p[1]->ins_code == X_DECLD_WS ||
			  p[1]->ins_code == X_DECLD_BS ||
			  p[1]->ins_code == X_DECLD_US ||
			  p[1]->ins_code == X_LDDEC_WS ||
			  p[1]->ins_code == X_LDDEC_BS ||
			  p[1]->ins_code == X_LDDEC_US ||
			  p[1]->ins_code == X_INCLD_WAR ||
			  p[1]->ins_code == X_LDINC_WAR ||
			  p[1]->ins_code == X_DECLD_WAR ||
			  p[1]->ins_code == X_LDDEC_WAR ||
			  p[1]->ins_code == X_INCLD_BAR ||
			  p[1]->ins_code == X_INCLD_UAR ||
			  p[1]->ins_code == X_LDINC_BAR ||
			  p[1]->ins_code == X_LDINC_UAR ||
			  p[1]->ins_code == X_DECLD_BAR ||
			  p[1]->ins_code == X_DECLD_UAR ||
			  p[1]->ins_code == X_LDDEC_BAR ||
			  p[1]->ins_code == X_LDDEC_UAR ||
			  p[1]->ins_code == X_INCLD_BAY ||
			  p[1]->ins_code == X_INCLD_UAY ||
			  p[1]->ins_code == X_LDINC_BAY ||
			  p[1]->ins_code == X_LDINC_UAY ||
			  p[1]->ins_code == X_DECLD_BAY ||
			  p[1]->ins_code == X_DECLD_UAY ||
			  p[1]->ins_code == X_LDDEC_BAY ||
			  p[1]->ins_code == X_LDDEC_UAY)
			) {
				/* replace code */
				switch (p[1]->ins_code) {
				case X_INCLD_WM:
				case X_LDINC_WM: p[1]->ins_code = X_INC_WMQ; break;
				case X_INCLD_BM:
				case X_INCLD_UM:
				case X_LDINC_BM:
				case X_LDINC_UM: p[1]->ins_code = X_INC_UMQ; break;
				case X_DECLD_WM:
				case X_LDDEC_WM: p[1]->ins_code = X_DEC_WMQ; break;
				case X_DECLD_BM:
				case X_DECLD_UM:
				case X_LDDEC_BM:
				case X_LDDEC_UM: p[1]->ins_code = X_DEC_UMQ; break;
				case X_INCLD_WS:
				case X_LDINC_WS: p[1]->ins_code = X_INC_WSQ; break;
				case X_INCLD_BS:
				case X_INCLD_US:
				case X_LDINC_BS:
				case X_LDINC_US: p[1]->ins_code = X_INC_USQ; break;
				case X_DECLD_WS:
				case X_LDDEC_WS: p[1]->ins_code = X_DEC_WSQ; break;
				case X_DECLD_BS:
				case X_DECLD_US:
				case X_LDDEC_BS:
				case X_LDDEC_US: p[1]->ins_code = X_DEC_USQ; break;
				case X_INCLD_WAR:
				case X_LDINC_WAR: p[1]->ins_code = X_INC_WARQ; break;
				case X_DECLD_WAR:
				case X_LDDEC_WAR: p[1]->ins_code = X_DEC_WARQ; break;
				case X_INCLD_BAR:
				case X_INCLD_UAR:
				case X_LDINC_BAR:
				case X_LDINC_UAR: p[1]->ins_code = X_INC_UARQ; break;
				case X_DECLD_BAR:
				case X_DECLD_UAR:
				case X_LDDEC_BAR:
				case X_LDDEC_UAR: p[1]->ins_code = X_DEC_UARQ; break;
				case X_INCLD_BAY:
				case X_INCLD_UAY:
				case X_LDINC_BAY:
				case X_LDINC_UAY: p[1]->ins_code = X_INC_UAYQ; break;
				case X_DECLD_BAY:
				case X_DECLD_UAY:
				case X_LDDEC_BAY:
				case X_LDDEC_UAY: p[1]->ins_code = X_DEC_UAYQ; break;
				default: abort();
				}
				nb = 1;
			}

			/* flush queue */
			if (nb) {
				q_wr -= nb;
				q_nb -= nb;
				nb = 0;

				if (q_wr < 0)
					q_wr += Q_SIZE;

				/* loop */
				goto lv1_loop;
			}
		}

		/* ********************************************************* */
		/* ********************************************************* */

		/* then check for I_SHORT, and remove it ASAP */
		if (q_nb >= 1 && p[0]->ins_code == I_SHORT) {
			/* remove I_SHORT after it has been checked */
			nb = 1;

			/*
			 *  __ld.wi		i	-->	__ld.uiq	i
			 *  __short
			 *
			 *  __ld.{w/b/u}m	symbol	-->	__ld.{w/b/u}mq	i
			 *  __short
			 *
			 *  __ld.{w/b/u}s	symbol	-->	__ld.{w/b/u}sq	i
			 *  __short
			 */
			if
			((q_nb >= 2) &&
			 (p[1]->ins_code == I_LD_WI ||
			  p[1]->ins_code == I_LD_WM ||
			  p[1]->ins_code == I_LD_BM ||
			  p[1]->ins_code == I_LD_UM ||
			  p[1]->ins_code == X_LD_WS ||
			  p[1]->ins_code == X_LD_BS ||
			  p[1]->ins_code == X_LD_US)
			) {
				switch (p[1]->ins_code) {
				case I_LD_WI: p[1]->ins_code = X_LD_UIQ; break;
				case I_LD_WM: p[1]->ins_code = I_LD_WMQ; break;
				case I_LD_BM: p[1]->ins_code = I_LD_BMQ; break;
				case I_LD_UM: p[1]->ins_code = I_LD_UMQ; break;
				case X_LD_WS: p[1]->ins_code = X_LD_WSQ; break;
				case X_LD_BS: p[1]->ins_code = X_LD_BSQ; break;
				case X_LD_US: p[1]->ins_code = X_LD_USQ; break;
				default: abort();
				}
				nb = 1;
			}

			/* flush queue */
			if (nb) {
				q_wr -= nb;
				q_nb -= nb;
				nb = 0;

				if (q_wr < 0)
					q_wr += Q_SIZE;

				/* loop */
				goto lv1_loop;
			}
		}

		/* ********************************************************* */
		/* then evaluate constant expressions */
		/* ********************************************************* */

		if (q_nb >= 3) {
			/*
			 *  __push.wr			-->	__add.wi	i
			 *  __ld.wi		i
			 *  __add.wt
			 *
			 *  __push.wr			-->	__sub.wi	i
			 *  __ld.wi		i
			 *  __sub.wt
			 *
			 *  __push.wr			-->	__and.wi	i
			 *  __ld.wi		i
			 *  __and.wt
			 *
			 *  __push.wr			-->	__eor.wi	i
			 *  __ld.wi		i
			 *  __eor.wt
			 *
			 *  __push.wr			-->	__or.wi		i
			 *  __ld.wi		i
			 *  __or.wt
			 *
			 *  __push.wr			-->	__asl.wi	i
			 *  __ld.wi		i
			 *  __asl.wt
			 *
			 *  __push.wr			-->	__asr.wi	i
			 *  __ld.wi		i
			 *  __asr.wt
			 *
			 *  __push.wr			-->	__lsr.wi	i
			 *  __ld.wi		i
			 *  __lsr.wt
			 *
			 *  __push.wr			-->	__mul.wi	i
			 *  __ld.wi		i
			 *  __mul.wt
			 *
			 *  __push.wr			-->	__{s/u}div.wi	i
			 *  __ld.wi		i
			 *  __{s/u}div.wt
			 *
			 *  __push.wr			-->	__{s/u}mod.wi	i
			 *  __ld.wi		i
			 *  __{s/u}mod.wt
			 */
			if
			((p[0]->ins_code == I_ADD_WT ||
			  p[0]->ins_code == I_SUB_WT ||
			  p[0]->ins_code == I_AND_WT ||
			  p[0]->ins_code == I_EOR_WT ||
			  p[0]->ins_code == I_OR_WT ||
			  p[0]->ins_code == I_ASL_WT ||
			  p[0]->ins_code == I_ASR_WT ||
			  p[0]->ins_code == I_LSR_WT ||
			  p[0]->ins_code == I_MUL_WT ||
			  p[0]->ins_code == I_SDIV_WT ||
			  p[0]->ins_code == I_UDIV_WT ||
			  p[0]->ins_code == I_SMOD_WT ||
			  p[0]->ins_code == I_UMOD_WT) &&
			 (p[1]->ins_code == I_LD_WI) &&
			 (p[2]->ins_code == I_PUSH_WR)
			) {
				/* replace code */
				*p[2] = *p[1];
				switch (p[0]->ins_code) {
				case I_ADD_WT: p[2]->ins_code = I_ADD_WI; break;
				case I_SUB_WT: p[2]->ins_code = I_SUB_WI; break;
				case I_AND_WT: p[2]->ins_code = I_AND_WI; break;
				case I_EOR_WT: p[2]->ins_code = I_EOR_WI; break;
				case I_OR_WT: p[2]->ins_code = I_OR_WI; break;
				case I_ASL_WT: p[2]->ins_code = I_ASL_WI; break;
				case I_ASR_WT: p[2]->ins_code = I_ASR_WI; break;
				case I_LSR_WT: p[2]->ins_code = I_LSR_WI; break;
				case I_MUL_WT: p[2]->ins_code = I_MUL_WI; break;
				case I_SDIV_WT: p[2]->ins_code = I_SDIV_WI; break;
				case I_UDIV_WT: p[2]->ins_code = I_UDIV_WI; break;
				case I_SMOD_WT: p[2]->ins_code = I_SMOD_WI; break;
				case I_UMOD_WT: p[2]->ins_code = I_UMOD_WI; break;
				default: abort();
				}
				nb = 2;

				if (p[2]->ins_type == T_VALUE && p[2]->ins_data == 0) {
					switch (p[2]->ins_code) {
					case I_SDIV_WT:
					case I_UDIV_WT:
					case I_SMOD_WT:
					case I_UMOD_WT:
						error("cannot optimize a divide-by-zero");
						break;
					case I_AND_WT:
					case I_MUL_WT:
						p[2]->ins_code = I_LD_WI;
						break;
					default:
						nb = 3;
						break;
					}
				}
			}

			/* flush queue */
			if (nb) {
				q_wr -= nb;
				q_nb -= nb;
				nb = 0;

				if (q_wr < 0)
					q_wr += Q_SIZE;

				/* loop */
				goto lv1_loop;
			}
		}

		if (q_nb >= 2) {
			/*
			 *  __ld.wi		i	-->	__ld.wi		(~i)
			 *  __com.wr
			 */
			if
			((p[0]->ins_code == I_COM_WR) &&
			 (p[1]->ins_code == I_LD_WI) &&
			 (p[1]->ins_type == T_VALUE)
			) {
				/* replace code */
				p[1]->ins_data = ~p[1]->ins_data;
				nb = 1;
			}

			/*
			 *  __ld.wi		i	-->	__ld.wi		(-i)
			 *  __neg.wr
			 */
			else if
			((p[0]->ins_code == I_NEG_WR) &&
			 (p[1]->ins_code == I_LD_WI) &&
			 (p[1]->ins_type == T_VALUE)
			) {
				/* replace code */
				p[1]->ins_data = -p[1]->ins_data;
				nb = 1;
			}

			/*
			 *  __ld.wi		i	-->	__ld.wi		(i + j)
			 *  __add.wi		j
			 */
			else if
			((p[0]->ins_code == I_ADD_WI) &&
			 (p[0]->ins_type == T_VALUE) &&
			 (p[1]->ins_code == I_LD_WI) &&
			 (p[1]->ins_type == T_VALUE)
			) {
				/* replace code */
				p[1]->ins_data += p[0]->ins_data;
				nb = 1;
			}

			/*
			 *  __ld.wi		i	-->	__ld.wi		(i - j)
			 *  __sub.wi		j
			 */
			else if
			((p[0]->ins_code == I_SUB_WI) &&
			 (p[0]->ins_type == T_VALUE) &&
			 (p[1]->ins_code == I_LD_WI) &&
			 (p[1]->ins_type == T_VALUE)
			) {
				/* replace code */
				p[1]->ins_data -= p[0]->ins_data;
				nb = 1;
			}

			/*
			 *  __ld.wi		i	-->	__ld.wi		(i & j)
			 *  __and.wi		j
			 */
			else if
			((p[0]->ins_code == I_AND_WI) &&
			 (p[0]->ins_type == T_VALUE) &&
			 (p[1]->ins_code == I_LD_WI) &&
			 (p[1]->ins_type == T_VALUE)
			) {
				/* replace code */
				p[1]->ins_data &= p[0]->ins_data;
				nb = 1;
			}

			/*
			 *  __ld.wi		i	-->	__ld.wi		(i ^ j)
			 *  __eor.wi		j
			 */
			else if
			((p[0]->ins_code == I_EOR_WI) &&
			 (p[0]->ins_type == T_VALUE) &&
			 (p[1]->ins_code == I_LD_WI) &&
			 (p[1]->ins_type == T_VALUE)
			) {
				/* replace code */
				p[1]->ins_data ^= p[0]->ins_data;
				nb = 1;
			}

			/*
			 *  __ld.wi		i	-->	__ld.wi		(i | j)
			 *  __or.wi		j
			 */
			else if
			((p[0]->ins_code == I_OR_WI) &&
			 (p[0]->ins_type == T_VALUE) &&
			 (p[1]->ins_code == I_LD_WI) &&
			 (p[1]->ins_type == T_VALUE)
			) {
				/* replace code */
				p[1]->ins_data |= p[0]->ins_data;
				nb = 1;
			}


			/*
			 *  __ld.wi		i	-->	__ld.wi		(i + i)
			 *  __asl.wr
			 */
			else if
			((p[0]->ins_code == I_ASL_WR) &&
			 (p[1]->ins_code == I_LD_WI) &&
			 (p[1]->ins_type == T_VALUE)
			) {
				/* replace code */
				p[1]->ins_data += p[1]->ins_data;
				nb = 1;
			}

			/*
			 *  __ld.wi		i	-->	__ld.wi		(i << j)
			 *  __asl.wi		j
			 */
			else if
			((p[0]->ins_code == I_ASL_WI) &&
			 (p[0]->ins_type == T_VALUE) &&
			 (p[1]->ins_code == I_LD_WI) &&
			 (p[1]->ins_type == T_VALUE)
			) {
				/* replace code */
				p[1]->ins_data <<= p[0]->ins_data;
				nb = 1;
			}

			/*
			 *  __ld.wi		i	-->	__ld.wi		(i >> j)
			 *  __asr.wi		j
			 */
			else if
			((p[0]->ins_code == I_ASR_WI) &&
			 (p[0]->ins_type == T_VALUE) &&
			 (p[1]->ins_code == I_LD_WI) &&
			 (p[1]->ins_type == T_VALUE)
			) {
				/* replace code */
				p[1]->ins_data >>= p[0]->ins_data;
				nb = 1;
			}

			/*
			 *  __ld.wi		i	-->	__ld.wi		(i >> j)
			 *  __lsr.wi		j
			 */
			else if
			((p[0]->ins_code == I_LSR_WI) &&
			 (p[0]->ins_type == T_VALUE) &&
			 (p[1]->ins_code == I_LD_WI) &&
			 (p[1]->ins_type == T_VALUE)
			) {
				/* replace code */
				p[1]->ins_data = ((uintptr_t) p[1]->ins_data) >> p[0]->ins_data;
				nb = 1;
			}

			/*
			 *  __ld.wi		i	-->	__ld.wi		(i * j)
			 *  __mul.wi		j
			 */
			else if
			((p[0]->ins_code == I_MUL_WI) &&
			 (p[0]->ins_type == T_VALUE) &&
			 (p[1]->ins_code == I_LD_WI) &&
			 (p[1]->ins_type == T_VALUE)
			) {
				/* replace code */
				p[1]->ins_data *= p[0]->ins_data;
				nb = 1;
			}

			/*
			 *  __ld.wi		i	-->	__ld.wi		(i / j)
			 *  __sdiv.wi		j
			 */
			else if
			((p[0]->ins_code == I_SDIV_WI) &&
			 (p[0]->ins_type == T_VALUE) &&
			 (p[1]->ins_code == I_LD_WI) &&
			 (p[1]->ins_type == T_VALUE)
			) {
				/* replace code */
				p[1]->ins_data = p[1]->ins_data / p[0]->ins_data;
				nb = 1;
			}

			/*
			 *  __ld.wi		i	-->	__ld.wi		(i / j)
			 *  __udiv.wi		j
			 */
			else if
			((p[0]->ins_code == I_UDIV_WI) &&
			 (p[0]->ins_type == T_VALUE) &&
			 (p[1]->ins_code == I_LD_WI) &&
			 (p[1]->ins_type == T_VALUE)
			) {
				/* replace code */
				p[1]->ins_data = ((unsigned) p[1]->ins_data) / ((unsigned) p[0]->ins_data);
				nb = 1;
			}

			/*
			 *  __ld.wi		i	-->	__ld.wi		(i % j)
			 *  __smod.wi		j
			 */
			else if
			((p[0]->ins_code == I_SMOD_WI) &&
			 (p[0]->ins_type == T_VALUE) &&
			 (p[1]->ins_code == I_LD_WI) &&
			 (p[1]->ins_type == T_VALUE)
			) {
				/* replace code */
				p[1]->ins_data = p[1]->ins_data % p[0]->ins_data;
				nb = 1;
			}

			/*
			 *  __ld.wi		i	-->	__ld.wi		(i % j)
			 *  __umod.wi		j
			 */
			else if
			((p[0]->ins_code == I_UMOD_WI) &&
			 (p[0]->ins_type == T_VALUE) &&
			 (p[1]->ins_code == I_LD_WI) &&
			 (p[1]->ins_type == T_VALUE)
			) {
				/* replace code */
				p[1]->ins_data = ((unsigned) p[1]->ins_data) % ((unsigned) p[0]->ins_data);
				nb = 1;
			}

			/* flush queue */
			if (nb) {
				q_wr -= nb;
				q_nb -= nb;
				nb = 0;

				if (q_wr < 0)
					q_wr += Q_SIZE;

				/* loop */
				goto lv1_loop;
			}
		}

		/* ********************************************************* */
		/* then transform muliplication and division by a power of 2 */
		/* ********************************************************* */

		if (q_nb >= 1) {
			/*
			 *  __mul.wi		i	-->	__asl.wi	log2(i)
			 */
			if
			((p[0]->ins_code == I_MUL_WI) &&
			 (p[0]->ins_type == T_VALUE) &&
			 (p[0]->ins_data >= -1) &&
			 (p[0]->ins_data <= 65536)
			) {
				/* replace code */
				if (p[0]->ins_data == -1) {
					p[0]->ins_code = I_NEG_WR;
					p[0]->ins_type = 0;
					p[0]->ins_data = 0;
					/* no instructions removed, just loop */
					goto lv1_loop;
				}
				else
				if (p[0]->ins_data == 0) {
					p[0]->ins_code = I_LD_WI;
					/* no instructions removed, just loop */
					goto lv1_loop;
				}
				else
				if (p[0]->ins_data == 1) {
					nb = 1;
				}
				else
				if (__builtin_popcount((unsigned int)p[0]->ins_data) == 1) {
					p[0]->ins_code = I_ASL_WI;
					p[0]->ins_type = T_VALUE;
					p[0]->ins_data = __builtin_ctz((unsigned int)p[0]->ins_data);
					/* no instructions removed, just loop */
					goto lv1_loop;
				}
			}

			/*
			 *  __udiv.wi		i	-->	__lsr.wi	log2(i)
			 *
			 *  Also possible after converting an __sdiv.wi into a __udiv.{w/u}i
			 *
			 *  N.B. You cannot convert __sdiv.wi into __asr.wi!
			 */
			if
			((p[0]->ins_code == I_UDIV_WI ||
			  p[0]->ins_code == I_UDIV_UI) &&
			 (p[0]->ins_type == T_VALUE) &&
			 (p[0]->ins_data >= 0) &&
			 (p[0]->ins_data <= 65536)
			) {
				/* replace code */
				if (p[0]->ins_data == 0) {
					error("cannot optimize a divide-by-zero");
				}
				else
				if (p[0]->ins_data == 1) {
					nb = 1;
				}
				else
				if (__builtin_popcount((unsigned int)p[0]->ins_data) == 1) {
					p[0]->ins_code = I_LSR_WI;
					p[0]->ins_type = T_VALUE;
					p[0]->ins_data = __builtin_ctz((unsigned int)p[0]->ins_data);
					/* no instructions removed, just loop */
					goto lv1_loop;
				}
			}

			/*
			 *  __umod.wi		i	-->	__and.wi	(i - 1)
			 *
			 *  Also possible after converting an __smod.wi into a __umod.{w/u}i
			 *
			 *  N.B. Modifying an __smod.wi is ugly!
			 */
			if
			((p[0]->ins_code == I_UMOD_WI ||
			  p[0]->ins_code == I_UMOD_UI) &&
			 (p[0]->ins_type == T_VALUE) &&
			 (p[0]->ins_data >= 0) &&
			 (p[0]->ins_data <= 65536)
			) {
				/* replace code */
				if (p[0]->ins_data == 0) {
					error("cannot optimize a divide-by-zero");
				}
				else
				if (p[0]->ins_data == 1) {
					p[0]->ins_code = I_LD_WI;
					p[0]->ins_type = T_VALUE;
					p[0]->ins_data = 0;
					/* no instructions removed, just loop */
					goto lv1_loop;
				}
				else
				if (__builtin_popcount((unsigned int)p[0]->ins_data) == 1) {
					p[0]->ins_code = I_AND_WI;
					p[0]->ins_data = p[0]->ins_data - 1;
					/* no instructions removed, just loop */
					goto lv1_loop;
				}
			}

			/* flush queue */
			if (nb) {
				q_wr -= nb;
				q_nb -= nb;
				nb = 0;

				if (q_wr < 0)
					q_wr += Q_SIZE;

				/* loop */
				goto lv1_loop;
			}
		}

		/* ********************************************************* */
		/* then optimize conditional tests */
		/* ********************************************************* */

		if (q_nb >= 4) {
			/*
			 *  __not.wr			-->	__tst.wr
			 *  __bool				__bool
			 *  __not.wr
			 *  __bool
			 *
			 *  Merge two consecutive I_NOT_WR into an I_TST_WR
			 *  with a boolean result.
			 */
			if
			((p[0]->ins_code == I_BOOLEAN) &&
			 (p[1]->ins_code == I_NOT_WR) &&
			 (p[2]->ins_code == I_BOOLEAN) &&
			 (p[3]->ins_code == I_NOT_WR)
			) {
				p[3]->ins_code = I_TST_WR;
				nb = 2;
			}

			/*
			 *  __tst.wr			-->	__not.wr
			 *  __bool				__bool
			 *  __not.wr
			 *  __bool
			 *
			 *  Unlikely, but possible if someone is writing a crazy
			 *  string of "!" commands.
			 */
			else if
			((p[0]->ins_code == I_BOOLEAN) &&
			 (p[1]->ins_code == I_NOT_WR) &&
			 (p[2]->ins_code == I_BOOLEAN) &&
			 (p[3]->ins_code == I_TST_WR)
			) {
				p[3]->ins_code = I_NOT_WR;
				nb = 2;
			}

			/*
			 *  LLnn:			-->	LLnn:
			 *  __bool				LLqq:
			 *  __tst.wr				__bool
			 *  LLqq:
			 *
			 *  Remove redundant __tst.wr from compound conditionals
			 *  that the compiler generates.
			 */
			else if
			((p[0]->ins_code == I_LABEL) &&
			 (p[1]->ins_code == I_TST_WR) &&
			 (p[2]->ins_code == I_BOOLEAN) &&
			 (p[3]->ins_code == I_LABEL)
			) {
				*p[1] = *p[2];
				*p[2] = *p[0];
				nb = 1;
			}

			/*
			 *  LLnn:			-->	LLnn:
			 *  __bool				__bfalse
			 *  __tst.wr
			 *  __bfalse
			 *
			 *  LLnn:			-->	LLnn:
			 *  __bool				__btrue
			 *  __tst.wr
			 *  __btrue
			 *
			 *  Remove redundant __tst.wr from compound conditionals
			 *  that the compiler generates.
			 */
			else if
			((p[0]->ins_code == I_BFALSE ||
			  p[0]->ins_code == I_BTRUE) &&
			 (p[1]->ins_code == I_TST_WR) &&
			 (p[2]->ins_code == I_BOOLEAN) &&
			 (p[3]->ins_code == I_LABEL)
			) {
				*p[2] = *p[0];
				nb = 2;
			}

			/*
			 *  LLnn:			-->	LLnn:
			 *  __bool				__not.cf
			 *  __not.wr				__bfalse
			 *  __bfalse
			 *
			 *  LLnn:			-->	LLnn:
			 *  __bool				__not.cf
			 *  __not.wr				__btrue
			 *  __btrue
			 *
			 *  Happens when a "!" follows a condition in brackets.
			 *
			 *  It is so tempting to just invert the branch, but
			 *  that would break subsequent branches!
			 */
			else if
			((p[0]->ins_code == I_BFALSE ||
			  p[0]->ins_code == I_BTRUE) &&
			 (p[1]->ins_code == I_NOT_WR) &&
			 (p[2]->ins_code == I_BOOLEAN) &&
			 (p[3]->ins_code == I_LABEL)
			) {
				*p[1] = *p[0];
				p[2]->ins_code = X_NOT_CF;
				nb = 1;
			}

			/* flush queue */
			if (nb) {
				q_wr -= nb;
				q_nb -= nb;
				nb = 0;

				if (q_wr < 0)
					q_wr += Q_SIZE;

				/* loop */
				goto lv1_loop;
			}
		}

		if (q_nb >= 3) {
			/*
			 *  __cmp.wt			-->	__cmp.wt
			 *  __bool
			 *  __tst.wr
			 *
			 *  __cmp.wi			-->	__cmp.wi
			 *  __bool
			 *  __tst.wr
			 *
			 *  __cmp.wm			-->	__cmp.wm
			 *  __bool
			 *  __tst.wr
			 *
			 *  __cmp.ws			-->	__cmp.ws
			 *  __bool
			 *  __tst.wr
			 *
			 *  __cmp.uiq			-->	__cmp.uiq
			 *  __bool
			 *  __tst.wr
			 *
			 *  __cmp.umq			-->	__cmp.umq
			 *  __bool
			 *  __tst.wr
			 *
			 *  __cmp.usq			-->	__cmp.usq
			 *  __bool
			 *  __tst.wr
			 *
			 *  __not.wr			-->	__not.wr
			 *  __bool
			 *  __tst.wr
			 *
			 *  Remove redundant __tst.wr in compound conditionals
			 *  that the compiler generates.
			 *
			 *  __tst.wr			-->	__tst.wr
			 *  __bool
			 *  __tst.wr
			 *
			 *  This can happen when two I_NOT_WR are merged.
			 */
			if
			((p[0]->ins_code == I_TST_WR) &&
			 (p[1]->ins_code == I_BOOLEAN) &&
			 (p[2]->ins_code == I_CMP_WT ||
			  p[2]->ins_code == X_CMP_WI ||
			  p[2]->ins_code == X_CMP_WM ||
			  p[2]->ins_code == X_CMP_WS ||
			  p[2]->ins_code == X_CMP_UIQ ||
			  p[2]->ins_code == X_CMP_UMQ ||
			  p[2]->ins_code == X_CMP_USQ ||
			  p[2]->ins_code == I_NOT_WR ||
			  p[2]->ins_code == I_TST_WR)
			) {
				nb = 2;
			}

			/*
			 *  __cmp.wi			-->	__cmp.wi
			 *  __bool
			 *  __not.wr
			 *
			 *  __cmp.wm			-->	__cmp.wm
			 *  __bool
			 *  __not.wr
			 *
			 *  __cmp.ws			-->	__cmp.ws
			 *  __bool
			 *  __not.wr
			 *
			 *  __cmp.uiq			-->	__cmp.uiq
			 *  __bool
			 *  __not.wr
			 *
			 *  __cmp.umq			-->	__cmp.umq
			 *  __bool
			 *  __not.wr
			 *
			 *  __cmp.usq			-->	__cmp.usq
			 *  __bool
			 *  __not.wr
			 *
			 *  N.B. This inverts the test condition of the __cmp.wi!
			 */
			else if
			((p[0]->ins_code == I_NOT_WR) &&
			 (p[1]->ins_code == I_BOOLEAN) &&
			 (p[2]->ins_code == X_CMP_WI ||
			  p[2]->ins_code == X_CMP_WM ||
			  p[2]->ins_code == X_CMP_WS ||
			  p[2]->ins_code == X_CMP_UIQ ||
			  p[2]->ins_code == X_CMP_UMQ ||
			  p[2]->ins_code == X_CMP_USQ)
			) {
				p[2]->cmp_type = compare2not[p[2]->cmp_type];
				nb = 2;
			}

			/* flush queue */
			if (nb) {
				q_wr -= nb;
				q_nb -= nb;
				nb = 0;

				if (q_wr < 0)
					q_wr += Q_SIZE;

				/* loop */
				goto lv1_loop;
			}
		}

		if (q_nb >= 2) {
			/*
			 *  __bool			-->	LLnn:
			 *  LLnn:				__bool
			 *
			 *  Delay boolean conversion until the end of the list of labels
			 *  that are generated when using multiple "&&" and "||".
			 *
			 *  N.B. This optimization should be done before the X_TST_WM optimization!
			 */
			if
			((p[0]->ins_code == I_LABEL) &&
			 (p[1]->ins_code == I_BOOLEAN)
			) {
				*p[1] = *p[0];
				p[0]->ins_code = I_BOOLEAN;
				p[0]->ins_type = 0;
				p[0]->ins_data = 0;
				/* no instructions removed, just loop */
				goto lv1_loop;
			}

			/*
			 *  __bool			-->	__bool
			 *  __bool
			 *
			 *  Remove redundant conversions of a flag into a boolean
			 *  that are generated when using multiple "&&" and "||".
			 *
			 *  N.B. This optimization should be done before the X_TST_WM optimization!
			 */
			else if
			((p[1]->ins_code == I_BOOLEAN) &&
			 (p[0]->ins_code == I_BOOLEAN)
			) {
				*p[1] = *p[0];
				nb = 1;
			}

			/*
			 *  __bra LLaa			-->	__bra LLaa
			 *  __bra LLbb
			 */
			else if
			((p[0]->ins_code == I_BRA) &&
			 (p[1]->ins_code == I_BRA)
			) {
				nb = 1;
			}

			/*
			 *  LLaa:				LLaa .alias LLbb
			 *  __bra LLbb			-->	__bra LLbb
			 */
			else if
			((p[0]->ins_code == I_BRA) &&
			 (p[1]->ins_code == I_LABEL)
			) {
				int i = 1;
				do	{
					if (p[i]->ins_data != p[0]->ins_data) {
						p[i]->ins_code = I_ALIAS;
						p[i]->imm_type = T_VALUE;
						p[i]->imm_data = p[0]->ins_data;
					}
				} while (++i < q_nb && i < 10 && p[i]->ins_code == I_LABEL);
			}

			/*
			 *  __bra LLaa			-->	LLaa:
			 *  LLaa:
			 */
			else if
			((p[0]->ins_code == I_LABEL) &&
			 (p[1]->ins_code == I_BRA) &&
			 (p[1]->ins_type == T_LABEL) &&
			 (p[0]->ins_data == p[1]->ins_data)
			) {
				*p[1] = *p[0];
				nb = 1;
			}

			/* flush queue */
			if (nb) {
				q_wr -= nb;
				q_nb -= nb;
				nb = 0;

				if (q_wr < 0)
					q_wr += Q_SIZE;

				/* loop */
				goto lv1_loop;
			}
		}

		/* ********************************************************* */
		/* 6-instruction patterns */
		/* ********************************************************* */

		if (q_nb >= 6) {
			/*
			 *  __ld.wi		p	-->	__ld.wm		p
			 *  __push.wr				__add.wi	i
			 *  __st.wm		__ptr		__st.wm		p
			 *  __ld.wp		__ptr
			 *  __add.wi		i
			 *  __st.wpt
			 *
			 *  JCB: Isn't this already handled by other rules?
			 */
			if
			((p[0]->ins_code == I_ST_WPT) &&
			 (p[1]->ins_code == I_ADD_WI ||
			  p[1]->ins_code == I_SUB_WI) &&
			 (p[2]->ins_code == I_LD_WP) &&
			 (p[2]->ins_type == T_PTR) &&
			 (p[3]->ins_code == I_ST_WM) &&
			 (p[3]->ins_type == T_PTR) &&
			 (p[4]->ins_code == I_PUSH_WR) &&
			 (p[5]->ins_code == I_LD_WI)
			) {
				*p[3] = *p[5];
				p[3]->ins_code = I_ST_WM;
				*p[4] = *p[1];
				p[5]->ins_code = I_LD_WM;
				nb = 3;
			}

			/* flush queue */
			if (nb) {
				q_wr -= nb;
				q_nb -= nb;
				nb = 0;

				if (q_wr < 0)
					q_wr += Q_SIZE;

				/* loop */
				goto lv1_loop;
			}
		}

		/* ********************************************************* */
		/* 5-instruction patterns */
		/* ********************************************************* */

		if (q_nb >= 5) {
			/* flush queue */
			if (nb) {
				q_wr -= nb;
				q_nb -= nb;
				nb = 0;

				if (q_wr < 0)
					q_wr += Q_SIZE;

				/* loop */
				goto lv1_loop;
			}
		}

		/* ********************************************************* */
		/* 4-instruction patterns */
		/* ********************************************************* */

		if (q_nb >= 4) {
			/*  __ld.wi		i	-->	__ld.wi		i
			 *  __push.wr				__push.wr
			 *  __st.wm		__ptr		__ld.{w/u}m	i
			 *  __ld.{w/u}p		__ptr
			 *
			 *  Load a variable from memory, this is generated for
			 *  code like a "+=", where the store can be optimized
			 *  later on.
			 */
			if
			((p[0]->ins_code == I_LD_WP ||
			  p[0]->ins_code == I_LD_BP ||
			  p[0]->ins_code == I_LD_UP) &&
			 (p[0]->ins_type == T_PTR) &&
			 (p[1]->ins_code == I_ST_WM) &&
			 (p[1]->ins_type == T_PTR) &&
			 (p[2]->ins_code == I_PUSH_WR) &&
			 (p[3]->ins_code == I_LD_WI)
			) {
				/* replace code */
				*p[1] = *p[3];
				if (p[0]->ins_code == I_LD_WP)
					p[1]->ins_code = I_LD_WM;
				else
				if (p[0]->ins_code == I_LD_BP)
					p[1]->ins_code = I_LD_BM;
				else
					p[1]->ins_code = I_LD_UM;
				nb = 1;
			}

			/*
			 *  __lea.s		i	-->	__lea.s		i
			 *  __push.wr				__push.wr
			 *  __st.wm		__ptr		__ld.{w/b/u}s	(i + 2)
			 *  __ld.{w/b/u}p	__ptr
			 *
			 *  Load a variable from memory, this is generated for
			 *  code like a "+=", where the store can be optimized
			 *  later on.
			 */
			else if
			((p[0]->ins_code == I_LD_WP ||
			  p[0]->ins_code == I_LD_BP ||
			  p[0]->ins_code == I_LD_UP) &&
			 (p[0]->ins_type == T_PTR) &&
			 (p[1]->ins_code == I_ST_WM) &&
			 (p[1]->ins_type == T_PTR) &&
			 (p[2]->ins_code == I_PUSH_WR) &&
			 (p[3]->ins_code == I_LEA_S)
			) {
				/* replace code */
				*p[1] = *p[3];
				if (p[0]->ins_code == I_LD_WP)
					p[1]->ins_code = X_LD_WS;
				else
				if (p[0]->ins_code == I_LD_BP)
					p[1]->ins_code = X_LD_BS;
				else
					p[1]->ins_code = X_LD_US;
				p[1]->ins_data += 2;
				nb = 1;
			}

			/*
			 *  __lea.s		i	-->	__lea.s		(i + j)
			 *  __push.wr
			 *  __ld.wi		j
			 *  __add.wt
			 *
			 *  This is generated for address calculations into local
			 *  arrays and structs on the stack.
			 */
			else if
			((p[0]->ins_code == I_ADD_WT) &&
			 (p[1]->ins_code == I_LD_WI) &&
			 (p[1]->ins_type == T_VALUE) &&
			 (p[2]->ins_code == I_PUSH_WR) &&
			 (p[3]->ins_code == I_LEA_S)
			) {
				/* replace code */
				p[3]->ins_code = I_LEA_S;
				p[3]->ins_data += p[1]->ins_data;
				nb = 3;
			}

#if OPT_ARRAY_RD
			/*
			 *  __asl.wr			-->	__ld.war	array
			 *  __add.wi		array
			 *  __st.wm		_ptr
			 *  __ld.wp		_ptr
			 *
			 *  Classic base+offset word-array read.
			 *
			 *  N.B. The byte-array peephole rule is further down in this file.
			 */
			else if
			((p[0]->ins_code == I_LD_WP) &&
			 (p[0]->ins_type == T_PTR) &&
			 (p[1]->ins_code == I_ST_WM) &&
			 (p[1]->ins_type == T_PTR) &&
			 (p[2]->ins_code == I_ADD_WI) &&
			 (p[2]->ins_type == T_SYMBOL) &&
			 (is_small_array((SYMBOL *)p[2]->ins_data)) &&
			 (p[3]->ins_code == I_ASL_WR)
			) {
				/* replace code */
				p[3]->ins_code = X_LD_WAR;
				p[3]->ins_type = T_SYMBOL;
				p[3]->ins_data = p[2]->ins_data;
				nb = 3;
			}
#endif

			/*
			 *  is_ubyte()			-->	is_ubyte()
			 *  __push.wr				__cmp.umq	type, symbol
			 *  __ld.um		symbol
			 *  __cmp.wt		type
			 *
			 *  is_ubyte()			-->	is_ubyte()
			 *  __push.wr				__cmp.usq	type, (n - 2)
			 *  __ld.us		n
			 *  __cmp.wt		type
			 */
			else if
			((p[0]->ins_code == I_CMP_WT) &&
			 (p[1]->ins_code == I_LD_UM ||
			  p[1]->ins_code == X_LD_US) &&
			 (p[2]->ins_code == I_PUSH_WR) &&
			 (is_ubyte(p[3]))
			) {
				/* replace code */
				*p[2] = *p[1];
				switch (p[1]->ins_code) {
				case I_LD_UM: p[2]->ins_code = X_CMP_UMQ; break;
				case X_LD_US: p[2]->ins_code = X_CMP_USQ; p[2]->ins_data -= 2; break;
				default:	break;
				}
				p[2]->cmp_type = compare2uchar[p[0]->cmp_type];
				nb = 2;
			}

			/* flush queue */
			if (nb) {
				q_wr -= nb;
				q_nb -= nb;
				nb = 0;

				if (q_wr < 0)
					q_wr += Q_SIZE;

				/* loop */
				goto lv1_loop;
			}
		}

		/* ********************************************************* */
		/* 3-instruction patterns */
		/* ********************************************************* */

		if (q_nb >= 3) {
			/*
			 *  __case			-->	LLnn:
			 *  __endcase
			 *  LLnn:
			 *
			 *  I_ENDCASE is only generated in order to catch which
			 *  case statements could fall through to the next case
			 *  so that an SAX instruction could be generated if or
			 *  when do_switch() uses a "JMP [table, x]".
			 *
			 *  This removes redundant I_CASE/I_ENDCASE i-codes that
			 *  just fall through to the next case.
			 */
			if
			((p[0]->ins_code == I_LABEL) &&
			 (p[1]->ins_code == I_ENDCASE) &&
			 (p[2]->ins_code == I_CASE)
			) {
				/* remove code */
				*p[2] = *p[0];
				nb = 2;
			}

			/*  __ld.wi		i	-->	__ld.{w/b/u}m	i
			 *  __st.wm		__ptr
			 *  __ld.{w/b/u}p	__ptr
			 *
			 *  Load a global/static variable from memory
			 */
			else if
			((p[0]->ins_code == I_LD_WP ||
			  p[0]->ins_code == I_LD_BP ||
			  p[0]->ins_code == I_LD_UP) &&
			 (p[0]->ins_type == T_PTR) &&
			 (p[1]->ins_code == I_ST_WM) &&
			 (p[1]->ins_type == T_PTR) &&
			 (p[2]->ins_code == I_LD_WI)
			) {
				/* replace code */
				if (p[0]->ins_code == I_LD_WP)
					p[2]->ins_code = I_LD_WM;
				else
				if (p[0]->ins_code == I_LD_BP)
					p[2]->ins_code = I_LD_BM;
				else
					p[2]->ins_code = I_LD_UM;
				nb = 2;
			}

			/*
			 *  __lea.s		i	-->	__ld.{w/b/u}s	i
			 *  __st.wm		__ptr
			 *  __ld.{w/b/u}p	__ptr
			 *
			 *  Load a local variable from memory
			 */
			else if
			((p[0]->ins_code == I_LD_WP ||
			  p[0]->ins_code == I_LD_BP ||
			  p[0]->ins_code == I_LD_UP) &&
			 (p[0]->ins_type == T_PTR) &&
			 (p[1]->ins_code == I_ST_WM) &&
			 (p[1]->ins_type == T_PTR) &&
			 (p[2]->ins_code == I_LEA_S)
			) {
				/* replace code */
				if (p[0]->ins_code == I_LD_WP)
					p[2]->ins_code = X_LD_WS;
				else
				if (p[0]->ins_code == I_LD_BP)
					p[2]->ins_code = X_LD_BS;
				else
					p[2]->ins_code = X_LD_US;
				nb = 2;
			}

			/*
			 *  __push.wr			-->	__add.wm	symbol
			 *  __ld.wm		symbol
			 *  __add.wt
			 *
			 *  __push.wr			-->	__sub.wm	symbol
			 *  __ld.wm		symbol
			 *  __sub.wt
			 *
			 *  etc/etc
			 */
			else if
			((p[0]->ins_code == I_ADD_WT ||
			  p[0]->ins_code == I_SUB_WT ||
			  p[0]->ins_code == I_AND_WT ||
			  p[0]->ins_code == I_EOR_WT ||
			  p[0]->ins_code == I_OR_WT) &&
			 (p[1]->ins_code == I_LD_WM) &&
			 (p[2]->ins_code == I_PUSH_WR)
			) {
				/* replace code */
				*p[2] = *p[1];
				switch (p[0]->ins_code) {
				case I_ADD_WT: p[2]->ins_code = I_ADD_WM; break;
				case I_SUB_WT: p[2]->ins_code = I_SUB_WM; break;
				case I_AND_WT: p[2]->ins_code = I_AND_WM; break;
				case I_EOR_WT: p[2]->ins_code = I_EOR_WM; break;
				case I_OR_WT: p[2]->ins_code = I_OR_WM; break;
				default: abort();
				}
				nb = 2;
			}

			/*
			 *  __push.wr			-->	__add.um	symbol
			 *  __ld.um		symbol
			 *  __add.wt
			 *
			 *  __push.wr			-->	__sub.um	symbol
			 *  __ld.um		symbol
			 *  __sub.wt
			 *
			 *  etc/etc
			 */
			else if
			((p[0]->ins_code == I_ADD_WT ||
			  p[0]->ins_code == I_SUB_WT ||
			  p[0]->ins_code == I_AND_WT ||
			  p[0]->ins_code == I_EOR_WT ||
			  p[0]->ins_code == I_OR_WT) &&
			 (p[1]->ins_code == I_LD_UM) &&
			 (p[2]->ins_code == I_PUSH_WR)
			) {
				/* replace code */
				*p[2] = *p[1];
				switch (p[0]->ins_code) {
				case I_ADD_WT: p[2]->ins_code = I_ADD_UM; break;
				case I_SUB_WT: p[2]->ins_code = I_SUB_UM; break;
				case I_AND_WT: p[2]->ins_code = I_AND_UM; break;
				case I_EOR_WT: p[2]->ins_code = I_EOR_UM; break;
				case I_OR_WT: p[2]->ins_code = I_OR_UM; break;
				default: abort();
				}
				nb = 2;
			}

			/*
			 *  __push.wr			-->	__add.ws	(i - 2)
			 *  __ld.ws		i
			 *  __add.wt
			 */
			else if
			((p[0]->ins_code == I_ADD_WT) &&
			 (p[1]->ins_code == X_LD_WS ||
			  p[1]->ins_code == X_LD_US) &&
			 (p[2]->ins_code == I_PUSH_WR)
			) {
				/* replace code */
				*p[2] = *p[1];
				switch (p[1]->ins_code) {
				case X_LD_WS: p[2]->ins_code = X_ADD_WS; break;
				case X_LD_US: p[2]->ins_code = X_ADD_US; break;
				default: abort();
				}
				p[2]->ins_data -= 2;
				nb = 2;
			}

			/*
			 *  __push.wr			-->	__not.wr
			 *  __ld.wi		0
			 *  __cmp.wt		equ_w
			 *
			 *  __push.wr			-->	__tst.wr
			 *  __ld.wi		0
			 *  __cmp.wt		neq_w
			 *
			 *  Check for this before converting to X_CMP_WI!
			 */
			else if
			((p[0]->ins_code == I_CMP_WT) &&
			 (p[0]->cmp_type == CMP_EQU ||
			  p[0]->cmp_type == CMP_NEQ) &&
			 (p[1]->ins_code == I_LD_WI) &&
			 (p[1]->ins_type == T_VALUE) &&
			 (p[1]->ins_data == 0) &&
			 (p[2]->ins_code == I_PUSH_WR)
			) {
				/* replace code */
				p[2]->ins_code = (p[0]->cmp_type == CMP_EQU) ? I_NOT_WR : I_TST_WR;
				p[2]->ins_type = 0;
				p[2]->ins_data = 0;
				nb = 2;
			}

			/*
			 *  __push.wr			-->	__cmp.wi	type, i
			 *  __ld.wi		i
			 *  __cmp.wt		type
			 *
			 *  __push.wr			-->	__cmp.wm	type, symbol
			 *  __ld.wm		symbol
			 *  __cmp.wt		type
			 *
			 *  __push.wr			-->	__cmp.ws	type, (n - 2)
			 *  __ld.ws		n
			 *  __cmp.wt		type
			 */
			else if
			((p[0]->ins_code == I_CMP_WT) &&
			 (p[1]->ins_code == I_LD_WI ||
			  p[1]->ins_code == I_LD_WM ||
			  p[1]->ins_code == X_LD_WS) &&
			 (p[2]->ins_code == I_PUSH_WR)
			) {
				/* replace code */
				*p[2] = *p[1];
				switch (p[1]->ins_code) {
				case I_LD_WI: p[2]->ins_code = X_CMP_WI; break;
				case I_LD_WM: p[2]->ins_code = X_CMP_WM; break;
				case X_LD_WS: p[2]->ins_code = X_CMP_WS; p[2]->ins_data -= 2; break;
				default:	break;
				}
				p[2]->cmp_type = p[0]->cmp_type;
				nb = 2;
			}

			/*
			 *  __ld{w/b/u}m	symbol	-->	__incld.{w/b/u}m  symbol
			 *  __add.wi		1
			 *  __st.{w/u}m		symbol
			 *
			 *  __ld{w/b/u}		symbol	-->	__decld.{w/b/u}m  symbol
			 *  __sub.wi		1
			 *  __st.{w/u}m		symbol
			 *
			 *  pre-increment, post-increment,
			 *  pre-decrement, post-decrement!
			 */
			else if
			((p[1]->ins_code == I_ADD_WI ||
			  p[1]->ins_code == I_SUB_WI) &&
			 (p[1]->ins_type == T_VALUE) &&
			 (p[1]->ins_data == 1) &&
			 (p[0]->ins_code == I_ST_WM ||
			  p[0]->ins_code == I_ST_UM) &&
			 (p[2]->ins_code == I_LD_WM ||
			  p[2]->ins_code == I_LD_BM ||
			  p[2]->ins_code == I_LD_UM) &&
			 (p[0]->ins_type == p[2]->ins_type) &&
			 (p[0]->ins_data == p[2]->ins_data)
//			 (cmp_operands(p[0], p[2]) == 1)
			) {
				/* replace code */
				switch (p[2]->ins_code) {
				case I_LD_WM:	p[2]->ins_code = (p[1]->ins_code == I_ADD_WI) ? X_INCLD_WM : X_DECLD_WM; break;
				case I_LD_BM:	p[2]->ins_code = (p[1]->ins_code == I_ADD_WI) ? X_INCLD_BM : X_DECLD_BM; break;
				case I_LD_UM:	p[2]->ins_code = (p[1]->ins_code == I_ADD_WI) ? X_INCLD_UM : X_DECLD_UM; break;
				default:	break;
				}
				nb = 2;
			}

			/*
			 *  __ld{w/b/u}s	symbol	-->	__incld.{w/b/u}s  symbol
			 *  __add.wi		1
			 *  __st.{w/u}s		symbol
			 *
			 *  __ld{w/b/u}s	symbol	-->	__decld.{w/b/u}s  symbol
			 *  __sub.wi		1
			 *  __st.{w/u}s		symbol
			 *
			 *  C pre-increment, post-increment,
			 *  C pre-decrement, post-decrement!
			 */
			else if
			((p[1]->ins_code == I_ADD_WI ||
			  p[1]->ins_code == I_SUB_WI) &&
			 (p[1]->ins_type == T_VALUE) &&
			 (p[1]->ins_data == 1) &&
			 (p[0]->ins_code == X_ST_WS ||
			  p[0]->ins_code == X_ST_US) &&
			 (p[2]->ins_code == X_LD_WS ||
			  p[2]->ins_code == X_LD_BS ||
			  p[2]->ins_code == X_LD_US) &&
			 (p[0]->ins_type == p[2]->ins_type) &&
			 (p[0]->ins_data == p[2]->ins_data)
			) {
				/* replace code */
				switch (p[2]->ins_code) {
				case X_LD_WS:	p[2]->ins_code = (p[1]->ins_code == I_ADD_WI) ? X_INCLD_WS : X_DECLD_WS; break;
				case X_LD_BS:	p[2]->ins_code = (p[1]->ins_code == I_ADD_WI) ? X_INCLD_BS : X_DECLD_BS; break;
				case X_LD_US:	p[2]->ins_code = (p[1]->ins_code == I_ADD_WI) ? X_INCLD_US : X_DECLD_US; break;
				default:	break;
				}
				nb = 2;
			}

			/*
			 *  __ldp.{w/b/u}ar	symbol	-->	__incld_{w/b/u}ar  symbol
			 *  __add.wi		1
			 *  __st.{w/u}at	symbol
			 *
			 *  __ldp.{w/b/u}ar	symbol	-->	__decld_{w/b/u}ar  symbol
			 *  __sub.wi		1
			 *  __st.{w/u}at	symbol
			 *
			 *  __ldp.{b/u}ay	symbol	-->	__incld_{b/u}ay  symbol
			 *  __add.wi		1
			 *  __st.{w/u}at	symbol
			 *
			 *  __ldp.{b/u}ay	symbol	-->	__decld_{b/u}ay  symbol
			 *  __sub.wi		1
			 *  __st.{w/u}at	symbol
			 *
			 *  pre-increment, post-increment,
			 *  pre-decrement, post-decrement!
			 */
			else if
			((p[1]->ins_code == I_ADD_WI ||
			  p[1]->ins_code == I_SUB_WI) &&
			 (p[1]->ins_type == T_VALUE) &&
			 (p[1]->ins_data == 1) &&
			 (p[0]->ins_code == X_ST_WAT ||
			  p[0]->ins_code == X_ST_UAT) &&
			 (p[2]->ins_code == X_LDP_WAR ||
			  p[2]->ins_code == X_LDP_BAR ||
			  p[2]->ins_code == X_LDP_UAR ||
			  p[2]->ins_code == X_LDP_BAY ||
			  p[2]->ins_code == X_LDP_UAY) &&
			 (p[0]->ins_type == p[2]->ins_type) &&
			 (p[0]->ins_data == p[2]->ins_data)
//			 (cmp_operands(p[0], p[2]) == 1)
			) {
				/* replace code */
				switch (p[2]->ins_code) {
				case X_LDP_WAR:	p[2]->ins_code = (p[1]->ins_code == I_ADD_WI) ? X_INCLD_WAR : X_DECLD_WAR; break;
				case X_LDP_BAR:	p[2]->ins_code = (p[1]->ins_code == I_ADD_WI) ? X_INCLD_BAR : X_DECLD_BAR; break;
				case X_LDP_UAR:	p[2]->ins_code = (p[1]->ins_code == I_ADD_WI) ? X_INCLD_UAR : X_DECLD_UAR; break;
				case X_LDP_BAY:	p[2]->ins_code = (p[1]->ins_code == I_ADD_WI) ? X_INCLD_BAY : X_DECLD_BAY; break;
				case X_LDP_UAY:	p[2]->ins_code = (p[1]->ins_code == I_ADD_WI) ? X_INCLD_UAY : X_DECLD_UAY; break;
				default:	break;
				}
				nb = 2;
			}

			/*
			 *  __ld.{w/b/u}p	symbol	-->	__not.{w/u}p	symbol
			 *  __tst.wr				not(__tst.wr or __not.wr)
			 *  not(__bool or __tst.wr or __not.wr)
			 *
			 *  __ld.{w/b/u}p	symbol	-->	__tst.{w/u}p	symbol
			 *  __not.wr				not(__tst.wr or __not.wr)
			 *  not(__bool or __tst.wr or __not.wr)
			 *
			 *  __ld.{w/b/u}m	symbol	-->	__not.{w/u}m	symbol
			 *  __tst.wr				not(__tst.wr or __not.wr)
			 *  not(__bool or __tst.wr or __not.wr)
			 *
			 *  __ld.{w/b/u}m	symbol	-->	__tst.{w/u}m	symbol
			 *  __not.wr				not(__tst.wr or __not.wr)
			 *  not(__bool or __tst.wr or __not.wr)
			 *
			 *  __ld.{w/b/u}s	symbol	-->	__not.{w/u}s	symbol
			 *  __tst.wr				not(__tst.wr or __not.wr)
			 *  not(__bool or __tst.wr or __not.wr)
			 *
			 *  __ld.{w/b/u}s	symbol	-->	__tst.{w/u}s	symbol
			 *  __not.wr				not(__tst.wr or __not.wr)
			 *  not(__bool or __tst.wr or __not.wr)
			 *
			 *  __ld.{w/b/u}ar	symbol	-->	__not.{w/u}ar	symbol
			 *  __tst.wr				not(__tst.wr or __not.wr)
			 *  not(__bool or __tst.wr or __not.wr)
			 *
			 *  __ld.{w/b/u}ar	symbol	-->	__tst.{w/u}ar	symbol
			 *  __not.wr				not(__tst.wr or __not.wr)
			 *  not(__bool or __tst.wr or __not.wr)
			 *
			 *  __ld.{b/u}ay	symbol	-->	__not.uay	symbol
			 *  __tst.wr				not(__tst.wr or __not.wr)
			 *  not(__bool or __tst.wr or __not.wr)
			 *
			 *  __ld.{b/u}ay	symbol	-->	__tst.uay	symbol
			 *  __not.wr				not(__tst.wr or __not.wr)
			 *  not(__bool or __tst.wr or __not.wr)
			 *
			 *  __and.{w/u}i	n	-->	__tand.wi	n
			 *  __tst.wr				not(__tst.wr or __not.wr)
			 *  not(__bool or __tst.wr or __not.wr)
			 *
			 *  __and.{w/u}i	n	-->	__nand.wi	n
			 *  __not.wr				not(__tst.wr or __not.wr)
			 *  not(__bool or __tst.wr or __not.wr)
			 *
			 *  N.B. This deliberately tests for the i-code after
			 *  the target I_TST_WR or I_NOT_WR in order to delay
			 *  the match until after merging the duplicate tests
			 *  that the code-generator often emits.
			 */
			else if
			((p[0]->ins_code != I_BOOLEAN) &&
			 (p[0]->ins_code != I_TST_WR) &&
			 (p[0]->ins_code != I_NOT_WR) &&
			 (p[1]->ins_code == I_TST_WR ||
			  p[1]->ins_code == I_NOT_WR) &&
			 (p[2]->ins_code == I_LD_WP ||
			  p[2]->ins_code == I_LD_WM ||
			  p[2]->ins_code == X_LD_WS ||
			  p[2]->ins_code == X_LD_WAR ||
			  p[2]->ins_code == I_LD_BP ||
			  p[2]->ins_code == I_LD_BM ||
			  p[2]->ins_code == X_LD_BS ||
			  p[2]->ins_code == X_LD_BAR ||
			  p[2]->ins_code == X_LD_BAY ||
			  p[2]->ins_code == I_LD_UP ||
			  p[2]->ins_code == I_LD_UM ||
			  p[2]->ins_code == X_LD_US ||
			  p[2]->ins_code == X_LD_UAR ||
			  p[2]->ins_code == X_LD_UAY ||
			  p[2]->ins_code == I_AND_WI ||
			  p[2]->ins_code == I_AND_UIQ)
			) {
				/* remove code */
				if (p[1]->ins_code == I_TST_WR) {
					switch (p[2]->ins_code) {
					case I_LD_WP:  p[2]->ins_code = X_TST_WP; break;
					case I_LD_WM:  p[2]->ins_code = X_TST_WM; break;
					case X_LD_WS:  p[2]->ins_code = X_TST_WS; break;
					case X_LD_WAR: p[2]->ins_code = X_TST_WAR; break;
					case I_LD_BP:
					case I_LD_UP:  p[2]->ins_code = X_TST_UP; break;
					case I_LD_BM:
					case I_LD_UM:  p[2]->ins_code = X_TST_UM; break;
					case X_LD_BS:
					case X_LD_US:  p[2]->ins_code = X_TST_US; break;
					case X_LD_BAR:
					case X_LD_UAR: p[2]->ins_code = X_TST_UAR; break;
					case X_LD_BAY:
					case X_LD_UAY: p[2]->ins_code = X_TST_UAY; break;
					case I_AND_UIQ:
					case I_AND_WI: p[2]->ins_code = X_TAND_WI; break;
					default: abort();
					}
				} else {
					switch (p[2]->ins_code) {
					case I_LD_WP:  p[2]->ins_code = X_NOT_WP; break;
					case I_LD_WM:  p[2]->ins_code = X_NOT_WM; break;
					case X_LD_WS:  p[2]->ins_code = X_NOT_WS; break;
					case X_LD_WAR: p[2]->ins_code = X_NOT_WAR; break;
					case I_LD_BP:
					case I_LD_UP:  p[2]->ins_code = X_NOT_UP; break;
					case I_LD_BM:
					case I_LD_UM:  p[2]->ins_code = X_NOT_UM; break;
					case X_LD_BS:
					case X_LD_US:  p[2]->ins_code = X_NOT_US; break;
					case X_LD_BAR:
					case X_LD_UAR: p[2]->ins_code = X_NOT_UAR; break;
					case X_LD_BAY:
					case X_LD_UAY: p[2]->ins_code = X_NOT_UAY; break;
					case I_AND_UIQ:
					case I_AND_WI: p[2]->ins_code = X_NAND_WI; break;
					default: abort();
					}
				}
				*p[1] = *p[0];
				nb = 1;
			}

#if OPT_ARRAY_RD
			/*
			 *  __add.wi		array	-->	__ld.{b/u}ar	array
			 *  __st.wm		_ptr
			 *  __ld.{b/u}p		_ptr
			 *
			 *  Classic base+offset byte-array read.
			 *
			 *  N.B. The word-array peephole rule is further up in this file.
			 */
			else if
			((p[0]->ins_code == I_LD_BP ||
			  p[0]->ins_code == I_LD_UP) &&
			 (p[0]->ins_type == T_PTR) &&
			 (p[1]->ins_code == I_ST_WM) &&
			 (p[1]->ins_type == T_PTR) &&
			 (p[2]->ins_code == I_ADD_WI) &&
			 (p[2]->ins_type == T_SYMBOL) &&
			 (is_small_array((SYMBOL *)p[2]->ins_data))
			) {
				/* replace code */
				p[2]->ins_code = (p[0]->ins_code == I_LD_BP) ? X_LD_BAR : X_LD_UAR;
				nb = 2;
			}
#endif

#if OPT_ARRAY_WR
			/*
			 *  __index.{w/u}r	array	-->	__ldp.{w/b/u}ar  array
			 *  __st.wm		_ptr
			 *  __ld.{w/b/u}p	_ptr
			 *
			 *  Optimized base+offset array write.
			 *
			 *  This MUST be enabled when the X_INDEX_WR / X_INDEX_UR
			 *  optimization is enabled, or array loads break because
			 *  the index is put into __ptr instead of an address!
			 */
			else if
			((p[0]->ins_code == I_LD_WP ||
			  p[0]->ins_code == I_LD_BP ||
			  p[0]->ins_code == I_LD_UP) &&
			 (p[0]->ins_type == T_PTR) &&
			 (p[1]->ins_code == I_ST_WM) &&
			 (p[1]->ins_type == T_PTR) &&
			 (p[2]->ins_code == X_INDEX_WR ||
			  p[2]->ins_code == X_INDEX_UR)
			) {
				/* replace code */
				if (p[0]->ins_code == I_LD_WP)
					p[2]->ins_code = X_LDP_WAR;
				else
					p[2]->ins_code = (p[0]->ins_code == I_LD_BP) ? X_LDP_BAR : X_LDP_UAR;
				nb = 2;
			}
#endif

			/* flush queue */
			if (nb) {
				q_wr -= nb;
				q_nb -= nb;
				nb = 0;

				if (q_wr < 0)
					q_wr += Q_SIZE;

				/* loop */
				goto lv1_loop;
			}
		}

		/* ********************************************************* */
		/* 2-instruction patterns */
		/* ********************************************************* */

		if (q_nb >= 2) {
			/*
			 *  __ld.{b/u}p		__ptr	-->	__ld.{b/u}p	__ptr
			 *  __switch.wr				__switch.ur
			 *
			 *  __ld.{b/u}m		symbol	-->	__ld.{b/u}m	symbol
			 *  __switch.wr				__switch.ur
			 *
			 *  __ld.{b/u}s		n	-->	__ld.{b/u}s	n
			 *  __switch.wr				__switch.ur
			 *
			 *  __ld.{b/u}ar	array	-->	__ld.{b/u}ar	array
			 *  __switch.wr				__switch.ur
			 */
			if
			((p[0]->ins_code == I_SWITCH_WR) &&
			 (p[1]->ins_code == I_LD_BP ||
			  p[1]->ins_code == I_LD_UP ||
			  p[1]->ins_code == I_LD_BM ||
			  p[1]->ins_code == I_LD_UM ||
			  p[1]->ins_code == X_LD_BS ||
			  p[1]->ins_code == X_LD_US ||
			  p[1]->ins_code == X_LD_BAR ||
			  p[1]->ins_code == X_LD_UAR)
			) {
				/* optimize code */
				p[0]->ins_code = I_SWITCH_UR;
				nb = 0;
			}

			/*
			 *  __switch.{w/u}r		-->	__switch.{w/u}rw
			 *  __endcase
			 *
			 *  __bra LLnn			-->	__bra LLnn
			 *  __endcase
			 *
			 *  I_ENDCASE is only generated in order to catch which
			 *  case statements could fall through to the next case
			 *  so that an SAX instruction could be inserted, which
			 *  is only needed *IF* "doswitch" uses "JMP table, x".
			 *
			 *  This removes obviously-redundant I_ENDCASE i-codes.
			 */
			else if
			((p[0]->ins_code == I_ENDCASE) &&
			 (p[1]->ins_code == I_SWITCH_WR ||
			  p[1]->ins_code == I_SWITCH_UR ||
			  p[1]->ins_code == I_BRA)
			) {
				/* remove code */
				nb = 1;
			}

			/*
			 *  __modsp		i	-->	__modsp		(i + j)
			 *  __modsp		j
			 */
			else if
			((p[0]->ins_code == I_MODSP) &&
			 (p[1]->ins_code == I_MODSP) &&

			 (p[0]->ins_type == T_STACK) &&
			 (p[1]->ins_type == T_STACK)
			) {
				/* replace code */
				p[1]->ins_data += p[0]->ins_data;
				nb = 1;
			}

			/*
			 *  __lea.s		i	-->	__lea.s		(i + j)
			 *  __add.wi		j
			 *
			 *  This is generated for address calculations into local
			 *  arrays and structs on the stack.
			 */
			else if
			((p[0]->ins_code == I_ADD_WI) &&
			 (p[0]->ins_type == T_VALUE) &&
			 (p[1]->ins_code == I_LEA_S)
			) {
				/* replace code */
				p[1]->ins_code = I_LEA_S;
				p[1]->ins_data += p[0]->ins_data;
				nb = 1;
			}

			/*
			 *  __add.wi		i	-->	__add.wi	(i + j)
			 *  __add.wi		j
			 */
			else if
			((p[0]->ins_code == I_ADD_WI) &&
			 (p[0]->ins_type == T_VALUE) &&
			 (p[1]->ins_code == I_ADD_WI) &&
			 (p[1]->ins_type == T_VALUE)
			) {
				/* replace code */
				p[1]->ins_data += p[0]->ins_data;
				nb = 1;
			}

			/*
			 *  __asl.wi		i	-->	__asl.wi	(i+1)
			 *  __asl.wr
			 *
			 *  __asl.wi		i	-->	__asl.wi	(i+j)
			 *  __asl.wi		j
			 *
			 *  __asl.wr			-->	__asl.wi	(1+j)
			 *  __asl.wi		j
			 *
			 *  __asl.wr			-->	__asl.wi	(1+1)
			 *  __asl.wr
			 *
			 *  sometimes generated for the address within an array of structs
			 */
			else if
			((p[0]->ins_code == I_ASL_WR ||
			  p[0]->ins_code == I_ASL_WI) &&
			 (p[1]->ins_code == I_ASL_WR ||
			  p[1]->ins_code == I_ASL_WI)
			) {
				/* replace code */
				intptr_t data = 0;
				data += (p[0]->ins_code == I_ASL_WR) ? 1 : p[0]->ins_data;
				data += (p[1]->ins_code == I_ASL_WR) ? 1 : p[1]->ins_data;
				p[1]->ins_code = I_ASL_WI;
				p[1]->ins_type = T_VALUE;
				p[1]->ins_data = data;
				nb = 1;
			}

			/*
			 *  __ld.wi		symbol	-->	__ld.wi		(symbol + j)
			 *  __add.wi		j
			 *
			 *  __add.wi		symbol	-->	__add.wi	(symbol + j)
			 *  __add.wi		j
			 */
			else if
			((p[0]->ins_code == I_ADD_WI) &&
			 (p[1]->ins_code == I_LD_WI ||
			  p[1]->ins_code == I_ADD_WI) &&

			 (p[0]->ins_type == T_VALUE) &&
			 (p[1]->ins_type == T_SYMBOL)
			) {
				/* replace code */
				if (p[0]->ins_data != 0) {
					SYMBOL * oldsym = (SYMBOL *)p[1]->ins_data;
					SYMBOL * newsym = copysym(oldsym);
					if (NAMEALLOC <=
						snprintf(newsym->name, NAMEALLOC, "%s + %ld", oldsym->name, (long) p[0]->ins_data))
						error("optimized symbol+offset name too long");
					p[1]->ins_type = T_SYMBOL;
					p[1]->ins_data = (intptr_t)newsym;
				}
				nb = 1;
			}

			/*
			 *  __ld.wi		j	-->	__ld.wi		(symbol + j)
			 *  __add.wi		symbol
			 */
			else if
			((p[0]->ins_code == I_ADD_WI) &&
			 (p[1]->ins_code == I_LD_WI) &&

			 (p[0]->ins_type == T_SYMBOL) &&
			 (p[1]->ins_type == T_VALUE)
			) {
				/* replace code */
				if (p[1]->ins_data != 0) {
					SYMBOL * oldsym = (SYMBOL *)p[0]->ins_data;
					SYMBOL * newsym = copysym(oldsym);
					if (NAMEALLOC <=
						snprintf(newsym->name, NAMEALLOC, "%s + %ld", oldsym->name, (long) p[1]->ins_data))
						error("optimized symbol+offset name too long");
					p[1]->ins_type = T_SYMBOL;
					p[1]->ins_data = (intptr_t)newsym;
				} else {
					*p[1] = *p[0];
					p[1]->ins_code = I_LD_WI;
				}
				nb = 1;
			}

			/*
			 *  __st.wm		a	-->	__st.wm		a
			 *  __ld.wm		a
			 */
			else if
			((p[0]->ins_code == I_LD_WM) &&
			 (p[1]->ins_code == I_ST_WM) &&
			 (cmp_operands(p[0], p[1]) == 1)
			) {
				/* remove code */
				nb = 1;
			}

			/*
			 *  __st.ws		i	-->	__st.ws i
			 *  __ld.ws		i
			 */
			else if
			((p[0]->ins_code == X_LD_WS) &&
			 (p[1]->ins_code == X_ST_WS) &&
			 (p[0]->ins_data == p[1]->ins_data)) {
				/* remove code */
				nb = 1;
			}

			/*
			 *  __st.us		i	-->	__st.us i
			 *  __ld.us		i
			 */
			else if
			((p[0]->ins_code == X_LD_BS ||
			  p[0]->ins_code == X_LD_US) &&
			 (p[1]->ins_code == X_ST_US) &&
			 (p[0]->ins_data == p[1]->ins_data)
			) {
				if (p[0]->ins_code == X_LD_BS)
					p[0]->ins_code = I_EXT_BR;
				else
					p[0]->ins_code = I_EXT_UR;
				p[0]->ins_data = p[0]->ins_type = 0;
			}

			/*
			 *  __ld.wm a (or __ld.wi a)	-->	__ld.wm b (or __ld.wi b)
			 *  __ld.wm b (or __ld.wi b)
			 *
			 *  JCB: Orphaned load, does this really happen?
			 */
			else if
			((p[0]->ins_code == I_LD_WM ||
			  p[0]->ins_code == I_LD_WI ||
			  p[0]->ins_code == X_LD_WS ||
			  p[0]->ins_code == I_LEA_S ||
			  p[0]->ins_code == I_LD_BM ||
			  p[0]->ins_code == I_LD_BP ||
			  p[0]->ins_code == X_LD_BS ||
			  p[0]->ins_code == I_LD_UM ||
			  p[0]->ins_code == I_LD_UP ||
			  p[0]->ins_code == X_LD_US) &&
			 (p[1]->ins_code == I_LD_WM ||
			  p[1]->ins_code == I_LD_WI ||
			  p[1]->ins_code == X_LD_WS ||
			  p[1]->ins_code == I_LEA_S ||
			  p[1]->ins_code == I_LD_BM ||
			  p[1]->ins_code == I_LD_BP ||
			  p[1]->ins_code == X_LD_BS ||
			  p[1]->ins_code == I_LD_UM ||
			  p[1]->ins_code == I_LD_UP ||
			  p[1]->ins_code == X_LD_US)
			) {
				/* remove code */
				*p[1] = *p[0];
				nb = 1;
			}

			/*
			 *  __decld.{w/b/u}m	symbol	-->	__lddec.{w/b/u}m  symbol
			 *  __add.wi  1
			 *
			 *  __decld.{w/b/u}s	n	-->	__lddec.{w/b/u}s  n
			 *  __add.wi  1
			 *
			 *  __decld.{w/b/u}ar	array	-->	__lddec.{w/b/u}ar array
			 *  __add.wi  1
			 *
			 *  __decld.{b/u}ay	array	-->	__lddec.{b/u}ay array
			 *  __add.wi  1
			 *
			 *  C post-decrement!
			 */
			else if
			((p[0]->ins_code == I_ADD_WI) &&
			 (p[0]->ins_type == T_VALUE) &&
			 (p[0]->ins_data == 1) &&
			 (p[1]->ins_code == X_DECLD_WM ||
			  p[1]->ins_code == X_DECLD_BM ||
			  p[1]->ins_code == X_DECLD_UM ||
			  p[1]->ins_code == X_DECLD_WS ||
			  p[1]->ins_code == X_DECLD_BS ||
			  p[1]->ins_code == X_DECLD_US ||
			  p[1]->ins_code == X_DECLD_WAR ||
			  p[1]->ins_code == X_DECLD_BAR ||
			  p[1]->ins_code == X_DECLD_UAR ||
			  p[1]->ins_code == X_DECLD_BAY ||
			  p[1]->ins_code == X_DECLD_UAY)
			) {
				/* replace code */
				switch (p[1]->ins_code) {
				case X_DECLD_WM: p[1]->ins_code = X_LDDEC_WM; break;
				case X_DECLD_BM: p[1]->ins_code = X_LDDEC_BM; break;
				case X_DECLD_UM: p[1]->ins_code = X_LDDEC_UM; break;
				case X_DECLD_WS: p[1]->ins_code = X_LDDEC_WS; break;
				case X_DECLD_BS: p[1]->ins_code = X_LDDEC_BS; break;
				case X_DECLD_US: p[1]->ins_code = X_LDDEC_US; break;
				case X_DECLD_WAR: p[1]->ins_code = X_LDDEC_WAR; break;
				case X_DECLD_BAR: p[1]->ins_code = X_LDDEC_BAR; break;
				case X_DECLD_UAR: p[1]->ins_code = X_LDDEC_UAR; break;
				case X_DECLD_BAY: p[1]->ins_code = X_LDDEC_BAY; break;
				case X_DECLD_UAY: p[1]->ins_code = X_LDDEC_UAY; break;
				default:	break;
				}
				nb = 1;
			}

			/*
			 *  __incld.{w/b/u}m	symbol	-->	__ldinc.{w/b/u}m  symbol
			 *  __sub.wi  1
			 *
			 *  __incld.{w/b/u}s	n	-->	__ldinc.{w/b/u}s  n
			 *  __sub.wi  1
			 *
			 *  __incld.{w/b/u}ar	array	-->	__ldinc.{w/b/u}ar array
			 *  __sub.wi  1
			 *
			 *  __incld.{w/b/u}ay	array	-->	__ldinc.{w/b/u}ay array
			 *  __sub.wi  1
			 *
			 *  C post-increment!
			 */
			else if
			((p[0]->ins_code == I_SUB_WI) &&
			 (p[0]->ins_type == T_VALUE) &&
			 (p[0]->ins_data == 1) &&
			 (p[1]->ins_code == X_INCLD_WM ||
			  p[1]->ins_code == X_INCLD_BM ||
			  p[1]->ins_code == X_INCLD_UM ||
			  p[1]->ins_code == X_INCLD_WS ||
			  p[1]->ins_code == X_INCLD_BS ||
			  p[1]->ins_code == X_INCLD_US ||
			  p[1]->ins_code == X_INCLD_WAR ||
			  p[1]->ins_code == X_INCLD_BAR ||
			  p[1]->ins_code == X_INCLD_UAR ||
			  p[1]->ins_code == X_INCLD_BAY ||
			  p[1]->ins_code == X_INCLD_UAY)
			) {
				/* replace code */
				switch (p[1]->ins_code) {
				case X_INCLD_WM: p[1]->ins_code = X_LDINC_WM; break;
				case X_INCLD_BM: p[1]->ins_code = X_LDINC_BM; break;
				case X_INCLD_UM: p[1]->ins_code = X_LDINC_UM; break;
				case X_INCLD_WS: p[1]->ins_code = X_LDINC_WS; break;
				case X_INCLD_BS: p[1]->ins_code = X_LDINC_BS; break;
				case X_INCLD_US: p[1]->ins_code = X_LDINC_US; break;
				case X_INCLD_WAR: p[1]->ins_code = X_LDINC_WAR; break;
				case X_INCLD_BAR: p[1]->ins_code = X_LDINC_BAR; break;
				case X_INCLD_UAR: p[1]->ins_code = X_LDINC_UAR; break;
				case X_INCLD_BAY: p[1]->ins_code = X_LDINC_BAY; break;
				case X_INCLD_UAY: p[1]->ins_code = X_LDINC_UAY; break;
				default:	break;
				}
				nb = 1;
			}

			/*
			 *  __ld.{w/b/u}m	symbol	-->	__ld.{w/b/u}mq	symbol
			 *  __index.wr		array		__index.wr	array
			 *
			 *  __ld.{w/b/u}m	symbol	-->	__ld.{w/b/u}mq	symbol
			 *  __index.ur		array		__index.ur	array
			 *
			 *  __ld.{w/b/u}m	symbol	-->	__ld.{w/b/u}mq	symbol
			 *  __ld.war		array		__ld.war	array
			 *
			 *
			 *  __ld.{w/b/u}s	n	-->	__ld.{w/b/u}sq	n
			 *  __index.wr		array		__index.wr	array
			 *
			 *  __ld.{w/b/u}s	n	-->	__ld.{w/b/u}sq	n
			 *  __index.ur		array		__index.ur	array
			 *
			 *  __ld.{w/b/u}s	n	-->	__ld.{w/b/u}sq	n
			 *  __ld.war		array		__ld.war	array
			 *
			 *  Index optimizations for base+offset array access.
			 */
			else if
			((p[0]->ins_code == X_INDEX_WR ||
			  p[0]->ins_code == X_INDEX_UR ||
			  p[0]->ins_code == X_LD_WAR) &&
			 (p[1]->ins_code == I_LD_WM ||
			  p[1]->ins_code == I_LD_BM ||
			  p[1]->ins_code == I_LD_UM ||
			  p[1]->ins_code == X_LD_WS ||
			  p[1]->ins_code == X_LD_BS ||
			  p[1]->ins_code == X_LD_US)
			) {
				/* replace code */
				switch (p[1]->ins_code) {
				case I_LD_WM: p[1]->ins_code = I_LD_WMQ; break;
				case I_LD_BM: p[1]->ins_code = I_LD_BMQ; break;
				case I_LD_UM: p[1]->ins_code = I_LD_UMQ; break;
				case X_LD_WS: p[1]->ins_code = X_LD_WSQ; break;
				case X_LD_BS: p[1]->ins_code = X_LD_BSQ; break;
				case X_LD_US: p[1]->ins_code = X_LD_USQ; break;
				default:	break;
				}
				nb = 0;
			}

			/*
			 *  __ld.{w/b/u}m	symbol	-->	__ldy.{w/b/u}mq
			 *  __ld.{b/u}ar	array		__ld.{b/u}ay	array
			 *
			 *  __ld.{w/b/u}s	n	-->	__ldy.{w/b/u}sq	n
			 *  __ld.{b/u}ar	array		__ld.{b/u}ay	array
			 *
			 *  Index optimizations for base+offset array access.
			 */
			else if
			((p[0]->ins_code == X_LD_BAR ||
			  p[0]->ins_code == X_LD_UAR) &&
			 (p[1]->ins_code == I_LD_WM ||
			  p[1]->ins_code == I_LD_BM ||
			  p[1]->ins_code == I_LD_UM ||
			  p[1]->ins_code == X_LD_WS ||
			  p[1]->ins_code == X_LD_BS ||
			  p[1]->ins_code == X_LD_US)
			) {
				/* replace code */
				switch (p[1]->ins_code) {
				case I_LD_WM: p[1]->ins_code = I_LDY_WMQ; break;
				case I_LD_BM: p[1]->ins_code = I_LDY_BMQ; break;
				case I_LD_UM: p[1]->ins_code = I_LDY_UMQ; break;
				case X_LD_WS: p[1]->ins_code = X_LDY_WSQ; break;
				case X_LD_BS: p[1]->ins_code = X_LDY_BSQ; break;
				case X_LD_US: p[1]->ins_code = X_LDY_USQ; break;
				default:	break;
				}
				switch (p[0]->ins_code) {
				case X_LD_BAR: p[0]->ins_code = X_LD_BAY; break;
				case X_LD_UAR: p[0]->ins_code = X_LD_UAY; break;
				case X_LDP_BAR: p[0]->ins_code = X_LDP_BAY; break;
				case X_LDP_UAR: p[0]->ins_code = X_LDP_UAY; break;
				default:	break;
				}
				nb = 0;
			}

			/*
			 *  __ld.{w/b/u}mq	symbol	-->	__ldy.{w/b/u}mq
			 *  __ldp.{b/u}ar	array		__ldp.{b/u}ay	array
			 *
			 *  __ld.{w/b/u}sq	n	-->	__ldy.{w/b/u}sq	n
			 *  __ldp.{b/u}ar	array		__ldp.{b/u}ay	array
			 *
			 *  Index optimizations for base+offset array access.
			 *
			 *  The X_INDEX_UR rule above already converted __ld.{w/b/u}m
			 *  to __ld.{w/b/u}mq!
			 */
			else if
			((p[0]->ins_code == X_LDP_BAR ||
			  p[0]->ins_code == X_LDP_UAR) &&
			 (p[1]->ins_code == I_LD_WMQ ||
			  p[1]->ins_code == I_LD_BMQ ||
			  p[1]->ins_code == I_LD_UMQ ||
			  p[1]->ins_code == X_LD_WSQ ||
			  p[1]->ins_code == X_LD_BSQ ||
			  p[1]->ins_code == X_LD_USQ)
			) {
				/* replace code */
				switch (p[1]->ins_code) {
				case I_LD_WMQ: p[1]->ins_code = I_LDY_WMQ; break;
				case I_LD_BMQ: p[1]->ins_code = I_LDY_BMQ; break;
				case I_LD_UMQ: p[1]->ins_code = I_LDY_UMQ; break;
				case X_LD_WSQ: p[1]->ins_code = X_LDY_WSQ; break;
				case X_LD_BSQ: p[1]->ins_code = X_LDY_BSQ; break;
				case X_LD_USQ: p[1]->ins_code = X_LDY_USQ; break;
				default:	break;
				}
				switch (p[0]->ins_code) {
				case X_LD_BAR: p[0]->ins_code = X_LD_BAY; break;
				case X_LD_UAR: p[0]->ins_code = X_LD_UAY; break;
				case X_LDP_BAR: p[0]->ins_code = X_LDP_BAY; break;
				case X_LDP_UAR: p[0]->ins_code = X_LDP_UAY; break;
				default:	break;
				}
				nb = 0;
			}

			/*
			 *  __ld.u{p/m/s/ar/ay}	symbol	-->	__ld.u{p/m/s/ar/ay}  symbol
			 *  __sdiv.wi		i		__udiv.wi	i
			 *
			 *  __ld.u{p/m/s/ar/ay}	symbol	-->	__ld.u{p/m/s/ar/ay}  symbol
			 *  __smod.wi		i		__umod.wi	i
			 *
			 *  C promotes an unsigned char to a signed int so this
			 *  must be done in the peephole, not the compiler.
			 */
			else if
			((p[0]->ins_code == I_SDIV_WI ||
			  p[0]->ins_code == I_SMOD_WI) &&
			 (p[0]->ins_type == T_VALUE) &&
			 (p[0]->ins_data >= 0) &&
			 (is_ubyte(p[1]))
			) {
				/* replace code */
				if (p[0]->ins_code == I_SDIV_WI)
					p[0]->ins_code = (p[0]->ins_data < 256) ? I_UDIV_UI : I_UDIV_WI;
				else
					p[0]->ins_code = (p[0]->ins_data < 256) ? I_UMOD_UI : I_UMOD_WI;
				/* no instructions removed, just loop */
				goto lv1_loop;
			}

			/*
			 *  __ld.u{p/m/s/ar/ay}	symbol	-->	__ld.u{p/m/s/ar/ay}  symbol
			 *  __asr.wi		i		__lsr.uiq	i
			 *
			 *  __ld.u{p/m/s/ar/ay}	symbol	-->	__ld.u{p/m/s/ar/ay}  symbol
			 *  __lsr.wi		i		__lsr.uiq	i
			 *
			 *  C promotes an unsigned char to a signed int so this
			 *  must be done in the peephole, not the compiler.
			 */
			else if
			((p[0]->ins_code == I_ASR_WI ||
			  p[0]->ins_code == I_LSR_WI) &&
			 (p[0]->ins_type == T_VALUE) &&
			 (is_ubyte(p[1]))
			) {
				/* replace code */
				if (p[0]->ins_data >= 8) {
					p[1]->ins_code = I_LD_WI;
					p[1]->ins_type = T_VALUE;
					p[1]->ins_data = 0;
					nb = 1;
				} else {
					p[0]->ins_code = I_LSR_UIQ;
					/* no instructions removed, just loop */
					goto lv1_loop;
				}
			}

			/*
			 *  __ld.u{p/m/s/ar/ay}	symbol	-->	__ld.u{p/m/s/ar/ay}  symbol
			 *  __and.wi		i		__and.uiq	i
			 *
			 *  C promotes an unsigned char to a signed int so this
			 *  must be done in the peephole, not the compiler.
			 */
			else if
			((p[0]->ins_code == I_AND_WI) &&
			 (p[0]->ins_type == T_VALUE) &&
			 (p[0]->ins_data >= 0) &&
			 (p[0]->ins_data <= 255) &&
			 (is_ubyte(p[1]))
			) {
				/* replace code */
				p[0]->ins_code = I_AND_UIQ;
				/* no instructions removed, just loop */
				goto lv1_loop;
			}

			/*
			 *  __ld.u{p/m/s/ar/ay}	symbol	-->	__ld.u{p/m/s/ar/ay}  symbol
			 *  __cmp_w.wi		j		__cmp_b.uiq	j
			 *
			 *  __and.uiq		i	-->	__and.uiq	i
			 *  __cmp_w.wi		j		__cmp_b.uiq	j
			 *
			 *  C promotes an unsigned char to a signed int so this
			 *  must be done in the peephole, not the compiler.
			 */
			else if
			((p[0]->ins_code == X_CMP_WI) &&
			 (p[0]->ins_type == T_VALUE) &&
			 (p[0]->ins_data >= 0) &&
			 (p[0]->ins_data <= 255) &&
			 (is_ubyte(p[1]))
			) {
				/* replace code */
				p[0]->ins_code = X_CMP_UIQ;
				p[0]->cmp_type = compare2uchar[p[0]->cmp_type];
				/* no instructions removed, just loop */
				goto lv1_loop;
			}

			/* flush queue */
			if (nb) {
				q_wr -= nb;
				q_nb -= nb;
				nb = 0;

				if (q_wr < 0)
					q_wr += Q_SIZE;

				/* loop */
				goto lv1_loop;
			}
		}

		/* ********************************************************* */
		/* 1-instruction patterns */
		/* ********************************************************* */

		if (q_nb >= 1) {
			/*
			 *  __add.wi		0	-->
			 *
			 *  arg_to_fptr() leaves a useless I_ADD_WI behind when
			 *  generating an I_FARPTR_I for an "array+n" parameter
			 *
			 *  __sub.wi		0	-->
			 *
			 *  might as well check for this too, while we're here
			 */
			if
			((p[0]->ins_code == I_ADD_WI ||
			  p[0]->ins_code == I_SUB_WI) &&
			 (p[0]->ins_type == T_VALUE) &&
			 (p[0]->ins_data == 0)
			) {
				nb = 1;
			}

			/* flush queue */
			if (nb) {
				q_wr -= nb;
				q_nb -= nb;
				nb = 0;

				if (q_wr < 0)
					q_wr += Q_SIZE;

				/* loop */
				goto lv1_loop;
			}
		}
	}

	/*
	 * ********************************************************************
	 * optimization level 2 - instruction re-scheduler,
	 * ********************************************************************
	 *
	 * change the instruction order to allow for direct assignments rather
	 * than the stack-based assignments that are generated by complex math
	 * or things like "var++" that are not covered by the simpler peephole
	 * rules earlier.
	 *
	 * this covers a bunch of math with immediate integers ...
	 *
	 *  __ld.wi			i	-->	...
	 *  __push.wr					__{add/sub}.wi	i
	 *    ...
	 *  __{add/sub}.wt
	 *
	 * this covers storing to global and static variables ...
	 *
	 *  __ld.wi			symbol	-->	...
	 *  __push.wr					__st.{w/u}m	symbol
	 *    ...
	 *  __st.{w/u}pt
	 *
	 * this covers storing to local variables ...
	 *
	 *  __lea.s			n	-->	...
	 *  __push.wr					__st.{w/u}s	n
	 *    ...
	 *  __st.{w/u}pt
	 *
	 * this covers storing to global and static arrays with "=" ...
	 *
	 *  __asl.wr				-->	__index.wr	array
	 *  __add.wi			array		...
	 *  __push.wr					__st.wat	array
	 *    ...
	 *  __st.wpt
	 *
	 *  __add.wi			array	-->	__index.ur	array
	 *  __push.wr					...
	 *    ...					__st.uat	array
	 *  __st.upt
	 *
	 * this covers storing to global and static arrays with "+=", "-=", etc ...
	 *
	 *  __asl.wr				-->	__ldp.war	array
	 *  __add.wi			array		...
	 *  __push.wr					__st.wat	array
	 *  __st.wm			__ptr
	 *  __ld.wp			__ptr
	 *    ...
	 *  __st.wpt
	 *
	 *  __add.wi			array	-->	__ldp.uar	array
	 *  __push.wr					...
	 *  __st.wm			__ptr		__st.uat	array
	 *  __ld.up			__ptr
	 *    ...
	 *  __st.upt
	 */
	if (optimize >= 2) {
		int offset;
		int copy, scan, prev;

		/* check last instruction */
		if (q_nb > 1 &&
		(q_ins[q_wr].ins_code == I_ST_WPT ||
		 q_ins[q_wr].ins_code == I_ST_UPT ||
		 q_ins[q_wr].ins_code == I_ADD_WT ||
		 q_ins[q_wr].ins_code == I_SUB_WT ||
		 q_ins[q_wr].ins_code == I_AND_WT ||
		 q_ins[q_wr].ins_code == I_EOR_WT ||
		 q_ins[q_wr].ins_code == I_OR_WT ||
		 q_ins[q_wr].ins_code == I_MUL_WT)
		) {
			/* browse back the instruction list and
			 * establish a stack history
			 */
			offset = 2;

			for (copy = 1, scan = q_wr; copy < q_nb; copy++) {
				scan -= 1;

				if (scan < 0)
					scan += Q_SIZE;

				/* index of insn preceding scan */
				prev = scan - 1;
				if (prev < 0)
					prev += Q_SIZE;

				/* check instruction */
				switch (q_ins[scan].ins_code) {
				case I_MODSP:
					if ((q_ins[scan].ins_type == T_STACK) ||
					    (q_ins[scan].ins_type == T_NOP))
						offset += (int)q_ins[scan].ins_data;
					break;

				case I_POP_WR:
				case I_CMP_WT:
				case I_ST_WPT:
				case I_ST_UPT:
				case I_ADD_WT:
				case I_SUB_WT:
				case I_AND_WT:
				case I_EOR_WT:
				case I_OR_WT:
				case I_ASL_WT:
				case I_ASR_WT:
				case I_LSR_WT:
				case I_MUL_WT:
				case I_SDIV_WT:
				case I_UDIV_WT:
				case I_SMOD_WT:
				case I_UMOD_WT:
					offset += 2;
					break;

				case I_PUSH_WR:
					offset -= 2;
					break;
				default:
					break;
				}

				/* check offset */
				if (offset == 0) {
					/*
					 * found the I_PUSH_WR that matches the I_ST_WPT
					 */
					int from = scan + 1; /* begin copying after the I_PUSH_WR */
					int drop = 2; /* drop I_PUSH_WR and the i-code before it */

					if (copy == 1) {
						/* hmm, may be not...
						 * there should be at least one instruction
						 * between I_PUSH_WR and I_ST_WPT.
						 * this case should never happen, though,
						 * but better skipping it
						 */
						break;
					}

					/*
					 * check the first instruction
					 */
					{
						/*
						 * only handle sequences that start with an
						 * I_PUSH_WR preceded by I_LEA_S/I_LD_WI/I_ADD_WI
						 */
						if (q_ins[scan].ins_code != I_PUSH_WR)
							break;

#if OPT_ARRAY_WR
						if (q_ins[prev].ins_code != I_LD_WI &&
						    q_ins[prev].ins_code != I_LEA_S &&
						    q_ins[prev].ins_code != I_ADD_WI)
							break;
#else
						if (q_ins[prev].ins_code != I_LD_WI &&
						    q_ins[prev].ins_code != I_LEA_S)
							break;
#endif

						if (q_ins[prev].ins_code != I_LD_WI &&
						    q_ins[q_wr].ins_code != I_ST_WPT &&
						    q_ins[q_wr].ins_code != I_ST_UPT)
							break;

						/* change __st.wpt into __st.w{m/s} */
						if (q_ins[prev].ins_code == I_LD_WI) {
							switch (q_ins[q_wr].ins_code) {
							case I_ST_WPT:
								q_ins[q_wr].ins_code = I_ST_WM;
								break;
							case I_ST_UPT:
								q_ins[q_wr].ins_code = I_ST_UM;
								break;
							case I_ADD_WT:
								q_ins[q_wr].ins_code = I_ADD_WI;
								break;
							case I_SUB_WT:
								q_ins[q_wr].ins_code = I_ISUB_WI;
								break;
							case I_AND_WT:
								q_ins[q_wr].ins_code = I_AND_WI;
								break;
							case I_EOR_WT:
								q_ins[q_wr].ins_code = I_EOR_WI;
								break;
							case I_OR_WT:
								q_ins[q_wr].ins_code = I_OR_WI;
								break;
							case I_MUL_WT:
								q_ins[q_wr].ins_code = I_MUL_WI;
								break;
							default:
								abort();
							}
							/* use data from the preceding I_LD_WI */
							q_ins[q_wr].ins_type = q_ins[prev].ins_type;
							q_ins[q_wr].ins_data = q_ins[prev].ins_data;
						} else
						if (q_ins[prev].ins_code == I_LEA_S) {
							if (q_ins[q_wr].ins_code == I_ST_WPT)
								q_ins[q_wr].ins_code = X_ST_WS;
							else
								q_ins[q_wr].ins_code = X_ST_US;
							/* use data from the preceding I_LEA_S */
							q_ins[q_wr].ins_type = q_ins[prev].ins_type;
							q_ins[q_wr].ins_data = q_ins[prev].ins_data;
							q_ins[q_wr].sym = q_ins[prev].sym;
#if OPT_ARRAY_WR

						} else {
							int push = X_INDEX_UR;
							int code = X_ST_UAT;

							/* make sure that I_ADD_WI is really a short array */
							if (q_ins[prev].ins_type != T_SYMBOL ||
							    !is_small_array((SYMBOL *)q_ins[prev].ins_data))
								break;

							copy = copy + 1;
							from = scan;
							drop = 1;

							/* make sure that an I_ST_WPT has an I_ASL_WR */
							if (q_ins[q_wr].ins_code == I_ST_WPT) {
								int aslw = prev - 1;
								if (aslw < 0)
									aslw += Q_SIZE;
								if (copy == q_nb || q_ins[aslw].ins_code != I_ASL_WR)
									break;
								drop = 2;
								push = X_INDEX_WR;
								code = X_ST_WAT;
							}

							/* push the index from the preceding I_ADD_WI */
							q_ins[scan].ins_code = push;
							q_ins[scan].ins_type = T_SYMBOL;
							q_ins[scan].ins_data = q_ins[prev].ins_data;

							/* use data from the preceding I_ADD_WI */
							q_ins[q_wr].ins_code = code;
							q_ins[q_wr].ins_type = T_SYMBOL;
							q_ins[q_wr].ins_data = q_ins[prev].ins_data;
#endif
						}
					}

					/*
					 * adjust stack references for the
					 * removal of the I_PUSH_WR
					 */
					for (int temp = copy; temp > 1; temp--) {
						scan += 1;
						if (scan >= Q_SIZE)
							scan -= Q_SIZE;

						/* check instruction */
						if (is_sprel(&q_ins[scan])) {
							/* adjust stack offset */
							q_ins[scan].ins_data -= 2;
						}
					}

					/*
					 * remove all the instructions ...
					 */
					q_nb -= (drop + copy);
					q_wr -= (drop + copy);
					if (q_wr < 0)
						q_wr += Q_SIZE;

					/*
					 * ... and re-insert them one by one
					 * in the queue (for further optimizations)
					 */
					for (; copy > 0; copy--) {
						if (from >= Q_SIZE)
							from -= Q_SIZE;
#ifdef DEBUG_OPTIMIZER
						printf("\nReinserting after rescheduling ...");
#endif
						push_ins(&q_ins[from++]);
					}
					break;
				}
			}
		}

		/*
		 * optimization level 2b - after the instruction re-scheduler
		 */
		if (q_nb >= 3) {
			INS *p[3];
			int i, j;
			int nb = 0;

lv2_loop:
			/* precalculate pointers to instructions */
			for (i = 0, j = q_wr; i < 3; i++) {
				/* save pointer */
				p[i] = &q_ins[j];

				/* next */
				j -= 1;
				if (j < 0)
					j += Q_SIZE;
			}

			/*
			 *  __push.wr			-->	__st.{w/u}pi	i
			 *  __ld.wi		i
			 *  __st.{w/u}pt
			 *
			 *  This cannot be done earlier because it screws up
			 *  the reordering optimization above.
			 *
			 *  JCB: This is optimizing writes though a pointer variable!
			 */
			if
			((p[0]->ins_code == I_ST_WPT ||
			  p[0]->ins_code == I_ST_UPT) &&
			 (p[1]->ins_code == I_LD_WI) &&
			 (p[1]->ins_type == T_VALUE) &&
			 (p[2]->ins_code == I_PUSH_WR)
			) {
				/* replace code */
				p[2]->ins_code = p[0]->ins_code == I_ST_WPT ? I_ST_WPI : I_ST_UPI;
				p[2]->ins_data = p[1]->ins_data;
				nb = 2;
			}

#if 0
			/*
			 *  __push.wr			-->	__st.wm		__ptr
			 *  <load>				<load>
			 *  __st.{w/u}pt			__st.{w/u}p	__ptr
			 *
			 *  This cannot be done earlier because it screws up
			 *  the reordering optimization above.
			 *
			 *  THIS IS VERY RARE, REMOVE IT FOR NOW AND RETHINK IT
			 */
			else if
			((p[0]->ins_code == I_ST_UPT ||
			  p[0]->ins_code == I_ST_WPT) &&
			 (is_load(p[1])) &&
			 (p[2]->ins_code == I_PUSH_WR)
			) {
				p[2]->ins_code = I_ST_WM;
				p[2]->ins_type = T_PTR;
				/* We just removed a push, adjust SP-relative
				   addresses. */
				if (is_sprel(p[1]))
					p[1]->ins_data -= 2;
				if (p[0]->ins_code == I_ST_UPT)
					p[0]->ins_code = I_ST_UP;
				else
					p[0]->ins_code = I_ST_WP;
				q_ins[q_wr].ins_type = T_PTR;
			}
#endif

			/* flush queue */
			if (nb) {
				q_wr -= nb;
				q_nb -= nb;
				nb = 0;

				if (q_wr < 0)
					q_wr += Q_SIZE;

				/* loop */
				goto lv2_loop;
			}
		}
	}
}

/* ----
 * flush_ins_label(int nextlabel)
 * ----
 * flush instruction queue, eliminating redundant trailing branches to a
 * label following immediately
 *
 */
void flush_ins_label (int nextlabel)
{
	while (q_nb) {
		/* skip last op if it's a branch to nextlabel */
		if (q_nb > 1 || nextlabel == -1 ||
		    (q_ins[q_rd].ins_code != I_BRA) ||
		    q_ins[q_rd].ins_data != nextlabel) {
			/* gen code */
			if (arg_stack_flag)
				arg_push_ins(&q_ins[q_rd]);
			else
				gen_code(&q_ins[q_rd]);
		}

		/* advance and wrap queue read pointer */
		q_rd++;
		q_nb--;

		if (q_rd == Q_SIZE)
			q_rd = 0;
	}

	/* reset queue */
	q_rd = 0;
	q_wr = Q_SIZE - 1;
	q_nb = 0;
}

/* ----
 * flush_ins()
 * ----
 * flush instruction queue
 *
 */
void flush_ins (void)
{
	flush_ins_label(-1);
}
