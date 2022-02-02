; ***************************************************************************
; ***************************************************************************
;
; huc-gfx.asm
;
; Compatibility stuff for HuC's library functions.
;
; These should be located in permanently-accessible memory!
;
; Copyright John Brandwood 2021.
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
		include "vce.asm"		; Useful VCE routines.
		include "vdc.asm"		; Useful VCE routines.



; ***************************************************************************
; ***************************************************************************
;
; HuC VRAM Functions
;
; ***************************************************************************
; ***************************************************************************

; ----
; load_vram
; ----
; copy a block of memory to VRAM
; ----
; __si		= BAT memory location
; __si_bank	= BAT bank
; __di		= VRAM base address
; __ax		= nb of words to copy
; ----
; N.B. BAT data *must* be word-aligned!

;	.if	!CDROM				; Only applies to a HuCard!
		.bss
ram_tia:	.ds	1
ram_tia_src:	.ds	2
ram_tia_dst:	.ds	2
ram_tia_len:	.ds	2
ram_tia_rts:	.ds	1
		.code
;	.endif

_load_vram	.proc

		tma3
		pha
		tma4
		pha

		jsr	__si_to_mpr34
		jsr	__di_to_vdc

		tii	.vdc_tai, ram_tia, 8

		ldx.l	<__si
		stx.l	ram_tia_src
		ldy.h	<__si
		sty.h	ram_tia_src

		lda	<__al			; length in chunks
		lsr	<__ah
		ror	a
		lsr	<__ah
		ror	a
		lsr	<__ah
		ror	a
		lsr	<__ah
		ror	a

		sax				; x=chunks-lo
		beq	.next_4kw		; a=source-lo, y=source-hi

.chunk_loop:	jsr	ram_tia			; transfer 32-bytes

		clc				; increment source
		adc	#$20
		sta.l	ram_tia_src
		bcc	.same_page
		iny
		bpl	.same_bank		; remap_data

		say
		tma4
		tam3
		inc	a
		tam4
		lda	#$60
		say

.same_bank:	sty.h	ram_tia_src

.same_page:	dex
		bne	.chunk_loop

.next_4kw:	dec	<__ah
		bpl	.chunk_loop

		lda	<__al
		and	#15
		beq	.done

		asl	a
		sta.l	ram_tia_len

		jsr	ram_tia			; transfer remainder

.done:		pla
		tam4
		pla
		tam3

		leave

.vdc_tai:	tia	$1234, VDC_DL, 32
		rts

		.endp



; ----
; load_bat
; ----
; transfer a BAT to VRAM
; ----
; __si		= BAT memory location
; __si_bank	= BAT bank
; __di		= VRAM base address
; __al		= nb of column to copy
; __ah		= nb of row
; ----
; N.B. BAT data *must* be word-aligned!

_load_bat	.proc

		tma3
		pha

		jsr	__si_to_mpr3

		ldy.l	<__si
		stz.l	<__si

.line_loop:	jsr	__di_to_vdc

		ldx	<__al
.tile_loop:	lda	[__si], y
		sta	VDC_DL
		iny
		lda	[__si], y
		sta	VDC_DH
		iny
		bne	!+
		jsr	__si_inc_page
!:		dex
		bne	.tile_loop

		lda	#64
		clc
		adc.l	<__di
		sta.l	<__di
		bcc	!+
		inc.h	<__di
!:		dec	<__ah
		bne	.line_loop

		pla
		tam3

		leave

		.endp



; ***************************************************************************
; ***************************************************************************
;
; HuC Sprite Functions
;
; ***************************************************************************
; ***************************************************************************

		.zp
spr_ptr:	ds	2
		.bss
spr_max:	ds	1
spr_clr:	ds	1
spr_sat:	ds	512
		.code



; ----
; init_satb()
; ----

_init_satb:	clx
		cla
!:		stz	spr_sat, x
		stz	spr_sat + 256, x
		inx
		bne	!-
		stz	spr_max
		inc	spr_clr
		rts

; ----
; satb_update()
; ----

_satb_update	.proc

;		lda.h	#$7F00			; HuC puts the SAT here in VRAM
		lda.h	#$0800			; but we put it here instead
		stz.l	<__di
		sta.h	<__di
		jsr	__di_to_vdc

		ldx	spr_max

		lda	spr_clr
		beq	!+
		stz	spr_clr
		ldx	#64

!:		txa
		beq	.exit

		dec	a			; round up to the next group of 4 sprites
		lsr	a
		lsr	a
		inc	a
		tax

		tii	.vdc_tai, ram_tia, 8

		lda	#<spr_sat

.chunk_loop:	jsr	ram_tia			; transfer 32-bytes

		clc				; increment source
		adc	#$20
		sta.l	ram_tia_src
		bcc	.same_page
		inc.h	ram_tia_src

.same_page:	dex
		bne	.chunk_loop

.exit:		leave

.vdc_tai:	tia	spr_sat, VDC_DL, 32
		rts

		.endp



; ----
; spr_set(char num)
; ----
; __al=num
; ----
; load SI with the offset of the sprite to change
; SI = satb + 8 * sprite_number

_spr_set:	lda	<__al
		cmp	#64
		bcs	.exit

		cmp	spr_max
		bcc	!+
		sta	spr_max

!:		asl	a
		asl	a
		asl	a
		stz	<spr_ptr + 1
		rol	<spr_ptr + 1
		adc	#<spr_sat
		sta	<spr_ptr
		lda	<spr_ptr + 1
		adc	#>spr_sat
		sta	<spr_ptr + 1

