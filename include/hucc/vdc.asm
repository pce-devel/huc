; ***************************************************************************
; ***************************************************************************
;
; vdc.asm
;
; Useful routines for operating the HuC6270 Video Display Controller.
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
; Include dependancies ...
;

		include "common.asm"		; Common helpers.
		include "vce.asm"		; Useful VCE routines.

;
; Choose how much to transfer to VRAM in a single chunk, normally 16-bytes.
;
; The cycle timings for a TIA-to-VRAM depend upon how the VDC's MWR CPU slots
; line up to the CPU's writes, and how long the VDC has to halt the CPU while
; it fetches the next scanline's sprite data.
;
; These cycle timings are for 0 sprites (best) and 16 sprites (worst) ...
;
; 32-byte TIA takes 270..364 cycles in 5MHz, 242..312 cycles in 7MHz. (8.44 cycles-per-byte best-case at 5MHz.)
; 24-byte TIA takes 210..298 cycles in 5MHz, 186..256 cycles in 7MHz. (8.75 cycles-per-byte best-case at 5MHz.)
; 16-byte TIA takes 142..234 cycles in 5MHz, 128..200 cycles in 7MHz. (8.88 cycles-per-byte best-case at 5MHz.)
;
; If a user wishes to be able to put RCR interrupts one-line-after-another,
; then it is only safe to use 32-byte chunks if there are no TIMER or IRQ2
; interrupts ... which is almost-impossible to rely on in library code!
;

	.ifndef	VRAM_XFER_SIZE
VRAM_XFER_SIZE	=	16
	.endif

;
; Enable BG & SPR layers, and RCR interrupt.
;

set_rcron:	lda	#$04			; Enable RCR interrupt.
		bra	!+

set_bgon:	lda	#$80			; Enable BG layer.
		bra	!+

set_spron:	lda	#$40			; Enable SPR layer.
		bra	!+

set_dspon:	lda	#$C0			; Enable BG & SPR layers.

!:		tsb	<vdc_crl		; These take effect when
	.if	SUPPORT_SGX			; the next VBLANK occurs.
		tsb	<sgx_crl
	.endif
		rts

;
; Disable BG & SPR layers, and RCR interrupt.
;

set_rcroff:	lda	#$04			; Disable RCR interrupt.
		bra	!+

set_bgoff:	lda	#$80			; Disable BG layer.
		bra	!+

set_sproff:	lda	#$40			; Disable SPR layer.
		bra	!+

set_dspoff:	lda	#$C0			; Disable BG & SPR layers.

!:		trb	<vdc_crl		; These take effect when
	.if	SUPPORT_SGX			; the next VBLANK occurs.
		trb	<sgx_crl
	.endif
		rts



; ***************************************************************************
; ***************************************************************************
;
; Put the _di data pointer into the VDC's MARR or MAWR register.
;
; N.B. Library code relies on this preserving Y!
;
; Args: _di + 0 = BAT X coordinate.
; Args: _di + 1 = BAT Y coordinate.
;
; Here because it relies on the "vdc_bat_width" that is defined in this file.
;

	.if	SUPPORT_SGX
sgx_di_xy_marr:	ldx	#SGX_VDC_OFFSET		; Offset to SGX VDC.
		db	$F0			; Turn "clx" into a "beq".
	.endif

vdc_di_xy_marr:	clx				; Offset to PCE VDC.

set_di_xy_mawr:	cla
		bit	vdc_bat_width, x	; Set by set_bat_size().
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
		jmp	set_di_to_mawr		; In "common.asm".



vdc_clear_vram	.procgroup			; These routines share code!

