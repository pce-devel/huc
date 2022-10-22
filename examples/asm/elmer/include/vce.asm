; ***************************************************************************
; ***************************************************************************
;
; vce.asm
;
; Useful routines for operating the HuC6260 Video Color Encoder
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
; Args: _ah = Palette count (0..32).
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
