; ***************************************************************************
; ***************************************************************************
;
; hugerom.asm
;
; A simple (mostly empty) test of using the Street Fighter 2 mapper.
;
; Copyright John Brandwood 2024.
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

		;
		; Create a 2MB (homebrew) or 2.5MB (standard) sized HuCARD?
		;

CREATE_2MB_ROM	=	0

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

		; Select the overlay containing saz_vdc
		;
		; Write to the SF2 mapper at offset $1FF0-$1FF3 in MRP7.

		stz	$FFF0 + overlay( saz_vdc )

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

		; Another way to select an overlay using an equate.
		;
		; Write to the SF2 mapper at offset $1FF0-$1FF3 in MRP7.

		stz	$FFF0 + overlay( SAZ_VCE_DATA )

		; Decompress the color palette data.

		lda.l	#PALETTE_BUFFER		; Destination buffer for the
		sta.l	<_di			; 256 colors.
		lda.h	#PALETTE_BUFFER
		sta.h	<_di

		lda.l	#saz_vce		; Address of font data.
		sta.l	<_bp
		lda.h	#saz_vce
		sta.h	<_bp
		ldy	#^SAZ_VCE_DATA

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

		; Select the overlay containing saz_satb0
		;
		; Write to the SF2 mapper at offset $1FF0-$1FF3 in MRP7.

		stz	$FFF0 + overlay( saz_satb0 )

		lda.l	#$0834			; Upload hand position sprites
		sta.l	<_di			; to the SATB in VRAM.
		lda.h	#$0834
		sta.h	<_di
		call	vdc_di_to_mawr
		lda	#bank( saz_satb0 )	; Put saz_satb0 in MPR3.
		tam3
		tia	saz_satb0, VDC_DL, 32	; Frame 1 of 2.

		ldy	#30			; Wait for 30 frames.
		call	wait_nvsync

		; Select the overlay containing saz_satb1 using an equate.
		;
		; Write to the SF2 mapper at offset $1FF0-$1FF3 in MRP7.

;		stz	$FFF0 + ((linear( SAZ_SATB_DATA ) >> 19) - 1)

		stz	$FFF0 + overlay( SAZ_SATB_DATA )

		lda.l	#$0834			; Upload hand position sprites
		sta.l	<_di			; to the SATB in VRAM.
		lda.h	#$0834
		sta.h	<_di
		call	vdc_di_to_mawr
		lda	#^SAZ_SATB_DATA		; Put saz_satb1 in MPR3.
		tam3
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
; The SATB entries for the two different hand positions.
;

		; Put this in the last bank of a 1MByte ROM.
		.bank	$7F

		; The earlier code maps this data into MPR3.
		.page	3

saz_satb0:	dw	$00A8,$0050,$01A8,$1080
		dw	$0088,$0040,$0182,$0080
		dw	$00A8,$0030,$0188,$1180
		dw	$0098,$0030,$0184,$0180

saz_satb1:	dw	$00A8,$0050,$01B8,$1080
		dw	$00A8,$0030,$0198,$1180
		dw	$0088,$0030,$0190,$1180
		dw	$0000,$0000,$0000,$0000

		; This is how to set the bank and overlay in an equate.
		;
		; Overlay .... none (banks $00-$7F)
		; Bank ........ $7F (the last bank of $40..$7F)
		; Address ... $6000 (the bank is mapped into MPR3)

SAZ_SATB_DATA	=	$7F:6000



; ***************************************************************************
; ***************************************************************************
;
; The compressed graphics for the error screen.
;

		; Put this at 1MB, larger than normal PCE ROMs.
		.bank	$080

		; Make sure the address > $6000 for set_bp_to_mpr
		.page	3

saz_vdc:	incbin	"saz_vdc.zx0.256"

		; This is how to set the bank and overlay in an equate.
		;
		; Overlay ...... $1 (banks $80-$BF)
		; Bank ........ $7F (the last bank of $40..$7F)
		; Address ... $6000 (the bank is mapped into MPR3)

SAZ_VDC_DATA	=	$1:7F:6000



; ***************************************************************************
; ***************************************************************************
;
; The compressed color palettes for the error screen.
;

	.if	CREATE_2MB_ROM

		; Put this in the last bank of a 2MByte ROM.
		.bank	$FF

		; Make sure the address > $6000 for set_bp_to_mpr
		.page	3

saz_vce:	incbin	"saz_vce.zx0"

		; This is how to set the bank and overlay in an equate.
		;
		; Overlay ...... $2 (banks $C0-$FF)
		; Bank ........ $7F (the last bank of $40..$7F)
		; Address ... $6000 (the bank is mapped into MPR3)

SAZ_VCE_DATA	=	$2:7F:6000

	.else

		; Put this in the last bank of a 2.5MByte ROM.
		.bank	$13F

		; Make sure the address > $6000 for set_bp_to_mpr
		.page	3

saz_vce:	incbin	"saz_vce.zx0"

		; This is how to set the bank and overlay in an equate.
		;
		; Overlay ...... $2 (banks $C0-$FF)
		; Bank ........ $7F (the last bank of $40..$7F)
		; Address ... $6000 (the bank is mapped into MPR3)

SAZ_VCE_DATA	=	$3:7F:6000

	.endif	CREATE_2MB_ROM
