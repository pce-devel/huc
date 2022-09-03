; ***************************************************************************
; ***************************************************************************
;
; okitest.asm
;
; OKI ADPCM tester HuCARD example.
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
; The purpose of this example is to compare the accuracy of emulation with
; real console hardware.
;
; ***************************************************************************
; ***************************************************************************
;
; The ADPCM PLAY FLAGS test #1 plays back sample 4 times, and then shows
; the control and status flags at various stages.
;
; Values of IFU_ADPCM_CTL ($180D), IFU_ADPCM_FLG ($180C), IFU_IRQ_FLG ($1803)
;
; A) After 1st playback is finished
; B) After clear ADPCM_PLAY and ADPCM_AUTO
; C) After clear ADPCM_PLAY and ADPCM_AUTO + 1 millisecond
; D) After set new ADPCM read address (which clears IFU_INT_HALF bit)
; E) After set new ADPCM length (which clears IFU_INT_END bit)
; F) After set ADPCM_AUTO
; G) After set ADPCM_AUTO + 1 millisecond
; H) After set ADPCM_PLAY
; I) After set ADPCM_PLAY + 1 millisecond
; J) After playing for 1.5 seconds
; K) After 2nd playback flags change
; L) After 2nd playback is finished
; M) After clear ADPCM_PLAY and ADPCM_AUTO
; N) After clear ADPCM_PLAY and ADPCM_AUTO and ADPCM_AD_BSY is clear
; O) After 3rd playback is started
; P) After playing for 1 second
; Q) After adpcm_reset() to terminate playback early
; R) After adpcm_reset() and ADPCM_AD_BSY is clear (approx 500us at 16KHz)
; S) After 4th playback is started
; T) After 4th playback is finished
; U) After clear ADPCM_PLAY and ADPCM_AUTO
;
; ***************************************************************************
; ***************************************************************************
;
; The ADPCM PLAY FLAGS test #2 plays back sample once, and then read/writes
; bytes and changes the LENGTH in order to check when INT_END and INT_HALF
; change.
;
; Values of IFU_ADPCM_CTL ($180D), IFU_ADPCM_FLG ($180C), IFU_IRQ_FLG ($1803)
;
; A) After 1st playback is finished
; B) After clear ADPCM_PLAY and ADPCM_AUTO
; C) After clear ADPCM_PLAY and ADPCM_AUTO + 1 millisecond
; D) After reading a byte, which clears IFU_INT_HALF bit. (LENGTH = $FFFF?)
; E) After setting LENGTH=$0002
; F) After reading a byte. (LENGTH = $0001?)
; G) After reading a byte. (LENGTH = $0000?)
; H) After reading a byte. (LENGTH = $FFFF?)
; I) After setting LENGTH=$8002
; J) After reading a byte. (LENGTH = $8001?)
; K) After reading a byte. (LENGTH = $8000?)
; L) Immediately after reading a byte. (LENGTH = $7FFF?)
; M) 24-cycles later. (LENGTH = $7FFF?) INT_HALF changes AFTER IFU reads.
; N) After reading a byte. (LENGTH = $7FFE?)
; O) After setting LENGTH=$7FFE
; P) After writing a byte. (LENGTH = $7FFF?)
; Q) Immediately after writing a byte. (LENGTH = $8000?)
; R) 36-cycles later. (LENGTH = $8000?)
; S) Immediately after writing a byte. (LENGTH = $8001?)
; T) 36-cycles later. (LENGTH = $8001?) INT_HALF cleared AFTER IFU writes.
; U)
;
; ***************************************************************************
; ***************************************************************************


		;
		; Create some equates for a very generic VRAM layout, with a
		; 64*32 BAT, followed by the SAT, then followed by the tiles
		; for the ASCII character set.
		;
		; This uses the first 8KBytes of VRAM ($0000-$0FFF).
		;

BAT_LINE	=	64
BAT_SIZE	=	64 * 32
SAT_ADDR	=	BAT_SIZE		; SAT takes 16 tiles of VRAM.
CHR_ZERO	=	BAT_SIZE / 16		; 1st tile # after the BAT.
CHR_0x10	=	CHR_ZERO + 16		; 1st tile # after the SAT.
CHR_0x20	=	CHR_ZERO + 32		; ASCII ' ' CHR tile #.

		;
		; You would normally just set this in your project's local
		; "core-config.inc", but this shows another way to achieve
		; the same result.
		;

SUPPORT_ADPCM	=	1	; Support the replacement ADPCM functions.

		;
		; Include the library, reading the project's configuration
		; settings from the local "core-config.inc", if it exists.
		;

		include "core.inc"		; Basic includes.

		.list
		.mlist

		include	"common.asm"		; Common helpers.
		include	"vdc.asm"		; Useful VDC routines.
		include	"font.asm"		; Useful font routines.
		include	"tty.asm"		; Useful TTY print routines.
		include	"adpcm.asm"		; Useful ADPCM routines.



; ***************************************************************************
; ***************************************************************************
;
; Constants and Variables.
;

WAIT_LINES	=	22			; #lines to wait for TIA.

		.zp

play_rate:	ds	1
xfer_size:	ds	2

which_rcr:	ds	1

;adpcm_flg0:	ds	1
;adpcm_ctl0:	ds	1
;adpcm_flg1:	ds	1
;adpcm_ctl1:	ds	1

