; ***************************************************************************
; ***************************************************************************
;
; unpack-zx0.asm
;
; HuC6280 decompressor for Einar Saukas's "classic" ZX0 format.
;
; The code length is 205 bytes for RAM, plus 222 (shorter) or 262 (faster)
; bytes for direct-to-VRAM, plus some generic utility code.
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

	.ifndef SUPPORT_ACD
SUPPORT_ACD	=	1
	.endif

	.ifndef SUPPORT_ZX0RAM
SUPPORT_ZX0RAM	=	1			; Include decompress-to-RAM?
	.endif

	.ifndef SUPPORT_ZX0VRAM
SUPPORT_ZX0VRAM =	1			; Include decompress-to-VRAM?
	.endif

	.ifndef ZX0_PREFER_SIZE
ZX0_PREFER_SIZE =	1			; Smaller size or 10% faster.
	.endif



; ***************************************************************************
; ***************************************************************************
;
; If you decompress directly to VRAM, then you need to define a ring-buffer
; in RAM, both sized and aligned to a power-of-two (i.e. 256, 512, 1KB, 2KB).
;
; You also need to make sure that you tell the compressor that it needs to
; limit the window size with its "-w" option.
;
; Note that CD-ROM developers should really just decompress to RAM, and then
; use a TIA to copy the data to VRAM, because that is faster, you get better
; compression without a window, and you save code memory by not needing both
; versions of the decompression routine.
;
; HuCARD developers who either can't afford the RAM, or who prefer execution
; speed over compression, can choose a ring-buffer size of 256 bytes so that
; a faster version of the decompression code is used.
;

	.ifndef ZX0_WINBUF

ZX0_WINBUF	=	($3800)			; Default to a 2KB window in
ZX0_WINMSK	=	($0800 - 1)		; RAM, located at $3800.

	.endif



; ***************************************************************************
; ***************************************************************************
;
; Data usage is 10..14 bytes of zero-page, using aliases for clarity.
;

zx0_srcptr	=	_bp			; 1 word.
zx0_dstptr	=	_di			; 1 word.
zx0_winptr	=	_si			; 1 word.

zx0_length	=	_ax			; 1 word.
zx0_offset	=	_bx			; 1 word.



	.if	(SUPPORT_ZX0RAM + SUPPORT_ZX0VRAM)

zx0_group	.procgroup			; Keep RAM and VDC together.

	.if	SUPPORT_ZX0RAM

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
; Uses: _bp, _di, _si, _ax, _bx !
;

zx0_to_ram	.proc

		tma3				; Preserve MPR3.
		pha

		jsr	set_bp_to_mpr3		; Map zx0_srcptr to MPR3.

		ldx	#$40			; Initialize bit-buffer.

		lda	#$FF			; Initialize offset to $FFFF.
		sta.l	<zx0_offset
		sta.h	<zx0_offset

		stz.h	<zx0_length		; Initialize hi-byte of length.

		;
		; Decide what to do next.
		;

.lz_finished:	ldy	#1			; Initialize length back to 1.
		sty.l	<zx0_length

		txa				; Restore bit-buffer.

		asl	a			; Copy from literals or new offset?
		bcc	.cp_literals

		;
		; Copy bytes from new offset.
		;

.new_offset:	jsr	zx0_gamma_flag		; Get offset MSB, returns CS.

		tax				; Preserve bit-buffer.

		cla				; Negate offset MSB and check
		sbc.l	<zx0_length		; for zero (EOF marker).
		beq	.got_eof

		sec
		ror	a
		sta.h	<zx0_offset		; Save offset MSB.

		lda	[zx0_srcptr]		; Get offset LSB.
		inc.l	<zx0_srcptr
		beq	.inc_off_src

.next_off_src:	ror	a			; Last offset bit starts gamma.
		sta.l	<zx0_offset		; Save offset LSB.

		lda	#-2			; Minimum length of 2?
		bcs	.get_lz_dst

		sty.l	<zx0_length		; Initialize length back to 1.

		txa				; Restore bit-buffer.

		bsr	zx0_gamma_data		; Get length, returns CS.

		tax				; Preserve bit-buffer.

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

		;
		; All done!
		;

