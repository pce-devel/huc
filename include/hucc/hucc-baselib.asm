; ***************************************************************************
; ***************************************************************************
;
; hucc-baselib.asm
;
; Basic library functions provided as macros.
;
; Copyright John Brandwood 2024.
;
; Distributed under the Boost Software License, Version 1.0.
; (See accompanying file LICENSE_1_0.txt or copy at
;  http://www.boost.org/LICENSE_1_0.txt)
;
; ***************************************************************************
; ***************************************************************************



; ***************************************************************************
; ***************************************************************************
;
; void __fastcall __xsafe dump_screen( void );
;
; THIS IS AN ILLEGAL INSTRUCTION ONLY IMPLEMENTED BY THE TGEMU EMULATOR!

_dump_screen:	db	0x33



; ***************************************************************************
; ***************************************************************************
;
; void __fastcall __xsafe abort( void );
;
; THIS IS AN ILLEGAL INSTRUCTION ONLY IMPLEMENTED BY THE TGEMU EMULATOR!

_abort:		db	0xE2



; ***************************************************************************
; ***************************************************************************
;
; void __fastcall __xsafe exit( int value<acc> );
;
; THIS IS AN ILLEGAL INSTRUCTION ONLY IMPLEMENTED BY THE TGEMU EMULATOR!

_exit.1:	tax				; Put the return code into X.
		db	0x63

.hang:		bra	.hang			; Hang if used in normal code.



; ***************************************************************************
; ***************************************************************************
;
; unsigned char __fastcall __xsafe __macro cd_execoverlay( unsigned char ovl_index<acc> );
;
; Execute program overlay from disc
;
; N.B. This does not return, even if there's an error.

		.macro	_cd_execoverlay.1
		tax
		jmp	exec_overlay
		.endm



; ***************************************************************************
; ***************************************************************************
;
; unsigned char __fastcall __xsafe __macro ac_exists( void );

_ac_exists	.macro
		cla
		ldy	ACD_FLAG
		cpy	#ACD_ID
		bne	!+
		inc	a
!:		cly
		.endm



; ***************************************************************************
; ***************************************************************************
;
; unsigned int __fastcall __xsafe __macro peek( unsigned int addr<__ptr> );

_peek.1		.macro
		lda	[__ptr]
		cly
		.endm



; ***************************************************************************
; ***************************************************************************
;
; unsigned int __fastcall __xsafe __macro peekw( unsigned int addr<__ptr> );

_peekw.1	.macro
		lda	[__ptr]
		pha
		ldy	#1
		lda	[__ptr], y
		tay
		pla
		.endm



; ***************************************************************************
; ***************************************************************************
;
; void __fastcall __macro __xsafe poke( unsigned int addr<__ptr>, unsigned char with<acc> );

_poke.2		.macro
		sta	[__ptr]
		.endm



; ***************************************************************************
; ***************************************************************************
;
; void __fastcall __macro __xsafe pokew( unsigned int addr<__ptr>, unsigned int with<acc> );

_pokew.2	.macro
		sta	[__ptr]
		tya
		ldy	#1
		sta	[__ptr], y
		.endm



; ***************************************************************************
; ***************************************************************************
;
; unsigned char __fastcall __xsafe __macro clock_hh( void );

_clock_hh	.macro
		lda	clock_hh
		cly
		.endm



; ***************************************************************************
; ***************************************************************************
;
; unsigned char __fastcall __xsafe __macro clock_mm( void );

_clock_mm	.macro
		lda	clock_mm
		cly
		.endm



; ***************************************************************************
; ***************************************************************************
;
; unsigned char __fastcall __xsafe __macro clock_ss( void );

_clock_ss	.macro
		lda	clock_ss
		cly
		.endm



; ***************************************************************************
; ***************************************************************************
;
; unsigned char __fastcall __xsafe __macro clock_tt( void );

_clock_tt	.macro
		lda	clock_tt
		cly
		.endm



; ***************************************************************************
; ***************************************************************************
;
; void __fastcall __xsafe __macro clock_reset( void );

_clock_reset	.macro
		stz	clock_hh
		stz	clock_mm
		stz	clock_ss
		stz	clock_tt
		.endm



; ***************************************************************************
; ***************************************************************************
;
; void __fastcall __xsafe __macro vsync( void );
; void __fastcall __xsafe __macro vsync( unsigned char count<acc> );

_vsync		.macro
		jsr	wait_vsync
		.endm

_vsync.1	.macro
		tay
		jsr	wait_nvsync
		.endm



; ***************************************************************************
; ***************************************************************************
;
; void __fastcall __xsafe __macro joy( unsigned char which<acc> );

_joy.1		.macro
	.if	SUPPORT_6BUTTON
		tay
		lda	joy6now, y
		pha
		lda	joynow, y
		ply
	.else
		tay
		lda	joynow, y
		cly
	.endif
		.endm



; ***************************************************************************
; ***************************************************************************
;
; void __fastcall __xsafe __macro joytrg( unsigned char which<acc> );

_joytrg.1	.macro
	.if	SUPPORT_6BUTTON
		tay
		lda	joy6trg, y
		pha
		lda	joytrg, y
		ply
	.else
		tay
		lda	joytrg, y
		cly
	.endif
		.endm



; ***************************************************************************
; ***************************************************************************
;
; void __fastcall __xsafe _nop set_color( unsigned int index<VCE_CTA>, unsigned int value<VCE_CTW> );
; void __fastcall __xsafe set_color_rgb( unsigned int index<VCE_CTA>, unsigned char r<_al>, unsigned char g<_ah>, unsigned char b<acc> );
;
; r:	red	RED:	bit 3-5
; g:	green	GREEN:	bit 6-8
; b:	blue	BLUE:	bit 0-2

_set_color_rgb.4:
;		lda	<_bl
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



; ***************************************************************************
; ***************************************************************************
;
; void __fastcall __xsafe _macro get_color( unsigned int index<VCE_CTA> );

_get_color.1	.macro
		lda.l	VCE_CTR
		ldy.h	VCE_CTR
		.endm



; ***************************************************************************
; ***************************************************************************
;
; void __fastcall __xsafe _macro set_xres( unsigned int x_pixels<_ax> );
; void __fastcall __xsafe _macro sgx_set_xres( unsigned int x_pixels<_ax> );
;
; void __fastcall __xsafe set_xres( unsigned int x_pixels<_ax>, unsigned char blur_flag<_bl> );
; void __fastcall __xsafe sgx_set_xres( unsigned int x_pixels<_ax>, unsigned char blur_flag<_bl> );

_set_xres.1	.macro
		lda	#XRES_SOFT
		sta	<_bl
		call	_set_xres.2
		.endm

	.if	SUPPORT_SGX
		.macro	_sgx_set_xres.1
		lda	#XRES_SOFT
		sta	<_bl
		call	_sgx_set_xres.2
		.endm
	.endif



; ***************************************************************************
; ***************************************************************************
;
; N.B. Declared in hucc-gfx.h, but defined here because they're macros!
;
; unsigned int __fastcall __xsafe __macro vram_addr( unsigned char bat_x<_al>, unsigned char bat_y<_ah> );
; unsigned int __fastcall __xsafe __macro sgx_vram_addr( unsigned char bat_x<_al>, unsigned char bat_y<_ah> );

		.macro	_vram_addr.2
		cla
		bit	vdc_bat_width
		bmi	!w128+
		bvs	!w64+
!w32:		lsr	<_ah
		ror	a
!w64:		lsr	<_ah
		ror	a
!w128:		lsr	<_ah
		ror	a
		ora	<_al
		ldy	<_ah
		.endm

	.if	SUPPORT_SGX
		.macro	_sgx_vram_addr.2
		cla
		bit	sgx_bat_width
		bmi	!w128+
		bvs	!w64+
!w32:		lsr	<_ah
		ror	a
!w64:		lsr	<_ah
		ror	a
!w128:		lsr	<_ah
		ror	a
		ora	<_al
		ldy	<_ah
		.endm
	.endif



; ***************************************************************************
; ***************************************************************************
;
; N.B. Declared in hucc-gfx.h, but defined here because they're macros!
;
; unsigned int __fastcall __xsafe __macro get_vram( unsigned int address<_di> );
; void __fastcall __xsafe __macro put_vram( unsigned int address<_di>, unsigned int data<acc> );
;
; unsigned int __fastcall __xsafe __macro sgx_get_vram( unsigned int address<_di> );
; void __fastcall __xsafe __macro sgx_put_vram( unsigned int address<_di>, unsigned int data<acc> );

		.macro	_get_vram.1
		phx
		jsr	vdc_di_to_marr
		plx
		lda	VDC_DL
		ldy	VDC_DH
		.endm

		.macro	_put_vram.2
		pha
		phx
		jsr	vdc_di_to_mawr
		plx
		pla
		sta	VDC_DL
		sty	VDC_DH
		.endm

	.if	SUPPORT_SGX
		.macro	_sgx_get_vram.1
		phx
		jsr	sgx_di_to_marr
		plx
		lda	SGX_DL
		ldy	SGX_DH
		.endm

		.macro	_sgx_put_vram.2
		pha
		phx
		jsr	sgx_di_to_mawr
		plx
		pla
		sta	SGX_DL
		sty	SGX_DH
		.endm
	.endif



; ***************************************************************************
; ***************************************************************************
;


	.if	0
__lbltsbi	.macro
		sec			; Subtract memory from A.
		sbc.l	#\1		; "cmp" does not set V flag!
		bvc	!+		
		eor	#$80		; +ve if A >= memory (signed).
!:		bmi	\2		; -ve if A  < memory (signed).
		.endm


__lbltswi	.macro
		cmp.l	#\1		; Subtract memory from Y:A.
		tya
		sbc.h	#\1
		bvc	!+
		eor	#$80		; +ve if Y:A >= memory (signed).
!:		bmi	\2		; -ve if Y:A  < memory (signed).
		.endm
	.endif



; ***************************************************************************
; ***************************************************************************
;
; void __fastcall __xsafe srand( unsigned char seed<acc> );
; unsigned char __fastcall __xsafe rand( void );
;
; unsigned char __fastcall __xsafe random( unsigned char limit<acc> );
;
; IN :	A = range (1 - 128)
; OUT : A = random number interval 0 <= x < A

	.ifndef	HUCC_NO_DEFAULT_RANDOM
		.alias	_srand.1		= init_random
		.alias	_rand			= get_random

_random.1:	pha				; Preserve the limit.
		jsr	get_random		; Get an 8-bit random number.
		ply				; Restore the limit.

		cpy	#128			; Check the limit.
		bcc	!+

		and	#$7F			; Just mask the random if
		cly				; the limit is >= 128.
		rts

!:		jsr	__muluchar		; This is __xsafe!
		tya				; If the limit is < 128 then
		cly				; do a 8.0 x 0.8 fixed point
		rts				; fractional multiply.
	.endif



; ***************************************************************************
; ***************************************************************************
;
; unsigned char __fastcall __builtin_ffs( unsigned int value<__temp> );

		.proc	___builtin_ffs.1

		lda.l	<__temp
		ldy	#-16

.search:	lsr.h	<__temp
		ror	a
		bcs	.found
		iny
		bne	.search
		bra	.finished

.found:		tya				; CS, return 17 + y.
		adc	#16

.finished:	tax				; Put return code in X.
		cly				; Return code in Y:X, X -> A.

		leave				; Return and copy X -> A.

		.endp
