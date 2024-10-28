; ***************************************************************************
; ***************************************************************************
;
; math.asm
;
; Basic (i.e. slow) math routines.
;
; Copyright John Brandwood 2021-2024.
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
; mul_8x8u - Simple 8-bit x 8-bit -> 16-bit unsigned multiply.
;
; Args:    _al       =  8-bit Multiplier
; Args:    _cl       = 16-bit Multiplicand
;
; Returns: _ax       = 16-bit Result
;
; Returns: Y=0,Z-flag,N-flag.
;
; Preserved: _cx,_dx
;
; Derived from Leo J Scanlon's 16x16 multiply in the book "6502 Software
; Design" with an optimization to rotate the result into the multiplier.
;
; This is very similar to TobyLobster's modificaton to the 16x16 multiply
; from "The Merlin 128 Macro Assembler" disk, but with optimized branches.
;
; See https://github.com/TobyLobster/multiply_test/blob/main/tests/mult2.a
;
; Takes between 158..190 cycles, 174 on average.
;
; Note: Unrolling the loop once saves 16 cycles, but IMHO that's not worth
; the extra 8-bytes of code.
;
; Note: Fully unrolling the loop saves 36 cycles, but the extra 49 bytes of
; code mean that it should be a procedure, and then the trampoline overhead
; would actually make it no faster!
;

mul_8x8u:	cla				; Clear top byte of result.

		ldy	#8			; Loop 8 times.

		lsr	<_ax + 0		; Divide multiplier by 2 and
		bcc	.rotate			; clear 8th bit (important).

.add:		clc				; Add the 8-bit multiplicand
		adc	<_cl			; to top 8-bits of the result.

.rotate:	ror	a			; Rotate result into the top
		ror	<_ax + 0		; bits of the multiplier.

		dey
		bcs	.add			; Add multiplicand to top byte?
		bne	.rotate			; Completed 8 loops?

		sta	<_ax + 1		; Save top byte of result.

		rts



; ***************************************************************************
; ***************************************************************************
;
; mul_16x8u - Simple 16-bit x 8-bit -> 24-bit unsigned multiply.
;
; Args:    _al       =  8-bit Multiplier
; Args:    _cx       = 16-bit Multiplicand
;
; Returns: _ax,_bl   = 24-bit Result
;
; Returns: A=Y=0,Z-flag,N-flag.
;
; Preserved: _cx,_dx
;
; Derived from Leo J Scanlon's 16x16 multiply in the book "6502 Software
; Design" with an optimization to rotate the result into the multiplier.
;
; This is very similar to TobyLobster's modificaton to the 16x16 multiply
; from "The Merlin 128 Macro Assembler" disk, but with optimized branches.
;
; See https://github.com/TobyLobster/multiply_test/blob/main/tests/mult2.a
;
; Takes between 243..403 cycles, 323 on average, including 47 cycle procedure
; call overhead vs 14 cycles for a JSR+RTS.
;

mul_16x8u:	.proc

		cla				; Clear top word of result.
		sta	<_ax + 1

		ldy	#8			; Loop 8 times.

		lsr	<_ax + 0		; Divide multiplier by 2 and
		bcc	.rotate			; clear 8th bit (important).

.add:		tax				; Preserve top byte of result.

		clc				; Add the 16-bit multiplicand
		lda	<_ax + 1		; to top 16-bits of the result.
		adc.l	<_cx
		sta	<_ax + 1

		txa				; Restore top byte of result.
		adc.h	<_cx

.rotate:	ror	a			; Rotate result into the top
		ror	<_ax + 1		; bits of the multiplier ...

		ror	<_ax + 0		; and divide multiplier by 2.

		dey
		bcs	.add			; Add multiplicand to top word?
		bne	.rotate			; Completed 8 loops?

		sta	<_ax + 2		; Save top byte of result.

		leave

		.endp



