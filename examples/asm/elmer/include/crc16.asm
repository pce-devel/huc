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

		leave				; All done!

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
; Args: _bp, Y = _farptr to data in MPR3.
; Args: _ax, _bl = 23-bit length (0-8MB).
;
; Returns: _cx = 16-bit CRC.
;

calc_crc16	.proc

		tma3				; Preserve MPR3.
		pha

	.ifdef	_KICKC
		jsr	set_bp_to_mpr3		; Map memory block to MPR3.
	.else
		tya				; Map memory block to MPR3.
		beq	!+
		tam3
	.endif

	.if	1
!:		lda	#$FF			; For CCITT.
		sta	<_cx + 0
		sta	<_cx + 1
	.else
!:		stz	<_cx + 0		; For XMODEM.
		stz	<_cx + 1
	.endif

		inc	<_ax + 1		; +1 for bytes in last page.
		ldy	<_ax + 0
		beq	.next_page

.byte_loop:	lda	[_bp]

		eor	<_cx + 1		; Update CRC-16.
		tax
		lda	<_cx + 1
		eor	crc16_tbl_hi, x
		sta	<_cx + 1
		lda	crc16_tbl_lo, x
		sta	<_cx + 0

		inc	<_bp + 0
		beq	.inc_page

.next_byte:	dey
		bne	.byte_loop
.next_page:	dec	<_ax + 1		; Is there a full page still
		bne	.byte_loop              ; left to CRC?
.next_64kb:	dec	<_ax + 2		; Is there a full 64KB block
		bpl	.byte_loop		; still left to CRC?

		pla				; Restore MPR3.
		tam3

		leave				; All done!

.inc_page:	jsr	inc.h_bp_mpr3
		bra	.next_byte

		.endp
