; ***************************************************************************
; ***************************************************************************
;
; hwdetect.asm
;
; Example of detecting PC Engine hardware and testing attached Joypads/Mice.
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
; This is an example of a simple HuCARD ROM that uses the "CORE(not TM)"
; library to provide interrupt-handling and joypad reading.
;
; It is running on a HuCARD, with Turbo Everdrive support, so the setup is
; a bit more complicated than usual!
;
; The PC Engine's memory map is set to ...
;
;   MPR0 = bank $FF : PCE hardware
;   MPR1 = bank $F8 : PCE RAM with Stack & ZP
;   MPR2 = bank $02 : HuCard ROM
;   MPR3 = bank $03 : HuCard ROM
;   MPR4 = bank $04 : HuCard ROM
;   MPR5 = bank $05 : HuCard ROM
;   MPR6 = bank $xx : HuCard ROM banked ASM library procedures.
;   MPR7 = bank $02 : HuCard ROM with "CORE(not TM)" kernel and IRQ vectors.
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

CHR_TALL	=	$0600			; 256 8x16 CHR ($6000-$7FFF).

		;
		; You would normally just set this in your project's local
		; "core-config.inc", but this shows another way to achieve
		; the same result.
		;

SUPPORT_TED2	=	1	; Support the Turbo Everdrive's use of bank0.

SUPPORT_SGX	=	1	; Support the SuperGrafx.

SUPPORT_6BUTTON	=	1	; Support both 6BUTTON and MOUSE so that we
SUPPORT_MOUSE	=	1	; can do some hardware testing.

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
		include	"ted2.asm"		; TED2 hardware routines.
		include	"mb128-base.asm"	; Lo-level MB128 routines.
		include	"joypad-extra.asm"	; Optional joypad routines.

		include	"tty.asm"		; Useful TTY print routines.



; ***************************************************************************
; ***************************************************************************
;
; Constants and Variables.
;

		.zp

pointer_xlo:	ds	5
pointer_xhi:	ds	5
pointer_ylo:	ds	5
pointer_yhi:	ds	5

		.bss

ports_found:	ds	1			; 0=undetermined, NZ=count.
which_port:	ds	1
port_type:	ds	5
port_mask:	ds	1
mtap_type:	ds	1

spr_sat:	ds	8*5			; Mouse pointer sprites.

		.code



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

		call	init_320x208		; Initialize VDC & VRAM.

		; Upload the 8x8 font to VRAM in colors 0-3, with box tiles
		; using colors 4-7.
		;
		; Thus we call dropfntbox_vdc() instead of dropfnt8x8_vdc()!

		stz	<_di + 0		; Destination VRAM address.
		lda	#>(CHR_0x10 * 16)
		sta	<_di + 1

;		lda	#$FF			; Put font in colors 0-3,
		stz	<_al			; so bitplane 2 = $00 and
		stz	<_ah			; bitplane 3 = $00.

		lda	#16 + 96		; 16 box graphics + 96 ASCII.
		sta	<_bl

		lda	#<my_font		; Address of font data.
		sta	<_bp + 0
		lda	#>my_font
		sta	<_bp + 1
		ldy	#^my_font

		call	dropfntbox_vdc		; Upload font to VRAM.

		; Upload the mouse pointer sprites to VRAM after the font.

		tia	pointer_spr, VDC_DL, 128*5

		; Upload the tall font to VRAM.

		stz	<_di + 0		; Destination VRAM address.
		lda	#>(CHR_TALL * 16)
		sta	<_di + 1

;		lda	#$FF			; Put font in colors 0-3,
		stz	<_al			; so bitplane 2 = $00 and
		stz	<_ah			; bitplane 3 = $00.

		lda.l	#256			; 256 characters.
		sta	<_bl

		lda	#<my_tall		; Address of font data.
		sta	<_bp + 0
		lda	#>my_tall
		sta	<_bp + 1
		ldy	#^my_tall

		call	dropfnt8x16_vdc		; Upload font to VRAM.

		; Upload textured SuperGRAFX background.

		lda	sgx_detected		; Is this a SuperGrafx?
		beq	!+

		stz.l	<_di			; Write the SGX BAT.
		stz.h	<_di
		jsr	sgx_di_to_mawr
		inc	VPC_STMODE		; Switch STx to SGX.
		ldx	#32/2
		lda	#64
.sgx_lines:	tay
.sgx_line0:	st1	#$A0			; Write a line of $00A0.
		st2	#$00
		dey
		bne	.sgx_line0
		tay
.sgx_line1:	st1	#$A1			; Write a line of $00A1.
		st2	#$00
		dey
		bne	.sgx_line1
		dex
		bne	.sgx_lines
		stz	VPC_STMODE		; Switch STx to VDC.

		stz.l	<_di			; Upload 2 textured tiles.
		lda	#>$0A00
		sta.h	<_di
		jsr	sgx_di_to_mawr
		tia	texture_chr, SGX_DL, 32*2

		; Upload the palette data to the VCE.

!:		stz	<_al			; Start at palette 0 (BKG).
		lda	#5			; Copy 4 palettes of 16 colors.
		sta	<_ah
		lda	#<bkg_palette		; Set the ptr to the palette
		sta	<_bp + 0		; data.
		lda	#>bkg_palette
		sta	<_bp + 1
		ldy	#^bkg_palette
		call	load_palettes		; Add to the palette queue.

		lda	#16			; Start at palette 16 (SPR).
		sta	<_al
		lda	#1			; Copy 1 palette of 16 colors.
		sta	<_ah
		lda	#<spr_palette		; Set the ptr to the palette
		sta	<_bp + 0		; data.
		lda	#>spr_palette
		sta	<_bp + 1
		ldy	#^spr_palette
		call	load_palettes		; Add to the palette queue.

		call	xfer_palettes		; Transfer queue to VCE now.

		; Display the optional PC Engine hardware that we can detect.

		PRINTF	"\e<\eP0\eX0\eY0\eB40,9,2\eX0\eY9\eB40,17,2"

		PRINTF	"\e>\eX2\eY1\eP0PC Engine Hardware Detection ...\eP1\e<\eXL2\eX2\eY4"

		PRINTF	"SuperGRAFX \eP2..............\eX25"
		lda	sgx_detected		; Is this a SuperGrafx?
		eor	#$7F
		jsr	print_result

		PRINTF	"CD Interface \eP2............\eX25"
		lda	IO_PORT			; Is there a CD Interface?
		and	#$80
		jsr	print_result

		PRINTF	"Memory Base 128 \eP2.........\eX25"
		call	mb1_detect		; Is there an MB128 attached?
		jsr	print_result

		PRINTF	"Turbo EverDrive v2 \eP2......\eX25"
		call	unlock_ted2		; Unlock the TED2 hardware.
		jsr	print_result

		PRINTF	"\e>\eX2\eY10\eP0PC Engine Joypad Ports ...\eP1\e<\eXL2\eX2\eY13"

		; Initialize the pointer sprite SAT in BSS RAM.

		tii	spr_sat_init, spr_sat, 8*5

		; Enable VBLANK IRQ processing for SGX scrolling.

		lda.l	#vsync_proc
		sta.l	vsync_hook
		lda.h	#vsync_proc
		sta.h	vsync_hook
		lda	#$10
		tsb	<irq_vec

		; Turn on the BG & SPR layers, then wait for a soft-reset.

		call	set_dspon		; Enable background.

		call	wait_vsync		; Wait for vsync.

		; Wait for user to reboot.

.main_loop:	call	wait_vsync_usb		; Wait for VBLANK, or xfer.

		; Update the multitap detection estimate (not 100% accurate).

		jsr	check_multitap
		jsr	print_multitap

		; Update the positions of the mouse pointer(s).

		jsr	move_mice		; Update the pointers.

		stz.l	<_di			; Update the SATB in VRAM.
		lda	#>SAT_ADDR
		sta.h	<_di
		jsr	vdc_di_to_mawr
		tia	spr_sat, VDC_DL, 8*5

		; Display each joypad's button state.

		PRINTF	"\eX2\eY14"

		ldx	#0			; Start at port 0.
		stx	which_port
.port_loop:	phx

		jsr	check_type		; Update the device type.

		inc	which_port		; Display 1..5, not 0..4!

		PRINTF	"\n\eP1Port %hu \eP2................\eX25", which_port

		plx				; Display device type.
		phx
		jsr	print_type

		plx				; Display device buttons.
		phx
		jsr	print_buttons

		plx
		inx
		cpx	num_ports
		bcc	.port_loop

		; Disabled because we want to detect the number of ports.

	.if	0

		bit	mouse_flg		; If NEC MultiTap was found
		bvs	.main_loop		; then all ports are unique.

		lda	port_type		; Get the Port-1 device type.
		ldx	#0
.test_port:	cmp	port_type, x		; If any other port contains
		bne	.different		; a different device, then a
		inx				; multitap MUST be present.
		cpx	num_ports
		bcc	.test_port

		bra	.main_loop		; Wait for user to reboot.

.different:	lda	#$40			; Signal that a multitap is
		tsb	mouse_flg		; present.

	.endif

		; All done for this frame, keep on looping!

		bra	.main_loop		; Wait for user to reboot.



; ***************************************************************************
; ***************************************************************************
;
; print_result - Print detection status for a device.
;
; Args: NZ/Z = Z if "DEVICE FOUND", NZ if "NOT DETECTED".
;

print_result:	beq	!+
		PRINTF	not_detected
		rts
!:		PRINTF	got_hardware
		rts

got_hardware:	STRING	"\eP2 DEVICE FOUND\eP1\n"
not_detected:	STRING	"\eP2 not detected\eP1\n"



; ***************************************************************************
; ***************************************************************************
;
; check_multitap - Check whether we can detect if a multitap is present.
;
; This is NOT a definitive test, because 2 people pressing the same button
; at the same instant will confuse it, and so will a button changing state
; in the middle of a read.
;
; The detect_1port() function is much safer function, but it should only be
; used to detect if there is NO multitap present (i.e. 1 port), because its
; detection of more-than-1-port suffers from the same problem when a joypad
; button changes state in the middle of a read.
;

check_multitap:	call	detect_ports		; Attempt to determine the
		beq	.finished		; number of joypad ports.

		sta	ports_found		; Update the multitap type.

.finished:	rts



; ***************************************************************************
; ***************************************************************************
;
; print_multitap - Display the current "best-estimate" of the multitap type.
;

print_multitap:	PRINTF	"\eX2\eY13\eP1MultiTap \eP2................\eX25"

		lda	ports_found
		asl	a
		tax
		ldy	port_table, x
		inx
		lda	port_table, x
		sxy
		ldy	#bank(port_table)
;		jmp	tty_printf
		jsr	tty_printf

		PRINTF	"\eX5\eP4Port 6 = $%0hx", port6
		rts

port_table:	dw	type_u
		dw	type_p1
		dw	type_p2
		dw	type_p3
		dw	type_p4
		dw	type_p5
;		dw	got_hardware

type_p1:	STRING	"\eP2........ NONE\n  "
type_p2:	STRING	"\eP2...... 2-PORT\n  "
type_p3:	STRING	"\eP2...... 3-PORT\n  "
type_p4:	STRING	"\eP2...... 4-PORT\n  "
type_p5:	STRING	"\eP2...... 5-PORT\n  "



; ***************************************************************************
; ***************************************************************************
;
; check_type - Check what kind of input device is attached to a port.
;
; Args: X = Port# (0-4).
;

check_type:	ldy	#3
		lda	bit_mask, x
		and	mouse_flg		; Mouse is present?
		bne	.set_type
		dey
		lda	joy6now, x		; 6-button pad is present?
		bmi	.set_type
		dey
		lda	port_type, x		; 6-button, but changed mode?
		bne	.set_type
		lda	joynow, x		; 2-button pad is present?
		beq	.done

.set_type:	tya
		sta	port_type, x
.done:		rts



; ***************************************************************************
; ***************************************************************************
;
; print_type - Print the type of the input device attached to a port.
;
; Args: X = Port# (0-4).
;

print_type:	lda	port_type, x
		asl	a
		tax
		ldy	type_table, x
		inx
		lda	type_table, x
		sxy
		ldy	#bank(type_table)
		jmp	tty_printf
;		rts

type_table:	dw	type_u
		dw	type_2
		dw	type_6
		dw	type_m

type_u:		STRING	"\eP2..... unknown\n   "
type_2:		STRING	"\eP2.... 2-BUTTON\n   "
type_6:		STRING	"\eP2.... 6-BUTTON\n   "
type_m:		STRING	"\eP2....... MOUSE\n   "



; ***************************************************************************
; ***************************************************************************
;
; print_buttons - Print the button status for each type of input device.
;
; Args: X = Port# (0-4).
;

print_buttons:	ldy	port_type, x
		lda	bit_mask, y
		sta	port_mask

		cly

.button_loop:	lda	.button_type, y		; Any more buttons?
		beq	.finished
		and	port_mask		; Does this button exist on
		beq	.skip_button		; this type of joypad?

		lda	#$F0			; Reset color.
		trb.h	tty_tile

		lda	.button_str, y		; Remember the string offset.
		pha

		lda	.button_lo, y
		and	joynow, x
		bne	.color_hi
		lda	.button_hi, y
		and	joy6now, x
		bne	.color_hi
.color_lo:	lda	#$30			; Button released.
		bra	.set_color
.color_hi:	lda	#$40			; Button pressed.
.set_color:	tsb.h	tty_tile

		pla				; Restore string offset.
		phy
		tay
		jsr	.print
		ply

.skip_button:	iny
		bra	.button_loop

.finished:	ldy	port_type, x		; End the line with spaces.
		lda	.eol_len, y
		clc
		adc	#.str_eol - .strings
		tay

.print:		lda	.strings, y		; Print a message string.
		beq	.printed
		clc
		adc.l	tty_tile
		sta	VDC_DL
		cla
		adc.h	tty_tile
		sta	VDC_DH
		iny
		bra	.print

.printed:	rts

.button_type:	db	%1110			; SEL
		db	%1110			; RUN
		db	%1110			; 1
		db	%1110			; 2
		db	%0100			; 3
		db	%0100			; 4
		db	%0100			; 5
		db	%0100			; 6
		db	%0110			; L
		db	%0110			; R
		db	%0110			; U
		db	%0110			; D
		db	0

.button_str:	db	.str_sel - .strings     ; SEL
		db	.str_run - .strings	; RUN
		db	.str_1 - .strings       ; 1
		db	.str_2 - .strings       ; 2
		db	.str_3 - .strings       ; 3
		db	.str_4 - .strings       ; 4
		db	.str_5 - .strings       ; 5
		db	.str_6 - .strings       ; 6
		db	.str_l - .strings       ; L
		db	.str_r - .strings       ; R
		db	.str_u - .strings       ; U
		db	.str_d - .strings       ; D

.button_lo:	dwl	JOY_SEL                 ; SEL
		dwl	JOY_RUN			; RUN
		dwl	JOY_B1                  ; 1
		dwl	JOY_B2                  ; 2
		dwl	JOY_B3                  ; 3
		dwl	JOY_B4                  ; 4
		dwl	JOY_B5                  ; 5
		dwl	JOY_B6                  ; 6
		dwl	JOY_L                   ; L
		dwl	JOY_R                   ; R
		dwl	JOY_U                   ; U
		dwl	JOY_D                   ; D

