	.bank LIB3_BANK

; sgx_load_vram
; ----
lib3_sgx_load_vram:
lib3_sgx_load_vram.3:

	; ----
	; map data
	;
	jsr	lib3_map_data

	; ----
	; setup call to TIA operation (fastest transfer)
	;
	; (instruction setup done during bootup...)

	stw	#sgx_video_data, ram_hdwr_tia_dest
;	stw	<__si, ram_hdwr_tia_src
;
;	asl	<__cl		; change from words to bytes (# to xfer)
;	rol	<__ch

	; ----
	; set vram address
	;
	jsr	lib3_sgx_set_write

	; ----
	; copy data
	;
	cly
	ldx	<__cl
	beq	.l3
	; --
.l1:	lda	[__si],Y
	sta	sgx_video_data_l
	iny
	lda	[__si],Y
	sta	sgx_video_data_h
	iny
	bne	.l2
	inc	<__si+1
	; --
.l2:	dex
	bne	.l1
	; --
	jsr	lib3_remap_data
	; --
.l3:	dec	<__ch
	bpl	.l1

;.l1:	lda	<__ch		; if zero-transfer, exit
;	ora	<__cl
;	beq	.out
;
;	lda	<__ch
;	cmp	#$20		; if more than $2000, repeat xfers of $2000
;	blo	.l2		; while adjusting banks
;	sub	#$20		; reduce remaining transfer amount
;	sta	<__ch
;
;	stw	#$2000, ram_hdwr_tia_size
;	jsr	ram_hdwr_tia
;
;	lda	<__si+1		; force bank adjust
;	add	#$20		; and next move starts at same location
;	sta	<__si+1
;
;	jsr	lib3_remap_data	; adjust banks
;	bra	.l1
;
;.l2:	sta	HIGH_BYTE ram_hdwr_tia_size	; 'remainder' transfer of < $2000
;	lda	<__cl
;	sta	LOW_BYTE  ram_hdwr_tia_size
;	jsr	ram_hdwr_tia

	; ----
	; unmap data
	;

.out:
	; restore PCE VDC address
	stw	#video_data, ram_hdwr_tia_dest
	jmp	lib3_unmap_data

; ----
; unmap_data
; ----
; IN :	_BX = old banks
; ----

lib3_unmap_data:

	lda	<__bl
	tam	#3
	lda	<__bh
	tam	#4
	rts

; ----
; remap_data
; ----

lib3_remap_data:
	lda	<__bp
	bne	.l1
	lda	<__si+1
	bpl	.l1
	sub	#$20
	sta	<__si+1
	tma	#4
	tam	#3
	inc a
	tam	#4
.l1:
	rts

; ----
; map_data
; ----
; map data in page 3-4 ($6000-$9FFF)
; ----
; IN :	_BL = data bank
;	_SI = data address
; ----
; OUT:	_BX = old banks
;	_SI = remapped data address
; ----

lib3_map_data:
	ldx	<__bl

	; ----
	; save current bank mapping
	;
	tma	#3
	sta	<__bl
	tma	#4
	sta	<__bh
	; --
	cpx	#$FE
	bne	.l1
	; --
	stx	<__bp
	rts

	; ----
	; map new bank
	;
.l1:	stz	<__bp
	; --
	txa
	tam	#3
	inc a
	tam	#4

	; ----
	; remap data address to page 3
	;
	lda	<__si+1
	and	#$1F
	ora	#$60
	sta	<__si+1
	rts


; ----
; sgx_set_write
; ----
; set the SGX VDC VRAM write pointer
; ----
; IN :	_DI = VRAM location
; ----

lib3_sgx_set_write:
	lda #$00
	sta	sgx_video_reg
	lda	<__di
	sta	sgx_video_data_l
.ifdef HUC
	sta	_sgx_vdc
.endif
	lda	<__di+1
	sta	sgx_video_data_h
.ifdef HUC
	sta	_sgx_vdc+1
.endif
	lda	#$02
	sta	sgx_video_reg
	rts


	.bank LIB1_BANK
