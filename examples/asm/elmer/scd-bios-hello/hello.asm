; ***************************************************************************
; ***************************************************************************
;
; hello.asm
;
; "hello, world" example of using the System Card BIOS functions.
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
; System Card BIOS to provide interrupt-handling and joypad reading.
;
; It is running on a SuperCD System, so the overlay is loaded from the ISO
; into banks $68-$87 (256KB max) by the customized "IPL-SCD" loader chosen
; on the "ISOlink" command-line.
;
; The PC Engine's memory map is set to ...
;
;   MPR0 = bank $FF : PCE hardware
;   MPR1 = bank $F8 : PCE RAM with Stack & ZP
;   MPR2 = bank $68 : SCD RAM
;   MPR3 = bank $69 : SCD RAM
;   MPR4 = bank $6A : SCD RAM
;   MPR5 = bank $6B : SCD RAM
;   MPR6 = bank $6C : SCD RAM
;   MPR7 = bank $68 : System Card's bank $00
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
		; Include the equates for the basic PCE hardware.
		;

		.nolist

		include "pceas.inc"
		include "pcengine.inc"

		.list
		.mlist

		;
		; Initialize the program sections/groups.
		;

		.zp				; Start at the beginning
		.org	$2000			; of zero-page.

		.bss				; Start main RAM after the
		.org	ramend			; System Card's variables.

		.code				; The CD-ROM IPL options have
		.org	$4000			; been set to map the program
		jmp	bios_main		; into MPR2 and jump to $4000.

		;
		; A lot of the library functions are written as procedures
		; with ".proc/.endp", so we need to decide where to put the
		; call-trampolines so that the procedures can be paged into
		; memory when needed.
		;
		; If you're not using the ".proc/.endp" functionality then
		; this doesn't need to be set.
		;

	.if	USING_NEWPROC			; If the ".proc" trampolines
__trampolinebnk	=	0			; are in MPR2, tell PCEAS to
__trampolineptr	=	$5FFF			; put them right at the end.
	.endif



; ***************************************************************************
; ***************************************************************************
;
; bios_main - This is executed after "CORE(not TM)" library initialization.
;
; This is the first code assembled after the library includes, so we're still
; in the CORE_BANK, usually ".bank 0"; and because this is assembled with the
; default configuration from "include/core-config.inc", which sets the option
; "USING_MPR7", then we're running in MPR7 ($E000-$FFFF).
;

bios_main:	; Turn the display off and initialize the screen mode.

		jsr	ex_dspoff		; Disable background/sprites.

		jsr	ex_vsync		; Wait for vsync.

		cla				; Set up a 256x224 video
		ldx	#256 / 8		; mode.
		ldy	#224 / 8
		jsr	ex_scrmod

		lda	#VDC_MWR_64x32 >> 4	; Set up 64x32 virtual
		jsr	ex_scrsiz		; screen.

		; Clear VRAM, including the BAT.

		lda	#<CHR_0x20		; Use tile # of ASCII ' '
		sta	<_al			; to fill the BAT, using
		lda	#>CHR_0x20		; palette 0.
		sta	<_ah

		lda	#>BAT_SIZE		; Size of BAT in words.
		sta	<_bl
		jsr	clear_bat

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
		sta	color_cmd		; Upload next VBLANK.

		; Display the classic "hello, world" on the screen.

		lda	#<(13*64 + 10)		; Destination VRAM address.
		ldx	#>(13*64 + 10)
		jsr	ex_setwrt

		cly				; Display the message.
		bsr	.print_message

		; Turn on the BG & SPR layers, then wait for a soft-reset.

		jsr	ex_dspon		; Enable background.

.hang:		jsr	ex_vsync		; Wait for vsync.
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

.message_text:	db	"HELLO, WORLD",0



; ***************************************************************************
; ***************************************************************************
;
; clear_bat - Clear the BAT in the PCE VDC.
;
; Args: _ax = word value to write to the BAT.
; Args: _bl = hi-byte of size of BAT (# of words).
;

clear_bat:	cla				; Set VDC or SGX destination
		clx				; address.
		jsr	ex_setwrt

		lda	<_bl			; Xvert hi-byte of # words
		lsr	a			; in screen to loop count.

		cly
.bat_loop:	pha
		lda	<_ax + 0
		sta	VDC_DL
		lda	<_ax + 1
.bat_pair:	sta	VDC_DH
		sta	VDC_DH
		dey
		bne	.bat_pair
		pla
		dec	a
		bne	.bat_loop

		rts



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
;  $0 = dark blue background
;  $F = yellow font
;

cpc464_colors:	dw	$0002,$0000,$0000,$0000,$0000,$0000,$0000,$0000
		dw	$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0169
