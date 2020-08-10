;
; SGX_GFX.ASM - HuC SGX graphic library
;
; Tomaitheous '07


; ----
; local variables

	.zp
sgx_spr_ptr	.ds 2
sgx_spr_max	.ds 1
sgx_spr_flag	.ds 1
_sgx_scroll_y	.ds 2
_sgx_scroll_x	.ds 2
sgx_vdc_sts	.ds 1
sgx_vdc_reg	.ds 1
sgx_vdc_cr	.ds 2

	.bss
_sgx_vdc		.ds 20*2
sgx_init_detect	.ds 1
_sgx_satb		.ds 512

vpc_win_a_size	.ds 2
vpc_win_b_size	.ds 2
vpc_win_blk_1	.ds 1
vpc_win_blk_2	.ds 1

sgx_mapbank		.ds 1
sgx_mapaddr		.ds 2
sgx_mapwidth		.ds 2
sgx_mapheight		.ds 2
sgx_maptiletype		.ds 1
sgx_maptilebank		.ds 1
sgx_maptileaddr		.ds 2
sgx_maptilebase		.ds 2
sgx_mapnbtile		.ds 2
sgx_mapctablebank	.ds 1
sgx_mapctable		.ds 2
sgx_mapwrap		.ds 1
sgx_mapbat_ptr		.ds 2
sgx_mapbat_top_base	.ds 2
sgx_mapbat_top		.ds 1
sgx_mapbat_bottom	.ds 1
sgx_mapbat_x		.ds 2

sgx_bat_width	.ds 2
sgx_bat_height	.ds 1
sgx_bat_hmask	.ds 1
sgx_bat_vmask	.ds 1
sgx_scr_width	.ds 1
sgx_scr_height	.ds 1


;
; SGX_VREG - set up video register to be read/written
;

.macro sgx_vreg
	lda	\1
	sta	<sgx_vdc_reg
.if (\?1 = ARG_IMMED)
	sta	$0010
.else
	sta	sgx_video_reg
.endif
.endm


; ----
; library code

	.code

; [library 3 code]

.include "lib3_versions.asm"


; [SPRITE CODE]

; sgx_spr_set(char num)
; ----
; load SI with the offset of the sprite to change
; SI = satb + 8 * sprite_number
; ----

_sgx_spr_set.1:
	maplibfunc	lib2_sgx_spr_set
	rts

	.bank	LIB2_BANK
lib2_sgx_spr_set:
	cpx	#64
	bhs	.l2
	txa
	inx
	cpx	<sgx_spr_max
	blo	.l1
	stx	<sgx_spr_max
	; --
.l1:	stz	<sgx_spr_ptr+1
	asl a
	asl a
	asl a
	rol	<sgx_spr_ptr+1
	adc	#low(_sgx_satb)
	sta	<sgx_spr_ptr
	lda	<sgx_spr_ptr+1
	adc	#high(_sgx_satb)
	sta	<sgx_spr_ptr+1
.l2:	rts
	.bank	LIB1_BANK


; spr_hide(char num)
; ----
_sgx_spr_hide:
	cly
	bra	sgx_spr_hide
_sgx_spr_hide.1:
	ldy	#$01
sgx_spr_hide:
	maplibfunc_y	lib2_sgx_spr_hide
	rts

	.bank	LIB2_BANK
lib2_sgx_spr_hide
	cpy	#0
	beq	.l2
	; -- hide sprite number #
	cpx	#64
	bhs	.l1
	jsr	_sgx_spr_hide.sub
	lda	[__ptr],Y
	ora	#$02
	sta	[__ptr],Y
.l1:	rts
	; -- hide current sprite
.l2:	ldy	#1
	lda	[sgx_spr_ptr],Y
	ora	#$02
	sta	[sgx_spr_ptr],Y
	rts
	; -- calc satb ptr
_sgx_spr_hide.sub:
	txa
	stz	<__ptr+1
	asl a
	asl a
	asl a
	rol	<__ptr+1
	adc	#low(_sgx_satb)
	sta	<__ptr
	lda	<__ptr+1
	adc	#high(_sgx_satb)
	sta	<__ptr+1
	ldy	#1
	rts

	.bank	LIB1_BANK


; spr_show(char num)
; ----
_sgx_spr_show:
	cly
	bra	sgx_spr_show
