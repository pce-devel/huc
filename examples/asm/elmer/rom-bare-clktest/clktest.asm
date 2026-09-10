; ***************************************************************************
; ***************************************************************************
;
; clktest.asm
;
; ADPCM playback clock tester using the basic HuCARD startup library code.
;
; Copyright John Brandwood 2026.
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
; This checks how many CPU cycles it takes to execute a TIA instruction to
; transfer data to the VDC while the screen is active.
;
; Because of the VDC's round-robin method of VDC memory access, and its need
; to read background data during the line's display, the CPU is subject to a
; lot of delays that aren't taken into account by the documented cycle times
; of (17 + 6n).
;
; The actual clock cycles taken depends upon the screen resolution, the time
; that the transfer is started, and the number of sprites on the line.
;
; OUCH!
;
; ***************************************************************************
; ***************************************************************************
;
; The PC Engine's memory map is set to ...
;
;   MPR0 = bank $FF : PCE hardware
;   MPR1 = bank $F8 : PCE RAM with Stack & ZP
;   MPR2 = bank $00 : HuCARD ROM
;   MPR3 = bank $01 : HuCARD ROM
;   MPR4 = bank $02 : HuCARD ROM
;   MPR5 = bank $03 : HuCARD ROM
;   MPR6 = bank $04 : HuCARD ROM
;   MPR7 = bank $00 : HuCARD ROM with the startup code and IRQ vectors.
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

		.list
		.mlist

		include "bare-startup.asm"	; No "CORE(not TM)" library!

		include	"common.asm"		; Common helpers.
		include	"vdc.asm"		; Useful VDC routines.
		include	"font.asm"		; Useful font routines.
		include "joypad.asm"		; Joypad routines.
		include	"tty.asm"		; Useful TTY print routines.



; ***************************************************************************
; ***************************************************************************
;
; Constants and Variables.
;

ADPCM_SAMPLES	=	250000			; Number of samples.
ADPCM_BYTES	=	(ADPCM_SAMPLES / 2)	; Number of bytes.
ADPCM_BLOCKS	=	ADPCM_BYTES >> 13	; Number of 8192 byte chunks.
ADPCM_EXTRA	=	ADPCM_BYTES - (ADPCM_BLOCKS << 13)

		.zp

loop_count	ds	2
addr_value	ds	2
cycle_count	ds	4



; ***************************************************************************
; ***************************************************************************
;
; It's a bank of NOPs for timing, plus a 16-bit loop counter.
;

		.data

		align	$2000

delay_loop:	ds	$2000, $EA

next_delay:


		db	$1A			; INC A
		db	$D0,$01			; BNE jump_delay
		db	$E8			; INX
jump_delay:	db	$4C,$00,$40		; JMP $4000



; ***************************************************************************
; ***************************************************************************
;
; bare_main - This is executed after startup library initialization.
;

		.code

		; Reset the IRQ hooks when switching resolution.

bare_main:	jsr	bare_clr_hooks

		; Turn the display off and initialize the screen mode.

		call	init_256x224		; Initialize VDC & VRAM.

		; Upload the font to VRAM.

		stz	<_di + 0		; Destination VRAM address.
		lda	#>(CHR_0x10 * 16)
		sta	<_di + 1

		lda	#$FF			; Put font in colors 4-7,
		sta	<_al			; so bitplane 2 = $FF and
		stz	<_ah			; bitplane 3 = $00.

		lda	#16 + 96		; 16 graphics + 96 ASCII.
		sta	<_bl

		lda	#<my_font		; Address of font data.
		sta	<_bp + 0
		lda	#>my_font
		sta	<_bp + 1
		ldy	#^my_font

		call	dropfnt8x8_vdc		; Upload font to VRAM.

		; Upload the palette data to the VCE.

		stz	<_al			; Start at palette 0 (BG).
		lda	#4			; Copy 4 palettes of 16 colors.
		sta	<_ah
		lda	#<screen_pal		; Set the ptr to the palette
		sta	<_bp + 0		; data.
		lda	#>screen_pal
		sta	<_bp + 1
		ldy	#^screen_pal
		call	load_palettes		; Add to the palette queue.

		lda	#16			; Start at palette 16 (SPR).
		sta	<_al
		lda	#4			; Copy 4 palettes of 16 colors.
		sta	<_ah
		lda	#<screen_pal		; Set the ptr to the palette
		sta	<_bp + 0		; data.
		lda	#>screen_pal
		sta	<_bp + 1
		ldy	#^screen_pal
		call	load_palettes		; Add to the palette queue.

		call	xfer_palettes		; Transfer queue to VCE now.

		; Turn on the BG & SPR layers, then wait for a soft-reset.

		call	set_dspon		; Enable background.

		; Display the test name.

		PRINTF	"\e<\eX1\eY1\eP0**PC ENGINE ADPCM SPEED TEST**\eP1\eX1\eY3\eP1This takes roughly 20 seconds.\eP0"

