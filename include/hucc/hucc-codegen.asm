; ***************************************************************************
; ***************************************************************************
;
; hucc-codegen.asm
;
; The HuCC compiler translates C code into these macros, it does not directly
; generate HuC6280 instructions.
;
; Based on the original HuC macros created by David Michel and the other HuC
; developers, later modified and improved by Ulrich Hecht.
;
; Modifications copyright John Brandwood 2024.
;
; Distributed under the Boost Software License, Version 1.0.
; (See accompanying file LICENSE_1_0.txt or copy at
;  http://www.boost.org/LICENSE_1_0.txt)
;
; ***************************************************************************
; ***************************************************************************
;
; NAMING SCHEME FOR HuCC MACROS ...
;
;   __function.parameters
;
; {parameters} is a list of alphanumeric specifiers, starting with {size} and
; followed by {where}, followed by {index} if an array, then optional {value}
; and finally ending with optional {suffix}
;
; {size}
;   w : 16-bit signed int (default "int" in HuCC)
;   c : 16-bit unsigned int (a "cardinal" in Pascal terms)
;   b :  8-bit signed char
;   u :  8-bit unsigned char (default "char" in HuCC)
;
; {where} or {index}
;   r : HuCC primary register, made up of the Y:A cpu registers
;   t : top of expression stack
;   p : indirect pointer, usually [__ptr]
;   i : immediate value, i.e. a decimal number
;   m : memory, i.e. C global, static, and "-fno-recursive" variables
;   s : stack, i.e. C function parameters and locals (not "-fno-recursive")
;   a : array, i.e. C global, static, "-fno-recursive" arrays <= 256 bytes
;   x : array index already in the X register
;   y : array index already in the Y register
;
; {value} OPTIONAL
;   i : immediate value, i.e. a decimal number
;   z : zero value
;
; {suffix} OPTIONAL
;   q : quick, used for optimized math on only 8-bit values, because all math
;       is normally promoted to "int" size in C; and when optimized stores do
;       not need to preserve the primary register contents
;
; ***************************************************************************
; ***************************************************************************



		.nolist

; ***************************************************************************
; ***************************************************************************
; i-code that retires the primary register contents
; ***************************************************************************
; ***************************************************************************

; **************
; this only exists to mark the end of a C statement when the primary register
; contents are no longer needed

__fence		.macro
		.endm



; ***************************************************************************
; ***************************************************************************
; i-code that declares a byte sized primary register
; ***************************************************************************
; ***************************************************************************

; **************
; this exists to mark that the primary register only needs byte accuracy

__short		.macro
		.endm



; ***************************************************************************
; ***************************************************************************
; i-codes for handling farptr
; ***************************************************************************
; ***************************************************************************

;	/* far pointer support funcs */
;	"fastcall farpeekb(farptr __fbank:__fptr)",
;	"fastcall farpeekw(farptr __fbank:__fptr)",
;	"fastcall farmemget(word __bx, farptr __fbank:__fptr, word acc)",
;	/* asm-lib wrappers */

;	case I_FARPTR_I:
;		ot("__farptr_i\t");
;		outsymbol((char *)data);
;		outstr(",");
;		outstr(tmp->arg[0]);
;		outstr(",");
;		outstr(tmp->arg[1]);
;		nl();
;		break;

;	case I_FARPTR_GET:
;		ot("__farptr_get\t");
;		outstr(tmp->arg[0]);
;		outstr(",");
;		outstr(tmp->arg[1]);
;		nl();
;		break;

;	case I_FGETB:
;		ot("__farptr_i\t");
;		outsymbol((char *)data);
;		nl();
;		ol("__fgetb");
;		break;

;	case I_FGETUB:
;		ot("__farptr_i\t");
;		outsymbol((char *)data);
;		nl();
;		ol("__fgetub");
;		break;

;	case I_FGETW:
;		ot("__farptr_i\t");
;		outsymbol((char *)data);
;		nl();
;		ol("  jsr\t_farpeekw.fast");
;		break;

; **************
; __farptr
; this is used for __far parameters to a __fastcall

__farptr	.macro
		lda.l	#$6000 + ($1FFF & (\1))
		sta.l	\3
		lda.h	#$6000 + ($1FFF & (\1))
		sta.h	\3
		lda	#bank(\1)
		sta	\2
		.endm

; **************
; adds 16-bit unsigned offset in Y:A to data address in \1
; if 1-param then Y=bank, __fptr=addr, used for farpeekb() and farpeekw()
; if 3-param then \2=bank, \3=addr

__farptr_i	.macro
	.if	(\# = 3)
		clc
		adc.l	#(\1) & $1FFF
		sta.l	\3
		tya
		adc.h	#(\1) & $1FFF
		tay
		and	#$1F
		ora	#$60
		sta.h	\3
		tya
		ror	a
		lsr	a
		lsr	a
		lsr	a
		lsr	a
		clc
		adc	#bank(\1)
		sta	\2
	.else
		clc
		adc.l	#(\1) & $1FFF
		sta.l	<__fptr
		tya
		adc.h	#(\1) & $1FFF
		tay
		and	#$1F
		ora	#$60
		sta.h	<__fptr
		tya
		ror	a
		lsr	a
		lsr	a
		lsr	a
		lsr	a
		clc
		adc	#bank(\1)
		tay
	.endif
		.endm

; **************
; JCB: I don't know what this is supposed to do!

	.if	0
__farptr_get	.macro
		sta	<\1
		ldy	#2
		lda	[__ptr], y
		sta.l	<\2
		iny
		lda	[__ptr], y
		sta.h	<\2
		.endm
	.endif

; **************
; only executed immediately after a 1-parameter __farptr_i!
; expects Y=bank, __fptr=addr
; N.B. Preserves MPR3 unlike original HuC code.
; **************
; unsigned int __fastcall farpeekw( void far *base<__fbank:__fptr> );
; **************
; _farpeekw.fast is called by HuCC after executing __farptr_i macro.

_farpeekw.1:	ldy	<__fbank
_farpeekw.fast:	tma3
		pha
		tya
		tam3
		lda	[__fptr]
		say
		inc.l	<__fptr
		bne	!+
		inc.h	<__fptr
		bpl	!+
		inc	a
		tam3
		lda	#$60
		sta.h	<__fptr
!:		lda	[__fptr]
		sta	<__temp
		pla
		tam3
		tya
		ldy	<__temp
		rts

; **************
; only executed immediately after a 1-parameter __farptr_i!
; expects Y=bank, __fptr=addr
; N.B. Preserves MPR3 unlike original HuC code.

__fgetb		.macro
		tma3
		pha
		tya
		tam3
		lda	[__fptr]
		tay
		pla
		tam3
		tya
		cly
		bpl	!+
		dey
!:
		.endm

; **************
; only executed immediately after a 1-parameter __farptr_i!
; expects Y=bank, __fptr=addr
; N.B. Preserves MPR3 unlike original HuC code.

__fgetub	.macro
		tma3
		pha
		tya
		tam3
		lda	[__fptr]
		tay
		pla
		tam3
		tya
		cly
		.endm



; ***************************************************************************
; ***************************************************************************
; i-codes for calling functions
; ***************************************************************************
; ***************************************************************************

; **************

__call		.macro
		call	\1
		.endm

; **************

__funcp.wr	.macro
		sta.l	__func
		sty.h	__func
		.endm

; **************

__callp		.macro
		jsr	call_indirect
		.endm

call_indirect:	jmp	[__func]



; ***************************************************************************
; ***************************************************************************
; i-codes for C functions and the C parameter stack
; ***************************************************************************
; ***************************************************************************

; **************
; function prolog

__enter		.macro
		.endm

; **************
; function epilog
; \1 == 0 if no return value

__return	.macro
	.if	(\1 != 0)
		tax
	.endif
		jmp	leave_proc
		.endm

; **************
; main C stack for local variables and arguments

__modsp		.macro	; __STACK
	.if	((\1 >= 0) && (\1 < 65536))
	.if	(\1 == 2)
		inc	<__sp
		inc	<__sp
	.else
		tax
		lda	<__sp
		clc
		adc	#\1
		sta	<__sp
		txa
	.endif
	.else
	.if	(\1 == -2)
		dec	<__sp
		dec	<__sp
	.else
		lda	<__sp
		clc
		adc	#\1
		sta	<__sp
	.endif
	.if	HUCC_DEBUG_SP
!overflow:	bmi	!overflow-
	.endif
	.endif
		.endm

; **************
; main C stack for local variables and arguments
; this is used to adjust the stack in a C "goto"

__modsp_goto	.macro	; __STACK
	.if	(\1 != 0)
		lda.l	<__sp
		clc
		adc	#\1
		sta.l	<__sp
	.endif
		.endm

; **************
; main C stack for local variables and arguments

__pusharg.wr	.macro	; __STACK
		ldx	<__sp
		dex
		dex
	.if	HUCC_DEBUG_SP
!overflow:	bmi	!overflow-
	.endif
		sta.l	<__stack, x
		sty.h	<__stack, x
		stx	<__sp
		.endm

; **************
; use the hardware stack for expressions

__push.wr	.macro
		phy
		pha
		.endm

; **************
; use the hardware stack for expressions

__pop.wr	.macro
		pla
		ply
		.endm

; **************
; use the hardware stack for the temporary storage of spilled arguments

__spush.wr	.macro
		phy
		pha
		.endm

; **************
; use the hardware stack for the temporary storage of spilled arguments

__spop.wr	.macro
		pla
		ply
		.endm

; **************
; use the hardware stack for the temporary storage of spilled arguments

__spush.ur	.macro
		pha
		.endm

; **************
; use the hardware stack for the temporary storage of spilled arguments

__spop.ur	.macro
		pla
		.endm



; ***************************************************************************
; ***************************************************************************
; i-codes for handling boolean tests and branching
; ***************************************************************************
; ***************************************************************************

; **************
; Y:A is the value to check for.
; \1 is the number of numeric cases to test for (3 in this example)

__switch_c.wr	.macro
		ldx	#<(\1 * 2 + 2)
!loop:		dex
		dex
		beq	!found+
		cmp.l	!table+ - 2, x
		bne	!loop-
		say
		cmp.h	!table+ - 2, x
		say
		bne	!loop-
!found:		jmp	[!table+ + \1 * 2, x]
		.endm

; !table:	dw	val3		; +01 x=2
;		dw	val2		; +23 x=4
;		dw	val1		; +45 x=6
;		dw	jmpdefault	; +67 x=0
;		dw	jmp3		; +89 x=2
;		dw	jmp2		; +AB x=4
;		dw	jmp1		; +CD x=6

; **************
; Y:A is the value to check for.
; \1 is the number of numeric cases to test for (3 in this example)

__switch_c.ur	.macro
		ldx	#\1 + 1
!loop:		dex
		beq	!found+
		cmp	!table+ - 1, x
		bne	!loop-
!found:		txa
		asl	a
		tax
		jmp	[!table+ + \1, x]
		.endm

; !table:	db	val3		; +0  x=1
;		db	val2            ; +1  x=2
;		db	val1            ; +2  x=3
;		dw	jmpdefault      ; +34 x=0
;		dw	jmp3            ; +56 x=1
;		dw	jmp2            ; +78 x=2
;		dw	jmp1            ; +9A x=3

; **************
; Y:A is the value to check for.
; \1 is the min case value in the table (must be >= 0)
; \2 is the max case value in the table (must be <= min+126)

__switch_r.wr	.macro
	.if	(\1 >= 0) && (\1 <= 65535)
	.if	(\1 != 0)
		sec
		sbc.l	#\1
		say
		sbc.h	#\1
		bcc	!default+
		bne	!default+
		tya
	.else
		cpy	#0
		bne	!default+
	.endif
	.else
		sec
		sbc.l	#\1
		say
		sbc.h	#\1
		bvc	!+
		eor	#$80
!:		bmi	!default+
		bne	!default+
		tya
	.endif
		cmp	#(\2 - \1) + 1
		bcc	!found+
!default:	lda	#(\2 - \1) + 1
!found:		asl	a
		tax
		jmp	[!table+, x]
		.endm

; !table:	dw	jmp1		; +01 x=\1
;		dw	jmp2		; +23 x=\1+1
;		dw	jmp3		; +45 x=\1+2
;		dw	jmp4		; +67 x=\2
;		dw	jmpdefault	; +89 x=\2+1

; **************
; Y:A is the value to check for.
; \1 is the min case value in the table (must be >= 0)
; \2 is the max case value in the table (must be <= min+126)

__switch_r.ur	.macro
	.if	(\1 >= 0) && (\1 <= 255)
	.if	(\1 != 0)
		sec
		sbc	#\1
		bcc	!default+
	.endif
	.else
		sec
		sbc.l	#\1
		bvc	!+
		eor	#$80
!:		bmi	!default+
	.endif
		cmp	#(\2 - \1) + 1
		bcc	!found+
!default:	lda	#(\2 - \1) + 1
!found:		asl	a
		tax
		jmp	[!table+, x]
		.endm

; !table:	dw	jmp1		; +01 x=\1
;		dw	jmp2		; +23 x=\1+1
;		dw	jmp3		; +45 x=\1+2
;		dw	jmp4		; +67 x=\2
;		dw	jmpdefault	; +89 x=\2+1

; **************
; the start of a "default" statement

__default	.macro
		.endm

; **************
; the start of a "case" statement

__case		.macro
		.endm

; **************
; the end of the previous "case" statement if it drops through

__endcase	.macro
		.endm

; **************
; branch to the end of a statement or to a C label

__bra		.macro
		bra	\1
		.endm

; **************
; always preceeded by a __tst.wr before peephole optimization

__bfalse	.macro
		bcc	\1
		.endm

; **************
; always preceeded by a __tst.wr before peephole optimization

__btrue		.macro
		bcs	\1
		.endm

; **************
; boolean test, always followed by a __tst.wr or __not.wr before peephole optimization
; C is true (1) if stacked-value == Y:A, else false (0)
; this MUST set the C flag for the subsequent branches!

__equ_w.wt	.macro
		tsx
		cmp.l	__tos, x
		bne	!false+
		tya
		cmp.h	__tos, x
		beq	!+
!false:		clc
!:		inx
		inx
		txs
		.endm

; **************
; boolean test, always followed by a __tst.wr or __not.wr before peephole optimization
; C is true (1) if stacked-value != Y:A, else false (0)
; this MUST set the C flag for the subsequent branches!

__neq_w.wt	.macro
		tsx
		sec
		eor.l	__tos, x
		bne	!+
		tya
		eor.h	__tos, x
		bne	!+
		clc
!:		inx
		inx
		txs
		.endm

; **************
; boolean test (signed word), always followed by a __tst.wr or __not.wr before peephole optimization
; C is true (1) if stacked-value < Y:A, else false (0)
; this MUST set the C flag for the susequent branches!

__slt_w.wt	.macro
		tsx
		clc			; Subtract memory+1 from Y:A.
		sbc.l	__tos, x
		tya
		sbc.h	__tos, x
		bvc	!+
		eor	#$80		; +ve if Y:A > memory (signed).
!:		eor	#$80
		asl	a
		inx
		inx
		txs
		.endm

; **************
; boolean test (signed word), always followed by a __tst.wr or __not.wr before peephole optimization
; C is true (1) if stacked-value <= Y:A, else false (0)
; this MUST set the C flag for the susequent branches!

__sle_w.wt	.macro
		tsx
		cmp.l	__tos, x	; Subtract memory from Y:A.
		tya
		sbc.h	__tos, x
		bvc	!+
		eor	#$80		; +ve if Y:A >= memory (signed).
!:		eor	#$80
		asl	a
		inx
		inx
		txs
		.endm

; **************
; boolean test (signed word), always followed by a __tst.wr or __not.wr before peephole optimization
; C is true (1) if stacked-value > Y:A, else false (0)
; this MUST set the C flag for the susequent branches!

__sgt_w.wt	.macro
		tsx
		cmp.l	__tos, x; Subtract memory from Y:A.
		tya
		sbc.h	__tos, x
		bvc	!+
		eor	#$80		; -ve if Y:A < memory (signed).
!:		asl	a
		inx
		inx
		txs
		.endm

; **************
; boolean test (signed word), always followed by a __tst.wr or __not.wr before peephole optimization
; C is true (1) if stacked-value >= Y:A, else false (0)
; this MUST set the C flag for the susequent branches!

__sge_w.wt	.macro
		tsx
		clc			; Subtract memory+1 from Y:A.
		sbc.l	__tos, x
		tya
		sbc.h	__tos, x
		bvc	!+
		eor	#$80		; -ve if Y:A <= memory (signed).
!:		asl	a
		inx
		inx
		txs
		.endm

; **************
; boolean test (unsigned word), always followed by a __tst.wr or __not.wr before peephole optimization
; C is true (1) if stacked-value < Y:A, else false (0)
; this MUST set the C flag for the susequent branches!

__ult_w.wt	.macro
		tsx
		clc			; Subtract memory+1 from Y:A.
		sbc.l	__tos, x
		tya
		sbc.h	__tos, x	; CS if Y:A > memory.
		inx
		inx
		txs
		.endm

; **************
; boolean test (unsigned word), always followed by a __tst.wr or __not.wr before peephole optimization
; C is true (1) if stacked-value <= Y:A, else false (0)
; this MUST set the C flag for the susequent branches!

__ule_w.wt	.macro
		tsx
		cmp.l	__tos, x	; Subtract memory from Y:A.
		tya
		sbc.h	__tos, x	; CS if Y:A >= memory.
		inx
		inx
		txs
		.endm

; **************
; boolean test (unsigned word), always followed by a __tst.wr or __not.wr before peephole optimization
; C is true (1) if stacked-value > Y:A, else false (0)
; this MUST set the C flag for the susequent branches!

__ugt_w.wt	.macro
		tsx
		cmp.l	__tos, x	; Subtract memory from Y:A.
		tya
		sbc.h	__tos, x	; CC if Y:A < memory.
		ror	a
		eor	#$80
		rol	a
		inx
		inx
		txs
		.endm

; **************
; boolean test (unsigned word), always followed by a __tst.wr or __not.wr before peephole optimization
; C is true (1) if stacked-value >= Y:A, else false (0)
; this MUST set the C flag for the susequent branches!

__uge_w.wt	.macro
		tsx
		clc			; Subtract memory+1 from Y:A.
		sbc.l	__tos, x
		tya
		sbc.h	__tos, x	; CC if Y:A <= memory.
		ror	a
		eor	#$80
		rol	a
		inx
		inx
		txs
		.endm

; **************
; optimized boolean test
; C is true (1) if Y:A == integer-value, else false (0)
; this MUST set the C flag for the subsequent branches!

__equ_w.wi	.macro
		cmp.l	#\1
		bne	!false+
		cpy.h	#\1
		beq	!+
!false:		clc
!:
		.endm

; **************
; optimized boolean test
; C is true (1) if Y:A != integer-value, else false (0)
; this MUST set the C flag for the subsequent branches!

__neq_w.wi	.macro
		sec
		eor.l	#\1
		bne	!+
		tya
		eor.h	#\1
		bne	!+
		clc
!:
		.endm

; **************
; optimized boolean test (signed word)
; C is true (1) if Y:A < integer-value, else false (0)
; this MUST set the C flag for the susequent branches!

__slt_w.wi	.macro
		cmp.l	#\1		; Subtract integer from Y:A.
		tya
		sbc.h	#\1
		bvc	!+
		eor	#$80		; -ve if Y:A < integer (signed).
!:		asl	a
		.endm

; **************
; optimized boolean test (signed word)
; C is true (1) if Y:A <= integer-value, else false (0)
; this MUST set the C flag for the susequent branches!

__sle_w.wi	.macro
		clc			; Subtract integer+1 from Y:A.
		sbc.l	#\1
		tya
		sbc.h	#\1
		bvc	!+
		eor	#$80		; -ve if Y:A <= integer (signed).
!:		asl	a
		.endm

; **************
; optimized boolean test (signed word)
; C is true (1) if Y:A > integer-value, else false (0)
; this MUST set the C flag for the susequent branches!

__sgt_w.wi	.macro
		clc			; Subtract integer+1 from Y:A.
		sbc.l	#\1
		tya
		sbc.h	#\1
		bvc	!+
		eor	#$80		; +ve if Y:A > integer (signed).
!:		eor	#$80
		asl	a
		.endm

; **************
; optimized boolean test (signed word)
; C is true (1) if Y:A >= integer-value, else false (0)
; this MUST set the C flag for the susequent branches!

__sge_w.wi	.macro
		cmp.l	#\1		; Subtract integer from Y:A.
		tya
		sbc.h	#\1
		bvc	!+
		eor	#$80		; +ve if Y:A >= integer (signed).
!:		eor	#$80
		asl	a
		.endm

; **************
; optimized boolean test (unsigned word)
; C is true (1) if Y:A < integer-value, else false (0)
; this MUST set the C flag for the susequent branches!

__ult_w.wi	.macro
		cmp.l	#\1		; Subtract integer from Y:A.
		tya
		sbc.h	#\1		; CC if Y:A < integer.
		ror	a
		eor	#$80
		rol	a
		.endm

; **************
; optimized boolean test (unsigned word)
; C is true (1) if Y:A <= integer-value, else false (0)
; this MUST set the C flag for the susequent branches!

__ule_w.wi	.macro
		clc			; Subtract integer+1 from Y:A.
		sbc.l	#\1
		tya
		sbc.h	#\1		; CC if Y:A <= integer.
		ror	a
		eor	#$80
		rol	a
		.endm

; **************
; optimized boolean test (unsigned word)
; C is true (1) if Y:A > integer-value, else false (0)
; this MUST set the C flag for the susequent branches!

__ugt_w.wi	.macro
		clc			; Subtract integer+1 from Y:A.
		sbc.l	#\1
		tya
		sbc.h	#\1		; CS if Y:A > integer.
		.endm

; **************
; optimized boolean test (unsigned word)
; C is true (1) if Y:A >= integer-value, else false (0)
; this MUST set the C flag for the susequent branches!

__uge_w.wi	.macro
		cmp.l	#\1		; Subtract integer from Y:A.
		tya
		sbc.h	#\1		; CS if Y:A >= integer.
		.endm

; **************
; optimized boolean test
; C is true (1) if A == integer-value, else false (0)
; this MUST set the C flag for the subsequent branches!

__equ_b.uiq	.macro
		cmp	#\1
		beq	!+
		clc
!:
		.endm

; **************
; optimized boolean test
; C is true (1) if A != integer-value, else false (0)
; this MUST set the C flag for the subsequent branches!

__neq_b.uiq	.macro
		sec
		eor	#\1
		bne	!+
		clc
!:
		.endm

; **************
; optimized boolean test (signed byte)
; C is true (1) if A < integer-value, else false (0)
; this MUST set the C flag for the susequent branches!

__slt_b.biq	.macro
		sec			; Subtract integer from A.
		sbc	#\1
		bvc	!+
		eor	#$80		; -ve if A < integer (signed).
!:		asl	a
		.endm

; **************
; optimized boolean test (signed byte)
; C is true (1) if A <= integer-value, else false (0)
; this MUST set the C flag for the susequent branches!

__sle_b.biq	.macro
		clc			; Subtract integer+1 from A.
		sbc	#\1
		bvc	!+
		eor	#$80		; -ve if A <= integer (signed).
!:		asl	a
		.endm

; **************
; optimized boolean test (signed byte)
; C is true (1) if A > integer-value, else false (0)
; this MUST set the C flag for the susequent branches!

__sgt_b.biq	.macro
		clc			; Subtract integer+1 from A.
		sbc.l	#\1
		bvc	!+
		eor	#$80		; +ve if A > integer (signed).
!:		eor	#$80
		asl	a
		.endm

; **************
; optimized boolean test (signed byte)
; C is true (1) if A >= integer-value, else false (0)
; this MUST set the C flag for the susequent branches!

__sge_b.biq	.macro
		sec			; Subtract integer from A.
		sbc	#\1
		bvc	!+
		eor	#$80		; +ve if A >= integer (signed).
!:		eor	#$80
		asl	a
		.endm

; **************
; optimized boolean test (unsigned byte)
; C is true (1) if A < integer-value, else false (0)
; this MUST set the C flag for the susequent branches!

__ult_b.uiq	.macro
		cmp	#\1		; Subtract integer from A.
		ror	a		; CC if A < integer.
		eor	#$80
		rol	a
		.endm

; **************
; optimized boolean test (unsigned byte)
; C is true (1) if A <= integer-value, else false (0)
; this MUST set the C flag for the susequent branches!

__ule_b.uiq	.macro
		clc			; Subtract integer+1 from A.
		sbc	#\1
		ror	a		; CC if A <= integer.
		eor	#$80
		rol	a
		.endm

; **************
; optimized boolean test (unsigned byte)
; C is true (1) if A > integer-value, else false (0)
; this MUST set the C flag for the susequent branches!

__ugt_b.uiq	.macro
		clc			; Subtract integer+1 from A.
		sbc	#\1		; CS if A > integer.
		.endm

; **************
; optimized boolean test (unsigned byte)
; C is true (1) if A >= integer-value, else false (0)
; this MUST set the C flag for the susequent branches!

__uge_b.uiq	.macro
		cmp	#\1		; Subtract integer from A.
					; CS if A >= integer.
		.endm

; **************
; optimized boolean test
; C is true (1) if Y:A == memory-value, else false (0)
; this MUST set the C flag for the subsequent branches!

__equ_w.wm	.macro
		cmp.l	\1
		bne	!false+
		cpy.h	\1
		beq	!+
!false:		clc
!:
		.endm

; **************
; optimized boolean test
; C is true (1) if Y:A != memory-value, else false (0)
; this MUST set the C flag for the subsequent branches!

__neq_w.wm	.macro
		sec
		eor.l	\1
		bne	!+
		tya
		eor.h	\1
		bne	!+
		clc
!:
		.endm

; **************
; optimized boolean test (signed word)
; C is true (1) if Y:A < memory-value, else false (0)
; this MUST set the C flag for the susequent branches!

__slt_w.wm	.macro
		cmp.l	\1		; Subtract memory from Y:A.
		tya
		sbc.h	\1
		bvc	!+
		eor	#$80		; -ve if Y:A < memory (signed).
!:		asl	a
		.endm

; **************
; optimized boolean test (signed word)
; C is true (1) if Y:A <= memory-value, else false (0)
; this MUST set the C flag for the susequent branches!

__sle_w.wm	.macro
		clc			; Subtract memory+1 from Y:A.
		sbc.l	\1
		tya
		sbc.h	\1
		bvc	!+
		eor	#$80		; -ve if Y:A <= memory (signed).
!:		asl	a
		.endm

; **************
; optimized boolean test (signed word)
; C is true (1) if Y:A > memory-value, else false (0)
; this MUST set the C flag for the susequent branches!

__sgt_w.wm	.macro
		clc			; Subtract memory+1 from Y:A.
		sbc.l	\1
		tya
		sbc.h	\1
		bvc	!+
		eor	#$80		; +ve if Y:A > memory (signed).
!:		eor	#$80
		asl	a
		.endm

; **************
; optimized boolean test (signed word)
; C is true (1) if Y:A >= memory-value, else false (0)
; this MUST set the C flag for the susequent branches!

__sge_w.wm	.macro
		cmp.l	\1		; Subtract memory from Y:A.
		tya
		sbc.h	\1
		bvc	!+
		eor	#$80		; +ve if Y:A >= memory (signed).
!:		eor	#$80
		asl	a
		.endm

; **************
; optimized boolean test (unsigned word)
; C is true (1) if Y:A < memory-value, else false (0)
; this MUST set the C flag for the susequent branches!

__ult_w.wm	.macro
		cmp.l	\1		; Subtract memory from Y:A.
		tya
		sbc.h	\1		; CC if Y:A < memory.
		ror	a
		eor	#$80
		rol	a
		.endm

; **************
; optimized boolean test (unsigned word)
; C is true (1) if Y:A <= memory-value, else false (0)
; this MUST set the C flag for the susequent branches!

__ule_w.wm	.macro
		clc			; Subtract memory+1 from Y:A.
		sbc.l	\1
		tya
		sbc.h	\1		; CC if Y:A <= memory.
		ror	a
		eor	#$80
		rol	a
		.endm

; **************
; optimized boolean test (unsigned word)
; C is true (1) if Y:A > memory-value, else false (0)
; this MUST set the C flag for the susequent branches!

__ugt_w.wm	.macro
		clc			; Subtract memory+1 from Y:A.
		sbc.l	\1
		tya
		sbc.h	\1		; CS if Y:A > memory.
		.endm

; **************
; optimized boolean test (unsigned word)
; C is true (1) if Y:A >= memory-value, else false (0)
; this MUST set the C flag for the susequent branches!

__uge_w.wm	.macro
		cmp.l	\1		; Subtract memory from Y:A.
		tya
		sbc.h	\1		; CS if Y:A >= memory.
		.endm

; **************
; optimized boolean test
; C is true (1) if Y:A == memory-value, else false (0)
; this MUST set the C flag for the subsequent branches!

__equ_w.um	.macro
		cmp	\1
		bne	!false+
		tya
		beq	!+
!false:		clc
!:
		.endm

; **************
; optimized boolean test
; C is true (1) if Y:A != memory-value, else false (0)
; this MUST set the C flag for the subsequent branches!

__neq_w.um	.macro
		sec
		eor	\1
		bne	!+
		tya
		bne	!+
		clc
!:
		.endm

; **************
; optimized boolean test (signed word)
; C is true (1) if Y:A < memory-value, else false (0)
; this MUST set the C flag for the susequent branches!

__slt_w.um	.macro
		cmp	\1		; Subtract memory from Y:A.
		tya
		sbc	#0
		bvc	!+
		eor	#$80		; -ve if Y:A < memory (signed).
!:		asl	a
		.endm

; **************
; optimized boolean test (signed word)
; C is true (1) if Y:A <= memory-value, else false (0)
; this MUST set the C flag for the susequent branches!

__sle_w.um	.macro
		clc			; Subtract memory+1 from Y:A.
		sbc	\1
		tya
		sbc	#0
		bvc	!+
		eor	#$80		; -ve if Y:A <= memory (signed).
!:		asl	a
		.endm

; **************
; optimized boolean test (signed word)
; C is true (1) if Y:A > memory-value, else false (0)
; this MUST set the C flag for the susequent branches!

__sgt_w.um	.macro
		clc			; Subtract memory+1 from Y:A.
		sbc	\1
		tya
		sbc	#0
		bvc	!+
		eor	#$80		; +ve if Y:A > memory (signed).
!:		eor	#$80
		asl	a
		.endm

; **************
; optimized boolean test (signed word)
; C is true (1) if Y:A >= memory-value, else false (0)
; this MUST set the C flag for the susequent branches!

__sge_w.um	.macro
		cmp	\1		; Subtract memory from Y:A.
		tya
		sbc	#0
		bvc	!+
		eor	#$80		; +ve if Y:A >= memory (signed).
!:		eor	#$80
		asl	a
		.endm

; **************
; optimized boolean test (unsigned word)
; C is true (1) if Y:A < memory-value, else false (0)
; this MUST set the C flag for the susequent branches!

__ult_w.um	.macro
		cmp	\1		; Subtract memory from Y:A.
		tya
		sbc	#0		; CC if Y:A < memory.
		ror	a
		eor	#$80
		rol	a
		.endm

; **************
; optimized boolean test (unsigned word)
; C is true (1) if Y:A <= memory-value, else false (0)
; this MUST set the C flag for the susequent branches!

__ule_w.um	.macro
		clc			; Subtract memory+1 from Y:A.
		sbc	\1
		tya
		sbc	#0		; CC if Y:A <= memory.
		ror	a
		eor	#$80
		rol	a
		.endm

; **************
; optimized boolean test (unsigned word)
; C is true (1) if Y:A > memory-value, else false (0)
; this MUST set the C flag for the susequent branches!

__ugt_w.um	.macro
		clc			; Subtract memory+1 from Y:A.
		sbc	\1
		tya
		sbc	#0		; CS if Y:A > memory.
		.endm

; **************
; optimized boolean test (unsigned word)
; C is true (1) if Y:A >= memory-value, else false (0)
; this MUST set the C flag for the susequent branches!

__uge_w.um	.macro
		cmp	\1		; Subtract memory from Y:A.
		tya
		sbc	#0		; CS if Y:A >= memory.
		.endm

; **************
; optimized boolean test
; C is true (1) if A == memory-value, else false (0)
; this MUST set the C flag for the subsequent branches!

__equ_b.umq	.macro
		cmp	\1
		beq	!+
		clc
!:
		.endm

; **************
; optimized boolean test
; C is true (1) if A != memory-value, else false (0)
; this MUST set the C flag for the subsequent branches!

__neq_b.umq	.macro
		sec
		eor	\1
		bne	!+
		clc
!:
		.endm

; **************
; optimized boolean test (signed byte)
; C is true (1) if A < memory-value, else false (0)
; this MUST set the C flag for the susequent branches!

__slt_b.bmq	.macro
		sec			; Subtract memory from A.
		sbc	\1
		bvc	!+
		eor	#$80		; -ve if A < memory (signed).
!:		asl	a
		.endm

; **************
; optimized boolean test (signed byte)
; C is true (1) if A <= memory-value, else false (0)
; this MUST set the C flag for the susequent branches!

__sle_b.bmq	.macro
		clc			; Subtract memory+1 from A.
		sbc	\1
		bvc	!+
		eor	#$80		; -ve if A <= memory (signed).
!:		asl	a
		.endm

; **************
; optimized boolean test (signed byte)
; C is true (1) if A > memory-value, else false (0)
; this MUST set the C flag for the susequent branches!

__sgt_b.bmq	.macro
		clc			; Subtract memory+1 from A.
		sbc.l	\1
		bvc	!+
		eor	#$80		; +ve if A > memory (signed).
!:		eor	#$80
		asl	a
		.endm

; **************
; optimized boolean test (signed byte)
; C is true (1) if A >= memory-value, else false (0)
; this MUST set the C flag for the susequent branches!

__sge_b.bmq	.macro
		sec			; Subtract memory from A.
		sbc	\1
		bvc	!+
		eor	#$80		; +ve if A >= memory (signed).
!:		eor	#$80
		asl	a
		.endm

; **************
; optimized boolean test (unsigned byte)
; C is true (1) if A < memory-value, else false (0)
; this MUST set the C flag for the susequent branches!

__ult_b.umq	.macro
		cmp	\1		; Subtract memory from A.
		ror	a		; CC if A < memory.
		eor	#$80
		rol	a
		.endm

; **************
; optimized boolean test (unsigned byte)
; C is true (1) if A <= memory-value, else false (0)
; this MUST set the C flag for the susequent branches!

__ule_b.umq	.macro
		clc			; Subtract memory+1 from A.
		sbc	\1
		ror	a		; CC if A <= memory.
		eor	#$80
		rol	a
		.endm

; **************
; optimized boolean test (unsigned byte)
; C is true (1) if A > memory-value, else false (0)
; this MUST set the C flag for the susequent branches!

__ugt_b.umq	.macro
		clc			; Subtract memory+1 from A.
		sbc	\1		; CS if A > memory.
		.endm

; **************
; optimized boolean test (unsigned byte)
; C is true (1) if A >= memory-value, else false (0)
; this MUST set the C flag for the susequent branches!

__uge_b.umq	.macro
		cmp	\1		; Subtract memory from A.
					; CS if A >= memory.
		.endm

; **************
; optimized boolean test
; C is true (1) if Y:A == memory-value, else false (0)
; this MUST set the C flag for the subsequent branches!

__equ_w.ws	.macro	; __STACK
		ldx	<__sp
		cmp.l	<__stack + \1, x
		bne	!false+
		tya
		cmp.h	<__stack + \1, x
		beq	!+
!false:		clc
!:
		.endm

; **************
; optimized boolean test
; C is true (1) if Y:A != memory-value, else false (0)
; this MUST set the C flag for the subsequent branches!

__neq_w.ws	.macro	; __STACK
		ldx	<__sp
		sec
		eor.l	<__stack + \1, x
		bne	!+
		tya
		eor.h	<__stack + \1, x
		bne	!+
		clc
!:
		.endm

; **************
; optimized boolean test (signed word)
; C is true (1) if Y:A < memory-value, else false (0)
; this MUST set the C flag for the susequent branches!

__slt_w.ws	.macro	; __STACK
		ldx	<__sp
		cmp.l	<__stack + \1, x; Subtract memory from Y:A.
		tya
		sbc.h	<__stack + \1, x
		bvc	!+
		eor	#$80		; -ve if Y:A < memory (signed).
!:		asl	a
		.endm

; **************
; optimized boolean test (signed word)
; C is true (1) if Y:A <= memory-value, else false (0)
; this MUST set the C flag for the susequent branches!

__sle_w.ws	.macro	; __STACK
		ldx	<__sp
		clc			; Subtract memory+1 from Y:A.
		sbc.l	<__stack + \1, x
		tya
		sbc.h	<__stack + \1, x
		bvc	!+
		eor	#$80		; -ve if Y:A <= memory (signed).
!:		asl	a
		.endm

; **************
; optimized boolean test (signed word)
; C is true (1) if Y:A > memory-value, else false (0)
; this MUST set the C flag for the susequent branches!

__sgt_w.ws	.macro	; __STACK
		ldx	<__sp
		clc			; Subtract memory+1 from Y:A.
		sbc.l	<__stack + \1, x
		tya
		sbc.h	<__stack + \1, x
		bvc	!+
		eor	#$80		; +ve if Y:A > memory (signed).
!:		eor	#$80
		asl	a
		.endm

; **************
; optimized boolean test (signed word)
; C is true (1) if Y:A >= memory-value, else false (0)
; this MUST set the C flag for the susequent branches!

__sge_w.ws	.macro	; __STACK
		ldx	<__sp
		cmp.l	<__stack + \1, x; Subtract memory from Y:A.
		tya
		sbc.h	<__stack + \1, x
		bvc	!+
		eor	#$80		; +ve if Y:A >= memory (signed).
!:		eor	#$80
		asl	a
		.endm

; **************
; optimized boolean test (unsigned word)
; C is true (1) if Y:A < memory-value, else false (0)
; this MUST set the C flag for the susequent branches!

__ult_w.ws	.macro	; __STACK
		ldx	<__sp
		cmp.l	<__stack + \1, x; Subtract memory from Y:A.
		tya
		sbc.h	<__stack + \1, x; CC if Y:A < memory.
		ror	a
		eor	#$80
		rol	a
		.endm

; **************
; optimized boolean test (unsigned word)
; C is true (1) if Y:A <= memory-value, else false (0)
; this MUST set the C flag for the susequent branches!

__ule_w.ws	.macro	; __STACK
		ldx	<__sp
		clc			; Subtract memory+1 from Y:A.
		sbc.l	<__stack + \1, x
		tya
		sbc.h	<__stack + \1, x; CC if Y:A <= memory.
		ror	a
		eor	#$80
		rol	a
		.endm

; **************
; optimized boolean test (unsigned word)
; C is true (1) if Y:A > memory-value, else false (0)
; this MUST set the C flag for the susequent branches!

__ugt_w.ws	.macro	; __STACK
		ldx	<__sp
		clc			; Subtract memory+1 from Y:A.
		sbc.l	<__stack + \1, x
		tya
		sbc.h	<__stack + \1, x; CS if Y:A > memory.
		.endm

; **************
; optimized boolean test (unsigned word)
; C is true (1) if Y:A >= memory-value, else false (0)
; this MUST set the C flag for the susequent branches!

__uge_w.ws	.macro	; __STACK
		ldx	<__sp
		cmp.l	<__stack + \1, x; Subtract memory from Y:A.
		tya
		sbc.h	<__stack + \1, x; CS if Y:A >= memory.
		.endm

; **************
; optimized boolean test
; C is true (1) if Y:A == memory-value, else false (0)
; this MUST set the C flag for the subsequent branches!

__equ_w.us	.macro	; __STACK
		ldx	<__sp
		cmp	<__stack + \1, x
		bne	!false+
		tya
		beq	!+
!false:		clc
!:
		.endm

; **************
; optimized boolean test
; C is true (1) if Y:A != memory-value, else false (0)
; this MUST set the C flag for the subsequent branches!

__neq_w.us	.macro	; __STACK
		ldx	<__sp
		sec
		eor	<__stack + \1, x
		bne	!+
		tya
		bne	!+
		clc
!:
		.endm

; **************
; optimized boolean test (signed word)
; C is true (1) if Y:A < memory-value, else false (0)
; this MUST set the C flag for the susequent branches!

__slt_w.us	.macro	; __STACK
		ldx	<__sp
		cmp	<__stack + \1, x; Subtract memory from Y:A.
		tya
		sbc	#0
		bvc	!+
		eor	#$80		; -ve if Y:A < memory (signed).
!:		asl	a
		.endm

; **************
; optimized boolean test (signed word)
; C is true (1) if Y:A <= memory-value, else false (0)
; this MUST set the C flag for the susequent branches!

__sle_w.us	.macro	; __STACK
		ldx	<__sp
		clc			; Subtract memory+1 from Y:A.
		sbc	<__stack + \1, x
		tya
		sbc	#0
		bvc	!+
		eor	#$80		; -ve if Y:A <= memory (signed).
!:		asl	a
		.endm

; **************
; optimized boolean test (signed word)
; C is true (1) if Y:A > memory-value, else false (0)
; this MUST set the C flag for the susequent branches!

__sgt_w.us	.macro	; __STACK
		ldx	<__sp
		clc			; Subtract memory+1 from Y:A.
		sbc	<__stack + \1, x
		tya
		sbc	#0
		bvc	!+
		eor	#$80		; +ve if Y:A > memory (signed).
!:		eor	#$80
		asl	a
		.endm

; **************
; optimized boolean test (signed word)
; C is true (1) if Y:A >= memory-value, else false (0)
; this MUST set the C flag for the susequent branches!

__sge_w.us	.macro	; __STACK
		ldx	<__sp
		cmp	<__stack + \1, x; Subtract memory from Y:A.
		tya
		sbc	#0
		bvc	!+
		eor	#$80		; +ve if Y:A >= memory (signed).
!:		eor	#$80
		asl	a
		.endm

; **************
; optimized boolean test (unsigned word)
; C is true (1) if Y:A < memory-value, else false (0)
; this MUST set the C flag for the susequent branches!

__ult_w.us	.macro	; __STACK
		ldx	<__sp
		cmp	<__stack + \1, x; Subtract memory from Y:A.
		tya
		sbc	#0		; CC if Y:A < memory.
		ror	a
		eor	#$80
		rol	a
		.endm

; **************
; optimized boolean test (unsigned word)
; C is true (1) if Y:A <= memory-value, else false (0)
; this MUST set the C flag for the susequent branches!

__ule_w.us	.macro	; __STACK
		ldx	<__sp
		clc			; Subtract memory+1 from Y:A.
		sbc	<__stack + \1, x
		tya
		sbc	#0		; CC if Y:A <= memory.
		ror	a
		eor	#$80
		rol	a
		.endm

; **************
; optimized boolean test (unsigned word)
; C is true (1) if Y:A > memory-value, else false (0)
; this MUST set the C flag for the susequent branches!

__ugt_w.us	.macro	; __STACK
		ldx	<__sp
		clc			; Subtract memory+1 from Y:A.
		sbc	<__stack + \1, x
		tya
		sbc	#0		; CS if Y:A > memory.
		.endm

; **************
; optimized boolean test (unsigned word)
; C is true (1) if Y:A >= memory-value, else false (0)
; this MUST set the C flag for the susequent branches!

__uge_w.us	.macro	; __STACK
		ldx	<__sp
		cmp	<__stack + \1, x; Subtract memory from Y:A.
		tya
		sbc	#0		; CS if Y:A >= memory.
		.endm

; **************
; optimized boolean test
; C is true (1) if A == memory-value, else false (0)
; this MUST set the C flag for the subsequent branches!

__equ_b.usq	.macro	; __STACK
		ldx	<__sp
		cmp	<__stack + \1, x
		beq	!+
		clc
!:
		.endm

; **************
; optimized boolean test
; C is true (1) if A != memory-value, else false (0)
; this MUST set the C flag for the subsequent branches!

__neq_b.usq	.macro	; __STACK
		ldx	<__sp
		sec
		eor	<__stack + \1, x
		bne	!+
		clc
!:
		.endm

; **************
; optimized boolean test (signed byte)
; C is true (1) if A < memory-value, else false (0)
; this MUST set the C flag for the susequent branches!

__slt_b.bsq	.macro	; __STACK
		ldx	<__sp
		sec			; Subtract memory from A.
		sbc	<__stack + \1, x
		bvc	!+
		eor	#$80		; -ve if A < memory (signed).
!:		asl	a
		.endm

; **************
; optimized boolean test (signed byte)
; C is true (1) if A <= memory-value, else false (0)
; this MUST set the C flag for the susequent branches!

__sle_b.bsq	.macro	; __STACK
		ldx	<__sp
		clc			; Subtract memory+1 from A.
		sbc	<__stack + \1, x
		bvc	!+
		eor	#$80		; -ve if A <= memory (signed).
!:		asl	a
		.endm

; **************
; optimized boolean test (signed byte)
; C is true (1) if A > memory-value, else false (0)
; this MUST set the C flag for the susequent branches!

__sgt_b.bsq	.macro	; __STACK
		ldx	<__sp
		clc			; Subtract memory+1 from A.
		sbc.l	<__stack + \1, x
		bvc	!+
		eor	#$80		; +ve if A > memory (signed).
!:		eor	#$80
		asl	a
		.endm

; **************
; optimized boolean test (signed byte)
; C is true (1) if A >= memory-value, else false (0)
; this MUST set the C flag for the susequent branches!

__sge_b.bsq	.macro	; __STACK
		ldx	<__sp
		sec			; Subtract memory from A.
		sbc	<__stack + \1, x
		bvc	!+
		eor	#$80		; +ve if A >= memory (signed).
!:		eor	#$80
		asl	a
		.endm

; **************
; optimized boolean test (unsigned byte)
; C is true (1) if A < memory-value, else false (0)
; this MUST set the C flag for the susequent branches!

__ult_b.usq	.macro	; __STACK
		ldx	<__sp
		cmp	<__stack + \1, x; Subtract memory from A.
		ror	a		; CC if A < memory.
		eor	#$80
		rol	a
		.endm

; **************
; optimized boolean test (unsigned byte)
; C is true (1) if A <= memory-value, else false (0)
; this MUST set the C flag for the susequent branches!

__ule_b.usq	.macro	; __STACK
		ldx	<__sp
		clc			; Subtract memory+1 from A.
		sbc	<__stack + \1, x
		ror	a		; CC if A <= memory.
		eor	#$80
		rol	a
		.endm

; **************
; optimized boolean test (unsigned byte)
; C is true (1) if A > memory-value, else false (0)
; this MUST set the C flag for the susequent branches!

__ugt_b.usq	.macro	; __STACK
		ldx	<__sp
		clc			; Subtract memory+1 from A.
		sbc	<__stack + \1, x; CS if A > memory.
		.endm

; **************
; optimized boolean test (unsigned byte)
; C is true (1) if A >= memory-value, else false (0)
; this MUST set the C flag for the susequent branches!

__uge_b.usq	.macro	; __STACK
		ldx	<__sp
		cmp	<__stack + \1, x; Subtract memory from A.
					; CS if A >= memory.
		.endm

; **************
; optimized boolean test
; C is true (1) if Y:A != memory-value, else false (0)
; this MUST set the C flag for the subsequent branches!

__neq_w.wax	.macro
		sec
		eor.l	\1, x
		bne	!+
		tya
		eor.h	\1, x
		bne	!+
		clc
!:
		.endm

; **************
; optimized boolean test (signed word)
; C is true (1) if Y:A < memory-value, else false (0)
; this MUST set the C flag for the susequent branches!

__slt_w.wax	.macro
		cmp.l	\1, x		; Subtract memory from Y:A.
		tya
		sbc.h	\1, x
		bvc	!+
		eor	#$80		; -ve if Y:A < memory (signed).
!:		asl	a
		.endm

; **************
; optimized boolean test (signed word)
; C is true (1) if Y:A <= memory-value, else false (0)
; this MUST set the C flag for the susequent branches!

__sle_w.wax	.macro
		clc			; Subtract memory+1 from Y:A.
		sbc.l	\1, x
		tya
		sbc.h	\1, x
		bvc	!+
		eor	#$80		; -ve if Y:A <= memory (signed).
!:		asl	a
		.endm

; **************
; optimized boolean test (signed word)
; C is true (1) if Y:A > memory-value, else false (0)
; this MUST set the C flag for the susequent branches!

__sgt_w.wax	.macro
		clc			; Subtract memory+1 from Y:A.
		sbc.l	\1, x
		tya
		sbc.h	\1, x
		bvc	!+
		eor	#$80		; +ve if Y:A > memory (signed).
!:		eor	#$80
		asl	a
		.endm

; **************
; optimized boolean test (signed word)
; C is true (1) if Y:A >= memory-value, else false (0)
; this MUST set the C flag for the susequent branches!

__sge_w.wax	.macro
		cmp.l	\1, x		; Subtract memory from Y:A.
		tya
		sbc.h	\1, x
		bvc	!+
		eor	#$80		; +ve if Y:A >= memory (signed).
!:		eor	#$80
		asl	a
		.endm

; **************
; optimized boolean test (unsigned word)
; C is true (1) if Y:A < memory-value, else false (0)
; this MUST set the C flag for the susequent branches!

__ult_w.wax	.macro
		cmp.l	\1, x		; Subtract memory from Y:A.
		tya
		sbc.h	\1, x		; CC if Y:A < memory.
		ror	a
		eor	#$80
		rol	a
		.endm

; **************
; optimized boolean test (unsigned word)
; C is true (1) if Y:A <= memory-value, else false (0)
; this MUST set the C flag for the susequent branches!

__ule_w.wax	.macro
		clc			; Subtract memory+1 from Y:A.
		sbc.l	\1, x
		tya
		sbc.h	\1, x		; CC if Y:A <= memory.
		ror	a
		eor	#$80
		rol	a
		.endm

; **************
; optimized boolean test (unsigned word)
; C is true (1) if Y:A > memory-value, else false (0)
; this MUST set the C flag for the susequent branches!

__ugt_w.wax	.macro
		clc			; Subtract memory+1 from Y:A.
		sbc.l	\1, x
		tya
		sbc.h	\1, x		; CS if Y:A > memory.
		.endm

; **************
; optimized boolean test (unsigned word)
; C is true (1) if Y:A >= memory-value, else false (0)
; this MUST set the C flag for the susequent branches!

__uge_w.wax	.macro
		cmp.l	\1, x		; Subtract memory from Y:A.
		tya
		sbc.h	\1, x		; CS if Y:A >= memory.
		.endm

; **************
; optimized boolean test
; C is true (1) if Y:A == memory-value, else false (0)
; this MUST set the C flag for the subsequent branches!

__equ_w.uax	.macro
		cmp	\1, x
		bne	!false+
		tya
		beq	!+
!false:		clc
!:
		.endm

; **************
; optimized boolean test
; C is true (1) if Y:A != memory-value, else false (0)
; this MUST set the C flag for the subsequent branches!

__neq_w.uax	.macro
		sec
		eor	\1, x
		bne	!+
		tya
		bne	!+
		clc
!:
		.endm

; **************
; optimized boolean test (signed word)
; C is true (1) if Y:A < memory-value, else false (0)
; this MUST set the C flag for the susequent branches!

__slt_w.uax	.macro
		cmp	\1, x		; Subtract memory from Y:A.
		tya
		sbc	#0
		bvc	!+
		eor	#$80		; -ve if Y:A < memory (signed).
!:		asl	a
		.endm

; **************
; optimized boolean test (signed word)
; C is true (1) if Y:A <= memory-value, else false (0)
; this MUST set the C flag for the susequent branches!

__sle_w.uax	.macro
		clc			; Subtract memory+1 from Y:A.
		sbc	\1, x
		tya
		sbc	#0
		bvc	!+
		eor	#$80		; -ve if Y:A <= memory (signed).
!:		asl	a
		.endm

; **************
; optimized boolean test (signed word)
; C is true (1) if Y:A > memory-value, else false (0)
; this MUST set the C flag for the susequent branches!

__sgt_w.uax	.macro
		clc			; Subtract memory+1 from Y:A.
		sbc	\1, x
		tya
		sbc	#0
		bvc	!+
		eor	#$80		; +ve if Y:A > memory (signed).
!:		eor	#$80
		asl	a
		.endm

; **************
; optimized boolean test (signed word)
; C is true (1) if Y:A >= memory-value, else false (0)
; this MUST set the C flag for the susequent branches!

__sge_w.uax	.macro
		cmp	\1, x		; Subtract memory from Y:A.
		tya
		sbc	#0
		bvc	!+
		eor	#$80		; +ve if Y:A >= memory (signed).
!:		eor	#$80
		asl	a
		.endm

; **************
; optimized boolean test (unsigned word)
; C is true (1) if Y:A < memory-value, else false (0)
; this MUST set the C flag for the susequent branches!

__ult_w.uax	.macro
		cmp	\1, x		; Subtract memory from Y:A.
		tya
		sbc	#0		; CC if Y:A < memory.
		ror	a
		eor	#$80
		rol	a
		.endm

; **************
; optimized boolean test (unsigned word)
; C is true (1) if Y:A <= memory-value, else false (0)
; this MUST set the C flag for the susequent branches!

__ule_w.uax	.macro
		clc			; Subtract memory+1 from Y:A.
		sbc	\1, x
		tya
		sbc	#0		; CC if Y:A <= memory.
		ror	a
		eor	#$80
		rol	a
		.endm

; **************
; optimized boolean test (unsigned word)
; C is true (1) if Y:A > memory-value, else false (0)
; this MUST set the C flag for the susequent branches!

__ugt_w.uax	.macro
		clc			; Subtract memory+1 from Y:A.
		sbc	\1, x
		tya
		sbc	#0		; CS if Y:A > memory.
		.endm

; **************
; optimized boolean test (unsigned word)
; C is true (1) if Y:A >= memory-value, else false (0)
; this MUST set the C flag for the susequent branches!

__uge_w.uax	.macro
		cmp	\1, x		; Subtract memory from Y:A.
		tya
		sbc	#0		; CS if Y:A >= memory.
		.endm

; **************
; optimized boolean test
; C is true (1) if A == memory-value, else false (0)
; this MUST set the C flag for the subsequent branches!

__equ_b.uaxq	.macro
		cmp	\1, x
		beq	!+
		clc
!:
		.endm

; **************
; optimized boolean test
; C is true (1) if A != memory-value, else false (0)
; this MUST set the C flag for the subsequent branches!

__neq_b.uaxq	.macro
		sec
		eor	\1, x
		bne	!+
		clc
!:
		.endm

; **************
; optimized boolean test (signed byte)
; C is true (1) if A < memory-value, else false (0)
; this MUST set the C flag for the susequent branches!

__slt_b.baxq	.macro
		sec			; Subtract memory from A.
		sbc	\1, x
		bvc	!+
		eor	#$80		; -ve if A < memory (signed).
!:		asl	a
		.endm

; **************
; optimized boolean test (signed byte)
; C is true (1) if A <= memory-value, else false (0)
; this MUST set the C flag for the susequent branches!

__sle_b.baxq	.macro
		clc			; Subtract memory+1 from A.
		sbc	\1, x
		bvc	!+
		eor	#$80		; -ve if A <= memory (signed).
!:		asl	a
		.endm

; **************
; optimized boolean test (signed byte)
; C is true (1) if A > memory-value, else false (0)
; this MUST set the C flag for the susequent branches!

__sgt_b.baxq	.macro
		clc			; Subtract memory+1 from A.
		sbc.l	\1, x
		bvc	!+
		eor	#$80		; +ve if A > memory (signed).
!:		eor	#$80
		asl	a
		.endm

; **************
; optimized boolean test (signed byte)
; C is true (1) if A >= memory-value, else false (0)
; this MUST set the C flag for the susequent branches!

__sge_b.baxq	.macro
		sec			; Subtract memory from A.
		sbc	\1, x
		bvc	!+
		eor	#$80		; +ve if A >= memory (signed).
!:		eor	#$80
		asl	a
		.endm

; **************
; optimized boolean test (unsigned byte)
; C is true (1) if A < memory-value, else false (0)
; this MUST set the C flag for the susequent branches!

__ult_b.uaxq	.macro
		cmp	\1, x		; Subtract memory from A.
		ror	a		; CC if A < memory.
		eor	#$80
		rol	a
		.endm

; **************
; optimized boolean test (unsigned byte)
; C is true (1) if A <= memory-value, else false (0)
; this MUST set the C flag for the susequent branches!

__ule_b.uaxq	.macro
		clc			; Subtract memory+1 from A.
		sbc	\1, x
		ror	a		; CC if A <= memory.
		eor	#$80
		rol	a
		.endm

; **************
; optimized boolean test (unsigned byte)
; C is true (1) if A > memory-value, else false (0)
; this MUST set the C flag for the susequent branches!

__ugt_b.uaxq	.macro
		clc			; Subtract memory+1 from A.
		sbc	\1, x		; CS if A > memory.
		.endm

; **************
; optimized boolean test (unsigned byte)
; C is true (1) if A >= memory-value, else false (0)
; this MUST set the C flag for the susequent branches!

__uge_b.uaxq	.macro
		cmp	\1, x		; Subtract memory from A.
					; CS if A >= memory.
		.endm

; **************
; boolean test, optimized into __not.wr if used before a __tst.wr
; C is true (1) if Y:A == 0, else false (0)
; this MUST set the C flag for the subsequent branches!

__not.wr	.macro
		sty	__temp
		ora	__temp
		clc
		bne	!+
		sec
!:
		.endm

; **************
; optimized boolean test
; C is true (1) if memory-value == 0, else false (0)
; this MUST set the C flag for the subsequent branches!

__not.wp	.macro
		ldy	#1
		lda	[\1], y
		ora	[\1]
		clc
		bne	!+
		sec
!:
		.endm

; **************
; optimized boolean test
; C is true (1) if memory-value == 0, else false (0)
; this MUST set the C flag for the subsequent branches!

__not.wm	.macro
		lda.l	\1
		ora.h	\1
		clc
		bne	!+
		sec
!:
		.endm

; **************
; optimized boolean test
; C is true (1) if memory-value == 0, else false (0)
; this MUST set the C flag for the subsequent branches!

__not.ws	.macro	; __STACK
		ldx	<__sp
		lda.l	<__stack + \1, x
		ora.h	<__stack + \1, x
		clc
		bne	!+
		sec
!:
		.endm

; **************
; optimized boolean test
; C is true (1) if memory-value == 0, else false (0)
; this MUST set the C flag for the subsequent branches!

__not.war	.macro
		asl	a
		tay
		lda.l	\1, y
		ora.h	\1, y
		clc
		bne	!+
		sec
!:
		.endm

; **************
; optimized boolean test
; C is true (1) if memory-value == 0, else false (0)
; this MUST set the C flag for the subsequent branches!

__not.wax	.macro
		lda.l	\1, x
		ora.h	\1, x
		clc
		bne	!+
		sec
!:
		.endm

; **************
; optimized boolean test
; C is true (1) if memory-value == 0, else false (0)
; this MUST set the C flag for the subsequent branches!

__not.up	.macro
		lda	[\1]
		clc
		bne	!+
		sec
!:
		.endm

; **************
; optimized boolean test
; C is true (1) if memory-value == 0, else false (0)
; this MUST set the C flag for the subsequent branches!

__not.um	.macro
		lda	\1
		clc
		bne	!+
		sec
!:
		.endm

; **************
; optimized boolean test
; C is true (1) if memory-value == 0, else false (0)
; this MUST set the C flag for the subsequent branches!

__not.us	.macro	; __STACK
		ldx	<__sp
		lda.l	<__stack + \1, x
		clc
		bne	!+
		sec
!:
		.endm

; **************
; optimized boolean test
; C is true (1) if memory-value == 0, else false (0)
; this MUST set the C flag for the subsequent branches!

__not.uar	.macro
		tay
		lda	\1, y
		clc
		bne	!+
		sec
!:
		.endm

; **************
; optimized boolean test
; C is true (1) if memory-value == 0, else false (0)
; this MUST set the C flag for the subsequent branches!

__not.uax	.macro
		lda	\1, x
		clc
		bne	!+
		sec
!:
		.endm

; **************
; optimized boolean test
; C is true (1) if memory-value == 0, else false (0)
; this MUST set the C flag for the subsequent branches!

__not.uay	.macro
		lda	\1, y
		clc
		bne	!+
		sec
!:
		.endm

; **************
; boolean test, always output immediately before a __bfalse or __btrue
; C is true (1) if Y:A != 0, else false (0)
; this MUST set the C flag for the subsequent branches!

__tst.wr	.macro
		sty	__temp
		ora	__temp
		cmp	#1
		.endm

; **************
; optimized boolean test
; C is true (1) if memory-value != 0, else false (0)
; this MUST set the C flag for the subsequent branches!

__tst.wp	.macro
		ldy	#1
		lda	[\1], y
		ora	[\1]
		cmp	#1
		.endm

; **************
; optimized boolean test
; C is true (1) if memory-value != 0, else false (0)
; this MUST set the C flag for the subsequent branches!

__tst.wm	.macro
		lda.l	\1
		ora.h	\1
		cmp	#1
		.endm

; **************
; optimized boolean test
; C is true (1) if memory-value != 0, else false (0)
; this MUST set the C flag for the subsequent branches!

__tst.ws	.macro	; __STACK
		ldx	<__sp
		lda.l	<__stack + \1, x
		ora.h	<__stack + \1, x
		cmp	#1
		.endm

; **************
; optimized boolean test
; C is true (1) if memory-value != 0, else false (0)
; this MUST set the C flag for the subsequent branches!

__tst.war	.macro
		asl	a
		tay
		lda.l	\1, y
		ora.h	\1, y
		cmp	#1
		.endm

; **************
; optimized boolean test
; C is true (1) if memory-value != 0, else false (0)
; this MUST set the C flag for the subsequent branches!

__tst.wax	.macro
		lda.l	\1, x
		ora.h	\1, x
		cmp	#1
		.endm

; **************
; optimized boolean test
; C is true (1) if memory-value != 0, else false (0)
; this MUST set the C flag for the subsequent branches!

__tst.up	.macro
		lda	[\1]
		cmp	#1
		.endm

; **************
; optimized boolean test
; C is true (1) if memory-value != 0, else false (0)
; this MUST set the C flag for the subsequent branches!

__tst.um	.macro
		lda	\1
		cmp	#1
		.endm

; **************
; optimized boolean test
; C is true (1) if memory-value != 0, else false (0)
; this MUST set the C flag for the subsequent branches!

__tst.us	.macro	; __STACK
		ldx	<__sp
		lda.l	<__stack + \1, x
		cmp	#1
		.endm

; **************
; optimized boolean test
; C is true (1) if memory-value != 0, else false (0)
; this MUST set the C flag for the subsequent branches!

__tst.uar	.macro
		tay
		lda	\1, y
		cmp	#1
		.endm

; **************
; optimized boolean test
; C is true (1) if memory-value != 0, else false (0)
; this MUST set the C flag for the subsequent branches!

__tst.uax	.macro
		lda	\1, x
		cmp	#1
		.endm

; **************
; optimized boolean test
; C is true (1) if memory-value != 0, else false (0)
; this MUST set the C flag for the subsequent branches!

__tst.uay	.macro
		lda	\1, y
		cmp	#1
		.endm

; **************
; optimized boolean test
; C is true (1) if (Y:A & integer) == 0, else false (0)
; this MUST set the C flag for the subsequent branches!

__nand.wi	.macro
		clc
	.if	((\1 & $FF00) == 0)
		and	#\1
	.else
	.if	((\1 & $00FF) == 0)
		tya
		and.h	#\1
	.else
		and.l	#\1
		bne	!+
		tya
		and.h	#\1
	.endif
	.endif
		bne	!+
		sec
!:
		.endm

; **************
; optimized boolean test
; C is true (1) if (Y:A & integer) != 0, else false (0)
; this MUST set the C flag for the subsequent branches!

__tand.wi	.macro
	.if	((\1 & $FF00) == 0)
		and	#\1
	.else
	.if	((\1 & $00FF) == 0)
		tya
		and.h	#\1
	.else
		and.l	#\1
		bne	!+
		tya
		and.h	#\1
	.endif
	.endif
!:		cmp	#1
		.endm

; **************
; optimized boolean test
; invert C flag

__not.cf	.macro
		ror	a
		eor	#$80
		rol	a
		.endm

; **************
; convert comparison result C flag into a 16-bit Y:A boolean integer

__bool		.macro
		cla
		rol	a
		cly
		.endm

; **************
; optimized boolean test used before a store, not a branch
; Y:A is true (1) if Y:A == 0, else false (0)

__boolnot.wr	.macro
		sty	__temp
		ora	__temp
		cla
		bne	!+
		inc	a
!:		cly
		.endm

; **************
; optimized boolean test used before a store, not a branch
; Y:A is true (1) if memory-value == 0, else false (0)

__boolnot.wp	.macro
		ldy	#1
		lda	[\1], y
		ora	[\1]
		cla
		bne	!+
		inc	a
!:		cly
		.endm

; **************
; optimized boolean test used before a store, not a branch
; Y:A is true (1) if memory-value == 0, else false (0)

__boolnot.wm	.macro
		lda.l	\1
		ora.h	\1
		cla
		bne	!+
		inc	a
!:		cly
		.endm

; **************
; optimized boolean test used before a store, not a branch
; Y:A is true (1) if memory-value == 0, else false (0)

__boolnot.ws	.macro	; __STACK
		ldx	<__sp
		lda.l	<__stack + \1, x
		ora.h	<__stack + \1, x
		cla
		bne	!+
		inc	a
!:		cly
		.endm

; **************
; optimized boolean test used before a store, not a branch
; Y:A is true (1) if memory-value == 0, else false (0)

__boolnot.war	.macro
		asl	a
		tay
		lda.l	\1, y
		ora.h	\1, y
		cla
		bne	!+
		inc	a
!:		cly
		.endm

; **************
; optimized boolean test used before a store, not a branch
; Y:A is true (1) if memory-value == 0, else false (0)

__boolnot.wax	.macro
		lda.l	\1, x
		ora.h	\1, x
		cla
		bne	!+
		inc	a
!:		cly
		.endm

; **************
; optimized boolean test used before a store, not a branch
; Y:A is true (1) if memory-value == 0, else false (0)

__boolnot.up	.macro
		lda	[\1]
		cla
		bne	!+
		inc	a
!:		cly
		.endm

; **************
; optimized boolean test used before a store, not a branch
; Y:A is true (1) if memory-value == 0, else false (0)

__boolnot.um	.macro
		lda	\1
		cla
		bne	!+
		inc	a
!:		cly
		.endm

; **************
; optimized boolean test used before a store, not a branch
; Y:A is true (1) if memory-value == 0, else false (0)

__boolnot.us	.macro	; __STACK
		ldx	<__sp
		lda.l	<__stack + \1, x
		cla
		bne	!+
		inc	a
!:		cly
		.endm

; **************
; optimized boolean test used before a store, not a branch
; Y:A is true (1) if memory-value == 0, else false (0)

__boolnot.uar	.macro
		tay
		lda	\1, y
		cla
		bne	!+
		inc	a
!:		cly
		.endm

; **************
; optimized boolean test used before a store, not a branch
; Y:A is true (1) if memory-value == 0, else false (0)

__boolnot.uax	.macro
		lda	\1, x
		cla
		bne	!+
		inc	a
!:		cly
		.endm

; **************
; optimized boolean test used before a store, not a branch
; Y:A is true (1) if memory-value == 0, else false (0)

__boolnot.uay	.macro
		lda	\1, y
		cla
		bne	!+
		inc	a
!:		cly
		.endm



; ***************************************************************************
; ***************************************************************************
; i-codes for loading the primary register
; ***************************************************************************
; ***************************************************************************

; **************

__ld.wi		.macro
	.if	(\?1 != ARG_ABS)
		lda.l	#\1
		ldy.h	#\1
	.else
	.if	(\1 & $00FF)
		lda.l	#\1
	.else
		cla
	.endif
	.if	(\1 & $FF00)
		ldy.h	#\1
	.else
		cly
	.endif
	.endif
		.endm

; **************

__ld.uiq	.macro
	.if	(\?1 != ARG_ABS)
		lda.l	#\1
	.else
	.if	(\1 & $00FF)
		lda	#\1
	.else
		cla
	.endif
	.endif
		.endm

; **************

__lea.s		.macro	; __STACK
		lda	<__sp
		clc
		adc.l	#__stack + (\1)
		ldy.h	#__stack
		.endm

; **************

__ld.wm		.macro
		lda.l	\1
		ldy.h	\1
		.endm

; **************

__ld.bm		.macro
		lda	\1
		cly
		bpl	!+
		dey
!:
		.endm

; **************

__ld.um		.macro
		lda	\1
		cly
		.endm

; **************

__ld.wmq	.macro
		lda.l	\1
		.endm

; **************

__ld.bmq	.macro
		lda	\1
		.endm

; **************

__ld.umq	.macro
		lda	\1
		.endm

; **************

__ldx.wmq	.macro
		ldx.l	\1
		.endm

; **************

__ldx.bmq	.macro
		ldx	\1
		.endm

; **************

__ldx.umq	.macro
		ldx	\1
		.endm

; **************

__ld2x.wm	.macro
		tax
		lda.l	\1
		asl	a
		sax
		.endm

; **************

__ld2x.bm	.macro
		tax
		lda	\1
		asl	a
		sax
		.endm

; **************

__ld2x.um	.macro
		tax
		lda	\1
		asl	a
		sax
		.endm

; **************

__ld2x.wmq	.macro
		lda.l	\1
		asl	a
		tax
		.endm

; **************

__ld2x.bmq	.macro
		lda	\1
		asl	a
		tax
		.endm

; **************

__ld2x.umq	.macro
		lda	\1
		asl	a
		tax
		.endm

; **************

__ldy.wmq	.macro
		ldy.l	\1
		.endm

; **************

__ldy.bmq	.macro
		ldy	\1
		.endm

; **************

__ldy.umq	.macro
		ldy	\1
		.endm

; **************

__ld.wp		.macro
		ldy	#1
		lda	[\1], y
		tay
		lda	[\1]
		.endm

; **************

__ld.bp		.macro
		lda	[\1]
		cly
		bpl	!+
		dey
!:
		.endm

; **************

__ld.up		.macro
		lda	[\1]
		cly
		.endm

; **************

__ld.upq	.macro
		lda	[\1]
		.endm

; **************

__ld.war	.macro
		asl	a
		tax
		lda.l	\1, x
		ldy.h	\1, x
		.endm

; **************

__ld.bar	.macro
		tay
		lda	\1, y
		cly
		bpl	!+
		dey
!:
		.endm

; **************

__ld.uar	.macro
		tay
		lda	\1, y
		cly
		.endm

; **************

__ld.uarq	.macro
		tay
		lda	\1, y
		.endm

; **************

__ld.wax	.macro
		lda.l	\1, x
		ldy.h	\1, x
		.endm

; **************

__ld.bax	.macro
		lda	\1, x
		cly
		bpl	!+
		dey
!:
		.endm

; **************

__ld.uax	.macro
		lda	\1, x
		cly
		.endm

; **************

__ld.uaxq	.macro
		lda	\1, x
		.endm

; **************

__ld.bay	.macro
		lda	\1, y
		cly
		bpl	!+
		dey
!:
		.endm

; **************

__ld.uay	.macro
		lda	\1, y
		cly
		.endm

; **************

__ld.wam	.macro
		lda	\2
		asl	a
		tax
		lda.l	\1, x
		ldy.h	\1, x
		.endm

; **************

__ld.bam	.macro
		ldy	\2
		lda	\1, y
		cly
		bpl	!+
		dey
!:
		.endm

; **************

__ld.uam	.macro
		ldy	\2
		lda	\1, y
		cly
		.endm

; **************
; special load for when array writes are optimized
; the cpu stack is balanced with an __st.wat

__ldp.war	.macro
		asl	a
		tax
		phx
		lda.l	\1, x
		ldy.h	\1, x
		.endm

; **************
; special load for when array writes are optimized
; the cpu stack is balanced with an __st.uat

__ldp.bar	.macro
		tay
		phy
		lda	\1, y
		cly
		bpl	!+
		dey
!:
		.endm

; **************
; special load for when array writes are optimized
; the cpu stack is balanced with an __st.uat

__ldp.uar	.macro
		tay
		phy
		lda	\1, y
		cly
		.endm

; **************
; special load for when array writes are optimized
; the cpu stack is balanced with an __st.uat

__ldp.wax	.macro
		phx
		lda.l	\1, x
		ldy.h	\1, x
		.endm

; **************
; special load for when array writes are optimized
; the cpu stack is balanced with an __st.uat

__ldp.bax	.macro
		phx
		lda	\1, x
		cly
		bpl	!+
		dey
!:
		.endm

; **************
; special load for when array writes are optimized
; the cpu stack is balanced with an __st.uat

__ldp.uax	.macro
		phx
		lda	\1, x
		cly
		.endm

; **************
; special load for when array writes are optimized
; the cpu stack is balanced with an __st.uat

__ldp.bay	.macro
		phy
		lda	\1, y
		cly
		bpl	!+
		dey
!:
		.endm

; **************
; special load for when array writes are optimized
; the cpu stack is balanced with an __st.uat

__ldp.uay	.macro
		phy
		lda	\1, y
		cly
		.endm

; **************

__ld.ws		.macro	; __STACK
		ldx	<__sp
		lda.l	<__stack + \1, x
		ldy.h	<__stack + \1, x
		.endm

; **************

__ld.bs		.macro	; __STACK
		ldx	<__sp
		lda	<__stack + \1, x
		cly
		bpl	!+	; signed
		dey
!:
		.endm

; **************

__ld.us		.macro	; __STACK
		ldx	<__sp
		lda	<__stack + \1, x
		cly
		.endm

; **************

__ld.wsq	.macro	; __STACK
		ldx	<__sp
		lda.l	<__stack + \1, x
		.endm

; **************

__ld.bsq	.macro	; __STACK
		ldx	<__sp
		lda	<__stack + \1, x
		.endm

; **************

__ld.usq	.macro	; __STACK
		ldx	<__sp
		lda	<__stack + \1, x
		.endm

; **************

__ldx.ws	.macro	; __STACK
		phy
		ldy	<__sp
		ldx.l	<__stack + \1, y
		ply
		.endm

; **************

__ldx.bs	.macro	; __STACK
		phy
		ldy	<__sp
		ldx	<__stack + \1, y
		ply
		.endm

; **************

__ldx.us	.macro	; __STACK
		phy
		ldy	<__sp
		ldx	<__stack + \1, y
		ply
		.endm

; **************

__ldx.wsq	.macro	; __STACK
		ldy	<__sp
		ldx.l	<__stack + \1, y
		.endm

; **************

__ldx.bsq	.macro	; __STACK
		ldy	<__sp
		ldx	<__stack + \1, y
		.endm

; **************

__ldx.usq	.macro	; __STACK
		ldy	<__sp
		ldx	<__stack + \1, y
		.endm

; **************

__ld2x.ws	.macro	; __STACK
		ldx	<__sp
		pha
		lda.l	<__stack + \1, x
		asl	a
		tax
		pla
		.endm

; **************

__ld2x.bs	.macro	; __STACK
		ldx	<__sp
		pha
		lda	<__stack + \1, x
		asl	a
		tax
		pla
		.endm

; **************

__ld2x.us	.macro	; __STACK
		ldx	<__sp
		pha
		lda	<__stack + \1, x
		asl	a
		tax
		pla
		.endm

; **************

__ld2x.wsq	.macro	; __STACK
		ldx	<__sp
		lda.l	<__stack + \1, x
		asl	a
		tax
		.endm

; **************

__ld2x.bsq	.macro	; __STACK
		ldx	<__sp
		lda	<__stack + \1, x
		asl	a
		tax
		.endm

; **************

__ld2x.usq	.macro	; __STACK
		ldx	<__sp
		lda	<__stack + \1, x
		asl	a
		tax
		.endm

; **************

__ldy.wsq	.macro	; __STACK
		ldx	<__sp
		ldy.l	<__stack + \1, x
		.endm

; **************

__ldy.bsq	.macro	; __STACK
		ldx	<__sp
		ldy	<__stack + \1, x
		.endm

; **************

__ldy.usq	.macro	; __STACK
		ldx	<__sp
		ldy	<__stack + \1, x
		.endm



; ***************************************************************************
; ***************************************************************************
; i-codes for pre- and post- increment and decrement
; ***************************************************************************
; ***************************************************************************

; **************

__incld.wm	.macro
		inc.l	\1
		bne	!+
		inc.h	\1
!:		lda.l	\1
		ldy.h	\1
		.endm

; **************

__ldinc.wm	.macro
		lda.l	\1
		ldy.h	\1
		inc.l	\1
		bne	!+
		inc.h	\1
!:
		.endm

; **************

__decld.wm	.macro
		lda.l	\1
		bne	!+
		dec.h	\1
!:		dec	a
		sta.l	\1
		ldy.h	\1
		.endm

; **************

__lddec.wm	.macro
		ldy.h	\1
		lda.l	\1
		bne	!+
		dec.h	\1
!:		dec.l	\1
		.endm

; **************

__incld.bm	.macro
		inc	\1
		lda	\1
		cly
		bpl	!+
		dey
!:
		.endm

; **************

__ldinc.bm	.macro
		lda	\1
		cly
		bpl	!+
		dey
!:		inc	\1
		.endm

; **************

__decld.bm	.macro
		dec	\1
		lda	\1
		cly
		bpl	!+
		dey
!:
		.endm

; **************

__lddec.bm	.macro
		lda	\1
		cly
		bpl	!+
		dey
!:		dec	\1
		.endm

; **************

__incld.um	.macro
		inc	\1
		lda	\1
		cly
		.endm

; **************

__ldinc.um	.macro
		lda	\1
		cly
		inc	\1
		.endm

; **************

__decld.um	.macro
		dec	\1
		lda	\1
		cly
		.endm

; **************

__lddec.um	.macro
		lda	\1
		cly
		dec	\1
		.endm

; **************
; optimized macro used when the value isn't needed in the primary register

__inc.wmq	.macro
		inc.l	\1
		bne	!+
		inc.h	\1
!:
		.endm

; **************
; optimized macro used when the value isn't needed in the primary register

__inc.umq	.macro
		inc	\1
		.endm

; **************
; optimized macro used when the value isn't needed in the primary register

__dec.wmq	.macro
		lda.l	\1
		bne	!+
		dec.h	\1
!:		dec.l	\1
		.endm

; **************
; optimized macro used when the value isn't needed in the primary register

__dec.umq	.macro
		dec	\1
		.endm

; **************

__incld.ws	.macro
		ldx	<__sp
		inc.l	<__stack + \1, x
		bne	!+
		inc.h	<__stack + \1, x
!:		lda.l	<__stack + \1, x
		ldy.h	<__stack + \1, x
		.endm

; **************

__ldinc.ws	.macro
		ldx	<__sp
		lda.l	<__stack + \1, x
		ldy.h	<__stack + \1, x
		inc.l	<__stack + \1, x
		bne	!+
		inc.h	<__stack + \1, x
!:
		.endm

; **************

__decld.ws	.macro
		ldx	<__sp
		lda.l	<__stack + \1, x
		bne	!+
		dec.h	<__stack + \1, x
!:		dec	a
		sta.l	<__stack + \1, x
		ldy.h	<__stack + \1, x
		.endm

; **************

__lddec.ws	.macro
		ldx	<__sp
		ldy.h	<__stack + \1, x
		lda.l	<__stack + \1, x
		bne	!+
		dec.h	<__stack + \1, x
!:		dec.l	<__stack + \1, x
		.endm

; **************

__incld.bs	.macro
		ldx	<__sp
		inc	<__stack + \1, x
		lda	<__stack + \1, x
		cly
		bpl	!+
		dey
!:
		.endm

; **************

__ldinc.bs	.macro
		ldx	<__sp
		lda	<__stack + \1, x
		cly
		bpl	!+
		dey
!:		inc	<__stack + \1, x
		.endm

; **************

__decld.bs	.macro
		ldx	<__sp
		dec	<__stack + \1, x
		lda	<__stack + \1, x
		cly
		bpl	!+
		dey
!:
		.endm

; **************

__lddec.bs	.macro
		ldx	<__sp
		lda	<__stack + \1, x
		cly
		bpl	!+
		dey
!:		dec	<__stack + \1, x
		.endm

; **************

__incld.us	.macro
		ldx	<__sp
		inc	<__stack + \1, x
		lda	<__stack + \1, x
		cly
		.endm

; **************

__ldinc.us	.macro
		ldx	<__sp
		lda	<__stack + \1, x
		cly
		inc	<__stack + \1, x
		.endm

; **************

__decld.us	.macro
		ldx	<__sp
		dec	<__stack + \1, x
		lda	<__stack + \1, x
		cly
		.endm

; **************

__lddec.us	.macro
		ldx	<__sp
		lda	<__stack + \1, x
		cly
		dec	<__stack + \1, x
		.endm

; **************
; optimized macro used when the value isn't needed in the primary register

__inc.wsq	.macro
		ldx	<__sp
		inc.l	<__stack + \1, x
		bne	!+
		inc.h	<__stack + \1, x
!:
		.endm

; **************
; optimized macro used when the value isn't needed in the primary register

__inc.usq	.macro
		ldx	<__sp
		inc	<__stack + \1, x
		.endm

; **************
; optimized macro used when the value isn't needed in the primary register

__dec.wsq	.macro
		ldx	<__sp
		lda.l	<__stack + \1, x
		bne	!+
		dec.h	<__stack + \1, x
!:		dec.l	<__stack + \1, x
		.endm

; **************
; optimized macro used when the value isn't needed in the primary register

__dec.usq	.macro
		ldx	<__sp
		dec	<__stack + \1, x
		.endm

; **************

__incld.war	.macro
		asl	a
		tax
		inc.l	\1, x
		bne	!+
		inc.h	\1, x
!:		lda.l	\1, x
		ldy.h	\1, x
		.endm

; **************

__ldinc.war	.macro
		asl	a
		tax
		lda.l	\1, x
		ldy.h	\1, x
		inc.l	\1, x
		bne	!+
		inc.h	\1, x
!:
		.endm

; **************

__decld.war	.macro
		asl	a
		tax
		lda.l	\1, x
		bne	!+
		dec.h	\1, x
!:		dec	a
		sta.l	\1, x
		ldy.h	\1, x
		.endm

; **************

__lddec.war	.macro
		asl	a
		tax
		ldy.h	\1, x
		lda.l	\1, x
		bne	!+
		dec.h	\1, x
!:		dec.l	\1, x
		.endm

; **************

__incld.bar	.macro
		tax
		inc	\1, x
		lda	\1, x
		cly
		bpl	!+
		dey
!:
		.endm

; **************

__ldinc.bar	.macro
		tax
		lda	\1, x
		cly
		bpl	!+
		dey
!:		inc	\1, x
		.endm

; **************

__decld.bar	.macro
		tax
		dec	\1, x
		lda	\1, x
		cly
		bpl	!+
		dey
!:
		.endm

; **************

__lddec.bar	.macro
		tax
		lda	\1, x
		cly
		bpl	!+
		dey
!:		dec	\1, x
		.endm

; **************

__incld.uar	.macro
		tax
		inc	\1, x
		lda	\1, x
		cly
		.endm

; **************

__ldinc.uar	.macro
		tax
		lda	\1, x
		cly
		inc	\1, x
		.endm

; **************

__decld.uar	.macro
		tax
		dec	\1, x
		lda	\1, x
		cly
		.endm

; **************

__lddec.uar	.macro
		tax
		lda	\1, x
		cly
		dec	\1, x
		.endm

; **************
; optimized macro used when the value isn't needed in the primary register

__inc.warq	.macro
		asl	a
		tax
		inc.l	\1, x
		bne	!+
		inc.h	\1, x
!:
		.endm

; **************
; optimized macro used when the value isn't needed in the primary register

__inc.uarq	.macro
		tax
		inc	\1, x
		.endm


; **************
; optimized macro used when the value isn't needed in the primary register

__dec.warq	.macro
		asl	a
		tax
		lda.l	\1, x
		bne	!+
		dec.h	\1, x
!:		dec.l	\1, x
		.endm

; **************
; optimized macro used when the value isn't needed in the primary register

__dec.uarq	.macro
		tax
		dec	\1, x
		.endm

; **************

__incld.wax	.macro
		inc.l	\1, x
		bne	!+
		inc.h	\1, x
!:		lda.l	\1, x
		ldy.h	\1, x
		.endm

; **************

__ldinc.wax	.macro
		lda.l	\1, x
		ldy.h	\1, x
		inc.l	\1, x
		bne	!+
		inc.h	\1, x
!:
		.endm

; **************

__decld.wax	.macro
		lda.l	\1, x
		bne	!+
		dec.h	\1, x
!:		dec	a
		sta.l	\1, x
		ldy.h	\1, x
		.endm

; **************

__lddec.wax	.macro
		ldy.h	\1, x
		lda.l	\1, x
		bne	!+
		dec.h	\1, x
!:		dec.l	\1, x
		.endm

; **************

__incld.bax	.macro
		inc	\1, x
		lda	\1, x
		cly
		bpl	!+
		dey
!:
		.endm

; **************

__ldinc.bax	.macro
		lda	\1, x
		cly
		bpl	!+
		dey
!:		inc	\1, x
		.endm

; **************

__decld.bax	.macro
		dec	\1, x
		lda	\1, x
		cly
		bpl	!+
		dey
!:
		.endm

; **************

__lddec.bax	.macro
		lda	\1, x
		cly
		bpl	!+
		dey
!:		dec	\1, x
		.endm

; **************

__incld.uax	.macro
		inc	\1, x
		lda	\1, x
		cly
		.endm

; **************

__ldinc.uax	.macro
		lda	\1, x
		cly
		inc	\1, x
		.endm

; **************

__decld.uax	.macro
		dec	\1, x
		lda	\1, x
		cly
		.endm

; **************

__lddec.uax	.macro
		lda	\1, x
		cly
		dec	\1, x
		.endm

; **************
; optimized macro used when the value isn't needed in the primary register

__inc.waxq	.macro
		inc.l	\1, x
		bne	!+
		inc.h	\1, x
!:
		.endm

; **************
; optimized macro used when the value isn't needed in the primary register

__inc.uaxq	.macro
		inc	\1, x
		.endm


; **************
; optimized macro used when the value isn't needed in the primary register

__dec.waxq	.macro
		lda.l	\1, x
		bne	!+
		dec.h	\1, x
!:		dec.l	\1, x
		.endm

; **************
; optimized macro used when the value isn't needed in the primary register

__dec.uaxq	.macro
		dec	\1, x
		.endm

; **************

__incld.bay	.macro
		lda	\1, y
		inc	a
		sta	\1, y
		cly
		bpl	!+
		dey
!:
		.endm

; **************

__ldinc.bay	.macro
		lda.l	\1, y
		inc	a
		sta.l	\1, y
		dec	a
		cly
		bpl	!+
		dey
!:
		.endm

; **************

__decld.bay	.macro
		lda	\1, y
		dec	a
		sta	\1, y
		cly
		bpl	!+
		dey
!:
		.endm

; **************

__lddec.bay	.macro
		lda.l	\1, y
		dec	a
		sta.l	\1, y
		inc	a
		cly
		bpl	!+
		dey
!:
		.endm

; **************

__incld.uay	.macro
		lda	\1, y
		inc	a
		sta	\1, y
		cly
		.endm

; **************

__ldinc.uay	.macro
		lda.l	\1, y
		inc	a
		sta.l	\1, y
		dec	a
		cly
		.endm

; **************

__decld.uay	.macro
		lda	\1, y
		dec	a
		sta	\1, y
		cly
		.endm

; **************

__lddec.uay	.macro
		lda.l	\1, y
		dec	a
		sta.l	\1, y
		inc	a
		cly
		.endm

; **************
; optimized macro used when the value isn't needed in the primary register

__inc.uayq	.macro
		sxy
		inc	\1, x
		.endm

; **************
; optimized macro used when the value isn't needed in the primary register

__dec.uayq	.macro
		sxy
		dec	\1, x
		.endm



; ***************************************************************************
; ***************************************************************************
; i-codes for saving the primary register
; ***************************************************************************
; ***************************************************************************

; **************

__st.wpt	.macro	; __STACK
		plx
		stx.l	<__ptr
		plx
		stx.h	<__ptr
		sta	[__ptr]
		tax
		tya
		ldy	#1
		sta	[__ptr], y
		tay
		txa
		.endm

; **************

__st.upt	.macro	; __STACK
		plx
		stx.l	<__ptr
		plx
		stx.h	<__ptr
		sta	[__ptr]
		.endm

; **************

__st.wptq	.macro	; __STACK
		plx
		stx.l	<__ptr
		plx
		stx.h	<__ptr
		sta	[__ptr]
		tya
		ldy	#1
		sta	[__ptr], y
		.endm

; **************

__st.uptq	.macro	; __STACK
		plx
		stx.l	<__ptr
		plx
		stx.h	<__ptr
		sta	[__ptr]
		.endm

; **************

__st.wm		.macro
		sta.l	\1
		sty.h	\1
		.endm

; **************

__st.um		.macro
		sta	\1
		.endm

; **************

__st.wmq	.macro
		sta.l	\1
		sty.h	\1
		.endm

; **************

__st.umq	.macro
		sta	\1
		.endm

; **************

__st.wmiq	.macro
	.if	(\?1 != ARG_ABS)
		lda.l	#\1
		sta.l	\2
		lda.h	#\1
		sta.h	\2
	.else
	.if	(\1 & $00FF)
		lda.l	#\1
		sta.l	\2
	.else
		stz.l	\2
	.endif
	.if	(\1 & $FF00)
		lda.h	#\1
		sta.h	\2
	.else
		stz.h	\2
	.endif
	.endif
		.endm

; **************

__st.umiq	.macro
	.if	(\1 != 0)
		lda.l	#\1
		sta	\2
	.else
		stz	\2
	.endif
		.endm

; **************

__st.wp		.macro
		sta	[\1]
		pha
		tya
		ldy	#1
		sta	[\1], y
		tay
		pla
		.endm

; **************

__st.up		.macro
		sta	[\1]
		.endm

; **************

__st.wpq	.macro
		sta	[\1]
		tya
		ldy	#1
		sta	[\1], y
		.endm

; **************

__st.upq	.macro
		sta	[\1]
		.endm

; **************

__st.wpi	.macro
		sta.l	__ptr
		sty.h	__ptr
		ldy	#1
		lda.h	#\1
		sta	[__ptr], y
		tay
		lda.l	#\1
		sta	[__ptr]
		.endm

; **************

__st.upi	.macro
		sta.l	__ptr
		sty.h	__ptr
		lda.l	#\1
		ldy.h	#\1
		sta	[__ptr]
		.endm

; **************

__st.wpiq	.macro
		sta.l	__ptr
		sty.h	__ptr
		ldy	#1
		lda.h	#\1
		sta	[__ptr], y
		lda.l	#\1
		sta	[__ptr]
		.endm

; **************

__st.upiq	.macro
		sta.l	__ptr
		sty.h	__ptr
		lda.l	#\1
		sta	[__ptr]
		.endm

; **************
; special store for when array writes are optimized
; the cpu stack is balanced with an __st.wat

__index.wr	.macro
		asl	a
		pha
		.endm

; **************
; special store for when array writes are optimized
; the cpu stack is balanced with an __st.uat

__index.ur	.macro
		pha
		.endm

; **************
; special store for when array writes are optimized
; this balances the cpu stack after an __index.wr or __ldp.war

__st.wat	.macro
		plx
		sta.l	\1, x
		say
		sta.h	\1, x
		say
		.endm

; **************
; special store for when array writes are optimized
; this balances the cpu stack after an __index.ur or __ldp.uar

__st.uat	.macro
		plx
		sta	\1, x
		.endm

; **************
; special store for when array writes are optimized
; this balances the cpu stack after an __index.wr or __ldp.war

__st.watq	.macro
		plx
		sta.l	\1, x
		tya
		sta.h	\1, x
		.endm

; **************
; special store for when array writes are optimized
; this balances the cpu stack after an __index.ur or __ldp.uar

__st.uatq	.macro
		plx
		sta	\1, x
		.endm

; **************

__st.watiq	.macro
		plx
	.if	(\?1 != ARG_ABS)
		lda.l	#\1
		sta.l	\2, x
		lda.h	#\1
		sta.h	\2, x
	.else
	.if	(\1 & $00FF)
		lda.l	#\1
		sta.l	\2, x
	.else
		stz.l	\2, x
	.endif
	.if	(\1 & $FF00)
		lda.h	#\1
		sta.h	\2, x
	.else
		stz.h	\2, x
	.endif
	.endif
		.endm

; **************

__st.uatiq	.macro
		plx
	.if	(\1 != 0)
		lda.l	#\1
		sta	\2, x
	.else
		stz	\2, x
	.endif
		.endm

; **************
; special store for when array writes are optimized
; this balances the cpu stack after an __index.wr or __ldp.war

__st.wax	.macro
		sta.l	\1, x
		say
		sta.h	\1, x
		say
		.endm

; **************
; special store for when array writes are optimized
; this balances the cpu stack after an __index.ur or __ldp.uar

__st.uax	.macro
		sta	\1, x
		.endm

; **************
; special store for when array writes are optimized
; this balances the cpu stack after an __index.wr or __ldp.war

__st.waxq	.macro
		sta.l	\1, x
		tya
		sta.h	\1, x
		.endm

; **************
; special store for when array writes are optimized
; this balances the cpu stack after an __index.ur or __ldp.uar

__st.uaxq	.macro
		sta	\1, x
		.endm

; **************

__st.waxiq	.macro
	.if	(\?1 != ARG_ABS)
		lda.l	#\1
		sta.l	\2, x
		lda.h	#\1
		sta.h	\2, x
	.else
	.if	(\1 & $00FF)
		lda.l	#\1
		sta.l	\2, x
	.else
		stz.l	\2, x
	.endif
	.if	(\1 & $FF00)
		lda.h	#\1
		sta.h	\2, x
	.else
		stz.h	\2, x
	.endif
	.endif
		.endm

; **************

__st.uaxiq	.macro
	.if	(\1 != 0)
		lda.l	#\1
		sta	\2, x
	.else
		stz	\2, x
	.endif
		.endm

; **************

__st.wsiq	.macro
		ldx	<__sp
	.if	(\?1 != ARG_ABS)
		lda.l	#\1
		sta.l	<__stack + \2, x
		lda.h	#\1
		sta.h	<__stack + \2, x
	.else
	.if	(\1 & $00FF)
		lda.l	#\1
		sta.l	<__stack + \2, x
	.else
		stz.l	<__stack + \2, x
	.endif
	.if	(\1 & $FF00)
		lda.h	#\1
		sta.h	<__stack + \2, x
	.else
		stz.h	<__stack + \2, x
	.endif
	.endif
		.endm

; **************

__st.usiq	.macro
		ldx	<__sp
	.if	(\1 != 0)
		lda.l	#\1
		sta	<__stack + \2, x
	.else
		stz	<__stack + \2, x
	.endif
		.endm

; **************

__st.ws		.macro	; __STACK
		ldx	<__sp
		sta.l	<__stack + \1, x
		sty.h	<__stack + \1, x
		.endm

; **************

__st.us		.macro	; __STACK
		ldx	<__sp
		sta	<__stack + \1, x
		.endm

; **************

__st.wsq	.macro	; __STACK
		ldx	<__sp
		sta.l	<__stack + \1, x
		sty.h	<__stack + \1, x
		.endm

; **************

__st.usq	.macro	; __STACK
		ldx	<__sp
		sta	<__stack + \1, x
		.endm



; ***************************************************************************
; ***************************************************************************
; i-codes for extending the primary register
; ***************************************************************************
; ***************************************************************************

; **************

__ext.br	.macro
		tay
		cly
		bpl	!+
		dey
!:
		.endm

; **************

__ext.ur	.macro
		cly
		.endm



; ***************************************************************************
; ***************************************************************************
; i-codes for 16-bit math with the primary register
; ***************************************************************************
; ***************************************************************************

; **************

__com.wr	.macro
		eor	#$FF
		say
		eor	#$FF
		say
		.endm

; **************

__neg.wr	.macro
		sec
		eor	#$FF
		adc	#0
		say
		eor	#$FF
		adc	#0
		say
		.endm

; **************

__add.wt	.macro	; __STACK
		tsx
		clc
		adc.l	__tos, x
		say
		adc.h	__tos, x
		say
		inx
		inx
		txs
		.endm

; **************

__add.wi	.macro
	.if	((\?1 == ARG_ABS) && ((\1) == 1))
		inc	a
		bne	!+
		iny
!:
	.else
	.if	((\?1 == ARG_ABS) && ((\1) >= 0) && ((\1) < 256))
		clc
		adc.l	#\1
		bcc	!+
		iny
!:
	.else
		clc
		adc.l	#\1
		say
		adc.h	#\1
		say
	.endif
	.endif
		.endm

; **************

__add.wm	.macro
		clc
		adc.l	\1
		say
		adc.h	\1
		say
		.endm

; **************

__add.um	.macro
		clc
		adc	\1
		bcc	!+
		iny
!:
		.endm

; **************

__add.wp	.macro
		clc
		adc	[\1]
		tax
		tya
		ldy	#1
		adc	[\1], y
		tay
		txa
		.endm

; **************

__add.up	.macro
		clc
		adc	[\1]
		bcc	!+
		iny
!:
		.endm
; **************

__add.ws	.macro	; __STACK
		ldx	<__sp
		clc
		adc.l	<__stack + \1, x
		say
		adc.h	<__stack + \1, x
		say
		.endm

; **************

__add.us	.macro	; __STACK
		ldx	<__sp
		clc
		adc	<__stack + \1, x
		bcc	!+
		iny
!:
		.endm

; **************
; special math for when array math is optimized
; this balances the cpu stack after an __index.wr

__add.wat	.macro
		plx
		clc
		adc.l	\1, x
		say
		adc.h	\1, x
		say
		.endm

; **************
; special math for when array math is optimized
; this balances the cpu stack after an __index.ur

__add.uat	.macro
		plx
		clc
		adc	\1, x
		bcc	!+
		iny
!:
		.endm

; **************

__add.wax	.macro
		clc
		adc.l	\1, x
		say
		adc.h	\1, x
		say
		.endm

; **************

__add.uax	.macro
		clc
		adc	\1, x
		bcc	!+
		iny
!:
		.endm

; **************
; Y:A = stack - Y:A

__sub.wt	.macro	; __STACK
		tsx
		sec
		eor	#$FF
		adc.l	__tos, x
		say
		eor	#$FF
		adc.h	__tos, x
		say
		inx
		inx
		txs
		.endm

; **************
; Y:A = Y:A - immediate

__sub.wi	.macro
	.if	((\?1 == ARG_ABS) && ((\1) >= 0) && ((\1) < 256))
		sec
		sbc.l	#\1
		bcs	!+
		dey
!:
	.else
		sec
		sbc.l	#\1
		say
		sbc.h	#\1
		say
	.endif
		.endm

; **************
; Y:A = Y:A - memory

__sub.wm	.macro
		sec
		sbc.l	\1
		say
		sbc.h	\1
		say
		.endm

; **************
; Y:A = Y:A - memory

__sub.um	.macro
		sec
		sbc	\1
		bcs	!+
		dey
!:
		.endm

; **************
; Y:A = Y:A - memory

__sub.wp	.macro
		sec
		sbc	[\1]
		tax
		tya
		ldy	#1
		sbc	[\1], y
		tay
		txa
		.endm

; **************
; Y:A = Y:A - memory

__sub.up	.macro
		sec
		sbc	[\1]
		bcs	!+
		dey
!:
		.endm

; **************
; Y:A = Y:A - memory

__sub.ws	.macro
		ldx	<__sp
		sec
		sbc.l	<__stack + \1, x
		say
		sbc.h	<__stack + \1, x
		say
		.endm

; **************
; Y:A = Y:A - memory

__sub.us	.macro
		ldx	<__sp
		sec
		sbc	<__stack + \1, x
		bcs	!+
		dey
!:
		.endm

; **************
; special math for when array math is optimized
; this balances the cpu stack after an __index.wr
; Y:A = Y:A - memory

__sub.wat	.macro
		plx
		sec
		sbc.l	\1, x
		say
		sbc.h	\1, x
		say
		.endm

; **************
; special math for when array math is optimized
; this balances the cpu stack after an __index.ur
; Y:A = Y:A - memory

__sub.uat	.macro
		plx
		sec
		sbc	\1, x
		bcs	!+
		dey
!:
		.endm

; **************
; Y:A = Y:A - memory

__sub.wax	.macro
		sec
		sbc.l	\1, x
		say
		sbc.h	\1, x
		say
		.endm

; **************
; Y:A = Y:A - memory

__sub.uax	.macro
		sec
		sbc	\1, x
		bcs	!+
		dey
!:
		.endm

; **************
; Y:A = Y:A - stack

__isub.wt	.macro	; __STACK
		tsx
		sec
		sbc.l	__tos, x
		say
		sbc.h	__tos, x
		say
		inx
		inx
		txs
		.endm

; **************
; Y:A = immediate - Y:A

__isub.wi	.macro
		sec
		eor	#$FF
		adc.l	#\1
		say
		eor	#$FF
		adc.h	#\1
		say
		.endm

; **************
; Y:A = memory - Y:A

__isub.wm	.macro
		sec
		eor	#$FF
		adc.l	\1
		say
		eor	#$FF
		adc.h	\1
		say
		.endm

; **************
; Y:A = memory - Y:A

__isub.um	.macro
		sec
		eor	#$FF
		adc	\1
		bcc	!+
		iny
!:
		.endm

; **************
; Y:A = memory - Y:A

__isub.wp	.macro
		sec
		eor	#$FF
		adc	[\1]
		tax
		tya
		eor	#$FF
		ldy	#1
		adc	[\1], y
		tay
		txa
		.endm

; **************
; Y:A = memory - Y:A

__isub.up	.macro
		sec
		eor	#$FF
		adc	[\1]
		bcc	!+
		iny
!:
		.endm

; **************
; Y:A = memory - Y:A

__isub.ws	.macro	; __STACK
		ldx	<__sp
		sec
		eor	#$FF
		adc.l	<__stack + \1, x
		say
		eor	#$FF
		adc.h	<__stack + \1, x
		say
		.endm

; **************
; Y:A = memory - Y:A

__isub.us	.macro	; __STACK
		ldx	<__sp
		sec
		eor	#$FF
		adc	<__stack + \1, x
		say
		eor	#$FF
		adc	#0
		say
		.endm

; **************
; special math for when array math is optimized
; this balances the cpu stack after an __index.wr
; Y:A = memory - Y:A

__isub.wat	.macro
		plx
		sec
		eor	#$FF
		adc.l	\1, x
		say
		eor	#$FF
		adc.h	\1, x
		say
		.endm

; **************
; special math for when array math is optimized
; this balances the cpu stack after an __index.ur
; Y:A = memory - Y:A

__isub.uat	.macro
		plx
		sec
		eor	#$FF
		adc	\1, x
		say
		eor	#$FF
		adc	#0
		say
		.endm

; **************
; Y:A = memory - Y:A

__isub.wax	.macro
		sec
		eor	#$FF
		adc.l	\1, x
		say
		eor	#$FF
		adc.h	\1, x
		say
		.endm

; **************
; Y:A = memory - Y:A

__isub.uax	.macro
		sec
		eor	#$FF
		adc	\1, x
		say
		eor	#$FF
		adc	#0
		say
		.endm

; **************

__and.wt	.macro	; __STACK
		tsx
		and.l	__tos, x
		say
		and.h	__tos, x
		say
		inx
		inx
		txs
		.endm

; **************

__and.wi	.macro
	.if	(\?1 != ARG_ABS)
		and.l	#\1
		say
		and.h	#\1
		say
	.else
	.if	((\1 >= 0) && (\1 < 256))
		and	#\1
		cly
	.else
	.if	(\1 & 255)
		and.l	#\1
		say
		and.h	#\1
		say
	.else
		tya
		and.h	#\1
		tay
		cla
	.endif
	.endif
	.endif
		.endm

; **************

__and.wm	.macro
		and.l	\1
		say
		and.h	\1
		say
		.endm

; **************

__and.um	.macro
		and	\1
		cly
		.endm

; **************

__and.wp	.macro
		and	[\1]
		tax
		tya
		ldy	#1
		and	[\1], y
		tay
		txa
		.endm

; **************

__and.up	.macro
		and	[\1]
		cly
		.endm

; **************

__and.ws	.macro
		ldx	<__sp
		and.l	<__stack + \1, x
		say
		and.h	<__stack + \1, x
		say
		.endm

; **************

__and.us	.macro
		ldx	<__sp
		and	<__stack + \1, x
		cly
		.endm

; **************
; special math for when array math is optimized
; this balances the cpu stack after an __index.wr

__and.wat	.macro
		plx
		and.l	\1, x
		say
		and.h	\1, x
		say
		.endm

; **************
; special math for when array math is optimized
; this balances the cpu stack after an __index.ur

__and.uat	.macro
		plx
		and	\1, x
		cly
		.endm

; **************

__and.wax	.macro
		and.l	\1, x
		say
		and.h	\1, x
		say
		.endm

; **************

__and.uax	.macro
		and	\1, x
		cly
		.endm

; **************

__eor.wt	.macro	; __STACK
		tsx
		eor.l	__tos, x
		say
		eor.h	__tos, x
		say
		inx
		inx
		txs
		.endm

; **************

__eor.wi	.macro
	.if	((\1 >= 0) && (\1 < 256))
		eor	#\1
	.else
	.if	(\1 & 255)
		eor.l	#\1
		say
		eor.h	#\1
		say
	.else
		say
		eor.h	#\1
		say
	.endif
	.endif
		.endm

; **************

__eor.wm	.macro
		eor.l	\1
		say
		eor.h	\1
		say
		.endm

; **************

__eor.um	.macro
		eor	\1
		.endm


; **************

__eor.wp	.macro
		eor	[\1]
		tax
		tya
		ldy	#1
		eor	[\1], y
		tay
		txa
		.endm

; **************

__eor.up	.macro
		eor	[\1]
		.endm

; **************

__eor.ws	.macro
		ldx	<__sp
		eor.l	<__stack + \1, x
		say
		eor.h	<__stack + \1, x
		say
		.endm

; **************

__eor.us	.macro
		ldx	<__sp
		eor	<__stack + \1, x
		.endm

; **************
; special math for when array math is optimized
; this balances the cpu stack after an __index.wr

__eor.wat	.macro
		plx
		eor.l	\1, x
		say
		eor.h	\1, x
		say
		.endm

; **************
; special math for when array math is optimized
; this balances the cpu stack after an __index.ur

__eor.uat	.macro
		plx
		eor	\1, x
		.endm

; **************

__eor.wax	.macro
		eor.l	\1, x
		say
		eor.h	\1, x
		say
		.endm

; **************

__eor.uax	.macro
		eor	\1, x
		.endm

; **************

__or.wt		.macro	; __STACK
		tsx
		ora.l	__tos, x
		say
		ora.h	__tos, x
		say
		inx
		inx
		txs
		.endm

; **************

__or.wi		.macro
	.if	((\1 >= 0) && (\1 < 256))
		ora	#\1
	.else
	.if	(\1 & 255)
		ora.l	#\1
		say
		ora.h	#\1
		say
	.else
		say
		ora.h	#\1
		say
	.endif
	.endif
		.endm

; **************

__or.wm		.macro
		ora.l	\1
		say
		ora.h	\1
		say
		.endm

; **************

__or.um		.macro
		ora	\1
		.endm

; **************

__or.wp		.macro
		ora	[\1]
		tax
		tya
		ldy	#1
		ora	[\1], y
		tay
		txa
		.endm

; **************

__or.up		.macro
		ora	[\1]
		.endm

; **************

__or.ws		.macro
		ldx	<__sp
		ora.l	<__stack + \1, x
		say
		ora.h	<__stack + \1, x
		say
		.endm

; **************

__or.us		.macro
		ldx	<__sp
		ora	<__stack + \1, x
		.endm

; **************
; special math for when array math is optimized
; this balances the cpu stack after an __index.wr

__or.wat	.macro
		plx
		ora.l	\1, x
		say
		ora.h	\1, x
		say
		.endm

; **************
; special math for when array math is optimized
; this balances the cpu stack after an __index.ur

__or.uat	.macro
		plx
		ora	\1, x
		.endm

; **************

__or.wax	.macro
		ora.l	\1, x
		say
		ora.h	\1, x
		say
		.endm

; **************

__or.uax	.macro
		ora	\1, x
		.endm

; **************
; N.B. Used when calculating a pointer into a word array.

__double	.macro
		tsx
		asl.l	__tos, x
		rol.h	__tos, x
		.endm

; **************

__asl.wt	.macro
		tax
		pla
		ply
		jsr	asl.wx
		.endm

; **************

__asl.wi	.macro
	.if (\1 = 1)
		asl	a
		say
		rol	a
		say
	.else
	.if (\1 = 2)
		asl	a
		say
		rol	a
		say
		asl	a
		say
		rol	a
		say
	.else
	.if (\1 < 5)
		sty	__temp
		jsr	aslw\1
	.else
	.if (\1 < 7)
		sta	__temp
		tya
		lsr	a
		jsr	aslw\1
		and.l	#$FF << \1
	.else
	.if (\1 = 7)
		say
		lsr	a
		say
		ror	a
		say
		ror	a
		and	#$80
	.else
	.if (\1 = 8)
		tay
		cla
	.else
	.if (\1 < 16)
		jsr	aslw\1
	.else
		cla
		cly
	.endif
	.endif
	.endif
	.endif
	.endif
	.endif
	.endif
		.endm

; **************

__asl.wr	.macro
		asl	a
		say
		rol	a
		say
		.endm

; **************

__asr.wt	.macro
		tax
		pla
		ply
		jsr	asr.wx
		.endm

; **************

__asr.wi	.macro
	.if (\1 = 1)
		cpy	#$80
		say
		ror	a
		say
		ror	a
	.else
	.if (\1 = 2)
		cpy	#$80
		say
		ror	a
		say
		ror	a
		cpy	#$80
		say
		ror	a
		say
		ror	a
	.else
	.if (\1 < 8)
		sty	__temp
		jsr	asrw\1
	.else
	.if (\1 = 8)
		tya
		cly
		bpl	!+
		dey
!:
	.else
	.if (\1 < 16)
		tya
		jsr	asrw\1
	.else
		cpy	#$80
		cly
		bcc	!+
		dey
!:		tya
	.endif
	.endif
	.endif
	.endif
	.endif
		.endm

; **************

__lsr.wt	.macro
		tax
		pla
		ply
		jsr	lsr.wx
		.endm

; **************

__lsr.wi	.macro
	.if (\1 = 1)
		say
		lsr	a
		say
		ror	a
	.else
	.if (\1 = 2)
		say
		lsr	a
		say
		ror	a
		say
		lsr	a
		say
		ror	a
	.else
	.if (\1 < 8)
		sty	__temp
		jsr	lsrw\1
	.else
	.if (\1 = 8)
		tya
		cly
		bmi	!+
		dey
!:
	.else
	.if (\1 < 16)
		tya
		jsr	lsrw\1
	.else
		cly
		bcc	!+
		dey
!:		tya
	.endif
	.endif
	.endif
	.endif
	.endif
		.endm

; **************
; Y:A = stacked-value * Y:A
;
; N.B. signed and unsigned multiply only differ in the top 16 of the 32bits!

__mul.wt	.macro
		sta.l	<multiplier
		sty.h	<multiplier
		pla
		ply
		jsr	__mulint
		.endm

; **************

__mul.wi	.macro
	.if (\1 = 2)
	__asl.wr
	.else
	.if (\1 = 3)
		sta.l	__temp
		sty.h	__temp
	__asl.wr
	__add.wm	__temp
	.else
	.if (\1 = 4)
	__asl.wr
	__asl.wr
	.else
	.if (\1 = 5)
		sta.l	__temp
		sty.h	__temp
	__asl.wr
	__asl.wr
	__add.wm	__temp
	.else
	.if (\1 = 6)
	__asl.wr
		sta.l	__temp
		sty.h	__temp
	__asl.wr
	__add.wm	__temp
	.else
	.if (\1 = 7)
		sta.l	__temp
		sty.h	__temp
	__asl.wr
	__asl.wr
	__asl.wr
	__sub.wm	__temp
	.else
	.if (\1 = 8)
	__asl.wr
	__asl.wr
	__asl.wr
	.else
	.if (\1 = 9)
		sta.l	__temp
		sty.h	__temp
	__asl.wr
	__asl.wr
	__asl.wr
	__add.wm	__temp
	.else
	.if (\1 = 10)
	__asl.wr
		sta.l	__temp
		sty.h	__temp
	__asl.wr
	__asl.wr
	__add.wm	__temp
	.else
		ldx.l	#\1
		stx.l	<multiplier
		ldx.h	#\1
		stx.h	<multiplier
		jsr	__mulint
	.endif
	.endif
	.endif
	.endif
	.endif
	.endif
	.endif
	.endif
	.endif
		.endm

; **************
; Y:A = stacked-value / Y:A

__sdiv.wt	.macro
		sta.l	<divisor
		sty.h	<divisor
		pla
		ply
		jsr	__divsint
		.endm

; **************

__sdiv.wi	.macro
		ldx.l	#\1
		stx.l	<divisor
		ldx.h	#\1
		stx.h	<divisor
		jsr	__divsint
		.endm

; **************
; Y:A = stacked-value / Y:A

__udiv.wt	.macro
		sta.l	<divisor
		sty.h	<divisor
		pla
		ply
		jsr	__divuint
		.endm

; **************

__udiv.wi	.macro
		ldx.l	#\1
		stx.l	<divisor
		ldx.h	#\1
		stx.h	<divisor
		jsr	__divuint
		.endm

; **************

__udiv.ui	.macro
		ldy	#\1
		jsr	__divuchar
		.endm

; **************
; Y:A = stacked-value % Y:A

__smod.wt	.macro
		sta.l	<divisor
		sty.h	<divisor
		pla
		ply
		jsr	__modsint
		.endm

; **************

__smod.wi	.macro
		ldx.l	#\1
		stx.l	<divisor
		ldx.h	#\1
		stx.h	<divisor
		jsr	__modsint
		.endm

; **************
; Y:A = stacked-value % Y:A

__umod.wt	.macro
		sta.l	<divisor
		sty.h	<divisor
		pla
		ply
		jsr	__moduint
		.endm

; **************

__umod.wi	.macro
		ldx.l	#\1
		stx.l	<divisor
		ldx.h	#\1
		stx.h	<divisor
		jsr	__moduint
		.endm

; **************

__umod.ui	.macro
		ldy	#\1
		jsr	__moduchar
		.endm



; ***************************************************************************
; ***************************************************************************
; i-codes for 8-bit math with lo-byte of the primary register
; ***************************************************************************
; ***************************************************************************

; **************

__add.uiq	.macro
	.if	((\?1 == ARG_ABS) && ((\1) == 1))
		inc	a
	.else
		clc
		adc.l	#\1
	.endif
		.endm

; **************

__add.umq	.macro
		clc
		adc	\1
		.endm

; **************

__add.upq	.macro
		clc
		adc	[\1]
		.endm

; **************

__add.usq	.macro	; __STACK
		ldx	<__sp
		clc
		adc	<__stack + \1, x
		.endm

; **************
; special math for when array math is optimized
; this balances the cpu stack after an __index.ur

__add.uatq	.macro
		plx
		clc
		adc	\1, x
		.endm

; **************

__add.uaxq	.macro
		clc
		adc	\1, x
		.endm

; **************
; Y:A = Y:A - immediate

__sub.uiq	.macro
	.if	((\?1 == ARG_ABS) && ((\1) >= 0) && ((\1) < 256))
		sec
		sbc.l	#\1
	.endif
		.endm

; **************
; Y:A = Y:A - memory

__sub.umq	.macro
		sec
		sbc	\1
		.endm

; **************
; Y:A = Y:A - memory

__sub.upq	.macro
		sec
		sbc	[\1]
		.endm

; **************
; Y:A = Y:A - memory

__sub.usq	.macro
		ldx	<__sp
		sec
		sbc	<__stack + \1, x
		.endm

; **************
; special math for when array math is optimized
; this balances the cpu stack after an __index.ur
; Y:A = Y:A - memory

__sub.uatq	.macro
		plx
		sec
		sbc	\1, x
		.endm

; **************
; Y:A = Y:A - memory

__sub.uaxq	.macro
		sec
		sbc	\1, x
		.endm

; **************
; Y:A = immediate - Y:A

__isub.uiq	.macro
		sec
		eor	#$FF
		adc.l	#\1
		.endm

; **************
; Y:A = memory - Y:A

__isub.umq	.macro
		sec
		eor	#$FF
		adc	\1
		.endm

; **************
; Y:A = memory - Y:A

__isub.upq	.macro
		sec
		eor	#$FF
		adc	[\1]
		.endm

; **************
; Y:A = memory - Y:A

__isub.usq	.macro	; __STACK
		ldx	<__sp
		sec
		eor	#$FF
		adc	<__stack + \1, x
		.endm

; **************
; special math for when array math is optimized
; this balances the cpu stack after an __index.ur
; Y:A = memory - Y:A

__isub.uatq	.macro
		plx
		sec
		eor	#$FF
		adc	\1, x
		.endm

; **************
; Y:A = memory - Y:A

__isub.uaxq	.macro
		sec
		eor	#$FF
		adc	\1, x
		.endm

; **************

__and.uiq	.macro
		and	#\1
		.endm

; **************

__and.umq	.macro
		and	\1
		.endm

; **************

__and.upq	.macro
		and	[\1]
		.endm

; **************

__and.usq	.macro
		ldx	<__sp
		and	<__stack + \1, x
		.endm

; **************
; special math for when array math is optimized
; this balances the cpu stack after an __index.ur

__and.uatq	.macro
		plx
		and	\1, x
		.endm

; **************

__and.uaxq	.macro
		and	\1, x
		.endm

; **************

__eor.uiq	.macro
		eor	#\1
		.endm

; **************

__eor.umq	.macro
		eor	\1
		.endm

; **************

__eor.upq	.macro
		eor	[\1]
		.endm

; **************

__eor.usq	.macro
		ldx	<__sp
		eor	<__stack + \1, x
		.endm

; **************
; special math for when array math is optimized
; this balances the cpu stack after an __index.ur

__eor.uatq	.macro
		plx
		eor	\1, x
		.endm

; **************

__eor.uaxq	.macro
		eor	\1, x
		.endm

; **************

__or.uiq	.macro
		ora	#\1
		.endm

; **************

__or.umq	.macro
		ora	\1
		.endm

; **************

__or.upq	.macro
		ora	[\1]
		.endm

; **************

__or.usq	.macro
		ldx	<__sp
		ora	<__stack + \1, x
		.endm

; **************
; special math for when array math is optimized
; this balances the cpu stack after an __index.ur

__or.uatq	.macro
		plx
		ora	\1, x
		.endm

; **************

__or.uaxq	.macro
		ora	\1, x
		.endm

; **************

__asl.uiq	.macro
	.if (\1 == 8)
		asl	a
	.endif
	.if (\1 >= 7)
		asl	a
	.endif
	.if (\1 >= 6)
		asl	a
	.endif
	.if (\1 >= 5)
		asl	a
	.endif
	.if (\1 >= 4)
		asl	a
	.endif
	.if (\1 >= 3)
		asl	a
	.endif
	.if (\1 >= 2)
		asl	a
	.endif
	.if (\1 >= 1)
		asl	a
	.endif
		.endm

; **************

__lsr.uiq	.macro
	.if (\1 < 8)
	.if (\1 >= 1)
		lsr	a
	.endif
	.if (\1 >= 2)
		lsr	a
	.endif
	.if (\1 >= 3)
		lsr	a
	.endif
	.if (\1 >= 4)
		lsr	a
	.endif
	.if (\1 >= 5)
		lsr	a
	.endif
	.if (\1 >= 6)
		lsr	a
	.endif
	.if (\1 >= 7)
		lsr	a
	.endif
	.else
		cla
	.endif
		.endm

; **************

__mul.uiq	.macro
	.if (\1 == 2)
		asl	a
	.endif
	.if (\1 == 3)
		sta	__temp
		asl	a
		adc	__temp
	.endif
	.if (\1 == 4)
		asl	a
		asl	a
	.endif
	.if (\1 == 5)
		sta	__temp
		asl	a
		asl	a
		adc	__temp
	.endif
	.if (\1 == 6)
		asl	a
		sta	__temp
		asl	a
		adc	__temp
	.endif
	.if (\1 == 7)
		sta	__temp
		asl	a
		asl	a
		asl	a
		sec
		sbc	__temp
	.endif
	.if (\1 == 8)
		asl	a
		asl	a
		asl	a
	.endif
	.if (\1 == 9)
		sta	__temp
		asl	a
		asl	a
		asl	a
		adc	__temp
	.endif
	.if (\1 == 10)
		asl	a
		sta	__temp
		asl	a
		asl	a
		adc	__temp
	.endif
	.if (\1 == 11)
		sta.l	__temp
		asl	a
		sta.h	__temp
		asl	a
		asl	a
		adc.l	__temp
		adc.h	__temp
	.endif
	.if (\1 == 12)
		asl	a
		asl	a
		sta	__temp
		asl	a
		adc	__temp
	.endif
	.if (\1 == 13)
		sta.l	__temp
		asl	a
		asl	a
		sta.h	__temp
		asl	a
		adc.l	__temp
		adc.h	__temp
	.endif
	.if (\1 == 14)
		asl	a
		sta	__temp
		asl	a
		asl	a
		asl	a
		sec
		sbc	__temp
	.endif
	.if (\1 == 15)
		sta	__temp
		asl	a
		asl	a
		asl	a
		asl	a
		sec
		sbc	__temp
	.endif
	.if (\1 == 16)
		asl	a
		asl	a
		asl	a
		asl	a
	.endif
	.if (\1 >= 17)
		ldy	#\1
		jsr	__muluchar
	.endif
		.endm



; ***************************************************************************
; ***************************************************************************
; i-codes for modifying a variable with "+=", "-=", "&=", "^=", "|="
; ***************************************************************************
; ***************************************************************************

; **************

__add_st.wmq	.macro
		clc
		adc.l	\1
		sta.l	\1
		tya
		adc.h	\1
		sta.h	\1
		.endm

; **************

__add_st.umq	.macro
		clc
		adc	\1
		sta	\1
		.endm

; **************

__add_st.wpq	.macro
		clc
		adc	[\1]
		sta	[\1]
		tya
		ldy	#1
		adc	[\1], y
		sta	[\1], y
		.endm

; **************

__add_st.upq	.macro
		clc
		adc	[\1]
		sta	[\1]
		.endm

; **************

__add_st.wsq	.macro	; __STACK
		ldx	<__sp
		clc
		adc.l	<__stack + \1, x
		sta.l	<__stack + \1, x
		tya
		adc.h	<__stack + \1, x
		sta.h	<__stack + \1, x
		.endm

; **************

__add_st.usq	.macro	; __STACK
		ldx	<__sp
		clc
		adc	<__stack + \1, x
		sta	<__stack + \1, x
		.endm

; **************

__add_st.watq	.macro
		plx
		clc
		adc.l	\1, x
		sta.l	\1, x
		tya
		adc.h	\1, x
		sta.h	\1, x
		.endm

; **************

__add_st.uatq	.macro
		plx
		clc
		adc	\1, x
		sta	\1, x
		.endm

; **************

__add_st.waxq	.macro
		clc
		adc.l	\1, x
		sta.l	\1, x
		tya
		adc.h	\1, x
		sta.h	\1, x
		.endm

; **************

__add_st.uaxq	.macro
		clc
		adc	\1, x
		sta	\1, x
		.endm

; **************
; Y:A = memory - Y:A

__isub_st.wmq	.macro
		sec
		eor	#$FF
		adc.l	\1
		sta.l	\1
		tya
		eor	#$FF
		adc.h	\1
		sta.h	\1
		.endm

; **************
; Y:A = memory - Y:A

__isub_st.umq	.macro
		sec
		eor	#$FF
		adc	\1
		sta	\1
		.endm

; **************

__isub_st.wpq	.macro
		sec
		eor	#$FF
		adc	[\1]
		sta	[\1]
		tya
		ldy	#1
		eor	#$FF
		adc	[\1], y
		sta	[\1], y
		.endm

; **************

__isub_st.upq	.macro
		sec
		eor	#$FF
		adc	[\1]
		sta	[\1]
		.endm

; **************
; Y:A = memory - Y:A

__isub_st.wsq	.macro	; __STACK
		ldx	<__sp
		sec
		eor	#$FF
		adc.l	<__stack + \1, x
		sta.l	<__stack + \1, x
		tya
		eor	#$FF
		adc.h	<__stack + \1, x
		sta.h	<__stack + \1, x
		.endm

; **************
; Y:A = memory - Y:A

__isub_st.usq	.macro	; __STACK
		ldx	<__sp
		sec
		eor	#$FF
		adc	<__stack + \1, x
		sta	<__stack + \1, x
		.endm

; **************
; Y:A = memory - Y:A

__isub_st.watq	.macro
		plx
		sec
		eor	#$FF
		adc.l	\1, x
		sta.l	\1, x
		tya
		eor	#$FF
		adc.h	\1, x
		sta.h	\1, x
		.endm

; **************
; Y:A = memory - Y:A

__isub_st.uatq	.macro
		plx
		sec
		eor	#$FF
		adc	\1, x
		sta	\1, x
		.endm

; **************
; Y:A = memory - Y:A

__isub_st.waxq	.macro
		sec
		eor	#$FF
		adc.l	\1, x
		sta.l	\1, x
		tya
		eor	#$FF
		adc.h	\1, x
		sta.h	\1, x
		.endm

; **************
; Y:A = memory - Y:A

__isub_st.uaxq	.macro
		sec
		eor	#$FF
		adc	\1, x
		sta	\1, x
		.endm

; **************

__and_st.wmq	.macro
		and.l	\1
		sta.l	\1
		tya
		and.h	\1
		sta.h	\1
		.endm

; **************

__and_st.umq	.macro
		and	\1
		sta	\1
		.endm

; **************

__and_st.wpq	.macro
		and	[\1]
		sta	[\1]
		tya
		ldy	#1
		and	[\1], y
		sta	[\1], y
		.endm

; **************

__and_st.upq	.macro
		and	[\1]
		sta	[\1]
		.endm

; **************

__and_st.wsq	.macro
		ldx	<__sp
		and.l	<__stack + \1, x
		sta.l	<__stack + \1, x
		tya
		and.h	<__stack + \1, x
		sta.h	<__stack + \1, x
		.endm

; **************

__and_st.usq	.macro
		ldx	<__sp
		and	<__stack + \1, x
		sta	<__stack + \1, x
		.endm

; **************

__and_st.watq	.macro
		plx
		and.l	\1, x
		sta.l	\1, x
		tya
		and.h	\1, x
		sta.h	\1, x
		.endm

; **************

__and_st.uatq	.macro
		plx
		and	\1, x
		sta	\1, x
		.endm

; **************

__and_st.waxq	.macro
		and.l	\1, x
		sta.l	\1, x
		tya
		and.h	\1, x
		sta.h	\1, x
		.endm

; **************

__and_st.uaxq	.macro
		and	\1, x
		sta	\1, x
		.endm

; **************

__eor_st.wmq	.macro
		eor.l	\1
		sta.l	\1
		tya
		eor.h	\1
		sta.h	\1
		.endm

; **************

__eor_st.umq	.macro
		eor	\1
		sta	\1
		.endm

; **************

__eor_st.wpq	.macro
		eor	[\1]
		sta	[\1]
		tya
		ldy	#1
		eor	[\1], y
		sta	[\1], y
		.endm

; **************

__eor_st.upq	.macro
		eor	[\1]
		sta	[\1]
		.endm

; **************

__eor_st.wsq	.macro
		ldx	<__sp
		eor.l	<__stack + \1, x
		sta.l	<__stack + \1, x
		tya
		eor.h	<__stack + \1, x
		sta.h	<__stack + \1, x
		.endm

; **************

__eor_st.usq	.macro
		ldx	<__sp
		eor	<__stack + \1, x
		sta	<__stack + \1, x
		.endm

; **************

__eor_st.watq	.macro
		plx
		eor.l	\1, x
		sta.l	\1, x
		tya
		eor.h	\1, x
		sta.h	\1, x
		.endm

; **************

__eor_st.uatq	.macro
		plx
		eor	\1, x
		sta	\1, x
		.endm

; **************

__eor_st.waxq	.macro
		eor.l	\1, x
		sta.l	\1, x
		tya
		eor.h	\1, x
		sta.h	\1, x
		.endm

; **************

__eor_st.uaxq	.macro
		eor	\1, x
		sta	\1, x
		.endm

; **************

__or_st.wmq	.macro
		ora.l	\1
		sta.l	\1
		tya
		ora.h	\1
		sta.h	\1
		.endm

; **************

__or_st.umq	.macro
		ora	\1
		sta	\1
		.endm

; **************

__or_st.wpq	.macro
		or	[\1]
		sta	[\1]
		tya
		ldy	#1
		or	[\1], y
		sta	[\1], y
		.endm

; **************

__or_st.upq	.macro
		or	[\1]
		sta	[\1]
		.endm

; **************

__or_st.wsq	.macro
		ldx	<__sp
		ora.l	<__stack + \1, x
		sta.l	<__stack + \1, x
		tya
		ora.h	<__stack + \1, x
		sta.h	<__stack + \1, x
		.endm

; **************

__or_st.usq	.macro
		ldx	<__sp
		ora	<__stack + \1, x
		sta	<__stack + \1, x
		.endm

; **************

__or_st.watq	.macro
		plx
		ora.l	\1, x
		sta.l	\1, x
		tya
		ora.h	\1, x
		sta.h	\1, x
		.endm

; **************

__or_st.uatq	.macro
		plx
		ora	\1, x
		sta	\1, x
		.endm

; **************

__or_st.waxq	.macro
		ora.l	\1, x
		sta.l	\1, x
		tya
		ora.h	\1, x
		sta.h	\1, x
		.endm

; **************

__or_st.uaxq	.macro
		ora	\1, x
		sta	\1, x
		.endm

; **************

__add_st.wmiq	.macro
	.if	((\?1 == ARG_ABS) && ((\1) == 1))
		inc.l	\2
		bne	!+
		inc.h	\2
!:
	.else
	.if	((\?1 == ARG_ABS) && ((\1) >= 0) && ((\1) < 256))
		clc
		lda.l	\2
		adc.l	#\1
		sta.l	\2
		bcc	!+
		inc.h	\2
!:
	.else
		clc
		lda.l	\2
		adc.l	#\1
		sta.l	\2
		lda.h	\2
		adc.h	#\1
		sta.h	\2
	.endif
	.endif
		.endm

; **************

__add_st.umiq	.macro
	.if	((\?1 == ARG_ABS) && ((\1) == 1))
		inc	\2
	.else
		clc
		lda	\2
		adc	#\1
		sta	\2
	.endif
		.endm

; **************

__add_st.wpiq	.macro
		clc
		lda	[\2]
		adc.l	#\1
		sta	[\2]
		ldy	#1
		lda	[\2], y
		adc.h	#\1
		sta	[\2], y
		.endm

; **************

__add_st.upiq	.macro
		clc
		lda	[\2]
		adc	#\1
		sta	[\2]
		.endm

; **************

__add_st.wsiq	.macro	; __STACK
		ldx	<__sp
	.if	((\?1 == ARG_ABS) && ((\1) == 1))
		inc.l	<__stack + \2, x
		bne	!+
		inc.h	<__stack + \2, x
!:
	.else
	.if	((\?1 == ARG_ABS) && ((\1) >= 0) && ((\1) < 256))
		clc
		lda.l	<__stack + \2, x
		adc.l	#\1
		sta.l	<__stack + \2, x
		bcc	!+
		inc.h	<__stack + \2, x
!:
	.else
		clc
		lda.l	<__stack + \2, x
		adc.l	#\1
		sta.l	<__stack + \2, x
		lda.h	<__stack + \2, x
		adc.h	#\1
		sta.h	<__stack + \2, x
	.endif
	.endif
		.endm

; **************

__add_st.usiq	.macro	; __STACK
		ldx	<__sp
	.if	((\?1 == ARG_ABS) && ((\1) == 1))
		inc	<__stack + \2, x
	.else
		clc
		lda	<__stack + \2, x
		adc	#\1
		sta	<__stack + \2, x
	.endif
		.endm

; **************

__add_st.watiq	.macro	; __STACK
		plx
	.if	((\?1 == ARG_ABS) && ((\1) == 1))
		inc.l	\2, x
		bne	!+
		inc.h	\2, x
!:
	.else
	.if	((\?1 == ARG_ABS) && ((\1) >= 0) && ((\1) < 256))
		clc
		lda.l	\2, x
		adc.l	#\1
		sta.l	\2, x
		bcc	!+
		inc.h	\2, x
!:
	.else
		clc
		lda.l	\2, x
		adc.l	#\1
		sta.l	\2, x
		lda.h	\2, x
		adc.h	#\1
		sta.h	\2, x
	.endif
	.endif
		.endm

; **************

__add_st.uatiq	.macro	; __STACK
		plx
	.if	((\?1 == ARG_ABS) && ((\1) == 1))
		inc	\2, x
	.else
		clc
		lda	\2, x
		adc	#\1
		sta	\2, x
	.endif
		.endm

; **************

__add_st.waxiq	.macro	; __STACK
	.if	((\?1 == ARG_ABS) && ((\1) == 1))
		inc.l	\2, x
		bne	!+
		inc.h	\2, x
!:
	.else
	.if	((\?1 == ARG_ABS) && ((\1) >= 0) && ((\1) < 256))
		clc
		lda.l	\2, x
		adc.l	#\1
		sta.l	\2, x
		bcc	!+
		inc.h	\2, x
!:
	.else
		clc
		lda.l	\2, x
		adc.l	#\1
		sta.l	\2, x
		lda.h	\2, x
		adc.h	#\1
		sta.h	\2, x
	.endif
	.endif
		.endm

; **************

__add_st.uaxiq	.macro	; __STACK
	.if	((\?1 == ARG_ABS) && ((\1) == 1))
		inc	\2, x
	.else
		clc
		lda	\2, x
		adc	#\1
		sta	\2, x
	.endif
		.endm

; **************

__sub_st.wmiq	.macro
	.if	((\?1 == ARG_ABS) && ((\1) == 1))
		lda.l	\2
		bne	!+
		dec.h	\2
!:		dec.l	\2
	.else
	.if	((\?1 == ARG_ABS) && ((\1) >= 0) && ((\1) < 256))
		sec
		lda.l	\2
		sbc.l	#\1
		sta.l	\2
		bcs	!+
		dec.h	\2
!:
	.else
		sec
		lda.l	\2
		sbc.l	#\1
		sta.l	\2
		lda.h	\2
		sbc.h	#\1
		sta.h	\2
	.endif
	.endif
		.endm

; **************

__sub_st.umiq	.macro
	.if	((\?1 == ARG_ABS) && ((\1) == 1))
		dec	\2
	.else
		sec
		lda	\2
		sbc	#\1
		sta	\2
	.endif
		.endm

; **************

__sub_st.wpiq	.macro
		sec
		lda	[\2]
		sbc.l	#\1
		sta	[\2]
		ldy	#1
		lda	[\2], y
		sbc.h	#\1
		sta	[\2], y
		.endm

; **************

__sub_st.upiq	.macro
		sec
		lda	[\2]
		sbc	#\1
		sta	[\2]
		.endm

; **************

__sub_st.wsiq	.macro	; __STACK
		ldx	<__sp
	.if	((\?1 == ARG_ABS) && ((\1) == 1))
		lda.l	<__stack + \2, x
		bne	!+
		dec.h	<__stack + \2, x
!:		dec.l	<__stack + \2, x
	.else
	.if	((\?1 == ARG_ABS) && ((\1) >= 0) && ((\1) < 256))
		sec
		lda.l	<__stack + \2, x
		sbc.l	#\1
		sta.l	<__stack + \2, x
		bcs	!+
		dec.h	<__stack + \2, x
!:
	.else
		sec
		lda.l	<__stack + \2, x
		sbc.l	#\1
		sta.l	<__stack + \2, x
		lda.h	<__stack + \2, x
		sbc.h	#\1
		sta.h	<__stack + \2, x
	.endif
	.endif
		.endm

; **************

__sub_st.usiq	.macro	; __STACK
		ldx	<__sp
	.if	((\?1 == ARG_ABS) && ((\1) == 1))
		dec	<__stack + \2, x
	.else
		sec
		lda	<__stack + \2, x
		sbc	#\1
		sta	<__stack + \2, x
	.endif
		.endm

; **************

__sub_st.watiq	.macro	; __STACK
		plx
	.if	((\?1 == ARG_ABS) && ((\1) == 1))
		lda.l	\2, x
		bne	!+
		dec.h	\2, x
!:		dec.l	\2, x
	.else
	.if	((\?1 == ARG_ABS) && ((\1) >= 0) && ((\1) < 256))
		sec
		lda.l	\2, x
		sbc.l	#\1
		sta.l	\2, x
		bcs	!+
		dec.h	\2, x
!:
	.else
		sec
		lda.l	\2, x
		sbc.l	#\1
		sta.l	\2, x
		lda.h	\2, x
		sbc.h	#\1
		sta.h	\2, x
	.endif
	.endif
		.endm

; **************

__sub_st.uatiq	.macro	; __STACK
		plx
	.if	((\?1 == ARG_ABS) && ((\1) == 1))
		dec	\2, x
	.else
		sec
		lda	\2, x
		sbc	#\1
		sta	\2, x
	.endif
		.endm

; **************

__sub_st.waxiq	.macro	; __STACK
	.if	((\?1 == ARG_ABS) && ((\1) == 1))
		lda.l	\2, x
		bne	!+
		dec.h	\2, x
!:		dec.l	\2, x
	.else
	.if	((\?1 == ARG_ABS) && ((\1) >= 0) && ((\1) < 256))
		sec
		lda.l	\2, x
		sbc.l	#\1
		sta.l	\2, x
		bcs	!+
		dec.h	\2, x
!:
	.else
		sec
		lda.l	\2, x
		sbc.l	#\1
		sta.l	\2, x
		lda.h	\2, x
		sbc.h	#\1
		sta.h	\2, x
	.endif
	.endif
		.endm

; **************

__sub_st.uaxiq	.macro	; __STACK
	.if	((\?1 == ARG_ABS) && ((\1) == 1))
		dec	\2, x
	.else
		sec
		lda	\2, x
		sbc	#\1
		sta	\2, x
	.endif
		.endm



; ***************************************************************************
; ***************************************************************************
; i-codes for 32-bit longs
; ***************************************************************************
; ***************************************************************************

; **************

__ldd_i		.macro
		lda.l	#(\1) >> 16
		sta.l	<\3
		lda.h	#(\1) >> 16
		sta.h	<\3
		lda.l	#\1
		sta.l	<\2
		ldy.h	#\1
		sty.h	<\2
		.endm

; **************

__ldd_w		.macro
		lda.l	\1
		ldy.h	\1
		sta.l	<\2
		sty.h	<\2
		stz.l	<\3
		stz.h	<\3
		.endm

; **************

__ldd_b		.macro
		lda	\1
		cly
		sta.l	<\2
		stz.h	<\2
		stz.l	<\3
		stz.h	<\3
		.endm

; **************

__ldd_s_w	.macro	; __STACK
		lda.l	<__stack + \1, x
		ldy.h	<__stack + \1, x
		sta.l	<\2
		sty.h	<\2
		stz.l	<\3
		stz.h	<\3
		.endm
; **************

__ldd_s_b	.macro	; __STACK
		lda.l	<__stack + \1, x
		cly
		bpl	!+
		dey
!:		sta.l	<\2
		sty.h	<\2
		stz.l	<\3
		stz.h	<\3
		.endm



		.list

; ***************************************************************************
; ***************************************************************************
; subroutines for logical and arithmetic shifts by a constant amount
; ***************************************************************************
; ***************************************************************************

; **************
; Y:A = Y:A << const

aslw15:		asl	a
aslw14:		asl	a
aslw13:		asl	a
aslw12:		asl	a
aslw11:		asl	a
aslw10:		asl	a
aslw9:		asl	a
aslw8:		tay
		cla
		rts

	.if	1
aslw5:		ror	<__temp
		ror	a
aslw6:		ror	<__temp
		ror	a
aslw7:		ror	<__temp
		ror	a
		ldy	<__temp
		rts
	.else
aslw7:		asl	a
		rol	<__temp
aslw6:		asl	a
		rol	<__temp
aslw5:		asl	a
		rol	<__temp
	.endif
aslw4:		asl	a
		rol	<__temp
aslw3:		asl	a
		rol	<__temp
aslw2:		asl	a
		rol	<__temp
aslw1:		asl	a
		rol	<__temp
aslw0:		ldy	<__temp
		rts

; **************
; Y:A = Y:A >> const

asrw15:		cmp	#$80
		ror	a
asrw14:		cmp	#$80
		ror	a
asrw13:		cmp	#$80
		ror	a
asrw12:		cmp	#$80
		ror	a
asrw11:		cmp	#$80
		ror	a
asrw10:		cmp	#$80
		ror	a
asrw9:		cmp	#$80
		ror	a
asrw8:		cmp	#$80
		cly
		bcc	!+
		dey
!:		rts

asrw7:		cpy	#$80
		ror	<__temp
		ror	a
asrw6:		cpy	#$80
		ror	<__temp
		ror	a
asrw5:		cpy	#$80
		ror	<__temp
		ror	a
asrw4:		cpy	#$80
		ror	<__temp
		ror	a
asrw3:		cpy	#$80
		ror	<__temp
		ror	a
asrw2:		cpy	#$80
		ror	<__temp
		ror	a
asrw1:		cpy	#$80
		ror	<__temp
		ror	a
asrw0:		ldy	<__temp
		rts

; **************
; Y:A = Y:A >> const

lsrw15:		lsr	a
lsrw14:		lsr	a
lsrw13:		lsr	a
lsrw12:		lsr	a
lsrw11:		lsr	a
lsrw10:		lsr	a
lsrw9:		lsr	a
lsrw8:		cly
		rts

lsrw7:		lsr	<__temp
		ror	a
lsrw6:		lsr	<__temp
		ror	a
lsrw5:		lsr	<__temp
		ror	a
lsrw4:		lsr	<__temp
		ror	a
lsrw3:		lsr	<__temp
		ror	a
lsrw2:		lsr	<__temp
		ror	a
lsrw1:		lsr	<__temp
		ror	a
lsrw0:		ldy	<__temp
		rts



; ***************************************************************************
; ***************************************************************************
; subroutines for logical and arithmetic shifts by a variable amount
; ***************************************************************************
; ***************************************************************************

; **************
; Y:A = Y:A << X

asl.wx:		sty	<__temp
		cpx	#16
		bcs	!zero+
		dex
		bmi	.done
.loop:		asl	a
		rol	<__temp
		dex
		bpl	.loop
.done:		ldy	<__temp
		rts

; **************
; Y:A = Y:A >> X

asr.wx:		sty	<__temp
		bpl	!positive+
!negative:	cpx	#16
		bcs	.sign
		dex
		bmi	.done
.loop:		sec
		ror	<__temp
		ror	a
		dex
		bpl	.loop
.done:		ldy	<__temp
		rts

.sign:		lda	#$FF
		tay
		rts

; **************
; Y:A = Y:A >> X

lsr.wx:		sty	<__temp
!positive:	cpx	#16
		bcs	!zero+
		dex
		bmi	.done
.loop:		lsr	<__temp
		ror	a
		dex
		bpl	.loop
.done:		ldy	<__temp
		rts

!zero:		cla
		cly
		rts



; ***************************************************************************
; ***************************************************************************
; POTENTIAL OPTIMIZATIONS, NOT YET ADDED
; ***************************************************************************
; ***************************************************************************
