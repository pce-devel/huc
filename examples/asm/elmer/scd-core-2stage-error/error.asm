; ***************************************************************************
; ***************************************************************************
;
; error.asm
;
; "Not running on a Super CD-ROM" example error overlay using "CORE(not TM)".
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
; This is a simple "Not running on a Super CD-ROM" error overlay program that
; is executed when either IPL-SCD or the "CORE(not TM)" library Stage1 loader
; detects that the CD has been booted with a System Card 2.0 or lower HuCARD.
;
; It is running on an old CD-ROM2 System, so the overlay is loaded from the
; ISO into banks $80-$87 (64KB max).
;
; The PC Engine's memory map is set to ...
;
;   MPR0 = bank $FF : PCE hardware
;   MPR1 = bank $F8 : PCE RAM with Stack & ZP & "CORE(not TM)" kernel
;   MPR2 = bank $80 : SCD RAM
;   MPR3 = bank $81 : SCD RAM
;   MPR4 = bank $82 : SCD RAM
;   MPR5 = bank $83 : SCD RAM
;   MPR6 = bank $84 : SCD RAM
;   MPR7 = bank $80 : SCD RAM or System Card's bank $00
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
		; This is an overlay program that relies on a Stage1 loader
		; to install the library kernel, so don't build the kernel.
		;
		; You would normally just set this in your project's local
		; "core-config.inc", but this shows another way to achieve
		; the same result.
		;

USING_STAGE1	=	1

		;
		; Choose a smaller ZX0 window than the default 2KB in order
		; to test the optimized decompressor for a 256-byte window.
		;

ZX0_PREFER_SIZE	=	0			; Smaller size or 10% faster.
ZX0_WINBUF	=	($3F00)			; Choose a 256 byte window in
ZX0_WINMSK	=	($0100 - 1)		; RAM, located at $3F00.

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
		include "unpack-zx0.asm"	; Decompressor for ZX0.



; ***************************************************************************
; ***************************************************************************
;
; Put a decompression-buffer at the end of the RAM bank, using the new
; syntax that allows for spcifying a bank number.
;

PALETTE_BUFFER	=	$F8:3C00		; Room for 512 colors.



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

		call	init_256x224		; Initialize VDC & VRAM.

		; Decompress the graphics data.
		;
		; The ZX0 library defaults to using a 2KB decompression
		; window at $3800. That's perfect for this example.

		stz.l	<_di			; VDC destination address.
		stz.h	<_di

		lda.l	#saz_vdc		; Address of VDC graphics.
		sta.l	<_bp
		lda.h	#saz_vdc
		sta.h	<_bp
		ldy	#^saz_vdc

		call	zx0_to_vdc		; Decompress the graphics.

		; Decompress the color palette data.

		lda.l	#PALETTE_BUFFER		; Destination buffer for the
		sta.l	<_di			; 256 colors.
		lda.h	#PALETTE_BUFFER
		sta.h	<_di

		lda.l	#saz_vce		; Address of font data.
		sta.l	<_bp
		lda.h	#saz_vce
		sta.h	<_bp
		ldy	#^saz_vce

		call	zx0_to_ram		; Decompress the palettes.

		; Upload the palette data to the VCE.

		stz	<_al			; Start at palette 0 (BG).
		lda	#32			; Copy 32 palette of 16 colors.
		sta	<_ah
		lda	#<PALETTE_BUFFER	; Set the ptr to the palette
		sta.l	<_bp			; data.
		lda	#>PALETTE_BUFFER
		sta.h	<_bp
		cly
		call	load_palettes		; Add to the palette queue.

		call	xfer_palettes		; Transfer queue to VCE now.

		; Turn on the BG & SPR layers, then wait for a soft-reset.

		call	set_dspon		; Enable the background.

.animate:	ldy	#30			; Wait for 30 frames.
		call	wait_nvsync

		lda.l	#$0834			; Upload hand position sprites
		sta.l	<_di			; to the SATB in VRAM.
		lda.h	#$0834
		sta.h	<_di
		call	vdc_di_to_mawr
		tia	saz_satb0, VDC_DL, 32	; Frame 1 of 2.

		ldy	#30			; Wait for 30 frames.
		call	wait_nvsync

		lda.l	#$0834			; Upload hand position sprites
		sta.l	<_di			; to the SATB in VRAM.
		lda.h	#$0834
		sta.h	<_di
		call	vdc_di_to_mawr
		tia	saz_satb1, VDC_DL, 32	; Frame 2 of 2.

		bra	.animate		; Wait for user to reboot.



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
; The compressed graphics for the error screen.
;

saz_vdc:	incbin	"saz_vdc.zx0.256"



; ***************************************************************************
; ***************************************************************************
;
; The compressed color palettes for the error screen.
;

saz_vce:	incbin	"saz_vce.zx0"



; ***************************************************************************
; ***************************************************************************
;
; The SATB entries for the two different hand positions.
;

saz_satb0:	dw	$00A8,$0050,$01A8,$1080
		dw	$0088,$0040,$0182,$0080
		dw	$00A8,$0030,$0188,$1180
		dw	$0098,$0030,$0184,$0180

saz_satb1:	dw	$00A8,$0050,$01B8,$1080
		dw	$00A8,$0030,$0198,$1180
		dw	$0088,$0030,$0190,$1180
		dw	$0000,$0000,$0000,$0000
