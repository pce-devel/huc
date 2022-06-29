; ***************************************************************************
; ***************************************************************************
;
; bare-startup.asm
;
; The basic PC Engine HuCARD startup code that runs at boot/reset.
;
; Copyright John Brandwood 2021-2022.
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
; as $4000..$DFFF, and initialization starts at $E000, with the developer's
; program getting control after initialization, with a jump to "bare_main".
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
; We're running on a HuCARD, so the initialization is simple!
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
; ***************************************************************************
; ***************************************************************************

		.nolist

;
; Sanity Check, this is only for HuCARD!
;

	.if	CDROM
		.fail	You cannot build for CD-ROM with bare-startup.asm!
	.endif	CDROM

;
; Support development for the SuperGrafx?
;
; This enables SGX hardware support in certain library functions.
;

	.ifndef	SUPPORT_SGX
SUPPORT_SGX	=	0	; (0 or 1)
	.endif

		; Include the equates for the basic PCE hardware.

		include	"pceas.inc"
		include	"pcengine.inc"

		.list
		.mlist

BASE_BANK	=	0

		.code
		.bank	BASE_BANK



; ***************************************************************************
; ***************************************************************************
;
; RESET VECTORS (when running in MPR7, either as a HuCard, or a CD overlay)
;

	.if	USING_NEWPROC			; If the ".proc" trampolines
__trampolineptr =	$FFF5			; are in MPR7, tell PCEAS to
	.endif					; put them below the vectors.

		; Hardware reset and interrupt vectors.

		.org	$FFF6

		dw	bare_irq2		; IRQ2	(from CD/ADPCM)
		dw	bare_irq1		; IRQ1	(from VDC)
		dw	bare_timer_irq		; TIMER (from CPU)
		dw	bare_nmi_irq		; NMI	(unused)
		dw	bare_hw_reset		; RESET (HuCARD)

		.org	$E000			; This will run in MPR7.



; ***************************************************************************
; ***************************************************************************
;
; Useful constants, needed by joypad library code, and used by many others.
;

const_FFFF:	dw	$FFFF			; Useful constant for TAI.
const_0000:	dw	$0000			; Useful constant for TAI.

bit_mask:	db	$01,$02,$04,$08,$10,$20,$40,$80



; ***************************************************************************
; ***************************************************************************
;
; Normal HuCard hardware-reset code, executed in MPR7.
;
; This does the basic PCE startup that every HuCard (including
; a System Card) needs to do, and then it remaps memory to be
; compatible with the "CORE(not TM)" CD overlay program start.
;

bare_hw_reset:	sei				; Disable interrupts.
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

		tma7				; Not bank 0 if a TED2!

		tam2				; Set CD-ROM overlay memory map,
		inc	a			; 1st 5 banks in MPR2-MPR6.
		tam3
		inc	a
		tam4
		inc	a
		tam5
		inc	a
		tam6

		lda	#%11111			; Enable joypad soft-reset.
		sta	joyena

		jsr	bare_clr_hooks		; Reset default hooks.

		lda	VDC_SR			; Purge any overdue VBL.
		stz	irq_cnt			; Make it easy to check.
		cli				; Restore interrupts.

		jmp	bare_main		; Start the game's code.



; ***************************************************************************
; ***************************************************************************
;
; bare_irq2	 - Minimal interrupt handler compatible with System Card.
; bare_irq1	 - Minimal interrupt handler compatible with System Card.
; bare_timer_irq - Minimal interrupt handler compatible with System Card.
; bare_nmi_irq	 - Minimal interrupt handler compatible with System Card.
;
; Note that it takes 8 cycles to respond to an IRQ.
;
; All of the game's actual interrupt handling itself is done in the "hook"
; functions so that everything works the same if the System Card is banked
; into MPR7 when an interrupt occurs.
;
; ***************************************************************************
; ***************************************************************************
;
; Bit settings for irq_vec  ...
;
;   7 : 1 to skip BIOS hsync processsing
;   6 : 1 to call [hsync_hook]
;   5 : 1 to skip BIOS vsync processsing
;   4 : 1 to call [vsync_hook]
;
;   3 : 1 to jump [nmi_hook]
;   2 : 1 to jump [timer_hook]
;   1 : 1 to jump [irq1_hook]
;   0 : 1 to jump [irq2_hook]
;
; ***************************************************************************
; ***************************************************************************

bare_irq2:	bbs0	<irq_vec, .hook		; 8 cycles if taken.

		rti				; No IRQ2 hardware on HuCard.

.hook:		jmp	[irq2_hook]		; 7 cycles.

		;

bare_irq1:	bbs1	<irq_vec, .hook		; 8 cycles if taken.

		bit	VDC_SR			; Clear VDC interrupt.
		rti

