; ***************************************************************************
; ***************************************************************************
;
; huc-gfx.asm
;
; Based on the original HuC and MagicKit functions by David Michel and the
; other original HuC developers.
;
; Modifications copyright John Brandwood 2024-2025.
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
; void __fastcall disp_on( void );
; void __fastcall disp_off( void );

		.alias	_disp_on		= set_dspon
		.alias	_disp_off		= set_dspoff



; ***************************************************************************
; ***************************************************************************
;
; void __fastcall set_screen_size( unsigned char value<_al> );
; void __fastcall sgx_set_screen_size( unsigned char value<_al> );
;

		.alias	_set_screen_size.1	= set_bat_vdc
		.alias	_sgx_set_screen_size.1	= set_bat_sgx



; ***************************************************************************
; ***************************************************************************
;
; void __fastcall init_256x224( void );

_init_256x224	.proc

.BAT_SIZE	=	64 * 32
.CHR_0x20	=	.BAT_SIZE / 16		; 1st tile # after the BAT.
.SAT_ADDR	=	$7F00			; SAT takes 16 tiles of VRAM.

		php				; Disable interrupts.
		sei

		call	clear_vce		; Clear all palettes.

		lda.l	#.CHR_0x20		; CHR # of ASCII ' '.
		sta.l	<_ax
		lda.h	#.CHR_0x20
		sta.h	<_ax

		lda	#>.BAT_SIZE		; Size of BAT in words.
		sta	<_bl

		call	clear_vram_vdc		; Clear VRAM.
	.if	SUPPORT_SGX
		call	clear_vram_sgx
	.endif

		lda	#<.mode_256x224		; Disable BKG & SPR layers but
		sta.l	<_bp			; enable RCR & VBLANK IRQ.
		lda	#>.mode_256x224
		sta.h	<_bp

	.if	SUPPORT_SGX
		call	sgx_detect		; Are we really on an SGX?
		bcc	!+
		ldy	#^.mode_256x224		; Set SGX 1st, with no VBL.
		call	set_mode_sgx
	.endif
!:		ldy	#^.mode_256x224		; Set VDC 2nd, VBL allowed.
		call	set_mode_vdc

	.if	SUPPORT_SGX
		bit	SGX_SR			; Purge any overdue RCR.
	.endif
		bit	VDC_SR			; Purge any overdue VBL.
		plp				; Restore interrupts.

		call	wait_vsync		; Wait for the next VBLANK.

		leave				; All done, phew!

		; A standard 256x224 screen with overscan.

.mode_256x224:	db	$80			; VCE Control Register.
		db	VCE_CR_5MHz + XRES_SOFT	;   Video Clock + Artifact Reduction

		db	VDC_MWR			; Memory-access Width Register
		dw	VDC_MWR_64x32 + VDC_MWR_1CYCLE
		db	VDC_HSR			; Horizontal Sync Register
		dw	VDC_HSR_256
		db	VDC_HDR			; Horizontal Display Register
		dw	VDC_HDR_256
		db	VDC_VPR			; Vertical Sync Register
		dw	VDC_VPR_224
		db	VDC_VDW			; Vertical Display Register
		dw	VDC_VDW_224
		db	VDC_VCR			; Vertical Display END position Register
		dw	VDC_VCR_224
		db	VDC_DCR			; DMA Control Register
		dw	$0010			;   Enable automatic VRAM->SATB
		db	VDC_DVSSR		; VRAM->SATB address $7F00
		dw	.SAT_ADDR
		db	VDC_BXR			; Background X-Scroll Register
		dw	$0000
		db	VDC_BYR			; Background Y-Scroll Register
		dw	$0000
		db	VDC_RCR			; Raster Counter Register
		dw	$0000			;   Never occurs!
		db	VDC_CR			; Control Register
		dw	$000C			;   Enable VSYNC & RCR IRQ
		db	0

		.endp



; ***************************************************************************
; ***************************************************************************
;
; void __fastcall init_240x208( void );

_init_240x208	.proc

.BAT_SIZE	=	32 * 32
.CHR_0x20	=	.BAT_SIZE / 16		; 1st tile # after the BAT.
.SAT_ADDR	=	$7F00			; SAT takes 16 tiles of VRAM.

		php				; Disable interrupts.
		sei

		call	clear_vce		; Clear all palettes.

		lda.l	#.CHR_0x20		; CHR # of ASCII ' '.
		sta.l	<_ax
		lda.h	#.CHR_0x20
		sta.h	<_ax

		lda	#>.BAT_SIZE		; Size of BAT in words.
		sta	<_bl

		call	clear_vram_vdc		; Clear VRAM.
	.if	SUPPORT_SGX
		call	clear_vram_sgx
	.endif

		lda	#<.mode_240x208		; Disable BKG & SPR layers but
		sta.l	<_bp			; enable RCR & VBLANK IRQ.
		lda	#>.mode_240x208
		sta.h	<_bp

	.if	SUPPORT_SGX
		call	sgx_detect		; Are we really on an SGX?
		bcc	!+
		ldy	#^.mode_240x208		; Set SGX 1st, with no VBL.
		call	set_mode_sgx
	.endif
