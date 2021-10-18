; ***************************************************************************
; ***************************************************************************
;
; pceas.inc
;
; Helpful macros for using PCEAS for assembly-language development.
;
; Distributed under the Boost Software License, Version 1.0.
; (See accompanying file LICENSE_1_0.txt or copy at
;  http://www.boost.org/LICENSE_1_0.txt)
;
; ***************************************************************************
; ***************************************************************************

;
; Define names for macro argument types (\?x).
;

ARG_NONE	.equ	0
ARG_REG		.equ	1
ARG_IMMED	.equ	2
ARG_ABS		.equ	3
ARG_ABSOLUTE	.equ	3
ARG_INDIRECT	.equ	4
ARG_STRING	.equ	5
ARG_LABEL	.equ	6

;
; Macro to shadow VDC_AR writes because the register is write-only.
;
; Any writes to the VDC_AR register in an IRQ routine should be restored
; using this zero-page shadow.
;

vreg		.macro 
		lda	\1
		sta	<vdc_reg
.if (\?1 = ARG_IMMED)
		st0	\1
.else
		sta	VDC_AR
.endif
		.endm

;
; Macro to use for the exit instruction of a ".proc" routine.
;
; In the old HuC method, procedures end with an "rts", with
; the "-newproc" option, they jump to a piece of shared code.
;

		.if	USING_NEWPROC

leave		.macro
		jmp	sys_leave_proc
		.endm

		.else

leave		.macro
		rts
		.endm

		.endif

;
; Macros to align data/code to boundaries commonly seen on the PCE.
;

SPR_ALIGN	.macro
		.if	(* & 127)
		ds	128 - (* & 127)		; 128-byte align the data.
		.endif
		.endm

CHR_ALIGN	.macro
		.if	(* & 31)
		ds	32 - (* & 31)		; 32-byte align the data.
		.endif
		.endm

WORD_ALIGN	.macro
		.if	(* & 1)
		ds	2 - (* & 1)		; Word Align the data.
		.endif
		.endm

LONG_ALIGN	.macro
		.if	(* & 3)
		ds	4 - (* & 3)		; Long Align the data.
		.endif
		.endm

PAGE_ALIGN	.macro
		.if	(* & 255)
		ds	256 - (* & 255)		; Page Align the data.
		.endif
		.endm

;
;
;
