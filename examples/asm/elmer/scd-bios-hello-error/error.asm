; ***************************************************************************
; ***************************************************************************
;
; error.asm
;
; "Not running on a Super CD-ROM" example error overlay using BIOS functions.
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
;   MPR1 = bank $F8 : PCE RAM with Stack & ZP
;   MPR2 = bank $80 : CD RAM
;   MPR3 = bank $81 : CD RAM
;   MPR4 = bank $82 : CD RAM
;   MPR5 = bank $83 : CD RAM
;   MPR6 = bank $84 : CD RAM
;   MPR7 = bank $00 : System Card
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

		;
		; A few definitions for readability.
		;

_si_bank	=	_dh			; Alias for far-pointers.

CHR_SIZ		=	32			; Size in bytes.
SPR_SIZ		=	128			; Size in bytes.



; ***************************************************************************
; ***************************************************************************
;
; bios_main - This is executed after "CORE(not TM)" library initialization.
;

bios_main:	; Turn the display off and initialize the screen mode.

		jsr	ex_dspoff		; Disable background/sprites.

		jsr	ex_vsync		; Wait for vsync.

		cla				; Set up a 256x224 video
		ldx	#256 / 8		; mode.
		ldy	#224 / 8
		jsr	ex_scrmod

		lda	#VDC_MWR_32x32 >> 4	; Set up 32x32 virtual
		jsr	ex_scrsiz		; screen.

		cla				; Reset VRAM increment.
		jsr	ex_imode

		; Upload the palette data to the VCE.

		lda	#<bkg_pal		; Tell BIOS to upload the
		sta	bgc_ptr + 0		; background palette which
		lda	#>bkg_pal		; we just mapped into MPR3
		sta	bgc_ptr + 1		; with the font.
		lda	#16
		sta	bgc_len
		lda	#<spr_pal
		sta	sprc_ptr + 0
		lda	#>spr_pal
		sta	sprc_ptr + 1
		lda	#16
		sta	sprc_len
		lda	#2
		sta	color_cmd		; Upload next VBLANK.

		; Clear VRAM, including the BAT.

		call	clear_vram

		; Ensure the palettes are uploaded before remapping banks.

.wait:		lda	color_cmd		; BIOS clears this when
		bne	.wait			; copied.

		; Copy the BAT to VRAM (@$0000).

		lda.l	#bkg_bat
		sta.l	<_si
		lda.h	#bkg_bat
		sta.h	<_si
		lda	#bank(bkg_bat)
		sta	<_si_bank
		stz.l	<_di
		stz.h	<_di
		lda.l	#32*28
		sta.l	<_ax
		lda.h	#32*28
		sta.h	<_ax
		jsr	load_vram

		; Copy the CHR to VRAM (@$0800).

		lda.l	#bkg_chr
		sta.l	<_si
		lda.h	#bkg_chr
		sta.h	<_si
		lda	#bank(bkg_chr)
		sta	<_si_bank
		stz.l	<_di
		lda.h	#$0800
		sta.h	<_di
		lda.l	#32*28*(CHR_SIZ/2)
		sta.l	<_ax
		lda.h	#32*28*(CHR_SIZ/2)
		sta.h	<_ax
		jsr	load_vram

		; Copy the SPR to VRAM (@$4000).

		lda.l	#spr_spr
		sta.l	<_si
		lda.h	#spr_spr
		sta.h	<_si
		lda	#bank(spr_spr)
		sta	<_si_bank
		stz.l	<_di
		lda.h	#$4000
		sta.h	<_di
		lda.l	#2*4*2*(SPR_SIZ/2)
		sta.l	<_ax
		lda.h	#2*4*2*(SPR_SIZ/2)
		sta.h	<_ax
		jsr	load_vram

		; Copy the SAT to VRAM (@$7F00).

		lda.l	#spr_sat
		sta.l	<_si
		lda.h	#spr_sat
		sta.h	<_si
		lda	#bank(spr_sat)
		sta	<_si_bank
		stz.l	<_di
		lda.h	#$7F00
		sta.h	<_di
		lda.l	#2*8/2
		sta.l	<_ax
		lda.h	#2*8/2
		sta.h	<_ax
		jsr	load_vram

		; Xfer the VRAM SATB to SAT next vblank.

		lda.h	#$7F00			; Xfer SATB @$7F00 to SAT.
		stz.l	satb_addr
		sta.h	satb_addr
		jsr	ex_sprdma

		jsr	ex_vsync		; Wait for vsync.

		; Turn on the BG & SPR layers, then wait for a soft-reset.

		jsr	ex_dspon		; Enable background.

