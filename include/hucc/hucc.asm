; ***************************************************************************
; ***************************************************************************
;
; hucc.asm
;
; Wrapper for using HuCC with the "CORE(not TM)" PC Engine library code.
;
; Copyright John Brandwood 2024-2025.
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
;      MPR3 = bank $xx : HuCC const data, init to $02 (DATA segment)
;      MPR4 = bank $xx : HuCC const data, init to $03
;      MPR5 = bank $01 : HuCC permanent code & data (HOME segment)
;      MPR6 = bank $xx : Banked ASM library & HuCC procedures
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
;      MPR3 = bank $xx : HuCC const data, init to $04 (DATA segment)
;      MPR4 = bank $xx : HuCC const data, init to $05
;      MPR5 = bank $03 : HuCC permanent code & data (HOME segment)
;      MPR6 = bank $xx : Banked ASM library & HuCC procedures
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
;      MPR3 = bank $xx : HuCC const data, init to $82 (DATA segment)
;      MPR4 = bank $xx : HuCC const data, init to $83
;      MPR5 = bank $81 : HuCC permanent code & data (HOME segment)
;      MPR6 = bank $xx : Banked ASM library & HuCC procedures
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
;      MPR3 = bank $xx : HuCC const data, init to $6A (DATA segment)
;      MPR4 = bank $xx : HuCC const data, init to $6B
;      MPR5 = bank $69 : HuCC permanent code & data (HOME segment)
;      MPR6 = bank $xx : Banked ASM library & HuCC procedures
;      MPR7 = bank $68 : CORE(not TM) base code & call-trampolines
;
; ***************************************************************************
; ***************************************************************************

		; Set the PCEAS options that HuCC code needs.

		.3pass				; Commit to running 3-passes.
		.opt	a+			; Enable auto-zp addressing.
		.opt	b+			; Fix out-of-range branches.

		; Tell the libraries that this is HuCC code.

HUCC		=	1

		; Read the project's HuCC configuration settings from the
		; local "hucc-config.inc", if it exists, before CORE.
		;
		; These settings configure the asm libraries for HuCC code,
		; which should be independent of whatever base library acts
		; as the foundation for the overall code structure.

		include "hucc-config.inc"	; Allow for HuCC customization.

		; FAST_MULTIPLY implies that we need a bank to put the data.
		;
		; This is undefined if someone has an old "hucc-config.inc".

	.ifndef	FAST_MULTIPLY
FAST_MULTIPLY	=	0
	.else
	.if	FAST_MULTIPLY
	.ifndef	NEED_HOME_BANK
NEED_HOME_BANK	=	1
	.endif
	.endif
	.endif

		; This is where you can change which asm library forms the
		; foundation structure.

	.ifndef	HUCC_NO_CORE

		; Use the CORE library, reading the project's configuration
		; settings from the local "core-config.inc", if it exists.

		include "core.inc"		; Foundation library.
	.else

		; Use a simple PCE-only and HuCARD-only foundation that is.
		; easy to understand and modify.

		include "bare-startup.asm"	; Foundation library.
	.endif

		; Allocate this as early as possible to ensure bank-aligned
		; so that there is no wasted space for aligning the table.

	.if	FAST_MULTIPLY
	.ifndef	HOME_BANK
		.fail	FAST_MULTIPLY is set but there is no HOME_BANK!
	.endif
		.home
		include	"squares.asm"
		.code
	.endif

		;

		.list
		.mlist

		; The hardware stack is used for expressions.

__tos		=	$F8:2101, 255

		.zp
		.align	2
__fptr:		ds	2
__fbank:	ds	1
__sp:		ds	1
__stack:	ds	HUCC_STACK_SZ
__ptr:		ds	2

		; HuCC's non-recursive consecutive varargs for printf().

__vararg1	=	__stack + 0
__vararg2	=	__stack + 2
__vararg3	=	__stack + 4
__vararg4	=	__stack + 6

		; Pointer used by poke() because __ptr could be overwritten.

__poke		=	__si

		; Used for indirect calls because __ptr could be overwritten.

__func		=	__si

		; Data pointer used by SDCC for indirect indexed memory access.

DPTR		=	__ptr

		; REGTEMP 8-byte stack for temporaries used by SDCC.
		; Keep the size in sync with NUM_TEMP_REGS in mos6502/gen.c!