.got_eof:	tma3				; Return final MPR3 in Y reg.
		tay

		pla				; Restore MPR3.
		tam3

		leave				; Finished decompression!

		;
		; Copy bytes from compressed source.
		;

.cp_literals:	bsr	zx0_gamma_flag		; Get length, returns CS.

		tax				; Preserve bit-buffer.

		ldy	<zx0_length + 0		; Check if lo-byte of length
		bne	.cp_byte		; == 0 without effecting CS.

.cp_page:	dec	<zx0_length + 1		; Decrement # pages to copy.

.cp_byte:	lda	[zx0_srcptr]		; Copy bytes from compressed
		sta	[zx0_dstptr]		; data to decompressed data.

		inc	<zx0_srcptr + 0
		beq	.inc_cp_src
.next_cp_src:	inc	<zx0_dstptr + 0
		beq	.inc_cp_dst

.next_cp_dst:	dey				; Any bytes left to copy?
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

.old_offset:	bsr	zx0_gamma_flag		; Get length, returns CS.

		tax				; Preserve bit-buffer.

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
		bra	.next_off_src

.inc_cp_src:	jsr	inc.h_bp_mpr3
		bra	.next_cp_src

.inc_cp_dst:	inc	<zx0_dstptr + 1
		bra	.next_cp_dst

		.endp

	.endif	SUPPORT_ZX0RAM



; ***************************************************************************
; ***************************************************************************
;
; zx0_gamma_data - Get 16-bit interlaced Elias gamma value.
; zx0_gamma_flag - Get 16-bit interlaced Elias gamma value.
;
; Shared support routine used by zx0_to_ram and zx0_to_vdc/zx0_to_sgx.
;

zx0_gamma_data: asl	a			; Get next bit.
		rol.l	<zx0_length
zx0_gamma_flag: asl	a
		bcc	zx0_gamma_data		; Loop until finished or empty.
		bne	.gamma_done		; Bit-buffer empty?

.gamma_reload:	lda	[zx0_srcptr]		; Reload the empty bit-buffer
		inc.l	<zx0_srcptr		; from the compressed source.
		beq	.gamma_page
.gamma_next:	rol	a
		bcs	.gamma_done		; Finished?

.get_gamma_loop:asl	a			; Get next bit.
		rol.l	<zx0_length
		rol.h	<zx0_length
		asl	a
		bcc	.get_gamma_loop		; Loop until finished or empty.
		beq	.gamma_reload		; Bit-buffer empty?

.gamma_done:	rts

.gamma_page:	jsr	inc.h_bp_mpr3
		bra	.gamma_next



	.if	SUPPORT_ZX0VRAM

	.if	ZX0_PREFER_SIZE

; ***************************************************************************
; ***************************************************************************
;
; zx0_to_sgx - Decompress data stored in Einar Saukas's ZX0 "classic" format.
; zx0_to_vdc - Decompress data stored in Einar Saukas's ZX0 "classic" format.
;
; Args: _bp, Y = _farptr to compressed data in MPR3.
; Args: _di = ptr to output address in VRAM.
;
; Returns: _bp, Y = _farptr to byte after compressed data.
;
; Uses: _bp, _di, _si, _ax, _bx !
;

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
		pha				; Preserve bit-buffer.

		lda	#$FF			; Initialize offset to $FFFF.
		sta.l	<zx0_offset
		sta.h	<zx0_offset

		stz.h	<zx0_length		; Initialize hi-byte of length.

		stz.l	<zx0_dstptr		; Initialize window ring-buffer
		lda.h	#ZX0_WINBUF		; location in RAM.
		sta.h	<zx0_dstptr

		bra	.lz_finished		; Let's get started!

		;
		; Optimized handling of pointers crossing page-boundaries.
		;

.inc_cp_src:	jsr	inc.h_bp_mpr3
		bra	.next_cp_src

