; ***************************************************************************
; ***************************************************************************
;
; hucc-string.asm
;
; Not-quite-standard, but fast, replacements for <string.h>.
;
; Copyright John Brandwood 2024.
;
; Distributed under the Boost Software License, Version 1.0.
; (See accompanying file LICENSE_1_0.txt or copy at
;  http://www.boost.org/LICENSE_1_0.txt)
;
; ***************************************************************************
; ***************************************************************************
;
; !!! WARNING : non-standard return values !!!
;
; Strings are limited to a maximum of 255 characters (+ the terminator)!
;
; The memcpy(), strcpy() and strcat() functions do NOT return the destination
; address, and they are declared "void" to check that the value is not used.
;
; mempcpy() is provided which returns the end address instead of the starting
; address, because this is typically more useful.
;
; Please note that both memcpy() and memset() are implemented using a TII for
; speed, and so the length should be < 16 bytes if used in time-critical bits
; of code (such as when using a split screen) because they delay interrupts.
;
; strncpy() and strncat() are not provided, because strncpy() was not created
; for the purpose of avoiding string overruns, and strncat() is just a poorly
; designed function.
;
; POSIX strlcpy() and strlcat() are provided instead, but once again they are
; slightly non-standard in that the return value when there is an overflow is
; the buffer size (so that the overflow can be detected), instead of the full
; size of the destination string that was too big to fit in the buffer.
;
; ***************************************************************************
; ***************************************************************************



; ***************************************************************************
; ***************************************************************************
;
; void __fastcall strcpy( char *destination<_di>, char *source<_bp> );
; void __fastcall strcat( char *destination<_di>, char *source<_bp> );
;
; unsigned int __fastcall strlcpy( char *destination<_di>, char *source<_bp>, unsigned char size<acc> );
; unsigned int __fastcall strlcat( char *destination<_di>, char *source<_bp>, unsigned char size<acc> );
; unsigned int __fastcall strlen( char *source<_bp> );
;
; NOT WORKING YET (needs compiler changes) ...
;
; void __fastcall strcpy( char *destination<_di>, char __far *source<_bp_bank:_bp> );
; void __fastcall strcat( char *destination<_di>, char __far *source<_bp_bank:_bp> );
;
; unsigned int __fastcall strlcpy( char *destination<_di>, char __far *source<_bp_bank:_bp>, unsigned char size<acc> );
; unsigned int __fastcall strlcat( char *destination<_di>, char __far *source<_bp_bank:_bp>, unsigned char size<acc> );
; unsigned int __fastcall strlen( char __far *source<_bp_bank:_bp> );

_strcat:	cla				; Max string length == 256!
		ldy.h	#256

_strlcat:	tax				; X = buffer length (1..256).

	.ifdef	_DEBUG
		bne	!+			; Sanity check buffer length. 
		dey
!:		tya
.hang:		bne	.hang			; Hang if 1 > buffer length > 256.
	.endif

		tma3				; Preserve MPR3 and MPR4.
		pha
		tma4
		pha

;		ldy	<_bp_bank		; Map the source string.
;		beq	.no_bank
;		jsr	map_bp_to_mpr34

.no_bank:	cly

.find:		lda	[_di], y		; Find the end of the string.
		beq	.adjust
		iny
		dex
		bne	.find
		tya				; A:Y = buffer length.
		cla
		bra	str_overflow

.adjust:	tya				; Subtract Y from _bp so that
		eor	#$FF			; _bp and _di use the same Y.
		sec
		adc.l	<_bp
		sta.l	<_bp
		bcs	str_copy
		dec.h	<_bp
		bra	str_copy

		;

_strcpy:	cla				; Max string length == 256!
		ldy.h	#256

_strlcpy:	tax				; X = buffer length (1..256).

	.ifdef	_DEBUG
		bne	!+			; Sanity check buffer length.
		dey
!:		tya
.hang:		bne	.hang			; Hang if 1 > buffer length > 256.
	.endif

		tma3				; Preserve MPR3 and MPR4.
		pha
		tma4
		pha

;		ldy	<_bp_bank		; Map the source string.
;		beq	.no_bank
;		jsr	map_bp_to_mpr34

.no_bank:	cly

str_copy:	lda	[_bp], y
		sta	[_di], y
		beq	str_exit		; A:Y = string length.
		iny
		dex
		bne	str_copy

		dey
		cla
		sta	[_di], y
		iny				; A:Y = buffer length.
str_overflow:	bne	str_exit
		inc	a			; A:Y = buffer length = 256.
		bra	str_exit

		;

_strlen:	tma3
		pha
		tma4
		pha

;		ldy	<_bp_bank
;		beq	.no_bank
;		jsr	map_bp_to_mpr34

.no_bank:	cly

.find:		lda	[_bp], y
		beq	str_exit
		iny
		bne	.find
		lda.h	#256			; A:Y = overflow length = 256.

str_exit:	tax				; X:Y = string or buffer length.

		pla				; Restore MPR3 and MPR4.
		tam4
		pla
		tam3

		txa				; A:Y = string or buffer length.
		say				; Y:A = string or buffer length.

		rts

		.alias	_strlen.1		= _strlen
		.alias	_strcpy.2		= _strcpy
		.alias	_strcat.2		= _strcat
		.alias	_strlcpy.3		= _strlcpy
		.alias	_strlcat.3		= _strlcat



; ***************************************************************************
; ***************************************************************************
;
; void __fastcall memcpy( unsigned char *destination<ram_tii_dst>, unsigned char  *source<ram_tii_src>, unsigned int count<acc> );
; unsigned char * __fastcall mempcpy( unsigned char *destination<ram_tii_dst>, unsigned char  *source<ram_tii_src>, unsigned int count<acc> );
;
; void __fastcall farmemcpy( unsigned char *destination<ram_tii_dst>, unsigned char __far *source<_bp_bank:ram_tii_src>, unsigned int count<acc> );
; unsigned char * __fastcall farmempcpy( unsigned char *destination<ram_tii_dst>, unsigned char __far *source<_bp_bank:ram_tii_src>, unsigned int count<acc> );
;
; void __fastcall far_memcpy( unsigned char *destination<ram_tii_dst>, unsigned int count<acc> );
; unsigned char * __fastcall far_mempcpy( unsigned char *destination<ram_tii_dst>, unsigned int count<acc> );
;

_memcpy.3:
_mempcpy.3:	stz	<_bp_bank		; Map the source memory.

_farmemcpy.3:
_farmempcpy.3:	sty.h	ram_tii_len		; Check for zero length.
		sta.l	ram_tii_len
		ora.h	ram_tii_len
		beq	.zero_length

		tma3				; Preserve MPR3 and MPR4.
		pha
		tma4
		pha

		lda	<_bp_bank		; Map the source memory.
		beq	.no_bank

		tam3				; Put bank into MPR3.
		inc	a
		tam4				; Put next into MPR4.

;		lda.h	ram_tii_src		; Remap ptr to MPR3.
;		and	#$1F
;		ora	#$60
;		sta.h	ram_tii_src

.no_bank:	jsr	ram_tii			; Copy the memory.

		pla				; Restore MPR3 and MPR4.
		tam4
		pla
		tam3

.zero_length:	clc				; Return the end address
		lda.l	ram_tii_dst		; like mempcpy().
		adc.l	ram_tii_len
		tay
		lda.h	ram_tii_dst
		adc.h	ram_tii_len
		say

		rts

		.alias	_far_memcpy.2		= _farmemcpy.3
		.alias	_far_mempcpy.2		= _farmempcpy.3



; ***************************************************************************
; ***************************************************************************
;
; void __fastcall memset( unsigned char *destination<ram_tii_src>, unsigned char value<_al>, unsigned int count<acc> );

_memset:	cmp	#0			; Decrement the length, check
		bne	!+			; for zero and set C. 
		cpy	#0
		beq	.zero_length
		dey
!:		dec	a
		sta.l	ram_tii_len
		sty.h	ram_tii_len

		lda.l	ram_tii_src		; ram_tii_dst = ram_tii_src + 1
		sta.l	<__ptr
		adc	#0
		sta.l	ram_tii_dst
		lda.h	ram_tii_src
		sta.h	<__ptr
		adc	#0
		sta.h	ram_tii_dst

		lda	<_al			; Set the fill value.
		sta	[__ptr]

		jmp	ram_tii			; Copy the memory.

.zero_length:	rts

		.alias	_memset.3		= _memset



; ***************************************************************************
; ***************************************************************************
;
; int __fastcall strcmp( char *string1<_di>, char *string2<_bp> );
; int __fastcall strncmp( char *string1<_di>, char *string2<_bp>, unsigned int count<_ax> );
;
; int __fastcall __macro memcmp( unsigned char *string1<_di>, unsigned char *string2<_bp>, unsigned int count<_ax> );
; int __fastcall farmemcmp( unsigned char *string1<_di>, unsigned char __far *string2<_bp_bank:_bp>, unsigned int count<_ax> );
; int __fastcall far_memcmp( unsigned char *string1<_di>, unsigned int count<_ax> );
;
;  0 	if strings are equal
;  1 	if the first non-matching character in string1 > string2 (in ASCII).
; -1 	if the first non-matching character in string1 < string2 (in ASCII).

hucc_memcmp	.procgroup

_strcmp.2	.proc
		stz.l	<_ax			; Max string length == 256!
		lda.h	#256
		sta.h	<_ax
		.ref	_strncmp.3		; Don't strip _strncmp.3!
		.endp				; Fall through.

_strncmp.3	.proc
		stz	<_bp_bank		; Assume strings are mapped.
		bit	#$40			; Set the V bit for strcmp.
		db	$50			; Turn "clv" into "bvc".
		.ref	_farmemcmp.3		; Don't strip _farmemcmp.3!
		.endp				; Fall through.

_farmemcmp.3	.proc
		clv				; Clr the V bit for memcmp.

		tma3				; Preserve MPR3 and MPR4.
		pha
		tma4
		pha

		ldy	<_bp_bank		; Map string2.
		beq	.no_bank

		jsr	map_bp_to_mpr34

.no_bank:	cly

		ldx.l	<_ax			; Increment length.l
		inx
.loop:		dex				; Decrement length.l
		beq	.page
.test:		lda	[_di], y		; string1 - string2
		cmp	[_bp], y
		bcc	.return_neg		; string1 < string2 
		bne	.return_pos		; string1 > string2
		bvc	!+			; Only check for end-of-string
		cmp	#0			; if the V flag is set.
		beq	.return_same
!:		iny
		bne	.loop
		inc.h	<_di
		inc.h	<_bp			; Limited to 8KB maximum!
;		jsr	inc.h_bp_mpr34
		bra	.loop

.page:		dec.h	<_ax			; Decrement length.h 
		bpl	.test			; Limit comparison to 32KB.
;		bra	cmp_same

.return_same:	clx				; Return code in Y:X, X -> A.
		cly
		bra	!+

.return_pos:	ldx	#$01			; Return code in Y:X, X -> A.
		cly
		bra	!+

.return_neg:	ldx	#$FF			; Return code in Y:X, X -> A.
		ldy	#$FF

!:		pla				; Restore MPR3 and MPR4.
		tam4
		pla
		tam3

		leave				; Return and copy X -> A.

		.endp

		.endprocgroup			; hucc_memcmp

		.alias	_far_memcmp.2		= _farmemcmp.3