!:		ldy	#^.mode_240x208		; Set VDC 2nd, VBL allowed.
		call	set_mode_vdc

	.if	SUPPORT_SGX
		bit	SGX_SR			; Purge any overdue RCR.
	.endif
		bit	VDC_SR			; Purge any overdue VBL.
		plp				; Restore interrupts.

		call	wait_vsync		; Wait for the next VBLANK.

		leave				; All done, phew!

		; A reduced 240x208 screen (Seiya Monogatari, Legend of Xanadu).

.mode_240x208:	db	$80			; VCE Control Register.
		db	VCE_CR_5MHz + XRES_SOFT	;   Video Clock + Artifact Reduction

		db	VDC_MWR			; Memory-access Width Register
		dw	VDC_MWR_32x32 + VDC_MWR_1CYCLE
		db	VDC_HSR			; Horizontal Sync Register
		dw	VDC_HSR_240
		db	VDC_HDR			; Horizontal Display Register
		dw	VDC_HDR_240
		db	VDC_VPR			; Vertical Sync Register
		dw	VDC_VPR_208
		db	VDC_VDW			; Vertical Display Register
		dw	VDC_VDW_208
		db	VDC_VCR			; Vertical Display END position Register
		dw	VDC_VCR_208
		db	VDC_DCR			; DMA Control Register
		dw	$0010			;   Enable automatic VRAM->SATB
		db	VDC_DVSSR		; VRAM->SATB address $7F00
		dw	.SAT_ADDR
		db	VDC_BXR			; Background X-Scroll Register
		dw	$0000
		db	VDC_BYR			; Background Y-Scroll Register
		dw	$0000
		db	VDC_RCR			; Raster Counter Register
		dw	$0000			;   Never occurs!
		db	VDC_CR			; Control Register
		dw	$000C			;   Enable VSYNC & RCR IRQ
		db	0

		.endp



; ***************************************************************************
; ***************************************************************************
;
; void __fastcall _macro set_xres( unsigned int x_pixels<_ax> );
; void __fastcall _macro sgx_set_xres( unsigned int x_pixels<_ax> );
;
; void __fastcall set_xres( unsigned int x_pixels<_ax>, unsigned char blur_flag<_bl> );
; void __fastcall sgx_set_xres( unsigned int x_pixels<_ax>, unsigned char blur_flag<_bl> );
;
; blur_flag = XRES_SOFT (default if not specified), XRES_SHARP or XRES_KEEP

set_xres_group	.procgroup			; These routines share code!

	.if	SUPPORT_SGX

_sgx_set_xres.2	.proc

		ldx	#SGX_VDC_OFFSET		; Offset to SGX VDC.
		db	$F0			; Turn "clx" into a "beq".

		.endp
	.endif

_set_xres.2	.proc

.hdw		=	_bh
.hds		=	_al
.hde		=	_ah

		clx				; Offset to PCE VDC.

		lda.l	<_ax			; Convert x pixel resolution to
		lsr.h	<_ax			; tiles.
		ror	a
		lsr.h	<_ax
		ror	a
		lsr.h	<_ax
		ror	a
		dec	a			; HDW = width - 1
		sta	<.hdw

		ldy	<_bl			; Keep the current VCE clock?
		bpl	.calc_speed

.read_speed:	tay				; Read the current VCE clock
		lda	vce_cr			; speed from the shadow.
		and	#3
		say
		bra	.got_speed

.calc_speed:	cly				; Calculate VCE clock speed.
		cmp	#36			; Is resolution <= 288?
		bcc	.got_speed
		iny
		cmp	#48			; Is resolution <= 384?
		bcc	.got_speed
		iny

.got_speed:	lda	.hds_tbl, y
		clc				; Subtract hdw+1
		sbc	<.hdw
		lsr	a
		sta	<.hds

		lda	.hde_tbl, y
		clc				; Subtract hdw+1
		sbc	<.hdw
		sbc	<.hds
		sta	<.hde

		php				; Disable interrupts.
		sei

		tya				; 0=5MHz, 1=7MHz, 2=10MHz.
		ora	<_bl			; SOFT setting.
		bmi	.set_hsr_hdr		; Keep the current VCE clock?

		sta	vce_cr			; No SGX shadow for this!
		sta	VCE_CR			; Set the VCE clock speed.
		and	#2			; Is this VCE_CR_10MHz?
		beq	.mwr_shadow		; VDC_MWR_1CYCLE when <= 7MHz.
		lda	#VDC_MWR_2CYCLE		; VDC_MWR_2CYCLE when == 10MHz.