REGTEMP:	ds	8

		; Values returned from SDCC functions that don't fit into XA.
		; These are also used as workspace for SDCC library functions,
		; including HuCC's multiplication and division functions.

___SDCC_m6502_ret0:	ds	1
___SDCC_m6502_ret1:	ds	1
___SDCC_m6502_ret2:	ds	1
___SDCC_m6502_ret3:	ds	1

	.if	0
___SDCC_m6502_ret4:	ds	1
___SDCC_m6502_ret5:	ds	1
___SDCC_m6502_ret6:	ds	1
___SDCC_m6502_ret7:	ds	1
	.endif

		; Permanent pointers for fast table-of-squares multiplication.

	.if	FAST_MULTIPLY
mul_sqrplus_lo:	ds	2
mul_sqrplus_hi:	ds	2
mul_sqrminus_lo:ds	2
mul_sqrminus_hi:ds	2
	.endif

		; HuCC keeps a realtime clock, updated in hucc_vbl.
		;
		; Defining this here means that it will go before any HuCC
		; variables in "globals.h", and so it won't get cleared in
		; a CDROM game when loading different overlays.

		.bss
clock_hh:	ds	1			; System Clock, hours	(0-11)
clock_mm:	ds	1			; System Clock, minutes (0-59)
clock_ss:	ds	1			; System Clock, seconds (0-59)
clock_tt:	ds	1			; System Clock, ticks	(0-59)
		.code

		; Critical HuCC libraries that the compiler depends upon.
		;
		; These include various macros that must be defined before
		; they are encountered in any compiler-generated code.

		include "hucc-codegen.asm"	; HuCC i-code macros and funcs.
		include	"hucc-baselib.asm"	; HuCC base library macros.

	.if	CDROM
		include	"hucc-systemcard.asm"	; HuCC System Card macros.
	.endif

		; Definitions for compatibility with old HuC/MagicKit projects.

	.ifndef	HUCC_NO_DEPRECATED
		include	"hucc-deprecated.inc"
	.endif

		;
		;
		;



; ***************************************************************************
; ***************************************************************************
;
; core_main - This is executed after "CORE(not TM)" library initialization.
;
; This is the first code assembled after the library includes, so we're still
; in the CORE_BANK, usually ".bank 0"; and because this is assembled with the
; default configuration from "include/core-config.inc", which sets the option
; "USING_MPR7", then we're running in MPR7 ($E000-$FFFF).

core_main	.proc

	.if	SUPPORT_SGX
		lda	#$F9			; Map the 2nd SGX RAM bank.
	.else
		lda	#$87			; Map the last CD RAM bank.
	.endif
		tam2

		lda	#CONST_BANK + _bank_base; Map HuCC's constant data.
		tam3
		inc	a
		tam4

	.ifdef	HOME_BANK
		lda	#HOME_BANK + _bank_base	; Map HuCC's .HOME bank.
	.else
		inc	a
	.endif
		tam5

		php				; Disable interrupts while
		sei				; clearing overlay's BSS.

	.ifndef	USING_RCR_MACROS
	.ifdef	hucc_rcr
		lda	#<hucc_rcr		; Setup HuCC's RCR IRQ handler
		sta.l	hsync_hook		; if not USING_RCR_MACROS.
		lda	#>hucc_rcr
		sta.h	hsync_hook
	.endif
	.if	SUPPORT_SGX
	.ifdef	hucc_rcr_sgx
		lda	#<hucc_rcr_sgx		; Setup HuCC's RCR IRQ handler
		sta.l	hsync_hook_sgx		; if not USING_RCR_MACROS.
		lda	#>hucc_rcr_sgx
		sta.h	hsync_hook_sgx
	.endif
	.endif
	.endif	USING_RCR_MACROS

		lda	#<hucc_vbl		; Setup HuCC's VBL IRQ handler.
		sta.l	vsync_hook
		lda	#>hucc_vbl
		sta.h	vsync_hook

	.if	CDROM				; Overlays should clear BSS.
		tai	const_0000, huc_globals_end, (_bss_end - huc_globals_end)
	.else
		tii	.rom_tia, ram_tia, 16	; Only needed on HuCARD.
	.endif	CDROM

	.ifdef	HAVE_INIT			; This method needs to be replaced!
		tii	huc_rodata, huc_data, huc_rodata_end - huc_rodata
	.endif	HAVE_INIT

		tai	.stack_fill, __stack, HUCC_STACK_SZ

	.if	FAST_MULTIPLY
		lda.h	#square_plus_lo		; Initialize the fast multiply
		sta.h	<mul_sqrplus_lo		; pointers.
		lda.h	#square_plus_hi
		sta.h	<mul_sqrplus_hi
		lda.h	#square_minus_lo
		sta.h	<mul_sqrminus_lo
		lda.h	#square_minus_hi
		sta.h	<mul_sqrminus_hi
	.endif

		lda	#$10			; Enable HuCC's vblank IRQ
		tsb	<irq_vec		; handler.

		__sound_init			; Initialize a sound driver.

		lda	VDC_SR			; Purge any overdue VBL.
		stz	irq_cnt			; Make it easy to check.

		plp				; Restore interrupts.

	.ifndef	HUCC_NO_DEFAULT_SCREEN
		call	_set_256x224		; HuCC initializes the VDC.
	.endif

	.ifndef	HUCC_NO_DEFAULT_FONT
		lda	#1			; HuCC loads a default font.
		sta	monofont_fg
		stz	monofont_bg
		_load_default_font
		lda	#$01			; Set the font palette entry to
		sta.l	VCE_CTA			; cyan which is a) visible, but
		stz.h	VCE_CTA			; b) a clear indicator that the
		ldy	#$96			; user hasn't set a palette yet.
		sty.l	VCE_CTW
		sta.h	VCE_CTW
	.endif

	.ifndef	HUCC_NO_DEFAULT_RANDOM
		ldy	irq_cnt			; Initialize random seed.
		call	init_random
	.endif

		ldx	#HUCC_STACK_SZ		; Initialize the HuCC stack.
		stx	<__sp

		call	_main			; Execute the HuCC program.

		jmp	_exit.1			; Pass the exit code on.

