; ***************************************************************************
; ***************************************************************************
;
; hucc-old-map.asm
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
; void __fastcall load_map( unsigned char bat_x<_al>, unsigned char bat_y<_ah>, int map_x<_bx>, int map_y<_dx>, unsigned char tiles_w<_cl>, unsigned char tiles_h<_ch> );
; void __fastcall sgx_load_map( unsigned char bat_x<_al>, unsigned char bat_y<_ah>, int map_x<_bx>, int map_y<_dx>, unsigned char tiles_w<_cl>, unsigned char tiles_h<_ch> );
;
; Transfer a HuC tiled map into the BAT in VRAM
;
; _al = BAT x screen coordinate (in tile units)
; _ah = BAT y screen coordinate
; _bl = MAP x start coordinate in the map
; _bh = MAP y start coordinate
; _cl = #TILES width to draw
; _ch = #TILES height to draw
;
; set by load_map_init()
; _di = BAT address
; _si = palette index table ptr
; _bp = map address

huc_map_funcs	.procgroup

		.bss

; **************
; 16-bytes of VDC HuC map info.
;
; N.B. MUST be 16-bytes before the SGX versions to use PCE_VDC_OFFSET.

; Initialized by set_map_data()
vdc_map_wrap	ds	1			; unused
vdc_map_bank	ds	1
vdc_map_addr	ds	2
vdc_map_width	ds	1
vdc_map_height	ds	1

; From hucc-old-spr.asm just to save space. This NEEDS to be changed!
spr_max:	ds	1
spr_clr:	ds	1

		ds	8			; WASTED (at the moment)

	.if	SUPPORT_SGX

; **************
; 16-bytes of SGX HuC map info.
;
; N.B. MUST be 16-bytes after the VDC versions to use SGX_VDC_OFFSET.

; Initialized by sgx_set_map_data()
sgx_map_wrap	ds	1			; unused
sgx_map_bank	ds	1
sgx_map_addr	ds	2
sgx_map_width	ds	1
sgx_map_height	ds	1

; From hucc-old-spr.asm just to save space. This NEEDS to be changed!
sgx_spr_max:	ds	1
sgx_spr_clr:	ds	1

	.endif

		.code

	.if	SUPPORT_SGX

		.proc	_sgx_load_map.6

		ldy	#SGX_VDC_OFFSET		; Offset to SGX VDC.
		db	$F0			; Turn "cly" into a "beq".

		.ref	_load_map.6
		.endp

	.endif

		.proc	_load_map.6

.bat_x		=	_al
.bat_y		=	_ah
.map_x		=	_bl
.map_y		=	_bh
.tiles_w	=	_cl
.tiles_h	=	_ch
.bat_line	=	_di
.map_line	=	_bp
.attr_tbl	=	_si

		cly				; Offset to PCE VDC.

		phx				; Preserve X (aka __sp).
		sxy				; Put VDC offset in X.

		; wrap 16-bit signed map x coordinate, and only use the bottom byte.

.wrap_map_x:	lda.l	<_bx
		ldy	vdc_map_width, x	; If map_width == 256, then
		beq	.mapx_ok		; just take the lo-byte of X.

		ldy.h	<_bx
		bmi	.mapx_neg
.mapx_pos:	sec
		sbc	vdc_map_width, x
		bcs	.mapx_pos
		dey
		bpl	.mapx_pos
.mapx_neg:	clc
		adc	vdc_map_width, x
		bcc	.mapx_neg
		iny
		bmi	.mapx_neg

.mapx_ok:	sta	<.map_x			; 0 <= map_x < mapwidth

		; wrap 16-bit signed map y coordinate, and only use the bottom byte.

.wrap_map_y:	lda.l	<_dx
		ldy	vdc_map_height, x	; If map_height == 256, then
		beq	.mapy_ok		; just take the lo-byte of Y.

		ldy.h	<_dx
		bmi	.mapy_neg
.mapy_pos:	sec
		sbc	vdc_map_height, x
		bcs	.mapy_pos
		dey
		bpl	.mapy_pos
.mapy_neg:	clc
		adc	vdc_map_height, x
		bcc	.mapy_neg
		iny
		bmi	.mapy_neg

