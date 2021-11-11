; ***************************************************************************
; ***************************************************************************
;
; core.inc
;
; Base include for the "CORE(not TM)" PC Engine library code.
;
; Copyright John Brandwood 2021.
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

CORE_VERSION	=	$01

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
; CORE kernel code in RAM, starts above the System Card's RAM variables.
;

	.if	USING_PSGDRIVER
core_ram1st     =	$2680
	.else
core_ram1st     =	$22D0
	.endif	USING_PSGDRIVER

;
; CORE Zero-Page variables, relative to the System Card's ZP variables.
;

	.if	USING_PSGDRIVER
core_zpend	=	$20E6
	.else

	.if	SUPPORT_SGX

		; Put some SGX ZP variables on top of the PSG variables.
		;
		; This allows us to put SGX_VDC_OFFSET or PCE_VDC_OFFSET into
		; the X register, and then index into either set of hardware
		; registers and the "sgx_reg" or "vdc_reg" shadow-variables.

PCE_VDC_OFFSET	=	$00			; Offset to PCE VDC hw & vars.
SGX_VDC_OFFSET	=	$F0			; Offset to SGX VDC hw & vars.

sgx_sr		=	$20E6			; Shadow of SGX status reg.
sgx_reg		=	$20E7			; Shadow of SGX address reg.

sgx_crl		=	$20E8			; Shadow of SGX control reg.
sgx_crh		=	$20E9			; Shadow of SGX control reg.

core_zpend	=	$20E6
	.else

core_zpend	=	$20EC
	.endif	SUPPORT_SGX

	.endif	USING_PSGDRIVER

		; Actual variables!

core_1stbank	=	core_zpend - 1		; 1st bank of engine code.

;
; Include the "CORE(not TM)" startup code to begin the HuCARD / Overlay.
;

		include	"core-startup.asm"
