; ***************************************************************************
; ***************************************************************************
;
; hucc-printf.asm
;
; A small and simple printf() and sprintf() implementation for HuCC.
;
; Copyright John Brandwood 2019-2025.
;
; Distributed under the Boost Software License, Version 1.0.
; (See accompanying file LICENSE_1_0.txt or copy at
;  http://www.boost.org/LICENSE_1_0.txt)
;
; ***************************************************************************
; ***************************************************************************
;
; WARNING: This only supports a subset of POSIX printf()/sprintf() behavior!
; WARNING: <width>, <precision> and numbers in ESC-sequences must be <= 127!
; WARNING: This only supports a maximum of 4 data arguments!
; WARNING: sprintf() must output less than 256 characters!
;
; ***************************************************************************
; ***************************************************************************
;
; A format specifier follows this prototype:
;
;   '%' [<flags>] [<width>] ['.' <precision>] [<length>] <specifier>
;
; <flags>
;
;   ('-' | '0' | '+' | ' ') = one or more of the following ...
;
;    '-' to left-justify output within the <width>
;    '0' to pad to <width> with zero instead of space
;    '+' to prepend positive integers with a '+' when "%d", "%i" or "%u"
;    ' ' to prepend positive integers with a ' ' when "%d", "%i" or "%u"
;
; <width>
;
;   (<decimal-number> | '*') = "1..127" or read value from an argument
;
; <precision>
;
;   (<decimal-number> | '*') = "1..127" or read value from an argument
;
; <length>
;
;   ('l') = only "%x" and "%X" : read two 16-bit arguments as a 32-bit value
;
; <specifier>
;
;   '%' = just print a '%'
;   'c' = print an ASCII character
;   'd' = print a signed integer variable in decimal
;   'i' = print a signed integer variable in decimal
;   's' = print string, stop at <precision> characters
;   'u' = print an unsigned integer variable in decimal
;   'x' = print an unsigned binary (or BCD) variable in hex
;   'X' = print an unsigned binary (or BCD) variable in hex
;
; Note:
;
;   HuCC follows GCC's behavior, except for "%u" with a '+' ...
;
;     If both '+' and ' ' flags are used, then '+' overrides the ' ' flag.
;     "%u" is unsigned-by-definition and treats '+' as ' ' (GCC ignores '+').
;     "%x", "%X", "%c" and "%s" ignore the '+' and ' ' flags.
;
; ***************************************************************************
;
; printf() and puts() output accept virtual-terminal control codes:
;
; <control-code>
;
;   '\n' = move cursor-X to XL, cursor-Y down 1 line.
;   '\r' = move cursor-X to XL.
;   '\f' = clear-screen + home cursor-X and cursor-Y to (XL, YT)
;
; ***************************************************************************
;
; printf() and puts() output accept virtual-terminal escape sequences:
;
;   ('\e' | '\x1B') <decimal-number> <escape-code>
;
; <escape-code>
;
;   'X' = set current X coordinate (0..127)
;   'Y' = set current Y coordinate (0..63)
;   'L' = set minimum X coordinate (0..127)
;   'T' = set minimum Y coordinate (0..63)
;   'P' = set current palette (0..15)
;
; ***************************************************************************
; ***************************************************************************

;
; Include dependancies ...
;

		include "common.asm"		; Common helpers.



;
; Variables that are remembered between invocations.
;

	.if	0
; **************
; 16-bytes of VDC TTY information (moved to hucc-gfx.asm to save .bss memory).
;
; N.B. MUST be 16-bytes before the SGX versions to use PCE_VDC_OFFSET.

vdc_tty_x_lhs:	ds	1	; TTY minimum X position.
vdc_tty_y_top:	ds	1	; TTY minimum Y position.
vdc_tty_x:	ds	1	; TTY current X position.
vdc_tty_y:	ds	1	; TTY current Y position.

	.if	SUPPORT_SGX

; **************
; 16-bytes of SGX TTY information (moved to hucc-gfx.asm to save .bss memory).
;
; N.B. MUST be 16-bytes after the VDC versions to use SGX_VDC_OFFSET.

		ds	12	; Padding to ensure the 16-byte delta.

sgx_tty_x_lhs:	ds	1	; TTY minimum X position.
sgx_tty_y_top:	ds	1	; TTY minimum Y position.
sgx_tty_x:	ds	1	; TTY current X position.
sgx_tty_y:	ds	1	; TTY current Y position.

	.endif
	.endif

;
; Temporary variables for each invocation using common zero-page locations.
;
; N.B. TTY form-feed output uses _ax and _bl!
;

tty_out1st	=	_al	; Is there a +/- sign?
tty_outpad	=	_ah	; Pad with zero, not space.
tty_outlhs	=	_bl	; Left justify, not right.
tty_outmin	=	_bh	; Minimum output width.
tty_outmax	=	_cl	; Maximum output width.
tty_outstk	=	_ch	; SP of the end of a converted string.
tty_vararg	=	_dl	; Index of next vararg parameter.
tty_outcnt	=	_dh	; Number of characters written.

tty_vdc		=	__ptr+0	; SGX_VDC_OFFSET or zero.
tty_xyok	=	__ptr+1	; Is the VDC_MAWR register correct?

tty_vector	=	__fptr	; Output vector for writing.



; ***************************************************************************
; ***************************************************************************
;
; int __fastcall sprintf( unsigned char *string<_di>, unsigned char *format<_bp>, unsigned int vararg1<__vararg1>, unsigned int vararg2<__vararg2>, unsigned int vararg3<__vararg3>, unsigned int vararg4<__vararg3> );
; int __fastcall printf( unsigned char *format<_bp>, unsigned int vararg1<__vararg1>, unsigned int vararg2<__vararg2>, unsigned int vararg3<__vararg3>, unsigned int vararg4<__vararg3> );
; int __fastcall sgx_printf( unsigned char *format<_bp>, unsigned int vararg1<__vararg1>, unsigned int vararg2<__vararg2>, unsigned int vararg3<__vararg3>, unsigned int vararg4<__vararg3> );
;
; Also uses: _si, _di, _ax, _bx, _cx, _dx, __ptr
;
; This is a reasonably-compliant C "printf", but don't expect wonders!
;
; Each number that is written is converted onto the stack before the output
; so that it can be justified and/or padded.
;

printf_group	.procgroup			; These routines share code!

_sprintf.2	.proc				; HuC entry point.

		lda.l	#write_to_str		; Output to a string.
		sta.l	tty_vector
		lda.h	#write_to_str
		sta.h	tty_vector

		ldx	#-1			; Set -ve VDC to signal output
		bra	!printf+		; to a string.

		.ref	_printf.1		; Need _printf.1
		.endp

	.if	SUPPORT_SGX

_sgx_printf.1	.proc

		ldx	#SGX_VDC_OFFSET		; Offset to SGX VDC.
		db	$F0			; Turn "clx" into a "beq".

		.ref	_printf.1		; Need _printf.1
		.endp
	.endif

_printf.1	.proc				; HuC entry point.

		clx				; Offset to PCE VDC.

		lda.l	#write_to_tty		; Output to one of the VDCs.
		sta.l	tty_vector
		lda.h	#write_to_tty
		sta.h	tty_vector

!printf:	stx	<tty_vdc		; Remember which VDC to use.

		stz	<tty_outcnt		; Keep track of bytes written.
		stz	<tty_xyok		; Make sure to set the MAWR!

		lda.l	#__vararg1 - 2		; Set up 1st vararg parameter
		sta	<tty_vararg		; for read with pre-increment.

		;
		; Read and display the next character.
		;

		cly				; First CHR in string.
.next_chr:	lda	[_bp], y		; Read the next chr to print.
		beq	.finished
		iny

		cmp	#'%'			; Is it a format-specifier?
		beq	.format

.got_chr:	jsr	.write_chr		; Preserves Y register!
		bra	.next_chr

		;
		; Got end-of-string.
		;

.finished:	ldx	<tty_outcnt		; Return #bytes written.
		cly

.exit:		bbr7	<tty_vdc, !+		; Is the output to a string?

		cla				; Terminate the output string.
		sta	[_di]

!:		leave				; All done!

		;
		; Got a "-" flag!
		;

.flag_minus:	dec	<tty_outlhs		; Set bit6 for yes so that it
		bra	.read_flag		; can be tested with a "bit".

		;
		; Got a " " flag!
		;

.flag_space:	ldx	<tty_out1st		; Ignore if already set which
		bne	.read_flag		; allows precedence for '+'.

		;
		; Got a "+" flag!
		;

.flag_plus:	sta	<tty_out1st		; Set '+' for yes.
		bra	.read_flag

		;
		; Got a "0" flag!
		;

.flag_zero:	sta	<tty_outpad		; Set '0' for yes.
		bra	.read_flag

		;
		; Got a "%" format sequence!
		;

.format:	lda	#' '			; Default to pad with space.
		sta	<tty_outpad
		stz	<tty_out1st		; Is there a +/- sign?
		stz	<tty_outlhs		; Left justify, not right.
		stz	<tty_outmin		; Clear the minimum output.
		stz	<tty_outmax		; Clear the maximum output.

		;
		; Read the format flags.
		;

.read_flag:	lda	[_bp], y		; Read the next flag.
		iny
		cmp	#'%'			; Just print a '%'.
		beq	.got_chr

		cmp	#'9'+1			; Ignore these if above '9'.
		bcs	.got_specifier
		cmp	#'0'			; Pad with zero?
		beq	.flag_zero
		bcs	.get_width		; Branch if '1..9'.
		cmp	#'-'			; Left-justify?
		beq	.flag_minus
		cmp	#'+'			; Prepend a '+' sign?
		beq	.flag_plus
		cmp	#' '			; Prepend a ' ' sign?
		beq	.flag_space

		;
		; Read the field width (minimum #chrs).
		;

		cmp	#'*'			; Or read the width from
		bne	.get_precision		; a parameter.

		jsr	.read_param		; Read next parameter.
		bmi	!+			; Ignore -ve width.
		sta	<tty_outmin		; Set the field width.

!:		lda	[_bp], y		; Get the next character.
		iny
		bra	.get_precision

.get_width:	sbc	#'0'			; Initialize the width.
		jsr	next_decimal		; Read the rest, return next.
		ldx	<__temp			; Set the minimum output.
		stx	<tty_outmin

		;
		; Read the output precision (maximum #chrs).
		;

.get_precision:	cmp	#'.'			; Is this a precision field?
		bne	.get_length

		rmb4	<tty_outpad		; Pad with ' ' not '0'.

		jsr	read_decimal		; Read the precision value.
		ldx	<__temp			; Set the maximum output.
		stx	<tty_outmax

		cmp	#'*'			; Or read the precision from
		bne	.get_length		; a parameter.

		jsr	.read_param		; Read next parameter.
		bmi	!+			; Ignore -ve <precision>.
		sta	<tty_outmax

!:		lda	[_bp], y		; Get the next character.
		iny

		;
		; Read the parameter length (only for long-hex output).
		;

.get_length:	cmp	#'l'
		bne	.got_specifier
		sta	<tty_out1st		; Overload for long BCD.

		lda	[_bp], y		; Get the next character.
		iny

		; Check the output format specifier.

.got_specifier:	phy				; Preserve string index.

		cmp	#'s'
		beq	.string

		clx				; Push the output terminator.
		phx
		tsx				; Remember the end address of
		stx	<tty_outstk		; characters put on the stack.

		cmp	#'d'			; These signed conversions obey
		beq	.signed			; the '+' and ' ' flags.
		cmp	#'i'
		beq	.signed

		cmp	#'u'			; Unsigned conversions clear
		beq	.unsigned		; tty_out1st before output.
		ldy	#'A' - ('9' + 1 + 1)
		cmp	#'X'
		beq	.hexadecimal
		ldy	#'a' - ('9' + 1 + 1)
		cmp	#'x'
		beq	.hexadecimal
		cmp	#'c'
		beq	.character

.bad_format:	ply				; Restore string index.

		ldx.l	#-1			; Return -ve error code.
		ldy.h	#-1
		bra	.exit

		;
		; String.
		;

.string:	ldx	<tty_vararg		; Restore params index.
		inx
		inx
		stx	<tty_vararg		; Preserve params index.
		lda.l	<0, x			; Read pointer lo-byte.
		sta.l	<_si
		lda.h	<0, x			; Read pointer hi-byte.
		sta.h	<_si
		ldy	<tty_outmax		; Get the maximum output.
		beq	.str_done

.str_loop:	lda	[_si]
		beq	.str_done
		inc.l	<_si
		bne	!+
		inc.h	<_si
!:		jsr	.write_chr
		dey
		bne	.str_loop

.str_done:	ply				; Restore string index.
		jmp	.next_chr

		;
		; Character.
		;

.character:	jsr	.read_param		; Read the character into A.
		pha
		rmb4	<tty_outpad		; Pad with ' ' not '0'.
		stz	<tty_out1st		; Remove ' ' or '+'.

		bra	.do_width		; Ignore the precision.

		;
		; Signed/Unsigned decimal integer output.
		;

.signed:	jsr	.read_param		; Read the signed dividend.

		bbr7	<dividend+1, .divide_by_ten

.negate:	sec				; Negate the parameter to make
		cla				; it positive.
		sbc.l	<dividend
		sta.l	<dividend
		cla
		sbc.h	<dividend
		sta.h	<dividend
		lda	#'-'			; Prepend a '-' sign!
		sta	<tty_out1st
		bra	.divide_by_ten

.unsigned:	jsr	.read_param		; Read the decimal dividend.

		lda	#$0F			; Convert '+' into ' ' while
		trb	<tty_out1st		; not changing zero.

.divide_by_ten:	ldx	#16
		cla				; Clear Remainder.
		asl.l	<dividend		; Rotate Dividend, MSB -> C.
		rol.h	<dividend
.divide_loop:	rol	a			; Rotate C into Remainder.
		cmp	#10			; Test Divisor.
		bcc	.divide_less		; CC if Divisor > Remainder.
		sbc	#10			; Subtract Divisor.
.divide_less:	rol.l	<dividend		; Quotient bit -> Dividend LSB.
		rol.h	<dividend		; Rotate Dividend, MSB -> C.
		dex
		bne	.divide_loop

		ora	#'0'			; Remainder onto the stack.
		pha
		lda.l	<dividend		; Repeat while non-zero.
		ora.h	<dividend
		bne	.divide_by_ten

		bra	.do_precision		; Justify the output.

		;
		; Hex (and BCD) unsigned integer output.
		;

.hexadecimal:	sty	<__temp			; Upper or lower case hex digit.

.hex_word:	jsr	.read_param		; Read a hexadecimal word.
		clx
.hex_byte:	lda	<dividend, x		; Convert 1 byte at a time.
		and	#$0F
		ora	#'0'			; Print a hex/bcd nibble.
		cmp	#'9' + 1
		bcc	!+
		adc	<__temp			; Upper or lower case hex digit.
!:		pha				; Nibble onto the stack.
		lda	<dividend, x
		lsr	a
		lsr	a
		lsr	a
		lsr	a
		ora	#'0'			; Print a hex/bcd nibble.
		cmp	#'9' + 1
		bcc	!+
		adc	<__temp			; Upper or lower case hex digit.
!:		pha				; Nibble onto the stack.
		inx
		cpx	#2
		bne	.hex_byte

		lda	<tty_out1st		; Test if a long hex parameter.
		stz	<tty_out1st		; No prepend for unsigned.
		cmp	#'l'
		beq	.hex_word

.strip_zero:	pla				; Strip leading '0' characters
		beq	!+			; from the hex string.
		cmp	#'0'
		beq	.strip_zero
!:		pha
		bne	.do_precision
		lda	#'0'			; If the hex number is all '0'.
		pha

.do_precision:	tsx				; Calc how many chrs are on
		txa				; the stack.
		eor	#$FF
		sec
		adc	<tty_outstk		; Always leaves C set.
		sbc	<tty_outmax		; Is there a precision?
		bcs	.do_width

		ldx	#'0'			; Pad to precision with '0'.
.pad_digit:	phx
		inc	a
		bne	.pad_digit

.do_width:	tsx				; Calc how many chrs are on
		txa				; the stack.
		eor	#$FF
		sec
		adc	<tty_outstk		; Always leaves C set.
		sbc	<tty_outmin		; Subtract the field width.
		tay				; Y = negative #chars to pad.
		bpl	.pad_sign

.pad_field:	bit	<tty_outlhs		; Set V if left-justified.
		lda	<tty_out1st		; Is there a sign to output?
		beq	.no_sign

		iny
		bpl	.out_sign		; Still space left to pad?
		bvs	.out_sign		; Is sign left-justified?

.no_sign:	bvs	.pop_digit		; Is field left-justified?

		bbs4	<tty_outpad, .pad_sign	; Output sign before '0' padding.

.pad_char:	lda	<tty_outpad		; Pad the LHS with ' ' or '0'.
		bsr	.write_chr		; Preserves Y!
		iny
		bmi	.pad_char

.pad_sign:	lda	<tty_out1st		; Is there a sign to output?
		beq	.chk_more
		bsr	.write_chr		; Preserves Y!
		stz	<tty_out1st		; Don't output the sign twice!

.chk_more:	tya				; Is there still padding?
		bmi	.pad_char

.pop_digit:	pla				; Pop the digits and output.
		beq	.chk_rhs
.out_sign:	jsr	.write_chr		; Preserves Y!
		bra	.pop_digit

.chk_rhs:	tya				; Negative if pad RHS.
		bpl	.output_done

.pad_rhs:	lda	#' '			; Pad the RHS with ' '.
		bsr	.write_chr		; Preserves Y!
		iny
		bmi	.pad_rhs

.output_done:	ply				; Restore string index.
		jmp	.next_chr

		;
		; Read a 16-bit data value, return with lo-byte in A.
		;

.read_param:	ldx	<tty_vararg		; Restore params index.
		inx
		inx
		stx	<tty_vararg		; Preserve params index.
		lda.h	<0, x			; Read data hi-byte.
		sta.h	<dividend
		lda.l	<0, x			; Read data lo-byte.
		sta.l	<dividend
!:		rts

		;
		; Write out an ASCII character.
		;
		; N.B. Y is preserved!
		;

.write_chr:	inc	<tty_outcnt		; Keep track of bytes written.

		jmp	[tty_vector]		; Vector to the output routine.

		;
		; Read decimal number, return with next in A.
		;
		; N.B. X is preserved!
		;

read_decimal:	cla
next_decimal:	sta	<__temp			; Initialize to zero.
		lda	[_bp], y		; Read char from string.
		iny
		cmp	#'9'+1			; Is this a decimal digit?
		bcs	!+
		cmp	#'0'
		bcc	!+
		sbc	#'0'			; Accumulate the decimal value
		asl	<__temp			; within the range (0..255).
		adc	<__temp
		asl	<__temp
		asl	<__temp
		adc	<__temp
		bpl	next_decimal
		bra	read_decimal		; Reset overflow or -ve number.

		;
		; Default output vector for sprintf().
		;
		; N.B. Y is preserved!
		;

write_to_str:	sta	[_di]			; Write the ASCII character to
		inc.l	<_di			; the string in RAM.
		bne	!+
		inc.h	<_di
!:		rts

		;
		; Default output vector for printf().
		;
		; N.B. Y is preserved!
		;

write_to_tty:	ldx	<tty_vdc		; Which VDC?

		cmp	#' '			; Is it an control code?
		bcc	.got_ctl

		bbs0	<tty_xyok, !+		; Is the VDC's MAWR valid?

		inc	<tty_xyok		; Set up the MAWR from the tty
		pha				; coordinates.
		lda	_vdc_tty_x, x
		sta.l	<_di
		lda	_vdc_tty_y, x
;		sec
;		sbc	_vdc_tty_0, x
;		and	vdc_bat_y_mask, x
		sta.h	<_di
		jsr	set_di_xy_mawr
		pla

!:		clc				; Write the ASCII character in
		adc.l	_vdc_font_base, x	; the current font and palette.
		sta	VDC_DL, x
		cla
		adc.h	_vdc_font_base, x
		sta	VDC_DH, x
		inc	_vdc_tty_x, x
		rts

		;
		; Handle TTY control codes (ASCII < $20).
		;

.got_ctl:	cmp	#'\e'			; Is it an escape-sequence?
		beq	.escape
		cmp	#'\n'			; Which control code?
		beq	got_tty_lf
		cmp	#'\r'
		beq	got_tty_cr
		cmp	#'\f'
		beq	got_tty_ff
		rts				; Ignore other control codes.

		;
		; Handle TTY escape sequences.
		;

.escape:	jsr	read_decimal		; Read decimal, return next.
		cmp	#'X'
		beq	.set_x_pos
		cmp	#'Y'
		beq	.set_y_pos
		cmp	#'P'
		beq	.set_palette
		cmp	#'L'
		beq	.set_x_lhs
		cmp	#'T'
		beq	.set_y_top
		rts				; Ignore other escape sequences.

		;
		; Set Cursor X coordinate.
		;

.set_x_pos:	lda	<__temp			; Get decimal value.
		sta	_vdc_tty_x, x
		stz	<tty_xyok
		rts

.set_x_lhs:	lda	<__temp			; Get decimal value.
		sta	_vdc_tty_x_lhs, x
		rts

		;
		; Set Cursor Y coordinate.
		;

.set_y_pos:	lda	<__temp			; Get decimal value.
		sta	_vdc_tty_y, x
		stz	<tty_xyok
		rts

.set_y_top:	lda	<__temp			; Get decimal value.
		sta	_vdc_tty_y_top, x
		rts

		;
		; Change Palette
		;

.set_palette:	lda	<__temp			; Get decimal value.
		asl	a
		asl	a
		asl	a
		asl	a
		sta	<__temp
		lda.h	_vdc_font_base, x	; Change the current palette.
		and	#$0F
		ora	<__temp
		sta.h	_vdc_font_base, x
		rts

		;
		; Handle LF (includes CR).
		;

got_tty_lf:	lda	_vdc_tty_y, x
		inc	a
		and	vdc_bat_y_mask, x
		sta	_vdc_tty_y, x

		;
		; Handle CR.
		;

got_tty_cr:	lda	_vdc_tty_x_lhs, x
		sta	_vdc_tty_x, x
		stz	<tty_xyok
		rts

		;
		; Handle FF (i.e. clear screen).
		;

got_tty_ff:	phy				; Preserve string index.
		jsr	clear_tty_x		; Clear the screen.
		ply				; Restore string index.

		lda	_vdc_tty_x_lhs, x	; Home the cursor.
		sta	_vdc_tty_x, x
		lda	_vdc_tty_y_top, x
		sta	_vdc_tty_y, x
		stz	<tty_xyok
!:		rts

		.endp

_printf.2	.alias	_printf.1
_printf.3	.alias	_printf.1
_printf.4	.alias	_printf.1
_printf.5	.alias	_printf.1

_sgx_printf.2	.alias	_sgx_printf.1
_sgx_printf.3	.alias	_sgx_printf.1
_sgx_printf.4	.alias	_sgx_printf.1
_sgx_printf.5	.alias	_sgx_printf.1

_sprintf.3	.alias	_sprintf.2
_sprintf.4	.alias	_sprintf.2
_sprintf.5	.alias	_sprintf.2
_sprintf.6	.alias	_sprintf.2



; ***************************************************************************
; ***************************************************************************
;
; void __fastcall __macro puts( unsigned char *string<_bp> );
; void __fastcall __macro sgx_puts( unsigned char *string<_bp> );

		.macro	_sgx_puts.1
		ldx	#SGX_VDC_OFFSET
		call	puts_x
		.endm

		.macro	_puts.1
		clx
		call	puts_x
		.endm

puts_x		.proc				; HuC entry point.

		stx	<tty_vdc		; Remember which VDC to use.
		stz	<tty_xyok		; Make sure to set the MAWR!

		cly				; First CHR in string.
.next_chr:	lda	[_bp], y		; Read the next chr to print.
		beq	.finished
		iny
		jsr	write_to_tty		; Preserves Y register!
		bra	.next_chr

.finished:	jsr	got_tty_lf		; Silly standard behavior.

		leave

		.endp



; ***************************************************************************
; ***************************************************************************
;
; void __fastcall __macro putchar( unsigned char ascii<_bp> );
; void __fastcall __macro sgx_putchar( unsigned char *string<_bp> );

		.macro	_sgx_putchar.1
		tay
		ldx	#SGX_VDC_OFFSET
		call	putchar_x
		.endm

		.macro	_putchar.1
		tay
		clx
		call	putchar_x
		.endm

putchar_x	.proc				; HuC entry point.

		stx	<tty_vdc		; Remember which VDC to use.
		stz	<tty_xyok		; Make sure to set the MAWR!

		tya
		jsr	write_to_tty		; Preserves Y register!

		leave

		.endp

		.endprocgroup	; printf_group
