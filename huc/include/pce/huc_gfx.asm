;
; HUC_GFX.ASM  -  HuC Graphic Library
;

; ----
; local variables

	.zp
spr_ptr		.ds 2
spr_max		.ds 1
spr_flag	.ds 1

	.bss
_font_base	.ds 2
font_color	.ds 2
satb		.ds 512	; the local SATB

gfx_pal		.ds 1

line_currx	.ds 2
line_curry	.ds 2
line_deltax	.ds 2
line_deltay	.ds 2
line_error	.ds 2
line_adjust	.ds 2
line_xdir	.ds 1
line_color	.ds 1


; ----
; library code

	 .code

; cls(int val [__dx])
; ----

_cls:
	stw	_font_base,<__dx
_cls.1:
	setvwaddr $0
	; --
	ldy	bat_height
.l2:	ldx	bat_width
	; --
.l3:	stw	<__dx,video_data
	dex
	bne	.l3
	dey
	bne	.l2
	rts

; set_font_pal(int pal)
; ----

_set_font_pal:
	txa
	asl	A
	asl	A
	asl	A
	asl	A
	sta	<__temp
	lda	_font_base+1
	and	#$0F
	ora	<__temp
	sta	_font_base+1
	rts

; set_font_color(char color, char bg)
; ----

_set_font_color.2:
	txa
	and	#$F
	sta	font_color+1
	lda	<__al
	and	#$F
	sta	font_color
	rts

; set_font_addr(int addr)
; ----

_set_font_addr:
	; --
	stx	_font_base
	lsr	A
	ror	_font_base
	lsr	A
	ror	_font_base
	lsr	A
	ror	_font_base
	lsr	A
	ror	_font_base
	sta	<__al
	; --
	lda	_font_base+1
	and	#$F0
	ora	<__al
	sta	_font_base+1
	rts

; get_font_pal()
; ----

_get_font_pal:
	lda	_font_base+1
	lsr	A
	lsr	A
	lsr	A
	lsr	A
	clx
	sax
	rts

; get_font_addr()
; ----

_get_font_addr:
	; --
	lda	_font_base+1
	sta	<__al
	lda	_font_base
	asl	A
	rol	<__al
	asl	A
	rol	<__al
	asl	A
	rol	<__al
	asl	A
	rol	<__al
	ldx	<__al
	sax
	rts

; load_default_font(char num [__dl], int addr [__di])
; ----

_load_default_font:
	; --
	stz	<__dl

_load_default_font.1:
	; --
	ldx	#$FF
	lda	#$FF
	jsr	calc_vram_addr
	incw	<__di

_load_default_font.2:
	; --
	lda	<__di
	ora	<__di+1
	bne	.l1
	jsr	_get_font_addr
	__stw	<__di
	bra	.l2
	; --
.l1:	__ldw	<__di
	jsr	_set_font_addr
	; --
.l2:	stb	#FONT_BANK+_bank_base,<__bl
	stb	#96,<__cl
	stb	font_color+1,<__ah
	lda	font_color
	bne	.l3
	inc	A
.l3:	sta	<__al
	lda	<__dl
	and	#$03
	asl	A
	tax
	lda	font_table,X
	sta	<__si
	inx
	lda	font_table,X
	sta	<__si+1
	jmp	load_font

; load_font(farptr font [__bl:__si], char nb [__cl], int addr [__di])
; ----

_load_font.2:
	; --
	ldx	#$FF
	lda	#$FF
	jsr	calc_vram_addr
	incw	<__di

_load_font.3:
	; --
	lda	<__di
	ora	<__di+1
	bne	.l1
	jsr	_get_font_addr
	__stw	<__di
	bra	.l2
	; --
.l1:	__ldw	<__di
	jsr	_set_font_addr
	; --
.l2:	lda	<__cl
	stz	<__ch
	asl	A
	rol	<__ch
	asl	A
	rol	<__ch
	asl	A
	rol	<__ch
	asl	A
	rol	<__ch
	sta	<__cl
	jmp	load_vram

; put_digit(char digit, int offset)
; put_digit(char digit, char x, char y)
; ----

_put_digit.3:
	lda	<__cl
	jsr	_put.xy
	bra	_put_digit.main
_put_digit.2:
	jsr	_put.vram
_put_digit.main:
	lda	<__dl
_put_digit.sub:
	cmp	#10
	blo	.l1
	add	#$07
.l1:	adc	#$10
	adc	_font_base
	sta	video_data_l
	cla
	adc	_font_base+1
	sta	video_data_h
	rts
_put.xy:
	sax
	jsr	calc_vram_addr
	jmp	set_write
_put.vram:
	stz	<vdc_reg
	stz	video_reg
	stx	video_data_l
	sta	video_data_h
	vreg	#$02
	rts

; put_char(char character, int offset)
; put_char(char character, char x, char y)
; ----

_put_char.3:
	lda	<__cl
	jsr	_put.xy
	bra	_put_char.main
_put_char.2:
	jsr	_put.vram
_put_char.main:
	lda	<__dl
	; --
	cmp	#32
	bhs	.l1
	lda	#32
	sec
.l1:	sbc	#32
	add	_font_base
	sta	video_data_l
	cla
	adc	_font_base+1
	sta	video_data_h
	rts

