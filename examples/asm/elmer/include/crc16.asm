; ***************************************************************************
; ***************************************************************************
;
; crc16.asm
;
; CCITT CRC-16 as used by ZMODEM/YMODEM/XMODEM/etc.
;
; ***************************************************************************
; ***************************************************************************

;
; Include dependancies ...
;

		include "common.asm"		; Common helpers.



; ***************************************************************************
; ***************************************************************************
;
; init_crc16 - Initialize the CRC-16 lookup tables in RAM.
;
; By Paul Guertin. See http://6502.org/source/integers/crc.htm
;
; CCITT CRC-16 as used by ZMODEM/YMODEM/XMODEM/etc.
;
; Takes 60,942 cycles to create the tables.
;

	.ifndef	crc16_tbl_lo
crc16_tbl_lo:	=	$3E00			; 256-bytes
crc16_tbl_hi:	=	$3F00			; 256-bytes
	.endif

init_crc16	.proc

		clx				; X counts from 0 to 255
.byte_loop:	cla				; Lo 8 bits of the CRC-16
		stx	<_al			; Hi 8 bits of the CRC-16
		ldy	#8			; Y counts bits in a byte

.bit_loop:	asl	a
		rol	<_al			; Shift CRC left
		bcc	.no_add			; Do nothing if no overflow
		eor	#$21			; else add CRC-16 polynomial $1021
		pha				; Save low byte
		lda	<_al			; Do high byte
		eor	#$10
		sta	<_al
		pla				; Restore low byte
.no_add:	dey
		bne	.bit_loop		; Do next bit

		sta	crc16_tbl_lo, x		; Save CRC into table, low byte
		lda	<_al			; then high byte
		sta	crc16_tbl_hi, x
		inx
		bne	.byte_loop		; Do next byte

		leave

		.endp



; ***************************************************************************
; ***************************************************************************
;
; calc_crc16 - Calculate the CRC-16 of a block of memory.
;
; CCITT CRC-16 as used by ZMODEM/YMODEM/XMODEM/etc.
;
; name    polynomial  initial val
; CCITT         1021         FFFF
; XModem        1021         0000
;

calc_crc16	.proc

		tma3				; Preserve MPR3.
		pha

		tya				; Map memory block to MPR3.
		beq	!+
		tam3

	.if	1
!:		lda	#$FF			; For CCITT.
		sta	<_cx + 0
		sta	<_cx + 1
	.else
!:		stz	<_cx + 0		; For XMODEM.
		stz	<_cx + 1
	.endif

		ldy	<_ax + 0
		beq	.byte_loop
		inc	<_ax + 1

.byte_loop:	lda	[_si]

		eor	<_cx + 1		; Update CRC-16.
		tax
		lda	<_cx + 1
		eor	crc16_tbl_hi, x
		sta	<_cx + 1
		lda	crc16_tbl_lo, x
		sta	<_cx + 0

		inc	<_si + 0
		beq	.next_page

.next_byte:	dey
		bne	.byte_loop
		dec	<_ax + 1
		bne	.byte_loop

		pla				; Restore MPR3.
		tam3

		leave

.next_page:	jsr	inc.h_si_mpr3
		bra	.next_byte

		.endp
