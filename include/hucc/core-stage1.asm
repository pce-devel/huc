; ***************************************************************************
; ***************************************************************************
;
; core-stage1.asm
;
; CD-ROM Stage1 loader using the "CORE(not TM)" PC Engine library code.
;
; Copyright John Brandwood 2021-2024.
;
; Distributed under the Boost Software License, Version 1.0.
; (See accompanying file LICENSE_1_0.txt or copy at
;  http://www.boost.org/LICENSE_1_0.txt)
;
; ***************************************************************************
; ***************************************************************************
;
; Using this Stage1 loader in a project provides an alternative to including
; the "CORE(not TM)" kernel code in every overlay program, which saves some
; precious memory that you could be using for other code.
;
;
; If you are using the "CORE(not TM)" library for a Super CD-ROM project,
; there are two different ways that you can build your ISO ...
;
; 1) isolink mygame.iso -boot="../include/ipl-scd.dat" -asm \
;      myoverlay1.ovl <more-files> <-cderr syscard2.ovl>
;
; 2) isolink mygame.iso -ipl="Game Name",0x4000,0x4000,0,1,2,3,4 -asm \
;      core-stage1.bin myoverlay1.ovl <more-files> <-cderr syscard2.ovl>
;
;
; If you are using the "CORE(not TM)" library for an old CD-ROM project,
; there are also two different ways that you can build your ISO ...
;
; 1) isolink mygame.iso -ipl="Game Name",0x4000,0x4000,0,1,2,3,4 -asm \
;      myoverlay1.ovl <more-files>
;
; 2) isolink mygame.iso -ipl="Game Name",0x4000,0x4000,0,1,2,3,4 -asm \
;      core-stage1.bin myoverlay1.ovl <more-files>
;
; ***************************************************************************
; ***************************************************************************

		; This is the Stage1 loader, so definitely build the kernel!

USING_STAGE1	=	0

		; This loader keeps the System Card permanently in MPR7.

USING_MPR7	=	0

		; Yes, we're currently builing the Stage1 loader.

BUILDING_STAGE1	=	1

		; Do NOT allocate HOME_BANK for the Stage1 loader!

NEED_HOME_BANK	=	0

		; Do NOT allocate SOUND_BANK for the Stage1 loader!

NEED_SOUND_BANK	=	0

		; This loader wants to keep DATA_BANK == CORE_BANK, so ...

RESERVE_BANKS	=	-1

		; Include the library, and override the project's settings of
		; the above two values that are in "core-config.inc".

		include "core.inc"		; Basic includes.

		.list
		.mlist



		.code

; ***************************************************************************
; ***************************************************************************
;
; core_main - This is executed after "CORE(not TM)" library initialization.
;
; There are a few hundred bytes of unused space between the startup code, and
; the kernel code that gets copied to RAM, and this code fits nicely into the
; space!
;
; The code just checks whether this is a System Card 3 (or higher), and that
; there is at-least 192KB of extra SCD RAM available, and then it chooses to
; either execute the 2nd overlay file, or the CDERR overlay file.
;

	.if	(CDROM == SUPER_CD)		; If SuperCDROM, then ...

core_main:	jsr	ex_getver		; Get System Card version.
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

		sta	<core_1stbank		; Set SCD RAM bank as 1st.

		ldx	#2			; Load & run the 2nd file.

.load_file:	lda	irq_cnt			; Wait a frame so that the
		beq	.load_file              ; new joypad code runs once.

		jmp	exec_overlay		; Call function in RAM.

.not_supercd:	ldx	iso_cderr		; Is there an error file
		bne	.load_file		; to run?

		; Fall back to a simple on-screen text message.

		jsr	ex_dspoff		; Disable background/sprites.

		jsr	ex_vsync		; Wait for vsync.

		jsr	clear_vram		; Initialize VRAM.

		lda	#<font_data		; Upload font data.
		sta	<_bp + 0
		lda	#>font_data
		sta	<_bp + 1
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
		jsr	ex_colorcmd		; Do immediately!

		jsr	write_string		; Display the message.

		jsr	ex_bgon			; Enable background.

.hang:		jsr	ex_vsync		; Wait for vsync.
		bra	.hang			; Wait for user to reboot.

	.else	(CDROM == SUPER_CD)		; If not SuperCDROM, then ...

core_main:	ldx	#2			; Load & run the 2nd file.

.load_file:	lda	irq_cnt			; Wait a frame so that the
		beq	.load_file              ; new joypad code runs once.

		jmp	exec_overlay		; Call function in RAM.

	.endif	(CDROM == SUPER_CD)



