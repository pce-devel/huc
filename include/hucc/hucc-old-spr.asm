; ***************************************************************************
; ***************************************************************************
;
; hucc-old-spr.asm
;
; Based on the original HuC and MagicKit functions by David Michel and the
; other original HuC developers.
;
; Modifications copyright John Brandwood 2024.
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
; HuC Sprite Functions
;
; ***************************************************************************
; ***************************************************************************

		.zp
spr_ptr:	ds	2
		.bss
spr_sat:	ds	512
		.code

	.if	SUPPORT_SGX
		.zp
sgx_spr_ptr:	ds	2
		.bss
sgx_spr_sat:	ds	512	; N.B. Directly after spr_sat!
		.code
	.endif

; Moved to hucc-old-map.asm just to save space. This NEEDS to be changed!
;
;		.bss
;spr_max:	ds	1
;spr_clr:	ds	1
;	.if	SUPPORT_SGX
;sgx_spr_max:	ds	1
;sgx_spr_clr:	ds	1
;	.endif
;		.code



; ***************************************************************************
; ***************************************************************************
;
; void __fastcall init_satb( void );
; void __fastcall reset_satb( void );
;
; void __fastcall sgx_init_satb( void );
; void __fastcall sgx_reset_satb( void );

_reset_satb:
_init_satb:	cly
		cla
!:		sta	spr_sat + $0000, y
		sta	spr_sat + $0100, y
		iny
		bne	!-
		sty	spr_max
		iny
		sty	spr_clr
		rts

	.if	SUPPORT_SGX
_sgx_reset_satb:
_sgx_init_satb:	cly
		cla
!:		sta	sgx_spr_sat + $0000, y
		sta	sgx_spr_sat + $0100, y
		iny
		bne	!-
		sty	sgx_spr_max
		iny
		sty	sgx_spr_clr
		rts
	.endif



; ***************************************************************************
; ***************************************************************************
;
; void __fastcall satb_update( void );
; void __fastcall sgx_satb_update( void );

old_satb_group	.procgroup

	.if	SUPPORT_SGX
		.proc	_sgx_satb_update

		ldy	#SGX_VDC_OFFSET		; Offset to SGX VDC.
		db	$F0			; Turn "cly" into a "beq".

		.ref	_satb_update
		.endp
	.endif

		.proc	_satb_update

		cly				; Offset to PCE VDC.

		phx				; Preserve X (aka __sp).
		sxy				; Put VDC offset in X.

		lda.h	#$7F00			; HuC puts the SAT here in VRAM
;		lda.h	#$0800			; but we put it here instead
		stz.l	<_di
		sta.h	<_di
		jsr	set_di_to_mawr

	.if	SUPPORT_SGX
		txa				; Select which VDC to write
		inc	a			; to.
		inc	a
		sta.l	ram_tia_dst
	.endif

		lda	#VRAM_XFER_SIZE		; Split into 16-byte chunks
		sta.l	ram_tia_len		; for stable IRQ response.

		ldy	spr_max, x		; Highest sprite that was set.
		iny

		lda	spr_clr, x
		beq	!+
		stz	spr_clr, x
		ldy	#64

!:		tya
		beq	.exit

		dec	a			; round up to the next group of 2 sprites
		lsr	a
	.if	VRAM_XFER_SIZE == 32
		lsr	a
	.endif
		inc	a
		tay

		lda.h	#spr_sat

	.if	SUPPORT_SGX
		cpx	#0
		beq	!+

		lda.h	#sgx_spr_sat
	.endif

!:		sta.h	ram_tia_src

		lda.l	#spr_sat		; Same for SGX and PCE!
.chunk_loop:	sta.l	ram_tia_src

		jsr	ram_tia			; transfer 16-bytes

		clc				; increment source
		adc	#VRAM_XFER_SIZE
		bcc	.same_page
		inc.h	ram_tia_src

.same_page:	dey
		bne	.chunk_loop

.exit:		plx				; Restore X.

		leave

		.endp

		.endprocgroup	; old_satb_group



; ***************************************************************************
; ***************************************************************************
;
; void __fastcall spr_set( unsigned char num<acc> );