.mwr_shadow:	sta	vdc_mwr			; No SGX shadow for this!

.set_hsr_hdr:	lda	#VDC_HSR		; Set the VDC's HSR register.
		sta	<vdc_reg, x
		sta	VDC_AR, x
		lda	.hsw_tbl, y
		sta	VDC_DL, x
		lda	<.hds
		sta	VDC_DH, x

		lda	#VDC_HDR		; Set the VDC's HDR register.
		sta	<vdc_reg, x
		sta	VDC_AR, x
		lda	<.hdw
		sta	VDC_DL, x
		lda	<.hde
		sta	VDC_DH, x

		plp				; Restore interrupts.

!exit:		leave				; All done!

.hsw_tbl:	.db	2, 3, 5
.hds_tbl:	.db	36,50,86
.hde_tbl:	.db	38,51,78

		.endp

		.endprocgroup	; set_xres_group



; ***************************************************************************
; ***************************************************************************
;
; HuC VRAM Functions
;
; ***************************************************************************
; ***************************************************************************


load_vram_group	.procgroup			; These routines share code!

; ***************************************************************************
; ***************************************************************************
;
; void __fastcall load_vram( unsigned int vram<_di>, unsigned char __far *data<_bp_bank:_bp>, unsigned int num_words<_ax> );
; void __fastcall sgx_load_vram( unsigned int vram<_di>, unsigned char __far *data<_bp_bank:_bp>, unsigned int num_words<_ax> );
;
; void __fastcall far_load_vram( unsigned int vram<_di>, unsigned int num_words<_ax> );
; void __fastcall sgx_far_load_vram( unsigned int vram<_di>, unsigned int num_words<_ax> );
;
; load_vram_sgx -  copy a block of memory to VRAM
; load_vram_vdc -  copy a block of memory to VRAM
;
; _bp		= BAT memory location
; _bp_bank	= BAT bank
; _di		= VRAM base address
; _ax		= nb of words to copy
; ----
; N.B. BAT data *must* be word-aligned!

	.ifndef	VRAM_XFER_SIZE
VRAM_XFER_SIZE	=	16
	.endif

load_vram_x	.proc

		tma3
		pha
		tma4
		pha

		ldy	<_bp_bank
		jsr	map_bp_to_mpr34		; Map data to MPR3 & MPR4.

		jsr	set_di_to_mawr

;		tii	.vdc_tai, ram_tia, 8

	.if	SUPPORT_SGX
		txa				; Select which VDC to write
		inc	a			; to.
		inc	a
		sta.l	ram_tia_dst
	.endif

		lda	#VRAM_XFER_SIZE		; Split into 16-byte chunks
		sta.l	ram_tia_len		; for stable IRQ response.

		ldx.l	<_bp
		stx.l	ram_tia_src
		ldy.h	<_bp
		sty.h	ram_tia_src

		lda.l	<_ax			; Length in words.
		pha				; Preserve length.l

		lsr.h	<_ax
		ror	a
		lsr.h	<_ax
		ror	a
		lsr.h	<_ax
		ror	a
	.if	VRAM_XFER_SIZE == 32
		lsr.h	<_ax
		ror	a
	.endif

		sax				; x=chunks-lo
		beq	.next_block		; a=source-lo, y=source-hi

.chunk_loop:	jsr	ram_tia			; transfer 16-bytes

		clc				; increment source
		adc	#VRAM_XFER_SIZE
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

.next_block:	dec.h	<_ax
		bpl	.chunk_loop

		pla				; Restore length.l
		and	#VRAM_XFER_SIZE / 2 - 1
		beq	.done

		asl	a			; Convert words to bytes.
		sta.l	ram_tia_len

		jsr	ram_tia			; transfer remainder

.done:		pla
		tam4
		pla
		tam3

		leave

		.endp

		.endprocgroup	; load_vram_group



; ***************************************************************************
; ***************************************************************************
;
; void __fastcall load_bat( unsigned int vram<_di>, unsigned char __far *data<_bp_bank:_bp>, unsigned char tiles_w<_al>, unsigned char tiles_h<_ah> );
; void __fastcall sgx_load_bat( unsigned int vram<_di>, unsigned char __far *data<_bp_bank:_bp>, unsigned char tiles_w<_al>, unsigned char tiles_h<_ah> );
;
; void __fastcall far_load_bat( unsigned int vram<_di>, unsigned char tiles_w<_al>, unsigned char tiles_h<_ah> );
; void __fastcall sgx_far_load_bat( unsigned int vram<_di>, unsigned char tiles_w<_al>, unsigned char tiles_h<_ah> );
;
; load_bat_sgx - transfer a BAT to VRAM
; load_bat_vdc - transfer a BAT to VRAM
;
; transfer a BAT to VRAM
; ----
; _bp		= BAT memory location
; _bp_bank	= BAT bank
; _di		= VRAM base address
; _al		= nb of column to copy
; _ah		= nb of row
; ----
; N.B. BAT data *must* be word-aligned!

