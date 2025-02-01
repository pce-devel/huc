/*	File opt.c: 2.1 (83/03/20,16:02:09) */
/*% cc -O -c %
 *
 */

// #define DEBUG_OPTIMIZER
// #define INFORM_VALUE_SWAP

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
#include "gen.h"
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

#ifdef DEBUG_OPTIMIZER
#define ODEBUG(...) printf( __VA_ARGS__ )
#else
#define ODEBUG(...)
#endif


/* flag information for each of the i-code instructions */
/*
 * N.B. this table MUST be kept updated and in the same order as the i-code
 * enum list in defs.h
 */
unsigned char icode_flags[] = {
	// i-code to mark an instruction as retired */

	/* I_RETIRED            */	0,

	// i-code for internal compiler information

	/* I_INFO               */	0,

	// i-code that retires the primary register contents

	/* I_FENCE              */	0,

	// i-code that declares a byte sized primary register

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
	/* I_FUNCP_WR           */	0,
	/* I_CALLP              */	0,

	// i-codes for C functions and the C parameter stack

	/* I_ENTER              */	0,
	/* I_RETURN             */	0,
	/* I_MODSP              */	0,
	/* I_PUSHARG_WR         */	IS_USEPR,
	/* I_PUSH_WR            */	IS_USEPR + IS_PUSHWT,
	/* I_POP_WR             */	IS_POPWT,
	/* I_SPUSH_WR           */	IS_USEPR,
	/* I_SPUSH_UR           */	IS_USEPR,
	/* I_SPOP_WR            */	0,
	/* I_SPOP_UR            */	0,

	// i-codes for handling boolean tests and branching

	/* I_SWITCH_WR          */	IS_USEPR,
	/* I_SWITCH_UR          */	IS_USEPR,
	/* I_DEFAULT            */	0,
	/* I_CASE               */	0,
	/* I_ENDCASE            */	0,
	/* I_LABEL              */	0,
	/* I_ALIAS              */	0,
	/* I_BRA                */	0,
	/* I_BFALSE             */	0,
	/* I_BTRUE              */	0,
	/* I_DEF                */	0,

	/* I_CMP_WT             */	IS_USEPR + IS_POPWT,
	/* X_CMP_WI             */	IS_USEPR,
	/* X_CMP_WM             */	IS_USEPR,
	/* X_CMP_UM             */	IS_USEPR,
	/* X_CMP_WS             */	IS_USEPR + IS_SPREL,
	/* X_CMP_US             */	IS_USEPR + IS_SPREL,
	/* X_CMP_WAX            */	IS_USEPR,
	/* X_CMP_UAX            */	IS_USEPR,

	/* X_CMP_UIQ            */	IS_USEPR + IS_UBYTE,
	/* X_CMP_UMQ            */	IS_USEPR + IS_UBYTE,
	/* X_CMP_USQ            */	IS_USEPR + IS_SPREL + IS_UBYTE,
	/* X_CMP_UAXQ           */	IS_USEPR + IS_UBYTE,

	/* I_NOT_WR             */	IS_USEPR,
	/* X_NOT_WP             */	0,
	/* X_NOT_WM             */	0,
	/* X_NOT_WS             */	IS_SPREL,
	/* X_NOT_WAR            */	0,
	/* X_NOT_WAX            */	0,
	/* X_NOT_UP             */	0,
	/* X_NOT_UM             */	0,
	/* X_NOT_US             */	IS_SPREL,
	/* X_NOT_UAR            */	0,
	/* X_NOT_UAX            */	0,
	/* X_NOT_UAY            */	0,

	/* I_TST_WR             */	IS_USEPR,
	/* X_TST_WP             */	0,
	/* X_TST_WM             */	0,
	/* X_TST_WS             */	IS_SPREL,
	/* X_TST_WAR            */	0,
	/* X_TST_WAX            */	0,
	/* X_TST_UP             */	0,
	/* X_TST_UM             */	0,
	/* X_TST_US             */	IS_SPREL,
	/* X_TST_UAR            */	0,
	/* X_TST_UAX            */	0,
	/* X_TST_UAY            */	0,

	/* X_NAND_WI            */	IS_USEPR,
	/* X_TAND_WI            */	IS_USEPR,

	/* X_NOT_CF             */	0,

	/* I_BOOLEAN            */	0,

	/* X_BOOLNOT_WR         */	IS_USEPR,
	/* X_BOOLNOT_WP         */	0,
	/* X_BOOLNOT_WM         */	0,
	/* X_BOOLNOT_WS         */	IS_SPREL,
	/* X_BOOLNOT_WAR        */	0,
	/* X_BOOLNOT_WAX        */	0,

	/* X_BOOLNOT_UP         */	0,
	/* X_BOOLNOT_UM         */	0,
	/* X_BOOLNOT_US         */	IS_SPREL,
	/* X_BOOLNOT_UAR        */	0,
	/* X_BOOLNOT_UAX        */	0,
	/* X_BOOLNOT_UAY        */	0,

	// i-codes for loading the primary register

	/* I_LD_WI              */	0,
	/* X_LD_UIQ             */	IS_SHORT,
	/* I_LEA_S              */	IS_SPREL,

	/* I_LD_WM              */	0,
	/* I_LD_BM              */	IS_SBYTE,
	/* I_LD_UM              */	IS_UBYTE,

	/* X_LD_WMQ             */	0,
	/* X_LD_BMQ             */	IS_SBYTE,
	/* X_LD_UMQ             */	IS_UBYTE,

	/* X_LDX_WMQ            */	0,
	/* X_LDX_BMQ            */	IS_SBYTE,
	/* X_LDX_UMQ            */	IS_UBYTE,

	/* X_LD2X_WM            */	0,
	/* X_LD2X_BM            */	IS_SBYTE,
	/* X_LD2X_UM            */	IS_UBYTE,

	/* X_LD2X_WMQ           */	0,
	/* X_LD2X_BMQ           */	IS_SBYTE,
	/* X_LD2X_UMQ           */	IS_UBYTE,

	/* X_LDY_WMQ            */	0,
	/* X_LDY_BMQ            */	IS_SBYTE,
	/* X_LDY_UMQ            */	IS_UBYTE,

	/* I_LD_WP              */	0,
	/* I_LD_BP              */	IS_SBYTE,
	/* I_LD_UP              */	IS_UBYTE,

	/* X_LD_UPQ             */	IS_UBYTE + IS_SHORT,

	/* X_LD_WAR             */	0,
	/* X_LD_BAR             */	IS_SBYTE,
	/* X_LD_UAR             */	IS_UBYTE,

	/* X_LD_UARQ            */	IS_UBYTE + IS_SHORT,

	/* X_LD_WAX             */	0,
	/* X_LD_BAX             */	IS_SBYTE,
	/* X_LD_UAX             */	IS_UBYTE,

	/* X_LD_UAXQ            */	IS_UBYTE + IS_SHORT,

	/* X_LD_BAY             */	IS_SBYTE,
	/* X_LD_UAY             */	IS_UBYTE,

	/* X_LD_WS              */	IS_SPREL,
	/* X_LD_BS              */	IS_SBYTE + IS_SPREL,
	/* X_LD_US              */	IS_UBYTE + IS_SPREL,

	/* X_LD_WSQ             */	IS_SPREL,
	/* X_LD_BSQ             */	IS_SBYTE + IS_SPREL,
	/* X_LD_USQ             */	IS_UBYTE + IS_SPREL,

	/* X_LDX_WS             */	IS_SPREL,
	/* X_LDX_BS             */	IS_SBYTE + IS_SPREL,
	/* X_LDX_US             */	IS_UBYTE + IS_SPREL,

	/* X_LDX_WSQ            */	IS_SPREL,
	/* X_LDX_BSQ            */	IS_SBYTE + IS_SPREL,
	/* X_LDX_USQ            */	IS_UBYTE + IS_SPREL,

	/* X_LD2X_WS            */	IS_SPREL,
	/* X_LD2X_BS            */	IS_SBYTE + IS_SPREL,
	/* X_LD2X_US            */	IS_UBYTE + IS_SPREL,

	/* X_LD2X_WSQ           */	IS_SPREL,
	/* X_LD2X_BSQ           */	IS_SBYTE + IS_SPREL,
	/* X_LD2X_USQ           */	IS_UBYTE + IS_SPREL,

	/* X_LDY_WSQ            */	IS_SPREL,
	/* X_LDY_BSQ            */	IS_SBYTE + IS_SPREL,
	/* X_LDY_USQ            */	IS_UBYTE + IS_SPREL,

	/* X_LDP_WAR            */	IS_PUSHWT,
	/* X_LDP_BAR            */	IS_SBYTE + IS_PUSHWT,
	/* X_LDP_UAR            */	IS_UBYTE + IS_PUSHWT,

	/* X_LDP_WAX            */	IS_PUSHWT,
	/* X_LDP_BAX            */	IS_SBYTE + IS_PUSHWT,
	/* X_LDP_UAX            */	IS_UBYTE + IS_PUSHWT,

	/* X_LDP_BAY            */	IS_SBYTE + IS_PUSHWT,
	/* X_LDP_UAY            */	IS_UBYTE + IS_PUSHWT,

	// i-codes for pre- and post- increment and decrement

	/* X_INCLD_WM           */	0,
	/* X_LDINC_WM           */	0,
	/* X_DECLD_WM           */	0,
	/* X_LDDEC_WM           */	0,

	/* X_INCLD_BM           */	IS_SBYTE,
	/* X_LDINC_BM           */	IS_SBYTE,
	/* X_DECLD_BM           */	IS_SBYTE,
	/* X_LDDEC_BM           */	IS_SBYTE,

	/* X_INCLD_UM           */	IS_UBYTE,
	/* X_LDINC_UM           */	IS_UBYTE,
	/* X_DECLD_UM           */	IS_UBYTE,
	/* X_LDDEC_UM           */	IS_UBYTE,

	/* X_INC_WMQ            */	0,
	/* X_INC_UMQ            */	0,

	/* X_DEC_WMQ            */	0,
	/* X_DEC_UMQ            */	0,

	/* X_INCLD_WS           */	IS_SPREL,
	/* X_LDINC_WS           */	IS_SPREL,
	/* X_DECLD_WS           */	IS_SPREL,
	/* X_LDDEC_WS           */	IS_SPREL,

	/* X_INCLD_BS           */	IS_SBYTE + IS_SPREL,
	/* X_LDINC_BS           */	IS_SBYTE + IS_SPREL,
	/* X_DECLD_BS           */	IS_SBYTE + IS_SPREL,
	/* X_LDDEC_BS           */	IS_SBYTE + IS_SPREL,

	/* X_INCLD_US           */	IS_UBYTE + IS_SPREL,
	/* X_LDINC_US           */	IS_UBYTE + IS_SPREL,
	/* X_DECLD_US           */	IS_UBYTE + IS_SPREL,
	/* X_LDDEC_US           */	IS_UBYTE + IS_SPREL,

	/* X_INC_WSQ            */	IS_SPREL,
	/* X_INC_USQ            */	IS_SPREL,

	/* X_DEC_WSQ            */	IS_SPREL,
	/* X_DEC_USQ            */	IS_SPREL,

	/* X_INCLD_WAR          */	0,
	/* X_LDINC_WAR          */	0,
	/* X_DECLD_WAR          */	0,
	/* X_LDDEC_WAR          */	0,

	/* X_INCLD_BAR          */	IS_SBYTE,
	/* X_LDINC_BAR          */	IS_SBYTE,
	/* X_DECLD_BAR          */	IS_SBYTE,
	/* X_LDDEC_BAR          */	IS_SBYTE,

	/* X_INCLD_UAR          */	IS_UBYTE,
	/* X_LDINC_UAR          */	IS_UBYTE,
	/* X_DECLD_UAR          */	IS_UBYTE,
	/* X_LDDEC_UAR          */	IS_UBYTE,

	/* X_INC_WARQ           */	0,
	/* X_INC_UARQ           */	0,

	/* X_DEC_WARQ           */	0,
	/* X_DEC_UARQ           */	0,

	/* X_INCLD_WAX          */	0,
	/* X_LDINC_WAX          */	0,
	/* X_DECLD_WAX          */	0,
	/* X_LDDEC_WAX          */	0,

	/* X_INCLD_BAX          */	IS_SBYTE,
	/* X_LDINC_BAX          */	IS_SBYTE,
	/* X_DECLD_BAX          */	IS_SBYTE,
	/* X_LDDEC_BAX          */	IS_SBYTE,

	/* X_INCLD_UAX          */	IS_UBYTE,
	/* X_LDINC_UAX          */	IS_UBYTE,
	/* X_DECLD_UAX          */	IS_UBYTE,
	/* X_LDDEC_UAX          */	IS_UBYTE,

	/* X_INC_WAXQ           */	0,
	/* X_INC_UAXQ           */	0,

	/* X_DEC_WAXQ           */	0,
	/* X_DEC_UAXQ           */	0,

	/* X_INCLD_BAY          */	IS_SBYTE,
	/* X_LDINC_BAY          */	IS_SBYTE,
	/* X_DECLD_BAY          */	IS_SBYTE,
	/* X_LDDEC_BAY          */	IS_SBYTE,

	/* X_INCLD_UAY          */	IS_UBYTE,
	/* X_LDINC_UAY          */	IS_UBYTE,
	/* X_DECLD_UAY          */	IS_UBYTE,
	/* X_LDDEC_UAY          */	IS_UBYTE,

	/* X_INC_UAYQ           */	0,

	/* X_DEC_UAYQ           */	0,

	// i-codes for saving the primary register

	/* I_ST_WPT             */	IS_USEPR + IS_STORE + IS_POPWT,
	/* I_ST_UPT             */	IS_USEPR + IS_STORE + IS_POPWT,
	/* X_ST_WPTQ            */	IS_USEPR + IS_STORE + IS_POPWT + IS_SHORT,
	/* X_ST_UPTQ            */	IS_USEPR + IS_STORE + IS_POPWT + IS_SHORT,

	/* I_ST_WM              */	IS_USEPR + IS_STORE,
	/* I_ST_UM              */	IS_USEPR + IS_STORE,
	/* X_ST_WMQ             */	IS_USEPR + IS_STORE + IS_SHORT,
	/* X_ST_UMQ             */	IS_USEPR + IS_STORE + IS_SHORT,
	/* I_ST_WMIQ            */	IS_USEPR + IS_STORE + IS_SHORT,
	/* I_ST_UMIQ            */	IS_USEPR + IS_STORE + IS_SHORT,
	/* X_ST_WP              */	IS_USEPR + IS_STORE,
	/* X_ST_UP              */	IS_USEPR + IS_STORE,
	/* X_ST_WPQ             */	IS_USEPR + IS_STORE + IS_SHORT,
	/* X_ST_UPQ             */	IS_USEPR + IS_STORE + IS_SHORT,
	/* X_ST_WPI             */	IS_USEPR + IS_STORE,
	/* X_ST_UPI             */	IS_USEPR + IS_STORE,
	/* X_ST_WS              */	IS_USEPR + IS_STORE + IS_SPREL,
	/* X_ST_US              */	IS_USEPR + IS_STORE + IS_SPREL,
	/* X_ST_WSQ             */	IS_USEPR + IS_STORE + IS_SPREL + IS_SHORT,
	/* X_ST_USQ             */	IS_USEPR + IS_STORE + IS_SPREL + IS_SHORT,
	/* I_ST_WSIQ            */	IS_USEPR + IS_STORE + IS_SPREL + IS_SHORT,
	/* I_ST_USIQ            */	IS_USEPR + IS_STORE + IS_SPREL + IS_SHORT,

	/* X_INDEX_WR           */	IS_USEPR + IS_PUSHWT,
	/* X_INDEX_UR           */	IS_USEPR + IS_PUSHWT,

	/* X_ST_WAT             */	IS_USEPR + IS_STORE + IS_POPWT,
	/* X_ST_UAT             */	IS_USEPR + IS_STORE + IS_POPWT,
	/* X_ST_WATQ            */	IS_USEPR + IS_STORE + IS_POPWT + IS_SHORT,
	/* X_ST_UATQ            */	IS_USEPR + IS_STORE + IS_POPWT + IS_SHORT,
	/* X_ST_WATIQ           */	IS_USEPR + IS_STORE + IS_POPWT + IS_SHORT,
	/* X_ST_UATIQ           */	IS_USEPR + IS_STORE + IS_POPWT + IS_SHORT,

	/* X_ST_WAX             */	IS_USEPR + IS_STORE,
	/* X_ST_UAX             */	IS_USEPR + IS_STORE,
	/* X_ST_WAXQ            */	IS_USEPR + IS_STORE + IS_SHORT,
	/* X_ST_UAXQ            */	IS_USEPR + IS_STORE + IS_SHORT,
	/* X_ST_WAXIQ           */	IS_USEPR + IS_STORE + IS_SHORT,
	/* X_ST_UAXIQ           */	IS_USEPR + IS_STORE + IS_SHORT,

	// i-codes for extending the primary register

	/* I_EXT_BR             */	IS_USEPR,
	/* I_EXT_UR             */	IS_USEPR,

	// i-codes for 16-bit math with the primary register

	/* I_COM_WR             */	IS_USEPR,
	/* I_NEG_WR             */	IS_USEPR,

	/* I_ADD_WT             */	IS_USEPR + IS_POPWT,
	/* I_ADD_WI             */	IS_USEPR,

	/* X_ADD_WM             */	IS_USEPR,
	/* X_ADD_UM             */	IS_USEPR,
	/* X_ADD_WP             */	IS_USEPR,
	/* X_ADD_UP             */	IS_USEPR,
	/* X_ADD_WS             */	IS_USEPR + IS_SPREL,
	/* X_ADD_US             */	IS_USEPR + IS_SPREL,
	/* X_ADD_WAT            */	IS_USEPR + IS_POPWT,
	/* X_ADD_UAT            */	IS_USEPR + IS_POPWT,
	/* X_ADD_WAX            */	IS_USEPR,
	/* X_ADD_UAX            */	IS_USEPR,

	/* I_SUB_WT             */	IS_USEPR + IS_POPWT,
	/* I_SUB_WI             */	IS_USEPR,

	/* X_SUB_WM             */	IS_USEPR,
	/* X_SUB_UM             */	IS_USEPR,
	/* X_SUB_WP             */	IS_USEPR,
	/* X_SUB_UP             */	IS_USEPR,
	/* X_SUB_WS             */	IS_USEPR + IS_SPREL,
	/* X_SUB_US             */	IS_USEPR + IS_SPREL,
	/* X_SUB_WAT            */	IS_USEPR + IS_POPWT,
	/* X_SUB_UAT            */	IS_USEPR + IS_POPWT,
	/* X_SUB_WAX            */	IS_USEPR,
	/* X_SUB_UAX            */	IS_USEPR,

	/* X_ISUB_WT            */	IS_USEPR + IS_POPWT,
	/* X_ISUB_WI            */	IS_USEPR,

	/* X_ISUB_WM            */	IS_USEPR,
	/* X_ISUB_UM            */	IS_USEPR,
	/* X_ISUB_WP            */	IS_USEPR,
	/* X_ISUB_UP            */	IS_USEPR,
	/* X_ISUB_WS            */	IS_USEPR + IS_SPREL,
	/* X_ISUB_US            */	IS_USEPR + IS_SPREL,
	/* X_ISUB_WAT           */	IS_USEPR + IS_POPWT,
	/* X_ISUB_UAT           */	IS_USEPR + IS_POPWT,
	/* X_ISUB_WAX           */	IS_USEPR,
	/* X_ISUB_UAX           */	IS_USEPR,

	/* I_AND_WT             */	IS_USEPR + IS_POPWT,
	/* X_AND_WI             */	IS_USEPR,

	/* X_AND_WM             */	IS_USEPR,
	/* X_AND_UM             */	IS_USEPR,
	/* X_AND_WP             */	IS_USEPR,
	/* X_AND_UP             */	IS_USEPR,
	/* X_AND_WS             */	IS_USEPR + IS_SPREL,
	/* X_AND_US             */	IS_USEPR + IS_SPREL,
	/* X_AND_WAT            */	IS_USEPR + IS_POPWT,
	/* X_AND_UAT            */	IS_USEPR + IS_POPWT,
	/* X_AND_WAX            */	IS_USEPR,
	/* X_AND_UAX            */	IS_USEPR,

	/* I_EOR_WT             */	IS_USEPR + IS_POPWT,
	/* X_EOR_WI             */	IS_USEPR,

	/* X_EOR_WM             */	IS_USEPR,
	/* X_EOR_UM             */	IS_USEPR,
	/* X_EOR_WP             */	IS_USEPR,
	/* X_EOR_UP             */	IS_USEPR,
	/* X_EOR_WS             */	IS_USEPR + IS_SPREL,
	/* X_EOR_US             */	IS_USEPR + IS_SPREL,
	/* X_EOR_WAT            */	IS_USEPR + IS_POPWT,
	/* X_EOR_UAT            */	IS_USEPR + IS_POPWT,
	/* X_EOR_WAX            */	IS_USEPR,
	/* X_EOR_UAX            */	IS_USEPR,

	/* I_OR_WT              */	IS_USEPR + IS_POPWT,
	/* X_OR_WI              */	IS_USEPR,

	/* X_OR_WM              */	IS_USEPR,
	/* X_OR_UM              */	IS_USEPR,
	/* X_OR_WP              */	IS_USEPR,
	/* X_OR_UP              */	IS_USEPR,
	/* X_OR_WS              */	IS_USEPR + IS_SPREL,
	/* X_OR_US              */	IS_USEPR + IS_SPREL,
	/* X_OR_WAT             */	IS_USEPR + IS_POPWT,
	/* X_OR_UAT             */	IS_USEPR + IS_POPWT,
	/* X_OR_WAX             */	IS_USEPR,
	/* X_OR_UAX             */	IS_USEPR,

	/* I_ASL_WR             */	IS_USEPR,
	/* I_ASL_WT             */	IS_USEPR + IS_POPWT,
	/* I_ASL_WI             */	IS_USEPR,

	/* I_ASR_WT             */	IS_USEPR + IS_POPWT,
	/* I_ASR_WI             */	IS_USEPR,

	/* I_LSR_WT             */	IS_USEPR + IS_POPWT,
	/* I_LSR_WI             */	IS_USEPR,

	/* I_MUL_WT             */	IS_USEPR + IS_POPWT,
	/* I_MUL_WI             */	IS_USEPR,

	/* I_SDIV_WT            */	IS_USEPR + IS_POPWT,
	/* I_SDIV_WI            */	IS_USEPR,

	/* I_UDIV_WT            */	IS_USEPR + IS_POPWT,
	/* I_UDIV_WI            */	IS_USEPR,
	/* I_UDIV_UI            */	IS_USEPR + IS_UBYTE,

	/* I_SMOD_WT            */	IS_USEPR + IS_POPWT,
	/* I_SMOD_WI            */	IS_USEPR,

	/* I_UMOD_WT            */	IS_USEPR + IS_POPWT,
	/* I_UMOD_WI            */	IS_USEPR,
	/* I_UMOD_UI            */	IS_USEPR + IS_UBYTE,

	/* I_DOUBLE_WT          */	0,

	// i-codes for 8-bit math with lo-byte of the primary register

	/* X_ADD_UIQ            */	IS_USEPR + IS_UBYTE + IS_SHORT,
	/* X_ADD_UMQ            */	IS_USEPR + IS_UBYTE + IS_SHORT,
	/* X_ADD_UPQ            */	IS_USEPR + IS_UBYTE + IS_SHORT,
	/* X_ADD_USQ            */	IS_USEPR + IS_UBYTE + IS_SPREL + IS_SHORT,
	/* X_ADD_UATQ           */	IS_USEPR + IS_UBYTE + IS_POPWT + IS_SHORT,
	/* X_ADD_UAXQ           */	IS_USEPR + IS_UBYTE + IS_SHORT,

	/* X_SUB_UIQ            */	IS_USEPR + IS_UBYTE + IS_SHORT,
	/* X_SUB_UMQ            */	IS_USEPR + IS_UBYTE + IS_SHORT,
	/* X_SUB_UPQ            */	IS_USEPR + IS_UBYTE + IS_SHORT,
	/* X_SUB_USQ            */	IS_USEPR + IS_UBYTE + IS_SPREL + IS_SHORT,
	/* X_SUB_UATQ           */	IS_USEPR + IS_UBYTE + IS_POPWT + IS_SHORT,
	/* X_SUB_UAXQ           */	IS_USEPR + IS_UBYTE + IS_SHORT,

	/* X_ISUB_UIQ           */	IS_USEPR + IS_UBYTE + IS_SHORT,
	/* X_ISUB_UMQ           */	IS_USEPR + IS_UBYTE + IS_SHORT,
	/* X_ISUB_UPQ           */	IS_USEPR + IS_UBYTE + IS_SHORT,
	/* X_ISUB_USQ           */	IS_USEPR + IS_UBYTE + IS_SPREL + IS_SHORT,
	/* X_ISUB_UATQ          */	IS_USEPR + IS_UBYTE + IS_POPWT + IS_SHORT,
	/* X_ISUB_UAXQ          */	IS_USEPR + IS_UBYTE + IS_SHORT,

	/* X_AND_UIQ            */	IS_USEPR + IS_UBYTE + IS_SHORT,
	/* X_AND_UMQ            */	IS_USEPR + IS_UBYTE + IS_SHORT,
	/* X_AND_UPQ            */	IS_USEPR + IS_UBYTE + IS_SHORT,
	/* X_AND_USQ            */	IS_USEPR + IS_UBYTE + IS_SPREL + IS_SHORT,
	/* X_AND_UATQ           */	IS_USEPR + IS_UBYTE + IS_POPWT + IS_SHORT,
	/* X_AND_UAXQ           */	IS_USEPR + IS_UBYTE + IS_SHORT,

	/* X_EOR_UIQ            */	IS_USEPR + IS_UBYTE + IS_SHORT,
	/* X_EOR_UMQ            */	IS_USEPR + IS_UBYTE + IS_SHORT,
	/* X_EOR_UPQ            */	IS_USEPR + IS_UBYTE + IS_SHORT,
	/* X_EOR_USQ            */	IS_USEPR + IS_UBYTE + IS_SPREL + IS_SHORT,
	/* X_EOR_UATQ           */	IS_USEPR + IS_UBYTE + IS_POPWT + IS_SHORT,
	/* X_EOR_UAXQ           */	IS_USEPR + IS_UBYTE + IS_SHORT,

	/* X_OR_UIQ             */	IS_USEPR + IS_UBYTE + IS_SHORT,
	/* X_OR_UMQ             */	IS_USEPR + IS_UBYTE + IS_SHORT,
	/* X_OR_UPQ             */	IS_USEPR + IS_UBYTE + IS_SHORT,
	/* X_OR_USQ             */	IS_USEPR + IS_UBYTE + IS_SPREL + IS_SHORT,
	/* X_OR_UATQ            */	IS_USEPR + IS_UBYTE + IS_POPWT + IS_SHORT,
	/* X_OR_UAXQ            */	IS_USEPR + IS_UBYTE + IS_SHORT,

	/* I_ASL_UIQ            */	IS_USEPR + IS_UBYTE,

	/* I_LSR_UIQ            */	IS_USEPR + IS_UBYTE + IS_SHORT,

	/* I_MUL_UIQ            */	IS_USEPR + IS_UBYTE,

	// i-codes for modifying a variable with "+=", "-=", "&=", "^=", "|="

	/* X_ADD_ST_WMQ         */	IS_USEPR + IS_STORE,
	/* X_ADD_ST_UMQ         */	IS_USEPR + IS_STORE + IS_SHORT,
	/* X_ADD_ST_WPQ         */	IS_USEPR + IS_STORE,
	/* X_ADD_ST_UPQ         */	IS_USEPR + IS_STORE + IS_SHORT,
	/* X_ADD_ST_WSQ         */	IS_USEPR + IS_STORE + IS_SPREL,
	/* X_ADD_ST_USQ         */	IS_USEPR + IS_STORE + IS_SPREL + IS_SHORT,
	/* X_ADD_ST_WATQ        */	IS_USEPR + IS_STORE + IS_POPWT,
	/* X_ADD_ST_UATQ        */	IS_USEPR + IS_STORE + IS_POPWT + IS_SHORT,
	/* X_ADD_ST_WAXQ        */	IS_USEPR + IS_STORE,
	/* X_ADD_ST_UAXQ        */	IS_USEPR + IS_STORE + IS_SHORT,

	/* X_ISUB_ST_WMQ        */	IS_USEPR + IS_STORE,
	/* X_ISUB_ST_UMQ        */	IS_USEPR + IS_STORE + IS_SHORT,
	/* X_ISUB_ST_WPQ        */	IS_USEPR + IS_STORE,
	/* X_ISUB_ST_UPQ        */	IS_USEPR + IS_STORE + IS_SHORT,
	/* X_ISUB_ST_WSQ        */	IS_USEPR + IS_STORE + IS_SPREL,
	/* X_ISUB_ST_USQ        */	IS_USEPR + IS_STORE + IS_SPREL + IS_SHORT,
	/* X_ISUB_ST_WATQ       */	IS_USEPR + IS_STORE + IS_POPWT,
	/* X_ISUB_ST_UATQ       */	IS_USEPR + IS_STORE + IS_POPWT + IS_SHORT,
	/* X_ISUB_ST_WAXQ       */	IS_USEPR + IS_STORE,
	/* X_ISUB_ST_UAXQ       */	IS_USEPR + IS_STORE + IS_SHORT,

	/* X_AND_ST_WMQ         */	IS_USEPR + IS_STORE,
	/* X_AND_ST_UMQ         */	IS_USEPR + IS_STORE + IS_SHORT,
	/* X_AND_ST_WPQ         */	IS_USEPR + IS_STORE,
	/* X_AND_ST_UPQ         */	IS_USEPR + IS_STORE + IS_SHORT,
	/* X_AND_ST_WSQ         */	IS_USEPR + IS_STORE + IS_SPREL,
	/* X_AND_ST_USQ         */	IS_USEPR + IS_STORE + IS_SPREL + IS_SHORT,
	/* X_AND_ST_WATQ        */	IS_USEPR + IS_STORE + IS_POPWT,
	/* X_AND_ST_UATQ        */	IS_USEPR + IS_STORE + IS_POPWT + IS_SHORT,
	/* X_AND_ST_WAXQ        */	IS_USEPR + IS_STORE,
	/* X_AND_ST_UAXQ        */	IS_USEPR + IS_STORE + IS_SHORT,

	/* X_EOR_ST_WMQ         */	IS_USEPR + IS_STORE,
	/* X_EOR_ST_UMQ         */	IS_USEPR + IS_STORE + IS_SHORT,
	/* X_EOR_ST_WPQ         */	IS_USEPR + IS_STORE,
	/* X_EOR_ST_UPQ         */	IS_USEPR + IS_STORE + IS_SHORT,
	/* X_EOR_ST_WSQ         */	IS_USEPR + IS_STORE + IS_SPREL,
	/* X_EOR_ST_USQ         */	IS_USEPR + IS_STORE + IS_SPREL + IS_SHORT,
	/* X_EOR_ST_WATQ        */	IS_USEPR + IS_STORE + IS_POPWT,
	/* X_EOR_ST_UATQ        */	IS_USEPR + IS_STORE + IS_POPWT + IS_SHORT,
	/* X_EOR_ST_WAXQ        */	IS_USEPR + IS_STORE,
	/* X_EOR_ST_UAXQ        */	IS_USEPR + IS_STORE + IS_SHORT,

	/* X_OR_ST_WMQ          */	IS_USEPR + IS_STORE,
	/* X_OR_ST_UMQ          */	IS_USEPR + IS_STORE + IS_SHORT,
	/* X_OR_ST_WPQ          */	IS_USEPR + IS_STORE,
	/* X_OR_ST_UPQ          */	IS_USEPR + IS_STORE + IS_SHORT,
	/* X_OR_ST_WSQ          */	IS_USEPR + IS_STORE + IS_SPREL,
	/* X_OR_ST_USQ          */	IS_USEPR + IS_STORE + IS_SPREL + IS_SHORT,
	/* X_OR_ST_WATQ         */	IS_USEPR + IS_STORE + IS_POPWT,
	/* X_OR_ST_UATQ         */	IS_USEPR + IS_STORE + IS_POPWT + IS_SHORT,
	/* X_OR_ST_WAXQ         */	IS_USEPR + IS_STORE,
	/* X_OR_ST_UAXQ         */	IS_USEPR + IS_STORE + IS_SHORT,

	/* X_ADD_ST_WMIQ        */	IS_USEPR + IS_STORE,
	/* X_ADD_ST_UMIQ        */	IS_USEPR + IS_STORE + IS_SHORT,
	/* X_ADD_ST_WPIQ        */	IS_USEPR + IS_STORE,
	/* X_ADD_ST_UPIQ        */	IS_USEPR + IS_STORE + IS_SHORT,
	/* X_ADD_ST_WSIQ        */	IS_USEPR + IS_STORE + IS_SPREL,
	/* X_ADD_ST_USIQ        */	IS_USEPR + IS_STORE + IS_SPREL + IS_SHORT,
	/* X_ADD_ST_WATIQ       */	IS_USEPR + IS_STORE + IS_POPWT,
	/* X_ADD_ST_UATIQ       */	IS_USEPR + IS_STORE + IS_POPWT + IS_SHORT,
	/* X_ADD_ST_WAXIQ       */	IS_USEPR + IS_STORE,
	/* X_ADD_ST_UAXIQ       */	IS_USEPR + IS_STORE + IS_SHORT,

	/* X_SUB_ST_WMIQ        */	IS_USEPR + IS_STORE,
	/* X_SUB_ST_UMIQ        */	IS_USEPR + IS_STORE + IS_SHORT,
	/* X_SUB_ST_WPIQ        */	IS_USEPR + IS_STORE,
	/* X_SUB_ST_UPIQ        */	IS_USEPR + IS_STORE + IS_SHORT,
	/* X_SUB_ST_WSIQ        */	IS_USEPR + IS_STORE + IS_SPREL,
	/* X_SUB_ST_USIQ        */	IS_USEPR + IS_STORE + IS_SPREL + IS_SHORT,
	/* X_SUB_ST_WATIQ       */	IS_USEPR + IS_STORE + IS_POPWT,
	/* X_SUB_ST_UATIQ       */	IS_USEPR + IS_STORE + IS_POPWT + IS_SHORT,
	/* X_SUB_ST_WAXIQ       */	IS_USEPR + IS_STORE,
	/* X_SUB_ST_UAXIQ       */	IS_USEPR + IS_STORE + IS_SHORT,

	// i-codes for 32-bit longs

	/* X_LDD_I              */	0,
	/* X_LDD_W              */	0,
	/* X_LDD_B              */	0,
	/* X_LDD_S_W            */	IS_SPREL,
	/* X_LDD_S_B            */	IS_SPREL
};