min_cycles	ds	2
max_cycles	ds	2
num_cycles	ds	2 * 2

screen		ds	1
state		ds	1
array		ds	4 * 26

		.bss

ram_nop		ds	6144			; TIA + lots of delay after.



; ***************************************************************************
; ***************************************************************************
;
; core_main - This is executed after "CORE(not TM)" library initialization.
;
; This is the first code assembled after the library includes, so we're still
; in the CORE_BANK, usually ".bank 0"; and because this is assembled with the
; default configuration from "include/core-config.inc", which sets the option
; "USING_MPR7", then we're running in MPR7 ($E000-$FFFF).
;

		.code

core_main:	; Turn the display off and initialize the screen mode.

		call	init_256x224		; Initialize VDC & VRAM.

		; Upload the font to VRAM.

		stz	<_di + 0		; Destination VRAM address.
		lda	#>(CHR_0x10 * 16)
		sta	<_di + 1

		lda	#$FF			; Put font in colors 4-7,
		sta	<_al			; so bitplane 2 = $FF and
		stz	<_ah			; bitplane 3 = $00.

		lda	#16 + 96		; 16 box graphics + 96 ASCII.
		sta	<_bl

		lda	#<my_font		; Address of font data.
		sta	<_si + 0
		lda	#>my_font
		sta	<_si + 1
		ldy	#^my_font

		call	dropfnt8x8_vdc		; Upload font to VRAM.

		; Upload the palette data to the VCE.

		stz	<_al			; Start at palette 0 (BG).
		lda	#4			; Copy 4 palettes of 16 colors.
		sta	<_ah
		lda	#<screen_pal		; Set the ptr to the palette
		sta	<_si + 0		; data.
		lda	#>screen_pal
		sta	<_si + 1
		ldy	#^screen_pal
		call	load_palettes		; Add to the palette queue.

		call	xfer_palettes		; Transfer queue to VCE now.

		;

		call	adpcm_reset		; Reset the ADPCM hardware.

		lda.l	#adpcm_size		; ADPCM size.
		sta.l	<_ax
		lda.h	#adpcm_size
		sta.h	<_ax
		lda.l	#adpcm_size * 0		; ADPCM addr.
		sta.l	<_bx
		lda.h	#adpcm_size * 0
		sta.h	<_bx
		lda	#<adpcm_data		; Source addr.
		sta	<_si + 0
		lda	#>adpcm_data
		sta	<_si + 1
		ldy	#^adpcm_data
		call	adpcm_write		; Write the ADPCM data.

		lda.l	#adpcm_size		; ADPCM size.
		sta.l	<_ax
		lda.h	#adpcm_size
		sta.h	<_ax
		lda.l	#adpcm_size * 1		; ADPCM addr.
		sta.l	<_bx
		lda.h	#adpcm_size * 1
		sta.h	<_bx
		lda	#<adpcm_data		; Source addr.
		sta	<_si + 0
		lda	#>adpcm_data
		sta	<_si + 1
		ldy	#^adpcm_data
		call	adpcm_write		; Write the ADPCM data.

		; Set the rest of ADPCM RAM to $08 (silence).

		ldy.h	#65536 - (adpcm_size * 2)
		ldx.l	#65536 - (adpcm_size * 2)
		beq	.fill
		iny
.fill:		lda	#ADPCM_WR_BSY
!:		bit	IFU_ADPCM_FLG
		bne	!-
		lda	#$08
		sta	IFU_ADPCM_DAT
		dex
		bne	.fill
		dey
		bne	.fill

		; Confirm correct read/write.

		lda.l	#adpcm_size		; ADPCM size.
		sta.l	<_ax
		lda.h	#adpcm_size
		sta.h	<_ax
		lda.l	#adpcm_size * 1		; ADPCM addr.
		sta.l	<_bx
		lda.h	#adpcm_size * 1
		sta.h	<_bx
		lda	#<adpcm_data		; Source addr.
		sta	<_si + 0
		lda	#>adpcm_data
		sta	<_si + 1
		ldy	#^adpcm_data
		call	adpcm_check		; Check the ADPCM data.
!:		bne	!-

		; Turn on the BG & SPR layers, then wait for a soft-reset.

		call	set_dspon		; Enable background.

		; Set up the TIA in ram, if it's not already done.

		lda	ram_nop
		bne	.start_test

		php
		sei
		lda	#14
		sta	play_rate
		lda	#64
		sta.l	xfer_size
		stz.h	xfer_size

		tai	.nopnop, ram_nop, 6144	; Setup NOP subroutine.
		lda	#$60			; RTS
		sta	ram_nop + 6143

		bit	VDC_SR
		plp

		; Begin with the playback test.

		jmp	.other_test

		; Enable the IRQ test processing.

.start_test:	lda.l	#vsync_proc
		sta.l	vsync_hook
		lda.h	#vsync_proc
		sta.h	vsync_hook
		lda	#$10
		tsb	<irq_vec

		; Loop around updating the display each frame.

		PRINTF	"\e<\f\eX1\eY1\eP0**PC ENGINE ADPCM WRITE RATE**\eP1\eX2\eY3\x1E\x1F\eP0:Speed\eP1 \x1C\x1D\eP0:Bytes\eP1 SEL\eP0:Sample\eP1\eX9\eY4RUN\eP0:Other Test\n"

		lda	#1			; Delay first VBLANK cycles
		sta	irq_cnt			; update.

