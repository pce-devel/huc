/*	File opt.c: 2.1 (83/03/20,16:02:09) */
/*% cc -O -c %
 *
 */

// #define DEBUG_OPTIMIZER

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
#include "io.h"
#include "error.h"

#ifdef _MSC_VER
 #include <intrin.h>
 #define __builtin_popcount __popcnt
 #define __builtin_ctz _tzcnt_u32
#endif

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


static int is_load (INS *i)
{
	return (i->code == I_LDWI ||
		i->code == I_LDW ||
		i->code == I_LDB ||
		i->code == I_LDUB ||
		i->code == I_LDWP ||
		i->code == I_LDBP ||
		i->code == I_LDUBP ||
		i->code == X_LDWA ||
		i->code == X_LDBA ||
		i->code == X_LDUBA ||
		i->code == X_LDW_S ||
		i->code == X_LDB_S ||
		i->code == X_LDUB_S ||
		i->code == X_LDWA_S ||
		i->code == X_LDBA_S ||
		i->code == X_LDUBA_S ||
		i->code == X_LDD_I ||
		i->code == X_LDD_S_W ||
		i->code == X_LDD_S_B);
}

static int is_sprel (INS *i)
{
	return (i->code == X_LEA_S ||
		i->code == X_PEA_S ||
		i->code == X_LDW_S ||
		i->code == X_LDB_S ||
		i->code == X_LDUB_S ||
		i->code == X_LDWA_S ||
		i->code == X_LDBA_S ||
		i->code == X_LDUBA_S ||
		i->code == X_STWI_S ||
		i->code == X_STBI_S ||
		i->code == X_STW_S ||
		i->code == X_STB_S ||
		i->code == X_STWA_S ||
		i->code == X_STBA_S ||
		i->code == X_INCW_S ||
		i->code == X_INCB_S ||
		i->code == X_ADDW_S ||
		i->code == X_ADDB_S ||
		i->code == X_ADDUB_S ||
		i->code == X_TZW_S ||
		i->code == X_TZB_S ||
		i->code == X_TNZW_S ||
		i->code == X_TNZB_S ||
		i->code == X_LDD_S_W ||
		i->code == X_LDD_S_B);
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
#if 0
			/*  Classical Base+offset word array access:
			 *
			 *  N.B. This arrary access optimization is now done in the code generator.
			 *
			 *  __ldwi  label		-->	@_ldw_s	 n-2
			 *  __pushw				__aslw
			 *  @_ldw_s n				__addwi	 label
			 *  __aslw
			 *  __addws
			 */
			if
			((p[0]->code == I_ADDWS) &&
			 (p[1]->code == I_ASLW) &&
			 (p[2]->code == X_LDW_S) &&
			 (p[3]->code == I_PUSHW) &&
			 (p[4]->code == I_LDWI)
			) {
				intptr_t tempdata;

				tempdata = p[2]->data;

				/* replace code */
				*p[2] = *p[4];
				p[2]->code = I_ADDWI;
				p[3]->code = I_ASLW;
				p[4]->code = X_LDW_S;
				p[4]->data = tempdata - 2;

				nb = 2;
			}

			/*  Classical Base+offset word array access:
			 *
			 *  N.B. This arrary access optimization is now done in the code generator.
			 *
			 *  __ldwi  label1		-->	__ldw	 label2
			 *  __pushw				__aslw
			 *  __ldw   label2			__addwi	 label1
			 *  __aslw
			 *  __addws
			 */
			else if
			((p[0]->code == I_ADDWS) &&
			 (p[1]->code == I_ASLW) &&
			 (p[2]->code == I_LDW) &&
			 (p[3]->code == I_PUSHW) &&
			 (p[4]->code == I_LDWI)
			) {
				intptr_t tempdata;
				SYMBOL *tempsym;
				char temptype;

				temptype = p[2]->type;
				tempdata = p[2]->data;
				tempsym = p[2]->sym;

				/* replace code */
				*p[2] = *p[4];
				p[2]->code = I_ADDWI;
				p[2]->type = p[4]->type;
				p[2]->data = p[4]->data;
				p[2]->sym = p[4]->sym;
				p[3]->code = I_ASLW;
				p[4]->code = I_LDW;
				p[4]->type = temptype;
				p[4]->data = tempdata;
				p[4]->sym = tempsym;

				nb = 2;
			}
#endif

			/*
			 *  __ldw or __ldw_s		-->	__ldwa p, i/__ldwa_s p, i
			 *  __aslw
			 *  __addwi_sym array
			 *  __stw _ptr
			 *  __ldwp _ptr
			 */
			if
			((p[0]->code == I_LDWP) &&
			 (p[0]->type == T_PTR) &&
			 (p[1]->code == I_STW) &&
			 (p[1]->type == T_PTR) &&
			 (p[2]->code == I_ADDWI) &&
			 (p[2]->type == T_SYMBOL) &&
			 (is_small_array((SYMBOL *)p[2]->data)) &&
			 (p[3]->code == I_ASLW) &&
			 (p[4]->code == I_LDW ||
			  p[4]->code == I_LDB ||
			  p[4]->code == I_LDUB ||
			  p[4]->code == X_LDW_S ||
			  p[4]->code == X_LDB_S ||
			  p[4]->code == X_LDUB_S)
			) {
				/* replace code */
				switch (p[4]->code) {
				case I_LDW:
				case I_LDB:
				case I_LDUB: p[4]->code = X_LDWA; break;
				case X_LDW_S:
				case X_LDB_S:
				case X_LDUB_S: p[4]->code = X_LDWA_S; break;
				default: abort();
				}
				p[4]->imm_type = p[2]->type;
				p[4]->imm_data = p[2]->data;
				nb = 4;
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

		/* 4-instruction patterns */
		if (q_nb >= 4) {
#if 0
			/*  Classical Base+offset byte array access:
			 *
			 *  N.B. This arrary access optimization is now done in the code generator.
			 *
			 *  __ldwi  label		-->	@_ldub_s n-2
			 *  __pushw				__addwi	 label
			 *  @_ldub_s n
			 *  __addws
			 */
			if
			((p[0]->code == I_ADDWS) &&
			 (p[1]->code == X_LDUB_S) &&
			 (p[2]->code == I_PUSHW) &&
			 (p[3]->code == I_LDWI)
			) {
				/* replace code */
				*p[2] = *p[3];
				p[2]->code = I_ADDWI;
				p[3]->code = X_LDUB_S;
				p[3]->data = p[1]->data - 2;

				nb = 2;
			}

			/*  Classical Base+offset byte array access:
			 *
			 *  N.B. This arrary access optimization is now done in the code generator.
			 *
			 *  __ldwi  label1		-->	__ldub	 label2
			 *  __pushw				__addwi	 label1
			 *  __ldub  label2
			 *  __addws
			 */
			else if
			((p[0]->code == I_ADDWS) &&
			 (p[1]->code == I_LDUB) &&
			 (p[2]->code == I_PUSHW) &&
			 (p[3]->code == I_LDWI)
			) {
				/* replace code */
				*p[2] = *p[3];
				p[2]->code = I_ADDWI;
				*p[3] = *p[1];

				nb = 2;
			}
#endif

			/*
			 *  @_ldw/b/ub_s i		-->	@_ldw/b/ub_s  i
			 *  __addwi 1				@_incw/b_s i
			 *  @_stw_s i
			 *  __subwi 1
			 */
			if
			((p[0]->code == I_SUBWI) &&
			 (p[1]->code == X_STW_S) &&
			 (p[2]->code == I_ADDWI) &&
			 (p[3]->code == X_LDW_S) &&

			 (p[0]->data == 1) &&
			 (p[2]->data == 1) &&
			 (p[1]->data == p[3]->data) &&
			 (p[1]->data < 255)
			) {
				/* replace code */
				p[2]->code = X_INCW_S;
				p[2]->data = p[3]->data;
				p[2]->sym = p[3]->sym;
				nb = 2;
			}

			else if
			((p[0]->code == I_SUBWI) &&
			 (p[1]->code == X_STB_S) &&
			 (p[2]->code == I_ADDWI) &&
			 (p[3]->code == X_LDB_S ||
			  p[3]->code == X_LDUB_S) &&
			 (p[0]->data == 1) &&
			 (p[2]->data == 1) &&
			 (p[1]->data == p[3]->data) &&
			 (p[1]->data < 255)
			) {
				/* replace code */
				p[2]->code = X_INCB_S;
				p[2]->data = p[3]->data;
				p[2]->sym = p[3]->sym;
				nb = 2;
			}

			/*
			 *  @_ldwi  i			-->	@_ldwi	 i * j
			 *  __pushw
			 *  __ldwi  j
			 *  jsr	    umul
			 */
			else if
			((p[0]->code == I_JSR) &&
			 (p[0]->type == T_LIB) &&
			 (strcmp((char *)p[0]->data, "umul") == 0) &&
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
			 *  @_ldwi p			-->	__stwi p, i
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

			/*
			 *  __ldub or __ldub_s		-->	__lduba p, i/__lduba_s p, i
			 *  __addwi_sym array
			 *  __stw _ptr
			 *  __ldbp/__ldubp _ptr
			 */
			else if
			((p[0]->code == I_LDBP ||
			  p[0]->code == I_LDUBP) &&
			 (p[0]->type == T_PTR) &&
			 (p[1]->code == I_STW) &&
			 (p[1]->type == T_PTR) &&
			 (p[2]->code == I_ADDWI) &&
			 (p[2]->type == T_SYMBOL) &&
			 (is_small_array((SYMBOL *)p[2]->data)) &&
			 (p[3]->code == I_LDW ||
			  p[3]->code == I_LDB ||
			  p[3]->code == I_LDUB ||
			  p[3]->code == X_LDW_S ||
			  p[3]->code == X_LDB_S ||
			  p[3]->code == X_LDUB_S)
			) {
				/* replace code */
				switch (p[3]->code) {
				case I_LDW:
				case I_LDB:
				case I_LDUB: p[3]->code = (p[0]->code == I_LDBP) ? X_LDBA : X_LDUBA; break;
				case X_LDW_S:
				case X_LDB_S:
				case X_LDUB_S: p[3]->code =  (p[0]->code == I_LDBP) ? X_LDBA_S : X_LDUBA_S; break;
				default: abort();
				}
				p[3]->imm_type = p[2]->type;
				p[3]->imm_data = p[2]->data;
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

			/*
			 *  __pushw			-->	__addwi/__subwi/__andwi/etc i
			 *  __ldwi  i
			 *  __addws/__subws/__andws/etc
			 */
			else if
			((p[0]->code == I_ADDWS ||
			  p[0]->code == I_ADDBS ||
			  p[0]->code == I_SUBWS ||
			  p[0]->code == I_ANDWS ||
			  p[0]->code == I_EORWS ||
			  p[0]->code == I_ORWS) &&
			 (p[1]->code == I_LDWI) &&
//			 (p[1]->type == T_VALUE) &&
			 (p[2]->code == I_PUSHW)
			) {
				/* replace code */
				*p[2] = *p[1];
				switch (p[0]->code) {
				case I_ADDWS: p[2]->code = I_ADDWI; break;
				case I_ADDBS: p[2]->code = I_ADDBI; break;
				case I_SUBWS: p[2]->code = I_SUBWI; break;
				case I_ANDWS: p[2]->code = I_ANDWI; break;
				case I_EORWS: p[2]->code = I_EORWI; break;
				case I_ORWS: p[2]->code = I_ORWI; break;
				default: abort();
				}
				nb = 2;
			}

			/*
			 *  __pushw			-->	__st{b|w}ip i
			 *  __ldwi  i
			 *  __st{b|w}ps
			 */
			else if
			((p[0]->code == I_STWPS ||
			  p[0]->code == I_STBPS) &&
			 (p[1]->code == I_LDWI) &&
			 (p[2]->code == I_PUSHW) &&

			 (p[1]->type == T_VALUE)
			) {
				/* replace code */
				p[2]->code = p[0]->code == I_STWPS ? I_STWIP : I_STBIP;
				p[2]->data = p[1]->data;
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
			 *  __pushw			-->	__addb/w/ub/b_s/ub_s/w_s  nnn
			 *  __ldb/w/ub/b_s/ub_s/w_s  nnn
			 *  __addws
			 */
			else if
			((p[0]->code == I_ADDWS) &&
			 (p[1]->code == I_LDW ||
			  p[1]->code == I_LDB ||
			  p[1]->code == I_LDUB ||
			  p[1]->code == X_LDB_S ||
			  p[1]->code == X_LDUB_S ||
			  p[1]->code == X_LDW_S) &&
			 (p[2]->code == I_PUSHW)
			) {
				/* replace code */
				switch (p[1]->code) {
				case I_LDW: p[2]->code = I_ADDW; break;
				case I_LDB: p[2]->code = I_ADDB; break;
				case I_LDUB: p[2]->code = I_ADDUB; break;
				case X_LDB_S: p[2]->code = X_ADDB_S; p[1]->data -= 2; break;
				case X_LDUB_S: p[2]->code = X_ADDUB_S; p[1]->data -= 2; break;
				case X_LDW_S: p[2]->code = X_ADDW_S; p[1]->data -= 2; break;
				default: abort();
				}
				p[2]->data = p[1]->data;
				p[2]->type = p[1]->type;
				nb = 2;
			}

			/*
			 *  __pushw			-->	__subw	nnn
			 *  __ldw  nnn
			 *  __subws
			 */
			else if
			((p[0]->code == I_SUBWS) &&
			 (p[1]->code == I_LDW) &&
			 (p[2]->code == I_PUSHW)
			) {
				/* replace code */
				p[2]->code = I_SUBW;
				p[2]->data = p[1]->data;
				p[2]->type = p[1]->type;
				nb = 2;
			}

			/*
			 *  __pushw			-->	@_addw_s i-2
			 *  @_ldw_s i
			 *  __addws
			 */
			else if
			((p[0]->code == I_ADDWS) &&
			 (p[1]->code == X_LDW_S) &&
			 (p[2]->code == I_PUSHW)
			) {
				/* replace code */
				p[2]->code = X_ADDW_S;
				p[2]->data = p[1]->data - 2;
				p[2]->sym = p[1]->sym;
				nb = 2;
			}

			/*
			 *  @_pea_s j			-->	@_stbi_s i,j
			 *  __ldwi  i
			 *  __stbps
			 */
			else if
			((p[0]->code == I_STBPS) &&
			 (p[1]->code == I_LDWI) &&
			 (p[2]->code == X_PEA_S) &&

			 (p[1]->type == T_VALUE)
			) {
				/* replace code */
				p[2]->code = X_STBI_S;
				p[2]->imm_type = p[1]->type;
				p[2]->imm_data = p[1]->data;
				nb = 2;
			}

			/*
			 *  @_pea_s j			-->	@_stwi_s i,j
			 *  __ldwi  i
			 *  __stwps
			 */
			else if
			((p[0]->code == I_STWPS) &&
			 (p[1]->code == I_LDWI) &&
			 (p[2]->code == X_PEA_S) &&

			 (p[1]->type == T_VALUE)
			) {
				/* replace code */
				p[2]->code = X_STWI_S;
				p[2]->imm_type = p[1]->type;
				p[2]->imm_data = p[1]->data;
				nb = 2;
			}

			/*
			 *  @_pea_s i			-->	@_lea_s i+j
			 *  __ldwi  j
			 *  __addws
			 */
			else if
			((p[0]->code == I_ADDWS) &&
			 (p[1]->code == I_LDWI) &&
			 (p[2]->code == X_PEA_S) &&

			 (p[1]->type == T_VALUE)
			) {
				/* replace code */
				p[2]->code = X_LEA_S;
				p[2]->data += p[1]->data;
				nb = 2;
			}

			/*
			 *  @_lea_s i			-->	@_ldw_s/_ldub_s i
			 *  __stw   __ptr
			 *  __ldwp  __ptr
			 */
			else if
			((p[0]->code == I_LDWP ||
			  p[0]->code == I_LDBP ||
			  p[0]->code == I_LDUBP) &&
			 (p[1]->code == I_STW) &&
			 (p[2]->code == X_LEA_S) &&

			 (p[0]->type == T_PTR) &&
			 (p[1]->type == T_PTR)
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

			/*  @_ldwi i			-->	@_ldw/_ldub i
			 *  __stw   __ptr
			 *  __ldwp/__ldubp  __ptr
			 */
			else if
			((p[0]->code == I_LDWP ||
			  p[0]->code == I_LDBP ||
			  p[0]->code == I_LDUBP) &&
			 (p[1]->code == I_STW) &&
			 (p[2]->code == I_LDWI) &&

			 (p[0]->type == T_PTR) &&
			 (p[1]->type == T_PTR)
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
			 *  @_pea_s i			-->	@_pea_s i
			 *  __stw   __ptr			@_ldw_s i+2
			 *  __ldwp  __ptr
			 */
			else if
			((p[0]->code == I_LDWP ||
			  p[0]->code == I_LDBP ||
			  p[0]->code == I_LDUBP) &&
			 (p[1]->code == I_STW) &&
			 (p[2]->code == X_PEA_S) &&

			 (p[0]->type == T_PTR) &&
			 (p[1]->type == T_PTR) &&

			 (optimize >= 2)
			) {
				/* replace code */
				if (p[0]->code == I_LDWP)
					p[1]->code = X_LDW_S;
				else
				if (p[0]->code == I_LDBP)
					p[1]->code = X_LDB_S;
				else
					p[1]->code = X_LDUB_S;
				p[1]->data = p[2]->data + 2;
				p[1]->sym = p[2]->sym;
				nb = 1;
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
			 *  __ldw/b/ub	 n		-->	__incw/b  n
			 *  __addwi 1				__ldw/b/ub   n
			 *  __stw/b/ub	 n
			 */
			else if
			((p[0]->code == I_STW ||
			  p[0]->code == I_STB) &&
			 (p[1]->code == I_ADDWI) &&
			 (p[1]->type == T_VALUE) &&
			 (p[1]->data == 1) &&

			 (p[2]->code == I_LDW ||
			  p[2]->code == I_LDB ||
			  p[2]->code == I_LDUB) &&
			 (cmp_operands(p[0], p[2]) == 1)
			) {
				/* replace code */
				p[1]->code = p[2]->code;
				p[1]->type = p[2]->type;
				p[1]->data = p[2]->data;
				p[2]->code = (p[0]->code == I_STW) ? I_INCW : I_INCB;
				nb = 1;
			}

			/*
			 *  __incw/b	 n		-->	__ldw/b/ub   n
			 *  __ldw/b/ub	  n			__incw/b  n
			 *  __subwi  1
			 */
			else if
			(((p[0]->code == I_SUBWI) &&
			  (p[0]->type == T_VALUE) &&
			  (p[0]->data == 1)) &&

			 (p[1]->code == I_LDW) &&
			 (p[2]->code == I_INCW) &&
			 (cmp_operands(p[1], p[2]) == 1)
			) {
				/* replace code */
				p[2]->code = p[1]->code;
				p[2]->type = p[1]->type;
				p[2]->data = p[1]->data;
				p[1]->code = I_INCW;
				nb = 1;
			}

			else if
			(((p[0]->code == I_SUBWI) &&
			  (p[0]->type == T_VALUE) &&
			  (p[0]->data == 1)) &&

			 (p[1]->code == I_LDB || p[1]->code == I_LDUB) &&
			 (p[2]->code == I_INCB) &&
			 (cmp_operands(p[1], p[2]) == 1)) {
				/* replace code */
				p[2]->code = p[1]->code;
				p[2]->type = p[1]->type;
				p[2]->data = p[1]->data;
				p[1]->code = I_INCB;
				nb = 1;
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
			  p[1]->code == X_LDBA ||
			  p[1]->code == X_LDUBA ||
			  p[1]->code == X_LDB_S ||
			  p[1]->code == X_LDUB_S ||
			  p[1]->code == X_LDBA_S ||
			  p[1]->code == X_LDUBA_S)
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
			 *  so that an SAX instruction can be generated.
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
			 *  __add[bw]i j
			 */
			else if
			((p[0]->code == I_ADDWI ||
			  p[0]->code == I_ADDBI) &&
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
			 *  __add[bw]i j
			 */
			else if
			((p[0]->code == I_ADDWI || p[0]->code == I_ADDBI) &&
			 (p[1]->code == I_LDWI) &&

			 (p[0]->type == T_VALUE) &&
			 (p[1]->type == T_SYMBOL)
			) {
				/* replace code */
				if (p[0]->data != 0) {
					SYMBOL * oldsym = (SYMBOL *)p[1]->data;
					SYMBOL * newsym = copysym(oldsym);
					if (NAMEALLOC <=
						snprintf(newsym->name, NAMEALLOC, "%s+%ld", oldsym->name, (long) p[0]->data))
						error("optimized symbol+offset name too long");
					p[1]->type = T_SYMBOL;
					p[1]->data = (intptr_t)newsym;
				}
				nb = 1;
			}

			/*
			 *  __ldwi  j			-->	__ldwi sym+j
			 *  __add[bw]i sym
			 */
			else if
			((p[0]->code == I_ADDWI ||
			  p[0]->code == I_ADDBI) &&
			 (p[1]->code == I_LDWI) &&

			 (p[0]->type == T_SYMBOL) &&
			 (p[1]->type == T_VALUE)
			) {
				/* replace code */
				if (p[1]->data != 0) {
					SYMBOL * oldsym = (SYMBOL *)p[0]->data;
					SYMBOL * newsym = copysym(oldsym);
					if (NAMEALLOC <=
						snprintf(newsym->name, NAMEALLOC, "%s+%ld", oldsym->name, (long) p[1]->data))
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
			  p[0]->code == X_LEA_S ||
			  p[0]->code == I_LDB ||
			  p[0]->code == I_LDBP ||
			  p[0]->code == X_LDB_S ||
			  p[0]->code == I_LDUB ||
			  p[0]->code == I_LDUBP ||
			  p[0]->code == X_LDUB_S) &&
			 (p[1]->code == I_LDW ||
			  p[1]->code == I_LDWI ||
			  p[1]->code == X_LDW_S ||
			  p[1]->code == X_LEA_S ||
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
			 *  ...				-->	...
			 *  __addwi etc. 0
			 */
			else if
			((p[0]->code == I_ADDWI ||
			  p[0]->code == I_SUBWI ||
			  p[0]->code == I_ASLWI ||
			  p[0]->code == I_ASRWI ||
			  p[0]->code == I_LSRWI) &&

			 (p[0]->data == 0) &&
			 (p[0]->type == T_VALUE)
			) {
				/* remove code */
				nb = 1;
			}

			/*
			 *  @_stw_s i			-->	@_stw_s i
			 *  @_ldw_s i
			 */
			else if
			((p[0]->code == X_LDW_S) &&
			 (p[1]->code == X_STW_S) &&

			 (p[0]->data == p[1]->data)) {
				/* remove code */
				nb = 1;
			}

			/*
			 *  @_stb_s i			-->	@_stb_s i
			 *  @_ldb_s i
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
			 *  @_lea_s i			-->	@_pea_s i
			 *  __pushw
			 */
			else if
			((p[0]->code == I_PUSHW) &&
			 (p[1]->code == X_LEA_S)
			) {
				/* replace code */
				p[1]->code = X_PEA_S;
				nb = 1;
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
			 * subwi/addwi i		-->	is_load()
			 * is_load()
			 *
			 * This is a frequent case in which the result
			 * of a post-increment or decrement is not used.
			 */
			else if
			((p[1]->code == I_SUBWI ||
			  p[1]->code == I_ADDWI) &&
			 (p[1]->type == T_VALUE) &&
			 (is_load(p[0]))
			) {
				*p[1] = *p[0];
				nb = 1;
			}

			/*
			 *  is_load()			-->	__stwz
			 *  __stwi 0
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
			 *  is_load()			-->	__stbz
			 *  __stbi 0
			 */
			else if
			((p[1]->code == I_STBI) &&
			 (p[1]->imm_type == T_VALUE) &&
			 (p[1]->imm_data == 0) &&
			 (is_load(p[0]))
			) {
				p[1]->code = I_STBZ;
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

	/* optimization level 2 - instruction re-scheduler,
	 * change instruction order to allow direct assignments
	 * rather than stack based assignments :
	 *
	 *  @_pea_s i			-->	...
	 *    ...				@_stw_s i
	 *  __stwps
	 *
	 *  ====
	 *  bytes  : 25+??+42 = ??+ 67	--> ??+ 9 = ??+ 9
	 *  cycles : 44+??+91 = ??+135	--> ??+22 = ??+22
	 *
	 */
	if (optimize >= 2) {
		int offset;
		int i, j;
		int t;
		int jp;

		/* check last instruction */
		if (q_nb > 1 &&
		    (q_ins[q_wr].code == I_STWPS ||
		     q_ins[q_wr].code == I_STBPS ||
		     q_ins[q_wr].code == I_ADDWS ||
		     q_ins[q_wr].code == I_ORWS ||
		     q_ins[q_wr].code == I_EORWS ||
		     q_ins[q_wr].code == I_ANDWS)) {
			/* browse back the instruction list and
			 * etablish a stack history
			 */
			offset = 2;

			for (i = 1, j = q_wr; i < q_nb; i++) {
				j -= 1;

				if (j < 0)
					j += Q_SIZE;

				/* Index of insn precdeing j. */
				jp = j - 1;
				if (jp < 0)
					jp += Q_SIZE;

				/* check instruction */
				switch (q_ins[j].code) {
				case I_JSR:
				case I_CMPB:
				case I_CMPW:
					if (q_ins[j].type == T_LIB)
						offset += 2;
					break;

				case I_MODSP:
					if ((q_ins[j].type == T_STACK) ||
					    (q_ins[j].type == T_NOP))
						offset += (int)q_ins[j].data;
					break;

				case I_ADDBS:
				case I_ADDWS:
				case I_SUBWS:
				case I_ORWS:
				case I_EORWS:
				case I_ANDWS:
				case I_POPW:
				case I_STWPS:
				case I_STBPS:
					offset += 2;
					break;

				case I_PUSHW:
				case X_PEA_S:
					offset -= 2;
					break;
				}

				/* check offset */
				if (offset == 0) {
					/* good! */
					if (i == 1) {
						/* hmm, may be not...
						 * there should be at least one instruction
						 * between pea_s and stwps.
						 * this case should never happen, though,
						 * but better skipping it
						 */
						break;
					}

					/* check the first instruction
					 */
					{
						/* Only handle sequences that start with
						   pea_s or ldwi/pushw. */
						if (q_ins[j].code != X_PEA_S &&
						    (q_ins[j].code != I_PUSHW || q_ins[jp].code != I_LDWI)
						    )
							break;

						if (q_ins[j].code == X_PEA_S &&
						    q_ins[q_wr].code != I_STBPS &&
						    q_ins[q_wr].code != I_STWPS)
							break;

						/* change stwps into stw_s/stw */
						if (q_ins[j].code == X_PEA_S) {
							if (q_ins[q_wr].code == I_STBPS)
								q_ins[q_wr].code = X_STB_S;
							else
								q_ins[q_wr].code = X_STW_S;
							q_ins[q_wr].data = q_ins[j].data;
						}
						else {
							switch (q_ins[q_wr].code) {
							case I_STBPS:
								q_ins[q_wr].code = I_STB;
								break;
							case I_STWPS:
								q_ins[q_wr].code = I_STW;
								break;
							case I_ADDWS:
								q_ins[q_wr].code = I_ADDWI;
								break;
							case I_ORWS:
								q_ins[q_wr].code = I_ORWI;
								break;
							case I_EORWS:
								q_ins[q_wr].code = I_EORWI;
								break;
							case I_ANDWS:
								q_ins[q_wr].code = I_ANDWI;
								break;
							default:
								abort();
							}
							/* Use data from the preceding ldwi. */
							q_ins[q_wr].type = q_ins[jp].type;
							q_ins[q_wr].data = q_ins[jp].data;
						}
					}

					/* adjust stack references;
					 * because of the removal of pea_s
					 */
					for (t = i; t > 1; t--) {
						j += 1;
						if (j >= Q_SIZE)
							j -= Q_SIZE;

						/* check instruction */
						if (is_sprel(&q_ins[j])) {
							/* adjust stack offset */
							q_ins[j].data -= 2;
						}
					}

					/* remove all the instructions... */
					q_wr -= (i + 1);
					q_nb -= (i + 1);
					j -= (i - 1);

					if (q_wr < 0)
						q_wr += Q_SIZE;
					if (j < 0)
						j += Q_SIZE;

					/* ... and re-insert them one by one
					 * in the queue (for further optimizations)
					 */
					for (; i > 0; i--) {
						j += 1;
						if (j >= Q_SIZE)
							j -= Q_SIZE;
#ifdef DEBUG_OPTIMIZER
						printf("\nReinserting after rescheduling ...");
#endif
						push_ins(&q_ins[j]);
					}
					break;
				}
			}
		}

		if (q_nb >= 3) {
			/*
			 *  __pushw			-->	__stw __ptr
			 *  <load>				<load>
			 *  __st*ps				__st*p __ptr
			 *
			 *  This cannot be done earlier because it screws up
			 *  the reordering optimization above.
			 */
			if
			((q_ins[q_wr].code == I_STBPS ||
			  q_ins[q_wr].code == I_STWPS) &&
			 (is_load(&q_ins[q_wr - 1])) &&
			 (q_ins[q_wr - 2].code == I_PUSHW)
			) {
				q_ins[q_wr - 2].code = I_STW;
				q_ins[q_wr - 2].type = T_PTR;
				/* We just removed a push, adjust SP-relative
				   addresses. */
				if (is_sprel(&q_ins[q_wr - 1]))
					q_ins[q_wr - 1].data -= 2;
				if (q_ins[q_wr].code == I_STBPS)
					q_ins[q_wr].code = I_STBP;
				else
					q_ins[q_wr].code = I_STWP;
				q_ins[q_wr].type = T_PTR;
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
