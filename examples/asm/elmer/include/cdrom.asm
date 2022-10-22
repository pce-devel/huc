; ***************************************************************************
; ***************************************************************************
;
; cdrom.asm
;
; Functions for developers using the PC Engine CD-ROM (fast versions).
;
; This is derived from Hudson's original fast CD code in Legend of Xanadu 2,
; together with their modifications for streaming from Gulliver Boy.
;
; Significant improvements have been made to optimize the code speed and to
; reduce the code size.
;
; Modifications copyright John Brandwood 2022.
;
; Distributed under the Boost Software License, Version 1.0.
; (See accompanying file LICENSE_1_0.txt or copy at
;  http://www.boost.org/LICENSE_1_0.txt)
;
; ***************************************************************************
; ***************************************************************************
;
; ONLY USE THESE PUBLIC INTERFACES ...
;
; cdr_read_ram   - with CPU memory as destination (like CD_READ to RAM).
; cdr_read_bnk   - with BNK memory as destination (like CD_READ to RAM).
; cdr_read_vram  - with SGX or VDC VRAM as destination (like CD_READ to VDC).
; cdr_read_acd   - with ACD memory as destination (like CD_READ to ACD).
;
; cdr_ad_trans   - transfer data from CD to ADPCM buffer (BIOS AD_TRANS).
; cdr_ad_cplay   - start ADPCM streaming playback from CD (BIOS AD_CPLAY).
; cdr_ad_stop    - stop streaming ADPCM playback (BIOS AD_STOP).
;
; cdr_stream_ram - with CPU memory as destination (like CD_READ to RAM).
; cdr_stream_acd - with ACD memory as destination (like CD_READ to ACD).
;
; cdr_init_disc  - Initialize the CD drive and scan the CD TOC.
;
; cdr_reset      - Reset CD drive (BIOS CD_RESET).
; cdr_stat       - Check CD drive status (BIOS CD_STAT).
; cdr_contents   - Scan the disc's TOC to find the layout (BIOS CD_CONTENTS).
; cdr_dinfo      - Read the disc's TOC (Table Of Contents) (BIOS CD_DINFO).
;
; ***************************************************************************
; ***************************************************************************

;
; Configure Library ...
;

	.ifndef	SUPPORT_ACD
SUPPORT_ACD	=	1
	.endif

	.ifndef	SUPPORT_CDINIT
SUPPORT_CDINIT	=	1
	.endif

	.ifndef	SUPPORT_CDVRAM
SUPPORT_CDVRAM	=	1
	.endif

	.ifndef	SUPPORT_ADPCM
SUPPORT_ADPCM	=	0
	.endif

	.ifndef	SUPPORT_HUVIDEO
SUPPORT_HUVIDEO	=	0
	.endif

	.ifndef	SUPPORT_TIMING
SUPPORT_TIMING	=	0
	.endif

;
; Include dependancies ...
;

	.if	SUPPORT_CDVRAM
		include "common.asm"		; Common helpers.
	.endif

;
; Include more functionality ...
;

	.ifdef	SUPPORT_ADPCM
		include	"adpcm.asm"		; Include ADPCM routines.
	.endif



; ***************************************************************************
; ***************************************************************************
;
; Definitions
;
; In technical terms, the PC Engine's communication with the CD-ROM drive is
; through a bit-banged SCSI-1 interface.
;
; This can look a bit like magic until you take a look at the SCSI standard,
; which is a tremendous help in figuring things out.
;
; Reference: https://en.wikipedia.org/wiki/Parallel_SCSI
; Reference: Search for the final Draft SCSI-2 Standard (X3T9.2 rev 10L)
;

; SCSI bus signals.

SCSI_MSK	=	$F8	; IFU_SCSI_FLG
SCSI_BSY	=	$80	; IFU_SCSI_FLG Bus is Busy or Free
SCSI_REQ	=	$40	; IFU_SCSI_FLG Target Requests next Xfer
SCSI_MSG	=	$20	; IFU_SCSI_FLG Bus contains a Message
SCSI_CXD	=	$10	; IFU_SCSI_FLG Bus contains Control or /Data
SCSI_IXO	=	$08	; IFU_SCSI_FLG Bus Initiator or Target

SCSI_ACK	=	$80	; IFU_IRQ_MSK

; SCSI bus phases.

PHASE_COMMAND	=	$D0	; (SCSI_BSY + SCSI_REQ + ........ + SCSI_CXD + ........)
PHASE_DATA_OUT	=	$C0	; (SCSI_BSY + SCSI_REQ + ........ + ........ + ........)
PHASE_DATA_IN	=	$C8	; (SCSI_BSY + SCSI_REQ + ........ + ........ + SCSI_IXO)
PHASE_STAT_IN	=	$D8	; (SCSI_BSY + SCSI_REQ + ........ + SCSI_CXD + SCSI_IXO)
PHASE_MESG_OUT	=	$F0	; (SCSI_BSY + SCSI_REQ + SCSI_MSG + SCSI_CXD + ........)
PHASE_MESG_IN	=	$F8	; (SCSI_BSY + SCSI_REQ + SCSI_MSG + SCSI_CXD + SCSI_IXO)

; CDROM SCSI commands.

CDROM_TESTRDY	=	$00	; (06 nd) TEST UNIT READY
CDROM_REQSENSE	=	$03	; (06 nd) REQUEST SENSE
CDROM_READ	=	$08	; (06 D ) READ
CDROM_MODESEL	=	$15	; (10 nd) MODE SELECT
CDROM_AUDIOTSRC	=	$D8	; (10 D ) AUDIO TRACK SEARCH
CDROM_PLAYAUDIO	=	$D9	; (10 nd) PLAY AUDIO
CDROM_PAUSE	=	$DA	; (10 nd) STILL
CDROM_READSUBQ	=	$DD	; (10 nd) READ SUBCODE-Q & PLAYING STATUS
CDROM_READTOC	=	$DE	; (10 nd) READ DISC INFORMATION (TOC)

CDROM_STSPUNT	=	$FF
CDROM_TRYOPEN	=	$FF
CDROM_TRYCLOS	=	$FF

; CDROM status codes.

CDSTS_GOOD	=	$00	; Good
CDSTS_CHECK	=	$02	; Check condition
CDSTS_TIMEOUT	=	$06	; Timeout

; CDROM SCSI errors.

CDERR_BAD_SENSE	=	$00
CDERR_NOT_READY	=	$04
CDERR_RESET	=	$06
CDERR_NO_DISC	=	$0B
CDERR_NO_COVER	=	$0D
CDERR_BAD_ECC	=	$11
CDERR_BAD_SEEK	=	$15
CDERR_NO_HEADER	=	$16
CDERR_NOT_AUDIO	=	$1C
CDERR_NOT_DATA	=	$1D
CDERR_BAD_CDB	=	$20
CDERR_BAD_ADDR	=	$21
CDERR_BAD_PARAM	=	$22
CDERR_DISC_END	=	$25
CDERR_BAD_LIST	=	$2A
CDERR_BAD_PLAY	=	$2C

; CDROM library errors, not actually SCSI.

CDERR_NO_CDIFU	=	$80
CDERR_NO_DRIVE	=	$81



; ***************************************************************************
; ***************************************************************************
;
; Data
;

		.zp

scsi_ram_ptr	=	_bp			; Sector write addr in RAM.

		.bss

	.if	SUPPORT_CDINIT			; Never read by System Card!
cdrom_track	=	vdtin_flg		; Now used to save the CDROM
	.endif	SUPPORT_CDINIT			; track#, or $FF if none.

	.if	SUPPORT_ADPCM
cplay_playing:	ds	1			; NZ is true.
cplay_filling:	ds	1			; NZ is true.
cplay_status:	ds	1			; Result of last CD read.
cplay_scsi_buf	=	scsi_send_buf		; Reuse the SCSI buffer.
	.endif	SUPPORT_ADPCM

	.if	SUPPORT_TIMING
scsi_stat_indx:	ds	1			; Track CD-ROM loading speed
scsi_stat_time:	ds	256			; (cdr_read_bnk & 
	.endif	SUPPORT_TIMING

		.code



; ***************************************************************************
; ***************************************************************************
; ***************************************************************************
;
; Routines that must be in permanently mapped memory.
;
; ***************************************************************************
; ***************************************************************************
; ***************************************************************************



	.if	SUPPORT_ADPCM

; ***************************************************************************
; ***************************************************************************
;
; scsi_handshake - Clock a byte of data onto the SCSI bus.
;
; N.B. On a HuCARD, this must be available for cdr_cplay_irq2!
;

scsi_handshake:	lda	#SCSI_ACK		; Send SCSI_ACK signal.
		tsb	IFU_SCSI_ACK

!:		tst	#SCSI_REQ, IFU_SCSI_FLG	; Wait for drive to clear
		bne	!-			; the SCSI_REQ signal.

		trb	IFU_SCSI_ACK		; Clear SCSI_ACK signal.
		rts



; ***************************************************************************
; ***************************************************************************
;
; cdr_cplay_irq2 - IRQ2 handler for the new AD_CPLAY functionality.
;
; This handles background-streaming the ADPCM data from the CD to ADPCM RAM.
;

cdr_cplay_irq2:	pha

		lda	IFU_IRQ_MSK		; What caused the interrupt?
		and	IFU_IRQ_FLG

		bit	#IFU_INT_HALF
		bne	.got_int_half
		bit	#IFU_INT_DAT_IN
		bne	.got_int_dat
		bit	#IFU_INT_MSG_IN
		bne	.got_int_msg
		bit	#IFU_INT_END
		bne	.got_int_stop
		bra	.irq2_done

		; Data read finished (PHASE_STAT_IN or PHASE_MESG_IN).

.got_int_msg:	lda	#IFU_INT_MSG_IN		; Disable IFU_INT_MSG_IN.
		trb	IFU_IRQ_MSK

		lda	IFU_SCSI_FLG		; Which IFU_INT_MSG_IN is it?
		and	#SCSI_MSK
		cmp	#PHASE_MESG_IN
		beq	.phase_mesg_in

		; PHASE_STAT_IN interrupt.

.phase_stat_in:	lda	#IFU_INT_DAT_IN		; Disable IFU_INT_DAT_IN.
		trb	IFU_IRQ_MSK

		stz	IFU_ADPCM_DMA		; Stop ADPCM DMA from the CD.

		lda	IFU_SCSI_DAT		; Get the status code.
		sta	cplay_status		; Remember it.
		jsr	scsi_handshake

		lda	#IFU_INT_MSG_IN		; Enable IFU_INT_MSG_IN so we
		tsb	IFU_IRQ_MSK		; respond to PHASE_MESG_IN.

		bra	.irq2_done

		; PHASE_MESG_IN interrupt.

.phase_mesg_in:	stz	cplay_filling		; Signal ADPCM not loading.

		lda	IFU_SCSI_DAT		; Flush the MESG byte.
		bsr	scsi_handshake

		lda	cplay_status		; So, how was the last read?
		bne	.read_error

		lda	#IFU_INT_HALF		; Enable IFU_INT_HALF now the
		tsb	IFU_IRQ_MSK		; buffer has been refilled.

		bra	.irq2_done

		; PHASE_DATA_IN interrupt.

.got_int_dat:	lda	#IFU_INT_DAT_IN		; Disable IFU_INT_DAT_IN.
		trb	IFU_IRQ_MSK

		lda	#$02			; Start ADPCM DMA from CD.
		sta	IFU_ADPCM_DMA

		bra	.irq2_done

		;

.got_int_stop:	lda	#IFU_INT_END		; Disable IFU_INT_END.
		trb	IFU_IRQ_MSK

		lda	#$01			; Disable IRQ2 vector.
		trb	<irq_vec

		stz	cplay_playing		; Signal ADPCM finished.
		bra	.irq2_done

		;

.read_error:	stz.h	cplay_len_l		; Stop further reads, but let
		stz.l	cplay_len_l		; the buffered audio finish.

		lda	#IFU_INT_DAT_IN + IFU_INT_MSG_IN + IFU_INT_HALF
		trb	IFU_IRQ_MSK

.irq2_done:	pla
		rti

		; Less than 50% left, refill?

.got_int_half:	lda	#IFU_INT_HALF		; Disable IFU_INT_HALF.
		trb	IFU_IRQ_MSK

		lda.h	cplay_len_l		; Are we finished yet?
		ora.l	cplay_len_l
		beq	.irq2_done

		phx
		phy
		call	cdr_cplay_next		; Read next 32KB of ADPCM.
		ply
		plx
		bcs	.read_error

		lda	#IFU_INT_DAT_IN + IFU_INT_MSG_IN
		tsb	IFU_IRQ_MSK
		bra	.irq2_done

	.endif	SUPPORT_ADPCM



; ***************************************************************************
; ***************************************************************************
; ***************************************************************************
;
; End of routines that must be in permanently mapped memory.
;
; ***************************************************************************
; ***************************************************************************
; ***************************************************************************

		.procgroup



	.if	!SUPPORT_ADPCM

; ***************************************************************************
; ***************************************************************************
;
; scsi_handshake - Clock a byte of data onto the SCSI bus.
;
; On a CDROM, this can be in the same bank as the rest!
;

scsi_handshake:	lda	#SCSI_ACK		; Send SCSI_ACK signal.
		tsb	IFU_SCSI_ACK

!:		tst	#SCSI_REQ, IFU_SCSI_FLG	; Wait for drive to clear
		bne	!-			; the SCSI_REQ signal.

		trb	IFU_SCSI_ACK		; Clear SCSI_ACK signal.
		rts

	.endif	!SUPPORT_ADPCM



; ***************************************************************************
; ***************************************************************************
;
; scsi_get_phase - As a subroutine this helps provide a delay between reads!
;

scsi_get_phase:	lda	IFU_SCSI_FLG
		and	#SCSI_MSK
		rts



; ***************************************************************************
; ***************************************************************************
;
; scsi_delay - Delay 500us * Y.
;

scsi_delay:	clx				; The inner loop takes 3584
.wait_500us:	sxy				; cycles, which is almost a
		sxy				; perfect 500 microseconds!
		nop
		dex
		bne	.wait_500us
		dey
		bne	.wait_500us
		rts



; ***************************************************************************
; ***************************************************************************
;
; scsi_initiate - Initiate CD-ROM command.
;

scsi_cd_busy:	lda	#IFU_INT_DAT_IN + IFU_INT_MSG_IN
		trb	IFU_IRQ_MSK		; Disable SCSI phase interrupts.

		sta	IFU_SCSI_CTL		; Set control bits.

		ldy	#30 * 2			; Wait 30ms.
		bsr	scsi_delay

		lda	#$FF
		sta	IFU_SCSI_DAT

		tst	#SCSI_REQ, IFU_SCSI_FLG
		beq	scsi_initiate

.flush_data:	jsr	scsi_handshake		; Flush out stale data.

.still_busy:	tst	#SCSI_REQ, IFU_SCSI_FLG
		bne	.flush_data
		tst	#SCSI_BSY, IFU_SCSI_FLG
		bne	.still_busy

scsi_initiate:	bsr	scsi_select_cd		; Acquire the SCSI bus.
		bcs	scsi_cd_busy

.test_scsi_bus:	ldy	#18			; Wait for up to 20ms for the
		clx				; CD-ROM to acknowledge.
.wait_scsi_bus:	bsr	scsi_get_phase
		and	#SCSI_BSY
		bne	.ready
		dex
		bne	.wait_scsi_bus
		dey
		bne	.wait_scsi_bus

		; Timeout, try again.

		bra	scsi_initiate

.ready:		rts



; ***************************************************************************
; ***************************************************************************
;
; scsi_select_cd - Select the SCSI PCE-CDROM device.
;

scsi_select_cd:	sec

		lda	#$81			; Abridged SCSI device
		sta	IFU_SCSI_DAT		; selection phase.

		tst	#SCSI_BSY, IFU_SCSI_FLG	; Is the SCSI bus already
		bne	.done			; busy?

		sta	IFU_SCSI_CTL		; Magic Number!
		clc

.done:		rts				; Returns CS if busy.



; ***************************************************************************
; ***************************************************************************
;
; Init SCSI command in scsi_send_buf.
;
; A = SCSI command byte to write at the head of the buffer.
;

scsi_init_cmd:	ldx	#10 - 1
.loop:		stz	scsi_send_buf, x
		dex
		bne	.loop
		sta	scsi_send_buf
		rts



; ***************************************************************************
; ***************************************************************************
;
; Send a SCSI CDROM_READ command to the CD drive.
;
; recbase = which data track (0 or 1)
;
; _cl = offset_h
; _ch = offset_m
; _dl = offset_l
; _al = length_l
;

scsi_read_cmd:	lda	recbase			; Which data track (0 or 1)?
		asl	a
		clc
		adc	recbase
		tax				; Which data track (0 or 3)?

		stz	scsi_send_buf + 5	; scsi_ctrl

		lda	<_al
		sta	scsi_send_buf + 4	; scsi_len_l =	bios_len_l

		clc
		lda	<_dl
		adc	recbase0_l, x		; Start of data track (x = 0 or 3).
		sta	scsi_send_buf + 3	; scsi_lba_l = bios_rec_l + data_lba_l

		lda	<_ch
		adc	recbase0_m, x		; Start of data track (x = 0 or 3).
		sta	scsi_send_buf + 2	; scsi_lba_m = bios_rec_m + data_lba_m

		lda	<_cl
		adc	recbase0_h, x		; Start of data track (x = 0 or 3).
		sta	scsi_send_buf + 1	; scsi_lba_h = bios_rec_h + data_lba_h

		lda	#CDROM_READ		; SCSI read command.
		sta	scsi_send_buf + 0	; scsi_opcode

		ldy	#6			; 6-byte SCSI command.
;		bra	scsi_send_cmd



; ***************************************************************************
; ***************************************************************************
;
; scsi_send_cmd - Send SCSI command in scsi_send_buf to the CD drive.
;
; Y = # bytes in SCSI command.
;

scsi_send_cmd:	clx

.send_byte:	lda	IFU_SCSI_FLG
		and	#SCSI_MSK
		cmp	#PHASE_COMMAND
		bne	.send_byte

		lda	scsi_send_buf, x
		sta	IFU_SCSI_DAT
		jsr	scsi_handshake

		inx
		dey
		bne	.send_byte

		rts



; ***************************************************************************
; ***************************************************************************
;
; scsi_get_status - Do the final phases of the SCSI transaction sequence.
;
; Returns: Y,Z-flag,N-flag = CDSTS_GOOD ($00) or an error code.
;

	.if	1

scsi_get_status:ldy	IFU_SCSI_DAT		; Read status code.
		jsr	scsi_handshake

.wait_mesg:	jsr	scsi_get_phase		; Do the final phases of the
		cmp	#PHASE_STAT_IN		; SCSI transaction sequence.
		beq	scsi_get_status
		cmp	#PHASE_MESG_IN
		bne	.wait_mesg

.read_mesg:	lda	IFU_SCSI_DAT		; Flush message byte.
		jsr	scsi_handshake

.wait_exit:	bit	IFU_SCSI_FLG		; Wait for the CD-ROM to
		bmi	.wait_exit		; release SCSI_BSY.

		tya				; Return status & set flags.
		rts

	.else

scsi_get_status:cly				; In case no PHASE_STAT_IN!

.wait_mesg:	jsr	scsi_get_phase		; Do the final phases of the
		cmp	#PHASE_STAT_IN		; SCSI transaction sequence.
		beq	.read_status
		cmp	#PHASE_MESG_IN
		beq	.read_mesg
		bra	.wait_mesg

.read_status:	ldy	IFU_SCSI_DAT		; Read status code.
		jsr	scsi_handshake
		bra	.wait_mesg

.read_mesg:	lda	IFU_SCSI_DAT		; Flush message byte.
		jsr	scsi_handshake

.wait_exit:	tst	#SCSI_BSY, IFU_SCSI_FLG	; Wait for the CD-ROM to
		bne	.wait_exit		; release the SCSI bus.

		tya				; Return status & set flags.
		rts

	.endif



; ***************************************************************************
; ***************************************************************************
;
; scsi_req_sense - Read the SENSE data to get the last command's error code.
;
; Return byte $00 = Response code ($70 or $71)
; Return byte $01 = Obsolete
; Return byte $02 = Flags
; Return byte $03 = Info + 0
; Return byte $04 = Info + 1
; Return byte $05 = Info + 2
; Return byte $06 = Info + 3
; Return byte $07 = Additional length (N-7)
; Return byte $08 = ???
; Return byte $09 = Previous error code
;
; Returns: Y,Z-flag,N-flag = CDSTS_GOOD ($00) or an error code.
;
; N.B. FOR INTERNAL USE ONLY, THIS IS NOT A PUBLIC FUNCTION!
;

		; Initiate CD-ROM command.

scsi_req_sense:	jsr	scsi_initiate		; Acquire the SCSI bus.

		; Process SCSI phases.

.proc_scsi_loop:jsr	scsi_get_phase
		cmp	#PHASE_COMMAND
		beq	.send_command
		cmp	#PHASE_DATA_IN
		beq	.get_response
		cmp	#PHASE_STAT_IN
		bne	.proc_scsi_loop

		; SCSI command done.

.command_done:	jsr	scsi_get_status		; Returns code in Y & A.
;		cmp	#CDSTS_GOOD
		bne	scsi_req_sense

		ldy	scsi_recv_buf + 9	; Return the final byte of the
		rts				; REQUEST SENSE reply.

		; Send SCSI command.

.send_command:	ldx	#6 - 1			; Send CDROM_REQSENSE command
.send_byte:	lda	IFU_SCSI_FLG		; while preserving the failed
		and	#SCSI_MSK		; command in scsi_send_buf so
		cmp	#PHASE_COMMAND		; that it can be retried!
		bne	.send_byte
		lda	.request_sense, x
		sta	IFU_SCSI_DAT
		jsr	scsi_handshake
		dex
		bpl	.send_byte
		bra	.proc_scsi_loop

		; Read SCSI reply.

.get_response:	ldy	#10			; Get the 10-byte SENSE data.
		jsr	scsi_recv_reply
		bra	.proc_scsi_loop

		; SCSI "Request Sense" command data.

.request_sense:	db	$00
		db	$0A			; # of bytes of SENSE to return.
		db	$00			; Reserved.
		db	$00			; Reserved.
		db	$00			; Return "fixed-format" SENSE data.
		db	CDROM_REQSENSE		; REQUEST SENSE.



; ***************************************************************************
; ***************************************************************************
;
; Read reply data from CD-ROM and put it in scsi_recv_buf.
;
; Y = # bytes in SCSI reply.
;

scsi_recv_reply:clx				; Receive buffer index.

.recv_byte:	lda	IFU_SCSI_FLG
		and	#SCSI_MSK
		cmp	#PHASE_DATA_IN
		bne	.recv_byte

		lda	IFU_SCSI_DAT
		sta	scsi_recv_buf, x
		jsr	scsi_handshake

		inx
		dey
		bne	.recv_byte
.finished:	rts



; ***************************************************************************
; ***************************************************************************
;
; Use return code from scsi_req_sense() to decide what to do.
;

choose_recbase:	cpy	#CDERR_NO_DISC
		beq	.keep_recbase
		cpy	#CDERR_NO_COVER
		beq	.keep_recbase
		cpy	#CDERR_NOT_AUDIO
		beq	.keep_recbase
		cpy	#CDERR_NOT_DATA
		beq	.keep_recbase
		cpy	#CDERR_BAD_CDB
		beq	.keep_recbase
		cpy	#CDERR_BAD_ADDR
		beq	.keep_recbase
		cpy	#CDERR_BAD_PARAM
		beq	.keep_recbase
		cpy	#CDERR_DISC_END
		beq	.keep_recbase
		cpy	#CDERR_BAD_LIST
		beq	.keep_recbase
		cpy	#CDERR_BAD_PLAY
		beq	.keep_recbase

;		cpy	#CDERR_BAD_SENSE	; Redundant code!
;		beq	.swap_recbase
;		cpy	#CDERR_NOT_READY
;		beq	.swap_recbase
;		cpy	#CDERR_BAD_ECC
;		beq	.swap_recbase
;		cpy	#CDERR_BAD_SEEK
;		beq	.swap_recbase
;		cpy	#CDERR_NO_HEADER
;		beq	.swap_recbase

.swap_recbase:	lda	recbase			; Swap to alternate track.
		eor	#1
		sta	recbase

.keep_recbase:	rts



; ***************************************************************************
; ***************************************************************************
;
; cdr_read_ram - with CPU memory as destination (like CD_READ to RAM).
;
; Returns: Y,Z-flag,N-flag = $00 or an error code.
;

cdr_read_ram	.proc

.wait_busy:	lda	bios_cd_mutex		; Wait until any background
		bne	.wait_busy		; BIOS access is finished.

		stz	recbase			; Reset data track to use.

		; Initiate CD-ROM command.

.retry_command:	jsr	scsi_initiate		; Acquire the SCSI bus.

		; Process SCSI phases.

.proc_scsi_loop:jsr	scsi_get_phase
		cmp	#PHASE_COMMAND
		beq	.send_command
		cmp	#PHASE_DATA_IN
		beq	.get_response
		cmp	#PHASE_STAT_IN
		bne	.proc_scsi_loop

		; SCSI command done.

.command_done:	jsr	scsi_get_status		; Returns code in Y & A.
		cmp	#CDSTS_CHECK
		beq	.read_error

		leave				; All Done!

.read_error:	jsr	scsi_req_sense		; Clear error, get code in Y.
		jsr	choose_recbase		; Code selects next recbase.
		bra	.retry_command

		; Send SCSI command.

.send_command:	lda.l	<_bx
		sta.l	<scsi_ram_ptr
		lda.h	<_bx
		sta.h	<scsi_ram_ptr

		jsr	scsi_read_cmd
		bra	.proc_scsi_loop

		; Read SCSI reply.

.get_response:	jsr	scsi_to_ram		; Read a sector's data.
		bra	.proc_scsi_loop

		.endp



; ***************************************************************************
; ***************************************************************************
;
; cdr_read_bnk - with BNK memory as destination (like CD_READ to RAM).
;
; Returns: Y,Z-flag,N-flag = $00 or an error code.
;
; N.B. For HuVIDEO compatibility, the bank is mapped into MPR2 for reading.
;

cdr_read_bnk	.proc

.wait_busy:	lda	bios_cd_mutex		; Wait until any background
		bne	.wait_busy		; BIOS access is finished.

	.if	SUPPORT_TIMING
		stz	irq_cnt			; Track loading speed.
		stz	scsi_stat_indx
	.endif	SUPPORT_TIMING

		stz	recbase			; Reset data track to use.

		tma2				; Preserve current MPR2.
		pha

		; Initiate CD-ROM command.

.retry_command:	jsr	scsi_initiate		; Acquire the SCSI bus.

		; Process SCSI phases.

.proc_scsi_loop:jsr	scsi_get_phase
		cmp	#PHASE_COMMAND
		beq	.send_command
		cmp	#PHASE_DATA_IN
		beq	.get_response
		cmp	#PHASE_STAT_IN
		bne	.proc_scsi_loop

		; SCSI command done.

.command_done:	jsr	scsi_get_status		; Returns code in Y & A.
		cmp	#CDSTS_CHECK
		beq	.read_error

		pla				; Restore original MPR2.
		tam2

		leave				; All Done!

.read_error:	jsr	scsi_req_sense		; Clear error, get code in Y.
		jsr	choose_recbase		; Code selects next recbase.
		bra	.retry_command

		; Send SCSI command.

.send_command:	lda	<_bl
		tam2

		stz.l	<scsi_ram_ptr
		lda	#>$4000
		sta.h	<scsi_ram_ptr

		jsr	scsi_read_cmd
		bra	.proc_scsi_loop

		; Read SCSI reply.

.get_response:	jsr	scsi_to_ram		; Read a sector's data.

		lda.h	<scsi_ram_ptr		; Wrapped bank?
		and	#$1F
		bne	.proc_scsi_loop

		tma2				; Move on to next bank.
		inc	a
		tam2
		lda	#>$4000
		sta.h	<scsi_ram_ptr
		jmp	.proc_scsi_loop

		.endp

;
; Fast-copy 2048 byte sector to RAM.
;
; It takes approx 55,000 cycles to read a sector.
;

scsi_to_ram:	ldx	#$07
		cly
!loop:		lda	IFU_SCSI_AUTO
		sta	[scsi_ram_ptr], y
		nop
		nop
		nop
		iny
		bne	!loop-
		inc.h	<scsi_ram_ptr
		dex
		bne	!loop-

!loop:		lda	IFU_SCSI_AUTO
		sta	[scsi_ram_ptr], y
		nop
		nop
		iny
		cpy	#$F6
		bne	!loop-

		php
		sei
		lda	IFU_SCSI_AUTO
		sta	[scsi_ram_ptr], y
		iny
		nop
		nop
		nop
		nop
		nop
		lda	IFU_SCSI_AUTO
		plp

		sta	[scsi_ram_ptr], y
		iny
		nop
		nop
		nop
		nop

!loop:		lda	IFU_SCSI_AUTO
		sta	[scsi_ram_ptr], y
		nop
		nop
		nop
		iny
		bne	!loop-
		inc.h	<scsi_ram_ptr
		cla
		rts



	.if	SUPPORT_CDVRAM

; ***************************************************************************
; ***************************************************************************
;
; cdr_read_vram - with SGX or VDC VRAM as destination (like CD_READ to VRAM).
;
; Returns: Y,Z-flag,N-flag = $00 or an error code.
;

cdr_read_vram	.proc

.wait_busy:	lda	bios_cd_mutex		; Wait until any background
		bne	.wait_busy		; BIOS access is finished.

		stz	recbase			; Reset data track to use.

		; Initiate CD-ROM command.

.retry_command:	jsr	scsi_initiate		; Acquire the SCSI bus.

		; Process SCSI phases.

.proc_scsi_loop:jsr	scsi_get_phase
		cmp	#PHASE_COMMAND
		beq	.send_command
		cmp	#PHASE_DATA_IN
		beq	.get_response
		cmp	#PHASE_STAT_IN
		bne	.proc_scsi_loop

		; SCSI command done.

.command_done:	jsr	scsi_get_status		; Returns code in Y & A.
		cmp	#CDSTS_CHECK
		beq	.read_error

		leave				; All Done!

.read_error:	jsr	scsi_req_sense		; Clear error, get code in Y.
		jsr	choose_recbase		; Code selects next recbase.
		bra	.retry_command

		; Send SCSI command.

.send_command:	ldx.l	<_bx
		ldy.h	<_bx
		jsr	set_vdc_wr_ptr

		jsr	scsi_read_cmd
		bra	.proc_scsi_loop

		; Read SCSI reply.

.get_response:	jsr	scsi_to_vram
		bra	.proc_scsi_loop

		.endp

;
; set_vdc_wr_ptr
;

set_vdc_wr_ptr:	lda	#VDC_MAWR
		sta	<vdc_reg
		sta	VDC_SR
		stx	VDC_DL
		sty	VDC_DH
		lda	#VDC_VWR
		sta	<vdc_reg
		sta	VDC_SR
		rts

;
; Fast-copy 2048 byte internal sector buffer to VDC.
;

scsi_to_vram:	ldx	#$04
		ldy	#$FB
!loop:		lda	IFU_SCSI_AUTO
		sta	VDC_DL
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		lda	IFU_SCSI_AUTO
		sta	VDC_DH
		nop
		nop
		nop
		nop
		dey
		bne	!loop-
		dex
		bne	!loop-

		php
		sei
		lda	IFU_SCSI_AUTO
		sta	VDC_DL
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		lda	IFU_SCSI_AUTO
		plp

		sta	VDC_DH
		nop
		nop
		nop
		nop
		nop

		ldx	#$04
!loop:		lda	IFU_SCSI_AUTO
		sta	VDC_DL
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		lda	IFU_SCSI_AUTO
		sta	VDC_DH
		nop
		nop
		nop
		nop
		dex
		bne	!loop-
		cla
		rts

	.endif	SUPPORT_CDVRAM



	.if	SUPPORT_ACD

; ***************************************************************************
; ***************************************************************************
;
; cdr_read_acd - with ACD memory as destination (like CD_READ to ACD).
;
; Returns: Y,Z-flag,N-flag = $00 or an error code.
;

cdr_read_acd	.proc

.wait_busy:	lda	bios_cd_mutex		; Wait until any background
		bne	.wait_busy		; BIOS access is finished.

		stz	recbase			; Reset data track to use.

		; Initiate CD-ROM command.

.retry_command:	jsr	scsi_initiate		; Acquire the SCSI bus.

		; Process SCSI phases.

.proc_scsi_loop:jsr	scsi_get_phase
		cmp	#PHASE_COMMAND
		beq	.send_command
		cmp	#PHASE_DATA_IN
		beq	.get_response
		cmp	#PHASE_STAT_IN
		bne	.proc_scsi_loop

		; SCSI command done.

.command_done:	jsr	scsi_get_status		; Returns code in Y & A.
		cmp	#CDSTS_CHECK
		beq	.read_error

		leave				; All Done!

.read_error:	jsr	scsi_req_sense		; Clear error, get code in Y.
		jsr	choose_recbase		; Code selects next recbase.
		bra	.retry_command

		; Send SCSI command.

.send_command:	ldx	<_ah			; output_l
		ldy	<_bl			; output_m
		lda	<_bh			; output_h
		stx	ACD0_BASE + 0
		sty	ACD0_BASE + 1
		sta	ACD0_BASE + 2

		lda	#1
		sta.l	ACD0_INCR
		stz.h	ACD0_INCR
		lda	#$11			; BASE, with INCR of BASE.
		sta	ACD0_CTRL

		jsr	scsi_read_cmd
		bra	.proc_scsi_loop

		; Read SCSI reply.

.get_response:	jsr	scsi_to_acd
		bra	.proc_scsi_loop

		.endp

;
; Fast-copy 2048 byte sector to ACD.
;

scsi_to_acd:	ldy	#$F6
		ldx	#$08
!loop:		lda	IFU_SCSI_AUTO
		sta	ACD0_DATA
		nop
		nop
		nop
		nop
		dey
		bne	!loop-
		dex
		bne	!loop-

		php
		sei
		lda	IFU_SCSI_AUTO
		sta	ACD0_DATA
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		lda	IFU_SCSI_AUTO
		plp

		sta	ACD0_DATA
		nop
		nop
		nop
		nop
		nop

		ldx	#$08
!loop:		lda	IFU_SCSI_AUTO
		sta	ACD0_DATA
		nop
		nop
		nop
		nop
		dex
		bne	!loop-
		cla
		rts

	.endif	SUPPORT_ACD



	.if	SUPPORT_ADPCM

; ***************************************************************************
; ***************************************************************************
;
; cdr_ad_trans - transfer data from CD to ADPCM buffer (BIOS AD_TRANS).
;
; _cl : rec h
; _ch : rec m
; _dl : rec l
;
; _al : # sectors
;
; _dh : transfer mode (ignored, always use address in _bx)
;
; _bx : ADPCM buffer address
;

cdr_ad_trans	.proc

.wait_busy:	lda	bios_cd_mutex		; Wait until any background
		bne	.wait_busy		; BIOS access is finished.

		stz	recbase			; Reset data track to use.

		; Initiate CD-ROM command.

.retry_command:	jsr	scsi_initiate		; Acquire the SCSI bus.

		; Process SCSI phases.

.proc_scsi_loop:jsr	scsi_get_phase
		cmp	#PHASE_COMMAND
		beq	.send_command
		cmp	#PHASE_DATA_IN
		beq	.get_response
		cmp	#PHASE_STAT_IN
		bne	.proc_scsi_loop

		; SCSI command done.

.command_done:	jsr	scsi_get_status		; Returns code in Y & A.
		cmp	#CDSTS_CHECK
		beq	.read_error

		tay				; Set return-code.

		leave				; All Done!

.read_error:	jsr	scsi_req_sense		; Clear error, get code in Y.
		jsr	choose_recbase		; Code selects next recbase.
		bra	.retry_command

		; Send SCSI command.

.send_command:	ldx.l	<_bx			; Set the ADPCM RAM desination address.
		ldy.h	<_bx
		jsr	adpcm_set_dst

		jsr	scsi_read_cmd
		bra	.proc_scsi_loop

		; Read SCSI reply.

.get_response:	lda	#$01			; Start ADPCM DMA from CD.
		sta	IFU_ADPCM_DMA

.wait:		tst	#$01, IFU_ADPCM_DMA	; Wait for the DMA to finish.
		bne	.wait
		bra	.proc_scsi_loop

		.endp



; ***************************************************************************
; ***************************************************************************
;
; cdr_ad_cplay - start ADPCM streaming playback from CD (BIOS AD_CPLAY).
;
; _cl : rec h
; _ch : rec m
; _dl : rec l
;
; _al : # sectors l
; _ah : # sectors m
; _bl : # sectors h
;
; _dh : sample rate
;

cdr_ad_cplay	.proc

		tii	_al, cplay_len_l, 3	; Save # sectors left to load.

		lda	<_dh			; Set playback rate.
		sta	IFU_ADPCM_SPD

		lda	#(65536 / 2048)		; 32 * 2KB sectors.
		sta	<_al
		stz	<_bl			; Set ADPCM address to $0000.
		stz	<_bh
		call	cdr_ad_trans		; Load the 1st 64KB of ADPCM.
		bne	.finished

	.if	(cplay_scsi_buf != scsi_send_buf)
		tii	scsi_send_buf, cplay_scsi_buf, 6
	.endif

		jsr	cdr_incdec_len		; Update cplay len and lba.

	.ifdef	CORE_VERSION
		setvec	irq2_hook, cdr_cplay_irq2
	.else
		ldx.l	#cdr_cplay_irq2		; Set IRQ2 vector.
		ldy.h	#cdr_cplay_irq2
		cla
		jsr	ex_setvec
	.endif

		smb0	<irq_vec		; Enable IRQ2 vector.

		cly				; Start playback from $0000.
		clx
		jsr	adpcm_set_src

		ldx	#<$FFFF			; Set 64KB playback length.
		ldy	#>$FFFF			; This clears IFU_INT_END,
		jsr	adpcm_set_len		; but not IFU_INT_HALF.

		lda	#ADPCM_AUTO
		tsb	IFU_ADPCM_CTL		; N.B. BIOS uses a STA!

		lda	#IFU_INT_HALF + IFU_INT_END
		tsb	IFU_IRQ_MSK

		lda	#ADPCM_PLAY
		tsb	IFU_ADPCM_CTL		; N.B. BIOS uses a STA!

		stz	cplay_filling		; Signal ADPCM not loading.
		lda	#1			; Signal ADPCM active.
		sta	cplay_playing

		cly				; Return code.

.finished:	leave				; All Done!

		.endp

		;

cdr_incdec_len:	lda.l	cplay_len_l		; Decrement the ADPCM length.
		sec
		sbc	cplay_scsi_buf + 4	; len_l
		sta.l	cplay_len_l
		bcs	!+
		dec.h	cplay_len_l

!:		clc				; Increment SCSI LBA address.
		lda	cplay_scsi_buf + 4	; len_l
		adc	cplay_scsi_buf + 3	; lba_l
		sta	cplay_scsi_buf + 3
		cla
		adc	cplay_scsi_buf + 2	; lba_m
		sta	cplay_scsi_buf + 2
		cla
		adc	cplay_scsi_buf + 1	; lba_h
		sta	cplay_scsi_buf + 1
		rts



; ***************************************************************************
; ***************************************************************************
;
; cdr_cplay_next - Load the next 32KB of streaming ADPCM data.
;
; This is called by the IRQ2 handler.
;

cdr_cplay_next	.proc

		jsr	scsi_select_cd		; Acquire the SCSI bus.
		bcs	.finished		; Signal that we've failed!

		ldy	#19			; Wait for up to 20ms for the
		clx				; CD-ROM to acknowledge.
.wait_scsi_bus:	jsr	scsi_get_phase
		and	#SCSI_BSY
		bne	.proc_scsi_loop
		dex
		bne	.wait_scsi_bus
		dey
		bne	.wait_scsi_bus

		; Timeout, retry.

		bra	cdr_cplay_next

		; Process SCSI phases.

.proc_scsi_loop:lda.h	cplay_len_l		; Is there any more ADPCM data?
		bne	.load_32kb

		lda.l	cplay_len_l
		cmp	#(32768 / 2048)
		bcc	.load_from_cd		; Load the last few sectors.
.load_32kb:	lda	#(32768 / 2048)		; Load the next 32KB of data.

.load_from_cd:	sta	cplay_scsi_buf + 4	; len_l

	.if	(cplay_scsi_buf == scsi_send_buf)
		ldy	#6			; Send SCSI command stored in
		jsr	scsi_send_cmd		; cplay_scsi_buf to the drive.
	.else
		ldx	#1			; Send SCSI command stored in
.send_byte:	lda	IFU_SCSI_FLG		; cplay_scsi_buf to the drive.
		and	#SCSI_MSK
		cmp	#PHASE_COMMAND
		bne	.send_byte
		lda	cplay_scsi_buf, x
		sta	IFU_SCSI_DAT
		jsr	scsi_handshake
		inx
		cpx	#7
		bne	.send_byte
	.endif	(cplay_scsi_buf == scsi_send_buf)

		lda	#1			; Signal ADPCM loading now.
		sta	cplay_filling

		jsr	cdr_incdec_len		; Update cplay len and lba.

		clc				; Signal that we're OK.

.finished:	leave				; All Done!

		.endp



; ***************************************************************************
; ***************************************************************************
;
; cdr_ad_stop - stop streaming ADPCM playback (BIOS AD_STOP).
;

cdr_ad_stop	.proc

		stz.h	cplay_len_l		; Stop background loading.
		stz.l	cplay_len_l

.wait:		lda	cplay_filling		; Wait for the current load
		bne	.wait			; to finished.

		lda	#IFU_INT_HALF + IFU_INT_END
		trb	IFU_IRQ_MSK

		lda	#ADPCM_PLAY + ADPCM_AUTO
		trb	IFU_ADPCM_CTL

		rmb0	<irq_vec		; Disable IRQ2 vector.

		stz	cplay_playing		; Signal ADPCM finished.

		leave				; All Done!

		.endp

	.endif	SUPPORT_ADPCM



	.if	SUPPORT_HUVIDEO

; ***************************************************************************
; ***************************************************************************
;
; cdr_stream_ram - with CPU memory as destination (like CD_READ to BNK).
;
; Returns: Y,Z-flag,N-flag = $00 or an error code.
;

	.ifndef	HUV_RETRY_ERROR
HUV_RETRY_ERROR	=	1
	.endif

cdr_stream_ram:	.proc

	.if	HUV_RETRY_ERROR
		lda	<huv_load_pages		; Preserve destination frame
		sta	<_ah			; information for a retry.
		lda.l	<huv_load_frame
		sta.l	<_bx
		lda.h	<huv_load_frame
		sta.h	<_bx
	.endif	HUV_RETRY_ERROR

	.if	SUPPORT_TIMING
		stz	irq_cnt			; Track loading speed.
		stz	scsi_stat_indx
	.endif	SUPPORT_TIMING

		stz	recbase			; Reset data track to use.

		; Initiate CD-ROM command.

.retry_command:	jsr	scsi_initiate		; Acquire the SCSI bus.

		; Process SCSI phases.

.proc_scsi_loop:jsr	scsi_get_phase
		cmp	#PHASE_COMMAND
		beq	.send_command
		cmp	#PHASE_DATA_IN
		beq	.get_response
		cmp	#PHASE_STAT_IN
		bne	.proc_scsi_loop

		; SCSI command done.

.command_done:	jsr	scsi_get_status		; Returns code in Y & A.
		cmp	#CDSTS_CHECK
		bne	.finished

.read_error:	jsr	scsi_req_sense		; Clear error, get code in Y.
;		jsr	choose_recbase		; VIDEO IS ONLY IN 1ST TRACK!

	.if	HUV_RETRY_ERROR
		lda	<_ah			; Restore destination frame
		sta	<huv_load_pages		; information for a retry.
		php
		sei
		lda.l	<_bx
		sta.l	<huv_load_frame
		lda.h	<_bx
		sta.h	<huv_load_frame
		plp
		bra	.retry_command
	.endif	HUV_RETRY_ERROR

.finished:	leave				; All Done!

		; Send SCSI command.

.send_command:	lda	#BUFFER_1ST_BANK
		tam2
		stz.l	<scsi_ram_ptr
		lda	#>$4000
		sta.h	<scsi_ram_ptr

		jsr	scsi_read_cmd
		bra	.proc_scsi_loop

		; Read SCSI reply.

	.if	SUPPORT_ACD
.get_response:	jsr	scsi_to_acd		; Read a sector's data.
	.else
.get_response:	jsr	scsi_to_ram		; Read a sector's data.
	.endif

	.if	SUPPORT_TIMING
		ldx	scsi_stat_indx		; Track loading speed.
		lda	irq_cnt
		sta	scsi_stat_time, x
		inx
		stx	scsi_stat_indx
	.endif	SUPPORT_TIMING

		lda.h	<scsi_ram_ptr		; Keep the destination addr
		and	#$1F			; within MPR2.
		bne	!+
		tma2
		inc	a
		tam2
		lda	#>$4000
		sta.h	<scsi_ram_ptr

!:		clc				; Delay acknowledging the new
		lda.h	#$0800			; frame by a sector or two to
		adc	<huv_load_pages		; make sure it loaded OK.
		sta	<huv_load_pages
		cmp.h	#BYTES_PER_FRAME + ($0800 * 2)
		bcc	.proc_scsi_loop

		php				; Disable interrupts while
		sei				; changing huv_load_frame!
		sbc.h	#BYTES_PER_FRAME
		inc.l	<huv_load_frame
		bne	!+
		inc.h	<huv_load_frame
!:		sta	<huv_load_pages
		plp
		bra	.proc_scsi_loop

		.endp

	.endif	SUPPORT_HUVIDEO



; ***************************************************************************
; ***************************************************************************
; ***************************************************************************
;
; Routines only needed on a standalone HuCARD.
;
; ***************************************************************************
; ***************************************************************************
; ***************************************************************************

	.if	SUPPORT_CDINIT



; ***************************************************************************
; ***************************************************************************
;
; cdr_init_disc - Initialize the CD drive and scan the CD TOC.
;
; Returns: Y,Z-flag,N-flag = $00 or an error code.
;

cdr_init_disc	.proc

;		; Check that the CD-ROM IFU is present.
;		;
;		; N.B. The System Card does not check this!
;
;		ldy	#CDERR_NO_CDIFU
;		lda	IO_PORT
;		bmi	.finished

		; Reset the CD-ROM drive.

		call	cdr_reset

		; Wait for the CD-ROM drive to release SCSI_BSY.

		stz.l	<_ax			; Retry 1024 times, which is
		lda.h	#1024			; *far* more than needed, if
		sta.h	<_ax			; the drive is attached.

.wait_busy:	ldy	#CDERR_NO_DRIVE
		lda.l	<_ax
		bne	!+
		dec.h	<_ax
		bmi	.finished		; Timeout!
!:		dec.l	<_ax

		cly				; Is the drive busy?
		call	cdr_stat
		bne	.wait_busy

		; Wait for the CD-ROM to spin-up and become ready.

.wait_ready:	ldy	#1			; Is the drive ready? Calling
		call	cdr_stat		; this is what actually makes
		beq	.drive_ready		; the drive start spinning up
		cmp	#CDERR_NOT_READY	; after reset.
		bne	.finished

		ldy	#60			; Wait a second and try again.
		jsr	wait_nvsync
		bra	.wait_ready

		; Get the CD-ROM disc's Table Of Contents info.

.drive_ready:	call	cdr_contents		; Find the CD-ROM data track.
		bne	.finished

.finished:	leave				; Return-code already in Y.

		.endp



; ***************************************************************************
; ***************************************************************************
;
; cdr_reset - Reset CD drive and stop the disc spinning (BIOS CD_RESET).
;
; Returns: nothing
;

cdr_reset:	.proc

		stz	IFU_IRQ_MSK		; Disable IFU interrupts.
		stz	IFU_AUDIO_FADE		; Disable CD fade.

		lda	#$02			; Assert reset signal.
		tsb	IFU_HW_RESET

		ldy	#1			; Wait 500us.
		jsr	scsi_delay

		trb	IFU_HW_RESET		; Release reset signal.

		lda	#119			; Wait 100us.
.wait_100us:	dec	a
		bne	.wait_100us

		tst	#IFU_INT_SUBC, IFU_IRQ_FLG
		beq	.finished

		bit	IFU_SUBCODE		; Flush subcode interrupt.

.finished:	leave

		.endp



; ***************************************************************************
; ***************************************************************************
;
; cdr_stat - Check CD drive status (BIOS CD_STAT).
;
; Returns: Y,Z-flag,N-flag = $00 or an error code.
;

cdr_stat	.proc

		cpy	#0			; Check for BUSY or READY?
		bne	.check_ready

		lda	IFU_SCSI_FLG		; $80 if drive BUSY, or $00.
		and	#SCSI_BSY
		tay
		bra	.finished

		; Initiate CD-ROM command.

.check_ready:	jsr	scsi_initiate		; Acquire the SCSI bus.

		; Process SCSI phases.

.proc_scsi_loop:jsr	scsi_get_phase
		cmp	#PHASE_COMMAND
		beq	.send_command
		cmp	#PHASE_STAT_IN
		bne	.proc_scsi_loop

		; SCSI command done.

.command_done:	jsr	scsi_get_status		; Returns code in Y & A.
;		cmp	#CDSTS_GOOD
		beq	.finished

		jsr	scsi_req_sense		; Clear error, get code in Y.

.finished:	leave				; All Done!

		; Send SCSI command.

.send_command:	lda	#CDROM_TESTRDY
		jsr	scsi_init_cmd

		ldy	#6
		jsr	scsi_send_cmd
		bra	.proc_scsi_loop

		.endp



; ***************************************************************************
; ***************************************************************************
;
; cdr_contents - Scan the disc's TOC to find the layout (BIOS CD_CONTENTS).
;
; Returns: Y,Z-flag,N-flag = $00 or an error code.
;

cdr_contents	.proc

		lda	#$FF			; Init no CD-ROM tack found.
		sta	cdrom_track

		; Read tnomin, tnomax (BCD values).

		stz	<_al			; dinfo type = 0
		stz	<_ah			; dinfo track#
		call	cdr_dinfo
;		cmp	#CDSTS_GOOD
		bne	.abort

		tii	scsi_recv_buf, tnomin, 2

		; Read outmin, outsec, outfrm (BCD values).

		inc	<_al			; dinfo type = 1
		call	cdr_dinfo
;		cmp	#CDSTS_GOOD
		bne	.abort

		tii	scsi_recv_buf, outmin, 3

		; Scan for the first and last CD-ROM data tracks.

		inc	<_al			; dinfo type = 2
		lda	tnomin

.find_cdrom:	sta	<_ah			; dinfo track#
		call	cdr_dinfo
;		cmp	#CDSTS_GOOD		; N.B. track# may not exist!
		bne	.next_track

		tst	#$04, scsi_recv_buf + 3	; Check the track's subcode
		beq	.next_track		; to see if it is data.

.found_cdrom:	call	.msf_to_lba		; Convert track's MSF to LBA.

		lda	cdrom_track		; Already found 1st track?
		bpl	.2nd_iso
.1st_iso:	lda	<_ah			; Signal CD-ROM tack found!
		sta	cdrom_track
		tii	.lba, recbase0_h, 3	; Save recbase0 LBA.
.2nd_iso:	tii	.lba, recbase1_h, 3	; Save recbase1 LBA.

.next_track:	lda	<_ah			; Have we check all tracks?
		cmp	tnomax
		bcs	.finished

		clc				; BCD increment the track
		sed				; number.
		adc	#1
		cld
		bra	.find_cdrom

.finished:	cla				; Return success code.

.abort:		tay				; Preserve return code.

		leave				; All Done!

		; Convert the TOC's MSF into an LBA.

.lba		=	_bx + 0			; 5-byte LBA workspace.
.temp		=	_bx + 5			; 1-byte temporary.

.msf_to_lba:	ldx	#3 - 1			; Convert the TOC's BCD byte
.bcd_to_bin:	lda	scsi_recv_buf, x	; values (0..99) to binary.
		and	#$F0
		lsr	a
		sta	<.temp
		lsr	a
		lsr	a
		adc	<.temp
		sta	<.temp
		lda	scsi_recv_buf, x
		and	#$0F
		adc	<.temp
		sta	<.lba + 2, x
		dex
		bpl	.bcd_to_bin

;		sta	<.lba + 2		; Minutes (0..79)
		stz	<.lba + 1
		stz	<.lba + 0
		lda	#60			; Multiplied by 60
;		ldx	<.lba + 3		; Add seconds (0..59)
;		stx	<.lba + 3
		bsr	.mac8x24

		sta	<.lba + 2		; Save the result.
		stx	<.lba + 1
		sty	<.lba + 0

		lda	#75			; Multiplied by 75
		ldx	<.lba + 4		; Add frames  (0..74)
		stx	<.lba + 3
		bsr	.mac8x24

		sec				; Subtract 2 seconds from the
		sbc	#2 * 75			; LBA because the TOC assumes
		sta	<.lba + 2		; 2 seconds of silence.
		txa
		sbc	#0
		sta	<.lba + 1
		tya
		sbc	#0
		sta	<.lba + 0

		rts

		; Multiply and accumulate a value.

.mac8x24:	sta	<.temp			; Preserve multiplier.
		cla				; Clear Result.
		clx
		cly
		lsr	<.temp			; Shift and test multiplier.
		bcc	.loop

.add:		clc				; Add .lba to the result.
		adc	<.lba + 2
		sax
		adc	<.lba + 1
		sax
		say
		adc	<.lba + 0
		say

.loop:		asl	<.lba + 2		; .lba = .lba * 2
		rol	<.lba + 1
		rol	<.lba + 0
		lsr	<.temp			; Shift and test multiplier.
		bcs	.add
		bne	.loop

.accumulate:	adc	<.lba + 3		; Add a byte value to the
		bcc	.mac8x24_done		; result.
		inx
		bne	.mac8x24_done
		iny

.mac8x24_done:	rts

		.endp



; ***************************************************************************
; ***************************************************************************
;
; cdr_dinfo - Read the disc's TOC data (Table Of Contents) (BIOS CD_DINFO).
;
; _al = 0 returns the track numbers
;
;  scsi_recv_buf + 0: Min track (BCD)
;  scsi_recv_buf + 1: Max track (BCD)
;  scsi_recv_buf + 2: 0
;  scsi_recv_buf + 3: 0
;
; _al = 1 returns the end-of-disc information
;
;  scsi_recv_buf + 0: Readout Minutes (BCD)
;  scsi_recv_buf + 1: Readout Seconds (BCD)
;  scsi_recv_buf + 2: Readout Frame   (BCD)
;  scsi_recv_buf + 3: 0
;
; _al = 2 returns the information for track# _ah in MSF format
;
;  scsi_recv_buf + 0: Track Minutes (BCD)
;  scsi_recv_buf + 1: Track Seconds (BCD)
;  scsi_recv_buf + 2: Track Frame   (BCD)
;  scsi_recv_buf + 3: Track SubQ Flags
;
; _al = 3 is documented, but returns unusable garbage instead of an LBA.
;
; Returns: Y,Z-flag,N-flag = $00 or an error code.
;

cdr_dinfo	.proc

		; Initiate CD-ROM command.

.retry_command:	jsr	scsi_initiate		; Acquire the SCSI bus.

		; Process SCSI phases.

.proc_scsi_loop:jsr	scsi_get_phase
		cmp	#PHASE_COMMAND
		beq	.send_command
		cmp	#PHASE_DATA_IN
		beq	.get_response
		cmp	#PHASE_STAT_IN
		bne	.proc_scsi_loop

		; SCSI command done.

.command_done:	jsr	scsi_get_status		; Returns code in Y & A.
;		cmp	#CDSTS_GOOD
		beq	.finished

		jsr	scsi_req_sense		; Clear error, get code in Y.

.finished:	leave				; All Done!

		; Send SCSI command.

.send_command:	lda	#CDROM_READTOC
		jsr	scsi_init_cmd

		lda	<_al			; Set TOC type.
		and	#3
		sta	scsi_send_buf + 1
		lda	<_ah			; Set TOC track.
		sta	scsi_send_buf + 2

		ldy	#10
		jsr	scsi_send_cmd
		bra	.proc_scsi_loop

		; Read SCSI reply.

.get_response:	ldy	#4			; Read 4-bytes of TOC info.
		jsr	scsi_recv_reply
		bra	.proc_scsi_loop

		.endp

	.endif	SUPPORT_CDINIT



; ***************************************************************************
; ***************************************************************************
; ***************************************************************************
;
; End of routines only needed on a standalone HuCARD.
;
; ***************************************************************************
; ***************************************************************************
; ***************************************************************************

		.endprocgroup