_gfx_load_bat_PARM_2	=	_bp
_gfx_load_bat_PARM_3	=	_di
_gfx_load_bat_PARM_4	=	_al
_gfx_load_bat_PARM_5	=	_ah

load_bat_group	.procgroup			; These routines share code!

	.if	SUPPORT_SGX
		.proc	_sgx_load_bat.4
		.alias	_sgx_far_load_bat.3	= _sgx_load_bat.4

		ldx	#SGX_VDC_OFFSET		; Offset to SGX VDC.
		db	$F0			; Turn "clx" into a "beq".

		.ref	_load_bat.4
		.endp
	.endif

		.proc	_load_bat.4
		.alias	_far_load_bat.3		= _load_bat.4

		clx				; Offset to PCE VDC.

		tma3
		pha

		ldy	<_bp_bank
		jsr	map_bp_to_mpr3		; Map data to MPR3.

		ldy.l	<_bp
		stz.l	<_bp

.line_loop:	jsr	set_di_to_mawr

		lda	<_al
		sta	<__temp
.tile_loop:	lda	[_bp], y
		sta	VDC_DL, x
		iny
		lda	[_bp], y
		sta	VDC_DH, x
		iny
		bne	!+
		jsr	inc.h_bp_mpr3
!:		dec	<__temp
		bne	.tile_loop

		lda	vdc_bat_width, x
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

		.endprocgroup	; load_bat_group



; ***************************************************************************
; ***************************************************************************
;
; void __fastcall load_palette( unsigned char palette<_al>, unsigned char __far *data<_bp_bank:_bp>, unsigned char num_palettes<_ah> );
;
; void __fastcall far_load_palette( unsigned char palette<_al>, unsigned char num_palettes<_ah> );

		.proc	_load_palette.3
		.alias	_far_load_palette.2	= _load_palette.3

		ldy	color_queue_w		; Get the queue's write index.

		lda.l	<_bp			; Add this set of palettes to
		sta	color_addr_l, y		; the queue.
		lda.h	<_bp
		sta	color_addr_h, y
		lda	<_bp_bank
		sta	color_bank, y
		lda	<_al
		sta	color_index, y
		lda	<_ah
		sta	color_count, y

		iny				; Increment the queue index.
		tya
		and	#7

.wait:		cmp	color_queue_r		; If the queue is full, wait
		beq	.wait			; for the next VBLANK.

		sta	color_queue_w		; Signal item is in the queue.

		leave				; All done, phew!

		.endp



	.if	0

_gfx_load_palette_PARM_2	=	_bp
_gfx_load_palette_PARM_3	=	_di
_gfx_load_palette_PARM_4	=	_al
_gfx_load_palette_PARM_5	=	_ah
_gfx_load_palette		.alias	load_palette

;		tay
;		jmp	_load_palette

; ----
; _gfx_load_vram
; ----

_gfx_load_vram_PARM_2		=	_bp
_gfx_load_vram_PARM_3		=	_di
_gfx_load_vram_PARM_4		=	_ax
_gfx_load_vram			.alias	load_vram_vdc

_gfx_load_vram:
		tay
		jmp	load_vram

	.endif



; ***************************************************************************
; ***************************************************************************
;
; void __fastcall load_background( unsigned char __far *tiles<_bp_bank:_bp>, unsigned char __far *palettes<__fbank:__fptr>, unsigned char __far *bat<_cl:_bx>, unsigned char w<_dl>, unsigned char w<_dh> );

		.proc	_load_background.5

		stz.l	<_di
		lda.h	#$1000
		sta.h	<_di
		stz.l	<_ax
		lda.h	#$4000
		sta.h	<_ax
		clx
		call	load_vram_x

		lda.l	<__fptr
		sta.l	<_bp
		lda.h	<__fptr
		sta.h	<_bp
		lda	<__fbank
		sta	<_bp_bank
		stz	<_al
		lda	#16
		sta	<_ah
		call	_load_palette.3

		jsr	wait_vsync

		stz.l	<_di
		stz.h	<_di
		lda.l	<_bx
		sta.l	<_bp
		lda.h	<_bx
		sta.h	<_bp
		lda	<_cl
		sta	<_bp_bank
		lda	<_dl
		sta	<_al
		lda	<_dh
		sta	<_ah
		jmp	_load_bat.4

		.endp



