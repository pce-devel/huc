; ***************************************************************************
; ***************************************************************************
;
; ipl-scd.asm
;
; A version of Hudson's IPL for the PC Engine SuperCD that is customized
; to load homebrew assembly-language CD games that are built using ISOlink.
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
; NOTE: This is not compatible with the current version of HuC!
;
; ***************************************************************************
; ***************************************************************************
;
; Hudson's IPL code allows for developers to bypass its built-in loading
; code for the game, and to provide their own loading code located just
; after the IPL Information Data Block in the game's 2nd sector.
;
; This code uses that capability, together with the directory information
; that ISOlink puts on the CD, to load the developer's game into memory
; in a way that is friendlier to modern development.
;
;
; 1) When the ISO is run on a SuperCD System, the 1st file is loaded into
;    banks $68-$87 (256KB max), and the game is executed at $4000.
;
;    The PC Engine's memory map is set to ...
;
;      MPR0 = bank $FF : PCE hardware
;      MPR1 = bank $F8 : ZP & Stack in RAM
;      MPR2 = bank $68 : SCD RAM
;      MPR3 = bank $69 : SCD RAM
;      MPR4 = bank $6A : SCD RAM
;      MPR5 = bank $6B : SCD RAM
;      MPR6 = bank $6C : SCD RAM
;      MPR7 = bank $00 : System Card
;
;
; 2) When the ISO is run on an old CD System, the CDERR file is loaded into
;    banks $80-$87 (64KB max), and the game is executed at $4000.
;
;    The PC Engine's memory map is set to ...
;
;      MPR0 = bank $FF : PCE hardware
;      MPR1 = bank $F8 : ZP & Stack in RAM
;      MPR2 = bank $80 : CD RAM
;      MPR3 = bank $81 : CD RAM
;      MPR4 = bank $82 : CD RAM
;      MPR5 = bank $83 : CD RAM
;      MPR6 = bank $84 : CD RAM
;      MPR7 = bank $00 : System Card
;
;    If there is no CDERR file on the ISO, then the following message is
;    shown instead ... "Super CD-ROM2 System required!".
;
;
; 3) If either file fails to load due to CD errors, then the following
;    message is shown ... "CD error! Unable to load game!".
;
;
; ***************************************************************************
; ***************************************************************************
;
; ISOlink CD-ROM File Directory :
;
; Recent versions of ISOlink and PCEAS store a directory of the files that
; are contained in the CD-ROM's ISO track.
;
; This directory is stored in the previously unused 515-byte space at the
; end of the second sector of the ISO, which contains the IPL's boot info.
;
; As long as the game developer loads their initial boot program at memory
; location $3800 or higher, then when the boot program starts to execute,
; memory locations $35FF-$37FF will hold a copy of the directory information.
;
; The format of the directory is shown below ...
;
;  ------$35FF: index # of CDERR file
;  $3600-$36FF: lo-byte of sector # of start of file
;  $3700-$37FF: hi-byte of sector # of start of file
;  ------$3600: # of files stored on CD-ROM
;  ------$3700: index # of 1st file beyond 128MB
;
; File 0  is the IPL		  (starts at sector 0, length 2 sectors)
; File 1  is the game's boot file (starts at sector 2, length ? sectors)
; File 2+ are the other files that are specified on the ISOlink command line
;
; There is space for up to 255 files (including the IPL), and the ISO track
; can be a maximum size of 256MB.
;
; To allow for files that start beyond the 16-bit limit of the sector
; number, the directory stores the index of the first file that starts
; beyond that boundary.
;
; ***************************************************************************
; ***************************************************************************

iso_cderr	=	$35FF			; index # of CDERR file
iso_dirlo	=	$3600			; lo-byte of file's sector #
iso_dirhi	=	$3700			; hi-byte of file's sector #
iso_count	=	iso_dirlo		; # of files stored on CD-ROM
iso_128mb	=	iso_dirhi		; index # of 1st beyond 128MB



; ***************************************************************************
; ***************************************************************************
;
; Include definitions for PC Engine assembly-language coding.
;

		include "pceas.inc"
		include "pcengine.inc"

		.list
		.mlist


; ***************************************************************************
; ***************************************************************************
;
; Set up the IPL Information Data Block to run the new loader.
;

		.code

		.bank	0
		.org	$3000

		db	0			; IPLBLK_H
		db	0			; IPLBLK_M
		db	0			; IPLBLK_L
		db	0			; IPLBLN
		dw	0			; IPLSTA (load address)
		dw	ipl_scd - $3000		; IPLJMP (execute address)
		db	0			; IPLMPR2 (bank $80)
		db	1			; IPLMPR3 (bank $81)
		db	2			; IPLMPR4 (bank $82)
		db	3			; IPLMPR5 (bank $83)
		db	4			; IPLMPR6 (bank $84)
		db	0			; OPENMODE

		db	0			; GRPBLK_H
		db	0			; GRPBLK_M
		db	0			; GRPBLK_L
		db	0			; GRPBLK_H
		db	0			; GRPBLN
		dw	0			; GRPADR (load address)

		db	0			; ADPBLK_H
		db	0			; ADPBLK_M
		db	0			; ADPBLK_L
		db	0			; ADPBLK_H
		db	0			; ADPBLN
		db	0			; ADPRATE



