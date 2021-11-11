; ***************************************************************************
; ***************************************************************************
;
; core-startup.asm
;
; The "CORE(not TM)" PC Engine library startup code that runs at boot/reset.
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
; This is the initialization code in the 1st bank, and it is responsible for
; setting up a consistant runtime environment for the developer's program,
; so that the developer can concentrate on writing the program itself.
;
; The idea is that when a program is loaded, the first 40KB of it is mapped
; as $4000..$DFFF, and initialization starts at $4000, with the developer's
; program getting control after initialization, with a jump to "core_main".
;
; The initializtion sets up a small kernel of code that provides interrupt
; handling that mimics a subset of the System Card's behavior, designed to
; act in the same way on HuCARD and CD-ROM, with either the System Card in
; MPR7, or with an overlay in MPR7.
;
; The kernel handles reading the joypad/mouse, and it offers handler hooks
; for running the developer's interrupt code.  On CD-ROM systems, it also
; handles the loading and running of subsequent overlay programs.
;
; On HuCARD, the kernel itself runs in MPR7; while on CD-ROM systems it is
; run from RAM in MPR1, so that overlay programs are independant from each
; other, and can be written in different programming languages.
;
;
; 1) If we're running on a HuCard, the initialization is simple!
;
;    The PC Engine's memory map is set to ...
;
;      MPR0 = bank $FF : PCE hardware
;      MPR1 = bank $F8 : PCE RAM with Stack & ZP
;      MPR2 = bank $00 : HuCard ROM
;      MPR3 = bank $01 : HuCard ROM
;      MPR4 = bank $02 : HuCard ROM
;      MPR5 = bank $03 : HuCard ROM
;      MPR6 = bank $04 : HuCard ROM
;      MPR7 = bank $00 : HuCard ROM
;
;
; 2) If we're running on a HuCard that supports the Turbo Everdrive, then the
;    first bank is reserved for mapping the TED2 hardware.
;
;    The PC Engine's memory map is set to ...
;
;      MPR0 = bank $FF : PCE hardware
;      MPR1 = bank $F8 : PCE RAM with Stack & ZP
;      MPR2 = bank $01 : HuCard ROM
;      MPR3 = bank $02 : HuCard ROM
;      MPR4 = bank $03 : HuCard ROM
;      MPR5 = bank $04 : HuCard ROM
;      MPR6 = bank $05 : HuCard ROM
;      MPR7 = bank $01 : HuCard ROM
;
;
; 3) If we're running on an old CD System, the overlay is loaded from the ISO
;    into banks $80-$87 (64KB max).
;
;    The PC Engine's memory map is set to ...
;
;      MPR0 = bank $FF : PCE hardware
;      MPR1 = bank $F8 : PCE RAM with Stack & ZP
;      MPR2 = bank $80 : CD RAM
;      MPR3 = bank $81 : CD RAM
;      MPR4 = bank $82 : CD RAM
;      MPR5 = bank $83 : CD RAM
;      MPR6 = bank $84 : CD RAM
;      MPR7 = bank $80 : CD RAM or System Card's bank $00
;
;
; 4) If we're running on a SuperCD System, the overlay is loaded from the ISO
;    into banks $68-$87 (256KB max).
;
;    The PC Engine's memory map is set to ...
;
;      MPR0 = bank $FF : PCE hardware
;      MPR1 = bank $F8 : PCE RAM with Stack & ZP
;      MPR2 = bank $68 : SCD RAM
;      MPR3 = bank $69 : SCD RAM
;      MPR4 = bank $6A : SCD RAM
;      MPR5 = bank $6B : SCD RAM
;      MPR6 = bank $6C : SCD RAM
;      MPR7 = bank $68 : SCD RAM or System Card's bank $00
;
; ***************************************************************************
; ***************************************************************************



		.code
		.bank	0

	.if	SUPPORT_TED2			; Do we want to use a TED2?
	.if	!CDROM				; Only applies to a HuCard!

; ***************************************************************************
; ***************************************************************************
;
; RESET VECTORS (when booted as a HuCard for the Turbo Everdrive 2)
;

		; Minimal HuCard startup code that immediately trampoline's
		; to the normal HuCard startup code in bank 1.

		.org	$FF00

ted2_hw_reset:	sei				; Disable interrupts.
		csh				; Set high-speed mode.
		cld

		ldx	#$FF			; Initialize stack pointer.
		txs
		txa				; MPR0 = $FF : PCE hardware
		tam0				; MPR1 = $F8 : ZP & Stack
		lda	#$F8
		tam1

		tii	.reboot_bank, __ax, 5	; Reboot with bank 1 in MPR7.
		lda	#1
		jmp	__ax