;		PRINTF	"\e<\eX1\eY1\eP0**PC ENGINE ADPCM SPEED TEST**\eP1\eX1\eY3\x1E\x1F\eP0:Del\eP1 \x1C\x1D\eP0:Siz\eP1 SEL\eP0:Mode\eP1 RUN\eP0:Spr\eP0"

		; Bank in 32KB of NOP instructions followed by a JMP $4000.

		lda	#^delay_loop
		tam2
		tam3
		tam4
		tam5
		lda	#^next_delay
		tam6

		stz	<cycle_count

		; Reset the CD subsystem.

		jsr	cdr_reset

		; Initialize the ADPCM hardware.

		jsr	adpcm_reset		; Reset the MSM5205.
		jsr	wait_vsync		; Give it time to settle.
		jsr	wait_vsync

		lda	#$0E			; Set 16KHz playback,
		sta	IFU_ADPCM_SPD		; which is the maximum.
		jsr	wait_vsync		; Give it time to settle,
		jsr	wait_vsync		; which it does not need.

		clx				; Lo byte.
		cly				; Hi byte.
		jsr	adpcm_set_src		; Clears IFU_INT_HALF!

		clx				; Lo byte.
		cly				; Hi byte.
		jsr	adpcm_set_dst

		ldx.l	#$2000			; Lo byte.
		ldy.h	#$2000                  ; Hi byte.
		jsr	adpcm_set_len		; Clears IFU_INT_END!

		ldx	#ADPCM_BLOCKS		; The ADPCM length is a 17-bit
.fill_bank:	phx				; counter!
		ldy.l	#$2000
		ldx.h	#$2000
		jsr	clr_adpcm
		plx
		cpx	#ADPCM_BLOCKS
		bne	!+

		phx
		ldx.l	#$2000			; Lo byte.
		ldy.h	#$2000                  ; Hi byte.
		jsr	adpcm_set_len		; Clears IFU_INT_END!
		plx

!:		dex
		bne	.fill_bank

		ldy.l	#ADPCM_EXTRA
		ldx.h	#ADPCM_EXTRA
	.if	(ADPCM_EXTRA & 255)
		inx
	.endif
		jsr	clr_adpcm

		; Disable everything but IRQ2.

		jsr	wait_vsync		; Sync to VBLANK for no reason.
		lda	#$06			; Disable IRQ1 and TIMER interrupts.
		sta	IRQ_MSK

		; Enable the IRQ test processing.

!:		lda.l	#adpcm_done
		sta.l	irq2_hook
		lda.h	#adpcm_done
		sta.h	irq2_hook
		lda	#$01
		tsb	<irq_vec

		; Trigger ADPCM playback.

		lda	#IFU_INT_END		; Enable interrupt when done.
		tsb	IFU_IRQ_MSK

		cla				; Loop counter lo-byte.
		clx				; Loop counter hi-byte.

		ldy	#ADPCM_PLAY + ADPCM_AUTO; Start sample playback.
		sty	IFU_ADPCM_CTL

		jmp	$4000			; Execute the delay loop.



; ***************************************************************************
; ***************************************************************************
;
; adpcm_done - When IFU_INT_END occurs, stop ADPCM and calc cycles taken.
;

adpcm_done:	sta.l	<loop_count
		stx.h	<loop_count

		sta	<cycle_count + 2	; loop_count * 65536.
		stx	<cycle_count + 3

		asl	a
		sax
		rol	a
		sax
		sta	<cycle_count + 0	; loop_count * 2.
		stx	<cycle_count + 1
		asl	a
		sax
		rol	a
		sax
		asl	a
		sax
		rol	a
		sax
.out_of_range:	bcs	.out_of_range		; if loop_count >= 8192

		adc	<cycle_count + 0	; loop_count * 8.
		sta	<cycle_count + 0
		sax
		adc	<cycle_count + 1
		sta	<cycle_count + 1
		bcc	.skip_inc1
		inc	<cycle_count + 2
		bne	.skip_inc1
		inc	<cycle_count + 3

.skip_inc1:	jsr	adpcm_stop		; Stop ADPCM, disable IRQ.

		plx				; Throw away P.
		plx				; Get addr-lo.
		pla				; Get addr-hi.

		stx.l	<addr_value
		sta.h	<addr_value

		sec				; Calc instruction offset
		sbc.h	#$4000			; into the delay_loop.

		sax				; Multiply instructions
		asl	a			; executed by 2.
		sax
		rol	a
		bcc	.easy_calc

		; Handle timing when IRQ was while incrementing the loop counter.

.ugly_calc:	cpx	#0			; If INC A is next ...
		beq	.add_block
		cpx	#8			; If JMP $4000 is next ... 
		bne	.test_high
		ldx	#6			; Then add 6 cycles.
		bra	.easy_calc
.test_high:	ldy.l	<loop_count		; IRQ just after INC A!
		bne	.correct_x
		inc	<cycle_count + 3	; Add 256 * 65536 cycles.
.correct_x:	cpx	#2			; If BNE is next ...
		beq	.easy_calc
		ldx	#4			; So INX is next ...

		; Simply add the number of instruction cycles to the total.

