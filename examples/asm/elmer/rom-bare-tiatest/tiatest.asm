; ***************************************************************************
; ***************************************************************************
;
; tiatest.asm
;
; TIA tester HuCARD example of using the basic HuCARD startup library code.
;
; Copyright John Brandwood 2022-2023.
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

WAIT_LINES	=	22			; #lines to wait for TIA.

		.zp

screen_rez	ds	1			; 0=256, 1=352, 2=512.
want_delay	ds	2			; Minimum  0, Maximum 500.
delay_count	ds	2			; Minimum  8, Maximum 510.
num_sprites	ds	1			; Minimum  1, Maximum 16.

move_spr	ds	1			;

which_rcr:	ds	1

		.bss

tia_delay	ds	2 * 2
delay_code	ds	256			; Modified delay before TIA.

ram_tia		ds	6144			; TIA + lots of delay after.
ram_sat		ds	128



; ***************************************************************************
; ***************************************************************************
;
; bare_main - This is executed after startup library initialization.
;

		.code

		; Reset the IRQ hooks when switching resolution.

bare_main:	jsr	bare_clr_hooks

		; Turn the display off and initialize the screen mode.

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

		; Turn all 16x16 sprites into 16x64 sprites, one pair at a time.

		stz.l	<_di
		lda.h	#$1000
		sta.h	<_di
		jsr	vdc_di_to_mawr

		lda.l	#hex_spr
		sta.l	<_bp
		lda.h	#hex_spr
		sta.h	<_bp
		cly
.loop_32x64:	ldx	#4
.loop_32x16:	lda	[_bp], y
		sta	VDC_DL
		iny
		lda	[_bp], y
		sta	VDC_DH
		iny
		bne	.loop_32x16
		dex
		bne	.loop_32x16
		inc.h	<_bp
		lda.h	<_bp
		cmp.h	#hex_spr + 16 * 128
		bcc	.loop_32x64

		; Write the checkerboard BAT areas.

		lda.l	#0 + (10 * 64)
		sta.l	<_di
		lda.h	#0 + (10 * 64)
		sta.h	<_di
		jsr	.checkerboard

		lda.l	#0 + (19 * 64)
		sta.l	<_di
		lda.h	#0 + (19 * 64)
		sta.h	<_di
		jsr	.checkerboard

		; Set up the TIA in ram, if it's not already done.

		lda	ram_tia
		bne	!+

		php
		sei
		tai	.nopnop, ram_tia, 6144	; Setup TIA subroutine.
		tii	rom_tia, ram_tia, 7
		lda	#$60			; RTS
		sta	ram_tia + 6143

		tii	hex_sat, ram_sat, 16 * 8

		stz	<move_spr

		jsr	init_delay
		bit	VDC_SR
		plp

		; Initialize the number of sprites on the line.

		lda	#16
		sta	<num_sprites

		; Enable the IRQ test processing.

!:		lda.l	#vsync_proc
		sta.l	vsync_hook
		lda.h	#vsync_proc
		sta.h	vsync_hook
		lda	#$10
		tsb	<irq_vec

		; Turn on the BG & SPR layers, then wait for a soft-reset.

		call	set_dspon		; Enable background.

		; Loop around updating the display each frame.

		PRINTF	"\e<\eX1\eY1\eP0***PC ENGINE TIA SPEED TEST***\eP1\eX1\eY3\x1E\x1F\eP0:Del\eP1 \x1C\x1D\eP0:Siz\eP1 SEL\eP0:Mode\eP1 RUN\eP0:Spr\eP0"

		lda	#1			; Delay first VBLANK cycles
		sta	irq_cnt			; update.

.next_frame:	PRINTF	"\eXL1\eY4\nCycles from RCR to TIA:  %5u\n", want_delay

		PRINTF	"TIA xfer size in bytes:  %5u\n", ram_tia + 5

		lda	irq_cnt			; Only update this 2 times a second!
		bit	#$1F
		bne	.skip_update

		PRINTF	"TIA cycles (%2hu sprites): %5u\n", num_sprites, tia_delay + 0
		PRINTF	"TIA cycles (no sprites): %5u\n", tia_delay + 2

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
		bit	#JOY_R
		bne	.size_up
		bit	#JOY_L
		bne	.size_down
		bit	#JOY_SEL
		bne	.chg_rez
		bit	#JOY_RUN
		bne	.dec_sprites

		jmp	.next_frame		; Wait for user to reboot.

; Change delay time.

.delay_up:	lda.l	<want_delay		; Increase delay.
		cmp.l	#500
		lda.h	<want_delay
		sbc.h	#500
		bcs	.du_done

;		inc.l	<want_delay
;		bne	.du_done
;		inc.h	<want_delay

		lda.l	<want_delay
		adc	#2
		sta.l	<want_delay
		bcc	.du_done
		inc.h	<want_delay

.du_done:	jmp	.next_frame