_sgx_spr_show.1:
	ldy	#$01
sgx_spr_show:
	maplibfunc_y	lib2_sgx_spr_show
	rts

	.bank	LIB2_BANK
lib2_sgx_spr_show
	cpy	#0
	beq	.l2
	; -- hide sprite number #
	cpx	#64
	bhs	.l1
	jsr	_sgx_spr_hide.sub
	lda	[__ptr],Y
	and	#$01
	sta	[__ptr],Y
.l1:	rts
	; -- hide current sprite
.l2:	ldy	#1
	lda	[sgx_spr_ptr],Y
	and	#$01
	sta	[sgx_spr_ptr],Y
	rts
	.bank	LIB1_BANK


; sgx_satb_update()
; ----

_sgx_satb_update.1:
	ldy	#1
	bra	sgx_satb_update
_sgx_satb_update:
	cly
sgx_satb_update:
	maplibfunc_y	lib2_sgx_satb_update
	rts

	.bank	LIB2_BANK
lib2_sgx_satb_update:
	lda	<sgx_spr_flag
	beq	.l1
	stz	<sgx_spr_flag
	ldx	#64
	bra	.l3
	; --
.l1:	cpy	#1
	beq	.l2
	ldx	<sgx_spr_max
.l2:	cpx	#0
	beq	.l4
	; --
.l3:	stx	<__al	; number of sprites
	txa
	dec a		; round up to the next group of 4 sprites
	lsr a
	lsr a
	inc a
	sta	<__cl

; Use TIA, but BLiT 16 words at a time (32 bytes)
; Because interrupt must not deferred too much
;
	stw	#32, ram_hdwr_tia_size
	stw	#sgx_video_data, ram_hdwr_tia_dest
	stw	#_sgx_satb, <__si

	stw	#$7F00, <__di
	jsr	sgx_set_write

.l3a:	stw	<__si, ram_hdwr_tia_src
	jsr	ram_hdwr_tia
	addw	#32,<__si
	dec	<__cl
	bne	.l3a

;.l3:	stx	<__al
;	stw	#_sgx_satb,<__si
;	stb	#BANK(_sgx_satb),<__bl
;	stw	#$7F00,<__di
;	txa
;	stz	<__ch
;	asl a
;	asl a
;	rol	<__ch
;	sta	<__cl
;	jsr	sgx_load_vram

	; --
	ldx	<__al
.l4:	cla
	rts
	.bank	LIB1_BANK


; init_satb()
; reset_satb()
; ----

_sgx_reset_satb:
_sgx_init_satb:
	clx
	cla
.l1:	stz	_sgx_satb,X
	stz	_sgx_satb+256,X
	inx
	bne	.l1
	; --
	ldy	#1
	sty	<sgx_spr_flag
	stz	<sgx_spr_max
	rts

_sgx_spr_x:
	cly
	bra	sgx_lib2_group_1
_sgx_spr_y:
	ldy	#1
	bra	sgx_lib2_group_1
_sgx_spr_get_x:
	ldy	#2
	bra	sgx_lib2_group_1
_sgx_spr_get_y:
	ldy	#3
sgx_lib2_group_1:
	maplibfunc_y	lib2_group_1
	rts

	.bank	LIB2_BANK
lib2_group_1:
	cpy	#$01
	beq	lib2_sgx_spr_y
	bcc	lib2_sgx_spr_x
	cpy	#$03
	bcc	lib2_sgx_spr_get_x
	bcs	lib2_sgx_spr_get_y


; sgx_spr_x(int value)
; ----

lib2_sgx_spr_x:
	ldy	#2
	sax
	add	#32
	sta	[sgx_spr_ptr],Y
	sax
	adc	#0
	iny
	sta	[sgx_spr_ptr],Y
	rts

lib2_sgx_spr_get_x:
	ldy	#2
	lda	[sgx_spr_ptr],Y
	sub	#32
	tax
	iny
	lda	[sgx_spr_ptr],Y
	sbc	#0
	rts


; sgx_spr_y(int value)
; ----

lib2_sgx_spr_y:
	sax
	add	#64
	sta	[sgx_spr_ptr]
	sax
	adc	#0
	and	#$01
	ldy	#1
	sta	[sgx_spr_ptr],Y
	rts

