; ***************************************************************************
; ***************************************************************************
;
; core-kernel.asm
;
; The "CORE(not TM)" PC Engine library kernel code that runs after startup.
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
; This code is permanently located in either MPR7 (HuCARD), or MPR1 (CD-ROM),
; and it provides a consistant method of interrupt-handling on both systems
; that is designed to be compatible with the System Card.
;
; The library uses the "irq1_hook" for its own VDC interrupt handler, and PCE
; developers are expected to use "vsync_hook" and "hsync_hook" for their VDC
; interrupt functions.  Plenty of memory is available in the 1st bank for the
; developer to put those functions.
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



; ***************************************************************************
; ***************************************************************************
;
; core_kernel - Start of kernel code.
;

core_kernel	=	*



; ***************************************************************************
; ***************************************************************************
;
; Useful constants, needed by joypad library code, and used by many others.
;
; The kernel starts with a non-zero byte so that core-startup.asm can check
; whether it has already been loaded into RAM.
;

const_FFFF:	dw	$FFFF			; Useful constant for TAI.
const_0000:	dw	$0000			; Useful constant for TAI.

bit_mask:	db	$01,$02,$04,$08,$10,$20,$40,$80



; ***************************************************************************
; ***************************************************************************
;
; core_irq2  - Minimal interrupt handler compatible with System Card.
; core_irq1  - Minimal interrupt handler compatible with System Card.
; core_timer - Minimal interrupt handler compatible with System Card.
; core_rti   - Minimal interrupt handler compatible with System Card.
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

core_irq2:	bbs0	<irq_vec, .hook		; 8 cycles if using hook.

	.if	CDROM
		; N.B. Do NOT change ANY of this code without understanding
		; how and why the RTI from the System Card's IRQ2 handler
		; changes "BSR .fake" & "DB 0" into "BSR .fake" & "ORA #0"!

		pha				; Page System Card into MPR7.
		tma7
		pha
		cla
		tam7

		bsr	.fake			; Call System Card's IRQ2.
		db	0			; Becomes "ORA #0" on RTI!

		pla				; Restore caller's MPR7.
		tam7

		pla
		rti

.hook:		jmp	[irq2_hook]		; 7 cycles.

.fake:		php				; Turn BSR into a fake IRQ2
		jmp	[$FFF6]			; without fixing return PC!
	.else
		; Does this HuCARD support the IFU's ADPCM hardware?

	.if	SUPPORT_ADPCM
		pha				; Handle IFU_INT_STOP so that
		lda	IFU_IRQ_MSK		; adpcm_play() stops playback
		and	IFU_IRQ_FLG		; properly.
		bit	#IFU_INT_END
		beq	!+

		lda	#IFU_INT_END + IFU_INT_HALF
		trb	IFU_IRQ_MSK
		lda	#ADPCM_PLAY + ADPCM_AUTO
		trb	IFU_ADPCM_CTL
!:		pla
	.endif	SUPPORT_ADPCM

		rti				; No IRQ2 hardware on HuCARD.

.hook:		jmp	[irq2_hook]		; 7 cycles.
	.endif	CDROM



; ***************************************************************************
; ***************************************************************************
;
; core_irq1 - Minimal interrupt handler compatible with System Card.
;
; irq1_handler - Basic "CORE(not TM)" IRQ1 handler to use as the "irq1_hook".
;
; Doing the IRQ1 handler processing in this hook means that things operate
; the same whether the System Card or an Overlay is paged into MPR7.

	.ifndef	HUCC
		; Traditional System Card IRQ servicing delay for code that
		; was written to assume the *exact* same timing.

jump_irq1:	jmp	[irq1_hook]		; 7 cycles.

core_irq1:	;;;				; 8 (cycles for the INT)
		bbs1	<irq_vec, jump_irq1	; 8 cycles if using hook.

	.else
		; Faster IRQ servicing for HuCC and code that would like to
		; avoid the 8 cycles used by the "bbs1" instruction that is
		; taken when the System Card is mapped into MPR7.

core_irq1:	;;;				; 8 (cycles for the INT)
	.if	CDROM || !defined(NO_CORE_IRQ1_HOOK)
		jmp	[irq1_hook]		; 7 cycles.

	.endif
	.endif

irq1_handler:	pha				; 3 Save all registers.
		phx				; 3
		phy				; 3

	.ifndef	USING_RCR_MACROS		;   This slows things down
	.if	CDROM				;   if using macros, so do
	.if	USING_MPR7			;   it only for VBLANK.
		tma7				; 4 Preserve MPR7.
		pha				; 3
		lda	<core_1stbank		; 4 Allow users to put IRQ
		tam7				; 5 handlers in CORE_BANK.
	.endif
	.endif	CDROM
	.endif	USING_RCR_MACROS

		lda	VDC_SR			; 6 Acknowledge the VDC's IRQ.
		sta	<vdc_sr			; 4 Remember what caused it.

	.if	SUPPORT_SGX
		ldx	SGX_SR			; 6 Read SGX_SR after VDC_SR in
		stx	<sgx_sr			; 4 case this is not an SGX!
	.endif

		; Handle the VDC's RCR interrupt.