.inc_cp_dst:	bsr	.next_dstpage
		bra	.next_cp_dst

		;
		; Copy bytes from compressed source.
		;

.cp_literals:	jsr	zx0_gamma_flag		; Get length, returns CS.

		pha				; Preserve bit-buffer.

		ldy.l	<zx0_length		; Check the lo-byte of length
		bne	.cp_byte		; without effecting CS.

.cp_page:	dec.h	<zx0_length

.cp_byte:	lda	[zx0_srcptr]		; Copy byte from compressed
		sta	[zx0_dstptr]		; data to decompressed data.
		sta	VDC_DL, x
		txa
		eor	#1
		tax

		inc.l	<zx0_srcptr
		beq	.inc_cp_src
.next_cp_src:	inc.l	<zx0_dstptr
		beq	.inc_cp_dst

.next_cp_dst:	dey				; Any bytes left to copy?
		bne	.cp_byte

		lda.h	<zx0_length		; Any pages left to copy?
		bne	.cp_page		; Optimized for branch-unlikely.

		iny				; Initialize length back to 1.
		sty.l	<zx0_length

		pla				; Restore bit-buffer.

		asl	a			; Copy from last offset or new offset?
		bcs	.new_offset

		;
		; Copy bytes from last offset (rare so slower).
		;

.old_offset:	jsr	zx0_gamma_flag		; Get length, returns CS.

		pha				; Preserve bit-buffer.

		ldy.l	<zx0_length		; Check the lo-byte of length.
		bne	.get_lz_win

		dec.h	<zx0_length		; Decrement because lo-byte=0.
		bra	.get_lz_win

		;
		; Increment destination page.
		;

.next_dstpage:	lda.h	<zx0_dstptr
		inc	a
		and.h	#ZX0_WINMSK
		ora.h	#ZX0_WINBUF
		sta.h	<zx0_dstptr
		rts

		;
		; Decide what to do next.
		;

.lz_finished:	ldy	#1			; Initialize length back to 1.
		sty.l	<zx0_length

		pla				; Restore bit-buffer.

		asl	a			; Copy from literals or new offset?
		bcc	.cp_literals

		;
		; Copy bytes from new offset.
		;

.new_offset:	jsr	zx0_gamma_flag		; Get offset MSB, returns CS.

		pha				; Preserve bit-buffer.

		cla				; Negate offset MSB and check
		sbc.l	<zx0_length		; for zero (EOF marker).
		beq	.got_eof

		sec
		ror	a
		sta.h	<zx0_offset		; Save offset MSB.

		lda	[zx0_srcptr]		; Get offset LSB.
		inc.l	<zx0_srcptr
		beq	.inc_off_src

.next_off_src:	ror	a			; Last offset bit starts gamma.
		sta.l	<zx0_offset		; Save offset LSB.

		bcs	.got_lz_two		; Minimum length of 2?

		sty.l	<zx0_length		; Initialize length back to 1.

		pla				; Restore bit-buffer.

		jsr	zx0_gamma_data		; Get length, returns CS.

		pha				; Preserve bit-buffer.

		ldy.l	<zx0_length		; Get lo-byte of (length+1).
.got_lz_two:	iny

;		bne	.get_lz_win		; N.B. Optimized to do nothing!
;
;		inc.h	<zx0_length		; Increment from (length+1).
;		dec.h	<zx0_length		; Decrement because lo-byte=0.

.get_lz_win:	clc				; Calc address of match.
		lda.l	<zx0_dstptr		; N.B. Offset is negative!
		adc.l	<zx0_offset
		sta.l	<zx0_winptr
		lda.h	<zx0_dstptr
		adc.h	<zx0_offset
		and.h	#ZX0_WINMSK
		ora.h	#ZX0_WINBUF
		sta.h	<zx0_winptr

.lz_byte:	lda	[zx0_winptr]		; Copy byte from window into
		sta	[zx0_dstptr]		; decompressed data.
		sta	VDC_DL, x
		txa
		eor	#1
		tax

		inc.l	<zx0_winptr
		beq	.inc_lz_win
