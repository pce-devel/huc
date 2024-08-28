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
; adds offset in Y:A to data address in \1
; if 1-param then Y=bank, __fptr=addr, used for farpeekb() and farpeekw()
; if 3-param then \2=bank, \3=addr

__farptr_i	.macro
		clc
		adc.l	#(\1) & $1FFF
	.if (\# = 3)
		sta.l	\3
	.else
		sta.l	<__fptr
	.endif
		tya
		adc.h	#(\1) & $1FFF
		tay
		and	#$1F
		ora	#$60
	.if (\# = 3)
		sta.h	\3
	.else
		sta.h	<__fptr
	.endif
		tya
		rol	a
		rol	a
		rol	a
		rol	a
		and	#$0F
		clc
		adc	#bank(\1)
	.if (\# = 3)
		sta	\2
	.else
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
; unsigned int __fastcall __xsafe farpeekw( void far *base<__fbank:__fptr> );
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
; the compiler can generate this to aid debugging C data-stack
; balance problems before/after a function call

__calling	.macro
		phx
		.endm

; **************
; the compiler can generate this to aid debugging C data-stack
; balance problems before/after a function call

__called	.macro
		stx	<__sp
		plx
		cpx	<__sp
!hang:		bne	!hang-
		.endm

; **************

__call		.macro
		call	\1
		.endm

; **************

__callp		.macro
		sta.l	<__ptr
		sty.h	<__ptr
		jsr	call_indirect
		.endm

call_indirect:	jmp	[__ptr]



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

__leave		.macro
	.if	(\1 != 0)
		sta	<__hucc_ret
	.endif
		jmp	leave_proc
		.endm

; **************
; get the acc lo-byte after returning from a function

__getacc	.macro
		lda	<__hucc_ret
		.endm

; **************
; main C data-stack for arguments/locals/expressions
; used when entering a #asm section that is not __xsafe

__savesp	.macro	; __STACK
		phx
		.endm

; **************
; main C data-stack for arguments/locals/expressions
; used when leaving a #asm section that is not __xsafe

__loadsp	.macro	; __STACK
		plx
		.endm

; **************
; main C data-stack for arguments/locals/expressions

__modsp		.macro
	.if (\1 < 0)

	.if (\1 == -2)
		dex
		dex
	.else
	.if (\1 == -4)
		dex
		dex
		dex
		dex
	.else
		sax
		clc
		adc.l	#\1
		sax
	.endif
	.endif
	.if	HUCC_DEBUG_SP
!overflow:	bmi	!overflow-
	.endif

	.else

	.if (\1 == 2)
		inx
		inx
	.else
	.if (\1 == 4)
		inx
		inx
		inx
		inx
	.else
		sax
		clc
		adc.l	#\1
		sax
	.endif
	.endif

	.endif
		.endm

; **************
; main C data-stack for arguments/locals/expressions
; this is used to adjust the stack in a C "goto"

__modsp_sym	.macro
		sax
		clc
		adc.l	#\1
		sax
		.endm

; **************
; main C data-stack for arguments/locals/expressions

__pushw		.macro	; __STACK
		dex
		dex
	.if	HUCC_DEBUG_SP
!overflow:	bmi	!overflow-
	.endif
		sta.l	<__stack, x
		sty.h	<__stack, x
		.endm

; **************
; main C data-stack for arguments/locals/expressions

__popw		.macro	; __STACK
		lda.l	<__stack, x
		ldy.h	<__stack, x
		inx
		inx
		.endm

; **************
; hardware-stack used for *temporary* storage of spilled arguments
__spushw	.macro
		phy
		pha
		.endm

; **************
; hardware-stack used for *temporary* storage of spilled arguments

__spopw		.macro
		pla
		ply
		.endm

; **************
; hardware-stack used for *temporary* storage of spilled arguments

__spushb	.macro
		pha
		.endm

; **************
; hardware-stack used for *temporary* storage of spilled arguments

__spopb		.macro
		pla
		.endm



; ***************************************************************************
; ***************************************************************************
; i-codes for handling boolean tests and branching
; ***************************************************************************
; ***************************************************************************

; **************

__switchw	.macro
		pha
		lda.l	#\1
		sta.l	__ptr
		lda.h	#\1
		sta.h	__ptr
		pla
		jmp	do_switchw
		.endm

; **************

__switchb	.macro
		ldy.l	#\1
		sty.l	__ptr
		ldy.h	#\1
		sty.h	__ptr
		jmp	do_switchb
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
; boolean test, always followed by a __tstw or __notw
; this MUST set the Z flag for the susequent branches!

__cmpw		.macro
		jsr	\1
		.endm

; **************
; boolean test, always followed by a __tstw or __notw
; this MUST set the Z flag for the susequent branches!

__cmpb		.macro
		jsr	\1
		.endm

; **************
; optimized boolean test
; this MUST set the Z flag for the susequent branches!

__cmpwi_eq	.macro
		eor.l	#\1
		bne	!false+
		cpy.h	#\1
		beq	!true+
!false:		lda	#-1
!true:		inc	a
		cly
		.endm

; **************
; optimized boolean test
; this MUST set the Z flag for the susequent branches!

__cmpwi_ne	.macro
		eor.l	#\1
		bne	!true+
		cpy.h	#\1
		beq	!false+
!true:		lda	#1
!false:		cly
		.endm

; **************
; optimized boolean test
; this MUST set the Z flag for the susequent branches!

__cmpbi_eq	.macro
		eor.l	#\1
		beq	!true+
!false:		lda	#-1
!true:		inc	a
		cly
		.endm

; **************
; optimized boolean test
; this MUST set the Z flag for the susequent branches!

__cmpbi_ne	.macro
		eor.l	#\1
		beq	!false+
		lda	#1
!false:		cly
		.endm

; **************
; boolean test, optimized into __notw if used before a __tstw
; this MUST set the Z flag for the susequent branches!

__notw		.macro
		sty	<__temp
		ora	<__temp
		beq	!+
		lda	#1
!:		eor	#1
		cly
		.endm

; **************
; boolean test, always output immediately before a __bfalse or __btrue
; this MUST set the Z flag for the susequent branches!

__tstw		.macro
		sty	<__temp
		ora	<__temp
		beq	!+
		lda	#1
!:		cly
		.endm

; **************
; always preceeded by a __tstw before peephole optimization

__bfalse	.macro
		beq	\1
		.endm

; **************
; always preceeded by a __tstw before peephole optimization

__btrue		.macro
		bne	\1
		.endm

; **************
; optimized boolean test
; this MUST set the Z flag for the susequent branches!

__tzb		.macro
		lda	\1
		beq	!+
		lda	#-1
!:		inc	a
		cly
		.endm

; **************
; optimized boolean test
; this MUST set the Z flag for the susequent branches!

__tzbp		.macro
		lda	[\1]
		beq	!+
		lda	#-1
!:		inc	a
		cly
		.endm

; **************
; optimized boolean test
; this MUST set the Z flag for the susequent branches!

__tzb_s		.macro	; __STACK
		lda.l	<__stack + \1, x
		beq	!+
		lda	#-1
!:		inc	a
		cly
		.endm

; **************
; optimized boolean test
; this MUST set the Z flag for the susequent branches!

__tzw		.macro
		lda.l	\1
		ora.h	\1
		beq	!+
		lda	#-1
!:		inc	a
		cly
		.endm

; **************
; optimized boolean test
; this MUST set the Z flag for the susequent branches!

__tzwp		.macro
		ldy	#1
		lda	[\1], y
		ora	[\1]
		beq	!+
		lda	#-1
!:		inc	a
		cly
		.endm

; **************
; optimized boolean test
; this MUST set the Z flag for the susequent branches!

__tzw_s		.macro	; __STACK
		lda.l	<__stack + \1, x
		ora.h	<__stack + \1, x
		beq	!+
		lda	#-1
!:		inc	a
		cly
		.endm

; **************
; optimized boolean test
; this MUST set the Z flag for the susequent branches!

__tnzb		.macro
		lda	\1
		beq	!+
		lda	#1
!:		cly
		.endm

; **************
; optimized boolean test
; this MUST set the Z flag for the susequent branches!

__tnzbp		.macro
		lda	[\1]
		beq	!+
		lda	#1
!:		cly
		.endm

; **************
; optimized boolean test
; this MUST set the Z flag for the susequent branches!

__tnzb_s	.macro	; __STACK
		lda.l	<__stack + \1, x
		beq	!+
		lda	#1
!:		cly
		.endm

; **************
; optimized boolean test
; this MUST set the Z flag for the susequent branches!

__tnzw		.macro
		lda.l	\1
		ora.h	\1
		beq	!+
		lda	#1
!:		cly
		.endm

; **************
; optimized boolean test
; this MUST set the Z flag for the susequent branches!

__tnzwp		.macro
		ldy	#1
		lda	[\1], y
		ora	[\1]
		beq	!+
		lda	#1
!:		cly
		.endm

; **************
; optimized boolean test
; this MUST set the Z flag for the susequent branches!

__tnzw_s	.macro	; __STACK
		lda.l	<__stack + \1, x
		ora.h	<__stack + \1, x
		beq	!+
		lda	#1
!:		cly
		.endm



; ***************************************************************************
; ***************************************************************************
; i-codes for loading the primary register
; ***************************************************************************
; ***************************************************************************

; **************

__ldwi		.macro
	.if	((\?1 == ARG_ABS) && (\1 >= 0) && (\1 < 256))
		lda.l	#\1
		cly
	.else
		lda.l	#\1
		ldy.h	#\1
	.endif
		.endm

; **************

__ldw		.macro
		lda.l	\1
		ldy.h	\1
		.endm

; **************

__ldb		.macro
		lda	\1
		cly
		bpl	!+
		dey
!:
		.endm

; **************

__ldub		.macro
		lda	\1
		cly
		.endm

; **************

__ldwp		.macro
		ldy	#1
		lda	[\1], y
		tay
		lda	[\1]
		.endm

; **************

__ldbp		.macro
		lda	[\1]
		cly
		bpl	!+
		dey
!:
		.endm

; **************

__ldubp		.macro
		lda	[\1]
		cly
		.endm



; ***************************************************************************
; ***************************************************************************
; i-codes for saving the primary register
; ***************************************************************************
; ***************************************************************************

; **************

__stwz		.macro
		stz.l	\1
		stz.h	\1
		.endm

; **************

__stbz		.macro
		stz	\1
		.endm

; **************

__stwi		.macro
		lda.l	#\2
		sta.l	\1
		ldy.h	#\2
		sty.h	\1
		.endm

; **************

__stbi		.macro
		lda.l	#\2
		sta	\1
		.endm

; **************

__stwip		.macro
		sta.l	<__ptr
		sty.h	<__ptr
		lda.h	#\1
		ldy	#1
		sta	[__ptr], y
		tay
		lda.l	#\1
		sta	[__ptr]
		.endm

; **************

__stbip		.macro
		sta.l	<__ptr
		sty.h	<__ptr
		lda.l	#\1
		sta	[__ptr]
		cly
		.endm

; **************

__stw		.macro
		sta.l	\1
		sty.h	\1
		.endm

; **************

__stb		.macro
		sta	\1
		.endm

; **************

__stwp		.macro
		sta	[\1]
		pha
		tya
		ldy	#1
		sta	[\1], y
		tay
		pla
		.endm

; **************

__stbp		.macro
		sta	[\1]
		.endm

; **************

__stwps		.macro	; __STACK
		sta	[__stack, x]
		inc.l	<__stack, x
		bne	!+
		inc.h	<__stack, x
!:		say
		sta	[__stack, x]
		say
		inx
		inx
		.endm

; **************

__stbps		.macro	; __STACK
		sta	[__stack, x]
		inx
		inx
		.endm



; ***************************************************************************
; ***************************************************************************
; i-codes for extending the primary register
; ***************************************************************************
; ***************************************************************************

; **************

__extw		.macro
		tay
		cly
		bpl	!+
		dey
!:
		.endm

; **************

__extuw		.macro
		cly
		.endm



; ***************************************************************************
; ***************************************************************************
; i-codes for math with the primary register
; ***************************************************************************
; ***************************************************************************

; **************

__comw		.macro
		eor	#$FF
		say
		eor	#$FF
		say
		.endm

; **************

__negw		.macro
		sec
		eor	#$FF
		adc	#0
		say
		eor	#$FF
		adc	#0
		say
		.endm

; **************

__incw		.macro
		inc.l	\1
		bne	!+
		inc.h	\1
!:
		.endm

; **************

__incb		.macro
		inc	\1
		.endm

;	; ************** NEW and UNIMPLEMENTED!
;
;	__decw_		.macro
;			lda.l	\1
;			bne	!+
;			dec.h	\1
;	!:		dec.l	\1
;			.endm
;
;	; ************** NEW and UNIMPLEMENTED!
;
;	__decwa_	.macro
;			cmp	#0
;			bne	!+
;			dey
;	!:		dec	a
;			.endm
;
;	__decwa2_	.macro
;			sec
;			sbc	#2
;			say
;			sbc	#0
;			say
;			.endm
;
;	__decwm_	.macro
;			lda.l	\1
;			bne	!+
;			dec.h	\1
;	!:		dec.l	\1
;			.endm
;
;	__decwm2_	.macro
;			lda.l	\1
;			bne	!+
;			dec.h	\1
;	!:		dec.l	\1
;			bne	!+
;			dec.h	\1
;	!:		dec.l	\1
;			.endm
;
;	__decws_	.macro
;			lda.l	<__stack, x
;			bne	!+
;			dec.h	<__stack, x
;	!:		dec.l	<__stack, x
;			.endm
;
;	__decws2_	.macro
;			lda.l	<__stack, x
;			bne	!+
;			dec.h	<__stack, x
;	!:		dec.l	<__stack, x
;			bne	!+
;			dec.h	<__stack, x
;	!:		dec.l	<__stack, x
;			.endm

; **************

__addwi		.macro
	.if (\1 == 0)
	.else
	.if ((\1 >= 0) && (\1 < 256))
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
; pceas workaround; the regular __addwi doesn't work if the argument is
; symbolic because the code size changes as it is resolved.
; **************

__addwi_sym	.macro
		clc
		adc.l	#\1
		say
		adc.h	#\1
		say
		.endm

; **************

__addbi		.macro
	.if (\1 = 1)
		inc	a
	.else
	.if (\1 = 2)
		inc	a
		inc	a
	.else
		clc
		adc	#\1
	.endif
	.endif
		cly
		.endm

; **************

__addws		.macro	; __STACK
		clc
		adc.l	<__stack, x
		say
		adc.h	<__stack, x
		say
		inx
		inx
		.endm

; **************

__addbs		.macro	; __STACK
		clc
		adc.l	<__stack, x
		cly
		inx
		inx
		.endm

; **************

__addw		.macro
		clc
		adc.l	\1
		say
		adc.h	\1
		say
		.endm

; **************

__addb		.macro
		clc
		adc	\1
		bcc	!+
		iny
!:		bit	\1
		bpl	!+
		dey
!:
		.endm

; **************

__addub		.macro
		clc
		adc	\1
		bcc	!+
		iny
!:
		.endm

; **************

__addbi_p	.macro
		sta.l	<__ptr
		sty.h	<__ptr
		lda	[__ptr]
		clc
		adc	#\1
		sta	[__ptr]
		cly
		.endm

; **************

__subwi		.macro
	.if (\1 == 0)
	.else
	.if ((\1 >= 0) && (\1 < 256))
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
	.endif
		.endm

; **************

__subws		.macro	; __STACK
		sec
		eor	#$FF
		adc.l	<__stack, x
		say
		eor	#$FF
		adc.h	<__stack, x
		say
		inx
		inx
		.endm

; **************

__subw		.macro
		sec
		sbc.l	\1
		say
		sbc.h	\1
		say
		.endm

; **************

__andwi		.macro
	.if	(\1 < 256)
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
		.endm

; **************

__andws		.macro	; __STACK
		and.l	<__stack, x
		say
		and.h	<__stack, x
		say
		inx
		inx
		.endm

;	; **************
;
;	__andw		.macro
;			and.l	\1
;			say
;			and.h	\1
;			say
;			.endm

; **************

__eorwi		.macro
		eor.l	#\1
		say
		eor.h	#\1
		say
		.endm

; **************

__eorws		.macro	; __STACK
		eor.l	<__stack, x
		say
		eor.h	<__stack, x
		say
		inx
		inx
		.endm

;	; **************
;
;	__eorw		.macro
;			eor.l	\1
;			say
;			eor.h	\1
;			say
;			.endm

; **************

__orwi		.macro
		ora.l	#\1
		say
		ora.h	#\1
		say
		.endm

; **************

__orws		.macro	; __STACK
		ora.l	<__stack, x
		say
		ora.h	<__stack, x
		say
		inx
		inx
		.endm

;	; **************
;
;	__orw		.macro
;			ora.l	\1
;			say
;			ora.h	\1
;			say
;			.endm

; **************

__aslwi		.macro
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
	.if (\1 < 8)
		sty	<__temp
		jsr	aslw\1
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
		.endm

; **************
; N.B. Used when calculating a pointer into a word array.

__aslws		.macro
		asl.l	<__stack, x
		rol.h	<__stack, x
		.endm

; **************

__aslw		.macro
		asl	a
		say
		rol	a
		say
		.endm

; **************

__asrwi		.macro
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
		sty	<__temp
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

__asrw		.macro
		cpy	#$80
		say
		ror	a
		say
		ror	a
		.endm

; **************

__lsrwi		.macro
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
		sty	<__temp
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

;	; **************
;
;	__lsrw		.macro
;			say
;			lsr	a
;			say
;			ror	a
;			.endm

; **************

__mulwi		.macro
	.if (\1 = 2)
		__aslw
	.else
	.if (\1 = 3)
		sta.l	<__temp
		sty.h	<__temp
		__aslw
		__addw	<__temp
	.else
	.if (\1 = 4)
		__aslw
		__aslw
	.else
	.if (\1 = 5)
		sta.l	<__temp
		sty.h	<__temp
		__aslw
		__aslw
		__addw	<__temp
	.else
	.if (\1 = 6)
		__aslw
		sta.l	<__temp
		sty.h	<__temp
		__aslw
		__addw	<__temp
	.else
	.if (\1 = 7)
		sta.l	<__temp
		sty.h	<__temp
		__aslw
		__aslw
		__aslw
		__subw	<__temp
	.else
	.if (\1 = 8)
		__aslw
		__aslw
		__aslw
	.else
	.if (\1 = 9)
		sta.l	<__temp
		sty.h	<__temp
		__aslw
		__aslw
		__aslw
		__addw	<__temp
	.else
	.if (\1 = 10)
		__aslw
		sta.l	<__temp
		sty.h	<__temp
		__aslw
		__aslw
		__addw	<__temp
	.else
		__pushw
		__ldwi	\1
		jsr	umul
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



; ***************************************************************************
; ***************************************************************************
; optimized i-codes for local variables on the C stack
; ***************************************************************************
; ***************************************************************************

; **************

__lea_s		.macro	; __STACK
		txa
		clc
		adc.l	#__stack + (\1)
		ldy.h	#__stack
		.endm

; **************

__pea_s		.macro	; __STACK
		txa
		clc
		adc.l	#__stack + (\1)
		ldy.h	#__stack
		dex
		dex
		sta.l	<__stack, x
		sty.h	<__stack, x
		.endm

; **************

__ldw_s		.macro	; __STACK
		lda.l	<__stack + \1, x
		ldy.h	<__stack + \1, x
		.endm

; **************

__ldb_s		.macro	; __STACK
		lda.l	<__stack + \1, x
		cly
		bpl	!+	; signed
		dey
!:
		.endm

; **************

__ldub_s	.macro	; __STACK
		lda	<__stack + \1, x
		cly
		.endm

; **************

__stwi_s	.macro	; __STACK
		lda.l	#\1
		sta.l	<__stack + \2, x
		ldy.h	#\1
		sty.h	<__stack + \2, x
		.endm

; **************

__stbi_s	.macro	; __STACK
		lda.l	#\1
		sta.l	<__stack + \2, x
		cly
		.endm

; **************

__stw_s		.macro	; __STACK
		sta.l	<__stack + \1, x
		sty.h	<__stack + \1, x
		.endm

; **************

__stb_s		.macro	; __STACK
		sta.l	<__stack + \1, x
		.endm

; **************

__addw_s	.macro	; __STACK
		clc
		adc.l	<__stack + \1, x
		say
		adc.h	<__stack + \1, x
		say
		.endm

; **************

__addb_s	.macro	; __STACK
		bit	<__stack + \1, x
		bpl	!+
		dey
!:		clc
		adc	<__stack + \1, x
		bcc	!+
		iny
!:
		.endm

; **************

__addub_s	.macro	; __STACK
		clc
		adc	<__stack + \1, x
		bcc	!+
		iny
!:
		.endm

; **************

__incw_s	.macro	; __STACK
		inc.l	<__stack + \1, x
		bne	!+
		inc.h	<__stack + \1, x
!:
		.endm

; **************

__incb_s	.macro	; __STACK
		inc	<__stack + \1, x
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



; ***************************************************************************
; ***************************************************************************
; subroutines for comparison tests with signed and unsigned words
; ***************************************************************************
; ***************************************************************************

; **************
; Y:A is true if stacked-value == Y:A, else false

eq_w:		cmp.l	<__stack, x
		bne	return_false
		say
		cmp.h	<__stack, x
		say
		bne	return_false
		bra	return_true

; **************
; Y:A is true if stacked-value != Y:A, else false

ne_w:		cmp.l	<__stack, x
		bne	return_true
		say
		cmp.h	<__stack, x
		say
		bne	return_true
		bra	return_false

; **************
; Y:A is true if stacked-value < Y:A, else false

lt_sw:		clc			; Subtract memory+1 from Y:A.
		sbc.l	<__stack, x
		tya
		sbc.h	<__stack, x
		bvc	!+
		eor	#$80
!:		bpl	return_true	; +ve if Y:A  > memory (signed).
		bra	return_false	; -ve if Y:A <= memory (signed).

; **************
; Y:A is true if stacked-value >= Y:A, else false

ge_sw:		clc			; Subtract memory+1 from Y:A.
		sbc.l	<__stack, x
		tya
		sbc.h	<__stack, x
		bvc	!+
		eor	#$80
!:		bmi	return_true	; -ve if Y:A <= memory (signed).
		bra	return_false	; +ve if Y:A  > memory (signed).

; **************
; Y:A is true if stacked-value <= Y:A, else false

le_sw:		cmp.l	<__stack, x	; Subtract memory from Y:A.
		tya
		sbc.h	<__stack, x
		bvc	!+
		eor	#$80
!:		bpl	return_true	; +ve if Y:A >= memory (signed).
		bra	return_false	; -ve if Y:A  < memory (signed).

; **************
; Y:A is true if stacked-value > Y:A, else false

gt_sw:		cmp.l	<__stack, x	; Subtract memory from Y:A.
		tya
		sbc.h	<__stack, x
		bvc	!+
		eor	#$80
!:		bmi	return_true	; -ve if Y:A  < memory (signed).
		bra	return_false	; +ve if Y:A >= memory (signed).

; **************
; Y:A is true if stacked-value < Y:A, else false

lt_uw:		clc			; Subtract memory+1 from Y:A.
		sbc.l	<__stack, x
		tya
		sbc.h	<__stack, x
		bcs	return_true	; CS if Y:A  > memory.
		bra	return_false

; **************
; Y:A is true if stacked-value >= Y:A, else false

ge_uw:		clc			; Subtract memory+1 from Y:A.
		sbc.l	<__stack, x
		tya
		sbc.h	<__stack, x
		bcc	return_true	; CC if Y:A <= memory.
		bra	return_false

; **************
; Y:A is true if stacked-value <= Y:A, else false

le_uw:		cmp.l	<__stack, x	; Subtract memory from Y:A.
		tya
		sbc.h	<__stack, x
		bcs	return_true	; CS if Y:A >= memory.
		bra	return_false

; **************
; Y:A is true if stacked-value > Y:A, else false

gt_uw:		cmp.l	<__stack, x	; Subtract memory from Y:A.
		tya
		sbc.h	<__stack, x
		bcc	return_true	; CC if Y:A  < memory.
		bra	return_false

; **************
; boolean result, this MUST set the Z flag for the susequent branches!

return_true:	inx
		inx			; don't push Y:A, they are thrown away
		cly
		lda	#1		; Also set valid Z flag.
		rts

; **************
; boolean result, this MUST set the Z flag for the susequent branches!

return_false:	inx
		inx			; don't push Y:A, they are thrown away
		cly
		lda	#0		; Also set valid Z flag.
		rts



; ***************************************************************************
; ***************************************************************************
; subroutines for comparison tests with signed and unsigned bytes
; ***************************************************************************
; ***************************************************************************

; **************
; Y:A is true if stacked-value == Y:A, else false

eq_b:		cmp	<__stack, x
		bne	return_false
		bra	return_true

; **************
; Y:A is true if stacked-value != Y:A, else false

ne_b:		cmp	<__stack, x
		bne	return_true
		bra	return_false

; **************
; Y:A is true if stacked-value < Y:A, else false

lt_sb:		clc			; Subtract memory+1 from A.
		sbc.l	<__stack, x
		bvc	!+
		eor	#$80
!:		bpl	return_true	; +ve if A  > memory (signed).
		bra	return_false	; -ve if A <= memory (signed).

; **************
; Y:A is true if stacked-value >= Y:A, else false

ge_sb:		clc			; Subtract memory+1 from A.
		sbc.l	<__stack, x
		bvc	!+
		eor	#$80
!:		bmi	return_true	; -ve if A <= memory (signed).
		bra	return_false	; +ve if A  > memory (signed).

; **************
; Y:A is true if stacked-value <= Y:A, else false

le_sb:		sec			; Subtract memory from A.
		sbc.l	<__stack, x	; Cannot use a "cmp" as it
		bvc	!+		; does not set the V flag!
		eor	#$80
!:		bpl	return_true	; +ve if A >= memory (signed).
		bra	return_false	; -ve if A  < memory (signed).

; **************
; Y:A is true if stacked-value > Y:A, else false

gt_sb:		sec			; Subtract memory from A.
		sbc.l	<__stack, x	; Cannot use a "cmp" as it
		bvc	!+		; does not set the V flag!
		eor	#$80
!:		bmi	return_true	; -ve if A  < memory (signed).
		bra	return_false	; +ve if A >= memory (signed).

; **************
; Y:A is true if stacked-value < Y:A, else false

lt_ub:		clc			; Subtract memory+1 from A.
		sbc.l	<__stack, x
		bcs	return_true	; CS if A  > memory.
		bra	return_false

; **************
; Y:A is true if stacked-value >= Y:A, else false

ge_ub:		clc			; Subtract memory+1 from A.
		sbc.l	<__stack, x
		bcc	return_true	; CC if A <= memory.
		bra	return_false

; **************
; Y:A is true if stacked-value <= Y:A, else false

le_ub:		cmp.l	<__stack, x	; Subtract memory from A.
		bcs	return_true	; CS if Y:A >= memory.
		bra	return_false

; **************
; Y:A is true if stacked-value > Y:A, else false

gt_ub:		cmp.l	<__stack, x	; Subtract memory from A.
		bcc	return_true	; CC if A  < memory.
		bra	return_false



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

aslw7:		asl	a
		rol	<__temp
aslw6:		asl	a
		rol	<__temp
aslw5:		asl	a
		rol	<__temp
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
; Y:A = stacked-value << Y:A

aslw:		pha				; preserve count
		lda.h	<__stack, x
		sta	<__temp
		lda.l	<__stack, x
		inx
		inx
		ply				; restore count
		beq	.done
		cpy	#16
		bcs	.zero
.loop:		asl	a
		rol	<__temp
		dey
		bne	.loop
.done:		ldy	<__temp
		rts
.zero:		cla
		cly
		rts

; **************
; Y:A = stacked-value >> Y:A

asrw:		pha				; preserve count
		lda.h	<__stack, x
		bpl	asrw_positive
asrw_negative:	sta	<__temp
		lda.l	<__stack, x
		inx
		inx
		ply				; restore count
		beq	.done
		cpy	#16
		bcs	.sign
.loop:		sec
		ror	<__temp
		ror	a
		dey
		bne	.loop
.done:		ldy	<__temp
		rts

.sign:		lda	#$FF
		tay
		rts

; **************
; Y:A = stacked-value >> Y:A

lsrw:		pha				; preserve count
		lda.h	<__stack, x
asrw_positive:	sta	<__temp
		lda.l	<__stack, x
		inx
		inx
		ply				; restore count
		beq	.done
		cpy	#16
		bcs	.zero
.loop:		lsr	<__temp
		ror	a
		dey
		bne	.loop
.done:		ldy	<__temp
		rts

.zero:		cla
		cly
		rts



; ***************************************************************************
; ***************************************************************************
; subroutines for signed and unsigned multiplication and division
; ***************************************************************************
; ***************************************************************************

; **************
; Y:A = stacked-value * Y:A
;
; N.B. signed and unsigned multiply only differ in the top 16 of the 32bits!

umul:
smul:		sta.l	<multiplier
		sty.h	<multiplier
		__popw
		phx				; Preserve X (aka __sp).
		jsr	__mulint		; SDCC library name for this.
		plx				; Restore X (aka __sp).
		rts

; **************
; Y:A = stacked-value / Y:A

udiv:		sta.l	<divisor
		sty.h	<divisor
		__popw
		phx				; Preserve X (aka __sp).
		jsr	__divuint		; SDCC library name for this.
		plx				; Restore X (aka __sp).
		rts

; **************
; Y:A = stacked-value / Y:A

sdiv:		sta.l	<divisor
		sty.h	<divisor
		__popw
		phx				; Preserve X (aka __sp).
		jsr	__divsint		; SDCC library name for this.
		plx				; Restore X (aka __sp).
		rts

; **************
; Y:A = stacked-value % Y:A

umod:		sta.l	<divisor
		sty.h	<divisor
		__popw
		phx				; Preserve X (aka __sp).
		jsr	__moduint		; SDCC library name for this.
		plx				; Restore X (aka __sp).
		rts

; **************
; Y:A = stacked-value % Y:A

smod:		sta.l	<divisor
		sty.h	<divisor
		__popw
		phx				; Preserve X (aka __sp).
		jsr	__modsint		; SDCC library name for this.
		plx				; Restore X (aka __sp).
		rts



; ***************************************************************************
; ***************************************************************************
; subroutine for implementing a switch() statement
;
; case_table:
; +  0		db	6		; #bytes of case values.
; + 12		dw	val1
; + 34		dw	val2
; + 56		dw	val3
; + 78		dw	jmpdefault
; + 9A		dw	jmp1
; + BC		dw	jmp2
; + DE		dw	jmp3
; ***************************************************************************
; ***************************************************************************

; **************

do_switchw:	sta.l	<__temp		; Remember the value to check for.
		sty.h	<__temp

		lda	[__ptr]		; Read #bytes of case values to check.
		tay
		beq	zero_cases

.loop:		lda	[__ptr], y	; Test the case value.
		dey
		cmp.h	<__temp
		bne	.fail
		lda	[__ptr], y
		cmp.l	<__temp
		beq	case_found	; CS if case_found.
.fail:		dey
		bne	.loop

.default:	clc			; CC if default.
		bra	case_found

; **************

do_switchb:	tay			; Remember the value to check for.

		lda	[__ptr]		; Read #bytes of case values to check.
		say
		beq	zero_cases

.loop:		dey			; Test the case value (lo-byte only).
		cmp	[__ptr], y
		beq	case_found	; CS if case_found.
		dey
		bne	.loop

.default:	clc			; CC if default.

case_found:	tya			; Add the offset to the label address.
		adc	[__ptr]
		tay
zero_cases:	iny

		lda	[__ptr], y	; Read label address lo-byte.
		sta.l	<__temp
		iny
		lda	[__ptr], y	; Read label address hi-byte.
		sta.h	<__temp
		jmp	[__temp]

; **************
; 29 bytes, max 127 cases
;
;___casew:	sta.l	<__temp		; store the value to check to
;		sty.h	<__temp
;		ldy	#6
;!loop:		lda	table-2, y
;		cmp.l	<__temp
;		bne	!fail+
;		lda	table-1, y
;		cmp.h	<__temp
;		beq	!found+
;!fail:		dey
;		dey
;		bne	!loop-
;!found:	tya
;		sax
;		jmp	[table+6, x]
;
;cmp_table:
;01		dw	75		;
;23		dw	75		;
;45		dw	75		;
;
;67		dw	def
;89		dw	jmp1
;AB		dw	jmp3
;CD		dw	jmp5
;
;
; **************
; 16 bytes, max 127 cases
;
;___caseb:	ldy	#3
;!loop:		cmp	table-1, y
;		beq	!found+
;		dey
;		bne	!loop-
;!found:	tya
;		asl	a
;		sax
;		jmp	[table+3, x]
;
;cmp_table:				;0 *2->0 +3->3
;0		db	75		;1 *2->2 +3->5
;1		db	75		;2 *2->4 +3->7
;2		db	75		;3 *2->6 +3->9
;
;34		dw	def
;56		dw	jmp1
;78		dw	jmp3
;9A		dw	jmp5