; put_raw(int character, int offset)
; put_raw(int character, char x, char y)
; ----

_put_raw.3:
	lda	<__cl
	jsr	_put.xy
	bra	_put_raw.main
_put_raw.2:
	jsr	_put.vram
_put_raw.main:
	lda	<__dl
	sta	video_data_l
	lda	<__dh
	sta	video_data_h
	rts

; put_number(int number, char n, int offset)
; put_number(int number, char n, char x, char y)
; ----


_put_number.3:	maplibfunc	lib2_put_number.3
		rts

_put_number.4:	maplibfunc	lib2_put_number.4
		rts


; put_hex(int number, char n, int offset)
; put_hex(int number, char n, char x, char y)
; ----

_put_hex.4:
	lda	<__bl
	jsr	_put.xy
	bra	_put_hex.main
_put_hex.3:
	jsr	_put.vram
_put_hex.main:
	ldx	<__cl
	beq	.l3
.l1:	cpx	#5
	blo	.l2
	cla
	jsr	_put_digit.sub
	dex
	bra	.l1
	; --
.l2:	txa
	dec	A
	asl	A
	tax
	jmp	[.tbl,X]
.l3:	rts
	; --
.tbl:	.dw	.h1,.h2,.h3,.h4
	; --
.h4:	lda	<__dh
	lsr	A
	lsr	A
	lsr	A
	lsr	A
	jsr	_put_digit.sub
	; --
.h3:	lda	<__dh
	and	#$0F
	jsr	_put_digit.sub
	; --
.h2:	lda	<__dl
	lsr	A
	lsr	A
	lsr	A
	lsr	A
	jsr	_put_digit.sub
	; --
.h1:	lda	<__dl
	and	#$0F
	jmp	_put_digit.sub

; put_string(char *string, int offset)
; put_string(char *string, char x, char y)
; ----

_put_string.3:
	lda	<__bl
	jsr	_put.xy
	bra	_put_string.main
_put_string.2:
	jsr	_put.vram
_put_string.main:
	bra	.l3
	; --
.l1:	cmp	#32
	bhs	.l2
	lda	#32
	sec
.l2:	sbc	#32
	add	_font_base
	sta	video_data_l
	cla
	adc	_font_base+1
	sta	video_data_h
	incw	<__si
.l3:	lda	[__si]
	bne	.l1
	rts

; vsync()
; vsync(char nb_frame)
; ----

_vsync:
	cla
	jmp	wait_vsync

_vsync.1:
	txa
	jmp	wait_vsync

; vreg(char reg)
; vreg(char reg, int data)
; ----

_vreg.1:
	stx	<vdc_reg
	stx	video_reg
	rts

_vreg.2:
	ldy	<__al
	sty	<vdc_reg
	sty	video_reg
	stx	video_data
	sta	video_data+1
	rts

; vram_addr(char x [__al], char y)
; ----

_vram_addr.2:
	lda	<__al
	sax
	jsr	calc_vram_addr
	__ldw	<__di
	rts

; scan_map_table(int *tbl [__si], int *x [__ax], int *y [__cx])
; ----
; tbl,
; x,
; y,
; ----

_scan_map_table.3:

	ldy	#1
	lda	[__ax]
	sta	<__bl
	lda	[__ax],Y
	sta	<__bh
	lda	[__cx]
	sta	<__dl
	lda	[__cx],Y
	sta	<__dh
	; --
	addw	#4,<__si

	; ----
	; check bounds
	;
	; -- bottom
.l1:	ldy	#7
	lda	[__si],Y
	cmp	<__dh
	blo	.x1
	bne	.l2
	dey
	lda	[__si],Y
	cmp	<__dl
	blo	.x1
	; -- top
.l2:	ldy	#3
	lda	<__dh
	cmp	[__si],Y
	blo	.x1
	bne	.l3
	dey
	lda	<__dl
	cmp	[__si],Y
	blo	.x1
	; -- right
.l3:	ldy	#5
	lda	[__si],Y
	cmp	<__bh
	blo	.x1
	bne	.l4
	dey
	lda	[__si],Y
	cmp	<__bl
	blo	.x1
	; -- left
.l4:	ldy	#1
	lda	<__bh
	cmp	[__si],Y
	blo	.x1
	bne	.x2
	lda	<__bl
	cmp	[__si]
	bhs	.x2

	; ----
	; next
	;
.x1:	addw	#12,<__si
	ldy	#1
	lda	[__si]
	and	[__si],Y
	cmp	#$FF
	bne	.l1

	; ----
	; didn't find map...
	;
	clx
	cla
	rts

	; ----
	; found map!
	;
.x2:	ldy	#1
	lda	<__bl
	sub	[__si]
	sta	[__ax]
	lda	<__bh
	sbc	[__si],Y
	sta	[__ax],Y
	; --
	iny
	lda	<__dl
	sub	[__si],Y
	sta	[__cx]
	iny
	lda	<__dh
	sbc	[__si],Y
	ldy	#1
	sta	[__cx],Y
	; --
	__ldw	<__si
	rts

