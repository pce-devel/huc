; ***************************************************************************
; ***************************************************************************
;
; adpcm.asm
;
; Functions for developers using ADPCM on the PC Engine (with the MSM5205).
;
; These should be located in permanently-accessible memory!
;
; Copyright John Brandwood 2022.
;
; Distributed under the Boost Software License, Version 1.0.
; (See accompanying file LICENSE_1_0.txt or copy at
;  http://www.boost.org/LICENSE_1_0.txt)
;
; ***************************************************************************
; ***************************************************************************
;
; Developers would normally just use the System Card functions for ADPCM, but
; this library adds alternative functions, and allows ADPCM use on a HuCARD.
;
; N.B. : The adpcm_play() routine in this library, and the System Card's own
; ad_play() routine, both require having an IRQ2 handler for IFU_INT_STOP so
; that ADPCM playback is properly stopped when the sample is finished.
;
; If not developing for CD-ROM, the CORE library provides a suitable stub on
; on a HuCARD game.
;
; ***************************************************************************
; ***************************************************************************

; ===============================================
;
; SYSTEM CARD code, 70.75 cycles-per-byte!
;
; .loop:	lda	[_si]			; 7
;		sta	IFU_ADPCM_DAT		; 5
;
; !:		tst	#ADPCM_WR_BUSY, IFU_ADPCM_FLG	; 8
;		bne	!-			; 2
;
;		inc.l	<_si			; 6
;		bne	!+			; 4
;		jsr	inc.h_si_mpr34
;
; !:		lda.l	<_ax			; 4
;		bne	!+			; 4
;		dec.h	<_ax
; !:		dec.l	<_ax			; 6
;
;		lda.l	<_ax			; 4
;		ora.h	<_ax			; 4
;		bne	.loop			; 4
;
; ===============================================
;
; FASTEST code that tests ADPCM_WR_BUSY
;
; This shows that a minimum of 22 (29-7) cycles
; is needed between writes to ADPCM_DAT without
; causing the wait loop to trigger.
;
; !:		lda.l	<_ax			; Lo-byte of length.
;		eor	#$FF			; Negate the lo-byte of length.
;		tay
;		iny
;		beq	.loop			; Is lo-byte of length zero?
;
;		inc.h	<_ax			; Fix hi-byte for partial page.
;
;		eor	#$FF			; Calc address of partial page.
;		clc
;		adc.l	<_si
;		sta.l	<_si
;		bcs	.loop
;		dec.h	<_si
;
; .loop:	lda	[_si], y		; Copy data from RAM to ADPCM.
;		tax				;
;		lda	#ADPCM_WR_BUSY		; This 29-cycle-per-byte loop
; .wait:	bit	IFU_ADPCM_FLG		; is as fast as you can write
;		bne	.wait			; before that .wait loop gets
;		stx	IFU_ADPCM_DAT		; triggered, which results in
;		iny				; more slowdown than the gain.
;		bne	.loop
;		jsr	inc.h_si_mpr34
;		dec.h	<_ax			; Any full pages left to copy?
;		bne	.loop
;
; ===============================================
;
; ALTERNATIVE code for 28 or 30 cycles that tests ADPCM_WR_BUSY
;
; The 28-cycle version triggers wait loops, and the 30-cycle
; version seems a safer choice than the 29-cycle version.
;
; !:		lda.l	<_ax			; Lo-byte of length.
;		eor	#$FF			; Negate the lo-byte of length.
;		tay
;		iny
;		beq	.loop			; Is lo-byte of length zero?
;
;		inc.h	<_ax			; Fix hi-byte for partial page.
;
;		eor	#$FF			; Calc address of partial page.
;		clc
;		adc.l	<_si
;		sta.l	<_si
;		bcs	.loop
;		dec.h	<_si
;
; .loop:	lda	[_si], y		; Copy data from RAM to ADPCM.
; .wait:	tst	#ADPCM_WR_BUSY, IFU_ADPCM_FLG
;		bne	.wait			; This 30-cycle-per-byte loop
;		sta	IFU_ADPCM_DAT		; may be a bit safer than the
;		nop				; 29-cycle-per-byte minimum.
;		iny
;		bne	.loop
;		jsr	inc.h_si_mpr34
;		dec.h	<_ax			; Any full pages left to copy?
;		bne	.loop
;
; ===============================================
;
; FAST code that doesn't test ADPCM_WR_BUSY, while allowing 2-cycles for safety.
;
; This is nearly triple the write rate of the SYSTEM CARD code!
;
; .wait:	tst	#ADPCM_WR_BUSY, IFU_ADPCM_FLG
;		bne	.wait			; Sync at the start.
;
; .loop:	lda	[_si], y		; 7 Copy data from RAM to ADPCM.
;		sta	IFU_ADPCM_DAT		; 5
;		sxy				; 3
;		sxy				; 3
;		iny				; 2
;		bne	.loop			; 4 = 24
;		jsr	inc.h_si_mpr34
;		dec.h	<_ax			; Any full pages left to copy?
;		bne	.loop
;
; ===============================================

;
; Configure Library ...
;

	.ifndef	SUPPORT_ADPCM
SUPPORT_ADPCM	=	1
	.endif

;
; Include dependancies ...
;

		include "pcengine.inc"		; Common helpers.
		include "common.asm"		; Common helpers.



; ***************************************************************************
; ***************************************************************************
;
; Definitions
;



; ***************************************************************************
; ***************************************************************************
;
;
; Data
;

		.bss
		.code



; ***************************************************************************
; ***************************************************************************
;
; adpcm_reset - Reset ADPCM hardware (BIOS AD_RESET).
;
; The playback rate that's set here actually seems to matter, because the
; OKI chip needs the IFU to keep the reset low for at-least 2 clocks, and
; hardware certainly *appears* to be using the playback rate that's set.
;

adpcm_reset:	lda	#ADPCM_RESET
		sta	IFU_ADPCM_CTL
		stz	IFU_ADPCM_CTL

		stz	IFU_ADPCM_DMA		; Stops DMA from CD ... ???

		lda	#$6F			; All except IFU_INT_SUBC!
		trb	IFU_IRQ_MSK

;		stz	IFU_ADPCM_SPD		; BIOS sets 2KHz playback!
;		lda	#$0C			; Set playback rate 8KHz.
		lda	#$0E			; Set playback rate 16KHz.
		sta	IFU_ADPCM_SPD
		rts



; ***************************************************************************
; ***************************************************************************
;
; adpcm_stop -	(BIOS AD_STOP).
;
; N.B. Use cdr_ad_stop() on a CD/SCD game which also handles ADPCM streaming.
;

	.if	(CDROM == 0) + (SUPPORT_ADPCM == 0)

adpcm_stop:	lda	#IFU_INT_HALF + IFU_INT_STOP
		trb	IFU_IRQ_MSK

		lda	#ADPCM_PLAY + ADPCM_INCR
		trb	IFU_ADPCM_CTL
		rts

	.endif



; ***************************************************************************
; ***************************************************************************
;
; adpcm_stat - (BIOS AD_STAT).
;
; returns:
;  A = $00 if not busy (not playing or ended)
;  X = $00 playing and more than 50% left
;      $01 playback stopped
;      $04 playing and less than 50% left
;

adpcm_stat:	lda	IFU_ADPCM_FLG		; $01 if playback stopped
		and	#ADPCM_AD_STOP
		bne	.skip0
		lda	IFU_IRQ_FLG		; $04 if playing and less than 50% left
		and	#IFU_INT_HALF
.skip0:		tax

		lda     IFU_ADPCM_CTL		; $20 if playing
		and	#ADPCM_PLAY
		bne	.exit

		lda     IFU_ADPCM_FLG		; $08 if busy
		and	#ADPCM_AD_BUSY
.exit:		rts



; ***************************************************************************
; ***************************************************************************
;
; Set ADPCM RAM read pointer (source address ADPCM playback).
;

adpcm_set_src:	stx	IFU_ADPCM_LSB
		sty	IFU_ADPCM_MSB

		lda	#ADPCM_SET_RD
		tsb	IFU_ADPCM_CTL

		lda	IFU_ADPCM_DAT
		ldx	#4
.delay:		dex
		bne	.delay

		lda	#ADPCM_SET_RD
		trb	IFU_ADPCM_CTL
		rts



; ***************************************************************************
; ***************************************************************************
;
; Set ADPCM RAM write pointer (destination address for a CD read).
;

adpcm_set_dst:	stx	IFU_ADPCM_LSB
		sty	IFU_ADPCM_MSB

		lda	#ADPCM_SET_WR + ADPCM_WR_CLK
		tsb	IFU_ADPCM_CTL

		lda	#ADPCM_WR_CLK
		trb	IFU_ADPCM_CTL

		lda	#ADPCM_SET_WR
		trb	IFU_ADPCM_CTL
		rts



; ***************************************************************************
; ***************************************************************************
;
; Set ADPCM playback length.
;

adpcm_set_len:	stx	IFU_ADPCM_LSB
		sty	IFU_ADPCM_MSB

		lda	#ADPCM_SET_SZ
		tsb	IFU_ADPCM_CTL
		trb	IFU_ADPCM_CTL
		rts



; ***************************************************************************
; ***************************************************************************
;
; adpcm_read - Read from ADPCM RAM (similar to BIOS AD_READ).
;
; Args: _si, Y = _farptr to destination address to map into MPR3.
; Args: _ax = #bytes of ADPCM data to copy (1-65536).
; Args: _bx = Source address in ADPCM RAM.
;
; Returns: Y,Z-flag,N-flag = NZ if error.
;

adpcm_read	.proc

		tma3				; Preserve MPR3.
		pha

		jsr	set_si_to_mpr3		; Map destination to MPR3.

!:		jsr	adpcm_stat		; Wait for ADPCM not busy.
		bne	!-

		ldx.l	<_bx			; Set ADPCM read pointer.
		ldy.h	<_bx
		jsr	adpcm_set_src

!:		lda	IFU_ADPCM_FLG		; Bit7 = ADPCM_RD_BUSY
		bmi	!-
		lda	IFU_ADPCM_DAT		; Discard first byte.

		ldy.l	<_si			; Keep lo-byte of _si in Y.
		stz.l	<_si

		ldx.l	<_ax			; Fix 16-bit length for the
		beq	.loop			; decrement.
		inc.h	<_ax

.loop:		lda	IFU_ADPCM_FLG		; Bit7 = ADPCM_RD_BUSY
		bmi	.loop

		lda	IFU_ADPCM_DAT		; Copy byte from ADPCM to RAM.
		sta	[_si], y
		iny				; Increment destination.
		bne	!+
		jsr	inc.h_si_mpr3

!:		dex				; Decrement 16-bit length.
		bne	.loop
		dec.h	<_ax			; Any full pages left to copy?
		bne	.loop

		cly				; Return success code.

		pla				; Restore MPR3.
		tam3

		leave				; All done, phew!

		.endp



; ***************************************************************************
; ***************************************************************************
;
; adpcm_write - Write to ADPCM RAM (similar to BIOS AD_WRITE).
;
; Args: _si, Y = _farptr to source address to map into MPR3.
; Args: _ax = #bytes of ADPCM data to copy (1-65536).
; Args: _bx = Destinaion address in ADPCM RAM.
;
; Returns: Y,Z-flag,N-flag = NZ if error.
;

adpcm_write	.proc

		tma3				; Preserve MPR3.
		pha

		jsr	set_si_to_mpr3		; Map source to MPR3.

		lda	IFU_ADPCM_DMA		; Is there DMA going on?
		and	#$03
		tay				; Return NZ as error code.
		bne	.finished

		lda.l	<_bx			; Continue from previous addr
		and.h	<_bx			; if ADPCM address is $FFFF.
		cmp	#$FF
		beq	!+

		ldx.l	<_bx			; Set ADPCM write pointer.
		ldy.h	<_bx
		jsr	adpcm_set_dst

!:		ldy.l	<_si			; Keep lo-byte of _si in Y.
		stz.l	<_si

		ldx.l	<_ax			; Fix 16-bit length for the
		beq	!+			; decrement.
		inc.h	<_ax

!:		tst	#ADPCM_WR_BUSY, IFU_ADPCM_FLG
		bne	!-

.loop:		lda	[_si], y		; Copy data from RAM to ADPCM.
		sta	IFU_ADPCM_DAT		;
		iny				; This 24-cycle-per-byte loop
		bne	!+			; is as fast as you can write
		jsr	inc.h_si_mpr3		; and not check the BUSY flag.

!:		dex
		bne	.loop
		dec.h	<_ax			; Any full pages left to copy?
		bne	.loop

		cly				; Return success code.

.finished:	pla				; Restore MPR3.
		tam3

		leave				; All done, phew!

		.endp



; ***************************************************************************
; ***************************************************************************
;
; adpcm_play - Play sample from ADPCM RAM (similar to BIOS AD_PLAY).
;
; Args: _ax = #bytes of ADPCM data to play. (0-65535).
; Args: _bx = Playback address in ADPCM RAM.
; Args: _dh : Sample rate (0..14), the playback rate is 32KHz / (16 - _dh).
; Args: _dl :
;       mode b0 = 0 if new sample
;		  1 if repeat last
;	mode b7 = 0 if stop at end
;		  1 if loop at end (not supported here)
;
; Returns: Y,Z-flag,N-flag = NZ if error.
;

		.procgroup

adpcm_play	.proc

		jsr	adpcm_stat		; Is the ADPCM busy?
		bne	.exit

		lda	<_dl			; Save LOOP mode for checking
		and	#$80			; in IRQ2 on IFU_INT_STOP.
		sta	ad_play_mode

		lda	<_dl			; Repeat last sample?
		and	#$7F
		bne	.trigger

		lda.l	<_bx			; Remember address for repeat
		sta.l	ad_play_addr		; or looping on IFU_INT_STOP.
		lda.h	<_bx
		sta.h	ad_play_addr

		lda.l	<_ax			; Remember length for repeat
		sta.l	ad_play_size		; or looping on IFU_INT_STOP.
		lda.h	<_ax
		sta.h	ad_play_size

		lda	<_dh			; Check sample rate.
		cmp	#$0F
		bcs	.exit			; Abort if illegal.
		sta	ad_play_rate		; Remember playback rate.

!:		lda	ad_play_rate		; Set playback rate.
		sta	IFU_ADPCM_SPD

		ldx.l	ad_play_addr		; Set playback address.
		ldy.h	ad_play_addr		; This clears IFU_INT_HALF,
		jsr	adpcm_set_src           ; but not IFU_INT_STOP.

.trigger:	ldx.l	ad_play_size		; Set playback length.
		ldy.h	ad_play_size		; This clears IFU_INT_STOP,
		jsr	adpcm_set_len           ; but not IFU_INT_HALF.

		lda	#IFU_INT_STOP		; Enable interrupt when done.
		tsb	IFU_IRQ_MSK

		lda	#ADPCM_PLAY + ADPCM_INCR; Start sample playback.
		sta	IFU_ADPCM_CTL

		cla				; Return success code.
.exit:		tay

		leave				; All done, phew!

		.endp

		; Entry point to loop the sample on IFU_INT_STOP.

adpcm_loop:	.proc
		bra	!-
		.endp

		.endprocgroup