; ***************************************************************************
; ***************************************************************************
;
; ipl_scd - Replacement IPL file loader using ISOlink diectory information.
;

		.org	$3090			; First free memory in IPL.

ipl_scd:	jsr	ex_getver		; Get System Card version.
		cpx	#3			; Need version 3 or higher.
		bcc	.not_supercd

		jsr	ex_memopen		; Is there any SCD memory?
		bcs	.not_supercd
		sax				; Remove internal-or-HuCard
		and	#$7F			; flag bit.
		sax
		cpx	#3			; Are there at-least three
		bcc	.not_supercd		; 64KB blocks of memory?

		lda	#$68			; PCEAS code assumes this!

		tam2				; Map SCD RAM banks to
		inc	a			; replace the CD RAM banks.
		tam3
		inc	a
		tam4
		inc	a
		tam5
		inc	a
		tam6

		ldy	#1			; Load & run the 1st file.

.load_file:	lda	iso_dirlo + 1, y	; Calculate file length.
		sec
		sbc	iso_dirlo + 0, y
		sta	<_al
		stz	<_ah

		tma2				; Set first bank to load.
		sta	<_bl
		stz	<_bh

		cla				; Set file start location.
		cpy	iso_128mb
		bcc	.set_rec_h
		inc	a
.set_rec_h:	sta	<_cl
		lda	iso_dirhi + 0, y
		sta	<_ch
		lda	iso_dirlo + 0, y
		sta	<_dl

		lda	#6			; Use MPR6 for loading.
		sta	<_dh

		jsr	cd_read			; Load the file from CD.

		cmp	#0			; Was there an error?
		beq	.execute_game

		lda	#<.cderror_msg
		ldx	#>.cderror_msg

		bra	.report_error

.execute_game:	jmp	$4000			; Execute the game.

.not_supercd:	ldy	iso_cderr		; Is there an error file
		bne	.load_file		; to run?

		lda	#<.supercd_msg
		ldx	#>.supercd_msg

		; Fall back to a simple on-screen text message.

.report_error:	pha				; Preserve message pointer.
		phx

		jsr	ex_dspoff		; Disable background/sprites.

		jsr	ex_vsync		; Wait for vsync.

		jsr	clear_vram		; Initialize VRAM.

		lda	#<font_data		; Upload font data.
		sta	<_si + 0
		lda	#>font_data
		sta	<_si + 1
		jsr	upload_font8x8

		cla				; Set up a 32x28 video
		ldx	#32			; mode.
		ldy	#28
		jsr	ex_scrmod

		lda	#VDC_MWR_64x32 >> 4	; Set up 64x32 virtual
		jsr	ex_scrsiz		; screen.

		lda	#<cpc464_colors		; Tell BIOS to upload the
		sta	bgc_ptr + 0		; background palette.
		lda	#>cpc464_colors
		sta	bgc_ptr + 1
		lda	#1
		sta	bgc_len
		stz	sprc_ptr + 0
		stz	sprc_ptr + 1
		stz	sprc_len
		lda	#2
		sta	color_cmd

		plx				; Restore message pointer.
		pla
		jsr	write_string

		jsr	ex_bgon			; Enable background.

		jsr	ex_vsync		; Wait for vsync.

.hang:		bra	.hang			; Wait for user to reboot.

		; The error message is remapped into a minimal font that
		; contains only 25 characters.
		;
		; N.B. The font is also used by "core-stage1.asm"!
		;
		; @ = ' '
		; A = 'S'
		; B = 'u'
		; C = 'p'
		; D = 'e'
		; E = 'r'
		; F = 'C'
		; G = 'D'
		; H = '-'
		; I = 'R'
		; J = 'O'
		; K = 'M'
		; L = '2'
		; M = 'y'
		; N = 's'
		; O = 't'
		; P = 'm'
		; Q = 'n'
		; R = 'd'
		; S = '!'
		; T = 'o'
		; U = 'a'
		; V = 'g'
		; W = 'q'
		; X = 'i'

	.if	0

.supercd_msg:	db	2			; Screen X
		db	13			; Screen Y
		db	"ABCDE@FGHIJKL@AMNODP@QDDRDRS"
;		db	"Super CD-ROM2 System needed!"
		db	0

	.else

.supercd_msg:	db	1			; Screen X
		db	13			; Screen Y
		db	"ABCDE@FGHIJKL@AMNODP@EDWBXEDRS"
;		db	"Super CD-ROM2 System required!"
		db	0

	.endif

.cderror_msg:	db	3			; Screen X
		db	13			; Screen Y
		db	"FG@DEETES@FUQQTO@EBQ@VUPDS"
