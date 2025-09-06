; ***************************************************************************
; ***************************************************************************
;
; vce.asm
;
; Useful routines for operating the HuC6260 Video Color Encoder
;
; These should be located in permanently-accessible memory!
;
; Copyright John Brandwood 2021-2025.
;
; Distributed under the Boost Software License, Version 1.0.
; (See accompanying file LICENSE_1_0.txt or copy at
;  http://www.boost.org/LICENSE_1_0.txt)
;
; ***************************************************************************
; ***************************************************************************

;
; Configure Library ...
;

	.ifndef VCE_SPLIT_CROSS
VCE_SPLIT_CROSS	=	1
	.endif



; ***************************************************************************
; ***************************************************************************
;
; xfer_palettes - Update the VCE with the queued palettes changes.
;
; This is normally called in a developer's vsync_hook handler, but it can
; be called manually as long as it will not also be called in an interrupt.
;
; The queued transfers are split into 32-byte chunks so that an HSYNC and/or
; TIMER IRQ is not delayed for too long during the VBLANK.
;

xfer_palettes:	lda	#$80			; Acquire color mutex to avoid
		tsb	color_mutex		; conflict with a delayed VBL.
		bmi	.busy

		ldy	color_queue_r		; Are there any palette xfers
		cpy	color_queue_w		; queued up?
		beq	.exit

		php				; Enable interrupts so that an
		cli				; HSYNC or TIMER IRQ can occur.

		tma3				; Preserve MPR3 & MPR4 because
		pha				; this normally runs in the
		tma4				; VBLANK IRQ.
		pha

	.if	!CDROM
		tii	.tia_func, color_tia, 8 ; Copy TIA to RAM.
	.endif

.next_item:	lda	color_index, y		; Get the next set of palettes
		asl	a			; from the queue.
		asl	a
		asl	a
		asl	a
		sta	VCE_CTA + 0
		cla
		rol	a
		sta	VCE_CTA + 1

		ldx	color_count,y		; How many palettes to xfer?

		lda	color_bank, y		; Map data into MPR3 & MPR4.
		tam3
		inc	a
		tam4
		lda	color_addr_h, y
		sta	.ram_tia + 2
		lda	color_addr_l, y
.palette_loop:	sta	.ram_tia + 1

	.if	CDROM
.ram_tia:	tia	0, VCE_CTW, 32		; Progam code is writable!
	.else
		jsr	.ram_tia		; Copy 32-bytes to the VCE.
	.endif

		clc				; Increment the data ptr to
		adc	#32			; the next 32-byte palette.
		bcs	.next_page

.next_palette:	dex				; Any palettes left to xfer?
		bne	.palette_loop

		iny				; Increment the queue index.
		tya
		and	#7
		tay

		cpy	color_queue_w		; Any more items in the queue?
		bne	.next_item
		sty	color_queue_r		; Signal the queue is empty.

		pla				; Restore MPR3 & MPR4.
		tam4
		pla
		tam3

		plp				; Restore interrupt state.

.exit:		stz	color_mutex		; Release color mutex.

.busy:		rts

.next_page:	inc	.ram_tia + 2
		bra	.next_palette

	.if	!CDROM
.ram_tia	=	color_tia		; Use a TIA in RAM.

.tia_func:	tia	0, VCE_CTW, 32
		rts
	.endif	!CDROM

		.bss

color_mutex:	ds	1			; Mutex for VCE changes.
color_queue_r:	ds	1			; Ring buffer read index.
color_queue_w:	ds	1			; Ring buffer write index.
color_index:	ds	8			; Ring buffer - Palette index.
color_count:	ds	8			; Ring buffer - Palette count.
color_addr_l:	ds	8			; Ring buffer - Data Ptr (lo).
color_addr_h:	ds	8			; Ring buffer - Data Ptr (hi).
color_bank:	ds	8			; Ring buffer - Data Ptr (bank).

		.code



; ***************************************************************************
; ***************************************************************************
;
; load_palettes - Queue a set of palettes to upload to the VCE next VBLANK.
;
; Args: _al = Palette index (0..15 for BG, 16..31 for SPR).
; Args: _ah = Palette count (1..32).
; Args: _bp = Pointer to palette data.
; Args:   Y = Bank to map into MPR3 & MPR4, or zero to leave unchanged.
;
; N.B. Y==0 is only useful if the palette data is permanently mapped!
;

load_palettes	.proc

		ldx	color_queue_w		; Get the queue's write index.

		lda.l	<_bp			; Add this set of palettes to
		sta	color_addr_l, x		; the queue.
		lda.h	<_bp
		sta	color_addr_h, x
		tya
		sta	color_bank, x
		lda	<_al
		sta	color_index, x
		lda	<_ah
		sta	color_count, x

		txa				; Increment the queue index.
		inc	a
		and	#7

.wait:		cmp	color_queue_r		; If the queue is full, wait
		beq	.wait			; for the next VBLANK.

		sta	color_queue_w		; Signal item is in the queue.

		leave				; All done, phew!

		.endp



; ***************************************************************************
; ***************************************************************************
;
; clear_vce - Clear all 512 of the VCE's palette entries.
;

clear_vce	.proc

		php				; Disable interrupts to avoid
		sei				; VBLANK palette upload.
		cly
		stz	VCE_CTA+0		; Set VCE write address.
		stz	VCE_CTA+1
.loop:		stz	VCE_CTW+0		; Set lo-byte of color.
		stz	VCE_CTW+1		; Write 1 color value.
		stz	VCE_CTW+0		; Set lo-byte of color.
		stz	VCE_CTW+1		; Write 1 color value.
		dey
		bne	.loop
		plp

		leave				; All done, phew!

		.endp

	.ifdef	HUCC
		.alias	_clear_palette		= clear_vce
	.endif



; ***************************************************************************
; ***************************************************************************
;
; read_palettes - Read palettes from the VCE into a buffer in RAM.
;
; Args: _al = Palette index (0..15 for BG, 16..31 for SPR).
; Args: _ah = Palette count (1..32).
; Args: _di = Pointer to palette data destination in RAM.
;
; The transfer is split into 32-byte chunks so that an HSYNC and/or TIMER
; IRQ is not delayed for too long while executing.
;

read_palettes	.proc

.wait:		lda	#$80			; Acquire color mutex to avoid
		tsb	color_mutex		; conflict with xfer_palettes.
		bmi	.wait

	.if	!CDROM
		tii	.tai_func, color_tia, 8 ; Copy TAI to RAM.
	.endif

.next_item:	lda	<_al			; Get the palette number.
		asl	a
		asl	a
		asl	a
		asl	a
		sta	VCE_CTA + 0
		cla
		rol	a
		sta	VCE_CTA + 1

		ldx	<_ah			; How many palettes to xfer?

		lda.h	<_di
		sta	.ram_tai + 4
		lda.l	<_di
.palette_loop:	sta	.ram_tai + 3

	.if	CDROM
.ram_tai:	tai	VCE_CTR, 0, 32		; Progam code is writable!
	.else
		jsr	.ram_tai		; Copy 32-bytes from the VCE.
	.endif

		clc				; Increment the data ptr to
		adc	#32			; the next 32-byte palette.
		bcs	.next_page

.next_palette:	dex				; Any palettes left to xfer?
		bne	.palette_loop

		stz	color_mutex		; Release color mutex.

		leave

.next_page:	inc	.ram_tai + 4
		bra	.next_palette

	.if	!CDROM
.ram_tai	=	color_tia		; Use a TIA in RAM.

.tai_func:	tai	VCE_CTR, 0, 32
		rts
	.endif	!CDROM

		.endp

	.ifdef	HUCC
		.alias	_read_palette.3		= read_palettes
	.endif



vce_fade_funcs	.procgroup

; ***************************************************************************
; ***************************************************************************
;
; fade_to_black - Create a faded palette in RAM from a reference palette.
;
; Args: _al = Number of colors (1..256).
; Args: _ah = Value to subtract (0..7) from each RGB component.
; Args: _di = Pointer to faded palette destination in RAM.
; Args: _bp = Pointer to reference palette data.
; Args:   Y = Bank to map into MPR3 & MPR4, or zero to leave unchanged.
;
; N.B. Y==0 is only useful if the reference palette data is already mapped!
;

fade_to_black	.proc

		tma3				; Preserve MPR3.
		pha
		tma4				; Preserve MPR4.
		pha

		tya				; Is there a bank to map?
		beq	!+

		jsr	map_bp_to_mpr34		; Map data to MPR3 & MPR4.

!:		lda	<_ah			; Value to subtract (0..7).
		and	#7
		tax

		lda	<_al			; # of colors (1..256).
		beq	.next_page		; Exactly 256 colors?
!:		asl	a
		bcc	.page_loop
		beq	!-			; Exactly 128 colors?

.next_page:	sec				; More than 128 colors.
		inc.h	<_bp
		inc.h	<_di

.page_loop:	php				; C is set if more than 128
		tay				; colors left to fade.

.green:		dey				; Subtract value from green bits.
		lda	[_bp], y
		lsr	a
		dey
		lda	[_bp], y
		and	#%11000000
		ror	a
		sec
		sbc	fade_table_g, x
		bcs	!+
		cla
!:		asl	a
		sta	<__temp
		cla
		rol	a
		iny
		sta	[_di], y

.red:		dey				; Subtract value from red bits.
		lda	[_bp], y
		and	#%00111000
		sec
		sbc	fade_table_r, x
		bcs	!+
		cla
!:		tsb	<__temp

.blue:		lda	[_bp], y		; Subtract value from blue bits.
		and	#%00000111
		sec
		sbc	fade_table_b, x
		bcs	!+
		cla
!:		ora	<__temp
		sta	[_di], y

		tya				; Last color in the page?
		bne	.green
		plp				; Are there another 128 colors?
		bcc	.finished

		dec.h	<_bp			; Fade the 1st 128 colors if
		dec.h	<_di			; there were more than 128.
		clc
		bra	.page_loop

.finished:	pla				; Restore MPR4.
		tam4
		pla				; Restore MPR3.
		tam3

		leave				; All done, phew!

		.endp



; ***************************************************************************
; ***************************************************************************
;
; fade_to_white - Create a faded palette in RAM from a reference palette.
;
; Args: _al = Number of colors (1..256).
; Args: _ah = Value to add (0..7) to each RGB component.
; Args: _di = Pointer to faded palette destination in RAM.
; Args: _bp = Pointer to reference palette data.
; Args:   Y = Bank to map into MPR3 & MPR4, or zero to leave unchanged.
;
; N.B. Y==0 is only useful if the reference palette data is already mapped!
;

fade_to_white	.proc

		tma3				; Preserve MPR3.
		pha
		tma4				; Preserve MPR4.
		pha

		tya				; Is there a bank to map?
		beq	!+

		jsr	map_bp_to_mpr34		; Map data to MPR3 & MPR4.

!:		lda	<_ah			; Value to add (0..7).
		and	#7
		tax

		lda	<_al			; # of colors (1..256).
		beq	.next_page		; Exactly 256 colors?
!:		asl	a
		bcc	.page_loop
		beq	!-			; Exactly 128 colors?

.next_page:	sec				; More than 128 colors.
		inc.h	<_bp
		inc.h	<_di

.page_loop:	php				; C is set if more than 128
		tay				; colors left to fade.

.green:		dey				; Add value to green bits.
		lda	[_bp], y
		lsr	a
		dey
		lda	[_bp], y
		and	#%11000000
		ror	a			; Leaves carry clear.
		adc	fade_table_g, x		; Carry was cleared earlier.
		bcc	!+
		lda	#%11100000
!:		asl	a
		sta	<__temp
		cla
		rol	a			; Leaves carry clear.
		iny
		sta	[_di], y

.red:		dey				; Add value to red bits.
		lda	[_bp], y
		ora	#%11000111
		adc	fade_table_r, x		; Carry was cleared earlier.
		bcc	!+
		clc				; Leaves carry clear.
		lda	#%11111111
!:		eor	#%11000111
		tsb	<__temp

.blue:		lda	[_bp], y		; Add value to blue bits.
		ora	#%11111000
		adc	fade_table_b, x		; Carry was cleared earlier.
		bcc	!+
		lda	#%11111111
!:		eor	#%11111000
		ora	<__temp
		sta	[_di], y

		tya				; Last color in the page?
		bne	.green
		plp				; Are there another 128 colors?
		bcc	.finished

		dec.h	<_bp			; Fade the 1st 128 colors if
		dec.h	<_di			; there were more than 128.
		clc
		bra	.page_loop

.finished:	pla				; Restore MPR4.
		tam4
		pla				; Restore MPR3.
		tam3

		leave				; All done, phew!

		.endp

fade_table_g:	db	%00000000
		db	%00100000
		db	%01000000
		db	%01100000
		db	%10000000
		db	%10100000
		db	%11000000
		db	%11100000

fade_table_r:	db	%00000000
		db	%00001000
		db	%00010000
		db	%00011000
		db	%00100000
		db	%00101000
		db	%00110000
		db	%00111000

fade_table_b:	db	%00000000
		db	%00000001
		db	%00000010
		db	%00000011
		db	%00000100
		db	%00000101
		db	%00000110
		db	%00000111

		.endprocgroup



; ***************************************************************************
; ***************************************************************************
;
; cross_fade_to - Cross fade a palette in RAM towards a reference palette.
;
; Args: _al = Number of colors (1..256).
; Args: _di = Pointer to faded palette destination in RAM.
; Args: _bp = Pointer to reference palette data.
; Args:   Y = Bank to map into MPR3 & MPR4, or zero to leave unchanged.
;
; N.B. Y==0 is only useful if the reference palette data is already mapped!
;
; N.B. This only updates the palette in RAM by 1 RGB step, so it will need
;      to be called 7 times to guarantee that you've reached the target.
;

cross_fade_to	.proc

		tma3				; Preserve MPR3.
		pha
		tma4				; Preserve MPR4.
		pha

		tya				; Is there a bank to map?
		beq	!+

		jsr	map_bp_to_mpr34		; Map data to MPR3 & MPR4.

!:		lda	<_al			; # of colors (1..256).
		beq	.next_page		; Exactly 256 colors?
!:		asl	a
		bcc	.page_loop
		beq	!-			; Exactly 128 colors?

.next_page:	sec				; More than 128 colors.
		inc.h	<_bp
		inc.h	<_di

.page_loop:	php				; C is set if more than 128
		tay				; colors left to fade.

.green:		dey				; Is target green > fade green?
	.if	VCE_SPLIT_CROSS
		bbs0	<_ah, .red
	.endif
		lda	[_bp], y
		lsr	a
		dey
		lda	[_bp], y
		ror	a
		and	#%11100000
		sta	<__temp
		iny
		lda	[_di], y
		lsr	a
		dey
		lda	[_di], y
		ror	a
		php
		tax
		and	#%11100000
		sec
		sbc	<__temp
		sax				; Move X to A but keep Z.
		beq	.set_green		; Don't update if equal green.
		bcs	.dec_green
.inc_green:	adc	#%00100000
		bra	.set_green
.dec_green:	sbc	#%00100000
.set_green:	plp
		rol	a
	.if	VCE_SPLIT_CROSS
		sta	[_di], y		; Save lo-byte of updated color.
	.else
		tax				; Save lo-byte of updated color.
	.endif
		cla
		rol	a
		iny
		sta	[_di], y		; Save hi-byte of updated color.
	.if	VCE_SPLIT_CROSS
		dey
		bra	.next_color
	.endif

.red:		dey
		lda	[_bp], y		; Is target red > fade red?
		and	#%00111000
		sta	<__temp
	.if	VCE_SPLIT_CROSS
		lda	[_di], y
		tax
	.else
		txa
	.endif
		and	#%00111000
		sec
		sbc	<__temp
		sax				; Move X to A but keep Z.
		beq	.set_red		; Don't update if equal red.
		bcs	.dec_red
.inc_red:	adc	#%00001000
		bra	.set_red
.dec_red:	sbc	#%00001000
.set_red:	tax

.blue:		lda	[_bp], y		; Is target blue > fade blue?
		and	#%00000111
		sta	<__temp
		txa
		and	#%00000111
		sec
		sbc	<__temp
		sax				; Move X to A but keep Z.
		beq	.set_blue		; Don't update if equal blue.
		bcs	.dec_blue
.inc_blue:	adc	#%00000001
		bra	.set_blue
.dec_blue:	sbc	#%00000001
.set_blue:	sta	[_di], y		; Save lo-byte of updated color.

.next_color:	tya				; Last color in the page?
		bne	.green
		plp				; Are there another 128 colors?
		bcc	.finished

		dec.h	<_bp			; Fade the 1st 128 colors if
		dec.h	<_di			; there were more than 128.
		clc
		bra	.page_loop

.finished:	pla				; Restore MPR4.
		tam4
		pla				; Restore MPR3.
		tam3

		leave				; All done, phew!

		.endp
