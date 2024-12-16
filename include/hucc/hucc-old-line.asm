; ***************************************************************************
; ***************************************************************************
;
; hucc-old-line.asm
;
; Based on the original HuC and MagicKit functions by David Michel and the
; other original HuC developers.
;
; Modifications copyright John Brandwood 2024.
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
; void __fastcall gfx_init( unsigned int start_vram_addr<_ax> );
;
; initialize graphics mode
; - points graphics map to tiles at start_vram_addr

_gfx_init.1	.proc

		stz.l	<_di
		stz.h	<_di
		jsr	vdc_di_to_mawr

		lda	vdc_bat_limit		; Size of BAT determines
		inc	a			; how many bitmapped BAT
		sta	<__temp			; entries to write.

		lda.h	<_ax
		lsr	a
		ror.l	<_ax
		lsr	a
		ror.l	<_ax
		lsr	a
		ror.l	<_ax
		lsr	a
		ror.l	<_ax
		ora	#0			; Palette 0.
		ldy.l	<_ax

		clx
.loop:		sty	VDC_DL
		sta	VDC_DH
		iny
		bne	!+
		inc	a
!:		dex
		bne	.loop
		dec	<__temp
		bne	.loop

		leave

		.endp



; ***************************************************************************
; ***************************************************************************
;
; void __fastcall gfx_clear( unsigned int start_vram_addr<_di> );
;
; Clear the values in the graphics tiles
; - places zeroes in graphics tiles at start_vram_addr

_gfx_clear.1	.proc

		jsr	vdc_di_to_mawr

		lda	vdc_bat_limit		; Size of BAT determines
		inc	a			; how many tiles to clear.
		asl	a
		asl	a
		asl	a
		cly
		st1	#0
.loop:		st2	#0
		st2	#0
		dey
		bne	.loop
		dec	a
		bne	.loop

		leave

		.endp



; ***************************************************************************
; ***************************************************************************
;
; void __fastcall gfx_line( unsigned int x1<_gfx_x1>, unsigned int y1<_gfx_y1>, unsigned int x2<_gfx_x2>, unsigned int y2<_gfx_y2>, unsigned char color<_gfx_color> );

huc_gfx_line	.procgroup

_gfx_line.5:	.proc

; On entry ...

_gfx_x1		=	_ax
_gfx_y1		=	_bx
_gfx_x2		=	_cx
_gfx_y2		=	_dx
_gfx_color	=	__fptr

; During routine ...

_gfx_err	=	_cx
_gfx_adj	=	_dx
_gfx_dx		=	_bp
_gfx_dy		=	_si
_gfx_xdir	=	__fptr + 1

		clx				; Offset to PCE VDC.

		sec
		lda.l	<_gfx_y2
		sbc.l	<_gfx_y1
		tay
		lda.h	<_gfx_y2
		sbc.h	<_gfx_y1
		bcc	.y2_lt_y1

.y2_ge_y1:	sty.l	<_gfx_dy
		sta.h	<_gfx_dy

		sec
		lda.l	<_gfx_x2
		sbc.l	<_gfx_x1
		tay
		lda.h	<_gfx_x2
		sbc.h	<_gfx_x1
		bra	.check_dx

.y2_lt_y1:	eor	#$FF
		say
		eor	#$FF
		inc	a
		bne	!+
		iny
!:		sta.l	<_gfx_dy
		sty.h	<_gfx_dy

		sec
		lda.l	<_gfx_x1
		sbc.l	<_gfx_x2
		tay
		lda.h	<_gfx_x1
		sbc.h	<_gfx_x2

		tii	_gfx_x2, _gfx_x1, 4

.check_dx:	sta	<_gfx_xdir
		bpl	.save_dx

.negate_dx:	say
		eor	#$FF
		adc	#1
		say
		eor	#$FF
		adc	#0

.save_dx:	sty.l	<_gfx_dx
		sta.h	<_gfx_dx

		say
		cmp.l	<_gfx_dy
		say
		sbc.h	<_gfx_dy
		bcc	.dy_gt_dx

		; ******
		; _gfx_dy is difference from end to start (positive)
		; _gfx_dx is difference from end to start (positive)
		; _gfx_xdir shows whether to apply deltax positive or negative

