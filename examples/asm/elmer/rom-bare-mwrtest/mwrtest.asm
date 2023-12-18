; ***************************************************************************
; ***************************************************************************
;
; mwrtest.asm
;
; VDC timing test HuCARD example using the basic HuCARD startup library code.
;
; Copyright John Brandwood 2023.
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
; This checks how many CPU cycles (on average) it takes to execute different
; code sequences that read/write to VRAM, in both burst mode and also if the
; the background is enabled, with different MWR access-slot settings, and if
; during VBLANK or not.
;
; Because of the VDC's round-robin method of VDC memory access, and its need
; to read background data during the line's display, the CPU is subject to a
; lot of delays that aren't taken into account by the documented cycle times
; in the HuC6280 manual.
;
; Sprites are disabled during the tests to avoid any access slot contention
; between the CPU and the loading of SPR data during hblank.
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

test_buff	=	$F8:2800		; 6KB area to run test code.

WAIT_LINES	=	16			; #lines to wait for test.

		.zp

screen_rez	ds	1			; 0=256, 1=352, 2=512.
want_delay	ds	2			; Minimum  0, Maximum 500.
delay_count	ds	2			; Minimum  8, Maximum 510.

which_rcr	ds	1

which_test	ds	1

test_flag	ds	1			;
test_addr	ds	2			; Addr of test sequence.
test_size	ds	1			; Addr of test sequence.

test_rti	ds	2			; Addr of test result.

burst_count:	ds	2			; #times that the test code was
burst_extra:	ds	1			; repeated and #extra bytes.

shown_count:	ds	2			; #times that the test code was
shown_extra:	ds	1			; repeated and #extra bytes.

		.bss

delay_code	ds	256			; Configurable delay code.



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

		; Identify this program.

		PRINTF	"\e<\eX1\eY1\eP0**PC ENGINE VDC MWR VRAM R/W**\eP1\eX3\eY3\x1C\x1D\eP0:Change Test\eP1 \x1E\x1F\eP0:VDC Mode\eP2\n\n"

		ldy	<screen_rez		; Identify the resolution.
		ldx	screen_slo, y
		lda	screen_shi, y
		ldy	#bank(*)
		jsr	tty_printf

		PRINTF	"\n\n Testing code sequence:\n\eP1"

		; Set up the test sequence code in ram.

		ldy	<which_test		; Set up the next test.
		lda	table_alo, y
		sta.l	test_addr
		lda	table_ahi, y
		sta.h	test_addr
		lda	table_len, y
		sta.l	test_size

		ldx	table_slo, y
		lda	table_shi, y
		ldy	#bank(*)
		jsr	tty_printf		; Announce the test.

		jsr	set_delay		; Setup the desired delay.

		jsr	fill_test		; Fill code buffer with test.

		; Enable the IRQ test processing.

!:		lda.l	#vsync_proc
		sta.l	vsync_hook
		lda.h	#vsync_proc
		sta.h	vsync_hook
		lda	#$10
		tsb	<irq_vec

		; Run the test code in the VDC's burst mode.

		jsr	wait_vsync		; Ensure BURST enabled.
		jsr	wait_vsync

		smb0	<test_flag		; Start test next vblank.
!:		bbs0	<test_flag, !-		; Wait for test completion.

		lda.l	<test_rti		; Calculate # of code repeats
		sta.l	<_ax			; during the test.
		lda.h	<test_rti
		sta.h	<_ax
		lda	<test_size
		sta	<_cl
		jsr	div16_7u

		sty	<burst_extra		; Save # extra bytes.
		lda.l	<_ax			; Save # of repeats.
		sta.l	<burst_count
		lda.h	<_ax
		sta.h	<burst_count

		; Turn on the BG layer, then wait for a soft-reset.

		call	set_bgon		; Enable background.

		; Run the test code in the VDC's normal mode.

		jsr	wait_vsync		; Ensure BURST disabled.
		jsr	wait_vsync

		smb0	<test_flag		; Start test next vblank.
