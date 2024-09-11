; ***************************************************************************
; ***************************************************************************
;
; hucc-systemcard.asm
;
; Macros and library functions for using the System Card.
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
; Because these are mainly macros, and so must be included before being used
; in compiled code, the actual functions here are written to avoid using any
; BSS memory so that HuCC's overlay global-shared-variables are not effected.
;
; ***************************************************************************
; ***************************************************************************

;
; Standard parameters for CD-ROM system functions
;

; These attributes are used for CD_PLAY:

CD_SECTOR	= %00000000
CD_MSF		= %01000000
CD_TRACK	= %10000000
CD_CURRPOS	= %11000000
CD_LEADOUT	= %11000000
CD_PLAYMODE	= %00111111

CDPLAY_MUTE	= 0
CDPLAY_REPEAT	= 1
CDPLAY_NORMAL	= 2
CDPLAY_ENDOFDISC= 0	; XXX: really?

CDFADE_CANCEL	= 0
CDFADE_PCM6	= 8
CDFADE_ADPCM6	= 10
CDFADE_PCM2	= 12
CDFADE_ADPCM2	= 14

CDTRK_AUDIO	= 0
CDTRK_DATA	= 4

GRP_FILL	= 1
GRP_NOFILL	= 0

BM_OK		= 0
BM_NOT_FOUND	= 1
BM_BAD_CHECKSUM = 2
BM_DIR_CORRUPTED= 3
BM_FILE_EMPTY	= 4
BM_FULL		= 5
BM_NOT_FORMATED = $FF			; HuC incorrect spelling.
BM_NOT_FORMATTED= $FF			; HuCC can use a dictionary!


		.bss
__bm_error	.ds	1
		.code


; ***************************************************************************
; ***************************************************************************
;
; unsigned int __fastcall __xsafe __macro cd_getver( void );

_cd_getver	.macro
		phx				; Preserve X (aka __sp).
		system	ex_getver
		txa				; Put version in A:Y
		say				; Put version in Y:A
		plx				; Restore X (aka __sp).
		.endm



; ***************************************************************************
; ***************************************************************************
;
; void __fastcall __xsafe __macro cd_boot( void );

_cd_boot	.macro
		system	cd_boot
		.endm



; ***************************************************************************
; ***************************************************************************
;
; void __fastcall __xsafe __macro cd_reset( void );

_cd_reset	.macro
		phx				; Preserve X (aka __sp).
		system	cd_reset
		plx				; Restore X (aka __sp).
		.endm



; ***************************************************************************
; ***************************************************************************
;
; unsigned char __fastcall __xsafe __macro cd_pause( void );

_cd_pause	.macro
		phx				; Preserve X (aka __sp).
		system	cd_pause
		cly
		plx				; Restore X (aka __sp).
		.endm



; ***************************************************************************
; ***************************************************************************
;
; void __fastcall __xsafe __macro cd_fade( unsigned char type<acc> );
;
;   type = $00 -> cancel fade
;	   $08 -> PCM fadeout 6 seconds
;	   $0A -> ADPCM fadeout 6 seconds
;	   $0C -> PCM fadeout 2.5 seconds
;	   $0E -> ADPCM fadeout 2.5 seconds

_cd_fade.1	.macro
		phx				; Preserve X (aka __sp).
		system	cd_fade
		plx				; Restore X (aka __sp).
		.endm



_cd_playgroup	.procgroup			; Routines share variables.

cdplay_end_ctl	ds	1	; saved 'cd_play end' information - type
cdplay_end_h	ds	1	; high byte of address
cdplay_end_m	ds	1	; mid byte
cdplay_end_l	ds	1	; low byte

; ***************************************************************************
; ***************************************************************************
;
; unsigned char __fastcall cd_unpause( void );

