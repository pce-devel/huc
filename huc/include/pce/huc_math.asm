;
; HUC_MATH.ASM  -  HuC Math Library
;

; abs(int val)
; ---

_abs:
	tay
	bpl	.l1
	sax
	eor	#$FF
	add	#1
	sax
	eor	#$FF
	adc	#0
.l1:
	rts

; mov32(void *dst [__di], void *src)
; ----

_mov32.2:
	__stw	<__si
_mov32.sub:
	ldy	#3
.l1:	lda	[__si],Y
	sta	[__di],Y
	dey
	bpl	.l1
	rts

; add32(void *dst [__di], void *src) /* ax|bx */
; ----

_add32.2:
	__stw	<__si
	clc
	cly
	ldx	#4
.l1:	lda	[__di],Y
	adc	[__si],Y
	sta	[__di],Y
	iny
	dex
	bne	.l1
	rts

; sub32(void *dst [__di], void *src)
; ----

_sub32.2:
	__stw	<__si
	sec
	cly
	ldx	#4
.l1:	lda	[__di],Y
	sbc	[__si],Y
	sta	[__di],Y
	iny
	dex
	bne	.l1
	rts

; mul32(void *dst [__bp], void *src)
; ----

_mul32.2:
	__stw	<__si
	stw	#__ax,<__di
	jsr	_mov32.sub
	stw	<__bp,<__si
	stw	#__cx,<__di
	jsr	_mov32.sub
	jsr	mulu32
	stw	<__bp,<__di
	stw	#__cx,<__si
	jmp	_mov32.sub

; div32(void *dst [__di], void *src)
; ----

_div32.2:
	rts

; com32(void *dst)
; ----

_com32.1:
	__stw	<__di
	ldy	#3
.l1:	lda	[__di],Y
	eor	#$FF
	sta	[__di],Y
	dey
	bpl	.l1
	rts
	
; cmp32(void *dst [__di], void *src)
; ----

_cmp32.2:
	__stw	<__si
	ldy	#3
.l1:	lda	[__di],Y
	cmp	[__si],Y
	bne	.l2
	dey
	bpl	.l1
	; --
	clx
	cla
	rts
	; --
.l2:	blo	.l3
	ldx	#$01
	cla
	rts
	; --
.l3:	ldx	#$FF
	txa
	rts


.ifdef BCD

; bcd_init(char *dst [__bx], char digits)
; ----

_bcd_init.2:
	; -- check digit number (max. 16)
	txa
	cmp	#16
	blo	.l1
	lda	#16
.l1:	inc	A
	lsr	A
	ora	#$80
	sta	[__bx]
_bcd_init.clear:
	; -- clear bcd number
	lda	[__bx]
	and	#$1F
	tay
	cla
.l2:	sta	[__bx],Y
	dey
	bne	.l2
	rts

; bcd_set(char *dst [__bx], char *src)
; bcd_mov(char *dst [__bx], char *src)
; ----

_bcd_set.2:
_bcd_mov.2:
	__stw	<__si
	ora	<__si
	beq	_bcd_init.clear
	; -- check dst
	lda	[__bx]
	bpl	.x1
	and	#$1F
	beq	.x1
	tax
	; -- check src type
	lda	[__si]
	bpl	_bcd_set.ascii
	bra	_bcd_set.bcd
.x1:	rts
	; ----
	; ... from an ascii string (ie. "100")
	;
_bcd_set.ascii:
	; -- get string length
	cly
.l1:	lda	[__si],Y
	cmp	#48
	blo	.l2
	cmp	#58
	bhs	.l2
	iny
	bra	.l1
	; -- check if the string is empty
.l2:	tya
	beq	_bcd_init.clear
	; -- copy number
.l3:	cla
	dey
	bmi	.l4
	lda	[__si],Y
	sub	#48
	sta	<__dl
	dey
	bmi	.l4
	lda	[__si],Y
	sub	#48
	asl	A
	asl	A
	asl	A
	asl	A
	ora	<__dl
.l4:	sxy
	sta	[__bx],Y
	sxy
	dex
	bne	.l3
	rts

	; ----
	; ... from another bcd number
	;
_bcd_set.bcd:
	; -- get src size
	lda	[__si]
	bpl	.x1
	and	#$1F
	beq	.x1
	tay
	; -- copy number
.l1:	lda	[__si],Y
	sxy
	sta	[__bx],Y
	dey
	beq	.x1
	sxy
	dey
	bne	.l1
	; -- adjust number
	sxy
	cla
.l2:	sta	[__bx],Y
	dey
	bne	.l2
.x1:	rts

; bcd_add(char *dst [__di], char *src)
; ----

_bcd_add.2:
	__stw	<__si
	ora	<__si
	beq	.x1
	; -- check dst
	lda	[__di]
	bpl	.x1
	and	#$1F
	beq	.x1
	tax
	stx	<__cl
	; -- check src
	lda	[__si]
	bmi	.l1
	; -- convert ascii string
	stw	#__temp,<__bx
	jsr	_bcd_set.ascii
	stw	#__temp,<__si
	ldx	<__cl
	ldy	<__cl
	bra	.l2
	; -- get src size
.l1:	and	#$1F
	beq	.x1
	tay
	; -- add numbers
	clc
	sed
.l2:	lda	[__di],Y
	sxy
	adc	[__di],Y
	sta	[__di],Y
	dex
	beq	.l4
	sxy
	dex
	bne	.l2
	; --
.x1:	cld
	rts
	; -- carry
.l3:	lda	[__di],Y
	adc	#0
	sta	[__di],Y
.l4:	bcc	.x1
	dey
	bne	.l3
	cld
	rts

.endif ; BCD