!:		and	#$04			; 2 Is this an HSYNC interrupt?
		beq	!+			; 2

	.ifdef	USING_RCR_MACROS
		VDC_RCR_MACRO
	.else
		jsr	.user_hsync_vdc		; 7 Call game's HSYNC code.
	.endif

	.if	SUPPORT_SGX

		; Handle the SGX's RCR interrupt.

!:		bbr2	<sgx_sr, !+		; 6 Is this an HSYNC interrupt?

	.ifdef	USING_RCR_MACROS
		SGX_RCR_MACRO
	.else
		jsr	.user_hsync_sgx		; 7 Call game's HSYNC code.
	.endif

	.endif	SUPPORT_SGX

		; Handle the VDC's VBL interrupt.

!:		bbr5	<vdc_sr, .exit_irq1	; 6 Is this a VBLANK interrupt?

	.ifdef	USING_RCR_MACROS		;   If we didn't do it earlier
	.if	CDROM				;   then we need to do it now.
	.if	USING_MPR7
		tma7				; 4 Preserve MPR7.
		pha				; 3
		lda	<core_1stbank		; 4 Allow users to put IRQ
		tam7				; 5 handlers in CORE_BANK.
	.endif
	.endif	CDROM
	.endif	USING_RCR_MACROS

	.if	SUPPORT_SGX
		lda	#VDC_CR			; Update the SGX's Control
		sta	SGX_AR			; Register first, just in
		sta	VDC_AR			; case this is not an SGX!
		lda	<sgx_crl		
		sta	SGX_DL
		lda	<vdc_crl
		sta	VDC_DL

;		lda	<sgx_crh		; Do not mess with the SGX's
;		sta	SGX_DH			; auto-increment!!!
;		lda	<vdc_crh		; Do not mess with the VDC's
;		sta	VDC_DH			; auto-increment!!!
	.else
		lda	#VDC_CR			; Update the VDC's Control
		sta	VDC_AR			; Register.
		lda	<vdc_crl
		sta	VDC_DL

;		lda	<vdc_crh		; Do not mess with the VDC's
;		sta	VDC_DH			; auto-increment!!!
	.endif	SUPPORT_SGX

		inc	irq_cnt			; Mark that a VBLANK occurred.

		bbr4	<irq_vec, .skip_hookv	; Is a driver registered?

		bsr	.user_vsync		; Call game's VBLANK code.

.skip_hookv:	bbs5	<irq_vec, .exit_vbl	; Should we skip "BIOS" stuff?

		cli				; Allow HSYNC and TIMER IRQ.

		call	read_joypads		; Update joypad state.

	.if	USING_PSGDRIVER
		lda	<main_sw		; Is the PSG driver running in
		cmp	#1			; VBLANK 60Hz update mode?
		bne	.exit_vbl

		lda	#$80			; Acquire sound mutex to avoid
		tsb	sound_mutex		; conflict with a delayed VBL.
		bmi	.exit_vbl

		cla				; Call the BIOS PSG driver in
		tam7				; the System Card.
		jsr	psg_driver

		stz	sound_mutex		; Release sound mutex.
	.else
		lda	sound_hook + 1		; Is a driver registered?
		beq	.exit_vbl

		lda	#$80			; Acquire sound mutex to avoid
		tsb	sound_mutex		; conflict with a delayed VBL.
		bmi	.exit_vbl

		bsr	.user_sound		; Call the driver hook.

		stz	sound_mutex		; Release sound mutex.
	.endif	USING_PSGDRIVER

.exit_vbl:

	.ifdef	USING_RCR_MACROS		; If USING_RCR_MACROS then 
	.if	CDROM				; restore after the VBLANK.
	.if	USING_MPR7
		pla				; Restore original MPR7.
		tam7
	.endif
	.endif	CDROM
	.endif	USING_RCR_MACROS

.exit_irq1:

	.ifndef	USING_RCR_MACROS		; If !USING_RCR_MACROS then
	.if	CDROM				; restore at IRQ1 exit.
	.if	USING_MPR7
		pla				; Restore original MPR7.
		tam7
	.endif
	.endif	CDROM
	.endif	USING_RCR_MACROS

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

.user_vsync:	jmp	[vsync_hook]		; 7
.user_sound:	jmp	[sound_hook]		; 7

	.ifndef	USING_RCR_MACROS
.user_hsync_vdc:jmp	[hsync_hook]		; 7
	.if	SUPPORT_SGX
.user_hsync_sgx:jmp	[hsync_hook_sgx]	; 7
	.endif
	.endif	USING_RCR_MACROS