_cd_unpause	.proc

		lda	cdplay_end_ctl
		sta	<_dh
		lda	cdplay_end_h
		sta	<_cl
		lda	cdplay_end_m
		sta	<_ch
		lda	cdplay_end_l
		sta	<_dl
		lda	#CD_CURRPOS
		sta	<_bh

		system	cd_play
		tax				; Put return code in X.

		cly				; Return code in Y:X, X -> A.

		leave				; Return and copy X -> A.

		.endp



; ***************************************************************************
; ***************************************************************************
;
; unsigned char __fastcall cd_playtrk(
;  unsigned char start_track<_bx>, unsigned char end_track<_cx>, unsigned char mode<_dh> );
;
; mode = CDPLAY_MUTE / CDPLAY_REPEAT / CDPLAY_NORMAL

_cd_playtrk.3	.proc

		lda	<_dh
		and	#CD_PLAYMODE
		ora	#CD_TRACK
		sta	<_dh			; End type + play mode
		sta	cdplay_end_ctl
		lda	#CD_TRACK
		sta	<_bh			; Start type

		lda	<_cl
		bne	.endtrk

.endofdisc:	lda	<_dh			; Repeat to end of disc
		ora	#CD_LEADOUT
		sta	<_dh
		sta	cdplay_end_ctl
		bra	.starttrk

.endtrk:	system	ex_binbcd
		sta	<_cl			; End track
		sta	cdplay_end_h
		stz	<_ch
		stz	cdplay_end_m
		stz	<_dl
		stz	cdplay_end_l

.starttrk:	lda	<_bl			; Track #
		system	ex_binbcd
		sta	<_al			; From track
		stz	<_ah
		stz	<_bl

		system	cd_play
		tax				; Put return code in X.

		cly				; Return code in Y:X, X -> A.

		leave				; Return and copy X -> A.

		.endp



; ***************************************************************************
; ***************************************************************************
;
; unsigned char __fastcall cd_playmsf(
;  unsigned char start_minute<_al>,  unsigned char start_second<_ah>,  unsigned char start_frame<_bl>,
;  unsigned char end_minute<_cl>,  unsigned char end_second<_ch>,  unsigned char end_frame<_dl>,  unsigned char mode<_dh> );
;
; mode = CDPLAY_MUTE / CDPLAY_REPEAT / CDPLAY_NORMAL

_cd_playmsf.7	.proc

		lda	<_dh
		and	#CD_PLAYMODE
		ora	#CD_MSF
		sta	<_dh			; End type + play mode
		sta	cdplay_end_ctl
		lda	#CD_MSF
		sta	<_bh			; Start type

.endmsf:	lda	<_dl			; End frame
		system	ex_binbcd
		sta	<_dl
		sta	cdplay_end_l

		lda	<_ch			; End second
		system	ex_binbcd
		sta	<_ch
		sta	cdplay_end_m

		lda	<_cl			; End minute
		system	ex_binbcd
		sta	<_cl
		sta	cdplay_end_h

.startmsf:	lda	<_bl			; Start frame
		system	ex_binbcd
		sta	<_bl

		lda	<_ah			; Start second
		system	ex_binbcd
		sta	<_ah

		lda	<_al			; Start minute
		system	ex_binbcd
		sta	<_al

		system	cd_play
		tax				; Put return code in X.

		cly				; Return code in Y:X, X -> A.

		leave				; Return and copy X -> A.

		.endp

		.endprocgroup			; _cd_playgroup



; ***************************************************************************
; ***************************************************************************
;
; unsigned char __fastcall cd_loadvram( unsigned char ovl_index<_cl>, unsigned int sect_offset<_si>, unsigned int vramaddr<_bx>, unsigned int bytes<_ax> );

_cd_loadvram.4	.proc

		ldx	<_cl			; Get file address and length.
		jsr	get_file_lba

		lda.l	<_si			; Add the sector offset.
		ldy.h	<_si
		jsr	_add_sectors

		lda	#$FE			; Signal a VRAM load.
		sta	<_dh

		system	cd_read
		tax				; Put return code in X.

		cly				; Return code in Y:X, X -> A.

		leave				; Return and copy X -> A.

		.endp