; set_map_data(int *ptr)
; set_map_data(char *map [__bl:__si], int w [__ax], int h)
; set_map_data(char *map [__bl:__si], int w [__ax], int h [__dx], char wrap)
; ----
; map,	map base address
; w,	map width
; h,	map height
; wrap, wrap flag (1 = wrap, 0 = do not wrap)
; ----

_set_map_data.1:
	__stw	<__si
	ora	<__si
	beq	.l1
	; -- calculate width
	lda	[__si].4
	sub	[__si]
	sta	mapwidth
	lda	[__si].5
	sbc	[__si].1
	sta	mapwidth+1
	incw	mapwidth
	; -- calculate height
	lda	[__si].6
	sub	[__si].2
	sta	mapheight
	lda	[__si].7
	sbc	[__si].3
	sta	mapheight+1
	incw	mapheight
	; -- get map bank
	lda	[__si].8
	sta	mapbank
	; -- get map addr
	lda	[__si].10
	sta	mapaddr
	iny
	lda	[__si]
	sta	mapaddr+1
	; -- no wrap
	stz	mapwrap
	rts
	; -- null pointer
.l1:	stwz	mapwidth
	stwz	mapheight
	stz	mapbank
	stwz	mapaddr
	stz	mapwrap
	rts
_set_map_data.4:
	stx	mapwrap
	__ldw	<__dx
	bra	_set_map_data.main
_set_map_data.3:
	stz	mapwrap
	inc	mapwrap
_set_map_data.main:
	__stw	mapheight
	stw	<__ax,mapwidth
	stb	<__bl,mapbank
	stw	<__si,mapaddr
	rts

; get_map_width()
; ----

_get_map_width:
	__ldw	mapwidth
	rts

; get_map_height()
; ----

_get_map_height:
	__ldw	mapheight
	rts

; set_tile_data(char *tile_ex [__di])
; set_tile_data(char *tile [__bl:__si], int nb_tile [__cx], char *ptable [__al:__dx], char type [__ah])
; ----
; tile,	tile base index
; nb_tile, number of tile
; ptable,	tile palette table address
; type, tile type (8 or 16)
; ----

_set_tile_data.1:
	cly
	lda	[__di],Y++
	sta	mapnbtile
	lda	[__di],Y++
	sta	mapnbtile+1
	lda	[__di],Y++
	sta	maptiletype
	iny
	lda	[__di],Y++
	sta	maptilebank
	iny
	lda	[__di],Y++
	sta	maptileaddr
	lda	[__di],Y++
	sta	maptileaddr+1
	lda	#(CONST_BANK+_bank_base)
	sta	mapctablebank
	lda	[__di],Y++
	sta	mapctable
	lda	[__di],Y
	sta	mapctable+1
	rts
_set_tile_data.4:
	stb	<__bl,maptilebank
	stw	<__si,maptileaddr
	stw	<__cx,mapnbtile
	stb	<__al,mapctablebank
	stw	<__dx,mapctable
	stb	<__ah,maptiletype
	rts

; load_tile(int addr)
; ----

_load_tile:
	__stw	<__di
	stx	<__al
	lsr	A
	ror	<__al
	lsr	A
	ror	<__al
	lsr	A
	ror	<__al
	lsr	A
	ror	<__al
	sta		maptilebase+1
	stb	<__al,maptilebase
	; --
	stw	mapnbtile,<__cx
	ldx	#4
	lda	maptiletype
	cmp	#8
	beq	.l1
	ldx	#6
.l1:	asl	<__cl
	rol	<__ch
	dex
	bne	.l1
	; --
	stb	maptilebank,<__bl
	stw	maptileaddr,<__si
	jmp	load_vram

; load_map(char x [__al], char y [__ah], int mx, int my, char w [__dl], char h [__dh])
; ----

_load_map.6:

	tstw	mapwidth
	beq	.l6
	tstw	mapheight
	beq	.l6

	; ----
	; adjust map y coordinate
	;
	lda	<__bh
	bmi	.l2
.l1:	cmpw	mapheight,<__bx
	blo	.l3
	subw	mapheight,<__bx
	bra	.l1
	; --
.l2:	lda	<__bh
	bpl	.l3
	addw	mapheight,<__bx
	bra	.l2

	; ----
	; adjust map x coordinate
	;
.l3:	stb	<__bl,<__ch
	lda	<__di+1
	bmi	.l5
.l4:	cmpw	mapwidth,<__di
	blo	.l7
	subw	mapwidth,<__di
	bra	.l4
	; --
.l5:	lda	<__di+1
	bpl	.l7
	addw	mapwidth,<__di
	bra	.l5

	; ----
	; exit
	;
.l6:	rts

	; ----
	; ok
	;
.l7:	stb	<__di,<__cl
	jmp	load_map

; spr_set(char num)
; ----
; load SI with the offset of the sprite to change
; SI = satb + 8 * sprite_number
; ----

_spr_set:
	cpx	#64
	bhs	.l2
	txa
	inx
	cpx	<spr_max
	blo	.l1
	stx	<spr_max
	; --
.l1:	stz	<spr_ptr+1
	asl	A
	asl	A
	asl	A
	rol	<spr_ptr+1
	adc	#low(satb)
	sta	<spr_ptr
	lda	<spr_ptr+1
	adc	#high(satb)
	sta	<spr_ptr+1