.next_lz_win:	inc.l	<zx0_dstptr
		beq	.inc_lz_dst

.next_lz_dst:	dey				; Any bytes left to copy?
		bne	.lz_byte

		lda.h	<zx0_length		; Any pages left to copy?
		beq	.lz_finished		; Optimized for branch-likely.

		dec.h	<zx0_length		; This is rare, so slower.
		bra	.lz_byte

		;
		; Optimized handling of pointers crossing page-boundaries.
		;

.inc_off_src:	jsr	inc.h_bp_mpr3
		bra	.next_off_src

.inc_lz_dst:	bsr	.next_dstpage
		bra	.next_lz_dst

.inc_lz_win:	lda.h	<zx0_winptr
		inc	a
		and.h	#ZX0_WINMSK
		ora.h	#ZX0_WINBUF
		sta.h	<zx0_winptr
		bra	.next_lz_win

		;
		; All done!
		;

.got_eof:	pla				; Discard bit-buffer.

		tma3				; Return final MPR3 in Y reg.
		tay

		pla				; Restore MPR3.
		tam3

		leave				; Finished decompression!

		.endp

	.else	ZX0_PREFER_SIZE



; ***************************************************************************
; ***************************************************************************
;
; zx0_to_sgx - Decompress data stored in Einar Saukas's ZX0 "classic" format.
; zx0_to_vdc - Decompress data stored in Einar Saukas's ZX0 "classic" format.
;
; Args: _bp, Y = _farptr to compressed data in MPR3.
; Args: _di = ptr to output address in VRAM.
;
; Returns: _bp, Y = _farptr to byte after compressed data.
;
; Uses: _bp, _di, _si, _ax, _bx, _cx, _dx!
;

	.if	SUPPORT_SGX

zx0_to_sgx	.proc
		ldx	#SGX_VDC_OFFSET		; Offset to SGX VDC.
		db	$F0			; Turn "clx" into a "beq".
		.endp

zx0_vdc_dl	=	_cx			; 1 word.
zx0_vdc_dh	=	_dx			; 1 word.

WRITE_VRAM_LO	.macro
		sta	[zx0_vdc_dl]
		.endm
WRITE_VRAM_HI	.macro
		sta	[zx0_vdc_dh]
		.endm

	.else

WRITE_VRAM_LO	.macro
		sta	VDC_DL
		.endm
WRITE_VRAM_HI	.macro
		sta	VDC_DH
		.endm

	.endif

zx0_to_vdc	.proc

		clx				; Offset to PCE VDC.

		tma3				; Preserve MPR3.
		pha

		jsr	set_bp_to_mpr3		; Map zx0_srcptr to MPR3.

		jsr	vdc_di_to_mawr		; Map zx0_dstptr to VRAM.

	.if	SUPPORT_SGX
		inx				; Use ZP aliases for which
		inx				; VDC chip to output to on
		stx.l	<zx0_vdc_dl		; a SuperGRAFX.
		stz.h	<zx0_vdc_dl		; It's a few cycles slower
		inx				; but you could decompress
		stx.l	<zx0_vdc_dh		; to RAM and copy if speed
		stz.h	<zx0_vdc_dh		; is a critical concern!
	.endif

		lda	#$40			; Initialize bit-buffer.
		pha				; Preserve bit-buffer.

		lda	#$FF			; Initialize offset to $FFFF.
		sta.l	<zx0_offset
		sta.h	<zx0_offset

		stz.h	<zx0_length		; Initialize hi-byte of length.

		cly				; Initialize window offset.

	.if	(ZX0_WINMSK == 0x00FF)

		; ****************************************
		; VERSION OPTIMIZED FOR WINDOW SIZE == 256
		; ****************************************

		bra	.lz_finished		; Let's get started!

		;
		; Optimized handling of pointers crossing page-boundaries.
		;

.inc_cp_src1:	jsr	inc.h_bp_mpr3
		bra	.nxt_cp_src1

