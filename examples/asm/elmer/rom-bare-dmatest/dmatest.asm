; ***************************************************************************
; ***************************************************************************
;
; dmatest.asm
;
; VDC DMA timing test HuCARD using the basic HuCARD startup library code.
;
; Copyright John Brandwood 2025
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
; This checks how many CPU cycles the VDC's SATB and VRAM DMA take, and when
; they begin, by counting the number of cycles between an RCR that is set on
; line 220, and subsequent VBLANK, SATB-DMA-END and VRAM-DMA-END interrupts.
;
; ***************************************************************************
; ***************************************************************************
;
; The PC Engine's memory map is set to ...
;
;   MPR0 = bank $FF : PCE hardware
;   MPR1 = bank $F8 : PCE RAM with Stack & ZP
;   MPR2 = bank $01 : HuCARD ROM
;   MPR3 = bank $01 : HuCARD ROM
;   MPR4 = bank $01 : HuCARD ROM
;   MPR5 = bank $02 : HuCARD ROM
;   MPR6 = bank $nn : HuCARD ROM
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

RESERVE_BANKS	=	2			; Two banks of timing NOPs.

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

		.zp

screen_rez	ds	1			; 0=352, 1=512, 2=256.

which_rcr	ds	1

which_test	ds	1

test_flag	ds	1			;

rcr2vbl:	ds	2
rcr2satb:	ds	2
rcr2vram1:	ds	2
rcr2vram2:	ds	2
rcr2vram3:	ds	2
rcr2both:	ds	2



; ***************************************************************************
; ***************************************************************************
;
; bare_main - This is executed after startup library initialization.
;

		.code

		; Reset the IRQ hooks when switching resolution.

bare_main:	jsr	bare_clr_hooks

		; Turn the display off and initialize the screen mode.
		;
		; N.B. BG & SPR are disabled, so the VDC is in "burst mode".

		jsr	init_vdc_rez		; Initialize VDC & VRAM.

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
		lda	#3			; Copy 3 palettes of 16 colors.
		sta	<_ah
		lda	#<screen_pal		; Set the ptr to the palette
		sta	<_bp + 0		; data.
		lda	#>screen_pal
		sta	<_bp + 1
		ldy	#^screen_pal
		call	load_palettes		; Add to the palette queue.

		call	xfer_palettes		; Transfer queue to VCE now.

		; Map in banks of NOP instructions from $4000-$BFFF.
		;
		; This provides 65,000+ cycles of timing.

		lda	#BASE_BANK + 1
		tam2
		tam3
		tam4
		inc	a
		tam5

		; Identify this program.

		PRINTF	"\e<\eX1\eY1\eP0***PC ENGINE VDC DMA TIMING***\eP1\eX3\eY3\x1C\x1D\eP0:Change Test\eP1 \x1E\x1F\eP0:VDC Mode\eP2\n\n"

		ldy	<screen_rez		; Identify the resolution.
		ldx	screen_slo, y
		lda	screen_shi, y
		ldy	#bank(*)
		jsr	tty_printf

		ldy	<which_test		; Set up the next test.
		lda	table_alo, y
		sta	$2100
		lda	table_ahi, y
		sta	$2101
		phy
		jsr	run_code
		ply

		ldy	<which_test		; Announce the test.
		ldx	table_slo, y
		lda	table_shi, y
		ldy	#bank(*)
		jsr	tty_printf

		;

		st0	#VDC_DCR		; Disable automatic VRAM-SATB.
		st1	#$00

		stz	<which_rcr		; Start at the 1st test.

		; Turn on the BG layer, then wait for a soft-reset.

		call	set_bgon		; Enable background.

		jsr	wait_vsync		; Wait for the test changes to
		jsr	wait_vsync		; stabilize.

		jsr	set_next_rcr		; Trigger the test.

		jmp	$4000			; Wait for the test to finish.

run_code:	jmp	[$2100]			; Execute test setup.

		; The test has finished!

test_finished:	ldx	#$FF			; Reset the stack to remove
		txs				; junk left from the test.

		; Display the results.

		PRINTF	"  Cycles from line 220 RCR to:\n\n"
		PRINTF	" VBLANK                 = %5d\n", rcr2vbl
		PRINTF	" SATB-DMA end           = %5d\n", rcr2satb
		PRINTF	" 128 word VRAM-DMA end  = %5d\n", rcr2vram1
		PRINTF	" 256 word VRAM-DMA end  = %5d\n", rcr2vram2
		PRINTF	" 512 word VRAM-DMA end  = %5d\n", rcr2vram3
		PRINTF	" SATB and 256 word VRAM = %5d\n", rcr2both

		; Loop around waiting for user input.