; ***************************************************************************
; ***************************************************************************
;
; core_timer - Minimal interrupt handler compatible with System Card.
;
; tirq_handler - Basic "CORE(not TM)" TIRQ handler to use as the "timer_hook".
;
; Doing the TIRQ handler processing in this hook means that things operate
; the same whether the System Card or an Overlay is paged into MPR7.

	.ifndef	HUCC
		; Traditional System Card IRQ servicing delay for code that
		; was written to assume the *exact* same timing.

jump_timer:	jmp	[timer_hook]		; 7 cycles.

core_timer:	;;;				; 8 (cycles for the INT)
		bbs2	<irq_vec, jump_timer	; 8 cycles if using hook.

	.else
		; Faster IRQ servicing for HuCC and code that would like to
		; avoid the 8 cycles used by the "bbs2" instruction that is
		; taken when the System Card is mapped into MPR7.

core_timer:	;;;				; 8 (cycles for the INT)
	.if	CDROM || !defined(NO_CORE_TIRQ_HOOK)
		jmp	[timer_hook]		; 7 cycles.
	.endif
	.endif

tirq_handler:
	.ifdef	USING_TIRQ_MACRO
		TIMER_IRQ_MACRO
	.else
		stz	IRQ_ACK			; 5 Clear timer interrupt.
	.endif

core_rti:	rti



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
		stz.h	sound_hook		; Disable sound driver calls.
	.endif

		lda	#<core_sw_reset		; Set up the soft-reset hook.
		sta.l	reset_hook
		lda	#>core_sw_reset
		sta.h	reset_hook

		lda	#<irq1_handler		; Set up the IRQ1 hook.
		sta.l	irq1_hook
		lda	#>irq1_handler
		sta.h	irq1_hook

		lda	#<tirq_handler		; Set up the TIRQ hook.
		sta.l	timer_hook
		lda	#>tirq_handler
		sta.h	timer_hook

		lda	#%00000110		; Replace the System Card's
		sta	<irq_vec		; IRQ1 and TIRQ processing.

		plp				; Restore interrupt state.
		rts



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
; core_sw_reset - Default HuCARD handler for a joypad "soft-reset".
;

core_sw_reset:	sei				; Disable interrupts.

		jmp	[$FFFE]			; Jump to the HuCARD reset.



	.else	!CDROM

; ***************************************************************************
; ***************************************************************************
;
; core_sw_reset - Default CD-ROM handler for a joypad "soft-reset".
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
; N.B. This is designed to work even if System Card is already in MPR7.
;

call_bios:	sty	.self_mod_func + 1	; Which System Card function?

		pha				; Page System Card into MPR7.
		tma7
		sta	.self_mod_bank + 1	; Preserve caller's MPR7.
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
; get_file_lba	- Get just a file's starting sector from the directory.
;
; Args: X reg = File #.
;
; ***************************************************************************
; ***************************************************************************

get_file_info:	sec				; Calculate file length in
		lda	iso_dirlo + 1, x	; sectors.
		sbc	iso_dirlo + 0, x
		sta	<_al
		lda	iso_dirhi + 1, x
		sbc	iso_dirhi + 0, x
		sta	<_ah

get_file_lba:	cla				; Set file start location.
		cpx	iso_128mb
		bcc	.set_rec_h
		inc	a
.set_rec_h:	sta	<_cl
		lda	iso_dirhi + 0, x
		sta	<_ch
		lda	iso_dirlo + 0, x
		sta	<_dl

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

exec_overlay:	jsr	core_clr_hooks		; Reset default hooks.

.retry:		phx				; Preserve file number.

		cla				; Page System Card into MPR7.
		tam7

		cli				; Enable interrupts for load.

		jsr	get_file_info		; Set CD-ROM sector & length.

		lda	<core_1stbank		; Set first bank to load.
		sta	<_bl
		stz	<_bh

		lda	#6			; Use MPR6 for loading.
		sta	<_dh

		jsr	cd_read			; Load the file from CD.

		plx				; Restore file number.

		cmp	#0			; Was there an error?
		bne	.retry			; Retry forever!

.execute_game:	lda	<core_1stbank		; Reset bank mapping back to
		tam2				; its "CORE(not TM)" default.
		inc	a			; This is not done earlier to
		tam3				; avoid the System Card error
		inc	a			; handling bug which causes a
		tam4				; cd_read to an MPRn to leave
		inc	a			; the MPRn value corrupted.
		tam5
		inc	a
		tam6

		jmp	$4000			; Execute the overlay.

	.endif	!CDROM



; ***************************************************************************
; ***************************************************************************

	.if	(core_kernel >= $4000)		; If not running in RAM, then
		.bss				; put these variables in RAM.
	.endif

sound_mutex:	ds	1			; NZ when controller port busy.

	.if	SUPPORT_SGX
sgx_detected:	ds	1			; NZ if SuperGrafx detected.
hsync_hook_sgx:	ds	2			; SGX version of hsync_hook.
	.endif

	.if	SUPPORT_ACD
acd_detected:	ds	1			; NZ if ArcadeCard detected.
	.endif

	.if	(core_kernel >= $4000)
		.code
	.endif
