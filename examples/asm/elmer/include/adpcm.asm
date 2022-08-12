; ***************************************************************************
; ***************************************************************************
;
; adpcm.asm
;
; Functions for developers using ADPCM on the PC Engine (with the MSM5205).
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
; Include dependancies ...
;

		include "pcengine.inc"		; Common helpers.



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

adpcm_reset:	lda	#ADPCM_RESET
		sta	IFU_ADPCM_CTL
		stz	IFU_ADPCM_CTL

		stz	IFU_ADPCM_DMA		; Stops DMA from CD ... ???

		lda	#$6F			; All except IFU_INT_SUBC!
		trb	IFU_IRQ_MSK

		stz	IFU_ADPCM_SPD		; 2KHz playback, hah!
		rts



; ***************************************************************************
; ***************************************************************************
;
; adpcm_stop -	(BIOS AD_STOP).
;
; N.B. Use cdr_ad_stop() on a CD/SCD game which also handles ADPCM streaming.
;

	.if	!CDROM

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
		and	#ADPCM_AD_END
		bne	.skip0
		lda	IFU_IRQ_FLG		; $04 if playing and less than 50% left
		and	#IFU_INT_HALF
.skip0:		tax

		lda     IFU_ADPCM_CTL		; $20 if playing or ended (don't know which)
		and	#ADPCM_PLAY
		bne	.exit

		lda     IFU_ADPCM_FLG		; $08 if playing or ended (don't know which)
		and	#ADPCM_AD_BSY
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
.wait:		dex
		bne	.wait

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
; Set ADPCM playback rate.
;

adpcm_set_spd:	sta	IFU_ADPCM_SPD
		rts