.next_frame:	jsr	wait_vsync

.got_vblank:	lda	joytrg + 0		; Read the joypad.
		bit	#JOY_R
		bne	.which_next
		bit	#JOY_L
		bne	.which_prev
		bit	#JOY_U
		bne	.screen_next
		bit	#JOY_D
		bne	.screen_prev

		jmp	.next_frame		; Wait for user to reboot.

; Change which test to run.

.which_next:	lda	<which_test		; Select next test.
		inc	a
		cmp	#table_ahi - table_alo
		bcc	!+
		cla
!:		sta	<which_test
		jmp	bare_main

.which_prev:	lda	<which_test		; Select previous test.
		bne	!+
		lda	#table_ahi - table_alo
!:		dec	a
		sta	<which_test
		jmp	bare_main

; Change resolution.

.screen_next:	lda	<screen_rez		; Select next test.
		inc	a
		cmp	#4
		bcc	!+
		cla
!:		sta	<screen_rez
		jmp	bare_main

.screen_prev:	lda	<screen_rez		; Select previous test.
		bne	!+
		lda	#4
!:		dec	a
		sta	<screen_rez
		jmp	bare_main

; Strings for displaying resolution.

screen_slo:	dwl	screen_mode1
		dwl	screen_mode2
		dwl	screen_mode3
		dwl	screen_mode4

screen_shi:	dwh	screen_mode1
		dwh	screen_mode2
		dwh	screen_mode3
		dwh	screen_mode4

screen_mode1:	STRING	"   VDC resolution 256, MWR=$0\n"
screen_mode2:	STRING	"   VDC resolution 352, MWR=$0\n"
screen_mode3:	STRING	"   VDC resolution 512, MWR=$A\n"
screen_mode4:	STRING	"   Check \"TV Sports Football\"\n"



; ***************************************************************************
; ***************************************************************************
;
; Define the different test code sequences that we want to run.
;

		; Tables pointing to each test code sequence.

table_slo:	dwl	test_name1
		dwl	test_name2
		dwl	test_name3
		dwl	test_name4
		dwl	test_name5
		dwl	test_name6

table_shi:	dwh	test_name1
		dwh	test_name2
		dwh	test_name3
		dwh	test_name4
		dwh	test_name5
		dwh	test_name6

table_alo:	dwl	test_code1
		dwl	test_code2
		dwl	test_code3
		dwl	test_code4
		dwl	test_code5
		dwl	test_code6

table_ahi:	dwh	test_code1
		dwh	test_code2
		dwh	test_code3
		dwh	test_code4
		dwh	test_code5
		dwh	test_code6

		; 1st test

test_name1:	STRING	"   222 line display, blur on\eP0\n\n"
test_code1:	st0	#VDC_VDW
		st1	#222 - 1

		lda	vce_cr			; No SGX shadow for this!
		and	#3
		ora	#XRES_SOFT
		sta	vce_cr
		sta	VCE_CR			; Set the VCE clock speed.
		rts

		; 2nd test

test_name2:	STRING	"   222 line display, blur off\eP0\n\n"
test_code2:	st0	#VDC_VDW
		st1	#222 - 1

		lda	vce_cr			; No SGX shadow for this!
		and	#3
;		ora	#XRES_SHARP
		sta	vce_cr
		sta	VCE_CR			; Set the VCE clock speed.
		rts

		; 3rd test

test_name3:	STRING	"   224 line display, blur on\eP0\n\n"
test_code3:	st0	#VDC_VDW
		st1	#224 - 1

		lda	vce_cr			; No SGX shadow for this!
		and	#3
		ora	#XRES_SOFT
		sta	vce_cr
		sta	VCE_CR			; Set the VCE clock speed.
		rts

		; 4th test

test_name4:	STRING	"   224 line display, blur off\eP0\n\n"
test_code4:	st0	#VDC_VDW
		st1	#224 - 1

		lda	vce_cr			; No SGX shadow for this!
		and	#3
;		ora	#XRES_SHARP
		sta	vce_cr
		sta	VCE_CR			; Set the VCE clock speed.
		rts

		; 5th test