_spr_set.1:	cmp	spr_max
		bcc	!+
		sta	spr_max
!:		ldy.h	#spr_sat
		asl	a
		asl	a
		asl	a
		bcc	!+
		iny
		clc
!:		adc.l	#spr_sat
		sta.l	<spr_ptr
		bcc	!+
		iny
!:		sty.h	<spr_ptr
		rts



; ***************************************************************************
; ***************************************************************************
;
; void __fastcall spr_hide( void );

_spr_hide:	ldy	#1
		lda	[spr_ptr], y
		ora	#2
		sta	[spr_ptr], y
		rts



; ***************************************************************************
; ***************************************************************************
;
; void __fastcall spr_show( void );

_spr_show:	ldy	#1
		lda	[spr_ptr], y
		and	#1
		sta	[spr_ptr], y
		rts



; ***************************************************************************
; ***************************************************************************
;
; void __fastcall spr_x( unsigned int value<acc> );

_spr_x.1:	phy
		clc
		adc	#32
		ldy	#2
		sta	[spr_ptr], y
		pla
		adc	#0
		iny
		sta	[spr_ptr], y
		rts



; ***************************************************************************
; ***************************************************************************
;
; void __fastcall spr_y( unsigned int value<acc> );

_spr_y.1:	clc
		adc	#64
		sta	[spr_ptr]
		tya
		adc	#0
		ldy	#1
		sta	[spr_ptr], y
		rts
		


; ***************************************************************************
; ***************************************************************************
;
; void __fastcall spr_pattern( unsigned int vaddr<acc> );

_spr_pattern.1:	sty	<__temp		;     zp=fedcba98 a=76543210
		asl	a		; c=f zp=edcba987 a=6543210_
		rol	<__temp
		rol	a		; c=e zp=dcba9876 a=543210_f
		rol	<__temp
		rol	a		; c=d zp=cba98765 a=43210_fe
		rol	<__temp
		rol	a		; c=4 zp=cba98765 a=3210_fed
		ldy	#5
		sta	[spr_ptr], y
		lda	<__temp
		dey
		sta	[spr_ptr], y
		rts



; ***************************************************************************
; ***************************************************************************
;
; void __fastcall spr_ctrl( unsigned char mask<_al>, unsigned char value<acc> );

_spr_ctrl.2:	and	<_al
		sta	<__temp
		lda	<_al
		eor	#$FF
		ldy	#7
		and	[spr_ptr], y
		ora	<__temp
		sta	[spr_ptr], y
		rts



; ***************************************************************************
; ***************************************************************************
;
; void __fastcall spr_pal( unsigned char palette<acc> )

_spr_pal.1:	and	#$0F
		sta	<__temp
		ldy	#6
		lda	[spr_ptr], y
		and	#$F0
		ora	<__temp
		sta	[spr_ptr], y
		rts



; ***************************************************************************
; ***************************************************************************
;
; void __fastcall spr_pri( unsigned char priority<acc> )

_spr_pri.1:	cmp	#1
		ldy	#6
		lda	[spr_ptr], y
		and	#$7F
		bcc	!+
		ora	#$80
!:		sta	[spr_ptr], y
		rts



; ***************************************************************************
; ***************************************************************************
;
; unsigned int __fastcall spr_get_x( void );

_spr_get_x:	sec
		ldy	#2
		lda	[spr_ptr], y
		sbc	#32
		pha
		iny
		lda	[spr_ptr], y
		sbc	#0
		tay
		pla
		rts



; ***************************************************************************
; ***************************************************************************
;
; unsigned int __fastcall spr_get_y( void );

_spr_get_y:	sec
		lda	[spr_ptr]
		sbc	#64
		pha
		ldy	#1
		lda	[spr_ptr], y
		sbc	#0
		tay
		pla
		rts



	.if	SUPPORT_SGX

; ***************************************************************************
; ***************************************************************************
;
; void __fastcall sgx_spr_set( unsigned char num<acc> );

_sgx_spr_set.1:	cmp	sgx_spr_max
		bcc	!+
		sta	sgx_spr_max