.dx_ge_dy:	asl.l	<_gfx_dy		; _gfx_dy * 2
		rol.h	<_gfx_dy
		sec				; _gfx_err = _gfx_dy * 2 - _gfx_dx
		lda.l	<_gfx_dy
		sbc.l	<_gfx_dx
		tay
		lda.h	<_gfx_dy
		sbc.h	<_gfx_dx
		sty.l	<_gfx_err
		sta.h	<_gfx_err
		say
		sec				; _gfx_adj = _gfx_dy * 2 - _gfx_dx * 2
		sbc.l	<_gfx_dx
		sta.l	<_gfx_adj
		tya
		sbc.h	<_gfx_dx
		sta.h	<_gfx_adj
		bra	.plot_x

.loop_x:	lda.l	<_gfx_dx		; used as counter - get both endpoints
		bne	!+
		dec.h	<_gfx_dx
		bmi	.finished
!:		dec.l	<_gfx_dx

.plot_x:	jsr	!plot_pixel+		; draw the current pixel at (_ax, _bx).

		lda.h	<_gfx_err
		bmi	.loop_x_keep_y

.loop_x_next_y:	inc.l	<_gfx_y1
		bne	!+
		inc.h	<_gfx_y1

!:		clc				; _gfx_err += _gfx_dy * 2 - _gfx_dx * 2
		lda.l	<_gfx_err
		adc.l	<_gfx_adj
		sta.l	<_gfx_err
		lda.h	<_gfx_err
		adc.h	<_gfx_adj
		sta.h	<_gfx_err
		bra	.loop_x_next_x

.loop_x_keep_y:	clc				; _gfx_err += _gfx_dy * 2
		lda.l	<_gfx_err
		adc.l	<_gfx_dy
		sta.l	<_gfx_err
		lda.h	<_gfx_err
		adc.h	<_gfx_dy
		sta.h	<_gfx_err

.loop_x_next_x:	lda	<_gfx_xdir		; adjust currx
		bpl	.loop_x_inc_x

.loop_x_dec_x:	lda.l	<_gfx_x1
		bne	!+
		dec.h	<_gfx_x1
!:		dec.l	<_gfx_x1
		bra	.loop_x

.loop_x_inc_x:	inc.l	<_gfx_x1
		bne	.loop_x
		inc.h	<_gfx_x1
		bra	.loop_x

		; ******

.finished:	leave

		; ******

.dy_gt_dx:	asl.l	<_gfx_dx		; _gfx_dx * 2
		rol.h	<_gfx_dx
		sec				; _gfx_err = _gfx_dx * 2 - _gfx_dy
		lda.l	<_gfx_dx
		sbc.l	<_gfx_dy
		tay
		lda.h	<_gfx_dx
		sbc.h	<_gfx_dy
		sty.l	<_gfx_err
		sta.h	<_gfx_err
		say
		sec				; _gfx_adj = _gfx_dx * 2 - _gfx_dy * 2
		sbc.l	<_gfx_dy
		sta.l	<_gfx_adj
		tya
		sbc.h	<_gfx_dy
		sta.h	<_gfx_adj
		bra	.plot_y

.loop_y:	lda.l	<_gfx_dy		; used as counter - get both endpoints
		bne	!+
		dec.h	<_gfx_dy
		bmi	.finished
!:		dec.l	<_gfx_dy

.plot_y:	jsr	!plot_pixel+		; draw the current pixel at (_ax, _bx).

.loop_y_next_y:	inc.l	<_gfx_y1
		bne	!+
		inc.h	<_gfx_y1

!:		lda.h	<_gfx_err
		bmi	.loop_y_keep_x

		clc				; _gfx_err += _gfx_dx * 2 - _gfx_dy * 2
		lda.l	<_gfx_err
		adc.l	<_gfx_adj
		sta.l	<_gfx_err
		lda.h	<_gfx_err
		adc.h	<_gfx_adj
		sta.h	<_gfx_err