test_name5:	STRING	"   240 line display, blur on\eP0\n\n"
test_code5:	st0	#VDC_VDW
		st1	#240 - 1

		lda	vce_cr			; No SGX shadow for this!
		and	#3
		ora	#XRES_SOFT
		sta	vce_cr
		sta	VCE_CR			; Set the VCE clock speed.
		rts

		; 6th test

test_name6:	STRING	"   242 line display, blur on\eP0\n\n"
test_code6:	st0	#VDC_VDW
		st1	#242 - 1

		lda	vce_cr			; No SGX shadow for this!
		and	#3
		ora	#XRES_SOFT
		sta	vce_cr
		sta	VCE_CR			; Set the VCE clock speed.
		rts



; ***************************************************************************
; ***************************************************************************
;
; vsync_proc - Initialize the RCR chain for the next test when VBLANK occurs.
;

vsync_proc:	bbr0	<test_flag, !+		; Do we want to run a test?

		stz	<which_rcr		; Prepare first RCR.
		jsr	set_next_rcr		

!:		jsr	xfer_palettes		; Transfer queue to VCE now.

		rts



; ***************************************************************************
; ***************************************************************************
;
; set_next_rcr - Set up the screen's next RCR and irq1_hook during an IRQ.
;
; N.B. This is designed to be run inside an IRQ handler, so it does NOT set
; VDC_AR back to vdc_reg!
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

tbl_rcr:	dw	64 + (220)		; RCR line 220
		dw	64 + (220)              ; VBL
		dw	64 + (220)              ; RCR line 220
		dw	64 + (220)              ; VBL
		dw	64 + (220)              ; RCR line 220
		dw	64 + (220)              ; VBL
		dw	64 + (220)              ; RCR line 220
		dw	64 + (220)              ; VBL
		dw	64 + (220)              ; RCR line 220
		dw	64 + (220)              ; VBL
		dw	64 + (220)              ; RCR line 220
		dw	64 + (220)              ; VBL

		dw	0			; Disable RCR

tbl_hook:	dw	init_rcr2vbl		; RCR line 220
		dw	calc_rcr2vbl		; VBL
		dw	init_rcr2satb		; RCR line 220
		dw	calc_rcr2satb		; VBL
		dw	init_rcr2vram1		; RCR line 220
		dw	calc_rcr2vram1		; VBL
		dw	init_rcr2vram2		; RCR line 220
		dw	calc_rcr2vram2		; VBL
		dw	init_rcr2vram3		; RCR line 220
		dw	calc_rcr2vram3		; VBL
		dw	init_rcr2both		; RCR line 220
		dw	calc_rcr2both		; VBL

		dw	irq1_handler		; Test finished.



; ***************************************************************************
; ***************************************************************************
;
; RCR interrupt
;
; N.B. All of these RCR-triggered "init_" functions have identical timing so
;      that the results are easy to calculate.
;
; It takes 23 cycles to get to this routine (including IRQ response).
;

;		;;;				; 8 (cycles for the INT)
;		bbs1	<irq_vec, .hook		; 8
;		jmp	[irq1_hook]		; 7

init_rcr2vbl:	bit	VDC_SR			; 6 Acknowledge the VDC's IRQ.
		jsr	set_next_rcr		; 75 = 7 + 68

		st0	#VDC_CR			; 5 Enable VBL, Enable RCR.
		st1	#$CC			; 5

		st0	#VDC_DCR		; 5 Disable SATB-DMA and VRAM-DMA complete IRQ.
		st1	#$00			; 5

		st0	#VDC_MAWR		; 5 Set MAWR.
		st1	#$00			; 5
		st2	#$10			; 5

		sax				; 3 Get even number results.

		cli				; 2
		jmp	$4000			; 4 Start executing NOPs.

INIT_CYCLES	=	148			; = 145 cycles from RCR.



; ***************************************************************************
; ***************************************************************************
;
; VBL interrupt
;
; It takes 23 cycles to get to this routine (including IRQ response).
;

;		;;;				; 8 (cycles for the INT)
;		bbs1	<irq_vec, .hook		; 8
;		jmp	[irq1_hook]		; 7

calc_rcr2vbl:	bit	VDC_SR			; Acknowledge the VDC's IRQ.
		jsr	set_next_rcr

		pla				; Throw away the flags.
		sec
		pla				; Get lo-byte of address.
		sbc.l	#$4000
		tax
		pla				; Get hi-byte of address.
		sbc.h	#$4000
		sax				; 2 cycles per NOP.
		asl	a
		sax
		rol	a
		sax
		adc	#INIT_CYCLES		; Add INIT_CYCLES.
		bcc	!+
		inx
