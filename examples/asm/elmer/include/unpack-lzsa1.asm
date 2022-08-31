; ***************************************************************************
; ***************************************************************************
;
; unpack-lzsa1.asm
;
; HuC6280 decompressor for Emmanuel Marty's LZSA1 format.
;
; The code is 171 bytes for the small version, and 197 bytes for the normal.
;
; Copyright John Brandwood 2019-2021.
;
; Distributed under the Boost Software License, Version 1.0.
; (See accompanying file LICENSE_1_0.txt or copy at
;  http://www.boost.org/LICENSE_1_0.txt)
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

LZSA1_SMALL =	0

		;
		; Macro to read a byte from the compressed source data.
		;

	.if	LZSA1_SMALL
LZSA1_GET_SRC	.macro
		jsr	.get_byte
		.endm
	.else
LZSA1_GET_SRC	.macro
		lda	[lzsa_srcptr]
		inc	<lzsa_srcptr + 0
		bne	.skip\@
		jsr	inc.h_si_mpr3
.skip\@:
		.endm
	.endif	LZSA1_SMALL



; ***************************************************************************
; ***************************************************************************
;
; Data usage is 7 bytes of zero-page.
;

lzsa1_srcptr	=	_si			; 1 word.
lzsa1_dstptr	=	_di			; 1 word.

lzsa1_winptr	=	_ax			; 1 word.
lzsa1_cmdbuf	=	_bl			; 1 byte.

lzsa1_offset	=	lzsa1_winptr



; ***************************************************************************
; ***************************************************************************
;
; lzsa1_to_ram - Decompress data stored in Emmanuel Marty's LZSA1 format.
;
; Args: _si, _si_bank = _farptr to compressed data in MPR3.
; Args: _di = ptr to output address in RAM.
;
; Uses: _si, _di, _ax, _bl !
;

lzsa1_to_ram	.proc

		tma3				; Preserve MPR3.
		pha

		tya				; Map lzsa1_srcptr to MPR3.
		beq	!+
		tam3

!:		clx				; Initialize hi-byte of length.
		cly				; Initialize source index.

		;
		; Copy bytes from compressed source data.
		;
		; N.B. X=0 is expected and guaranteed when we get here.
		;

.cp_length:	LZSA1_GET_SRC

		sta	<lzsa1_cmdbuf		; Preserve this for later.
		and	#$70			; Extract literal length.
		lsr	a			; Set CC before ...
		beq	.lz_offset		; Skip directly to match?

		lsr	a			; Get 3-bit literal length.
		lsr	a
		lsr	a
		cmp	#$07			; Extended length?
		bcc	.cp_got_len

		jsr	.get_length		; X=0, CS from CMP, returns CC.
		stx	.cp_npages + 1		; Hi-byte of length.

.cp_got_len:	tax				; Lo-byte of length.

.cp_byte:	lda	[lzsa1_srcptr]		; CC throughout the execution of
		sta	[lzsa1_dstptr]		; of this .cp_page loop.

		inc.l	<lzsa1_srcptr
		bne	.cp_skip1
		jsr	inc.h_si_mpr3

.cp_skip1:	inc.l	<lzsa1_dstptr
		bne	.cp_skip2
		inc.h	<lzsa1_dstptr

.cp_skip2:	dex
		bne	.cp_byte
.cp_npages:	lda	#0			; Any full pages left to copy?
		beq	.lz_offset

		dec	.cp_npages + 1		; Unlikely, so can be slow.
		bcc	.cp_byte		; Always true!

		.if	LZSA1_SMALL

		;
		; Copy bytes from decompressed window.
		;
		; Shorter but slower version.
		;
		; N.B. X=0 is expected and guaranteed when we get here.
		;

.lz_offset:	jsr	.get_byte		; Get offset-lo.

.offset_lo:	adc.l	<lzsa1_dstptr		; Always CC from .cp_page loop.
		sta.l	<lzsa1_winptr

		lda	#$FF
		bit	<lzsa1_cmdbuf
		bpl	.offset_hi

		jsr	.get_byte		; Get offset-hi.

.offset_hi:	adc.h	<lzsa1_dstptr		; lzsa1_winptr < lzsa1_dstptr, so
		sta.h	<lzsa1_winptr		; always leaves CS.

.lz_length:	lda	<lzsa1_cmdbuf		; X=0 from previous loop.
		and	#$0F
		adc	#$03 - 1		; CS from previous ADC.
		cmp	#$12			; Extended length?
		bcc	.lz_got_len

		jsr	.get_length		; CS from CMP, X=0, returns CC.
		stx	.lz_npages + 1		; Hi-byte of length.

.lz_got_len:	tax				; Lo-byte of length.

.lz_byte:	lda	[lzsa1_winptr]		; CC throughout the execution of
		sta	[lzsa1_dstptr]		; of this .lz_page loop.

		inc.l	<lzsa1_winptr
		bne	.lz_skip1
		inc.h	<lzsa1_winptr

.lz_skip1:	inc.l	<lzsa1_dstptr
		bne	.lz_skip2
		inc.h	<lzsa1_dstptr

.lz_skip2:	dex
		bne	.lz_byte
.lz_npages:	lda	#0			; Any full pages left to copy?
		beq	.cp_length

		dec	.lz_npages + 1		; Unlikely, so can be slow.
		bcc	.lz_byte		; Always true!

		.else	LZSA1_SMALL

		;
		; Copy bytes from decompressed window.
		;
		; Longer but faster.
		;
		; N.B. X=0 is expected and guaranteed when we get here.
		;

.lz_offset:	lda	[lzsa1_srcptr]		; Get offset-lo.
		inc.l	<lzsa1_srcptr
		bne	.offset_lo
		jsr	inc.h_si_mpr3

.offset_lo:	sta.l	<lzsa1_offset

		lda	#$FF
		bit	<lzsa1_cmdbuf		; Get offset-hi.
		bpl	.offset_hi

		lda	[lzsa1_srcptr]
		inc.l	<lzsa1_srcptr
		bne	.offset_hi
		jsr	inc.h_si_mpr3

.offset_hi:	sta.h	<lzsa1_offset

.lz_length:	lda	<lzsa1_cmdbuf		; X=0 from previous loop.
		and	#$0F
		adc	#$03			; Always CC from .cp_page loop.
		cmp	#$12			; Extended length?
		bcc	.got_lz_len

		jsr	.get_length		; X=0, CS from CMP, returns CC.

.got_lz_len:	inx				; Hi-byte of length+256.

		eor	#$FF			; Negate the lo-byte of length
		tay
		eor	#$FF

.get_lz_dst:	adc.l	<lzsa1_dstptr		; Calc address of partial page.
		sta.l	<lzsa1_dstptr		; Always CC from previous CMP.
		iny
		bcs	.get_lz_win
		beq	.get_lz_win		; Is lo-byte of length zero?
		dec.h	<lzsa1_dstptr

.get_lz_win:	clc				; Calc address of match.
		adc.l	<lzsa1_offset		; N.B. Offset is negative!
		sta.l	<lzsa1_winptr
		lda.h	<lzsa1_dstptr
		adc.h	<lzsa1_offset
		sta.h	<lzsa1_winptr

.lz_byte:	lda	[lzsa1_winptr], y
		sta	[lzsa1_dstptr], y
		iny
		bne	.lz_byte
		inc.h	<lzsa1_dstptr
		dex				; Any full pages left to copy?
		bne	.lz_more

		jmp	.cp_length		; Loop around to the beginning.

.lz_more:	inc.h	<lzsa1_winptr		; Unlikely, so can be slow.
		bne	.lz_byte		; Always true!

		.endif	LZSA1_SMALL

		;
		; Get 16-bit length in X:A register pair, return with CC.
		;
		; N.B. X=0 is expected and guaranteed when we get here.
		;

.get_length:	clc				; Add on the next byte to get
		adc	[lzsa1_srcptr]		; the length.
		inc.l	<lzsa1_srcptr
		bne	.skip_inc
		jsr	inc.h_si_mpr3

.skip_inc:	bcc	.got_length		; No overflow means done.
		clc				; MUST return CC!
		tax				; Preserve overflow value.

.extra_byte:	jsr	.get_byte		; So rare, this can be slow!
		pha
		txa				; Overflow to 256 or 257?
		beq	.extra_word

.check_length:	pla				; Length-lo.
		bne	.got_length		; Check for zero.
		dex				; Do one less page loop if so.
.got_length:	rts

.extra_word:	jsr	.get_byte		; So rare, this can be slow!
		tax
		bne	.check_length		; Length-hi == 0 at EOF.

.finished:	pla				; Length-lo.
		pla				; Decompression completed, pop
		pla				; return address.

		pla				; Restore MPR3.
		tam3

		leave				; Finished decompression!

.get_byte:	lda	[lzsa1_srcptr]		; Subroutine version for when
		inc.l	<lzsa1_srcptr		; inlining isn't advantageous.
		beq	.next_page
.got_byte:	rts

.next_page:	jmp	inc.h_si_mpr3		; Inc & test for bank overflow.

		.endp