.stack_fill:	db	$EA,$EA			; To make it easier to see.

	.if	!CDROM

.rom_tia:	tia	$1234, VDC_DL, 32
		rts

.rom_tii:	tii	$1234, $5678, $9ABC
		rts

		.bss
ram_tia:	ds	8
ram_tii:	ds	8
		.code

	.endif	!CDROM

		.endp

		;
		; Self-Modifying TIA and TII instruction subroutines.
		;
		; These need to be in permanently-accessible memory so that
		; HuCC code can modify the values as __fastcall parameters.
		;

	.if	CDROM

ram_tia:	tia	$1234, VDC_DL, 32
		rts

ram_tii:	tii	$1234, $5678, $9ABC
		rts

	.endif	CDROM

		rsset	ram_tia
ram_tia_opc	rs	1
ram_tia_src	rs	2
ram_tia_dst	rs	2
ram_tia_len	rs	2
ram_tia_rts	rs	1

		rsset	ram_tii
ram_tii_opc	rs	1
ram_tii_src	rs	2
ram_tii_dst	rs	2
ram_tii_len	rs	2
ram_tii_rts	rs	1



; ***************************************************************************
; ***************************************************************************
;
; hucc_vbl - vblank IRQ handler, called by the "CORE(not TM)" kernel.
;
; This uses the kernel's hook to process things during vblank. It returns
; with a simple RTS, and it can corrupt any register.
;
; Any slow routines in here should enable interrupts and protect itself from
; re-entrancy problems.
;

hucc_vbl:	call	vbl_init_scroll		; Prepare for the next frame.

		sed				; Update the HuC system clock
		sec				; which is in BCD here rather
		lda	clock_tt		; than the binary one in HuC.
		adc	#0			; BCD add ...
		cmp	#$60			; ... but binary comparison.
		bcc	.ticks
		lda	clock_ss
		adc	#0			; BCD add ...
		cmp	#$60			; ... but binary comparison.
		bcc	.seconds
		lda	clock_mm
		adc	#0			; BCD add ...
		cmp	#$60			; ... but binary comparison.
		bcc	.minutes
		lda	clock_hh
		adc	#0			; BCD add ...
		cmp	#$12			; ... but binary comparison.
		bcc	.hours
		cla
.hours:		sta	clock_hh
		cla
.minutes:	sta	clock_mm
		cla
.seconds:	sta	clock_ss
		cla
.ticks:		sta	clock_tt
		cld

		jmp	xfer_palettes		; Upload any palette changes.
