; ***************************************************************************
; ***************************************************************************
;
; hucc-math.asm
;
; Basic (i.e. very slow) 8-bit and 16-bit multiply and divide routines.
;
; Copyright John Brandwood 2021-2024.
;
; Distributed under the Boost Software License, Version 1.0.
; (See accompanying file LICENSE_1_0.txt or copy at
;  http://www.boost.org/LICENSE_1_0.txt)
;
; ***************************************************************************
; ***************************************************************************
;
; This is basically a set of SDCC-compatible routines, but using Y:A for the
; primary register instead of X:A.
;
; Using Y:A makes the routines instantly usable with HuCC, and it also makes
; them usable with SDCC with just an "sxy" before and after the call.
;
; ***************************************************************************
; ***************************************************************************

		.code

multiplier	=	__temp
multiplicand	=	___SDCC_m6502_ret0
product		=	multiplicand

__mulint_PARM_2	=	multiplier

divisor		=	__temp
dividend	=	___SDCC_m6502_ret0
quotient	=	dividend
remainder	=	___SDCC_m6502_ret2

__moduint_PARM_2 =	divisor
__modsint_PARM_2 =	divisor
__divuint_PARM_2 =	divisor
__divsint_PARM_2 =	divisor



; ***************************************************************************
; int
; _mulint (int a, int b)
;
; 1st parameter in Y:A (multiplicand)
; 2nd parameter in __mulint_PARM_2 (multiplier)
; result in Y:A
;
; N.B. signed and unsigned multiply only differ in the top 16 of the 32bits!

__mulint:	sta	<multiplicand + 0
		sty	<multiplicand + 1

		ldx	#16			; Loop 16 times.

		lsr	<multiplicand + 1	; Divide multiplicand by 2
		ror	<multiplicand + 0	; and clear the 16th bit.

		cla				; Clear top word of product.
		sta.l	<multiplicand + 2
		bcc	.rotate

.add:		tay				; Add the 16-bit multiplier to
		clc				; top 16-bits of the product.
		lda	<multiplicand + 2
		adc.l	<multiplier
		sta	<multiplicand + 2
		tya
		adc.h	<multiplier

.rotate:	ror	a			; Rotate product into the top
		ror	<multiplicand + 2	; bits of the multiplicand ...
		ror	<multiplicand + 1	; and divide multiplicand by 2.
		ror	<multiplicand + 0

		dex
		bcs	.add			; Add multiplier to top word?
		bne	.rotate			; Completed 16 bits?

		sta	<multiplicand + 3	; Save top byte of product.

		lda.l	<multiplicand		; Return the bottom 16-bits of
		ldy.h	<multiplicand		; the 32-bit product.

		rts


; ***************************************************************************
; unsigned int
; _divuint (unsigned int x, unsigned int y)
;
; 1st parameter in Y:A (unsigned dividend)
; 2nd parameter in __divuint_PARM_2 (unsigned divisor)
; result in Y:A

__divuint:	jsr	__moduint		; Call the basic uint division.

		lda.l	<quotient		; Then get the result from where
		ldy.h	<quotient		; it was calculated.
		rts


; ***************************************************************************
; unsigned int
; _moduint (unsigned int x, unsigned int y)
;
; 1st parameter in Y:A (unsigned dividend)
; 2nd parameter in __moduint_PARM_2 (unsigned divisor)
; result in Y:A
;
; If the dividend has more bits than the divisor, then we need to check the
; 17th bit of the remainder!

	.if	0

__moduint:	sta.l	<dividend		; 1st SDCC parameter in Y:A.
		sty.h	<dividend

divmoduint:	lda.l	<divisor		; Check for a divide-by-zero.
		ora.h	<divisor
.zero:		beq	.zero

		cla				; Clear remainder.
		sta.h	<remainder

		ldx	#16

.loop:		asl.l	<dividend		; Rotate dividend, MSB -> C.
		rol.h	<dividend
		rol	a			; Rotate C into remainder.
		rol.h	<remainder
;		php				; Preserve remainder 17th bit.

		tay				; Preserve remainder lo-byte.

		cmp.l	<divisor		; Test divisor.
		lda.h	<remainder
		sbc.h	<divisor

;		bcs	.subtract		; CS if divisor <= remainder.
;		plp				; Restore remainder 17th bit.
;		bcc	.skip			; CC if divisor > remainder.
;		db	$90			; Turn "plp" into "bcc" to skip.
;.subtract:	plp				; Discard remainder 17th bit.
;		sec

;		tax				; If dividend has more bits
;		pla				; than the divisor, then we
;		and	#1			; need to check rotated bit.
;		sbc	#0
;		txa
		bcc	.skip			; CC if divisor > remainder.

		sta.h	<remainder		; Subtract divisor.
		tya
		sbc.l	<divisor
		inc.l	<dividend		; Quotient bit -> dividend LSB.

		db	$F0			; Turn "tya" into "beq" to skip.
.skip:		tya				; Restore remainder lo-byte.

		dex
		bne	.loop

		ldy.h	<remainder		; Get the remainder hi-byte.

		rts

	.else

__moduint:	sta.l	<dividend		; 1st SDCC parameter in Y:A.
		sty.h	<dividend

divmoduint:	lda.l	<divisor		; Check for a divide-by-zero.
		ora.h	<divisor
.zero:		beq	.zero

		ldx	#16 + 1

		cly				; Clear remainder.
		sty.h	<remainder

.skip:		tya				; Restore remainder lo-byte.

.loop:		rol.l	<dividend		; Quotient bit -> dividend LSB.
		rol.h	<dividend		; Rotate dividend, MSB -> C.

		dex
		beq	.finished

		rol	a			; Rotate C into remainder.
		rol.h	<remainder
;		php				; Preserve remainder 17th bit.

		tay				; Preserve remainder lo-byte.

		cmp.l	<divisor		; Test divisor.
		lda.h	<remainder
		sbc.h	<divisor
		bcc	.skip			; CC if divisor > remainder.

;		cmp.l	<divisor		; If the dividend has more bits
;		lda.h	<remainder		; than the divisor then we need
;		sbc.h	<divisor		; to check the remainder hi-bit.
;		bcs	.subtract		; CS if divisor <= remainder.
;		plp				; Restore remainder 17th bit.
;		bcc	.skip			; CC if divisor > remainder.
;		db	$90			; Turn "plp" into "bcc" to skip.
;.subtract:	plp				; Discard remainder 17th bit.
;		sec

		sta.h	<remainder		; Subtract divisor.
		tya
		sbc.l	<divisor
		sec				
		bra	.loop

.finished:	ldy.h	<remainder		; Get the remainder hi-byte.

		rts

	.endif



; ***************************************************************************
; int
; _divsint (int x, int y)
;
; 1st parameter in Y:A (signed dividend)
; 2nd parameter in __divsint_PARM_2 (signed divisor)
; result in Y:A

__divsint:	jsr	__modsint		; Call the basic sint division.

		lda.l	<quotient		; Then get the result from where
		ldy.h	<quotient		; it was calculated.
		rts


; ***************************************************************************
; int
; _modsint (int x, int y)
;
; 1st parameter in Y:A (signed dividend)
; 2nd parameter in __modsint_PARM_2 (signed divisor)
; result in Y:A

__modsint:	sty.h	<dividend

		cpy	#$80			; Remainder -ve if dividend
		php				; was -ve.
		bcc	!+
		jsr	neg_yacs		; Negate the dividend.

!:		sta.l	<dividend		; Store the dividend.
		lda.h	<dividend
		sty.h	<dividend

		eor.h	<divisor		; Quotient is -ve if divisor
		php				; and dividend signs differ.

		lda.h	<divisor		; Is the divisor -ve?
		bpl	!+

		sec				; Negate the divisor.
		cla
		sbc.l	<divisor
		sta.l	<divisor
		cla
		sbc.h	<divisor
		sta.h	<divisor

!:		jsr	divmoduint		; Do the unsigned division.

.result:	plp				; Should the quotient be -ve?
		bpl	.remainder

		tax				; Preserve remainder lo-byte.

		sec				; Then negate the quotient.
		cla
		sbc.l	<dividend
		sta.l	<dividend
		cla
		sbc.h	<dividend
		sta.h	<dividend

		txa				; Restore remainder lo-byte.

.remainder:	plp				; Was the dividend -ve?
		bcs	neg_yacs		; Then negate the remainder.
		rts


; ***************************************************************************
; unsigned int
; _muluchar (unsigned char x, unsigned char y)
;
; 1st parameter in A (unsigned multiplicand)
; 2nd parameter in Y (unsigned multiplier)
; result in Y:A

__muluchar:	sty	<multiplier

muluchar_a:	ldy	#8			; Loop 8 times.

		lsr	a			; Divide multiplicand by 2
		sta	<multiplicand		; and clear the 8th bit.

		cla				; Clear top byte of product.
		bcc	.rotate

.add:		clc				; Add the 8-bit multiplier to
		adc	<multiplier		; top 8-bits of the product.

.rotate:	ror	a			; Rotate product into the top
		ror	<multiplicand		; bits of the multiplicand.

		dey
		bcs	.add			; Add multiplier to top byte?
		bne	.rotate			; Completed 8 bits?

		tay				; Return the 16-bit product.
		lda	<multiplicand

		rts


; ***************************************************************************
; signed int
; abs (signed int x)
;
; 1st parameter in Y:A (signed)
; result in Y:A

_abs:		cpy	#$80			; Is the hi-byte -ve?
		bcc	!+

neg_ya:		sec
neg_yacs:	eor	#$FF
		adc	#0
		say
		eor	#$FF
		adc	#0
		say
!:		rts


; ***************************************************************************
; signed int
; _mulschar (signed char x, signed char y)
;
; 1st parameter in A (signed multiplicand)
; 2nd parameter in Y (signed multiplier)
; result in Y:A
;
; N.B. Y and A get swapped to make the code shorter.

__mulschar:	sta	<multiplicand		; Remember multiplicand sign.

		cmp	#$80			; Is the multiplicand -ve?
		bcc	!+
		eor	#$FF			; Negate the multiplicand.
		inc	a
!:		sta	<multiplier		; Then save it as multiplier.

		tya				; Product -ve if multiplicand
		eor	<multiplicand		; and multiplier signs differ.
		php				; Remember product sign.

		tya				; Is the multiplicand -ve?
		bpl	!+
		eor	#$FF			; Negate the multiplicand.
		inc	a

!:		jsr	muluchar_a		; Multiplier already saved.

		plp				; Is the product -ve?
		bmi	neg_ya
		rts


; ***************************************************************************
; unsigned int
; _mulsuchar (signed char x, signed char y)
;
; 1st parameter in A (unsigned multiplicand)
; 2nd parameter in Y (signed multiplier)
; result in Y:A
;
; N.B. Y and A get swapped to make the code shorter.

__mulsuchar:	say				; Put the signed param in A.
		; drop through to __muluschar


; ***************************************************************************
; signed int
; _muluschar (unsigned char x, unsigned char y)
;
; 1st parameter in A (signed multiplicand)
; 2nd parameter in Y (unsigned multiplier)
; result in Y:A

__muluschar:	cmp	#$80			; Is multiplicand -ve?
		php				; Remember the sign.
		bcc	!+
		eor	#$FF			; Negate multiplicand.
		inc	a

!:		jsr	__muluchar		; Do the unsigned multiply.

		plp				; Was multiplicand -ve?
		bcs	neg_yacs		; Then negate the product.
		rts


; ***************************************************************************
; unsigned int
; _moduchar (unsigned char x, unsigned char y)
;
; 1st parameter in A (unsigned dividend)
; 2nd parameter in Y (unsigned divisor)
; result in Y:A

__moduchar:	sty.l	<divisor

divmodu8_a:	asl	a			; Rotate dividend, MSB -> C.
		sta.l	<dividend
		stz.h	<dividend		; Clear quotient hi-byte.

		ldy	#8
		cla				; Clear remainder.
.loop:		rol	a			; Rotate C into remainder.
		cmp	<divisor		; Test divisor.
		bcc	.skip			; CC if divisor > remainder.
		sbc	<divisor		; Subtract divisor.
.skip:		rol	<dividend		; Quotient bit -> dividend LSB.
		dey
		bne	.loop

		cly				; Clear hi-byte of return.
		rts				; Return the 16-bit remainder.


; ***************************************************************************
; unsigned int
; _divuchar (unsigned char x, unsigned char y)
;
; 1st parameter in A (unsigned dividend)
; 2nd parameter in Y (unsigned divisor)
; result in Y:A

__divuchar:	jsr	__moduchar

		lda	<dividend		; Get the dividend lo-byte.
		rts				; Return the 16-bit dividend.


; ***************************************************************************
; signed int
; _modschar (signed char x, signed char y)
;
; 1st parameter in A (signed dividend)
; 2nd parameter in Y (signed divisor)
; result in Y:A

__modschar:	tax				; Preserve the dividend.
		php				; Remember remainder sign.

		sty	<divisor		; Quotient negative if divisor
		eor	<divisor		; and dividend signs differ.
		php				; Remember the quotient sign.

		tya				; Is the divisor -ve?
		bpl	!+
		eor	#$FF			; Negate the divisor.
		inc	a
		sta	<divisor

!:		txa				; Is the dividend -ve?
		bpl	divmods8_a

divmods8_neg:	eor	#$FF			; Negate the dividend.
		inc	a

divmods8_a:	jsr	divmodu8_a		; Do the unsigned division.

		plp				; Should the quotient be -ve?
		bpl	!+

		tax				; Preserve remainder lo-byte.

		sec				; Negate the quotient.
		cla
		sbc.l	<dividend
		sta.l	<dividend
		lda	#$FF
		sta.h	<dividend

		txa				; Restore remainder lo-byte.

!:		plp				; Was the dividend -ve?
		bpl	!+

		eor	#$FF			; Then negate the remainder.
		inc	a
		ldy	#$FF

!:		rts


; ***************************************************************************
; signed int
; _moduschar (unsigned char x, unsigned char y)
;
; 1st parameter in A (signed dividend)
; 2nd parameter in Y (unsigned divisor)
; result in Y:A

__moduschar:	sty.l	<divisor

		tay				; Is the dividend -ve?
		php				; Remember remainder sign.
		php				; Remember quotient sign.
		bmi	divmods8_neg
		bra	divmods8_a


; ***************************************************************************
; unsigned int
; _modsuchar (signed char x, signed char y)
;
; 1st parameter in A (unsigned dividend)
; 2nd parameter in Y (signed divisor)
; result in Y:A

__modsuchar:	tax				; Preserve the dividend.
		php				; Remember remainder sign.

		tya				; Check the divisor sign.
		php				; Remember quotient sign.
		bpl	!+
		eor	#$FF			; Negate the divisor.
		inc	a
!:		sta.l	<divisor

		txa				; Restore the dividend.
		bra	divmods8_a


; ***************************************************************************
; signed int
; _divschar (signed char x, signed char y)
;
; 1st parameter in A (signed dividend)
; 2nd parameter in Y (signed divisor)
; result in Y:A

__divschar:	jsr	__modschar

		lda.l	<dividend
		ldy.h	<dividend
		rts


; ***************************************************************************
; signed int
; _divuschar (unsigned char x, unsigned char y)
;
; 1st parameter in A (signed dividend)
; 2nd parameter in Y (unsigned divisor)
; result in Y:A

__divuschar:	jsr	__moduschar

		lda.l	<dividend
		ldy.h	<dividend
		rts


; ***************************************************************************
; unsigned int
; _divsuchar (signed char x, signed char y)
;
; 1st parameter in A (unsigned dividend)
; 2nd parameter in Y (signed divisor)
; result in Y:A

__divsuchar:	jsr	__modsuchar

		lda.l	<dividend
		ldy.h	<dividend
		rts
