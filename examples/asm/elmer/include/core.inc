; ***************************************************************************
; ***************************************************************************
;
; core.inc
;
; Base include for the "CORE(not TM)" PC Engine library code.
;
; Copyright John Brandwood 2021-2022.
;
; Distributed under the Boost Software License, Version 1.0.
; (See accompanying file LICENSE_1_0.txt or copy at
;  http://www.boost.org/LICENSE_1_0.txt)
;
; ***************************************************************************
; ***************************************************************************

		.nolist

		; Include the equates for the basic PCE hardware.

		include	"pceas.inc"
		include	"pcengine.inc"
		include	"ted2.inc"

		.list
		.mlist

CORE_VERSION	=	$10			; Version 1.0

		; Get the user program's configuration settings, this will
		; be read from the current directory, if it exists, or the
		; "../include/" directory if not.

		include	"core-config.inc"

;
; This is a simple macro to call System Card functions when using MPR7 for
; running the "CORE(not TM)" PC Engine library code.
;

system		.macro
		ldy     #<\1
		jsr	call_bios
		.endm

;
; This replaces the System Card's ex_setvec, because "call_bios" uses the
; Y register.
;

setvec		.macro
		php
		sei
		lda	#<\2
		sta	\1 + 0
		lda	#>\2
		sta	\1 + 1
		plp
		.endm

;
; Repurpose this System Card variable, since nmi_hook is never used.
;
; We'll use it as a vector to a user-supplied sound driver instead.
;

sound_hook	=	nmi_hook		; Sound Driver to run in VBL.

;
; CORE Zero-Page variables, relative to the System Card's ZP variables.
;

	.if	USING_PSGDRIVER
core_zpend	=	$20E6
	.else
core_zpend	=	$20EC
	.endif	USING_PSGDRIVER

	.if	SUPPORT_SGX

		; Put some "CORE(not TM)" variables at the start of Zero Page.
		;
		; These are at the *start* of ZP so that the SuperGrafx shadow
		; "sgx_" registers are at a specific offset from their System
		; Card "vdc_" counterparts, and they MUST NOT be moved!
		;
		; This allows us to put SGX_VDC_OFFSET or PCE_VDC_OFFSET into
		; the X register, and then index into either set of hardware
		; registers and the "sgx_reg" or "vdc_reg" shadow-variables.

PCE_VDC_OFFSET	=	$00			; Offset to PCE VDC hw & vars.
SGX_VDC_OFFSET	=	$10			; Offset to SGX VDC hw & vars.

_temp		=	$2000			; Use within any ASM routine.
_bank		=	$2002			; Use within any ASM routine.
sgx_crl		=	$2003			; SGX shadow (vdc_crl = $20F3).
sgx_crh		=	$2004			; SGX shadow (vdc_crh = $20F4).
core_1stbank	=	$2005			; 1st bank of library code.
sgx_sr		=	$2006			; SGX shadow (vdc_sr  = $20F6).
sgx_reg		=	$2007			; SGX shadow (vdc_reg = $20F7).

core_zp1st	=	$2008			; 1st free user address.

	.else	SUPPORT_SGX

_temp		=	$2000			; Use within any ASM routine.
_bank		=	$2002			; Use within any ASM routine.
core_1stbank	=	$2003			; 1st bank of engine code.

core_zp1st	=	$2004			; 1st free user address.

	.endif	SUPPORT_SGX

		.zp
		.org	core_zp1st
		.code

;
; The kernel code in RAM follows the System Card's RAM variables.
;

	.if	USING_PSGDRIVER
core_ram1st     =	$2680
	.else
core_ram1st     =	$22D0
	.endif	USING_PSGDRIVER

		.bss
		.org	core_ram1st
		.code

;
; Include the "CORE(not TM)" startup code to begin the HuCARD / Overlay.
;

		include	"core-startup.asm"