.l2:	rts

; spr_hide(char num)
; ----

	; -- hide current sprite
_spr_hide:
	ldy	#1
	lda	[spr_ptr],Y
	ora	#$02
	sta	[spr_ptr],Y
	rts

_spr_hide.1:
	; -- hide sprite number #
	cpx	#64
	bhs	.l1
	jsr	_spr_hide.sub
	lda	[__ptr],Y
	ora	#$02
	sta	[__ptr],Y
.l1:	rts

	; -- calc satb ptr
_spr_hide.sub:
	txa
	stz	<__ptr+1
	asl	A
	asl	A
	asl	A
	rol	<__ptr+1
	adc	#low(satb)
	sta	<__ptr
	lda	<__ptr+1
	adc	#high(satb)
	sta	<__ptr+1
	ldy	#1
	rts

; spr_show(char num)
; ----

	; -- hide current sprite
_spr_show:
	ldy	#1
	lda	[spr_ptr],Y
	and	#$01
	sta	[spr_ptr],Y
	rts

_spr_show.1:
	; -- hide sprite number #
	cpx	#64
	bhs	.l1
	jsr	_spr_hide.sub
	lda	[__ptr],Y
	and	#$01
	sta	[__ptr],Y
.l1:	rts

; spr_x(int value)
; ----

_spr_x:
	ldy	#2
	sax
	add	#32
	sta	[spr_ptr],Y
	sax
	adc	#0
	iny
	sta	[spr_ptr],Y
	rts

_spr_get_x:
	ldy	#2
	lda	[spr_ptr],Y
	sub	#32
	tax
	iny
	lda	[spr_ptr],Y
	sbc	#0
	rts

; spr_y(int value)
; ----

_spr_y:
	sax
	add	#64
	sta	[spr_ptr]
	sax
	adc	#0
	and	#$01
	ldy	#1
	sta	[spr_ptr],Y
	rts

_spr_get_y:
	lda	[spr_ptr]
	sub	#64
	tax
	ldy	#1
	lda	[spr_ptr],Y
	sbc	#0
	rts

; spr_pattern(int vaddr)
; ----

_spr_pattern:
	sta	<__temp
	txa
	asl	A
	rol	<__temp
	rol	A
	rol	<__temp
	rol	A
	rol	<__temp
	rol	A
	and	#$7
	ldy	#5
	sta	[spr_ptr],Y
	lda	<__temp
	dey
	sta	[spr_ptr],Y
	rts

_spr_get_pattern:
	ldy	#4
	lda	[spr_ptr],Y
	sta	<__temp
	iny
	lda	[spr_ptr],Y
	lsr	A
	ror	<__temp
	ror	A
	ror	<__temp
	ror	A
	ror	<__temp
	ror	A
	and	#$E0
	tax
	lda	<__temp
	rts

; spr_ctrl(char mask [__al], char value)
; ----

_spr_ctrl.2:
	txa
	and	<__al
	sta	<__temp
	lda	<__al
	eor	#$FF
	ldy	#7
	and	[spr_ptr],Y
	ora	<__temp
	sta	[spr_ptr],Y
	rts

; spr_pal(char pal)
; ----

_spr_pal:
	txa
	and	#$0F
	sta	<__temp
	ldy	#6
	lda	[spr_ptr],Y
	and	#$F0
	ora	<__temp
	sta	[spr_ptr],Y
	rts

_spr_get_pal:
	ldy	#6
	lda	[spr_ptr],Y
	and	#$0F
	tax
	cla
	rts

; spr_pri(char pri)
; ----

_spr_pri:
	ldy	#6
	lda	[spr_ptr],Y
	and	#$7F
	cpx	#$00
	beq	.l1
	ora	#$80
.l1:
	sta	[spr_ptr],Y
	rts

; satb_update()
; satb_update(char max)
; ----

_satb_update:
	ldx	<spr_max
	bra	satb_update

_satb_update.1:
	lda	<spr_flag
	beq	satb_update
	stz	<spr_flag
	ldx	#64

satb_update:
	cpx	#0
	beq	.l4
	; --
	stx	<__al	; number of sprites
	txa
	dec	A	; round up to the next group of 4 sprites
	lsr	A
	lsr	A
	inc	A
	sta	<__cl

; Use TIA, but BLiT 16 words at a time (32 bytes)
; Because interrupt must not deferred too much
;
	stw	#32, ram_hdwr_tia_size
	stw	#video_data, ram_hdwr_tia_dest
	stw	#satb, <__si

	stw	#$7F00, <__di
	jsr	set_write

.l3a:	stw	<__si, ram_hdwr_tia_src
	jsr	ram_hdwr_tia
	addw	#32,<__si
	dec	<__cl
	bne	.l3a

;.l3:	stx	<__al
;	stw	#satb,<__si
;	stb	#BANK(satb),<__bl
;	stw	#$7F00,<__di
;	txa
;	stz	<__ch
;	asl	A
;	asl	A
;	rol	<__ch
;	sta	<__cl
;	jsr	load_vram

	; --
	ldx	<__al
.l4:	cla
	rts

; init_satb()
; reset_satb()
; ----