;		db	"CD error! Cannot run game!"
		db	0



; ***************************************************************************
; ***************************************************************************
;
; clear_vram - Clear all of VRAM.
;

clear_vram:	bsr	clear_screen		; Clear the BAT	 ($1000).

		ldx	#$7000 / $400		; Clear the rest ($7000).
		cly
		st1	#$00
.clr_loop:	st2	#$00
		st2	#$00
		st2	#$00
		st2	#$00
		dey
		bne	.clr_loop
		dex
		bne	.clr_loop

		rts



; ***************************************************************************
; ***************************************************************************
;
; clear_screen - Clear the BAT.
;

clear_screen:	vreg	#VDC_CR
		st2	#%00000000

		vreg	#VDC_MAWR
		stz	VDC_DL
		stz	VDC_DH
		vreg	#VDC_VWR

		ldx	#$1000 / $200
		cly
		st1	#$40			; Write "space" character
.bat_loop:	st2	#$01			; in palette 0.
		st2	#$01
		dey
		bne	.bat_loop
		dex
		bne	.bat_loop

		rts



; ***************************************************************************
; ***************************************************************************
;
; upload_font8x8 - Upload an 8x8 font with a CPU generated drop-shadow.
;
; 12 = trans
; 13 = shadow
; 14 = font
;

tmp_shadow_buf	equ	$2100			; Interleaved 16 + 1 lines.
tmp_normal_buf	equ	$2101			; Interleaved 16 lines.

upload_font8x8: ldx	#$14			; Upload solid version to
		lda	#$FF			; VRAM $1400-$156F.

		pha				; Preserve bit 2 & 3 fill.

		vreg	#VDC_MAWR		; Set VRAM destination.
		stz	VDC_DL
		stx	VDC_DH

		vreg	#VDC_VWR
		lda	#25			; Minimal font of 25 characters.
		sta	<_al

		cly

.tile_loop:	clx				; Create a drop-shadowed version
		stz	tmp_shadow_buf, x	; of the glyph.

		.if	1
.line_loop:	lda	[_si], y		; Drop-shadow on the LHS.
		sta	tmp_normal_buf, x	; Font data is RHS justified.
		asl	a
		.else
.line_loop:	lda	[_si], y		; Drop-shadow on the RHS.
		sta	tmp_normal_buf, x	; Font data is LHS justified.
		lsr	a
		.endif

		ora	[_si], y
		sta	tmp_shadow_buf+2, x
		ora	tmp_shadow_buf, x
		eor	tmp_normal_buf, x
		sta	tmp_shadow_buf, x

		iny
		bne	.next_line
		inc	<_si + 1
.next_line:	inx
		inx
		cpx	#2*8
		bne	.line_loop

.copy_tile:	tia	tmp_shadow_buf, VDC_DL, 16

		pla				; Restore bit 2 & 3 byte.

		ldx	#8
		sta	VDC_DL			; Solid uses colors 12-14.
.plane23_loop:	sta	VDC_DH			; Transparent uses 0-2.
		dex
		bne	.plane23_loop
		pha				; Preserve bit 2 & 3 byte.

		dec	<_al
		bne	.tile_loop

		pla

.exit:		rts



; ***************************************************************************
; ***************************************************************************
;
; write_string - Upload an 8x8 font with a drop-shadow generated by the CPU.
;

write_string:	sta	<_al			; String address.
		stx	<_ah

		ldy	#1			; Calculate screen address.
		lda	[_ax], y		; Get Y coordinate.
		lsr	a
		tax
		cla
		ror	a

		.if	1
		sax				; If virtual screen is 64
		lsr	a			; wide instead of 128 wide.
		sax
		ror	a
		.endif

		ora	[_ax]			; Get X coordinate.
		iny

		php				; Nasty code to set up the
		sei				; VDC read and write addr
		st0	#VDC_MARR		; without using "vreg".
		sta	VDC_DL
		stx	VDC_DH
		st0	#VDC_MAWR
		sta	VDC_DL
		stx	VDC_DH
		st0	#VDC_VWR
		lda	#VDC_VWR
		sta	<vdc_reg
		plp

.loop:		lda	[_ax], y		; Get next ASCII character.
		beq	.done
		iny

		clc				; Add on base + palette.
		adc	#<$0100
		sta	VDC_DL
		cla
		adc	#>$0100
		sta	VDC_DH
		bra	.loop

.done:		rts



; ***************************************************************************
; ***************************************************************************
;
; cpc464_colors - Palette data (a blast-from-the-past!)
;
; 12 = dark blue background
; 13 = blue shadow
; 14 = yellow font
;

cpc464_colors:	dw	$0000,$0000,$0000,$0000,$0000,$0000,$0000,$0000
		dw	$0000,$0000,$0000,$0000,$0002,$004C,$0169,$01B2



; ***************************************************************************
; ***************************************************************************
;
; Compressed font data.
;

font_data:	incbin	"../font/font8x8-stage1-exos.dat"
