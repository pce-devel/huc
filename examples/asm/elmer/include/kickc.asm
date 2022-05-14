; ***************************************************************************
; ***************************************************************************
;
; kickc.asm
;
; Wrapper for using KickC with the "CORE(not TM)" PC Engine library code.
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
; 1) If we're running on a HuCard, the initialization is simple!
;
;    The PC Engine's memory map is set to ...
;
;      MPR0 = bank $FF : PCE hardware
;      MPR1 = bank $F8 : PCE RAM with ZP & Stack (BSS segment)
;      MPR2 = bank $87 : SGX RAM or CD RAM
;      MPR3 = bank $xx : Freely mapped data, init to $02 (DATA segment)
;      MPR4 = bank $xx : Freely mapped data, init to $03
;      MPR5 = bank $01 : KickC permanent code & data (CODE segment)
;      MPR6 = bank $xx : Banked ASM library & KickC procedures
;      MPR7 = bank $00 : CORE(not TM) base code & call-trampolines
;
;
; 2) If we're running on a HuCard that supports the Turbo Everdrive, then the
;    first bank is reserved for mapping the TED2 hardware.
;
;    The PC Engine's memory map is set to ...
;
;      MPR0 = bank $FF : PCE hardware
;      MPR1 = bank $F8 : PCE RAM with ZP & Stack (BSS segment)
;      MPR2 = bank $01 : TED2 RAM
;      MPR3 = bank $xx : Freely mapped data, init to $03 (DATA segment)
;      MPR4 = bank $xx : Freely mapped data, init to $04
;      MPR5 = bank $03 : KickC permanent code & data (CODE segment)
;      MPR6 = bank $xx : Banked ASM library & KickC procedures
;      MPR7 = bank $02 : CORE(not TM) base code & call-trampolines
;
;
; 3) If we're running on an old CD System, the overlay is loaded from the ISO
;    into banks $80-$87 (64KB max).
;
;    The PC Engine's memory map is set to ...
;
;      MPR0 = bank $FF : PCE hardware
;      MPR1 = bank $F8 : PCE RAM & CORE(not TM) kernel (BSS segment)
;      MPR2 = bank $87 : SGX RAM or CD RAM
;      MPR3 = bank $xx : Freely mapped data, init to $82 (DATA segment)
;      MPR4 = bank $xx : Freely mapped data, init to $83
;      MPR5 = bank $81 : KickC permanent code & data (CODE segment)
;      MPR6 = bank $xx : Banked ASM library & KickC procedures
;      MPR7 = bank $80 : CORE(not TM) base code & call-trampolines
;
;
; 4) If we're running on a SuperCD System, the overlay is loaded from the ISO
;    into banks $68-$87 (256KB max).
;
;    The PC Engine's memory map is set to ...
;
;      MPR0 = bank $FF : PCE hardware
;      MPR1 = bank $F8 : PCE RAM & CORE(not TM) kernel (BSS segment)
;      MPR2 = bank $87 : SGX RAM or CD RAM
;      MPR3 = bank $xx : Freely mapped data, init to $6A (DATA segment)
;      MPR4 = bank $xx : Freely mapped data, init to $6B
;      MPR5 = bank $69 : KickC permanent code & data (CODE segment)
;      MPR6 = bank $xx : Banked ASM library & KickC procedures
;      MPR7 = bank $68 : CORE(not TM) base code & call-trampolines
;
; ***************************************************************************
; ***************************************************************************

		; Tell the libraries that this is KickC code.

_KICKC		=	1

		; Include the library, reading the project's configuration
		; settings from the local "core-config.inc", if it exists.

		include "core.inc"		; Basic includes.

		.list
		.mlist

		include	"common.asm"		; Common helpers.
		include	"vdc.asm"		; Useful VDC routines.
		include	"font.asm"		; Useful font routines.
		include	"random.asm"		; Random number generator.

		include	"huc-gfx.asm"		; HuC library replacements.



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

core_main:	tma7				; Get the CORE_BANK.

		inc	a			; MPR0 = $FF
		tam6                            ; MPR1 = PCE_RAM
;		inc	a                       ; MPR2 = SGX_RAM or CD_RAM
		tam5                            ; MPR3 = CORE_BANK + 2
		inc	a                       ; MPR4 = CORE_BANK + 3
		tam3                            ; MPR5 = CORE_BANK + 1
		inc	a                       ; MPR6 = CORE_BANK + 1
		tam4                            ; MPR7 = CORE_BANK + 0
	.if	SUPPORT_SGX
		lda	#$F9			; Map the 2nd SGX RAM bank.
	.else
		lda	#$87			; Map the last CD RAM bank.
	.endif
		tam2

	.if	CDROM				; Overlays should clear BSS.

		php				; Disable interrupts while
		sei				; clearing overlay's BSS.

		lda	#<kickc_vbl		; Setup KickC's vblank IRQ
		sta.l	vsync_hook		; handler.
		lda	#>kickc_vbl
		sta.h	vsync_hook

		tai	const_0000, core_ramend, (_bss_end - core_ramend)

		lda	VDC_SR			; Purge any overdue VBL.
		stz	irq_cnt			; Make it easy to check.

		plp				; Restore interrupts.

	.else					; HuCARD clears RAM on boot.

		lda	#<kickc_vbl		; Setup KickC's vblank IRQ
		sta.l	vsync_hook		; handler.
		lda	#>kickc_vbl
		sta.h	vsync_hook

	.endif	CDROM

		lda	#$10			; Enable KickC's vblank IRQ
		tsb	<irq_vec		; handler.

		call	init_random		; Initialize random seed.

		call	main			; Execute the KickC program.

.hang:		jmp	.hang			; Why did it return???



; ***************************************************************************
; ***************************************************************************
;
; kickc_vbl - vblank IRQ handler, called by the "CORE(not TM)" kernel.
;
; This uses the kernel's hook to process things during vblank. It returns
; with a simple RTS, and it can corrupt any register.
;
; Any slow routines in here should enable interrupts and protect itself from
; re-entrancy problems.
;

kickc_vbl:	jmp	xfer_palettes		; Upload any palette changes.



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

		.code				; KickC permanent code and
		.bank	CORE_BANK + 1		; initialized data runs in
		.org	$A000			; MPR5 (.section CODE).

		include	"fastmath.asm"		; 2KBytes of fast-multiply.

		.code				; Start the KickC code ...
