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



; ***************************************************************************
; ***************************************************************************
;
; Wait for the next VBLANK IRQ.
;

wait_vsync:	lda	irq_cnt			; System Card variable, changed
.loop:		cmp	irq_cnt			; every VBLANK interrupt.
		beq	.loop
		rts



; ***************************************************************************
; ***************************************************************************
;
; Delay for the next Y VBLANK IRQs.
;

wait_nvsync:	bsr	wait_vsync		; # of VBLANK IRQs to wait in
		dey				; the Y register.
		bne	wait_nvsync
		rts



; ***************************************************************************
; ***************************************************************************
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



; ***************************************************************************
; ***************************************************************************
;
; Put the __si data pointer into the VDC's MARR register.
;

	.if	SUPPORT_SGX
__si_to_sgx:	ldx	#SGX_VDC_OFFSET		; Offset to SGX VDC.
		db	$E0			; Turn "clx" into a "cpx #".
	.endif

__si_to_vdc:	clx				; Offset to PCE VDC.

__si_to_vram:	lda	#VDC_MARR		; Set VDC or SGX destination
		sta	<vdc_reg, x		; address.

		sta	VDC_AR, x
		lda	<__si + 0
		sta	VDC_DL, x
		lda	<__si + 1
		sta	VDC_DH, x

		lda	#VDC_VRR		; Select the VRR data-read
		sta	<vdc_reg, x		; register.
		sta	VDC_AR, x
		rts



; ***************************************************************************
; ***************************************************************************
;
; Increment the hi-byte of __si and change TMA3 if necessary.
;

__si_inc_mpr3:	inc.h	<__si			; Increment hi-byte of __si.

		bpl	!+			; OK if within MPR0-MPR3.
		tst	#%01100000, <__si + 1	; Check if address is in MPR4.
		bne	!+			; OK if within MPR5-MPR7.

		pha				; Increment the bank in MPR3,
		tma3				; usually when pointer moves
		inc	a			; from MPR3 into MPR4.
		tam3
		lda.h	<__si			; Remap addr from $8000-$9FFF
		eor	#%11100000		; back into $6000-$7FFF.
;		lda	#$60
		sta.h	<__si
		pla
!:		rts



; ***************************************************************************
; ***************************************************************************
;
; Increment the hi-byte of __si and change TMA4 if necessary.
;

	.if	0				; Save memory, for now.

__si_inc_mpr4:	inc.h	<__si			; Increment hi-byte of __si.

		bpl	!+			; OK if within MPR0-MPR3.
		tst	#%00100000, <__si + 1	; Check if address is in MPR5.
		beq	!+			; OK if within MPR4 or MPR6.
		bvs	!+			; OK if within MPR6-MPR7.

		pha				; Increment the bank in MPR4,
		tma4				; usually when pointer moves
		inc	a			; from MPR4 into MPR5.
		tam4
		lda.h	<__si			; Remap addr from $A000-$BFFF
		eor	#%00100000		; back into $8000-$9FFF.
;		lda	#$80
		sta.h	<__si
		pla
!:		rts
	.endif



; ***************************************************************************
; ***************************************************************************
;
; Map the __di data far-pointer into MPR3 (& MPR4).
;

__di_to_mpr34:	bsr	__di_to_mpr3		; Remap ptr to MPR3.
		inc	a			; Put next bank into MPR4.
		tam4
		rts

__di_to_mpr3:	lda.h	<__di			; Remap ptr to MPR3.
		and	#$1F
		ora	#$60
		sta.h	<__di

		lda	<__di_bank		; Put bank into MPR3.
		tam3
		rts



; ***************************************************************************
; ***************************************************************************
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



; ***************************************************************************
; ***************************************************************************
;
; Increment the hi-byte of __di and change TMA3 if necessary.
;

__di_inc_mpr3:	inc.h	<__di			; Increment hi-byte of __di.

		bpl	!+			; OK if within MPR0-MPR3.
		tst	#%01100000, <__di + 1	; Check if address is in MPR4.
		bne	!+			; OK if within MPR5-MPR7.

		pha				; Increment the bank in MPR4,
		tma3				; usually when pointer moves
		inc	a			; from MPR4 into MPR5.
		tam3
		lda.h	<__di			; Remap addr from $8000-$9FFF
		eor	#%11100000		; back into $6000-$7FFF.
;		lda	#$60
		sta.h	<__di
		pla
!:		rts



; ***************************************************************************
; ***************************************************************************
;
; Increment the hi-byte of __di and change TMA4 if necessary.
;

	.if	0				; Save memory, for now.

__di_inc_mpr4:	inc.h	<__di			; Increment hi-byte of __di.

		bpl	!+			; OK if within MPR0-MPR3.
		tst	#%00100000, <__di + 1	; Check if address is in MPR5.
		beq	!+			; OK if within MPR4 or MPR6.
		bvs	!+			; OK if within MPR6-MPR7.

		pha				; Increment the bank in MPR4,
		tma4				; usually when pointer moves
		inc	a			; from MPR4 into MPR5.
		tam4
		lda.h	<__di			; Remap addr from $A000-$BFFF
		eor	#%00100000		; back into $8000-$9FFF.
;		lda	#$80
		sta.h	<__di
		pla
!:		rts
	.endif
