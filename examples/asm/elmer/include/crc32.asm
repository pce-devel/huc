; ***************************************************************************
; ***************************************************************************
;
; crc32.asm
;
; CRC-32 as used by ZIP/PNG/ZMODEM/etc.
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
; init_crc32 - Initialize the CRC32 lookup tables in RAM.
;
; By Paul Guertin. See http://6502.org/source/integers/crc.htm
;
; CRC-32 as used by ZIP/PNG/ZMODEM/etc.
;
; Takes 112,654 cycles to create the tables.
;

	.ifndef	crc32_tbl_b0
crc32_tbl_b0	=	$3C00			; 256-bytes
crc32_tbl_b1	=	$3D00			; 256-bytes
crc32_tbl_b2	=	$3E00			; 256-bytes
crc32_tbl_b3	=	$3F00			; 256-bytes
	.endif

init_crc32	.proc

		clx				; X counts from 0 to 255
.byte_loop:	cla				; A contains the high byte of the CRC-32
		sta	<_ax + 2		; The other three bytes are in memory
		sta	<_ax + 1
		stx	<_ax + 0

		ldy	#8			; Y counts bits in a byte
.bit_loop:	lsr	a			; The CRC-32 algorithm is similar to CRC-16
		ror	<_ax + 2		; except that it is reversed (originally for
		ror	<_ax + 1		; hardware reasons). This is why we shift
		ror	<_ax + 0		; right instead of left here.
		bcc	.no_add			; Do nothing if no overflow
		eor	#$ED			; else add CRC-32 polynomial $EDB88320
		pha				; Save high byte while we do others
		lda	<_ax + 2
		eor	#$B8			; Most reference books give the CRC-32 poly
		sta	<_ax + 2		; as $04C11DB7. This is actually the same if
		lda	<_ax + 1		; you write it in binary and read it right-
		eor	#$83			; to-left instead of left-to-right. Doing it
		sta	<_ax + 1		; this way means we won't have to explicitly
		lda	<_ax + 0		; reverse things afterwards.
		eor	#$20
		sta	<_ax + 0
		pla				; Restore high byte.
.no_add:	dey
		bne	.bit_loop		; Do next bit

		sta	crc32_tbl_b3, x		; Save CRC into table, high to low bytes.
		lda	<_ax + 2
		sta	crc32_tbl_b2, x
		lda	<_ax + 1
		sta	crc32_tbl_b1, x
		lda	<_ax + 0
		sta	crc32_tbl_b0, x
		inx
		bne	.byte_loop		; Do next byte

		leave

		.endp



; ***************************************************************************
; ***************************************************************************
;
; calc_crc32 - Calculate the CRC32 of a block of memory.
;
; CRC-32 as used by ZIP/PNG/ZMODEM/etc.
;

calc_crc32	.proc

		tma3				; Preserve MPR3.
		pha

		tya				; Map memory block to MPR3.
		beq	!+
		tam3

!:		lda	#$FF			; Initialize CRC-32.
		sta	<_cx + 0
		sta	<_cx + 1
		sta	<_cx + 2
		sta	<_cx + 3

		ldy	<_ax + 0
		beq	.byte_loop
		inc	<_ax + 1

.byte_loop:	lda	[_si]

		eor	<_cx + 0		; Update CRC-32.
		tax
		lda	<_cx + 1
		eor	crc32_tbl_b0, x
		sta	<_cx + 0
		lda	<_cx + 2
		eor	crc32_tbl_b1, x
		sta	<_cx + 1
		lda	<_cx + 3
		eor	crc32_tbl_b2, x
		sta	<_cx + 2
		lda	crc32_tbl_b3, x
		sta	<_cx + 3

		inc	<_si + 0
		beq	.next_page

.next_byte:	dey
		bne	.byte_loop
		dec	<_ax + 1		; Next page.
		bne	.byte_loop

		ldx	#3			; Complement CRC-32 when done
.flip_bits:	lda	<_cx, x		; to follow standard.
		eor	#$FF
		sta	<_cx, x
		dex
		bpl	.flip_bits

		pla				; Restore MPR3.
		tam3

		leave

.next_page:	jsr	inc.h_si_mpr3
		bra	.next_byte

		.endp
