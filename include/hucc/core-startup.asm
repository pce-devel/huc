; ***************************************************************************
; ***************************************************************************
;
; core-startup.asm
;
; The "CORE(not TM)" PC Engine library startup code that runs at boot/reset.
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
; 1) If we're running on a HuCARD, the initialization is simple!
;
;    The PC Engine's memory map is set to ...
;
;      MPR0 = bank $FF : PCE hardware
;      MPR1 = bank $F8 : PCE RAM with Stack & ZP
;      MPR2 = bank $00 : HuCARD ROM
;      MPR3 = bank $01 : HuCARD ROM
;      MPR4 = bank $02 : HuCARD ROM
;      MPR5 = bank $03 : HuCARD ROM
;      MPR6 = bank $04 : HuCARD ROM
;      MPR7 = bank $00 : HuCARD ROM
;
;
; 2) If we're running on a HuCARD that supports the Turbo Everdrive, then the
;    first 2 banks are reserved for mapping the TED2 hardware and a RAM bank.
;
;    The PC Engine's memory map is set to ...
;
;      MPR0 = bank $FF : PCE hardware
;      MPR1 = bank $F8 : PCE RAM with Stack & ZP
;      MPR2 = bank $02 : HuCARD ROM
;      MPR3 = bank $03 : HuCARD ROM
;      MPR4 = bank $04 : HuCARD ROM
;      MPR5 = bank $05 : HuCARD ROM
;      MPR6 = bank $06 : HuCARD ROM
;      MPR7 = bank $02 : HuCARD ROM
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
	.if	!CDROM				; Only applies to a HuCARD!

; ***************************************************************************
; ***************************************************************************
;
; RESET VECTORS (when booted as a HuCARD for the Turbo Everdrive 2)
;

		; Minimal HuCARD startup code that immediately trampoline's
		; to the normal HuCARD startup code in bank 2.

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

		tii	.reboot_bank, _ax, 7	; Reboot with bank 2 in MPR7.
		jmp	_ax
.reboot_bank:	lda	#2			; Put bank 2 in MPR7.
		tam7
		jmp	[$FFFE]			; Call reset, just like boot.

		; This string at this location tells both TEOS and Mednafen
		; that this HuCARD ROM is TED2-aware, and to run the HuCARD
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

		.data

		.bank	1			; Keep bank 1 for MPR2 RAM!

		.org	$5FFE			; Put something at the end so
		dw	$FFFF			; that procedures are not put
		.org	$4000			; in this bank!

		.code

		.bank	2			; Continue in bank 2.

	.if	USING_NEWPROC			; If the ".proc" trampolines
__trampolinebnk =	2			; are in MPR7, tell PCEAS to
	.endif					; put them in bank 2.

	.else	!CDROM
		.fail	You cannot currently build for CD-ROM and SUPPORT_TED2!
	.endif	!CDROM
	.endif	SUPPORT_TED2



	.if	USING_MPR7

; ***************************************************************************
; ***************************************************************************
;
; RESET VECTORS (when running in MPR7, either as a HuCARD, or a CD overlay)
;

		; Hardware reset and interrupt vectors.

		.org	$FFF4

core_version:	db	CORE_VERSION		; CORE(not TM) Version.
		db	$80			; System Card compatibility.

		dw	core_irq2		; IRQ2	(from CD/ADPCM)
		dw	core_irq1		; IRQ1	(from VDC)
		dw	core_timer		; TIMER (from CPU)
		dw	core_rti		; NMI	(unused)
	.if	CDROM
		dw	core_sw_reset		; RESET (CD-ROM)
	.else
		dw	core_hw_reset		; RESET (HuCARD)
	.endif	CDROM

	.if	USING_NEWPROC			; If the ".proc" trampolines
__trampolineptr =	$FFF3			; are in MPR7, tell PCEAS to
	.endif					; put them below the vectors.

	.else	USING_MPR7

		; Sanity Check.

	.if	!CDROM
		.fail	You cannot build for HuCARD without using MPR7!
	.endif	!CDROM

	.if	USING_NEWPROC			; If the ".proc" trampolines
__trampolineptr =	$5FFF			; are in MPR2, tell PCEAS to
	.endif					; put them right at the end.

	.endif	USING_MPR7



; ***************************************************************************
; ***************************************************************************
;
; !!! THE HuCARD/OVERLAY PROGRAM'S FIRST BANK STARTS HERE !!!
;

		; Switch to MPR2 for the "CORE(not TM)" library init.
		;
		; This is also executed by a HuCARD once it has run
		; its initial hardware-reset code.
		;
		; When run, MPR2-MPR6 are always mapped to the 1st 5 banks of
		; the overlay program, and MPR7 contains the System Card.

		.org	$4000