.mapy_ok:	sta	<.map_y			; 0 <= map_y < mapheight

		tma2				; Preserve MPR2, MPR3, MPR4.
		pha
		tma3
		pha
		tma4
		pha

		; ******************************
		; Calculate the map/bat pointers
		; ******************************

		jsr	!calc_bat_tile+		; Calculate bat tile address.

		jsr	!calc_map_line+		; Calculate map line address.

		; Now actually draw the map!

		lda	vdc_tile_type, x
		cmp	#16
		bcc	.draw_line8
		jmp	.draw_line16

		; ******************************
		; Draw a bytemap of 8x8 tiles
		; ******************************

.next_line8:	clc				; 8x8 tiles
		jsr	.next_map_line

.draw_line8:	lda	<.tiles_w
		sta.l	<__temp			; width counter in __temp.l

		lda	vdc_bat_width, x	; calculate #tiles to edge
		sec
		sbc	<.bat_x
		sta.h	<__temp			; wrap counter in __temp.h

		ldy	<.map_x

		stz	<vdc_reg, x		; select MAWR
		stz	VDC_AR, x

		lda.l	<.bat_line
		sta	VDC_DL, x
		lda.h	<.bat_line
		sta	VDC_DH, x

		lda	#VDC_VWR		; select VWR
		sta	<vdc_reg, x
		sta	VDC_AR, x
		bra	.draw_tile8

.next_tile8:	dec.h	<__temp			; wrap counter in __temp.h
		bne	!+			; no need to reset

		stz	<vdc_reg, x		; select MAWR
		stz	VDC_AR, x

		lda	vdc_bat_x_mask, x	; mask bat_line to start of line
		eor	#$FF
		and.l	<.bat_line
		sta	VDC_DL, x
		lda.h	<.bat_line
		sta	VDC_DH, x

		lda	#VDC_VWR		; select VWR
		sta	<vdc_reg, x
		sta	VDC_AR, x

!:		iny				; inc map_x
		tya
		cmp	vdc_map_width, x	; horizontal map wrapping
		bne	.draw_tile8
		cly

.draw_tile8:	phy				; Preserve map_x offset.

		clc
		lda	[.map_line], y		; get tile index from map data
		tay
		adc.l	vdc_tile_base, x	; calculate BAT value (tile + palette)
		sta	VDC_DL, x
		lda	[.attr_tbl], y		; get tile palette
		adc.h	vdc_tile_base, x
		sta	VDC_DH, x

		ply				; Restore map_x offset.

		dec.l	<__temp			; width counter in __temp.l
		bne	.next_tile8

		dec	<.tiles_h		; height counter
		bne	.next_line8

.draw_finished:	pla				; Restore MPR2, MPR3, MPR4.
		tam4
		pla
		tam3
		pla
		tam2

		plx				; Restore X (aka __sp).

		leave				; All done!

		; ******************************
		; Draw a bytemap of 16x16 tiles
		; ******************************

.next_line16:	sec				; 16x16 tiles
		jsr	.next_map_line

.draw_line16:	lda	<.tiles_w		; width counter in __temp.l
		sta.l	<__temp

		ldy	<.map_x			; map_x
		bra	.draw_tile16

.next_tile16:	lda.l	<.bat_line		; inc bat_line
		inc	a
		inc	a
		bit	vdc_bat_x_mask, x	; reached the next bat line?
		bne	!+
		sec				; go back one line
		sbc	vdc_bat_width, x
!:		sta.l	<.bat_line

		iny				; inc map_x
		tya
		cmp	vdc_map_width, x	; horizontal map wrapping
		bne	.draw_tile16
		cly

.draw_tile16:	phy				; Preserve map_x offset.

		lda	[.map_line], y		; get tile index from map data
		tay				; get tile palette from index
		lda	[.attr_tbl], y
		say				; put tile palette in Y
		asl	a			; tile# lo-byte
		bcc	!+
		iny				; tile# hi-byte
		iny
!:		asl	a			; tile# lo-byte
		bcc	!+
		iny				; tile# hi-byte
		clc