/* short version mapping for each of the i-code instructions */
/*
 * N.B. this table MUST be kept updated and in the same order as the i-code
 * enum list in defs.h
 */
enum ICODE short_icode[] = {
	// i-code to mark an instruction as retired */

	/* I_RETIRED            */	0,

	// i-code for internal compiler information

	/* I_INFO               */	0,

	// i-code that retires the primary register contents

	/* I_FENCE              */	0,

	// i-code that declares a byte sized primary register

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
	/* I_FUNCP_WR           */	0,
	/* I_CALLP              */	0,

	// i-codes for C functions and the C parameter stack

	/* I_ENTER              */	0,
	/* I_RETURN             */	0,
	/* I_MODSP              */	0,
	/* I_PUSHARG_WR         */	0,
	/* I_PUSH_WR            */	0,
	/* I_POP_WR             */	0,
	/* I_SPUSH_WR           */	0,
	/* I_SPUSH_UR           */	0,
	/* I_SPOP_WR            */	0,
	/* I_SPOP_UR            */	0,

	// i-codes for handling boolean tests and branching

	/* I_SWITCH_WR          */	0,
	/* I_SWITCH_UR          */	0,
	/* I_DEFAULT            */	0,
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
	/* X_CMP_UM             */	0,
	/* X_CMP_WS             */	0,
	/* X_CMP_US             */	0,
	/* X_CMP_WAX            */	0,
	/* X_CMP_UAX            */	0,

	/* X_CMP_UIQ            */	0,
	/* X_CMP_UMQ            */	0,
	/* X_CMP_USQ            */	0,
	/* X_CMP_UAXQ           */	0,

	/* I_NOT_WR             */	0,
	/* X_NOT_WP             */	0,
	/* X_NOT_WM             */	0,
	/* X_NOT_WS             */	0,
	/* X_NOT_WAR            */	0,
	/* X_NOT_WAX            */	0,
	/* X_NOT_UP             */	0,
	/* X_NOT_UM             */	0,
	/* X_NOT_US             */	0,
	/* X_NOT_UAR            */	0,
	/* X_NOT_UAX            */	0,
	/* X_NOT_UAY            */	0,

	/* I_TST_WR             */	0,
	/* X_TST_WP             */	0,
	/* X_TST_WM             */	0,
	/* X_TST_WS             */	0,
	/* X_TST_WAR            */	0,
	/* X_TST_WAX            */	0,
	/* X_TST_UP             */	0,
	/* X_TST_UM             */	0,
	/* X_TST_US             */	0,
	/* X_TST_UAR            */	0,
	/* X_TST_UAX            */	0,
	/* X_TST_UAY            */	0,

	/* X_NAND_WI            */	0,
	/* X_TAND_WI            */	0,

	/* X_NOT_CF             */	0,

	/* I_BOOLEAN            */	0,

	/* X_BOOLNOT_WR         */	0,
	/* X_BOOLNOT_WP         */	0,
	/* X_BOOLNOT_WM         */	0,
	/* X_BOOLNOT_WS         */	0,
	/* X_BOOLNOT_WAR        */	0,
	/* X_BOOLNOT_WAX        */	0,

	/* X_BOOLNOT_UP         */	0,
	/* X_BOOLNOT_UM         */	0,
	/* X_BOOLNOT_US         */	0,
	/* X_BOOLNOT_UAR        */	0,
	/* X_BOOLNOT_UAX        */	0,
	/* X_BOOLNOT_UAY        */	0,

	// i-codes for loading the primary register

	/* I_LD_WI              */	0,
	/* X_LD_UIQ             */	0,
	/* I_LEA_S              */	0,

	/* I_LD_WM              */	0,
	/* I_LD_BM              */	0,
	/* I_LD_UM              */	0,

	/* X_LD_WMQ             */	0,
	/* X_LD_BMQ             */	0,
	/* X_LD_UMQ             */	0,

	/* X_LDX_WMQ            */	0,
	/* X_LDX_BMQ            */	0,
	/* X_LDX_UMQ            */	0,

	/* X_LD2X_WM            */	0,
	/* X_LD2X_BM            */	0,
	/* X_LD2X_UM            */	0,

	/* X_LD2X_WMQ           */	0,
	/* X_LD2X_BMQ           */	0,
	/* X_LD2X_UMQ           */	0,

	/* X_LDY_WMQ            */	0,
	/* X_LDY_BMQ            */	0,
	/* X_LDY_UMQ            */	0,

	/* I_LD_WP              */	0,
	/* I_LD_BP              */	0,
	/* I_LD_UP              */	0,

	/* I_LD_UPQ             */	0,

	/* X_LD_WAR             */	0,
	/* X_LD_BAR             */	0,
	/* X_LD_UAR             */	0,

	/* X_LD_UARQ            */	0,

	/* X_LD_WAX             */	0,
	/* X_LD_BAX             */	0,
	/* X_LD_UAX             */	0,

	/* X_LD_UAXQ            */	0,

	/* X_LD_BAY             */	0,
	/* X_LD_UAY             */	0,

	/* X_LD_WS              */	0,
	/* X_LD_BS              */	0,
	/* X_LD_US              */	0,

	/* X_LD_WSQ             */	0,
	/* X_LD_BSQ             */	0,
	/* X_LD_USQ             */	0,

	/* X_LDX_WS             */	0,
	/* X_LDX_BS             */	0,
	/* X_LDX_US             */	0,

	/* X_LDX_WSQ            */	0,
	/* X_LDX_BSQ            */	0,
	/* X_LDX_USQ            */	0,

	/* X_LD2X_WS            */	0,
	/* X_LD2X_BS            */	0,
	/* X_LD2X_US            */	0,

	/* X_LD2X_WSQ           */	0,
	/* X_LD2X_BSQ           */	0,
	/* X_LD2X_USQ           */	0,

	/* X_LDY_WSQ            */	0,
	/* X_LDY_BSQ            */	0,
	/* X_LDY_USQ            */	0,

	/* X_LDP_WAR            */	0,
	/* X_LDP_BAR            */	0,
	/* X_LDP_UAR            */	0,

	/* X_LDP_WAX            */	0,
	/* X_LDP_BAX            */	0,
	/* X_LDP_UAX            */	0,

	/* X_LDP_BAY            */	0,
	/* X_LDP_UAY            */	0,

	// i-codes for pre- and post- increment and decrement

	/* X_INCLD_WM           */	0,
	/* X_LDINC_WM           */	0,
	/* X_DECLD_WM           */	0,
	/* X_LDDEC_WM           */	0,

	/* X_INCLD_BM           */	0,
	/* X_LDINC_BM           */	0,
	/* X_DECLD_BM           */	0,
	/* X_LDDEC_BM           */	0,

	/* X_INCLD_UM           */	0,
	/* X_LDINC_UM           */	0,
	/* X_DECLD_UM           */	0,
	/* X_LDDEC_UM           */	0,

	/* X_INC_WMQ            */	0,
	/* X_INC_UMQ            */	0,

	/* X_DEC_WMQ            */	0,
	/* X_DEC_UMQ            */	0,

	/* X_INCLD_WS           */	0,
	/* X_LDINC_WS           */	0,
	/* X_DECLD_WS           */	0,
	/* X_LDDEC_WS           */	0,

	/* X_INCLD_BS           */	0,
	/* X_LDINC_BS           */	0,
	/* X_DECLD_BS           */	0,
	/* X_LDDEC_BS           */	0,

	/* X_INCLD_US           */	0,
	/* X_LDINC_US           */	0,
	/* X_DECLD_US           */	0,
	/* X_LDDEC_US           */	0,

	/* X_INC_WSQ            */	0,
	/* X_INC_USQ            */	0,

	/* X_DEC_WSQ            */	0,
	/* X_DEC_USQ            */	0,

	/* X_INCLD_WAR          */	0,
	/* X_LDINC_WAR          */	0,
	/* X_DECLD_WAR          */	0,
	/* X_LDDEC_WAR          */	0,

	/* X_INCLD_BAR          */	0,
	/* X_LDINC_BAR          */	0,
	/* X_DECLD_BAR          */	0,
	/* X_LDDEC_BAR          */	0,

	/* X_INCLD_UAR          */	0,
	/* X_LDINC_UAR          */	0,
	/* X_DECLD_UAR          */	0,
	/* X_LDDEC_UAR          */	0,

	/* X_INC_WARQ           */	0,
	/* X_INC_UARQ           */	0,

	/* X_DEC_WARQ           */	0,
	/* X_DEC_UARQ           */	0,

	/* X_INCLD_WAX          */	0,
	/* X_LDINC_WAX          */	0,
	/* X_DECLD_WAX          */	0,
	/* X_LDDEC_WAX          */	0,

	/* X_INCLD_BAX          */	0,
	/* X_LDINC_BAX          */	0,
	/* X_DECLD_BAX          */	0,
	/* X_LDDEC_BAX          */	0,

	/* X_INCLD_UAX          */	0,
	/* X_LDINC_UAX          */	0,
	/* X_DECLD_UAX          */	0,
	/* X_LDDEC_UAX          */	0,

	/* X_INC_WAXQ           */	0,
	/* X_INC_UAXQ           */	0,

	/* X_DEC_WAXQ           */	0,
	/* X_DEC_UAXQ           */	0,

	/* X_INCLD_BAY          */	0,
	/* X_LDINC_BAY          */	0,
	/* X_DECLD_BAY          */	0,
	/* X_LDDEC_BAY          */	0,

	/* X_INCLD_UAY          */	0,
	/* X_LDINC_UAY          */	0,
	/* X_DECLD_UAY          */	0,
	/* X_LDDEC_UAY          */	0,

	/* X_INC_UAYQ           */	0,

	/* X_DEC_UAYQ           */	0,

	// i-codes for saving the primary register

	/* I_ST_WPT             */	0,
	/* I_ST_UPT             */	0,
	/* X_ST_WPTQ            */	0,
	/* X_ST_UPTQ            */	0,

	/* I_ST_WM              */	0,
	/* I_ST_UM              */	0,
	/* X_ST_WMQ             */	0,
	/* X_ST_UMQ             */	0,
	/* I_ST_WMIQ            */	0,
	/* I_ST_UMIQ            */	0,
	/* X_ST_WP              */	0,
	/* X_ST_UP              */	0,
	/* X_ST_WPQ             */	0,
	/* X_ST_UPQ             */	0,
	/* X_ST_WPI             */	0,
	/* X_ST_UPI             */	0,
	/* X_ST_WS              */	0,
	/* X_ST_US              */	0,
	/* X_ST_WSQ             */	0,
	/* X_ST_USQ             */	0,
	/* I_ST_WSIQ            */	0,
	/* I_ST_USIQ            */	0,

	/* X_INDEX_WR           */	0,
	/* X_INDEX_UR           */	0,

	/* X_ST_WAT             */	0,
	/* X_ST_UAT             */	0,
	/* X_ST_WATQ            */	0,
	/* X_ST_UATQ            */	0,
	/* X_ST_WATIQ           */	0,
	/* X_ST_UATIQ           */	0,

	/* X_ST_WAX             */	0,
	/* X_ST_UAX             */	0,
	/* X_ST_WAXQ            */	0,
	/* X_ST_UAXQ            */	0,
	/* X_ST_WAXIQ           */	0,
	/* X_ST_UAXIQ           */	0,

	// i-codes for extending the primary register

	/* I_EXT_BR             */	0,
	/* I_EXT_UR             */	0,

	// i-codes for 16-bit math with the primary register

	/* I_COM_WR             */	0,
	/* I_NEG_WR             */	0,

	/* I_ADD_WT             */	0,
	/* I_ADD_WI             */	0,

	/* X_ADD_WM             */	0,
	/* X_ADD_UM             */	0,
	/* X_ADD_WP             */	0,
	/* X_ADD_UP             */	0,
	/* X_ADD_WS             */	0,
	/* X_ADD_US             */	0,
	/* X_ADD_WAT            */	0,
	/* X_ADD_UAT            */	0,
	/* X_ADD_WAX            */	0,
	/* X_ADD_UAX            */	0,

	/* I_SUB_WT             */	0,
	/* I_SUB_WI             */	0,

	/* X_SUB_WM             */	0,
	/* X_SUB_UM             */	0,
	/* X_SUB_WP             */	0,
	/* X_SUB_UP             */	0,
	/* X_SUB_WS             */	0,
	/* X_SUB_US             */	0,
	/* X_SUB_WAT            */	0,
	/* X_SUB_UAT            */	0,
	/* X_SUB_WAX            */	0,
	/* X_SUB_UAX            */	0,

	/* X_ISUB_WT            */	0,
	/* X_ISUB_WI            */	0,

	/* X_ISUB_WM            */	0,
	/* X_ISUB_UM            */	0,
	/* X_ISUB_WP            */	0,
	/* X_ISUB_UP            */	0,
	/* X_ISUB_WS            */	0,
	/* X_ISUB_US            */	0,
	/* X_ISUB_WAT           */	0,
	/* X_ISUB_UAT           */	0,
	/* X_ISUB_WAX           */	0,
	/* X_ISUB_UAX           */	0,

	/* I_AND_WT             */	0,
	/* X_AND_WI             */	0,