!:		sta.l	<rcr2vbl
		stx.h	<rcr2vbl

		st0	#VDC_CR			; Disable VBL, Enable RCR.
		st1	#$C4

		st0	#VDC_VWR		; Write $55AA to VRAM $1000.
		st1	#$AA
		st2	#$55

		cli
		jmp	$4000			; Start executing NOPs.



; ***************************************************************************
; ***************************************************************************
;
; RCR interrupt
;
; N.B. All of these RCR-triggered "init_" functions have identical timing so
;      that the results are easy to calculate.
;
; It takes 23 cycles to get to this routine (including IRQ response).
;

;		;;;				; 8 (cycles for the INT)
;		bbs1	<irq_vec, .hook		; 8
;		jmp	[irq1_hook]		; 7

init_rcr2satb:	bit	VDC_SR			; 6 Acknowledge the VDC's IRQ.
		jsr	set_next_rcr		; 75 = 7 + 68

		st0	#VDC_CR			; 5 Disable VBL, Enable RCR.
		st1	#$C4			; 5

		st0	#VDC_DCR		; 5 Enable SATB-DMA complete IRQ.
		st1	#$01			; 5

		st0	#VDC_DVSSR		; 5 Trigger SATB-DMA.
		st1	#$00			; 5
		st2	#$08			; 5

		sax				; 3 Get even number results.

		cli				; 2
		jmp	$4000			; 4 Start executing NOPs.



; ***************************************************************************
; ***************************************************************************
;
; SATB-DMA interrupt
;
; It takes 23 cycles to get to this routine (including IRQ response).
;

;		;;;				; 8 (cycles for the INT)
;		bbs1	<irq_vec, .hook		; 8
;		jmp	[irq1_hook]		; 7

calc_rcr2satb:	bit	VDC_SR			; Acknowledge the VDC's IRQ.
		jsr	set_next_rcr

		pla				; Throw away the flags.
		sec
		pla				; Get lo-byte of address.
		sbc.l	#$4000
		tax
		pla				; Get hi-byte of address.
		sbc.h	#$4000
		sax				; 2 cycles per NOP.
		asl	a
		sax
		rol	a
		sax
		adc	#INIT_CYCLES		; Add INIT_CYCLES.
		bcc	!+
		inx
!:		sta.l	<rcr2satb
		stx.h	<rcr2satb

		st0	#VDC_CR			; Disable VBL, Enable RCR.
		st1	#$C4			;

		st0	#VDC_DCR		; Enable VRAM-VRAM complete IRQ.
		st1	#$02			;

		st0	#VDC_SOUR		; Set VRAM-VRAM source.
		st1	#$00
		st2	#$10

		st0	#VDC_DESR		; Set VRAM-VRAM destination.
		st1	#$01
		st2	#$10

		cli				;
		jmp	$4000			; Start executing NOPs.



; ***************************************************************************
; ***************************************************************************
;
; RCR interrupt
;
; N.B. All of these RCR-triggered "init_" functions have identical timing so
;      that the results are easy to calculate.
;
; It takes 23 cycles to get to this routine (including IRQ response).
;

;		;;;				; 8 (cycles for the INT)
;		bbs1	<irq_vec, .hook		; 8
;		jmp	[irq1_hook]		; 7

init_rcr2vram1:	bit	VDC_SR			; 6 Acknowledge the VDC's IRQ.
		jsr	set_next_rcr		; 75 = 7 + 68

		st0	#VDC_CR			; 5 Disable VBL, Enable RCR.
		st1	#$C4			; 5

		st0	#VDC_DCR		; 5 Enable VRAM-DMA complete IRQ.
		st1	#$02			; 5

		st0	#VDC_LENR		; 5 Trigger VRAM-DMA.
		st1	#$7F			; 5
		st2	#$00			; 5

		sax				; 3 Get even number results.

		cli				; 2
		jmp	$4000			; 4 Start executing NOPs.



; ***************************************************************************
; ***************************************************************************
;
; VRAM-DMA interrupt
;
; It takes 23 cycles to get to this routine (including IRQ response).
;

;		;;;				; 8 (cycles for the INT)
;		bbs1	<irq_vec, .hook		; 8
;		jmp	[irq1_hook]		; 7