.reboot_bank:	tam7				; Put bank 1 in MPR7.
		jmp	[$FFFE]			; Call reset, just like boot.

		; This string at this location tells both TEOS and Mednafen
		; that this HuCard ROM is TED2-aware, and to run the HuCard
		; with 1MB RAM enabled.

		.org	$FFD0

		db	"TED2CARD"		; Signature.
		dw	0			; Reserved word.
		dw	0			; Reserved word.

		; Hardware reset and interrupt vectors.

		.org	$FFF4

		db	CORE_VERSION		; CORE(not TM) Version.
		db	$80			; System Card compatibility.
		dw	ted2_hw_reset		; IRQ2	(from CD/ADPCM)
		dw	ted2_hw_reset		; IRQ1	(from VDC)
		dw	ted2_hw_reset		; TIMER (from CPU)
		dw	ted2_hw_reset		; NMI	(unused)
		dw	ted2_hw_reset		; RESET

		.bank	1			; Continue in bank 1.

	.if	USING_NEWPROC			; If the ".proc" trampolines
__trampolinebnk =	1			; are in MPR7, tell PCEAS to
	.endif					; put them in bank 1.

	.endif	!CDROM
	.endif	SUPPORT_TED2



	.if	USING_MPR7

; ***************************************************************************
; ***************************************************************************
;
; RESET VECTORS (when running in MPR7, either as a HuCard, or a CD overlay)
;

		; Hardware reset and interrupt vectors.

		.org	$FFF4

core_version:	db	CORE_VERSION		; CORE(not TM) Version.
		db	$80			; System Card compatibility.

		dw	core_irq2		; IRQ2	(from CD/ADPCM)
		dw	core_irq1		; IRQ1	(from VDC)
		dw	core_timer_irq		; TIMER (from CPU)
		dw	core_nmi_irq		; NMI	(unused)
	.if	CDROM
		dw	core_sw_reset		; RESET (CD-ROM)
	.else
		dw	core_hw_reset		; RESET (HuCARD)
	.endif	CDROM

	.if	USING_NEWPROC			; If the ".proc" trampolines
__trampolineptr =	$FFF4			; are in MPR7, tell PCEAS to
	.endif					; put them below the vectors.

	.else	USING_MPR7

		; Sanity Check.

	.if	!CDROM
		.fail	You cannot build for HuCARD without using MPR7!
	.endif	!CDROM

	.if	USING_NEWPROC			; If the ".proc" trampolines
__trampolineptr =	$6000			; are in MPR2, tell PCEAS to
	.endif					; put them right at the end.

	.endif	USING_MPR7



; ***************************************************************************
; ***************************************************************************
;
; !!! THE HuCARD/OVERLAY PROGRAM'S FIRST BANK STARTS HERE !!!
;

		; Switch to MPR2 for the "CORE(not TM)" library init.
		;
		; This is also executed by a HuCard once it has run
		; its initial hardware-reset code.
		;
		; When run, MPR2-MPR6 are always mapped to the 1st 5 banks of
		; the overlay program, and MPR7 contains the System Card.

		.org	$4000

core_boot:

	.if	CDROM

	.if	!USING_STAGE1			; Because the Stage1 does it!

		lda	core_ram1st		; Did a previous overlay set
		bne	.not_first		; up the kernel already?

		; Is there a duplicate of the main ISO data track?

	.if	SUPPORT_2ISO
		lda	tnomax			; Set the last track as the
		sta	<__al			; backup area for reading
		lda	#$80			; CD data in case of disc
		sta	<__bh			; errors.
		lda	#2
		sta	<__cl
		jsr	cd_base
	.endif	SUPPORT_2ISO

		sei				; Disable interrupts!

		; Copy the ISO's directory from IPL memory to a safe spot.

		tii	$2DFD, $3DFD, 515

		; Copy the kernel code to its destination in MPR1.

		tii	$4000 + (core_ram1st & $1FFF), core_ram1st, (core_ramend - core_ram1st)

		; Copy the ISO's directory into kernel memory in MPR1.

		tii	$3DFD, iso_count, MAX_DIRSIZE + 3
		tii	$3F00, iso_dirhi, MAX_DIRSIZE

		; Clear the rest of the RAM bank on the 1st time through.

		tai	const_0000, core_ramclr, ($4000 - core_ramclr)

	.endif	!USING_STAGE1

		; Subsequent overlays only clear zero-page, and it is up to
		; the overlay itself to clear any part of the BSS RAM that
		; needs to be cleared.

.not_first:	sei				; Disable interrupts!
		tai	const_0000, $2000, (core_zpend - $2000)

	.else	CDROM

		; Set up HuCard RAM in a compatible way to the System Card.

	.endif	CDROM

		; Now that RAM is initialized ...

		jsr	core_clr_hooks		; Reset default hooks.

		tma2				; Remember overlay's 1st bank
		sta	<core_1stbank		; $00, $01, $68 or $80!

	.if	USING_MPR7
		tam7				; "CORE(not TM)" takes MPR7!
	.endif

		ldx	#$FF			; Initialize stack pointer.
		txs

		lda	VDC_SR			; Purge any overdue VBL.
		stz	irq_cnt			; Make it easy to check.
		cli				; Restore interrupts.

		jmp	core_main		; Start the game's code.



	.if	!CDROM

