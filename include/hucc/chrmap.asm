; ***************************************************************************
; ***************************************************************************
;
; charmap.asm
;
; A simple map system based on 8x8 characters in VDC/BAT format.
;
; Copyright John Brandwood 2025.
;
; Distributed under the Boost Software License, Version 1.0.
; (See accompanying file LICENSE_1_0.txt or copy at
;  http://www.boost.org/LICENSE_1_0.txt)
;
; ***************************************************************************
; ***************************************************************************
;
; The maximum X and Y size for charmaps is 256 characters (2048 pixels).
;
; The maximum total size for a charmap is 16KBytes, which allows for maps up
; to 256x32 tiles (2048x256 pixels).
;
; ***************************************************************************
; ***************************************************************************

;
; Include dependancies ...
;

		include "metamap.asm"		; This defines the variables.

;
; Charmaps in BAT format can either directly store the character number, or
; they can be limited to use the 32KByte of characters in VRAM $1000..$4FFF
; which then frees up 2-bits for flag information for each character in the
; BAT entry.
;
; These 2-bits are perfect for using as collision information in game maps,
; allowing storage of states like transparent, solid, up-slope, down-slope.
;
; Typically this flag information is set by the map conversion tools from a
; seperate "collision" map layer.
;

	.ifndef	CHARDEF_CHR_FLAG
CHARDEF_CHR_FLAG=0		; (0 or 1)
	.endif

;
;
;



charmap_group	.procgroup

; ***************************************************************************
; ***************************************************************************
;
; _draw_bat - Draw the entire screen at the current coordinates.
; _sgx_draw_bat - Draw the entire screen at the current coordinates.
;
; void __fastcall draw_bat( void );
; void __fastcall sgx_draw_bat( void );
;

	.if	SUPPORT_SGX

_sgx_draw_bat	.proc

		ldx	#SGX_VDC_OFFSET		; Offset to SGX VDC.
		db	$F0			; Turn "clx" into a "beq".

		.ref	_draw_bat
		.endp
	.endif

_draw_bat	.proc

		clx				; Offset to PCE VDC.

		tma3				; Preserve MPR3..MPR5.
		pha
		tma4
		pha
		tma5
		pha

		jsr	bat_pxl_2_chr		; Set up the draw coordinates.

		lda	<map_chr_x		; Reset previous X position.
		sta	vdc_old_chr_x, x

		lda	<map_chr_y		; Reset previous Y position,
		inc	a			; ready to draw rows upwards.
		sta	vdc_old_chr_y, x

		lda	vdc_map_draw_w, x	; Draw the whole screen.
		sta	<map_draw_w
		lda	vdc_map_draw_h, x
		sta	<map_draw_h

		jsr	bat_scroll_y		; Draw N row of CHR to the BAT.

		pla				; Restore MPR3..MPR5.
		tam5
		pla
		tam4
		pla
		tam3

		leave

		.endp



; ***************************************************************************
; ***************************************************************************
;
; _scroll_bat - Draw a single row of CHR into the BAT to update the edge.
; _sgx_scroll_bat - Draw a single row of CHR into the BAT to update the edge.
;
; void __fastcall scroll_bat( void );
; void __fastcall sgx_scroll_bat( void );
;

	.if	SUPPORT_SGX

_sgx_scroll_bat	.proc

		ldx	#SGX_VDC_OFFSET		; Offset to SGX VDC.
		db	$F0			; Turn "clx" into a "beq".

		.ref	_scroll_bat
		.endp
	.endif

_scroll_bat	.proc

		clx				; Offset to PCE VDC.

		tma3				; Preserve MPR3..MPR5.
		pha
		tma4
		pha
		tma5
		pha

		jsr	bat_pxl_2_chr		; Set up the draw coordinates.

		lda	vdc_map_draw_h, x	; Draw new LHS or RHS if needed.
		sta	<map_draw_h
;		lda	#1			; bat_scroll_x only ever draws a
;		sta	<map_draw_w		; single column.
		jsr	bat_scroll_x

		lda	vdc_old_chr_x, x	; Restore map_chr_x which could
		sta	<map_chr_x		; be changed by bat_scroll_x.

		lda	vdc_map_draw_w, x	; Draw new TOP or BTM if needed.
		sta	<map_draw_w
		lda	#1
		sta	<map_draw_h
		jsr	bat_scroll_y

		pla				; Restore MPR3..MPR5.
		tam5
		pla
		tam4
		pla
		tam3

		leave

		.endp



; ***************************************************************************
; ***************************************************************************
;
; _blit_bat - Draw a map rectangle to specific BAT coordinates.
; _sgx_blit_bat - Draw a map rectangle to specific BAT coordinates.
;
; void __fastcall blit_bat( unsigned char tile_x<map_bat_x>, unsigned char tile_y<map_bat_y>, unsigned char tile_w<map_draw_x>, unsigned char tile_h<map_draw_y> );
; void __fastcall sgx_blit_bat( unsigned char tile_x<map_bat_x>, unsigned char tile_y<map_bat_y>, unsigned char tile_w<map_draw_x>, unsigned char tile_h<map_draw_y> );
;
; Normally you'd just use _draw_bat() and _scroll_bat(), but for those folks
; who really wish to take manual control, you can use this.
;

	.if	SUPPORT_SGX

_sgx_blit_bat	.proc

		ldx	#SGX_VDC_OFFSET		; Offset to SGX VDC.
		db	$F0			; Turn "clx" into a "beq".

		.ref	_blit_bat
		.endp
	.endif

_blit_bat	.proc

		clx				; Offset to PCE VDC.

		tma3				; Preserve MPR3..MPR5.
		pha
		tma4
		pha
		tma5
		pha

		lda	vdc_map_option, x	; Preserve current map options.
		pha
		ora	#MAP_UNALIGNED_X | MAP_UNALIGNED_Y
		sta	vdc_map_option, x

		jsr	bat_pxl_2_chr		; Set up the draw coordinates.

		lda	<map_draw_w		; Are we drawing just 1 column?
		cmp	#1
		beq	.draw_column

		; Draw N rows.

.draw_rows:	lda	<map_chr_x		; Reset previous X position.
		sta	vdc_old_chr_x, x

		lda	<map_chr_y		; Reset previous Y position,
		inc	a			; ready to draw rows upwards.
		sta	vdc_old_chr_y, x

		jsr	bat_scroll_y		; Draw a row of CHR to the BAT.

		; Drawing completed.

.finished:	pla				; Restore previous map options.
		sta	vdc_map_option, x

		pla				; Restore MPR3..MPR5.
		tam5
		pla
		tam4
		pla
		tam3

.exit:		leave

		; Draw 1 column.

.draw_column:	lda	<map_chr_x		; Reset previous X position.
		inc	a			; ready to draw 1 column.
		sta	vdc_old_chr_x, x

		lda	<map_chr_y		; Reset previous Y position.
		sta	vdc_old_chr_y, x

		jsr	bat_scroll_x		; Draw a single column of CHR.

		bra	.finished

		.endp



; ***************************************************************************
; ***************************************************************************
;

bat_pxl_2_chr:	lda.l	vdc_map_pxl_x, x	; Get current map X coordinate.
		sta.l	<map_pxl_x
		lda.h	vdc_map_pxl_x, x	; Xvert map_pxl_x to map_chr_x.
		lsr	a
		ror.l	<map_pxl_x
		lsr	a
		ror.l	<map_pxl_x
		lsr	a
		ror.l	<map_pxl_x		; Max map width is 256 CHR.

		lda.l	vdc_map_pxl_y, x	; Get current map Y coordinate.
		sta.l	<map_pxl_y
		lda.h	vdc_map_pxl_y, x	; Xvert map_pxl_y to map_chr_y.
		lsr	a
		ror.l	<map_pxl_y
		lsr	a
		ror.l	<map_pxl_y
		lsr	a
		ror.l	<map_pxl_y		; Max map width is 256 CHR.

		rts



; ***************************************************************************
; ***************************************************************************
;
; bat_scroll_x - Update the BAT when X coordinate changes.
;
; N.B. This only ever draws a single column!
;

!no_change:	rts

bat_scroll_x:	lda	<map_chr_x		; Compare old_x with cur_x.
		cmp	vdc_old_chr_x, x
		beq	!no_change-		; Do nothing if no change.
		sta	vdc_old_chr_x, x
		bmi	.moved			; Test the sign of the change.

		clc				; Draw RHS if chr_x >= old_x.
		adc	vdc_map_draw_w, x	; Usually (SCR_WIDTH / 8) + 1.
		dec	a

.moved:		pha				; Push chr_x in map section.

		bit	vdc_map_option, x	; Set bit7 to disable aligning
		bmi	!+			; BAT X with the map X.
		and	vdc_bat_x_mask, x
		sta	<map_bat_x		; Save BAT X chr coordinate.

!:		lda	<map_chr_y		; Y = chr_y in map section.
		tay

		bit	vdc_map_option, x	; Set bit6 to disable aligning
		bvs	!+			; BAT Y with the map Y.
		and	vdc_bat_y_mask, x
		sta	<map_bat_y		; Save BAT Y chr coordinate.

	.if	FAST_MULTIPLY
!:		lda	#^square_plus_lo	; Put the table-of-squares back
		tma5				; into MPR5.
	.endif

!:		lda	vdc_map_line_w, x	; Map width in CHR (0 == 256).
		bne	!+

		stz.l	<map_line		; Multiply by 256 is easy!
		tya
		bra	.double

	.if	FAST_MULTIPLY
!:		sta	<mul_sqrplus_lo		; Takes 54 cycles.
		sta	<mul_sqrplus_hi
		eor	#$FF
		sta	<mul_sqrminus_lo
		sta	<mul_sqrminus_hi
		sec
		lda	[mul_sqrplus_lo], y
		sbc	[mul_sqrminus_lo], y
		sta.l	<map_line		; Lo-byte of (CHR Y * width).
		lda	[mul_sqrplus_hi], y
		sbc	[mul_sqrminus_hi], y
	.else
!:		sty.h	<map_line		; Takes 144..176 cycles.
		ldy	#8
		lsr	a
		sta.l	<map_line
		cla
		bcc	.rotate
.add:		clc
		adc.h	<map_line
.rotate:	ror	a
		ror.l	<map_line		; Lo-byte of (CHR Y * width).
		dey
		bcs	.add
		bne	.rotate
	.endif

.double:	asl.l	<map_line		; Lo-byte of (CHR Y * width * 2).
		rol	a
		tay				; Hi-byte of (CHR Y * width * 2).

		lda	vdc_map_bank, x		; Put the MAP into MPR3-MPR5.
		tam3				; Allow for 16KByte charmap.
		inc	a
		tam4
		inc	a
		tam5

		pla				; Pop chr_x in map section.
		asl	a			; 2-bytes for a BAT value.
		bcc	!+
		iny				; Hi-byte of (CHR Y * width * 2).
		clc
!:		adc.l	<map_line		; Lo-byte of (CHR Y * width * 2).
		bcc	!+
		iny

!:		clc				; Calc map data pointer.
		adc.l	vdc_map_addr, x
		sta.l	<_bp			; Maximum map size is 16KBytes
		tya				; so we don't need to consider
		adc.h	vdc_map_addr, x		; bank overflow.
		sta.h	<_bp

		cly				; Calculate the map line delta
		lda	vdc_map_line_w, x	; in bytes.
		bne	!+
		iny
!:		asl	a
		sta.l	<map_line
		tya
		rol	a
		sta.h	<map_line

		; Draw the first part of the column.

.draw_col:	lda	<map_bat_x		; Set the BAT VRAM destination
		sta.l	<_di			; coordinates.
		lda	<map_bat_y
		sta.h	<_di

		eor	vdc_bat_y_mask, x	; Calc CHR before wrap.
		inc	a
		cmp	<map_draw_h		; Usually (SCR_HEIGHT / 8) + 1.
		bcc	!+
		lda	<map_draw_h		; Maximum CHR to draw.
!:		sta	<map_count		; Set number of CHR to draw.
		sta	<map_drawn		; Preserve number of CHR drawn.

		lda	#VDC_CR			; Set VDC auto-increment from
		sta	<vdc_reg, x		; the BAT width, which is set
		sta	VDC_AR, x		; up by set_screen_size().
		lda	<vdc_crh, x
		sta	VDC_DH, x

		jsr	bat_col_strip		; Draw top of vertical strip.

		; Wrap around and draw the rest of the column (if needed).

		sec				; Are there any more CHR that
		lda	<map_draw_h		; need to be drawn?
		sbc	<map_drawn
		beq	.done

		sta	<map_count		; Set number of CHR to draw.

		lda	<map_bat_x		; Set the BAT VRAM destination
		sta.l	<_di			; coordinates.
		stz.h	<_di			; Reset 1st row to draw.

		jsr	bat_col_strip		; Draw btm of vertical strip.

.done:		lda	#VDC_CR			; Set VDC auto-increment to 1.
		sta	<vdc_reg, x
		sta	VDC_AR, x
		stz	VDC_DH, x

		rts



; ***************************************************************************
; ***************************************************************************
;
; bat_scroll_y - Update the BAT when Y coordinate changes.
;
; N.B. This draws multiple rows when called from _draw_bat or _blit_bat.
;

!no_change:	rts

bat_scroll_y:	lda	<map_chr_y		; Compare old_y with cur_y.
		cmp	vdc_old_chr_y, x
		beq	!no_change-		; Do nothing if no change.
		sta	vdc_old_chr_y, x
		bmi	.moved			; Test the sign of the change.

		clc				; Draw bottom if chr_y >= old_y.
		adc	vdc_map_draw_h, x	; Usually (SCR_HEIGHT / 8) + 1.
		dec	a

.moved:		tay				; Y = chr_y in map section.

		bit	vdc_map_option, x	; Set bit6 to disable aligning
		bvs	!+			; BAT Y with the map Y.
		and	vdc_bat_y_mask, x
		sta	<map_bat_y		; Save BAT Y chr coordinate.

!:		bit	vdc_map_option, x	; Set bit7 to disable aligning
		bmi	!+			; BAT X with the map X.
		lda	<map_chr_x
		and	vdc_bat_x_mask, x
		sta	<map_bat_x		; Save BAT X chr coordinate.

	.if	FAST_MULTIPLY
!:		lda	#^square_plus_lo	; Put the table-of-squares back
		tma5				; into MPR5.
	.endif

!:		lda	vdc_map_line_w, x	; Map width in CHR (0 == 256).
		bne	!+

		stz.l	<map_line		; Multiply by 256 is easy!
		tya
		bra	.double

	.if	FAST_MULTIPLY
!:		sta	<mul_sqrplus_lo		; Takes 54 cycles.
		sta	<mul_sqrplus_hi
		eor	#$FF
		sta	<mul_sqrminus_lo
		sta	<mul_sqrminus_hi
		sec
		lda	[mul_sqrplus_lo], y
		sbc	[mul_sqrminus_lo], y
		sta.l	<map_line		; Lo-byte of (CHR Y * width).
		lda	[mul_sqrplus_hi], y
		sbc	[mul_sqrminus_hi], y
	.else
!:		sty.h	<map_line		; Takes 144..176 cycles.
		ldy	#8
		lsr	a
		sta.l	<map_line
		cla
		bcc	.rotate
.add:		clc
		adc.h	<map_line
.rotate:	ror	a
		ror.l	<map_line		; Lo-byte of (CHR Y * width).
		dey
		bcs	.add
		bne	.rotate
	.endif

.double:	asl.l	<map_line		; Lo-byte of (CHR Y * width * 2).
		rol	a
		sta.h	<map_line		; Hi-byte of (CHR Y * width * 2).
		tay

		lda	vdc_map_bank, x		; Put the MAP into MPR3-MPR5.
		tam3				; Allow for 16KByte charmap.
		inc	a
		tam4
		inc	a
		tam5

		; Loop to here if drawing multiple rows.

.next_row:	lda	<map_chr_x		; Map CHR X coordinate.
		asl	a			; 2-bytes for a BAT value!
		bcc	!+
		iny				; Hi-byte of (CHR Y * width * 2).
		clc
!:		adc.l	<map_line		; Lo-byte of (CHR Y * width * 2).
		bcc	!+
		iny
!:		clc				; Calc map data pointer.
		adc.l	vdc_map_addr, x
		sta.l	<_bp			; Maximum map size is 16KBytes
		tya				; so we don't need to consider
		adc.h	vdc_map_addr, x		; bank overflow.
		sta.h	<_bp

		; Draw the first part of the row.

.draw_row:	lda	<map_bat_y		; Set the BAT VRAM destination
		sta.h	<_di			; coordinates.
		lda	<map_bat_x
		sta.l	<_di

		eor	vdc_bat_x_mask, x	; Calc CHR before wrap.
		inc	a
		cmp	<map_draw_w		; Usually (SCR_WIDTH / 8) + 1.
		bcc	!+
		lda	<map_draw_w		; Maximum CHR to draw.
!:		sta	<map_count		; Set number of CHR to draw.
		sta	<map_drawn		; Preserve number of CHR drawn.

		jsr	bat_row_strip		; Draw lhs of horizontal strip.

		; Wrap around and draw the rest of the row (if needed).

		sec				; Are there any more CHR that
		lda	<map_draw_w		; need to be drawn?
		sbc	<map_drawn
		beq	.done_row

		sta	<map_count		; Set number of CHR to draw.

		lda	<map_bat_y		; Set the BAT VRAM destination
		sta.h	<_di			; coordinates.
		stz.l	<_di			; Reset 1st column to draw.

		jsr	bat_row_strip		; Draw rhs of horizontal strip.

.done_row:	dec	<map_draw_h		; Are all desired rows drawn?
		beq	.finished

		lda	<map_bat_y		; Move BAT Y down by 1.
		inc	a
		and	vdc_bat_y_mask, x
		sta	<map_bat_y

		ldy.h	<map_line		; Move the map line pointer to
		iny				; the next line.
		lda	vdc_map_line_w, x
		beq	!++			; vdc_map_line_w == 0 == 256.
		asl	a			; 2-bytes for a BAT value!
		bcs	!+
		dey				; Fix if vdc_map_line_w <= 127.
!:		clc
		adc.l	<map_line
		sta.l	<map_line
		bcc	!++
!:		iny
!:		sty.h	<map_line		; Needed for .next_row!

		bra	.next_row		; Draw next row.

.finished:	rts



; ***************************************************************************
; ***************************************************************************
;
; bat_row_strip - Draw a single row of CHR into the BAT.
;

bat_row_strip:	jsr	set_di_xy_mawr		; Set the BAT VRAM destination.

		cly				; 2
.repeat:	lda	[_bp], y		; 7
		sta	VDC_DL, x		; 6
		iny				; 2
		lda	[_bp], y		; 7
	.if	CHARDEF_CHR_FLAG
		and	#%11110011		; 2
		inc	a			; 2
;	.else
;		and	#%11110111		; 2
	.endif
		sta	VDC_DH, x		; 6
		iny				; 2

!:		dec	<map_count		; 6
		bne	.repeat			; 4

		tya				; 2
		beq	!+			; 2/4
		clc				; 2
		adc.l	<_bp			; 4
		sta.l	<_bp			; 4
		bcc	.done			; 2/4
!:		inc.h	<_bp			; 6

.done:		rts



; ***************************************************************************
; ***************************************************************************
;
; bat_col_strip - Draw a single column of CHR into the BAT.
;

bat_col_strip:	jsr	set_di_xy_mawr		; Set the BAT VRAM destination.

		ldy	#1			; 2
.repeat:	lda	[_bp]			; 7
		sta	VDC_DL, x		; 6
		lda	[_bp], y		; 7
	.if	CHARDEF_CHR_FLAG
		and	#%11110011		; 2
		inc	a			; 2
;	.else
;		and	#%11110111		; 2
	.endif
		sta	VDC_DH, x		; 6

		clc				; 2
		lda.l	<map_line		; 4
		adc.l	<_bp			; 4
		sta.l	<_bp			; 4
		lda.h	<map_line		; 4
		adc.h	<_bp			; 4
		sta.h	<_bp			; 4

		dec	<map_count		; 6
		bne	.repeat			; 4

		rts

	.endprocgroup	; charmap_group