	/* X_AND_WM             */	0,
	/* X_AND_UM             */	0,
	/* X_AND_WP             */	0,
	/* X_AND_UP             */	0,
	/* X_AND_WS             */	0,
	/* X_AND_US             */	0,
	/* X_AND_WAT            */	0,
	/* X_AND_UAT            */	0,
	/* X_AND_WAX            */	0,
	/* X_AND_UAX            */	0,

	/* I_EOR_WT             */	0,
	/* X_EOR_WI             */	0,

	/* X_EOR_WM             */	0,
	/* X_EOR_UM             */	0,
	/* X_EOR_WP             */	0,
	/* X_EOR_UP             */	0,
	/* X_EOR_WS             */	0,
	/* X_EOR_US             */	0,
	/* X_EOR_WAT            */	0,
	/* X_EOR_UAT            */	0,
	/* X_EOR_WAX            */	0,
	/* X_EOR_UAX            */	0,

	/* I_OR_WT              */	0,
	/* X_OR_WI              */	0,

	/* X_OR_WM              */	0,
	/* X_OR_UM              */	0,
	/* X_OR_WP              */	0,
	/* X_OR_UP              */	0,
	/* X_OR_WS              */	0,
	/* X_OR_US              */	0,
	/* X_OR_WAT             */	0,
	/* X_OR_UAT             */	0,
	/* X_OR_WAX             */	0,
	/* X_OR_UAX             */	0,

	/* I_ASL_WR             */	0,
	/* I_ASL_WT             */	0,
	/* I_ASL_WI             */	0,

	/* I_ASR_WT             */	0,
	/* I_ASR_WI             */	0,

	/* I_LSR_WT             */	0,
	/* I_LSR_WI             */	0,

	/* I_MUL_WT             */	0,
	/* I_MUL_WI             */	0,

	/* I_SDIV_WT            */	0,
	/* I_SDIV_WI            */	0,

	/* I_UDIV_WT            */	0,
	/* I_UDIV_WI            */	0,
	/* I_UDIV_UI            */	0,

	/* I_SMOD_WT            */	0,
	/* I_SMOD_WI            */	0,

	/* I_UMOD_WT            */	0,
	/* I_UMOD_WI            */	0,
	/* I_UMOD_UI            */	0,

	/* I_DOUBLE_WT          */	0,

	// i-codes for 8-bit math with lo-byte of the primary register

	/* X_ADD_UIQ            */	0,
	/* X_ADD_UMQ            */	0,
	/* X_ADD_UPQ            */	0,
	/* X_ADD_USQ            */	0,
	/* X_ADD_UATQ           */	0,
	/* X_ADD_UAXQ           */	0,

	/* X_SUB_UIQ            */	0,
	/* X_SUB_UMQ            */	0,
	/* X_SUB_UPQ            */	0,
	/* X_SUB_USQ            */	0,
	/* X_SUB_UATQ           */	0,
	/* X_SUB_UAXQ           */	0,

	/* X_ISUB_UIQ           */	0,
	/* X_ISUB_UMQ           */	0,
	/* X_ISUB_UPQ           */	0,
	/* X_ISUB_USQ           */	0,
	/* X_ISUB_UATQ          */	0,
	/* X_ISUB_UAXQ          */	0,

	/* X_AND_UIQ            */	0,
	/* X_AND_UMQ            */	0,
	/* X_AND_UPQ            */	0,
	/* X_AND_USQ            */	0,
	/* X_AND_UATQ           */	0,
	/* X_AND_UAXQ           */	0,

	/* X_EOR_UIQ            */	0,
	/* X_EOR_UMQ            */	0,
	/* X_EOR_UPQ            */	0,
	/* X_EOR_USQ            */	0,
	/* X_EOR_UATQ           */	0,
	/* X_EOR_UAXQ           */	0,

	/* X_OR_UIQ             */	0,
	/* X_OR_UMQ             */	0,
	/* X_OR_UPQ             */	0,
	/* X_OR_USQ             */	0,
	/* X_OR_UATQ            */	0,
	/* X_OR_UAXQ            */	0,

	/* I_ASL_UIQ            */	0,

	/* I_LSR_UIQ            */	0,

	/* I_MUL_UIQ            */	0,

	// i-codes for modifying a variable with "+=", "-=", "&=", "^=", "|="

	/* X_ADD_ST_WMQ         */	0,
	/* X_ADD_ST_UMQ         */	0,
	/* X_ADD_ST_WPQ         */	0,
	/* X_ADD_ST_UPQ         */	0,
	/* X_ADD_ST_WSQ         */	0,
	/* X_ADD_ST_USQ         */	0,
	/* X_ADD_ST_WATQ        */	0,
	/* X_ADD_ST_UATQ        */	0,
	/* X_ADD_ST_WAXQ        */	0,
	/* X_ADD_ST_UAXQ        */	0,

	/* X_ISUB_ST_WMQ        */	0,
	/* X_ISUB_ST_UMQ        */	0,
	/* X_ISUB_ST_WPQ        */	0,
	/* X_ISUB_ST_UPQ        */	0,
	/* X_ISUB_ST_WSQ        */	0,
	/* X_ISUB_ST_USQ        */	0,
	/* X_ISUB_ST_WATQ       */	0,
	/* X_ISUB_ST_UATQ       */	0,
	/* X_ISUB_ST_WAXQ       */	0,
	/* X_ISUB_ST_UAXQ       */	0,

	/* X_AND_ST_WMQ         */	0,
	/* X_AND_ST_UMQ         */	0,
	/* X_AND_ST_WPQ         */	0,
	/* X_AND_ST_UPQ         */	0,
	/* X_AND_ST_WSQ         */	0,
	/* X_AND_ST_USQ         */	0,
	/* X_AND_ST_WATQ        */	0,
	/* X_AND_ST_UATQ        */	0,
	/* X_AND_ST_WAXQ        */	0,
	/* X_AND_ST_UAXQ        */	0,

	/* X_EOR_ST_WMQ         */	0,
	/* X_EOR_ST_UMQ         */	0,
	/* X_EOR_ST_WPQ         */	0,
	/* X_EOR_ST_UPQ         */	0,
	/* X_EOR_ST_WSQ         */	0,
	/* X_EOR_ST_USQ         */	0,
	/* X_EOR_ST_WATQ        */	0,
	/* X_EOR_ST_UATQ        */	0,
	/* X_EOR_ST_WAXQ        */	0,
	/* X_EOR_ST_UAXQ        */	0,

	/* X_OR_ST_WMQ          */	0,
	/* X_OR_ST_UMQ          */	0,
	/* X_OR_ST_WPQ          */	0,
	/* X_OR_ST_UPQ          */	0,
	/* X_OR_ST_WSQ          */	0,
	/* X_OR_ST_USQ          */	0,
	/* X_OR_ST_WATQ         */	0,
	/* X_OR_ST_UATQ         */	0,
	/* X_OR_ST_WAXQ         */	0,
	/* X_OR_ST_UAXQ         */	0,

	/* X_ADD_ST_WMIQ        */	0,
	/* X_ADD_ST_UMIQ        */	0,
	/* X_ADD_ST_WPIQ        */	0,
	/* X_ADD_ST_UPIQ        */	0,
	/* X_ADD_ST_WSIQ        */	0,
	/* X_ADD_ST_USIQ        */	0,
	/* X_ADD_ST_WATIQ       */	0,
	/* X_ADD_ST_UATIQ       */	0,
	/* X_ADD_ST_WAXIQ       */	0,
	/* X_ADD_ST_UAXIQ       */	0,

	/* X_SUB_ST_WMIQ        */	0,
	/* X_SUB_ST_UMIQ        */	0,
	/* X_SUB_ST_WPIQ        */	0,
	/* X_SUB_ST_UPIQ        */	0,
	/* X_SUB_ST_WSIQ        */	0,
	/* X_SUB_ST_USIQ        */	0,
	/* X_SUB_ST_WATIQ       */	0,
	/* X_SUB_ST_UATIQ       */	0,
	/* X_SUB_ST_WAXIQ       */	0,
	/* X_SUB_ST_UAXIQ       */	0,

	// i-codes for 32-bit longs

	/* X_LDD_I              */	0,
	/* X_LDD_W              */	0,
	/* X_LDD_B              */	0,
	/* X_LDD_S_W            */	0,
	/* X_LDD_S_B            */	0,
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

/* instruction queue */
INS ins_queue[Q_SIZE];
INS *q_ins = ins_queue;
int q_rd;
int q_wr;
int q_nb;

/* externs */
extern int arg_stack_flag;

bool cmp_operands (INS *p1, INS *p2)
{
#ifdef DEBUG_OPTIMIZER
	printf("cmp"); dump_ins(p1); dump_ins(p2);
#endif
	if (p1->ins_type != p2->ins_type)
		return (false);

	if (p1->ins_type == T_SYMBOL) {
		if (strcmp(((SYMBOL *)p1->ins_data)->name, ((SYMBOL *)p2->ins_data)->name) != 0)
			return (false);
	}
	else {
		if (p1->ins_data != p2->ins_data)
			return (false);
	}
	return (true);
}

inline bool is_sprel (INS *i)
{
	return (icode_flags[i->ins_code] & IS_SPREL);
}

inline bool is_ubyte (INS *i)
{
	return (icode_flags[i->ins_code] & IS_UBYTE);
}

inline bool is_sbyte (INS *i)
{
	return (icode_flags[i->ins_code] & IS_SBYTE);
}

inline bool is_usepr (INS *i)
{
	return (icode_flags[i->ins_code] & IS_USEPR);
}

bool is_small_array (SYMBOL *sym)
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
	INS *p[Q_SIZE];
	int p_nb;
	int i, j;
	int remove = 0;

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

	/* can't optimize internal compiler information */
	if (ins->ins_code == I_INFO)
		return;

#ifdef DEBUG_OPTIMIZER
	printf("\npush "); dump_ins(ins);
#endif

	/* optimization level 1 - simple peephole optimizer,
	 * replace known instruction patterns by highly
	 * optimized asm code
	 */
	if (optimize >= 1) {
lv1_loop:
#ifdef DEBUG_OPTIMIZER
		printf("\nlv1_loop:\n");
#endif

		/* remove instructions from queue but preserve comments */
		if (remove) {
			q_nb -= remove;
			i = q_wr;
			while (remove) {
				if (q_ins[i].ins_code != I_INFO)
					--remove;
				if ((--i) < 0)
					i += Q_SIZE;
			}
			j = i;
			do {
				if ((++j) >= Q_SIZE)
					j -= Q_SIZE;
				if (q_ins[j].ins_code == I_INFO) {
					if ((++i) >= Q_SIZE)
						i -= Q_SIZE;
					q_ins[i] = q_ins[j];
				}
			} while (j != q_wr);
			q_wr = i;
		}

		/* precalculate pointers to the last 6 instructions */
		p_nb = 0;
		i = q_nb;
		j = q_wr;
		while (i != 0 && p_nb < 6) {
			if (q_ins[j].ins_code != I_INFO) {
#ifdef DEBUG_OPTIMIZER
				printf("%d ", p_nb);
				dump_ins(&q_ins[j]);
#endif
				p[p_nb++] = &q_ins[j];
			}
			--i;
			if ((--j) < 0)
				j += Q_SIZE;
		}
		remove = 0;

		/* LEVEL 1 - FUN STUFF STARTS HERE */

		/* ********************************************************* */
		/* ********************************************************* */

		/* first check for I_FENCE, and remove it ASAP */
		if (p_nb >= 1 && p[0]->ins_code == I_FENCE) {
			/* remove I_FENCE after it has been checked */
			remove = 1;

			/*
			 *  __ld.wi		i	-->	__st.{w/u}miq	i, symbol
			 *  __st.{w/u}m		symbol
			 *  __fence
			 *
			 *  __ld.wi		i	-->	__st.{w/u}siq	i, n
			 *  __st.{w/u}s		n
			 *  __fence
			 *
			 *  __ld.wi		i	-->	__st.{w/u}atiq	i, array
			 *  __st.{w/u}at	array
			 *  __fence
			 *
			 *  __ld.wi		i	-->	__st.{w/u}axiq	i, array
			 *  __st.{w/u}ax	array
			 *  __fence
			 */
			if
			((p_nb >= 3) &&
			 (p[1]->ins_code == I_ST_WM ||
			  p[1]->ins_code == I_ST_UM ||
			  p[1]->ins_code == X_ST_WS ||
			  p[1]->ins_code == X_ST_US ||
			  p[1]->ins_code == X_ST_WAT ||
			  p[1]->ins_code == X_ST_UAT ||
			  p[1]->ins_code == X_ST_WAX ||
			  p[1]->ins_code == X_ST_UAX) &&
			 (p[2]->ins_code == I_LD_WI) &&
			 (p[2]->ins_type == T_VALUE)
			) {
				/* replace code */
				intptr_t data = p[2]->ins_data;
				*p[2] = *p[1];
				switch (p[1]->ins_code) {
				case I_ST_WM: p[2]->ins_code = I_ST_WMIQ; break;
				case I_ST_UM: p[2]->ins_code = I_ST_UMIQ; break;
				case X_ST_WS: p[2]->ins_code = I_ST_WSIQ; break;
				case X_ST_US: p[2]->ins_code = I_ST_USIQ; break;
				case X_ST_WAT: p[2]->ins_code = X_ST_WATIQ; break;
				case X_ST_UAT: p[2]->ins_code = X_ST_UATIQ; break;
				case X_ST_WAX: p[2]->ins_code = X_ST_WAXIQ; break;
				case X_ST_UAX: p[2]->ins_code = X_ST_UAXIQ; break;
				default: break;
				}
				p[2]->imm_type = T_VALUE;
				p[2]->imm_data = data;
				remove = 2;
			}

			/*
			 *  __add.wi / __sub.wi		-->
			 *  __fence
			 */
			else if
			((p_nb >= 2) &&
			 (p[1]->ins_code == I_ADD_WI ||
			  p[1]->ins_code == I_SUB_WI)
			) {
				remove = 2;
			}

			/*
			 *  __add.wm		symbol	-->	__add_st.wmq	symbol
			 *  __st.wm		symbol
			 *  __fence
			 *
			 *  __add.um		symbol	-->	__add_st.umq	symbol
			 *  __st.um		symbol
			 *  __fence
			 *
			 *  __isub.wm		symbol	-->	__isub_st.wmq	symbol
			 *  __st.wm		symbol
			 *  __fence
			 *
			 *  __isub.um		symbol	-->	__isub_st.umq	symbol
			 *  __st.um		symbol
			 *  __fence
			 *
			 *  etc, etc
			 *
			 *  this optimizes the store for a "+=", "-=", "&=", "^=", "|=".
			 */
			else if
			((p_nb >= 3) &&
			 (p[1]->ins_code == I_ST_WM ||
			  p[1]->ins_code == I_ST_UM) &&
			 (p[2]->ins_code == X_ADD_WM ||
			  p[2]->ins_code == X_ADD_UM ||
			  p[2]->ins_code == X_ISUB_WM ||
			  p[2]->ins_code == X_ISUB_UM ||
			  p[2]->ins_code == X_AND_WM ||
			  p[2]->ins_code == X_AND_UM ||
			  p[2]->ins_code == X_EOR_WM ||
			  p[2]->ins_code == X_EOR_UM ||
			  p[2]->ins_code == X_OR_WM ||
			  p[2]->ins_code == X_OR_UM) &&
			 (cmp_operands(p[1], p[2]))
			) {
				/* replace code */
				remove = 2;
				if (p[1]->ins_code == I_ST_WM) {
					switch (p[2]->ins_code) {
					case  X_ADD_WM: p[2]->ins_code =  X_ADD_ST_WMQ; break;
					case X_ISUB_WM: p[2]->ins_code = X_ISUB_ST_WMQ; break;
					case  X_AND_WM: p[2]->ins_code =  X_AND_ST_WMQ; break;
					case  X_EOR_WM: p[2]->ins_code =  X_EOR_ST_WMQ; break;
					case   X_OR_WM: p[2]->ins_code =   X_OR_ST_WMQ; break;
					default: remove = 1; break; /* this should never happen! */
					}
				} else {
					switch (p[2]->ins_code) {
					case  X_ADD_UM: p[2]->ins_code =  X_ADD_ST_UMQ; break;
					case X_ISUB_UM: p[2]->ins_code = X_ISUB_ST_UMQ; break;
					case  X_AND_UM: p[2]->ins_code =  X_AND_ST_UMQ; break;
					case  X_EOR_UM: p[2]->ins_code =  X_EOR_ST_UMQ; break;
					case   X_OR_UM: p[2]->ins_code =   X_OR_ST_UMQ; break;
					default: remove = 1; break; /* this should never happen! */
					}
				}
			}

			/*
			 *  __add.ws		n	-->	__add_st.wsq	n
			 *  __st.ws		n
			 *  __fence
			 *
			 *  __add.us		n	-->	__add_st.usq	n
			 *  __st.us		n
			 *  __fence
			 *
			 *  __isub.ws		n	-->	__isub_st.wsq	n
			 *  __st.ws		n
			 *  __fence
			 *
			 *  __isub.us		n	-->	__isub_st.usq	n
			 *  __st.us		n
			 *  __fence
			 *
			 *  etc, etc
			 *
			 *  this optimizes the store for a "+=", "-=", "&=", "^=", "|=".
			 */
			else if
			((p_nb >= 3) &&
			 (p[1]->ins_code == X_ST_WS ||
			  p[1]->ins_code == X_ST_US) &&
			 (p[2]->ins_code == X_ADD_WS ||
			  p[2]->ins_code == X_ADD_US ||
			  p[2]->ins_code == X_ISUB_WS ||
			  p[2]->ins_code == X_ISUB_US ||
			  p[2]->ins_code == X_AND_WS ||
			  p[2]->ins_code == X_AND_US ||
			  p[2]->ins_code == X_EOR_WS ||
			  p[2]->ins_code == X_EOR_US ||
			  p[2]->ins_code == X_OR_WS ||
			  p[2]->ins_code == X_OR_US) &&
			 (cmp_operands(p[1], p[2]))
			) {
				/* replace code */
				remove = 2;
				if (p[1]->ins_code == X_ST_WS) {
					switch (p[2]->ins_code) {
					case  X_ADD_WS: p[2]->ins_code =  X_ADD_ST_WSQ; break;
					case X_ISUB_WS: p[2]->ins_code = X_ISUB_ST_WSQ; break;
					case  X_AND_WS: p[2]->ins_code =  X_AND_ST_WSQ; break;
					case  X_EOR_WS: p[2]->ins_code =  X_EOR_ST_WSQ; break;
					case   X_OR_WS: p[2]->ins_code =   X_OR_ST_WSQ; break;
					default: remove = 1; break; /* this should never happen! */
					}
				} else {
					switch (p[2]->ins_code) {
					case  X_ADD_US: p[2]->ins_code =  X_ADD_ST_USQ; break;
					case X_ISUB_US: p[2]->ins_code = X_ISUB_ST_USQ; break;
					case  X_AND_US: p[2]->ins_code =  X_AND_ST_USQ; break;
					case  X_EOR_US: p[2]->ins_code =  X_EOR_ST_USQ; break;
					case   X_OR_US: p[2]->ins_code =   X_OR_ST_USQ; break;
					default: remove = 1; break; /* this should never happen! */
					}
				}
			}

			/*
			 *  __add.wat		array	-->	__add_st.watq	array
			 *  __st.wax		array
			 *  __fence
			 *
			 *  __add.uat		array	-->	__add_st.uatq	array
			 *  __st.uax		array
			 *  __fence
			 *
			 *  __isub.wat		array	-->	__isub_st.watq	array
			 *  __st.wax		array
			 *  __fence
			 *
			 *  __isub.uat		array	-->	__isub_st.uatq	array
			 *  __st.uax		array
			 *  __fence
			 *
			 *  etc, etc
			 *
			 *  this optimizes the store for a "+=", "-=", "&=", "^=", "|=".
			 */
			else if
			((p_nb >= 3) &&
			 (p[1]->ins_code == X_ST_WAX ||
			  p[1]->ins_code == X_ST_UAX) &&
			 (p[2]->ins_code == X_ADD_WAT ||
			  p[2]->ins_code == X_ADD_UAT ||
			  p[2]->ins_code == X_ISUB_WAT ||
			  p[2]->ins_code == X_ISUB_UAT ||
			  p[2]->ins_code == X_AND_WAT ||
			  p[2]->ins_code == X_AND_UAT ||
			  p[2]->ins_code == X_EOR_WAT ||
			  p[2]->ins_code == X_EOR_UAT ||
			  p[2]->ins_code == X_OR_WAT ||
			  p[2]->ins_code == X_OR_UAT) &&
			 (cmp_operands(p[1], p[2]))
			) {
				/* replace code */
				remove = 2;
				if (p[1]->ins_code == X_ST_WAX) {
					switch (p[2]->ins_code) {
					case  X_ADD_WAT: p[2]->ins_code =  X_ADD_ST_WATQ; break;
					case X_ISUB_WAT: p[2]->ins_code = X_ISUB_ST_WATQ; break;
					case  X_AND_WAT: p[2]->ins_code =  X_AND_ST_WATQ; break;
					case  X_EOR_WAT: p[2]->ins_code =  X_EOR_ST_WATQ; break;
					case   X_OR_WAT: p[2]->ins_code =   X_OR_ST_WATQ; break;
					default: remove = 1; break; /* this should never happen! */
					}
				} else {
					switch (p[2]->ins_code) {
					case  X_ADD_UAT: p[2]->ins_code =  X_ADD_ST_UATQ; break;
					case X_ISUB_UAT: p[2]->ins_code = X_ISUB_ST_UATQ; break;
					case  X_AND_UAT: p[2]->ins_code =  X_AND_ST_UATQ; break;
					case  X_EOR_UAT: p[2]->ins_code =  X_EOR_ST_UATQ; break;
					case   X_OR_UAT: p[2]->ins_code =   X_OR_ST_UATQ; break;
					default: remove = 1; break; /* this should never happen! */
					}
				}
			}

			/*
			 *  __add.wax		array	-->	__add_st.waxq	array
			 *  __st.wax		array
			 *  __fence
			 *
			 *  __add.uax		array	-->	__add_st.uaxq	array
			 *  __st.uax		array
			 *  __fence
			 *
			 *  __isub.wax		array	-->	__isub_st.waxq	array
			 *  __st.wax		array
			 *  __fence
			 *
			 *  __isub.uax		array	-->	__isub_st.uaxq	array
			 *  __st.uax		array
			 *  __fence
			 *
			 *  etc, etc
			 *
			 *  this optimizes the store for a "+=", "-=", "&=", "^=", "|=".
			 */
			else if
			((p_nb >= 3) &&
			 (p[1]->ins_code == X_ST_WAX ||
			  p[1]->ins_code == X_ST_UAX) &&
			 (p[2]->ins_code == X_ADD_WAX ||
			  p[2]->ins_code == X_ADD_UAX ||
			  p[2]->ins_code == X_ISUB_WAX ||
			  p[2]->ins_code == X_ISUB_UAX ||
			  p[2]->ins_code == X_AND_WAX ||
			  p[2]->ins_code == X_AND_UAX ||
			  p[2]->ins_code == X_EOR_WAX ||
			  p[2]->ins_code == X_EOR_UAX ||
			  p[2]->ins_code == X_OR_WAX ||
			  p[2]->ins_code == X_OR_UAX) &&
			 (cmp_operands(p[1], p[2]))
			) {
				/* replace code */
				remove = 2;
				if (p[1]->ins_code == X_ST_WAX) {
					switch (p[2]->ins_code) {
					case  X_ADD_WAX: p[2]->ins_code =  X_ADD_ST_WAXQ; break;
					case X_ISUB_WAX: p[2]->ins_code = X_ISUB_ST_WAXQ; break;
					case  X_AND_WAX: p[2]->ins_code =  X_AND_ST_WAXQ; break;
					case  X_EOR_WAX: p[2]->ins_code =  X_EOR_ST_WAXQ; break;
					case   X_OR_WAX: p[2]->ins_code =   X_OR_ST_WAXQ; break;
					default: remove = 1; break; /* this should never happen! */
					}
				} else {
					switch (p[2]->ins_code) {
					case  X_ADD_UAX: p[2]->ins_code =  X_ADD_ST_UAXQ; break;
					case X_ISUB_UAX: p[2]->ins_code = X_ISUB_ST_UAXQ; break;
					case  X_AND_UAX: p[2]->ins_code =  X_AND_ST_UAXQ; break;
					case  X_EOR_UAX: p[2]->ins_code =  X_EOR_ST_UAXQ; break;
					case   X_OR_UAX: p[2]->ins_code =   X_OR_ST_UAXQ; break;
					default: remove = 1; break; /* this should never happen! */
					}
				}
			}

			/*
			 *  __st.{w/u}m		symbol	-->	__st.{w/u}mq	symbol
			 *  __fence
			 *
			 *  __st.{w/u}p		symbol	-->	__st.{w/u}pq	symbol
			 *  __fence
			 *
			 *  __st.{w/u}s		n	-->	__st.{w/u}sq	n
			 *  __fence
			 *
			 *  __st.{w/u}at	array	-->	__st.{w/u}atq	array
			 *  __fence
			 *
			 *  __st.{w/u}ax	array	-->	__st.{w/u}axq	array
			 *  __fence
			 */
			else if
			((p_nb >= 2) &&
			 (icode_flags[p[1]->ins_code] & IS_STORE)
			) {
				/* replace code */
				switch (p[1]->ins_code) {
				case I_ST_WPT: p[1]->ins_code = X_ST_WPTQ; break;
				case I_ST_UPT: p[1]->ins_code = X_ST_UPTQ; break;
				case  I_ST_WM: p[1]->ins_code =  X_ST_WMQ; break;
				case  I_ST_UM: p[1]->ins_code =  X_ST_UMQ; break;
				case  X_ST_WP: p[1]->ins_code =  X_ST_WPQ; break;
				case  X_ST_UP: p[1]->ins_code =  X_ST_UPQ; break;
				case  X_ST_WS: p[1]->ins_code =  X_ST_WSQ; break;
				case  X_ST_US: p[1]->ins_code =  X_ST_USQ; break;
				case X_ST_WAT: p[1]->ins_code = X_ST_WATQ; break;
				case X_ST_UAT: p[1]->ins_code = X_ST_UATQ; break;
				case X_ST_WAX: p[1]->ins_code = X_ST_WAXQ; break;
				case X_ST_UAX: p[1]->ins_code = X_ST_UAXQ; break;
				default: break;
				}
				remove = 1;
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
			((p_nb >= 2) &&
			 (p[1]->ins_code == X_INCLD_WM ||
			  p[1]->ins_code == X_LDINC_WM ||
			  p[1]->ins_code == X_DECLD_WM ||
			  p[1]->ins_code == X_LDDEC_WM ||
			  p[1]->ins_code == X_INCLD_BM ||
			  p[1]->ins_code == X_LDINC_BM ||
			  p[1]->ins_code == X_DECLD_BM ||
			  p[1]->ins_code == X_LDDEC_BM ||
			  p[1]->ins_code == X_INCLD_UM ||
			  p[1]->ins_code == X_LDINC_UM ||
			  p[1]->ins_code == X_DECLD_UM ||
			  p[1]->ins_code == X_LDDEC_UM ||

			  p[1]->ins_code == X_INCLD_WS ||
			  p[1]->ins_code == X_LDINC_WS ||
			  p[1]->ins_code == X_DECLD_WS ||
			  p[1]->ins_code == X_LDDEC_WS ||
			  p[1]->ins_code == X_INCLD_BS ||
			  p[1]->ins_code == X_LDINC_BS ||
			  p[1]->ins_code == X_DECLD_BS ||
			  p[1]->ins_code == X_LDDEC_BS ||
			  p[1]->ins_code == X_INCLD_US ||
			  p[1]->ins_code == X_LDINC_US ||
			  p[1]->ins_code == X_DECLD_US ||
			  p[1]->ins_code == X_LDDEC_US ||

			  p[1]->ins_code == X_INCLD_WAR ||
			  p[1]->ins_code == X_LDINC_WAR ||
			  p[1]->ins_code == X_DECLD_WAR ||
			  p[1]->ins_code == X_LDDEC_WAR ||
			  p[1]->ins_code == X_INCLD_BAR ||
			  p[1]->ins_code == X_LDINC_BAR ||
			  p[1]->ins_code == X_DECLD_BAR ||
			  p[1]->ins_code == X_LDDEC_BAR ||
			  p[1]->ins_code == X_INCLD_UAR ||
			  p[1]->ins_code == X_LDINC_UAR ||
			  p[1]->ins_code == X_DECLD_UAR ||
			  p[1]->ins_code == X_LDDEC_UAR ||

			  p[1]->ins_code == X_INCLD_WAX ||
			  p[1]->ins_code == X_LDINC_WAX ||
			  p[1]->ins_code == X_DECLD_WAX ||
			  p[1]->ins_code == X_LDDEC_WAX ||
			  p[1]->ins_code == X_INCLD_BAX ||
			  p[1]->ins_code == X_LDINC_BAX ||
			  p[1]->ins_code == X_DECLD_BAX ||
			  p[1]->ins_code == X_LDDEC_BAX ||
			  p[1]->ins_code == X_INCLD_UAX ||
			  p[1]->ins_code == X_LDINC_UAX ||
			  p[1]->ins_code == X_DECLD_UAX ||
			  p[1]->ins_code == X_LDDEC_UAX ||

			  p[1]->ins_code == X_INCLD_BAY ||
			  p[1]->ins_code == X_LDINC_BAY ||
			  p[1]->ins_code == X_DECLD_BAY ||
			  p[1]->ins_code == X_LDDEC_BAY ||
			  p[1]->ins_code == X_INCLD_UAY ||
			  p[1]->ins_code == X_LDINC_UAY ||
			  p[1]->ins_code == X_DECLD_UAY ||
			  p[1]->ins_code == X_LDDEC_UAY)
			) {
				/* replace code */
				switch (p[1]->ins_code) {
				case X_INCLD_WM:
				case X_LDINC_WM: p[1]->ins_code = X_INC_WMQ; break;
				case X_DECLD_WM:
				case X_LDDEC_WM: p[1]->ins_code = X_DEC_WMQ; break;
				case X_INCLD_BM:
				case X_LDINC_BM:
				case X_INCLD_UM:
				case X_LDINC_UM: p[1]->ins_code = X_INC_UMQ; break;
				case X_DECLD_BM:
				case X_LDDEC_BM:
				case X_DECLD_UM:
				case X_LDDEC_UM: p[1]->ins_code = X_DEC_UMQ; break;

				case X_INCLD_WS:
				case X_LDINC_WS: p[1]->ins_code = X_INC_WSQ; break;
				case X_DECLD_WS:
				case X_LDDEC_WS: p[1]->ins_code = X_DEC_WSQ; break;
				case X_INCLD_BS:
				case X_LDINC_BS:
				case X_INCLD_US:
				case X_LDINC_US: p[1]->ins_code = X_INC_USQ; break;
				case X_DECLD_BS:
				case X_LDDEC_BS:
				case X_DECLD_US:
				case X_LDDEC_US: p[1]->ins_code = X_DEC_USQ; break;

				case X_INCLD_WAR:
				case X_LDINC_WAR: p[1]->ins_code = X_INC_WARQ; break;
				case X_DECLD_WAR:
				case X_LDDEC_WAR: p[1]->ins_code = X_DEC_WARQ; break;
				case X_INCLD_BAR:
				case X_LDINC_BAR:
				case X_INCLD_UAR:
				case X_LDINC_UAR: p[1]->ins_code = X_INC_UARQ; break;
				case X_DECLD_BAR:
				case X_LDDEC_BAR:
				case X_DECLD_UAR:
				case X_LDDEC_UAR: p[1]->ins_code = X_DEC_UARQ; break;

				case X_INCLD_WAX:
				case X_LDINC_WAX: p[1]->ins_code = X_INC_WAXQ; break;
				case X_DECLD_WAX:
				case X_LDDEC_WAX: p[1]->ins_code = X_DEC_WAXQ; break;
				case X_INCLD_BAX:
				case X_INCLD_UAX:
				case X_LDINC_BAX:
				case X_LDINC_UAX: p[1]->ins_code = X_INC_UAXQ; break;
				case X_DECLD_BAX:
				case X_DECLD_UAX:
				case X_LDDEC_BAX:
				case X_LDDEC_UAX: p[1]->ins_code = X_DEC_UAXQ; break;

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
				remove = 1;
			}

			/* remove instructions from queue and begin again */
			if (remove)
				goto lv1_loop;
		}

		/* ********************************************************* */
		/* ********************************************************* */

		/* then check for I_SHORT, and remove it ASAP */
		if (p_nb >= 1 && p[0]->ins_code == I_SHORT) {
			/* remove I_SHORT after it has been checked */
			remove = 1;

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
			((p_nb >= 2) &&
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
				case I_LD_WM: p[1]->ins_code = X_LD_WMQ; break;
				case I_LD_BM: p[1]->ins_code = X_LD_BMQ; break;
				case I_LD_UM: p[1]->ins_code = X_LD_UMQ; break;
				case X_LD_WS: p[1]->ins_code = X_LD_WSQ; break;
				case X_LD_BS: p[1]->ins_code = X_LD_BSQ; break;
				case X_LD_US: p[1]->ins_code = X_LD_USQ; break;
				default: abort();
				}
				remove = 1;
			}

			/* remove instructions from queue and begin again */
			if (remove)
				goto lv1_loop;
		}

		/* ********************************************************* */
		/* then evaluate constant expressions */
		/* ********************************************************* */

		if (p_nb >= 3) {
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
				case I_AND_WT: p[2]->ins_code = X_AND_WI; break;
				case I_EOR_WT: p[2]->ins_code = X_EOR_WI; break;
				case I_OR_WT:  p[2]->ins_code = X_OR_WI; break;
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
				remove = 2;

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
						remove = 3;
						break;
					}
				}
			}

