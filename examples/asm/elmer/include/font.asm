; ***************************************************************************
; ***************************************************************************
;
; font.asm
;
; Code for working with fonts.
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

;
;
;

; ***************************************************************************
; ***************************************************************************
;
; dropfnt8x8_sgx - Upload an 8x8 drop-shadowed font to the SGX VDC.
; dropfnt8x8_vdc - Upload an 8x8 drop-shadowed font to the PCE VDC.
;
; Args: _si, _si_bank = _farptr to font data (maps to MPR3 & MPR4).
; Args: _di = ptr to output address in VRAM.
; Args: _al = bitplane 2 value for the tile data ($00 or $FF).
; Args: _ah = bitplane 3 value for the tile data ($00 or $FF).
; Args: _bl = # of font glyphs to upload.
;
; N.B. The font is 1bpp, and the drop-shadow is generated by the CPU.
;
; 12 = background
; 13 = shadow
; 14 = font

		; Temporary 32-byte workspace at the bottom of the stack.

tmp_shadow_buf	equ	$2100			; Interleaved 16 + 1 lines.
tmp_normal_buf	equ	$2101			; Interleaved 16 lines.

		.procgroup			; Keep this code together!

	.if	SUPPORT_SGX
dropfnt8x8_sgx	.proc

		ldx	#SGX_VDC_OFFSET		; Offset to SGX VDC.
		db	$E0			; Turn "clx" into a "cpx #".

		.endp
	.endif

dropfnt8x8_vdc	.proc

		clx				; Offset to PCE VDC.

		tma3				; Preserve MPR3.
		pha

		jsr	set_si_to_mpr3		; Map font to MPR3.
		jsr	set_di_to_vram		; Map _di to VRAM.

		; Generate shadowed glyph.

.tile_loop:	phx				; Preserve VDC/SGX offset.

		clx				; Create a drop-shadowed version
		stz	tmp_shadow_buf, x	; of the glyph.

	.if	0
.line_loop:	lda	[_si]			; Drop-shadow on the LHS.
		sta	tmp_normal_buf, x	; Font data is RHS justified.
		asl	a
	.else
.line_loop:	lda	[_si]			; Drop-shadow on the RHS.
		sta	tmp_normal_buf, x	; Font data is LHS justified.
		lsr	a
	.endif

	.if	0
		ora	[_si]			; Composite font and shadow
		sta	tmp_shadow_buf + 2, x	; planes (wide shadow).
	.else

		sta	tmp_shadow_buf + 2, x	; Composite font and shadow
		ora	[_si]			; planes (normal shadow).
	.endif

		ora	tmp_shadow_buf, x
		eor	tmp_normal_buf, x
		sta	tmp_shadow_buf, x

		inc	<_si			; Increment ptr to font.
		bne	.next_line
		jsr	inc.h_si_mpr3
.next_line:	inx
		inx
		cpx	#8 * 2			; 8 lines high per glyph.
		bne	.line_loop

		plx				; Restore VDC/SGX offset.

		; Upload glyph to VRAM.

.copy_tile:	cly
.plane01_loop:	lda	tmp_shadow_buf, y	; Write bitplane 0 data.
		sta	VDC_DL, x
		iny
		lda	tmp_shadow_buf, y	; Write bitplane 1 data.
		sta	VDC_DH, x
		iny
		cpy	#16
		bne	.plane01_loop

		ldy	#8			; A tile is 8 pixels high.
		lda	<_al			; Write bitplane 2 data.
		sta	VDC_DL, x
		lda	<_ah			; Write bitplane 3 data.
.plane23_loop:	sta	VDC_DH, x
		dey
		bne	.plane23_loop

.next_tile:	dec	<_bl			; Upload next glyph.
		bne	.tile_loop

.exit:		pla				; Restore MPR3.
		tam3

		leave				; All done, phew!

		.endp
		.endprocgroup



; ***************************************************************************
; ***************************************************************************
;
; dropfnt8x16_sgx - Upload an 8x16 drop-shadowed font to the SGX VDC.
; dropfnt8x16_vdc - Upload an 8x16 drop-shadowed font to the PCE VDC.
;
; Args: _si, _si_bank = _farptr to font data (maps to MPR3 & MPR4).
; Args: _di = ptr to output address in VRAM.
; Args: _al = bitplane 2 value for the tile data ($00 or $FF).
; Args: _ah = bitplane 3 value for the tile data ($00 or $FF).
; Args: _bl = # of font glyphs to upload.
;
; N.B. The font is 1bpp, and the drop-shadow is generated by the CPU.
;
; 12 = background
; 13 = shadow
; 14 = font
;
; 0 = trans
; 1 = shadow
; 2 = font

		.procgroup			; Keep this code together!

	.if	SUPPORT_SGX

dropfnt8x16_sgx	.proc

		ldx	#SGX_VDC_OFFSET		; Offset to SGX VDC.
		db	$E0			; Turn "clx" into a "cpx #".

		.endp
	.endif

dropfnt8x16_vdc	.proc
		clx				; Offset to PCE VDC.

		tma3				; Preserve MPR3.
		pha

		jsr	set_si_to_mpr3		; Map font to MPR3.
		jsr	set_di_to_vram		; Map _di to VRAM.

		; Generate shadowed glyph.

.tile_loop:	phx				; Preserve VDC/SGX offset.

		clx				; Create a drop-shadowed version
		stz	tmp_shadow_buf, x	; of the glyph.

	.if	0
.line_loop:	lda	[_si]			; Drop-shadow on the LHS.
		sta	tmp_normal_buf, x	; Font data is RHS justified.
		asl	a
	.else
.line_loop:	lda	[_si]			; Drop-shadow on the RHS.
		sta	tmp_normal_buf, x	; Font data is LHS justified.
		lsr	a
	.endif

	.if	0
		ora	[_si]			; Composite font and shadow
		sta	tmp_shadow_buf + 2, x	; planes (wide shadow).
	.else

		sta	tmp_shadow_buf + 2, x	; Composite font and shadow
		ora	[_si]			; planes (normal shadow).
	.endif

		ora	tmp_shadow_buf, x
		eor	tmp_normal_buf, x
		sta	tmp_shadow_buf, x

		inc	<_si			; Increment ptr to font.
		bne	.next_line
		jsr	inc.h_si_mpr3
.next_line:	inx
		inx
		cpx	#16 * 2			; 16 lines high per glyph.
		bne	.line_loop

		plx				; Restore VDC/SGX offset.

		; Upload glyph to VRAM.

		cly
		bsr	.plane01_loop
		bsr	.fill_plane23

		ldy	#16
		bsr	.plane01_loop
		bsr	.fill_plane23

		dec	<_bl			; Upload next glyph.
		bne	.tile_loop

.exit:		pla				; Restore MPR3.
		tam3

		leave				; All done, phew!

.plane01_loop:	lda	tmp_shadow_buf, y	; Write bitplane 0 data.
		sta	VDC_DL, x
		iny
		lda	tmp_shadow_buf, y	; Write bitplane 1 data.
		sta	VDC_DH, x
		iny
		tya
		and	#$0F
		bne	.plane01_loop
		rts

.fill_plane23:	ldy	#8			; A tile is 8 pixels high.
		lda	<_al			; Write bitplane 2 data.
		sta	VDC_DL, x
		lda	<_ah			; Write bitplane 3 data.
.plane23_loop:	sta	VDC_DH, x
		dey
		bne	.plane23_loop
		rts

		.endp
		.endprocgroup