; ***************************************************************************
; ***************************************************************************
;
; unsigned char __fastcall cd_loaddata( unsigned char ovl_index<_cl>, unsigned int sect_offset<_si>, unsigned char __far *buffer<_bp_bank:_bp>, unsigned int bytes<__ptr> );
;
; This is overly complicated because the HuC/MagicKit design reads 0..65535
; bytes instead of complete sectors.

_cd_loaddata.4	.proc

		tma3				; Preserve MPR3 and MPR4.
		pha
		tma4
		pha

		ldx	<_cl			; Get file's LBA (not length).
		jsr	get_file_lba

		lda.l	<_si			; Add the sector offset.
		ldy.h	<_si
		jsr	_add_sectors

		ldy	<_bp_bank		; Map data to MPR3 & MPR4.
		jsr	set_bp_to_mpr34

		tii	_bp, _bx, 2		; Set destination ptr.

.loop:		ldy.l	.cdread_len		; Still bytes to read?
		lda.h	.cdread_len
		bne	!+
		cpy	#0
		beq	.done

!:		cmp.h	#$2000			; Is there 8KB left to read?
		bcc	.read_end

.read_max:	sbc.h	#$2000			; Read the next 8KB of data.
		sta.h	.cdread_len
		cly
		lda.h	#$2000
		bra	!+

.read_end:	stz.l	.cdread_len		; Read the final data.
		stz.h	.cdread_len

!:		sty.l	<_ax			; Set read length in bytes.
		sta.h	<_ax

		stz	<_dh			; Signal bytes, not sectors.

		tii	_bx, .cdread_ptr, 5	; Preserve parameters.

		system	cd_read
		tax				; Put return code in X.
		bne	.done			; Exit if NZ error code.

		tii	.cdread_ptr, _bx, 5	; Restore parameters.

		lda	#4			; Add the #sectors just read.
		cly
		jsr	_add_sectors

		tma4				; Increment the bank.
		tam3
		inc	a
		tam4

		bra	.loop			; Load the next chunk of data.

.done:		pla				; Restore MPR3 and MPR4.
		tam4
		pla
		tam3

		cly				; Return code in Y:X, X -> A.

		leave				; Return and copy X -> A.

.cdread_len	=	__ptr			; Reuse ZP location.
.cdread_ptr	.ds	2
.cdread_lba	.ds	3

		.endp



; ***************************************************************************
; ***************************************************************************
;
; unsigned char __fastcall cd_loadbank( unsigned char ovl_index<_cl>, unsigned int sect_offset<_si>, unsigned char bank<_bl>, unsigned int sectors<_al> );

_cd_loadbank.4	.macro
		phx				; Preserve X (aka __sp).

		ldx	<_cl			; Get file address and length.
		jsr	get_file_info

		lda.l	<_si			; Add the sector offset.
		ldy.h	<_si
		jsr	_add_sectors

		lda	<_dh			; Get #sectors (0=whole file).
		beq	!+
		sta	<_al

!:		lda	#4			; Use MPR4.
		sta	<_dh

		tma4				; Preserve MPR4.
		pha

		system	cd_read

		tax				; Restore MPR4 because the
		pla				; System Card function has
		tam4				; a bug which destroys the
		txa				; value on a CD error.

		cly
		plx				; Restore X (aka __sp).
		.endm



; ***************************************************************************
; ***************************************************************************
;
; void __fastcall __xsafe __macro cd_status( unsigned char mode<acc> );

_cd_status.1	.macro
		phx				; Preserve X (aka __sp).
		system	cd_stat
		cly
		plx				; Restore X (aka __sp).
		.endm



; ***************************************************************************
; ***************************************************************************
;
; void __fastcall __xsafe __macro ad_reset( void );

