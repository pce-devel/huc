; ***************************************************************************
; ***************************************************************************
;
; vdctest.asm
;
; VDC tester HuCARD example of using the basic HuCARD startup library code.
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
; An RCR interrupt is set on line 127 of the display, marked on the screen.
;
; Use the joypad arrows to increase/decrease the delay between getting the
; RCR interrupt, and when the CPU writes to the BYR, BXR and CR registers.
;
; Those registers change the Y-Scroll, X-Scroll, and Sprite-Display for the
; bottom part of the screen.
;
; Altering the delay lets you see exactly when the VDC makes shadow-copies
; of the register values to use for displaying the next line on the screen.
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

REDLINE         = 127				; Line for RCR IRQ.
GREENLINE       = 24+1				; Line to set BYR to.

		.zp

screen_rez	ds	1			; 0=256, 1=352, 2=512.
delay_count	ds	1			; Minimum 8, Maximum 200.

		.bss

byr_cycle	ds	1			; Calculated for display.
bxr_cycle	ds	1			; Calculated for display.
crl_cycle	ds	1			; Calculated for display.
vbl_delay	ds	2			; Calculated for display.

delay_code	ds	128+4			; Code for adjustable delay.



; ***************************************************************************
; ***************************************************************************
;
; bare_main - This is executed after startup library initialization.
;

		.code

bare_main:	; Turn the display off and initialize the screen mode.

		jsr	init_vdc_rez		; Initialize VDC & VRAM.

		; Upload the background graphics.

		php				; Disable IRQ during xfer.
		sei

		st0	#VDC_VPR		; Vertical Sync Register
		st1	#<VDC_VPR_192
		st2	#>VDC_VPR_192
		st0	#VDC_VDW		; Vertical Display Register
		st1	#<VDC_VDW_192
		st2	#>VDC_VDW_192
		st0	#VDC_VCR		; Vertical Display END position Register
		st1	#<VDC_VCR_192
		st2	#>VDC_VCR_192

		stz.l	<_di
		stz.h	<_di
		jsr	vdc_di_to_mawr
		tia	screen_bat, VDC_DL, $1000
		tia	sprite_sat, VDC_DL, 8*3

		stz.l	<_di
		lda	#$10
		sta.h	<_di
		jsr	vdc_di_to_mawr
		tia	screen_chr, VDC_DL, linear(screen_end) - linear(screen_chr)

		stz.l	<_di
		lda	#$20
		sta.h	<_di
		jsr	vdc_di_to_mawr
		tia	screen_spr, VDC_DL, $0800

		bit	VDC_SR			; Purge any overdue VBL.
		plp

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
		sta	<_si + 0
		lda	#>my_font
		sta	<_si + 1
		ldy	#^my_font

		call	dropfnt8x8_vdc		; Upload font to VRAM.

		; Upload the palette data to the VCE.

		stz	<_al			; Start at palette 0 (BG).
		lda	#3			; Copy 3 palettes of 16 colors.
		sta	<_ah
		lda	#<screen_pal		; Set the ptr to the palette
		sta	<_si + 0		; data.
		lda	#>screen_pal
		sta	<_si + 1
		ldy	#^screen_pal
		call	load_palettes		; Add to the palette queue.

		lda	#16			; Start at palette 16 (SPR).
		sta	<_al
		lda	#1			; Copy 1 palettes of 16 colors.
		sta	<_ah
		lda	#<screen_pal + 32	; Set the ptr to the palette
		sta	<_si + 0		; data.
		lda	#>screen_pal + 32
		sta	<_si + 1
		ldy	#^screen_pal
		call	load_palettes		; Add to the palette queue.

		call	xfer_palettes		; Transfer queue to VCE now.

		; Enable the IRQ test processing.

		lda.l	#vsync_proc
		sta.l	vsync_hook
		lda.h	#vsync_proc
		sta.h	vsync_hook
		lda	#$10
		tsb	<irq_vec

		; Turn on the BG & SPR layers, then wait for a soft-reset.

		call	set_dspon		; Enable background.

		; Loop around updating the display each frame.

		PRINTF	"\e<\eX1\eY5\eP2****PC ENGINE VDC IRQ TEST****\eP0\eX1\eY7\x1E\x1F\eP2:Change Delay\eP0 SEL\eP2:Resolution\eP0"

		lda	#1			; Delay first VBLANK cycles
		sta	irq_cnt			; update.