_reset_satb:
_init_satb:
	clx
	cla
.l1:	stz	satb,X
	stz	satb+256,X
	inx
	bne	.l1
	; --
	ldy	#1
	sty	<spr_flag
	stz	<spr_max
	rts

; get_color(int index [color_reg])
; ----
; index: index in the palette (0-511)
; ----

_get_color.1:
	ldx	color_data_l
	lda	color_data_h
	and	#$01
	rts

; set_color(int index [color_reg], int color [color_data])
; ----
; set one palette entry to the specified color
; ----
; index: index in the palette (0-511)
; color: color value,	GREEN:	bit 6-8
;			RED:	bit 3-5
;			BLUE:	bit 0-2
; ----
; NOTE : inlined
; ----

; fade_color(int color [__ax], char level)
; fade_color(int index [color_reg], int color [__ax], char level)
; ----
; set one palette entry to the specified color
; ----
; index: index in the palette (0-511)
; color: color value,	GREEN:	bit 6-8
;			RED:	bit 3-5
;			BLUE:	bit 0-2
; level: level of fading (0 = black, 8 = full)
; ----

_fade_color.2:
_fade_color.3:
	cpx	#0
	beq	.l4
	cpx	#8
	bhs	.l5
	; -- fading
	ldy	#3
	stx	<__bl
	stwz	<__dx
.l1:	lsr	<__bl
	bcc	.l2
	addw	<__ax,<__dx
.l2:	aslw	<__ax
	dey
	bne	.l1
	lda	<__dh
	lsr	A
	ror	<__dl
	lsr	A
	ror	<__dl
	lsr	A
	ror	<__dl
	; -- set color
	ldx	<__dl
.l3:	stx	color_data_l
	sta	color_data_h
	rts
	; -- black
.l4:	cla
	bra	.l3
	; -- full
.l5:	ldx	<__al
	lda	<__ah
	bra	.l3

; set_color_rgb(int index [color_reg], char r [__al], char g [__ah], char b)
; ----
; set one palette entry to the specified color
; ----
; index: index in the palette (0-511)
; r:	red	RED:	bit 3-5
; g:	green	GREEN:	bit 6-8
; b:	blue	BLUE:	bit 0-2
; ----

_set_color_rgb.4:
	txa
	and	#$7
	sta	<__temp
	lda	<__al
	asl	A
	asl	A
	asl	A
	ora	<__temp
	asl	A
	asl	A
	sta	<__temp
	lda	<__ah
	lsr	A
	ror	<__temp
	lsr	A
	ror	<__temp
	ldx	<__temp
	stx	color_data_l
	sta	color_data_h
	rts

; put_tile(int tile_num [__dx], int position)
; put_tile(int tile_num [__dx], char x [__al], char y)
; ----
; draw a single 8x8 or 16x16 tile at a given position
; ----
; pattern:	vram address of the tile pattern
; position:	position on screen where to put the tile
; ----

_put_tile.3:
	lda	<__al
	ldy	maptiletype
	cpy	#8
	beq	.l1
	; --
	asl	A
	sax
	asl	A
	jsr	calc_vram_addr
	bra	_put_tile_16
	; --
.l1:	sax
	jsr	calc_vram_addr
	bra	_put_tile_8
_put_tile.2:
	__stw	<__di
	ldy	maptiletype
	cpy	#8
	bne	_put_tile_16
_put_tile_8:
	jsr	set_write
	; -- calculate tile vram address
	tma	#2
	pha
	lda	mapctablebank
	tam	#2
	stb	mapctable,<__bx
	lda	mapctable+1
	and	#$1F
	ora	#$40
	sta	<__bx+1
	lda	<__dl
	tay
	add	maptilebase
	tax
	cla
	adc	maptilebase+1
	adc	[__bx],Y
	; -- copy tile
	stx	video_data_l
	sta	video_data_h
	pla
	tam	#2
	rts
_put_tile_16:
	jsr	set_write
	; -- calculate tile vram address
	tma	#2
	pha
	lda	mapctablebank
	tam	#2
	stb	mapctable,<__bx
	lda	mapctable+1
	and	#$1F
	ora	#$40
	sta	<__bx+1
	stz	<__dh
	lda	<__dl
	tay
	asl	A
	rol	<__dh
	asl	A
	rol	<__dh
	add	maptilebase
	sta	<__dl
	lda	<__dh
	adc	maptilebase+1
	adc	[__bx],Y
	sta	<__dh
	; -- copy tile
	stw	<__dx,video_data
	incw	<__dx
	stw	<__dx,video_data
	incw	<__dx
	vreg	#0
	addw	bat_width,<__di,video_data
	vreg	#2
	stw	<__dx,video_data
	incw	<__dx
	stw	<__dx,video_data
	pla
	tam	#2
	rts

; map_get_tile(char x [__dl], char y)
; map_put_tile(char x [__dl], char y [__dh], char tile)
; ----

_map_get_tile.2:
	stx	<__dh
	jsr	_map_calc_tile_addr
	; --
	lda	[__cx]
	tax
	cla
	rts

_map_put_tile.3:
	phx
	jsr	_map_calc_tile_addr
	pla
	sta	[__cx]
	rts