.new_change:	lda.l	#9999			; Reset max and min cycles values.
		sta.l	min_cycles
		lda.h	#9999
		sta.h	min_cycles
		stz.l	max_cycles
		stz.h	max_cycles
		bra	.show_frame

.next_frame:	lda.l	num_cycles		; Update max and min cycle values.
		cmp.l	min_cycles
		lda.h	num_cycles
		beq	!+
		sbc.h	min_cycles
		bcs	!+

		lda.l	num_cycles
		sta.l	min_cycles
		lda.h	num_cycles
		sta.h	min_cycles

!:		lda.l	num_cycles
		cmp.l	max_cycles
		lda.h	num_cycles
		sbc.h	max_cycles
		bcc	.show_frame

		lda.l	num_cycles
		sta.l	max_cycles
		lda.h	num_cycles
		sta.h	max_cycles

.show_frame:	PRINTF	"\eXL1\eY5\nADPCM playback rate:     %5hu\n", play_rate

		PRINTF	"ADPCM write length:      %5u\n", xfer_size

		lda	irq_cnt			; Only update this 4 times a second!
		bit	#$0F
		bne	.skip_update

		PRINTF	"Cycles taken for write:  %5u\n", num_cycles + 0
		PRINTF	"Minimum cycles taken:    %5u\n", min_cycles
		PRINTF	"Maximum cycles taken:    %5u\n", max_cycles

.skip_update:	lda	irq_cnt			; Wait for vsync.

.wait_frame:	ds	256,$EA			; LOTs of 2-cycle "NOP" opcodes
		ds	128,$EA			; so that IRQ response is ASAP.

		cmp	irq_cnt
		bne	.got_vblank
		jmp	.wait_frame

.got_vblank:	call	do_autorepeat		; Autorepeat UDLR buttons.

		lda	joytrg + 0		; Read the joypad.
		bit	#JOY_U
		bne	.rate_up
		bit	#JOY_D
		bne	.rate_down
		bit	#JOY_R
		bne	.size_up
		bit	#JOY_L
		bne	.size_down
		bit	#JOY_SEL
		bne	.play_adpcm
		bit	#JOY_RUN
		bne	.other_test

		jmp	.next_frame		; Wait for user to reboot.

; Change playback rate.

.rate_up:	lda.l	<play_rate		; Increase playback rate.
		cmp.l	#14
		bcs	.ru_done
		inc.l	<play_rate
.ru_done:	lda.l	<play_rate
		sta	IFU_ADPCM_SPD
		jmp	.new_change

.rate_down:	lda.l	<play_rate		; Reduce playback rate.
		cmp.l	#0 + 1
		bcc	.rd_done
		dec.l	<play_rate
.rd_done:	lda.l	<play_rate
		sta	IFU_ADPCM_SPD
		jmp	.new_change

; Change transfer size.

.size_up:	lda.l	<xfer_size		; Increase transfer size.
		cmp.l	#256
		lda.h	<xfer_size
		sbc.h	#256
		bcs	.su_done

		lda.l	<xfer_size
		adc	#32
		sta.l	<xfer_size
		bcc	.su_done
		inc.h	<xfer_size

.su_done:	jmp	.new_change

.size_down:	lda.l	<xfer_size		; Reduce transfer size.
		cmp.l	#32 + 1
		lda.h	<xfer_size
		sbc.h	#32 + 1
		bcc	.sd_done

		lda.l	<xfer_size
		sbc	#32
		sta.l	<xfer_size
		bcs	.sd_done
		dec.h	<xfer_size

.sd_done:	jmp	.new_change

; Play a sample.

.play_adpcm:	jsr	adpcm_stat		; Is the ADPCM busy?
		bne	.stop_adpcm

		lda.l	#adpcm_size		; ADPCM size.
		sta.l	<_ax
		lda.h	#adpcm_size
		sta.h	<_ax
		lda.l	#adpcm_size * 1		; ADPCM addr.
		sta.l	<_bx
		lda.h	#adpcm_size * 1
		sta.h	<_bx
		stz	<_dl			; ADPCM mode.
		lda	<play_rate		; ADPCM rate.
		sta	<_dh
		call	adpcm_play		; Play an ADPCM sample.
		jmp	.new_change

.stop_adpcm:	call	adpcm_reset
		jmp	.new_change

; Perform 2nd test.

.other_test:
;		jsr	core_clr_hooks

		lda	#$10			; Disable VBLANK handler.
		trb	<irq_vec
		lda	#1			; Clear RCR IRQ.
		sta	<which_rcr
		jsr	set_next_rcr

		jsr	check_flags
		jsr	wait_vsync

		jmp	.start_test

; Simple VBLANK IRQ handler for the CORE library.

.vblank_irq:	cli				; Allow RCR and TIMER IRQ.
		jmp	xfer_palettes		; Transfer queue to VCE now.

;

.nopnop:	dw	$EAEA



; ***************************************************************************
; ***************************************************************************
;
; check_flags - Check the ADPCM flag states during playback.
;

check_flags:	PRINTF	"\e<\f\eXL0\eX1\eY1\eP0**PC ENGINE ADPCM PLAY FLAGS**\eP1\eX9\eY3RUN\eP0:Other Test\n\n"

		tai	const_0000, array, 4 * 26

		jsr	adpcm_reset