!:		bbs0	<test_flag, !-		; Wait for test completion.

		lda.l	<test_rti		; Calculate # of code repeats
		sta.l	<_ax			; during the test.
		lda.h	<test_rti
		sta.h	<_ax
		lda	<test_size
		sta	<_cl
		jsr	div16_7u

		sty	<shown_extra		; Save # extra bytes.
		lda.l	<_ax			; Save # of repeats.
		sta.l	<shown_count
		lda.h	<_ax
		sta.h	<shown_count

		; Display the results.

		PRINTF	"\n\n\eP2 Test time is %d cycles.\n\n", test_time

		PRINTF	" In burst mode: Code ran %4d\n times, plus %hd extra bytes.\n\n", burst_count, burst_extra

		PRINTF	" Background on: Code ran %4d\n times, plus %hd extra bytes.\n\n", shown_count, shown_extra

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
		cmp	#5
		bcc	!+
		cla
!:		sta	<screen_rez
		jmp	bare_main

.screen_prev:	lda	<screen_rez		; Select previous test.
		bne	!+
		lda	#5
!:		dec	a
		sta	<screen_rez
		jmp	bare_main

; Constant for printing.

test_time:	dw	455 * WAIT_LINES

; Strings for displaying resolution.

screen_slo:	dwl	screen_mode1
		dwl	screen_mode2
		dwl	screen_mode3
		dwl	screen_mode4
		dwl	screen_mode5

screen_shi:	dwh	screen_mode1
		dwh	screen_mode2
		dwh	screen_mode3
		dwh	screen_mode4
		dwh	screen_mode5

screen_mode1:	STRING	" VDC resolution 240*192, MWR=$0\eP0\n"
screen_mode2:	STRING	" VDC resolution 256*224, MWR=$0\eP0\n"
screen_mode3:	STRING	" VDC resolution 336*224, MWR=$A\eP0\n"
screen_mode4:	STRING	" VDC resolution 352*224, MWR=$0\eP0\n"
screen_mode5:	STRING	" VDC resolution 512*224, MWR=$A\eP0\n"



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

tbl_rcr:	dw	64 + (192 + 3)
		dw	64 + (192 + 4)
		dw	64 + (192 + 4 + (1 + WAIT_LINES))
		dw	0			; Disable RCR

tbl_hook:	dw	hsync_proc1
		dw	hsync_proc2
		dw	hsync_proc3
		dw	irq1_handler



; ***************************************************************************
; ***************************************************************************
;
; hsync_proc1 - Ensure 0..2 cycle variance for the next line's RCR interrupt.
;
; It takes 23 cycles to get to "hsync_proc1" (including IRQ response).
;

hsync_proc1:	pha				; 3 Save all registers.
		phx				; 3
		phy				; 3
		bit	VDC_SR			; 6

		jsr	set_next_rcr		; 75 = 7 + 68

		lda	<vdc_reg		; 4 Preserve vdc_reg!
		pha				; 3

		st0	#VDC_MARR		; 5 Prepare VDC for reading
		st1	#<$7000			; 5 and writing during test.
		st2	#>$7000			; 5
		st0	#VDC_MAWR		; 5
		st1	#<$7000			; 5
		st2	#>$7000			; 5

		cli				; 2 Allow next RCR.

		; Make sure that the next line's RCR interrupt is stable.

.wait:		ds	256, $EA		; 256 NOP instructions.

		jmp	.wait			; Shouldn't get here!



; ***************************************************************************
; ***************************************************************************
;
; hsync_proc2 - Wait (455 + want_delay) cycles before starting a TIA.
;
; It takes 23 cycles to get to "hsync_proc2" (including IRQ response).
;

hsync_proc2:	pla				; 4 Throw away the flags and
		pla				; 4 the return address to
		pla				; 4 hsync_proc1

		bit	VDC_SR			; 6 Acknowledge IRQ.

		; It takes 41 cycles from RCR to get to here ...

		jsr	set_next_rcr		; 75 = 7 + 68

		st0	#VDC_VWR		; 5
		lda	#VDC_VWR		; 2
		sta	<vdc_reg		; 4
		
		cli				; 2 Allow next RCR.

		; It takes 129 cycles from RCR to get to here ...
		;
		; Burn 312 cycles.

		ldx	#26			; 2
