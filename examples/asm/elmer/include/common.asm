; ***************************************************************************
; ***************************************************************************
;
; common.asm
;
; Small, generic, PCE subroutines that are commonly useful when developing.
;
; These should be located in permanently-accessible memory!
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
; Overload this System Card variable for use by a far data pointer argument.
;

	.ifndef	__si_bank
__si_bank	=	__dh
__di_bank	=	__dh
__bp_bank	=	__dh
	.endif

;
; Useful variables.
;

	.ifndef	__temp
		.zp
__temp		ds	2			; For use within a subroutine.
		.code
	.endif

;
; Wait for the next VBLANK IRQ.
;

wait_vsync:	lda	irq_cnt			; System Card variable, changed
.loop:		cmp	irq_cnt			; every VBLANK interrupt.
		beq	.loop
		rts

;
; Map the __si data far-pointer into MPR3 (& MPR4).
;

__si_to_mpr34:	bsr	__si_to_mpr3		; Remap ptr to MPR3.
		inc	a			; Put next bank into MPR4.
		tam4
		rts

__si_to_mpr3:	lda	<__si + 1		; Remap ptr to MPR3.
		and	#$1F
		ora	#$60
		sta	<__si + 1

		lda	<__si_bank		; Put bank into MPR3.
		tam3
		rts

;
; Increment the hi-byte of __si and change TMA3 if necessary.
;

__si_inc_page:	inc	<__si + 1
		bmi	__si_inc_bank
		rts

__si_inc_bank:	pha
		tma3
		inc	a
		tam3
		lda	#$60
		sta	<__si + 1
		pla
		rts

;
; Map the __di data far-pointer into MPR3 (& MPR4).
;

__di_to_mpr34:	bsr	__di_to_mpr3		; Remap ptr to MPR3.
		inc	a			; Put next bank into MPR4.
		tam4
		rts

__di_to_mpr3:	lda	<__di + 1		; Remap ptr to MPR3.
		and	#$1F
		ora	#$60
		sta	<__di + 1

		lda	<__di_bank		; Put bank into MPR3.
		tam3
		rts

;
; Put the __di data pointer into the VDC's MAWR register.
;

	.if	SUPPORT_SGX
__di_to_sgx:	ldx	#SGX_VDC_OFFSET		; Offset to SGX VDC.
		db	$E0			; Turn "clx" into a "cpx #".
	.endif

__di_to_vdc:	clx				; Offset to PCE VDC.

__di_to_vram;	lda	#VDC_MAWR		; Set VDC or SGX destination
		stz	<vdc_reg, x		; address.

		stz	VDC_AR, x
		lda	<__di + 0
		sta	VDC_DL, x
		lda	<__di + 1
		sta	VDC_DH, x

		lda	#VDC_VWR		; Select the VWR data-write
		sta	<vdc_reg, x		; register.
		sta	VDC_AR, x
		rts

;
; That's all, for now!
;