!:		jsr	adpcm_stat		; Is the ADPCM busy?
		bne	!-

		;
		; Play back the 1st sample (one loop "You're all going to die").
		;

		lda	#$0E			; Set playback rate.
		sta	IFU_ADPCM_SPD

		ldx.l	#adpcm_size * 0		; Set playback address.
		ldy.h	#adpcm_size * 0		; This clears IFU_INT_HALF,
		jsr	adpcm_set_src		; but not IFU_INT_END.

		ldx.l	#adpcm_size * 1		; Set playback length.
		ldy.h	#adpcm_size * 1		; This clears IFU_INT_END,
		jsr	adpcm_set_len		; but not IFU_INT_HALF.

		lda	#ADPCM_PLAY + ADPCM_AUTO; Start sample playback.
		sta	IFU_ADPCM_CTL

!:		tst	#ADPCM_AD_END, IFU_ADPCM_FLG
		beq	!-

		ldx	IFU_ADPCM_CTL		; State A
		ldy	IFU_ADPCM_FLG
		lda	IFU_IRQ_FLG
		stx	array + 0 * 4 + 0
		sty	array + 0 * 4 + 1
		and	#$0C
		sta	array + 0 * 4 + 2

		lda	#ADPCM_PLAY + ADPCM_AUTO; Stop sample playback.
		trb	IFU_ADPCM_CTL

		ldx	IFU_ADPCM_CTL		; State B
		ldy	IFU_ADPCM_FLG
		lda	IFU_IRQ_FLG
		stx	array + 1 * 4 + 0
		sty	array + 1 * 4 + 1
		and	#$0C
		sta	array + 1 * 4 + 2

		ldy	#2			; 1ms delay.
		jsr	.delay

		ldx	IFU_ADPCM_CTL		; State C
		ldy	IFU_ADPCM_FLG
		lda	IFU_IRQ_FLG
		stx	array + 2 * 4 + 0
		sty	array + 2 * 4 + 1
		and	#$0C
		sta	array + 2 * 4 + 2

		;
		;
		;

		lda	<screen
		eor	#1
		sta	<screen
		beq	.test0
		jmp	.test1

.test0:		lda	IFU_ADPCM_DAT
		ldx	#4			; 24-cycle delay, what a
!:		dex				; coincidence!
		bne	!-

;		ldx	IFU_ADPCM_CTL		; State D ($FFFF)
		ldy	IFU_ADPCM_FLG
		lda	IFU_IRQ_FLG
		stz	array + 3 * 4 + 0
		sty	array + 3 * 4 + 1
		and	#$0C
		sta	array + 3 * 4 + 2

		ldx.l	#$0002			; Set playback length.
		ldy.h	#$0002
		jsr	adpcm_set_len

;		ldx	IFU_ADPCM_CTL		; State E ($0002)
		ldy	IFU_ADPCM_FLG
		lda	IFU_IRQ_FLG
		stz	array + 4 * 4 + 0
		sty	array + 4 * 4 + 1
		and	#$0C
		sta	array + 4 * 4 + 2

		lda	IFU_ADPCM_DAT
		ldx	#4			; 24-cycle delay, what a
!:		dex				; coincidence!
		bne	!-

;		ldx	IFU_ADPCM_CTL		; State F ($0001)
		ldy	IFU_ADPCM_FLG
		lda	IFU_IRQ_FLG
		stz	array + 5 * 4 + 0
		sty	array + 5 * 4 + 1
		and	#$0C
		sta	array + 5 * 4 + 2

		lda	IFU_ADPCM_DAT
		ldx	#4			; 24-cycle delay, what a
!:		dex				; coincidence!
		bne	!-

;		ldx	IFU_ADPCM_CTL		; State G ($0000)
		ldy	IFU_ADPCM_FLG
		lda	IFU_IRQ_FLG
		stz	array + 6 * 4 + 0
		sty	array + 6 * 4 + 1
		and	#$0C
		sta	array + 6 * 4 + 2

		lda	IFU_ADPCM_DAT
		ldx	#4			; 24-cycle delay, what a
!:		dex				; coincidence!
		bne	!-

;		ldx	IFU_ADPCM_CTL		; State H ($FFFF)
		ldy	IFU_ADPCM_FLG
		lda	IFU_IRQ_FLG
		stz	array + 7 * 4 + 0
		sty	array + 7 * 4 + 1
		and	#$0C
		sta	array + 7 * 4 + 2

		ldx.l	#$8002			; Set playback length.
		ldy.h	#$8002
		jsr	adpcm_set_len

;		ldx	IFU_ADPCM_CTL		; State I ($8002)
		ldy	IFU_ADPCM_FLG
		lda	IFU_IRQ_FLG
		stz	array + 8 * 4 + 0
		sty	array + 8 * 4 + 1
		and	#$0C
		sta	array + 8 * 4 + 2

		lda	IFU_ADPCM_DAT
		ldx	#4			; 24-cycle delay, what a
!:		dex				; coincidence!
		bne	!-

;		ldx	IFU_ADPCM_CTL		; State J ($8001)
		ldy	IFU_ADPCM_FLG
		lda	IFU_IRQ_FLG
		stz	array + 9 * 4 + 0
		sty	array + 9 * 4 + 1
		and	#$0C
		sta	array + 9 * 4 + 2

		lda	IFU_ADPCM_DAT
		ldx	#4			; 24-cycle delay, what a
