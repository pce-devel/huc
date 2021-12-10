; ***************************************************************************
; ***************************************************************************
;
; stage2.asm
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
; This CD-ROM overlay does not include the "CORE(not TM)" kernel code, since
; it is run after the core-stage1 overlay has already installed the kernel.
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


		; This is an overlay program that relies on a Stage1 loader
		; to install the library kernel, so don't build the kernel.
		;
		; You would normally just set this in your project's local
		; "core-config.inc", but this shows another way to achieve
		; the same result.

USING_STAGE1	=	1

		; Include the library, reading the project's configuration
		; settings from the local "core-config.inc", if it exists.

		include "core.inc"		; Basic includes.

		.list
		.mlist

		include	"common.asm"		; Common helpers.
		include	"vdc.asm"		; Useful VDC routines.
		include	"font.asm"		; Useful font routines.
		include "unpack-zx0.asm"	; Decompressor for ZX0.



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
; Put a decompression-buffer at the end of the RAM bank.
;

FONT_BUFFER	=	$3C00			; Temporary for decompresion.



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

		; Decompress the font data.

		lda.l	#my_font		; Address of font data.
		sta.l	<__si
		lda.h	#my_font
		sta.h	<__si
		lda	#^my_font
		sta	<__si_bank

		lda.l	#FONT_BUFFER		; Destination buffer.
		sta.l	<__di
		lda.h	#FONT_BUFFER
		sta.h	<__di

		call	zx0_to_ram		; Decompress the font.

		; Upload the font to VRAM.

		lda.l	#FONT_BUFFER		; Address of font data.
		sta.l	<__si
		lda.h	#FONT_BUFFER
		sta.h	<__si
		lda	#$F8			; RAM bank.
		sta	<__si_bank

		stz.l	<__di			; Destination VRAM address.
		lda	#>(CHR_0x20 * 16)
		sta.h	<__di

		lda	#$FF			; Put font in colors 4-7,
		sta	<__al			; so bitplane 2 = $FF and
		stz	<__ah			; bitplane 3 = $00.

		lda	#96			; 96 ASCII.
		sta	<__bl

		call	dropfnt8x8_vdc		; Upload font to VRAM.

		; Upload the palette data to the VCE.

		lda.l	#cpc464_colors		; Set the ptr to the palette
		sta.l	<__si			; data.
		lda.h	#cpc464_colors
		sta.h	<__si
		lda	#^cpc464_colors
		sta	<__si_bank
		stz	<__al			; Start at palette 0 (BG).
		lda	#1			; Copy 1 palette of 16 colors.
		sta	<__ah
		call	load_palettes		; Add to the palette queue.

		call	xfer_palettes		; Transfer queue to VCE now.

		; Display the classic "hello, world" on the screen.

		lda.l	#(13*64 + 10)		; Destination VRAM address.
		sta.l	<__di
		lda.h	#(13*64 + 10)
		sta.h	<__di
		call	__di_to_vdc

		cly				; Display the message.
		bsr	.print_message

		; Turn on the BG & SPR layers, then wait for a soft-reset.

		call	set_dspon		; Enable the background.

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
; It's still the font data, but compressed in this example!
;

my_font:	incbin	"font8x8-ascii-exos.zx0"