_ad_reset	.macro
		phx				; Preserve X (aka __sp).
		system	ad_reset
		plx				; Restore X (aka __sp).
		.endm


; ***************************************************************************
; ***************************************************************************
;
; unsigned char __fastcall __xsafe __macro ad_trans( unsigned char ovl_index<_cl>, unsigned int sect_offset<_si>, unsigned char nb_sectors<_dh>, unsigned int ad_addr<_bx> );

_ad_trans.4	.macro
		phx				; Preserve X (aka __sp).
		ldx	<_cl			; Get file address and length.
		jsr	get_file_info

		lda.l	<_si			; Add the sector offset.
		ldy.h	<_si
		jsr	_add_sectors

		lda	<_dh			; Get #sectors (0=whole file).
		stz	<_dh			;
		beq	!+
		sta	<_al			; #sectors if not whole file.

!:		system	ad_trans
		cly
		plx				; Restore X (aka __sp).
		.endm

; ***************************************************************************
; ***************************************************************************
;
; void __fastcall __xsafe __macro ad_read( unsigned int ad_addr<_cx>, unsigned char mode<_dh>, unsigned int buf<_bx>, unsigned int bytes<_ax> );

_ad_read.4	.macro
		phx				; Preserve X (aka __sp).
		system	ad_read
		plx				; Restore X (aka __sp).
		.endm



; ***************************************************************************
; ***************************************************************************
;
; void __fastcall __xsafe __macro ad_write( unsigned int ad_addr<_cx>, unsigned char mode<_dh>, unsigned int buf<_bx>, unsigned int bytes<_ax> );

_ad_write.4	.macro
		phx				; Preserve X (aka __sp).
		system	ad_write
		plx				; Restore X (aka __sp).
		.endm


; ***************************************************************************
; ***************************************************************************
;
; unsigned char __fastcall __xsafe __macro ad_play( unsigned int ad_addr<_bx>, unsigned int bytes<_ax>, unsigned char freq<_dh>, unsigned char mode<_dl> );

_ad_play.4	.macro
		phx				; Preserve X (aka __sp).
		system	ad_play
		cly
		plx				; Restore X (aka __sp).
		.endm



; ***************************************************************************
; ***************************************************************************
;
; unsigned char __fastcall __xsafe __macro ad_cplay( unsigned char ovl_index<_cl>, unsigned int sect_offset<_si>, unsigned int nb_sectors<_bx>, unsigned char freq<_dh> );

_ad_cplay.4	.macro
		phx				; Preserve X (aka __sp).
		ldx	<_cl			; Get file address and length.
		jsr	get_file_info

		lda.l	<_si			; Add the sector offset.
		ldy.h	<_si
		jsr	_add_sectors

		lda.l	<_bx			; Get #sectors (0=whole file).
		ora.h	<_bx
		beq	!+
		lda.l	<_bx
		ldy.h	<_bx
		sta.l	<_ax
		sty.h	<_ax
!:		stz	<_bl			; Clear length bits 16..23.

		system	ad_cplay
		cly
		plx				; Restore X (aka __sp).
		.endm



; ***************************************************************************
; ***************************************************************************
;
; void __fastcall __xsafe __macro ad_stop( void );

_ad_stop	.macro
		phx				; Preserve X (aka __sp).
		system	ad_stop
		plx				; Restore X (aka __sp).
		.endm



; ***************************************************************************
; ***************************************************************************
;
; unsigned char __fastcall __xsafe __macro ad_stat( void );

_ad_stat	.macro
		phx				; Preserve X (aka __sp).
		system	ad_stat
		cly
		plx				; Restore X (aka __sp).
		.endm



; ***************************************************************************
; ***************************************************************************
;
; void __fastcall __xsafe add_sectors( unsigned int sector_offset<acc> );

_add_sectors:	clc
		adc	<_dl
		sta	<_dl
		tya
		adc	<_ch
		sta	<_ch
		bcc	!+
		inc	<_cl
