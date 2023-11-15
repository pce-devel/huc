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
; mul_8x16u - Simple 16-bit x 8-bit multiply.
;
; Args: _ax = 16-bit Multiplicand / Result
; Args: _cl =  8-bit Multiplier
;

mul_8x16u:	.proc

		cla				; Clear Result.
		clx
		lsr	<_cl			; Shift and test multiplier.
		bcc	.loop

.add:		clc				; Add _ax to the Result.
		adc.l	<_ax
		sax
		adc.h	<_ax
		sax

.loop:		asl.l	<_ax			; _ax = _ax * 2
		rol.h	<_ax
		lsr	<_cl			; Shift and test multiplier.
		bcs	.add
		bne	.loop
		sta.l	<_ax			; Save Result.
		stx.h	<_ax

		leave

		.endp



; ***************************************************************************
; ***************************************************************************
;
; mul_8x24u - Simple 24-bit x 8-bit multiply.
;
; Args: _ax,_bl = 24-bit Multiplicand / Result
; Args: _cl     =  8-bit Multiplier
;

mul_8x24u:	.proc

		cla				; Clear Result.
		clx
		cly
		lsr	<_cl			; Shift and test multiplier.
		bcc	.loop

.add:		clc				; Add _ax to the Result.
		adc	<_ax + 0
		sax				; x = lo-byte, a = mi-byte
		adc	<_ax + 1
		sax
		say				; y = lo-byte, a = hi-byte
		adc	<_ax + 2
		say

.loop:		asl	<_ax + 0		; _ax = _ax * 2
		rol	<_ax + 1
		rol	<_ax + 2
		lsr	<_cl			; Shift and test multiplier.
		bcs	.add
		bne	.loop
		sta	<_ax + 0		; Save Result.
		stx	<_ax + 1
		sty	<_ax + 2

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
;
; Returns: Y=A,Z-flag,N-flag = Remainder.
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
		tay				; Return the remainder.

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
;
; Returns: Y=A,Z-flag,N-flag = Remainder.
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
		tay				; Return the remainder.

		leave

		.endp
