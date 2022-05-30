; ***************************************************************************
; ***************************************************************************
;
; stage1.asm
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
; This is an example of a simple Super CD-ROM overlay program that uses the
; "CORE(not TM)" library to provide interrupt-handling, joypad reading, and
; the ability to replace the System Card in MPR7 ($E000-$FFFF).
;
; This CD-ROM overlay includes the "CORE(not TM)" kernel code (using 1KB-2KB
; of memory), which is copied to PCE RAM when the overlay starts.
;
; It is running on a SuperCD System, so the overlay is loaded from the ISO
; into banks $68-$87 (256KB max).
;
; The PC Engine's memory map is set to ...
;
;   MPR0 = bank $FF : PCE hardware
;   MPR1 = bank $F8 : PCE RAM with Stack & ZP & "CORE(not TM)" kernel
;   MPR2 = bank $68 : SCD RAM
;   MPR3 = bank $69 : SCD RAM
;   MPR4 = bank $6A : SCD RAM
;   MPR5 = bank $6B : SCD RAM
;   MPR6 = bank $6C : SCD RAM
;   MPR7 = bank $68 : SCD RAM or System Card's bank $00
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

		include "core.inc"		; Basic includes.

		.list
		.mlist

		include	"common.asm"		; Common helpers.
		include	"vdc.asm"		; Useful VDC routines.
		include	"font.asm"		; Useful font routines.



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

core_main:	; Turn the display off and initialize the screen mode.

		system	ex_dspoff		; Disable background/sprites.

		system	ex_vsync		; Wait for vsync.

		cla				; Set up a 256x224 video
		ldx	#256 / 8		; mode.
		ldy	#224 / 8
		system	ex_scrmod

		lda	#VDC_MWR_64x32 >> 4	; Set up 64x32 virtual
		system	ex_scrsiz		; screen.

		; Clear VRAM, including the BAT.

		lda	#<CHR_0x20		; Use tile # of ASCII ' '
		sta	<_al			; to fill the BAT, using
		lda	#>CHR_0x20		; palette 0.
		sta	<_ah

		lda	#>BAT_SIZE		; Size of BAT in words.
		sta	<_bl

		call	clear_vram_vdc		; Initialize VRAM.

		; Upload the font to VRAM.

		lda	#<my_font		; Address of font data.
		sta	<_si + 0
		lda	#>my_font
		sta	<_si + 1
		lda	#^my_font
		sta	<_si_bank

		stz	<_di + 0		; Destination VRAM address.
		lda	#>(CHR_0x10 * 16)
		sta	<_di + 1

		lda	#$FF			; Put font in colors 4-7,
		sta	<_al			; so bitplane 2 = $FF and
		stz	<_ah			; bitplane 3 = $00.

		lda	#16 + 96		; 16 graphics + 96 ASCII.
		sta	<_bl

		call	dropfnt8x8_vdc		; Upload font to VRAM.

		; Upload the palette data to the VCE.

		lda	#<cpc464_colors		; Tell BIOS to upload the
		sta	bgc_ptr + 0		; background palette which
		lda	#>cpc464_colors		; we just mapped into MPR3
		sta	bgc_ptr + 1		; with the font.
		lda	#1
		sta	bgc_len
		stz	sprc_ptr + 0
		stz	sprc_ptr + 1
		stz	sprc_len
		lda	#2
		sta	color_cmd
		system	ex_colorcmd		; Do immediately!

		; Display the classic "hello, world" on the screen.

		lda	#<(13*64 + 10)		; Destination VRAM address.
		sta	<_di + 0
		lda	#>(13*64 + 10)
		sta	<_di + 1
		call	vdc_di_to_mawr

		cly				; Display the message.
		bsr	.print_message

		; Turn on the BG & SPR layers, then wait for a soft-reset.

		system	ex_dspon		; Enable background.

.hang:		system	ex_vsync		; Wait for vsync.
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

cpc464_colors:	dw	$0000,$0001,$01B2,$01B2,$0002,$004C,$0169,$01B2
		dw	$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000



; ***************************************************************************
; ***************************************************************************
;
; It's the font data, nothing exciting to see here!
;

my_font:	incbin	"font8x8-ascii-bold-short.dat"