; ***************************************************************************
; ***************************************************************************
;
; void __fastcall vram2vram( unsigned int vram_dst<_ax>, unsigned int vram_src<_bx>, unsigned int word_len<_cx> );
; void __fastcall sgx_vram2vram( unsigned int vram_dst<_ax>, unsigned int vram_src<_bx>, unsigned int word_len<_cx> );
;

	.if	SUPPORT_SGX
		.proc	_sgx_vram2vram.3

		ldx	#SGX_VDC_OFFSET		; Offset to SGX VDC.
		db	$F0			; Turn "clx" into a "beq".

		.ref	_vram2vram.3		; Need _vram2vram
		.endp
	.endif

		.proc	_vram2vram.3

		clx				; Offset to PCE VDC.

		php
		sei
		lda	#VDC_DESR
		sta	VDC_AR, x
		lda.l	<_ax
		sta	VDC_DL, x
		lda.h	<_ax
		sta	VDC_DH, x

		lda	#VDC_SOUR
		sta	VDC_AR, x
		lda.l	<_bx
		sta	VDC_DL, x
		lda.h	<_bx
		sta	VDC_DH, x

		lda	#VDC_LENR
		sta	VDC_AR, x
		clc
		lda.l	<_cx
		adc	#$FF
		sta	VDC_DL, x
		lda.h	<_cx
		adc	#$FF
		sta	VDC_DH, x

		lda	<vdc_reg, x
		sta	VDC_AR
		plp

		leave

		.endp



; ***************************************************************************
; ***************************************************************************
;
; HuC Font Functions
;
; ***************************************************************************
; ***************************************************************************



		.bss

; **************
; 16-bytes of VDC BAT information.
;
; N.B. MUST be 16-bytes before the SGX versions to use PCE_VDC_OFFSET.

_vdc_font_base:	ds	2	; Tile number of ASCII '\0'.

; From hucc-printf.asm just to avoid wasting .bss space with padding.
_vdc_tty_x_lhs:	ds	1	; TTY minimum X position.
_vdc_tty_y_top:	ds	1	; TTY minimum Y position.
_vdc_tty_x:	ds	1	; TTY current X position.
_vdc_tty_y:	ds	1	; TTY current Y position.

	.if	SUPPORT_SGX

; **************
; 16-bytes of SGX BAT information.
;
; N.B. MUST be 16-bytes after the VDC versions to use SGX_VDC_OFFSET.

		ds	10	; Padding to ensure the 16-byte delta.

_sgx_font_base:	ds	2	; Tile number of ASCII '\0'.

; From hucc-printf.asm just to avoid wasting .bss space with padding.
_sgx_tty_x_lhs:	ds	1	; TTY minimum X position.
_sgx_tty_y_top:	ds	1	; TTY minimum Y position.
_sgx_tty_x:	ds	1	; TTY current X position.
_sgx_tty_y:	ds	1	; TTY current Y position.

	.endif

;_font_base	.alias	vdc_font_base

		.code



; ***************************************************************************
; ***************************************************************************
;
; void __fastcall set_font_addr( unsigned int vram<acc> );
; void __fastcall sgx_set_font_addr( unsigned int vram<acc> );

	.if	SUPPORT_SGX
_sgx_set_font_addr.1:
		ldx	#SGX_VDC_OFFSET		; Offset to SGX VDC.
		db	$F0			; Turn "clx" into a "beq".
	.endif

_set_font_addr.1:
		clx				; Offset to PCE VDC.

set_font_addr:	sty	<__temp
		lsr	<__temp
		ror	a
		lsr	<__temp
		ror	a
		lsr	<__temp
		ror	a
		lsr	<__temp
		ror	a
		sec
		sbc	#$20
		sta.l	_vdc_font_base,x
		bcs	!+
		dec	<__temp

!:		lda.h	_vdc_font_base, x
		and	#$F0
		ora	<__temp
		sta.h	_vdc_font_base, x
		rts



; ***************************************************************************
; ***************************************************************************
;
; void __fastcall set_font_pal( unsigned char palette<acc> );
; void __fastcall sgx_set_font_pal( unsigned char palette<acc> );

	.if	SUPPORT_SGX
_sgx_set_font_pal.1:
		ldx	#SGX_VDC_OFFSET		; Offset to SGX VDC.
		db	$F0			; Turn "clx" into a "beq".
	.endif

_set_font_pal:					; For compatibility with HuC.
_set_font_pal.1:
		clx				; Offset to PCE VDC.
		asl	a
		asl	a
		asl	a
		asl	a
		sta	<__temp

		lda.h	_vdc_font_base, x
		and	#$0F
		ora	<__temp
		sta.h	_vdc_font_base, x
		rts



; ***************************************************************************
; ***************************************************************************
;
; void __fastcall load_font( char far *font<_bp_bank:_bp>, unsigned char count<_al> );
;
; void __fastcall load_font( char far *font<_bp_bank:_bp>, unsigned char count<_al>, unsigned int vram<acc> );
; void __fastcall sgx_load_font( char far *font<_bp_bank:_bp>, unsigned char count<_al>, unsigned int vram<acc> );
;
; void __fastcall far_load_font( unsigned char count<_al>, unsigned int vram<acc> );
; void __fastcall sgx_far_load_font( unsigned char count<_al>, unsigned int vram<acc> );