.loop_y_next_x:	lda	<_gfx_xdir
		bpl	.loop_y_inc_x

.loop_y_dec_x:	lda.l	<_gfx_x1
		bne	!+
		dec.h	<_gfx_x1
!:		dec.l	<_gfx_x1
		bra	.loop_y

.loop_y_inc_x:	inc.l	<_gfx_x1
		bne	.loop_y
		inc.h	<_gfx_x1
		bra	.loop_y

.loop_y_keep_x:	clc				; _gfx_err += _gfx_dx * 2
		lda.l	<_gfx_err
		adc.l	<_gfx_dx
		sta.l	<_gfx_err
		lda.h	<_gfx_err
		adc.h	<_gfx_dx
		sta.h	<_gfx_err
		bra	.loop_y

		.endp

		; ******
		; Plot a point at location (x,y) in color

!plot_pixel:	jsr	.set_vram_addr

		lda.l	<_gfx_x1		; X coordinate.
		and	#7
		eor	#7			; Flip bits.
		tay

		lda	bit_mask, y
		bbr0	<_si, .clr_bit0
.set_bit0:	ora	VDC_DL, x
		bra	!+
.clr_bit0:	eor	#$FF
		and	VDC_DL, x
!:		sta	VDC_DL, x

		lda	bit_mask, y
		bbr1	<_si, .clr_bit1
.set_bit1:	ora	VDC_DH, x
		bra	!+
.clr_bit1:	eor	#$FF
		and	VDC_DH, x
!:		sta	VDC_DH, x

		smb3	<_di
		jsr	set_di_to_marr
		jsr	set_di_to_mawr

		lda	bit_mask, y
		bbr2	<_si, .clr_bit2
.set_bit2:	ora	VDC_DL, x
		bra	!+
.clr_bit2:	eor	#$FF
		and	VDC_DL, x
!:		sta	VDC_DL, x

		lda	bit_mask, y
		bbr3	<_si, .clr_bit3
.set_bit3:	ora	VDC_DH, x
		bra	!+
.clr_bit3:	eor	#$FF
		and	VDC_DH, x
!:		sta	VDC_DH, x

		rts

		; ******
		; Utility routine to select x/y pixel co-ordinates in VRAM

.set_vram_addr:	lda.h	<_gfx_x1		; X pixel (0..1023) to tile (0..127).
		lsr	a
		tay
		lda.l	<_gfx_x1
		ror	a
		say
		lsr	a
		tya
		ror	a
		lsr	a
		and	vdc_bat_x_mask, x
		sta.l	<_di

		lda.h	<_gfx_y1		; Y pixel (0..511) to tile (0..63).
		lsr	a
		lda.l	<_gfx_y1
		ror	a
		lsr	a
		lsr	a
		sta.h	<_di
		cla
		bit	vdc_bat_width, x
		bmi	.w128
		bvs	.w64
.w32:		lsr.h	<_di
		ror	a
.w64:		lsr.h	<_di
		ror	a
.w128:		lsr.h	<_di
		ror	a
		ora.l	<_di
		sta.l	<_di
		jsr	set_di_to_marr

		lda	VDC_DL, x		; Read tile# from BAT
		sta.l	<_di			; and calc its address.
		lda	VDC_DH, x
		asl.l	<_di
		asl	a
		asl.l	<_di
		asl	a
		asl.l	<_di
		asl	a
		asl.l	<_di
		asl	a
		sta.h	<_di

		lda.l	<_gfx_y1		; Add row within tile.
		and	#7
		clc
		adc.l	<_di
		sta.l	<_di

		jsr	set_di_to_marr
		jmp	set_di_to_mawr



; ***************************************************************************
; ***************************************************************************
;
; void __fastcall gfx_plot( unsigned int x<_gfx_x1>, unsigned int y<_gfx_y1>, char color<_gfx_color> );

_gfx_plot.3:	.proc

		clx				; Offset to PCE VDC.
		jsr	!plot_pixel-
		leave

		.endp

		.endprocgroup	; huc_gfx_line