!:		adc.l	vdc_tile_base, x
		say				; tile# lo-byte in Y
		adc.h	vdc_tile_base, x
		sta.h	<__temp			; tile# hi-byte in __temp.h

		stz	<vdc_reg, x		; set MAWR to bat_line
		stz	VDC_AR, x
		lda.l	<.bat_line
		sta	VDC_DL, x
		lda.h	<.bat_line
		sta	VDC_DH, x
		lda	#VDC_VWR
		sta	<vdc_reg, x
		sta	VDC_AR, x

		tya				; tile# lo-byte
		sta	VDC_DL, x
		lda.h	<__temp			; tile# hi-byte
		sta	VDC_DH, x
		iny
		tya
		sta	VDC_DL, x
		lda.h	<__temp
		sta	VDC_DH, x
		iny

		stz	<vdc_reg, x		; set MAWR to bat_line + bat_width
		stz	VDC_AR, x
		lda.l	<.bat_line
		ora	vdc_bat_width, x
		sta	VDC_DL, x
		lda.h	<.bat_line
		sta	VDC_DH, x
		lda	#VDC_VWR
		sta	<vdc_reg, x
		sta	VDC_AR, x

		tya				; tile# lo-byte
		sta	VDC_DL, x
		lda.h	<__temp			; tile# hi-byte
		sta	VDC_DH, x
		iny
		tya
		sta	VDC_DL, x
		lda.h	<__temp
		sta	VDC_DH, x
;		iny

		ply				; Restore map_x offset.

		dec.l	<__temp			; width counter in __temp.l
		bne	.next_tile16

		dec	<.tiles_h		; height counter
		bne	.next_line16

		bra	.draw_finished

		; ******************************
		; Move to the next map/bat line
		; ******************************
		; CC if 8x8 tiles
		; CS if 16x16 tiles

.next_map_line:	lda	vdc_bat_x_mask, x
		eor	#$FF
		and.l	<.bat_line
		bcc	!+			; CC = 8x8, CS = 16x16
		ora	vdc_bat_width, x
		clc
!:		adc	vdc_bat_width, x	; Widths are a power-of-two
		bne	!+			; so just check for zero.

		lda.h	<.bat_line
		inc	a
		and	vdc_bat_limit, x
		sta.h	<.bat_line
		cla

!:		ora	<.bat_x
		sta.l	<.bat_line

		inc	<.map_y			; increment map_y
		lda	<.map_y
		cmp	vdc_map_height, x	; wrap map?
		beq	.wrap_to_top

.inc_map_lo:	lda	vdc_map_width, x	; Is map_width == 256?
		beq	.inc_map_hi
		clc
		adc.l	<.map_line
		sta.l	<.map_line
		bcc	!+
.inc_map_hi:	inc.h	<.map_line
		bpl	!+			; Still in MPR3?

		lda	#$60			; Normalize map_line back
		sta.h	<.map_line		; to MPR3.
		tma4
		tam3
		inc	a
		tam4
!:		rts

.wrap_to_top:	stz	<.map_y			; Reset map_y

		lda	vdc_map_bank, x		; Reset map bank.
		tam3
		inc	a
		tam4

		lda.l	vdc_map_addr, x		; Reset map_line
		sta.l	<.map_line
		lda.h	vdc_map_addr, x
		and	#$1F
		ora	#$60
		sta.h	<.map_line
		rts

		; ******************************
		; Calculate the bat tile pointer
		; ******************************

!calc_bat_tile:	lda	vdc_tile_type, x	; what tile type?
		cmp	#16

		ldy	<.bat_x
		lda	<.bat_y
		bcc	!+			; CC if 8x8 not 16x16

		say				; 16x16 aligned in the bat
		asl	a			; bat_x = bat_x * 2;
		say
		asl	a			; bat_y = bat_y * 2;

!:		and	vdc_bat_y_mask, x	; Calculate bat_addr
		sta	<.bat_y			; bat_y (masked)

		stz.l	<.bat_line		; A = bat_y, Y = bat_x
		bit	vdc_bat_width, x	; set N and V bits
		bmi	.width128
		bvs	.width64
.width32:	lsr	a			; bat_line = bat_y * 32
		ror.l	<.bat_line
.width64:	lsr	a			; bat_line = bat_y * 64
		ror.l	<.bat_line