!:		rts



; ***************************************************************************
; ***************************************************************************
;
; unsigned char __fastcall bm_check( void );
;
; Determine whether BRAM exists on this system
;
; Returns 1 (TRUE) if BRAM exists, else 0 (FALSE).

_bm_check	.proc

		php				; Disable interrupts!
		sei

		tma3				; Preserve MPR3.
		pha

		csl				; BRAM needs lo-speed access.

		lda	#$48			; Extended 3-byte unlock
		sta	IFU_BRAM_UNLOCK		; sequence for Tennokoe 2.
		lda	#$75
		sta	IFU_BRAM_UNLOCK
		lda	#$80
		sta	IFU_BRAM_UNLOCK

		lda	#$F7			; Map in the BRAM and shadow.
		tam3

		lda	$6000			; Confirm BRAM present.
		tay
		eor	#$FF
		sta	$6000
		eor	$6000
		sty	$6000
		sta	<__temp

		lda	IFU_BRAM_LOCK		; Lock BRAM.

		pla				; Restore MPR3.
		tam3

		csh				; Restore hi-speed access.
		plp				; Restore original IRQ mask.

		ldx	<__temp			; $00 if BRAM exists, else NZ.
		beq	.done
		ldx	#$FF

.done:		inx				; $01 if BRAM exists, else $00.

		cly				; Return code in Y:X, X -> A.
		leave				; Return and copy X -> A.

		.endp



; ***************************************************************************
; ***************************************************************************
;
; unsigned char __fastcall bm_format( void );
;
; If BRAM is already formatted (*_PROPERLY_*) return OK, else format it.
;
; Returns either BM_OK, BM_NOT_FOUND.
;
; After return __bm_error = BM_OK, BM_NOT_FOUND.

_bm_format	.proc

		system	bm_free
		sta	__bm_error
		tax
		beq	.done

		lda.l	#.password
		sta.l	<_ax
		lda.h	#.password
		sta.h	<_ax

		system	bm_format
		sta	__bm_error
		tax
		beq	.done

		ldx	#BM_NOT_FORMATTED

.done:		stx	__bm_error		; Return code in Y:X, X -> A.
		cly
		leave				; Return and copy X -> A.

.password:	db	"!BM FORMAT!"

		.endp



; ***************************************************************************
; ***************************************************************************
;
; unsigned int __fastcall __xsafe __macro bm_free( void );
;
; Returns (int) number of user bytes available in BRAM, or 0 if error.
;
; After return __bm_error = BM_OK or BM_NOT_FORMATTED.

_bm_free	.macro
		phx				; Preserve X (aka __sp).
		system	bm_free
		sta	__bm_error
		tay
		beq	!ok+
		stz.l	<_cx			; Return size 0 if error.
		stz.h	<_cx
!ok:		lda.l	<_cx
		ldy.h	<_cx
		plx				; Restore X (aka __sp).
		.endm



; ***************************************************************************
; ***************************************************************************
;
; unsigned char __fastcall __xsafe __macro bm_read( unsigned char *buffer<_bx>, unsigned char *name<_ax>, unsigned int offset<_dx>, unsigned int length<_cx> );
;
; Check for existence of BRAM file with a matching name
;
; Note: name is 12 bytes; first 2 bytes are a uniqueness value
; (should be zeroes), and the next 10 bytes are the ASCII name
; with trailing spaces as padding.
;
; Returns either BM_OK, BM_NOT_FOUND, BM_BAD_CHECKSUM or BM_NOT_FORMATTED.
;
; After return __bm_error = BM_OK or BM_NOT_FORMATTED.

_bm_read.4	.macro
		phx				; Preserve X (aka __sp).
		system	bm_read
		sta	__bm_error
		cly
		plx				; Restore X (aka __sp).
		.endm