lib2_sgx_spr_get_y:
	lda	[sgx_spr_ptr]
	sub	#64
	tax
	ldy	#1
	lda	[sgx_spr_ptr],Y
	sbc	#0
	rts
	.bank	LIB1_BANK


_sgx_spr_pattern:
	cly
	bra	sgx_lib2_group_2
_sgx_spr_get_pattern:
	ldy	#1
	bra	sgx_lib2_group_2
_sgx_spr_ctrl.2:
	ldy	#2
	bra	sgx_lib2_group_2
_sgx_spr_pal:
	ldy	#3
	bra	sgx_lib2_group_2
_sgx_spr_get_pal:
	ldy	#4
	bra	sgx_lib2_group_2
_sgx_spr_pri:
	ldy	#5
sgx_lib2_group_2:
	maplibfunc_y	lib2_group_2
	rts

	.bank	LIB2_BANK
lib2_group_2:
	cpy	#01
	beq	lib2_sgx_spr_get_pattern
	bcc	lib2_sgx_spr_pattern
	cpy	#03
	bcc	lib2_sgx_spr_ctrl.2
	beq	lib2_sgx_spr_pal
	cpy	#05
	bcc	lib2_sgx_spr_get_pal
	bra	lib2_sgx_spr_pri


; sgx_spr_pattern(int vaddr)
; ----

lib2_sgx_spr_pattern:
	sta	<__temp
	txa
	asl a
	rol	<__temp
	rol a
	rol	<__temp
	rol a
	rol	<__temp
	rol a
	and	#$7
	ldy	#5
	sta	[sgx_spr_ptr],Y
	lda	<__temp
	dey
	sta	[sgx_spr_ptr],Y
	rts

lib2_sgx_spr_get_pattern:
	ldy	#4
	lda	[sgx_spr_ptr],Y
	sta	<__temp
	iny
	lda	[sgx_spr_ptr],Y
	lsr a
	ror	<__temp
	ror a
	ror	<__temp
	ror a
	ror	<__temp
	ror a
	and	#$E0
	tax
	lda	<__temp
	rts


; sgx_spr_ctrl(char mask [__al], char value)
; ----

lib2_sgx_spr_ctrl.2:
	txa
	and	<__al
	sta	<__temp
	lda	<__al
	eor	#$FF
	ldy	#7
	and	[sgx_spr_ptr],Y
	ora	<__temp
	sta	[sgx_spr_ptr],Y
	rts


; sgx_spr_pal(char pal)
; ----

lib2_sgx_spr_pal:
	txa
	and	#$0F
	sta	<__temp
	ldy	#6
	lda	[sgx_spr_ptr],Y
	and	#$F0
	ora	<__temp
	sta	[sgx_spr_ptr],Y
	rts

lib2_sgx_spr_get_pal:
	ldy	#6
	lda	[sgx_spr_ptr],Y
	and	#$0F
	tax
	cla
	rts


; sgx_spr_pri(char pri)
; ----

lib2_sgx_spr_pri:
	ldy	#6
	lda	[sgx_spr_ptr],Y
	and	#$7F
	cpx	#$00
	beq	.l1
	ora	#$80
.l1:
	sta	[sgx_spr_ptr],Y
	rts
	.bank	LIB1_BANK


; [VDC CODE]

; sgx_detect()
; ----
; returns 1 if true and 0 if false
; ----
_sgx_detect:
	ldx	sgx_init_detect
	lda	sgx_init_detect
	rts


; sgx_vreg(char reg)
; ----
; sgx_vreg(char reg, int data)
; ----

_sgx_vreg.1:
	stx	<sgx_vdc_reg
	stx sgx_video_reg
	rts

_sgx_vreg.2:
	ldx	<__al
	stx	<sgx_vdc_reg
	stx sgx_video_reg

	lda	<__cl
	sta sgx_video_data
	lda	<__ch
	sta sgx_video_data+1
	rts


; ----
; sgx_set_write
; ----
; set the SGX VDC VRAM write pointer
; ----
; IN :	_DI = VRAM location
; ----

sgx_set_write:
	lda	#$00
	sta sgx_video_reg
	lda	<__di
	sta	sgx_video_data_l
.ifdef HUC
	sta	_sgx_vdc
.endif
	lda	<__di+1
	sta	sgx_video_data_h
