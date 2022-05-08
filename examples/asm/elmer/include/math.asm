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
; Args: _ax = Multiplier / Result
; Args: _bl = Multiplier
;

mul8u		.proc

		cla				; Clear Result.
		clx
		lsr	<_bl			; Shift and test multiplier.
		bcc	.loop
.add:		clc				; Add _ax to the Result.
		adc	<_al
		sax
		adc	<_ah
		sax
.loop:		asl	<_al			; _ax = _ax * 2
		rol	<_ah
		lsr	<_bl			; Shift and test multiplier.
		bcs	.add
		bne	.loop
		sta	<_al			; Save Result.
		stx	<_ah

		leave

		.endp



; ***************************************************************************
; ***************************************************************************
;
; div16_7u - Simple 16-bit /  7-bit divide.
;
; Args: _ax = Dividend / Quotient
; Args: _bx = Dividend / Quotient (if 32-bit)
; Args: _cl = Dividor
; Uses: _dl = Remainder
;

div16_7u	.proc

		ldx	#16
		cla				; Clear Remainder.
		asl	<_ax + 0		; Rotate Dividend, MSB -> C.
		rol	<_ax + 1
.loop:		rol	a			; Rotate C into Remainder.
		cmp	<_cl			; Test Divisor.
		bcc	.less			; CC if Divisor > Remainder.
		sbc	<_cl			; Subtract Divisor.
.less:		rol	<_ax + 0		; Quotient bit -> Dividend LSB.
		rol	<_ax + 1		; Rotate Dividend, MSB -> C.
		dex
		bne	.loop
		sta	<_dl			; Save the remainder.

		leave

		.endp



; ***************************************************************************
; ***************************************************************************
;
; div32_7u - Simple 32-bit /  7-bit divide.
;
; Args: _ax = Dividend / Quotient
; Args: _bx = Dividend / Quotient (if 32-bit)
; Args: _cl = Dividor
; Uses: _dl = Remainder
;

div32_7u	.proc

		ldx	#32
		cla				; Clear Remainder.
		asl	<_ax + 0		; Rotate Dividend, MSB -> C.
		rol	<_ax + 1
		rol	<_ax + 2
		rol	<_ax + 3
.loop:		rol	a			; Rotate C into Remainder.
		cmp	<_cl			; Test Divisor.
		bcc	.skip			; CC if Divisor > Remainder.
		sbc	<_cl			; Subtract Divisor.
.skip:		rol	<_ax + 0		; Quotient bit -> Dividend LSB.
		rol	<_ax + 1		; Rotate Dividend, MSB -> C.
		rol	<_ax + 2
		rol	<_ax + 3
		dex
		bne	.loop
		sta	<_dl			; Save the remainder.

		leave

		.endp
