; ***************************************************************************
; ***************************************************************************
;
; acd.asm
;
; This is basically Hudson's ArcadeCard detection code from GulliverBoy,
; and many other ArcadeCard games.
;
; Modifications copyright John Brandwood 2022.
;
; Distributed under the Boost Software License, Version 1.0.
; (See accompanying file LICENSE_1_0.txt or copy at
;  http://www.boost.org/LICENSE_1_0.txt)
;
; ***************************************************************************
; ***************************************************************************

;
; Configure Library ...
;

	.ifndef	SUPPORT_ACD
SUPPORT_ACD	=	1
	.endif

;
; Include dependancies ...
;

		include "pcengine.inc"		; Common helpers.



	.if	SUPPORT_ACD

; ***************************************************************************
; ***************************************************************************
;
; acd_detect - Detect whether ArcadeCard hardware is present.
;
; Returns: Y,Z-flag,N-flag, and "acd_detected" = NZ if detected.
;

acd_detect	.proc

		cly				; Assume no ACD result.

		lda	ACD_FLAG		; Check the ACD ident byte.
		cmp	#$51
		bne	.not_acd

		clx				; Clear each channel's BASE,
!:		stz	ACD0_BASE, x		; INDX, INCR, & CTRL values.
		stz	ACD1_BASE, x
		stz	ACD2_BASE, x
		stz	ACD3_BASE, x
		inx
		cpx	#8
		bne	!-

		clx				; Clear the SHIFT register,
!:		stz	ACD_SHIFT, x		; and ASL and ROL counts.
		inx
		cpx	#6
		bne	!-

		clx				; Make sure the BASE, INDX,
!:		lda	ACD0_BASE, x		; and INCR values are zero.
		ora	ACD1_BASE, x
		ora	ACD2_BASE, x
		ora	ACD3_BASE, x
		bne	.not_acd
		inx
		cpx	#7
		bne	!-

		lda	ACD0_BASE, x		; Make sure the low 7-bits
		ora	ACD1_BASE, x		; of CTRL are zero.
		ora	ACD2_BASE, x		; The hi-bit of CTRL isn't
		ora	ACD3_BASE, x    	; readable.
		and	#$7F
		bne	.not_acd

		clx				; Check the SHIFT register is
!:		lda	ACD_SHIFT, x		; zero.
		bne	.not_acd
		inx
		cpx	#4
		bne	!-

		lda	ACD_ASL			; Check the low 4-bits of the
		ora	ACD_ROL			; shift counts are zero.
		and	#$0F
		bne	.not_acd

		dey				; ACD found, return non-zero.

.not_acd:	sty	acd_detected		; NZ if ArcadeCard detected.

		leave

		.endp

	.ifndef	CORE_VERSION			; CORE has this in the kernel.
		.bss
acd_detected:	ds	1			; NZ if ArcadeCard detected.
		.code
	.endif	CORE_VERSION

	.endif	SUPPORT_ACD
