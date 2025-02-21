; ***************************************************************************
; ***************************************************************************
;
; hucc-baselib.asm
;
; Basic library functions provided (mostly) as macros.
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
; void __fastcall dump_screen( void );
;
; THIS IS AN ILLEGAL INSTRUCTION ONLY IMPLEMENTED BY THE TGEMU EMULATOR!

_dump_screen:	db	0x33



; ***************************************************************************
; ***************************************************************************
;
; void __fastcall abort( void );
;
; THIS IS AN ILLEGAL INSTRUCTION ONLY IMPLEMENTED BY THE TGEMU EMULATOR!

_abort:		db	0xE2



; ***************************************************************************
; ***************************************************************************
;
; void __fastcall exit( int value<acc> );
;
; THIS IS AN ILLEGAL INSTRUCTION ONLY IMPLEMENTED BY THE TGEMU EMULATOR!

_exit.1:	tax				; Put the return code into X.
		db	0x63

.hang:		bra	.hang			; Hang if used in normal code.



; ***************************************************************************
; ***************************************************************************
;
; unsigned char __fastcall __macro cd_execoverlay( unsigned char ovl_index<acc> );
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
; void __fastcall __nop set_far_base( unsigned char data_bank<_bp_bank>, unsigned char *data_addr<_bp> );
; void __fastcall set_far_offset( unsigned int offset<_bp>, unsigned char data_bank<_bp_bank>, unsigned char *data_addr<acc> );

_set_far_offset.3:
		clc
		adc.l	<_bp
		sta.l	<_bp
		tya
		and	#$1F
		adc.h	<_bp
		tay
		and	#$1F
		ora	#$60
		sta.h	<_bp
		tya
		ror	a
		lsr	a
		lsr	a
		lsr	a
		lsr	a
		clc
		adc	<_bp_bank
		sta	<_bp_bank
		rts



; ***************************************************************************
; ***************************************************************************
;
; unsigned char __fastcall __macro ac_exists( void );

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
; unsigned char __fastcall __macro _sgx_detect( void );

_sgx_detect	.macro
		lda	sgx_detected
		cly
		.endm



; ***************************************************************************
; ***************************************************************************
;
; unsigned int __fastcall __macro peek( unsigned int addr<__ptr> );

_peek.1		.macro
		lda	[__ptr]
		cly
		.endm



; ***************************************************************************
; ***************************************************************************
;
; unsigned int __fastcall __macro peekw( unsigned int addr<__ptr> );

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
; void __fastcall __macro poke( unsigned int addr<__poke>, unsigned char with<acc> );
;
; N.B. Because the <acc> value can be a complex C calculation, it isn't safe
; to use __ptr as the destination, which can be overwritten in C macros.

_poke.2		.macro
		sta	[__poke]
		.endm



; ***************************************************************************
; ***************************************************************************
;
; void __fastcall __macro pokew( unsigned int addr<__poke>, unsigned int with<acc> );
;
; N.B. Because the <acc> value can be a complex C calculation, it isn't safe
; to use __ptr as the destination, which can be overwritten in C macros.

_pokew.2	.macro
		sta	[__poke]
		tya
		ldy	#1
		sta	[__poke], y
		.endm



; ***************************************************************************
; ***************************************************************************
;
; unsigned char __fastcall __macro clock_hh( void );

_clock_hh	.macro
		lda	clock_hh
		cly
		.endm



; ***************************************************************************
; ***************************************************************************
;
; unsigned char __fastcall __macro clock_mm( void );

_clock_mm	.macro
		lda	clock_mm
		cly
		.endm



; ***************************************************************************
; ***************************************************************************
;
; unsigned char __fastcall __macro clock_ss( void );

_clock_ss	.macro
		lda	clock_ss
		cly
		.endm



; ***************************************************************************
; ***************************************************************************
;
; unsigned char __fastcall __macro clock_tt( void );

_clock_tt	.macro
		lda	clock_tt
		cly
		.endm



; ***************************************************************************
; ***************************************************************************
;
; void __fastcall __macro clock_reset( void );

_clock_reset	.macro
		stz	clock_hh
		stz	clock_mm
		stz	clock_ss
		stz	clock_tt
		.endm



; ***************************************************************************
; ***************************************************************************
;
; void __fastcall __macro vsync( void );
; void __fastcall __macro vsync( unsigned char count<acc> );

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
; unsigned int __fastcall __macro joy( unsigned char which<acc> );

_joy.1		.macro
		tax
		lda	joynow, x
	.if	SUPPORT_6BUTTON
		ldy	joy6now, x
	.else
		cly
	.endif
		.endm



