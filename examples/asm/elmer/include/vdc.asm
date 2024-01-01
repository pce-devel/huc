; ***************************************************************************
; ***************************************************************************
;
; vdc.asm
;
; Useful routines for operating the HuC6270 Video Display Controller.
;
; These should be located in permanently-accessible memory!
;
; Copyright John Brandwood 2021-2022.
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

;
;
;

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

		.endp
	.endif

clear_vram_vdc	.proc

		clx				; Offset to PCE VDC.

clear_vram_x:	bsr	clear_bat_x		; Clear the BAT.

		lda	#$80			; Xvert hi-byte of # words
		sec				; in screen to loop count.
		sbc	<_bl
		lsr	a

;		cly				; Clear the rest of VRAM.
.clr_loop:	pha
		stz	VDC_DL, x
.clr_pair:	stz	VDC_DH, x		; Seperate writes to minimize
		dey				; VDC MWR penalty.
		stz	VDC_DH, x
		bne	.clr_pair
		pla
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

		.endp
	.endif

clear_bat_vdc	.proc

		clx				; Offset to PCE VDC.

		bsr	clear_bat_x		; Clear the BAT.

		leave				; All done, phew!

		.endp

		; Written as a subroutine because of "leave"!

clear_bat_x:	stz	<_di + 0		; Set VDC or SGX destination
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

		rts

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

.set_mode_x:	tma3				; Preserve MPR3.
		pha
		tma4				; Preserve MPR4.
		pha

		jsr	set_bp_to_mpr34		; Map data to MPR3 & MPR4.

		php				; Disable interrupts.
		sei

		cly				; Table size is < 256 bytes.

.loop:		lda	[_bp], y		; Get the register #, +ve for
		beq	.done			; VDC, -128 for VCE.
		bpl	.set_vdc_reg

		; Set a VCE register.

.set_vce_reg:	iny

		lda	[_bp], y		; Get lo-byte of register.
		iny
		sta	VCE_CR			; Set the VCE clock speed and
		bra	.loop			; artifact reduction.

		; Set a VDC register.

.set_vdc_reg:	iny
		sta	VDC_AR, x		; Set which VDC register.

		cmp	#VDC_CR			; CS if VDC_CR or higher.
		beq	.skip_cc
		clc				; CC if not VDC_CR.

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

.done:		lda	<vdc_reg, x		; Restore previous VDC_AR from
		sta	VDC_AR, x		; the shadow variable.

;		; I'm deliberately NOT purging any delayed IRQ in this routine
;		; because it might legitimately be used during a frame.
;
;	.if	SUPPORT_SGX
;		bit	SGX_SR			; Purge any overdue RCR.
;	.endif
;		bit	VDC_SR			; Purge any overdue RCR/VBL.

		plp				; Restore interrupts.

		pla				; Restore MPR4.
		tam4
		pla				; Restore MPR3.
		tam3

		leave				; All done, phew!

		.endp

		.endprocgroup



