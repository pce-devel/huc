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
; _si		= BAT memory location
; _si_bank	= BAT bank
; _di		= VRAM base address
; _ax		= nb of words to copy
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

		jsr	set_si_to_mpr34
		jsr	vdc_di_to_mawr

		tii	.vdc_tai, ram_tia, 8

		ldx.l	<_si
		stx.l	ram_tia_src
		ldy.h	<_si
		sty.h	ram_tia_src

		lda	<_al			; length in chunks
		lsr	<_ah
		ror	a
		lsr	<_ah
		ror	a
		lsr	<_ah
		ror	a
		lsr	<_ah
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

.next_4kw:	dec	<_ah
		bpl	.chunk_loop

		lda	<_al
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
; _si		= BAT memory location
; _si_bank	= BAT bank
; _di		= VRAM base address
; _al		= nb of column to copy
; _ah		= nb of row
; ----
; N.B. BAT data *must* be word-aligned!

_load_bat	.proc

		tma3
		pha

		jsr	set_si_to_mpr3

		ldy.l	<_si
		stz.l	<_si

.line_loop:	jsr	vdc_di_to_mawr

		ldx	<_al
.tile_loop:	lda	[_si], y
		sta	VDC_DL
		iny
		lda	[_si], y
		sta	VDC_DH
		iny
		bne	!+
		jsr	inc.h_si_mpr3
!:		dex
		bne	.tile_loop

		lda	#64
		clc
		adc.l	<_di
		sta.l	<_di
		bcc	!+
		inc.h	<_di
!:		dec	<_ah
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
		stz.l	<_di
		sta.h	<_di
		jsr	vdc_di_to_mawr

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
; _al=num
; ----
; load SI with the offset of the sprite to change
; SI = satb + 8 * sprite_number

_spr_set:	lda	<_al
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
; _ax=value

_spr_x:		ldy	#2
		clc
		lda	<_al
		adc	#32
		sta	[spr_ptr], y
		lda	<_ah
		adc	#0
		iny
		sta	[spr_ptr], y
		rts

; ----
; spr_y(int value)
; ----
; _ax=value

_spr_y:		clc
		lda	<_al
		adc	#64
		sta	[spr_ptr]
		lda	<_ah
		adc	#0
;		and	#$01
		ldy	#1
		sta	[spr_ptr], y
		rts

; ----
; spr_pattern(int vaddr >> 5)
; ----
; _ax=value

_spr_pattern:	lda	<_al		;     zp=fedcba98 a=76543210
		asl	a		; c=f zp=edcba987 a=6543210x
		rol	<_ah
		rol	a		; c=e zp=dcba9876 a=543210xf
		rol	<_ah
		rol	a		; c=d zp=cba98765 a=43210xfe
		rol	<_ah
		rol	a		; c=4 zp=cba98765 a=3210xfed
;		and	#$7
		ldy	#5
		sta	[spr_ptr], y
		lda	<_ah
		dey
		sta	[spr_ptr], y
		rts

; ----
; spr_ctrl(char mask, char value)
; ----
; _al=!mask _ah=mask&value

_spr_ctrl:	ldy	#7
		lda	<_al
		and	[spr_ptr], y
		ora	<_ah
		sta	[spr_ptr], y
		rts

; ----
; spr_pal(char pal)
; ----
; _al=pal

_spr_pal:	lda	<_al
		and	#$0F
		sta	<_temp
		ldy	#6
		lda	[spr_ptr], y
		and	#$F0
		ora	<_temp
		sta	[spr_ptr], y
		rts

; ----
; spr_pri(char pri)
; ----
; _al=pri

_spr_pri:	ldy	#6
		cla
		cmp	<_al
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
; _al		= x coordinate
; _ah		= y coordinate
; _si		= string address
; _si_bank	= string bank
; ----
; _di		= corrupted
; ----

_put_string	.proc

		jsr	_put_xy

.chr_loop:	lda	[_si]
		beq	.done

		clc
		adc	#$80
		sta	VDC_DL
		lda	#$10
;		lda	#$00
		sta	VDC_DH
		inc.l	<_si
		bne	.chr_loop
		inc.h	<_si
		bra	.chr_loop

.done:		leave

		.endp


; ----
; put_number(int number, char n, char x, char y)
; ----
; _al		= x coordinate
; _ah		= y coordinate
; _bx          = number 
; _cl          = minimum characters
; ----
; _di		= corrupted
; ----

_put_number	.proc

		jsr	_put_xy

		ldy	<_cl			; Minimum #characters.

		cla				; Push EOL marker.
		pha

.divide_by_ten:	ldx	#16
		cla				; Clear Remainder.
		asl.l	<_bx			; Rotate Dividend, MSB -> C.
		rol.h	<_bx
.divide_loop:	rol	a			; Rotate C into Remainder.
		cmp	#10			; Test Divisor.
		bcc	.divide_less		; CC if Divisor > Remainder.
		sbc	#10			; Subtract Divisor.
.divide_less:	rol.l	<_bx			; Quotient bit -> Dividend LSB.
		rol.h	<_bx			; Rotate Dividend, MSB -> C.
		dex
		bne	.divide_loop

		clc
		adc	#'0'			; Always leaves C clr.
		pha
		dey
		lda.l	<_bx			; Repeat while non-zero.
		ora.h	<_bx
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
; _al		= x coordinate
; _ah		= y coordinate
; ----
; _di		= VRAM address
; ----

_put_xy:	cla
		lsr	<_ah
		ror	a
		lsr	<_ah
		ror	a
		ora	<_al
		sta.l	<_di
		lda	<_ah
		sta.h	<_di
		jmp	vdc_di_to_mawr



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
; _al		= random value 0..209
; ----

_random210:	jsr	get_random

		tay
		sec
		lda	square_pos_lo + (210), y
		sbc	square_neg_lo + (210 ^ $FF), y
;		sta	<_al
		lda	square_pos_hi + (210), y
		sbc	square_neg_hi + (210 ^ $FF), y
		sta	<_al
		rts
