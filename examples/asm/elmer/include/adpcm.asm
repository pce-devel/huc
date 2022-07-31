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
; Reset ADPCM hardware (UNUSED HERE).
;

adpcm_reset:	lda	#ADPCM_RESET
		tsb	IFU_ADPCM_CTL
		trb	IFU_ADPCM_CTL
		rts



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
