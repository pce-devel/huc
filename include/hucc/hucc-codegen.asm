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
;
;
; ***************************************************************************
; ***************************************************************************



; ***************************************************************************
; ***************************************************************************

; **************
; **************
; **************
; macros
; ----


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

;		stx	<__sp
;		lda.l	\1, x
;		ldy.h	\1, x
;		ldx	<__sp

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



; ***************************************************************************
; ***************************************************************************

; **************
; function prolog

__enter		.macro
;
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
; main C data-stack for arguments/locals/expressions
; used when calling a function that is not __xsafe

;__push_sp	.macro	; __STACK
;		phx
;		.endm

; **************
; main C data-stack for arguments/locals/expressions
; used when calling a function that is not __xsafe

;__pop_sp	.macro	; __STACK
;		plx
;		.endm

; **************
; main C data-stack for arguments/locals/expressions
; used when entering a #asm section that is not __xsafe
; cannot push/pop because the user can jump to another #asm section

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

;__modsp		.macro
;		sax
;		clc
;		adc.l	#\1
;		sax
;		.endm

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

; **************

__callp		.macro
		sta.l	<__ptr
		sty.h	<__ptr
		jsr	call_indirect
		.endm

; **************

__ldb		.macro
	.if (\# = 2)
		lda	\1
		clc
		adc.l	#\2
		tay
		cla
		adc.h	#\2
		say
	.else
		lda	\1
		cly
		bpl	!+
		dey
!:
	.endif
		.endm

; **************

__ldby		.macro
		lda	\1, y
		cly
		bpl	!+
		dey
!:
		.endm

;		stx	<__sp
;		lda.l	\1, x
;		ldy.h	\1, x
;		ldx	<__sp

;		lda.h	\1, y
;		pha
;		lda.l	\1, y
;		ply

; **************

__ldub		.macro
	.if (\# = 2)
		lda	\1
		clc
		adc.l	#\2
		cly
	.else
		lda	\1
		cly
	.endif
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

; **************

__ldw		.macro
	.if (\# = 2)
		clc
		lda.l	\1
		adc.l	#\2
		tay
		lda.h	\1
		adc.h	#\2
		say
	.else
		lda.l	\1
		ldy.h	\1
	.endif
		.endm

; ************** NEW and UNIMPLEMENTED!

__incw_		.macro
		inc.l	\1
		bne	!+
		inc.h	\1
!:
		.endm

; ************** NEW and UNIMPLEMENTED!

__decw_		.macro
		lda.l	\1
		bne	!+
		dec.h	\1
!:		dec.l	\1
		.endm

; ************** NEW and UNIMPLEMENTED!

__decwa_	.macro
		cmp	#0
		bne	!+
		dey
!:		dec	a
		.endm

__decwa2_	.macro
		sec
		sbc	#2
		say
		sbc	#0
		say
		.endm

__decwm_	.macro
		lda.l	\1
		bne	!+
		dec.h	\1
!:		dec.l	\1
		.endm

__decwm2_	.macro
		lda.l	\1
		bne	!+
		dec.h	\1
!:		dec.l	\1
		bne	!+
		dec.h	\1
!:		dec.l	\1
		.endm

__decws_	.macro
		lda.l	<__stack, x
		bne	!+
		dec.h	<__stack, x
!:		dec.l	<__stack, x
		.endm

__decws2_	.macro
		lda.l	<__stack, x
		bne	!+
		dec.h	<__stack, x
!:		dec.l	<__stack, x
		bne	!+
		dec.h	<__stack, x
!:		dec.l	<__stack, x
		.endm

; **************

__ldwp		.macro
		ldy	#1
		lda	[\1], y
		tay
		lda	[\1]
		.endm

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

__stw		.macro
		sta.l	\1
		sty.h	\1
		.endm

; **************

__stwi		.macro
		lda.l	#\2
		sta.l	\1
		ldy.h	#\2
		sty.h	\1
		.endm

; **************

__stb		.macro
		sta	\1
		.endm

; **************

__stbi		.macro
		lda.l	#\2
		sta	\1
		.endm

;		cly

; **************

__stbz		.macro
		stz	\1
		.endm

; **************

__stwz		.macro
		stz.l	\1
		stz.h	\1
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

; ************** NEW and UNIMPLEMENTED!

__stwp_ns	.macro
		sta	[\1]
		tya
		ldy	#1
		sta	[\1], y
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

;__stwps_ns	sta	[__stack, x]
;		inc.l	<__stack, x
;!:		say
;		sta	[__stack, x]
;		say
;		inx
;		inx

;__stwps_ns	sta	[__stack, x]
;		inc.l	<__stack, x
;!:		tya
;		sta	[__stack, x]
;		inx
;		inx

; **************

__stbps		.macro	; __STACK
		sta	[__stack, x]
		inx
		inx
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

__tstw		.macro
		sty	<__temp
		ora	<__temp
		beq	!+
		lda	#1
!:		cly
		.endm

; **************

__aslw		.macro
		asl	a
		say
		rol	a
		say
		.endm

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

__aslws		.macro
		asl.l	<__stack, x
		rol.h	<__stack, x
		.endm

;__asrws	.macro
;		lda.h	<__stack, x
;		asl	a
;		ror.h	<__stack, x
;		ror.l	<__stack, x
;		.endm

; **************

__asrw		.macro
		cpy	#$80
		say
		ror	a
		say
		ror	a
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

__lsrw		.macro
		say
		lsr	a
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

; **************

__extuw		.macro
		cly
		.endm

; **************

__extw		.macro
		tay
		cly
		bpl	!+
		dey
!:
		.endm

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

__boolw		.macro
		sty	<__temp
		ora	<__temp
		beq	!+
		lda	#1
!:		cly
		.endm

; **************

__notw		.macro
		sty	<__temp
		ora	<__temp
		cla
		bne	!+
		inc	a
!:		cly
		.endm

;		sty	<__temp
;		ora	<__temp
;		cly
;		bne	!+
;		iny
;!:		tya
;		cly

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

__subw		.macro
		sec
		sbc.l	\1
		say
		sbc.h	\1
		say
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

__orws		.macro	; __STACK
		ora.l	<__stack, x
		say
		ora.h	<__stack, x
		say
		inx
		inx
		.endm

; **************

__orwi		.macro
		ora.l	#\1
		say
		ora.h	#\1
		say
		.endm

; **************

__orw		.macro
		ora.l	\1
		say
		ora.h	\1
		say
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

__andw		.macro
		and.l	\1
		say
		and.h	\1
		say
		.endm

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

; **************

__bra		.macro
		bra	\1
		.endm

; **************
; boolean test and branch

__beq		.macro
		cmp	#0
		beq	\1
		.endm

; **************
; boolean test and branch

__bne		.macro
		cmp	#0
		bne	\1
		.endm

; ************** NEW and UNIMPLEMENTED!

__lbeqw		.macro
		sty	<__temp
		ora	<__temp
		beq	\1
		.endm

; **************

__cmpwi_eq	.macro
		cmp.l	#\1
		cla
		bne	!f+
		cpy.h	#\1
		bne	!f+
		inc	a
!f:		cly
		.endm

; **************

__cmpwi_ne	.macro
		cmp.l	#\1
		cla
		bne	!t+
		cpy.h	#\1
		beq	!f+
!t:		lda	#1
!f:		cly
		.endm

; **************

__call		.macro
		call	\1
		.endm

; **************

_set_bgpal.2	.macro
		lda	#1
		sta	<_ah
		call	_load_palette.3
		.endm

; **************

_set_bgpal.3	.macro
		call	_load_palette.3
		.endm

; **************

_set_sprpal.2	.macro
		lda	#1
		sta	<_ah
		smb4	<_al
		call	_load_palette.3
		.endm

; **************

_set_sprpal.3	.macro
		smb4	<_al
		call	_load_palette.3
		.endm

; **************

;_load_background	.macro
;		vload		$1000,\1,#$4000
;		vsync
;		set_bgpal	#0, \2,#16
;		batcpy		#$0, \3, \4, \5
;		.endm

; **************

; **************
; **************
; **************
; **************
; **************
; **************
; **************
; **************
; **************
; **************
; **************
; **************

;
; Hu-C internal include file
;

; optimized macros
; ----

; **************
; lea_s

__lea_s		.macro	; __STACK
		txa
		clc
		adc.l	#__stack + (\1)
		ldy.h	#__stack
		.endm

;		tsx
;		txa
;		clc
;		adc.l	#__stack + (\1)
;		ldy.h	#__stack

; **************

__pea_s		.macro	; __STACK
		dex
		dex
		txa
		clc
		adc.l	#__stack + 2 + (\1)
		sta.l	<__stack, x
		ldy.h	#__stack
		sty.h	<__stack, x
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

__ldub_s	.macro	; __STACK
		lda	<__stack + \1, x
		cly
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

__ldb_p		.macro
		sta.l	<__ptr
		sty.h	<__ptr
		lda	[__ptr]
		cly
		bpl	!+
		dey
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

__ldub_p	.macro
		sta.l	<__ptr
		sty.h	<__ptr
		lda	[__ptr]
		cly
		.endm

; **************
;
__ldw_s		.macro	; __STACK
		lda.l	<__stack + \1, x
		ldy.h	<__stack + \1, x
		.endm

; **************

__stbi_s	.macro	; __STACK
		lda.l	#\1
		sta.l	<__stack + \2, x
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

__stb_s		.macro	; __STACK
		sta.l	<__stack + \1, x
		.endm

; **************

__stw_s		.macro	; __STACK
		sta.l	<__stack + \1, x
		sty.h	<__stack + \1, x
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
; get the acc lo-byte after returning from a function

__getacc	.macro
		lda	<__hucc_ret
		.endm

; **************
; XXX: the semantics of this are ridiculous: It assumes the value of
; the incremented variable to be in AX, the memory location to
; be incremented and the previous value retained in AX, making it
; necessary to spill A.
; incw_s
;
; JCB: Hahaha, the joys of C post-increment!

__incw_s	.macro	; __STACK
		inc.l	<__stack + \1, x
		bne	!+
		inc.h	<__stack + \1, x
!:
		.endm

; **************
; incb_s

__incb_s	.macro	; __STACK
		inc	<__stack + \1, x
		.endm

; **************
; ldd_i

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
; ldd_b

__ldd_b		.macro
		lda	\1
		cly
		sta.l	<\2
		stz.h	<\2
		stz.l	<\3
		stz.h	<\3
		.endm

; **************
; ldd_w

__ldd_w		.macro
		lda.l	\1
		ldy.h	\1
		sta.l	<\2
		sty.h	<\2
		stz.l	<\3
		stz.h	<\3
		.endm

; **************
; ldd_s_b

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

; **************
; ldd_s_w

__ldd_s_w	.macro	; __STACK
		lda.l	<__stack + \1, x
		ldy.h	<__stack + \1, x
		sta.l	<\2
		sty.h	<\2
		stz.l	<\3
		stz.h	<\3
		.endm

; **************


; ***************************************************************************
; ***************************************************************************
;
; ***************************************************************************
; ***************************************************************************
;




; **************
; **************
; **************


; **************
; eq_w, eq_b
; **************
; test equality of two words
; ----
; IN :	1st word in memory, 2nd word in Y:A
; ----
; OUT : Y:A is true if memory == Y:A, else false.
; ----
; REMARK : signed compatible
; ----

eq_bzp:		cmp.l	<__temp
		bne	ret_false_zp
		bra	ret_true_zp

eq_wzp:		cmp.l	<__temp
		bne	ret_false_zp
		cpy.h	<__temp
		bne	ret_false_zp
		bra	ret_true_zp

eq_b:		cmp	<__stack, x
		bne	ret_false_sp
		bra	ret_true_sp

eq_w:		cmp.l	<__stack, x
		bne	ret_false_sp
		say
		cmp.h	<__stack, x
		say
		bne	ret_false_sp
		bra	ret_true_sp

; **************
; ne_w, ne_b
; **************
; compare two words
; ----
; IN :	1st word in memory, 2nd word in Y:A
; ----
; OUT : Y:A is true if memory != Y:A, else false.
; ----
; REMARK : signed compatible
; ----

ne_bzp:		cmp.l	<__temp
		bne	ret_true_zp
		bra	ret_false_zp

ne_wzp:		cmp.l	<__temp
		bne	ret_true_zp
		cpy.h	<__temp
		bne	ret_true_zp
		bra	ret_false_zp

ne_b:		cmp	<__stack, x
		bne	ret_true_sp
		bra	ret_false_sp

ne_w:		cmp.l	<__stack, x
		bne	ret_true_sp
		say
		cmp.h	<__stack, x
		say
		bne	ret_true_sp
		bra	ret_false_sp

;
;
;

ret_true_sp:	inx
		inx			; don't push Y:A, they are thrown away
ret_true_zp:	cly
		lda	#1		; Also set valid Z flag.
		rts

ret_false_sp:	inx
		inx			; don't push Y:A, they are thrown away
ret_false_zp:	cly
		lda	#0		; Also set valid Z flag.
		rts

;
;
;

; **************
; lt_uw, lt_ub
; **************
; compare two words
; ----
; IN :	1st word in memory, 2nd word in Y:A
; ----
; OUT : Y:A is true if memory < Y:A, else false.
; ----

lt_ub:		clc			; Subtract memory+1 from A.
		sbc.l	<__stack, x
		bcs	ret_true_sp	; CS if A  > memory.
		bra	ret_false_sp

lt_uw:		clc			; Subtract memory+1 from Y:A.
		sbc.l	<__stack, x
		tya
		sbc.h	<__stack, x
		bcs	ret_true_sp	; CS if Y:A  > memory.
		bra	ret_false_sp

; **************
; ge_uw, ge_ub
; **************
; compare two signed words
; ----
; IN :	1st word in memory, 2nd word in Y:A
; ----
; OUT : Y:A is true if memory >= Y:A, else false.
; ----

ge_ub:		clc			; Subtract memory+1 from A.
		sbc.l	<__stack, x
		bcc	ret_true_sp	; CC if A <= memory.
		bra	ret_false_sp

ge_uw:		clc			; Subtract memory+1 from Y:A.
		sbc.l	<__stack, x
		tya
		sbc.h	<__stack, x
		bcc	ret_true_sp	; CC if Y:A <= memory.
		bra	ret_false_sp

; **************
; le_uw, le_ub
; **************
; compare two words
; ----
; IN :	1st word in memory, 2nd word in Y:A
; ----
; OUT : Y:A is true if memory is <= Y:A, else false.
; ----

le_ub:		cmp.l	<__stack, x	; Subtract memory from A.
		bcs	ret_true_sp	; CS if Y:A >= memory.
		bra	ret_false_sp

le_uw:		cmp.l	<__stack, x	; Subtract memory from Y:A.
		tya
		sbc.h	<__stack, x
		bcs	ret_true_sp	; CS if Y:A >= memory.
		bra	ret_false_sp

; **************
; gt_uw, gt_ub
; **************
; compare two words
; ----
; IN :	1st word in memory, 2nd word in Y:A
; ----
; OUT : Y:A is true if memory > Y:A, else false.
; ----

gt_ub:		cmp.l	<__stack, x	; Subtract memory from A.
		bcc	ret_true_sp	; CC if A  < memory.
		bra	ret_false_sp

gt_uw:		cmp.l	<__stack, x	; Subtract memory from Y:A.
		tya
		sbc.h	<__stack, x
		bcc	ret_true_sp	; CC if Y:A  < memory.
		bra	ret_false_sp

; **************
; lt_sw, lt_sb
; **************
; compare two words
; ----
; IN :	1st word in memory, 2nd word in Y:A
; ----
; OUT : Y:A is true if memory < Y:A, else false.
; ----

lt_sb:		clc			; Subtract memory+1 from A.
		sbc.l	<__stack, x
		bvc	!+
		eor	#$80
!:		bpl	ret_true_sp	; +ve if A  > memory (signed).
		bra	ret_false_sp	; -ve if A <= memory (signed).

lt_sw:		clc			; Subtract memory+1 from Y:A.
		sbc.l	<__stack, x
		tya
		sbc.h	<__stack, x
		bvc	!+
		eor	#$80
!:		bpl	ret_true_sp	; +ve if Y:A  > memory (signed).
		bra	ret_false_sp	; -ve if Y:A <= memory (signed).

; **************
; ge_sw, ge_sb
; **************
; compare two signed words
; ----
; IN :	1st word in memory, 2nd word in Y:A
; ----
; OUT : Y:A is true if memory >= Y:A, else false.
; ----

ge_sb:		clc			; Subtract memory+1 from A.
		sbc.l	<__stack, x
		bvc	!+
		eor	#$80
!:		bmi	ret_true_sp	; -ve if A <= memory (signed).
		bra	ret_false_sp	; +ve if A  > memory (signed).

ge_sw:		clc			; Subtract memory+1 from Y:A.
		sbc.l	<__stack, x
		tya
		sbc.h	<__stack, x
		bvc	!+
		eor	#$80
!:		bmi	ret_true_sp	; -ve if Y:A <= memory (signed).
		bra	ret_false_sp	; +ve if Y:A  > memory (signed).

; **************
; le_sw, le_sb
; **************
; compare two signed words
; ----
; IN :	1st word in memory, 2nd word in Y:A
; ----
; OUT : Y:A is true if memory <= Y:A, else false.
; ----

le_sb:		sec			; Subtract memory from A.
		sbc.l	<__stack, x	; Cannot use a "cmp" as it
		bvc	!+		; does not set the V flag!
		eor	#$80
!:		bpl	ret_true_sp	; +ve if A >= memory (signed).
		bra	ret_false_sp	; -ve if A  < memory (signed).

le_sw:		cmp.l	<__stack, x	; Subtract memory from Y:A.
		tya
		sbc.h	<__stack, x
		bvc	!+
		eor	#$80
!:		bpl	ret_true_sp	; +ve if Y:A >= memory (signed).
		bra	ret_false_sp	; -ve if Y:A  < memory (signed).

; **************
; gt_sw, gt_sb
; **************
; compare two words
; ----
; IN :	1st word in memory, 2nd word in Y:A
; ----
; OUT : Y:A is true if memory > Y:A, else false.
; ----

gt_sb:		sec			; Subtract memory from A.
		sbc.l	<__stack, x	; Cannot use a "cmp" as it
		bvc	!+		; does not set the V flag!
		eor	#$80
!:		bmi	ret_true_sp	; -ve if A  < memory (signed).
		bra	ret_false_sp	; +ve if A >= memory (signed).

gt_sw:		cmp.l	<__stack, x	; Subtract memory from Y:A.
		tya
		sbc.h	<__stack, x
		bvc	!+
		eor	#$80
!:		bmi	ret_true_sp	; -ve if Y:A  < memory (signed).
		bra	ret_false_sp	; +ve if Y:A >= memory (signed).




;*******************************************************************
;*******************************************************************
;*******************************************************************
;*******************************************************************
;*******************************************************************



; **************
; asl - shift the memory left by the register word
;
; IN :	First word on the C stack
;	Another word in A:X
;
; OUT : Register word egals the previous pushed value
;	shifted left by A:X
; ----
; REMARK :	only the lower byte of the right operand is taken in account
;		signed compatible
; ----

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

;		.dw	aslw0
;		.dw	aslw1
;		.dw	aslw2
;		.dw	aslw3
;		.dw	aslw4
;		.dw	aslw5
;		.dw	aslw6
;		.dw	aslw7
;		.dw	aslw8
;		.dw	aslw9
;		.dw	aslw10
;		.dw	aslw11
;		.dw	aslw12
;		.dw	aslw13
;		.dw	aslw14
;		.dw	aslw15

; **************

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
; asr - shift the memory right by the register word
;
; IN :	First word on the C stack
;	Another word in A:X
;
; OUT : Register word egals the previous pushed value
;	shifted right by A:X
; ----
; REMARK :	only the lower byte of the right operand is taken in account
;		signed compatible
; ----

; **************

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
; lsr - shift the memory right by the register word
;
; IN :	First word on the C stack
;	Another word in A:X
;
; OUT : Register word egals the previous pushed value
;	shifted right by A:X
;
; REMARK :	only the lower byte of the right operand is taken in account
;		signed compatible

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

; **************

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

; **************
; umul - multiply two UNSIGNED words
; smul - multiply two SIGNED words
;
; IN :	First word on the C stack
;	Another word in Y:A
;
; OUT : Register word equals the previous pushed value
;	multiplied by Y:A
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
; udiv - divide two UNSIGNED words
;
; IN :	First word on the C stack
;	Another word in Y:A:
;
; OUT : Register word = stacked-value / Y:A

udiv:		sta.l	<divisor
		sty.h	<divisor
		__popw
		phx				; Preserve X (aka __sp).
		jsr	__divuint		; SDCC library name for this.
		plx				; Restore X (aka __sp).
		rts

; **************
; sdiv - divide two SIGNED words
;
; IN :	First word on the C stack
;	Another word in Y:A
;
; OUT : Register word = stacked-value / Y:A

sdiv:		sta.l	<divisor
		sty.h	<divisor
		__popw
		phx				; Preserve X (aka __sp).
		jsr	__divsint		; SDCC library name for this.
		plx				; Restore X (aka __sp).
		rts

; **************
; umod - give the integer remainder of the two words
;
; IN :	First word on the C stack
;	Another word in Y:A
;
; OUT : Register word = the remainder of stacked-value / Y:A

umod:		sta.l	<divisor
		sty.h	<divisor
		__popw
		phx				; Preserve X (aka __sp).
		jsr	__moduint		; SDCC library name for this.
		plx				; Restore X (aka __sp).
		rts

; **************
; smod - give the integer remainder of the two words
;
; IN :	First word on the C stack
;	Another word in Y:A
;
; OUT : Register word = the remainder of stacked-value / Y:A

smod:		sta.l	<divisor
		sty.h	<divisor
		__popw
		phx				; Preserve X (aka __sp).
		jsr	__modsint		; SDCC library name for this.
		plx				; Restore X (aka __sp).
		rts

; **************
; ___case
; **************
; implement a switch instruction in C
; ----
; IN :	primary register (A:X) contain the discriminant value
;	i.e. the one that will be checked against those indicated in the
;	various case instructions
;	On the stack, a pointer is passed
;	This is a pointer toward an array
;	Each item of this array is a 4 bytes long structure
;	The structure is the following :
;	  WORD value_to_check
;	  WORD label_to_jump_to
;	We have to parse the whole array in order to compare the primary
;	register with all the 'value_to_check' field. If we ever find that
;	the primary register is egal to such a value, we must jump to the
;	corresponding 'label_to_jump_to'.
;	The default value (which also means that we reached the end of the
;	array) can be recognized with its 'label_to_jump_to' field set to 0.
;	Then the 'value_to_check' field become the default label we have to
;	use for the rest of the execution.
; ----
; OUT : The execution goes to another place
; ----
; REMARK : Also use remain variable as a temporary value
; ----

___case:	sta.l	<__temp		; store the value to check to
		sty.h	<__temp

		__popw

		sta.l	<__ptr		; __ptr contain the address of the array
		sty.h	<__ptr

		cly
		bra	.begin_case

.next_case_lo:	iny

.next_case_hi:	iny

.begin_case:	iny			; skip func lo-byte
		lda	[__ptr], y	; test func hi-byte
		beq	.case_default
		iny
		lda	[__ptr], y
		cmp.l	<__temp
		bne	.next_case_lo
		iny
		lda	[__ptr], y
		cmp.h	<__temp
		bne	.next_case_hi
		dey			; found match
		dey

.case_vector:	lda	[__ptr], y	; read func hi-byte
		sta.h	<__temp
		dey
		lda	[__ptr], y	; read func lo-byte
		sta.l	<__temp
		jmp	[__temp]

.case_default:	iny
		iny
		bra	.case_vector




;___case:	lda	[__ptr]
;		tay
;
;.next_hi:	dey
;		dey
;.test_hi:	lda	[__ptr], y
;		cmp.h	<__temp
;		bne	.next_hi
;		dey
;		lda	[__ptr], y
;		cmp.l	<__temp
;		beq	.found		; also CS
;		dey
;		bne	.test_hi
;		dey
;
;.found:	tya
;		clc
;		adc	[_ptr]
;		sax
;		rts
;
;cmp_table:	db	6+2		; length
;12		dw	75		;
;34		dw	75		;
;56		dw	75		;
;
;78		dw	def
;9A		dw	jmp1
;BC		dw	jmp3
;DE		dw	jmp5
;
;
;
;**NEEDS FIXING**
;
;___case:	ldy	table
;.next_hi:	dey
;		dey
;!loop:		lda	table, y
;		dey
;		cmp.h	<__temp
;		bne	!fail+
;		lda	table, y
;		cmp.l	<__temp
;		beq	.found		; also CS
;!fail:		dey
;		bne	!loop-
;		dey
;.found:	tya
;		clc
;		adc	table
;		sax
;		jmp	[table, x]
;
;cmp_table:	db	6+2		; length
;12		dw	75		;
;34		dw	75		;
;56		dw	75		;
;
;78		dw	def
;9A		dw	jmp1
;BC		dw	jmp3
;DE		dw	jmp5
;
;
;
;
;___casew:	sta.l	<__temp		; store the value to check to
;		sty.h	<__temp
;		ldy	#5
;!loop:		lda	table, y
;		dey
;		cmp.h	<__temp
;		bne	!fail+
;		lda	table, y
;		cmp.l	<__temp
;		beq	!found+		; also CS
;!fail:		dey
;		bpl	!loop-
;		clc
;!found:	tya
;		adc	#7
;		sax
;		jmp	[table, x]
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
;
;
;___caseb:	ldy	#2
;!loop:		cmp	table, y
;		beq	!found+		; also CS
;		dey
;		bpl	!loop-
;!found:	iny
;		tya
;		asl	a
;		adc	#3
;		sax
;		jmp	[table, x]
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


; **************

; ----
; hook
; ----
; indirect call to sub-routine
; ----
; IN :	sub-routine addr in __ptr
; ----

call_indirect:	jmp	[__ptr]