; ***************************************************************************
; ***************************************************************************
;
; unsigned int __fastcall __macro joytrg( unsigned char which<acc> );

_joytrg.1	.macro
		tax
		lda	joytrg, x
	.if	SUPPORT_6BUTTON
		ldy	joy6trg, x
	.else
		cly
	.endif
		.endm



; ***************************************************************************
; ***************************************************************************
;
; unsigned int __fastcall __macro joybuf( unsigned char which<acc> );

_joybuf.1	.macro
	.if	HUC_JOY_EVENTS
		tax
		lda	joybuf, x
	.if	SUPPORT_6BUTTON
		ldy	joy6buf, x
	.else
		cly
	.endif
	.else
		.fail	You must enable HUC_JOY_EVENTS in your hucc-config.inc!
	.endif
		.endm



; ***************************************************************************
; ***************************************************************************
;
; unsigned int __fastcall __macro get_joy_events( unsigned char which<acc> );
;
; N.B. This is just a version of joybuf() that clears the accumulated events.

		.macro	_get_joy_events.1
	.if	HUC_JOY_EVENTS
		tax
		lda	joybuf, x
		stz	joybuf, x
	.if	SUPPORT_6BUTTON
		ldy	joy6buf, x
		stz	joy6buf, x
	.else
		cly
	.endif
	.else
	.if	ACCUMULATE_JOY
		tax
		lda	joytrg, x
		stz	joytrg, x
	.if	SUPPORT_6BUTTON
		ldy	joy6trg, x
		stz	joy6trg, x
	.else
		cly
	.endif
	.else
		.fail	You must enable HUC_JOY_EVENTS or ACCUMULATE_JOY in your hucc-config.inc!
	.endif
	.endif
		.endm



; ***************************************************************************
; ***************************************************************************
;
; void __fastcall __macro clear_joy_events( unsigned char mask<acc> );

		.macro	_clear_joy_events.1
		php
		sei
		and	#(1 << MAX_PADS) - 1
		ldx	#$FF
.loop:		inx
		lsr	a
		bcc	.next
	.if	HUC_JOY_EVENTS
		stz	joybuf, x
	.if	SUPPORT_6BUTTON
		stz	joy6buf, x
	.endif
	.else
	.if	ACCUMULATE_JOY
		stz	joytrg, x
	.if	SUPPORT_6BUTTON
		stz	joy6trg, x
	.endif
	.else
		.fail	You must enable HUC_JOY_EVENTS or ACCUMULATE_JOY in your hucc-config.inc!
	.endif
	.endif
.next:		bne	.loop
		plp
		.endm



; ***************************************************************************
; ***************************************************************************
;
; void __fastcall _nop set_color( unsigned int index<VCE_CTA>, unsigned int value<VCE_CTW> );
; void __fastcall set_color_rgb( unsigned int index<VCE_CTA>, unsigned char r<_al>, unsigned char g<_ah>, unsigned char b<acc> );
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
; void __fastcall _macro get_color( unsigned int index<VCE_CTA> );

_get_color.1	.macro
		lda.l	VCE_CTR
		ldy.h	VCE_CTR
		.endm



; ***************************************************************************
; ***************************************************************************
;
; void __fastcall srand( unsigned char seed<acc> );
; unsigned int __fastcall rand( void );
; unsigned char __fastcall rand8( void );

	.ifndef	HUCC_NO_DEFAULT_RANDOM
		.alias	_srand.1		= init_random

_rand:		jsr	get_random		; Random in A, preserve Y.
		tay
		jmp	get_random		; Random in A, preserve Y.
	.endif



; ***************************************************************************
; ***************************************************************************
;
; unsigned char __fastcall random8( unsigned char limit<acc> );
;
; IN :	A = range (0..255)
; OUT : A = random number interval 0 <= x < A

	.ifndef	HUCC_NO_DEFAULT_RANDOM

_random8.1:	tay				; Preserve the limit.
		jsr	get_random		; Random in A, preserve Y.

		jsr	__muluchar
		tya				; Do a 8.0 x 0.8 fixed point
		cly				; fractional multiply.
		rts
	.endif



; ***************************************************************************
; ***************************************************************************
;
; unsigned char __fastcall random( unsigned char limit<acc> );
;
; IN :	A = range (0..128), 129..255 is treated as 128
; OUT : A = random number interval 0 <= x < A

	.ifndef	HUCC_NO_DEFAULT_RANDOM

_random.1:	tay				; Preserve the limit.
		jsr	get_random		; Random in A, preserve Y.

		cpy	#128			; Check the limit.
		bcc	!+

		and	#$7F			; Just mask the random if
		cly				; the limit is >= 128.
		rts