!:		dex				; coincidence!
		bne	!-

;		ldx	IFU_ADPCM_CTL		; State K ($8000)
		ldy	IFU_ADPCM_FLG
		lda	IFU_IRQ_FLG
		stz	array + 10 * 4 + 0
		sty	array + 10 * 4 + 1
		and	#$0C
		sta	array + 10 * 4 + 2

		lda	IFU_ADPCM_DAT
;		ldx	#4			; 24-cycle delay, what a
;!:		dex				; coincidence!
;		bne	!-

;		ldx	IFU_ADPCM_CTL		; State L ($7FFF)
		ldy	IFU_ADPCM_FLG
		lda	IFU_IRQ_FLG
		stz	array + 11 * 4 + 0
		sty	array + 11 * 4 + 1
		and	#$0C
		sta	array + 11 * 4 + 2

		ldx	#4			; 24-cycle delay, what a
!:		dex				; coincidence!
		bne	!-

;		ldx	IFU_ADPCM_CTL		; State M ($7FFF)
		ldy	IFU_ADPCM_FLG
		lda	IFU_IRQ_FLG
		stz	array + 12 * 4 + 0
		sty	array + 12 * 4 + 1
		and	#$0C
		sta	array + 12 * 4 + 2

		lda	IFU_ADPCM_DAT
		ldx	#4			; 24-cycle delay, what a
!:		dex				; coincidence!
		bne	!-

;		ldx	IFU_ADPCM_CTL		; State N ($7FFE)
		ldy	IFU_ADPCM_FLG
		lda	IFU_IRQ_FLG
		stz	array + 13 * 4 + 0
		sty	array + 13 * 4 + 1
		and	#$0C
		sta	array + 13 * 4 + 2

		ldx.l	#$7FFE			; Set playback length.
		ldy.h	#$7FFE
		jsr	adpcm_set_len

;		ldx	IFU_ADPCM_CTL		; State O ($7FFE)
		ldy	IFU_ADPCM_FLG
		lda	IFU_IRQ_FLG
		stz	array + 14 * 4 + 0
		sty	array + 14 * 4 + 1
		and	#$0C
		sta	array + 14 * 4 + 2

		stz	IFU_ADPCM_DAT
		ldx	#4			; 24-cycle delay, what a
!:		dex				; coincidence!
		bne	!-

;		ldx	IFU_ADPCM_CTL		; State P ($7FFF)
		ldy	IFU_ADPCM_FLG
		lda	IFU_IRQ_FLG
		stz	array + 15 * 4 + 0
		sty	array + 15 * 4 + 1
		and	#$0C
		sta	array + 15 * 4 + 2

		stz	IFU_ADPCM_DAT
;		ldx	#4			; 24-cycle delay, what a
;!:		dex				; coincidence!
;		bne	!-

;		ldx	IFU_ADPCM_CTL		; State Q ($8000)
		ldy	IFU_ADPCM_FLG
		lda	IFU_IRQ_FLG
		stz	array + 16 * 4 + 0
		sty	array + 16 * 4 + 1
		and	#$0C
		sta	array + 16 * 4 + 2

		ldx	#6			; 36-cycle delay.
!:		dex
		bne	!-

;		ldx	IFU_ADPCM_CTL		; State R ($8000)
		ldy	IFU_ADPCM_FLG
		lda	IFU_IRQ_FLG
		stz	array + 17 * 4 + 0
		sty	array + 17 * 4 + 1
		and	#$0C
		sta	array + 17 * 4 + 2

		stz	IFU_ADPCM_DAT
;		ldx	#4			; 24-cycle delay, what a
;!:		dex				; coincidence!
;		bne	!-

;		ldx	IFU_ADPCM_CTL		; State S ($8001)
		ldy	IFU_ADPCM_FLG
		lda	IFU_IRQ_FLG
		stz	array + 18 * 4 + 0
		sty	array + 18 * 4 + 1
		and	#$0C
		sta	array + 18 * 4 + 2

		ldx	#6			; 36-cycle delay.
!:		dex
		bne	!-

;		ldx	IFU_ADPCM_CTL		; State T ($8001)
		ldy	IFU_ADPCM_FLG
		lda	IFU_IRQ_FLG
		stz	array + 19 * 4 + 0
		sty	array + 19 * 4 + 1
		and	#$0C
		sta	array + 19 * 4 + 2

		jmp	.results

		;
		; Play back the 2nd sample (two loops "You're all going to die").
		;