.next_frame:	lda	#41 - 8 ; Exclude 8-cycle IRQ response!
		clc
		adc	<delay_count
		sta	byr_cycle

		lda	#51 - 8 ; Exclude 8-cycle IRQ response!
		clc
		adc	<delay_count
		sta	bxr_cycle

		lda	#61 - 8 ; Exclude 8-cycle IRQ response!
		clc
		adc	<delay_count
		sta	crl_cycle

		PRINTF	"\eXL2\eY8\nRCR to BYR write delay: %3hu.\nRCR to BXR write delay: %3hu.\nRCR to CR  write delay: %3hu.\n", \
			byr_cycle, bxr_cycle, crl_cycle

		lda	irq_cnt			; Only update this 2 times a second!
		bit	#$1F
		bne	.skip_update

		PRINTF	"\nCycles RCR to VBLANK: %5u.\n", vbl_delay

.skip_update:	lda	irq_cnt			; Wait for vsync.

.wait_frame:	ds	256,$EA			; LOTs of 2-cycle "NOP" opcodes
		ds	128,$EA			; so that IRQ response is ASAP.

		cmp	irq_cnt
		bne	.got_vblank
		jmp	.wait_frame

.got_vblank:	call	do_autorepeat		; Autorepeat UDLR buttons.

		lda	joytrg + 0		; Read the joypad.
		bit	#JOY_U
		bne	.delay_up
		bit	#JOY_D
		bne	.delay_down
		bit	#JOY_SEL
		bne	.chg_rez

		jmp	.next_frame		; Wait for user to reboot.

; Change delay time.

.delay_up:	lda	<delay_count		; Increase delay.
		inc	a
		cmp	#200
		bcs	!+
		sta	<delay_count
!:		jmp	.next_frame

.delay_down:	lda	<delay_count		; Reduce delay.
		dec	a
		cmp	#8
		bcc	!+
		sta	<delay_count
!:		jmp	.next_frame

; Change resolution.

.chg_rez:	lda	<screen_rez		; 0=256, 1=352, 2=512. 
		inc	a
		cmp	#3
		bne	.set_rez
		cla
.set_rez:	sta	<screen_rez
		jmp	bare_main

; Sprites to show when VDC CR register is locked to disable sprites.

sprite_sat:	dw	64 + 128 - 12		; 'A' sprite.
		dw	32 + 208 - 16
		dw	$2280 >> 5
		dw	$0080

		dw	64 + 128 - 12		; 'B' sprite.
		dw	32 + 208
		dw	$22C0 >> 5
		dw	$0080

		dw	64 + 128 - 12		; 'C' sprite.
		dw	32 + 208 + 16
		dw	$2300 >> 5
		dw	$0080



; ***************************************************************************
; ***************************************************************************
;
; hsync_proc1 - RCR at line 127 to see when registers locked for next line.
;
; It takes 23 cycles to get to "delay_code" (including IRQ response).
; It takes 8+ cycles to get to "hsync_proc1" from "delay_code".
;

hsync_proc1:	pha				; 3 Save all registers.
		bit	VDC_SR			; 6

		; It takes 32 cycles + delay_count to get to here ...

		st0	#VDC_BYR		; 5
		st1	#<(GREENLINE-1)		; 5 (write is 4th cycle)

		st0	#VDC_BXR		; 5
		st1	#<64			; 5 (write is 4th cycle)

		st0	#VDC_CR			; 5
		st1	#$8C			; 5 (write is 4th cycle)

	.if	0
		lda	#<irq1_handler		; Set up the IRQ1 hook.
		sta	irq1_hook + 0
		lda	#>irq1_handler
		sta	irq1_hook + 1
	.else
		st0	#VDC_RCR		; Set up the RCR on the
		st1	#<192+64		; last display line.
		st2	#>192+64

		lda	#<hsync_proc2
		sta	irq1_hook + 0
		lda	#>hsync_proc2
		sta	irq1_hook + 1
	.endif

		lda	<vdc_reg		; Restore VDC_AR in case we
		sta	VDC_AR			; changed it.

		pla				; Restore all registers.
		rti				; Return from interrupt.



; ***************************************************************************
; ***************************************************************************
;
; hsync_proc2 - RCR at line 192 to see when VBLANK interrupt is fired.
;
; It takes 23 cycles to get to "hsync_proc2" (including IRQ response).
;