calc_rcr2vram1:	bit	VDC_SR			; Acknowledge the VDC's IRQ.
		jsr	set_next_rcr

		pla				; Throw away the flags.
		sec
		pla				; Get lo-byte of address.
		sbc.l	#$4000
		tax
		pla				; Get hi-byte of address.
		sbc.h	#$4000
		sax				; 2 cycles per NOP.
		asl	a
		sax
		rol	a
		sax
		adc	#INIT_CYCLES		; Add INIT_CYCLES.
		bcc	!+
		inx
!:		sta.l	<rcr2vram1
		stx.h	<rcr2vram1

		st0	#VDC_CR			; 5 Disable VBL, Enable RCR.
		st1	#$C4			; 5

		st0	#VDC_DCR		; 5 Enable VRAM-DMA complete IRQ.
		st1	#$02			; 5

;		Don't reset these to see if they have changed!
;
;		st0	#VDC_SOUR		; 5 Set VRAM-VRAM source.
;		st1	#$00
;		st2	#$10

;		st0	#VDC_DESR		; 5 Set VRAM-VRAM destination.
;		st1	#$01
;		st2	#$10

		cli				; 2
		jmp	$4000			; 4 Start executing NOPs.



; ***************************************************************************
; ***************************************************************************
;
; RCR interrupt
;
; N.B. All of these RCR-triggered "init_" functions have identical timing so
;      that the results are easy to calculate.
;
; It takes 23 cycles to get to this routine (including IRQ response).
;

;		;;;				; 8 (cycles for the INT)
;		bbs1	<irq_vec, .hook		; 8
;		jmp	[irq1_hook]		; 7

init_rcr2vram2:	bit	VDC_SR			; 6 Acknowledge the VDC's IRQ.
		jsr	set_next_rcr		; 75 = 7 + 68

		st0	#VDC_CR			; 5 Disable VBL, Enable RCR.
		st1	#$C4			; 5

		st0	#VDC_DCR		; 5 Enable VRAM-DMA complete IRQ.
		st1	#$02			; 5

		st0	#VDC_LENR		; 5 Trigger VRAM-DMA.
		st1	#$FF			; 5
		st2	#$00			; 5

		sax				; 3 Get even number results.

		cli				; 2
		jmp	$4000			; 4 Start executing NOPs.



; ***************************************************************************
; ***************************************************************************
;
; VRAM-DMA interrupt
;
; It takes 23 cycles to get to this routine (including IRQ response).
;

;		;;;				; 8 (cycles for the INT)
;		bbs1	<irq_vec, .hook		; 8
;		jmp	[irq1_hook]		; 7

calc_rcr2vram2:	bit	VDC_SR			; Acknowledge the VDC's IRQ.
		jsr	set_next_rcr

		pla				; Throw away the flags.
		sec
		pla				; Get lo-byte of address.
		sbc.l	#$4000
		tax
		pla				; Get hi-byte of address.
		sbc.h	#$4000
		sax				; 2 cycles per NOP.
		asl	a
		sax
		rol	a
		sax
		adc	#INIT_CYCLES		; Add INIT_CYCLES.
		bcc	!+
		inx
!:		sta.l	<rcr2vram2
		stx.h	<rcr2vram2

		st0	#VDC_CR			; 5 Disable VBL, Enable RCR.
		st1	#$C4			; 5

		st0	#VDC_DCR		; 5 Enable VRAM-DMA complete IRQ.
		st1	#$02			; 5 Enable auto VRAM-SATB.

;		Don't reset these to see if they have changed!
;
;		st0	#VDC_SOUR		; 5 Set VRAM-VRAM source.
;		st1	#$00
;		st2	#$10

;		st0	#VDC_DESR		; 5 Set VRAM-VRAM destination.
;		st1	#$01
;		st2	#$10

		cli				; 2
		jmp	$4000			; 4 Start executing NOPs.



; ***************************************************************************
; ***************************************************************************
;
; RCR interrupt
;
; N.B. All of these RCR-triggered "init_" functions have identical timing so
;      that the results are easy to calculate.
;
; It takes 23 cycles to get to this routine (including IRQ response).
;

;		;;;				; 8 (cycles for the INT)
;		bbs1	<irq_vec, .hook		; 8
;		jmp	[irq1_hook]		; 7