.test1:		ldx.l	#adpcm_size * 0		; Set playback address.
		ldy.h	#adpcm_size * 0		; This clears IFU_INT_HALF,
		jsr	adpcm_set_src           ; but not IFU_INT_END.

		lda	IFU_ADPCM_CTL		; State D
		sta	array + 3 * 4 + 0
		lda	IFU_ADPCM_FLG
		sta	array + 3 * 4 + 1
		lda	IFU_IRQ_FLG
		and	#$0C
		sta	array + 3 * 4 + 2

		ldx.l	#adpcm_size * 2		; Set playback length.
		ldy.h	#adpcm_size * 2		; This clears IFU_INT_END,
		jsr	adpcm_set_len           ; but not IFU_INT_HALF.

		lda	IFU_ADPCM_CTL		; State E
		sta	array + 4 * 4 + 0
		lda	IFU_ADPCM_FLG
		sta	array + 4 * 4 + 1
		lda	IFU_IRQ_FLG
		and	#$0C
		sta	array + 4 * 4 + 2

		lda	#ADPCM_AUTO
		tsb	IFU_ADPCM_CTL

		lda	IFU_ADPCM_CTL		; State F
		sta	array + 5 * 4 + 0
		lda	IFU_ADPCM_FLG
		sta	array + 5 * 4 + 1
		lda	IFU_IRQ_FLG
		and	#$0C
		sta	array + 5 * 4 + 2

		ldy	#2			; 1ms delay.
		jsr	.delay

		lda	IFU_ADPCM_CTL		; State G
		sta	array + 6 * 4 + 0
		lda	IFU_ADPCM_FLG
		sta	array + 6 * 4 + 1
		lda	IFU_IRQ_FLG
		and	#$0C
		sta	array + 6 * 4 + 2

		lda	#ADPCM_PLAY
		tsb	IFU_ADPCM_CTL

		lda	IFU_ADPCM_CTL		; State H
		sta	array + 7 * 4 + 0
		lda	IFU_ADPCM_FLG
		sta	array + 7 * 4 + 1
		lda	IFU_IRQ_FLG
		and	#$0C
		sta	array + 7 * 4 + 2

		ldy	#2			; 1ms delay.
		jsr	.delay

		lda	IFU_ADPCM_CTL		; State I
		sta	array + 8 * 4 + 0
		lda	IFU_ADPCM_FLG
		sta	array + 8 * 4 + 1
		lda	IFU_IRQ_FLG
		and	#$0C
		sta	array + 8 * 4 + 2

		ldy	#90
		jsr	wait_nvsync

		lda	IFU_ADPCM_CTL		; State J
		sta	array + 9 * 4 + 0
		lda	IFU_ADPCM_FLG
		sta	array + 9 * 4 + 1
		lda	IFU_IRQ_FLG
		and	#$0C
		sta	array + 9 * 4 + 2

!:		lda	IFU_ADPCM_FLG
		cmp	#8
		beq	!-
		sta	array + 10 * 4 + 1

		lda	IFU_ADPCM_CTL		; State K
		sta	array + 10 * 4 + 0
		lda	IFU_ADPCM_FLG
;		sta	array + 10 * 4 + 1
		lda	IFU_IRQ_FLG
		and	#$0C
		sta	array + 10 * 4 + 2

!:		tst	#ADPCM_AD_END, IFU_ADPCM_FLG
		beq	!-

		lda	IFU_ADPCM_CTL		; State L
		sta	array + 11 * 4 + 0
		lda	IFU_ADPCM_FLG
		sta	array + 11 * 4 + 1
		lda	IFU_IRQ_FLG
		and	#$0C
		sta	array + 11 * 4 + 2

		lda	#ADPCM_PLAY + ADPCM_AUTO; Stop sample playback.
		trb	IFU_ADPCM_CTL

		lda	IFU_ADPCM_CTL		; State M
		sta	array + 12 * 4 + 0
		lda	IFU_ADPCM_FLG
		sta	array + 12 * 4 + 1
		lda	IFU_IRQ_FLG
		and	#$0C
		sta	array + 12 * 4 + 2

!:		tst	#ADPCM_AD_BSY, IFU_ADPCM_FLG
		bne	!-

		lda	IFU_ADPCM_CTL		; State N
		sta	array + 13 * 4 + 0
		lda	IFU_ADPCM_FLG
		sta	array + 13 * 4 + 1
		lda	IFU_IRQ_FLG
		and	#$0C
		sta	array + 13 * 4 + 2

		;
		; Play back the 3rd sample (one cutoff "You're all going to die").
		;

		lda	#$0E			; Set playback rate 16KHz.
		sta	IFU_ADPCM_SPD

		ldx.l	#adpcm_size * 0		; Set playback address.
		ldy.h	#adpcm_size * 0		; This clears IFU_INT_HALF,
		jsr	adpcm_set_src		; but not IFU_INT_END.

		ldx.l	#adpcm_size * 1		; Set playback length.
		ldy.h	#adpcm_size * 1		; This clears IFU_INT_END,
		jsr	adpcm_set_len		; but not IFU_INT_HALF.

		lda	#ADPCM_PLAY + ADPCM_AUTO; Start sample playback.
		sta	IFU_ADPCM_CTL

		lda	IFU_ADPCM_CTL		; State O
		sta	array + 14 * 4 + 0
		lda	IFU_ADPCM_FLG
		sta	array + 14 * 4 + 1
		lda	IFU_IRQ_FLG
		and	#$0C
		sta	array + 14 * 4 + 2

		ldy	#60
		jsr	wait_nvsync

		lda	IFU_ADPCM_CTL		; State P
		sta	array + 15 * 4 + 0
		lda	IFU_ADPCM_FLG
		sta	array + 15 * 4 + 1
		lda	IFU_IRQ_FLG
		and	#$0C
		sta	array + 15 * 4 + 2

		jsr	adpcm_reset

		lda	IFU_ADPCM_CTL		; State Q
		sta	array + 16 * 4 + 0
		lda	IFU_ADPCM_FLG
		sta	array + 16 * 4 + 1
		lda	IFU_IRQ_FLG
		and	#$0C
		sta	array + 16 * 4 + 2