hsync_proc2:	pha				; 3 Save all registers.
		bit	VDC_SR			; 6

		lda	#<irq1_handler		; 2 Set up the IRQ1 hook.
		sta	irq1_hook + 0		; 5
		lda	#>irq1_handler		; 2
		sta	irq1_hook + 1		; 5

		cli				; 2

		; It takes 48 cycles to get here (including IRQ response).

delay_proc2:	ds	256,$EA			; LOTs of 2-cycle "NOP" opcodes.
		ds	256,$EA			; LOTs of 2-cycle "NOP" opcodes.

		pla				; Restore all registers.
		rti				; Return from interrupt.



; ***************************************************************************
; ***************************************************************************
;
; vsync_proc - When VBLANK occurs, reset the RCR chain for the next frame.
;

vsync_proc:	st0	#VDC_BYR
		st1	#<0
		st2	#>0

		st0	#VDC_BXR
		st1	#<0
		st2	#>0

		st0	#VDC_RCR
		st1	#<(REDLINE+64)
		st2	#>(REDLINE+64)

		jsr	set_delay		; Set up the delay.

		lda	#<delay_code		; Set up the IRQ1 hook.
		sta	irq1_hook + 0
		lda	#>delay_code
		sta	irq1_hook + 1

		tsx				; Calculate the number of
		sec				; delay_proc2 instructions
		lda	$2107, x		; that were executed.
		sbc	#<delay_proc2
		sta.l	vbl_delay
		lda	$2108, x
		sbc	#>delay_proc2
		sta.h	vbl_delay

		asl.l	vbl_delay		; #instructions -> #cycles.
		rol.h	vbl_delay

		lda.l	vbl_delay		; Add the cycles executed in
		adc.l	#48			; hsync_proc2.
		sta.l	vbl_delay
		bcc	!+
		inc.h	vbl_delay

!:		rts



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



; ***************************************************************************
; ***************************************************************************
;
; set_delay - Set up an 'n'-cycle delay routine.
;
; n x NOP = 2n
; 1 x JMP = 4
;
; n x NOP = 2n
; 1 x BIT = 5
; 1 x JMP = 4
;

set_delay:	tai	.nopnop, delay_code, 128

		lda	<delay_count		; Minumum delay of 8 cycles.
		cmp	#8
		bcs	.calc
		lda	#8
		sta	<delay_count

.calc:		sec
		sbc	#4
		lsr	a
		tax
		bcc	.done

		dex	        		; A "BIT" replaces two "NOP".
		dex

		lda	#$2C			; "BIT abs" opcode.
		sta	delay_code, x
		inx
		lda	#<delay_code
		sta	delay_code, x
		inx
		lda	#>delay_code
		sta	delay_code, x
		inx

.done:		lda	#$4C			; "JMP abs" opcode.
		sta	delay_code, x
		inx
		lda	#<hsync_proc1
		sta	delay_code, x
		inx
		lda	#>hsync_proc1
		sta	delay_code, x
		rts

;		lda	#$60			; "RTS" opcode.
;		sta	delay_code, x
;		rts

.nopnop:	dw	$EAEA



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
;  $0 = transparent
;  $1 = dark blue shadow
;  $2 = white font
;
;  $4 = dark blue background
;  $5 = light blue shadow
;  $6 = yellow font
;

		.data

		align	2

screen_pal:	; Main Text.
		dw	$0008,$0001,$01B2,$01B2,$0008,$0061,$01FF,$01B2
		dw	$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000

		; Background lines, Sprites
		dw	$0000,$0159,$0007,$00F9,$0038,$01FF,$00AD,$0039
		dw	$0002,$01B2,$0000,$0000,$0000,$0000,$0002,$0006

		; Help Text
		dw	$0000,$0001,$01B2,$01B2,$0008,$0061,$0186,$01B2
		dw	$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000



; ***************************************************************************
; ***************************************************************************
;
; It's the font data, nothing exciting to see here!
;

my_font:	incbin	"font8x8-ascii-bold-short.dat"



; ***************************************************************************
; ***************************************************************************
;
; Graphics for the lines on the screen.
;

screen_spr:	incspr	"testspr.png",0,32,16,1
screen_bat:	incbin	"c8hvuld.bat"
screen_chr:	incbin	"c8hvuld.chr"
screen_end:
