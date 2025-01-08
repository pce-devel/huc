; ***************************************************************************
; ***************************************************************************
;
; hello.asm
;
; "hello, world" example of using the "CORE(not TM)" PC Engine library code.
;
; Copyright John Brandwood 2021.
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
; It is running on a HuCARD, without Turbo Everdrive support, so the setup
; is the simplest possible!
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

		;
		; Include the library, reading the project's configuration
		; settings from the local "core-config.inc", if it exists.
		;

		include "bare-startup.asm"	; Minimal startup code.

		.list
		.mlist

		include	"common.asm"		; Common helpers.
		include	"vdc.asm"		; Useful VDC routines.
		include	"font.asm"		; Useful font routines.
		include "joypad.asm"		; Joypad routines.



; ***************************************************************************
; ***************************************************************************
;
; bare_main - This is executed after startup library initialization.
;

		.code

bare_main:	; Turn the display off and initialize the screen mode.

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

	.if	1

		; Upload the palette data to the VCE.
		;
		; This method shows writing directly to the VCE.

		lda	#<cpc464_colors		; Set the ptr to the palette
		sta	<_bp + 0		; data.
		lda	#>cpc464_colors
		sta	<_bp + 1
		lda	#^cpc464_colors
		tam3

		stz	VCE_CTA + 0		; Set VCE write address.
		stz	VCE_CTA + 1

		ldy	#16
.color_loop:	lda	[_bp]
		sta	VCE_CTW + 0		; Set lo-byte of color.
		inc	<_bp + 0
		lda	[_bp]
		sta	VCE_CTW + 1		; Set hi-byte of color.
		inc	<_bp + 0
		bne	.color_page
		jsr	inc.h_bp_mpr3
.color_page:	dey

		bne	.color_loop

	.else

		; Upload the palette data to the VCE.
		;
		; This method does the same thing using library functions.

		stz	<_al			; Start at palette 0 (BG).
		lda	#1			; Copy 1 palette of 16 colors.
		sta	<_ah
		lda	#<cpc464_colors		; Set the ptr to the palette
		sta	<_bp + 0		; data.
		lda	#>cpc464_colors
		sta	<_bp + 1
		ldy	#^cpc464_colors
		call	load_palettes		; Add to the palette queue.

		call	xfer_palettes		; Transfer queue to VCE now.

	.endif

		; Display the classic "hello, world" on the screen.

		lda	#<(13*64 + 10)		; Destination VRAM address.
		sta	<_di + 0
		lda	#>(13*64 + 10)
		sta	<_di + 1
		call	vdc_di_to_mawr

		cly				; Display the message.
		bsr	.print_message

		; Turn on the BG & SPR layers, then wait for a soft-reset.

		call	set_dspon		; Enable background.

.hang:		call	wait_vsync		; Wait for vsync.
		bra	.hang			; Wait for user to reboot.

;
; Write an ASCII text string directly to the VDC's VWR "data write" register.
;

.print_loop:	clc
		adc	#<CHR_ZERO
		sta	VDC_DL
		lda	#$00
		adc	#>CHR_ZERO
		sta	VDC_DH
		iny
.print_message:	lda	.message_text, y
		bne	.print_loop
		rts

.message_text:	db	"hello, world",0



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
; cpc464_colors - Palette data (a blast-from-the-past!)
;
;  $0 = transparent
;  $1 = dark blue shadow
;  $2 = white font
;
;  $4 = dark blue background
;  $5 = light blue shadow
;  $6 = yellow font
;

		align	2
cpc464_colors:	dw	$0000,$0001,$01B2,$01B2,$0002,$004C,$0169,$01B2
		dw	$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000



; ***************************************************************************
; ***************************************************************************
;
; It's the font data, nothing exciting to see here!
;

my_font:	incbin	"font8x8-ascii-bold-short.dat"
