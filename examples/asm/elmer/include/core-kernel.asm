; ***************************************************************************
; ***************************************************************************
;
; core-kernel.asm
;
; The "CORE(not TM)" PC Engine library kernel code that runs after startup.
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
; This code is permanently located in either MPR7 (HuCARD), or MPR1 (CD-ROM),
; and it provides a consistant method of interrupt-handling on both systems
; that is designed to be compatible with the System Card.
;
; The library uses the "irq1_hook" for its own VDC interrupt handler, and PCE
; developers are expected to use "vsync_hook" and "hsync_hook" for their own
; VDC interrupt functions.  Plenty of memory is available in the 1st bank for
; the developer to put those functions.
;
; The reason for using "irq1_hook", is so that the library can work properly
; if either the current overlay program, or the System Card, are mapped into
; MPR7 whenever an interrupt occurs.
;
; The VDC interupt handler itself is changed from the System Card's handler,
; and it is designed to provide faster response to vsync_hook, and to enable
; interrupts during the slow(ish) joypad and sound driver code, so that both
; raster and timer interrupts are not delayed.
;
; Developers are free to enable interrupts during their own vsync_hook code,
; if they wish to do so.
;
; On CD-ROM systems, this library kernel also provides a function to load and
; run a new overlay program, without relying upon any of the code/data within
; the current overlay program's memory (unlike HuC v3).
;
; ***************************************************************************
; ***************************************************************************



	.if	USING_NEWPROC

; ***************************************************************************
; ***************************************************************************
;
; leave_proc - Code used by ".proc/.endp" functions to return to the caller.
;
; This restores the original bank that was in MPR6 when the ".proc" function
; was called.
;

leave_proc:	pla
		tma6
		rts

	.endif	USING_NEWPROC



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
; core_irq2	 - Minimal System Card compatible interrupt handler stub.
; core_irq1	 - Minimal System Card compatible interrupt handler stub.
; core_timer_irq - Minimal System Card compatible interrupt handler stub.
; core_nmi_irq	 - Minimal System Card compatible interrupt handler stub.
;
; Note that it takes 8 cycles to respond to an IRQ.
;
; These routines are copied to a location in RAM that does not vary when
; a different overlay program is loaded.
;
; All overlay programs are set up to vector interrupts to these routines,
; which is designed to avoid interrupt-related crashes when loading a new
; overlay on top of an old overlay in memory.
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

core_irq2:	bbs0	<irq_vec, .hook		; 8 cycles if taken.

	.if	CDROM
		pha				; Page System Card into MPR7.
		tma7
		pha
		cla
		tam7

		bsr	.fake			; Call System Card's IRQ2.

		pla				; Restore caller's MPR7.
		tam7
		pla
		rti

.fake:		php				; The System Card's ADPCM
		jmp	[$FFF6]			; & CD code uses this IRQ.
	.else
		rti				; No IRQ2 hardware on HuCard.
	.endif	CDROM

.hook:		jmp	[irq2_hook]		; 7 cycles.

		;

core_irq1:	bbs1	<irq_vec, .hook		; 8 cycles if taken.

		bit	VDC_SR			; Clear VDC interrupt.
		rti

.hook:		jmp	[irq1_hook]		; 7 cycles.

		;

core_timer_irq: bbs2	<irq_vec, .hook		; 8 cycles if taken.

		stz	IRQ_ACK			; Clear timer interrupt.
		rti

.hook:		jmp	[timer_hook]		; 7 cycles.

		;

core_nmi_irq:	rti				; No NMI on the PC Engine!



; ***************************************************************************
; ***************************************************************************
;
; core_clr_hooks - Reset default "CORE(not TM)" interrupt-handling hooks.
;

core_clr_hooks:	php				; Preserve interrupt state.
		sei				; Disable interrupts.

	.if	USING_PSGDRIVER
		lda	#$80			; Disable sound driver calls.
		sta	<main_sw
	.else
		stz	sound_hook + 1		; Disable sound driver calls.
	.endif

		lda	#<core_sw_reset		; Set up the soft-reset hook.
		sta	reset_hook + 0
		lda	#>core_sw_reset
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

	.if	SUPPORT_SGX
		lda	#VDC_CR			; Update the SGX's Control
		sta	SGX_AR			; Register first, just in
		lda	<sgx_crl		; case this is not an SGX!
		sta	SGX_DL
;		lda	<sgx_crh		; Do not mess with the SGX's
;		sta	SGX_DH			; auto-increment!!!
	.endif

		lda	#VDC_CR			; Update the VDC's Control
		sta	VDC_AR			; Register.
		lda	<vdc_crl
		sta	VDC_DL
;		lda	<vdc_crh		; Do not mess with the VDC's
;		sta	VDC_DH			; auto-increment!!!

		inc	irq_cnt			; Mark that a VBLANK occurred.

;		inc	rndseed			; Keep random ticking over.

		bbr4	<irq_vec, .skip_hookv	; Is a driver registered?

		bsr	.user_vsync		; Call game's VBLANK code.