; map_calc_tile_addr(char x [__dl], char y [__dh])
; ----
_map_calc_tile_addr:
	ldx	<__dh
	lda	mapwidth+1
	beq	.l1
	stx	<__ch
	lda	<__dl
	sta	<__cl
	bra	.l2
	; --
.l1:	stx	<__al
	lda	mapwidth
	sta	<__bl
	jsr	mulu8
	; --
	lda	<__cl
	add	<__dl
	bcc	.l2
	inc	<__ch
	; --
.l2:	add	mapaddr
	sta	<__cl
	lda	mapaddr+1
	and	#$1F
	adc	<__ch
	tax
	; --
;	rol	A
;	rol	A
;	rol	A
;	rol	A
	lsr	A
	lsr	A
	lsr	A
	lsr	A
	lsr	A
	and	#$0F
	add	mapbank
	tam	#3
	; --
	txa
	and	#$1F
	ora	#$60
	sta	<__ch
	ldx	<__cl
	rts

; scroll(char num, int x, int y, char top, char bottom, char disp)
; ----
; set screen scrolling
; ----

_scroll:
	ldy	#8
	lda	[__sp],Y
	and	#$03
	; --
	sax
	and	#$C0
	ora	#$01
	sta	scroll_cr,X
	lda	[__sp]
	inc	A
	sta	scroll_bottom,X
	ldy	#2
	lda	[__sp],Y
	sta	scroll_top,X
	ldy	#4
	lda	[__sp],Y
	sta	scroll_yl,X
	iny
	lda	[__sp],Y
	sta	scroll_yh,X
	iny
	lda	[__sp],Y
	sta	scroll_xl,X
	iny
	lda	[__sp],Y
	sta	scroll_xh,X
	__addmi	#10,__sp
	rts

; scroll_disable(char num)
; ----
; disable screen scrolling for a scroll region
; ----

_scroll_disable:
	lda	scroll_cr,X
	and	#$fe
	sta	scroll_cr,X
	rts

; set_screen_size(char size)
; ----
; set screen virtual size
; ----

_set_screen_size:
	txa
	jmp	set_bat_size

; set_xres(int xres)
; ----
; set horizontal display resolution
; ----

_set_xres.1:
	lda	#XRES_SOFT
	sta	<__cl
_set_xres.2:
	jsr	set_xres
	ldx	<__al
	lda	<__ah
	rts


; ------------------------
; Graphics functions
; ------------------------

; readvram
; ----
; leftover from asm library
; needed for 'a = vram[n]'
; semantic
; ----
readvram:
	ldy	#1
	sty	<vdc_reg
	sty	video_reg
	stx	video_data_l
	sta	video_data_h
	vreg	#$02
	ldx	video_data_l
	lda	video_data_h
	rts


; writevram
; ----
; leftover from asm library
; needed for 'vram[n] = a'
; semantic
; ----
writevram:
	tay
	stz	<vdc_reg
	stz	video_reg
	lda	[__sp]
	sta	video_data_l
	incw	<__sp
	lda	[__sp]
	sta	video_data_h
	incw	<__sp
	vreg	#2
	stx	video_data_l
	sty	video_data_h
	rts


; gfx_setbgpal(char pal)
; ----
; set default major palette for gfx_* func's
; ----

_gfx_setbgpal:
	txa
	asl	A
	asl	A
	asl	A
	asl	A
	sta	gfx_pal
	rts


; gfx_init(int start_vram_addr)
; ----
; initialize graphics mode
; - points graphics map to tiles at start_vram_addr
; ----

_gfx_init:
	maplibfunc lib2_gfx_init
	rts

	.bank LIB2_BANK
lib2_gfx_init:
	__stw	<__dx	; vram addr

	lsrw	<__dx	; shift address to make char pattern
	lsrw	<__dx
	lsrw	<__dx
	lsrw	<__dx
	lda	<__dx+1
	and	#$0f
	ora	gfx_pal	; and add major palette info
	sta	<__dx+1

	setvwaddr $0
	; --
	ldy	bat_height
.l2:	ldx	bat_width
	; --
.l3:	stw	<__dx,video_data
	incw	<__dx
	dex
	bne	.l3
	dey
	bne	.l2
	rts
	.bank LIB1_BANK

; gfx_clear(int start_vram_addr)
; ----
; Clear the values in the graphics tiles
; - places zeroes in graphics tiles at start_vram_addr
; ----

_gfx_clear:
	__stw	<__di		; start_vram_addr
	jsr	set_write	; setup VRAM addr for writing

	lda	bat_height
	sta	<__bl		; loop for all lines
.l2:	ldx	bat_width	; loop for all characters
.l3:	ldy	#8		; loop for 16 words
.l4:	stw	#0,video_data	; unrolled a bit (8 iterations
	stw	#0,video_data	; @ 2 words each iteration)
	dey
	bne	.l4
	dex
	bne	.l3
	dec	<__bl
	bne	.l2
	rts


; gfx_plot(int x [__bx] int y [__cx] char color [reg acc])
; ----
; Plot a point at location (x,y) in color
; ----

_gfx_plot.3:
	maplibfunc	lib2_gfx_plot.3
	rts


