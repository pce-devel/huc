;
; HUC_BRAM.ASM  -  HuC Backup RAM Library
;

	.bss
bm_mpr4:	.ds 1
bm_error:	.ds 1
	.code

; NOTE: the BRAM format is as follows:
;
; BRAM bank header (0x10 bytes):
; ------------------------------
; - Header tag (4 bytes) = 'HUBM'
;
; - Pointer to 1st byte after BRAM (2 byte int)
;   Note: relative to 0x8000 segment, hex example: 00 88 ($8800)
;
; - Pointer to next available BRAM slot (2 byte int)
;   Note: relative to 0x8000 segment, hex example: 22 80 ($8022)
;
; - Zeroes (8 bytes)
;
; BRAM Entry Header (0x10 bytes):
; -------------------------------
; - Size of entry (2 bytes), hex example: 12 00 ($0012)
;   This size includes the 0x10 bytes used by this header information
;
; - Checksum (2 bytes)
;   This number added to the data should total to 0
;
; - Name (12 bytes)
;   This subdivides into:
;   - Unique ID (2 bytes); I have only ever seen 00 00 in this entry
;     However, it appears to be an integral part of the name
;   - ASCII Name (10 bytes)
;     This should be padded with spaces
;
; BRAM Entry:
; -----------
; - Miscellaneous data, size described by header parameters
;
; BRAM Entry Trailer (0x02 bytes):
; --------------------------------
; - Zeores (2 bytes)
;   Technically, this trailer is "not used", but rather a terminator
;   to the linked list.  The BRAM bank header points at it by the
;   'next available slot' pointer
;


; bm_check()
; ---
; Determine whether BRAM exists on this system
; without damaging contents on BRAM
;

_bm_check:
	jsr	_bm_unlock

	stw	#$8000,<__di	; test area at $8000
	jsr	_bm_testram

	; -- result
	jsr	_bm_disable
	cpx	#0
	bne	.err
	; -- ok
	stz	bm_error
	ldx	#1
	cla
	clc
	rts
	; -- no bram
.err:	clx
	cla
	sec
	rts


