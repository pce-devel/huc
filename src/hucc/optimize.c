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
 /* GCC doesn't like "strcmp((char *)p[0]->data,"! */
 #pragma GCC diagnostic ignored "-Wstringop-overread"
#endif

/* flag information for each of the i-code instructions */
/*
 * N.B. this table MUST be kept updated and in the same order as the i-code
 * enum list in defs.h
 */
unsigned char icode_flags[] = {
	0,

	/* i-code that retires the primary register contents */

	/* I_FENCE              */	0,

	/* i-codes for handling farptr */

	/* I_FARPTR             */	0,
	/* I_FARPTR_I           */	0,
	/* I_FARPTR_GET         */	0,
	/* I_FGETW              */	0,
	/* I_FGETB              */	0,
	/* I_FGETUB             */	0,

	/* i-codes for interrupts */

	/* I_SEI                */	0,
	/* I_CLI                */	0,

	/* i-codes for calling functions */

	/* I_MACRO              */	0,
	/* I_CALL               */	0,
	/* I_CALLP              */	0,
	/* I_JSR                */	0,

	/* i-codes for C functions and the C parameter stack */

	/* I_ENTER              */	0,
	/* I_LEAVE              */	0,
	/* I_GETACC             */	0,
	/* I_SAVESP             */	0,
	/* I_LOADSP             */	0,
	/* I_MODSP              */	0,
	/* I_PUSHW              */	0,
	/* I_POPW               */	0,
	/* I_SPUSHW             */	0,
	/* I_SPUSHB             */	0,
	/* I_SPOPW              */	0,
	/* I_SPOPB              */	0,

	/* i-codes for handling boolean tests and branching */

	/* I_SWITCHW            */	0,
	/* I_SWITCHB            */	0,
	/* I_CASE               */	0,
	/* I_ENDCASE            */	0,
	/* I_LABEL              */	0,
	/* I_ALIAS              */	0,
	/* I_BRA                */	0,
	/* I_DEF                */	0,
	/* I_CMPW               */	0,
	/* I_CMPB               */	0,
	/* X_CMPWI_EQ           */	0,
	/* X_CMPWI_NE           */	0,
	/* I_NOTW               */	0,
	/* I_TSTW               */	0,
	/* I_BFALSE             */	0,
	/* I_BTRUE              */	0,
	/* X_TZB                */	0,
	/* X_TZBP               */	0,
	/* X_TZB_S              */	IS_SPREL,
	/* X_TZW                */	0,
	/* X_TZWP               */	0,
	/* X_TZW_S              */	IS_SPREL,
	/* X_TNZB               */	0,
	/* X_TNZBP              */	0,
	/* X_TNZB_S             */	IS_SPREL,
	/* X_TNZW               */	0,
	/* X_TNZWP              */	0,
	/* X_TNZW_S             */	IS_SPREL,

	/* i-codes for loading the primary register */

	/* I_LDWI               */	0,
	/* I_LEA_S              */	IS_SPREL,

	/* I_LDW                */	0,
	/* I_LDB                */	0,
	/* I_LDUB               */	0,

	/* I_LDWP               */	0,
	/* I_LDBP               */	0,
	/* I_LDUBP              */	0,

	/* X_LDWA_A             */	0,
	/* X_LDBA_A             */	0,
	/* X_LDUBA_A            */	0,

	/* X_LDW_S              */	IS_SPREL,
	/* X_LDB_S              */	IS_SPREL,
	/* X_LDUB_S             */	IS_SPREL,

	/* X_LDPWA_A            */	0,
	/* X_LDPBA_A            */	0,
	/* X_LDPUBA_A           */	0,

	/* i-codes for pre- and post- increment and decrement */

	/* X_INCLD_WM           */	0,
	/* X_INCLD_BM           */	0,
	/* X_INCLD_CM           */	0,

	/* X_DECLD_WM           */	0,
	/* X_DECLD_BM           */	0,
	/* X_DECLD_CM           */	0,

	/* X_LDINC_WM           */	0,
	/* X_LDINC_BM           */	0,
	/* X_LDINC_CM           */	0,

	/* X_LDDEC_WM           */	0,
	/* X_LDDEC_BM           */	0,
	/* X_LDDEC_CM           */	0,

	/* X_INC_WMQ            */	0,
	/* X_INC_CMQ            */	0,

	/* X_DEC_WMQ            */	0,
	/* X_DEC_CMQ            */	0,

	/* X_INCLD_WS           */	IS_SPREL,
	/* X_INCLD_BS           */	IS_SPREL,
	/* X_INCLD_CS           */	IS_SPREL,

	/* X_DECLD_WS           */	IS_SPREL,
	/* X_DECLD_BS           */	IS_SPREL,
	/* X_DECLD_CS           */	IS_SPREL,

	/* X_LDINC_WS           */	IS_SPREL,
	/* X_LDINC_BS           */	IS_SPREL,
	/* X_LDINC_CS           */	IS_SPREL,

	/* X_LDDEC_WS           */	IS_SPREL,
	/* X_LDDEC_BS           */	IS_SPREL,
	/* X_LDDEC_CS           */	IS_SPREL,

	/* X_INC_WSQ            */	IS_SPREL,
	/* X_INC_CSQ            */	IS_SPREL,

	/* X_DEC_WSQ            */	IS_SPREL,
	/* X_DEC_CSQ            */	IS_SPREL,

	/* i-codes for saving the primary register */

	/* I_STWZ               */	0,
	/* I_STBZ               */	0,
	/* I_STWI               */	0,
	/* I_STBI               */	0,
	/* I_STWIP              */	0,
	/* I_STBIP              */	0,
	/* I_STW                */	0,
	/* I_STB                */	0,
	/* I_STWP               */	0,
	/* I_STBP               */	0,
	/* I_STWPS              */	0,
	/* I_STBPS              */	0,
	/* X_STWI_S             */	IS_SPREL,
	/* X_STBI_S             */	IS_SPREL,
	/* X_STW_S              */	IS_SPREL,
	/* X_STB_S              */	IS_SPREL,

	/* X_INDEXW             */	0,
	/* X_INDEXB             */	0,
	/* X_STWAS              */	0,
	/* X_STBAS              */	0,

	/* i-codes for extending the primary register */

	/* I_EXTW               */	0,
	/* I_EXTUW              */	0,

	/* i-codes for math with the primary register  */

	/* I_COMW               */	0,
	/* I_NEGW               */	0,

	/* I_ADDWS              */	0,
	/* I_ADDWI              */	0,
	/* I_ADDW               */	0,
	/* I_ADDUB              */	0,
	/* X_ADDW_S             */	IS_SPREL,
	/* X_ADDUB_S            */	IS_SPREL,

	/* I_ADDBI_P            */	0,

	/* I_SUBWS              */	0,
	/* I_SUBWI              */	0,
	/* I_SUBW               */	0,
	/* I_SUBUB              */	0,

	/* I_ISUBWI             */	0,

	/* I_ANDWS              */	0,
	/* I_ANDWI              */	0,
	/* I_ANDW               */	0,
	/* I_ANDUB              */	0,

	/* I_EORWS              */	0,
	/* I_EORWI              */	0,
	/* I_EORW               */	0,
	/* I_EORUB              */	0,

	/* I_ORWS               */	0,
	/* I_ORWI               */	0,
	/* I_ORW                */	0,
	/* I_ORUB               */	0,

	/* I_ASLWS              */	0,
	/* I_ASLWI              */	0,
	/* I_ASLW               */	0,

	/* I_ASRWI              */	0,
	/* I_ASRW               */	0,

	/* I_LSRWI              */	0,

	/* I_MULWI              */	0,

	/* i-codes for 32-bit longs */

	/* X_LDD_I              */	0,
	/* X_LDD_W              */	0,
	/* X_LDD_B              */	0,
	/* X_LDD_S_W            */	IS_SPREL,
	/* X_LDD_S_B            */	IS_SPREL
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
	if (p1->type != p2->type)
		return (0);

	if (p1->type == T_SYMBOL) {
		if (strcmp(((SYMBOL *)p1->data)->name, ((SYMBOL *)p2->data)->name) != 0)
			return (0);
	}
	else {
		if (p1->data != p2->data)
			return (0);
	}
	return (1);
}