.hook:		jmp	[irq1_hook]		; 7 cycles.

		;

bare_timer_irq: bbs2	<irq_vec, .hook		; 8 cycles if taken.

		stz	IRQ_ACK			; Clear timer interrupt.
		rti

.hook:		jmp	[timer_hook]		; 7 cycles.

		;

bare_nmi_irq:	rti				; No NMI on the PC Engine!



; ***************************************************************************
; ***************************************************************************
;
; bare_clr_hooks - Reset default "CORE(not TM)" interrupt-handling hooks.
;

bare_clr_hooks:	php				; Preserve interrupt state.
		sei				; Disable interrupts.

		lda	#<bare_sw_reset		; Set up the soft-reset hook.
		sta	reset_hook + 0
		lda	#>bare_sw_reset
		sta	reset_hook + 1

		lda	#<irq1_handler		; Set up the IRQ1 hook.
		sta	irq1_hook + 0
		lda	#>irq1_handler
		sta	irq1_hook + 1

		lda	#%10100010		; Disable System Card IRQ1
		sta	<irq_vec		; processing and take over.

		plp				; Restore interrupt state.
		rts



; ***************************************************************************
; ***************************************************************************
;
; bare_sw_reset - Default HuCARD handler for a joypad "soft-reset".
;

bare_sw_reset:	sei				; Disable interrupts.

		jmp	[$FFFE]			; Jump to the HuCARD reset.



; ***************************************************************************
; ***************************************************************************
;
; irq1_handler - Basic "CORE(not TM)" IRQ1 handler to use as the "irq1_hook".
;
; Doing the IRQ1 handler processing in this hook means that things operate
; the same whether the System Card or an Overlay is paged into MPR7.
;

irq1_handler:	pha				; Save all registers.
		phx
		phy

		lda	VDC_SR			; Acknowledge the VDC's IRQ.
		sta	<vdc_sr			; Remember what caused it.

		; HSYNC ?

.check_hsync:	bbr2	<vdc_sr, .check_vsync	; Is this an HSYNC interrupt?

		bbr6	<irq_vec, .check_vsync	; Is a driver registered?

		bsr	.user_hsync		; Call game's HSYNC code.

		; END OF HSYNC HANDLER

		; VSYNC ?

.check_vsync:	bbr5	<vdc_sr, .exit_irq1	; Is this a VBLANK interrupt?

		lda	#VDC_CR			; Update the VDC's Control
		sta	VDC_AR			; Register.
		lda	<vdc_crl
		sta	VDC_DL

		inc	irq_cnt			; Mark that a VBLANK occurred.

		bbr4	<irq_vec, .skip_hookv	; Is a driver registered?

		bsr	.user_vsync		; Call game's VBLANK code.

.skip_hookv:	cli				; Allow HSYNC and TIMER IRQ.

		call	read_joypads		; Update joypad state.

		; END OF VSYNC HANDLER

.exit_irq1:
		lda	<vdc_reg		; Restore VDC_AR in case we
		sta	VDC_AR			; changed it.

		ply				; Restore all registers.
		plx
		pla

		rti				; Return from interrupt.

.user_hsync:	jmp	[hsync_hook]
.user_vsync:	jmp	[vsync_hook]



; ***************************************************************************
; ***************************************************************************
;
; The DATA_BANK location needs to be set as early as possible so that library
; code is able to put data in there before the total overall size of the code
; is known.
;
; By default, DATA_BANK is the next bank after the CORE_BANK.
;
; RESERVE_BANKS allows developers to reserve some banks between the BASE_BANK
; and the DATA_BANK that they can use however they wish.
;
; One use for RESERVE_BANKS is to create empty space that PCEAS can use when
; it relocates procedures. This provides a way for a developer to group code
; together at the start of memory, and leave the maximum number of banks for
; loading dynamic data from CD-ROM.
;
; RESERVE_BANKS is normally defined in each project's "core-config.inc".
;

	.ifndef	RESERVE_BANKS
RESERVE_BANKS	=	0
	.endif	RESERVE_BANKS

DATA_BANK	=	BASE_BANK + 1 + RESERVE_BANKS

		.zp
		.org	$2000
_temp		ds	2			; Use within any ASM routine.
_bank		ds	1			; Use within any ASM routine.

base_zp1st	=	$2003			; 1st free user address.
base_zpend	=	$20EC

base_ram1st     =	$22D0			; After the System Card!

		.bss
		.org	base_ram1st

DATA_BANK	=	1

		.data
		.bank	DATA_BANK
		.org	$6000
		.opt	d+			; Force DATA labels to MPR3.

		.code