.button_hi:	dwh	JOY_SEL                 ; SEL
		dwh	JOY_RUN			; RUN
		dwh	JOY_B1                  ; 1
		dwh	JOY_B2                  ; 2
		dwh	JOY_B3                  ; 3
		dwh	JOY_B4                  ; 4
		dwh	JOY_B5                  ; 5
		dwh	JOY_B6                  ; 6
		dwh	JOY_L                   ; L
		dwh	JOY_R                   ; R
		dwh	JOY_U                   ; U
		dwh	JOY_D                   ; D

.strings:
.str_sel:	db	"Select ",0
.str_run:	db	"Run ",0
.str_1:		db	"1 ",0
.str_2:		db	"2 ",0
.str_3:		db	"3 ",0
.str_4:		db	"4 ",0
.str_5:		db	"5 ",0
.str_6:		db	"6 ",0
.str_l:		db	"\x1C ",0
.str_r:		db	"\x1D ",0
.str_u:		db	"\x1E ",0
.str_d:		db	"\x1F ",0
.str_eol:	db	"                ",0

.eol_len:	db	16-8
		db	16-8
		db	16-0
		db	16-16



; ***************************************************************************
; ***************************************************************************
;
; move_mice - Move the mouse pointers, staying within the screen.
;

move_mice:	clx

.port_loop:	lda	bit_mask, x		; Xvert index into mask.
		and	mouse_flg		; Mouse is present?
		bne	.init

.next_port:	inx
		cpx	num_ports
		bcc	.port_loop

		rts

		;

.init:		lda	<pointer_ylo, x
		ora	<pointer_yhi, x
		bne	.move_x

		lda	#<32+0
		sta	<pointer_xlo, x
		lda	#<64+64
		sta	<pointer_ylo, x

		;

.move_x:	clc
		lda	mouse_x, x
;		eor	#$FF
;		inc	a
		bmi	.move_l

.move_r:	adc	<pointer_xlo, x
		tay
		lda	#$00
		adc	<pointer_xhi, x
		sta	<pointer_xhi, x
		cpy	#<32+320-16
		sbc	#>32+320-16
		bcc	.done_x
		lda	#>32+320-16
		sta	<pointer_xhi, x
		ldy	#<32+320-16
		bra	.done_x

.move_l:	adc	<pointer_xlo, x
		tay
		lda	#$FF
		adc	<pointer_xhi, x
		bmi	.stop_l
		sta	<pointer_xhi, x
		cpy	#<32+0
		sbc	#>32+0
		bcs	.done_x
.stop_l:	lda	#>32+0
		sta	<pointer_xhi, x
		ldy	#<32+0

.done_x:	sty	<pointer_xlo, x

		;

.move_y:	clc
		lda	mouse_y, x
;		eor	#$FF
;		inc	a
		bmi	.move_u

.move_d:	adc	<pointer_ylo, x
		tay
		lda	#$00
		adc	<pointer_yhi, x
		sta	<pointer_yhi, x
		cpy	#<64+208-16
		sbc	#>64+208-16
		bcc	.done_y
		lda	#>64+208-16
		sta	<pointer_yhi, x
		ldy	#<64+208-16
		bra	.done_y

.move_u:	adc	<pointer_ylo, x
		tay
		lda	#$FF
		adc	<pointer_yhi, x
		bmi	.stop_u
		sta	<pointer_yhi, x
		cpy	#<64+0
		sbc	#>64+0
		bcs	.done_y
.stop_u:	lda	#>64+0
		sta	<pointer_yhi, x
		ldy	#<64+0

.done_y:	sty	<pointer_ylo, x

		;

.set_sprite:	ldy	.sat_yoffset, x
		lda	<pointer_ylo, x
		sta	spr_sat, y
		iny
		lda	<pointer_yhi, x
		sta	spr_sat, y

		ldy	.sat_xoffset, x
		lda	<pointer_xlo, x
		sta	spr_sat, y
		iny
		lda	<pointer_xhi, x
		sta	spr_sat, y

		jmp	.next_port