			/* remove instructions from queue and begin again */
			if (remove)
				goto lv1_loop;
		}

		if (p_nb >= 2) {
			/*
			 *  __add.wi		i	-->	__add.wi	(i + j)
			 *  __add.wi		j
			 *
			 *  __add.wi		i	-->	__add.wi	(i - j)
			 *  __sub.wi		j
			 *
			 *  __sub.wi		i	-->	__add.wi	(-i + j)
			 *  __add.wi		j
			 *
			 *  __sub.wi		i	-->	__add.wi	(-i - j)
			 *  __sub.wi		j
			 */
			if
			((p[0]->ins_code == I_ADD_WI ||
			  p[0]->ins_code == I_SUB_WI) &&
			 (p[0]->ins_type == T_VALUE) &&
			 (p[1]->ins_code == I_ADD_WI ||
			  p[1]->ins_code == I_SUB_WI) &&
			 (p[1]->ins_type == T_VALUE)
			) {
				/* replace code */
				if (p[1]->ins_code == I_ADD_WI) {
					if (p[0]->ins_code == I_ADD_WI)
						p[1]->ins_data = p[1]->ins_data + p[0]->ins_data;
					else
						p[1]->ins_data = p[1]->ins_data - p[0]->ins_data;
				} else {
					if (p[0]->ins_code == I_ADD_WI)
						p[1]->ins_data = (- p[1]->ins_data) + p[0]->ins_data;
					else
						p[1]->ins_data = (- p[1]->ins_data) - p[0]->ins_data;
				}

				if (p[1]->ins_data == 0)
					remove = 2;
				else {
					if (p[1]->ins_data >= 0)
						p[1]->ins_code = I_ADD_WI;
					else {
						p[1]->ins_code = I_SUB_WI;
						p[1]->ins_data = - p[1]->ins_data;
					}
					remove = 1;
				}
			}

			/*
			 *  __ld.wi		i	-->	__ld.wi		(~i)
			 *  __com.wr
			 */
			else if
			((p[0]->ins_code == I_COM_WR) &&
			 (p[1]->ins_code == I_LD_WI) &&
			 (p[1]->ins_type == T_VALUE)
			) {
				/* replace code */
				p[1]->ins_data = ~p[1]->ins_data;
				remove = 1;
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
				remove = 1;
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
				remove = 1;
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
				remove = 1;
			}

			/*
			 *  __ld.wi		i	-->	__ld.wi		(i & j)
			 *  __and.wi		j
			 */
			else if
			((p[0]->ins_code == X_AND_WI) &&
			 (p[0]->ins_type == T_VALUE) &&
			 (p[1]->ins_code == I_LD_WI) &&
			 (p[1]->ins_type == T_VALUE)
			) {
				/* replace code */
				p[1]->ins_data &= p[0]->ins_data;
				remove = 1;
			}

			/*
			 *  __ld.wi		i	-->	__ld.wi		(i ^ j)
			 *  __eor.wi		j
			 */
			else if
			((p[0]->ins_code == X_EOR_WI) &&
			 (p[0]->ins_type == T_VALUE) &&
			 (p[1]->ins_code == I_LD_WI) &&
			 (p[1]->ins_type == T_VALUE)
			) {
				/* replace code */
				p[1]->ins_data ^= p[0]->ins_data;
				remove = 1;
			}

			/*
			 *  __ld.wi		i	-->	__ld.wi		(i | j)
			 *  __or.wi		j
			 */
			else if
			((p[0]->ins_code == X_OR_WI) &&
			 (p[0]->ins_type == T_VALUE) &&
			 (p[1]->ins_code == I_LD_WI) &&
			 (p[1]->ins_type == T_VALUE)
			) {
				/* replace code */
				p[1]->ins_data |= p[0]->ins_data;
				remove = 1;
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
				remove = 1;
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
				remove = 1;
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
				remove = 1;
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
				remove = 1;
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
				remove = 1;
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
				remove = 1;
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
				remove = 1;
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
				remove = 1;
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
				remove = 1;
			}

			/* remove instructions from queue and begin again */
			if (remove)
				goto lv1_loop;
		}

		/* ********************************************************* */
		/* then transform muliplication and division by a power of 2 */
		/* ********************************************************* */

		if (p_nb >= 1) {
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
					remove = 1;
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
			else if
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
					remove = 1;
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
			else if
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
					p[0]->ins_code = X_AND_WI;
					p[0]->ins_data = p[0]->ins_data - 1;
					/* no instructions removed, just loop */
					goto lv1_loop;
				}
			}

			/* remove instructions from queue and begin again */
			if (remove)
				goto lv1_loop;
		}

		/* ********************************************************* */
		/* then optimize conditional tests */
		/* ********************************************************* */

		if (p_nb >= 5) {
			/*
			 *  is_ubyte()			-->	is_ubyte()
			 *  __push.wr				__ldx.{w/b/u}mq	index
			 *  __ldx.{w/b/u}mq	index		__cmp_b.uaxq	type, symbol
			 *  __ld.uax		symbol
			 *  __cmp_w.wt		type
			 *
			 *  is_ubyte()			-->	is_ubyte()
			 *  __push.wr				__ldx.{w/b/u}sq	index
			 *  __ldx.{w/b/u}sq	index		__cmp_b.uaxq	type, symbol
			 *  __ld.uax		symbol
			 *  __cmp_w.wt		type
			 */
			if
			((p[0]->ins_code == I_CMP_WT) &&
			 (p[1]->ins_code == X_LD_UAX) &&
			 (p[2]->ins_code == X_LDX_WMQ ||
			  p[2]->ins_code == X_LDX_BMQ ||
			  p[2]->ins_code == X_LDX_UMQ ||
			  p[2]->ins_code == X_LDX_WSQ ||
			  p[2]->ins_code == X_LDX_BSQ ||
			  p[2]->ins_code == X_LDX_USQ) &&
			 (p[3]->ins_code == I_PUSH_WR) &&
			 (is_ubyte(p[4]))
			) {
				/* replace code */
				*p[3] = *p[2];
				*p[2] = *p[1];
				p[2]->ins_code = X_CMP_UAXQ;
				p[2]->cmp_type = compare2uchar[p[0]->cmp_type];
				switch (p[4]->ins_code) {
				case I_LD_UM:  p[4]->ins_code = X_LD_UMQ; break;
				case I_LD_UP:  p[4]->ins_code = X_LD_UPQ; break;
				case X_LD_US:  p[4]->ins_code = X_LD_USQ; break;
				case X_LD_UAR: p[4]->ins_code = X_LD_UARQ; break;
				case X_LD_UAX: p[4]->ins_code = X_LD_UAXQ; break;
				default: break;
				}
				remove = 2;
			}

			/* remove instructions from queue and begin again */
			if (remove)
				goto lv1_loop;
		}

		if (p_nb >= 4) {
			/*
			 *  is_ubyte()			-->	is_ubyte()
			 *  __push.wr				__cmp_b.umq	type, symbol
			 *  __ld.um		symbol
			 *  __cmp_w.wt		type
			 *
			 *  is_ubyte()			-->	is_ubyte()
			 *  __push.wr				__cmp_b.usq	type, n
			 *  __ld.us		n
			 *  __cmp_w.wt		type
			 */
			if
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
				case X_LD_US: p[2]->ins_code = X_CMP_USQ; break;
				default:	break;
				}
				p[2]->cmp_type = compare2uchar[p[0]->cmp_type];
				switch (p[3]->ins_code) {
				case I_LD_UM:  p[3]->ins_code = X_LD_UMQ; break;
				case I_LD_UP:  p[3]->ins_code = X_LD_UPQ; break;
				case X_LD_US:  p[3]->ins_code = X_LD_USQ; break;
				case X_LD_UAR: p[3]->ins_code = X_LD_UARQ; break;
				case X_LD_UAX: p[3]->ins_code = X_LD_UAXQ; break;
				default: break;
				}
				remove = 2;
			}

			/*
			 *  __push.wr			-->	__ld2x.{w/b/u}m	index
			 *  __ld2x.{w/b/u}mq	index		__cmp_w.wax	type, symbol
			 *  __ld.wax		symbol
			 *  __cmp_w.wt		type
			 *
			 *  __push.wr			-->	__ld2x.{w/b/u}s	index
			 *  __ld2x.{w/b/u}sq	index		__cmp_w.wax	type, symbol
			 *  __ld.wax		symbol
			 *  __cmp_w.wt		type
			 *
			 *  __push.wr			-->	__ldx.{w/b/u}mq	index
			 *  __ldx.{w/b/u}mq	index		__cmp_w.uax	type, symbol
			 *  __ld.uax		symbol
			 *  __cmp_w.wt		type
			 *
			 *  __push.wr			-->	__ldx.{w/b/u}s	index
			 *  __ldx.{w/b/u}sq	index		__cmp_w.uax	type, symbol
			 *  __ld.uax		symbol
			 *  __cmp_w.wt		type
			 */
			else if
			((p[0]->ins_code == I_CMP_WT) &&
			 (p[1]->ins_code == X_LD_WAX ||
			  p[1]->ins_code == X_LD_UAX) &&
			 (p[2]->ins_code == X_LD2X_WMQ ||
			  p[2]->ins_code == X_LD2X_BMQ ||
			  p[2]->ins_code == X_LD2X_UMQ ||
			  p[2]->ins_code == X_LD2X_WSQ ||
			  p[2]->ins_code == X_LD2X_BSQ ||
			  p[2]->ins_code == X_LD2X_USQ ||
			  p[2]->ins_code == X_LDX_WMQ ||
			  p[2]->ins_code == X_LDX_BMQ ||
			  p[2]->ins_code == X_LDX_UMQ ||
			  p[2]->ins_code == X_LDX_WSQ ||
			  p[2]->ins_code == X_LDX_BSQ ||
			  p[2]->ins_code == X_LDX_USQ) &&
			 (p[3]->ins_code == I_PUSH_WR)
			) {
				/* replace code */
				*p[3] = *p[2];
				*p[2] = *p[1];
				switch (p[1]->ins_code) {
				case X_LD_WAX:
					switch (p[3]->ins_code) {
					case X_LD2X_WMQ: p[3]->ins_code = X_LD2X_WM; break;
					case X_LD2X_BMQ: p[3]->ins_code = X_LD2X_BM; break;
					case X_LD2X_UMQ: p[3]->ins_code = X_LD2X_UM; break;
					case X_LD2X_WSQ: p[3]->ins_code = X_LD2X_WS; break;
					case X_LD2X_BSQ: p[3]->ins_code = X_LD2X_BS; break;
					case X_LD2X_USQ: p[3]->ins_code = X_LD2X_US; break;
					default: break;
					}
					p[2]->ins_code = X_CMP_WAX;
					break;
				case X_LD_UAX:
					switch (p[3]->ins_code) {
					case X_LDX_WMQ: p[3]->ins_code = X_LDX_WMQ; break;
					case X_LDX_BMQ: p[3]->ins_code = X_LDX_BMQ; break;
					case X_LDX_UMQ: p[3]->ins_code = X_LDX_UMQ; break;
					case X_LDX_WSQ: p[3]->ins_code = X_LDX_WS; break;
					case X_LDX_BSQ: p[3]->ins_code = X_LDX_BS; break;
					case X_LDX_USQ: p[3]->ins_code = X_LDX_US; break;
					default: break;
					}
					p[2]->ins_code = X_CMP_UAX;
					break;
				default: break;
				}
				p[2]->cmp_type = p[0]->cmp_type;
				remove = 2;
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
				remove = 1;
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
			 *  that the compiler always generates with back-to-back
			 *  "&&" or "||" sub-expressions.
			 */
			else if
			((p[0]->ins_code == I_BFALSE ||
			  p[0]->ins_code == I_BTRUE) &&
			 (p[1]->ins_code == I_TST_WR) &&
			 (p[2]->ins_code == I_BOOLEAN) &&
			 (p[3]->ins_code == I_LABEL)
			) {
				*p[2] = *p[0];
				remove = 2;
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
				remove = 1;
			}

			/* remove instructions from queue and begin again */
			if (remove)
				goto lv1_loop;
		}

		if (p_nb >= 3) {
			/*
			 *  __push.wr			-->	__not.wr
			 *  __ld.wi		0
			 *  __cmp_w.wt		equ_w
			 *
			 *  __push.wr			-->	__tst.wr
			 *  __ld.wi		0
			 *  __cmp_w.wt		neq_w
			 *
			 *  Check for this before converting to __cmp_w.wi!
			 */
			if
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
				remove = 2;
			}

			/*
			 *  __push.wr			-->	__cmp_w.wi	type, i
			 *  __ld.wi		i
			 *  __cmp_w.wt		type
			 *
			 *  __push.wr			-->	__cmp_w.wm	type, symbol
			 *  __ld.wm		symbol
			 *  __cmp_w.wt		type
			 *
			 *  __push.wr			-->	__cmp_w.um	type, symbol
			 *  __ld.um		symbol
			 *  __cmp_w.wt		type
			 *
			 *  __push.wr			-->	__cmp_w.ws	type, n
			 *  __ld.ws		n
			 *  __cmp_w.wt		type
			 *
			 *  __push.wr			-->	__cmp_w.us	type, n
			 *  __ld.us		n
			 *  __cmp_w.wt		type
			 */
			else if
			((p[0]->ins_code == I_CMP_WT) &&
			 (p[1]->ins_code == I_LD_WI ||
			  p[1]->ins_code == I_LD_WM ||
			  p[1]->ins_code == I_LD_UM ||
			  p[1]->ins_code == X_LD_WS ||
			  p[1]->ins_code == X_LD_US) &&
			 (p[2]->ins_code == I_PUSH_WR)
			) {
				/* replace code */
				*p[2] = *p[1];
				switch (p[1]->ins_code) {
				case I_LD_WI: p[2]->ins_code = X_CMP_WI; break;
				case I_LD_WM: p[2]->ins_code = X_CMP_WM; break;
				case I_LD_UM: p[2]->ins_code = X_CMP_UM; break;
				case X_LD_WS: p[2]->ins_code = X_CMP_WS; break;
				case X_LD_US: p[2]->ins_code = X_CMP_US; break;
				default:	break;
				}
				p[2]->cmp_type = p[0]->cmp_type;
				remove = 2;
			}

			/*
			 *  __cmp_w.wt			-->	__cmp_w.wt
			 *  __bool
			 *  __not.wr
			 *
			 *  __cmp_w.wi			-->	__cmp_w.wi
			 *  __bool
			 *  __not.wr
			 *
			 *  __cmp_w.wm			-->	__cmp_w.wm
			 *  __bool
			 *  __not.wr
			 *
			 *  __cmp_w.um			-->	__cmp_w.um
			 *  __bool
			 *  __not.wr
			 *
			 *  __cmp_w.ws			-->	__cmp_w.ws
			 *  __bool
			 *  __not.wr
			 *
			 *  __cmp_w.us			-->	__cmp_w.us
			 *  __bool
			 *  __not.wr
			 *
			 *  __cmp_w.wax			-->	__cmp_w.wax
			 *  __bool
			 *  __not.wr
			 *
			 *  __cmp_w.uax			-->	__cmp_w.uax
			 *  __bool
			 *  __not.wr
			 *
			 *  __cmp_b.uiq			-->	__cmp_b.uiq
			 *  __bool
			 *  __not.wr
			 *
			 *  __cmp_b.umq			-->	__cmp_b.umq
			 *  __bool
			 *  __not.wr
			 *
			 *  __cmp_b.usq			-->	__cmp_b.usq
			 *  __bool
			 *  __not.wr
			 *
			 *  __cmp_b.uaxq		-->	__cmp_b.uaxq
			 *  __bool
			 *  __not.wr
			 *
			 *  N.B. This inverts the test condition of the __cmp.wt!
			 */
			else if
			(
			 (p[0]->ins_code == I_NOT_WR) &&
			 (p[1]->ins_code == I_BOOLEAN) &&
			 (p[2]->ins_code == I_CMP_WT ||
			  p[2]->ins_code == X_CMP_WI ||
			  p[2]->ins_code == X_CMP_WM ||
			  p[2]->ins_code == X_CMP_UM ||
			  p[2]->ins_code == X_CMP_WS ||
			  p[2]->ins_code == X_CMP_US ||
			  p[2]->ins_code == X_CMP_WAX ||
			  p[2]->ins_code == X_CMP_UAX ||
			  p[2]->ins_code == X_CMP_UIQ ||
			  p[2]->ins_code == X_CMP_UMQ ||
			  p[2]->ins_code == X_CMP_USQ ||
			  p[2]->ins_code == X_CMP_UAXQ)
			) {
				p[2]->cmp_type = compare2not[p[2]->cmp_type];
				remove = 2;
			}

			/*
			 *  __not.wr			-->	__tst.wr
			 *  __bool
			 *  __not.wr
			 *
			 *  __not.{w/u}p		-->	__tst.{w/u}p
			 *  __bool
			 *  __not.wr
			 *
			 *  __not.{w/u}m	symbol	-->	__tst.{w/u}m	symbol
			 *  __bool
			 *  __not.wr
			 *
			 *  __not.{w/u}s	n	-->	__tst.{w/u}s	n
			 *  __bool
			 *  __not.wr
			 *
			 *  __not.{w/u}ar	symbol	-->	__tst.{w/u}ar	symbol
			 *  __bool
			 *  __not.wr
			 *
			 *  __not.{w/u}ax	symbol	-->	__tst.{w/u}ax	symbol
			 *  __bool
			 *  __not.wr
			 *
			 *  __not.uay		symbol	-->	__tst.uay	symbol
			 *  __bool
			 *  __not.wr
			 *
			 *  __nand.wi		i	-->	__tand.wi	i
			 *  __bool
			 *  __not.wr
			 *
			 *  __tst.wr			-->	__not.wr
			 *  __bool
			 *  __not.wr
			 *
			 *  __tst.{w/u}p		-->	__not.{w/u}p
			 *  __bool
			 *  __not.wr
			 *
			 *  __tst.{w/u}m	symbol	-->	__not.{w/u}m	symbol
			 *  __bool
			 *  __not.wr
			 *
			 *  __tst.{w/u}s	n	-->	__not.{w/u}s	n
			 *  __bool
			 *  __not.wr
			 *
			 *  __tst.{w/u}ar	symbol	-->	__not.{w/u}ar	symbol
			 *  __bool
			 *  __not.wr
			 *
			 *  __tst.{w/u}ax	symbol	-->	__not.{w/u}ax	symbol
			 *  __bool
			 *  __not.wr
			 *
			 *  __tst.uay		symbol	-->	__not.uay	symbol
			 *  __bool
			 *  __not.wr
			 *
			 *  __tand.wi		i	-->	__nand.wi	i
			 *  __bool
			 *  __not.wr
			 *
			 *  N.B. This inverts the test condition of the __tst.* or __not.*
			 */
			else if
			((p[0]->ins_code == I_NOT_WR) &&
			 (p[1]->ins_code == I_BOOLEAN) &&
			 (p[2]->ins_code == I_NOT_WR ||
			  p[2]->ins_code == X_NOT_WP ||
			  p[2]->ins_code == X_NOT_WM ||
			  p[2]->ins_code == X_NOT_WS ||
			  p[2]->ins_code == X_NOT_WAR ||
			  p[2]->ins_code == X_NOT_WAX ||
			  p[2]->ins_code == X_NOT_UP ||
			  p[2]->ins_code == X_NOT_UM ||
			  p[2]->ins_code == X_NOT_US ||
			  p[2]->ins_code == X_NOT_UAR ||
			  p[2]->ins_code == X_NOT_UAX ||
			  p[2]->ins_code == X_NOT_UAY ||
			  p[2]->ins_code == X_NAND_WI ||
			  p[2]->ins_code == I_TST_WR ||
			  p[2]->ins_code == X_TST_WP ||
			  p[2]->ins_code == X_TST_WM ||
			  p[2]->ins_code == X_TST_WS ||
			  p[2]->ins_code == X_TST_WAR ||
			  p[2]->ins_code == X_TST_WAX ||
			  p[2]->ins_code == X_TST_UP ||
			  p[2]->ins_code == X_TST_UM ||
			  p[2]->ins_code == X_TST_US ||
			  p[2]->ins_code == X_TST_UAR ||
			  p[2]->ins_code == X_TST_UAX ||
			  p[2]->ins_code == X_TST_UAY ||
			  p[2]->ins_code == X_TAND_WI)
			) {
				switch (p[2]->ins_code) {
				case I_NOT_WR:   p[2]->ins_code = I_TST_WR; break;
				case X_NOT_WP:   p[2]->ins_code = X_TST_WP; break;
				case X_NOT_WM:   p[2]->ins_code = X_TST_WM; break;
				case X_NOT_WS:   p[2]->ins_code = X_TST_WS; break;
				case X_NOT_WAR:  p[2]->ins_code = X_TST_WAR; break;
				case X_NOT_WAX:  p[2]->ins_code = X_TST_WAX; break;
				case X_NOT_UP:   p[2]->ins_code = X_TST_UP; break;
				case X_NOT_UM:   p[2]->ins_code = X_TST_UM; break;
				case X_NOT_US:   p[2]->ins_code = X_TST_US; break;
				case X_NOT_UAR:  p[2]->ins_code = X_TST_UAR; break;
				case X_NOT_UAX:  p[2]->ins_code = X_TST_UAX; break;
				case X_NOT_UAY:  p[2]->ins_code = X_TST_UAY; break;
				case X_NAND_WI:  p[2]->ins_code = X_TAND_WI; break;
				case I_TST_WR:   p[2]->ins_code = I_NOT_WR; break;
				case X_TST_WP:   p[2]->ins_code = X_NOT_WP; break;
				case X_TST_WM:   p[2]->ins_code = X_NOT_WM; break;
				case X_TST_WS:   p[2]->ins_code = X_NOT_WS; break;
				case X_TST_WAR:  p[2]->ins_code = X_NOT_WAR; break;
				case X_TST_WAX:  p[2]->ins_code = X_NOT_WAX; break;
				case X_TST_UP:   p[2]->ins_code = X_NOT_UP; break;
				case X_TST_UM:   p[2]->ins_code = X_NOT_UM; break;
				case X_TST_US:   p[2]->ins_code = X_NOT_US; break;
				case X_TST_UAR:  p[2]->ins_code = X_NOT_UAR; break;
				case X_TST_UAX:  p[2]->ins_code = X_NOT_UAX; break;
				case X_TST_UAY:  p[2]->ins_code = X_NOT_UAY; break;
				case X_TAND_WI:  p[2]->ins_code = X_NAND_WI; break;
				default: abort();
				}
				remove = 2;
			}

			/*
			 *  __cmp_w.wt			-->	__cmp_w.wt
			 *  __bool
			 *  __tst.wr
			 *
			 *  __cmp_w.wi			-->	__cmp_w.wi
			 *  __bool
			 *  __tst.wr
			 *
			 *  __cmp_w.{w/u}m		-->	__cmp_w.{w/u}m
			 *  __bool
			 *  __tst.wr
			 *
			 *  __cmp_w.{w/u}s		-->	__cmp_w.{w/u}s
			 *  __bool
			 *  __tst.wr
			 *
			 *  __cmp_w.{w/u}ax		-->	__cmp_w.{w/s}ax
			 *  __bool
			 *  __tst.wr
			 *
			 *  __cmp_b.uiq			-->	__cmp_b.uiq
			 *  __bool
			 *  __tst.wr
			 *
			 *  __cmp_b.umq			-->	__cmp_b.umq
			 *  __bool
			 *  __tst.wr
			 *
			 *  __cmp_b.usq			-->	__cmp_b.usq
			 *  __bool
			 *  __tst.wr
			 *
			 *  __cmp_b.uaxq		-->	__cmp_b.uaxq
			 *  __bool
			 *  __tst.wr
			 *
			 *  __not.wr			-->	__not.wr
			 *  __bool
			 *  __tst.wr
			 *
			 *  __not.{w/u}p		-->	__not.{w/u}p
			 *  __bool
			 *  __tst.wr
			 *
			 *  __not.{w/u}m	symbol	-->	__not.{w/u}m	symbol
			 *  __bool
			 *  __tst.wr
			 *
			 *  __not.{w/u}s	n	-->	__not.{w/u}s	n
			 *  __bool
			 *  __tst.wr
			 *
			 *  __not.{w/u}ar	symbol	-->	__not.{w/u}ar	symbol
			 *  __bool
			 *  __tst.wr
			 *
			 *  __not.{w/u}ax	symbol	-->	__not.{w/u}ax	symbol
			 *  __bool
			 *  __tst.wr
			 *
			 *  __not.uay		symbol	-->	__not.uay	symbol
			 *  __bool
			 *  __tst.wr
			 *
			 *  __nand.wi		i	-->	__nand.wi	i
			 *  __bool
			 *  __tst.wr
			 *
			 *  __tst.wr			-->	__tst.wr
			 *  __bool
			 *  __tst.wr
			 *
			 *  __tst.{w/u}p		-->	__tst.{w/u}p
			 *  __bool
			 *  __tst.wr
			 *
			 *  __tst.{w/u}m	symbol	-->	__tst.{w/u}m	symbol
			 *  __bool
			 *  __tst.wr
			 *
			 *  __tst.{w/u}s	n	-->	__tst.{w/u}s	n
			 *  __bool
			 *  __tst.wr
			 *
			 *  __tst.{w/u}ar	symbol	-->	__tst.{w/u}ar	symbol
			 *  __bool
			 *  __tst.wr
			 *
			 *  __tst.{w/u}ax	symbol	-->	__tst.{w/u}ax	symbol
			 *  __bool
			 *  __tst.wr
			 *
			 *  __tst.uay		symbol	-->	__tst.uay	symbol
			 *  __bool
			 *  __tst.wr
			 *
			 *  __tand.wi		i	-->	__tand.wi	i
			 *  __bool
			 *  __tst.wr
			 *
			 *  Remove redundant __tst.wr in compound conditionals
			 *  that the compiler often generates in testjump() or
			 *  at the end of an "&&" or "||".
			 */
			else if
			((p[0]->ins_code == I_TST_WR) &&
			 (p[1]->ins_code == I_BOOLEAN) &&
			 (p[2]->ins_code == I_CMP_WT ||
			  p[2]->ins_code == X_CMP_WI ||
			  p[2]->ins_code == X_CMP_WM ||
			  p[2]->ins_code == X_CMP_UM ||
			  p[2]->ins_code == X_CMP_WS ||
			  p[2]->ins_code == X_CMP_US ||
			  p[2]->ins_code == X_CMP_WAX ||
			  p[2]->ins_code == X_CMP_UAX ||
			  p[2]->ins_code == X_CMP_UIQ ||
			  p[2]->ins_code == X_CMP_UMQ ||
			  p[2]->ins_code == X_CMP_USQ ||
			  p[2]->ins_code == X_CMP_UAXQ ||
			  p[2]->ins_code == I_NOT_WR ||
			  p[2]->ins_code == X_NOT_WP ||
			  p[2]->ins_code == X_NOT_WM ||
			  p[2]->ins_code == X_NOT_WS ||
			  p[2]->ins_code == X_NOT_WAR ||
			  p[2]->ins_code == X_NOT_WAX ||
			  p[2]->ins_code == X_NOT_UP ||
			  p[2]->ins_code == X_NOT_UM ||
			  p[2]->ins_code == X_NOT_US ||
			  p[2]->ins_code == X_NOT_UAR ||
			  p[2]->ins_code == X_NOT_UAX ||
			  p[2]->ins_code == X_NOT_UAY ||
			  p[2]->ins_code == X_NAND_WI ||
			  p[2]->ins_code == I_TST_WR ||
			  p[2]->ins_code == X_TST_WP ||
			  p[2]->ins_code == X_TST_WM ||
			  p[2]->ins_code == X_TST_WS ||
			  p[2]->ins_code == X_TST_WAR ||
			  p[2]->ins_code == X_TST_WAX ||
			  p[2]->ins_code == X_TST_UP ||
			  p[2]->ins_code == X_TST_UM ||
			  p[2]->ins_code == X_TST_US ||
			  p[2]->ins_code == X_TST_UAR ||
			  p[2]->ins_code == X_TST_UAX ||
			  p[2]->ins_code == X_TST_UAY ||
			  p[2]->ins_code == X_TAND_WI)
			) {
				remove = 2;
			}

			/* remove instructions from queue and begin again */
			if (remove)
				goto lv1_loop;
		}

		if (p_nb >= 2) {
			/*
			 *  __ld.{w/b/u}p		-->	__tst.{w/u}p
			 *  __tst.wr
			 *
			 *  __ld.{w/b/u}m	symbol	-->	__tst.{w/u}m	symbol
			 *  __tst.wr
			 *
			 *  __ld.{w/b/u}s	n	-->	__tst.{w/u}s	n
			 *  __tst.wr
			 *
			 *  __ld.{w/b/u}ar	symbol	-->	__tst.{w/u}ar	symbol
			 *  __tst.wr
			 *
			 *  __ld.{w/b/u}ax	symbol	-->	__tst.{w/u}ax	symbol
			 *  __tst.wr
			 *
			 *  __ld.{b/u}ay	symbol	-->	__tst.uay	symbol
			 *  __tst.wr
			 *
			 *  __and.{w/u}i	i	-->	__tand.wi	i
			 *  __tst.wr
			 *
			 *  __ld.{w/b/u}p		-->	__not.{w/u}p
			 *  __not.wr
			 *
			 *  __ld.{w/b/u}m	symbol	-->	__not.{w/u}m	symbol
			 *  __not.wr
			 *
			 *  __ld.{w/b/u}s	n	-->	__not.{w/u}s	n
			 *  __not.wr
			 *
			 *  __ld.{w/b/u}ar	symbol	-->	__not.{w/u}ar	symbol
			 *  __not.wr
			 *
			 *  __ld.{w/b/u}ax	symbol	-->	__not.{w/u}ax	symbol
			 *  __not.wr
			 *
			 *  __ld.{b/u}ay	symbol	-->	__not.uay	symbol
			 *  __not.wr
			 *
			 *  __and.{w/u}i	i	-->	__nand.wi	i
			 *  __not.wr
			 */
			if
			(
			 (p[0]->ins_code == I_TST_WR ||
			  p[0]->ins_code == I_NOT_WR) &&
			 (p[1]->ins_code == I_LD_WP ||
			  p[1]->ins_code == I_LD_WM ||
			  p[1]->ins_code == X_LD_WS ||
			  p[1]->ins_code == X_LD_WAR ||
			  p[1]->ins_code == X_LD_WAX ||
			  p[1]->ins_code == I_LD_BP ||
			  p[1]->ins_code == I_LD_BM ||
			  p[1]->ins_code == X_LD_BS ||
			  p[1]->ins_code == X_LD_BAR ||
			  p[1]->ins_code == X_LD_BAX ||
			  p[1]->ins_code == X_LD_BAY ||
			  p[1]->ins_code == I_LD_UP ||
			  p[1]->ins_code == I_LD_UM ||
			  p[1]->ins_code == X_LD_US ||
			  p[1]->ins_code == X_LD_UAR ||
			  p[1]->ins_code == X_LD_UAX ||
			  p[1]->ins_code == X_LD_UAY ||
			  p[1]->ins_code == X_AND_WI ||
			  p[1]->ins_code == X_AND_UIQ)
			) {
				/* remove code */
				if (p[0]->ins_code == I_TST_WR) {
					switch (p[1]->ins_code) {
					case I_LD_WP:  p[1]->ins_code = X_TST_WP; break;
					case I_LD_WM:  p[1]->ins_code = X_TST_WM; break;
					case X_LD_WS:  p[1]->ins_code = X_TST_WS; break;
					case X_LD_WAR: p[1]->ins_code = X_TST_WAR; break;
					case X_LD_WAX: p[1]->ins_code = X_TST_WAX; break;
					case I_LD_BP:
					case I_LD_UP:  p[1]->ins_code = X_TST_UP; break;
					case I_LD_BM:
					case I_LD_UM:  p[1]->ins_code = X_TST_UM; break;
					case X_LD_BS:
					case X_LD_US:  p[1]->ins_code = X_TST_US; break;
					case X_LD_BAR:
					case X_LD_UAR: p[1]->ins_code = X_TST_UAR; break;
					case X_LD_BAX:
					case X_LD_UAX: p[1]->ins_code = X_TST_UAX; break;
					case X_LD_BAY:
					case X_LD_UAY: p[1]->ins_code = X_TST_UAY; break;
					case X_AND_UIQ:
					case X_AND_WI: p[1]->ins_code = X_TAND_WI; break;
					default: abort();
					}
				} else {
					switch (p[1]->ins_code) {
					case I_LD_WP:  p[1]->ins_code = X_NOT_WP; break;
					case I_LD_WM:  p[1]->ins_code = X_NOT_WM; break;
					case X_LD_WS:  p[1]->ins_code = X_NOT_WS; break;
					case X_LD_WAR: p[1]->ins_code = X_NOT_WAR; break;
					case X_LD_WAX: p[1]->ins_code = X_NOT_WAX; break;
					case I_LD_BP:
					case I_LD_UP:  p[1]->ins_code = X_NOT_UP; break;
					case I_LD_BM:
					case I_LD_UM:  p[1]->ins_code = X_NOT_UM; break;
					case X_LD_BS:
					case X_LD_US:  p[1]->ins_code = X_NOT_US; break;
					case X_LD_BAR:
					case X_LD_UAR: p[1]->ins_code = X_NOT_UAR; break;
					case X_LD_BAX:
					case X_LD_UAX: p[1]->ins_code = X_NOT_UAX; break;
					case X_LD_BAY:
					case X_LD_UAY: p[1]->ins_code = X_NOT_UAY; break;
					case X_AND_UIQ:
					case X_AND_WI: p[1]->ins_code = X_NAND_WI; break;
					default: abort();
					}
				}
				remove = 1;
			}

			/*
			 *  __ld.u{m/p/s/ar/ax}	symbol	-->	__ld.u{m/p/s/ar/ax}q  symbol
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
				switch (p[1]->ins_code) {
				case I_LD_UM:  p[1]->ins_code = X_LD_UMQ; break;
				case I_LD_UP:  p[1]->ins_code = X_LD_UPQ; break;
				case X_LD_US:  p[1]->ins_code = X_LD_USQ; break;
				case X_LD_UAR: p[1]->ins_code = X_LD_UARQ; break;
				case X_LD_UAX: p[1]->ins_code = X_LD_UAXQ; break;
				default: break;
				}
				/* no instructions removed, just loop */
				goto lv1_loop;
			}

			/*
			 *  __bool			-->	LLnn:
			 *  LLnn:				__bool
			 *
			 *  Delay boolean conversion until the end of the list of labels
			 *  that are generated when using multiple "&&" and "||".
			 *
			 *  N.B. This optimization should be done before the X_TST_WM optimization!
			 */
			else if
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
				remove = 1;
			}

			/*
			 *  __bra LLaa			-->	__bra LLaa
			 *  __bra LLbb
			 */
			else if
			((p[0]->ins_code == I_BRA) &&
			 (p[1]->ins_code == I_BRA)
			) {
				remove = 1;
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
				remove = 1;
			}

			/*
			 *  LLaa:				LLaa .alias LLbb
			 *  __bra LLbb			-->	__bra LLbb
			 */
			else if
			((p[0]->ins_code == I_BRA) &&
			 (p[1]->ins_code == I_LABEL)
			) {
				int i = q_nb;
				int j = q_wr;
				while (--i) {
					if ((--j) < 0)
						j += Q_SIZE;
					if (q_ins[j].ins_code == I_LABEL) {
						if (q_ins[j].ins_data != p[0]->ins_data) {
							q_ins[j].ins_code = I_ALIAS;
							q_ins[j].imm_type = T_VALUE;
							q_ins[j].imm_data = p[0]->ins_data;
						}
					}
					else
					if (q_ins[j].ins_code != I_INFO)
						break;
				}
			}

			/* remove instructions from queue and begin again */
			if (remove)
				goto lv1_loop;
		}

		/* ********************************************************* */
		/* 4-instruction patterns */
		/* ********************************************************* */

		if (p_nb >= 4) {
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
				remove = 1;
			}

			/*
			 *  __lea.s		i	-->	__lea.s		i
			 *  __push.wr				__push.wr
			 *  __st.wm		__ptr		__ld.{w/b/u}s	i
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
				remove = 1;
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
				remove = 3;
			}

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
				remove = 3;
			}

			/*
			 *  __push.wr			-->	__ld2x.{w/b/u}m	index
			 *  __ld2x.{w/b/u}mq	index		__isub.wax	symbol
			 *  __ld.wax		symbol
			 *  __isub.wt
			 *
			 *  __push.wr			-->	__ld2x.{w/b/u}m	index
			 *  __ld2x.{w/b/u}mq	index		__add.wax	symbol
			 *  __ld.wax		symbol
			 *  __add.wt
			 *
			 *  etc/etc
			 *
			 *  __push.wr			-->	__ldx.{w/b/u}mq	index
			 *  __ldx.{w/b/u}mq	index		__isub.uax	symbol
			 *  __ld.uax		symbol
			 *  __isub.wt
			 *
			 *  __push.wr			-->	__ldx.{w/b/u}mq	index
			 *  __ldx.{w/b/u}mq	index		__add.uax	symbol
			 *  __ld.uax		symbol
			 *  __add.wt
			 *
			 *  etc/etc
			 *
			 *  __push.wr			-->	__ld2x.{w/b/u}s	index
			 *  __ld2x.{w/b/u}sq	index		__isub.wax	symbol
			 *  __ld.wax		symbol
			 *  __isub.wt
			 *
			 *  __push.wr			-->	__ld2x.{w/b/u}s	index
			 *  __ld2x.{w/b/u}sq	index		__add.wax	symbol
			 *  __ld.wax		symbol
			 *  __add.wt
			 *
			 *  etc/etc
			 *
			 *  __push.wr			-->	__ldx.{w/b/u}sq	index
			 *  __ldx.{w/b/u}sq	index		__isub.uax	symbol
			 *  __ld.uax		symbol
			 *  __isub.wt
			 *
			 *  __push.wr			-->	__ldx.{w/b/u}sq	index
			 *  __ldx.{w/b/u}sq	index		__add.uax	symbol
			 *  __ld.uax		symbol
			 *  __add.wt
			 *
			 *  etc/etc
			 */
			else if
			((p[0]->ins_code == X_ISUB_WT ||
			  p[0]->ins_code == I_ADD_WT ||
			  p[0]->ins_code == I_SUB_WT ||
			  p[0]->ins_code == I_AND_WT ||
			  p[0]->ins_code == I_EOR_WT ||
			  p[0]->ins_code == I_OR_WT) &&
			 (p[1]->ins_code == X_LD_WAX ||
			  p[1]->ins_code == X_LD_UAX) &&
			 (p[2]->ins_code == X_LD2X_WMQ ||
			  p[2]->ins_code == X_LD2X_BMQ ||
			  p[2]->ins_code == X_LD2X_UMQ ||
			  p[2]->ins_code == X_LD2X_WSQ ||
			  p[2]->ins_code == X_LD2X_BSQ ||
			  p[2]->ins_code == X_LD2X_USQ ||
			  p[2]->ins_code == X_LDX_WMQ ||
			  p[2]->ins_code == X_LDX_BMQ ||
			  p[2]->ins_code == X_LDX_UMQ ||
			  p[2]->ins_code == X_LDX_WSQ ||
			  p[2]->ins_code == X_LDX_BSQ ||
			  p[2]->ins_code == X_LDX_USQ) &&
			 (p[3]->ins_code == I_PUSH_WR)
			) {
				/* replace code */
				*p[3] = *p[2];
				*p[2] = *p[1];
				switch (p[1]->ins_code) {
				case X_LD_WAX:
					switch (p[3]->ins_code) {
					case X_LD2X_WMQ: p[3]->ins_code = X_LD2X_WM; break;
					case X_LD2X_BMQ: p[3]->ins_code = X_LD2X_BM; break;
					case X_LD2X_UMQ: p[3]->ins_code = X_LD2X_UM; break;
					case X_LD2X_WSQ: p[3]->ins_code = X_LD2X_WS; break;
					case X_LD2X_BSQ: p[3]->ins_code = X_LD2X_BS; break;
					case X_LD2X_USQ: p[3]->ins_code = X_LD2X_US; break;
					default: break;
					}
					switch (p[0]->ins_code) {
					case X_ISUB_WT: p[2]->ins_code = X_ISUB_WAX; break;
					case I_ADD_WT:  p[2]->ins_code = X_ADD_WAX; break;
					case I_SUB_WT:  p[2]->ins_code = X_SUB_WAX; break;
					case I_AND_WT:  p[2]->ins_code = X_AND_WAX; break;
					case I_EOR_WT:  p[2]->ins_code = X_EOR_WAX; break;
					case I_OR_WT:   p[2]->ins_code = X_OR_WAX; break;
					default: break;
					}
					break;
				case X_LD_UAX:
					switch (p[3]->ins_code) {
					case X_LDX_WMQ: p[3]->ins_code = X_LDX_WMQ; break;
					case X_LDX_BMQ: p[3]->ins_code = X_LDX_BMQ; break;
					case X_LDX_UMQ: p[3]->ins_code = X_LDX_UMQ; break;
					case X_LDX_WSQ: p[3]->ins_code = X_LDX_WS; break;
					case X_LDX_BSQ: p[3]->ins_code = X_LDX_BS; break;
					case X_LDX_USQ: p[3]->ins_code = X_LDX_US; break;
					default: break;
					}
					switch (p[0]->ins_code) {
					case X_ISUB_WT: p[2]->ins_code = X_ISUB_UAX; break;
					case I_ADD_WT:  p[2]->ins_code = X_ADD_UAX; break;
					case I_SUB_WT:  p[2]->ins_code = X_SUB_UAX; break;
					case I_AND_WT:  p[2]->ins_code = X_AND_UAX; break;
					case I_EOR_WT:  p[2]->ins_code = X_EOR_UAX; break;
					case I_OR_WT:   p[2]->ins_code = X_OR_UAX; break;
					default: break;
					}
					break;
				default: break;
				}
				remove = 2;
			}

			/* remove instructions from queue and begin again */
			if (remove)
				goto lv1_loop;
		}

		/* ********************************************************* */
		/* 3-instruction patterns */
		/* ********************************************************* */

		if (p_nb >= 3) {
			/*
			 *  __case			-->	LLnn:
			 *  __endcase
			 *  LLnn:
			 *
			 *  __default			-->	LLnn:
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
			 (p[2]->ins_code == I_CASE ||
			  p[2]->ins_code == I_DEFAULT)
			) {
				/* remove code */
				*p[2] = *p[0];
				remove = 2;
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
				remove = 2;
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
				remove = 2;
			}

			/*
			 *  __push.wr			-->	__isub.wm	symbol
			 *  __ld.wm		symbol
			 *  __isub.wt
			 *
			 *  __push.wr			-->	__add.wm	symbol
			 *  __ld.wm		symbol
			 *  __add.wt
			 *
			 *  __push.wr			-->	__sub.wm	symbol
			 *  __ld.wm		symbol
			 *  __sub.wt
			 *
			 *  __push.wr			-->	__and.wm	symbol
			 *  __ld.wm		symbol
			 *  __and.wt
			 *
			 *  __push.wr			-->	__eor.wm	symbol
			 *  __ld.wm		symbol
			 *  __eor.wt
			 *
			 *  __push.wr			-->	__or.wm		symbol
			 *  __ld.wm		symbol
			 *  __or.wt
			 *
			 *  etc/etc
			 */
			else if
			((p[0]->ins_code == X_ISUB_WT ||
			  p[0]->ins_code == I_ADD_WT ||
			  p[0]->ins_code == I_SUB_WT ||
			  p[0]->ins_code == I_AND_WT ||
			  p[0]->ins_code == I_EOR_WT ||
			  p[0]->ins_code == I_OR_WT) &&
			 (p[1]->ins_code == I_LD_WM ||
			  p[1]->ins_code == I_LD_UM ||
			  p[1]->ins_code == X_LD_WS ||
			  p[1]->ins_code == X_LD_US) &&
			 (p[2]->ins_code == I_PUSH_WR)
			) {
				/* replace code */
				*p[2] = *p[1];
				switch (p[1]->ins_code) {
				case I_LD_WM:
					switch (p[0]->ins_code) {
					case X_ISUB_WT: p[2]->ins_code = X_ISUB_WM; break;
					case I_ADD_WT:  p[2]->ins_code = X_ADD_WM; break;
					case I_SUB_WT:  p[2]->ins_code = X_SUB_WM; break;
					case I_AND_WT:  p[2]->ins_code = X_AND_WM; break;
					case I_EOR_WT:  p[2]->ins_code = X_EOR_WM; break;
					case I_OR_WT:   p[2]->ins_code = X_OR_WM; break;
					default: break;
					}
					break;
				case I_LD_UM:
					switch (p[0]->ins_code) {
					case X_ISUB_WT: p[2]->ins_code = X_ISUB_UM; break;
					case I_ADD_WT:  p[2]->ins_code = X_ADD_UM; break;
					case I_SUB_WT:  p[2]->ins_code = X_SUB_UM; break;
					case I_AND_WT:  p[2]->ins_code = X_AND_UM; break;
					case I_EOR_WT:  p[2]->ins_code = X_EOR_UM; break;
					case I_OR_WT:   p[2]->ins_code = X_OR_UM; break;
					default: break;
					}
					break;

				case X_LD_WS:
					switch (p[0]->ins_code) {
					case X_ISUB_WT: p[2]->ins_code = X_ISUB_WS; break;
					case I_ADD_WT:  p[2]->ins_code = X_ADD_WS; break;
					case I_SUB_WT:  p[2]->ins_code = X_SUB_WS; break;
					case I_AND_WT:  p[2]->ins_code = X_AND_WS; break;
					case I_EOR_WT:  p[2]->ins_code = X_EOR_WS; break;
					case I_OR_WT:   p[2]->ins_code = X_OR_WS; break;
					default: break;
					}
					break;
				case X_LD_US:
					switch (p[0]->ins_code) {
					case X_ISUB_WT: p[2]->ins_code = X_ISUB_US; break;
					case I_ADD_WT:  p[2]->ins_code = X_ADD_US; break;
					case I_SUB_WT:  p[2]->ins_code = X_SUB_US; break;
					case I_AND_WT:  p[2]->ins_code = X_AND_US; break;
					case I_EOR_WT:  p[2]->ins_code = X_EOR_US; break;
					case I_OR_WT:   p[2]->ins_code = X_OR_US; break;
					default: break;
					}
					break;
				default: break;
				}
				remove = 2;
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
//			 (cmp_operands(p[0], p[2]))
			) {
				/* replace code */
				switch (p[2]->ins_code) {
				case I_LD_WM:	p[2]->ins_code = (p[1]->ins_code == I_ADD_WI) ? X_INCLD_WM : X_DECLD_WM; break;
				case I_LD_BM:	p[2]->ins_code = (p[1]->ins_code == I_ADD_WI) ? X_INCLD_BM : X_DECLD_BM; break;
				case I_LD_UM:	p[2]->ins_code = (p[1]->ins_code == I_ADD_WI) ? X_INCLD_UM : X_DECLD_UM; break;
				default:	break;
				}
				remove = 2;
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
				remove = 2;
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
			 *  __ldp.{w/b/u}ax	symbol	-->	__incld_{w/b/u}ax  symbol
			 *  __add.wi		1
			 *  __st.{w/u}at	symbol
			 *
			 *  __ldp.{w/b/u}ax	symbol	-->	__decld_{w/b/u}ax  symbol
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
			 *  pre-increment, pre-decrement, AND post-increment, post-decrement!
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
			  p[2]->ins_code == X_LDP_WAX ||
			  p[2]->ins_code == X_LDP_BAX ||
			  p[2]->ins_code == X_LDP_UAX ||
			  p[2]->ins_code == X_LDP_BAY ||
			  p[2]->ins_code == X_LDP_UAY) &&
			 (p[0]->ins_type == p[2]->ins_type) &&
			 (p[0]->ins_data == p[2]->ins_data)