init_rcr2vram3:	bit	VDC_SR			; 6 Acknowledge the VDC's IRQ.
		jsr	set_next_rcr		; 75 = 7 + 68

		st0	#VDC_CR			; 5 Disable VBL, Enable RCR.
		st1	#$C4			; 5

		st0	#VDC_DCR		; 5 Enable VRAM-DMA complete IRQ.
		st1	#$02			; 5

		st0	#VDC_LENR		; 5 Trigger VRAM-DMA.
		st1	#$FF			; 5
		st2	#$01			; 5

		sax				; 3 Get even number results.

		cli				; 2
		jmp	$4000			; 4 Start executing NOPs.



; ***************************************************************************
; ***************************************************************************
;
; VRAM-DMA interrupt
;
; It takes 23 cycles to get to this routine (including IRQ response).
;

;		;;;				; 8 (cycles for the INT)
;		bbs1	<irq_vec, .hook		; 8
;		jmp	[irq1_hook]		; 7

calc_rcr2vram3:	bit	VDC_SR			; Acknowledge the VDC's IRQ.
		jsr	set_next_rcr

		pla				; Throw away the flags.
		sec
		pla				; Get lo-byte of address.
		sbc.l	#$4000
		tax
		pla				; Get hi-byte of address.
		sbc.h	#$4000
		sax				; 2 cycles per NOP.
		asl	a
		sax
		rol	a
		sax
		adc	#INIT_CYCLES		; Add INIT_CYCLES.
		bcc	!+
		inx
!:		sta.l	<rcr2vram3
		stx.h	<rcr2vram3

		st0	#VDC_CR			; 5 Disable VBL, Enable RCR.
		st1	#$C4			; 5

		st0	#VDC_DCR		; 5 Enable VRAM-DMA complete IRQ.
		st1	#$12			; 5 Enable auto VRAM-SATB.

;		Don't reset these to see if they have changed!
;
;		st0	#VDC_SOUR		; 5 Set VRAM-VRAM source.
;		st1	#$00
;		st2	#$10

;		st0	#VDC_DESR		; 5 Set VRAM-VRAM destination.
;		st1	#$01
;		st2	#$10

		cli				; 2
		jmp	$4000			; 4 Start executing NOPs.



; ***************************************************************************
; ***************************************************************************
;
; RCR interrupt
;
; N.B. All of these RCR-triggered "init_" functions have identical timing so
;      that the results are easy to calculate.
;
; It takes 23 cycles to get to this routine (including IRQ response).
;

;		;;;				; 8 (cycles for the INT)
;		bbs1	<irq_vec, .hook		; 8
;		jmp	[irq1_hook]		; 7

init_rcr2both:	bit	VDC_SR			; 6 Acknowledge the VDC's IRQ.
		jsr	set_next_rcr		; 75 = 7 + 68

		st0	#VDC_CR			; 5 Disable VBL, Enable RCR.
		st1	#$C4			; 5

		st0	#VDC_DCR		; 5 Enable VRAM-DMA complete IRQ.
		st1	#$12			; 5 Enable auto VRAM-SATB.

		st0	#VDC_LENR		; 5 Trigger VRAM-DMA.
		st1	#$FF			; 5
		st2	#$00			; 5

		sax				; 3 Get even number results.

		cli				; 2
		jmp	$4000			; 4 Start executing NOPs.



; ***************************************************************************
; ***************************************************************************
;
; VRAM-DMA interrupt
;
; It takes 23 cycles to get to this routine (including IRQ response).
;

;		;;;				; 8 (cycles for the INT)
;		bbs1	<irq_vec, .hook		; 8
;		jmp	[irq1_hook]		; 7

calc_rcr2both:	bit	VDC_SR			; 6 Acknowledge the VDC's IRQ.
		jsr	set_next_rcr		; 75 = 7 + 68

		pla				; Throw away the flags.
		sec
		pla				; Get lo-byte of address.
		sbc.l	#$4000
		tax
		pla				; Get hi-byte of address.
		sbc.h	#$4000
		sax				; 2 cycles per NOP.
		asl	a
		sax
		rol	a
		sax
		adc	#INIT_CYCLES		; Add INIT_CYCLES.
		bcc	!+
		inx
!:		sta.l	<rcr2both
		stx.h	<rcr2both

		st0	#VDC_CR			; 5 Enable VBL, Enable RCR.
		st1	#$CC			; 5

		st0	#VDC_DCR		; 5 Disable SATB-DMA and VRAM-DMA complete IRQ.
		st1	#$10			; 5 Enable auto VRAM-SATB.

		cli
		jmp	test_finished



