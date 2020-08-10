;
; MATH.ASM  -  MagicKit Standard Math Routines
;
;


; ----
; divu8
; ----
; 8-bit unsigned division
; ----
; OUT : _CL = _AL / _BL
;	_DL = _AL % _BL
; ----

divu8:
	lda	<__al
	asl a
	sta	<__cl
	cla
	ldy	#8
.l1:
	rol a
	cmp	<__bl
	bcc	.l2
	sbc	<__bl
.l2:
	rol	<__cl
	dey
	bne	.l1

	sta	<__dl
	rts


; ----
; divu10
; ----
; 16-bit unsigned division by 10
; ----
; OUT : _DX = _DX / 10
;	A = _DX % 10
; ----

divu10:
	ldy	#16
	cla
	asl	<__dl
	rol	<__dh
.l1:	rol	a
	cmp	#10
	blo	.l2
	sbc	#10
.l2:	rol	<__dl
	rol	<__dh
	dey
	bne	.l1
	rts


.if (!CDROM)

; ----
; mulu8
; ----
; 8-bit unsigned multiplication
; ----
; OUT : _CX = _AL * _BL
; ----

mulu8:
	lda	<__bl
	sta	<__ch

	cla
	ldy	#8
.l1:
	asl a
	rol	<__ch
	bcc	.next
	add	<__al
	bcc	.next
	inc	<__ch
.next:
	dey
	bne	.l1

	sta	<__cl
	rts


; ----
; mulu16
; ----
; 16-bit unsigned multiplication
; ----
; OUT : _DX/CX = _AX * _BX
; ----

mulu16:
	lda	<__ah
	ora	<__bh
	bne	.l1

	stwz	<__dx		; 8-bit multiplication
	jmp	mulu8

.l1:	stw	<__bx,<__dx	; 16-bit multiplication
	stwz	<__cx
	ldy	#16

.l2:	aslw	<__cx
	rolw	<__dx
	bcc	.l3

	addw	<__ax,<__cx
	bcc	.l3
	incw	<__dx

.l3:	dey
	bne	.l2
	rts

.endif ; (!CDROM)


; ----
; mulu32
; ----
; 32-bit unsigned multiplication
; ----
; OUT : _DX/CX = _BX/AX * _DX/CX
; ----

mulu32:
	stw	<__cx,<__si
	stw	<__dx,<__di
	stwz	<__cx
	stwz	<__dx
	ldy	#32
.loop:
	aslw	<__cx
	rolw	<__dx
	rolw	<__si
	rolw	<__di
	bcc	.next

	addw	<__ax,<__cx
	adcw	<__bx,<__dx
.next:
	dey
	bne	.loop
	rts


; ----
; srand
; ----
; set random seed
; ----
; IN : _DX/CX = 32-bit seed
; ----

	.bss
rndptr		.ds 2
rndseed2	.ds 2
rndn1		.ds 1
rndn2		.ds 1

	.code
srand:
	stw	<__cx,rndptr
	stw	<__dx,rndn1
	lda	rndptr+1
	ora	#$e0
	sta	rndptr+1
	cmp	#$f4
	blo	.exit
	lda	#$e0
	sta	rndptr+1
.exit:
	rts


; ----
; rand
; ----
; return 16-bit random number
; ----
; OUT: _DX
; ----

	.zp
rndzp	.ds	2

	.code
rand:	jsr	randomize
	stw	rndn1,<__dx
	rts

randomize:
	stw	rndptr,<rndzp

	lda	rndn1	; rotate 3 bits right
	ldx	rndn2
	ror a
	sax
	ror a
	sax
	ror a
	sax
	ror a
	sax
	ror a
	sax
	ror a
	stx	rndn1
	sta	rndn2

	addw	#$05A2,rndn1 ; add #$05A2 to number

	incw	<rndzp	; eor with next 2 bytes of ROM
	lda	rndn2
	eor	[rndzp]
	and	#$7f
	sta	rndn2

	incw	<rndzp
	lda	rndn1
	eor	[rndzp]
	sta	rndn1

	incw	<rndzp		; don't use every consecutive byte

	lda	<rndzp+1	; reset pointer to $e000 if > $f400
	cmp	#$f4
	blo	.l1
	lda	#$e0
	sta	<rndzp+1
.l1:
	stw	<rndzp,rndptr
	rts


; ----
; random
; ----
; return a random number in the interval 0 <= x < A
; ----
; IN :	A = range (1 - 128)
; ----
; OUT : A = random number
; ----
;

random:
	pha
	jsr	rand
	pla
	; ----
	cmp	#128
	blo	.l1

	lda	<__dh
	and	#$7f
	rts

.l1:	; asl a
	sta	<__al
	lda	<__dl
	sta	<__bl
	jsr	mulu8

	lda	<__ch
	rts