//			 (cmp_operands(p[0], p[2]))
			) {
				/* replace code */
				switch (p[2]->ins_code) {
				case X_LDP_WAR:	p[2]->ins_code = (p[1]->ins_code == I_ADD_WI) ? X_INCLD_WAR : X_DECLD_WAR; break;
				case X_LDP_BAR:	p[2]->ins_code = (p[1]->ins_code == I_ADD_WI) ? X_INCLD_BAR : X_DECLD_BAR; break;
				case X_LDP_UAR:	p[2]->ins_code = (p[1]->ins_code == I_ADD_WI) ? X_INCLD_UAR : X_DECLD_UAR; break;
				case X_LDP_WAX:	p[2]->ins_code = (p[1]->ins_code == I_ADD_WI) ? X_INCLD_WAX : X_DECLD_WAX; break;
				case X_LDP_BAX:	p[2]->ins_code = (p[1]->ins_code == I_ADD_WI) ? X_INCLD_BAX : X_DECLD_BAX; break;
				case X_LDP_UAX:	p[2]->ins_code = (p[1]->ins_code == I_ADD_WI) ? X_INCLD_UAX : X_DECLD_UAX; break;
				case X_LDP_BAY:	p[2]->ins_code = (p[1]->ins_code == I_ADD_WI) ? X_INCLD_BAY : X_DECLD_BAY; break;
				case X_LDP_UAY:	p[2]->ins_code = (p[1]->ins_code == I_ADD_WI) ? X_INCLD_UAY : X_DECLD_UAY; break;
				default:	break;
				}
				remove = 2;
			}

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
				remove = 2;
			}

			/*
			 *  __not.wr			-->	__boolnot.wr
			 *  __bool				is_usepr()
			 *  is_usepr()
			 *
			 *  __not.{w/u}p		-->	__boolnot.{w/u}p
			 *  __bool				is_usepr()
			 *  is_usepr()
			 *
			 *  __not.{w/u}m	symbol	-->	__boolnot.{w/u}m	symbol
			 *  __bool				is_usepr()
			 *  is_usepr()
			 *
			 *  __not.{w/u}s	n	-->	__boolnot.{w/u}s	n
			 *  __bool				is_usepr()
			 *  is_usepr()
			 *
			 *  __not.{w/u}ar	symbol	-->	__boolnot.{w/u}ar	symbol
			 *  __bool				is_usepr()
			 *  is_usepr()
			 *
			 *  __not.uax		symbol	-->	__boolnot.uax	symbol
			 *  __bool				is_usepr()
			 *  is_usepr()
			 *
			 *  __not.uay		symbol	-->	__boolnot.uay	symbol
			 *  __bool				is_usepr()
			 *  is_usepr()
			 *
			 *  Optimize "var = !var" which doesn't need to set the flags.
			 *
			 *  N.B. This MUST be done after the rule for merging two "!" because
			 *  I_NOT_WR is included in the is_usepr() test!
			 */
			else if
			((is_usepr(p[0])) &&
			 (p[1]->ins_code == I_BOOLEAN) &&
			 (p[2]->ins_code == I_NOT_WR ||
			  p[2]->ins_code == X_NOT_WP ||
			  p[2]->ins_code == X_NOT_WM ||
			  p[2]->ins_code == X_NOT_WS ||
			  p[2]->ins_code == X_NOT_WAR ||
			  p[2]->ins_code == X_NOT_UP ||
			  p[2]->ins_code == X_NOT_UM ||
			  p[2]->ins_code == X_NOT_US ||
			  p[2]->ins_code == X_NOT_UAR ||
			  p[2]->ins_code == X_NOT_UAX ||
			  p[2]->ins_code == X_NOT_UAY)
			) {
				/* replace code */
				*p[1] = *p[0];
				switch (p[2]->ins_code) {
				case I_NOT_WR:   p[2]->ins_code = X_BOOLNOT_WR; break;
				case X_NOT_WP:   p[2]->ins_code = X_BOOLNOT_WP; break;
				case X_NOT_WM:   p[2]->ins_code = X_BOOLNOT_WM; break;
				case X_NOT_WS:   p[2]->ins_code = X_BOOLNOT_WS; break;
				case X_NOT_WAR:  p[2]->ins_code = X_BOOLNOT_WAR; break;
				case X_NOT_UP:   p[2]->ins_code = X_BOOLNOT_UP; break;
				case X_NOT_UM:   p[2]->ins_code = X_BOOLNOT_UM; break;
				case X_NOT_US:   p[2]->ins_code = X_BOOLNOT_US; break;
				case X_NOT_UAR:  p[2]->ins_code = X_BOOLNOT_UAR; break;
				case X_NOT_UAX:  p[2]->ins_code = X_BOOLNOT_UAX; break;
				case X_NOT_UAY:  p[2]->ins_code = X_BOOLNOT_UAY; break;
				default: abort();
				}
				remove = 1;
			}

			/* remove instructions from queue and begin again */
			if (remove)
				goto lv1_loop;
		}

		/* ********************************************************* */
		/* 2-instruction patterns */
		/* ********************************************************* */

		if (p_nb >= 2) {
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
				remove = 0;
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
				remove = 1;
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
				remove = 1;
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
				remove = 1;
			}