.skip_hookv:	cli				; Allow HSYNC and TIMER IRQ.

		call	read_joypads		; Update joypad state.

	.if	USING_PSGDRIVER
		lda	<main_sw		; Is the PSG driver running in
		cmp	#1			; VBLANK 60Hz update mode?
		bne	.exit_irq1

		lda	#$80			; Acquire sound mutex to avoid
		tsb	sound_mutex		; conflict with a delayed VBL.
		bmi	.exit_irq1

		tma7				; Call the BIOS PSG driver in
		pha				; the System Card.
		cla
		tam7
		jsr	psg_driver
		pla
		tam7

		stz	sound_mutex		; Release sound mutex.
	.else
		lda	sound_hook + 1		; Is a driver registered?
		beq	.exit_irq1

		lda	#$80			; Acquire sound mutex to avoid
		tsb	sound_mutex		; conflict with a delayed VBL.
		bmi	.exit_irq1

		bsr	.user_sound		; Call the driver hook.

		stz	sound_mutex		; Release sound mutex.
	.endif	USING_PSGDRIVER

		; END OF VSYNC HANDLER

.exit_irq1:
	.if	SUPPORT_SGX
		lda	<sgx_reg		; Restore SGX_AR first, just
		sta	SGX_AR			; in case this is not an SGX!
	.endif

		lda	<vdc_reg		; Restore VDC_AR in case we
		sta	VDC_AR			; changed it.

		ply				; Restore all registers.
		plx
		pla

		rti				; Return from interrupt.

.user_hsync:	jmp	[hsync_hook]
.user_vsync:	jmp	[vsync_hook]
.user_sound:	jmp	[sound_hook]

	.if	(* >= $E000)			; If not running in RAM, then
		.bss				; put the variables in RAM.
	.endif

sound_mutex:	ds	1			; NZ when controller port busy.

		.code



; ***************************************************************************
; ***************************************************************************
;
; Include the joypad library, with configuration from "core-config.inc".
;

		include "joypad.asm"



	.if	!CDROM

; ***************************************************************************
; ***************************************************************************
;
; core_sw_reset - Default handler for a joypad "soft-reset" button press.
;

core_sw_reset:	sei				; Disable interrupts.

		jmp	[$FFFE]			; Jump to the HuCARD reset.



	.else	!CDROM

; ***************************************************************************
; ***************************************************************************
;
; core_sw_reset - Default handler for a joypad "soft-reset" button press.
;

core_sw_reset:	sei				; Disable interrupts.

		cly				; Perform a BIOS soft-reset.
;		bra	call_bios		; Fall through ...



; ***************************************************************************
; ***************************************************************************
;
; call_bios - Page the System Card into MPR7 and call a function.
;
; Args: Y reg = lo-byte of address of System Card function to call.
;
; Uses: Whatever the System Card function uses!
;
; N.B. Because the Y register is used, this cannot call "ex_setvec", so the
;      "setvec" macro is provided to perform the same function.
;

call_bios:	sty	.self_mod_func + 1	; Which System Card function?

		pha				; Page System Card into MPR7.
		tma7
		sta	.self_mod_bank
		cla
		tam7
		pla

.self_mod_func: jsr	$E000			; Call System Card vector.

		pha				; Restore caller's MPR7.
.self_mod_bank:	lda	#$00
		tam7
		pla

		rts



; ***************************************************************************
; ***************************************************************************
;
; get_file_info - Get a file's starting sector and length from the directory.
;
; Args: X reg = File #.
;
; ***************************************************************************
; ***************************************************************************

get_file_info:	sec				; Calculate file length.
		lda	iso_dirlo + 1, x
		sbc	iso_dirlo + 0, x
		sta	<__al
		lda	iso_dirhi + 1, x
		sbc	iso_dirhi + 0, x
		sta	<__ah

		cla				; Set file start location.
		cpx	iso_128mb
		bcc	.set_rec_h
		inc	a
.set_rec_h:	sta	<__cl
		lda	iso_dirhi + 0, x
		sta	<__ch
		lda	iso_dirlo + 0, x
		sta	<__dl

		rts



; ***************************************************************************
; ***************************************************************************
;
; exec_overlay - Load and execute an overlay from the CD-ROM.
;
; Args: X reg = Overlay # to load and run.
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

exec_overlay:	phx				; Preserve file number.

		jsr	core_clr_hooks		; Reset default hooks.

		cla				; Page System Card into MPR7.
		tam7

		cli				; Enable interrupts for load.

		jsr	get_file_info		; Set CD-ROM sector & length.

		lda	<core_1stbank		; Set first bank to load.
		sta	<__bl
		stz	<__bh

		tam2				; Reset bank mapping back to
		inc	a                       ; its "CORE(not TM)" default.
		tam3
		inc	a
		tam4
		inc	a
		tam5
		inc	a
		tam6

		lda	#6			; Use MPR6 for loading.
		sta	<__dh

		jsr	cd_read			; Load the file from CD.

		plx				; Restore file number.

		cmp	#0			; Was there an error?
		bne	exec_overlay		; Retry forever!

.execute_game:	jmp	$4000			; Execute the overlay.

	.endif	!CDROM



; ***************************************************************************
; ***************************************************************************
;
; core_ramcpy - End of code to relocate.
;

core_ramcpy	=	*
