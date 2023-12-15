; ***************************************************************************
; ***************************************************************************
;
; gulliver.asm
;
; Example of HuVIDEO playback using Hudson's GulliverBoy CD.
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
; This is an example of a playing back HuVIDEO, which uses a 240KB RAM buffer
; in its SCD configuration, and so the playback code must fit within 2 banks.
;
; If it is running as a HuCARD, with Turbo Everdrive support, the setup is a
; a bit simpler because of the extra RAM available on the TED2.
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
; ===========================================
; Gulliver CD
; ===========================================
; 00 00 02 03 00 3A 00 3A 00 01 02 03 04
; 47 75 6C 6C 69 76 65 72 20 20 20 20 20 20 20 20
; 48 75 64 73 6F 6E
;
; LBA $0002D858
; HuVideo
; E0 03 C0 00 70 00 10 00 06 -> 192x112, 16-palette, 10fps
;
; ===========================================
; Yuna2 game CD (both FAAT and FABT versions)
; ===========================================
; 00 00 02 0C 00 40 00 40 00 01 02 03 04
; 8B E2 89 CD 82 A8 8F EC 97 6C 93 60 90 E0 20 20
; 83 86 83 69 87 55
;
; LBA $0B83F000
; HuVideo
; C6 02 A0 00 80 00 10 00 06 -> 160x128, 16-palette, 10fps
;
; ===========================================
; Yuna HuVIUDEO CD
; ===========================================
; 00 00 02 0C 00 40 06 40 00 01 02 03 04
; 8B E2 89 CD 82 A8 8F EC 97 6C 93 60 90 E0 20 20
; 83 86 83 69 87 55
;
; LBA $0B83F000
; HuVideo
; C6 02 A0 00 80 00 10 00 06 -> 160x128, 16-palette, 10fps
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

BAT_NULL	=	CHR_0x20 + (0 << 12)	; Default BAT tile & palette.

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

;SUPPORT_HUVIDEO	=	1	; Include the HuVIDEO playback functions.
SUPPORT_TIMING	=	1	; Include the HuVIDEO timing information.

		include	"movie.asm"		; HuVIDEO movie playback.

	.if	CDROM
		include	"acd.asm"		; ArcadeCard routines.
	.else
		include	"ted2.asm"		; TED2 hardware routines.
	.endif



; ***************************************************************************
; ***************************************************************************
;
; Constants and Variables.
;

		.zp

which_video:	ds	1

		.bss
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

		; Upload an inverted 8x8 font to VRAM in colors 0 and 1.

		stz	<_di + 0		; Destination VRAM address.
		lda	#>(CHR_0x10 * 16)
		sta	<_di + 1

		lda	#16 + 96		; 16 box graphics + 96 ASCII.
		sta	<_bl

		lda	#<my_font		; Address of font data.
		sta	<_bp + 0
		lda	#>my_font
		sta	<_bp + 1
		ldy	#^my_font

		call	transfnt8x8_vdc		; Upload font to VRAM.

		; Write a 32x64 color 15 sprite @ VRAM $1000.

		clx
		st1	#$FF
.spr_loop:	st2	#$FF
		st2	#$FF
		dex
		bne	.spr_loop

		; Write the SAT data for the underlay sprites.

		stz	<_di + 0		; Destination VRAM address.
		lda	#>SAT_ADDR
		sta	<_di + 1
		jsr	vdc_di_to_mawr

		tia	.spr_sat, VDC_DL, 8 * 8

		; Clear screen in palette 15.

		lda.l	#CHR_0x20 + (15 << 12)
		sta.l	<_ax
		lda.h	#CHR_0x20 + (15 << 12)
		sta.h	<_ax
		lda	#>BAT_SIZE		; Size of BAT in words.
		sta	<_bl
		call	clear_bat_vdc

		; Upload the palette data to the VCE.

		lda	#15			; Start at palette 15 (BG).
		sta	<_al
		lda	#2			; Copy 2 palette of 16 colors.
		sta	<_ah
		lda	#<bkg_palettes		; Set the ptr to the palette
		sta.l	<_bp			; data.
		lda	#>bkg_palettes
		sta.h	<_bp
		ldy	#^bkg_palettes
		call	load_palettes		; Add to the palette queue.

;		call	xfer_palettes		; Transfer queue to VCE now.

		; Turn on the BG & SPR layers, then wait for a soft-reset.

		setvec	vsync_hook, .vblank_irq	; Enable vsync_hook.
		smb4	<irq_vec

		call	set_dspon		; Enable background and sprites.

		call	wait_vsync		; Wait for vsync.

		; Display the title message.
		;
		; These are drawn twice so that they show up on both
		; of the screens that HuVIDEO flips between.

		PRINTF	"\e<\ePF"

	.if	!CDROM
		call	unlock_ted2
		beq	.sign_on
		PRINTF	"\eX2\eY22Turbo Everdrive v2 required!"
.hang:		bra	.hang
	.endif

.sign_on:	PRINTF	"\eX3\eY22GulliverBoy HuVIDEO Player"

		PRINTF	"\eX2\eY23Buffer used when refill: $xx"
		PRINTF	"\eX2\eY24Buffer used refill done: $xx"
		PRINTF	"\eX2\eY25Buffer refill time  1st: $xx"
		PRINTF	"\eX2\eY26Buffer refill time rest: $xx"

		PRINTF	"\eX35\eY22GulliverBoy HuVIDEO Player"

		PRINTF	"\eX34\eY23Buffer used when refill: $xx"
		PRINTF	"\eX34\eY24Buffer used refill done: $xx"
		PRINTF	"\eX34\eY25Buffer refill time  1st: $xx"
		PRINTF	"\eX34\eY26Buffer refill time rest: $xx"

		;

.init_cd:	ldy	#30			; Wait for CD drive to get
		call	wait_nvsync		; stable after power-on.

		call	cdr_init_disc		; Without messages!
		beq	!+			; Are we ready?
		jmp	.main_loop

!:		lda	#$25			; Default to NOT Gulliver CD!

		ldx	tnomax			; Gulliver CD has 10 BCD tracks.
		cpx	#$10
		bne	!+
		ldx	outmin			; Gulliver CD has 73 BCD minutes.
		cpx	#$73
		bne	!+
		ldx	outsec			; Gulliver CD has 23 BCD seconds.
		cpx	#$23
		bne	!+
		ldx	outfrm			; Gulliver CD has 53 BCD frames.
		cpx	#$53
		bne	!+

;		lda	#$24			; Star Boy Movie.
		lda	#$22			; Title Movie

		; Play the "GulliverBoy" title HuVIDEO.

!:		sta	<which_video		; Which video to play?
		cmp	#$25			; Skip HuVIDEO if this is not
		bcs	!+			; the GulliverBoy CD.

		sta	<_al
		call	play_huvideo
		beq	!+			; Are we ready?
		jmp	.main_loop

		; Run another simple test to determine the CD xfer rate.

!:		PRINTF	"\eX2\eY23System Card seek time:   $xx"
		PRINTF	"\eX2\eY24System Card read time:   $xx"

		PRINTF	"\eX34\eY23System Card seek time:   $xx"
		PRINTF	"\eX34\eY24System Card read time:   $xx"

		cly
.test_loop:	phy
		clx
.read_block:	phx

		lda	#1			; Seek to the start of the data
		sta	<_al			; to read.
		lda	#BUFFER_1ST_BANK	; This will be a varying time to
		sta	<_bl			; complete, but will always end
		lda	#3			; just after the sector has been
		sta	<_dh			; read.

		lda	<which_video		; Which video to play?
		asl	a
		asl	a
		tax
		lda	gulliver_lba + 0, x
		sta	<_dl
		lda	gulliver_lba + 1, x
		sta	<_ch
		lda	gulliver_lba + 2, x
		sta	<_cl
		stz	irq_cnt
	.if	CDROM
		system	cd_seek			; Call System Card function.
	.else
		call	cdr_read_bnk		; Call library function.
	.endif
		lda	irq_cnt
		sta	<huv_buffer_fill	; Remember time for display.

		plx				; Alternately read either 90KB
		phx				; or 240KB as-soon-as-possible
		lda	.size, x		; after the seek. This gives a
		sta	<_al			; consistent start condition.
		lda	#BUFFER_1ST_BANK	; The difference between the
		sta	<_bl			; time taken for the reads is
		lda	#3			; how long it takes to read
		sta	<_dh			; 150KB.

		lda	<which_video		; Which video to play?
		asl	a
		asl	a
		tax
		clc
		lda	gulliver_lba + 0, x
		adc	#1
		sta	<_dl
		lda	gulliver_lba + 1, x
		adc	#0
		sta	<_ch
		lda	gulliver_lba + 2, x
		adc	#0
		sta	<_cl
		stz	irq_cnt
	.if	CDROM
		system	cd_read			; Call System Card function.
	.else
		call	cdr_read_bnk		; Call library function.
	.endif
		lda	irq_cnt
		sta	<huv_buffer_full	; Remember time for display.

		jsr	huv_show_stats		; Display the results.

		ldy	#120			; Wait for a bit to see the
		call	wait_nvsync		; result and not stress the CD.

		plx				; Repeat the loop and get the
		inx				; next result.
		cpx	#2
		bne	.read_block

		ply				; Repeat the pair of tests.
		iny
		cpy	#5
		bne	.test_loop

		call	cdr_reset		; Stop the CD.

		; Wait for user to reboot.

	.if	!CDROM
.main_loop:	call	wait_vsync_usb		; Wait for VBLANK, or xfer.
	.else
.main_loop:	call	wait_vsync		; Wait for VBLANK.
	.endif

		bra	.main_loop		; Wait for user to reboot.

.size:		db	90 / 2, 240 / 2

		; All done for this frame, keep on looping!


		; Simple VBLANK IRQ handler for the CORE library.

.vblank_irq:	cli				; Allow RCR and TIMER interrupts.
		jmp	xfer_palettes		; Transfer queue to VCE now.

		; Low priority sprites to go underneath the background to
		; provide a color for the transparent font.

.spr_sat:	dw	64 + 224 - 48		; Sprite in color 15.
		dw	32 + $00
		dw	$1000 >> 5
		dw	%0011000100000000	; CGY=3, CGX=1 (32x64).

		dw	64 + 224 - 48		; Sprite in color 15.
		dw	32 + $20
		dw	$1000 >> 5
		dw	%0011000100000000	; CGY=3, CGX=1 (32x64).

		dw	64 + 224 - 48		; Sprite in color 15.
		dw	32 + $40
		dw	$1000 >> 5
		dw	%0011000100000000	; CGY=3, CGX=1 (32x64).

		dw	64 + 224 - 48		; Sprite in color 15.
		dw	32 + $60
		dw	$1000 >> 5
		dw	%0011000100000000	; CGY=3, CGX=1 (32x64).

		dw	64 + 224 - 48		; Sprite in color 15.
		dw	32 + $80
		dw	$1000 >> 5
		dw	%0011000100000000	; CGY=3, CGX=1 (32x64).

		dw	64 + 224 - 48		; Sprite in color 15.
		dw	32 + $A0
		dw	$1000 >> 5
		dw	%0011000100000000	; CGY=3, CGX=1 (32x64).

		dw	64 + 224 - 48		; Sprite in color 15.
		dw	32 + $C0
		dw	$1000 >> 5
		dw	%0011000100000000	; CGY=3, CGX=1 (32x64).

		dw	64 + 224 - 48		; Sprite in color 15.
		dw	32 + $E0
		dw	$1000 >> 5
		dw	%0011000100000000	; CGY=3, CGX=1 (32x64).



; ***************************************************************************
; ***************************************************************************
;
; transfnt8x8_vdc - Upload an 8x8 transparent font to the PCE VDC.
;
; Args: _bp, Y = _farptr to font data (maps to MPR3).
; Args: _di = ptr to output address in VRAM.
; Args: _bl = # of font glyphs to upload.
;
; N.B. The font is 1bpp, and the drop-shadow is generated by the CPU.
;
; When _al == $00 $FF $00 $FF
; When _ah == $00 $00 $FF $FF
;
; BKG pixels    0   4   8  12
; Shadow pixels 1   5   9  13
; Font pixels   2   6  10  14
;

transfnt8x8_vdc	.proc

		clx				; Offset to PCE VDC.

		tma3				; Preserve MPR3.
		pha

		jsr	set_bp_to_mpr3		; Map memory block to MPR3.
		jsr	set_di_to_mawr		; Map _di to VRAM.

		; Generate inverted font in color 0, background in color 1.

.tile_loop:	cly

.plane01_loop:	lda	[_bp], y
		eor	#$FF
		sta	VDC_DL, x
		stz	VDC_DH, x
		iny
		cpy	#8
		bne	.plane01_loop
		stz	VDC_DL, x
.plane23_loop:	stz	VDC_DH, x
		dey
		bne	.plane23_loop

		clc				; Increment ptr to font.
		lda	#8
		adc.l	<_bp
		sta.l	<_bp
		bcc	.next_tile
		inc.h	<_bp

.next_tile:	dec	<_bl			; Upload next glyph.
		bne	.tile_loop

.exit:		pla				; Restore MPR3.
		tam3

		leave				; All done, phew!

		.endp



; ***************************************************************************
; ***************************************************************************
;
; GulliverBoy Movie Directory - LBA of "HuVideo" sector.
;
; tnomin = 1
; tnomax = 10
; outmin = 73
; outsec = 23
; outfrm = 53
;
; track	 1 MSF = 00,02,00 SUBQ = $00
; track	 2 MSF = 01,20,21 SUBQ = $04 -> recbase $0016EF VERIFIED!
; track 10 MSF = 67,01,28 SUBQ = $04 -> recbase $04998D VERIFIED!
;

gulliver_lba:	dd	$015002			; $00 Game Start
		dd	$018660			; $01
		dd	$018FA0			; $02
		dd	$019A90			; $03
		dd	$019CCE			; $04
		dd	$01A34A			; $05
		dd	$01A6DE			; $06
		dd	$01B27C			; $07
		dd	$01B736			; $08
		dd	$01CF1C			; $09
		dd	$01F038			; $0A
		dd	$01FC2A			; $0B
		dd	$01FFFA			; $0C
		dd	$020D12			; $0D
		dd	$021196			; $0E
		dd	$02149A			; $0F
		dd	$021840			; $10
		dd	$021C7C			; $11
		dd	$02288C			; $12
		dd	$022BEA			; $13
		dd	$023476			; $14
		dd	$0239F0			; $15
		dd	$023F82			; $16
		dd	$02448A			; $17
		dd	$0258E6			; $18
		dd	$025F20			; $19
		dd	$026D9A			; $1A
		dd	$0270CE			; $1B
		dd	$027750			; $1C
		dd	$027CDC			; $1D
		dd	$028D54			; $1E
		dd	$029586			; $1F
		dd	$02A07C			; $20
		dd	$02C23A			; $21 Epilog
		dd	$02D858			; $22 Title Movie
		dd	$02EF9C			; $23 Baddie Test?
		dd	$02F4B0			; $24 Star Boy

		; Sector # for read-speed test if not a GulliverBoy CD.
		;
		; This is the sector # of the Sherlock Holmes Consulting
		; Detective (USA) Intro movie, used as a quick-and-dirty
		; test that ICOM didn't do anything clever with the data
		; encoding on the CD itself in order to get that game's
		; 120KB/s unbuffered playback rate.

		dd	$000F26 - 1		; $25 LBA for 2nd video frame



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

		align	2

none		=	$000

bkg_palettes:	defpal	$002,$000,none,none,none,none,none,none
		defpal	none,none,none,none,none,none,none,none

spr_palettes:	defpal	none,none,none,none,none,none,none,none
		defpal	none,none,none,none,none,none,none,$662



; ***************************************************************************
; ***************************************************************************
;
; It's the font data, nothing exciting to see here!
;

my_font:	incbin	"font8x8-ascii-bold-short.dat"