; ***************************************************************************
; ***************************************************************************
;
; HuCARD Kernel Code
;
; core_ram1st - Start of code to relocate to MPR1.
; core_ramend - End of code to relocate to MPR1.
;

		; In a HuCard, BSS variables start as low as possible.

		.bss
		.org	core_ram1st
core_ramend	=	*
		.code

		; Normal HuCard hardware-reset code, executed in MPR7.
		;
		; This does the basic PCE startup that every HuCard (including
		; a System Card) needs to do, and then it remaps memory to be
		; compatible with the "CORE(not TM)" CD overlay program start.

		.org	$E000 + (* & $1FFF)

core_hw_reset:	sei				; Disable interrupts.
		csh				; Set high-speed mode.
		cld

		ldx	#$FF			; Initialize stack pointer.
		txs
		txa				; MPR0 = $FF : PCE hardware
		tam0				; MPR1 = $F8 : PCE RAM
		lda	#$F8
		tam1

		tai	const_0000, $2000, 8192 ; Clear RAM.

		tma7				; Set CD-ROM overlay memory map,
		tam2				; 1st 5 banks in MPR2-MPR6.
		inc	a
		tam3
		inc	a
		tam4
		inc	a
		tam5
		inc	a
		tam6

		jmp	core_boot		; Continue execution in MPR2.

		; In a HuCard, the kernel code is permanently in MPR7.

		include "core-kernel.asm"



	.else	!CDROM

; ***************************************************************************
; ***************************************************************************
;
; CD-ROM Kernel Code
;
; core_ram1st - Start of code to relocate to MPR1.
; core_ramend - End of code to relocate to MPR1.
;

		;
		; Set the BSS location to its initial value.
		;

		.bss
		.org	core_ram1st
		.code

	.if	USING_STAGE1

		; Read the already-assembled equates for the kernel code.

		include "core-stage1.s"

	.else	USING_STAGE1

		; Remember where the startup code ends in MPR2.
		;
		; There are a few hundred bytes of unused space between the
		; startup code, and the kernel code that gets copied to RAM,
		; and the Stage1 loader code fits nicely into that space!

core_stage1	=	*

		; Switch to MPR1 to build the kernel code that runs in RAM.

		.org	core_ram1st

		include "core-kernel.asm"

		; Remember where the kernel code ends in MPR1.

core_ramcpy	=	*

		rsset	core_ramcpy

		; ISOlink CD-ROM File Directory :
		;
		; When this program is run, the IPL has loaded the directory
		; into memory at $2DFD-$2FFF.
		;
		; The format of the directory is shown below ...
		;
		;  ------$2DFD: # of files stored on CD-ROM
		;  ------$2DFE: index # of CDERR file
		;  ------$2DFF: index # of 1st file beyond 128MB
		;  $2E00-$2EFF: lo-byte of sector # of start of file
		;  $2F00-$2FFF: hi-byte of sector # of start of file
		;
		; File 0  is the IPL (the 1st 2 sectors of the ISO).
		; File 1  is the first file on the ISOlink command line.
		; File 2+ are the extra files on the ISOlink command line.
		;
		; There is space for up to 255 files, including the IPL, and
		; the ISO track can be a maximum size of 256MB.
		;
		; In order to allow for files beyond the 16-bit limit of the
		; sector number, the directory stores the index of the first
		; file that starts beyond that boundary.

iso_count	rs	1			; # of files stored on CD-ROM
iso_cderr	rs	1			; index # of CDERR file
iso_128mb	rs	1			; index # of 1st beyond 128MB
iso_dirlo	rs	MAX_DIRSIZE		; lo-byte of file's sector #
iso_dirhi	rs	MAX_DIRSIZE		; hi-byte of file's sector #

core_ramclr	rs	0

		; Define persistant BSS variables here, before core_ramend.

core_ramend	rs	0

	.endif	USING_STAGE1

		; Make sure that no BSS variables were defined that
		; overlap the kernel code's location in RAM.

		.bss

	.if	(* != core_ram1st)
		.fail	BSS variables overwrite the "CORE(not TM)" kernel!
	.endif

		; In a CD-ROM, BSS variables start after the kernel code.

		.org	core_ramend

		.code


	.endif	!CDROM



; ***************************************************************************
; ***************************************************************************
;
; With the availability of so many different configuration options, we've now
; built somewhere between a few hundred bytes, and a couple of KB, of code in
; this first bank of the HuCARD / overlay program. But this is the end of the
; "CORE(not TM)" library code!
;
; Remember that the ".proc" trampolines are located at the end of this bank,
; so the amount of free space left depends upon the number of ".proc" calls.
;

	.if	USING_MPR7
		; Switch to MPR7 to run the developer's game code.

		.org	$E000 + (* & $1FFF)
	.else
		; Switch to MPR2 to run the developer's game code.

		.org	$4000 + (* & $1FFF)
	.endif	USING_MPR7