; ***************************************************************************
; ***************************************************************************
;
; clear_vram_sgx - Clear all of VRAM in the SGX VDC.
; clear_vram_vdc - Clear all of VRAM in the PCE VDC.
;
; Args: _ax = word value to write to the BAT.
; Args: _bl = hi-byte of size of BAT (# of words).
;

	.if	SUPPORT_SGX
clear_vram_sgx	.proc

		ldx	#SGX_VDC_OFFSET		; Offset to SGX VDC.
		db	$F0			; Turn "clx" into a "beq".

		.ref	clear_vram_vdc		; Need clear_vram_vdc
		.endp
	.endif

clear_vram_vdc	.proc

		clx				; Offset to PCE VDC.

clear_vram_x:	call	clear_bat_x		; Clear the BAT.

		lda	#$80			; Xvert hi-byte of # words
		sec				; in screen to loop count.
		sbc	<_bl
		lsr	a

;		cly				; Clear the rest of VRAM.
		stz	VDC_DL, x
.clr_loop:	stz	VDC_DH, x		; Seperate writes to minimize
		dey				; VDC MWR penalty.
		stz	VDC_DH, x
		bne	.clr_loop
		dec	a
		bne	.clr_loop

		leave				; All done, phew!

		.endp



; ***************************************************************************
; ***************************************************************************
;
; clear_bat_sgx - Clear the BAT in the SGX VDC.
; clear_bat_vdc - Clear the BAT in the PCE VDC.
;
; Args: _ax = word value to write to the BAT.
; Args: _bl = hi-byte of size of BAT (# of words).
;

	.if	SUPPORT_SGX
clear_bat_sgx	.proc

		ldx	#SGX_VDC_OFFSET		; Offset to SGX VDC.
		db	$F0			; Turn "clx" into a "beq".

		.ref	clear_bat_vdc		; Need clear_bat_vdc
		.endp
	.endif

clear_bat_vdc	.proc

		clx				; Offset to PCE VDC.

		.ref	clear_bat_x		; Need clear_bat_x
		.endp

clear_bat_x	.proc				; HuCC uses this entry point.

		stz	<_di + 0		; Set VDC or SGX destination
		stz	<_di + 1		; address.
		jsr	set_di_to_mawr

		lda	<_bl			; Xvert hi-byte of # words
		lsr	a			; in screen to loop count.

		cly
.bat_loop:	pha
		lda	<_ax + 0
		sta	VDC_DL, x
		lda	<_ax + 1
.bat_pair:	sta	VDC_DH, x		; Seperate writes to minimize
		dey				; VDC MWR penalty.
		sta	VDC_DH, x
		bne	.bat_pair

		pla
		dec	a
		bne	.bat_loop

		leave

		.endp

		.endprocgroup

;
;
;

vdc_set_mode	.procgroup			; These routines share code!

; ***************************************************************************
; ***************************************************************************
;
; set_mode_sgx - Set video hardware registers from a data table.
; set_mode_vdc - Set video hardware registers from a data table.
;
; Args: _bp, Y = _farptr to data table mapped into MPR3 & MPR4.
;

	.if	SUPPORT_SGX
set_mode_sgx	.proc

		ldx	#SGX_VDC_OFFSET		; Offset to SGX VDC.
		db	$F0			; Turn "clx" into a "beq".

		.endp
	.endif

set_mode_vdc	.proc

		clx				; Offset to PCE VDC.

		smb7	<_al			; Signal no set_bat_size() yet.

		tma3				; Preserve MPR3.
		pha
		tma4				; Preserve MPR4.
		pha

		jsr	set_bp_to_mpr34		; Map data to MPR3 & MPR4.

		php				; Disable interrupts.
		sei

		cly				; Table size is < 256 bytes.

.loop:		lda	[_bp], y		; Get the register #, +ve for
		beq	.done			; VDC, -128 for VCE_CR.
		bpl	.set_vdc_reg

		; Set the VCE_CR register.

.set_vce_cr:	iny

		lda	[_bp], y		; Get lo-byte of register.
		iny
		sta	vce_cr			; No SGX shadow for this!
		sta	VCE_CR			; Set the VCE clock speed.
		bra	.loop			; Do not set VDC_MWR reg bits!

		; Set a VDC register.

.set_vdc_reg:	iny
		sta	VDC_AR, x		; Set which VDC register.

		cmp	#VDC_CR			; CS if VDC_CR or higher.
		beq	.skip_cc
		clc				; CC if not VDC_CR.

		eor	#VDC_MWR		; Check if this the VDC_MWR
		bne	.skip_cc		; without changing CC.

		lda	[_bp], y		; Remember the BAT size so that
		sta	vdc_mwr			; set_bat_size() can be called.
		lsr	a
		lsr	a
		lsr	a
		lsr	a
		sta	<_al
		iny
		iny
		bra	.loop

.skip_cc:	lda	[_bp], y		; Get lo-byte of register.
		iny
		bcc	.not_vdc_cr

	.if	SUPPORT_SGX
		cpx	#0			; Writing to the VDC or SGX?
		beq	.save_crl
		and	#$F7			; We only need 1 vblank IRQ!
	.endif

.save_crl:	sta	<vdc_crl, x		; Save VDC_CR shadow register.

.not_vdc_cr:	sta	VDC_DL, x		; Write to VDC.

		lda	[_bp], y		; Get hi-byte of register.
		iny
		sta	VDC_DH, x
		bcc	.loop			; Next register, please!

		sta	<vdc_crh, x		; Save VDC_CR shadow register.

		bra	.loop			; Next register, please!

		; All registers set!

.done:		lda	#VDC_VWR		; Leave with VDC_VWR set.
		sta	<vdc_reg, x
;		lda	<vdc_reg, x		; Restore previous VDC_AR from
		sta	VDC_AR, x		; the shadow variable.

		plp				; Restore interrupts.

		pla				; Restore MPR4.
		tam4
		pla				; Restore MPR3.
		tam3

		bbr7	<_al, set_bat_size	; Update if BAT size changed.

		leave				; All done, phew!

		.ref	set_bat_vdc
		.endp



; ***************************************************************************
; ***************************************************************************
;
; set_bat_sgx - Change the SGX BAT size and initialize variables based on it.
; set_bat_vdc - Change the PCE BAT size and initialize variables based on it.
;
; Args: _al = new size (0-7).
;
; (VDC_MWR_32x32  >> 4) or in HuCC, SCR_SIZE_32x32.
; (VDC_MWR_32x64  >> 4) or in HuCC, SCR_SIZE_32x64.
; (VDC_MWR_64x32  >> 4) or in HuCC, SCR_SIZE_64x32.
; (VDC_MWR_64x64  >> 4) or in HuCC, SCR_SIZE_64x64.
; (VDC_MWR_128x32 >> 4) or in HuCC, SCR_SIZE_128x32.
; (VDC_MWR_128x64 >> 4) or in HuCC, SCR_SIZE_128x64.
;

	.if	SUPPORT_SGX
set_bat_sgx	.proc

		ldx	#SGX_VDC_OFFSET		; Offset to SGX VDC.
		db	$F0			; Turn "clx" into a "beq".

		.ref	set_bat_vdc
		.endp
	.endif

set_bat_vdc	.proc

		clx				; Offset to PCE VDC.

set_bat_size:	lda	<_al			; Get BAT size value.
		and	#7			; Sanitize screen size value.
		tay
		asl	a			; Put it in bits 4..6.
		asl	a
		asl	a
		asl	a
		sta	<__temp

		lda	.width, y
		sta	vdc_bat_width, x
		dec	a
		sta	vdc_bat_x_mask, x

		lda	.height, y
		sta	vdc_bat_height, x
		dec	a
		sta	vdc_bat_y_mask, x

		lda	.limit, y
		sta	vdc_bat_limit, x

		lda	.increment, y		; Put the VRAM increment for a
		sta	<vdc_crh, x		; line into vdc_crh for later.

		php
		sei

		lda	#VDC_MWR
		sta	VDC_AR, x

		lda	vdc_mwr			; Get the MWR access width bits.
		and	#%10001111
		ora	<__temp
	.if	SUPPORT_SGX
		cpx	#PCE_VDC_OFFSET		; This has no SGX shadow!
		bne	!+
	.endif
		sta	vdc_mwr
!:		sta	VDC_DL, x

		lda	<vdc_reg, x		; Restore previous VDC_AR from
		sta	VDC_AR, x		; the shadow variable.

		plp

		leave

.width:		db	$20,$40,$80,$80,$20,$40,$80,$80
.height:	db	$20,$20,$20,$20,$40,$40,$40,$40
.limit:		db	$03,$07,$0F,$0F,$07,$0F,$1F,$1F
.increment	db	$08,$10,$18,$18,$08,$10,$18,$18

		.bss

; **************
; 16-bytes of VDC BAT information.
;
; N.B. MUST be 16-bytes before the SGX versions to use PCE_VDC_OFFSET.
;
; N.B. Declared inside this .proc so that they can be stripped if unused.

; Initialized by set_bat_vdc.
vdc_bat_width:	ds	1	; $20, $40, $80
vdc_bat_height:	ds	1	; $20, $40
vdc_bat_x_mask:	ds	1	; $1F, $3F, $7F
vdc_bat_y_mask:	ds	1	; $1F, $3F
vdc_bat_limit:	ds	1	; (>$03FF), (>$07FF), (>$0FFF), (>$1FFF)

; From blkmap.asm just to avoid wasting .bss space with padding.
vdc_map_draw_w:	ds	1	; (SCR_WIDTH / 8) + 1
vdc_map_draw_h:	ds	1	; (SCR_HEIGHT / 8) + 1
vdc_map_line_w:	ds	1	; Line width of map data in tiles.
vdc_map_scrn_w:	ds	1	; Line width of map data in screens.
vdc_map_pxl_x:	ds	2	; Current top-left X in pixels.
vdc_map_pxl_y:	ds	2	; Current top-left Y in pixels.
vdc_map_option:	ds	1	; Flags to disable BAT alignment.

; From hucc-old-spr.asm just to avoid wasting .bss space with padding.
spr_max:	ds	1
spr_clr:	ds	1

	.if	SUPPORT_SGX

; **************
; 16-bytes of SGX BAT information.
;
; N.B. MUST be 16-bytes after the VDC versions to use SGX_VDC_OFFSET.
;
; N.B. Declared inside this .proc so that they can be stripped if unused.

; Initialized by set_bat_sgx.
sgx_bat_width:	ds	1	; $20, $40, $80
sgx_bat_height:	ds	1	; $20, $40
sgx_bat_x_mask:	ds	1	; $1F, $3F, $7F
sgx_bat_y_mask:	ds	1	; $1F, $3F
sgx_bat_limit:	ds	1	; (>$03FF), (>$07FF), (>$0FFF), (>$1FFF)

; From blkmap.asm just to avoid wasting .bss space with padding.
sgx_map_draw_w:	ds	1	; (SCR_WIDTH / 8) + 1
sgx_map_draw_h:	ds	1	; (SCR_HEIGHT / 8) + 1
sgx_map_line_w:	ds	1	; Line width of map data in tiles.
sgx_map_scrn_w:	ds	1	; Line width of map data in screens.
sgx_map_pxl_x:	ds	2	; Current top-left X in pixels.
sgx_map_pxl_y:	ds	2	; Current top-left Y in pixels.
sgx_map_option:	ds	1	; Flags to disable BAT alignment.

; From hucc-old-spr.asm just to avoid wasting .bss space with padding.
sgx_spr_max:	ds	1
sgx_spr_clr:	ds	1

	.endif

		.code

		.endp

		.endprocgroup



; ***************************************************************************
; ***************************************************************************
;
; sgx_detect - Detect whether we're running on a SuperGrafx (and init VPC).
;
; Returns: X,C-flag, and "sgx_detected" = NZ, CS if detected.
;
; ***************************************************************************
;
; https://web.archive.org/web/20161129055659/http://cgfm2.emuviews.com/txt/sgxtech.txt
;
; ***************************************************************************
;
; HuC6202 VIDEO PRIORITY CONTROLLER (huge thanks to Charles MacDonald!)
;
; The VPC has no access to sprite priority data, it can only sort pixels
; based upon which VDC and whether they are "sprite" or "background".
;
; This can sometimes lead to unexpected results with low-priority sprites.
;
; VPC registers $0008 and $0009 make up four 4-bit values that define the
; enabled layers and priority setting for the four possible window areas.
;
; Bits 3-0 of $0008 are for the region where Window 1 and 2 overlap
; Bits 7-4 of $0008 are for the region occupied by only Window 2
; Bits 3-0 of $0009 are for the region occupied by only Window 1
; Bits 7-4 of $0009 are for the region where no Window is present
;
;  Each 4-bit value has the same format:
;
;  Bit 0: VDC #1 graphics are 0=disabled, 1=enabled
;  Bit 1: VDC #2 graphics are 0=disabled, 1=enabled
;  Bit 2: Bit 0 of priority setting
;  Bit 3: Bit 1 of priority setting
;
;   Priority Setting 0b00xx: (useful when VDC #1 is a fullscreen HUD)
;
;    FRONT
;     SP1 = VDC #1 (pce) sprite pixels
;     BG1 = VDC #1 (pce) background pixels
;     SP2 = VDC #2 (sgx) sprite pixels
;     BG2 = VDC #2 (sgx) background pixels
;    BACK
;
;   Priority Setting 0b01xx: (useful for parallax backgrounds)
;
;    FRONT
;     SP1 = VDC #1 (pce) sprite pixels
;     SP2 = VDC #2 (sgx) sprite pixels
;     BG1 = VDC #1 (pce) background pixels
;     BG2 = VDC #2 (sgx) background pixels
;    BACK
;
;   Priority Setting 0b10xx: (only useful for special effects)
;
;    FRONT
;     BG1 = VDC #1 (pce) background pixels (transparent where sprites)
;     BG2 = VDC #2 (sgx) background pixels
;     SP1 = VDC #1 (pce) sprite pixels
;     SP2 = VDC #2 (sgx) sprite pixels
;    BACK

	.if	SUPPORT_SGX
	.if	1
sgx_detect	.proc

		ldy	#$7F			; Use VRAM address $7F7F
		sty.l	<_di			; because it won't cause
		sty.h	<_di			; a screen glitch.

		jsr	sgx_di_to_mawr		; Write $0001 to SGX VRAM.
		ldy	#$01
		sty	SGX_DL
		stz	SGX_DH

		jsr	vdc_di_to_mawr		; Write $0000 to VDC VRAM.
		stz	VDC_DL
		stz	VDC_DH

		jsr	sgx_di_to_marr		; Check value in SGX VRAM.
		ldy	SGX_DL			; $01 if found, $00 if not.
		sty	sgx_detected
		beq	!+			; Skip the rest if not SGX.

		jsr	sgx_di_to_mawr		; Write $0000 to SGX VRAM
		stz	SGX_DL			; to clean VRAM contents.
		stz	SGX_DH

		tii	.vpc_mode, VPC_CR, 8	; Initialize the HuC6202 VPC.

!:		tya
		tax				; "leave" copies X back to A.
		lsr	a			; Also CC if PCE, CS if SGX.

		leave				; All done, phew!
	.else
sgx_detect	.proc

		lda	#$80			; Check the top bit, 1 if SGX
		and	SGX_DETECT		; or 0 if a mirror of VDC_SR.
		beq	!+

		tii	.vpc_mode, VPC_CR, 8	; Initialize the HuC6202 VPC.
		lda	#$01

!:		sta	sgx_detected
		tax				; "leave" copies X back to A.
		lsr	a			; Also CC if PCE, CS if SGX.

		leave				; All done, phew!
	.endif

	.ifndef	SGX_PARALLAX
SGX_PARALLAX	=	1			; The most common default.
	.endif

	.if	SGX_PARALLAX
.vpc_mode:	dw	$7000			; Use SGX as a parallax layer
		dw	$0000			; behind a VDC background.
		dw	$0000
		dw	$0000
	.else
.vpc_mode:	dw	$3000			; Use SGX as the background
		dw	$0000			; behind a fullscreen HUD.
		dw	$0000
		dw	$0000
	.endif	SGX_PARALLAX

		.endp

	.ifndef	CORE_VERSION			; CORE has this in the kernel.
		.bss
sgx_detected:	ds	1			; NZ if SuperGrafx detected.
		.code
	.endif	CORE_VERSION

	.endif	SUPPORT_SGX




vdc_copy_to	.procgroup			; These routines share code!

; ***************************************************************************
; ***************************************************************************
;
; copy_to_sgx - Copy data from CPU memory to VRAM.
; copy_to_vdc - Copy data from CPU memory to VRAM.
;
; Args: _bp, Y = _farptr to data table mapped into MPR3 & MPR4.
; Args: _di = VRAM destination address.
; Args: _ax = Number of VRAM_XFER_SIZE chunks to copy (max 32768).
;
; N.B. This kind of self-modifying spaghetti code is why folks hate asm! ;-)
;

		;
		; Main procedure entry points that set up the registers for the
		; self-modifying subroutine that must be in RAM.
		;

	.if	SUPPORT_SGX
copy_to_sgx	.proc

		ldx	#SGX_VDC_OFFSET		; Offset to SGX VDC.
		db	$F0			; Turn "clx" into a "beq".

		.endp
	.endif

copy_to_vdc	.proc

		clx				; Offset to PCE VDC.

		tma3				; Preserve MPR3.
		pha
		tma4				; Preserve MPR4.
		pha

		jsr	set_bp_to_mpr34		; Map data to MPR3 & MPR4.

	.if	!CDROM
		lda	tia_to_vram		; Is the self-modifying code
		bne	!+			; already in RAM?
		tii	tia_to_vram_rom, tia_to_vram_ram, tia_to_vram_len
	.endif

!:		jsr	set_di_to_mawr		; Set VRAM write address.

	.if	SUPPORT_SGX
		inx				; Set VDC or SGX destination.
		inx
		stx	!modify_tia+ + 3
	.endif

	.if	0
		ldx.l	<_bp			; Source address in CPU RAM.
		ldy.h	<_bp

		lda	#VRAM_XFER_SIZE
		sta	!modify_tia+ + 5	; TIA length lo-byte.

		lda.l	<_ax
	.if	VRAM_XFER_SIZE == 16
		lsr.h	<_ax
		ror	a
	.endif
		lsr.h	<_ax
		ror	a
		lsr.h	<_ax
		ror	a
		lsr.h	<_ax
		ror	a			; Number of chunks (lo-byte).
		sax
	.else
		lda.l	<_bp			; Source address in CPU RAM.
		ldy.h	<_bp
		ldx.l	<_ax			; Number of chunks (lo-byte).
	.endif

	.if	CDROM
		bne	tia_to_vram		; On CD-ROM, the code is close.
	.else
		bne	.execute		; On HuCARD, the code is far.
	.endif

		dec.h	<_ax			; Number of chunks (hi-byte).

.execute:	jmp	tia_to_vram		; Now execute the VRAM copy.

		.endp

		;
		; This subroutine does the core of the transfer.
		;
		; Because it self-modifies, it needs to be in RAM on a HuCARD.
		;

	.if	!CDROM
tia_to_vram_rom	.phase	tia_to_vram_ram		; Assemble this to run in RAM.
	.endif

!next_page:	iny				; Increment source page.
		bpl	!same_bank+		; Still in MPR3?
!next_bank:	tay				; Preserve address lo-byte.
		tma4
		tam3
		inc	a
		tam4
		tya				; Restore address lo-byte.
		ldy.h	#$6000			; Wrap address hi-byte.
!same_bank:	dex				; Number of chunks (lo-byte).
		beq	!next_block+		; ... or drop through!

		; The subroutine execution actually starts here!

tia_to_vram:	clc
		sty.h	!modify_tia+ + 1	; TIA source address hi-byte.
!chunk_loop:	sta.l	!modify_tia+ + 1	; TIA source address lo-byte.
!modify_tia:	tia	$1234, VDC_DL, VRAM_XFER_SIZE
		adc	#VRAM_XFER_SIZE		; Increment source address.
		bcs	!next_page-
!same_page:	dex				; Number of chunks (lo-byte).
		bne	!chunk_loop-
!next_block:	dec.h	<_ax			; Number of chunks (hi-byte).
		bpl	tia_to_vram

;		lda.l	<_ax
;		and	#VRAM_XFER_SIZE - 1
;		bne	!last_words+

		pla				; Restore MPR4.
		tam4
		pla				; Restore MPR3.
		tam3

		leave				; All done, phew!

;!last_words:	stz.l	<_ax
;		sta	!modify_tia- + 5	; TIA length lo-byte.
;		inx
;		bra	tia_to_vram

	.if	!CDROM
		.dephase			; Continue assembling in ROM.
tia_to_vram_len =	(* - tia_to_vram_rom)
		.bss
tia_to_vram_ram	ds	tia_to_vram_len		; HuCARD must put code in RAM.
		.code
	.endif

		.endprocgroup



; ***************************************************************************
; ***************************************************************************
;
; init_240x208 - An example of initializing screen and VRAM.
;
; This can be used as-is, or copied to your own program and modified.
;

init_240x208	.proc

.BAT_SIZE	=	32 * 32
.SAT_ADDR	=	.BAT_SIZE		; SAT takes 16 tiles of VRAM.
.CHR_ZERO	=	.BAT_SIZE / 16		; 1st tile # after the BAT.
.CHR_0x10	=	.CHR_ZERO + 16		; 1st tile # after the SAT.
.CHR_0x20	=	.CHR_ZERO + 32		; ASCII ' ' CHR tile #.

		php				; Disable interrupts.
		sei

		call	clear_vce		; Clear all palettes.

		lda.l	#.CHR_0x20		; Tile # of ' ' CHR.
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
		bit	VDC_SR			; Purge any overdue RCR/VBL.
		plp				; Restore interrupts.

		call	wait_vsync		; Wait for the next VBLANK.

		leave				; All done, phew!

		; A reduced 240x208 screen to save VRAM.

.mode_240x208:	db	$80			; VCE Control Register.
		db	VCE_CR_5MHz		; Video Clock

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
		db	VDC_DVSSR		; VRAM->SATB address $0400
		dw	$0400
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
; init_256x224 - An example of initializing screen and VRAM.
;
; This can be used as-is, or copied to your own program and modified.
;

init_256x224	.proc

.BAT_SIZE	=	64 * 32
.SAT_ADDR	=	.BAT_SIZE		; SAT takes 16 tiles of VRAM.
.CHR_ZERO	=	.BAT_SIZE / 16		; 1st tile # after the BAT.
.CHR_0x10	=	.CHR_ZERO + 16		; 1st tile # after the SAT.
.CHR_0x20	=	.CHR_ZERO + 32		; ASCII ' ' CHR tile #.

		php				; Disable interrupts.
		sei

		call	clear_vce		; Clear all palettes.

		lda.l	#.CHR_0x20		; Tile # of ' ' CHR.
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
		db	VDC_DVSSR		; VRAM->SATB address $0800
		dw	$0800
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
; init_352x224 - An example of initializing screen and VRAM.
;
; This can be used as-is, or copied to your own program and modified.
;

init_352x224	.proc

.BAT_SIZE	=	64 * 32
.SAT_ADDR	=	.BAT_SIZE		; SAT takes 16 tiles of VRAM.
.CHR_ZERO	=	.BAT_SIZE / 16		; 1st tile # after the BAT.
.CHR_0x10	=	.CHR_ZERO + 16		; 1st tile # after the SAT.
.CHR_0x20	=	.CHR_ZERO + 32		; ASCII ' ' CHR tile #.

		php				; Disable interrupts.
		sei

		call	clear_vce		; Clear all palettes.

		lda.l	#.CHR_0x20		; Tile # of ' ' CHR.
		sta.l	<_ax
		lda.h	#.CHR_0x20
		sta.h	<_ax

		lda	#>.BAT_SIZE		; Size of BAT in words.
		sta	<_bl

		call	clear_vram_vdc		; Clear VRAM.
	.if	SUPPORT_SGX
		call	clear_vram_sgx
	.endif

		lda	#<.mode_352x224		; Disable BKG & SPR layers but
		sta.l	<_bp			; enable RCR & VBLANK IRQ.
		lda	#>.mode_352x224
		sta.h	<_bp

	.if	SUPPORT_SGX
		call	sgx_detect		; Are we really on an SGX?
		bcc	!+
		ldy	#^.mode_352x224		; Set SGX 1st, with no VBL.
		call	set_mode_sgx
	.endif
!:		ldy	#^.mode_352x224		; Set VDC 2nd, VBL allowed.
		call	set_mode_vdc

	.if	SUPPORT_SGX
		bit	SGX_SR			; Purge any overdue RCR.
	.endif
		bit	VDC_SR			; Purge any overdue RCR/VBL.
		plp				; Restore interrupts.

		call	wait_vsync		; Wait for the next VBLANK.

		leave				; All done, phew!

		; A standard 352x224 screen with overscan.

.mode_352x224:	db	$80			; VCE Control Register.
		db	VCE_CR_7MHz + XRES_SOFT	;   Video Clock + Artifact Reduction

		db	VDC_MWR			; Memory-access Width Register
		dw	VDC_MWR_64x32 + VDC_MWR_1CYCLE
		db	VDC_HSR			; Horizontal Sync Register
		dw	VDC_HSR_352
		db	VDC_HDR			; Horizontal Display Register
		dw	VDC_HDR_352
		db	VDC_VPR			; Vertical Sync Register
		dw	VDC_VPR_224
		db	VDC_VDW			; Vertical Display Register
		dw	VDC_VDW_224
		db	VDC_VCR			; Vertical Display END position Register
		dw	VDC_VCR_224
		db	VDC_DCR			; DMA Control Register
		dw	$0010			;   Enable automatic VRAM->SATB
		db	VDC_DVSSR		; VRAM->SATB address $0800
		dw	$0800
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
; init_512x224 - An example of initializing screen and VRAM.
;
; This can be used as-is, or copied to your own program and modified.
;

init_512x224	.proc

.BAT_SIZE	=	64 * 32
.SAT_ADDR	=	.BAT_SIZE		; SAT takes 16 tiles of VRAM.
.CHR_ZERO	=	.BAT_SIZE / 16		; 1st tile # after the BAT.
.CHR_0x10	=	.CHR_ZERO + 16		; 1st tile # after the SAT.
.CHR_0x20	=	.CHR_ZERO + 32		; ASCII ' ' CHR tile #.

		php				; Disable interrupts.
		sei

		call	clear_vce		; Clear all palettes.

		lda.l	#.CHR_0x20		; Tile # of ' ' CHR.
		sta.l	<_ax
		lda.h	#.CHR_0x20
		sta.h	<_ax

		lda	#>.BAT_SIZE		; Size of BAT in words.
		sta	<_bl

		call	clear_vram_vdc		; Clear VRAM.
	.if	SUPPORT_SGX
		call	clear_vram_sgx
	.endif

		lda	#<.mode_512x224		; Disable BKG & SPR layers but
		sta.l	<_bp			; enable RCR & VBLANK IRQ.
		lda	#>.mode_512x224
		sta.h	<_bp

	.if	SUPPORT_SGX
		call	sgx_detect		; Are we really on an SGX?
		bcc	!+
		ldy	#^.mode_512x224		; Set SGX 1st, with no VBL.
		call	set_mode_sgx
	.endif
!:		ldy	#^.mode_512x224		; Set VDC 2nd, VBL allowed.
		call	set_mode_vdc

	.if	SUPPORT_SGX
		bit	SGX_SR			; Purge any overdue RCR.
	.endif
		bit	VDC_SR			; Purge any overdue RCR/VBL.
		plp				; Restore interrupts.

		call	wait_vsync		; Wait for the next VBLANK.

		leave				; All done, phew!

		; A standard 512x224 screen with overscan.

.mode_512x224:	db	$80			; VCE Control Register.
		db	VCE_CR_10MHz + XRES_SOFT;   Video Clock + Artifact Reduction

		db	VDC_MWR			; Memory-access Width Register
		dw	VDC_MWR_64x32 + VDC_MWR_2CYCLE
		db	VDC_HSR			; Horizontal Sync Register
		dw	VDC_HSR_512
		db	VDC_HDR			; Horizontal Display Register
		dw	VDC_HDR_512
		db	VDC_VPR			; Vertical Sync Register
		dw	VDC_VPR_224
		db	VDC_VDW			; Vertical Display Register
		dw	VDC_VDW_224
		db	VDC_VCR			; Vertical Display END position Register
		dw	VDC_VCR_224
		db	VDC_DCR			; DMA Control Register
		dw	$0010			;   Enable automatic VRAM->SATB
		db	VDC_DVSSR		; VRAM->SATB address $0800
		dw	$0800
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
; init_320x208 - An example of initializing screen and VRAM.
;
; This can be used as-is, or copied to your own program and modified.
;
; This resolution is rarely-seen, but it has no overscan, so it has a use.
;

init_320x208	.proc

.BAT_SIZE	=	64 * 32
.SAT_ADDR	=	.BAT_SIZE		; SAT takes 16 tiles of VRAM.
.CHR_ZERO	=	.BAT_SIZE / 16		; 1st tile # after the BAT.
.CHR_0x10	=	.CHR_ZERO + 16		; 1st tile # after the SAT.
.CHR_0x20	=	.CHR_ZERO + 32		; ASCII ' ' CHR tile #.

		php				; Disable interrupts.
		sei

		call	clear_vce		; Clear all palettes.

		lda.l	#.CHR_0x20		; Tile # of ' ' CHR.
		sta.l	<_ax
		lda.h	#.CHR_0x20
		sta.h	<_ax

		lda	#>.BAT_SIZE		; Size of BAT in words.
		sta	<_bl

		call	clear_vram_vdc		; Clear VRAM.
	.if	SUPPORT_SGX
		call	clear_vram_sgx
	.endif

		lda	#<.mode_320x208		; Disable BKG & SPR layers but
		sta.l	<_bp			; enable RCR & VBLANK IRQ.
		lda	#>.mode_320x208
		sta.h	<_bp

	.if	SUPPORT_SGX
		call	sgx_detect		; Are we really on an SGX?
		bcc	!+
		ldy	#^.mode_320x208		; Set SGX 1st, with no VBL.
		call	set_mode_sgx
	.endif
!:		ldy	#^.mode_320x208		; Set VDC 2nd, VBL allowed.
		call	set_mode_vdc

	.if	SUPPORT_SGX
		bit	SGX_SR			; Purge any overdue RCR.
	.endif
		bit	VDC_SR			; Purge any overdue VBL.
		plp				; Restore interrupts.

		call	wait_vsync		; Wait for the next VBLANK.

		leave				; All done, phew!

		; A standard 352x224 screen with overscan.

.mode_320x208:	db	$80			; VCE Control Register.
		db	VCE_CR_7MHz + XRES_SOFT	;   Video Clock + Artifact Reduction

		db	VDC_MWR			; Memory-access Width Register
		dw	VDC_MWR_64x32 + VDC_MWR_1CYCLE
		db	VDC_HSR			; Horizontal Sync Register
		dw	VDC_HSR_320
		db	VDC_HDR			; Horizontal Display Register
		dw	VDC_HDR_320
		db	VDC_VPR			; Vertical Sync Register
		dw	VDC_VPR_208
		db	VDC_VDW			; Vertical Display Register
		dw	VDC_VDW_208
		db	VDC_VCR			; Vertical Display END position Register
		dw	VDC_VCR_208
		db	VDC_DCR			; DMA Control Register
		dw	$0010			;   Enable automatic VRAM->SATB
		db	VDC_DVSSR		; VRAM->SATB address $0800
		dw	$0800
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