.loop:		sxy				; 3
		sxy				; 3
		dex				; 2
		bne	.loop			; 4

		; It takes 441 cycles from RCR to get to here ...
		;
		; Burn 2 cycles.

		nop

		; It takes 443 cycles from RCR to get to here ...
		;
		; Delay, then execute the test_buff, which is required to
		; never finish!
		;
		; We want get here (455 - 12) cycles from the RCR.

		jmp	delay_code		; 4 + 8 (min) delay.

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
; hsync_proc3 - This RCR happens WAIT_LINES after the TIA started.
;
; It takes 23 cycles to get to "hsync_proc3" (including IRQ response).
;

hsync_proc3:	bit	VDC_SR			; Acknowledge interrupt.

		jsr	set_next_rcr		; Prepare next RCR.

		pla				; Remove saved flags.

		sec				; Use the RTI address to calc
		pla				; the number of instructions
		sbc.l	#test_buff		; that were executed.
		sta.l	test_rti ; , y
		pla
		sbc.h	#test_buff
		sta.h	test_rti ; , y

	.if	0

		sxy
		asl.l	tia_delay, x		; #instructions -> #cycles.
		rol.h	tia_delay, x

		clc				; Add the delay_count that
		lda.l	tia_delay, x		; we used.
		adc.l	delay_count
		sta.l	tia_delay, x
		lda.h	tia_delay, x
		adc.h	delay_count
		sta.h	tia_delay, x

		sec				; Subtract 8, i.e.
		lda.l	tia_delay, x		; (want_delay - delay_count)
		sbc.l	#8
		sta.l	tia_delay, x		
		lda.h	tia_delay, x
		sbc.h	#8
		sta.h	tia_delay, x

		sec				; Subtract the #cycles total
		lda.l	#455 * WAIT_LINES	; from the time between the
		sbc.l	tia_delay, x		; two RCR interrupts to get
		sta.l	tia_delay, x		; the time taken for the TIA.
		lda.h	#455 * WAIT_LINES
		sbc.h	tia_delay, x
		sta.h	tia_delay, x

	.endif

		rmb0	<test_flag		; Signal test is complete.

		pla				; Restore VDC_AR to the value
		sta	<vdc_reg		; it was before the test.
		sta	VDC_AR

		ply				; Restore all registers.
		plx
		pla
		rti				; Return from interrupt.



; ***************************************************************************
; ***************************************************************************
;
; init_240x192 - A reduced 240x192 screen to run the tests during VBLANK.
;

init_240x192	.proc

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

		lda	#<.mode_240x192		; Disable BKG & SPR layers but
		sta.l	<_bp			; enable RCR & VBLANK IRQ.
		lda	#>.mode_240x192
		sta.h	<_bp

	.if	SUPPORT_SGX
		call	sgx_detect		; Are we really on an SGX?
		beq	!+
		ldy	#^.mode_240x192		; Set SGX 1st, with no VBL.
		call	set_mode_sgx
	.endif
!:		ldy	#^.mode_240x192		; Set VDC 2nd, VBL allowed.
		call	set_mode_vdc

		call	wait_vsync		; Wait for the next VBLANK.

		leave				; All done, phew!

		; A reduced 240x192 screen to run the tests during VBLANK.

.mode_240x192:	db	$80			; VCE Control Register.
		db	VCE_CR_5MHz		; Video Clock

		db	VDC_MWR			; Memory-access Width Register
		dw	VDC_MWR_64x32 + VDC_MWR_1CYCLE
		db	VDC_HSR			; Horizontal Sync Register
		dw	VDC_HSR_240
		db	VDC_HDR			; Horizontal Display Register
		dw	VDC_HDR_240
		db	VDC_VPR			; Vertical Sync Register
		dw	VDC_VPR_192
		db	VDC_VDW			; Vertical Display Register
		dw	VDC_VDW_192
		db	VDC_VCR			; Vertical Display END position Register
		dw	VDC_VCR_192
		db	VDC_DCR			; DMA Control Register
		dw	$0010			;   Enable automatic VRAM->SATB
		db	VDC_DVSSR		; VRAM->SATB address $0400
		dw	$0800
		db	VDC_BXR			; Background X-Scroll Register
		dw	$0008
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
; init_336x224 - R-Type USA's 336x224 screen with slow 2-cycle MWR.
;