.sat_yoffset:	db	0*8+0, 1*8+0, 2*8+0, 3*8+0, 4*8+0
.sat_xoffset:	db	0*8+2, 1*8+2, 2*8+2, 3*8+2, 4*8+2

		;

spr_sat_init:	dw	0			; Pointer1 Y
		dw	0			; Pointer1 X
		dw	$1000 >> 5		; Pointer1 Pattern
		dw	$0080			; Pointer1 Flags

		dw	0			; Pointer2 Y
		dw	0			; Pointer2 X
		dw	$1040 >> 5		; Pointer2 Pattern
		dw	$0080			; Pointer2 Flags

		dw	0			; Pointer3 Y
		dw	0			; Pointer3 X
		dw	$1080 >> 5		; Pointer3 Pattern
		dw	$0080			; Pointer3 Flags

		dw	0			; Pointer4 Y
		dw	0			; Pointer4 X
		dw	$10C0 >> 5		; Pointer4 Pattern
		dw	$0080			; Pointer4 Flags

		dw	0			; Pointer5 Y
		dw	0			; Pointer5 X
		dw	$1100 >> 5		; Pointer5 Pattern
		dw	$0080			; Pointer5 Flags



; ***************************************************************************
; ***************************************************************************
;
; vsync_proc - Slowly scroll the SGX background.
;

vsync_proc:	lda	sgx_detected		; Is this a SuperGrafx?
		beq	!+

		lda	irq_cnt			; Slow to 15 pixels/second.
		and	#$03
		bne	!+

		inc	bg_x2
		inc	bg_y2

		ldx	#VDC_BYR
		stx	SGX_AR
		lda	bg_y2
		sta	SGX_DL

		ldx	#VDC_BXR
		stx	SGX_AR
		lda	bg_x2
		sta	SGX_DL

!:		rts



; ***************************************************************************
; ***************************************************************************
;
; Put these in BANK 0 for easy non-banked load to VRAM with a TIA.
;

pointer_spr:	incspr	"pointer.png",0,0,5,1

texture_chr:	incbin	"texture.chr"



; ***************************************************************************
; ***************************************************************************
;
; The "CORE(not TM)" library initializes .DATA, so we don't do it again!
;
; ***************************************************************************
; ***************************************************************************

		.data



; ***************************************************************************
; ***************************************************************************
;
; bkg_palette - Palette data
; spr_palette - Palette data
;
; Note: DEFPAL palette data is in RGB format, 4-bits per value.
; Note: Packed palette data is in GRB format, 3-bits per value.
;

		align	2

none		=	$000

bkg_palette:	defpal	$010,$000,$662,none,$030,$010,$662,none
		defpal	none,none,none,none,none,$110,$210,$220

		defpal	none,$000,$777,none,none,none,none,none
		defpal	none,none,none,none,none,none,none,none

		defpal	none,$000,$555,none,none,none,none,none
		defpal	none,none,none,none,none,none,none,none

		defpal	none,$000,$130,none,none,none,none,none
		defpal	none,none,none,none,none,none,none,none

		defpal	none,$000,$473,none,none,none,none,none
		defpal	none,none,none,none,none,none,none,none

spr_palette:	defpal	$030,$000,$022,$277,$777,none,none,none
		defpal	none,none,none,none,none,none,none,none



; ***************************************************************************
; ***************************************************************************
;
; It's the font data, and there is actually something exciting to see here!
;
; We want the box tiles to use a different colors, so the font begins with
; an array of 1-bit-per-tile flags to signal which are the box tiles.
;
; Then we upload with dropfntbox_vdc() instead of dropfnt8x8_vdc().
;

my_font:	db	%11111111,%00001000,0,0,0,0,0,0,0,0,0,0,0,0
		incbin	"font8x8-ascii-bold-short.dat"

my_tall:	incbin	"font8x16-syscard-bold.dat"
