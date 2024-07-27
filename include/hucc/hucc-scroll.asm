; ***************************************************************************
; ***************************************************************************
;
; hucc-scroll.asm
;
; Routines for a fast split-screen scrolling system.
;
; Copyright John Brandwood 2024.
;
; Distributed under the Boost Software License, Version 1.0.
; (See accompanying file LICENSE_1_0.txt or copy at
;  http://www.boost.org/LICENSE_1_0.txt)
;
; ***************************************************************************
; ***************************************************************************



	.ifndef	HUCC_PCE_SPLITS
HUCC_PCE_SPLITS = 8				; #splits on foreground layer.
	.endif

	.ifndef	HUCC_SGX_SPLITS
HUCC_SGX_SPLITS = 8				; #splits on background layer.
	.endif

	.if	(HUCC_PCE_SPLITS < 2) || (HUCC_PCE_SPLITS > 128)
		.fail	HUCC_PCE_SPLITS must be >= 2 and <= 128
	.endif

	.if	(HUCC_SGX_SPLITS < 2) || (HUCC_SGX_SPLITS > 128)
		.fail	HUCC_SGX_SPLITS must be >= 2 and <= 128
	.endif

HUCC_1ST_RCR	=	$144
HUCC_SCR_HEIGHT	=	224

		.code



; ***************************************************************************
; ***************************************************************************
;
; void __fastcall __xsafe scroll_split( unsigned char index<_al>, unsigned char top<_ah>, unsigned int x<_bx>, unsigned int y<_cx>, unsigned char disp<_dl> );
; void __fastcall __xsafe sgx_scroll_split( unsigned char index<_al>, unsigned char top<_ah>, unsigned int x<_bx>, unsigned int y<_cx>, unsigned char disp<_dl> );
;
; set screen scrolling

		.proc	_scroll_split.5

		ldy	<_al			; Region number.
		cpy	#HUCC_PCE_SPLITS
.hang:		bcs	.hang			; Better a hang than a crash!

		lda	vdc_region_sel, y	; Update the parameter copy
		bne	.regionA		; that is not displayed now.

.regionB:	lda	<_ah			; Scanline (i.e. top).
		cmp	#HUCC_SCR_HEIGHT	; Skip if offscreen.
		bcs	!done+
		sta	vdc_regionB_rcr, y

		cmp	#0			; Either Y at top of the frame
		beq	!+			; or Y-1 because the RCR code
		clc				; sets it on the line before.
!:		lda.l	<_cx
		sbc	#0
		sta	vdc_regionB_yl, y
		lda.h	<_cx
		sbc	#0
		sta	vdc_regionB_yh, y

		lda.l	<_bx
		sta	vdc_regionB_xl, y
		lda.h	<_bx
		sta	vdc_regionB_xh, y

		lda	<_dl			; Flags (mark it as enabled).
		and	#$C0
		ora	#$0C
		sta	vdc_regionB_crl, y

		lda	#1			; Update last so there is no
		sta	vdc_region_sel, y	; need to disable irqs.

!done:		leave				; All done!

.regionA:	lda	<_ah			; Scanline (i.e. top).
		cmp	#HUCC_SCR_HEIGHT	; Skip if offscreen.
		bcs	!done+
		sta	vdc_regionA_rcr, y

		cmp	#0			; Either Y at top of the frame
		beq	!+			; or Y-1 because the RCR code
		clc				; sets it on the line before.
!:		lda.l	<_cx
		sbc	#0
		sta	vdc_regionA_yl, y
		lda.h	<_cx
		sbc	#0
		sta	vdc_regionA_yh, y

		lda.l	<_bx
		sta	vdc_regionA_xl, y
		lda.h	<_bx
		sta	vdc_regionA_xh, y

		lda	<_dl			; Flags (mark it as enabled).
		and	#$C0
		ora	#$0C
		sta	vdc_regionA_crl, y

		cla				; Update last so there is no
		sta	vdc_region_sel, y	; need to disable irqs.

!done:		leave				; All done!

		.endp

	.if	SUPPORT_SGX

		.proc	_sgx_scroll_split.5

		ldy	<_al			; Region number.
		cpy	#HUCC_SGX_SPLITS
.hang:		bcs	.hang			; Better a hang than a crash!

		lda	sgx_region_sel, y	; Update the parameter copy
		bne	.regionA		; that is not displayed now.

.regionB:	lda	<_ah			; Scanline (i.e. top).
		cmp	#HUCC_SCR_HEIGHT	; Skip if offscreen.
		bcs	!done+
		sta	sgx_regionB_rcr, y

		cmp	#0			; Either Y at top of the frame
		beq	!+			; or Y-1 because the RCR code
		clc				; sets it on the line before.
!:		lda.l	<_cx
		sbc	#0
		sta	sgx_regionB_yl, y
		lda.h	<_cx
		sbc	#0
		sta	sgx_regionB_yh, y

		lda.l	<_bx
		sta	sgx_regionB_xl, y
		lda.h	<_bx
		sta	sgx_regionB_xh, y

		lda	<_dl
		and	#$C0			; Flags (mark it as enabled).
		ora	#$0C
		sta	sgx_regionB_crl, y

		lda	#1			; Update last so there is no
		sta	sgx_region_sel, y	; need to disable irqs.

!done:		leave				; All done!

.regionA:	lda	<_ah			; Scanline (i.e. top).
		cmp	#HUCC_SCR_HEIGHT	; Skip if offscreen.
		bcs	!done+
		sta	sgx_regionA_rcr, y

		cmp	#0			; Either Y at top of the frame
		beq	!+			; or Y-1 because the RCR code
		clc				; sets it on the line before.
!:		lda.l	<_cx
		sbc	#0
		sta	sgx_regionA_yl, y
		lda.h	<_cx
		sbc	#0
		sta	sgx_regionA_yh, y

		lda.l	<_bx
		sta	sgx_regionA_xl, y
		lda.h	<_bx
		sta	sgx_regionA_xh, y

		lda	<_dl
		and	#$C0			; Flags (mark it as enabled).
		ora	#$0C
		sta	sgx_regionA_crl, y

		cla				; Update last so there is no
		sta	sgx_region_sel, y	; need to disable irqs.

!done:		leave				; All done!

		.endp

	.endif	SUPPORT_SGX



; ***************************************************************************
; ***************************************************************************
;
; void __fastcall __xsafe disable_split( unsigned char index<acc> );
; void __fastcall __xsafe sgx_disable_split( unsigned char index<acc> );
;
; disable screen scrolling for a scroll region

_disable_split.1:
		cmp	#HUCC_PCE_SPLITS
.hang:		bcs	.hang			; Better a hang than a crash!
		sax

		ldy	vdc_region_sel, x	; Update the parameter copy
		bne	.regionA                ; that is not displayed now.

.regionB:	stz	vdc_regionB_crl, x	; Region disabled if $00.

		inc	vdc_region_sel, x	; Update last so there is no
		tax                             ; need to disable irqs.
		rts

.regionA:	stz	vdc_regionA_crl, x	; Region disabled if $00.

		stz	vdc_region_sel, x	; Update last so there is no
		tax                             ; need to disable irqs.
		rts

	.if	SUPPORT_SGX

_sgx_disable_split.1:
		cmp	#HUCC_SGX_SPLITS
.hang:		bcs	.hang			; Better a hang than a crash!
		sax

		ldy	sgx_region_sel, x	; Update the parameter copy
		bne	.regionA                ; that is not displayed now.

.regionB:	stz	sgx_regionB_crl, x	; Region disabled if $00.

		inc	sgx_region_sel, x	; Update last so there is no
		tax                             ; need to disable irqs.
		rts

.regionA:	stz	sgx_regionA_crl, x	; Region disabled if $00.

		stz	sgx_region_sel, x	; Update last so there is no
		tax                             ; need to disable irqs.
		rts

	.endif	SUPPORT_SGX



; ***************************************************************************
; ***************************************************************************
;
; vbl_init_scroll
;
; From Charles MacDonald's pcetech.txt ...
;
;  Raster Compare Register (RCR):
;
;  The range of the RCR is 263 lines, relative to the start of the active
;  display period. (defined by VSW, VDS, and VCR) The VDC treats the first
;  scanline of the active display period as $0040, so the valid ranges for
;  the RCR register are $0040 to $0146.
;
;  For example, assume VSW=$02, VDS=$17. This positions the first line of
;  the active display period at line 25 of the frame. An RCR value of $0040
;  (zero) causes an interrupt at line 25, and a value of $0146 (262) causes an
;  interrupt at line 24 of the next frame.
;
;  Any other RCR values that are out of range ($00-$3F, $147-$3FF) will never
;  result in a successful line compare.
;
; Memory used is 15 bytes per scroll per VDC!
;
; Old HuC rcr_init: 2148 cycles if all 8 regions pre-sorted
; Old HuC rcr_init: 4346 cycles if all 8 regions need sorting
;
; New HuCC vbl_init_scroll:   8 disabled splits:  246 cycles
; New HuCC vbl_init_scroll:   8  enabled splits:  340 cycles
;
; New HuCC vbl_init_scroll:  16 disabled splits:  422 cycles
; New HuCC vbl_init_scroll:  16  enabled splits:  588 cycles
;
; New HuCC vbl_init_scroll:  32 disabled splits:  774 cycles
; New HuCC vbl_init_scroll:  32  enabled splits: 1084 cycles
;
; New HuCC vbl_init_scroll:  64 disabled splits: 1478 cycles
; New HuCC vbl_init_scroll:  64  enabled splits: 2076 cycles
;
; New HuCC vbl_init_scroll: 128 disabled splits: 2886 cycles
; New HuCC vbl_init_scroll: 128  enabled splits: 4060 cycles

		.bss

vdc_region_sel:	.ds	HUCC_PCE_SPLITS		; Use A or B region next frame?

vdc_regionA_crl:.ds	HUCC_PCE_SPLITS		; Two copies of each setting
vdc_regionB_crl:.ds	HUCC_PCE_SPLITS		; HUCC_PCE_SPLITS bytes apart.
vdc_regionA_rcr:.ds	HUCC_PCE_SPLITS
vdc_regionB_rcr:.ds	HUCC_PCE_SPLITS
vdc_regionA_xl:	.ds	HUCC_PCE_SPLITS
vdc_regionB_xl:	.ds	HUCC_PCE_SPLITS
vdc_regionA_xh:	.ds	HUCC_PCE_SPLITS
vdc_regionB_xh:	.ds	HUCC_PCE_SPLITS
vdc_regionA_yl:	.ds	HUCC_PCE_SPLITS
vdc_regionB_yl:	.ds	HUCC_PCE_SPLITS
vdc_regionA_yh:	.ds	HUCC_PCE_SPLITS
vdc_regionB_yh:	.ds	HUCC_PCE_SPLITS

vdc_next_region:.ds	1			; Linked list of region indexes
vdc_regionA_nxt:.ds	HUCC_PCE_SPLITS		; for the current frame.
vdc_regionB_nxt:.ds	HUCC_PCE_SPLITS

	.if	SUPPORT_SGX

sgx_region_sel:	.ds	HUCC_SGX_SPLITS		; Use A or B region next frame?

sgx_regionA_crl:.ds	HUCC_SGX_SPLITS		; Two copies of each setting
sgx_regionB_crl:.ds	HUCC_SGX_SPLITS		; HUCC_SGX_SPLITS bytes apart.
sgx_regionA_rcr:.ds	HUCC_SGX_SPLITS
sgx_regionB_rcr:.ds	HUCC_SGX_SPLITS
sgx_regionA_xl:	.ds	HUCC_SGX_SPLITS
sgx_regionB_xl:	.ds	HUCC_SGX_SPLITS
sgx_regionA_xh:	.ds	HUCC_SGX_SPLITS
sgx_regionB_xh:	.ds	HUCC_SGX_SPLITS
sgx_regionA_yl:	.ds	HUCC_SGX_SPLITS
sgx_regionB_yl:	.ds	HUCC_SGX_SPLITS
sgx_regionA_yh:	.ds	HUCC_SGX_SPLITS
sgx_regionB_yh:	.ds	HUCC_SGX_SPLITS

sgx_next_region:.ds	1			; Linked list of region indexes
sgx_regionA_nxt:.ds	HUCC_SGX_SPLITS		; for the current frame.
sgx_regionB_nxt:.ds	HUCC_SGX_SPLITS

	.endif

		.code

vbl_init_scroll	.proc

		lda	#$FF			; A = previous active index
		ldx	#HUCC_PCE_SPLITS	; so $FF for end-of-screen.

		clc				; For regionB indexes.

!next_region:	dex				; All regions updated?
		bmi	!save_first+

		ldy	vdc_region_sel, x	; 0=regionA or 1=regionB.
		beq	!use_regionA+

!use_regionB:	ldy	vdc_regionB_crl, x	; Region disabled if $00.
		beq	!next_region-
		sta	vdc_regionB_nxt, x	; Save index of next region.
		txa				; A = current region index.
		adc	#HUCC_PCE_SPLITS	; Always leaves CC!
		bra	!next_region-

!use_regionA:	ldy	vdc_regionA_crl, x	; Region disabled if $00.
		beq	!next_region-
		sta	vdc_regionA_nxt, x	; Save index of next region.
		txa				; A = current region index.
		bra	!next_region-

!save_first:	sta	vdc_next_region		; Save index of 1st region.
		tax				; $FF if no active regions.
		bmi	!+

		smb7	<vdc_crl		; Ensure BURST MODE is off.

		lda	#VDC_RCR		; 1st RCR always happens just
		sta	VDC_AR			; before the display starts.
		lda.l	#HUCC_1ST_RCR
		sta	VDC_DL
		lda.h	#HUCC_1ST_RCR
		sta	VDC_DH

	.if	SUPPORT_SGX

!:		lda	#$FF			; A = previous active index
		ldx	#HUCC_SGX_SPLITS	; so $FF for end-of-screen.

		clc				; For regionB indexes.

!next_region:	dex				; All regions updated?
		bmi	!save_first+

		ldy	sgx_region_sel, x	; 0=regionA or 1=regionB.
		beq	!use_regionA+

!use_regionB:	ldy	sgx_regionB_crl, x	; Region disabled if $00.
		beq	!next_region-
		sta	sgx_regionB_nxt, x	; Save index of next region.
		txa				; A = current region index.
		adc	#HUCC_SGX_SPLITS	; Always leaves CC!
		bra	!next_region-

!use_regionA:	ldy	sgx_regionA_crl, x	; Region disabled if $00.
		beq	!next_region-
		sta	sgx_regionA_nxt, x	; Save index of next region.
		txa				; A = current region index.
		bra	!next_region-

!save_first:	sta	sgx_next_region		; Save index of 1st region.
		tax				; $FF if no active regions.
		bmi	!+

		smb7	<sgx_crl		; Ensure BURST MODE is off.

		lda	#VDC_RCR		; 1st RCR always happens just
		sta	SGX_AR			; before the display starts.
		lda.l	#HUCC_1ST_RCR
		sta	SGX_DL
		lda.h	#HUCC_1ST_RCR
		sta	SGX_DH

	.endif	SUPPORT_SGX

!:		leave				; All done!

		.endp



	.if	SUPPORT_SGX

; ***************************************************************************
; ***************************************************************************
;
; VDC_RCR_MACRO and SGX_RCR_MACRO
;
; A 16-byte TIA takes 142..234 cycles in 5MHz, 128..200 cycles in 7MHz.
; A 32-byte TIA takes 270..364 cycles in 5MHz, 242..312 cycles in 7MHz. (527 DUO)
;
; You need to write the last RCR setting within 540 cycles in order to catch
; the next line (at 256/336/512 resolution).
;
; That gives 305 cycles from the RCR to write the last setting, or there
; will be a visible glitch on the screen.

	.ifndef	USING_RCR_MACROS
USING_RCR_MACROS	=	1		; Tell IRQ1 to use the macros.
	.endif

;		;;;				; 8 (cycles for the INT)
;		bbs1	<irq_vec, .hook		; 8
;		jmp	[irq1_hook]		; 7

;		;;;				; 8 (cycles for the INT)
;		jmp	irq1_handler		; 4

;irq1_handler:	pha				; 3 Save all registers.
;		phx				; 3
;		phy				; 3
;
;		lda	VDC_SR			; 6 Acknowledge the VDC's IRQ.
;		sta	<vdc_sr			; 4 Remember what caused it.
;
;		ldx	SGX_SR			; 6 Read SGX_SR after VDC_SR in
;		stx	<sgx_sr			; 4 case this is not an SGX!
;
;!:		and	#$04			; 2 Is this an HSYNC interrupt?
;		beq	!+			; 2

		.macro	VDC_RCR_MACRO

		st0	#VDC_RCR		; 5

		ldx	vdc_next_region		; 5 X and Y can be greater than
		ldy	vdc_regionA_nxt, x	; 5 HUCC_PCE_SPLITS if regionB!
		clc				; 2
		bpl	!set_next_rcr+		; 4

		and	const_0000		; 5 A=$00 with the same #cycles
		bra	!clr_next_rcr+		; 4 as if the branch were taken.

!set_next_rcr:	lda	vdc_regionA_rcr, y	; 5 Set next RCR 1 line before
		adc	#64-1			; 2 the region begins.
!clr_next_rcr:	sta	VDC_DL			; 6
		cla				; 2
		rol	a			; 2
		sta	VDC_DH			; 6

		st0	#VDC_BYR		; 5 Do BYR first to mitigate the
		lda	vdc_regionA_yl, x	; 5 glitch if the IRQ is delayed.
		sta	VDC_DL			; 6 = 105 cycles from RCR on SGX
		lda	vdc_regionA_yh, x	; 5
		sta	VDC_DH			; 6

		st0	#VDC_CR			; 5
		lda	vdc_regionA_crl, x	; 5
;		asl	a
		sta	VDC_DL			; 6

		st0	#VDC_BXR		; 2
		lda	vdc_regionA_xl, x	; 5
		sta	VDC_DL			; 6
		lda	vdc_regionA_xh, x	; 5
		sta	VDC_DH			; 6 = 156 cycles from RCR if DUO

;		bcc	!+
;		lda	vdc_regionA_yh, x	; 5
;		sta.l	VCE_CTW			; 6

		sty	vdc_next_region		; 5

		.endm

;!:		bbr2	<sgx_sr, !+		; 6 Is this an HSYNC interrupt?

		.macro	SGX_RCR_MACRO

		lda	#VDC_RCR		; 2
		sta	SGX_AR			; 6

		ldx	sgx_next_region		; 5 X and Y can be greater than
		ldy	sgx_regionA_nxt, x	; 5 HUCC_SGX_SPLITS if regionB!
		clc				; 2
		bpl	!set_next_rcr+		; 4

		and	const_0000		; 5 A=$00 with the same #cycles
		bra	!clr_next_rcr+		; 4 as if the branch were taken.

!set_next_rcr:	lda	sgx_regionA_rcr, y	; 5 Set next RCR 1 line before
		adc	#64-1			; 2 the region begins.
!clr_next_rcr:	sta	SGX_DL			; 6
		cla				; 2
		rol	a			; 2
		sta	SGX_DH			; 6

		lda	#VDC_BYR		; 2 Do BYR first to mitigate the
		sta	SGX_AR			; 6 glitch if the IRQ is delayed.
		lda	sgx_regionA_yl, x	; 5
		sta	SGX_DL			; 6
		lda	sgx_regionA_yh, x	; 5
		sta	SGX_DH			; 6

		lda	#VDC_CR			; 2
		sta	SGX_AR			; 6
		lda	sgx_regionA_crl, x	; 5
		sta	SGX_DL			; 6

		lda	#VDC_BXR		; 2
		sta	SGX_AR			; 6
		lda	sgx_regionA_xl, x	; 5
		sta	SGX_DL			; 6
		lda	sgx_regionA_xh, x	; 5
		sta	SGX_DH			; 6 = 293 cycles (282 if no YH)

		sty	sgx_next_region		; 5

		.endm

	.else	SUPPORT_SGX

; ***************************************************************************
; ***************************************************************************
;
; VDC_RCR_MACRO
;
; A 16-byte TIA takes 142..234 cycles in 5MHz, 128..200 cycles in 7MHz.
; A 32-byte TIA takes 270..364 cycles in 5MHz, 242..312 cycles in 7MHz. (527 DUO)
;
; You need to write the last RCR setting within 540 cycles in order to catch
; the next line (at 256/336/512 resolution).
;
; That gives 305 cycles from the RCR to write the last setting, or there
; will be a visible glitch on the screen.

	.ifndef	USING_RCR_MACROS
USING_RCR_MACROS	=	1		; Tell IRQ1 to use the macros.
	.endif

;		;;;				; 8 (cycles for the INT)
;		bbs1	<irq_vec, .hook		; 8
;		jmp	[irq1_hook]		; 7

;		;;;				; 8 (cycles for the INT)
;		jmp	irq1_handler		; 4

;irq1_handler:	pha				; 3 Save all registers.
;		phx				; 3
;		phy				; 3
;
;		lda	VDC_SR			; 6 Acknowledge the VDC's IRQ.
;		sta	<vdc_sr			; 4 Remember what caused it.
;
;!:		and	#$04			; 2 Is this an HSYNC interrupt?
;		beq	!+			; 2

		.macro	VDC_RCR_MACRO

		lda	#VDC_RCR		; 2
		sta	VDC_AR			; 6

		ldx	vdc_next_region		; 5 X and Y can be greater than
		ldy	vdc_regionA_nxt, x	; 5 HUCC_PCE_SPLITS if regionB!
		clc				; 2
		bpl	!set_next_rcr+		; 4

		and	const_0000		; 5 A=$00 with the same #cycles
		bra	!clr_next_rcr+		; 4 as if the branch were taken.

!set_next_rcr:	lda	vdc_regionA_rcr, y	; 5 Set next RCR 1 line before
		adc	#64-1			; 2 the region begins.
!clr_next_rcr:	sta	VDC_DL			; 6
		cla				; 2
		rol	a			; 2
		sta	VDC_DH			; 6

		lda	#VDC_CR			; 2
		sta	VDC_AR			; 6
		lda	vdc_regionA_crl, x	; 5
		sta	VDC_DL			; 6 = 101 cycles from RCR if DUO

		lda	#VDC_BYR		; 2
		sta	VDC_AR			; 6
		lda	vdc_regionA_yl, x	; 5
		sta	VDC_DL			; 6
		lda	vdc_regionA_yh, x	; 5
		sta	VDC_DH			; 6

		lda	#VDC_BXR		; 2
		sta	VDC_AR			; 6
		lda	vdc_regionA_xl, x	; 5
		sta	VDC_DL			; 6
		lda	vdc_regionA_xh, x	; 5
		sta	VDC_DH			; 6 = 161 cycles from RCR if DUO

		sty	vdc_next_region		; 5

		.endm

	.endif	SUPPORT_SGX