;		ldy	#3			; Need somewhere between 1.0-1.5ms delay @ 2KHz
;		ldy	#1			; Need somewhere between < 500us delay @ 16KHz
;		jsr	.delay

!:		tst	#ADPCM_AD_BSY, IFU_ADPCM_FLG
		bne	!-

		lda	IFU_ADPCM_CTL		; State R
		sta	array + 17 * 4 + 0
		lda	IFU_ADPCM_FLG
		sta	array + 17 * 4 + 1
		lda	IFU_IRQ_FLG
		and	#$0C
		sta	array + 17 * 4 + 2

		;
		; Play back the 4th sample (one loop "You're all going to die").
		;

		lda	#$0E			; Set playback rate.
		sta	IFU_ADPCM_SPD

		ldx.l	#adpcm_size * 0		; Set playback address.
		ldy.h	#adpcm_size * 0		; This clears IFU_INT_HALF,
		jsr	adpcm_set_src		; but not IFU_INT_END.

		ldx.l	#adpcm_size * 1		; Set playback length.
		ldy.h	#adpcm_size * 1		; This clears IFU_INT_END,
		jsr	adpcm_set_len		; but not IFU_INT_HALF.

		lda	#ADPCM_PLAY + ADPCM_AUTO; Start sample playback.
		sta	IFU_ADPCM_CTL

		lda	IFU_ADPCM_CTL		; State S
		sta	array + 18 * 4 + 0
		lda	IFU_ADPCM_FLG
		sta	array + 18 * 4 + 1
		lda	IFU_IRQ_FLG
		and	#$0C
		sta	array + 18 * 4 + 2

!:		tst	#ADPCM_AD_END, IFU_ADPCM_FLG
		beq	!-

		lda	IFU_ADPCM_CTL		; State T
		sta	array + 19 * 4 + 0
		lda	IFU_ADPCM_FLG
		sta	array + 19 * 4 + 1
		lda	IFU_IRQ_FLG
		and	#$0C
		sta	array + 19 * 4 + 2

		lda	#ADPCM_PLAY + ADPCM_AUTO; Stop sample playback.
		trb	IFU_ADPCM_CTL

		lda	IFU_ADPCM_CTL		; State U
		sta	array + 20 * 4 + 0
		lda	IFU_ADPCM_FLG
		sta	array + 20 * 4 + 1
		lda	IFU_IRQ_FLG
		and	#$0C
		sta	array + 20 * 4 + 2

		; Display the results.

.results:	cla
.loop:		pha
		clc
		adc	#'A'
		sta	<state
		pla
		pha
		asl	a
		asl	a
		tax
		lda	<array + 0, x
		sta	<array + 0
		lda	<array + 1, x
		sta	<array + 1
		lda	<array + 2, x
		sta	<array + 2

		PRINTF	"   %1c) CTL=$%0hx FLG=$%0hx IRQ=$%0hx\n", state, array + 0, array + 1, array + 2

		pla
		inc	a
		cmp	#21
		bne	.loop

!:		lda	joytrg + 0		; Read the joypad.
		bit	#JOY_RUN
		beq	!-

		rts				; All done, phew!

		;

.delay:		clx				; The inner loop takes 3584
.wait_500us:	sxy				; cycles, which is almost a
		sxy				; perfect 500 microseconds!
		nop
		dex
		bne	.wait_500us
		dey
		bne	.wait_500us
		rts



; ***************************************************************************
; ***************************************************************************
;
; hsync_proc1 - Wait (455 + want_delay) cycles before starting a TIA.
;
; It takes 23 cycles to get to "hsync_proc1" (including IRQ response).
;

hsync_proc1:	pha				; 3 Save all registers.
		phx				; 3
		phy				; 3
		bit	VDC_SR			; 6

		; It takes 38 cycles from RCR to get to here ...

		lda	<vdc_reg		; 4
		pha				; 3

		jsr	set_next_rcr		; 75 = 7 + 68

		st0	#VDC_MAWR		; 5
		st1	#<$7000			; 5
		st2	#>$7000			; 5
		st0	#VDC_VWR		; 5
		lda	#VDC_VWR		; 2
		sta	<vdc_reg		; 4

		cli				; 2

		; It takes 148 cycles from RCR to get to here ...
		;
		; Burn 292 cycles.

		ldx	#24			; 2
.loop:		sxy				; 3
		sxy				; 3
		dex				; 2
		bne	.loop			; 4
		nop
		nop
		nop
		nop
		nop
		nop

		; Delay, then TIA, then lots and lots of delay!
		;
		; We want get here (455 - 7) cycles from the RCR.

do_xfer:	lda.l	<xfer_size		; ADPCM size.
		sta.l	<_ax
		lda.h	<xfer_size
		sta.h	<_ax
		lda.l	#adpcm_size * 0		; ADPCM addr.
		sta.l	<_bx
		lda.h	#adpcm_size * 0
		sta.h	<_bx
		lda	#<adpcm_data		; Source addr.
		sta	<_si + 0
		lda	#>adpcm_data
		sta	<_si + 1
		ldy	#^adpcm_data
		call	adpcm_write		; Write the ADPCM data.

		jsr	ram_nop			; 7 + 2 x #nops.

		pla				; Restore VDC_AR in case we
		sta	<vdc_reg		; changed it.
		sta	VDC_AR

		ply				; Restore all registers.
		plx
		pla
		rti				; Return from interrupt.



