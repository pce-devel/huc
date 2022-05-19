;
; SOUND.INC - Included by STARTUP.ASM to customize the sound driver.
;
; A customized sound driver for HuCard or CDROM usage can create a
; version of this file, and then have startup.asm use the new
; definitions just by including the new sound driver's path
; in the PCE_INCLUDE environment variable BEFORE the standard
; "huc/include/pce" path.
;
; This default version is comptabile with the CDROM System Card usage.
;

;---------------------------------------------------------------------------
; tell startup.asm that we need the SOUND_BANK and to include sound.asm

NEED_SOUND_BANK	= 0
NEED_SOUND_CODE	= 0

;---------------------------------------------------------------------------------------
; This macro is invoked in HuC startup.asm after the RAM is cleared, and just before
; interrupts are enabled.

__sound_init	.macro
		.endm

;---------------------------------------------------------------------------------------
; This macro is invoked in HuC startup.asm inside the vsync handler code.

__sound_vsync	.macro
		.endm

;---------------------------------------------------------------------------------------
; This macro is invoked in HuC startup.asm inside the timer handler code.
;
; N.B. HuC startup.asm only invokes this if NOT building for CDROM!!!
;

__sound_timer	.macro
		.endm

;---------------------------------------------------------------------------------------
; this is the location to start allocating HuC's internal variables.

		.org			$2680
