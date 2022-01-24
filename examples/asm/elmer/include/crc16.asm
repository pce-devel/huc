; ***************************************************************************
; ***************************************************************************
;
; crc16.asm
;
; CCITT CRC-16 as used by ZMODEM/YMODEM/XMODEM/etc.
;
; ***************************************************************************
; ***************************************************************************



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
crc16_tbl_lo:	=	$3700			; 256-bytes
crc16_tbl_hi:	=	$3800			; 256-bytes
	.endif

init_crc16	.proc

		clx				; X counts from 0 to 255
.byte_loop:	cla				; Lo 8 bits of the CRC-16
		stx	<__al			; Hi 8 bits of the CRC-16
		ldy	#8			; Y counts bits in a byte

.bit_loop:	asl	a
		rol	<__al			; Shift CRC left
		bcc	.no_add			; Do nothing if no overflow
		eor	#$21			; else add CRC-16 polynomial $1021
		pha				; Save low byte
		lda	<__al			; Do high byte
		eor	#$10
		sta	<__al
		pla				; Restore low byte
.no_add:	dey
		bne	.bit_loop		; Do next bit

		sta	crc16_tbl_lo,x		; Save CRC into table, low byte
		lda	<__al			; then high byte
		sta	crc16_tbl_hi,x
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
		jsr	__si_to_mpr3		; Map memory block to MPR3.

	.if	1
		lda	#$FF			; For CCITT.
		sta	<__ax + 0
		sta	<__ax + 1
	.else
		stz	<__ax + 0		; For XMODEM.
		stz	<__ax + 1
	.endif

		ldy	<__cx + 0
		beq	.byte_loop
		inc	<__cx + 1

.byte_loop:	lda	[__si]

		eor	<__ax + 1		; Update CRC-16.
		tax
		lda	<__ax + 1
		eor	crc16_tbl_hi,x
		sta	<__ax + 1
		lda	crc16_tbl_lo,x
		sta	<__ax + 0

		inc	<__si + 0
		beq	.next_page

.next_byte:	dey
		bne	.byte_loop
		dec	<__cx + 1
		bne	.byte_loop

		pla				; Restore MPR3.
		tam3

		leave

.next_page:	jsr	__si_inc_page
		bra	.next_byte

		.endp
