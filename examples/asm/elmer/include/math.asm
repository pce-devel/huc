; ***************************************************************************
; ***************************************************************************
;
; math.asm
;
; Basic (i.e. slow) math routines.
;
; Copyright John Brandwood 2021-2022.
;
; Distributed under the Boost Software License, Version 1.0.
; (See accompanying file LICENSE_1_0.txt or copy at
;  http://www.boost.org/LICENSE_1_0.txt)
;
; ***************************************************************************
; ***************************************************************************



; ***************************************************************************
; ***************************************************************************
;
; mul8u - Simple 16-bit x 8-bit multiply.
;
; Args: __ax = Multiplier / Result
; Args: __bl = Multiplier
;

mul8u		.proc

		cla				; Clear Result.
		clx
		lsr	<__bl			; Shift and test multiplier.
		bcc	.loop
.add:		clc				; Add __ax to the Result.
		adc	<__al
		sax
		adc	<__ah
		sax
.loop:		asl	<__al			; __ax = __ax * 2
		rol	<__ah
		lsr	<__bl			; Shift and test multiplier.
		bcs	.add
		bne	.loop
		sta	<__al			; Save Result.
		stx	<__ah

		leave

		.endp



; ***************************************************************************
; ***************************************************************************
;
; div16_7u - Simple 16-bit /  7-bit divide.
;
; Args: __ax = Dividend / Quotient
; Args: __bx = Dividend / Quotient (if 32-bit)
; Args: __cl = Dividor
; Uses: __dl = Remainder
;

div16_7u	.proc

		ldx	#16
		cla				; Clear Remainder.
		asl	<__ax + 0		; Rotate Dividend, MSB -> C.
		rol	<__ax + 1
.loop:		rol	a			; Rotate C into Remainder.
		cmp	<__cl			; Test Divisor.
		bcc	.less			; CC if Divisor > Remainder.
		sbc	<__cl			; Subtract Divisor.
.less:		rol	<__ax + 0		; Quotient bit -> Dividend LSB.
		rol	<__ax + 1		; Rotate Dividend, MSB -> C.
		dex
		bne	.loop
		sta	<__dl			; Save the remainder.

		leave

		.endp



; ***************************************************************************
; ***************************************************************************
;
; div32_7u - Simple 32-bit /  7-bit divide.
;
; Args: __ax = Dividend / Quotient
; Args: __bx = Dividend / Quotient (if 32-bit)
; Args: __cl = Dividor
; Uses: __dl = Remainder
;

div32_7u	.proc

		ldx	#32
		cla				; Clear Remainder.
		asl	<__ax + 0		; Rotate Dividend, MSB -> C.
		rol	<__ax + 1
		rol	<__ax + 2
		rol	<__ax + 3
.loop:		rol	a			; Rotate C into Remainder.
		cmp	<__cl			; Test Divisor.
		bcc	.skip			; CC if Divisor > Remainder.
		sbc	<__cl			; Subtract Divisor.
.skip:		rol	<__ax + 0		; Quotient bit -> Dividend LSB.
		rol	<__ax + 1		; Rotate Dividend, MSB -> C.
		rol	<__ax + 2
		rol	<__ax + 3
		dex
		bne	.loop
		sta	<__dl			; Save the remainder.

		leave

		.endp