.delay_down:	lda.l	<want_delay		; Reduce delay.
		cmp.l	#0 + 1
		lda.h	<want_delay
		sbc.h	#0 + 1
		bcc	.dd_done

;		lda.l	<want_delay
;		bne	!+
;		dec.h	<want_delay
;!:		dec.l	<want_delay

		lda.l	<want_delay
		sbc	#2
		sta.l	<want_delay
		bcs	.dd_done
		dec.h	<want_delay

.dd_done:	jmp	.next_frame

; Change transfer size.

.size_up:	lda.l	ram_tia + 5		; Increase TIA size.
		cmp.l	#1024
		lda.h	ram_tia + 5
		sbc.h	#1024
		bcs	.su_done

		lda.l	ram_tia + 5
		adc	#8
		sta.l	ram_tia + 5
		bcc	.su_done
		inc.h	ram_tia + 5

.su_done:	jmp	.next_frame

.size_down:	lda.l	ram_tia + 5		; Reduce TIA size.
		cmp.l	#8 + 1
		lda.h	ram_tia + 5
		sbc.h	#8 + 1
		bcc	.sd_done

		lda.l	ram_tia + 5
		sbc	#8
		sta.l	ram_tia + 5
		bcs	.sd_done
		dec.h	ram_tia + 5

.sd_done:	jmp	.next_frame

; Change # sprites.

.dec_sprites:	lda	num_sprites		; Change # of sprites shown.
		dec	a
		bne	!+
		lda	#16
!:		sta	num_sprites
		jmp	.next_frame

; Change resolution.

.chg_rez:	lda	<screen_rez		; 0=256, 1=352, 2=512. 
		inc	a
		cmp	#3
		bne	.set_rez
		cla
.set_rez:	sta	<screen_rez
		jmp	bare_main

; Simple VBLANK IRQ handler for the BARE library.

.vblank_irq:	cli				; Allow RCR and TIMER IRQ.
		jmp	xfer_palettes		; Transfer queue to VCE now.

; Write a checkerboard BAT section.

.checkerboard:	jsr	vdc_di_to_mawr
		lda	#>(CHR_0x20 + $2000)
		st1	#<(CHR_0x20 + $2000)
		ldy	#2
.next_line:	ldx	#8 * 4
.line_loop:	sta	VDC_DH
		sta	VDC_DH
		sta	VDC_DH
		sta	VDC_DH
		eor.h	#$1000
		sta	VDC_DH
		sta	VDC_DH
		sta	VDC_DH
		sta	VDC_DH
		eor.h	#$1000
		dex
		bne	.line_loop
		eor.h	#$1000
		dey
		bne	.next_line

		rts

; Write a checkerboard BAT section.

.nopnop:	dw	$EAEA

rom_tia:	tia	$8000, VDC_DL, 32



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

		; Delay, then TIA, then lots and lots of delay!
		;
		; We want get here (455 - 15) cycles from the RCR.

		jsr	delay_code		; 7 plus minimum 8 before TIA.

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
		sbc.l	#ram_tia + 7
		sta.l	tia_delay, y
		lda.h	$2105, x
		sbc.h	#ram_tia + 7
		sta.h	tia_delay, y

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

		jsr	clr_delay		; Reset up the delay.

		clc				; delay_count = want_delay + 8
		lda.l	<want_delay
		adc.l	#8
		sta.l	<delay_count
		lda.h	<want_delay
		adc.h	#8
		sta.h	<delay_count

		jsr	set_delay		; Set up the delay.

		jsr	xfer_palettes		; Transfer queue to VCE now.

		; Change # sprites shown by moving others down 256 lines.

		ldy	<num_sprites
		clc
		cla
		clx
!:		stz.h	ram_sat, x
		adc	#8
		tax
		dey
		bne	!-
		bra	.check

!:		lda	#1
		sta.h	ram_sat, x
		txa
		adc	#8
		tax
.check:		cpx	#8 * 16
		bne	!-

		; Wibble the sprites.

!:		lda	irq_cnt			; Move every 8th 1/60th.
		and	#$07
		bne	!+

		lda	<move_spr		; Flip between moving up and
		inc	<move_spr		; down.
		and	#$08
		jsr	.inc_spr
		eor	#$08
		jsr	.dec_spr

		stz.l	<_di			; Copy RAM SATB to VDC SATB.
		lda.h	#$0800
		sta.h	<_di
		jsr	vdc_di_to_mawr

		tia	ram_sat, VDC_DL, 16 * 8

!:		rts

.inc_spr:	tax				; Move alternate the sprites.
		inc	ram_sat + $00, x
		inc	ram_sat + $10, x
		inc	ram_sat + $20, x
		inc	ram_sat + $30, x
		inc	ram_sat + $40, x
		inc	ram_sat + $50, x
		inc	ram_sat + $60, x
		inc	ram_sat + $70, x
		rts

.dec_spr:	tax				; Move alternate the sprites.
		dec	ram_sat + $00, x
		dec	ram_sat + $10, x
		dec	ram_sat + $20, x
		dec	ram_sat + $30, x
		dec	ram_sat + $40, x
		dec	ram_sat + $50, x
		dec	ram_sat + $60, x
		dec	ram_sat + $70, x
		rts



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