_load_font.2:	ldy	vdc_bat_limit		; Load the font directly
		iny				; after the BAT (stupid!).
		cla
		bra	_load_font.3

	.if	SUPPORT_SGX
_sgx_load_font.3:
		ldx	#SGX_VDC_OFFSET		; Offset to SGX VDC.
		db	$F0			; Turn "clx" into a "beq".
	.endif

_load_font.3:	clx				; Offset to PCE VDC.

		sta.l	<_di			; Load the font directly
		sty.h	<_di			; after the BAT (stupid!).

		jsr	set_font_addr		; Set xxx_font_base from addr.

		lda	<__al			; Convert #tiles into #words.
		stz	<__ah
		asl	a
		rol	<__ah
		asl	a
		rol	<__ah
		asl	a
		rol	<__ah
		asl	a
		rol	<__ah
		sta	<__al
		jmp	load_vram_x

		.alias	_far_load_font.2	= _load_font.3
		.alias	_sgx_far_load_font.2	= _sgx_load_font.3



; ***************************************************************************
; ***************************************************************************
;
; void __fastcall cls();
; void __fastcall sgx_cls();
;
; void __fastcall cls( int tile<acc> );
; void __fastcall sgx_cls( int tile<acc> );

	.if	SUPPORT_SGX
_sgx_cls:	ldx	#SGX_VDC_OFFSET		; Offset to SGX VDC.
		db	$F0			; Turn "clx" into a "beq".
	.endif

_cls:		clx				; Offset to PCE VDC.

clear_tty_x:	lda.l	_vdc_font_base, x
		ldy.h	_vdc_font_base, x
		clc
		adc	#' '
		bcc	!+
		iny
		bra	!+

	.if	SUPPORT_SGX
_sgx_cls.1:	ldx	#SGX_VDC_OFFSET		; Offset to SGX VDC.
		db	$F0			; Turn "clx" into a "beq".
	.endif

_cls.1:		clx

!:		sta.l	<_ax			; VRAM word to write.
		sty.h	<_ax
		lda	vdc_bat_limit, x	; BAT size hi-byte.
		inc	a
		sta	<_bl
		jmp	clear_bat_x



; ***************************************************************************
; ***************************************************************************
;
; void __fastcall __macro load_default_font( void );
; void __fastcall __macro sgx_load_default_font( void );
;
; Args: _bp, _bp_bank = _farptr to font data mapped into MPR3 & MPR4.
; Args: _di = VRAM destination address.
; Args: monofont_fg = font color (0..15)
; Args: monofont_bg = background color (0..15)
; Args: _al = number of tiles (aka characters) 0==256

huc_monofont_x	.proc

		.bss
monofont_fg:	.ds	1
monofont_bg:	.ds	1
		.code

		ldy	vdc_bat_limit, x	; BAT limit mask hi-byte.
		iny
		cla
		sta.l	<_di			; Load the font directly
		sty.h	<_di			; after the BAT (stupid!).

		jsr	set_font_addr		; Set xxx_font_base from addr.

		lda.l	#.font
		sta.l	<_bp
		lda.h	#.font
		sta.h	<_bp

		lda	#$60			; #characters.
		sta	<_al

		jsr	set_di_to_mawr

		lda	monofont_fg		; Foreground pixel color.
		sta	<__temp
		lda	monofont_bg		; Background pixel color.
		phx
		ldx.l	#_cx			; Create a bit mask for each
.bg_loop:	stz	$2000, x		; plane of the background.
		lsr	a
		bcc	.bg_plane
		dec	$2000, x
.bg_plane:	inx
		bne	.bg_loop
		plx

.tile_loop:	cly

.plane01:	lda	[_bp], y		; Get font byte.
		bbs0	<__temp, .set_plane0
.clr_plane0:	eor	#$FF			; Clr font bits in background.
		and	<_cx + 0
		bra	.put_plane0
.set_plane0:	ora	<_cx + 0		; Set font bits in background.
.put_plane0:	sta	VDC_DL, x

		lda	[_bp], y		; Get font byte.
		bbs1	<__temp, .set_plane1
.clr_plane1:	eor	#$FF			; Clr font bits in background.
		and	<_cx + 1
		bra	.put_plane1
.set_plane1:	ora	<_cx + 1		; Set font bits in background.
.put_plane1:	sta	VDC_DH, x

		iny
		cpy	#8
		bcc	.plane01

		cly

.plane23:	lda	[_bp], y		; Get font byte.
		bbs2	<__temp, .set_plane2
.clr_plane2:	eor	#$FF			; Clr font bits in background.
		and	<_cx + 2
		bra	.put_plane2
.set_plane2:	ora	<_cx + 2		; Set font bits in background.
.put_plane2:	sta	VDC_DL, x

		lda	[_bp], y		; Get font byte.
		bbs3	<__temp, .set_plane3
.clr_plane3:	eor	#$FF			; Clr font bits in background.
		and	<_cx + 3
		bra	.put_plane3
.set_plane3:	ora	<_cx + 3		; Set font bits in background.
.put_plane3:	sta	VDC_DH, x

		iny
		cpy	#8
		bcc	.plane23

		lda.l	<_bp
		adc	#8-1
		sta.l	<_bp
		bcc	!+
		inc.h	<_bp

!:		dec	<_al
		bne	.tile_loop

		leave				; All done, phew!

.font:		incbin	"data/font8x8-bold-short-iso646-fr.dat", 128

		.endp



; ***************************************************************************
; ***************************************************************************
;
; HuC Text Output
;
; ***************************************************************************
; ***************************************************************************



vdc_tty_out	.procgroup			; These routines share code!

; ***************************************************************************
; ***************************************************************************
;
; void __fastcall put_char( unsigned char digit<_bl>, unsigned char bat_x<_dil>, unsigned char bat_y<_dih> );

	.if	SUPPORT_SGX
put_char_sgx	.proc

		ldx	#SGX_VDC_OFFSET		; Offset to SGX VDC.
		db	$F0			; Turn "clx" into a "beq".

		.ref	put_char_vdc		; Need put_char_vdc
		.endp
	.endif

put_char_vdc	.proc

		clx				; Offset to PCE VDC.

		jsr	put_di_xy_wrap		; Comply with HuC's wrapping.

		cla				; Push EOL marker.
		pha

		lda	<_bl
		pha				; Push character to output.
		bra	!output+

		.ref	put_hex_vdc		; Need put_number_vdc
		.endp

		.alias	_put_char.3		= put_char_vdc



; ***************************************************************************
; ***************************************************************************
;
; void __fastcall put_digit( unsigned char digit<_bl>, unsigned char bat_x<_dil>, unsigned char bat_y<_dih> );

	.if	SUPPORT_SGX
put_digit_sgx	.proc

		ldx	#SGX_VDC_OFFSET		; Offset to SGX VDC.
		db	$F0			; Turn "clx" into a "beq".

		.ref	put_digit_vdc		; Need put_digit_vdc
		.endp
	.endif

put_digit_vdc	.proc

		clx				; Offset to PCE VDC.

		jsr	put_di_xy_wrap		; Comply with HuC's wrapping.

		cla				; Push EOL marker.
		pha

		lda	<_bl			; Convert hex digit to ASCII.
		and	#$0F
		cmp	#10
		bcc	!+
		adc	#6
!:		adc	#'0'
		pha				; Push character to output.
		bra	!output+

		.ref	put_hex_vdc		; Need put_number_vdc
		.endp

		.alias	_put_digit.3		= put_digit_vdc



; ***************************************************************************
; ***************************************************************************
;
; void __fastcall put_hex( unsigned int number<_bx>, unsigned char length<_cl>, unsigned char bat_x<_dil>, unsigned char bat_y<_dih> );

	.if	SUPPORT_SGX
put_hex_sgx	.proc

		ldx	#SGX_VDC_OFFSET		; Offset to SGX VDC.
		db	$F0			; Turn "clx" into a "beq".

		.endp
	.endif

put_hex_vdc	.proc

		clx				; Offset to PCE VDC.

		jsr	put_di_xy_wrap		; Comply with HuC's wrapping.

		ldy	<_cl			; Total #characters to print,
		beq	!exit+			; NOT minimum #characters!

		stx	<__temp			; Preserve which VDC.

		clx				; Push EOL marker.
		phx

.hex_byte:	lda.l	<_bx, x			; Convert hex digit to ASCII.
		and	#$0F
		cmp	#10
		bcc	!+
		adc	#6
!:		adc	#'0'
		pha				; Push character to output.
		dey
		beq	.hex_done

		lda.l	<_bx, x			; Convert hex digit to ASCII.
		lsr	a
		lsr	a
		lsr	a
		lsr	a
		cmp	#10
		bcc	!+
		adc	#6
!:		adc	#'0'
		pha				; Push character to output.
		dey
		beq	.hex_done

		inx
		bra	.hex_byte

.hex_done:	ldx	<__temp			; Restore which VDC.
		bra	!output+

.write:		clc
		adc.l	_vdc_font_base, x
		sta	VDC_DL, x
		cla
		adc.h	_vdc_font_base, x
		sta	VDC_DH, x

!output:	pla				; Pop the digits and output.
		bne	.write

!exit:		leave				; All done!

		.endp

		.alias	_put_hex.4 = put_hex_vdc



; ***************************************************************************
; ***************************************************************************
;
; void __fastcall put_number( unsigned int number<_bx>, unsigned char length<_cl>, unsigned char bat_x<_dil>, unsigned char bat_y<_dih> );

	.if	SUPPORT_SGX
put_number_sgx	.proc

		ldx	#SGX_VDC_OFFSET		; Offset to SGX VDC.
		db	$F0			; Turn "clx" into a "beq".

		.endp
	.endif

put_number_vdc	.proc

		clx				; Offset to PCE VDC.

		jsr	put_di_xy_wrap		; Comply with HuC's wrapping.

		ldy	<_cl			; Total #characters to print,
		beq	!exit-			; NOT minimum #characters!

		stx	<__temp			; Preserve which VDC.

		clx				; Push EOL marker.
		phx

		ldx.h	<_bx			; Is the number -ve?
		stx	<_cl			; Remember this.
		bpl	.divide_by_ten

		sec				; Make the number +ve.
		lda.l	<_bx
		eor	#$FF
		adc	#0
		sta.l	<_bx
		txa
		eor	#$FF
		adc	#0
		sta.h	<_bx

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
		pha				; Push character to output.
		dey
		beq	!pad+
		lda.l	<_bx			; Repeat while non-zero.
		ora.h	<_bx
		bne	.divide_by_ten

		ldx	<__temp			; Restore which VDC.

		lda	<_cl			; Was the number -ve?
		bpl	!pad+
		lda	#'-'			; Output a leading '-'.
		pha
		dey

!pad:		lda	#' '			; Add padding characters.
.loop:		dey
		bmi	!output-
		pha
		bra	.loop

		.ref	put_hex_vdc		; Need put_number_vdc

		.endp

		.alias	_put_number.4 = put_number_vdc



; ***************************************************************************
; ***************************************************************************
;
; void __fastcall put_raw( unsigned int data<_bx>, unsigned char bat_x<_dil>, unsigned char bat_y<_dih> );

	.if	SUPPORT_SGX
put_raw_sgx	.proc

		ldx	#SGX_VDC_OFFSET		; Offset to SGX VDC.
		db	$F0			; Turn "clx" into a "beq".

		.endp
	.endif

put_raw_vdc	.proc

		clx				; Offset to PCE VDC.

		jsr	put_di_xy_wrap		; Comply with HuC's wrapping.

		lda.l	<_bx
		sta	VDC_DL, x
		lda.h	<_bx
		sta	VDC_DH, x

		leave				; All done!

		.endp

		.alias	_put_raw.3		= put_raw_vdc

		;
		; Ugly hack to wrap the X & Y coordinates like HuC.
		;

put_di_xy_wrap:	lda.l	<_di			; Sanitize X coordinate.
		and	vdc_bat_x_mask, x
		sta.l	<_di

		lda.h	<_di			; Sanitize Y coordinate.
		and	vdc_bat_y_mask, x
		sta.h	<_di

		jmp	set_di_xy_mawr		; In "common.asm".

		.endprocgroup			; vdc_tty_out



; ***************************************************************************
; ***************************************************************************
;
; void __fastcall put_string( unsigned char *string<_bp>, unsigned char bat_x<_dil>, unsigned char bat_y<_dih> );
;
; N.B. This is not a .proc right now because it is called from procedures
; that contain embedded strings, and the string aren't banked in before
; printing (yet).

	.if	SUPPORT_SGX
_sgx_put_string.3:
		ldx	#SGX_VDC_OFFSET		; Offset to SGX VDC.
		db	$F0			; Turn "clx" into a "beq".
	.endif

_put_string.3:	clx				; Offset to PCE VDC.

		jsr	set_di_xy_mawr

		cly
.chr_loop:	lda	[_bp], y
		beq	.done

		clc
		adc.l	_vdc_font_base, x
		sta	VDC_DL, x
		cla
		adc.h	_vdc_font_base, x
		sta	VDC_DH, x

		iny
		bne	.chr_loop
		inc.h	<_bp
		bra	.chr_loop

.done:		rts



; ***************************************************************************
; ***************************************************************************
;
; void __fastcall set_color_rgb( unsigned int index<VCE_CTA>, unsigned char r<_al>, unsigned char g<_ah>, unsigned char b<acc> );
;
; r:	red	RED:	bit 3-5
; g:	green	GREEN:	bit 6-8
; b:	blue	BLUE:	bit 0-2

_set_color_rgb.4:
;		and	#7
		sta	<__temp
		lda	<_al
;		and	#7
		asl	a
		asl	a
		asl	a
		ora	<__temp
		asl	a
		asl	a
		sta	<__temp
		lda	<_ah
;		and	#7
		lsr	a
		ror	<__temp
		lsr	a
		ror	<__temp
		tay
		lda	<__temp
		sta.l	VCE_CTW
		sty.h	VCE_CTW
		rts