!:		jsr	__muluchar
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



; ***************************************************************************
; ***************************************************************************
;
; N.B. Declared in hucc-gfx.h, but defined here because they're macros!
;
; void __fastcall _macro set_xres( unsigned int x_pixels<_ax> );
; void __fastcall _macro sgx_set_xres( unsigned int x_pixels<_ax> );
;
; void __fastcall set_xres( unsigned int x_pixels<_ax>, unsigned char blur_flag<_bl> );
; void __fastcall sgx_set_xres( unsigned int x_pixels<_ax>, unsigned char blur_flag<_bl> );

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
; unsigned int __fastcall __macro vram_addr( unsigned char bat_x<_al>, unsigned char bat_y<_ah> );
; unsigned int __fastcall __macro sgx_vram_addr( unsigned char bat_x<_al>, unsigned char bat_y<_ah> );

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
; unsigned int __fastcall __macro get_vram( unsigned int address<_di> );
; void __fastcall __macro put_vram( unsigned int address<_di>, unsigned int data<acc> );
;
; unsigned int __fastcall __macro sgx_get_vram( unsigned int address<_di> );
; void __fastcall __macro sgx_put_vram( unsigned int address<_di>, unsigned int data<acc> );

		.macro	_get_vram.1
		jsr	vdc_di_to_marr
		lda	VDC_DL
		ldy	VDC_DH
		.endm

		.macro	_put_vram.2
		pha
		jsr	vdc_di_to_mawr
		pla
		sta	VDC_DL
		sty	VDC_DH
		.endm

	.if	SUPPORT_SGX
		.macro	_sgx_get_vram.1
		jsr	sgx_di_to_marr
		lda	SGX_DL
		ldy	SGX_DH
		.endm

		.macro	_sgx_put_vram.2
		pha
		jsr	sgx_di_to_mawr
		pla
		sta	SGX_DL
		sty	SGX_DH
		.endm
	.endif



; ***************************************************************************
; ***************************************************************************
;
; N.B. Declared in hucc-gfx.h, but defined here because they're macros!
;
; void __fastcall __macro set_bgpal( unsigned char palette<_al>, unsigned char __far *data<_bp_bank:_bp> );
; void __fastcall __macro set_bgpal( unsigned char palette<_al>, unsigned char __far *data<_bp_bank:_bp>, unsigned int num_palettes<_ah> );
; void __fastcall __macro set_sprpal( unsigned char palette<_al>, unsigned char __far *data<_bp_bank:_bp> );
; void __fastcall __macro set_sprpal( unsigned char palette<_al>, unsigned char __far *data<_bp_bank:_bp>, unsigned int num_palettes<_ah> );

_set_bgpal.2	.macro
		lda	#1
		sta	<_ah
		call	_load_palette.3
		.endm

_set_bgpal.3	.macro
		call	_load_palette.3
		.endm

_set_sprpal.2	.macro
		lda	#1
		sta	<_ah
		smb4	<_al
		call	_load_palette.3
		.endm

_set_sprpal.3	.macro
		smb4	<_al
		call	_load_palette.3
		.endm



; ***************************************************************************
; ***************************************************************************
;
; N.B. Declared in hucc-gfx.h, but defined here because they're macros!
;
; void __fastcall __macro load_sprites( unsigned int vram<_di>, unsigned char __far *data<_bp_bank:_bp>, unsigned int num_groups<acc> );
; void __fastcall __macro sgx_load_sprites( unsigned int vram<_di>, unsigned char __far *data<_bp_bank:_bp>, unsigned int num_groups<acc> );
; void __fastcall __macro far_load_sprites( unsigned int vram<_di>, unsigned int num_groups<acc> );
; void __fastcall __macro sgx_far_load_sprites( unsigned int vram<_di>, unsigned int num_groups<acc> );

	.if	SUPPORT_SGX
		.macro	_sgx_load_sprites.3
		stz.l	<_ax
		asl	a
		sta.h	<_ax
		call	_sgx_load_vram.3
		.endm

		.macro	_sgx_far_load_sprites.2
		stz.l	<_ax
		asl	a
		sta.h	<_ax
		call	_sgx_load_vram.3
		.endm
	.endif

		.macro	_load_sprites.3
		stz.l	<_ax
		asl	a
		sta.h	<_ax
		call	_load_vram.3
		.endm

		.macro	_far_load_sprites.2
		stz.l	<_ax
		asl	a
		sta.h	<_ax
		call	_load_vram.3
		.endm



; ***************************************************************************
; ***************************************************************************

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