.exit:		rts



; ----
; spr_hide()
; ----

_spr_hide:	ldy	#1
		lda	[spr_ptr], y
		ora	#$02
		sta	[spr_ptr], y
		rts



; ----
; spr_show()
; ----

_spr_show:	ldy	#1
		lda	[spr_ptr], y
		and	#$01
		sta	[spr_ptr], y
		rts

; ----
; spr_x(int value)
; ----
; __ax=value

_spr_x:		ldy	#2
		clc
		lda	<__al
		adc	#32
		sta	[spr_ptr], y
		lda	<__ah
		adc	#0
		iny
		sta	[spr_ptr], y
		rts

; ----
; spr_y(int value)
; ----
; __ax=value

_spr_y:		clc
		lda	<__al
		adc	#64
		sta	[spr_ptr]
		lda	<__ah
		adc	#0
;		and	#$01
		ldy	#1
		sta	[spr_ptr], y
		rts

; ----
; spr_pattern(int vaddr >> 5)
; ----
; __ax=value

_spr_pattern:	lda	<__al		;     zp=fedcba98 a=76543210
		asl	a		; c=f zp=edcba987 a=6543210x
		rol	<__ah
		rol	a		; c=e zp=dcba9876 a=543210xf
		rol	<__ah
		rol	a		; c=d zp=cba98765 a=43210xfe
		rol	<__ah
		rol	a		; c=4 zp=cba98765 a=3210xfed
;		and	#$7
		ldy	#5
		sta	[spr_ptr], y
		lda	<__ah
		dey
		sta	[spr_ptr], y
		rts

; ----
; spr_ctrl(char mask, char value)
; ----
; __al=!mask __ah=mask&value

_spr_ctrl:	ldy	#7
		lda	<__al
		and	[spr_ptr], y
		ora	<__ah
		sta	[spr_ptr], y
		rts

; ----
; spr_pal(char pal)
; ----
; __al=pal

_spr_pal:	lda	<__al
		and	#$0F
		sta	<__temp
		ldy	#6
		lda	[spr_ptr], y
		and	#$F0
		ora	<__temp
		sta	[spr_ptr], y
		rts

; ----
; spr_pri(char pri)
; ----
; __al=pri

_spr_pri:	ldy	#6
		cla
		cmp	<__al
		lda	[spr_ptr], y
		and	#$7F
		bcs	!+
		ora	#$80
!:		sta	[spr_ptr], y
		rts




; ***************************************************************************
; ***************************************************************************
;
; HuC Text Output
;
; ***************************************************************************
; ***************************************************************************

; ----
; put_string(char *string, char x, char y)
; ----
; __al		= x coordinate
; __ah		= y coordinate
; __si		= string address
; __si_bank	= string bank
; ----
; __di		= corrupted
; ----

_put_string	.proc

		jsr	_put_xy

.chr_loop:	lda	[__si]
		beq	.done

		clc
		adc	#$80
		sta	VDC_DL
		lda	#$10
;		lda	#$00
		sta	VDC_DH
		inc.l	<__si
		bne	.chr_loop
		inc.h	<__si
		bra	.chr_loop

.done:		leave

		.endp


; ----
; put_number(int number, char n, char x, char y)
; ----
; __al		= x coordinate
; __ah		= y coordinate
; __bx          = number 
; __cl          = minimum characters
; ----
; __di		= corrupted
; ----

_put_number	.proc

		jsr	_put_xy

		ldy	<__cl			; Minimum #characters.

		cla				; Push EOL marker.
		pha

.divide_by_ten:	ldx	#16
		cla				; Clear Remainder.
		asl.l	<__bx			; Rotate Dividend, MSB -> C.
		rol.h	<__bx
.divide_loop:	rol	a			; Rotate C into Remainder.
		cmp	#10			; Test Divisor.
		bcc	.divide_less		; CC if Divisor > Remainder.
		sbc	#10			; Subtract Divisor.
.divide_less:	rol.l	<__bx			; Quotient bit -> Dividend LSB.
		rol.h	<__bx			; Rotate Dividend, MSB -> C.
		dex
		bne	.divide_loop

		clc
		adc	#'0'			; Always leaves C clr.
		pha
		dey
		lda.l	<__bx			; Repeat while non-zero.
		ora.h	<__bx
		bne	.divide_by_ten

		lda	#'0'
.padding:	dey
		bmi	.output
		pha
		bra	.padding

.write:		clc
		adc	#$80
		sta	VDC_DL
		lda	#$10			; Use palette 1.
;		lda	#$00
		sta	VDC_DH

.output:	pla				; Pop the digits and output.
		bne	.write

		leave				; All done!

		.endp


; ----
; put_xy(char x, char y)
; ----
; __al		= x coordinate
; __ah		= y coordinate
; ----
; __di		= VRAM address
; ----

_put_xy:	cla
		lsr	<__ah
		ror	a
		lsr	<__ah
		ror	a
		ora	<__al
		sta.l	<__di
		lda	<__ah
		sta.h	<__di
		jmp	__di_to_vdc



; ***************************************************************************
; ***************************************************************************
;
; Specifically for shmup.c ...
;
; ***************************************************************************
; ***************************************************************************

; ----
; _random210()
; ----
; __al		= random value 0..209
; ----

_random210:	jsr	get_random

		tay
		sec
		lda	square_pos_lo + (210), y
		sbc	square_neg_lo + (210 ^ $FF), y
;		sta	<__al
		lda	square_pos_hi + (210), y
		sbc	square_neg_hi + (210 ^ $FF), y
		sta	<__al
		rts