.inc_cp_src2:	jsr	inc.h_bp_mpr3
		bra	.nxt_cp_src2

		;
		; Copy bytes from compressed source.
		;

.cp_literals:	jsr	zx0_gamma_flag		; Get length, returns CS.

		pha				; Preserve bit-buffer.

		ldx.l	<zx0_length		; Check the lo-byte of length.
		bne	.cp_which

.cp_page:	dec.h	<zx0_length

.cp_which:	tya				; Is this written to an even or
		lsr	a			; odd output address?
		bcs	.cp_odd

.cp_even:	lda	[zx0_srcptr]		; Copy byte from compressed
		inc.l	<zx0_srcptr		; data to decompressed data.
		beq	.inc_cp_src1
.nxt_cp_src1:	WRITE_VRAM_LO
		sta	ZX0_WINBUF, y
		iny


		dex				; Any bytes left to copy?
		beq	.cp_next

.cp_odd:	lda	[zx0_srcptr]		; Copy byte from compressed
		inc.l	<zx0_srcptr		; data to decompressed data.
		beq	.inc_cp_src2
.nxt_cp_src2:	WRITE_VRAM_HI
		sta	ZX0_WINBUF, y
		iny

		dex				; Any bytes left to copy?
		bne	.cp_even

.cp_next:	lda.h	<zx0_length		; Any pages left to copy?
		bne	.cp_page		; Optimized for branch-unlikely.

		inx				; Initialize length back to 1.
		stx.l	<zx0_length

		pla				; Restore bit-buffer.

		asl	a			; Copy from last offset or new offset?
		bcs	.new_offset

		;
		; Copy bytes from last offset (rare so slower).
		;

.old_offset:	jsr	zx0_gamma_flag		; Get length, returns CS.

		pha				; Preserve bit-buffer.

		ldx.l	<zx0_length		; Check the lo-byte of length.
		bne	.get_lz_win

		dec.h	<zx0_length		; Decrement because lo-byte=0.
		bra	.get_lz_win

		;
		; Decide what to do next.
		;

.lz_finished:	ldx	#1			; Initialize length back to 1.
		stx.l	<zx0_length

		pla				; Restore bit-buffer.

		asl	a			; Copy from literals or new offset?
		bcc	.cp_literals

		;
		; Copy bytes from new offset.
		;

.new_offset:	jsr	zx0_gamma_flag		; Get offset MSB, returns CS.

		pha				; Preserve bit-buffer.

		cla				; Negate offset MSB and check
		sbc.l	<zx0_length		; for zero (EOF marker).
		beq	.got_eof

		lsr	a			; Put offset MSB lo-bit in C.

		lda	[zx0_srcptr]		; Get offset LSB.
		inc.l	<zx0_srcptr
		beq	.inc_off_src

.next_off_src:	ror	a			; Last offset bit starts gamma.
		sta.l	<zx0_offset		; Save offset LSB.

		stx.l	<zx0_length		; Initialize length back to 1.

		bcs	.got_lz_two		; Minimum length of 2?

		pla				; Restore bit-buffer.

		jsr	zx0_gamma_data		; Get length, returns CS.

		pha				; Preserve bit-buffer.

.got_lz_two:	inc.l	<zx0_length		; Get lo-byte of (length+1).

;		bne	.get_lz_win		; N.B. Optimized to do nothing!
;
;		inc.h	<zx0_length		; Increment from (length+1).
;		dec.h	<zx0_length		; Decrement because lo-byte=0.

.get_lz_win:	clc				; Calc address of match.
		tya				; N.B. Offset is negative!
		adc.l	<zx0_offset
		tax

.lz_which:	tya				; Is this written to an even or
		lsr	a			; odd output address?
		bcs	.lz_odd

.lz_even:	lda	ZX0_WINBUF, x		; Copy byte from window into
		inx				; decompressed data.
		WRITE_VRAM_LO
		sta	ZX0_WINBUF, y
		iny

		dec	<zx0_length		; Any bytes left to copy?
		beq	.lz_page

