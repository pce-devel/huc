;
; LIBRARY.ASM  -  MagicKit Standard Library
;
;

; IMPORTANT NOTE:
; ----
; almost all the library functions have been changed to automatically
; handle bank mapping (you don't have to map data yourself anymore),
; the change will be transparent to you if you were using only library
; macros to call functions, but you will have to update your code
; in case you were directly calling those functions!


; ----
; map_data
; ----
; map data in page 3-4 ($6000-$9FFF)
; ----
; IN :	_BL = data bank
;	_SI = data address
; ----
; OUT:	_BX = old banks
;	_SI = remapped data address
; ----

map_data:
	ldx	<__bl

	; ----
	; save current bank mapping
	;
	tma	#3
	sta	<__bl
	tma	#4
	sta	<__bh
	; --
	cpx	#$FE
	bne	.l1
	; --
	stx	<__bp
	rts

	; ----
	; map new bank
	;
.l1:	stz	<__bp
	; --
	txa
	tam	#3
	inc	A
	tam	#4

	; ----
	; remap data address to page 3
	;
	lda	<__si+1
	and	#$1F
	ora	#$60
	sta	<__si+1
	rts


; ----
; unmap_data
; ----
; IN :	_BX = old banks
; ----

unmap_data:

	lda	<__bl
	tam	#3
	lda	<__bh
	tam	#4
	rts

; ----
; remap_data
; ----

remap_data:
	lda	<__bp
	bne	.l1
	lda	<__si+1
	bpl	.l1
	sub	#$20
	sta	<__si+1
	tma	#4
	tam	#3
	inc	A
	tam	#4
.l1:
	rts


; ----
; load_palette
; ----
; initialize one or more sub-palette
; ----
; IN :	_AL = index of the first sub-palette (0-31)
;	_BL = data bank
;	_SI = address of data
;	_CL = number of sub-palette to copy
; ----

.ifdef HUC
_load_palette.3:
.endif
load_palette:
	maplibfunc	lib2_load_palette
	rts

xfer_palette:
	maplibfunc	lib2_xfer_palette
	rts

	.bank	LIB2_BANK
lib2_load_palette:
	ldy	color_queue_w

	lda	<__si + 0
	sta	color_addr_l,y
	lda	<__si + 1
	and	#$1F
	ora	#$60
	sta	color_addr_h,y
	lda	<__bl
	sta	color_bank,y
	lda	<__al
	sta	color_index,y
	lda	<__cl
	sta	color_count,y

	tya
	inc	a
	and	#7
.wait:	cmp	color_queue_r
	beq	.wait
	sta	color_queue_w
	rts

lib2_xfer_palette:
	ldy	color_queue_r
	cpy	color_queue_w
	beq	.done

	tma3
	pha
	tma4
	pha

	tii	.func, color_tia, 8

.next:	lda	color_index,y
	asl	a
	asl	a
	asl	a
	asl	a
	sta	color_reg_l
	cla
	rol	a
	sta	color_reg_h

	lda	color_bank,y
	tam3
	inc	a
	tam4
	lda	color_addr_l,y
	sta	color_tia + 1
	lda	color_addr_h,y
	sta	color_tia + 2

	ldx	color_count,y
.loop:	jsr	color_tia
	clc
	lda	#32
	adc	color_tia + 1
	sta	color_tia + 1
	bcc	.skip
	inc	color_tia + 2
.skip:	dex
	bne	.loop

	iny
	tya
	and	#7
	tay

	cpy	color_queue_w
	bne	.next
	sty	color_queue_r

	pla
	tam4
	pla
	tam3

.done:	rts

.func:	tia	0, color_data, 32
	rts

	.bank	LIB1_BANK

; ----
; load_bat
; ----
; transfer a BAT in VRAM
; ----
; IN :	_DI = VRAM base address
;	_BL = BAT bank
;	_SI = BAT memory location
;	_CL = nb of column to copy
;	_CH = nb of row
; ----

.ifdef HUC
_load_bat.4:
.endif
load_bat:
	maplibfunc	lib2_load_bat
	rts

	.bank	LIB2_BANK

lib2_load_bat:

	; ----
	; map data
	;
	jsr	map_data

	; ----
	; copy BAT
	;
	cly
	; --
.l1:	jsr	set_write
	ldx	<__cl
	; --
.l2:	lda	[__si],Y
	sta	video_data_l
	iny
	lda	[__si],Y
	sta	video_data_h
	iny
	bne	.l3
	inc	<__si+1
.l3:	dex
	bne	.l2
	; --
	jsr	remap_data
	; --
	addw	bat_width,<__di
	dec	<__ch
	bne	.l1

	; ----
	; unmap data
	;
	jmp	unmap_data

	.bank	LIB1_BANK

; ----
; load_map_8/16
; ----
; transfer a tiled map in VRAM
; ----
; IN :	_AL = x screen coordinate (tile unit)
;	_AH = y screen coordinate
;	_CL = x start coordinate in the map
;	_CH = y start coordinate
;	_DL = nb of column to copy
;	_DH = nb of row
; ----

	.bss
mapbank		.ds 1
mapaddr		.ds 2
mapwidth	.ds 2
mapheight	.ds 2
maptiletype	.ds 1
maptilebank	.ds 1
maptileaddr	.ds 2
maptilebase	.ds 2
mapnbtile	.ds 2
mapctablebank	.ds 1
mapctable	.ds 2
mapwrap		.ds 1
mapbat_ptr	.ds 2
mapbat_top_base .ds 2
mapbat_top	.ds 1
mapbat_bottom	.ds 1
mapbat_x	.ds 2
	.code
load_map:
	lda	maptiletype
	cmp	#8
	beq	.l1
	; --
	maplibfunc	lib2_load_map_16
	rts
.l1:	maplibfunc	lib2_load_map_8
	rts

	.bank	LIB2_BANK

lib2_load_map_16:

	; ----
	; save bank mapping
	;
	tma	#2
	pha
	tma	#3
	pha
	tma	#4
	pha

	; ----
	; init
	;
	jsr	load_map_init
	lda	<__ch
	sta	mapbat_x+1

	; ----
	; vertical loop
	;
.l1:	ldy	<__ah
	lda	<__dl
	sta	<__al
	lda	mapbat_x+1
	sta	mapbat_x
	bra	.l5

	; ----
	; horizontal loop
	;
.l2:	lda	mapbat_x	; bat wrapping
	add	#2
	and	bat_hmask
	sta	mapbat_x
	bne	.l3
	; --
	lda	bat_hmask
	eor	#$ff
	and	<__di
	sta	<__di
	bra	.l4
.l3:
	incw	<__di
	incw	<__di
.l4:
	iny
	; --
	cpy	mapwidth		; horizontal map wrapping
	bne	.l5
	cly
	ldx	mapwrap
	bne	.l5
	ldy	mapwidth
	lda	maptilebase
	sta	<__cl
	lda	maptilebase+1
	ora	[__bp]
	sta	<__ch
	dey
	bra	.l6
.l5:
	lda	[__si],Y		; get tile index
	tax			; calculate BAT value (tile + palette)
	sxy
	stz	<__ch
	asl	A
	rol	<__ch
	asl	A
	rol	<__ch
	add	maptilebase
	sta	<__cl
	lda	<__ch
	adc	maptilebase+1
	adc	[__bp],Y
	sta	<__ch
	sxy
.l6:
	vreg	#0		; copy tile
	stw	<__di,video_data
	vreg	#2
	stw	<__cx,video_data
	incw	<__cx
	stw	<__cx,video_data
	incw	<__cx
	vreg	#0
	addw	bat_width,<__di,video_data
	vreg	#2
	stw	<__cx,video_data
	incw	<__cx
	stw	<__cx,video_data

	dec	<__al		; next tile
	lbne	.l2

	; ----
	; next line
	;
	ldx	#2
	jsr	load_map_next_line
	dec	<__dh
	lbne	.l1

	; ----
	; restore bank mapping
	;
	jmp	load_map_exit

lib2_load_map_8:

	; ----
	; save bank mapping
	;
	tma	#2
	pha
	tma	#3
	pha
	tma	#4
	pha

	; ----
	; init
	;
	jsr	load_map_init
	bra	.l2

	; ----
	; vertical loop
	;
.l1:	ldx	#1
	jsr	load_map_next_line
	; --
.l2:	ldy	<__ah
	lda	<__dl
	sta	<__al
	lda	<__ch
	sta	<__cl
	vreg	#0		; set vram write ptr
	stw	<__di,video_data
	vreg	#2
	bra	.l5

	; ----
	; horizontal loop
	;
.l3:	lda	<__cl		; bat wrapping
	inc	A
	and	bat_hmask
	sta	<__cl
	bne	.l4
	; --
	vreg	#0
	lda	bat_hmask
	eor	#$ff
	and	<__di
	sta	video_data_l
	lda	<__di+1
	sta	video_data_h
	vreg	#2
.l4:
	iny			; next tile
	; --
	cpy	mapwidth		; map wrapping
	bne	.l5
	; --
	cly
	lda	mapwrap
	bne	.l5
	ldy	mapwidth
	dey
	cla
	bra	.l6
.l5:
	lda	[__si],Y		; get tile index
.l6:	tax			; calculate BAT value (tile + palette)
	sxy
	add	maptilebase
	sta	video_data_l
	lda	maptilebase+1
	adc	[__bp],Y
	sta	video_data_h
	sxy

	dec	<__al
	bne	.l3

	; ----
	; next line
	;
	dec	<__dh
	bne	.l1

	; ----
	; restore bank mapping
	;
load_map_exit:
	pla
	tam	#4
	pla
	tam	#3
	pla
	tam	#2
	rts


; ----
; load_map_init
; ----
; load_map sub routine
; ----
; OUT:	_DI = BAT address
;	_SI = map address
;	_BP = palette index table ptr
;	_AH = map X pos
;	_BH = map Y pos
;	_CH = BAT X pos
;	_BL = BAT Y pos
; ----

load_map_init:

	; ----
	; calculate vram address
	;
	ldx	<__al
	lda	<__ah
	ldy	maptiletype
	cpy	#8
	beq	.l1
	asl	A
	sax
	asl	A
	sax
.l1:	phx
	pha
	jsr	calc_vram_addr
	stw	<__di,mapbat_ptr

	; ----
	; calculate map address
	;
	stb	mapaddr,<__si
	lda	mapaddr+1
	and	#$1F
	sta	<__si+1
	; --
	ldx	<__cl
	stx	<__ah
	ldy	<__ch
	sty	<__bh
	; --
	lda	mapwidth+1
	beq	.l2
	tya
	add	<__si+1
	sta	<__si+1
	bra	.l3
	; --
.l2:	sty	<__al
	lda	mapwidth
	sta	<__bl
	jsr	mulu8
	addw	<__cx,<__si

	; ----
	; calculate map bank
	;
.l3:	rol	A
	rol	A
	rol	A
	rol	A
	and	#$0F
	add	mapbank

	; ----
	; map data
	;
	tam	#3
	inc	A
	tam	#4
	lda	mapctablebank
	tam	#2

	; ----
	; adjust data addresses
	;
	lda	<__si+1		; tile ptr
	and	#$1F
	ora	#$60
	sta	<__si+1
	; --
	stb	mapctable,<__bp	; color table ptr
	lda	mapctable+1
	and	#$1F
	ora	#$40
	sta	<__bp+1

	; ----
	; bat pos
	;
	pla
	and	bat_vmask
	sta	<__bl
	pla
	and	bat_hmask
	sta	<__ch
	rts


; ----
; load_map_next_line
; ----
; load_map sub routine
; ----
; IN :	X = BAT line inc value (1-2)
; ----
; OUT:	_DI = BAT address
;	_SI = map address
; ----
; USE:	_BL = BAT Y pos
;	_BH = map Y pos
;	_SI = map address
; ----

load_map_next_line:

	; ----
	; incremente vram address
	;
	txa
	add	<__bl
	cmp	mapbat_bottom
	blo	.l1
	; --
	sub	mapbat_bottom	; 1/ vram wrapping
	tax
	inx
	add	mapbat_top
	sta	<__bl
	lda	mapbat_ptr
	and	bat_hmask
	add	mapbat_top_base
	sta	mapbat_ptr
	cla
	adc	mapbat_top_base+1
	sta	mapbat_ptr+1
	bra	.l3
	; --
.l1:	sta	<__bl		; 2/ vram inc
.l2:	lda	bat_width
	add	mapbat_ptr
	sta	mapbat_ptr
	cla
	adc	mapbat_ptr+1
	sta	mapbat_ptr+1
	; --
.l3:	dex
	bne	.l2
	; --
	stw	mapbat_ptr,<__di

	; ----
	; increment map address
	;
	inc	<__bh
	lda	<__bh
	cmp	mapheight
	bne	.l4
	; --
	lda	mapbank		; 1/ map wrapping
	tam	#3
	inc	A
	tam	#4
	stb	mapaddr,<__si
	lda	mapaddr+1
	and	#$1F
	ora	#$60
	sta	<__si+1
	stz	<__bh
	bra	.l5
	; --
.l4:	addw	mapwidth,<__si	; 2/ map inc
	cmp	#$80
	blo	.l5
	sub	#$20
	sta	<__si+1
	tma	#4
	tam	#3
	inc	A
	tam	#4
.l5:
	rts


	.bank	LIB1_BANK

; ----
; load_font
; ----
; transfer a 8x8 monochrome font into VRAM, slow but can be useful
; ----
; IN :	_DI = VRAM base address
;	_BL = font bank
;	_SI = font memory location
;	_AL = font color (0-15)
;	_AH = bg color (0-15)
;	_CL = number of characters to copy
; ----

load_font:
	maplibfunc	lib2_load_font
	rts

	.bank	LIB2_BANK

	; ----
	; map data
	;
lib2_load_font:
	jsr	map_data
	jsr	set_write

	; ----
	; init bg color
	;
	lda	<__cl
	pha
	; --
	ldx	#3
.l1:	cla
	lsr	<__ah
	bcc	.l2
	lda	#$FF
.l2:	sta	<__cl,X
	dex
	bpl	.l1

	; ----
	; character loop
	;
	plx
.copy:

	; ----
	; plane 1
	;
	cly
.p1:	bbs0	<__al,.p2
	lda	[__si],Y
	eor	#$FF
	and	<__dh
	bra	.p3
	; --
.p2:	lda	<__dh
	ora	[__si],Y
.p3:	sta	video_data_l

	; ----
	; plane 2
	;
	bbs1	<__al,.p4
	lda	[__si],Y
	eor	#$FF
	and	<__dl
	bra	.p5
	; --
.p4:	lda	<__dl
	ora	[__si],Y
.p5:	sta	video_data_h
	; --
	iny
	cpy	#8
	bne	.p1

	; ----
	; plane 3
	;
	cly
.t1:	bbs2	<__al,.t2
	lda	[__si],Y
	eor	#$FF
	and	<__ch
	bra	.t3
	; --
.t2:	lda	<__ch
	ora	[__si],Y
.t3:	sta	video_data_l

	; ----
	; plane 4
	;
	bbs3	<__al,.t4
	lda	[__si],Y
	eor	#$FF
	and	<__cl
	bra	.t5
	; --
.t4:	lda	<__cl
	ora	[__si],Y
.t5:	sta	video_data_h
	; --
	iny
	cpy	#8
	bne	.t1

	; ----
	; next character
	;
	addw	#8,<__si
	; --
	dex
	bne	.copy

	; ----
	; unmap data
	;
	jmp	unmap_data

	; ----
	; restore bank mapping
	;
	.bank	LIB1_BANK


; ----
; load_vram
; ----
; copy a block of memory to VRAM
; ----
; IN :	_DI = VRAM location
;	_BL = data bank
;	_SI = data memory location
;	_CX = number of words to copy
; ----
	.bss

; This actually places a 'TIA' command
; into RAM, from which to execute
; for BLiT to VRAM for SATB transfer
; and other VRAM load functions

ram_hdwr_tia		.ds	1
ram_hdwr_tia_src	.ds	2
ram_hdwr_tia_dest	.ds	2
ram_hdwr_tia_size	.ds	2
ram_hdwr_tia_rts	.ds	1

	.code

.ifdef HUC
_load_vram.3:
_load_vram2.4:
.endif
load_vram:

	; ----
	; map data
	;
	jsr	map_data

	; ----
	; set vram address
	;
	jsr	set_write

	; ----
	; copy data
	;
	lda	#<video_data
	sta	ram_hdwr_tia_dest+0
	stz	ram_hdwr_tia_dest+1
	lda	#$20
	sta	ram_hdwr_tia_size+0
	stz	ram_hdwr_tia_size+1

	ldx	<__si+0
	stx	ram_hdwr_tia_src+0
	ldy	<__si+1
	sty	ram_hdwr_tia_src+1

	lda	<__cl			; length in chunks
	lsr	<__ch
	ror	a
	lsr	<__ch
	ror	a
	lsr	<__ch
	ror	a
	lsr	<__ch
	ror	a
	sax				; x=chunks-lo
	beq	.l4			; a=source-lo, y=source-hi

	; ----
	; copy data (32-byte chunks)
	;
.l1:	jsr	ram_hdwr_tia		; transfer 32-bytes

	clc				; increment source
	adc	#$20
	sta	ram_hdwr_tia_src+0
	bcc	.l3
	iny

	bpl	.l2			; remap_data
	tay
	tma4
	tam3
	inc	a
	tam4
	tya
	ldy	#$60
.l2:	sty	ram_hdwr_tia_src+1

.l3:	dex
	bne	.l1
.l4:	dec	<__ch
	bpl	.l1

	; ----
	; copy data (remainder)
	;
	lda	<__cl
	and	#15
	beq	.l5

	asl	a
	sta	ram_hdwr_tia_size+0
	jsr	ram_hdwr_tia		; transfer remainder

	; ----
	; unmap data
	;

.l5:	jmp	unmap_data


; ----
; set_read
; ----
; set the VDC VRAM read pointer
; ----
; IN :	_DI = VRAM location
; ----

set_read:
	vreg	#$01
	lda	<__di
	sta	video_data_l
	lda	<__di+1
	sta	video_data_h
	vreg	#$02
	rts


; ----
; set_write
; ----
; set the VDC VRAM write pointer
; ----
; IN :	_DI = VRAM location
; ----

set_write:
	vreg	#$00
	lda	<__di
	sta	video_data_l
	lda	<__di+1
	sta	video_data_h
	vreg	#$02
	rts


; ----
; calc_vram_addr
; ----
; calculate VRAM address
; ----
; IN :	X = x coordinates
;	A = y	"
; ----
; OUT:	_DI = VRAM location
; ----

calc_vram_addr:
	phx
	and	bat_vmask
	stz	<__di
	ldx	bat_width
	cpx	#64
	beq	.s64
	cpx	#128
	beq	.s128
	; --
.s32:	lsr	A
	ror	<__di
	; --
.s64:	lsr	A
	ror	<__di
	; --
.s128:	lsr	A
	ror	<__di
	sta	<__di+1
	; --
	pla
	and	bat_hmask
	ora	<__di
	sta	<__di
	rts

; ----
; HSR(xres)
; ----
; macros to calculate the value of the HSR VDC register
; ----
; IN :	xres, horizontal screen resolution
; ----

HSR	.macro
.if (\1 < 268)
	; low res
	.db $02
	.db (18 - (\1 / 16))
.else
.if (\1 < 356)
	; high res
	.db $03
	.db (25 - (\1 / 16))
.else
	; very high res
	.db $05
	.db (42 - (\1 / 16))
.endif
.endif
.endm


; ----
; HDR(xres)
; ----
; macros to calculate the value of the HDR VDC register
; ----
; IN :	xres, horizontal screen resolution
; ----

HDR	.macro
	.db ((\1 / 8) - 1)
.if (\1 < 268)
	; low res
	.db (38 - ((18 - (\1 / 16)) + (\1 / 8)))
.else
.if (\1 < 356)
	; high res
	.db (51 - ((25 - (\1 / 16)) + (\1 / 8)))
.else
	; high res
	.db (82 - ((42 - (\1 / 16)) + (\1 / 8)))
.endif
.endif
.endm


; ----
; init_vdc
; ----
; initialize the video controller
;  - 256x224 screen mode
;  - 64x32 virtual bgmap size
;  - display and sprites off
;  - interrupts disabled
;  - SATB at $7F00
;  - VRAM cleared
; ----

	.bss
bat_width	.ds 2
bat_height	.ds 1
bat_hmask	.ds 1
bat_vmask	.ds 1
scr_width	.ds 1
scr_height	.ds 1

	.code

init_vdc:
	maplibfunc lib2_init_vdc
	rts

	.bank	LIB2_BANK
lib2_init_vdc:
; ----
; default screen resolution
;
.ifndef xres
xres	.equ 256
.endif

	; ----
	; initialize the VDC registers
	;
	stw	#.table,<__si 	; register table address in 'si'
	cly
.l1:	lda	[__si],Y		; select the VDC register
	iny
	sta	<vdc_reg
	sta	video_reg
	lda	[__si],Y		; send the 16-bit data
	iny
	sta	video_data_l
	lda	[__si],Y
	iny
	sta	video_data_h
	cpy	#36		; loop if not at the end of the
	bne	.l1		; table

	; ----
	; set the screen mode
	;
.if (xres < 268)
	lda	#(0 | XRES_SOFT)
.else
.if (xres < 356)
	lda	#(1 | XRES_SOFT)
.else
	lda	#(2 | XRES_SOFT)
.endif
.endif

; This stuff alters display position from HuCard position

;	.if (CDROM)
;
;	ldx	#xres/8
;	ldy	#30
;	jsr	ex_scrmod
;	lda	#$01
;	jsr	ex_scrsiz
;	lda	#0
;	jsr	ex_imode
;
;	.else

	; pixel clock frequency
	sta	color_ctrl
;
;	.endif


	; ----
	; set the background & border colors to black
	;
	stw	#256,color_reg
	stwz	color_data
	stwz	color_reg
	stwz	color_data

.if (CDROM)

	; ----
	; reset scrolling position (0,0)
	;
	vreg	#7
	stwz	video_data
	vreg	#8
	stwz	video_data
	stwz	bg_x1
	stwz	bg_y1

	; ----
	; set SATB address
	;
	stw	#$7F00,satb_addr
	jsr	ex_sprdma
	lda	#$10
	jsr	ex_dmamod

.endif	; (CDROM)

	; ----
	; clear the video RAM
	;
	st0	#0
	st1	#0
	st2	#0
	st0	#2

	ldx	#128
.l2:	cly
.l3:	st1	#0
	st2	#0
	dey
	bne	.l3
	dex
	bne	.l2

	; ----
	; save screen infos
	;
	stw	#xres,scr_width	; resolution
	stw	#224,scr_height
	; --
	lda	#BGMAP_SIZE_64x32	; virtual size
	jmp	set_bat_size

	; ----
	; VDC register table
	;
.table:
.ifdef _SGX
_sgx_init_point:
.endif
	.db $05,$00,$00		; CR	control register
	.db $06,$00,$00		; RCR	scanline interrupt counter
	.db $07,$00,$00		; BXR	background horizontal scroll offset
	.db $08,$00,$00		; BYR        "     vertical     "      "
	.db $09,$10,$00		; MWR	size of the virtual screen
	.db $0A			; HSR +
	HSR xres		;     |			[$02,$02]
	.db $0B			; HDR | display size
	HDR xres		;     | and synchro	[$1F,$04]
	.db $0C,$02,$17		; VPR |
	.db $0D,$DF,$00		; VDW |
	.db $0E,$0C,$00		; VCR +
	.db $0F,$10,$00		; DCR	DMA control register
	.db $13,$00,$7F		; SATB	address of the SATB
	.bank	LIB1_BANK


; ----
; set_xres
; ----
; set horizontal display resolution
; ----
; IN :	_AX = new x resolution (ie. 320)
;	_CL = 'blur bit' for color register
; USES: _BX
; ----

set_xres:
	maplibfunc	lib2_set_xres
	rts


	.bank	LIB2_BANK

	.bss
vdc_blur	.ds 1	; blur bit
hsw		.ds 1	; temporary parameters for calculating video registers
hds		.ds 1
hdw		.ds 1
hde		.ds 1

	.code

lib2_set_xres:
	lda	#$20		; reset resource-usage flag
	tsb	<irq_m		; to skip joystick read portion of vsync
				; (temporarily disable VSYNC processing)
	lda	<__ah
	sta	<__bh
	lda	<__al
	sta	<__bl		; bx now has x-res

	lsr	<__bh
	ror	<__bl
	lsr	<__bh
	ror	<__bl
	lsr	<__bl		; bl now has x/8

	cly			; offset into numeric tables
				; 0=low-res, 1=mid-res, 2=high-res

	lda	<__ah
	beq	.xres_calc	; < 256
	cmp	#3
	bhs	.xres_calc

	cmpw	#$10C,<__ax
	blo	.xres_calc	; < 268

	iny
	cmpw	#$164,<__ax
	blo	.xres_calc	; < 356

	iny			; 356 < x < 512

.xres_calc:
	lda	.vce_tab,Y
	ora	<__cl
	sta	color_ctrl	; dot-clock (x-resolution)

	lda	.hsw_tab,Y	; example calc's (using "low-res" numbers)
	sta	hsw		; hsw = $2
	lda	<__bl
	sta	hds		; hds = (x/8) temporarily
	dec	A
	sta	hdw		; hdw = (x/8)-1
	lsr	hds		; hds = (x/16) temporarily

	lda	.hds_tab,Y
	sub	hds
	sta	hds		; hds = 18 - (x/16)

	lda	.hde_tab,Y
	sub	hds
	sub	<__bl		; hde = (38 - ( (18-(x/16)) + (x/8) ))
	sta	hde

.xres_putit:
	lda	#$0a
	sta	<vdc_reg
	sta	video_reg
	lda	hsw
	sta	video_data_l
	lda	hds
	sta	video_data_h

	lda	#$0b
	sta	<vdc_reg
	sta	video_reg
	lda	hdw
	sta	video_data_l
	lda	hde
	sta	video_data_h

.xres_err:
	lda	#$20
	trb	<irq_m		; re-enable VSYNC processing
	rts

.vce_tab:	.db	0, 1, 2
.hsw_tab:	.db	2, 3, 5
.hds_tab:	.db	18,25,42
.hde_tab:	.db	38,51,82

	.bank	LIB1_BANK	; restore bank context

; ----
; set_bat_size
; ----
; set bg map virtual size
; ----
; IN : A = new size (0-7)
; ----

set_bat_size:
	and	#$07
	pha
	; --
.if (CDROM)
	jsr	ex_scrsiz
	plx
.else
	vreg	#9
	pla
	tax
	asl	A
	asl	A
	asl	A
	asl	A
	sta	video_data_l
.endif
	; --
	lda	.width,X
	sta	bat_width
	stz	bat_width+1
	dec	A
	sta	bat_hmask
	; --
	lda	.height,X
	sta	bat_height
	sta	mapbat_bottom
	stz	mapbat_top
	stz	mapbat_top_base
	stz	mapbat_top_base+1
	dec	A
	sta	bat_vmask
	rts

.width:	.db $20,$40,$80,$80,$20,$40,$80,$80
.height: .db $20,$20,$20,$20,$40,$40,$40,$40


; ----
; init_psg
; ----
; initialize the sound generator.
; ----

init_psg:
	maplibfunc lib2_init_psg
	rts

	.bank	LIB2_BANK
lib2_init_psg:
	stz	psg_mainvol	; main volume to zero
	stz	psg_lfoctrl	; disable the LFO

	lda	#5		; set volume to zero for each channel
.clear:	sta	psg_ch		; and disable them
	stz	psg_ctrl
	stz	psg_pan
	dec	A
	bpl	.clear

	lda	#4		; disable noise for channel 5 & 6
	sta	psg_ch
	stz	psg_noise
	lda	#5
	sta	psg_ch
	stz	psg_noise
	rts
	.bank	LIB1_BANK


; ----------------------------------
; Some simple copy/compare functions
; ----------------------------------

; ----
; char * _strncpy(char *dest [__di], char *src [__si], unsigned char count [acc])
; char * _strncat(char *dest [__di], char *src [__si], unsigned char count [acc])
; ----
; Copy/Concatenate a string to another string
; ----

str_find_end:	pha
.loop:		lda	[__di]
		beq	.found
		inc	<__di
		bne	.loop
		inc	<__di+1
		bra	.loop
.found:		pla
		rts

_strncat.3:     bsr	str_find_end

_strncpy.3:	txa
		eor	#$ff
		tax

		cly
.loop:		inx
		beq	str_terminate
.copy:		lda	[__si],y
		beq	str_terminate
		sta	[__di],y
		iny
		bne	.loop
		bra	str_overflow

; ----
; char * _strcat(char *dest [__di], char *src [__si])
; char * _strcpy(char *dest [__di], char *src [__si])
; ----
; Copy/Concatenate a string to another string
; ----

_strcat.2:	bsr	str_find_end

_strcpy.2:	cly
.loop:		lda	[__si],y
		sta	[__di],y
		beq	memstr_finish
		iny
		bne	.loop
str_overflow:	dey
str_terminate:	cla
		sta	[__di],y
		bra	memstr_finish

; ----
; char * _memcpy(char *dest [__di], char *src [__si], int count [acc])
; ----
; Copy memory
; ----

_mempcpy.3:
_memcpy.3:	stx	<__temp
		cly
		tax
		beq	.done_pages
.copy_page:	lda	[__si],y
		sta	[__di],y
		iny
;		lda	[__si],y
;		sta	[__di],y
;		iny
		bne	.copy_page
		inc	<__si+1
		inc	<__di+1
		dex
		bne	.copy_page

;.done_pages:	lsr	<__temp
;		ldx	<__temp
;		beq	memstr_finish
;		bcs	.copy_1byte
;		dex
;.copy_2bytes:	lda	[__si],y
;		sta	[__di],y
;		iny
;.copy_1byte:	lda	[__si],y
;		sta	[__di],y
;		iny
;		dex
;		bpl	.copy_2bytes
;.done_bytes:	bra	memstr_finish

.done_pages:	ldx	<__temp
		beq	memstr_finish
.copy_byte:	lda	[__si],y
		sta	[__di],y
		iny
		dex
		bne	.copy_byte
.done_bytes: ;	bra	memstr_finish

		; !!! WARNING : non-standard return value !!!
		; it's actually a lot more useful to have these
		; return a ptr to the end of the strcpy/strcat.

memstr_finish:	tya
		clc
		adc	<__di
		tax
		lda	<__di+1
		bcc	.exit
		inc	a
.exit:		rts

; ----
; char * _memset(char *s [__di], int c [__bx], int n [acc])
; ----
; Set memory
; ----

_memset.3:	stx	<__temp
		cly
		tax
		beq	.done_pages
		lda	<__bx
.set_page:	sta	[__di],y
		iny
;		sta	[__di],y
;		iny
		bne	.set_page
		inc	<__si+1
		inc	<__di+1
		dex
		bne	.set_page

;.done_pages:	lsr	<__temp
;		ldx	<__temp
;		beq	memstr_finish
;		bcs	.set_1byte
;		dex
;.set_2bytes:	sta	[__di],y
;		iny
;.set_1byte:	sta	[__di],y
;		iny
;		dex
;		bpl	.set_2bytes
;.done_bytes:	bra	memstr_finish

.done_pages:	ldx	<__temp
		beq	memstr_finish
		lda	<__bx
.set_byte:	sta	[__di],y
		iny
		dex
		bne	.set_byte
.done_bytes:	bra	memstr_finish

; ----
; int _memcmp(char *dest [__di], char *src [__si], int count [acc])
; ----
; Compare memory
; ----

_memcmp.3:	eor	#$ff
		sta	<__temp
		txa
		eor	#$ff
		tax
		cly
.loop:		inx
		beq	.page
.test:		lda	[__di],y
		cmp	[__si],y
		bmi	cmp_minus
		bne	cmp_plus
		iny
		bne	.loop
		inc	<__si+1
		inc	<__di+1
		bra	.loop
.page:		inc	<__temp
		bne	.test
;		bra	cmp_same

cmp_same:	ldx	#$00
		cla
		rts

cmp_plus:	ldx	#$01
		cla
		rts

cmp_minus:	ldx	#$FF
		txa
		rts

; ----
; int _strcmp(char *dest [__di], char *src [__si])
; ----
; Compare strings
; ----

_strcmp.2:	cly
.loop:		lda	[__di],y
		cmp	[__si],y
		bmi	cmp_minus
		bne	cmp_plus
		cmp	#0
		beq	cmp_same
		iny
		bne	.loop
		bra	cmp_same

; ----
; int _strncmp(char *dest [__di], char *src [__si], unsigned char count [acc])
; ----
; Compare strings
; ----

_strncmp.3:	txa
		eor	#$ff
		tax
		cly
.loop:		inx
		beq	cmp_same
.test:		lda	[__di],y
		cmp	[__si],y
		bmi	cmp_minus
		bne	cmp_plus
		cmp	#0
		beq	cmp_same
		iny
		bne	.loop
		bra	cmp_same

; ----
; unsigned char _strlen(char *src [__si])
; ----
; Strings length
; ----

_strlen.1:	cly
.loop:		lda	[__si],y
		beq	.done
		iny
		bne	.loop
.done:		sxy
		cla
		rts

; ----

___builtin_ffs.1:
	maplibfunc lib2____builtin_ffs.1
	rts

	.bank LIB2_BANK
lib2____builtin_ffs.1:
	sax
	ldy #-8
.search_lo:	ror a
	bcs .found_in_lo
	iny
	bne .search_lo
	txa
	ldy #-8
.search_hi:	ror a
	bcs .found_in_hi
	iny
	bne .search_hi
	clx		; no bits set, return 0 (A is already 0)
	rts
.found_in_hi:tya		; found bit in the high byte
	adc #16		; carry is set
	tax
	cla		; return 17 + y
	rts
.found_in_lo:tya		; found bit in the low byte
	adc #8		; carry is set
	tax
	cla		; return 9 + y
	rts
	.bank LIB1_BANK

; TurboXray (7/18/21): Wth is this?? Banks and pages are two different things. Why are they mixed like this?! This is part of SimpleTracker. Removing this..
; _mem_mapdatabanks:
; 	tay		; y = new upper bank
; 	tma #DATA_BANK+1; a = old upper bank
; 	say		; y = old upper bank, a = new upper bank
; 	tam #DATA_BANK+1
; do_mapdatabank:
; 	tma #DATA_BANK	; a = old lower bank
; 	sax		; x = old lower bank, a = new lower bank
; 	tam #DATA_BANK
; 	tya		; a = old upper bank
; 	rts
; _mem_mapdatabank:
; 	cly
; 	bra do_mapdatabank

_timer_set:
	stx	timer_cnt
	rts
_timer_start:
	lda	#1
	sta	timer_ctrl
	rts
_timer_stop:
	stz	timer_ctrl
	rts
_timer_get:
	lda	timer_cnt
	and	#$7f
	tax
	cla
	rts
_irq_disable:
	txa
	sei
	ora	irq_disable
	sta	irq_disable
	cli
	rts
_irq_enable:
	txa
	eor	#$ff
	sei
	and	irq_disable
	sta	irq_disable
	cli
	rts

_abort:
	.db 0xe2

_exit:
	.db 0x63

_dump_screen:
	.db 0x33

	.include "libslot1.asm"
	.include "libslot2.asm"
	.include "libslot3.asm"