; gfx_point(int x [__bx], int y [__cx])
; ----
; Returns color of point at location (x,y)
; ----

_gfx_point.2:
	maplibfunc	lib2_gfx_point.2
	rts


; gfx_line(int x1 [__bx], int y1 [__cx], int x2 [__si], int y2 [__bp], char color [reg acc])
; ----
; Plot a line from location (x1,y1) to location (x2,y2) in color
; ----

_gfx_line.5:
	maplibfunc	lib2_gfx_line.5
	rts

;---------------------------------

;
; Change to context LIB2_BANK for these functions
; because they are larger than LIB1_BANK functions
; should be
;

	.bank	LIB2_BANK

; put_number(int number, char n, int offset)
; put_number(int number, char n, char x, char y)
; ----

lib2_put_number.4:
	lda	<__bl
	jsr	_put.xy
	bra	putnum.main
lib2_put_number.3:
	jsr	_put.vram
putnum.main:
	ldx	<__cl
	; --
	stz	<__al ; sign flag
	dex
	cpx	#16
	bhs	.l5
	; --
	lda	<__dh ; check sign
	bpl	.l1
	negw	<__dx ; negate
	lda	#1
	sta	<__al
	; --
.l1:	jsr	divu10
	ora	#$10
	pha
	dex
	bmi	.l3
	tstw	<__dx
	bne	.l1
	; --
	lda	<__al
	beq	.l2
	lda	#$0D
	pha
	dex
	bmi	.l3
	; --
	cla
.l2:	pha
	dex
	bpl	.l2
	; --
.l3:	ldx	<__cl
.l4:	pla
	add	_font_base
	sta	video_data_l
	cla
	adc	_font_base+1
	sta	video_data_h
	dex
	bne	.l4
.l5:	rts


; gfx_line(int x1 [__bx], int y1 [__cx], int x2 [__si], int y2 [__bp], char color [reg acc])
; ----
; Plot a line from location (x1,y1) to locations (x2,y2) in color
; ----
lib2_gfx_line.5:		; Bresenham line drawing algorithm
	stx	line_color

	cmpw	<__cx,<__bp	; make y always ascending by swapping
	bhs	.l1		; co-ordinates
				; jump over swap if bp > cx

	stw	<__bp,line_curry	; swap coordinates
	stw	<__cx,<__bp
	stw	<__si,line_currx
	stw	<__bx,<__si

	bra	.l2

.l1:	stw	<__bx,line_currx
	stw	<__cx,line_curry

; now:
;	line_currx and line_curry are start point
;	<__si and <__bp are end point
;	<__bx and <__cx are 'dont care'

.l2:
	lda	LOW_BYTE  <__bp
	sub	LOW_BYTE  line_curry
	sta	LOW_BYTE  line_deltay
	lda	HIGH_BYTE <__bp
	sbc	HIGH_BYTE line_curry
	sta	HIGH_BYTE line_deltay

	lda	LOW_BYTE  <__si
	sub	LOW_BYTE  line_currx
	sta	LOW_BYTE  line_deltax
	lda	HIGH_BYTE <__si
	sbc	HIGH_BYTE line_currx
	sta	HIGH_BYTE line_deltax

	stz	line_xdir	; 0 = positive

	lda	HIGH_BYTE line_deltax
	bpl	.l3

	lda	#1
	sta	line_xdir	; 1 = negative
	negw	line_deltax

; now:
;	line_deltay is difference from end to start (positive)
;	line_deltax is difference from end to start (positive)
;	line_xdir shows whether to apply deltax positive or negative


.l3:
	cmpw	line_deltax,line_deltay
	lbhs	.ybiglp		; jump if deltay > |deltax|

.xbiglp:
	__ldw	line_deltay
	__aslw
	__stw	line_adjust
	__stw	line_error

	subw	line_deltax,line_adjust
	subw	line_deltax,line_adjust

	subw	line_deltax,line_error

	incw	line_deltax		; used as counter - get both endpoints

.xlp1:
	stw	line_currx,<__bx	; draw pixel
	stw	line_curry,<__cx
	ldx	line_color
	cla
	jsr	lib2_gfx_plot.3

	decw	line_deltax		; dec counter
	tstw	line_deltax
	lbeq	.out

	lda	line_xdir		; adjust currx
	beq	.xlppos

	decw	line_currx
	bra	.xlp2

.xlppos:	incw	line_currx

.xlp2:
	lda	HIGH_BYTE line_error
	bmi	.xlp3

	addw	line_adjust,line_error
	incw	line_curry
	bra	.xlp1
.xlp3:
	addw	line_deltay,line_error
	addw	line_deltay,line_error
	jmp	.xlp1

.ybiglp:
	__ldw	line_deltax
	__aslw
	__stw	line_adjust
	__stw	line_error

	subw	line_deltay,line_adjust
	subw	line_deltay,line_adjust

	subw	line_deltay,line_error

	incw	line_deltay		; used as counter - get both endpoints

.ylp1:
	stw	line_currx,<__bx	; draw pixel
	stw	line_curry,<__cx
	ldx	line_color
	cla
	jsr	lib2_gfx_plot.3

	decw	line_deltay		; dec counter
	tstw	line_deltay
	beq	.out

	incw	line_curry

	lda	HIGH_BYTE line_error
	bmi	.ylp2

	addw	line_adjust,line_error
	lda	line_xdir
	beq	.ylppos

	decw	line_currx
	bra	.ylp1