; ***************************************************************************
; ***************************************************************************
;
; mul_16x16u - Simple 16-bit x 16-bit -> 32-bit unsigned multiply.
;
; Args:    _ax       = 16-bit Multiplier
; Args:    _cx       = 16-bit Multiplicand
;
; Returns: _ax,_bx   = 32-bit Result
;
; Returns: A=Y=0,Z-flag,N-flag.
;
; Preserved: _cx,_dx
;
; Derived from Leo J Scanlon's 16x16 multiply in the book "6502 Software
; Design" with an optimization to rotate the result into the multiplier.
;
; This is the same as TobyLobster's modificaton to the 16x16 multiply from
; Dr Jefyll "http://forum.6502.org/viewtopic.php?f=9&t=689&start=0#p19958",
; but with optimized branches.
;
; See https://github.com/TobyLobster/multiply_test/blob/main/tests/mult60.a
;
; Takes between 441..809 cycles, 625 on average, including 47 cycle procedure
; call overhead vs 14 cycles for a JSR+RTS.
;
; Note: Unrolling the outer loop saves 12..60 cycles, but IMHO that's not
; worth the extra 22-bytes of code!
;

mul_16x16u:	.proc

		cla				; Clear top word of result.
		sta	<_ax + 2

		ldx	#-2			; Multiplier is 2 bytes long.

.loop:		ldy	#8			; 8 bits in a byte.

		lsr	<_ax + 2, x		; Divide multiplier by 2 and
		bcc	.rotate			; clear 8th bit (important).

.add:		pha				; Preserve top byte of result.

		clc				; Add the 16-bit multiplicand
		lda	<_ax + 2		; to top 16-bits of the result.
		adc.l	<_cx
		sta	<_ax + 2

		pla				; Restore top byte of result.
		adc.h	<_cx

.rotate:	ror	a			; Rotate result into the top
		ror	<_ax + 2		; bits of the multiplier ...

		ror	<_ax + 2, x		; and divide multiplier by 2.

		dey
		bcs	.add			; Add multiplicand to top word?
		bne	.rotate			; Completed 8 bits?

		inx				; Loop to the next higher byte
		bne	.loop			; in the multiplier.

		sta	<_ax + 3		; Save top byte of result.

		leave

		.endp



; ***************************************************************************
; ***************************************************************************
;
; mul_24x8u - Simple 24-bit x 8-bit -> 32-bit unsigned multiply.
;
; Args:    _al       =  8-bit Multiplier
; Args:    _cx,_dl   = 24-bit Multiplicand
;
; Returns: _ax,_bx   = 32-bit Result
;
; Returns: A=Y=0,Z-flag,N-flag.
;
; Preserved: _cx,_dx
;
; Derived from Leo J Scanlon's 16x16 multiply in the book "6502 Software
; Design" with an optimization to rotate the result into the multiplier.
;
; This is very similar to TobyLobster's modificaton to the 16x16 multiply
; from "The Merlin 128 Macro Assembler" disk, but with optimized branches.
;
; See https://github.com/TobyLobster/multiply_test/blob/main/tests/mult2.a
;
; Takes between 295..551 cycles, 423 on average, including 47 cycle procedure
; call overhead vs 14 cycles for a JSR+RTS.
;

mul_24x8u:	.proc

		cla				; Clear top 24-bits of result.
		sta	<_ax + 2
		sta	<_ax + 1

		ldy	#8			; Loop 8 times.

		lsr	<_ax + 0		; Divide multiplier by 2 and
		bcc	.rotate			; clear 8th bit (important).

.add:		tax				; Preserve top byte of result.

		clc				; Add the 24-bit multiplicand
		lda	<_ax + 1		; to top 24-bits of the result.
		adc	<_cx + 0
		sta	<_ax + 1

		lda	<_ax + 2
		adc	<_cx + 1
		sta	<_ax + 2

		txa				; Restore top byte of result.
		adc	<_cx + 2

.rotate:	ror	a			; Rotate result into the top
		ror	<_ax + 2		; bits of the multiplier ...
		ror	<_ax + 1

		ror	<_ax + 0		; and divide multiplier by 2.

		dey
		bcs	.add			; Add multiplicand to top bits?
		bne	.rotate			; Completed 8 loops?

		sta	<_ax + 3		; Save top byte of result.

		leave

		.endp



; ***************************************************************************
; ***************************************************************************
;
; mul_24x16u - Simple 24-bit x 16-bit -> 40-bit unsigned multiply.
;
; Args:    _ax       = 16-bit Multiplier
; Args:    _cx,_dl   = 24-bit Multiplicand
;
; Returns: _ax,_bx,Y = 40-bit Result
;
; Returns: A=Y,Z-flag,N-flag.
;
; Preserved: _cx,_dx
;
; Derived from Leo J Scanlon's 16x16 multiply in the book "6502 Software
; Design" with an optimization to rotate the result into the multiplier.
;
; This is the same as TobyLobster's modificaton to the 16x16 multiply from
; Dr Jefyll "http://forum.6502.org/viewtopic.php?f=9&t=689&start=0#p19958",
; but with optimized branches.
;
; See https://github.com/TobyLobster/multiply_test/blob/main/tests/mult60.a
;
; Takes between 539..1099 cycles, 819 on average, including 47 cycle procedure
; call overhead vs 14 cycles for a JSR+RTS.
;