init_336x224	.proc

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

		lda	#<.mode_336x224		; Disable BKG & SPR layers but
		sta.l	<_bp			; enable RCR & VBLANK IRQ.
		lda	#>.mode_336x224
		sta.h	<_bp

	.if	SUPPORT_SGX
		call	sgx_detect		; Are we really on an SGX?
		beq	!+
		ldy	#^.mode_336x224		; Set SGX 1st, with no VBL.
		call	set_mode_sgx
	.endif
!:		ldy	#^.mode_336x224		; Set VDC 2nd, VBL allowed.
		call	set_mode_vdc

		call	wait_vsync		; Wait for the next VBLANK.

		leave				; All done, phew!

		; R-Type USA's 336x224 screen with slow 2-cycle MWR.

.mode_336x224:	db	$80			; VCE Control Register.
		db	VCE_CR_7MHz + 4		;   Video Clock + Artifact Reduction

		db	VDC_MWR			; Memory-access Width Register
		dw	VDC_MWR_64x32 + VDC_MWR_2CYCLE
		db	VDC_HSR			; Horizontal Sync Register
		dw	VDC_HSR_336
		db	VDC_HDR			; Horizontal Display Register
		dw	VDC_HDR_336
		db	VDC_VPR			; Vertical Sync Register
		dw	VDC_VPR_224
		db	VDC_VDW			; Vertical Display Register
		dw	VDC_VDW_224
		db	VDC_VCR			; Vertical Display END position Register
		dw	VDC_VCR_224
		db	VDC_DCR			; DMA Control Register
		dw	$0010			;   Enable automatic VRAM->SATB
		db	VDC_DVSSR		; VRAM->SATB address $0800
		dw	$0800
		db	VDC_BXR			; Background X-Scroll Register
		dw	$0008
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

.vector:	jmp	init_240x192		; Initialize VDC & VRAM.
		jmp	init_256x224		; Initialize VDC & VRAM.
		jmp	init_336x224		; Initialize VDC & VRAM.
		jmp	init_352x224		; Initialize VDC & VRAM.
		jmp	init_512x224		; Initialize VDC & VRAM.



; ***************************************************************************
; ***************************************************************************
;
; set_delay - Set up an 'n'-cycle delay routine (min 8, max 510).
;
; n x NOP = 2n
; 1 x JMP = 4
;
; n x NOP = 2n
; 1 x BIT = 5
; 1 x JMP = 4
;

set_delay:	lda.h	<delay_count		; Minumum delay of 8 cycles.
		bne	.calc
		lda.l	<delay_count
		cmp	#8
		bcs	.calc
		lda	#8
		sta.l	<delay_count
		stz.h	<delay_count

.calc:		sec
		lda.l	<delay_count
		sbc	#4
		tax
		lda.h	<delay_count
		sbc	#0
		lsr	a
		txa
		ror	a
		tax

		tay				; Fill the delay_code buffer
		lda	#$EA			; with "NOP" instructions.
.fill:		dey
		sta	delay_code, y
		bne	.fill
		bcc	.done

		dex	        		; A "BIT" replaces two "NOP".
		dex

		lda	#$2C			; "BIT abs" opcode.
		sta	delay_code, x
		inx
		lda.l	#$2100
		sta	delay_code, x
		inx
		lda.h	#$2100
		sta	delay_code, x
		inx

.done:		lda	#$4C			; "JMP abs" opcode.
		sta	delay_code, x
		inx
		lda.l	#test_buff
		sta	delay_code, x
		inx
		lda.h	#test_buff
		sta	delay_code, x
		rts



; ***************************************************************************
; ***************************************************************************
;
; fill_test - Copy a repeating sequence of the test code to the exec buffer.
;



fill_test:	ldy.l	#test_buff		; Set start of the buffer
		lda.h	#test_buff		; to write the repeating
		stz.l	<_di			; sequence to.
		sta.h	<_di

.fill_code:	lda.l	<test_addr		; Set pointer to test code
		sta.l	<_si			; source.
		lda.h	<test_addr
		sta.h	<_si
		clx				; Set test code length.