.width128:	lsr	a			; bat_line = bat_y * 128
		ror.l	<.bat_line
		say
		and	vdc_bat_x_mask, x
		sta	<.bat_x			; bat_x (masked)

		ora.l	<.bat_line		; bat_line |= bat_x
		sta.l	<.bat_line
		sty.h	<.bat_line

		lda.l	vdc_attr_addr, x	; Put attr_ table in MPR2
		sta.l	<.attr_tbl
		lda.h	vdc_attr_addr, x
		and	#$1F
		ora	#$40
		sta.h	<.attr_tbl
		lda	vdc_attr_bank, x
		tam2

		rts

		; ******************************
		; Calculate the map line pointer
		; ******************************

!calc_map_line:	lda.l	vdc_map_addr, x		; Calculate map line address
		sta.l	<.map_line		; (excluding x coordinate)
		lda.h	vdc_map_addr, x
		and	#$1F			; Normalize map line hi-byte.
		sta.h	<.map_line
		clc
		adc	<.map_y			; map_y (if mapwidth == 256)

		ldy	vdc_map_width, x	; So, is mapwidth == 256?
		beq	.width_eq_256

.width_ne_256:	lda	<.map_y			; Calculate map_y * map_width
		lsr	a
		sta	<__temp
		cla
		ldy	#8
		bcc	.rotate
.add:		clc
		adc	vdc_map_width, x
.rotate:	ror	a
		ror	<__temp
		dey
		bcs	.add
		bne	.rotate
		tay
		lda.l	<__temp			; CC from final rotation.
		adc.l	<.map_line
		sta.l	<.map_line
		tya
		adc.h	<.map_line

.width_eq_256:	tay				; Preserve map_line hi-byte.
		and	#$1F
		ora	#$60
		sta.h	<.map_line		; Normalized map_line to MPR3.

		tya				; Restore map_line hi-byte.
		and	#$E0			; Get #banks in offset.
		rol	a
		rol	a
		rol	a
		rol	a
		adc	vdc_map_bank, x

		tam3				; Put map_line in MPR3 and MPR4.
		inc	a
		tam4

		rts

		.endp



; ***************************************************************************
; ***************************************************************************
;
; unsigned char __fastcall map_get_tile( unsigned char map_x<_bl>, unsigned char map_y<_bh> );
; unsigned char __fastcall sgx_map_get_tile( unsigned char map_x<_bl>, unsigned char map_y<_bh> );

	.if	SUPPORT_SGX

		.proc	_sgx_map_get_tile.2

		ldx	#SGX_VDC_OFFSET		; Offset to SGX VDC.
		db	$F0			; Turn "clx" into a "beq".

		.ref	_map_get_tile.2
		.endp

	.endif

		.proc	_map_get_tile.2

.map_x		=	_bl			; N.B. These MUST be the same
.map_y		=	_bh			; addresses used by _load_map.6
.map_line	=	_bp

		clx				; Offset to PCE VDC.

		tma3				; Preserve MPR3, MPR4.
		pha
		tma4
		pha

		jsr	!calc_map_line-		; Calculate map line address.

		ldy	<.map_x			; Read the tile from the map.
		lda	[.map_line], y
		tax				; Put return code in X.

		pla				; Restore MPR3, MPR4.
		tam4
		pla
		tam3

		cly				; Return code in Y:X, X -> A.

		leave				; Return and copy X -> A.

		.ref	_load_map.6
		.endp



; ***************************************************************************
; ***************************************************************************
;
; void __fastcall map_put_tile( unsigned char map_x<_bl>, unsigned char map_y<_bh>, unsigned char tile<_al> );
; void __fastcall sgx_map_put_tile( unsigned char map_x<_bl>, unsigned char map_y<_bh>, unsigned char tile<_al> );

	.if	SUPPORT_SGX

		.proc	_sgx_map_put_tile.3

		ldy	#SGX_VDC_OFFSET		; Offset to SGX VDC.
		db	$F0			; Turn "cly" into a "beq".

		.ref	_map_put_tile.3
		.endp

	.endif

		.proc	_map_put_tile.3

