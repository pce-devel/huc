; ***************************************************************************
; ***************************************************************************
;
; vdcbuffer.asm
;
; VDC buffer test HuCARD example of using the basic HuCARD startup library code.
;
; Copyright John Brandwood 2023.
;
; Distributed under the Boost Software License, Version 1.0.
; (See accompanying file LICENSE_1_0.txt or copy at
;  http://www.boost.org/LICENSE_1_0.txt)
;
; ***************************************************************************
; ***************************************************************************
;
; The purpose of this example is to check whether the VDC's VRAM reading is
; buffered or direct.
;
; ***************************************************************************
; ***************************************************************************
;
; The PC Engine's memory map is set to ...
;
;   MPR0 = bank $FF : PCE hardware
;   MPR1 = bank $F8 : PCE RAM with Stack & ZP
;   MPR2 = bank $00 : HuCARD ROM
;   MPR3 = bank $01 : HuCARD ROM
;   MPR4 = bank $02 : HuCARD ROM
;   MPR5 = bank $03 : HuCARD ROM
;   MPR6 = bank $04 : HuCARD ROM
;   MPR7 = bank $00 : HuCARD ROM with the startup code and IRQ vectors.
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

		.list
		.mlist

		include "bare-startup.asm"	; No "CORE(not TM)" library!

		include	"common.asm"		; Common helpers.
		include	"vdc.asm"		; Useful VDC routines.
		include	"font.asm"		; Useful font routines.
		include "joypad.asm"		; Joypad routines.
		include	"tty.asm"		; Useful TTY print routines.



; ***************************************************************************
; ***************************************************************************
;
; Constants and Variables.
;

		.zp

value		ds	2			; Data value from VRAM read.

		.bss



; ***************************************************************************
; ***************************************************************************
;
; bare_main - This is executed after startup library initialization.
;

		.code

bare_main:	; Turn the display off and initialize the screen mode.

		jsr	init_256x224		; Initialize VDC & VRAM.

		; Upload the font to VRAM.

		stz	<_di + 0		; Destination VRAM address.
		lda	#>(CHR_0x10 * 16)
		sta	<_di + 1

		lda	#$FF			; Put font in colors 4-7,
		sta	<_al			; so bitplane 2 = $FF and
		stz	<_ah			; bitplane 3 = $00.

		lda	#16 + 96		; 16 graphics + 96 ASCII.
		sta	<_bl

		lda	#<my_font		; Address of font data.
		sta	<_bp + 0
		lda	#>my_font
		sta	<_bp + 1
		ldy	#^my_font

		call	dropfnt8x8_vdc		; Upload font to VRAM.

		; Upload the palette data to the VCE.

		stz	<_al			; Start at palette 0 (BG).
		lda	#2			; Copy 2 palettes of 16 colors.
		sta	<_ah
		lda	#<screen_pal		; Set the ptr to the palette
		sta	<_bp + 0		; data.
		lda	#>screen_pal
		sta	<_bp + 1
		ldy	#^screen_pal
		call	load_palettes		; Add to the palette queue.

		call	xfer_palettes		; Transfer queue to VCE now.

		; Turn on the BG & SPR layers, then wait for a soft-reset.

		call	set_dspon		; Enable background.

		;

		PRINTF	"\e<\eX1\eY1\eP0***PC ENGINE VDC R/W BUFFER***\n\n\eP1"

		stz.l	<_di			; _di = $8000
		lda	#$80
		sta	<_di

		jsr	vdc_di_to_mawr		; Set MAWR=$8000.

		lda	#$34			; Write $1234 to VRAM $8000.
		sta	VDC_DL
		lda	#$12
		sta	VDC_DH

		tii	$2200,$2200,16		; 113 cycle delay.

		jsr	vdc_di_to_marr		; Set MARR=$8000, trigger read.

		tii	$2200,$2200,16		; 113 cycle delay.

		jsr	vdc_di_to_mawr		; Set MAWR=$8000.

		lda	#$55			; Write $AA55 to VRAM $8000.
		sta	VDC_DL
		lda	#$AA
		sta	VDC_DH

		tii	$2200,$2200,16		; 113 cycle delay.

		lda	VDC_DL			; Read contents of VRAM $8000.
		sta.l	<value
		lda	VDC_DH
		sta.h	<value

		PRINTF	" Write $1234 to VRAM $8000.\n"
		PRINTF	" Set $8000 read address.\n"
		PRINTF	" Write $AA55 to VRAM $8000.\n"
		PRINTF	" VRAM $8000 contents are $%x.\n", value

		lda.l	<value
		cmp.l	#$1234
		bne	.is_direct
		lda.h	<value
		cmp.h	#$1234
		bne	.is_direct

.is_buffered:	PRINTF	" The VRAM read is buffered.\n"
		jmp	.next_frame

.is_direct:	PRINTF	" The VRAM read is direct.\n"
		jmp	.next_frame

		; Loop around updating the display each frame.

.next_frame:	lda	irq_cnt			; Wait for vsync.
.wait_frame:	cmp	irq_cnt
		beq	.wait_frame

.got_vblank:	jmp	.next_frame		; Wait for user to reboot.



; ***************************************************************************
; ***************************************************************************
;
; screen_pal - Palette data
;
; Note: DEFPAL palette data is in RGB format, 4-bits per value.
; Note: Packed palette data is in GRB format, 3-bits per value.
;
;  $4 = dark blue background
;  $5 = light blue shadow
;  $6 = yellow font
;

		.data

		align	2

none		=	$000

screen_pal:	defpal	$000,none,none,none,$002,$114,$772,none
		defpal	none,none,none,none,none,none,$111,$777

		defpal	$000,none,none,none,$002,$333,$777,none
		defpal	none,none,none,none,none,none,$101,$727



; ***************************************************************************
; ***************************************************************************
;
; It's the font data, nothing exciting to see here!
;

my_font:	incbin	"font8x8-ascii-bold-short.dat"