inline bool is_sprel (INS *i)
{
	return (icode_flags[i->code] & IS_SPREL);
}

inline bool is_small_array (SYMBOL *sym)
{
	return (sym->ident == ARRAY && sym->size > 0 && sym->size <= 256);
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
		for (i = 0, j = q_wr; i < nb; i++) {
			/* save pointer */
			p[i] = &q_ins[j];
#ifdef DEBUG_OPTIMIZER
			printf("%d ", i); dump_ins(p[i]);
#endif

			/* next */
			j -= 1;
			if (j < 0)
				j += Q_SIZE;
		}

		/* LEVEL 1 - FUN STUFF STARTS HERE */
		nb = 0;

		/* first check for I_FENCE, then remove it ASAP */
		if (q_nb >= 1 && p[0]->code == I_FENCE) {
			/* remove I_FENCE after it has been checked */
			nb = 1;

			/*
			 *  __getacc			-->
			 *  __fence
			 *
			 *  __addwi / __subwi		-->
			 *  __fence
			 */
			if
			((q_nb >= 2) &&
			 (p[1]->code == I_ADDWI ||
			  p[1]->code == I_SUBWI ||
			  p[1]->code == I_GETACC)
			) {
				nb = 2;
			}

			/*
			 *  __ldinc_wm / __incld_wm	-->	__inc_wm
			 *  __fence
			 *
			 *  __lddec_wm / __decld_wm	-->	__dec_wm
			 *  __fence
			 *
			 *  unused pre-increment or post-increment value
			 *  unused pre-decrement or post-decrement value
			 */
			else if
			((q_nb >= 2) &&
			 (p[1]->code == X_INCLD_WM ||
			  p[1]->code == X_INCLD_BM ||
			  p[1]->code == X_INCLD_CM ||
			  p[1]->code == X_LDINC_WM ||
			  p[1]->code == X_LDINC_BM ||
			  p[1]->code == X_LDINC_CM ||
			  p[1]->code == X_DECLD_WM ||
			  p[1]->code == X_DECLD_BM ||
			  p[1]->code == X_DECLD_CM ||
			  p[1]->code == X_LDDEC_WM ||
			  p[1]->code == X_LDDEC_BM ||
			  p[1]->code == X_LDDEC_CM ||
			  p[1]->code == X_INCLD_WS ||
			  p[1]->code == X_INCLD_BS ||
			  p[1]->code == X_INCLD_CS ||
			  p[1]->code == X_LDINC_WS ||
			  p[1]->code == X_LDINC_BS ||
			  p[1]->code == X_LDINC_CS ||
			  p[1]->code == X_DECLD_WS ||
			  p[1]->code == X_DECLD_BS ||
			  p[1]->code == X_DECLD_CS ||
			  p[1]->code == X_LDDEC_WS ||
			  p[1]->code == X_LDDEC_BS ||
			  p[1]->code == X_LDDEC_CS)
			) {
				/* replace code */
				switch (p[1]->code) {
				case X_INCLD_WM:
				case X_LDINC_WM: p[1]->code = X_INC_WMQ; break;
				case X_INCLD_BM:
				case X_INCLD_CM:
				case X_LDINC_BM:
				case X_LDINC_CM: p[1]->code = X_INC_CMQ; break;
				case X_DECLD_WM:
				case X_LDDEC_WM: p[1]->code = X_DEC_WMQ; break;
				case X_DECLD_BM:
				case X_DECLD_CM:
				case X_LDDEC_BM:
				case X_LDDEC_CM: p[1]->code = X_DEC_CMQ; break;
				case X_INCLD_WS:
				case X_LDINC_WS: p[1]->code = X_INC_WSQ; break;
				case X_INCLD_BS:
				case X_INCLD_CS:
				case X_LDINC_BS:
				case X_LDINC_CS: p[1]->code = X_INC_CSQ; break;
				case X_DECLD_WS:
				case X_LDDEC_WS: p[1]->code = X_DEC_WSQ; break;
				case X_DECLD_BS:
				case X_DECLD_CS:
				case X_LDDEC_BS:
				case X_LDDEC_CS: p[1]->code = X_DEC_CSQ; break;
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

		/* 6-instruction patterns */
		if (q_nb >= 6) {
			/*
			 *  __ldwi  p			-->	__ldw p
			 *  __pushw				__addwi i
			 *  __stw __ptr				__stw p
			 *  __ldwp __ptr
			 *  __addwi i
			 *  __stwps
			 */
			if
			((p[0]->code == I_STWPS) &&
			 (p[1]->code == I_ADDWI ||
			  p[1]->code == I_SUBWI) &&
			 (p[2]->code == I_LDWP) &&
			 (p[2]->type == T_PTR) &&
			 (p[3]->code == I_STW) &&
			 (p[3]->type == T_PTR) &&
			 (p[4]->code == I_PUSHW) &&
			 (p[5]->code == I_LDWI)
			) {
				*p[3] = *p[5];
				p[3]->code = I_STW;
				*p[4] = *p[1];
				p[5]->code = I_LDW;
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

		/* 5-instruction patterns */
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

		/* 4-instruction patterns */
		if (q_nb >= 4) {
			/*  __ldwi i			-->	__ldwi i
			 *  __pushw				__pushw
			 *  __stw   __ptr			__ldw/_ldub i
			 *  __ldwp/__ldubp  __ptr
			 *
			 *  Load a variable from memory, this is generated for
			 *  code like a "+=", where the store can be optimized
			 *  later on.
			 */
			if
			((p[0]->code == I_LDWP ||
			  p[0]->code == I_LDBP ||
			  p[0]->code == I_LDUBP) &&
			 (p[0]->type == T_PTR) &&
			 (p[1]->code == I_STW) &&
			 (p[1]->type == T_PTR) &&
			 (p[2]->code == I_PUSHW) &&
			 (p[3]->code == I_LDWI)
			) {
				/* replace code */
				*p[1] = *p[3];
				if (p[0]->code == I_LDWP)
					p[1]->code = I_LDW;
				else
				if (p[0]->code == I_LDBP)
					p[1]->code = I_LDB;
				else
					p[1]->code = I_LDUB;
				nb = 1;
			}

			/*
			 *  __lea_s i			-->	__lea_s i
			 *  __pushw				__pushw
			 *  __stw   __ptr			__ldw_s/_ldub_s i + 2
			 *  __ldwp/__ldubp  __ptr
			 *
			 *  Load a variable from memory, this is generated for
			 *  code like a "+=", where the store can be optimized
			 *  later on.
			 */
			else if
			((p[0]->code == I_LDWP ||
			  p[0]->code == I_LDBP ||
			  p[0]->code == I_LDUBP) &&
			 (p[0]->type == T_PTR) &&
			 (p[1]->code == I_STW) &&
			 (p[1]->type == T_PTR) &&
			 (p[2]->code == I_PUSHW) &&
			 (p[3]->code == I_LEA_S)
			) {
				/* replace code */
				*p[1] = *p[3];
				if (p[0]->code == I_LDWP)
					p[1]->code = X_LDW_S;
				else
				if (p[0]->code == I_LDBP)
					p[1]->code = X_LDB_S;
				else
					p[1]->code = X_LDUB_S;
				p[1]-> data += 2;
				nb = 1;
			}

			/*
			 *  __lea_s i			-->	__stwi_s/__stbi_s i,j
			 *  __pushw
			 *  __ldwi  i
			 *  __stwps/__stbps
			 */
			else if
			((p[0]->code == I_STWPS ||
			  p[0]->code == I_STBPS) &&
			 (p[1]->code == I_LDWI) &&
			 (p[1]->type == T_VALUE) &&
			 (p[2]->code == I_PUSHW) &&
			 (p[3]->code == I_LEA_S)

			) {
				/* replace code */
				p[3]->code = (p[0]->code == I_STWPS) ? X_STWI_S : X_STBI_S;
				p[3]->imm_type = p[1]->type;
				p[3]->imm_data = p[1]->data;
				nb = 3;
			}

			/*
			 *  __lea_s i			-->	__lea_s i+j
			 *  __pushw
			 *  __ldwi  j
			 *  __addws
			 *
			 *  This is generated for address calculations into local
			 *  arrays and structs on the stack.
			 */
			else if
			((p[0]->code == I_ADDWS) &&
			 (p[1]->code == I_LDWI) &&
			 (p[1]->type == T_VALUE) &&
			 (p[2]->code == I_PUSHW) &&
			 (p[3]->code == I_LEA_S)
			) {
				/* replace code */
				p[3]->code = I_LEA_S;
				p[3]->data += p[1]->data;
				nb = 3;
			}

			/*
			 *  __ldwi  i			-->	__ldwi	 i * j
			 *  __pushw
			 *  __ldwi  j
			 *  jsr	    umul
			 */
			else if
			((p[0]->code == I_JSR) &&
			 (p[0]->type == T_LIB) &&
			 (!strcmp((char *)p[0]->data, "umul")) &&
			 (p[1]->code == I_LDWI) &&
			 (p[1]->type == T_VALUE) &&
			 (p[2]->code == I_PUSHW) &&
			 (p[3]->code == I_LDWI) &&
			 (p[3]->type == T_VALUE)
			) {
				p[3]->data *= p[1]->data;
				nb = 3;
			}

			/*
			 *  __ldwi p			-->	__stwi p, i
			 *  __pushw
			 *  __ldwi  i
			 *  __st{b|w}ps
			 */
			else if
			((p[0]->code == I_STWPS ||
			  p[0]->code == I_STBPS) &&
			 (p[1]->code == I_LDWI) &&
			 (p[2]->code == I_PUSHW) &&
			 (p[3]->code == I_LDWI)
			) {
				/* replace code */
				p[3]->code = p[0]->code == I_STWPS ? I_STWI : I_STBI;
				p[3]->imm_type = p[1]->type;
				p[3]->imm_data = p[1]->data;
				nb = 3;
			}

#if 0
			/*
			 * __pushw			-->	__addbi_p i
			 * __ldb_p
			 * __addwi i
			 * __stbps
			 *
			 */

			/*
			 * __pushw			-->	__addbi_p i
			 * __stw   __ptr
			 * __ldbp  __ptr
			 * __addwi i
			 * __stbps
			 *
			 */

			else if
			((p[0]->code == I_STBPS) &&
			 (p[1]->code == I_ADDWI) &&
			 (p[2]->code == X_LDB_P) &&
			 (p[3]->code == I_PUSHW)
			) {
				*p[3] = *p[1];
				p[3]->code = I_ADDBI_P;
				nb = 3;
			}
#endif

#if OPT_ARRAY_RD
			/*
			 *  __aslw			-->	__ldwa array
			 *  __addwi array
			 *  __stw _ptr
			 *  __ldwp _ptr
			 *
			 *  Classic base+offset word-array access.
			 */
			else if
			((p[0]->code == I_LDWP) &&
			 (p[0]->type == T_PTR) &&
			 (p[1]->code == I_STW) &&
			 (p[1]->type == T_PTR) &&
			 (p[2]->code == I_ADDWI) &&
			 (p[2]->type == T_SYMBOL) &&
			 (is_small_array((SYMBOL *)p[2]->data)) &&
			 (p[3]->code == I_ASLW)
			) {
				/* replace code */
				p[3]->code = X_LDWA_A;
				p[3]->type = T_SYMBOL;
				p[3]->data = p[2]->data;
				nb = 3;
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

		/* 3-instruction patterns */
		if (q_nb >= 3) {
			/*
			 *  __case			-->	LLnn:
			 *  __endcase
			 *  LLnn:
			 *
			 *  I_ENDCASE is only generated in order to catch which
			 *  case statements could fall through to the next case
			 *  so that an SAX instruction can be generated.
			 *
			 *  This removes redundant I_CASE/I_ENDCASE i-codes that
			 *  just fall through to the next case.
			 */
			if
			((p[0]->code == I_LABEL) &&
			 (p[1]->code == I_ENDCASE) &&
			 (p[2]->code == I_CASE)
			) {
				/* remove code */
				*p[2] = *p[0];
				nb = 2;
			}

			/*  __ldwi i			-->	__ldw/_ldub i
			 *  __stw   __ptr
			 *  __ldwp/__ldubp  __ptr
			 *
			 *  Load a global/static variable from memory
			 */
			else if
			((p[0]->code == I_LDWP ||
			  p[0]->code == I_LDBP ||
			  p[0]->code == I_LDUBP) &&
			 (p[0]->type == T_PTR) &&
			 (p[1]->code == I_STW) &&
			 (p[1]->type == T_PTR) &&
			 (p[2]->code == I_LDWI)
			) {
				/* replace code */
				if (p[0]->code == I_LDWP)
					p[2]->code = I_LDW;
				else
				if (p[0]->code == I_LDBP)
					p[2]->code = I_LDB;
				else
					p[2]->code = I_LDUB;
				nb = 2;
			}

			/*
			 *  __lea_s i			-->	__ldw_s/_ldub_s i
			 *  __stw   __ptr
			 *  __ldwp  __ptr
			 *
			 *  Load a local variable from memory
			 */
			else if
			((p[0]->code == I_LDWP ||
			  p[0]->code == I_LDBP ||
			  p[0]->code == I_LDUBP) &&
			 (p[0]->type == T_PTR) &&
			 (p[1]->code == I_STW) &&
			 (p[1]->type == T_PTR) &&
			 (p[2]->code == I_LEA_S)
			) {
				/* replace code */
				if (p[0]->code == I_LDWP)
					p[2]->code = X_LDW_S;
				else
				if (p[0]->code == I_LDBP)
					p[2]->code = X_LDB_S;
				else
					p[2]->code = X_LDUB_S;
				nb = 2;
			}

			/*
			 *  __pushw			-->	__addwi/__subwi/__andwi/etc i
			 *  __ldwi  i
			 *  __addws/__subws/__andws/etc
			 */
			else if
			((p[0]->code == I_ADDWS ||
			  p[0]->code == I_SUBWS ||
			  p[0]->code == I_ANDWS ||
			  p[0]->code == I_EORWS ||
			  p[0]->code == I_ORWS) &&
			 (p[1]->code == I_LDWI) &&
			 (p[2]->code == I_PUSHW)
			) {
				/* replace code */
				*p[2] = *p[1];
				switch (p[0]->code) {
				case I_ADDWS: p[2]->code = I_ADDWI; break;
				case I_SUBWS: p[2]->code = I_SUBWI; break;
				case I_ANDWS: p[2]->code = I_ANDWI; break;
				case I_EORWS: p[2]->code = I_EORWI; break;
				case I_ORWS: p[2]->code = I_ORWI; break;
				default: abort();
				}
				nb = 2;
				if (p[2]->type == T_VALUE && p[2]->data == 0) {
					if (p[2]->code == I_ANDWI)
						p[2]->code = I_LDWI;
					else
						nb = 3;
				}
			}

			/*
			 *  __pushw			-->	__addw/__subw/__andw/etc symbol
			 *  __ldw  symbol
			 *  __addws/__subws/__andws/etc
			 */
			else if
			((p[0]->code == I_ADDWS ||
			  p[0]->code == I_SUBWS ||
			  p[0]->code == I_ANDWS ||
			  p[0]->code == I_EORWS ||
			  p[0]->code == I_ORWS) &&
			 (p[1]->code == I_LDW) &&
			 (p[2]->code == I_PUSHW)
			) {
				/* replace code */
				*p[2] = *p[1];
				switch (p[0]->code) {
				case I_ADDWS: p[2]->code = I_ADDW; break;
				case I_SUBWS: p[2]->code = I_SUBW; break;
				case I_ANDWS: p[2]->code = I_ANDW; break;
				case I_EORWS: p[2]->code = I_EORW; break;
				case I_ORWS: p[2]->code = I_ORW; break;
				default: abort();
				}
				nb = 2;
			}

			/*
			 *  __pushw			-->	__addub/__subub/__andub/etc symbol
			 *  __ldub  symbol
			 *  __addws/__subws/__andws/etc
			 */
			else if
			((p[0]->code == I_ADDWS ||
			  p[0]->code == I_SUBWS ||
			  p[0]->code == I_ANDWS ||
			  p[0]->code == I_EORWS ||
			  p[0]->code == I_ORWS) &&
			 (p[1]->code == I_LDUB) &&
			 (p[2]->code == I_PUSHW)
			) {
				/* replace code */
				*p[2] = *p[1];
				switch (p[0]->code) {
				case I_ADDWS: p[2]->code = I_ADDUB; break;
				case I_SUBWS: p[2]->code = I_SUBUB; break;
				case I_ANDWS: p[2]->code = I_ANDUB; break;
				case I_EORWS: p[2]->code = I_EORUB; break;
				case I_ORWS: p[2]->code = I_ORUB; break;
				default: abort();
				}
				nb = 2;
			}

			/*
			 *  __pushw			-->	__addw_s  i-2
			 *  __ldw_s  i
			 *  __addws
			 */
			else if
			((p[0]->code == I_ADDWS) &&
			 (p[1]->code == X_LDW_S ||
			  p[1]->code == X_LDUB_S) &&
			 (p[2]->code == I_PUSHW)
			) {
				/* replace code */
				*p[2] = *p[1];
				switch (p[1]->code) {
				case X_LDW_S: p[2]->code = X_ADDW_S; break;
				case X_LDUB_S: p[2]->code = X_ADDUB_S; break;
				default: abort();
				}
				p[2]->data -= 2;
				nb = 2;
			}

			/*
			 *  __pushw			-->	__asl/lsr/asrwi i
			 *  __ldwi i
			 *  jsr asl/lsr/asr
			 */
			else if
			((p[0]->code == I_JSR) &&
			 (!strcmp((char *)p[0]->data, "aslw") ||
			  !strcmp((char *)p[0]->data, "lsrw") ||
			  !strcmp((char *)p[0]->data, "asrw")) &&
			 (p[1]->code == I_LDWI) &&
			 (p[2]->code == I_PUSHW)
			) {
				/* replace code */
				if (!strcmp((char *)p[0]->data, "aslw"))
					p[2]->code = I_ASLWI;
				else if (!strcmp((char *)p[0]->data, "lsrw"))
					p[2]->code = I_LSRWI;
				else
					p[2]->code = I_ASRWI;
				p[2]->type = p[1]->type;
				p[2]->data = p[1]->data;
				nb = 2;
				if (p[2]->type == T_VALUE && p[2]->data == 0) {
					nb = 3;
				}
			}

			/*
			 *  __pushw			-->	__aslwi log2(i)
			 *  __ldwi i			or	__mulwi i
			 *  jsr {u|s}mul
			 */
			else if
			((p[0]->code == I_JSR) &&
			 (!strcmp((char *)p[0]->data, "umul") ||
			  !strcmp((char *)p[0]->data, "smul")) &&
			 (p[1]->code == I_LDWI) &&
			 (p[1]->type == T_VALUE) &&
			 (p[1]->data > 0) &&
			 (p[1]->data < 0x8000) &&
			 (p[2]->code == I_PUSHW)
			) {
				p[2]->type = T_VALUE;
				if (__builtin_popcount((unsigned int)p[1]->data) == 1) {
					p[2]->code = I_ASLWI;
					p[2]->data = __builtin_ctz((unsigned int)p[1]->data);
				}
				else {
					p[2]->code = I_MULWI;
					p[2]->data = p[1]->data;
				}
				nb = 2;
			}

			/*
			 *  __pushw			-->	__notw
			 *  __ldwi 0
			 *  __cmpw eq_w
			 *
			 *  __pushw			-->	__tstw
			 *  __ldwi 0
			 *  __cmpw ne_w
			 */
			else if
			((p[0]->code == I_CMPW) &&
			 (p[1]->code == I_LDWI) &&
			 (p[2]->code == I_PUSHW) &&

			 (p[1]->type == T_VALUE || p[1]->type == T_SYMBOL) &&
			 (strcmp((char *)p[0]->data, "eq_w") == 0 ||
			  strcmp((char *)p[0]->data, "eq_b") == 0 ||
			  strcmp((char *)p[0]->data, "ne_w") == 0 ||
			  strcmp((char *)p[0]->data, "ne_b") == 0)
			) {
				/* replace code */
				if (p[1]->data == 0) {
					if (strcmp((char *)p[0]->data, "eq_w") == 0)
						p[2]->code = I_NOTW;
					else if (strcmp((char *)p[0]->data, "eq_b") == 0)
						p[2]->code = I_NOTW;
					else if (strcmp((char *)p[0]->data, "ne_w") == 0)
						p[2]->code = I_TSTW;
					else if (strcmp((char *)p[0]->data, "ne_b") == 0)
						p[2]->code = I_TSTW;
					p[2]->type = 0;
					p[2]->data = 0;
				} else {
					if (strcmp((char *)p[0]->data, "eq_w") == 0)
						p[2]->code = X_CMPWI_EQ;
					else if (strcmp((char *)p[0]->data, "eq_b") == 0)
						p[2]->code = X_CMPWI_EQ;
					else if (strcmp((char *)p[0]->data, "ne_w") == 0)
						p[2]->code = X_CMPWI_NE;
					else if (strcmp((char *)p[0]->data, "ne_b") == 0)
						p[2]->code = X_CMPWI_NE;
					p[2]->type = p[1]->type;
					p[2]->data = p[1]->data;
				}

				nb = 2;
			}

			/*
			 *  __ld{w/b/c}  n		-->	__incld_{w/b/c}m  n
			 *  __addwi  1
			 *  __st{w/c}  n
			 *
			 *  __ld{w/b/c}  n		-->	__decld_{w/b/c}m  n
			 *  __subwi  1
			 *  __st{w/c}  n
			 *
			 *  pre-increment, post-increment,
			 *  pre-decrement, post-decrement!
			 */
			else if
			((p[1]->code == I_ADDWI ||
			  p[1]->code == I_SUBWI) &&
			 (p[1]->type == T_VALUE) &&
			 (p[1]->data == 1) &&
			 (p[0]->code == I_STW ||
			  p[0]->code == I_STB) &&
			 (p[2]->code == I_LDW ||
			  p[2]->code == I_LDB ||
			  p[2]->code == I_LDUB) &&
			 (p[0]->type == p[2]->type) &&
			 (p[0]->data == p[2]->data)
//			 (cmp_operands(p[0], p[2]) == 1)
			) {
				/* replace code */
				switch (p[2]->code) {
				case I_LDW:	p[2]->code = (p[1]->code == I_ADDWI) ? X_INCLD_WM : X_DECLD_WM; break;
				case I_LDB:	p[2]->code = (p[1]->code == I_ADDWI) ? X_INCLD_BM : X_DECLD_BM; break;
				case I_LDUB:	p[2]->code = (p[1]->code == I_ADDWI) ? X_INCLD_CM : X_DECLD_CM; break;
				default:	break;
				}
				nb = 2;
			}

			/*
			 *  __ld{w/b/c}_s  n		-->	__incld_{w/b/c}s  n
			 *  __addwi  1
			 *  __st{w/c}_s  n
			 *
			 *  __ld{w/b/c}_s  n		-->	__decld_{w/b/c}s  n
			 *  __subwi  1
			 *  __st{w/c}_s  n
			 *
			 *  C pre-increment, post-increment,
			 *  C pre-decrement, post-decrement!
			 */
			else if
			((p[1]->code == I_ADDWI ||
			  p[1]->code == I_SUBWI) &&
			 (p[1]->type == T_VALUE) &&
			 (p[1]->data == 1) &&
			 (p[0]->code == X_STW_S ||
			  p[0]->code == X_STB_S) &&
			 (p[2]->code == X_LDW_S ||
			  p[2]->code == X_LDB_S ||
			  p[2]->code == X_LDUB_S) &&
			 (p[0]->type == p[2]->type) &&
			 (p[0]->data == p[2]->data)
			) {
				/* replace code */
				switch (p[2]->code) {
				case X_LDW_S:	p[2]->code = (p[1]->code == I_ADDWI) ? X_INCLD_WS : X_DECLD_WS; break;
				case X_LDB_S:	p[2]->code = (p[1]->code == I_ADDWI) ? X_INCLD_BS : X_DECLD_BS; break;
				case X_LDUB_S:	p[2]->code = (p[1]->code == I_ADDWI) ? X_INCLD_CS : X_DECLD_CS; break;
				default:	break;
				}
				nb = 2;
			}

			/*
			 *  __ldw/b/ub			-->	__tzw/b/ub
			 *  __tstw				__btrue (or __bfalse)
			 *  __btrue (or __bfalse)
			 *
			 *  __ldw/b/ub			-->	__tnzw/b/ub
			 *  __notw				__btrue (or __bfalse)
			 *  __btrue (or __bfalse)
			 *
			 *  N.B. This deliberately tests for the branch/label
			 *  i-codes in order to delay a match until after any
			 *  duplicate I_TSTW and I_NOTW i-codes are merged.
			 */
			else if
			((p[0]->code == I_LABEL ||
			  p[0]->code == I_BTRUE ||
			  p[0]->code == I_BFALSE) &&
			 (p[1]->code == I_TSTW ||
			  p[1]->code == I_NOTW) &&
			 (p[2]->code == I_LDB ||
			  p[2]->code == I_LDUB ||
			  p[2]->code == I_LDBP ||
			  p[2]->code == I_LDUBP ||
			  p[2]->code == I_LDW ||
			  p[2]->code == I_LDWP ||
			  p[2]->code == X_LDB_S ||
			  p[2]->code == X_LDUB_S ||
			  p[2]->code == X_LDW_S)
			) {
				/* remove code */
				if (p[1]->code == I_TSTW) {
					switch (p[2]->code) {
					case I_LDB:
					case I_LDUB:   p[2]->code = X_TNZB; break;
					case I_LDBP:
					case I_LDUBP:  p[2]->code = X_TNZBP; break;
					case I_LDW:    p[2]->code = X_TNZW; break;
					case I_LDWP:   p[2]->code = X_TNZWP; break;
					case X_LDB_S:
					case X_LDUB_S: p[2]->code = X_TNZB_S; break;
					case X_LDW_S:  p[2]->code = X_TNZW_S; break;
					default: abort();
					}
				} else {
					switch (p[2]->code) {
					case I_LDB:
					case I_LDUB:   p[2]->code = X_TZB; break;
					case I_LDBP:
					case I_LDUBP:  p[2]->code = X_TZBP; break;
					case I_LDW:    p[2]->code = X_TZW; break;
					case I_LDWP:   p[2]->code = X_TZWP; break;
					case X_LDB_S:
					case X_LDUB_S: p[2]->code = X_TZB_S; break;
					case X_LDW_S:  p[2]->code = X_TZW_S; break;
					default: abort();
					}
				}

				*p[1] = *p[0];
				nb = 1;
			}

#if OPT_ARRAY_RD
			/*
			 *  __addwi array		-->	__ldba/__lduba array
			 *  __stw _ptr
			 *  __ldbp/__ldubp _ptr
			 *
			 *  Classic base+offset byte-array access.
			 */
			else if
			((p[0]->code == I_LDBP ||
			  p[0]->code == I_LDUBP) &&
			 (p[0]->type == T_PTR) &&
			 (p[1]->code == I_STW) &&
			 (p[1]->type == T_PTR) &&
			 (p[2]->code == I_ADDWI) &&
			 (p[2]->type == T_SYMBOL) &&
			 (is_small_array((SYMBOL *)p[2]->data))
			) {
				/* replace code */
				p[2]->code = (p[0]->code == I_LDBP) ? X_LDBA_A : X_LDUBA_A;
				nb = 2;
			}
#endif

#if OPT_ARRAY_WR
			/*
			 *  __indexw or __indexb array	-->	__pldwa/__pldba/__plduba array
			 *  __stw _ptr
			 *  __ldwp/__ldbp/__ldubp _ptr
			 *
			 *  Optimized base+offset array access.
			 *
			 *  This MUST be enabled when the X_INDEXW/X_INDEXB
			 *  optimization is enabled, else array loads break
			 *  because the index is pushed instead of an array
			 *  address!
			 */
			else if
			((p[0]->code == I_LDWP ||
			  p[0]->code == I_LDBP ||
			  p[0]->code == I_LDUBP) &&
			 (p[0]->type == T_PTR) &&
			 (p[1]->code == I_STW) &&
			 (p[1]->type == T_PTR) &&
			 (p[2]->code == X_INDEXW ||
			  p[2]->code == X_INDEXB)
			) {
				/* replace code */
				if (p[0]->code == I_LDWP)
					p[2]->code = X_LDPWA_A;
				else
					p[2]->code = (p[0]->code == I_LDBP) ? X_LDPBA_A : X_LDPUBA_A;
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

		/* 2-instruction patterns */
		if (q_nb >= 2) {
			/*
			 *  __bra LLaa			-->	__bra LLaa
			 *  __bra LLbb
			 */
			if
			((p[0]->code == I_BRA) &&
			 (p[1]->code == I_BRA)
			) {
				nb = 1;
			}

			/*
			 *  LLaa:				LLaa .alias LLbb
			 *  __bra LLbb			-->	__bra LLbb
			 */
			if
			((p[0]->code == I_BRA) &&
			 (p[1]->code == I_LABEL)
			) {
				int i = 1;
				do	{
					if (p[i]->data != p[0]->data) {
						p[i]->code = I_ALIAS;
						p[i]->imm_type = T_VALUE;
						p[i]->imm_data = p[0]->data;
					}
				} while (++i < q_nb && i < 10 && p[i]->code == I_LABEL);
			}

			/*
			 *  __bra LLaa			-->	LLaa:
			 *  LLaa:
			 */
			else if
			((p[0]->code == I_LABEL) &&
			 (p[1]->code == I_BRA) &&
			 (p[1]->type == T_LABEL) &&
			 (p[0]->data == p[1]->data)
			) {
				*p[1] = *p[0];
				nb = 1;
			}

			/*
			 *  __ldb/__lbub/etc		-->	__ldb/__lbub/etc
			 *  __switchw				__switchb
			 *
			 */
			else if
			((p[0]->code == I_SWITCHW) &&
			 (p[1]->code == I_LDB ||
			  p[1]->code == I_LDUB ||
			  p[1]->code == I_LDBP ||
			  p[1]->code == I_LDUBP ||
			  p[1]->code == X_LDBA_A ||
			  p[1]->code == X_LDUBA_A ||
			  p[1]->code == X_LDB_S ||
			  p[1]->code == X_LDUB_S)
			) {
				/* optimize code */
				p[0]->code = I_SWITCHB;
				nb = 0;
			}

			/*
			 *  __switchw/__switchb		-->	__switchw/__switchb
			 *  __endcase
			 *
			 *  __bra LLnn			-->	__bra LLnn
			 *  __endcase
			 *
			 *  I_ENDCASE is only generated in order to catch which
			 *  case statements could fall through to the next case
			 *  so that an SAX instruction can be inserted.
			 *
			 *  This removes obviously-redundant I_ENDCASE i-codes.
			 */
			else if
			((p[0]->code == I_ENDCASE) &&
			 (p[1]->code == I_SWITCHW ||
			  p[1]->code == I_SWITCHB ||
			  p[1]->code == I_BRA)
			) {
				/* remove code */
				nb = 1;
			}

			/*
			 *  __tstw			-->	__notw
			 *  __notw
			 */
			else if
			((p[0]->code == I_NOTW) &&
			 (p[1]->code == I_TSTW)
			) {
				p[1]->code = I_NOTW;
				nb = 1;
			}

			/*
			 *  __notw			-->	__tstw
			 *  __notw
			 */
			else if
			((p[0]->code == I_NOTW) &&
			 (p[1]->code == I_NOTW)
			) {
				p[1]->code = I_TSTW;
				nb = 1;
			}

			/*
			 *  __cmpw			-->	__cmpw
			 *  __tstw
			 *
			 *  __notw			-->	__notw
			 *  __tstw
			 *
			 *  __tstw			-->	__tstw
			 *  __tstw
			 *
			 *  This removes redundant tests in compound conditionals ...
			 *
			 *  LLnn:			-->	LLnn:
			 *  __tstw
			 */
			else if
			((p[0]->code == I_TSTW) &&
			 (p[1]->code == I_CMPW ||
			  p[1]->code == X_CMPWI_EQ ||
			  p[1]->code == X_CMPWI_NE ||
			  p[1]->code == I_NOTW ||
			  p[1]->code == I_TSTW ||
			  p[1]->code == I_LABEL)
			) {
				nb = 1;
			}

			/*
			 *  __addmi i, __stack		-->	__addmi i+j, __stack
			 *  __addmi j, __stack
			 */
			else if
			((p[0]->code == I_MODSP) &&
			 (p[1]->code == I_MODSP) &&

			 (p[0]->type == T_STACK) &&
			 (p[1]->type == T_STACK)
			) {
				/* replace code */
				p[1]->data += p[0]->data;
				nb = 1;
			}

			/*
			 *  __addwi i			-->	__addwi i+j
			 *  __addwi j
			 */
			else if
			((p[0]->code == I_ADDWI) &&
			 (p[0]->type == T_VALUE) &&
			 (p[1]->code == I_ADDWI) &&
			 (p[1]->type == T_VALUE)
			) {
				/* replace code */
				p[1]->data += p[0]->data;
				nb = 1;
			}

			/*
			 *  __ldwi  i			-->	__ldwi i+j
			 *  __addwi j
			 */
			else if
			((p[0]->code == I_ADDWI) &&
			 (p[1]->code == I_LDWI) &&

			 (p[0]->type == T_VALUE) &&
			 (p[1]->type == T_VALUE)
			) {
				/* replace code */
				p[1]->data += p[0]->data;
				nb = 1;
			}

			/*
			 *  __ldwi  sym			-->	__ldwi sym+j
			 *  __addwi j
			 */
			else if
			((p[0]->code == I_ADDWI) &&
			 (p[1]->code == I_LDWI) &&

			 (p[0]->type == T_VALUE) &&
			 (p[1]->type == T_SYMBOL)
			) {
				/* replace code */
				if (p[0]->data != 0) {
					SYMBOL * oldsym = (SYMBOL *)p[1]->data;
					SYMBOL * newsym = copysym(oldsym);
					if (NAMEALLOC <=
						snprintf(newsym->name, NAMEALLOC, "%s + %ld", oldsym->name, (long) p[0]->data))
						error("optimized symbol+offset name too long");
					p[1]->type = T_SYMBOL;
					p[1]->data = (intptr_t)newsym;
				}
				nb = 1;
			}

			/*
			 *  __ldwi  j			-->	__ldwi sym+j
			 *  __addwi sym
			 */
			else if
			((p[0]->code == I_ADDWI) &&
			 (p[1]->code == I_LDWI) &&

			 (p[0]->type == T_SYMBOL) &&
			 (p[1]->type == T_VALUE)
			) {
				/* replace code */
				if (p[1]->data != 0) {
					SYMBOL * oldsym = (SYMBOL *)p[0]->data;
					SYMBOL * newsym = copysym(oldsym);
					if (NAMEALLOC <=
						snprintf(newsym->name, NAMEALLOC, "%s + %ld", oldsym->name, (long) p[1]->data))
						error("optimized symbol+offset name too long");
					p[1]->type = T_SYMBOL;
					p[1]->data = (intptr_t)newsym;
				} else {
					*p[1] = *p[0];
					p[1]->code = I_LDWI;
				}
				nb = 1;
			}

			/*
			 *  __ldwi  i			--> __ldwi (i-j)
			 *  __subwi j
			 */
			else if
			((p[0]->code == I_SUBWI) &&
			 (p[1]->code == I_LDWI) &&

			 (p[1]->type == T_VALUE)
			) {
				/* replace code */
				p[1]->data -= p[0]->data;
				nb = 1;
			}

			/*
			 *  __ldwi  i			--> __ldwi (i&j)
			 *  __andwi j
			 */
			else if
			((p[0]->code == I_ANDWI) &&
			 (p[1]->code == I_LDWI) &&

			 (p[1]->type == T_VALUE)
			) {
				/* replace code */
				p[1]->data &= p[0]->data;
				nb = 1;
			}

			/*
			 *  __ldwi  i			--> __ldwi (i|j)
			 *  __orwi j
			 */
			else if
			((p[0]->code == I_ORWI) &&
			 (p[1]->code == I_LDWI) &&

			 (p[1]->type == T_VALUE)
			) {
				/* replace code */
				p[1]->data |= p[0]->data;
				nb = 1;
			}

			/*
			 *  __ldwi  i			--> __ldwi (i*j)
			 *  __mulwi j
			 */
			else if
			((p[0]->code == I_MULWI) &&
			 (p[1]->code == I_LDWI) &&

			 (p[1]->type == T_VALUE)
			) {
				/* replace code */
				p[1]->data *= p[0]->data;
				nb = 1;
			}

			/*
			 *  __ldwi i			--> __ldwi i+i
			 *  __aslw
			 */
			else if
			((p[0]->code == I_ASLW) &&
			 (p[1]->code == I_LDWI) &&

			 (p[1]->type == T_VALUE)
			) {
				/* replace code */
				p[1]->data += p[1]->data;
				nb = 1;
			}

			/*
			 *  __ldwi i			-->	__ldwi i ^ 0xffff
			 *  __comw
			 */
			else if
			((p[0]->code == I_COMW) &&
			 (p[1]->code == I_LDWI) &&

			 (p[1]->type == T_VALUE)
			) {
				/* replace code */
				p[1]->data = p[1]->data ^ 0xffff;
				nb = 1;
			}

			/*
			 *  __ldwi i			-->	__ldwi i ^ 0xffff
			 *  __negw
			 */
			else if
			((p[0]->code == I_NEGW) &&
			 (p[1]->code == I_LDWI) &&

			 (p[1]->type == T_VALUE)
			) {
				/* replace code */
				p[1]->data = -p[1]->data;
				nb = 1;
			}

			/*
			 *  __ldwi <power of two>	-->	__ldwi <log2>
			 *  jsr {u|s}mul		-->	__jsr asl
			 */
			else if
			((p[0]->code == I_JSR) &&
			 (!strcmp((char *)p[0]->data, "umul") ||
			  !strcmp((char *)p[0]->data, "smul")) &&
			 (p[1]->code == I_LDWI) &&
			 (p[1]->type == T_VALUE) &&
			 (__builtin_popcount((unsigned int)p[1]->data) == 1) &&
			 (p[1]->data > 0) &&
			 (p[1]->data < 0x8000)
			) {
				p[0]->data = (intptr_t)"asl";
				p[1]->data = __builtin_ctz((unsigned int)p[1]->data);
				nb = 0;
			}

			/*
			 *  __stw a			-->	__stw a
			 *  __ldw a
			 */
			else if
			((p[0]->code == I_LDW) &&
			 (p[1]->code == I_STW) &&
			 (cmp_operands(p[0], p[1]) == 1)
			) {
				/* remove code */
				nb = 1;
			}

			/*
			 *  __ldw a (or __ldwi a)	-->	__ldw b (or __ldwi b)
			 *  __ldw b (or __ldwi b)
			 */
			else if
			((p[0]->code == I_LDW ||
			  p[0]->code == I_LDWI ||
			  p[0]->code == X_LDW_S ||
			  p[0]->code == I_LEA_S ||
			  p[0]->code == I_LDB ||
			  p[0]->code == I_LDBP ||
			  p[0]->code == X_LDB_S ||
			  p[0]->code == I_LDUB ||
			  p[0]->code == I_LDUBP ||
			  p[0]->code == X_LDUB_S) &&
			 (p[1]->code == I_LDW ||
			  p[1]->code == I_LDWI ||
			  p[1]->code == X_LDW_S ||
			  p[1]->code == I_LEA_S ||
			  p[1]->code == I_LDB ||
			  p[1]->code == I_LDBP ||
			  p[1]->code == X_LDB_S ||
			  p[1]->code == I_LDUB ||
			  p[1]->code == I_LDUBP ||
			  p[1]->code == X_LDUB_S)
			) {
				/* remove code */
				*p[1] = *p[0];
				nb = 1;
			}

			/*
			 *  __stw_s i			-->	__stw_s i
			 *  __ldw_s i
			 */
			else if
			((p[0]->code == X_LDW_S) &&
			 (p[1]->code == X_STW_S) &&

			 (p[0]->data == p[1]->data)) {
				/* remove code */
				nb = 1;
			}

			/*
			 *  __stb_s i			-->	__stb_s i
			 *  __ldb_s i
			 */
			else if
			((p[0]->code == X_LDB_S ||
			  p[0]->code == X_LDUB_S) &&
			 (p[1]->code == X_STB_S) &&
			 (p[0]->data == p[1]->data)
			) {
				if (p[0]->code == X_LDB_S)
					p[0]->code = I_EXTW;
				else
					p[0]->code = I_EXTUW;
				p[0]->data = p[0]->type = 0;
			}

			/*
			 *  ldwi i			-->	stwi const, i
			 *  stw const
			 *
			 * XXX: This doesn't really do anything...
			 */
			else if
			((p[0]->code == I_STW ||
			  p[0]->code == I_STB) &&
			 (p[0]->type == T_VALUE) &&
			 (p[1]->code == I_LDWI)
			) {
				p[1]->code = (p[0]->code == I_STW) ? I_STWI : I_STBI;
				p[1]->imm_type = p[1]->type;
				p[1]->imm_data = p[1]->data;
				p[1]->type = p[0]->type;
				p[1]->data = p[0]->data;
				nb = 1;
			}

			/*
			 *  __decld_{w/b/c}m  n		-->	__lddec_{w/b/c}m  n
			 *  __addwi  1
			 *
			 *  __decld_{w/b/c}s  n		-->	__lddec_{w/b/c}s  n
			 *  __addwi  1
			 *
			 *  C post-decrement!
			 */
			else if
			((p[0]->code == I_ADDWI) &&
			 (p[0]->type == T_VALUE) &&
			 (p[0]->data == 1) &&
			 (p[1]->code == X_DECLD_WM ||
			  p[1]->code == X_DECLD_BM ||
			  p[1]->code == X_DECLD_CM ||
			  p[1]->code == X_DECLD_WS ||
			  p[1]->code == X_DECLD_BS ||
			  p[1]->code == X_DECLD_CS)
			) {
				/* replace code */
				switch (p[1]->code) {
				case X_DECLD_WM: p[1]->code = X_LDDEC_WM; break;
				case X_DECLD_BM: p[1]->code = X_LDDEC_BM; break;
				case X_DECLD_CM: p[1]->code = X_LDDEC_CM; break;
				case X_DECLD_WS: p[1]->code = X_LDDEC_WS; break;
				case X_DECLD_BS: p[1]->code = X_LDDEC_BS; break;
				case X_DECLD_CS: p[1]->code = X_LDDEC_CS; break;
				default:	break;
				}
				nb = 1;
			}

			/*
			 *  __incld_{w/b/c}m  n		-->	__ldinc_{w/b/c}m  n
			 *  __subwi  1
			 *
			 *  __incld_{w/b/c}s  n		-->	__ldinc_{w/b/c}s  n
			 *  __subwi  1
			 *
			 *  C post-increment!
			 */
			else if
			((p[0]->code == I_SUBWI) &&
			 (p[0]->type == T_VALUE) &&
			 (p[0]->data == 1) &&
			 (p[1]->code == X_INCLD_WM ||
			  p[1]->code == X_INCLD_BM ||
			  p[1]->code == X_INCLD_CM ||
			  p[1]->code == X_INCLD_WS ||
			  p[1]->code == X_INCLD_BS ||
			  p[1]->code == X_INCLD_CS)
			) {
				/* replace code */
				switch (p[1]->code) {
				case X_INCLD_WM: p[1]->code = X_LDINC_WM; break;
				case X_INCLD_BM: p[1]->code = X_LDINC_BM; break;
				case X_INCLD_CM: p[1]->code = X_LDINC_CM; break;
				case X_INCLD_WS: p[1]->code = X_LDINC_WS; break;
				case X_INCLD_BS: p[1]->code = X_LDINC_BS; break;
				case X_INCLD_CS: p[1]->code = X_LDINC_CS; break;
				default:	break;
				}
				nb = 1;
			}

#if 0
			/*
			 *  __stwi 0			-->	__stwz
			 *  is_load()
			 *
			 * BETTER HANDLED WITH I_FENCE!
			 */
			else if
			((p[1]->code == I_STWI) &&
			 (p[1]->imm_type == T_VALUE) &&
			 (p[1]->imm_data == 0) &&
			 (is_load(p[0]))
			) {
				p[1]->code = I_STWZ;
			}

			/*
			 *  __stbi 0			-->	__stwz
			 *  is_load()
			 *
			 * BETTER HANDLED WITH I_FENCE!
			 */
			else if
			((p[1]->code == I_STBI) &&
			 (p[1]->imm_type == T_VALUE) &&
			 (p[1]->imm_data == 0) &&
			 (is_load(p[0]))
			) {
				p[1]->code = I_STBZ;
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
	}

	/*
	 * optimization level 2 - instruction re-scheduler,
	 *
	 * change the instruction order to allow for direct assignments rather
	 * than the stack-based assignments that are generated by complex math
	 * or things like "var++" that are not covered by the simpler peephole
	 * rules earlier.
	 *
	 * this covers storing to global and static variables, plus
	 * a whole bunch of math with integers ...
	 *
	 *  __ldwi i				-->	...
	 *  __pushw					__stw i or __stb i or __addwi i
	 *    ...
	 *  __stwps or __stbps or __addws
	 *
	 * this covers storing to local variables ...
	 *
	 *  __lea_s i				-->	...
	 *  __pushw					__stw_s i or __stb_s i
	 *    ...
	 *  __stwps or __stbps
	 *
	 * this covers storing to global and static arrays with "=" ...
	 *
	 *  __aslw				-->	__index2
	 *  __addwi array				...
	 *  __pushw					__stwa_a array
	 *    ...
	 *  __stwps
	 *
	 *  __addwi array			-->	__index1
	 *  __pushw					...
	 *    ...					__stba_a array
	 *  __stbps
	 *
	 * this covers storing to global and static arrays with "+=", "-=", etc ...
	 *
	 *  __aslw				-->	__ldpwa_a array
	 *  __addwi array				...
	 *  __pushw					__stwa_a array
	 *  __stw _ptr
	 *  __ldwp _ptr
	 *    ...
	 *  __stwps
	 *
	 *  __addwi array			-->	__ldpuba_a array
	 *  __pushw					...
	 *  __stw _ptr					__stba_a array
	 *  __ldubp _ptr
	 *    ...
	 *  __stbps
	 */
	if (optimize >= 2) {
		int offset;
		int copy, scan, prev;

		/* check last instruction */
		if (q_nb > 1 &&
		(q_ins[q_wr].code == I_STWPS ||
		 q_ins[q_wr].code == I_STBPS ||
		 q_ins[q_wr].code == I_ADDWS ||
		 q_ins[q_wr].code == I_SUBWS ||
		 q_ins[q_wr].code == I_ANDWS ||
		 q_ins[q_wr].code == I_EORWS ||
		 q_ins[q_wr].code == I_ORWS)
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
				switch (q_ins[scan].code) {
				case I_JSR:
				case I_CMPW:
				case I_CMPB:
					if (q_ins[scan].type == T_LIB)
						offset += 2;
					break;

				case I_MODSP:
					if ((q_ins[scan].type == T_STACK) ||
					    (q_ins[scan].type == T_NOP))
						offset += (int)q_ins[scan].data;
					break;

				case I_POPW:
				case I_STWPS:
				case I_STBPS:
				case I_ADDWS:
				case I_SUBWS:
				case I_ANDWS:
				case I_EORWS:
				case I_ORWS:
					offset += 2;
					break;

				case I_PUSHW:
					offset -= 2;
					break;
				default:
					break;
				}

				/* check offset */
				if (offset == 0) {
					/*
					 * found the I_PUSHW that matches the I_STWPS
					 */
					int from = scan + 1; /* begin copying after the I_PUSHW */
					int drop = 2; /* drop I_PUSHW and the i-code before it */

					if (copy == 1) {
						/* hmm, may be not...
						 * there should be at least one instruction
						 * between I_PUSHW and I_STWPS.
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
						 * I_PUSHW preceded by I_LEA_S/I_LDWI/I_ADDWI
						 */
						if (q_ins[scan].code != I_PUSHW)
							break;

#if OPT_ARRAY_WR
						if (q_ins[prev].code != I_LDWI &&
						    q_ins[prev].code != I_LEA_S &&
						    q_ins[prev].code != I_ADDWI)
							break;
#else
						if (q_ins[prev].code != I_LDWI &&
						    q_ins[prev].code != I_LEA_S)
							break;
#endif

						if (q_ins[prev].code != I_LDWI &&
						    q_ins[q_wr].code != I_STWPS &&
						    q_ins[q_wr].code != I_STBPS)
							break;

						/* change stwps into stw_s/stw */
						if (q_ins[prev].code == I_LDWI) {
							switch (q_ins[q_wr].code) {
							case I_STWPS:
								q_ins[q_wr].code = I_STW;
								break;
							case I_STBPS:
								q_ins[q_wr].code = I_STB;
								break;
							case I_ADDWS:
								q_ins[q_wr].code = I_ADDWI;
								break;
							case I_SUBWS:
								q_ins[q_wr].code = I_ISUBWI;
								break;
							case I_ANDWS:
								q_ins[q_wr].code = I_ANDWI;
								break;
							case I_EORWS:
								q_ins[q_wr].code = I_EORWI;
								break;
							case I_ORWS:
								q_ins[q_wr].code = I_ORWI;
								break;
							default:
								abort();
							}
							/* use data from the preceding I_LDWI */
							q_ins[q_wr].type = q_ins[prev].type;
							q_ins[q_wr].data = q_ins[prev].data;
						} else
						if (q_ins[prev].code == I_LEA_S) {
							if (q_ins[q_wr].code == I_STWPS)
								q_ins[q_wr].code = X_STW_S;
							else
								q_ins[q_wr].code = X_STB_S;
							/* use data from the preceding I_LEA_S */
							q_ins[q_wr].type = q_ins[prev].type;
							q_ins[q_wr].data = q_ins[prev].data;
							q_ins[q_wr].sym = q_ins[prev].sym;
#if OPT_ARRAY_WR

						} else {
							int push = X_INDEXB;
							int code = X_STBAS;

							/* make sure that I_ADDWI is really a short array */
							if (q_ins[prev].type != T_SYMBOL ||
							    !is_small_array((SYMBOL *)q_ins[prev].data))
								break;

							copy = copy + 1;
							from = scan;
							drop = 1;

							/* make sure that an I_STWPS has an I_ASLW */
							if (q_ins[q_wr].code == I_STWPS) {
								int aslw = prev - 1;
								if (aslw < 0)
									aslw += Q_SIZE;
								if (copy == q_nb || q_ins[aslw].code != I_ASLW)
									break;
								drop = 2;
								push = X_INDEXW;
								code = X_STWAS;
							}

							/* push the index from the preceding I_ADDWI */
							q_ins[scan].code = push;
							q_ins[scan].type = T_SYMBOL;
							q_ins[scan].data = q_ins[prev].data;

							/* use data from the preceding I_ADDWI */
							q_ins[q_wr].code = code;
							q_ins[q_wr].type = T_SYMBOL;
							q_ins[q_wr].data = q_ins[prev].data;
#endif
						}
					}

					/*
					 * adjust stack references for the
					 * removal of the I_PUSHW
					 */
					for (int temp = copy; temp > 1; temp--) {
						scan += 1;
						if (scan >= Q_SIZE)
							scan -= Q_SIZE;

						/* check instruction */
						if (is_sprel(&q_ins[scan])) {
							/* adjust stack offset */
							q_ins[scan].data -= 2;
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
			 *  __pushw			-->	__st{b|w}ip i
			 *  __ldwi  i
			 *  __st{b|w}ps
			 *
			 *  This cannot be done earlier because it screws up
			 *  the reordering optimization above.
			 */
			if
			((p[0]->code == I_STWPS ||
			  p[0]->code == I_STBPS) &&
			 (p[1]->code == I_LDWI) &&
			 (p[1]->type == T_VALUE) &&
			 (p[2]->code == I_PUSHW)
			) {
				/* replace code */
				p[2]->code = p[0]->code == I_STWPS ? I_STWIP : I_STBIP;
				p[2]->data = p[1]->data;
				nb = 2;
			}

			/*
			 *  ldwi i			-->	stwi i, j
			 *  stwip j
			 */
			else if
			((p[0]->code == I_STWIP) &&
			 (p[1]->code == I_LDWI)
			) {
				p[1]->code = I_STWI;
				p[1]->imm_type = p[0]->type;
				p[1]->imm_data = p[0]->data;
				nb = 1;
			}

			/*
			 *  ldwi i			-->	stbi i, j
			 *  stbip j
			 */
			else if
			((p[0]->code == I_STBIP) &&
			 (p[1]->code == I_LDWI)
			) {
				p[1]->code = I_STBI;
				p[1]->imm_type = p[0]->type;
				p[1]->imm_data = p[0]->data;
				nb = 1;
			}

#if 0
			/*
			 *  __pushw			-->	__stw __ptr
			 *  <load>				<load>
			 *  __st{b|w}ps				__st{b|w}p __ptr
			 *
			 *  This cannot be done earlier because it screws up
			 *  the reordering optimization above.
			 *
			 *  THIS IS VERY RARE, REMOVE IT FOR NOW AND RETHINK IT
			 */
			else if
			((p[0]->code == I_STBPS ||
			  p[0]->code == I_STWPS) &&
			 (is_load(p[1])) &&
			 (p[2]->code == I_PUSHW)
			) {
				p[2]->code = I_STW;
				p[2]->type = T_PTR;
				/* We just removed a push, adjust SP-relative
				   addresses. */
				if (is_sprel(p[1]))
					p[1]->data -= 2;
				if (p[0]->code == I_STBPS)
					p[0]->code = I_STBP;
				else
					p[0]->code = I_STWP;
				q_ins[q_wr].type = T_PTR;
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
		    (q_ins[q_rd].code != I_BRA) ||
		    q_ins[q_rd].data != nextlabel) {
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