!:		ldy.h	#sgx_spr_sat
		asl	a
		asl	a
		asl	a
		bcc	!+
		iny
		clc
!:		adc.l	#sgx_spr_sat
		sta.l	<sgx_spr_ptr
		bcc	!+
		iny
!:		sty.h	<sgx_spr_ptr
		rts



; ***************************************************************************
; ***************************************************************************
;
; void __fastcall sgx_spr_hide( void );

_sgx_spr_hide:	ldy	#1
		lda	[sgx_spr_ptr], y
		ora	#2
		sta	[sgx_spr_ptr], y
		rts



; ***************************************************************************
; ***************************************************************************
;
; void __fastcall sgx_spr_show( void );

_sgx_spr_show:	ldy	#1
		lda	[sgx_spr_ptr], y
		and	#1
		sta	[sgx_spr_ptr], y
		rts



; ***************************************************************************
; ***************************************************************************
;
; void __fastcall sgx_spr_x( unsigned int value<acc> );

_sgx_spr_x.1:	phy
		clc
		adc	#32
		ldy	#2
		sta	[sgx_spr_ptr], y
		pla
		adc	#0
		iny
		sta	[sgx_spr_ptr], y
		rts



; ***************************************************************************
; ***************************************************************************
;
; void __fastcall sgx_spr_y( unsigned int value<acc> );

_sgx_spr_y.1:	clc
		adc	#64
		sta	[sgx_spr_ptr]
		tya
		adc	#0
		ldy	#1
		sta	[sgx_spr_ptr], y
		rts
		


; ***************************************************************************
; ***************************************************************************
;
; void __fastcall sgx_spr_pattern( unsigned int vaddr<acc> );

_sgx_spr_pattern.1:
		sty	<__temp		;     zp=fedcba98 a=76543210
		asl	a		; c=f zp=edcba987 a=6543210_
		rol	<__temp
		rol	a		; c=e zp=dcba9876 a=543210_f
		rol	<__temp
		rol	a		; c=d zp=cba98765 a=43210_fe
		rol	<__temp
		rol	a		; c=4 zp=cba98765 a=3210_fed
		ldy	#5
		sta	[sgx_spr_ptr], y
		lda	<__temp
		dey
		sta	[sgx_spr_ptr], y
		rts



; ***************************************************************************
; ***************************************************************************
;
; void __fastcall sgx_spr_ctrl( unsigned char mask<_al>, unsigned char value<acc> );

_sgx_spr_ctrl.2:and	<_al
		sta	<__temp
		lda	<_al
		eor	#$FF
		ldy	#7
		and	[sgx_spr_ptr], y
		ora	<__temp
		sta	[sgx_spr_ptr], y
		rts



; ***************************************************************************
; ***************************************************************************
;
; void __fastcall sgx_spr_pal( unsigned char palette<acc> )

_sgx_spr_pal.1:	and	#$0F
		sta	<__temp
		ldy	#6
		lda	[sgx_spr_ptr], y
		and	#$F0
		ora	<__temp
		sta	[sgx_spr_ptr], y
		rts



; ***************************************************************************
; ***************************************************************************
;
; void __fastcall sgx_spr_pri( unsigned char priority<acc> )

_sgx_spr_pri.1:	cmp	#1
		ldy	#6
		lda	[sgx_spr_ptr], y
		and	#$7F
		bcc	!+
		ora	#$80
!:		sta	[sgx_spr_ptr], y
		rts



; ***************************************************************************
; ***************************************************************************
;
; unsigned int __fastcall sgx_spr_get_x( void );

_sgx_spr_get_x:	sec
		ldy	#2
		lda	[sgx_spr_ptr], y
		sbc	#32
		pha
		iny
		lda	[sgx_spr_ptr], y
		sbc	#0
		tay
		pla
		rts



; ***************************************************************************
; ***************************************************************************
;
; unsigned int __fastcall sgx_spr_get_y( void );

_sgx_spr_get_y:	sec
		lda	[sgx_spr_ptr]
		sbc	#64
		pha
		ldy	#1
		lda	[sgx_spr_ptr], y
		sbc	#0
		tay
		pla
		rts

	.endif	SUPPORT_SGX
