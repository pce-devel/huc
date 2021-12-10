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
;   MPR2 = bank $00 : HuCard ROM
;   MPR3 = bank $01 : HuCard ROM
;   MPR4 = bank $02 : HuCard ROM
;   MPR5 = bank $03 : HuCard ROM
;   MPR6 = bank $04 : HuCard ROM
;   MPR7 = bank $00 : HuCard ROM with "CORE(not TM)" kernel and IRQ vectors.
;
; ***************************************************************************
; ***************************************************************************


		; Include the library, reading the project's configuration
		; settings from the local "core-config.inc", if it exists.

		include "core.inc"		; Basic includes.

		.list
		.mlist

		include	"common.asm"		; Common helpers.
		include	"vdc.asm"		; Useful VDC routines.
		include	"font.asm"		; Useful font routines.



; ***************************************************************************
; ***************************************************************************
;
; Create some equates for a simple VRAM layout, with a 64*32 BAT, followed by
; the SAT, then followed by the tiles for the ASCII character set.
;

BAT_SIZE	=	64 * 32
SAT_ADDR	=	BAT_SIZE		; SAT takes 16 tiles of VRAM.
CHR_ZERO	=	BAT_SIZE / 16		; 1st tile # after the BAT.
CHR_0x10	=	CHR_ZERO + 16		; 1st tile # after the SAT.
CHR_0x20	=	CHR_ZERO + 32		; ASCII ' ' CHR tile #.



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

		lda	#<my_font		; Address of font data.
		sta	<__si + 0
		lda	#>my_font
		sta	<__si + 1
		lda	#^my_font
		sta	<__si_bank

		stz	<__di + 0		; Destination VRAM address.
		lda	#>(CHR_0x10 * 16)
		sta	<__di + 1

		lda	#$FF			; Put font in colors 4-7,
		sta	<__al			; so bitplane 2 = $FF and
		stz	<__ah			; bitplane 3 = $00.

		lda	#16 + 96		; 16 graphics + 96 ASCII.
		sta	<__bl

		call	dropfnt8x8_vdc		; Upload font to VRAM.

	.if	1

		; Upload the palette data to the VCE.
		;
		; This method shows writing directly to the VCE.

		lda	#<cpc464_colors		; Set the ptr to the palette
		sta	<__si + 0		; data.
		lda	#>cpc464_colors
		sta	<__si + 1
		lda	#^cpc464_colors
		sta	<__si_bank
		jsr	__si_to_mpr3		; Map data to MPR3 & MPR4.

		stz	VCE_CTA + 0		; Set VCE write address.
		stz	VCE_CTA + 1

		ldy	#16
.color_loop:	lda	[__si]
		sta	VCE_CTW + 0		; Set lo-byte of color.
		inc	<__si + 0
		lda	[__si]
		sta	VCE_CTW + 1		; Set hi-byte of color.
		inc	<__si + 0
		bne	.color_page
		jsr	__si_inc_page
.color_page:	dey

		bne	.color_loop

	.else

		; Upload the palette data to the VCE.
		;
		; This method does the same thing using library functions.

		lda	#<cpc464_colors		; Set the ptr to the palette
		sta	<__si + 0		; data.
		lda	#>cpc464_colors
		sta	<__si + 1
		lda	#^cpc464_colors
		sta	<__si_bank
		stz	<__al			; Start at palette 0 (BG).
		lda	#1			; Copy 1 palette of 16 colors.
		sta	<__ah
		call	load_palettes		; Add to the palette queue.

		call	xfer_palettes		; Transfer queue to VCE now.

	.endif

		; Display the classic "hello, world" on the screen.

		lda	#<(13*64 + 10)		; Destination VRAM address.
		sta	<__di + 0
		lda	#>(13*64 + 10)
		sta	<__di + 1
		call	__di_to_vdc

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
; Put the data at the start of the next bank after the code.
;
; This is complicated by PCEAS's "bank()" function reporting the target bank,
; which is $68..$87 for a Super CD-ROM, but the ".bank" pseudo-op uses a bank
; number relative to the start of the overlay.
;
; The difference can be fixed by using the "_bank_base" value that PCEAS sets
; at the start of the assembly.
;
; _bank_base = $00 when building a HuCARD ROM.
; _bank_base = $68 when building with the "-scd" flag (for Super CD-ROM).
; _bank_base = $80 when building with the "-cd" flag (for original CD-ROM).
;

DATA_BANK	=	bank(*) + 1 - _bank_base

		.data
		.bank	DATA_BANK
		.org	$6000



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

		WORD_ALIGN
cpc464_colors:	dw	$0000,$0001,$01B2,$01B2,$0002,$004C,$0169,$01B2
		dw	$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000



; ***************************************************************************
; ***************************************************************************
;
; It's the font data, nothing exciting to see here!
;

my_font:	incbin	"font8x8-ascii-bold-short.dat"