#if 0
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
				remove = 1;
			}
#endif

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
				remove = 1;
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
				remove = 1;
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
				remove = 1;
			}

			/*
			 *  __st.wm		a	-->	__st.wm		a
			 *  __ld.wm		a
			 */
			else if
			((p[0]->ins_code == I_LD_WM) &&
			 (p[1]->ins_code == I_ST_WM) &&
			 (cmp_operands(p[0], p[1]))
			) {
				/* remove code */
				remove = 1;
			}

			/*
			 *  __st.ws		i	-->	__st.ws		i
			 *  __ld.ws		i
			 */
			else if
			((p[0]->ins_code == X_LD_WS) &&
			 (p[1]->ins_code == X_ST_WS) &&
			 (p[0]->ins_data == p[1]->ins_data)) {
				/* remove code */
				remove = 1;
			}

			/*
			 *  __st.us		i	-->	__st.us		i
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
				remove = 1;
			}

			/*
			 *  __decld.{w/b/u}m	symbol	-->	__lddec.{w/b/u}m  symbol
			 *  __add.wi  1
			 *
			 *  __decld.{w/b/u}s	n	-->	__lddec.{w/b/u}s  n
			 *  __add.wi  1
			 *
			 *  __decld.{w/b/u}ar	array	-->	__lddec.{w/b/u}ar  array
			 *  __add.wi  1
			 *
			 *  __decld.{w/b/u}ax	array	-->	__lddec.{w/b/u}ax  array
			 *  __add.wi  1
			 *
			 *  __decld.{b/u}ay	array	-->	__lddec.{b/u}ay  array
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
			  p[1]->ins_code == X_DECLD_WAX ||
			  p[1]->ins_code == X_DECLD_BAX ||
			  p[1]->ins_code == X_DECLD_UAX ||
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
				case X_DECLD_WAX: p[1]->ins_code = X_LDDEC_WAX; break;
				case X_DECLD_BAX: p[1]->ins_code = X_LDDEC_BAX; break;
				case X_DECLD_UAX: p[1]->ins_code = X_LDDEC_UAX; break;
				case X_DECLD_BAY: p[1]->ins_code = X_LDDEC_BAY; break;
				case X_DECLD_UAY: p[1]->ins_code = X_LDDEC_UAY; break;
				default:	break;
				}
				remove = 1;
			}

			/*
			 *  __incld.{w/b/u}m	symbol	-->	__ldinc.{w/b/u}m  symbol
			 *  __sub.wi  1
			 *
			 *  __incld.{w/b/u}s	n	-->	__ldinc.{w/b/u}s  n
			 *  __sub.wi  1
			 *
			 *  __incld.{w/b/u}ar	array	-->	__ldinc.{w/b/u}ar  array
			 *  __sub.wi  1
			 *
			 *  __incld.{w/b/u}ax	array	-->	__ldinc.{w/b/u}ax  array
			 *  __sub.wi  1
			 *
			 *  __incld.{b/u}ay	array	-->	__ldinc.{b/u}ay  array
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
			  p[1]->ins_code == X_INCLD_WAX ||
			  p[1]->ins_code == X_INCLD_BAX ||
			  p[1]->ins_code == X_INCLD_UAX ||
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
				case X_INCLD_WAX: p[1]->ins_code = X_LDINC_WAX; break;
				case X_INCLD_BAX: p[1]->ins_code = X_LDINC_BAX; break;
				case X_INCLD_UAX: p[1]->ins_code = X_LDINC_UAX; break;
				case X_INCLD_BAY: p[1]->ins_code = X_LDINC_BAY; break;
				case X_INCLD_UAY: p[1]->ins_code = X_LDINC_UAY; break;
				default:	break;
				}
				remove = 1;
			}

			/*
			 *  __ld.{w/b/u}m	symbol	-->	__ld.{w/b/u}mq	symbol
			 *  __index.wr		array		__index.wr	array
			 *
			 *  __ld.{w/b/u}m	symbol	-->	__ld.{w/b/u}mq	symbol
			 *  __index.ur		array		__index.ur	array
			 *
			 *  __ld.{w/b/u}s	n	-->	__ld.{w/b/u}sq	n
			 *  __index.wr		array		__index.wr	array
			 *
			 *  __ld.{w/b/u}s	n	-->	__ld.{w/b/u}sq	n
			 *  __index.ur		array		__index.ur	array
			 *
			 *  Index optimizations for base+offset array access.
			 */
			else if
			((p[0]->ins_code == X_INDEX_WR ||
			  p[0]->ins_code == X_INDEX_UR) &&
			 (p[1]->ins_code == I_LD_WM ||
			  p[1]->ins_code == I_LD_BM ||
			  p[1]->ins_code == I_LD_UM ||
			  p[1]->ins_code == X_LD_WS ||
			  p[1]->ins_code == X_LD_BS ||
			  p[1]->ins_code == X_LD_US)
			) {
				/* replace code */
				switch (p[1]->ins_code) {
				case I_LD_WM: p[1]->ins_code = X_LD_WMQ; break;
				case I_LD_BM: p[1]->ins_code = X_LD_BMQ; break;
				case I_LD_UM: p[1]->ins_code = X_LD_UMQ; break;
				case X_LD_WS: p[1]->ins_code = X_LD_WSQ; break;
				case X_LD_BS: p[1]->ins_code = X_LD_BSQ; break;
				case X_LD_US: p[1]->ins_code = X_LD_USQ; break;
				default:	break;
				}
				remove = 0;
			}

			/*
			 *  __ld.{w/b/u}m	symbol	-->	__ld2x.{w/b/u}mq
			 *  __ld.war		array		__ld.wax	array
			 *
			 *  __ld.{w/b/u}s	n	-->	__ld2x.{w/b/u}sq n
			 *  __ld.war		array		__ld.wax	array
			 *
			 *  __ld.{w/b/u}m	symbol	-->	__ldx.{w/b/u}mq
			 *  __ld.{b/u}ar	array		__ld.{b/u}ax	array
			 *
			 *  __ld.{w/b/u}s	n	-->	__ldx.{w/b/u}sq	n
			 *  __ld.{b/u}ar	array		__ld.{b/u}ax	array
			 *
			 *  __ld.{w/b/u}m	symbol	-->	__ld2x.{w/b/u}mq
			 *  __ldp.war		array		__ldp.wax	array
			 *
			 *  __ld.{w/b/u}s	n	-->	__ld2x.{w/b/u}sq n
			 *  __ldp.war		array		__ldp.wax	array
			 *
			 *  __ld.{w/b/u}m	symbol	-->	__ldx.{w/b/u}mq
			 *  __ldp.{b/u}ar	array		__ldp.{b/u}ax	array
			 *
			 *  __ld.{w/b/u}s	n	-->	__ldx.{w/b/u}sq	n
			 *  __ldp.{b/u}ar	array		__ldp.{b/u}ax	array
			 *
			 *  Index optimizations for base+offset array access.
			 */
			else if
			((p[0]->ins_code == X_LD_WAR ||
			  p[0]->ins_code == X_LD_BAR ||
			  p[0]->ins_code == X_LD_UAR ||
			  p[0]->ins_code == X_LDP_WAR ||
			  p[0]->ins_code == X_LDP_BAR ||
			  p[0]->ins_code == X_LDP_UAR) &&
			 (p[1]->ins_code == I_LD_WM ||
			  p[1]->ins_code == I_LD_BM ||
			  p[1]->ins_code == I_LD_UM ||
			  p[1]->ins_code == X_LD_WS ||
			  p[1]->ins_code == X_LD_BS ||
			  p[1]->ins_code == X_LD_US)
			) {
				/* replace code */
				if (p[0]->ins_code == X_LD_WAR || p[0]->ins_code == X_LDP_WAR) {
					switch (p[1]->ins_code) {
					case I_LD_WM: p[1]->ins_code = X_LD2X_WMQ; break;
					case I_LD_BM: p[1]->ins_code = X_LD2X_BMQ; break;
					case I_LD_UM: p[1]->ins_code = X_LD2X_UMQ; break;
					case X_LD_WS: p[1]->ins_code = X_LD2X_WSQ; break;
					case X_LD_BS: p[1]->ins_code = X_LD2X_BSQ; break;
					case X_LD_US: p[1]->ins_code = X_LD2X_USQ; break;
					default:	break;
					}
					switch (p[0]->ins_code) {
					case X_LD_WAR:  p[0]->ins_code = X_LD_WAX; break;
					case X_LDP_WAR: p[0]->ins_code = X_LDP_WAX; break;
					default:	break;
					}
					remove = 0;
				} else {
					switch (p[1]->ins_code) {
					case I_LD_WM: p[1]->ins_code = X_LDX_WMQ; break;
					case I_LD_BM: p[1]->ins_code = X_LDX_BMQ; break;
					case I_LD_UM: p[1]->ins_code = X_LDX_UMQ; break;
					case X_LD_WS: p[1]->ins_code = X_LDX_WSQ; break;
					case X_LD_BS: p[1]->ins_code = X_LDX_BSQ; break;
					case X_LD_US: p[1]->ins_code = X_LDX_USQ; break;
					default:	break;
					}
					switch (p[0]->ins_code) {
					case X_LD_BAR:  p[0]->ins_code = X_LD_BAX; break;
					case X_LD_UAR:  p[0]->ins_code = X_LD_UAX; break;
					case X_LDP_BAR: p[0]->ins_code = X_LDP_BAX; break;
					case X_LDP_UAR: p[0]->ins_code = X_LDP_UAX; break;
					default:	break;
					}
					remove = 0;
				}
			}

			/*
			 *  __mul.wi		n	-->	__mul.uiq	n
			 *  __index.{w/u}r	array		__index.{w/u}r	array
			 *
			 *  __asl.wi		n	-->	__asl.uiq	n
			 *  __index.{w/u}r	array		__index.{w/u}r	array
			 *
			 *  __mul.wi		n	-->	__mul.uiq	n
			 *  __ld.{w/b/u}ar	array		__ld.{w/b/u}ar	array
			 *
			 *  __asl.wi		n	-->	__asl.uiq	n
			 *  __ld.{w/b/u}ar	array		__ld.{w/b/u}ar	array
			 *
			 *  __mul.wi		n	-->	__mul.uiq	n
			 *  __ldp.{w/b/u}ar	array		__ldp.{w/b/u}ar	array
			 *
			 *  __asl.wi		n	-->	__asl.uiq	n
			 *  __ldp.{w/b/u}ar	array		__ldp.{w/b/u}ar	array
			 *
			 *  Index optimizations for access to an array of structs.
			 */
			else if
			((p[0]->ins_code == X_INDEX_WR ||
			  p[0]->ins_code == X_INDEX_UR ||
			  p[0]->ins_code == X_LD_WAR ||
			  p[0]->ins_code == X_LD_BAR ||
			  p[0]->ins_code == X_LD_UAR ||
			  p[0]->ins_code == X_LDP_WAR ||
			  p[0]->ins_code == X_LDP_BAR ||
			  p[0]->ins_code == X_LDP_UAR) &&
			 (p[1]->ins_code == I_ASL_WI ||
			  p[1]->ins_code == I_MUL_WI)
			) {
				/* replace code */
				if (p[1]->ins_code == I_ASL_WI)
					p[1]->ins_code = I_ASL_UIQ;
				else
					p[1]->ins_code = I_MUL_UIQ;
				switch (p[2]->ins_code) {
				case I_LD_WM: p[2]->ins_code = X_LD_WMQ; break;
				case I_LD_BM: p[2]->ins_code = X_LD_BMQ; break;
				case I_LD_UM: p[2]->ins_code = X_LD_UMQ; break;
				case X_LD_WS: p[2]->ins_code = X_LD_WSQ; break;
				case X_LD_BS: p[2]->ins_code = X_LD_BSQ; break;
				case X_LD_US: p[2]->ins_code = X_LD_USQ; break;
				default:	break;
				}
				remove = 0;
			}