CORE_BANK	=	bank(*) - _bank_base	; It isn't always zero! ;-)

core_boot:	jmp	* + 6			; Allow someone to patch this.

		; Add an ident string so isoLINK can autodetect configuration.
		;
		; This allows isoLINK to decide whether to use the IPL-SCD boot
		; sector hack, and whether to include a SuperGRAFX signature in
		; the boot sector, without the user having to manually use some
		; command line parameters, although they are free to do that if
		; they wish.

	.if	SUPPORT_SGX
	.if	BUILDING_STAGE1
		db	"SG1"			; SGX SuperCD Stage1 loader.
	.else
		db	"SGX"			; SGX SuperCD or SGX HuCARD.
	.endif	BUILDING_STAGE1
	.else
	.if	CDROM
	.if	CDROM == 2
	.if	BUILDING_STAGE1
		db	"SC1"			; PCE SuperCD Stage1 loader.
	.else
		db	"SCD"			; PCE SuperCD.
	.endif	BUILDING_STAGE1
	.else
		db	" CD"			; PCE CD.
	.endif	CDROM == 2
	.else
		db	"PCE"			; PCE HuCARD.
	.endif	CDROM
	.endif	SUPPORT_SGX

	.if	CDROM

	.if	!USING_STAGE1			; Because the Stage1 does it!

		lda	core_kernel		; Did a previous overlay set
		bne	.not_first		; up the kernel already?

		; Is there a duplicate of the main ISO data track?

	.if	SUPPORT_2ISO
		lda	tnomax			; Set the last track as the
		sta	<_al			; backup area for reading
		lda	#$80			; CD data in case of disc
		sta	<_bh			; errors.
		lda	#2
		sta	<_cl
		jsr	cd_base
	.endif	SUPPORT_2ISO

		sei				; Disable interrupts!

		; Copy the ISO's directory from IPL memory to a safe spot.

		tii	$35FF, $3DFF, 512 + 1

		; Copy the kernel code to its destination in MPR1.

		tii	core_rom1st, core_ram1st, (core_ramiso - core_ram1st)

		; Copy the ISO's directory into kernel memory in MPR1.

		tii	$3DFF, iso_cderr, MAX_DIRSIZE + 1
		tii	$3F00, iso_dirhi, MAX_DIRSIZE

		; Clear the rest of the RAM bank on the 1st time through.

		tai	const_0000, core_ramend, ($4000 - core_ramend)

	.endif	!USING_STAGE1

		; Subsequent overlays only clear zero-page, and it is up to
		; the overlay itself to clear any part of the BSS RAM that
		; needs to be cleared.

.not_first:	sei				; Disable interrupts!
		tai	const_0000, core_zp1st, (core_zpend - core_zp1st)

	.else	CDROM

		; Set up HuCARD RAM in a compatible way to the System Card.
		;
		; Note that the entire RAM is cleared by "core_hw_reset".

		lda	#%11111			; Enable joypad soft-reset.
		sta	joyena

	.endif	CDROM

		; Now that RAM is initialized ...

		tma2				; Remember overlay's 1st bank
		sta	<core_1stbank		; $00, $02, $68 or $80!
	.if	USING_MPR7
		tam7				; "CORE(not TM)" takes MPR7!
	.endif

		ldx	#$FF			; Initialize stack pointer.
		txs

		jsr	core_clr_hooks		; Reset default hooks.

;		stz	TIMER_CR		; Stop HuC6280 timer.
;		stz	IRQ_ACK			; Clr HuC6280 timer interrupt.
;		stz	IRQ_MSK			; Clr HuC6280 interrupt mask.

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

		; In a HuCARD, BSS variables start as low as possible.

		.bss
		.org	core_ram1st
core_ramend	=	*
		.code

		; Normal HuCARD hardware-reset code, executed in MPR7.
		;
		; This does the basic PCE startup that every HuCARD (including
		; a System Card) needs to do, and then it remaps memory to be
		; compatible with the "CORE(not TM)" CD overlay program start.

		.page	7			; This will run in MPR7.

core_hw_reset:	sei				; Disable interrupts.
		csh				; Set high-speed mode.
		cld

		ldx	#$FF			; Initialize stack pointer.
		txs
		txa				; MPR0 = $FF : PCE hardware
		tam0				; MPR1 = $F8 : PCE RAM
		lda	#$F8
		tam1

		stz	TIMER_CR		; HW reset already does these,
		stz	IRQ_ACK			; but this may be a reset from
		stz	IRQ_MSK			; software (i.e. joypad).

		tai	const_0000, $2000, 8192 ; Clear RAM.

		tma7				; Not always bank 0!

		tam2				; Set CD-ROM overlay memory map,
		inc	a			; 1st 5 banks in MPR2-MPR6.
		tam3
		inc	a
		tam4
		inc	a
		tam5
		inc	a
		tam6

		jmp	core_boot		; Continue execution in MPR2.

		; In a HuCARD, the kernel code is permanently in MPR7.

		include "core-kernel.asm"



	.else	!CDROM