; ***************************************************************************
; ***************************************************************************
;
; cpc464_colors - Palette data (a blast-from-the-past!)
;
;  0 = transparent
;  1 = dark blue shadow
;  2 = white font
;
; 12 = dark blue background
; 13 = light blue shadow
; 14 = yellow font

cpc464_colors:	dw	$0000,$0001,$01B2,$01B2,$0000,$0000,$0000,$0000
		dw	$0000,$0000,$0000,$0000,$0002,$004C,$0169,$01B2



; ***************************************************************************
; ***************************************************************************
;
; clear_vram - Clear all of VRAM.
;

clear_vram:	bsr	clear_screen		; Clear the BAT	 ($1000).

		ldx	#$7000 / $200		; Clear the rest ($7000).
		cly
		st1	#$00
.clr_loop:	st2	#$00
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
		st1	#0
		st2	#0
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
; 12 = background
; 13 = shadow
; 14 = font

tmp_shadow_buf	equ	$2100			; Interleaved 16 + 1 lines.
tmp_normal_buf	equ	$2101			; Interleaved 16 lines.

upload_font8x8: ldx	#$14			; Upload solid version to
		lda	#$FF			; VRAM $1400.

		pha				; Preserve bit 2 & 3 fill.

		vreg	#VDC_MAWR		; Set VRAM destination.
		stz	VDC_DL
		stx	VDC_DH

		vreg	#VDC_VWR
		lda	#25			; Minimal font of 23 characters.
		sta	<_al

		cly

.tile_loop:	clx				; Create a drop-shadowed version
		stz	tmp_shadow_buf, x	; of the glyph.

		.if	1
.line_loop:	lda	[_bp], y		; Drop-shadow on the LHS.
		sta	tmp_normal_buf, x	; Font data is RHS justified.
		asl	a
		.else
.line_loop:	lda	[_bp], y		; Drop-shadow on the RHS.
		sta	tmp_normal_buf, x	; Font data is LHS justified.
		lsr	a
		.endif

		ora	[_bp], y
		sta	tmp_shadow_buf+2, x
		ora	tmp_shadow_buf, x
		eor	tmp_normal_buf, x
		sta	tmp_shadow_buf, x

		iny
		bne	.next_line
		inc	<_bp + 1
.next_line:	inx
		inx
		cpx	#2*8
		bne	.line_loop

.copy_tile:	tia	tmp_shadow_buf, VDC_DL, 16

		pla				; Restore bit 2 & 3 byte.

		ldx	#8
		sta	VDC_DL			; Solid uses colors 12-14.
.plane23_loop:	sta	VDC_DH
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
; write_string - Simplified string-write to the screen.
;

write_string:	php				; Nasty code to set up the
		sei				; VDC write address without
		st0	#VDC_MAWR		; using the "vreg" macro.
	.if	0
		st1	#<(2 + (13 * 64))
		st2	#>(2 + (13 * 64))
	.else
		st1	#<(1 + (13 * 64))
		st2	#>(1 + (13 * 64))
	.endif
		st0	#VDC_VWR
		lda	#VDC_VWR
		sta	<vdc_reg
		plp

		clx
.loop:		lda	.supercd_msg,x		; Get next ASCII character.
		beq	.done
		inx

		sta	VDC_DL			; Add on base + palette.
		lda	#>$0140
		sta	VDC_DH
		bra	.loop

.done:		rts

		; The error message is remapped into a minimal font that
		; contains only 25 characters.
		;
		; N.B. The font is also used by "ipl-scd.asm"!
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

.supercd_msg:	db	"ABCDE@FGHIJKL@AMNODP@QDDRDRS"
;		db	"Super CD-ROM2 System needed!"
		db	0

	.else

.supercd_msg:	db	"ABCDE@FGHIJKL@AMNODP@EDWBXEDRS"
;		db	"Super CD-ROM2 System required!"
		db	0

	.endif



; ***************************************************************************
; ***************************************************************************
;
; Include the minimal font data for the SuperCD error string.
;

font_data:	incbin	"font8x8-stage1-exos.dat"



; ***************************************************************************
; ***************************************************************************
;
; Pad this loader out to the end of the current CD sector.
;
; This file is being assembled with the PCEAS "-bin" option,
; so only the data used is written out, allowing the output
; file to be sector-aligned rather than bank-aligned.

	.if	(* & 2047)
		ds	2048 - (* & 2047), 255	; Pad to end of CD sector.
	.endif
