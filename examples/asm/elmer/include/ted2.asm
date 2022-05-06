; ***************************************************************************
; ***************************************************************************
;
; ted2.asm
;
; Functions for developers using the Turbo Everdrive v2 hardware.
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
; Include dependancies ...
;

		include "common.asm"		; Common helpers.

;
; Include more functionality ...
;

		include	"ted2-sd.asm"		; TED2 SDcard routines.
		include	"ted2-fat32.asm"	; TED2 FAT-32 routines.



; ***************************************************************************
; ***************************************************************************
;
; Definitions
;

; Return Codes for the SD card functions.

TED2_OK			= $00
TED2_NOT_FOUND		= $80



; ***************************************************************************
; ***************************************************************************
;
;
; Data
;

		.bss

ted2_unlocked:	ds	1			; NZ = TED2 found & unlocked.

		.code



; ***************************************************************************
; ***************************************************************************
;
; unlock_ted2 - Try to unlock the TED2 hardware which maps it into bank 0.
;
; Returns: Y,Z-flag,N-flag = TED2_OK or an error code.
;

unlock_ted2	.proc

		tma4				; Preserve MPR4.
		pha

		cla				; Map the TED2 into MPR4.
		tam4

		ldy	#TED2_NOT_FOUND		; Default return code.

		TED_CFG_FPGA			; Unlock the TED2 for use.

		ldx	#5			; Check that KRIKzz's FPGA
.check:		lda	TED_STR_KRIKZZ, x	; ROM is now in bank 0.
		cmp	.string, x
		bne	.finished
		dex
		bpl	.check

		stx	ted2_unlocked		; Set NZ to signal found.

		ldy	#TED2_OK		; Woohoo!

.finished:	pla				; Restore MPR4.
		tam4

		leave				; Return the result.

.string:	db	"KRIKZZ"		; Ident in FPGA ROM.

		.endp



; ***************************************************************************
; ***************************************************************************
;
; wait_vsync_usb - Wait for the next VSYNC, while checking for USB download.
;
; Returns: -
;

wait_vsync_usb	.proc

		pha
		phx
		phy

		tma4				; Preserve MPR4.
		pha

		cla				; Map the TED2 into MPR4.
		tam4

		ldy	irq_cnt
.wait:		lda	ted2_unlocked		; Don't check USB until the
		beq	.skip			; TED2 has been unlocked!

		TED_USB_RD_TEST			; Have we received a USB byte?
		bne	.got_usb

.skip:		cpy	irq_cnt			; If not, continue waiting for
		beq	.wait			; the vblank.

.finished:	pla				; Restore MPR4.
		tam4

		ply
		plx
		pla

		leave				; All done!

		; turbo-usb2.exe is sending us a command!

.got_usb:	TED_USB_RD_BYTE
		cmp	#'*'
		beq	.got_star
.had_star:	ldx	#0			; Was this preceded by a '*'?
		beq	.ignore
		cmp	#'t'
		beq	.got_t
		cmp	#'g'
		beq	.got_g
.ignore:	stz	.had_star + 1		; Self-modifying code!
		bra	.wait

		; turbo-usb2.exe is sending us a command!

.got_star:	sta	.had_star + 1		; Self-modifying code!
		bra	.wait

		; turbo-usb2.exe wants to know if we're listening!

.got_t:		lda	#'k'
		TED_USB_WR_BYTE
		bra	.wait

		; turbo-usb2.exe wants to send a HuCard ROM image!

.got_g:		sei				; Disable interrupts for xfer.
		nop

		tii	.usb_xfer, $2100, .usb_done - .usb_xfer

		jmp	$2100			; Run the download code.

; This download code will be overwritten, so run it from RAM instead.
;
; N.B. This code supports a maximum transfer of 2.5MB for "Street Fighter II".

.usb_xfer:	lda	#$4F			; Map in TED2 512KB block 4.
		sta	TED_BASE_ADDR + TED_REG_MAP

		pha				; Preserve TED2 512KB block.

		lda	#$40			; Select bank 0 of the HuCard
		tam3				; image in TED2 memory.

.bank_loop:	clx
		lda	#$60			; Reset the desination page.
		sta	$2100 + (.save_page + 2 - .usb_xfer)

.byte_loop:	lda	#(1 << TED_FLG_USB_RD)	; Copy a byte from USB.
.wait_byte:	bit	TED_BASE_ADDR + TED_REG_STATE
		beq	.wait_byte
		lda	TED_BASE_ADDR + TED_REG_FIFO
.save_page:	sta	$6000,x
		inx
		bne	.byte_loop		; Same page?
		inc	$2100 + (.save_page + 2 - .usb_xfer)
		bpl	.byte_loop		; Same bank?

		tma3				; Map in next PCE 8KB bank.
		inc	a
		bpl	.next_bank		; Have we just loaded 512KB?

.next_512kb:	pla				; Map in next TED 512KB block.
		clc
		adc	#$10
		bit	#$40			; Wrap from block 4 to block 0.
		beq	.wrap_512kb
		and	#$0F

.wrap_512kb:	pha				; Preserve TED2 512KB block.
		sta	TED_BASE_ADDR + TED_REG_MAP

		lda	#$40			; Reset destination PCE bank.
.next_bank:	tam3

		TED_USB_RD_BYTE			; What next?
		cmp	#'+'			; Load another bank!
		beq	.bank_loop

		pla				; Discard TED 512KB block.

		stz	VCE_CR			; Reset VCE speed.
		st0	#VDC_CR			; Disable VDC interrupts and
		st1	#$00			; turn off the display.
		st2	#$00
		stz	TIMER_CR		; Stop timer.
		stz	IRQ_ACK			; Clear timer interrupt.
		bit	VDC_SR			; Clear VDC interrupt.

		lda	IO_PORT			; Is there a CD unit attached?
		bmi	.card_type

		; Code to reset CD hardware should go here!

.card_type:	TED_USB_RD_BYTE			; Is this a special HuCard?
		cmp	#'p'			; It is Populous?
		beq	.populous
		cmp	#'s'			; It is Street Fighter?
		beq	.streetfighter

		ldy	#%00010011		; FPGA=off+locked, RAM=readonly.

.execute:	lda	#$4F			; Map in TED2 512KB bank 4.
		sta	TED_BASE_ADDR + TED_REG_MAP
		lda	#%00010000		; FPGA=on+writable, RAM=readonly.
		sta	TED_BASE_ADDR + TED_REG_CFG
		lda	#$04			; Set 512KB mapping for HuCard.
		sta	TED_BASE_ADDR + TED_REG_MAP
		sty	TED_BASE_ADDR + TED_REG_CFG

		cla				; Put Bank $00 in MPR7.
		tam7

		csl				; Simulate a reset.
		jmp	[$FFFE]

		; Populous is a ROMRAM HuCard, and needs writable RAM.

.populous:	ldy	#%00000011		; FPGA=off+locked, RAM=writable.
		bra	.execute

		; Street Fighter II needs the TED2's SF2 mapper enabled.

.streetfighter:	ldy	#%00110011		; FPGA=off+locked, RAM=readonly,
		bra	.execute		; SF2=enabled.

.usb_done:	nop

		.endp