.hang:		jsr	ex_vsync		; Wait for vsync.
		bra	.hang			; Wait for user to reboot.



; ***************************************************************************
; ***************************************************************************
;
; clear_vram - Clear the VRAM in the PCE VDC.
;

clear_vram:	cla				; Set VDC or SGX destination
		clx				; address.
		jsr	ex_setwrt

		ldx	#65536/(256*4)
		cly
		st1	#0
.clr_loop:	st2	#0
		st2	#0
		dey
		bne	.clr_loop
		dex
		bne	.clr_loop

		rts



; ***************************************************************************
; ***************************************************************************
;
; load_vram - Copy far-data from memory to VRAM.
;
; _si		= Source memory location
; _si_bank	= Source bank
; _di		= Destination VRAM address
; _ax		= # of words to copy
;

load_vram:	tma3				; Preserve MPR3 & MPR4.
		pha
		tma4
		pha

		lda	<_si + 1		; Remap ptr to MPR3.
		and	#$1F
		ora	#$60
		sta	<_si + 1
		lda	<_si_bank		; Put bank into MPR3.
		tam3
		inc	a			; Put next bank into MPR4.
		tam4

		lda	#VDC_MAWR		; Set VDC or SGX destination
		stz	<vdc_reg		; address.
		stz	VDC_AR
		lda	<_di + 0
		sta	VDC_DL
		lda	<_di + 1
		sta	VDC_DH
		lda	#VDC_VWR		; Select the VWR data-write
		sta	<vdc_reg		; register.
		sta	VDC_AR

		ldx.l	<_si
		stx	.ram_tia + 1
		ldy.h	<_si
		sty	.ram_tia + 2
		lda	#32
		sta	.ram_tia + 5

		lda	<_al			; length in chunks
		lsr	<_ah
		ror	a
		lsr	<_ah
		ror	a
		lsr	<_ah
		ror	a
		lsr	<_ah
		ror	a

		sax				; x=chunks-lo
		beq	.next_4kw		; a=source-lo, y=source-hi

.copy_chunk:	bsr	.ram_tia		; transfer 32-bytes

		clc				; increment source
		adc	#$20
		sta	.ram_tia + 1
		bcc	.same_page
		iny
		bpl	.same_bank		; remap_data

		say
		tma4
		tam3
		inc	a
		tam4
		lda	#$60
		say

.same_bank:	sty	.ram_tia + 2		; increment source page

.same_page:	dex
		bne	.copy_chunk

.next_4kw:	dec	<_ah
		bpl	.copy_chunk

		lda	<_al
		and	#15
		beq	.done

		asl	a
		sta	.ram_tia + 5

		bsr	.ram_tia		; transfer remainder

.done:		pla				; Restore MPR3 & MPR4.
		tam4
		pla
		tam3
		rts

.ram_tia:	tia	$1234, VDC_DL, 32	; transfer 32-bytes
		rts



; ***************************************************************************
; ***************************************************************************
;
; The SATB entries for the two overlay sprites that make the 31-color girl.
;

spr_sat:	dw	64+(119)		; Y
		dw	32+(173)		; X
		dw	$4000>>5		; SPR VRAM
		db	%10000000		; Sprite priority.
		db	%00110001		; CGY=3, CGX=1 (64x32).

		dw	64+(119)		; Y
		dw	32+(173+32)		; X
		dw	$4200>>5		; SPR VRAM
		db	%10000000		; Sprite priority.
		db	%00110001		; CGY=3, CGX=1 (64x32).



; ***************************************************************************
; ***************************************************************************
;
; Put the data next.
;
; Since no library functions are used, and there are no procdures, and so no
; trampolines at the end of bank 0, then the data does not need to start in
; a different bank !
;

; Overlay sprites used to make the girl more colorful.

spr_pal:	.incpal "error-spr-15.png"
spr_spr:	.incspr "error-spr-15.png",173+ 0,119,2,4
		.incspr "error-spr-15.png",173+32,119,2,4

; Main background image.

bkg_pal:	.incpal "error-bkg-16.png"
bkg_chr:	.incchr "error-bkg-16.png"
bkg_bat:	.incbat "error-bkg-16.png",$0800,32,28