tbl_rcr:	dw	64 + ((10 * 8) + 8)
		dw	64 + ((10 * 8) + 8 + 1 + WAIT_LINES)
		dw	64 + ((19 * 8) + 8)
		dw	64 + ((19 * 8) + 8 + 1 + WAIT_LINES)
		dw	0

tbl_hook:	dw	hsync_proc1
		dw	hsync_proc2
		dw	hsync_proc1
		dw	hsync_proc2
		dw	irq1_handler



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
; set_delay - Set up an 'n'-cycle delay routine (min 8, max 510).
;
; n x NOP = 2n
; 1 x JMP = 4
;
; n x NOP = 2n
; 1 x BIT = 5
; 1 x JMP = 4
;

init_delay:	tai	.nopnop, delay_code, 256
		rts

.nopnop:	dw	$EAEA

		;
		;
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
		lda	#<ram_tia
		sta	delay_code, x
		inx
		lda	#>ram_tia
		sta	delay_code, x
		rts

		;
		;
		;

clr_delay:	lda.h	<delay_count		; Minumum delay of 8 cycles.
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
		lda	#$EA			; NOP
		bcc	.done

		dex	        		; A "BIT" replaces two "NOP".
		dex

		sta	delay_code, x		; Replace "BIT" with "NOP".
		inx
		sta	delay_code, x
		inx
		sta	delay_code, x
		inx

.done:		sta	delay_code, x		; Replace "JMP" with "NOP".
		inx
		sta	delay_code, x
		inx
		sta	delay_code, x
		rts



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
; Graphics for the lines on the screen.
;

hex_spr:	incspr	"testspr.png",0,32,16,1

hex_sat:	dw	64 + 80 - 4		; '0' sprite.
		dw	32 + $40
		dw	$1000 >> 5
		dw	%0011000010000000	; CGY=3, CGX=0 (16x64).

		dw	64 + 80 + 4		; '1' sprite.
		dw	32 + $48
		dw	$1040 >> 5
		dw	%0011000010000001	; CGY=3, CGX=0 (16x64).

		dw	64 + 80 - 4		; '2' sprite.
		dw	32 + $50
		dw	$1200 >> 5
		dw	%0011000010000000	; CGY=3, CGX=0 (16x64).

		dw	64 + 80 + 4		; '3' sprite.
		dw	32 + $58
		dw	$1240 >> 5
		dw	%0011000010000001	; CGY=3, CGX=0 (16x64).

		dw	64 + 80 - 4		; '4' sprite.
		dw	32 + $60
		dw	$1400 >> 5
		dw	%0011000010000000	; CGY=3, CGX=0 (16x64).

		dw	64 + 80 + 4		; '5' sprite.
		dw	32 + $68
		dw	$1440 >> 5
		dw	%0011000010000001	; CGY=3, CGX=0 (16x64).

		dw	64 + 80 - 4		; '6' sprite.
		dw	32 + $70
		dw	$1600 >> 5
		dw	%0011000010000000	; CGY=3, CGX=0 (16x64).

		dw	64 + 80 + 4		; '7' sprite.
		dw	32 + $78
		dw	$1640 >> 5
		dw	%0011000010000001	; CGY=3, CGX=0 (16x64).

		dw	64 + 80 - 4		; '8' sprite.
		dw	32 + $80
		dw	$1800 >> 5
		dw	%0011000010000000	; CGY=3, CGX=0 (16x64).

		dw	64 + 80 + 4		; '9' sprite.
		dw	32 + $88
		dw	$1840 >> 5
		dw	%0011000010000001	; CGY=3, CGX=0 (16x64).

		dw	64 + 80 - 4		; 'A' sprite.
		dw	32 + $90
		dw	$1A00 >> 5
		dw	%0011000010000000	; CGY=3, CGX=0 (16x64).

		dw	64 + 80 + 4		; 'B' sprite.
		dw	32 + $98
		dw	$1A40 >> 5
		dw	%0011000010000001	; CGY=3, CGX=0 (16x64).

		dw	64 + 80 - 4		; 'C' sprite.
		dw	32 + $A0
		dw	$1C00 >> 5
		dw	%0011000010000000	; CGY=3, CGX=0 (16x64).

		dw	64 + 80 + 4		; 'D' sprite.
		dw	32 + $A8
		dw	$1C40 >> 5
		dw	%0011000010000001	; CGY=3, CGX=0 (16x64).

		dw	64 + 80 - 4		; 'E' sprite.
		dw	32 + $B0
		dw	$1E00 >> 5
		dw	%0011000010000000	; CGY=3, CGX=0 (16x64).

		dw	64 + 80 + 4		; 'F' sprite.
		dw	32 + $B8
		dw	$1E40 >> 5
		dw	%0011000010000001	; CGY=3, CGX=0 (16x64).