; ***************************************************************************
; ***************************************************************************
;
; CD-ROM Kernel Code
;
; core_rom1st - Start of code to relocate from MPR2 to MPR1.
; core_ram1st - Start of code when relocated to MPR1.
; core_ramend - End of code when relocated to MPR1.
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

	.if	(core_kernel != core_ram1st)
		.fail	Stage1 kernel has not been built with the same core_ram1st!
	.endif

	.else	USING_STAGE1

		; Use .PHASE/.DEPHASE to assemble the kernel to run in MPR1.

core_rom1st:	.phase	core_ram1st		; Assemble the code for RAM.

		include "core-kernel.asm"

		; Remember where the kernel code ends in MPR1.

core_ramiso:	.dephase			; Restore normal assembly.

		; ISOlink CD-ROM File Directory :
		;
		; When this program is run, the IPL has loaded the directory
		; into memory at $35FF-$37FF.
		;
		; The format of the directory is shown below ...
		;
		;  ------$35FF: index # of CDERR file
		;  $3600-$36FF: lo-byte of sector # of start of file
		;  $3700-$37FF: hi-byte of sector # of start of file
		;  ------$3600: # of files stored on CD-ROM
		;  ------$3700: index # of 1st file beyond 128MB
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

		rsset	core_ramiso

iso_cderr	rs	1			; index # of CDERR file
iso_dirlo	rs	MAX_DIRSIZE		; lo-byte of file's sector #
iso_dirhi	rs	MAX_DIRSIZE		; hi-byte of file's sector #
iso_count	=	iso_dirlo		; # of files stored on CD-ROM
iso_128mb	=	iso_dirhi		; index # of 1st beyond 128MB

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
; the CORE_BANK of the HuCARD / overlay program.
;
; This is the end of the "CORE(not TM)" library code!
;
; Remember that the ".proc" trampolines are located at the end of this bank,
; so the amount of free space left depends upon the number of ".proc" calls.
;

	.if	USING_MPR7
		; Switch to MPR7 to run the developer's game code.

		.page	7			; User code runs in MPR7.
	.else
		; Switch to MPR2 to run the developer's game code.

		.page	2			; User code runs in MPR2.
	.endif	USING_MPR7



; ***************************************************************************
; ***************************************************************************
;
; The DATA_BANK location needs to be set as early as possible so that library
; code is able to put data in there before the total overall size of the code
; is known.
;
; By default, DATA_BANK is the next bank after the CORE_BANK.
;
; RESERVE_BANKS allows developers to reserve some banks between the CORE_BANK
; and the DATA_BANK that they can use however they wish.
;
; One use for RESERVE_BANKS is to create empty space that PCEAS can use when
; it relocates procedures. This provides a way for a developer to group code
; together at the start of memory, and leave the maximum number of banks for
; loading dynamic data from CD-ROM.
;
; The KickC environment sets RESERVE_BANKS=1 (or higher) so that there is a
; a bank for the permanent C code and static constants.
;
; RESERVE_BANKS is normally defined in each project's "core-config.inc".
;

		.opt	d+			; DATA labels use fixed MPR.

		.rsset	CORE_BANK + 1

	.ifdef	NEED_HOME_BANK
	.if	NEED_HOME_BANK
HOME_BANK	.rs	1
		.home				; HuCC permanent code and
		.bank	HOME_BANK, ".home"	; initialized data runs in
		.org	$A000			; MPR5 (.section CODE).
	.endif
	.endif

	.ifdef	NEED_SOUND_BANK			; Defined in hucc-sound.inc
	.if	NEED_SOUND_BANK			; if the driver wants a bank.
SOUND_BANK	.rs	1
	.endif
	.endif

	.ifdef	RESERVE_BANKS			; For CORE projects.
RESERVED_BANK	.rs	RESERVE_BANKS
	.endif

	.ifdef	HUCC
	.ifdef	HUC_RESERVE_BANKS		; For HuCC projects.
HUC_USER_BANK	.rs	HUC_RESERVE_BANKS
	.endif

CONST_BANK	.rs	2
		.data
		.bank	CONST_BANK, ".const"
		.org	$6000
	.endif	HUCC

DATA_BANK	.rs	0
		.data
		.bank	DATA_BANK, ".data"
		.org	$6000

		.code