.ylppos:
	incw	line_currx
	bra	.ylp1

.ylp2:
	addw	line_deltax,line_error
	addw	line_deltax,line_error
	jmp	.ylp1

.out:
	rts


; gfx_plot(int x [__bx], int y [__cx], char color [reg acc])
; ----
; Plot a point at location (x,y) in color
; ----

lib2_gfx_plot.3:
	stx	<__dl		; color
	jsr	gfx_getaddr

	; same as vm_rawread - save 21 cycles by inlining
	;
	vreg	#1		; video read register
	stw	<__cx,video_data	; VRAM address
	vreg	#2		; set R/W memory mode
	__ldw	video_data
	;
	; end inline

	ldy	<__al		; bit offset
	bbr1	<__dl,.l1
	ora	gfx_bittbl,Y	; set bit
	bra	.l1a
.l1:	and	gfx_bittbl2,Y	; else mask bit
.l1a:
	sax
	bbr0	<__dl,.l2
	ora	gfx_bittbl,Y	; set bit
	bra	.l2a
.l2:	and	gfx_bittbl2,Y	; else mask bit
.l2a:
	; same as vm_rawwrite - save >14 cycles by inlining
	;
	phx
	tax
	vreg	#0		; video write register
	stw	<__cx,video_data	; VRAM address
	vreg	#2		; set R/W memory mode
	pla
	__stw	video_data	; write
	;
	; end inline

	addw	#8,<__cx		; other half of pixel

	; same as vm_rawread - save 21 cycles by inlining
	;
	vreg	#1		; video read register
	stw	<__cx,video_data	; VRAM address
	vreg	#2		; set R/W memory mode
	__ldw	video_data
	;
	; end inline

	ldy	<__al		; bit offset
	bbr3	<__dl,.l3
	ora	gfx_bittbl,Y	; set bit
	bra	.l3a
.l3:	and	gfx_bittbl2,Y	; else mask bit
.l3a:
	sax
	bbr2	<__dl,.l4
	ora	gfx_bittbl,Y	; set bit
	bra	.l4a
.l4:	and	gfx_bittbl2,Y	; mask bit
.l4a:
	; same as vm_rawwrite - save >14 cycles by inlining
	;
	phx
	tax
	vreg	#0		; video write register
	stw	<__cx,video_data	; VRAM address
	vreg	#2		; set R/W memory mode
	pla
	__stw	video_data	; write
	;
	; end inline

	rts


; gfx_point(int x [__bx], int y [__cx])
; ----
; Returns color of point at location (x,y)
; ----

lib2_gfx_point.2:
	jsr	gfx_getaddr
	stz	<__ah		; will be color
	__ldw	<__cx		; VRAM address
	jsr	readvram

	ldy	<__al		; bit offset
	and	gfx_bittbl,Y
	beq	.l1
	smb1	<__ah
.l1:	txa
	and	gfx_bittbl,Y
	beq	.l2
	smb0	<__ah
.l2:
	addw	#8,<__cx
	__ldw	<__cx		; VRAM address part 2
	jsr	readvram

	ldy	<__al
	and	gfx_bittbl,Y
	beq	.l3
	smb3	<__ah
.l3:	txa
	and	gfx_bittbl,Y
	beq	.l4
	smb2	<__ah
.l4:
	ldx	<__ah
	cla
	rts


gfx_bittbl:
	.db	$80,$40,$20,$10,$08,$04,$02,$01
gfx_bittbl2:
	.db	$7f,$bf,$df,$ef,$f7,$fb,$fd,$fe


; gfx_getaddr
; ----
; Utility routine to switch x/y pixel
; co-ordinates into VRAM addr and bit #
; ----

gfx_getaddr:
	lda	<__cl
	and	#7
	sta	<__al	; al = lines from tile base

	lda	<__bl
	and	#7
	pha		; = bit offset

	__ldw	<__bx
	__lsrw		; should be only 2 bits in MSB are possible
	__lsrw		; but we'll shift 3 times anyway
	__lsrw
	phx		; X = character column

	__ldw	<__cx
	__lsrw		; should be only 2 bits in MSB are possible
	__lsrw		; but we'll shift 3 times anyway
	__lsrw
	txa		; A = character row

	plx
	jsr	calc_vram_addr

	__ldw	<__di		; to get BAT addr
	jsr	readvram	; read BAT value
	__aslw			; change into VRAM tile addr
	__aslw
	__aslw
	__aslw			; cx = VRAM addr start of tile

	sax
	clc			; add row within tile
	adc	<__al
	sax
	adc	#0
	__stw	<__cx

	pla
	sta	<__al		; al = bit offset

	rts

; Change back to original LIB1_BANK context

	.bank	LIB1_BANK

_set_map_tile_type:
	stx	maptiletype
	rts

_set_map_tile_base:
	__lsrwi 4
	stx	maptilebase
	sta	maptilebase+1
	rts

_set_map_pals.1:
	stb	<__bl, mapctablebank
	stw	<__si, mapctable
	rts