; ***************************************************************************
; ***************************************************************************
;
; hsync_proc2 - This RCR happens WAIT_LINES after the TIA started.
;
; It takes 23 cycles to get to "hsync_proc2" (including IRQ response).
;

hsync_proc2:	pha				; Save all registers.
		phx
		phy
		bit	VDC_SR

		lda	<which_rcr		; Convert which_rcr into
		dec	a			; an index, 0 or 2.
		dec	a
		and	#$FE
		tay

		jsr	set_next_rcr		; Prepare next RCR.

		tsx				; Calculate the number of
		sec				; "nop" instructions that
		lda.l	$2105, x		; were executed.
		sbc.l	#ram_nop + 7
		sta.l	num_cycles, y
		lda.h	$2105, x
		sbc.h	#ram_nop + 7
		sta.h	num_cycles, y

		sxy
		asl.l	num_cycles, x		; #instructions -> #cycles.
		rol.h	num_cycles, x

		sec				; Subtract the #cycles total
		lda.l	#455 * WAIT_LINES	; from the time between the
		sbc.l	num_cycles, x		; two RCR interrupts to get
		sta.l	num_cycles, x		; the time taken for the TIA.
		lda.h	#455 * WAIT_LINES
		sbc.h	num_cycles, x
		sta.h	num_cycles, x

		lda	<vdc_reg		; Restore VDC_AR in case we
		sta	VDC_AR			; changed it.

		ply				; Restore all registers.
		plx
		pla
		rti				; Return from interrupt.



; ***************************************************************************
; ***************************************************************************
;
; vsync_proc - When VBLANK occurs, reset the RCR chain for the next frame.
;

vsync_proc:	stz	<which_rcr		; Prepare first RCR.
		jsr	set_next_rcr

		jsr	xfer_palettes		; Transfer queue to VCE now.

!:		rts



; ***************************************************************************
; ***************************************************************************
;
; set_next_rcr - Set up the screen's next RCR and irq1_hook.
;

set_next_rcr:	lda	<which_rcr		; 4
		inc	<which_rcr		; 6
		asl	a			; 2
		tax				; 2

		st0	#VDC_RCR		; 5 Set up the next RCR.
		lda.l	tbl_rcr, x		; 5
		sta	VDC_DL			; 6
		lda.h	tbl_rcr, x		; 5
		sta	VDC_DH			; 6

		lda.l	tbl_hook, x		; 5 Set up the IRQ1 hook.
		sta.l	irq1_hook		; 5
		lda.h	tbl_hook, x		; 5
		sta.h	irq1_hook		; 5
		rts				; 7 = 68 cycles

tbl_rcr:	dw	64 + ((12 * 8) + 8)
		dw	64 + ((12 * 8) + 8 + 1 + WAIT_LINES)
		dw	0

tbl_hook:	dw	hsync_proc1
		dw	hsync_proc2
		dw	irq1_handler



; ***************************************************************************
; ***************************************************************************
;
; do_autorepeat - auto-repeat UDLR button presses, useful for menu screens.
;

SLOW_RPT_UDLR	=	30
FAST_RPT_UDLR	=	8 ; 10

do_autorepeat	.proc

		; Do auto-repeat processing on the d-pad.

		ldx	#MAX_PADS - 1

.loop:		lda	joynow, x		; Auto-Repeat UDLR buttons
		ldy	#SLOW_RPT_UDLR		; while they are held.
		and	#$F0
		beq	.set_delay
		dec	joyrpt, x
		bne	.no_repeat
		ora	joytrg, x
		sta	joytrg, x
		ldy	#FAST_RPT_UDLR
.set_delay:	tya
		sta	joyrpt, x

.no_repeat:	dex				; Check the next pad from the
		bpl	.loop			; multitap.

		leave				; All done, phew!

		.endp

		.bss
joyrpt:		ds	MAX_PADS
		.code



; ***************************************************************************
; ***************************************************************************
;
; screen_pal - Palette data
;
; Note: DEFPAL palette data is in RGB format, 4-bits per value.
; Note: Packed palette data is in GRB format, 3-bits per value.
;
;  $4 = dark blue background
;  $5 = light blue shadow
;  $6 = yellow font
;

		.data

		align	2

none		=	$000

screen_pal:	defpal	$000,none,none,none,$002,$114,$772,none
		defpal	none,none,none,none,none,none,$111,$777

		defpal	$000,none,none,none,$002,$333,$777,none
		defpal	none,none,none,none,none,none,$101,$727

		defpal	$000,none,none,none,$030,$114,$772,none
		defpal	none,none,none,none,none,none,none,none

		defpal	$000,none,none,none,$210,$114,$772,none
		defpal	none,none,none,none,none,none,none,none



; ***************************************************************************
; ***************************************************************************
;
; It's the font data, nothing exciting to see here!
;

my_font:	incbin	"font8x8-ascii-bold-short.dat"



; ***************************************************************************
; ***************************************************************************
;
; It's the font data, nothing exciting to see here!
;

adpcm_data:	incbin	"red_queen-16k.vox"
adpcm_size	=	linear(*) - linear(adpcm_data)
