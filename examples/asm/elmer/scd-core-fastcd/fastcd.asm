; ***************************************************************************
; ***************************************************************************
;
; fastcd.asm
;
; Example of using the "fast" CD loading routines.
;
; This also makes a simple TED2 HuCARD to test the CD drive.
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

		;
		; You would normally just set this in your project's local
		; "core-config.inc", but this shows another way to achieve
		; the same result.
		;

	.if	CDROM
SUPPORT_ACD	=	1	; Support the Arcade Card.
	.else
SUPPORT_TED2	=	1	; Support the Turbo Everdrive's use of bank0.
	.endif

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
		include	"tty.asm"		; Useful TTY print routines.
		include	"cdrom.asm"		; Fast CD-ROM routines.

	.if	!CDROM
		include	"ted2.asm"		; TED2 hardware routines.
	.endif



; ***************************************************************************
; ***************************************************************************
;
; Constants and Variables.
;

		.zp

		.bss

retry_count:	ds	2
ram_buffer:	ds	2048

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

		call	init_256x224		; Initialize VDC & VRAM.

		; Upload the 8x8 font to VRAM in colors 0-3, with box tiles
		; using colors 4-7.
		;
		; Thus we call dropfntbox_vdc() instead of dropfnt8x8_vdc()!

		stz	<_di + 0		; Destination VRAM address.
		lda	#>(CHR_0x10 * 16)
		sta	<_di + 1

		lda	#$FF			; Put font in colors 4-7,
		sta	<_al			; so bitplane 2 = $FF and
		stz	<_ah			; bitplane 3 = $00.

		lda	#16 + 96		; 16 box graphics + 96 ASCII.
		sta	<_bl

		lda	#<my_font		; Address of font data.
		sta	<_si + 0
		lda	#>my_font
		sta	<_si + 1
		ldy	#^my_font

		call	dropfnt8x8_vdc		; Upload font to VRAM.

		; Clear screen in palette 15.

		lda.l	#CHR_0x20 + (15 << 12)
		sta.l	<_ax
		lda.h	#CHR_0x20 + (15 << 12)
		sta.h	<_ax
		lda	#>BAT_SIZE		; Size of BAT in words.
		sta	<_bl
		call	clear_bat_vdc

		; Copy the SAT to VRAM (@$0800).

		tia	.alice_sat, VDC_DL, 2*8

		; Upload the palette data to the VCE.

		lda	#15			; Start at palette 15 (BG).
		sta	<_al
		lda	#1			; Copy 1 palette of 16 colors.
		sta	<_ah
		lda	#<cpc464_colors		; Set the ptr to the palette
		sta.l	<_si			; data.
		lda	#>cpc464_colors
		sta.h	<_si
		ldy	#^cpc464_colors
		call	load_palettes		; Add to the palette queue.

		; Turn on the BG & SPR layers, then wait for a soft-reset.

		setvec	vsync_hook, .vblank_irq	; Enable vsync_hook.
		smb4	<irq_vec

		call	set_bgon		; Enable background.

		call	wait_vsync		; Wait for vsync.

		; Display the optional PC Engine hardware that we can detect.

		PRINTF	"\e<\ePF\eXL1\eY0\n"

		;

		ldy	#30			; Wait for CD drive to get
		call	wait_nvsync		; stable after power-on.

;		call	cdr_init_disc		; Without messages!
		call	test_init_disc		; With messages!		
		bne	.main_loop		; Are we ready?

	.if	CDROM
		call	load_graphics
	.endif

		; Wait for user to reboot.

	.if	!CDROM
		call	unlock_ted2
.main_loop:	call	wait_vsync_usb		; Wait for VBLANK, or xfer.
	.else
.main_loop:	call	wait_vsync		; Wait for VBLANK.
	.endif

		bra	.main_loop		; Wait for user to reboot.

		; All done for this frame, keep on looping!

		bra	.main_loop		; Wait for user to reboot.

		; SATB entries for the two overlay sprites that make Alice.

.alice_sat:	dw	64+(119)+16		; Y
		dw	32+(173)-8		; X
		dw	$1C00>>5		; SPR VRAM
		dw	%0011000110000000	; CGY=3, CGX=1 (64x32).

		dw	64+(119)+16		; Y
		dw	32+(173+32)-8		; X
		dw	$1E00>>5		; SPR VRAM
		dw	%0011000110000000	; CGY=3, CGX=1 (64x32).

		; Simple VBLANK IRQ handler for the CORE library.

.vblank_irq:	cli				; Allow RCR and TIMER IRQ.
		jmp	xfer_palettes		; Transfer queue to VCE now.



; ***************************************************************************
; ***************************************************************************
;
; test_init_disc - Initialize the CD drive and scan the CD TOC.
;
; Returns: Y,Z-flag,N-flag = $00 or an error code.
;

test_init_disc	.proc

		lda	IO_PORT
		bpl	!+

		PRINTF	"No CD-ROM Interface Unit!\n\n"	

		ldy	#CDERR_NO_CDIFU
		jmp	.finished

!:		PRINTF	"CD-ROM Interface present\n"

		; Reset the CD-ROM drive.

		call	cdr_reset

		; Wait for the CD-ROM drive to release SCSI_BSY.

		stz.l	retry_count		; Retry 1024 times, which is
		lda.h	#1024			; *far* more than needed, if
		sta.h	retry_count		; the drive is attached.

.wait_busy:	lda.l	retry_count
		bne	!+
		dec.h	retry_count
		bpl	!+			; Check for timeout!

		PRINTF	"CD-ROM Drive not attached!\n"
		ldy	#CDERR_NO_DRIVE
		jmp	.finished

!:		dec.l	retry_count

		cly				; Is the drive busy?
		call	cdr_stat
		bne	.wait_busy

		; Wait for the CD-ROM to spin-up and become ready.

		PRINTF	"CD-ROM Drive present\n"

		stz.l	retry_count
		stz.h	retry_count

.wait_ready:	PRINTF	"\rWaiting for CD-ROM (%d seconds)", retry_count

		ldy	#1			; Is the drive ready? Calling
		call	cdr_stat		; this is what actually makes
		beq	.drive_ready		; the drive start spinning up
		cmp	#CDERR_NOT_READY	; after reset.
		bne	.error

		ldy	#60			; Wait a second and try again.
		jsr	wait_nvsync

		inc.l	retry_count
		bne	.wait_ready
		inc.h	retry_count
		bra	.wait_ready

.error:		sta	retry_count
		PRINTF	"Got error $hx!\n", retry_count
		ldy	retry_count
		bra	.finished

		; Get the CD-ROM disc's Table Of Contents info.

.drive_ready:	PRINTF	"\nGetting CD-ROM TOC information\n"

		call	cdr_contents
		bne	.finished

		PRINTF	" tnomin   = %hx\n", tnomin
		PRINTF	" tnomax   = %hx\n", tnomax
		PRINTF	" outmin   = %hx\n", outmin
		PRINTF	" outsec   = %hx\n", outsec
		PRINTF	" outfrm   = %hx\n", outfrm
		PRINTF	" recbase0 = $%0hx%0hx%0hx\n", recbase0_h, recbase0_m, recbase0_l
		PRINTF	" recbase1 = $%0hx%0hx%0hx\n", recbase1_h, recbase1_m, recbase1_l

		; Stop the CD if this is a HuCARD test-rom.

	.if	!CDROM
		PRINTF	"Executing CD_RESET to stop the CD\n"
		call	cdr_reset
	.endif

		cly				; Set return-code.

.finished:	leave

		.endp



	.if	CDROM

; ***************************************************************************
; ***************************************************************************
;
; load_graphics - Test the fast CD-ROM loading routines.
;
; Returns: Y,Z-flag,N-flag = $00 or an error code.
;

load_graphics	.proc

		; Load the graphics into SCD memory.

		PRINTF	"Loading graphics from CD-ROM\n"

		ldx	#2			; Load graphic data into VRAM.
		jsr	get_file_info
		stz	<_bl			; Destination address.
		lda.h	#$1000
		sta	<_bh
		stz	<_dh			; PCE_VDC_OFFSET for VDC.
		call	cdr_read_vram
		beq	!+
		jmp	.finished

!:		ldx	#3			; Load graphic data into RAM.
		jsr	get_file_info
		lda.l	#ram_buffer		; Destination address.
		sta	<_bl
		lda.h	#ram_buffer
		sta	<_bh
		call	cdr_read_ram
		beq	!+
		jmp	.finished

!:		ldx	#3			; Load graphic data into BNK.
		jsr	get_file_info
		lda	#_bank_base + _nb_bank	; First unused bank, reported
		tam3				; by PCEAS!
		sta	<_bl
		call	cdr_read_bnk
		beq	!+
		jmp	.finished

		; Upload the palettes for Alice.

!:		lda	#0			; Start at palette 1 (BG).
		sta	<_al
		lda	#15			; Copy 15 palettes of 16 colors.
		sta	<_ah
		lda.l	#ram_buffer + $000	; Set the ptr to the palette
		sta.l	<_si			; data.
		lda.h	#ram_buffer + $000
		sta.h	<_si
		ldy	#^ram_buffer
		call	load_palettes		; Add to the palette queue.

		lda	#16			; Start at palette 1 (BG).
		sta	<_al
		lda	#16			; Copy 16 palettes of 16 colors.
		sta	<_ah
		lda.l	#ram_buffer + $200	; Set the ptr to the palette
		sta.l	<_si			; data.
		lda.h	#ram_buffer + $200
		sta.h	<_si
		ldy	#^ram_buffer
		call	load_palettes		; Add to the palette queue.

		call	set_spron		; Enable background.

		jsr	wait_vsync

		; Write the BAT.

		stz.l	<_si
		lda.h	#$6400
		sta.h	<_si

		lda.l	#15 + (15 * 64)
		sta.l	<_di
		lda.h	#15 + (15 * 64)
		sta.h	<_di

		lda	#12
		sta	<_al

.line_loop:	jsr	vdc_di_to_mawr

		cly
.tile_loop:	lda	[_si], y
		sta	VDC_DL
		iny
		lda	[_si], y
		sta	VDC_DH
		iny
		cpy	#16 * 2
		bne	.tile_loop

		clc
		lda.l	<_si
		adc	#16 * 2
		sta.l	<_si
		bcc	!+
		inc.h	<_si

!:		clc
		lda.l	<_di
		adc	#64
		sta.l	<_di
		bcc	!+
		inc.h	<_di

!:		dec	<_al
		bne	.line_loop

		; Finally, play the voice-over.

		PRINTF	"Playing streamed ADPCM\n"

		ldx	#4			; Locate the ADPCM file to
		jsr	get_file_info		; play.
		stz	<_bl
		lda	#$0E			; 16KHz
		sta	<_dh
		call	cdr_ad_cplay

		;

		cly				; Set return-code.

.finished:	leave

		;

.update_sat:	php
		sei
		lda	<vdc_reg
		st0	#VDC_DVSSR
;		st1	#<SAT_ADDR + $000
		st2	#>SAT_ADDR + $000
		sta	VDC_AR
		plp
		rts

		.endp

	.endif	CDROM



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
; Note: DEFPAL palette data is in RGB format, 4-bits per value.
; Note: Packed palette data is in GRB format, 3-bits per value.
;
;  $4 = dark blue background
;  $5 = light blue shadow
;  $6 = yellow font
;

		align	2

none		=	$000

cpc464_colors:	defpal	$000,none,none,none,$002,$114,$662,none
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

my_font:	incbin	"font8x8-ascii-bold-short.dat"



; ***************************************************************************
; ***************************************************************************
;
; This is how "alice.ram" and "alice.vdc" were generated ...
;

	.if	0

		; alice.ram

bkg_pal:	.incpal "error-bkg-16.png"
spr_pal:	.incpal "error-spr-15.png"
bkg_bat:	.incbat "error-bkg-16.png",$1000,128,104,16,12

		; alice.vdc

bkg_chr:	.incchr "error-bkg-16.png",128,104,16,12
spr_spr:	.incspr "error-spr-15.png",173+ 0,119,2,4
		.incspr "error-spr-15.png",173+32,119,2,4
spr_end:

	.endif