#if 0
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
				remove = 0;
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
				remove = 0;
			}
#endif

			/*
			 *  __ld.u{p/m/s/ar/ax}	symbol	-->	__ld.u{p/m/s/ar/ax}  symbol
			 *  __sdiv.wi		i		__udiv.wi	i
			 *
			 *  __ld.u{p/m/s/ar/ax}	symbol	-->	__ld.u{p/m/s/ar/ax}  symbol
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
			 *  __ld.u{p/m/s/ar/ax}	symbol	-->	__ld.u{p/m/s/ar/ax}  symbol
			 *  __asr.wi		i		__lsr.uiq	i
			 *
			 *  __ld.u{p/m/s/ar/ax}	symbol	-->	__ld.u{p/m/s/ar/ax}  symbol
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
					remove = 1;
				} else {
					p[0]->ins_code = I_LSR_UIQ;
					/* no instructions removed, just loop */
					goto lv1_loop;
				}
			}

			/*
			 *  __ld.u{p/m/s/ar/ax}	symbol	-->	__ld.u{p/m/s/ar/ax}  symbol
			 *  __and.wi		i		__and.uiq	i
			 *
			 *  C promotes an unsigned char to a signed int so this
			 *  must be done in the peephole, not the compiler.
			 */
			else if
			((p[0]->ins_code == X_AND_WI) &&
			 (p[0]->ins_type == T_VALUE) &&
			 (p[0]->ins_data >= 0) &&
			 (p[0]->ins_data <= 255) &&
			 (is_ubyte(p[1]))
			) {
				/* replace code */
				p[0]->ins_code = X_AND_UIQ;
				/* no instructions removed, just loop */
				goto lv1_loop;
			}

			/*
			 *  __ld.wi		i	-->	__ldx.{w/b/u}mq	index
			 *  __ldx.{w/b/u}mq	index		__ld.wi		i
			 *
			 *  __ld.wi		i	-->	__ldx.{w/b/u}sq index
			 *  __ldx.{w/b/u}s	index		__ld.wi		i
			 *
			 *  __ld.wi		i	-->	__ld2x.{w/b/u}mq index
			 *  __ld2x.{w/b/u}m	index		__ld.wi		i
			 *
			 *  __ld.wi		i	-->	__ld2x.{w/b/u}sq index
			 *  __ld2x.{w/b/u}s	index		__ld.wi		i
			 *
			 *  __ld.{w/b/u}m	symbol	-->	__ldx.{w/b/u}mq	index
			 *  __ldx.{w/b/u}mq	index		__ld.{w/b/u}m	symbol
			 *
			 *  __ld.{w/b/u}m	symbol	-->	__ldx.{w/b/u}sq index
			 *  __ldx.{w/b/u}s	index		__ld.{w/b/u}m	symbol
			 *
			 *  __ld.{w/b/u}m	symbol	-->	__ld2x.{w/b/u}mq index
			 *  __ld2x.{w/b/u}m	index		__ld.{w/b/u}m	symbol
			 *
			 *  __ld.{w/b/u}m	symbol	-->	__ld2x.{w/b/u}sq index
			 *  __ld2x.{w/b/u}s	index		__ld.{w/b/u}m	symbol
			 *
			 *  swap the LDX so that it doesn't have to preserve the primary register
			 */
			else if
			((p[0]->ins_code == X_LDX_WMQ ||
			  p[0]->ins_code == X_LDX_BMQ ||
			  p[0]->ins_code == X_LDX_UMQ ||
			  p[0]->ins_code == X_LDX_WS  ||
			  p[0]->ins_code == X_LDX_BS  ||
			  p[0]->ins_code == X_LDX_US  ||
			  p[0]->ins_code == X_LD2X_WM ||
			  p[0]->ins_code == X_LD2X_BM ||
			  p[0]->ins_code == X_LD2X_UM ||
			  p[0]->ins_code == X_LD2X_WS ||
			  p[0]->ins_code == X_LD2X_BS ||
			  p[0]->ins_code == X_LD2X_US) &&
			 (p[1]->ins_code == I_LD_WI ||
			  p[1]->ins_code == I_LD_WM ||
			  p[1]->ins_code == I_LD_BM ||
			  p[1]->ins_code == I_LD_UM)
			) {
				/* replace code */
				INS parked = *p[1];
				*p[1] = *p[0];
				*p[0] = parked;
				switch (p[1]->ins_code) {
				case X_LDX_WMQ: p[1]->ins_code = X_LDX_WMQ; break;
				case X_LDX_BMQ: p[1]->ins_code = X_LDX_BMQ; break;
				case X_LDX_UMQ: p[1]->ins_code = X_LDX_UMQ; break;
				case X_LDX_WS : p[1]->ins_code = X_LDX_WSQ; break;
				case X_LDX_BS : p[1]->ins_code = X_LDX_BSQ; break;
				case X_LDX_US : p[1]->ins_code = X_LDX_USQ; break;
				case X_LD2X_WM: p[1]->ins_code = X_LD2X_WMQ; break;
				case X_LD2X_BM: p[1]->ins_code = X_LD2X_BMQ; break;
				case X_LD2X_UM: p[1]->ins_code = X_LD2X_UMQ; break;
				case X_LD2X_WS: p[1]->ins_code = X_LD2X_WSQ; break;
				case X_LD2X_BS: p[1]->ins_code = X_LD2X_BSQ; break;
				case X_LD2X_US: p[1]->ins_code = X_LD2X_USQ; break;
				default: break;
				}
				/* no instructions removed, just loop */
				goto lv1_loop;
			}

			/*
			 *  __ld.wi		i	-->	__add_st.{w/u}miq  i, symbol
			 *  __add_st.{w/u}mq	symbol
			 *
			 *  __ld.wi		i	-->	__sub_st.{w/u}miq  i, symbol
			 *  __isub_st.{w/u}mq	symbol
			 *
			 *  __ld.wi		i	-->	__add_st.{w/u}piq  i, symbol
			 *  __add_st.{w/u}pq	symbol
			 *
			 *  __ld.wi		i	-->	__sub_st.{w/u}piq  i, symbol
			 *  __isub_st.{w/u}pq	symbol
			 *
			 *  __ld.wi		i	-->	__add_st.{w/u}siq  i, n
			 *  __add_st.{w/u}sq	n
			 *
			 *  __ld.wi		i	-->	__sub_st.{w/u}siq  i, n
			 *  __isub_st.{w/u}sq	n
			 *
			 *  __ld.wi		i	-->	__add_st.{w/u}atiq  i, symbol
			 *  __add_st.{w/u}atq	symbol
			 *
			 *  __ld.wi		i	-->	__sub_st.{w/u}atiq  i, symbol
			 *  __isub_st.{w/u}atq	symbol
			 *
			 *  __ld.wi		i	-->	__add_st.{w/u}axiq  i, symbol
			 *  __add_st.{w/u}axq	symbol
			 *
			 *  __ld.wi		i	-->	__sub_st.{w/u}axiq  i, symbol
			 *  __isub_st.{w/u}axq	symbol
			 */
			else if
			((p[1]->ins_code == I_LD_WI) &&
			 (p[0]->ins_code == X_ADD_ST_WMQ ||
			  p[0]->ins_code == X_ADD_ST_UMQ ||
			  p[0]->ins_code == X_ADD_ST_WPQ ||
			  p[0]->ins_code == X_ADD_ST_UPQ ||
			  p[0]->ins_code == X_ADD_ST_WSQ ||
			  p[0]->ins_code == X_ADD_ST_USQ ||
			  p[0]->ins_code == X_ADD_ST_WATQ ||
			  p[0]->ins_code == X_ADD_ST_UATQ ||
			  p[0]->ins_code == X_ADD_ST_WAXQ ||
			  p[0]->ins_code == X_ADD_ST_UAXQ ||
			  p[0]->ins_code == X_ISUB_ST_WMQ ||
			  p[0]->ins_code == X_ISUB_ST_UMQ ||
			  p[0]->ins_code == X_ISUB_ST_WPQ ||
			  p[0]->ins_code == X_ISUB_ST_UPQ ||
			  p[0]->ins_code == X_ISUB_ST_WSQ ||
			  p[0]->ins_code == X_ISUB_ST_USQ ||
			  p[0]->ins_code == X_ISUB_ST_WATQ ||
			  p[0]->ins_code == X_ISUB_ST_UATQ ||
			  p[0]->ins_code == X_ISUB_ST_WAXQ ||
			  p[0]->ins_code == X_ISUB_ST_UAXQ)
			) {
				/* replace code */
				intptr_t data = p[1]->ins_data;
				*p[1] = *p[0];
				switch (p[1]->ins_code) {
				case X_ADD_ST_WMQ:   p[1]->ins_code = X_ADD_ST_WMIQ; break;
				case X_ADD_ST_UMQ:   p[1]->ins_code = X_ADD_ST_UMIQ; break;
				case X_ADD_ST_WPQ:   p[1]->ins_code = X_ADD_ST_WPIQ; break;
				case X_ADD_ST_UPQ:   p[1]->ins_code = X_ADD_ST_UPIQ; break;
				case X_ADD_ST_WSQ:   p[1]->ins_code = X_ADD_ST_WSIQ; break;
				case X_ADD_ST_USQ:   p[1]->ins_code = X_ADD_ST_USIQ; break;
				case X_ADD_ST_WATQ:  p[1]->ins_code = X_ADD_ST_WATIQ; break;
				case X_ADD_ST_UATQ:  p[1]->ins_code = X_ADD_ST_UATIQ; break;
				case X_ADD_ST_WAXQ:  p[1]->ins_code = X_ADD_ST_WAXIQ; break;
				case X_ADD_ST_UAXQ:  p[1]->ins_code = X_ADD_ST_UAXQ; break;
				case X_ISUB_ST_WMQ:  p[1]->ins_code = X_SUB_ST_WMIQ; break;
				case X_ISUB_ST_UMQ:  p[1]->ins_code = X_SUB_ST_UMIQ; break;
				case X_ISUB_ST_WPQ:  p[1]->ins_code = X_SUB_ST_WPIQ; break;
				case X_ISUB_ST_UPQ:  p[1]->ins_code = X_SUB_ST_UPIQ; break;
				case X_ISUB_ST_WSQ:  p[1]->ins_code = X_SUB_ST_WSIQ; break;
				case X_ISUB_ST_USQ:  p[1]->ins_code = X_SUB_ST_USIQ; break;
				case X_ISUB_ST_WATQ: p[1]->ins_code = X_SUB_ST_WATIQ; break;
				case X_ISUB_ST_UATQ: p[1]->ins_code = X_SUB_ST_UATIQ; break;
				case X_ISUB_ST_WAXQ: p[1]->ins_code = X_SUB_ST_WAXIQ; break;
				case X_ISUB_ST_UAXQ: p[1]->ins_code = X_SUB_ST_UAXIQ; break;
				default: break;
				}
				p[1]->imm_type = T_VALUE;
				p[1]->imm_data = data;
				remove = 1;
			}

			/* remove instructions from queue and begin again */
			if (remove)
				goto lv1_loop;
		}

		/* ********************************************************* */
		/* 1-instruction patterns */
		/* ********************************************************* */

		if (p_nb >= 1) {
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
				remove = 1;
			}

			/* remove instructions from queue and begin again */
			if (remove)
				goto lv1_loop;
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
	 */
	if (optimize >= 2) {
		int offset;
		int copy, drop, from, scan, prev, next;
		int index, operation;
		INS parked;
		parked.ins_code = 0;

		/* check last instruction */
		int old_code = q_ins[q_wr].ins_code;

		/*
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
		 *  __ld.{w/b/u}{m/s}		symbol	-->	__index.wr	array
		 *  __asl.wr
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

		if (q_nb > 1 &&
		(old_code == I_ST_WPT ||
		 old_code == I_ST_UPT)
		) {
			/* scan backwards to find the matching I_PUSH_WR */
			for (offset = 2, copy = 1, scan = q_wr; copy < q_nb; copy++) {
				/* check instruction */
				if (--scan < 0)
					scan += Q_SIZE;

				if (icode_flags[q_ins[scan].ins_code] & IS_PUSHWT)
					offset -= 2;
				else
				if (icode_flags[q_ins[scan].ins_code] & IS_POPWT)
					offset += 2;

				/* check offset */
				if (offset != 0)
					continue;

				/* there should be at least one instruction between I_PUSH_WR and I_ST_WPT */
				if (copy == 1)
					break;

				/* found the I_PUSH_WR that matches the I_ST_WPT */
				from = scan + 1;  /* begin copying after the I_PUSH_WR */
				drop = 2;  /* drop I_PUSH_WR and the i-code before it */

				/*
				 * only handle sequences that start with an
				 * I_PUSH_WR preceded by I_LEA_S/I_LD_WI/I_ADD_WI
				 */
				if (q_ins[scan].ins_code != I_PUSH_WR)
					break;

				if (--scan < 0)
					scan += Q_SIZE;
				if ((prev = scan - 1) < 0)
					prev += Q_SIZE;

				if ((q_ins[scan].ins_code != I_LD_WI) &&
				    (q_ins[scan].ins_code != I_LEA_S) &&
				    (q_ins[scan].ins_code != I_ADD_WI))
					break;

				if ((q_ins[scan].ins_code == I_ADD_WI) &&
				    (q_ins[scan].ins_type != T_SYMBOL ||
				     !is_small_array((SYMBOL *)q_ins[scan].ins_data)))
					break;

				/* change __st.wpt into __st.{w/u}m */
				if (q_ins[scan].ins_code == I_LD_WI) {
					q_ins[q_wr] = q_ins[scan];
					if (old_code == I_ST_WPT)
						q_ins[q_wr].ins_code = I_ST_WM;
					else
						q_ins[q_wr].ins_code = I_ST_UM;
				} else

				/* change __st.wpt into __st.{w/u}s */
				if (q_ins[scan].ins_code == I_LEA_S) {
					q_ins[q_wr] = q_ins[scan];
					if (old_code == I_ST_WPT)
						q_ins[q_wr].ins_code = X_ST_WS;
					else
						q_ins[q_wr].ins_code = X_ST_US;
				} else

				/* change __st.wpt into __st.{w/u}at */
				if ((q_ins[scan].ins_code == I_ADD_WI) &&
				    (q_ins[scan].ins_type == T_SYMBOL) &&
				    (is_small_array((SYMBOL *)q_ins[scan].ins_data))) {
					int push = X_INDEX_UR;
					int code = X_ST_UAT;
					int test;

					/* assume that we'll replace the I_PUSH_WR with X_INDEX_WR or X_INDEX_UR */
					--from;
					--drop;
					++copy;

					/* make sure that an I_ST_WPT index has an I_ASL_WR */
					if (old_code == I_ST_WPT) {
						if (copy == q_nb || q_ins[prev].ins_code != I_ASL_WR)
							break;
						push = X_INDEX_WR;
						code = X_ST_WAT;
						++drop;
						if (--prev < 0)
							prev += Q_SIZE;
					}

					/* check after the I_PUSH_WR for an I_ST_WM and I_LD_WP/I_LD_BP/I_LD_UP */
					/* which are generated for a "+=", "-=", "*=", "/=", etc, etc */
					test = scan + 2;
					if (test >= Q_SIZE)
						test -= Q_SIZE;
					if
					((q_ins[test].ins_code == I_ST_WM) &&
					 (q_ins[test].ins_type == T_PTR)
					) {
						if (++test >= Q_SIZE)
							test -= Q_SIZE;

						switch (q_ins[test].ins_code) {
						case I_LD_WP: push = X_LDP_WAR; break;
						case I_LD_BP: push = X_LDP_BAR; break;
						case I_LD_UP: push = X_LDP_UAR; break;
						default: push = 0; break; /* unexpected code, don't reschedule it! */
						}
						if (push == 0)
							break;

						/* drop the I_ST_M and I_LD_WP/I_LD_BP/I_LD_UP i-codes */
						from += 2;
						drop += 2;
						copy -= 2;
					}

					/* push the index that preceded I_ADD_WI */
					if (from >= Q_SIZE)
						from -= Q_SIZE;
					q_ins[from].ins_code = push;
					q_ins[from].ins_type = T_SYMBOL;
					q_ins[from].ins_data = q_ins[scan].ins_data;

					/* use symbol from the I_ADD_WI */
					q_ins[q_wr].ins_code = code;
					q_ins[q_wr].ins_type = T_SYMBOL;
					q_ins[q_wr].ins_data = q_ins[scan].ins_data;
				} else

					/* this is something that we can't reschedule */
					break;

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

				/* reordering completed, begin again */
				goto lv1_loop;
			}
		}

		/*
		 * this covers a bunch of math where the lval is pushed and the rval is
		 * not a simple value/variable that would be optimized by earlier rules
		 *
		 *  __ld.wi			i	-->	...
		 *  __push.wr					__add.wi	i
		 *    ...
		 *  __add.wt
		 *
		 *  __ld.{w/u}m			symbol	-->	...
		 *  __push.wr					__add.{w/u}m	symbol
		 *    ...
		 *  __add.wt
		 *
		 *  __ld.{w/u}s			symbol	-->	...
		 *  __push.wr					__add.{w/u}s	symbol
		 *    ...
		 *  __add.wt
		 *
		 *  __ld.{w/u}ar		array	-->	__index_{w/u}r	array
		 *  __push.wr					...
		 *    ...					__add.{w/u}at	array
		 *  __add.wt
		 *
		 *  __ldx.{w/b/u}{m/s}q		symbol	-->	...
		 *  __ld.{w/u}ax		array	-->	__ldx.{w/b/u}{m/s} symbol
		 *  __push.wr					__add.{w/u}ax	array
		 *    ...
		 *  __add.wt
		 */

		else if (q_nb > 1 &&
		(old_code == X_ISUB_WT ||
		 old_code == I_ADD_WT ||
		 old_code == I_SUB_WT ||
		 old_code == I_AND_WT ||
		 old_code == I_EOR_WT ||
		 old_code == I_OR_WT ||
		 old_code == I_MUL_WT)
		) {
			/* scan backwards to find the matching I_PUSH_WR */
			for (offset = 2, copy = 1, scan = q_wr; copy < q_nb; copy++) {
				/* check instruction */
				if (--scan < 0)
					scan += Q_SIZE;

				if (icode_flags[q_ins[scan].ins_code] & IS_PUSHWT)
					offset -= 2;
				else
				if (icode_flags[q_ins[scan].ins_code] & IS_POPWT)
					offset += 2;

				/* check offset */
				if (offset != 0)
					continue;

				/* there should be at least one instruction between I_PUSH_WR and I_ADD_WT */
				if (copy == 1)
					break;

				/* don't reorder a scaled pointer addition */
				if ((prev = q_wr - 1) < 0)
					prev += Q_SIZE;
				if (old_code == I_ADD_WT &&
				    q_ins[prev].ins_code == I_DOUBLE_WT)
					break;

				/* found the I_PUSH_WR that matches the I_ADD_WT */
				from = scan + 1;  /* begin copying after the I_PUSH_WR */
				drop = 2;  /* drop I_PUSH_WR and the i-code before it */

				/* check for I_PUSH_WR preceded by loading from a variable */
				if (q_ins[scan].ins_code != I_PUSH_WR)
					break;

				if (--scan < 0)
					scan += Q_SIZE;
				if ((prev = scan - 1) < 0)
					prev += Q_SIZE;

				if (q_ins[scan].ins_code != I_LD_WI &&
				    q_ins[scan].ins_code != I_LD_WM &&
				    q_ins[scan].ins_code != I_LD_UM &&
				    q_ins[scan].ins_code != X_LD_WS &&
				    q_ins[scan].ins_code != X_LD_US &&
				    q_ins[scan].ins_code != X_LD_WAR &&
				    q_ins[scan].ins_code != X_LD_UAR &&
				    q_ins[scan].ins_code != X_LD_WAX &&
				    q_ins[scan].ins_code != X_LD_UAX)
					break;

				/* it is only worth reordering a multiply with an integer */
				if (old_code == I_MUL_WT &&
				    q_ins[scan].ins_code != I_LD_WI)
					break;

				/* change __add.wt into __add.{w/u}{i/m/s/war/uar/uax} */
				if (q_ins[scan].ins_code == I_LD_WI) {
					q_ins[q_wr] = q_ins[scan];
					switch (old_code) {
					case X_ISUB_WT: q_ins[q_wr].ins_code =  I_SUB_WI; break;
					case  I_ADD_WT: q_ins[q_wr].ins_code =  I_ADD_WI; break;
					case  I_SUB_WT: q_ins[q_wr].ins_code = X_ISUB_WI; break;
					case  I_AND_WT: q_ins[q_wr].ins_code =  X_AND_WI; break;
					case  I_EOR_WT: q_ins[q_wr].ins_code =  X_EOR_WI; break;
					case   I_OR_WT: q_ins[q_wr].ins_code =   X_OR_WI; break;
					case  I_MUL_WT: q_ins[q_wr].ins_code =  I_MUL_WI; break;
					default: break;
					}
				} else

				if (q_ins[scan].ins_code == I_LD_WM) {
					q_ins[q_wr] = q_ins[scan];
					switch (old_code) {
					case X_ISUB_WT: q_ins[q_wr].ins_code =  X_SUB_WM; break;
					case  I_ADD_WT: q_ins[q_wr].ins_code =  X_ADD_WM; break;
					case  I_SUB_WT: q_ins[q_wr].ins_code = X_ISUB_WM; break;
					case  I_AND_WT: q_ins[q_wr].ins_code =  X_AND_WM; break;
					case  I_EOR_WT: q_ins[q_wr].ins_code =  X_EOR_WM; break;
					case   I_OR_WT: q_ins[q_wr].ins_code =   X_OR_WM; break;
					default: break;
					}
				} else

				if (q_ins[scan].ins_code == I_LD_UM) {
					q_ins[q_wr] = q_ins[scan];
					switch (old_code) {
					case X_ISUB_WT: q_ins[q_wr].ins_code =  X_SUB_UM; break;
					case  I_ADD_WT: q_ins[q_wr].ins_code =  X_ADD_UM; break;
					case  I_SUB_WT: q_ins[q_wr].ins_code = X_ISUB_UM; break;
					case  I_AND_WT: q_ins[q_wr].ins_code =  X_AND_UM; break;
					case  I_EOR_WT: q_ins[q_wr].ins_code =  X_EOR_UM; break;
					case   I_OR_WT: q_ins[q_wr].ins_code =   X_OR_UM; break;
					default: break;
					}
				} else

				if (q_ins[scan].ins_code == X_LD_WS) {
					q_ins[q_wr] = q_ins[scan];
					switch (old_code) {
					case X_ISUB_WT: q_ins[q_wr].ins_code =  X_SUB_WS; break;
					case  I_ADD_WT: q_ins[q_wr].ins_code =  X_ADD_WS; break;
					case  I_SUB_WT: q_ins[q_wr].ins_code = X_ISUB_WS; break;
					case  I_AND_WT: q_ins[q_wr].ins_code =  X_AND_WS; break;
					case  I_EOR_WT: q_ins[q_wr].ins_code =  X_EOR_WS; break;
					case   I_OR_WT: q_ins[q_wr].ins_code =   X_OR_WS; break;
					default: break;
					}
				} else

				if (q_ins[scan].ins_code == X_LD_US) {
					q_ins[q_wr] = q_ins[scan];
					switch (old_code) {
					case X_ISUB_WT: q_ins[q_wr].ins_code =  X_SUB_US; break;
					case  I_ADD_WT: q_ins[q_wr].ins_code =  X_ADD_US; break;
					case  I_SUB_WT: q_ins[q_wr].ins_code = X_ISUB_US; break;
					case  I_AND_WT: q_ins[q_wr].ins_code =  X_AND_US; break;
					case  I_EOR_WT: q_ins[q_wr].ins_code =  X_EOR_US; break;
					case   I_OR_WT: q_ins[q_wr].ins_code =   X_OR_US; break;
					default: break;
					}
				} else

				if (q_ins[scan].ins_code == X_LD_WAR) {
					q_ins[scan].ins_code = X_INDEX_WR;
					q_ins[q_wr] = q_ins[scan];
					--drop;  /* keep the X_INDEX_WR as well */
					switch (old_code) {
					case X_ISUB_WT: q_ins[q_wr].ins_code =  X_SUB_WAT; break;
					case  I_ADD_WT: q_ins[q_wr].ins_code =  X_ADD_WAT; break;
					case  I_SUB_WT: q_ins[q_wr].ins_code = X_ISUB_WAT; break;
					case  I_AND_WT: q_ins[q_wr].ins_code =  X_AND_WAT; break;
					case  I_EOR_WT: q_ins[q_wr].ins_code =  X_EOR_WAT; break;
					case   I_OR_WT: q_ins[q_wr].ins_code =   X_OR_WAT; break;
					default: break;
					}
				} else

				if (q_ins[scan].ins_code == X_LD_UAR) {
					q_ins[scan].ins_code = X_INDEX_UR;
					q_ins[q_wr] = q_ins[scan];
					--drop;  /* keep the X_INDEX_UR as well */
					switch (old_code) {
					case X_ISUB_WT: q_ins[q_wr].ins_code =  X_SUB_UAT; break;
					case  I_ADD_WT: q_ins[q_wr].ins_code =  X_ADD_UAT; break;
					case  I_SUB_WT: q_ins[q_wr].ins_code = X_ISUB_UAT; break;
					case  I_AND_WT: q_ins[q_wr].ins_code =  X_AND_UAT; break;
					case  I_EOR_WT: q_ins[q_wr].ins_code =  X_EOR_UAT; break;
					case   I_OR_WT: q_ins[q_wr].ins_code =   X_OR_UAT; break;
					default: break;
					}
				} else

				if (q_ins[scan].ins_code == X_LD_WAX) {
					q_ins[q_wr] = q_ins[prev];
					parked = q_ins[scan];
					++drop;  /* drop the X_LD2X_{W/B/U}{M/S} as well */
					switch (q_ins[q_wr].ins_code) {
					case X_LD2X_WMQ: q_ins[q_wr].ins_code = X_LD2X_WM; break;
					case X_LD2X_BMQ: q_ins[q_wr].ins_code = X_LD2X_BM; break;
					case X_LD2X_UMQ: q_ins[q_wr].ins_code = X_LD2X_UM; break;
					case X_LD2X_WSQ: q_ins[q_wr].ins_code = X_LD2X_WS; break;
					case X_LD2X_BSQ: q_ins[q_wr].ins_code = X_LD2X_BS; break;
					case X_LD2X_USQ: q_ins[q_wr].ins_code = X_LD2X_US; break;
					default: break;
					}
					switch (old_code) {
					case X_ISUB_WT: parked.ins_code =  X_SUB_WAX; break;
					case  I_ADD_WT: parked.ins_code =  X_ADD_WAX; break;
					case  I_SUB_WT: parked.ins_code = X_ISUB_WAX; break;
					case  I_AND_WT: parked.ins_code =  X_AND_WAX; break;
					case  I_EOR_WT: parked.ins_code =  X_EOR_WAX; break;
					case   I_OR_WT: parked.ins_code =   X_OR_WAX; break;
					default: break;
					}
				} else

				if (q_ins[scan].ins_code == X_LD_UAX) {
					q_ins[q_wr] = q_ins[prev];
					parked = q_ins[scan];
					++drop;  /* drop the X_LDX_{W/B/U}{M/S} as well */
					switch (old_code) {
					case X_ISUB_WT: parked.ins_code =  X_SUB_UAX; break;
					case  I_ADD_WT: parked.ins_code =  X_ADD_UAX; break;
					case  I_SUB_WT: parked.ins_code = X_ISUB_UAX; break;
					case  I_AND_WT: parked.ins_code =  X_AND_UAX; break;
					case  I_EOR_WT: parked.ins_code =  X_EOR_UAX; break;
					case   I_OR_WT: parked.ins_code =   X_OR_UAX; break;
					default: break;
					}
				} else

					/* this is something that we can't reschedule */
					break;

				/* remove all the instructions ... */
				q_nb -= (drop + copy);
				q_wr -= (drop + copy);
				if (q_wr < 0)
					q_wr += Q_SIZE;

				/* ... then re-insert them in the queue (for further optimizations) */
				for (; copy > 0; copy--) {
					if (from >= Q_SIZE)
						from -= Q_SIZE;
#ifdef DEBUG_OPTIMIZER
					printf("\nReinserting after rescheduling ...");
#endif
					push_ins(&q_ins[from++]);
				}

				if (parked.ins_code)
					push_ins(&parked);

				/* reordering completed, begin again */
				goto lv1_loop;
			}
		}

		/*
		 * this covers storing to global and static arrays with "+=", "-=", etc ...
		 *
		 * this covers a bunch of math where the lval is pushed and the rval is
		 * not a simple value/variable that would be optimized by earlier rules
		 *
		 *  __ld.{w/b/u}{m/s}q	symbol	-->	  ...
		 *  __index.wr		array		__ldx.{w/b/u}{m/s}	symbol
		 *    ...				__st.{w/u}ax		array
		 *  __st.{w/u}at	array
		 *
		 *  __ldp.{w/u}ar	array	-->	__index.{w/u}r		array
		 *  __push.wr				  ...
		 *    ...				__add.{w/u}at		array
		 *  __add.wt				__st.{w/u}ax		array
		 *  __st.{w/u}at	array
		 *
		 *  __ldx.{w/b/u}{m/s}q	symbol	-->	  ...
		 *  __ldp.{w/u}ax	array	-->	__ldx.{w/b/u}{m/s}	symbol
		 *  __push.wr				__add.{w/u}ax		array
		 *    ...				__st.{w/u}ax		array
		 *  __add.wt
		 *  __st.{w/u}at	array
		 */

		else if (q_nb > 1 &&
		(old_code == X_ST_WAT ||
		 old_code == X_ST_UAT)
		) {
			/* scan backwards to find the matching I_PUSH_WR */
			for (offset = 2, copy = 1, scan = q_wr; copy < q_nb; copy++) {
				/* check instruction */
				if (--scan < 0)
					scan += Q_SIZE;

				if (icode_flags[q_ins[scan].ins_code] & IS_PUSHWT)
					offset -= 2;
				else
				if (icode_flags[q_ins[scan].ins_code] & IS_POPWT)
					offset += 2;

				/* check offset */
				if (offset != 0)
					continue;

				/* there should be at least one instruction between X_INDEX_{W/U}R and X_ST_{W/U}AT */
				if (copy == 1)
					break;

				/* change __index_wr into __ld2x.{w/b/u}{m/s} */
				if (q_ins[scan].ins_code == X_INDEX_WR) {
					if ((prev = scan - 1) < 0)
						prev += Q_SIZE;
					switch (q_ins[prev].ins_code) {
					case X_LD_WMQ: index = X_LD2X_WM; break;
					case X_LD_BMQ: index = X_LD2X_BM; break;
					case X_LD_UMQ: index = X_LD2X_UM; break;
					case X_LD_WSQ: index = X_LD2X_WS; break;
					case X_LD_BSQ: index = X_LD2X_BS; break;
					case X_LD_USQ: index = X_LD2X_US; break;
					default: index = 0; break;
					}
					if (index == 0)
						break;
					parked = q_ins[q_wr];
					parked.ins_code = X_ST_WAX;
					q_ins[q_wr] = q_ins[prev];
					q_ins[q_wr].ins_code = index;
					from = scan + 1;
					drop = 2;
				} else

				/* change __index_ur into __ldx.{w/b/u}{m/s}q */
				if (q_ins[scan].ins_code == X_INDEX_UR) {
					if ((prev = scan - 1) < 0)
						prev += Q_SIZE;
					switch (q_ins[prev].ins_code) {
					case X_LD_WMQ: index = X_LDX_WMQ; break;
					case X_LD_BMQ: index = X_LDX_BMQ; break;
					case X_LD_UMQ: index = X_LDX_UMQ; break;
					case X_LD_WSQ: index = X_LDX_WS; break;
					case X_LD_BSQ: index = X_LDX_BS; break;
					case X_LD_USQ: index = X_LDX_US; break;
					default: index = 0; break;
					}
					if (index == 0)
						break;
					parked = q_ins[q_wr];
					parked.ins_code = X_ST_UAX;
					q_ins[q_wr] = q_ins[prev];
					q_ins[q_wr].ins_code = index;
					from = scan + 1;
					drop = 2;
				} else

				/* change __ldp.war into __index.wr */
				if (q_ins[scan].ins_code == X_LDP_WAR) {
					if ((next = scan + 1) >= Q_SIZE)
						next -= Q_SIZE;
					if (q_ins[next].ins_code != I_PUSH_WR)
						break;
					if ((prev = q_wr - 1) < 0)
						prev += Q_SIZE;
					switch (q_ins[prev].ins_code) {
					case X_ISUB_WT: operation =  X_SUB_WAT; break;
					case  I_ADD_WT: operation =  X_ADD_WAT; break;
					case  I_SUB_WT: operation = X_ISUB_WAT; break;
					case  I_AND_WT: operation =  X_AND_WAT; break;
					case  I_EOR_WT: operation =  X_EOR_WAT; break;
					case   I_OR_WT: operation =   X_OR_WAT; break;
					default: operation = 0; break;
					}
					if (operation == 0)
						break;
					q_ins[next] = q_ins[scan];
					q_ins[next].ins_code = X_INDEX_WR;
					q_ins[prev] = q_ins[scan];
					q_ins[prev].ins_code = operation;
					q_ins[q_wr].ins_code = X_ST_WAX;
					from = scan + 1;
					drop = 1;
				} else

				/* change __ldp.uar into __index.ur */
				if (q_ins[scan].ins_code == X_LDP_UAR) {
					if ((next = scan + 1) >= Q_SIZE)
						next -= Q_SIZE;
					if (q_ins[next].ins_code != I_PUSH_WR)
						break;
					if ((prev = q_wr - 1) < 0)
						prev += Q_SIZE;
					switch (q_ins[prev].ins_code) {
					case X_ISUB_WT: operation =  X_SUB_UAT; break;
					case  I_ADD_WT: operation =  X_ADD_UAT; break;
					case  I_SUB_WT: operation = X_ISUB_UAT; break;
					case  I_AND_WT: operation =  X_AND_UAT; break;
					case  I_EOR_WT: operation =  X_EOR_UAT; break;
					case   I_OR_WT: operation =   X_OR_UAT; break;
					default: operation = 0; break;
					}
					if (operation == 0)
						break;
					q_ins[next] = q_ins[scan];
					q_ins[next].ins_code = X_INDEX_UR;
					q_ins[prev] = q_ins[scan];
					q_ins[prev].ins_code = operation;
					q_ins[q_wr].ins_code = X_ST_UAX;
					from = scan + 1;
					drop = 1;
				} else

				/* change __ldp.wax into __add.wax */
				if (q_ins[scan].ins_code == X_LDP_WAX) {
					if ((next = scan + 1) >= Q_SIZE)
						next -= Q_SIZE;
					if (q_ins[next].ins_code != I_PUSH_WR)
						break;
					if ((next = scan - 1) < 0)
						next += Q_SIZE;
					switch (q_ins[next].ins_code) {
					case X_LD2X_WMQ: index = X_LD2X_WM; break;
					case X_LD2X_BMQ: index = X_LD2X_BM; break;
					case X_LD2X_UMQ: index = X_LD2X_UM; break;
					case X_LD2X_WSQ: index = X_LD2X_WS; break;
					case X_LD2X_BSQ: index = X_LD2X_BS; break;
					case X_LD2X_USQ: index = X_LD2X_US; break;
					default: index = 0; break;
					}
					if (index == 0)
						break;
					if ((prev = q_wr - 1) < 0)
						prev += Q_SIZE;
					switch (q_ins[prev].ins_code) {
					case X_ISUB_WT: operation =  X_SUB_WAX; break;
					case  I_ADD_WT: operation =  X_ADD_WAX; break;
					case  I_SUB_WT: operation = X_ISUB_WAX; break;
					case  I_AND_WT: operation =  X_AND_WAX; break;
					case  I_EOR_WT: operation =  X_EOR_WAX; break;
					case   I_OR_WT: operation =   X_OR_WAX; break;
					default: operation = 0; break;
					}
					if (operation == 0)
						break;
					parked = q_ins[q_wr];
					parked.ins_code = X_ST_WAX;
					q_ins[prev] = q_ins[next];
					q_ins[prev].ins_code = index;
					q_ins[q_wr].ins_code = operation;
					from = scan + 2;
					copy = copy - 1;
					drop = 3;
				} else

				/* change __ldp.uax into __add.uax */
				if (q_ins[scan].ins_code == X_LDP_UAX) {
					if ((next = scan + 1) >= Q_SIZE)
						next -= Q_SIZE;
					if (q_ins[next].ins_code != I_PUSH_WR)
						break;
					if ((next = scan - 1) < 0)
						next += Q_SIZE;
					switch (q_ins[next].ins_code) {
					case X_LDX_WMQ: index = X_LDX_WMQ; break;
					case X_LDX_BMQ: index = X_LDX_BMQ; break;
					case X_LDX_UMQ: index = X_LDX_UMQ; break;
					case X_LDX_WSQ: index = X_LDX_WS; break;
					case X_LDX_BSQ: index = X_LDX_BS; break;
					case X_LDX_USQ: index = X_LDX_US; break;
					default: index = 0; break;
					}
					if (index == 0)
						break;
					if ((prev = q_wr - 1) < 0)
						prev += Q_SIZE;
					switch (q_ins[prev].ins_code) {
					case X_ISUB_WT: operation =  X_SUB_UAX; break;
					case  I_ADD_WT: operation =  X_ADD_UAX; break;
					case  I_SUB_WT: operation = X_ISUB_UAX; break;
					case  I_AND_WT: operation =  X_AND_UAX; break;
					case  I_EOR_WT: operation =  X_EOR_UAX; break;
					case   I_OR_WT: operation =   X_OR_UAX; break;
					default: operation = 0; break;
					}
					if (operation == 0)
						break;
					parked = q_ins[q_wr];
					parked.ins_code = X_ST_UAX;
					q_ins[prev] = q_ins[next];
					q_ins[prev].ins_code = index;
					q_ins[q_wr].ins_code = operation;
					from = scan + 2;
					copy = copy - 1;
					drop = 3;
				} else

					/* this is something that we can't reschedule */
					break;

				/* remove all the instructions ... */
				q_nb -= (drop + copy);
				q_wr -= (drop + copy);
				if (q_wr < 0)
					q_wr += Q_SIZE;

				/* ... then re-insert them in the queue (for further optimizations) */
				for (; copy > 0; copy--) {
					if (from >= Q_SIZE)
						from -= Q_SIZE;
#ifdef DEBUG_OPTIMIZER
					printf("\nReinserting after rescheduling ...");
#endif
					push_ins(&q_ins[from++]);
				}

				if (parked.ins_code)
					push_ins(&parked);

				/* reordering completed, begin again */
				goto lv1_loop;
			}
		}

		/*
		 * optimization level 2b - after the instruction re-scheduler
		 */