.lz_odd:	lda	ZX0_WINBUF, x		; Copy byte from window into
		inx				; decompressed data.
		WRITE_VRAM_HI
		sta	ZX0_WINBUF, y
		iny

		dec	<zx0_length		; Any bytes left to copy?
		bne	.lz_even

.lz_page:	lda.h	<zx0_length		; Any pages left to copy?
		beq	.lz_finished		; Optimized for branch-likely.

		dec.h	<zx0_length		; This is rare, so slower.
		bra	.lz_which

		;
		; Optimized handling of pointers crossing page-boundaries.
		;

.inc_off_src:	jsr	inc.h_bp_mpr3
		bra	.next_off_src

	.else	(ZX0_WINMSK == 0x00FF)

		; ****************************************
		; VERSION OPTIMIZED FOR WINDOW SIZE != 256
		; ****************************************

		stz.l	<zx0_dstptr		; Initialize window ring-buffer
		lda.h	#ZX0_WINBUF		; location in RAM.
		sta.h	<zx0_dstptr

		bra	.lz_finished		; Let's get started!

		;
		; Optimized handling of pointers crossing page-boundaries.
		;

.inc_cp_src1:	jsr	inc.h_bp_mpr3
		bra	.nxt_cp_src1

.inc_cp_src2:	jsr	inc.h_bp_mpr3
		bra	.nxt_cp_src2

.inc_cp_dst2:	bsr	.next_dstpage
		bra	.nxt_cp_dst2

		;
		; Copy bytes from compressed source.
		;

.cp_literals:	jsr	zx0_gamma_flag		; Get length, returns CS.

		pha				; Preserve bit-buffer.

		ldx.l	<zx0_length		; Check the lo-byte of length.
		bne	.cp_which

.cp_page:	dec.h	<zx0_length

.cp_which:	tya				; Is this written to an even or
		lsr	a			; odd output address?
		bcs	.cp_odd

.cp_even:	lda	[zx0_srcptr]		; Copy byte from compressed
		sta	[zx0_dstptr], y		; data to decompressed data.
		WRITE_VRAM_LO

		inc.l	<zx0_srcptr
		beq	.inc_cp_src1
.nxt_cp_src1:	iny

		dex				; Any bytes left to copy?
		beq	.cp_next

.cp_odd:	lda	[zx0_srcptr]		; Copy byte from compressed
		sta	[zx0_dstptr], y		; data to decompressed data.
		WRITE_VRAM_HI

		inc.l	<zx0_srcptr
		beq	.inc_cp_src2
.nxt_cp_src2:	iny
		beq	.inc_cp_dst2

.nxt_cp_dst2:	dex				; Any bytes left to copy?
		bne	.cp_even

.cp_next:	lda.h	<zx0_length		; Any pages left to copy?
		bne	.cp_page		; Optimized for branch-unlikely.

		inx				; Initialize length back to 1.
		stx.l	<zx0_length

		pla				; Restore bit-buffer.

		asl	a			; Copy from last offset or new offset?
		bcs	.new_offset

		;
		; Copy bytes from last offset (rare so slower).
		;

.old_offset:	jsr	zx0_gamma_flag		; Get length, returns CS.

		pha				; Preserve bit-buffer.

		ldx.l	<zx0_length		; Check the lo-byte of length.
		bne	.get_lz_win

		dec.h	<zx0_length		; Decrement because lo-byte=0.
		bra	.get_lz_win

		;
		; Increment destination page.
		;

.next_dstpage:	lda.h	<zx0_dstptr
		inc	a
		and.h	#ZX0_WINMSK
		ora.h	#ZX0_WINBUF
		sta.h	<zx0_dstptr
		rts

		;
		; Decide what to do next.
		;

.lz_finished:	ldx	#1			; Initialize length back to 1.
		stx.l	<zx0_length

		pla				; Restore bit-buffer.

		asl	a			; Copy from literals or new offset?
		bcc	.cp_literals

		;
		; Copy bytes from new offset.
		;

