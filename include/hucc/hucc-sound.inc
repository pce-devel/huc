; ***************************************************************************
; ***************************************************************************
;
; hucc-sound.inc
;
; Included by STARTUP.ASM to customize the sound driver.
;
; Copyright John Brandwood 2024.
;
; Distributed under the Boost Software License, Version 1.0.
; (See accompanying file LICENSE_1_0.txt or copy at
;  http://www.boost.org/LICENSE_1_0.txt)
;
; ***************************************************************************
; ***************************************************************************
;
; A customized sound driver for HuCard or CDROM usage can create a
; version of this file, and then have startup.asm use the new
; definitions just by including the new sound driver's path
; in the PCE_INCLUDE environment variable BEFORE the standard
; "include/hucc" path.
;
; ***************************************************************************
; ***************************************************************************



; ***************************************************************************
; ***************************************************************************
;
; Tell startup.asm that we need the SOUND_BANK and to include sound.asm

NEED_SOUND_BANK	= 0



; ***************************************************************************
; ***************************************************************************
;
; The driver might wish to reserve some extra banks, or it can just let the
; actual project do it in hucc-config.inc

; RESERVE_BANKS	= 0



; ***************************************************************************
; ***************************************************************************
;
; This macro is invoked in hucc.asm just after the RAM is cleared, and before
; interrupts are enabled.

__sound_init	.macro
		.endm