.easy_calc:	clc
		sax				; Add to the cycle_count.
		adc	<cycle_count + 0
		sta	<cycle_count + 0
		sax
		adc	<cycle_count + 1
		sta	<cycle_count + 1
		bcc	.skip_inc2
.add_block:	inc	<cycle_count + 2
		bne	.skip_inc2
		inc	<cycle_count + 3

		;

.skip_inc2:	PRINTF	"\e<\eX1\eY3\eP1Played 250000 16KHz samples in\n %ld CPU cycles.\n\n\eP0 Press any button to retest.", cycle_count

		jsr	read_joypads		; Sync with current state.

		bit	VDC_SR			; Purge any delayed IRQ1.
		stz	IRQ_MSK			; Restore IRQ1 and TIMER.
		cli				; Enable interrupts.

		; Loop around waiting for user input.

.next_frame:	jsr	wait_vsync

.got_vblank:	lda	joytrg + 0		; Read the joypad.
		bit	#JOY_U + JOY_D + JOY_L + JOY_R + JOY_RUN + JOY_SEL
		beq	.next_frame

		jmp	bare_sw_reset



; ***************************************************************************
; ***************************************************************************
;
; cdr_reset - Reset CD drive and stop the disc spinning (BIOS CD_RESET).
;
; Returns: nothing
;

cdr_reset:	stz	IFU_IRQ_MSK		; Disable IFU interrupts.
		stz	IFU_AUDIO_FADE		; Disable CD fade.

		lda	#$02			; Assert reset signal.
		tsb	IFU_HW_RESET

		ldy	#1			; Wait 500us.
		jsr	scsi_delay

		trb	IFU_HW_RESET		; Release reset signal.

		lda	#119			; Wait 100us.
.wait_100us:	dec	a
		bne	.wait_100us

		tst	#IFU_INT_SUBC, IFU_IRQ_FLG
		beq	.finished

		bit	IFU_SUBCODE		; Flush subcode interrupt.

.finished:	rts



; ***************************************************************************
; ***************************************************************************
;
; scsi_delay - Delay 500us * Y.
;

scsi_delay:	clx				; The inner loop takes 3584
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

		stz	IFU_ADPCM_SPD		; BIOS sets 2KHz playback!
		rts



; ***************************************************************************
; ***************************************************************************
;
; adpcm_stop -	(BIOS AD_STOP).
;
; N.B. Use cdr_ad_stop() on a CD/SCD game which also handles ADPCM streaming.
;

adpcm_stop:	lda	#IFU_INT_HALF + IFU_INT_END
		trb	IFU_IRQ_MSK

		lda	#ADPCM_PLAY + ADPCM_AUTO
		trb	IFU_ADPCM_CTL
		rts



; ***************************************************************************
; ***************************************************************************
;
; adpcm_stat - (BIOS AD_STAT).
;
; Returns: X = $01 if ADPCM_AD_END, $04 if IFU_INT_HALF, or $00.
; Returns: A,Z-flag = NZ if busy (ADPCM_PLAY or ADPCM_AD_BSY).
;

adpcm_stat:	lda	IFU_ADPCM_FLG		; $01 if playback stopped.
		and	#ADPCM_AD_END
		bne	!+
		lda	IFU_IRQ_FLG		; $04 if length < $8000.
		and	#IFU_INT_HALF

!:		tax

		lda	IFU_ADPCM_CTL		; $20 if playing.
		and	#ADPCM_PLAY
		bne	!+
		lda	IFU_ADPCM_FLG		; $08 if busy.
		and	#ADPCM_AD_BSY

!:		rts



; ***************************************************************************
; ***************************************************************************
;
; Set ADPCM RAM read pointer (source address ADPCM playback).
;
; If the LENGTH is currently zero, then this clears IFU_INT_HALF.
;

adpcm_set_src:	stx	IFU_ADPCM_LSB
		sty	IFU_ADPCM_MSB

		lda	#ADPCM_SET_RD
		tsb	IFU_ADPCM_CTL

		lda	IFU_ADPCM_DAT		; This clears IFU_INT_HALF!

		ldx	#4			; A 24-cycle delay, what a
!:		dex				; coincidence!
		bne	!-

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
; This clears IFU_INT_END, but not IFU_INT_HALF!
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
; clr_adpcm - Write XY of +/- 1 values to ADPCM RAM.
;
; 30 cycles-per-byte vs 71 cycles-per-byte for BIOS AD_WRITE.
;

		; 29 cycle *minimum* with no ADPCM_WR_BSY loops.
		; 30 cycle *safer* with no ADPCM_WR_BSY loops.

clr_adpcm:	lda	#$80				; Write $80 to ADPCM RAM.
.wait:		tst	#ADPCM_WR_BSY, IFU_ADPCM_FLG	; 8
		bne	.wait				; 2
		sta	IFU_ADPCM_DAT			; 5
		pha					; 3
		pla					; 4
		nop					; 2
		dey					; 2
		bne	.wait				; 4
		dex					; 2
		bne	.wait				; 4
		rts



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