.map_x		=	_bl			; N.B. These MUST be the same
.map_y		=	_bh			; addresses used by _load_map.6
.map_line	=	_bp

		cly				; Offset to PCE VDC.

		phx				; Preserve X (aka __sp).
		sxy				; Put VDC offset in X.

		tma3				; Preserve MPR3, MPR4.
		pha
		tma4
		pha

		jsr	!calc_map_line-		; Calculate map line address.

		ldy	<.map_x			; Write the tile to the map.
		lda	<_al
		sta	[.map_line], y

		pla				; Restore MPR3, MPR4.
		tam4
		pla
		tam3

		plx				; Restore X (aka __sp).

		leave				; All done!

		.ref	_load_map.6
		.endp



; ***************************************************************************
; ***************************************************************************
;
; void __fastcall put_tile( unsigned char tile<_bl>, unsigned char bat_x<_al>, unsigned char bat_y<_ah> );
; void __fastcall sgx_put_tile( unsigned char tile<_bl>, unsigned char bat_x<_al>, unsigned char bat_y<_ah> );
;
; Transfer a single HuC map tile to the BAT in VRAM
;
; _al = BAT x screen coordinate (in tile units)
; _ah = BAT y screen coordinate
; _di = BAT address
; _bl = MAP tile number to write

	.if	SUPPORT_SGX

		.proc	_sgx_put_tile.3

		ldy	#SGX_VDC_OFFSET		; Offset to SGX VDC.
		db	$F0			; Turn "cly" into a "beq".

		.ref	_put_tile.3
		.endp

	.endif

		.proc	_put_tile.3

.bat_x		=	_al			; N.B. These MUST be the same
.bat_y		=	_ah			; addresses used by _load_map.6
.bat_tile	=	_bl
.bat_line	=	_di
.attr_tbl	=	_si

		cly				; Offset to PCE VDC.

		phx				; Preserve X (aka __sp).
		sxy				; Put VDC offset in X.

		tma2				; Preserve MPR2.
		pha

		jsr	!calc_bat_tile-		; Calculate bat tile address.

		stz	<vdc_reg, x		; set MAWR to bat_line
		stz	VDC_AR, x
		lda.l	<.bat_line
		sta	VDC_DL, x
		lda.h	<.bat_line
		sta	VDC_DH, x
		lda	#VDC_VWR
		sta	<vdc_reg, x
		sta	VDC_AR, x

		lda	vdc_tile_type, x
		cmp	#16
		bcs	.draw_tile16

.draw_tile8:	clc
		lda	<.bat_tile		; get tile index to write
		tay
		adc.l	vdc_tile_base, x	; calculate BAT value (tile + palette)
		sta	VDC_DL, x
		lda	[.attr_tbl], y		; get tile palette
		adc.h	vdc_tile_base, x
		sta	VDC_DH, x
		bra	.finished

.draw_tile16:	lda	<.bat_tile		; get tile index to write
		tay				; get tile palette from index
		lda	[.attr_tbl], y
		say				; put tile palette in Y
		asl	a			; tile# lo-byte
		bcc	!+
		iny				; tile# hi-byte
		iny
!:		asl	a			; tile# lo-byte
		bcc	!+
		iny				; tile# hi-byte
		clc
!:		adc.l	vdc_tile_base, x
		say				; tile# lo-byte in Y
		adc.h	vdc_tile_base, x
		sta.h	<__temp			; tile# hi-byte in __temp.h

		tya				; tile# lo-byte
		sta	VDC_DL, x
		lda.h	<__temp			; tile# hi-byte
		sta	VDC_DH, x
		iny
		tya
		sta	VDC_DL, x
		lda.h	<__temp
		sta	VDC_DH, x
		iny

		stz	<vdc_reg, x		; set MAWR to bat_line + bat_width
		stz	VDC_AR, x
		lda.l	<.bat_line
		ora	vdc_bat_width, x
		sta	VDC_DL, x
		lda.h	<.bat_line
		sta	VDC_DH, x
		lda	#VDC_VWR
		sta	<vdc_reg, x
		sta	VDC_AR, x

		tya				; tile# lo-byte
		sta	VDC_DL, x
		lda.h	<__temp			; tile# hi-byte
		sta	VDC_DH, x
		iny
		tya
		sta	VDC_DL, x
		lda.h	<__temp
		sta	VDC_DH, x
;		iny

.finished:	pla				; Restore MPR2.
		tam2

		plx				; Restore X (aka __sp).

		leave				; All done!

		.ref	_load_map.6
		.endp

		.endprocgroup			; huc_map_funcs