lv2_loop:
		/* remove instructions from queue but preserve comments */
		if (remove) {
			q_nb -= remove;
			i = q_wr;
			while (remove) {
				if (q_ins[i].ins_type != I_INFO)
					--remove;
				if ((--i) < 0)
					i += Q_SIZE;
			}
			j = i;
			do {
				if ((++j) >= Q_SIZE)
					j -= Q_SIZE;
				if (q_ins[j].ins_type == I_INFO) {
					if ((++i) >= Q_SIZE)
						i -= Q_SIZE;
					memcpy(&q_ins[i], &q_ins[j], sizeof(INS));
				}
			} while (j != q_wr);
			q_wr = i;
		}

		/* precalculate pointers to instructions */
		p_nb = 0;
		i = q_nb;
		j = q_wr;
		while (i != 0 && p_nb < 3) {
			if (q_ins[j].ins_type != I_INFO) {
				p[p_nb++] = &q_ins[j];
			}
			--i;
			--j;
			if ((--j) < 0)
				j += Q_SIZE;
		}
		remove = 0;

		if (p_nb >= 3) {
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
				p[2]->ins_code = p[0]->ins_code == I_ST_WPT ? X_ST_WPI : X_ST_UPI;
				p[2]->ins_data = p[1]->ins_data;
				remove = 2;
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
				if (p[0]->ins_code == I_ST_UPT)
					p[0]->ins_code = X_ST_UP;
				else
					p[0]->ins_code = X_ST_WP;
				q_ins[q_wr].ins_type = T_PTR;
			}
#endif

			/* remove instructions from queue and begin again */
			if (remove)
				goto lv2_loop;
		}
	}
}

/* ----
 * try_swap_order()
 * ----
 * swap the lval and the rval, if that would allow an optimization
 *
 */
void try_swap_order (int linst, int lseqn, INS *operation)
{
	/* is the lval still in the peephole instruction queue? */
	if (q_ins[linst].ins_code != I_RETIRED && q_ins[linst].sequence == lseqn) {
		INS parked[2];
		int from, copy = 0;
		int lprev = linst - 1;
		if (lprev < 0)
			lprev += Q_SIZE;

#ifdef INFORM_VALUE_SWAP
		printf("Stacked operator: ");
		dump_ins(&q_ins[q_wr]);
		printf("with LVAL: ");
		dump_ins(&q_ins[linst]);
		printf("File \"%s\", Line %d\n", (inclsp) ? inclstk_name[inclsp - 1] : fname_copy, line_number);
		printf("%s\n\n", line);
#endif
		if
		((q_ins[linst].ins_code == I_LD_WI) ||
		 (q_ins[linst].ins_code == I_LD_WM) ||
		 (q_ins[linst].ins_code == I_LD_UM) ||
		 (q_ins[linst].ins_code == X_LD_WS) ||
		 (q_ins[linst].ins_code == X_LD_US)
		) {
			/* preserve the lval instructions */
			parked[0] = q_ins[linst];
			parked[1].ins_code = I_RETIRED;
			/* remove both the lval and rval instructions */
			copy = q_wr - linst;
			if (copy++ < 0)
				copy += Q_SIZE;
			q_nb -= copy;
			q_wr = linst - 1;
			if (q_wr < 0)
				q_wr += Q_SIZE;
			from = linst + 2; /* skip I_LD_WM, I_PUSH_WR */
			copy = copy - 3; /* skip I_LD_WM, I_PUSH_WR and final stacked op */
		}
		else if
		((q_ins[linst].ins_code == X_LD_WAX ||
		  q_ins[linst].ins_code == X_LD_UAX) &&
		 (q_ins[lprev].ins_code == X_LD2X_WMQ ||
		  q_ins[lprev].ins_code == X_LD2X_BMQ ||
		  q_ins[lprev].ins_code == X_LD2X_UMQ ||
		  q_ins[lprev].ins_code == X_LD2X_WSQ ||
		  q_ins[lprev].ins_code == X_LD2X_BSQ ||
		  q_ins[lprev].ins_code == X_LD2X_USQ ||
		  q_ins[lprev].ins_code == X_LDX_WMQ ||
		  q_ins[lprev].ins_code == X_LDX_BMQ ||
		  q_ins[lprev].ins_code == X_LDX_UMQ ||
		  q_ins[lprev].ins_code == X_LDX_WSQ ||
		  q_ins[lprev].ins_code == X_LDX_BSQ ||
		  q_ins[lprev].ins_code == X_LDX_USQ)
		) {
			/* preserve the lval instructions */
			parked[0] = q_ins[lprev];
			parked[1] = q_ins[linst];
			/* remove both the lval and rval instructions */
			copy = q_wr - lprev;
			if (copy++ < 0)
				copy += Q_SIZE;
			q_nb -= copy;
			q_wr = lprev - 1;
			if (q_wr < 0)
				q_wr += Q_SIZE;
			from = lprev + 3; /* skip X_LDX_WMQ, X_LD_WAX, I_PUSH_WR */
			copy = copy - 4; /* skip X_LDX_WMQ, X_LD_WAX, I_PUSH_WR and final stacked op */
		}

		/* reorder the operators */
		if (copy) {
			/* re-insert the rval instructions */
			for (; copy > 0; copy--) {
				if (from >= Q_SIZE)
					from -= Q_SIZE;
#ifdef DEBUG_OPTIMIZER
				printf("\nReinserting after reordering ...");
#endif
				push_ins(&q_ins[from++]);
			}
			/* re-insert the lval instructions */
			gpush();
			push_ins(&parked[0]);
			if (parked[1].ins_code != I_RETIRED)
				push_ins(&parked[1]);
			push_ins(operation);
#ifdef INFORM_VALUE_SWAP
			printf("Reordered operator is: ");
			dump_ins(&q_ins[q_wr]);
			printf("\n\n");
#endif
		}
	}
}

/* ----
 * flush_ins()
 * ----
 * flush instruction queue
 *
 */
void flush_ins (void)
{
	while (q_nb) {
		/* gen code */
		if (arg_stack_flag)
			arg_push_ins(&q_ins[q_rd]);
		else
			gen_code(&q_ins[q_rd]);

		/* advance and wrap queue read pointer */
		--q_nb;
		if (++q_rd == Q_SIZE)
			q_rd = 0;
	}

	/* reset queue */
	q_rd = 0;
	q_wr = Q_SIZE - 1;
	q_nb = 0;
}