; ***************************************************************************
; ***************************************************************************
;
; unsigned char __fastcall __xsafe __macro bm_write( unsigned char *buffer<_bx>, unsigned char *name<_ax>, unsigned int offset<_dx>, unsigned int length<_cx> );
;
; Given the name of a BRAM file, update some info inside of it
;
; Note: name is 12 bytes; first 2 bytes are a uniqueness value
; (should be zeroes), and the next 10 bytes are the ASCII name
; with trailing spaces as padding.
;
; Returns either BM_OK, BM_NOT_FOUND (i.e. not enough memory) or BM_NOT_FORMATTED.
;
; After return __bm_error = BM_OK, BM_NOT_FOUND (i.e. not enough memory) or BM_NOT_FORMATTED.

_bm_write.4	.macro
		phx				; Preserve X (aka __sp).
		system	bm_write
		sta	__bm_error
		cly
		plx				; Restore X (aka __sp).
		.endm



; ***************************************************************************
; ***************************************************************************
;
; unsigned char __fastcall __xsafe __macro bm_delete( unsigned char *name<_ax> );
;
; Delete the entry specified by the name provided
;
; Note: name is 12 bytes; first 2 bytes are a uniqueness value
; (should be zeroes), and the next 10 bytes are the ASCII name
; with trailing spaces as padding.
;
; Returns either BM_OK, BM_NOT_FOUND or BM_NOT_FORMATTED.
;
; After return __bm_error = BM_OK, BM_NOT_FOUND or BM_NOT_FORMATTED.

_bm_delete.1	.macro
		phx				; Preserve X (aka __sp).
		system	bm_delete
		sta	__bm_error
		cly
		plx				; Restore X (aka __sp).
		.endm



; ***************************************************************************
; ***************************************************************************
;
; unsigned char __fastcall __xsafe __macro bm_exist( unsigned char *name<_ax> );
;
; Check for existence of BRAM file with a matching name
;
; Note: name is 12 bytes; first 2 bytes are a uniqueness value
; (should be zeroes), and the next 10 bytes are the ASCII name
; with trailing spaces as padding.
;
; Returns 1 (TRUE) if the file exists and is OK, else 0 (FALSE).
;
; N.B. Pointless HuC function, it's easier to try to read the file.

_bm_exist.1	.macro
		stz.l	<_bx			; Destination buffer.
		stz.h	<_bx
		stz.l	<_cx			; Length to read.
		stz.h	<_cx
		stz.l	<_dx			; Offset from start.
		stz.h	<_dx
		phx				; Preserve X (aka __sp).
		system	bm_read
		sta	__bm_error
		tay				; $00 if file OK, else NZ.
		beq	!done+
		lda	#$FF
!done:		inc	a			; $01 if file OK, else $00.
		cly
		plx				; Restore X (aka __sp).
		.endm



; ***************************************************************************
; ***************************************************************************
;
; unsigned char __fastcall __xsafe __macro bm_create( unsigned char *name<_ax>, unsigned int length<_cx> );
;
; Create a new BRAM file, given the name and size
;
; Note: name is 12 bytes; first 2 bytes are a uniqueness value
; (should be zeroes), and the next 10 bytes are the ASCII name
; with trailing spaces as padding.
;
; Returns either BM_OK, BM_NOT_FOUND (i.e. not enough memory) or BM_NOT_FORMATTED.
;
; After return __bm_error = BM_OK, BM_NOT_FOUND (i.e. not enough memory) or BM_NOT_FORMATTED.
;
; N.B. Pointless HuC function, it's easier to try to write the file.

_bm_create.2	.macro
		stz.l	<_bx			; Source buffer.
		lda.h	#$6000
		sta.h	<_bx
		stz.l	<_dx			; Offset from start.
		stz.h	<_dx
		phx				; Preserve X (aka __sp).
		system	bm_write
		sta	__bm_error
		cly
		plx				; Restore X (aka __sp).
		.endm
