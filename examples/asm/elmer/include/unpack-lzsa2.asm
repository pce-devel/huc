; ***************************************************************************
; ***************************************************************************
;
; unpack-lzsa2.asm
;
; HuC6280 decompressor for Emmanuel Marty's LZSA2 format.
;
; The code is 248 bytes for the small version, and 263 bytes for the normal.
;
; Copyright John Brandwood 2019-2024.
;
; Distributed under the Boost Software License, Version 1.0.
; (See accompanying file LICENSE_1_0.txt or copy at
;  http://www.boost.org/LICENSE_1_0.txt)
;
; ***************************************************************************
; ***************************************************************************
;
; N.B. The decompressor expects the data to be compressed without a header!
;
; Use Emmanuel Marty's LZSA compressor which can be found here ...
;  https://github.com/emmanuel-marty/lzsa
;
; To create an LZSA2 file to decompress to RAM
;
;  lzsa -r -f 2 <infile> <outfile>
;
; ***************************************************************************
; ***************************************************************************



; ***************************************************************************
; ***************************************************************************
;
; Decompression Options & Macros
;

		;
		; Choose size over speed (within sane limits)?
		;

LZSA2_SMALL	=	0

		;
		; Macro to read a byte from the compressed source data.
		;

	.if	LZSA2_SMALL
LZSA2_GET_SRC	.macro
		jsr	.get_byte
		.endm
	.else
LZSA2_GET_SRC	.macro
		lda	[lzsa_srcptr]
		inc	<lzsa_srcptr + 0
		bne	.skip\@
		jsr	inc.h_bp_mpr3
.skip\@:
		.endm
	.endif	LZSA2_SMALL



; ***************************************************************************
; ***************************************************************************
;
; Data usage is 11 bytes of zero-page.
;

lzsa2_srcptr	=	_bp			; 1 word.
lzsa2_dstptr	=	_di			; 1 word.

lzsa2_offset	=	_ax			; 1 word.
lzsa2_winptr	=	_bx			; 1 word.
lzsa2_cmdbuf	=	_cl			; 1 byte.
lzsa2_nibflg	=	_ch			; 1 byte.
lzsa2_nibble	=	_dl			; 1 byte.



; ***************************************************************************
; ***************************************************************************
;
; lzsa2_to_ram - Decompress data stored in Emmanuel Marty's LZSA2 format.
;
; Args: _bp, Y = _farptr to compressed data in MPR3.
; Args: _di = ptr to output address in RAM (anywhere except MPR3!).
;
; Returns: _bp, Y = _farptr to byte after compressed data.
;
; Uses: _bp, _di, _ax, _bx, _cx, _dl !
;

lzsa2_to_ram	.proc

		tma3				; Preserve MPR3.
		pha

		jsr	set_bp_to_mpr3		; Map lzsa2_srcptr to MPR3.

		clx				; Hi-byte of length or offset.
		cly				; Initialize source index.
		stz	<lzsa2_nibflg		; Initialize nibble buffer.

		;
		; Copy bytes from compressed source data.
		;
		; N.B. X=0 is expected and guaranteed when we get here.
		;

.cp_length:	LZSA2_GET_SRC

		sta	<lzsa2_cmdbuf		; Preserve this for later.
		and	#$18			; Extract literal length.
		beq	.lz_offset		; Skip directly to match?

		lsr	a			; Get 2-bit literal length.
		lsr	a
		lsr	a
		cmp	#$03
		bcc	.cp_got_len		; Extended length?

		jsr	.get_length		; X=0 for literals, returns CC.
		stx	.cp_npages + 1		; Hi-byte of length.

.cp_got_len:	tax				; Lo-byte of length.

.cp_byte:	lda	[lzsa2_srcptr]		; CC throughout the execution of
		sta	[lzsa2_dstptr]		; of this .cp_page loop.

		inc.l	<lzsa2_srcptr
		bne	.cp_skip1
		inc.h	<lzsa2_srcptr

.cp_skip1:	inc.l	<lzsa2_dstptr
		bne	.cp_skip2
		inc.h	<lzsa2_dstptr

.cp_skip2:	dex
		bne	.cp_byte
.cp_npages:	lda	#0			; Any full pages left to copy?
		beq	.lz_offset

		dec	.cp_npages + 1		; Unlikely, so can be slow.
		bcc	.cp_byte		; Always true!

		;
		; Copy bytes from decompressed window.
		;
		; N.B. X=0 is expected and guaranteed when we get here.
		;
		; xyz
		; ===========================
		; 00z  5-bit offset
		; 01z  9-bit offset
		; 10z  13-bit offset
		; 110  16-bit offset
		; 111  repeat offset
		;

.lz_offset:	lda	<lzsa2_cmdbuf
		asl	a
		bcs	.get_13_16_rep

.get_5_9_bits:	dex				; X=$FF for a 5-bit offset.
		asl	a
		bcs	.get_9_bits		; Fall through if 5-bit.

.get_13_bits:	asl	a			; Both 5-bit and 13-bit read
		php				; a nibble.
		jsr	.get_nibble
		plp
		rol	a			; Shift into position, clr C.
		eor	#$E1
		cpx	#$00			; X=$FF for a 5-bit offset.
		bne	.set_offset
		sbc	#2			; 13-bit offset from $FE00.
		bne	.set_hi_8		; Always NZ from previous SBC.

.get_9_bits:	asl	a			; X=$FF if CC, X=$FE if CS.
		bcc	.get_lo_8
		dex
		bcs	.get_lo_8		; Always CS from previous BCC.

.get_13_16_rep:	asl	a
		bcc	.get_13_bits		; Shares code with 5-bit path.

.get_16_rep:	bmi	.lz_length		; Repeat previous offset.

.get_16_bits:	jsr	.get_byte		; Get hi-byte of offset.

.set_hi_8:	tax

.get_lo_8:	LZSA2_GET_SRC			; Get lo-byte of offset.

.set_offset:	sta.l	<lzsa2_offset		; Save new offset.
		stx.h	<lzsa2_offset

.lz_length:	ldx	#1			; Hi-byte of length+256.

		lda	<lzsa2_cmdbuf
		and	#$07
		clc
		adc	#$02
		cmp	#$09			; Extended length?
		bcc	.got_lz_len

		jsr	.get_length		; X=1 for match, returns CC.
		inx				; Hi-byte of length+256.

.got_lz_len:	eor	#$FF			; Negate the lo-byte of length.
		tay
		eor	#$FF

.get_lz_dst:	adc.l	<lzsa2_dstptr		; Calc address of partial page.
		sta.l	<lzsa2_dstptr		; Always CC from .cp_page loop.
		iny
		bcs	.get_lz_win
		beq	.get_lz_win		; Is lo-byte of length zero?
		dec.h	<lzsa2_dstptr

.get_lz_win:	clc				; Calc address of match.
		adc.l	<lzsa2_offset		; N.B. Offset is negative!
		sta.l	<lzsa2_winptr
		lda.h	<lzsa2_dstptr
		adc.h	<lzsa2_offset
		sta.h	<lzsa2_winptr

.lz_byte:	lda	[lzsa2_winptr], y
		sta	[lzsa2_dstptr], y
		iny
		bne	.lz_byte
		inc.h	<lzsa2_dstptr
		dex				; Any full pages left to copy?
		bne	.lz_more

		jmp	.cp_length		; Loop around to the beginning.

.lz_more:	inc.h	<lzsa2_winptr		; Unlikely, so can be slow.
		bne	.lz_byte		; Always true!

		;
		; Lookup tables to differentiate literal and match lengths.
		;

.nibl_len_tbl:	db	3			; 0+3 (for literal).
		db	9			; 2+7 (for match).

.byte_len_tbl:	db	18 - 1			; 0+3+15 - CS (for literal).
		db	24 - 1			; 2+7+15 - CS (for match).

		;
		; Get 16-bit length in X:A register pair, return with CC.
		;

.get_length:	jsr	.get_nibble
		cmp	#$0F			; Extended length?
		bcs	.byte_length
		adc	.nibl_len_tbl, x	; Always CC from previous CMP.

.got_length:	clx				; Set hi-byte of 4 & 8 bit
		rts				; lengths.

.byte_length:	jsr	.get_byte		; So rare, this can be slow!
		adc	.byte_len_tbl, x	; Always CS from previous CMP.
		bcc	.got_length
		beq	.finished

.word_length:	clc				; MUST return CC!
		jsr	.get_byte		; So rare, this can be slow!
		pha
		jsr	.get_byte		; So rare, this can be slow!
		tax
		pla
		bne	.got_word		; Check for zero lo-byte.
		dex				; Do one less page loop if so.
.got_word:	rts

.get_byte:	lda	[lzsa2_srcptr]		; Subroutine version for when
		inc.l	<lzsa2_srcptr		; inlining isn't advantageous.
		beq	.next_page
.got_byte:	rts

.next_page:	jmp	inc.h_bp_mpr3		; Inc & test for bank overflow.

.finished:	pla				; Decompression completed, pop
		pla				; return address.

		tma3				; Return final MPR3 in Y reg.
		tay

		pla				; Restore MPR3.
		tam3

		leave				; Finished decompression!

		;
		; Get a nibble value from compressed data in A.
		;

.get_nibble:	lsr	<lzsa2_nibflg		; Is there a nibble waiting?
		lda	<lzsa2_nibble		; Extract the lo-nibble.
		bcs	.got_nibble

		inc	<lzsa2_nibflg		; Reset the flag.
		LZSA2_GET_SRC
		sta	<lzsa2_nibble		; Preserve for next time.
		lsr	a			; Extract the hi-nibble.
		lsr	a
		lsr	a
		lsr	a

.got_nibble:	and	#$0F
		rts

		.endp
