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
; void __fastcall __xsafe strcpy( char *destination<_di>, char *source<_bp> );
; void __fastcall __xsafe strcat( char *destination<_di>, char *source<_bp> );
;
; unsigned int __fastcall __xsafe strlcpy( char *destination<_di>, char *source<_bp>, unsigned char size<acc> );
; unsigned int __fastcall __xsafe strlcat( char *destination<_di>, char *source<_bp>, unsigned char size<acc> );
; unsigned int __fastcall __xsafe strlen( char *source<_bp> );
;
; NOT WORKING YET (needs compiler changes) ...
;
; void __fastcall __xsafe strcpy( char *destination<_di>, char __far *source<_bp_bank:_bp> );
; void __fastcall __xsafe strcat( char *destination<_di>, char __far *source<_bp_bank:_bp> );
;
; unsigned int __fastcall __xsafe strlcpy( char *destination<_di>, char __far *source<_bp_bank:_bp>, unsigned char size<acc> );
; unsigned int __fastcall __xsafe strlcat( char *destination<_di>, char __far *source<_bp_bank:_bp>, unsigned char size<acc> );
; unsigned int __fastcall __xsafe strlen( char __far *source<_bp_bank:_bp> );

_strcat:	cla				; Max string length == 256!
		ldy.h	#256

_strlcat:	phx				; Preserve X (aka __sp).
		tax				; X = buffer length (1..256).

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
		ldy	#0			; Map the source memory.
		beq	.no_bank
		jsr	set_bp_to_mpr34

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

_strlcpy:	phx				; Preserve X (aka __sp).
		tax				; X = buffer length (1..256).

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
		ldy	#0			; Map the source memory.
		beq	.no_bank
		jsr	set_bp_to_mpr34

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

_strlen:	phx				; Preserve X (aka __sp).

		tma3
		pha
		tma4
		pha

;		ldy	<_bp_bank
		ldy	#0			; Map the source memory.
		beq	.no_bank
		jsr	set_bp_to_mpr34

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

		plx				; Restore X (aka __sp).
		rts

		.alias	_strlen.1		= _strlen
		.alias	_strcpy.2		= _strcpy
		.alias	_strcat.2		= _strcat
		.alias	_strlcpy.3		= _strlcpy
		.alias	_strlcat.3		= _strlcat



; ***************************************************************************
; ***************************************************************************
;
; void __fastcall __xsafe memcpy( unsigned char *destination<ram_tii_dst>, unsigned char  *source<ram_tii_src>, unsigned int count<acc> );
; unsigned char * __fastcall __xsafe mempcpy( unsigned char *destination<ram_tii_dst>, unsigned char  *source<ram_tii_src>, unsigned int count<acc> );
;
; NOT WORKING YET (needs compiler changes) ...
; void __fastcall __xsafe memcpy( unsigned char *destination<ram_tii_dst>, unsigned char __far *source<_bp_bank:ram_tii_src>, unsigned int count<acc> );
; unsigned char * __fastcall __xsafe mempcpy( unsigned char *destination<ram_tii_dst>, unsigned char __far *source<_bp_bank:ram_tii_src>, unsigned int count<acc> );

_memcpy:
_mempcpy:	sty.h	ram_tii_len		; Check for zero length.
		sta.l	ram_tii_len
		ora.h	ram_tii_len
		beq	.zero_length

		tma3				; Preserve MPR3 and MPR4.
		pha
		tma4
		pha

;		ldy	<_bp_bank		; Map the source memory.
		ldy	#0			; Map the source memory.
		beq	.no_bank

		lda.h	ram_tii_src		
		cmp	#$60
		bcc	.no_bank
		and	#$1F			; Remap ptr to MPR3.
		ora	#$60
		sta.h	ram_tii_src

		tya				; Put bank into MPR3.
		tam3
		inc	a			; Put next into MPR4.
		tam4

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

		.alias	_memcpy.3		= _memcpy
		.alias	_mempcpy.3		= _mempcpy



; ***************************************************************************
; ***************************************************************************
;
; void __fastcall __xsafe memset( unsigned char *destination<ram_tii_src>, unsigned char value<_al>, unsigned int count<acc> );

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
; int __fastcall strcmp( char *destination<_di>, char *source<_bp> );
; int __fastcall strncmp( char *destination<_di>, char *source<_bp>, unsigned int count<_ax> );
; int __fastcall memcmp( unsigned char *destination<_di>, unsigned char *source<_bp>, unsigned int count<_ax> );
;
; NOT WORKING YET (needs compiler changes) ...
;
; int __fastcall strcmp( char *destination<_di>, char __far *source<_bp_bank:_bp> );
; int __fastcall strncmp( char *destination<_di>, char __far *source<_bp_bank:_bp>, unsigned char count<_al> );
; int __fastcall memcmp( unsigned char *destination<_di>, unsigned char __far *source<_bp_bank:_bp>, unsigned int count<_ax> );
;
;  0 	if strings are equal
;  1 	if the first non-matching character in string1 > string2 (in ASCII).
; -1 	if the first non-matching character in string1 < string2 (in ASCII).

hucc_memcmp	.procgroup

_strcmp		.proc
		stz.l	<_ax			; Max string length == 256!
		lda.h	#256
		sta.h	<_ax
		.ref	_strncmp		; Don't strip _strncmp.3!
		.endp				; Fall through.

_strncmp	.proc
		bit	#$40			; Set the V bit for strcmp.
		db	$50			; Turn "clv" into "bvc".
		.ref	_memcmp			; Don't strip _memcmp.3!
		.endp				; Fall through.

_memcmp		.proc
		clv				; Clr the V bit for memcmp.

		tma3				; Preserve MPR3 and MPR4.
		pha
		tma4
		pha

;		ldy	<_bp_bank		; Map the source string.
		ldy	#0			; Map the source memory.
		beq	.no_bank
		jsr	set_bp_to_mpr34

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
		jsr	inc.h_bp_mpr34
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

		.alias	_strcmp.2		= _strcmp
		.alias	_strncmp.3		= _strncmp
		.alias	_memcmp.3		= _memcmp