; bm_testram
; ---
; internal function to test whether BRAM exists at a location
; input  = di (pointer to memory area to test)
; output = register x (# errors)
;

_bm_testram:
	; -- swap bits
	ldy	#7
.l1:	lda	[__di],Y
	eor	#$FF
	sta	__ax,Y
	sta	[__di],Y
	dey
	bpl	.l1
	; -- cmp
	clx
	ldy	#7
.l2:	lda	__ax,Y
	cmp	[__di],Y
	beq	.l3
	inx
.l3:	eor	#$FF
	sta	[__di],Y
	dey
	bpl	.l2

	rts


; bm_format()
; ---
; If BRAM is already formatted (*_PROPERLY_*), return OK
; Otherwise, set header info, and set limit of BRAM to
; maximum amount of memory available on this hardware
;

_bm_format:
	jsr	_bm_enable
	bcc	.ok
	; -- format
	ldx	#7
.l1:	lda	_bm_id,X
	sta	$8000,X
	dex
	bpl	.l1
	stz	$8010
	stz	$8011

	stw	#$8000,<__di	; test area at $8000
.l2:	jsr	_bm_testram
	cpx	#0
	bne	.setsz
	lda	<__di+1
	cmp	#$A0		; and keep going until either
	beq	.setsz		; (a) bad memory, or
	inc	A		; (b) next bank
	sta	<__di+1
	bra	.l2

.setsz:	lda	<__di+1
	sta	$8005

	; -- ok
.ok:	jsr	_bm_disable
	stz	bm_error
	clx
	cla
	clc
	rts


; bm_free()
; ---
; Returns (int) number of user bytes available in BRAM
; BRAM header entry and trailer overhead is already deducted
;

_bm_free:
	jsr	_bm_enable
	bcs	.err
	; -- calculate free space
	stw	$8004,<__cx
	subw	$8006,<__cx
	subw	#$12,<__cx
	lda	<__ch
	bpl	.ok
	stwz	<__cx
	; -- ok
.ok:	jsr	_bm_disable
	stz	bm_error
	__ldw	<__cx
	clc
	rts
	; -- error, bram not formatted
.err:	sta	bm_error
	jsr	_bm_disable
	lda	#$FF
	tax
	sec
	rts


; bm_size()
; ---
; Returns (int) number of bytes in BRAM - should normally
; be 2K, but can be as large as 8KB

_bm_size:
	jsr	_bm_enable
	bcs	.err
	; -- calculate free space
	stw	$8004,<__cx
	subw	#$8000,<__cx
	lda	<__ch
	cmp	#$21
	bcs	.err1
	lda	<__ch
	bpl	.ok
	stwz	<__cx
	; -- ok
.ok:	jsr	_bm_disable
	stz	bm_error
	__ldw	<__cx
	clc
	rts
	; -- error, bram not formatted
.err1:	lda	#$ff
.err:	sta	bm_error
	jsr	_bm_disable
	lda	#$FF
	tax
	sec
	rts


; bm_rawread(int location)
; ---
; Similar to peek(), but for BRAM
; Automatically handles mapping of memory ; and address range
;
_bm_rawread:
	__stw	<__bx
	lda	<__bh
	and	#$1F
	ora	#$80
	sta	<__bh
	jsr	_bm_enable
	bcs	.err
	lda	[__bx]
	sax
	stz	bm_error
	jsr	_bm_disable
	cla
	clc
	rts
.err:	sta	bm_error
	jsr	_bm_disable
	sec
	rts


; bm_rawwrite(int location [__bx], char val [reg acc])
; ---
; Similar to peek(), but for BRAM
; Automatically handles mapping of memory ; and address range
;
_bm_rawwrite.2:
	__stw	<__ax
	lda	<__bh
	and	#$1F
	ora	#$80
	sta	<__bh
	jsr	_bm_enable
	bcs	.err
	lda	<__al
	sta	[__bx]
	stz	bm_error
	jsr	_bm_disable
	cla
	clx
	clc
	rts
.err:	sta	bm_error
	jsr	_bm_disable
	sec
	rts


; bm_exist(char *name)
; ---
; Check for existence of BRAM file with a matching name
; Note: name is 12 bytes; first 2 bytes are a uniqueness value
;       (should be zeroes), and the next10 bytes are ASCII name
;       with trailing spaces as padding
;

_bm_exist:
	__stw	<__bx
	jsr	_bm_open
	bcs	.l1
	jsr	_bm_disable
	ldx	#1
	lda	bm_error
	beq	.noerr
.l1:	clx
	cla
.noerr:	rts


; bm_sizeof(char *name)
; ---
; Return the size of the user data in a RAM file with the given name
; Note: name is 12 bytes; first 2 bytes are a uniqueness value
;       (should be zeroes), and the next10 bytes are ASCII name
;       with trailing spaces as padding
;

_bm_sizeof:
	__stw	<__bx
	jsr	_bm_open
	bcs	.l1
	subw	#$10,<__cx
	jsr	_bm_disable
	stz	bm_error
	ldx	<__cl
	lda	<__ch
	rts
.l1:	clx
	cla
	rts


; bm_errno()
; ---
; Return error type
;

_bm_errno:
	ldx	bm_error
	cla
	rts


_bm_id:
	.db	$48,$55,$42,$4D
	.db	$00,$88,$10,$80


; ------------------------

; bm_getptr(int ptr [__bp], char *namebuf [reg acc])
; ---
; Given a pointer with the BRAM, obtain the name of the entry
; and the pointer to the next entry
; Use BRAM_START for first entry
;

_bm_getptr.2:
	maplibfunc	lib2_bm_getptr.2
	rts


; bm_delete(char *namebuf)
; ---
; Delete the entry specified by the name provided
;

_bm_delete:
	maplibfunc	lib2_bm_delete
	rts


; bm_read(char *buf [__di], char *name [__bx], int offset [__bp], int nb)
; ---
; Given a name of a file, grab some info from the file
;

_bm_read.4:
	maplibfunc	lib2_bm_read.4
	rts


; bm_write(char *buf [__di], char *name [__bx], int offset [__bp], int nb)
; ---
; Given the name of a BRAM file, update some info inside of it
;

_bm_write.4:
	maplibfunc	lib2_bm_write.4
	rts


; bm_create(char *name [__bx], int size)
; ---
; Create a new BRAM file, given the name and size

_bm_create.2:
	maplibfunc	lib2_bm_create.2
	rts


; bm_open(char *name [__bx])
; ---
; Internal function to obtain access to a named file

_bm_open:
	maplibfunc	lib2_bm_open
	rts


; bm_enable()
; ---
; Internal function to enable the BRAM area and do a quick check

_bm_enable:
	maplibfunc	lib2_bm_enable
	rts


; bm_disable()
; ---
; This internal function handles the fixup of BRAM segment/locking

_bm_disable:
	maplibfunc	lib2_bm_disable
	rts


; bm_unlock()
; ---
; This internal function handles only the map/unlock of the BRAM area

_bm_unlock:
	maplibfunc	lib2_bm_unlock
	rts


; ---------------

;
; From here, we have the implementation of various
; BRAM routines in LIB2_BANK rather than LIB1_BANK
; for reasons of speed and size
;
; Their external linkages will have specified above
; and be begin with an underscore.  The private
; definitions will not have an underscore
;

	.bank	LIB2_BANK

; bm_getptr(int ptr [__bp], char *namebuf [reg acc])
; ---
; Given a pointer with the BRAM, obtain the name of the entry
; and the pointer to the next entry
; Use BRAM_START for first entry
;

lib2_bm_getptr.2:
	__stw	<__di		; namebuf is destination of a copy
	jsr	lib2_bm_enable
	bcs	.x2

	tstw	<__bp		; error - 0 input
	beq	.x2
	lda	[__bp]
	sta	<__cl
	ldy	#1
	lda	[__bp],Y
	sta	<__ch		; <__cx is length of entry
	tstw	<__cx
	beq	.empty

	addw	#4,<__bp,<__si	; <__si is now ptr to name of current entry
	cla
	ldx	#12
	jsr	_memcpy.3	; copy 12 bytes of name to namebuf
	addw	<__cx,<__bp,<__ax	; next pointer
	jsr	lib2_bm_disable
	stz	bm_error
	lda	<__ah
	ldx	<__al
	clc
	rts

.empty:	cla

	; -- error, bram not formatted
.x2:	sta	bm_error
	jsr	lib2_bm_disable
	cla
	clx
	sec
	rts


; bm_delete(char *namebuf)
; ---
; Delete the entry specified by the name provided
;

lib2_bm_delete:
	__stw	<__ax
	jsr	lib2_bm_open
	bcs	.out
	stw	$8006,<__bx	; ptr to end
	stw	<__si,<__di	; setup currptr as dest
	stw	<__dx,<__si	; setup nextptr as src
	subw	<__dx,<__bx	; #bytes = end-next + 2
	addw	#2,<__bx
	subw	<__cx,$8006	; adjust ptr to end
	lda	<__bh
	ldx	<__bl
	jsr	_memcpy.3
	jsr	lib2_bm_disable
	stz	bm_error
	clx
	cla
	clc
.out:	rts


; bm_read(char *buf [__di], char *name [__bx], int offset [__bp], int nb)
; ---
; Given a name of a file, grab some info from the file
;

lib2_bm_read.4:
	__stw	<__ax
	; -- open file
	jsr	lib2_bm_open
	bcs	.x2
	; -- checksum test
	jsr	lib2_bm_checksum
	ldy	#2
	lda	[__si],Y
	add	<__dl
	sta	<__dl
	iny
	lda	[__si],Y
	adc	<__dh
	ora	<__dl
	bne	.x1
	; -- setup ptr
	jsr	lib2_bm_setup_ptr
	bcs	.ok
	; -- read
	cly
.l1:	lda	[__dx],Y
	sta	[__di],Y
	iny
	bne	.l2
	inc	<__dx+1
	inc	<__di+1
.l2:	dec	<__cl
	bne	.l1
	dec	<__ch
	bpl	.l1
	; -- ok
.ok:	jsr	lib2_bm_disable
	stz	bm_error
	__ldw	<__ax
	clc
	rts
	; -- error, bad file checksum
.x1:	lda	#2
	sta	bm_error
	jsr	lib2_bm_disable
	clx
	cla
	sec
.x2:	rts


; bm_write(char *buf [__di], char *name [__bx], int offset [__bp], int nb)
; ---
; Given the name of a BRAM file, update some info inside of it
;

lib2_bm_write.4:
	__stw	<__ax
	; -- open file
	jsr	lib2_bm_open
	bcs	.x1
	; -- setup ptr
	jsr	lib2_bm_setup_ptr
	bcs	.ok
	; -- write data
	cly
.l1:	lda	[__di],Y
	sta	[__dx],Y
	iny
	bne	.l2
	inc	<__dx+1
	inc	<__di+1
.l2:	dec	<__cl
	bne	.l1
	dec	<__ch
	bpl	.l1
	; -- update checksum
	jsr	lib2_bm_checksum
	ldy	#2
	cla
	sub	<__dl
	sta	[__si],Y
	iny
	cla
	sbc	<__dh
	sta	[__si],Y
	; -- ok
.ok:	jsr	lib2_bm_disable
	stz	bm_error
	__ldw	<__ax
	clc
.x1:	rts


; bm_create(char *name [__bx], int size)
; ---
; Create a new BRAM file, given the name and size

lib2_bm_create.2:
	__stw	<__ax
	jsr	lib2_bm_enable
	bcc	.go
	bra	.x2
	; -- error, not enough ram
.x1:	lda	#5
	; -- error, bram not formatted
.x2:	sta	bm_error
	jsr	lib2_bm_disable
	ldx	bm_error
	cla
	sec
	rts
	; -- check free space
.go:	addw	#$12,$8006,<__dx
	addw	<__ax,<__dx
	cmpw	<__dx,$8004
	blo	.x1
	; -- create file
	stw	$8006,<__si
	ldy	#1
	lda	<__al
	add	#$10
	sta	[__si]
	lda	<__ah
	adc	#$00
	sta	[__si],Y
	; --
	lda	$8006
	add	[__si]
	sta	<__dl
	sta	$8006
	lda	$8007
	adc	[__si],Y
	sta	<__dh
	sta	$8007
	cla
	sta	[__dx]
	sta	[__dx],Y
	; -- copy name
	clx
	ldy	#4
.l1:	sxy
	lda	[__bx],Y
	sxy
	sta	[__si],Y
	iny
	inx
	cpx	#12
	bne	.l1
	; -- clear file
	lda	<__al
	ora	<__ah
	beq	.sum
	stw	<__si,<__bx
	ldy	#16
	cla
.l2:	sta	[__bx],Y
	iny
	bne	.l3
	inc	<__bh
.l3:	dec	<__al
	bne	.l2
	dec	<__ah
	bpl	.l2
	; -- update checksum
.sum:	jsr	lib2_bm_checksum
	ldy	#2
	cla
	sub	<__dl
	sta	[__si],Y
	iny
	cla
	sbc	<__dh
	sta	[__si],Y
	; -- ok
.ok:	jsr	lib2_bm_disable
	stz	bm_error
	clx
	cla
	clc
	rts


; bm_open(char *name [__bx])
; ---
; Internal function to obtain access to a named file

lib2_bm_open:
	jsr	lib2_bm_enable
	bcs	.x2
	; -- get dir entry
	stw	#$8010,<__si
.l1:	lda	[__si]
	sta	<__cl
	ldy	#1
	lda	[__si],Y
	sta	<__ch
	ora	<__cl
	beq	.x1
	addw	<__cx,<__si,<__dx
	cmpw	<__dx,$8004
	blo	.x3
	cmpw	#16,<__cx
	blo	.x3
	; -- compare names
	cly
	ldx	#4
.l2:	lda	[__bx],Y
	sxy
	cmp	[__si],Y
	bne	.next
	sxy
	inx
	iny
	cpy	#12
	blo	.l2
	; -- check file size
	lda	<__ch
	bne	.ok
	lda	<__cl
	cmp	#16
	beq	.x4
	; -- ok
.ok:	stz	bm_error
	clc
	rts
	; -- next entry
.next:	addw	<__cx,<__si
	bra	.l1
	; -- error, file not found
.x1:	lda	#1
	; -- error, bram not formatted
.x2:	sta	bm_error
	jsr	lib2_bm_disable
	sec
	rts
	; -- error, directory corrupted
.x3:	lda	#3
	bra	.x2
	; -- error, empty file
.x4:	lda	#4
	bra	.x2


; bm_enable()
; ---
; Internal function to enable the BRAM area and do a quick check

lib2_bm_enable:
	jsr	lib2_bm_unlock
	; -- check if formatted
	ldx	#3
.l1:	lda	$8000,X
	cmp	_bm_id,X
	bne	.x1
	dex
	bpl	.l1
	; -- ok
	cla
	clc
	rts
	; -- error, not formatted!
.x1:	lda	#$FF
	sec
	rts


; bm_unlock()
; ---
; This internal function handles only the map/unlock of the BRAM area

lib2_bm_unlock:
	sei
	tma	#4
	sta	bm_mpr4
	lda	#$F7
	tam	#4
	csl
	lda	#$48
	sta	bram_unlock
	lda	#$75
	sta	bram_unlock
	lda	#$80
	sta	bram_unlock
	rts


; bm_disable()
; ---
; This internal function handles the fixup of BRAM segment/locking

lib2_bm_disable:
	lda	bm_mpr4
	tam	#4
	lda	bram_lock
	csh
	cli
	rts


; bm_checksum(char *fcb [__si])
; ---
; Internal function to generate checksum
;

lib2_bm_checksum:
	stwz	<__dx
	; -- get file size
	lda	[__si]
	sub	#4
	sta	<__cl
	ldy	#1
	lda	[__si],Y
	sbc	#0
	sta	<__ch
	stw	<__si,<__bx
	; -- calc checksum
	ldy	#4
.l1:	lda	[__bx],Y
	add	<__dl
	sta	<__dl
	bcc	.l2
	inc	<__dh
.l2:	iny
	bne	.l3
	inc	<__bh
.l3:	dec	<__cl
	bne	.l1
	dec	<__ch
	bpl	.l1
	rts


; bm_setup_ptr(char *fcb [__si], char *buf [__di], int offset [__bp], int nb [__ax])
; ---

lib2_bm_setup_ptr:
	; -- check length
	tstw	<__ax
	beq	.x1
	; -- check ptr
	tstw	<__di
	beq	.x1
	; -- check offset
	addw	#16,<__bp,<__bx
	ldy	#1
	lda	<__bh
	cmp	[__si],Y
	bne	.l1
	lda	<__bl
	cmp	[__si]
.l1:	blo	.l2
	; -- eof
.x1:	stwz	<__ax
	sec
	rts
	; -- set base ptr
.l2:	addw	<__bx,<__si,<__dx
	; -- check length
	addw	<__ax,<__bx
	lda	[__si]
	sub	<__bl
	sta	<__bl
	lda	[__si],Y
	sbc	<__bh
	sta	<__bh
	bpl	.ok
	; -- adjust size
	addw	<__bx,<__ax
.ok:	stw	<__ax,<__cx
	clc
	rts

;
; restore the context of the library routines
; in LIB1_BANK
;
	.bank	LIB1_BANK
