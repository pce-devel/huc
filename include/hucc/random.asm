; ***************************************************************************
; ***************************************************************************
;
; random.asm
;
; Pseudo-random number generator (https://github.com/bbbradsmith/prng_6502)
;
; Copyright Brad Smith 2019.
;
; License:
;
; This code and may be used, reused, and modified for any purpose, commercial
; or non-commercial.
;
; Attribution in released binaries or documentation is appreciated but not
; required.
;
; ***************************************************************************
; ***************************************************************************
;
; This is a linear feedback shift register (LFSR) in Galois form, which is
; iterated 8 times to produce an 8-bit pseudo-random number.
;
; Two widths of LFSR are provided:
;
;  24-bit requires 3 bytes, and repeats after 16777215 calls.
;  32-bit requires 4 bytes, and repeats after 4294967295 calls.
;
; Usage:
;
;  Initialize the zero-page "random" variable to any value other than 0.
;  The size of "random" is 3 or 4 bytes, depending on the width of LFSR
;  chosen.
;
;  Call one of the RNG functions and an 8-bit result will be returned in the
;  A-register (with flags), and the Y-register will be clobbered.
;
;  Do not mix RNGs of different width in the same program, unless you can
;  give them each separate "random" state storage.
;
; ***************************************************************************
; ***************************************************************************


	.ifndef	_KICKC				; Variables defined in C?
		.zp
random:		.ds	4			; Seed is 3 or 4 bytes.
		.code
	.endif	_KICKC



; ***************************************************************************
; ***************************************************************************
;
; init_random - Initialize a 32-bit LFSR using an 8-bit seed value in Y.
;
; The LFSR is initialized to n'th entry of a standard CRC-32 lookup-table,
; which gives it a decent distribution of bits.
;
; Since seed is an 8-bit value, there are 255 (256-1) possible starting
; states for the LFSR, because 0 would generate a 0 state.
;
; CRC-32 code by Paul Guertin. See http://6502.org/source/integers/crc.htm
;

init_random	.proc

		lda	#1			; Init CRC-32 table value.
		sta	<random + 0
		tya				; Get and check the seed value.
		bne	.reverse_seed
		dec	a			; Which must be non-zero!
.reverse_seed:	lsr	a			; Reverse the bits so that small changes
		rol	<random + 0		; in the seed make larger differences in
		bcc	.reverse_seed		; the initial state.

		stz	<random + 1		; A contains the high byte of the CRC-32.
		stz	<random + 2		; The other three bytes are in memory.
		cla

		ldy	#8			; Y counts bits in a byte.
.bit_loop:	lsr	a			; The CRC-32 algorithm is similar to CRC-16
		ror	<random + 2		; except that it is reversed (originally for
		ror	<random + 1		; hardware reasons). This is why we shift
		ror	<random + 0		; right instead of left here.
		bcc	.no_add			; Do nothing if no overflow,
		eor	#$ED			; else add CRC-32 polynomial $EDB88320.

		pha				; Save high byte while we do others.
		lda	<random + 2
		eor	#$B8			; Most reference books give the CRC-32 poly
		sta	<random + 2		; as $04C11DB7. This is actually the same if
		lda	<random + 1		; you write it in binary and read it right-
		eor	#$83			; to-left instead of left-to-right. Doing it
		sta	<random + 1		; this way means we won't have to explicitly
		lda	<random + 0		; reverse things afterwards.
		eor	#$20
		sta	<random + 0
		pla				; Restore high byte.

.no_add:	dey				; Do next bit.
		bne	.bit_loop

		sta	<random + 3		; Save CRC-32 high-byte.

		leave				; All done!

		.endp



; ***************************************************************************
; ***************************************************************************
;
; get_random - 8-bit LFSR pseudo-random number with a 24-bit cycle.
;
; The pseudo-random sequence repeats after (2^24)-1 calls.
;
; Written by Wim Couwenberg, see ...
;
; "https://wimcouwenberg.wordpress.com/2020/11/15/ ...
;  a-fast-24-bit-prng-algorithm-for-the-6502-processor/"
;
; Takes 68 cycles on the HuC6280, incl JSR & RTS.
;
; N.B. HuCC library code relies on this preserving X and Y!
;

_rand8:		cly				; Entry point for HuCC.

get_random:	lda	<random + 0		; Operation 7 (with carry clear).
		asl	a
		eor	<random + 1
		sta	<random + 1
		rol	a             		; Operation 9.
		eor	<random + 2
		sta	<random + 2
		eor	<random + 0		; Operation 5.
		sta	<random + 0
		lda	<random + 1		; Operation 15.
		ror	a
		eor	<random + 2
		sta	<random + 2
		eor	<random + 1		; Operation 6.
		sta	<random + 1
		rts



	.if	0

; ***************************************************************************
; ***************************************************************************
;
; get_random - 8-bit LFSR pseudo-random number with a 32-bit cycle.
;
; The pseudo-random sequence repeats after (2^32)-1 calls.
;
; Written by Brad Smith, see https://github.com/bbbradsmith/prng_6502
;
; Takes 100 cycles on the HuC6280, incl JSR & RTS.
;

get_random32:	; Rotate the middle bytes left

		ldy	<random + 2		; will move to random+3 at the end
		lda	<random + 1
		sta	<random + 2

		; Compute random+1 ($C5>>1 = %01100010)

		lda	<random + 3		; original high byte
		lsr	a
		sta	<random + 1		; reverse: %100011
		lsr	a
		lsr	a
		lsr	a
		lsr	a
		eor	<random + 1
		lsr	a
		eor	<random + 1
		eor	<random + 0		; combine with original low byte
		sta	<random + 1

		; Compute random+0 ($C5 = %11000101)

		lda	<random + 3		; original high byte
		asl	a
		eor	<random + 3
		asl	a
		asl	a
		asl	a
		asl	a
		eor	<random + 3
		asl	a
		asl	a
		eor	<random + 3
		sty	<random + 3		; finish rotating byte 2 into 3
		sta	<random + 0
		rts



; ***************************************************************************
; ***************************************************************************
;
; get_random - 8-bit LFSR pseudo-random number with a 24-bit cycle.
;
; The pseudo-random sequence repeats after (2^24)-1 calls.
;
; Written by Brad Smith, see https://github.com/bbbradsmith/prng_6502
;
; Takes 88 cycles on the HuC6280, incl JSR & RTS.
;

get_random24:	; Rotate the middle byte left

		ldy	<random + 1		; will move to random+2 at the end

		; Compute random+1 ($1B>>1 = %00001101)

		lda	<random + 2
		lsr	a
		lsr	a
		lsr	a
		lsr	a
		sta	<random + 1		; reverse: %1011
		lsr	a
		lsr	a
		eor	<random + 1
		lsr	a
		eor	<random + 1
		eor	<random + 0
		sta	<random + 1

		; Compute random+0 ($1B = %00011011)

		lda	<random + 2
		asl	a
		eor	<random + 2
		asl	a
		asl	a
		eor	<random + 2
		asl	a
		eor	<random + 2
		sty	<random + 2		; finish rotating byte 1 into 2
		sta	<random + 0
		rts



; ***************************************************************************
; ***************************************************************************
;
; get_random16 - 8-bit xorshift pseudo-random number with a 16-bit cycle.
;
; The pseudo-random sequence repeats after (2^16)-1 calls.
;
; Too short for general use, but good for "random" clearing a 256x256 screen.
;
; John Metcalf's version of George Marsaglia's 16-bit xorshift algorithm.
;
; http://www.retroprogramming.com/2017/07/xorshift-pseudorandom-numbers-in-z80.html
;
; Takes 52 cycles on the HuC6280, incl JSR & RTS.
;

get_random16:	lda	<random + 1
		lsr	a
		lda	<random + 0
		ror	a
		eor	<random + 1
		sta	<random + 1
		ror	a
		eor	<random + 0
		sta	<random + 0
		eor	<random + 1
		sta	<random + 1
		rts



; ***************************************************************************
; ***************************************************************************
;
; get_random - 8-bit LFSR pseudo-random number with a 24-bit cycle.
;
; The pseudo-random sequence repeats after (2^24)-1 calls.
;
; Written by Wim Couwenberg, see ...
;
; "https://wimcouwenberg.wordpress.com/2020/11/15/ ...
;  a-fast-24-bit-prng-algorithm-for-the-6502-processor/"
;
; Takes 68 cycles on the HuC6280, incl JSR & RTS.
;

get_random24:	lda	<random + 0		; Operation 7 (with carry clear).
		asl	a
		eor	<random + 1
		sta	<random + 1
		rol	a             		; Operation 9.
		eor	<random + 2
		sta	<random + 2
		eor	<random + 0		; Operation 5.
		sta	<random + 0
		lda	<random + 1		; Operation 15.
		ror	a
		eor	<random + 2
		sta	<random + 2
		eor	<random + 1		; Operation 6.
		sta	<random + 1
		rts

	.endif