.fill_byte:	sxy
		lda	[_si], y
		sxy
		sta	[_di], y
		iny
		bne	.next_byte
		inc	<_di + 1
		bbs6	<_di + 1, .fill_done	; Moved from $3F00 to $4000?
.next_byte:	inx
		cpx	<test_size		; #bytes of code sequence to
		bne	.fill_byte		; test.
		bra	.fill_code

.fill_done:	rts



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
		dwl	test_name7
		dwl	test_name8
		dwl	test_name9
		dwl	test_name10
		dwl	test_name11

table_shi:	dwh	test_name1
		dwh	test_name2
		dwh	test_name3
		dwh	test_name4
		dwh	test_name5
		dwh	test_name6
		dwh	test_name7
		dwh	test_name8
		dwh	test_name9
		dwh	test_name10
		dwh	test_name11

table_alo:	dwl	test_code1
		dwl	test_code2
		dwl	test_code3
		dwl	test_code4
		dwl	test_code5
		dwl	test_code6
		dwl	test_code7
		dwl	test_code8
		dwl	test_code9
		dwl	test_code10
		dwl	test_code11

table_ahi:	dwh	test_code1
		dwh	test_code2
		dwh	test_code3
		dwh	test_code4
		dwh	test_code5
		dwh	test_code6
		dwh	test_code7
		dwh	test_code8
		dwh	test_code9
		dwh	test_code10
		dwh	test_code11

table_len:	db	test_size1
		db	test_size2
		db	test_size3
		db	test_size4
		db	test_size5
		db	test_size6
		db	test_size7
		db	test_size8
		db	test_size9
		db	test_size10
		db	test_size11

		; 1st test - Baseline, should take 5 cycles.

test_name1:	STRING	"  st1 #$00\n"
test_code1:	st1	#0
test_size1	=	* - test_code1

		; 2nd test - How fast can you clear VRAM?

test_name2:	STRING	"  st2 #$00\n"
test_code2:	st2	#0
test_size2	=	* - test_code2

		; 3rd test - How fast can you clear VRAM?

test_name3:	STRING	"  st1 #$00\n  st2 #$00\n"
test_code3:	st1	#0
		st2	#0
test_size3	=	* - test_code3

		; 4th test - Baseline, should take 6 cycles.

test_name4:	STRING	"  stz $0002\n"
test_code4:	stz	VDC_DL
test_size4	=	* - test_code4

		; 5th test - How fast can you clear VRAM?

test_name5:	STRING	"  stz $0003\n"
test_code5:	stz	VDC_DH
test_size5	=	* - test_code5

		; 6th test - How fast can you clear VRAM?

test_name6:	STRING	"  stz $0002\n  stz $0003\n"
test_code6:	stz	VDC_DL
		stz	VDC_DH
test_size6	=	* - test_code6

		; 7th test - Baseline, should take 9 cycles.

test_name7:	STRING	"  trb $0002\n"
test_code7:	trb	VDC_DL
test_size7	=	* - test_code7

		; 8th test - How fast can you modify VRAM?

test_name8:	STRING	"  trb $0003\n"
test_code8:	trb	VDC_DH
test_size8	=	* - test_code8

		; 9th test - How fast can you modify VRAM?

test_name9:	STRING	"  trb $0002\n  trb $0003\n"
test_code9:	trb	VDC_DL
		trb	VDC_DH
test_size9	=	* - test_code9

		; 10th test - Can you change a register while the VDC is busy?

test_name10:	STRING	"  st0 #$00\n  st1 #$00\n  st2 #$70\n  st0 #$02\n"
test_code10:	st0	#VDC_MAWR
		st1	#$00
		st2	#$70
		st0	#VDC_VWR
test_size10	=	* - test_code10

		; 11th test - Can you change a register while the VDC is busy?

test_name11:	STRING	"  st0 #$01\n  st1 #$00\n  st2 #$70\n  st0 #$02\n"
test_code11:	st0	#VDC_MARR
		st1	#$00
		st2	#$70
		st0	#VDC_VRR
test_size11	=	* - test_code11



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