.new_offset:	jsr	zx0_gamma_flag		; Get offset MSB, returns CS.

		pha				; Preserve bit-buffer.

		cla				; Negate offset MSB and check
		sbc.l	<zx0_length		; for zero (EOF marker).
		beq	.got_eof

		sec
		ror	a
		sta.h	<zx0_offset		; Save offset MSB.

		lda	[zx0_srcptr]		; Get offset LSB.
		inc.l	<zx0_srcptr
		beq	.inc_off_src

.next_off_src:	ror	a			; Last offset bit starts gamma.
		sta.l	<zx0_offset		; Save offset LSB.

		bcs	.got_lz_two		; Minimum length of 2?

		stx.l	<zx0_length		; Initialize length back to 1.

		pla				; Restore bit-buffer.

		jsr	zx0_gamma_data		; Get length, returns CS.

		pha				; Preserve bit-buffer.

		ldx.l	<zx0_length		; Get lo-byte of (length+1).
.got_lz_two:	inx

;		bne	.get_lz_win		; N.B. Optimized to do nothing!
;
;		inc.h	<zx0_length		; Increment from (length+1).
;		dec.h	<zx0_length		; Decrement because lo-byte=0.

.get_lz_win:	clc				; Calc address of match.
		tya				; N.B. Offset is negative!
		adc.l	<zx0_offset
		sta.l	<zx0_winptr
		lda.h	<zx0_dstptr
		adc.h	<zx0_offset
		and.h	#ZX0_WINMSK
		ora.h	#ZX0_WINBUF
		sta.h	<zx0_winptr

.lz_which:	tya				; Is this written to an even or
		lsr	a			; odd output address?
		bcs	.lz_odd

.lz_even:	lda	[zx0_winptr]		; Copy byte from window into
		sta	[zx0_dstptr], y		; decompressed data.
		WRITE_VRAM_LO

		inc.l	<zx0_winptr
		beq	.inc_lz_win1
.nxt_lz_win1:	iny

		dex				; Any bytes left to copy?
		beq	.lz_page

.lz_odd:	lda	[zx0_winptr]		; Copy byte from window into
		sta	[zx0_dstptr], y		; decompressed data.
		WRITE_VRAM_HI

		inc.l	<zx0_winptr
		beq	.inc_lz_win2
.nxt_lz_win2:	iny
		beq	.inc_lz_dst2

.nxt_lz_dst2:	dex				; Any bytes left to copy?
		bne	.lz_even

.lz_page:	lda.h	<zx0_length		; Any pages left to copy?
		beq	.lz_finished		; Optimized for branch-likely.

		dec.h	<zx0_length		; This is rare, so slower.
		bra	.lz_which

		;
		; Optimized handling of pointers crossing page-boundaries.
		;

.inc_off_src:	jsr	inc.h_bp_mpr3
		bra	.next_off_src

.inc_lz_dst2:	bsr	.next_dstpage
		bra	.nxt_lz_dst2

.inc_lz_win1:	bsr	.next_winpage
		bra	.nxt_lz_win1

.inc_lz_win2:	bsr	.next_winpage
		bra	.nxt_lz_win2

		;
		; Increment window page.
		;

.next_winpage:	lda.h	<zx0_winptr
		inc	a
		and.h	#ZX0_WINMSK
		ora.h	#ZX0_WINBUF
		sta.h	<zx0_winptr
		rts

	.endif	(ZX0_WINMSK == 0x00FF)

		;
		; All done!
		;

.got_eof:	pla				; Discard bit-buffer.

		tma3				; Return final MPR3 in Y reg.
		tay

		pla				; Restore MPR3.
		tam3

		leave				; Finished decompression!

		.endp

	.endif	ZX0_PREFER_SIZE

	.endif	SUPPORT_ZX0VRAM

		.endprocgroup			; zx0_group

	.endif	(SUPPORT_ZX0RAM + SUPPORT_ZX0VRAM)



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