mul_24x16u:	.proc

		cla				; Clear top 24-bits of result.
		sta	<_ax + 3
		sta	<_ax + 2

		ldx	#-2			; Multiplier is 2 bytes long.

.loop:		ldy	#8			; 8 bits in a byte.

		lsr	<_ax + 2, x		; Divide multiplier by 2 and
		bcc	.rotate			; clear 8th bit (important).

.add:		pha				; Preserve top byte of result.

		clc				; Add the 24-bit multiplicand
		lda	<_ax + 2		; to top 24-bits of the result.
		adc	<_cx + 0
		sta	<_ax + 2

		lda	<_ax + 3
		adc	<_cx + 1
		sta	<_ax + 3

		pla				; Restore top byte of result.
		adc	<_cx + 2

.rotate:	ror	a			; Rotate result into the top
		ror	<_ax + 3		; bits of the multiplier ...
		ror	<_ax + 2

		ror	<_ax + 2, x		; and divide multiplier by 2.

		dey
		bcs	.add			; Add multiplicand to top bits?
		bne	.rotate			; Completed 8 bits?

		inx				; Loop to the next higher byte
		bne	.loop			; in the multiplier.

		tay				; Save top byte of result.

		leave

		.endp



; ***************************************************************************
; ***************************************************************************
;
; mul_32x8u - Simple 32-bit x 8-bit -> 40-bit unsigned multiply.
;
; Args:    _ax       =  8-bit Multiplier
; Args:    _cx,_dx   = 32-bit Multiplicand
;
; Returns: _ax,_bx,Y = 40-bit Result
;
; Returns: A=Y,Z-flag,N-flag.
;
; Preserved: _cx,_dx
;
; Derived from Leo J Scanlon's 16x16 multiply in the book "6502 Software
; Design" with an optimization to rotate the result into the multiplier.
;
; This is very similar to TobyLobster's modificaton to the 16x16 multiply
; from "The Merlin 128 Macro Assembler" disk, but with optimized branches.
;
; See https://github.com/TobyLobster/multiply_test/blob/main/tests/mult2.a
;
; Takes between 345..697 cycles, 521 on average, including 47 cycle procedure
; call overhead vs 14 cycles for a JSR+RTS.
;

mul_32x8u:	.proc

		cla				; Clear top 32-bits of result.
		sta	<_ax + 3
		sta	<_ax + 2
		sta	<_ax + 1

		ldy	#8			; Loop 8 times.

		lsr	<_ax + 0		; Divide multiplier by 2 and
		bcc	.rotate			; clear 8th bit (important).

.add:		tax				; Preserve top byte of result.

		clc				; Add the 32-bit multiplicand
		lda	<_ax + 1		; to top 32-bits of the result.
		adc	<_cx + 0
		sta	<_ax + 1

		lda	<_ax + 2
		adc	<_cx + 1
		sta	<_ax + 2

		lda	<_ax + 3
		adc	<_cx + 2
		sta	<_ax + 3

		txa				; Restore top byte of result.
		adc	<_cx + 3

.rotate:	ror	a			; Rotate result into the top
		ror	<_ax + 3		; bits of the multiplier ...
		ror	<_ax + 2
		ror	<_ax + 1

		ror	<_ax + 0		; and divide multiplier by 2.

		dey
		bcs	.add			; Add multiplicand to top bits?
		bne	.rotate			; Completed 8 loops?

		tay				; Save top byte of result.

		leave

		.endp



; ***************************************************************************
; ***************************************************************************
;
; div16_7u - Simple 16-bit /  7-bit divide.
;
; Args: _ax = Dividend / Quotient
; Args: _bx = Dividend / Quotient (if 32-bit)
; Args: _cl = Divisor
;
; Returns: Y=A,Z-flag,N-flag = Remainder.
;
; Preserved: _cx,_dx
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
; Preserved: _cx,_dx
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
