; ***************************************************************************
; ***************************************************************************
;
; unpack-zx0.asm
;
; HuC6280 decompressor for Einar Saukas's "classic" ZX0 format.
;
; The code length is 200 bytes for RAM, 250 bytes for direct-to-VRAM, plus
; some generic utility code.
;
; Copyright John Brandwood 2021-2024.
;
; Distributed under the Boost Software License, Version 1.0.
; (See accompanying file LICENSE_1_0.txt or copy at
;  http://www.boost.org/LICENSE_1_0.txt)
;
; ***************************************************************************
; ***************************************************************************
;
; ZX0 "modern" format is not supported, because it costs an extra 4 bytes of
; code in this decompressor, and it runs slower.
;
; Use Emmanuel Marty's SALVADOR ZX0 compressor which can be found here ...
;  https://github.com/emmanuel-marty/salvador
;
; To create a ZX0 file to decompress to RAM
;
;  salvador -classic <infile> <outfile>
;
; To create a ZX0 file to decompress to VRAM, using a 2KB ring-buffer in RAM
;
;  salvador -classic -w 2048 <infile> <outfile>
;
; ***************************************************************************
; ***************************************************************************

;
; Configure Library ...
;

	.ifndef	SUPPORT_ACD
SUPPORT_ACD	=	1
	.endif



; ***************************************************************************
; ***************************************************************************
;
; If you decompress directly to VRAM, then you need to define a ring-buffer
; in RAM, both sized and aligned to a power-of-two (i.e. 512, 1KB, 2KB, 4KB).
;
; You also need to make sure that you tell the compressor that it needs to
; limit the window size with its "-w" option.
;
; Note that CD-ROM developers should really just decompress to RAM, and then
; use a TIA to copy the data to VRAM; because that is faster, you get better
; compression without a window, and you save code memory by not needing both
; versions of the decompression routine.
;

	.ifndef	ZX0_WINBUF

ZX0_WINBUF	=	($3800) >> 8		; Default to a 2KB window in
ZX0_WINMSK	=	($0800 - 1) >> 8	; RAM, located at $3800.

	.endif



; ***************************************************************************
; ***************************************************************************
;
; Data usage is 11 bytes of zero-page, using aliases for clarity.
;

zx0_srcptr	=	_bp			; 1 word.
zx0_dstptr	=	_di			; 1 word.

zx0_length	=	_ax			; 1 word.
zx0_offset	=	_bx			; 1 word.
zx0_winptr	=	_cx			; 1 word.
zx0_bitbuf	=	_dl			; 1 byte.



; ***************************************************************************
; ***************************************************************************
;
; zx0_to_ram - Decompress data stored in Einar Saukas's ZX0 "classic" format.
;
; Args: _bp, Y = _farptr to compressed data in MPR3.
; Args: _di = ptr to output address in RAM (anywhere except MPR3!).
;
; Returns: _bp, Y = _farptr to byte after compressed data.
;
; Uses: _bp, _di, _ax, _bx, _cx, _dh !
;

zx0_to_ram	.proc

		tma3				; Preserve MPR3.
		pha

		jsr	set_bp_to_mpr3		; Map zx0_srcptr to MPR3.

		ldx	#$40			; Initialize bit-buffer.

		ldy	#$FF			; Initialize offset to $FFFF.
		sty	<zx0_offset + 0
		sty	<zx0_offset + 1

		iny				; Initialize hi-byte of length
		sty	<zx0_length + 1		; to zero.

.lz_finished:	iny				; Initialize length back to 1.
		sty	<zx0_length + 0

		txa				; Restore bit-buffer.

		asl	a			; Copy from literals or new offset?
		bcc	.cp_literals

		;
		; Copy bytes from new offset.
		;

.new_offset:	jsr	.get_gamma_flag		; Get offset MSB, returns CS.

		cla				; Negate offset MSB and check
		sbc	<zx0_length + 0		; for zero (EOF marker).
		beq	.got_eof

		sec
		ror	a
		sta	<zx0_offset + 1		; Save offset MSB.

		lda	[zx0_srcptr]		; Get offset LSB.
		inc	<zx0_srcptr + 0
		beq	.inc_off_src

.off_skip1:	ror	a			; Last offset bit starts gamma.
		sta	<zx0_offset + 0		; Save offset LSB.

		lda	#-2			; Minimum length of 2?
		bcs	.get_lz_dst

		sty	<zx0_length + 0		; Initialize length back to 1.

		txa				; Restore bit-buffer.

		bsr	.get_gamma_data		; Get length, returns CS.

		lda	<zx0_length + 0		; Negate lo-byte of (length+1).
		eor	#$FF

;		bne	.get_lz_dst		; N.B. Optimized to do nothing!
;
;		inc	<zx0_length + 1		; Increment from (length+1).
;		dec	<zx0_length + 1		; Decrement because lo-byte=0.

.get_lz_dst:	tay				; Calc address of partial page.
		eor	#$FF
		adc	<zx0_dstptr + 0		; Always CS from .get_gamma_data.
		sta	<zx0_dstptr + 0
		bcs	.get_lz_win

		dec	<zx0_dstptr + 1

.get_lz_win:	clc				; Calc address of match.
		adc	<zx0_offset + 0		; N.B. Offset is negative!
		sta	<zx0_winptr + 0
		lda	<zx0_dstptr + 1
		adc	<zx0_offset + 1
		sta	<zx0_winptr + 1

.lz_byte:	lda	[zx0_winptr], y		; Copy bytes from window into
		sta	[zx0_dstptr], y		; decompressed data.
		iny
		bne	.lz_byte
		inc	<zx0_dstptr + 1

		lda	<zx0_length + 1		; Any full pages left to copy?
		beq	.lz_finished

		dec	<zx0_length + 1		; This is rare, so slower.
		inc	<zx0_winptr + 1
		bra	.lz_byte

.got_eof:	tma3				; Return final MPR3 in Y reg.
		tay

		pla				; Restore MPR3.
		tam3

		leave				; Finished decompression!

		;
		; Copy bytes from compressed source.
		;

.cp_literals:	bsr	.get_gamma_flag		; Get length, returns CS.

		ldy	<zx0_length + 0		; Check if lo-byte of length
		bne	.cp_byte		; == 0 without effecting CS.

.cp_page:	dec	<zx0_length + 1		; Decrement # pages to copy.

.cp_byte:	lda	[zx0_srcptr]		; Copy bytes from compressed
		sta	[zx0_dstptr]		; data to decompressed data.

		inc	<zx0_srcptr + 0
		beq	.inc_cp_src
.cp_skip1:	inc	<zx0_dstptr + 0
		beq	.inc_cp_dst

.cp_skip2:	dey				; Any bytes left to copy?
		bne	.cp_byte

		lda	<zx0_length + 1		; Any pages left to copy?
		bne	.cp_page		; Optimized for branch-unlikely.

		iny				; Initialize length back to 1.
		sty	<zx0_length + 0

		txa				; Restore bit-buffer.

		asl	a			; Copy from last offset or new offset?
		bcs	.new_offset

		;
		; Copy bytes from last offset (rare so slower).
		;

.old_offset:	bsr	.get_gamma_flag		; Get length, returns CS.

		cla				; Negate the lo-byte of length.
		sbc	<zx0_length + 0
		sec				; Ensure CS before .get_lz_dst!
		bne	.get_lz_dst

		dec	<zx0_length + 1		; Decrement because lo-byte=0.
		bra	.get_lz_dst

		;
		; Optimized handling of pointers crossing page-boundaries.
		;

.inc_off_src:	jsr	inc.h_bp_mpr3
		bra	.off_skip1

.inc_cp_src:	jsr	inc.h_bp_mpr3
		bra	.cp_skip1

.inc_cp_dst:	inc	<zx0_dstptr + 1
		bra	.cp_skip2

.gamma_page:	jsr	inc.h_bp_mpr3
		bra	.gamma_skip1

		;
		; Get 16-bit interlaced Elias gamma value.
		;

.get_gamma_data:asl	a			; Get next bit.
		rol	<zx0_length + 0
.get_gamma_flag:asl	a
		bcc	.get_gamma_data		; Loop until finished or empty.
		bne	.gamma_done		; Bit-buffer empty?

.gamma_reload:	lda	[zx0_srcptr]		; Reload the empty bit-buffer
		inc	<zx0_srcptr + 0		; from the compressed source.
		beq	.gamma_page
.gamma_skip1:	rol	a
		bcs	.gamma_done		; Finished?

.get_gamma_loop:asl	a			; Get next bit.
		rol	<zx0_length + 0
		rol	<zx0_length + 1
		asl	a
		bcc	.get_gamma_loop		; Loop until finished or empty.
		beq	.gamma_reload		; Bit-buffer empty?

.gamma_done:	tax				; Preserve bit-buffer.
		rts

		.endp



; ***************************************************************************
; ***************************************************************************
;
; zx0_to_vdc - Decompress data stored in Einar Saukas's ZX0 "classic" format.
;
; Args: _bp, Y = _farptr to compressed data in MPR3.
; Args: _di = ptr to output address in VRAM.
;
; Returns: _bp, Y = _farptr to byte after compressed data.
;
; Uses: _bp, _di, _ax, _bx, _cx, _dl, _dh!
;

		.procgroup			; Group code in the same bank.

	.if	SUPPORT_SGX
zx0_to_sgx	.proc
		ldx	#SGX_VDC_OFFSET		; Offset to SGX VDC.
		db	$F0			; Turn "clx" into a "beq".
		.endp
	.endif

zx0_to_vdc	.proc

		clx				; Offset to PCE VDC.

		tma3				; Preserve MPR3.
		pha

		jsr	set_bp_to_mpr3		; Map zx0_srcptr to MPR3.

		jsr	set_di_to_mawr		; Map zx0_dstptr to VRAM.

		lda	#$40			; Initialize bit-buffer.
		sta	<zx0_bitbuf

		ldy	#$FF			; Initialize offset to $FFFF.
		sty	<zx0_offset + 0
		sty	<zx0_offset + 1

		iny				; Initialize hi-byte of length
		sty	<zx0_length + 1		; to zero.

		lda	#ZX0_WINBUF		; Initialize window ring-buffer
		sta	<zx0_dstptr + 1		; location in RAM.
		sty	<zx0_dstptr + 0

.lz_finished:	iny				; Initialize length back to 1.
		sty	<zx0_length + 0

		lda	<zx0_bitbuf		; Restore bit-buffer.

		asl	a			; Copy from literals or new offset?
		bcc	.cp_literals

		;
		; Copy bytes from new offset.
		;

.new_offset:	jsr	.get_gamma_flag		; Get offset MSB, returns CS.

		cla				; Negate offset MSB and check
		sbc	<zx0_length + 0		; for zero (EOF marker).
		beq	.got_eof

		sec
		ror	a
		sta	<zx0_offset + 1		; Save offset MSB.

		lda	[zx0_srcptr]		; Get offset LSB.
		inc	<zx0_srcptr + 0
		beq	.inc_off_src

.off_skip1:	ror	a			; Last offset bit starts gamma.
		sta	<zx0_offset + 0		; Save offset LSB.

		bcs	.got_lz_two		; Minimum length of 2?

		sty	<zx0_length + 0		; Initialize length back to 1.

		lda	<zx0_bitbuf		; Restore bit-buffer.

		jsr	.get_gamma_data		; Get length, returns CS.

		ldy	<zx0_length + 0		; Get lo-byte of (length+1).
.got_lz_two:	iny

;		bne	.get_lz_win		; N.B. Optimized to do nothing!
;
;		inc	<zx0_length + 1		; Increment from (length+1).
;		dec	<zx0_length + 1		; Decrement because lo-byte=0.

.get_lz_win:	clc				; Calc address of match.
		lda	<zx0_dstptr + 0		; N.B. Offset is negative!
		adc	<zx0_offset + 0
		sta	<zx0_winptr + 0
		lda	<zx0_dstptr + 1
		adc	<zx0_offset + 1
		and	#ZX0_WINMSK
		ora	#ZX0_WINBUF
		sta	<zx0_winptr + 1

.lz_byte:	lda	[zx0_winptr]		; Copy bytes from window into
		sta	[zx0_dstptr]		; decompressed data.
		sta	VDC_DL, x
		txa
		eor	#1
		tax

		inc	<zx0_winptr + 0
		beq	.inc_lz_win
.lz_skip1:	inc	<zx0_dstptr + 0
		beq	.inc_lz_dst

.lz_skip2:	dey				; Any bytes left to copy?
		bne	.lz_byte

		lda	<zx0_length + 1		; Any pages left to copy?
		beq	.lz_finished		; Optimized for branch-likely.

		dec	<zx0_length + 1		; This is rare, so slower.
		bra	.lz_byte

		;
		; Copy bytes from compressed source.
		;

.cp_literals:	bsr	.get_gamma_flag		; Get length, returns CS.

		ldy	<zx0_length + 0		; Check the lo-byte of length
		bne	.cp_byte		; without effecting CS.

.cp_page:	dec	<zx0_length + 1

.cp_byte:	lda	[zx0_srcptr]		; Copy bytes from compressed
		sta	[zx0_dstptr]		; data to decompressed data.
		sta	VDC_DL, x
		txa
		eor	#1
		tax

		inc	<zx0_srcptr + 0
		beq	.inc_cp_src
.cp_skip1:	inc	<zx0_dstptr + 0
		beq	.inc_cp_dst

.cp_skip2:	dey				; Any bytes left to copy?
		bne	.cp_byte

		lda	<zx0_length + 1		; Any pages left to copy?
		bne	.cp_page		; Optimized for branch-unlikely.

		iny				; Initialize length back to 1.
		sty	<zx0_length + 0

		lda	<zx0_bitbuf		; Restore bit-buffer.

		asl	a			; Copy from last offset or new offset?
		bcs	.new_offset

		;
		; Copy bytes from last offset (rare so slower).
		;

.old_offset:	bsr	.get_gamma_flag		; Get length, returns CS.

		ldy	<zx0_length + 0		; Check the lo-byte of length.
		bne	.get_lz_win

		dec	<zx0_length + 1		; Decrement because lo-byte=0.
		bra	.get_lz_win

		;
		; All done!
		;

.got_eof:	tma3				; Return final MPR3 in Y reg.
		tay

		pla				; Restore MPR3.
		tam3

		leave				; Finished decompression!

		;
		; Optimized handling of pointers crossing page-boundaries.
		;

.inc_off_src:	jsr	inc.h_bp_mpr3
		bra	.off_skip1

.inc_lz_dst:	bsr	.next_dstpage
		bra	.lz_skip2

.inc_cp_src:	jsr	inc.h_bp_mpr3
		bra	.cp_skip1

.inc_cp_dst:	bsr	.next_dstpage
		bra	.cp_skip2

.inc_lz_win:	lda	<zx0_winptr + 1
		inc	a
		and	#ZX0_WINMSK
		ora	#ZX0_WINBUF
		sta	<zx0_winptr + 1
		bra	.lz_skip1

.next_dstpage:	lda	<zx0_dstptr + 1
		inc	a
		and	#ZX0_WINMSK
		ora	#ZX0_WINBUF
		sta	<zx0_dstptr + 1
		rts

.gamma_page:	jsr	inc.h_bp_mpr3
		bra	.gamma_skip1

		;
		; Get 16-bit interlaced Elias gamma value.
		;

.get_gamma_data:asl	a			; Get next bit.
		rol	<zx0_length + 0
.get_gamma_flag:asl	a
		bcc	.get_gamma_data		; Loop until finished or empty.
		bne	.gamma_done		; Bit-buffer empty?

.gamma_reload:	lda	[zx0_srcptr]		; Reload the empty bit-buffer
		inc	<zx0_srcptr + 0		; from the compressed source.
		beq	.gamma_page
.gamma_skip1:	rol	a
		bcs	.gamma_done		; Finished?

.get_gamma_loop:asl	a			; Get next bit.
		rol	<zx0_length + 0
		rol	<zx0_length + 1
		asl	a
		bcc	.get_gamma_loop		; Loop until finished or empty.
		beq	.gamma_reload		; Bit-buffer empty?

.gamma_done:	sta	<zx0_bitbuf		; Preserve bit-buffer.
		rts

		.endp
		.endprocgroup



	.if	SUPPORT_ACD

; ***************************************************************************
; ***************************************************************************
;
; zx0_acd_to_ram - Decompress data stored in ZX0 "classic" format.
;
; Args: _bp, Y = _farptr to compressed data in ACD0.
; Args: _di = ptr to output address in RAM.
;
; Uses: _bp, _di, _ax, _bx, _cx, _dh !
;

zx0_acd_to_ram	.proc

		lda.l	<_bp			; Map zx0_srcptr to ACD0.
		sta	ACD0_BASE + 0
		lda.h	<_bp
		sta	ACD0_BASE + 1
		sty	ACD0_BASE + 2

		lda.l	#1
		sta.l	ACD0_INCR
		stz.h	ACD0_INCR
		lda	#$11
		sta	ACD0_CTRL

		ldx	#$40			; Initialize bit-buffer.

		ldy	#$FF			; Initialize offset to $FFFF.
		sty.l	<zx0_offset
		sty.h	<zx0_offset

		iny				; Initialize hi-byte of length
		sty.h	<zx0_length		; to zero.

.lz_finished:	iny				; Initialize length back to 1.
		sty.l	<zx0_length

		txa				; Restore bit-buffer.

		asl	a			; Copy from literals or new offset?
		bcc	.cp_literals

		;
		; Copy bytes from new offset.
		;

.new_offset:	jsr	.get_gamma_flag		; Get offset MSB, returns CS.

		cla				; Negate offset MSB and check
		sbc.l	<zx0_length		; for zero (EOF marker).
		beq	.got_eof

		sec
		ror	a
		sta.h	<zx0_offset		; Save offset MSB.

		lda	ACD0_DATA		; Get offset LSB.
		ror	a			; Last offset bit starts gamma.
		sta.l	<zx0_offset		; Save offset LSB.

		lda	#-2			; Minimum length of 2?
		bcs	.get_lz_dst

		sty.l	<zx0_length		; Initialize length back to 1.

		txa				; Restore bit-buffer.

		bsr	.get_gamma_data		; Get length, returns CS.

		lda.l	<zx0_length		; Negate lo-byte of (length+1).
		eor	#$FF

;		bne	.get_lz_dst		; N.B. Optimized to do nothing!
;
;		inc.h	<zx0_length		; Increment from (length+1).
;		dec.h	<zx0_length		; Decrement because lo-byte=0.

.get_lz_dst:	tay				; Calc address of partial page.
		eor	#$FF
		adc.l	<zx0_dstptr		; Always CS from .get_gamma_data.
		sta.l	<zx0_dstptr
		bcs	.get_lz_win

		dec.h	<zx0_dstptr

.get_lz_win:	clc				; Calc address of match.
		adc.l	<zx0_offset		; N.B. Offset is negative!
		sta.l	<zx0_winptr
		lda.h	<zx0_dstptr
		adc.h	<zx0_offset
		sta.h	<zx0_winptr

.lz_byte:	lda	[zx0_winptr], y		; Copy bytes from window into
		sta	[zx0_dstptr], y		; decompressed data.
		iny
		bne	.lz_byte
		inc.h	<zx0_dstptr

		lda.h	<zx0_length		; Any full pages left to copy?
		beq	.lz_finished

		dec.h	<zx0_length		; This is rare, so slower.
		inc.h	<zx0_winptr
		bra	.lz_byte

.got_eof:	leave				; Finished decompression!

		;
		; Copy bytes from compressed source.
		;

.cp_literals:	bsr	.get_gamma_flag		; Get length, returns CS.

		ldy.l	<zx0_length		; Check if lo-byte of length
		bne	.cp_byte		; == 0 without effecting CS.

.cp_page:	dec.h	<zx0_length		; Decrement # pages to copy.

.cp_byte:	lda	ACD0_DATA		; Copy bytes from compressed
		sta	[zx0_dstptr]		; data to decompressed data.

.cp_skip1:	inc.l	<zx0_dstptr
		beq	.inc_cp_dst

.cp_skip2:	dey				; Any bytes left to copy?
		bne	.cp_byte

		lda.h	<zx0_length		; Any pages left to copy?
		bne	.cp_page		; Optimized for branch-unlikely.

		iny				; Initialize length back to 1.
		sty.l	<zx0_length

		txa				; Restore bit-buffer.

		asl	a			; Copy from last offset or new offset?
		bcs	.new_offset

		;
		; Copy bytes from last offset (rare so slower).
		;

.old_offset:	bsr	.get_gamma_flag		; Get length, returns CS.

		cla				; Negate the lo-byte of length.
		sbc.l	<zx0_length
		sec				; Ensure CS before .get_lz_dst!
		bne	.get_lz_dst

		dec.h	<zx0_length		; Decrement because lo-byte=0.
		bra	.get_lz_dst

		;
		; Optimized handling of pointers crossing page-boundaries.
		;

.inc_cp_dst:	inc.h	<zx0_dstptr
		bra	.cp_skip2

		;
		; Get 16-bit interlaced Elias gamma value.
		;

.get_gamma_data:asl	a			; Get next bit.
		rol.l	<zx0_length
.get_gamma_flag:asl	a
		bcc	.get_gamma_data		; Loop until finished or empty.
		bne	.gamma_done		; Bit-buffer empty?

.gamma_reload:	lda	ACD0_DATA		; Reload the empty bit-buffer
.gamma_skip1:	rol	a			; from the compressed source.
		bcs	.gamma_done		; Finished?

.get_gamma_loop:asl	a			; Get next bit.
		rol.l	<zx0_length
		rol.h	<zx0_length
		asl	a
		bcc	.get_gamma_loop		; Loop until finished or empty.
		beq	.gamma_reload		; Bit-buffer empty?

.gamma_done:	tax				; Preserve bit-buffer.
		rts

		.endp

	.endif	SUPPORT_ACD
