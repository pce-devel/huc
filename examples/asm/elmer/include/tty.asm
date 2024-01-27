; ***************************************************************************
; ***************************************************************************
;
; tty.asm
;
; TTY terminal output functions, for debugging or simple menus.
;
; Copyright John Brandwood 2019-2022.
;
; Distributed under the Boost Software License, Version 1.0.
; (See accompanying file LICENSE_1_0.txt or copy at
;  http://www.boost.org/LICENSE_1_0.txt)
;
; ***************************************************************************
; ***************************************************************************

;
; Include dependancies ...
;

		include "common.asm"		; Common helpers.
		include "math.asm"		; Math routines.

;
; Last test 2022-05-16:
;
; $C000 = tty_printf
; $C32a = everthing off
; $C3e3 = with box
; $C441 = with unsigned int
; $C461 = with unsigned and signed int
; $C477 = with unsigned and signed int and 32-bit
; $C49c = with unsigned and signed int and 32-bit and '.' precision
;

TTY_NO_DOT	=	0	; Remove support for '.' precision?
TTY_NO_INT	=	0	; Remove support for printing integers?
TTY_NO_NEG	=	0	; Remove support for negative integers?
TTY_NO_INT32	=	0	; Remove support for long integers?
TTY_NO_BOX	=	0	; Remove support for drawing boxes?
TTY_NO_0HEX	=	0	; Remove final '0' from a 0 hex value.

;
; A macro to make it easier to call ...
;

STRING		.macro
	.if	\?1 == ARG_STRING
;		.data
!string:	db	(!end+ - !string-)
		db	\1
!end:		db	0
	.endif	\?1 == ARG_STRING
		.endm

PRINTF		.macro
	.if	\?1 == ARG_STRING
		.data
!string:	db	(!end+ - !string-)
		db	\1
!end:		db	0

		.if	\?2 != 0
		.if	(\?2 == ARG_LABEL) | (\?2 == ARG_ABSOLUTE)
		dw	\2
		.else
		.fail	All arguments to printf macro must be pointers!
		.endif

		.if	\?3 != 0
		.if	(\?3 == ARG_LABEL) | (\?3 == ARG_ABSOLUTE)
		dw	\3
		.else
		.fail	All arguments to printf macro must be pointers!
		.endif

		.if	\?4 != 0
		.if	(\?4 == ARG_LABEL) | (\?4 == ARG_ABSOLUTE)
		dw	\4
		.else
		.fail	All arguments to printf macro must be pointers!
		.endif

		.if	\?5 != 0
		.if	(\?5 == ARG_LABEL) | (\?5 == ARG_ABSOLUTE)
		dw	\5
		.else
		.fail	All arguments to printf macro must be pointers!
		.endif

		.if	\?6 != 0
		.if	(\?6 == ARG_LABEL) | (\?6 == ARG_ABSOLUTE)
		dw	\6
		.else
		.fail	All arguments to printf macro must be pointers!
		.endif

		.if	\?7 != 0
		.if	(\?7 == ARG_LABEL) | (\?7 == ARG_ABSOLUTE)
		dw	\7
		.else
		.fail	All arguments to printf macro must be pointers!
		.endif

		.if	\?8 != 0
		.if	(\?8 == ARG_LABEL) | (\?8 == ARG_ABSOLUTE)
		dw	\8
		.else
		.fail	All arguments to printf macro must be pointers!
		.endif

		.if	\?9 != 0
		.if	(\?9 == ARG_LABEL) | (\?9 == ARG_ABSOLUTE)
		dw	\9
		.else
		.fail	All arguments to printf macro must be pointers!
		.endif

		.endif	\?9 != 0
		.endif	\?8 != 0
		.endif	\?7 != 0
		.endif	\?6 != 0
		.endif	\?5 != 0
		.endif	\?4 != 0
		.endif	\?3 != 0
		.endif	\?2 != 0

		.if	(* - !string-) > 256
		.fail	printf string+arguments cannot be longer than 256 bytes!
		.endif

		.code
		ldx	#<!string-
		lda	#>!string-
		ldy	#^!string-
		jsr	tty_printf
	.else
		ldx	#<\1
		lda	#>\1
		ldy	#^\1
		jsr	tty_printf
	.endif	\?1 == ARG_STRING
		.endm

;
;
;

		.bss

tty_xlhs:	ds	1			; TTY minimum X position.
tty_ytop:	ds	1			; TTY minimum Y position.
tty_xpos:	ds	1			; TTY current X position.
tty_ypos:	ds	1			; TTY current Y position.
tty_xyok:	ds	1			; Is the VDC_MAWR correct?
tty_8x16:	ds	1			; NZ if using an 8x16 font.
tty_tile:	ds	2			; Current base tile# (CHR 0).

tty_param:	ds	1                       ; Index of next parameter.

tty_out1st:	ds	1			; Is there a +/- sign?
tty_outpad:	ds	1			; Pad with zero, not space.
tty_outlhs:	ds	1                       ; Left justify, not right.
tty_outmin:	ds	1			; Minimum output width.
tty_outmax:	ds	1			; Maximum output width.
tty_outstk:	ds	1                       ;

		.code



; ***************************************************************************
; ***************************************************************************
;
; tty_printf - A formatted-print routine for text output.
;
; Args: _bp, _bp_bank = _farptr to string that will be mapped into MPR3.
;
; Uses: _bp, _di, _ax, _bx, _cx, _dx
;
; Preserves: _si
;
; This is NOT a standard C "printf", so do not expect it to behave like one!
;
; But it provides a lot of similar functionality, with a different syntax.
;
;

tty_printf:	stx.l	<_bp			; Preserve message pointer.
		sta.h	<_bp
		jmp	tty_printf_huc		; Map in the procedure code.



tty_group	.procgroup			; Group main TTY in 1 bank!

; ***************************************************************************
; ***************************************************************************
;
; tty_printf - A formatted-print routine for text output.
;
; Args: _bp, _bp_bank = _farptr to string that will be mapped into MPR3.
;
; Uses: _bp, _di, _ax, _bx, _cx, _dx
;
; Preserves: _si
;
; This is NOT a standard C "printf", so do not expect it to behave like one!
;
; But it provides a lot of similar functionality, with a different syntax.
;
;
; ***************************************************************************
;
; A virtual-terminal escape code follows this prototype:
;
;   ('\e' | '\x1B') <escape-code>
;
; <escape-code>
;
;   'X' 'L' <decimal-coordinate> = set minimum X coordinate (0..127)
;   'Y' 'T' <decimal-coordinate> = set minimum Y coordinate (0..31)
;   'X' <decimal-coordinate> = set current X coordinate (0..127)
;   'Y' <decimal-coordinate> = set current Y coordinate (0..31)
;   'P' <hexadecimal-digit> = set current palette (0..9, A..F, a..f)
;   'B' <decimal-width> ',' <decimal-height> ',' <decimal-type> = draw box
;
;   '<' = select  8-pixel-high font (at VRAM tile# CHR_ZERO)
;   '>' = select 16-pixel-high font (at VRAM tile# CHR_TALL)
;   'T' <16bit-binary-tile#> = select any VRAM tile# as base
;
;
; ***************************************************************************
;
; A virtual-terminal control code follows this prototype:
;
; <control-code>
;
;   '\n' = move cursor-X to XL, cursor-Y down 1 line.
;   '\r' = move cursor-X to XL.
;   '\f' = clear-screen + home cursor-X and cursor-Y at (XL, YT).
;
;
; ***************************************************************************
;
; A format specifier follows this prototype:
;
;   '%' [<flags>] [<minwidth>] ['.' <maxwidth>] [<length>] <specifier>
;
; <flags>
;
;   ('-' | '0' | '+' | ' ') = one or more of the following ...
;
;    '-' to left-justify
;    '0' to pad to <minwidth> with zero instead of space
;    '+' to prepend positive integer numbers with a '+'
;    ' ' to prepend positive integer numbers with a ' '
;
; <minwidth>
;
;   (<decimal-width> | '*' <16bit-pointer> | '#') = "1..127" or read 1-byte
;
; <maxwidth>
;
;   (<decimal-width> | '*' <16bit-pointer> | '#') = "1..127" or read 1-byte
;
; <length>
;
;   ('h' | 'l') = read 1-byte or 4-bytes from pointer instead of 2-bytes
;
; <specifier>
;
;   '%' = just print a '%'
;
;   'd' <16bit-pointer> = print a signed integer variable in decimal
;   'u' <16bit-pointer> = print an unsigned integer variable in decimal
;   'x' <16bit-pointer> = print an unsigned binary (or BCD) variable in hex
;   'c' <16bit-pointer> = print <minwidth> ASCII characters from array
;   's' <16bit-pointer> = print string, stop at <maxwidth> characters
;
;   'D' = print a signed integer variable in decimal
;   'U' = print an unsigned integer variable in decimal
;   'X' = print an unsigned binary (or BCD) variable in hex
;   'C' = print <minwidth> ASCII characters from array
;   'S' = print string, stop at <maxwidth> characters
;
; <16bit-pointer>
;
;   '*','d','u','x','c','s' all read a 16bit data-pointer parameter into <_di
;   '#','D','U','X','C','S' all use the current data-pointer in <_di
;

tty_printf_huc	.proc				; HuC entry point.

		tma4				; Preserve MPR4.
		pha
		tma3				; Preserve MPR3.
		pha

		jsr	set_bp_to_mpr34		; Map farptr to MPR3 & MPR4.

		stz	tty_xyok		; Make sure VRAM addr is set!

		lda	[_bp]			; Get string length from the
		inc	a			; PASCAL-format string, the
		sta	tty_param		; parameter pointers follow.

		ldy	#1			; First CHR in string.

		;
		; Read and display the next character.
		;

.new_font:	jsr	!tty_increment+		; Set VRAM increment (X,Y ok!).

.next_chr:	lda	[_bp], y		; Read the next chr to print.
		iny

.test_chr:	cmp	#'\e'			; Is it an escape-sequence?
		beq	.got_esc
		bcs	.not_ctl		; Shortcut most text chr.

.got_ctl:	cmp	#'\n'			; Which control code?
		beq	.got_lf
		cmp	#'\f'
		beq	.got_ff
		cmp	#'\r'
		beq	.got_cr
		cmp	#0			; Is this the end, my friend?
		beq	.finished

.not_ctl:	cmp	#'%'			; Is it a format-specifier?
		beq	.format

.got_chr:	jsr	!tty_write+		; Preserves Y register!
		bra	.next_chr

		;
		; Handle ESC terminal commands.
		;

.got_esc:	jmp	.escape

		;
		; Handle LF (includes CR).
		;

.got_lf:	lda	tty_ypos
		inc	a
;		and	#$1F			; Max 32 lines.
		and	#(BAT_SIZE/BAT_LINE)-1	; Max 32 or 64 lines.
		sta	tty_ypos

		;
		; Handle CR.
		;

.got_cr:	lda	tty_xlhs
		sta	tty_xpos
		stz	tty_xyok
		bra	.next_chr

		;
		; Handle FF (i.e. clear screen).
		;

.got_ff:	stz	tty_8x16		; Reset to 8x8 font.
		jsr	!tty_increment+		; Set VRAM increment.

		phy				; Preserve string index.

		lda.l	#CHR_0x20		; Clear the screen.
		sta.l	<_ax
		lda.h	#CHR_0x20
		sta.h	<_ax
		lda.h	#BAT_SIZE
		sta	<_bl
		jsr	clear_bat_vdc

		ply				; Restore string index.

		lda	tty_xlhs		; Home the cursor.
		sta	tty_xpos
		lda	tty_ytop
		sta	tty_ypos
		stz	tty_xyok
		bra	.next_chr

		;
		; Got end-of-string.
		;

.finished:	stz	tty_8x16		; Reset VRAM increment.
		jsr	!tty_increment+

		lda	#VDC_VWR
		sta	<vdc_reg
		st0	#VDC_VWR

		pla				; Restore MPR3.
		tam3
		pla				; Restore MPR4.
		tam4

		leave				; All done!

		;
		; Got a "-" flag!
		;

.flag_minus:	sta	tty_outlhs		; Set NZ for yes.
		bra	.read_flag

		;
		; Got a "+" or " " flag!
		;

.flag_prefix:	sta	tty_out1st		; Set '+' for yes.
		bra	.read_flag

		;
		; Got a "0" flag!
		;

.flag_zero:	sta	tty_outpad		; Set '0' for yes.
		bra	.read_flag

		;
		; Got a "%" format sequence!
		;

.format:	stz	tty_out1st		; Is there a +/- sign?
		stz	tty_outpad		; Pad with zero, not space.
		stz	tty_outlhs		; Left justify, not right.
		stz	tty_outmin		; Clear the minimum output.
		stz	tty_outmax		; Clear the maximum output.

		;
		; Read the format flags.
		;

.read_flag:	lda	[_bp], y		; Read the next flag.
		iny
		cmp	#'%'			; Just print a '%'.
		beq	.got_chr

		cmp	#'9'+1			; Ignore these if above '9'.
		bcs	.read_length
		cmp	#'0'			; Pad with zero?
		beq	.flag_zero
		bcs	.read_minimum		; Branch if '1..9'.
		cmp	#'-'			; Left-justify?
		beq	.flag_minus
		cmp	#'+'			; Prepend a '+' sign?
		beq	.flag_prefix
		cmp	#' '			; Prepend a ' ' sign?
		beq	.flag_prefix

		;
		; Read the output width (minimum #chrs).
		;

		cmp	'#'			; Or read the minimum from
		beq	.data_minimum		; current data pointer.
		cmp	'*'			; Or read the minimum from
	.if	TTY_NO_DOT == 0
		bne	.read_maximum		; a parameter pointer.
	.else
		bne	.read_length
	.endif	TTY_NO_DOT == 0

		phy				; Preserve string index.
		jsr	.read_pointer		; Read the next parameter.
		ply				; Restore string index.

.data_minimum:	jsr	.read_dataval		; Read value from the pointer.
		sta	tty_outmin

		lda	[_bp], y		; Get the next character.
		iny
	.if	TTY_NO_DOT == 0
		bra	.read_maximum
	.else
		bra	.read_length
	.endif	TTY_NO_DOT == 0

.read_minimum:	sbc	#'0'			; Initialize the width.
		sta	<_temp
		jsr	.next_decimal		; Read the rest.
		ldx	<_temp			; Set the minimum output.
		stx	tty_outmin

	.if	TTY_NO_DOT == 0
		;
		; Read the output precision (maximum #chrs).
		;
		; Uses 37 bytes of code.
		;

.read_maximum:	cmp	'.'			; Is this a precision field?
		bne	.read_length

		jsr	.read_decimal		; Read the precision value.
		ldx	<_temp			; Set the maximum output.
		stx	tty_outmax

		cmp	'#'			; Or read the maximum from
		beq	.data_maximum		; current data pointer.
		cmp	'*'			; Or read the precision from
		bne	.read_length		; a parameter pointer.

		phy				; Preserve string index.
		jsr	.read_pointer		; Read the next parameter.
		ply				; Restore string index.

.data_maximum:	jsr	.read_dataval		; Read value from the pointer.
		sta	tty_outmax

		lda	[_bp], y		; Get the next character.
		iny
	.endif	TTY_NO_DOT == 0

		;
		; Read the parameter length.
		;

.read_length:	ldx	#2			; None = 2-byte parameter.

		cmp	#'h'
		beq	.length_half
		cmp	#'l'
		bne	.got_specifier

.length_long:	ldx	#4			; Long = 4-byte parameter.
		bra	.get_specifier

.length_half:	ldx	#1			; Half = 1-byte parameter.
;		bra	.get_specifier

		; Read the output format specifier.

.get_specifier:	lda	[_bp], y		; Read char from string.
		iny

.got_specifier:	phy				; Preserve string index.

		tay				; Remember the specifier.
		ora	#$20			; Make it lowercase.

		;
		; Hex (and BCD) unsigned integer output.
		;

.specifier_x:	cmp	#'x'
		bne	.specifier_s

.hexadecimal:	jsr	.maybe_pointer		; Read the next parameter.

		cly
.read_hexvar:	jsr	.read_dataval		; Read the desired # of bytes
		sta	_ax, y			; into the dividend.
		iny
		dex
		bne	.read_hexvar

		lda	tty_outlhs		; Is this left-justified?
		bne	.hex_head

		lda	tty_outpad		; Get padding '0' or zero.
		bne	.hex_head
		lda	#' '			; Set padding to ' ' if not.
		sta	tty_outpad

.hex_head:	dey				; Are there any more bytes?
	.if	TTY_NO_0HEX != 0
		bmi	.hex_done
	.endif	TTY_NO_0HEX != 0

		lda	_ax, y			; Output lead-in hi-nibble.
		lsr	a
		lsr	a
		lsr	a
		lsr	a
		bne	.hex_tail_hi
		lda	tty_outpad		; Nibble is 0, is there a
		beq	.hex_skip		; padding chr?
		dec	tty_outmin
		jsr	!tty_write+		; Print the padding chr.

.hex_skip:	lda	_ax, y			; Output lead-in lo-nibble.
		and	#$0F
		bne	.hex_tail_lo
	.if	TTY_NO_0HEX == 0
		tya				; Is this the final digit?
		beq	.hex_tail_lo
	.endif	TTY_NO_0HEX == 0
		lda	tty_outpad		; Nibble is 0, is there a
		beq	.hex_head		; padding chr?
		dec	tty_outmin
		jsr	!tty_write+		; Print the padding chr.
		bra	.hex_head

.hex_tail:	dey				; Are there any more bytes?
		bmi	.hex_done

		lda	_ax, y			; Output displayed hi-nibble.
		lsr	a
		lsr	a
		lsr	a
		lsr	a
.hex_tail_hi:	jsr	!tty_nibble+		; Print the nibble in hex.

		lda	_ax, y			; Output displayed lo-nibble.
		and	#$0F
.hex_tail_lo:	jsr	!tty_nibble+		; Print the nibble in hex.
		bra	.hex_tail

.hex_done:	cla				; Have we output the minimum
		sec				; number of characters, yet?
		sbc	tty_outmin
		tay
.check_rhs:	bpl	.output_done		; Negative if pad RHS.

.pad_rhs:	lda	#' '			; Pad the RHS with spaces.
		jsr	!tty_write+
		iny
		bne	.pad_rhs

.output_done:	ply				; Restore string index.
		jmp	.next_chr

		;
		; String
		;

.specifier_s:	cmp	#'s'
		bne	.specifier_c

		jsr	.maybe_pointer		; Read the next parameter.

		ldy	tty_outmax		; Get the maximum output.
.str_loop:	jsr	.read_dataval
		cmp	#0
		beq	.str_done
		jsr	!tty_write+
		dey
		bne	.str_loop

.str_done:	ply				; Restore string index.
		jmp	.next_chr

		;
		; Chr (width is #chrs in array)
		;

.specifier_c:	cmp	#'c'
	.if	TTY_NO_INT == 0
		bne	.specifier_u
	.else
		bne	.bad_format
	.endif	TTY_NO_INT == 0

		jsr	.maybe_pointer		; Read the next parameter.

		ldy	tty_outmin		; Get the output length, with
		bne	.char_loop		; a minimum of 1.
		iny
.char_loop:	jsr	.read_dataval
		jsr	!tty_write+
		dey
		bne	.char_loop

		ply				; Restore string index.
		jmp	.next_chr

		;
		;
		;

.bad_format:	ply				; Restore string index.
		jmp	.finished		; Unknown specifier!

	.if	TTY_NO_INT == 0
		;
		; Signed/Unsigned decimal integer output.
		;

.specifier_u:
	.if	TTY_NO_NEG == 0
		clv				; Clr overflow flag.
		cmp	#'u'			; Clr overflow if unsigned.
		beq	.unsigned
		cmp	#'d'			; Set overflow if signed.
		bne	.bad_format
		
.signed:	adc	#'d'			; Set overflow flag.
	.else
		cmp	#'u'			; Clr overflow if unsigned.
		bne	.bad_format
	.endif	TTY_NO_NEG == 0

.unsigned:	jsr	.maybe_pointer		; Read the next parameter.

		stz	<_ax + 1		; Promote 8-bit to 16-bit.

		cly
.read_variable:	jsr	.read_dataval		; Read the desired # of bytes
		sta	_ax, y			; into the dividend.
		iny
		dex
		bne	.read_variable

	.if	TTY_NO_NEG == 0
		bvc	.positive		; Treat variable as unsigned?
		ora	#0			; Is the dataval hi-byte -ve?
		bpl	.positive

		sec				; Negate the dataval so that
.negate:	lda	<_ax, x			; it is positive.
		eor	#$FF
		adc	#0
		sta	<_ax, x
		inx
		dey
		bne	.negate		
		sxy				; X=0, Y=#bytes.

		lda	#'-'			; Prepend a '-' sign!
		sta	tty_out1st
	.endif	TTY_NO_NEG == 0

.positive:	phx				; Push the output terminator.
		tsx				; Remember the end address of
		stx	tty_outstk		; numbers pushed on the stack.

		lda	#10			; Set divisor.
		sta	<_cl

	.if	TTY_NO_INT32 == 0
		cpy	#4			; Is this a long dataval?
		bne	.word_decimal

.long_decimal:	call	div32_7u		; Divide by 10 and push the
		ora	#'0'			; remainder onto the stack.
		pha
		lda	<_ax + 0		; Repeat while non-zero.
		ora	<_ax + 1
		ora	<_ax + 2
		ora	<_ax + 3
		bne	.long_decimal
		bra	.justify
	.endif	TTY_NO_INT32 == 0

.word_decimal:	call	div16_7u		; Divide by 10 and push the
		ora	#'0'			; remainder onto the stack.
		pha
		lda	<_ax + 0		; Repeat while non-zero.
		ora	<_ax + 1
		bne	.word_decimal

.justify:	tsx				; Calc how many chrs are on
		txa				; the stack.
		eor	#$FF
		inc	a
		clc
		adc	tty_outstk		; Always leaves C set.
		sbc	tty_outmin		; Is there a minimum length?
		bcs	.set_rhs

		ldx	tty_outlhs		; Is this left-justified?
		bne	.set_rhs

		ldx	tty_outpad		; Get padding '0' or zero.
		bne	.pad_lhs
		ldx	#' '			; Get padding '0' or ' '.
.pad_lhs:	phx
		inc	a
		bne	.pad_lhs

.set_rhs:	sta	tty_outmin		; Negative if pad RHS.

		lda	tty_out1st		; Is there a prepend to do?
		beq	.pop_digit
		pha				; Push the sign character.

.pop_digit:	pla				; Pop the digits and output.
		beq	.tst_rhs
		jsr	!tty_write+
		bra	.pop_digit

.tst_rhs:	ldy	tty_outmin		; Negative if pad RHS.
		jmp	.check_rhs
	.endif	TTY_NO_INT == 0

		; ***********************************************************
		;
		; TERMINAL EMULATION
		;

.escape:	lda	[_bp], y		; Read the escape sequence.
		iny

		;
		; Set Cursor X coordinate.
		;

.escape_X:	cmp	#'X'
		bne	.escape_Y

		lda	[_bp], y		; Check the next character.
		cmp	#'L'
		beq	.set_xlhs

.set_xpos:	jsr	.read_decimal		; Read decimal, return chr.
		ldx	<_temp
		stx	tty_xpos
		stz	tty_xyok
		jmp	.test_chr		; Process the next chr.

.set_xlhs:	iny				; Swallow the 'L'.
		jsr	.read_decimal		; Read decimal, return chr.
		ldx	<_temp
		stx	tty_xlhs
		jmp	.test_chr		; Process the next chr.

		;
		; Set Cursor Y coordinate.
		;

.escape_Y:	cmp	#'Y'
		bne	.escape_P

		lda	[_bp], y		; Check the next character.
		cmp	#'T'
		beq	.set_ytop

.set_ypos:	jsr	.read_decimal		; Read decimal, return chr.
		ldx	<_temp
		stx	tty_ypos
		stz	tty_xyok
		jmp	.test_chr		; Process the next chr.

.set_ytop:	iny				; Swallow the 'T'.
		jsr	.read_decimal		; Read decimal, return chr.
		ldx	<_temp
		stx	tty_ytop
		jmp	.test_chr		; Process the next chr.

		;
		; Change Palette
		;

.escape_P:	cmp	#'P'
		bne	.escape_T

		lda	#$F0			; Read a single hex digit.
		trb.h	tty_tile
		jsr	.read_hex
		asl	a
		asl	a
		asl	a
		asl	a
		tsb.h	tty_tile
		jmp	.next_chr

		;
		; Change VRAM Base Tile + Palette
		;

.escape_T:	cmp	#'T'
	.if	TTY_NO_BOX == 0
		bne	.escape_B
	.else
		bne	.escape_lo
	.endif	TTY_NO_BOX == 0

		lda	[_bp], y		; Read byte from string.
		iny
		sta	tty_tile + 0
		lda	[_bp], y		; Read byte from string.
		iny
		sta	tty_tile + 1
		jmp	.next_chr

	.if	TTY_NO_BOX == 0
		;
		; Draw a box.
		;

.escape_B:	cmp	#'B'
		bne	.escape_lo

		jsr	.read_decimal		; Box width.
		cmp	#','
		bne	.box_done		; Abort if parameter missing.
		lda	<_temp
		sta	<_al
		jsr	.read_decimal		; Box height.
		cmp	#','
		bne	.box_done		; Abort if parameter missing.
		lda	<_temp
		sta	<_ah
		jsr	.read_decimal		; Box type.

		phy				; Preserve string index.
		pha				; Preserve next string char.

		lda	<_temp
		sta	<_bl
		call	tty_draw_box

		pla				; Restore next string char.
		ply				; Restore string index.

.box_done:	jmp	.test_chr		; Process the next chr.
	.endif	TTY_NO_BOX == 0

		;
		; Set small (8-high) text.
		;

.escape_lo:	cmp	#'<'
	.ifdef	CHR_TALL
		bne	.escape_hi
	.else
		bne	.bad_terminal
	.endif
		stz	tty_8x16		; Set to zero for 8x8.

		lda.l	#CHR_ZERO
		sta.l	tty_tile		; Font base = VRAM $0800.
		lda.h	tty_tile
		and	#$F0
		ora.h	#CHR_ZERO
		sta.h	tty_tile
		jmp	.new_font		; Change VRAM increment.

	.ifdef	CHR_TALL			; Is there an 8x16 font?
		;
		; Set large (16-high) text.
		;

.escape_hi:	cmp	#'>'
		bne	.bad_terminal
		sta	tty_8x16		; Set to non-zero for 8x16.

		lda.l	#CHR_TALL
		sta.l	tty_tile		; Font base = VRAM $1000.
		lda.h	tty_tile
		and	#$F0
		ora.h	#CHR_TALL
		sta.h	tty_tile
		jmp	.new_font		; Change VRAM increment.
	.endif

		;
		; Illegal Value!!!
		;

.bad_terminal:	jmp	.finished

		; ***********************************************************
		;
		; STRING INPUT
		;

		;
		; Read single hex digit.
		;

.read_hex:	lda	[_bp], y		; Read char from string.
		iny
		cmp	#'9'+1			; Is this a decimal digit?
		bcs	.hex_gt_9
		sbc	#'0'-1
		bcc	.hex_bad
		rts

.hex_gt_9:	and	#$9F			; Is it 'A'..'F' or 'a'..'f'?
		bmi	.hex_bad
		beq	.hex_bad
		cmp	#7
		bcs	.hex_bad
		adc	#9
		rts

.hex_bad:	bra	.hex_bad		; Hang on error to allow debugging.

		;
		; Read decimal number (returns next non-decimal chr read).
		;

.read_decimal:	stz	<_temp			; Initialize to zero.

.next_decimal:	lda	[_bp], y		; Read char from string.
		iny
		cmp	#'9'+1			; Is this a decimal digit?
		bcs	.param_exit
		cmp	#'0'
		bcc	.param_exit

		sbc	#'0'			; Accumulate the decimal value
		asl	<_temp			; within the range (0..255).
		adc	<_temp
		asl	<_temp
		asl	<_temp
		adc	<_temp
		sta	<_temp
		bra	.next_decimal

		;
		; Read a 16-bit pointer to data.
		;
		; N.B. Destroys Y!
		;

.maybe_pointer:	cpy	#'a'			; Skip if specifier is in
		bcc	!+			; uppercase!

.read_pointer:	ldy	tty_param		; Restore params index.
		lda	[_bp], y		; Read pointer lo-byte.
		iny
		sta.l	<_di
		lda	[_bp], y		; Read pointer hi-byte.
		iny
		sta.h	<_di
		sty	tty_param		; Preserve params index.
!:		rts

		;
		; Read next byte from data memory.
		;
		; N.B. X and Y preserved!
		;

.read_dataval:	lda	[_di]
		inc.l	<_di
		bne	.param_exit
		inc.h	<_di
.param_exit:	rts

		; ***********************************************************
		;
		; TERMINAL OUTPUT
		;

		;
		; Write an ASCII/Japanese character to the BAT.
		;
		; N.B. Y preserved!
		;

!tty_nibble:	ora	#'0'			; Print a hex/bcd nibble.
		cmp	#'9' + 1
		bcc	!+
		adc	#6
!:		dec	tty_outmin

!tty_write:	ldx	tty_xyok		; Is the VDC_MAWR correct?
		bne	.which_font

.write_pos:	pha
		inc	tty_xyok		; Signal correct VDC_MAWR.

		lda	tty_ypos		; Calc BAT address for Ypos
		lsr	a			; assuming a width of 128.
		tax
		cla
		ror	a

	.if	BAT_LINE != 128
		sax				; Do more if a width of 64.
		lsr	a
		sax
		ror	a
	.if	BAT_LINE != 64
		sax				; Do more if a width of 32.
		lsr	a
		sax
		ror	a
	.endif	BAT_LINE != 64
	.endif	BAT_LINE != 128

		php
		sei
		ora	tty_xpos
		st0	#VDC_MARR		; Set VDC_MARR, not sure why.
		sta	VDC_DL
		stx	VDC_DH
		st0	#VDC_MAWR		; Set VDC_MAWR.
		sta	VDC_DL
		stx	VDC_DH
		lda	#VDC_VWR
		sta	<vdc_reg
		st0	#VDC_VWR
		plp
		pla

.which_font:	ldx	tty_8x16		; Are we writing 8x16?
		clx				; Hi-byte of tile#.
		bne	.write_8x16

.write_8x8:	clc				; Write top of 8x8 character.
		adc.l	tty_tile
		sta	VDC_DL
		sax
		adc.h	tty_tile
		sta	VDC_DH
		inc	tty_xpos
		rts

.write_8x16:	asl	a			; Is this a Japanese glyph?
		bcc	.write_top
		inx				; Add 256 to tty_tile.
.write_top:	bsr	.write_8x8		; Write top of 8x16 character.
.write_btm:	inx				; Write btm of 8x16 character.
		stx	VDC_DL
		sta	VDC_DH
		stz	tty_xyok		; Need to fix VDC_MAWR!
		rts

		;
		; Set the VRAM increment depending upon the font height.
		;
		; N.B. X and Y preserved!
		;

!tty_increment:	lda	#VDC_CR
		sta	<vdc_reg
		st0	#VDC_CR
		lda	tty_8x16		; Display 8-high or 16-high?
		beq	.set_delta		;  0 == increment by 1.

	.if	BAT_LINE == 128
		lda	#%00011000		; NZ == increment by 128.
	.else
	.if	BAT_LINE == 64
		lda	#%00010000		; NZ == increment by 64.
	.else
	.if	BAT_LINE == 32
		lda	#%00001000		; NZ == increment by 32.
	.else
		.fail	"What value does BAT_LINE have???"
	.endif	BAT_LINE == 32
	.endif	BAT_LINE == 64
	.endif	BAT_LINE == 128

.set_delta:	sta	VDC_DH
		rts

		.endp



	.if	TTY_NO_BOX == 0

; ***************************************************************************
; ***************************************************************************
;
; tty_draw_box - draw a box on the screen
;
; Args: _al = Box width  (minimum 2).
; Args: _ah = Box height (minimum 2).
; Args: _bl = Box type.
;
; Preserves: _si
;

tty_draw_box	.proc

		lda	<_bl			; Xvert box type to index.
		asl	a
		asl	a
		asl	a
		adc	<_bl
		tay				; Index into .box_edge_tbl.

		stz	tty_xyok

.box_top_line:	bsr	.box_line_lhs		; Draw top line of box.

		ldx	<_ah			; Box height.
		dex
		dex
		dex
		bmi	.box_btm_line		; Skip middle if height < 3.

.box_mid_line:	phx
		bsr	.box_line_lhs		; Draw mid line(s) of box.
		plx
		dex
		bmi	.box_btm_line
		dey				; Rewind .box_edge_tbl index
		dey				; for a repeat of mid line.
		dey
		bra	.box_mid_line

.box_btm_line:	bsr	.box_line_lhs		; Draw btm line of box.

		leave				; All done!

		;
		;
		;

.box_line_lhs:	lda	tty_xpos
		pha
		stz	tty_xyok

		lda	.box_edge_tbl, y
		iny
		jsr	!tty_write-

		lda	.box_edge_tbl, y
		iny
		phy

		ldy	<_al			; Box width.
		dey
		dey
		dey
		bmi	.box_line_rhs

		tax				; Center character zero?
		beq	.box_no_center

		jsr	!tty_write-
.box_line_mid:	dey
		bmi	.box_line_rhs
		stx	VDC_DL
		sta	VDC_DH
		bra	.box_line_mid

.box_no_center:	tya				; Skip the middle of the line.
		sec
		adc	tty_xpos
		sta	tty_xpos
		stz	tty_xyok

.box_line_rhs:	ply
		lda	.box_edge_tbl, y
		iny
		jsr	!tty_write-

		pla
		sta	tty_xpos
		inc	tty_ypos
		stz	tty_xyok
		rts

.box_edge_tbl:	db	$20,$20,$20		; 0 : Blank Box.
		db	$20,$20,$20
		db	$20,$20,$20

		db	$10,$11,$12		; 1 : Thick Edge.
		db	$13,$20,$14
		db	$15,$16,$17

		db	$10,$11,$12		; 2 : Thick Edge, no center.
		db	$13,$00,$14
		db	$15,$16,$17

		leave				; All done!

		.endp
	.endif	TTY_NO_BOX == 0

		.endprocgroup			; Group main TTY in 1 bank!



; ***************************************************************************
; ***************************************************************************
;
; tty_dump_page - show a 128-byte hex dump (16 lines of 8 bytes)
;
; Args: X = lo-byte of address
; Args: Y = hi-byte of address
;
; Preserves: _si
;
; N.B. This is mainly a debugging function!
;

tty_dump_page	.proc

		lda	#16
.loop:		pha
		call	tty_dump_line
		clc
		txa
		adc	#$08
		bcc	.skip
		iny
.skip:		tax
		pla
		dec	a
		bne	.loop

		leave				; All done!

		.endp



; ***************************************************************************
; ***************************************************************************
;
; tty_dump_line - show an 8-byte hex (and ASCII) dump
;
; Args: X = lo-byte of address
; Args: Y = hi-byte of address
;
; Preserves: _si
;
; N.B. This is mainly a debugging function!
;

tty_dump_line	.proc

		php
		sei

		phx
		phy
		stx.l	<_temp
		sty.h	<_temp

		cly

.line_loop:	st0	#VDC_MAWR

		lda	tty_ypos		; Calc BAT address for Ypos
		lsr	a			; assuming a width of 128.
		tax
		cla
		ror	a
	.if	BAT_LINE != 128
		sax				; Do more if a width of 64.
		lsr	a
		sax
		ror	a
	.if	BAT_LINE != 64
		sax				; Do more if a width of 32.
		lsr	a
		sax
		ror	a
	.endif	BAT_LINE != 64
	.endif	BAT_LINE != 128

		sta	VDC_DL
		stx	VDC_DH

		st0	#VDC_VWR
		st1.l	#CHR_0x20
		st2.h	#CHR_0x20

		phy

		ldx	#1
.addr_loop:	lda	<_temp, x
		lsr	a
		lsr	a
		lsr	a
		lsr	a
		ora.l	#CHR_ZERO + '0'
		cmp.l	#CHR_ZERO + '0' + 10
		bcc	.addr_lo
		adc	#6
.addr_lo:	sta	VDC_DL
		st2.h	#CHR_ZERO

		lda	<_temp, x
		and	#$0F
		ora.l	#CHR_ZERO + '0'
		cmp.l	#CHR_ZERO + '0' + 10
		bcc	.addr_hi
		adc	#6
.addr_hi:	sta	VDC_DL
		st2.h	#CHR_ZERO
		dex
		bpl	.addr_loop

		st1.l	#CHR_ZERO + ':'
		st2.h	#CHR_ZERO

		st1.l	#CHR_ZERO + ' '
		st2.h	#CHR_ZERO

.byte_loop:	lda	[_temp], y
		lsr	a
		lsr	a
		lsr	a
		lsr	a
		ora.l	#CHR_ZERO + '0'
		cmp.l	#CHR_ZERO + '0' + 10
		bcc	.skip_lo
		adc	#6
.skip_lo:	sta	VDC_DL
		st2.h	#CHR_ZERO

		lda	[_temp], y
		and	#$0F
		ora.l	#CHR_ZERO + '0'
		cmp.l	#CHR_ZERO + '0' + 10
		bcc	.skip_hi
		adc	#6
.skip_hi:	sta	VDC_DL
		st2.h	#CHR_ZERO

		iny
		tya
		bit	#$00		; $01 for 16-bits-per-block
		bne	.skip_space

		st1.l	#CHR_0x20
		st2.h	#CHR_0x20

.skip_space:	bit	#$07		; $0F for 16-bytes-per-line
		bne	.byte_loop

		ply

.char_loop:	lda	[_temp], y
		cmp	#$20
		bcc	.non_ascii
		cmp	#$7F
		bcc	.show_char
.non_ascii:	lda	#'.'
.show_char:	clc
		adc.l	#CHR_ZERO
		sta	VDC_DL
		st2.h	#CHR_ZERO

		iny
		tya
		bit	#$07		; $0F for 16-bytes-per-line
		bne	.char_loop

		inc	tty_ypos
		cpy	#$08		; $10 for 16-bytes-per-line
		beq	.finished

		jmp	.line_loop

.finished:	lda	<vdc_reg
		sta	VDC_AR

		ply
		plx

		plp

		leave				; All done!

		.endp



; ***************************************************************************
; ***************************************************************************
;
; tty_dump_mpr - show an 8-byte hex dump of the MPR registers.
;
; Preserves: _si
;
; N.B. This is mainly a debugging function!
;

tty_dump_mpr	.proc

		tsx				; Remember initial stack.

		tma7				; Push the MPR registers.
		pha
		tma6
		pha
		tma5
		pha
		tma4
		pha
		tma3
		pha
		tma2
		pha
		tma1
		pha
		tma0
		pha

		phx				; Preserve initial stack.

		tsx				; Display MPR registers
		inx				; on the stack.
		inx
		ldy.h	#$2100
		call	tty_dump_line

		plx				; Restore initial stack.
		txs

		leave				; All done!

		.endp