.ifdef HUC
	sta	_sgx_vdc+1
.endif
	lda	#$02
	sta sgx_video_reg
	rts


; sgx_load_vram
; ----
; copy a block of memory to SGX VRAM
; ----
; IN :	_DI = VRAM location
;	_BL = data bank
;	_SI = data memory location
;	_CX = number of words to copy
; ----

sgx_load_vram:
_sgx_load_vram.3:
	maplibfunc_y	lib2_sgx_load_vram.3
	rts

	.bank	LIB2_BANK
; sgx_load_vram
; ----
lib2_sgx_load_vram.3:
	; ----
	; map data
	;
	jsr	map_data

	; ----
	; setup call to TIA operation (fastest transfer)
	;
	; (instruction setup done during bootup...)

	stw	#sgx_video_data, ram_hdwr_tia_dest
;	stw	<__si, ram_hdwr_tia_src
;
;	asl	<__cl	; change from words to bytes (# to xfer)
;	rol	<__ch

	; ----
	; set vram address
	;
	jsr	sgx_set_write

	; ----
	; copy data
	;
	cly
	ldx	<__cl
	beq	.l3
	; --
.l1:	lda	[__si],Y
	sta	sgx_video_data_l
	iny
	lda	[__si],Y
	sta	sgx_video_data_h
	iny
	bne	.l2
	inc	<__si+1
	; --
.l2:	dex
	bne	.l1
	; --
	jsr	remap_data
	; --
.l3:	dec	<__ch
	bpl	.l1

;.l1:	lda	<__ch	; if zero-transfer, exit
;	ora	<__cl
;	beq	.out
;
;	lda	<__ch
;	cmp	#$20	; if more than $2000, repeat xfers of $2000
;	blo	.l2	; while adjusting banks
;	sub	#$20	; reduce remaining transfer amount
;	sta	<__ch
;
;	stw	#$2000, ram_hdwr_tia_size
;	jsr	ram_hdwr_tia
;
;	lda	<__si+1	; force bank adjust
;	add	#$20	; and next move starts at same location
;	sta	<__si+1
;
;	jsr	remap_data	; adjust banks
;	bra	.l1
;
;.l2:	sta	HIGH_BYTE ram_hdwr_tia_size	; 'remainder' transfer of < $2000
;	lda	<__cl
;	sta	LOW_BYTE  ram_hdwr_tia_size
;	jsr	ram_hdwr_tia

	; ----
	; unmap data
	;
.out:
	; restore PCE VDC address
	stw	#video_data, ram_hdwr_tia_dest
	jmp	unmap_data

	.bank	LIB1_BANK


; init_sgx_vdc()
; ----
; same as init_vdc except it initializes SGX VDC.
; ----

init_sgx_vdc:
	maplibfunc	lib2_init_sgx_vdc
	rts

	.bank	LIB2_BANK
; init_sgx_vdc()
; ----

lib2_init_sgx_vdc:
	stw	#_sgx_init_point,<__si	; register table address in 'si'
	cly

.sgx_init_LL1:
	lda	[__si],y
	sta sgx_video_reg
	iny
	lda	[__si],y
	sta sgx_video_data
	iny
	lda	[__si],y
	sta sgx_video_data+1
	iny
	cpy	#$24
	bne	.sgx_init_LL1

	; ----
	; clear the video RAM
	;
	stz	$0010
	stz	$0012
	stz	$0013
	lda	#2
	sta	$0010

	ldx	#128
.l2:	cly
.l3:	stz	$0012
	stz	$0013
	dey
	bne	.l3
	dex
	bne	.l2

	stz	$0010	; Write SGX2 to VDC 2 vram
	stz	$0012
	stz	$0013
	lda	#2
	sta	$0010
	lda	#$53
	sta	$0012
	lda	#$47
	sta	$0013
	lda	#$58
	sta	$0012
	lda	#$32
	sta	$0013

	stz vdc_reg	; write PCE1 to VDC 1 vram
	st0	#0
	st1	#0
	st2	#0
	lda	#2
	sta vdc_reg
	st0	#2
	st1	#$50
	st2	#$43
	st1	#$45
	st2	#$31

	lda	#1
	sta	$0010
	stz	$0012
	stz	$0013
	lda	#2
	sta	$0010

	lda	$0012
	cmp	#$53
	bne	.no_sgx
	lda	$0013
	cmp	#$47
	bne	.no_sgx
	lda	$0012
	cmp	#$58
	bne	.no_sgx
	lda	$0013
	cmp	#$32
	bne	.no_sgx
	bra	.yes_sgx
.no_sgx:
	stz	sgx_init_detect
	rts
.yes_sgx:
	lda	#1
	sta	sgx_init_detect

	;disable display and interrupts and update offline status
	stz	<sgx_vdc_cr
	stz	<sgx_vdc_cr+1
	lda	#$05
	sta	<sgx_vdc_sts
	sta sgx_videoport
	stz sgx_video_data
	stz sgx_video_data+1

	;reset scroll registers
	stz	<__al
	stz	<__ah
	stz	<__bl
	stz	<__bh
	jsr	_sgx_scroll.2

	rts

	.bank	LIB1_BANK

_sgx_disp_on:
	cly
	bra	lib2_group_4
_sgx_disp_off:
	ldy	#1
	bra	lib2_group_4
_sgx_spr_on:
	ldy	#2
	bra	lib2_group_4
_sgx_spr_off:
	ldy	#3
	bra	lib2_group_4
_sgx_bg_on:
	ldy	#4
	bra	lib2_group_4
_sgx_bg_off:
	ldy	#5

lib2_group_4:
	maplibfunc_y	lib2_group_4_case
	rts

	.bank	LIB2_BANK
lib2_group_4_case:
	cpy	#1
	bcc	lib2_group_4_lbl0
	beq	lib2_group_4_lbl1
	cpy	#3
	bcc	lib2_group_4_lbl2
	beq	lib2_group_4_lbl3
	cpy	#5
	bcc	lib2_group_4_lbl4
	bcs	lib2_group_4_lbl5

lib2_group_4_lbl0:
	jmp lib2_sgx_disp_on
lib2_group_4_lbl1:
	jmp lib2_sgx_disp_off
lib2_group_4_lbl2:
	jmp lib2_sgx_spr_on
lib2_group_4_lbl3:
	jmp lib2_sgx_spr_off
lib2_group_4_lbl4:
	jmp lib2_sgx_bg_on
lib2_group_4_lbl5:
	jmp lib2_sgx_bg_off


; sgx_disp_on()
; ----
; Turns on SGX BG and SPR. Interrupts are disabled.
; ----

lib2_sgx_disp_on:
	lda	#$05
	sta sgx_video_reg
	lda	<sgx_vdc_cr
	ora	#$C0
	sta	<sgx_vdc_cr
	sta sgx_video_data_l
	rts


; sgx_disp_off()
; ----
; Turns off SGX BG and SPR. Interrupts are disabled.
; ----

lib2_sgx_disp_off:
	lda	#$05
	sta sgx_video_reg
	cla
	sta	<sgx_vdc_cr
	sta sgx_video_data_l
	rts


; sgx_spr_on()
; ----
; Turns on SGX SPR. Interrupts are disabled.
; ----

lib2_sgx_spr_on:
	lda	#$05
	sta sgx_video_reg
	lda	<sgx_vdc_cr
	ora	#$40
	sta	<sgx_vdc_cr
	sta sgx_video_data_l
	rts


; sgx_spr_off()
; ----
; Turns off SGX SPR. Interrupts are disabled.
; ----

lib2_sgx_spr_off:
	lda	#$05
	sta sgx_video_reg
	lda	<sgx_vdc_cr
	and	#$BF
	sta	<sgx_vdc_cr
	sta sgx_video_data_l
	rts


; sgx_bg_on()
; ----
; Turns on SGX BG. Interrupts are disabled.
; ----

lib2_sgx_bg_on:
	lda	#$05
	sta sgx_video_reg
	lda	<sgx_vdc_cr
	ora	#$80
	sta	<sgx_vdc_cr
	sta sgx_video_data_l
	rts


; sgx_bg_off()
; ----
; Turns off SGX BG. Interrupts are disabled.
; ----

lib2_sgx_bg_off:
	lda	#$05
	sta sgx_video_reg
	lda	<sgx_vdc_cr
	and	#$7F
	sta	<sgx_vdc_cr
	sta sgx_video_data_l
	rts

	.bank	LIB1_BANK

; [VPC CODE]

; vpc_win_size( char window_num (& 0x01) , int size );
; ----
;	Change the VPC window size
; ----
;
_vpc_win_size.2:
	lda	<__al
	and	#$01
	asl a
	tay
	lda	<__bl
	sta	$000A, y
	lda	<__bh
	sta	$000A+1, y
	rts


; vpc_win_reg( char window_num (& 0x01) , char var );
; ----
;	Change the VPC window control settings: Sprite priorities and enable/disable VDCs
;	based on windows configuration.
; ----
;
_vpc_win_reg.2:
	maplibfunc_y	lib2_vpc_win_reg.2
	rts

	.bank	LIB2_BANK
; ----
lib2_vpc_win_reg.2:
	lda	<__al
	beq	.win_a
	cmp	#$01
	beq	.win_b
	cmp	#$02
	beq	.win_ab
	;else

.win_none:
	lda	<__bl
	asl a
	asl a
	asl a
	asl a
	sta	<__bl

	lda vpc_win_blk_2
	and	#0x0F
	clc
	adc	<__bl
	sta vpc_win_blk_2
	sta vpc_ctrl_2
	rts

.win_a:
	lda	<__bl
	and	#$0F
	sta	<__bl

	lda vpc_win_blk_2
	and	#0xF0
	clc
	adc	<__bl
	sta vpc_win_blk_2
	sta vpc_ctrl_2
	rts

.win_b:
	lda	<__bl
	asl a
	asl a
	asl a
	asl a
	sta	<__bl

	lda vpc_win_blk_1
	and	#0x0F
	clc
	adc	<__bl
	sta vpc_win_blk_1
	sta vpc_ctrl_1
	rts

.win_ab:
	lda	<__bl
	and	#$0F
	sta	<__bl

	lda vpc_win_blk_1
	and	#0xF0
	clc
	adc	<__bl
	sta vpc_win_blk_1
	sta vpc_ctrl_1
	rts

	rts
	.bank	LIB1_BANK


; [BACKGROUND CODE]

; sgx_load_map(char x [__al], char y [__ah], int mx, int my, char w [__dl], char h [__dh])
; ----

_sgx_load_map.6:
	maplibfunc_y	lib2_sgx_load_map.6
	rts

	.bank	LIB2_BANK
lib2_sgx_load_map.6
	tstw	sgx_mapwidth
	beq	.l6
	tstw	sgx_mapheight
	beq	.l6

	; ----
	; adjust map y coordinate
	;
	lda	<__bh
	bmi	.l2
.l1:	cmpw	sgx_mapheight,<__bx
	blo	.l3
	subw	sgx_mapheight,<__bx
	bra	.l1
	; --
.l2:	lda	<__bh
	bpl	.l3
	addw	sgx_mapheight,<__bx
	bra	.l2

	; ----
	; adjust map x coordinate
	;
.l3:	stb	<__bl,<__ch
	lda	<__di+1
	bmi	.l5
.l4:	cmpw	sgx_mapwidth,<__di
	blo	.l7
	subw	sgx_mapwidth,<__di
	bra	.l4
	; --
.l5:	lda	<__di+1
	bpl	.l7
	addw	sgx_mapwidth,<__di
	bra	.l5

	; ----
	; exit
	;
.l6:	rts

	; ----
	; ok
	;
.l7:	stb	<__di,<__cl
	jmp	sgx_load_map

.include "sgx_load_map.asm"

	.bank	LIB1_BANK


;	sgx_scroll( int X [__ax], int Y [__bx])
; ----
;	updates the SGX background scroll registers
; ----
;
_sgx_scroll.2:
	maplibfunc_y	lib2_sgx_scroll.2
	rts

	.bank	LIB2_BANK
; ----
lib2_sgx_scroll.2:
	lda	#$07
	sta	<sgx_vdc_sts
	sta sgx_video_reg
	lda	<__al
	sta sgx_video_data_l
	sta	<_sgx_scroll_x
	lda	<__ah
	sta sgx_video_data_h
	sta	<_sgx_scroll_x+1

	lda	#$08
	sta	<sgx_vdc_sts
	sta sgx_video_reg
	lda	<__bl
	sta sgx_video_data_l
	sta	<_sgx_scroll_y
	lda	<__bh
	sta sgx_video_data_h
	sta	<_sgx_scroll_y+1

	rts
	.bank	LIB1_BANK


; sgx_load_bat(int vaddr, int *bat_data, char w, char h)
; ----
; load_bat equiv for SGX
; ----
_sgx_load_bat.4:
	maplibfunc_y	lib2_sgx_load_bat
	rts

	.bank	LIB2_BANK

; sgx_load_bat(int vaddr, int *bat_data, char w, char h)
; ----

lib2_sgx_load_bat:
	; ----
	; map data
	;
	jsr	map_data

	; ----
	; copy BAT
	;
	cly
	; --
.l1:	jsr	sgx_set_write
	ldx	<__cl
	; --
.l2:	lda	[__si],Y
	sta	sgx_video_data_l
	iny
	lda	[__si],Y
	sta	sgx_video_data_h
	iny
	bne	.l3
	inc	<__si+1
.l3:	dex
	bne	.l2
	; --
	jsr	remap_data
	; --
	addw	sgx_bat_width,<__di
	dec	<__ch
	bne	.l1

	; ----
	; unmap data
	;
	jmp	unmap_data
	.bank	LIB1_BANK


; sgx_set_bat_size ( char size )
; ----
sgx_set_bat_size:
	maplibfunc_y	lib2_sgx_set_bat_size
	rts

	.bank	LIB2_BANK

; ----
; sgx_set_bat_size
; ----
; set bg map virtual size for SGX
; ----
; IN : A = new size (0-7)
; ----

lib2_sgx_set_bat_size:
	and	#$07
	pha
	; --
	lda	#09
	sta sgx_video_reg
	pla
	tax
	asl a
	asl a
	asl a
	asl a
.ifdef HUC
	sta	_sgx_vdc+18
.endif
	sta	sgx_video_data_l
	; --
	lda	.sgx_width,X
	sta	sgx_bat_width
	stz	sgx_bat_width+1
	dec a
	sta	sgx_bat_hmask
	; --
	lda	.sgx_height,X
	sta	sgx_bat_height
	sta	sgx_mapbat_bottom
	stz	sgx_mapbat_top
	stz	sgx_mapbat_top_base
	stz	sgx_mapbat_top_base+1
	dec a
	sta	sgx_bat_vmask
	rts

.sgx_width:	.db $20,$40,$80,$80,$20,$40,$80,$80
.sgx_height:	.db $20,$20,$20,$20,$40,$40,$40,$40
	.bank	LIB1_BANK


; sgx_set_screen_size(char size)
; ----
; sgx_set screen virtual size
; ----

_sgx_set_screen_size.1:
	txa
	jmp	sgx_set_bat_size


; sgx_set_map_data(int *ptr)
; sgx_set_map_data(char *map [__bl:__si], int w [__ax], int h)
; sgx_set_map_data(char *map [__bl:__si], int w [__ax], int h [__dx], char wrap)
; ----
; map,	map base address
; w,	map width
; h,	map height
; wrap, wrap flag (1 = wrap, 0 = do not wrap)
; ----

_sgx_set_map_data.1:
	cly
	bra	lib3_group_1
_sgx_set_map_data.3:
	ldy	#1
	bra	lib3_group_1
_sgx_set_map_data.4:
	ldy	#2
	bra	lib3_group_1
_sgx_set_tile_data.1:
	ldy	#3
	bra	lib3_group_1
_sgx_set_tile_data.4:
	ldy	#4
	bra	lib3_group_1
_sgx_load_tile:
	ldy	#5

lib3_group_1:
	maplibfunc_y	lib3_group_select_1
	rts

	.bank	LIB2_BANK
lib3_group_select_1:
	cpy	#1
	bcc	lib3_group_case_1 ;lib3_sgx_set_map_data.1
	beq	lib3_group_case_2 ;lib3_sgx_set_map_data.3
	cpy	#3
	bcc	lib3_group_case_3 ;lib3_sgx_set_map_data.4
	beq	lib3_group_case_4 ;lib3_sgx_set_tile_data.1
	cpy	#5
	bcc	lib3_group_case_5 ;lib3_sgx_set_tile_data.4
	bcs	lib3_group_case_6 ;lib3_sgx_load_tile

lib3_group_case_6:
	jmp lib3_sgx_load_tile
lib3_group_case_5:
	jmp lib3_sgx_set_tile_data.4
lib3_group_case_4:
	jmp lib3_sgx_set_tile_data.1
lib3_group_case_3:
	jmp lib3_sgx_set_map_data.4
lib3_group_case_2:
	jmp lib3_sgx_set_map_data.3
lib3_group_case_1:

lib3_sgx_set_map_data.1:
	__stw	<__si
	ora	<__si
	beq	.l1
	; -- calculate width
	lda	[__si].4
	sub	[__si]
	sta	sgx_mapwidth
	lda	[__si].5
	sbc	[__si].1
	sta	sgx_mapwidth+1
	incw	sgx_mapwidth
	; -- calculate height
	lda	[__si].6
	sub	[__si].2
	sta	sgx_mapheight
	lda	[__si].7
	sbc	[__si].3
	sta	sgx_mapheight+1
	incw	sgx_mapheight
	; -- get map bank
	lda	[__si].8
	sta	sgx_mapbank
	; -- get map addr
	lda	[__si].10
	sta	sgx_mapaddr
	iny
	lda	[__si]
	sta	sgx_mapaddr+1
	; -- no wrap
	stz	sgx_mapwrap
	rts
	; -- null pointer
.l1:	stwz	sgx_mapwidth
	stwz	sgx_mapheight
	stz	sgx_mapbank
	stwz	sgx_mapaddr
	stz	sgx_mapwrap
	rts
lib3_sgx_set_map_data.4:
	stx	sgx_mapwrap
	__ldw	<__dx
	bra	sgx_set_map_data.main
lib3_sgx_set_map_data.3:
	stz	sgx_mapwrap
	inc	sgx_mapwrap
sgx_set_map_data.main:
	__stw	sgx_mapheight
	stw	<__ax,sgx_mapwidth
	stb	<__bl,sgx_mapbank
	stw	<__si,sgx_mapaddr
	rts
	.bank	LIB1_BANK


; sgx_set_tile_data(char *tile_ex [__di])
; sgx_set_tile_data(char *tile [__bl:__si], int nb_tile [__cx], char *ptable [__al:__dx], char type [__ah])
; ----
; tile,	tile base index
; nb_tile, number of tile
; ptable,	tile palette table address
; type, tile type (8 or 16)
; ----

	.bank	LIB2_BANK
lib3_sgx_set_tile_data.1:
	cly
	lda	[__di],Y++
	sta	sgx_mapnbtile
	lda	[__di],Y++
	sta	sgx_mapnbtile+1
	lda	[__di],Y++
	sta	sgx_maptiletype
	iny
	lda	[__di],Y++
	sta	sgx_maptilebank
	iny
	lda	[__di],Y++
	sta	sgx_maptileaddr
	lda	[__di],Y++
	sta	sgx_maptileaddr+1
	lda	#(CONST_BANK+_bank_base)
	sta	sgx_mapctablebank
	lda	[__di],Y++
	sta	sgx_mapctable
	lda	[__di],Y
	sta	sgx_mapctable+1
	rts
lib3_sgx_set_tile_data.4:
	stb	<__bl,sgx_maptilebank
	stw	<__si,sgx_maptileaddr
	stw	<__cx,sgx_mapnbtile
	stb	<__al,sgx_mapctablebank
	stw	<__dx,sgx_mapctable
	stb	<__ah,sgx_maptiletype
	rts
	.bank	LIB1_BANK


	.bank	LIB2_BANK
; sgx_load_tile(int addr)
; ----

lib3_sgx_load_tile:
	__stw	<__di
	stx	<__al
	lsr a
	ror	<__al
	lsr a
	ror	<__al
	lsr a
	ror	<__al
	lsr a
	ror	<__al
	sta	sgx_maptilebase+1
	stb	<__al,sgx_maptilebase
	; --
	stw	sgx_mapnbtile,<__cx
	ldx	#4
	lda	sgx_maptiletype
	cmp	#8
	beq	.l1
	ldx	#6
.l1:	asl	<__cl
	rol	<__ch
	dex
	bne	.l1
	; --
	stb	sgx_maptilebank,<__bl
	stw	sgx_maptileaddr,<__si
	jmp	lib3_sgx_load_vram

	.bank	LIB1_BANK