; ***************************************************************************
; ***************************************************************************
;
; sgx_detect - Detect whether we're running on a SuperGrafx (and init VPC).
;
; Returns: Y,Z-flag,N-flag, and "sgx_detected" = NZ if detected.
;
; Note that this clears VRAM address $7F7F in both the VDC and SGX.
;
; ***************************************************************************
;
; https://web.archive.org/web/20161129055659/http://cgfm2.emuviews.com/txt/sgxtech.txt
;
; ***************************************************************************
;
; HuC6202 VIDEO PRIORITY CONTROLLER (huge thanks to Charles MacDonald!)
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
;   Priority Setting $00xx: (useful when VDC #1 is a fullscreen HUD)
;
;    FRONT
;     SP1'= VDC #1 high priority sprites
;     BG1 = VDC #1 background
;     SP1 = VDC #1 low priority sprites
;     SP2'= VDC #2 high priority sprites
;     BG2 = VDC #2 background
;     SP2 = VDC #2 low priority sprites
;    BACK
;
;   Priority Setting $01xx: (useful for parallax backgrounds)
;
;    FRONT
;     SP1'= VDC #1 high priority sprites
;     SP2'= VDC #2 high priority sprites
;     BG1 = VDC #1 background
;     SP1 = VDC #1 low priority sprites
;     BG2 = VDC #2 background
;     SP2 = VDC #2 low priority sprites
;    BACK
;
;   Priority Setting $10xx: (only useful for special effects)
;
;    FRONT
;     BG1 = VDC #1 background (with holes for sprites)
;     SP2'= VDC #2 high priority sprites
;     BG2 = VDC #2 background
;     SP2 = VDC #2 low priority sprites
;     SP1'= VDC #1 high priority sprites
;     SP1 = VDC #1 low priority sprites
;    BACK
;

	.if	SUPPORT_SGX
sgx_detect	.proc

		ldy	#$7F			; Use VRAM address $7F7F
		sty.l	<_di			; because it won't cause
		sty.h	<_di			; a screen glitch.

		jsr	sgx_di_to_mawr		; Write $007F to SGX VRAM.
		sty	SGX_DL
		stz	SGX_DH

		jsr	vdc_di_to_mawr		; Write $0000 to VDC VRAM.
		stz	VDC_DL
		stz	VDC_DH

		jsr	sgx_di_to_marr		; Check value in SGX VRAM.
		ldy	SGX_DL			; $7F if found, $00 if not.
		sty	sgx_detected
		beq	!+			; Skip the rest if not SGX.

		jsr	sgx_di_to_mawr		; Write $0000 to SGX VRAM
		stz	SGX_DL			; to clean VRAM contents.
		stz	SGX_DH

		tii	.vpc_mode, VPC_CR, 8	; Initialize the HuC6202 VPC.

!:		leave				; All done, phew!

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

;
;
;

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
		; This subroutine does the core of the transfer.
		;
		; Because it self-modifies, it needs to be in RAM on a HuCARD.
		;

tia_to_vram_len	=	51			; Size of self-modifying code.

	.if	CDROM
tia_to_vram_ram:				; CD-ROM code can self-modify.
	.else
		.bss
tia_to_vram_ram:ds	tia_to_vram_len		; HuCARD must put code in RAM.
		.code
	.endif

tia_to_vram_rom:

.next_page:	iny				; Increment source page.
		bpl	.same_bank		; Still in MPR3?
.next_bank:	tay				; Preserve address lo-byte.
		tma4
		tam3
		inc	a
		tam4
		tya				; Restore address lo-byte.
		ldy.h	#$6000			; Wrap address hi-byte.
.same_bank:	dex				; Number of chunks (lo-byte).
		beq	.next_block		; ... or drop through!

		;
		; The subroutine execution actually starts here!
		;
		; Because procedures are relocated just before the final
		; pass, these addresses can change in the final pass and
		; so must be ".set" because ".equ" will flag the changed
		; address as an error!
		;

tia_to_vram	.set	tia_to_vram_ram + (* - .next_page)
tia_to_vram_tia	.set	tia_to_vram + 7

.entry_point:	clc
		sty.h	tia_to_vram_tia + 1	; TIA source address hi-byte.
.chunk_loop:	sta.l	tia_to_vram_tia + 1	; TIA source address lo-byte.
		tia	$1234, VDC_DL, VRAM_XFER_SIZE
		adc	#VRAM_XFER_SIZE		; Increment source address.
		bcs	.next_page
.same_page:	dex				; Number of chunks (lo-byte).
		bne	.chunk_loop
.next_block:	dec.h	<_ax			; Number of chunks (hi-byte).
		bpl	.entry_point

		pla				; Restore MPR4.
		tam4
		pla				; Restore MPR3.
		tam3

		leave				; All done, phew!

	.if	tia_to_vram_len != (* - .next_page)
		.fail	The tia_to_vram code length has changed!
	.endif

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

		jsr	set_di_to_mawr		; Set VRAM write address.

	.if	SUPPORT_SGX
		inx				; Set VDC or SGX destination.
		inx
		stx	tia_to_vram_tia + 3
	.endif

		lda.l	<_bp			; Source address in CPU RAM.
		ldy.h	<_bp
		ldx.l	<_ax			; Number of chunks (lo-byte).

	.if	CDROM
		bne	tia_to_vram		; On CD-ROM, the code is close.
	.else
		bne	.execute		; On HuCARD, the code is far.
	.endif

		dec.h	<_ax			; Number of chunks (hi-byte).

.execute:	jmp	tia_to_vram		; Now execute the VRAM copy.

		.endp



	.if	!CDROM

; ***************************************************************************
; ***************************************************************************
;
; init_vram_copy - Initialize the copy-to-vram subroutine in PCE RAM.
;
; N.B. Only needed on HuCARD, because SCD/ACD can just modify in-place.
;

init_vram_copy	.proc

		tii	tia_to_vram_rom, tia_to_vram_ram, tia_to_vram_len

		leave				; All done, phew!

		.endp

	.endif	!CDROM

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
		beq	!+
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
		beq	!+
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
		db	VCE_CR_5MHz + 4		;   Video Clock + Artifact Reduction

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
		beq	!+
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
		db	VCE_CR_7MHz + 4		;   Video Clock + Artifact Reduction

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
		beq	!+
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
		db	VCE_CR_10MHz + 4	;   Video Clock + Artifact Reduction

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
		beq	!+
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
		db	VCE_CR_7MHz + 4		;   Video Clock + Artifact Reduction

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