; ***************************************************************************
; ***************************************************************************
;
; init_352x242 - TV Sports Football's too-many-lines screen mode.
;

init_352x242	.proc

.BAT_SIZE	=	64 * 32
.SAT_ADDR	=	.BAT_SIZE		; SAT takes 16 tiles of VRAM.
.CHR_ZERO	=	.BAT_SIZE / 16		; 1st tile # after the BAT.
.CHR_0x10	=	.CHR_ZERO + 16		; 1st tile # after the SAT.
.CHR_0x20	=	.CHR_ZERO + 32		; ASCII ' ' CHR tile #.

		call	clear_vce		; Clear all palettes.

		lda.l	#.CHR_0x20		; Tile # of ' ' CHR.
		sta.l	<_ax
		lda.h	#.CHR_0x20
		sta.h	<_ax

		lda	#>.BAT_SIZE		; Size of BAT in words.
		sta	<_bl

		call	clear_vram_vdc		; Clear VRAM.
	.if	SUPPORT_SGX
		call	clear_vram_sgx
	.endif

		lda	#<.mode_352x242		; Disable BKG & SPR layers but
		sta.l	<_bp			; enable RCR & VBLANK IRQ.
		lda	#>.mode_352x242
		sta.h	<_bp

	.if	SUPPORT_SGX
		call	sgx_detect		; Are we really on an SGX?
		beq	!+
		ldy	#^.mode_352x242		; Set SGX 1st, with no VBL.
		call	set_mode_sgx
	.endif
!:		ldy	#^.mode_352x242		; Set VDC 2nd, VBL allowed.
		call	set_mode_vdc

		call	wait_vsync		; Wait for the next VBLANK.

		leave				; All done, phew!

		; TV Sports Football's too-many-lines screen mode.

.mode_352x242:	db	$80			; VCE Control Register.
		db	VCE_CR_7MHz + 4		;   Video Clock + Artifact Reduction

		db	VDC_MWR			; Memory-access Width Register
		dw	VDC_MWR_64x32
		db	VDC_HSR			; Horizontal Sync Register
		dw	$0302
		db	VDC_HDR			; Horizontal Display Register
		dw	$032B
		db	VDC_VPR			; Vertical Sync Register
		dw	$1400
		db	VDC_VDW			; Vertical Display Register
		dw	$00F1
		db	VDC_VCR			; Vertical Display END position Register
		dw	$0008
		db	VDC_DCR			; DMA Control Register
		dw	$0010			;   Enable automatic VRAM->SATB
		db	VDC_DVSSR		; VRAM->SATB address $0800
		dw	$0800
		db	VDC_BXR			; Background X-Scroll Register
		dw	$0000
		db	VDC_BYR			; Background Y-Scroll Register
		dw	$0000
		db	VDC_RCR			; Raster Counter Register
		dw	$0000			;   Never occurs!
		db	VDC_CR			; Control Register
		dw	$000C			;   Enable VSYNC & RCR IRQ
		db	0

		.endp



; ***************************************************************************
; ***************************************************************************
;
; init_vdc_rez - Switch between screen resolutions.
;

		.code

init_vdc_rez:	lda	<screen_rez		; Multiply by 3.
		asl	a
		adc	<screen_rez
		tax
		jmp	[.vector + 1, x]	; Skip JMP instruction.

		; JMP instructions in order to fixup procedure addresses!

.vector:	jmp	init_256x224		; Initialize VDC & VRAM.
		jmp	init_352x224		; Initialize VDC & VRAM.
		jmp	init_512x224		; Initialize VDC & VRAM.
		jmp	init_352x242		; Initialize VDC & VRAM.



; ***************************************************************************
; ***************************************************************************
;
; Create a bank of NOP instructions for timing.
;

		.bank	BASE_BANK + 1
		.ds	8192, $EA
		.bank	BASE_BANK + 2
		.ds	8189, $EA
		jmp	$4000



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

		defpal	$000,none,none,none,$002,$222,$777,none
		defpal	none,none,none,none,none,none,$101,$727

		defpal	$000,none,none,none,$002,$114,$277,none
		defpal	none,none,none,none,none,none,none,none



; ***************************************************************************
; ***************************************************************************
;
; It's the font data, nothing exciting to see here!
;

my_font:	incbin	"font8x8-ascii-bold-short.dat"
