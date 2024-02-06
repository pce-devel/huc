; ***************************************************************************
; ***************************************************************************
;
; sherlock-hack.asm
;
; This patches the "Sherlock Holmes Consulting Detective" executable code to
; add a 1ms timer and a lot of event-logging to unused SuperCD memory banks.
;
; A copy of Sherlock's executable code must exist in this directory with the
; filename "sherlock-boot.bin". (Size = 22,528 bytes with CRC32 = $35AE4D73)
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
; If you are using an emulator, then you can run the game normally but put a
; breakpoint at address $2A00.
;
; When the breakpoint is hit, just load "sherlock-hack.ovl" at $2A00 and run
; the code.
;
; Sherlock's introductory movie should immediately play, with a log of event
; information being written to banks $6A..$81.
;
; When the movie has completed, the game will hang on the next screen, which
; allows you to enter your emulator's debugger and save the contents of bank
; $6A..$81 ($0D4000..$103FFF physical) as the SHERLOCK.DAT file.
;
; Then run the "dat2csv" program in the same directory as the SHERLOCK.DAT
; file and it will output the human-readable SHERLOCK.CSV spreadsheet file.
;
; ***************************************************************************
; ***************************************************************************



; ***************************************************************************
; ***************************************************************************
;
; Use the original Sherlock Holmes boot program as a base for testing.
;

		include	"pcengine.inc"

		.bank	0			; Bank $F8 at $2000-$3FFF.
		.page	1

		.bank	1			; Bank $82 at $4000-$5FFF.
		.page	2

		.bank	2			; Bank $83 at $6000-$7FFF.
		.page	3

		.bank	3			; Bank $84 at $8000-$9FFF.
		.page	4

		.org	$8000			; Sherlock loads $2A00-$81FF.
		ds	$0A00			; Clear space to $89FF.

		.bank	0
		.org	$2A00
        	incbin	"sherlock-boot.bin"	; Load 11 sector boot code.

		;
		; Sherlock's internal playback variables.
		;

which_screen	=	$F8:2008		; $00=VRAM $0800, $FF=VRAM $1CA0
sector_counter	=	$F8:2009		; #vram sectors left to read
sectors_remain	=	$F8:200A		; #sectors left to read in SCSI read command, send new SCSI when 0.
		;
video_wrong	=	$F8:200D		; cumulative #ticks playback is out
frame_wrong	=	$F8:200E		; #ticks taken this frame - 6 (nominal playback #ticks taken)
delay_ticks	=	$F8:200F		; #ticks to delay (if video is running too fast)

tick_count	=	$F8:2026		; reset at the start of every video frame

frame_num_lo	=	$F8:2047		; current movie frame number
frame_num_hi	=	$F8:2048		;

movie_lba_l	=	$F8:26CA
movie_lba_m	=	$F8:26CB
movie_lba_h	=	$F8:26CC

		;
		; Use previously-unused locations in Sherlock for instrumentation.
		;

timer_val	=	$F8:20C0
info_addr	=	$F8:20C1

		;
		;
		;

INFO_VBLANK	=	1

INFO_WAITVBL	=	2

INFO_SCSIRMV	=	3
INFO_SCSIIDLE	=	4
INFO_SCSISEND	=	5

INFO_DATAEND	=	6
INFO_FILLEND	=	7

INFO_SEEK_BEGIN	=	10
INFO_SEEK_ENDED	=	11

INFO_NOTBUSY	=	12

INFO_SCSI80	=	$80	; PHASE_DATA_OUT (CD-ROM uses this after a command is received)
INFO_SCSI88	=	$88	; PHASE_DATA_IN
INFO_SCSI98	=	$98	; PHASE_STAT_IN

;PHASE_COMMAND	=	$D0	; (SCSI_BSY + SCSI_REQ + ........ + SCSI_CXD + ........)
;PHASE_DATA_OUT	=	$C0	; (SCSI_BSY + SCSI_REQ + ........ + ........ + ........)
;PHASE_DATA_IN	=	$C8	; (SCSI_BSY + SCSI_REQ + ........ + ........ + SCSI_IXO)
;PHASE_STAT_IN	=	$D8	; (SCSI_BSY + SCSI_REQ + ........ + SCSI_CXD + SCSI_IXO)
;PHASE_MESG_OUT	=	$F0	; (SCSI_BSY + SCSI_REQ + SCSI_MSG + SCSI_CXD + ........)
;PHASE_MESG_IN	=	$F8	; (SCSI_BSY + SCSI_REQ + SCSI_MSG + SCSI_CXD + SCSI_IXO)

;REQ_DATA_OUT	=	($C0+1)	; (SCSI_BSY + SCSI_REQ + ........ + ........ + ........)
;REQ_DATA_IN	=	($C8+1)	; (SCSI_BSY + SCSI_REQ + ........ + ........ + SCSI_IXO)
;REQ_STAT_IN	=	($D8+1)	; (SCSI_BSY + SCSI_REQ + ........ + SCSI_CXD + SCSI_IXO)

;RMV_COMMAND	=	($D0+2)	; (SCSI_BSY + SCSI_REQ + ........ + SCSI_CXD + ........)
;RMV_DATA_OUT	=	($C0+2)	; (SCSI_BSY + SCSI_REQ + ........ + ........ + ........)
;RMV_DATA_IN	=	($C8+2)	; (SCSI_BSY + SCSI_REQ + ........ + ........ + SCSI_IXO)
;RMV_STAT_IN	=	($D8+2)	; (SCSI_BSY + SCSI_REQ + ........ + SCSI_CXD + SCSI_IXO)
;RMV_MESG_OUT	=	($F0+2)	; (SCSI_BSY + SCSI_REQ + SCSI_MSG + SCSI_CXD + ........)
;RMV_MESG_IN	=	($F8+2)	; (SCSI_BSY + SCSI_REQ + SCSI_MSG + SCSI_CXD + SCSI_IXO)

INFO_EXIT	=	$7F

		;
		; Hack Sherlock's code to add the instrumentation.
		;

		.list
		.mlist

		.bank	0

		.org	$2A29			; Disable System Card joypad.
		ora	#$30

		.org	$2A54			; Replace joypad read with tracking.
		jsr	track_vbl

		.org	$2B8B			; Start intro video and recording
		jmp	hijack_start		; when the game code executes.

		.bank	2

		.org	$7A10			; Stop recording when the video ends.
		jmp	hijack_end

		;
		; play_movie() routine, after movie has completed
		;

		.bank	0
		.org	$328D
		jsr	begin_seek

		.org	$3290
		jsr	ended_seek

		;
		; finish_movie()
		;

		.bank	0
		.org	$31CE
		jsr	start_recording

		;
		; scsi_send_cmd() and scsi_next_cmd()
		;

		.bank	0
		.org	$3ECE
		jsr	delay_next_read

		.org	$3EF6
		jsr	scsi_flush_byte

		.org	$3F07
		jsr	scsi_is_idle

		.org	$3F1D
		jsr	scsi_cmd_out

		.org	$3F4D
		jsr	scsi_cmd_end

		.org	$3F6F
		jsr	scsi_got_req

		;
		; play_movie_loop()
		;

		.bank	0
		.org	$3FCE
		jsr	end_then_wait

		.org	$3FF7
		jsr	end_then_wait

		.bank	1
		.org	$40C9
		jsr	end_of_frame

		.org	$41BF
		jsr	end_then_wait

		.org	$42A2
		jmp	scsi_got_phase

		;
		; prefill_adpcm()
		;

		.bank	1
		.org	$43F9
		jsr	end_then_wait

		.org	$43FE
		jmp	prefill_end

		;
		; There is LOTS of unused space at the end of the Sherlock code.
		;

		.bank	2
		.org	$7C80

		;
		; 1ms timer interrupt.
		;

timer_irq:	stz	IRQ_ACK			; Clear timer interrupt.
		inc	<timer_val
		rti

		;
		;
		;

hijack_start:	lda	#$6A			; Initialize the bank and
		tam6				; location for the recording.
		stz.l	<info_addr
		stz.h	<info_addr		; Disable recording (for now).

		lda.l	#timer_irq		; Initialize the 1ms timer.
		sta.l	timer_hook
		lda.h	#timer_irq
		sta.h	timer_hook
		smb2	<irq_vec
		stz	IRQ_ACK
		stz	TIMER_CR
		lda	#7-1			; 7 -> 7144 cycles -> 1.00124457ms
		sta	TIMER_DR

		jmp	$2DA0			; Play intro video at boot.

		;
		;
		;

start_recording:stz	irq_cnt			; Reset IRQ count.
		lda.h	#$C000			; Start recording now.
		sta.h	<info_addr
		lda	#1			; Start timer now.
		sta	TIMER_CR

		jmp	$4415

		;
		;
		;

hijack_end:	lda	#INFO_EXIT		; Mark the end of the movie.
		jsr	track_event

		stz.h	<info_addr		; Disable recording.

		lda	#$68			; Is there a SuperCD program
		tam6				; loaded to execute now?
		lda	$C000
		cmp	#$4C			; Look for a JMP instruction.

!:		bne	!-			; Just hang if nothing to run.

goto_ted2:	sei
		ldx	#$FF
		txs
		stz	TIMER_CR
		stz	IRQ_ACK
		rmb2	<irq_vec

		tii	ram_goto_ted2, $2100, len_goto_ted2
		jmp	$2100

ram_goto_ted2:	lda	#$68
		tam2
		tam7
		inc	a
		tam3
		inc	a
		tam4
		inc	a
		tam5
		inc	a
		tam6
		jmp	[$FFFC]			; Execute NMI vector.

len_goto_ted2	=	* - ram_goto_ted2

		;
		; Log interesting events.
		;

		;	********

delay_next_read:lda	#INFO_WAITVBL
		jsr	track_event
		jmp	ex_vsync

		;	********

scsi_got_phase:	sta	$26C1
		jmp	track_event

		;	********

end_of_frame:	jsr	$436E			; read_adpcm
		lda	#INFO_DATAEND
		jmp	track_event

		;	********

prefill_end:	jsr	end_data_phase
		lda	#INFO_FILLEND
		jmp	track_event

		;	********

end_data_phase:	lda	#INFO_DATAEND
		jmp	track_event

		;	********

end_then_wait:	jsr	end_data_phase
		jmp	$428B

		;	********

scsi_flush_byte:lda	IFU_SCSI_FLG
		and	#$F8
		inc	a
		inc	a
		jsr	track_event
		jmp	$4401

		;	********

scsi_is_idle:	sta	IFU_SCSI_CTL
		lda	#INFO_SCSIIDLE
		jmp	track_event

		;	********

scsi_cmd_out:	stz	$26C1
		lda	#INFO_SCSISEND
		jmp	track_event

		;	********

scsi_cmd_end:	jsr	track_event
		lda	$26BF
		rts

		;	********

scsi_got_req:	sta	$26C1
		bit	#$40
		beq	!+
		pha
		ora	#1
		jsr	track_event
		pla
!:		rts

		;	********

begin_seek:	jsr	scsi_initiate		; Added to see how CD behaves!

		lda	#INFO_SEEK_BEGIN
		jsr	track_event
		jmp	$E00C

		;	********

ended_seek:	lda	#INFO_SEEK_ENDED
		jsr	track_event
		jmp	$33f8

		;	********

		;
		;
		;

track_vbl:	lda	#INFO_VBLANK		; INFO+0

track_event:	php
		sei
		pha
		phy

		ldy.h	<info_addr		; Are we recording?
		beq	done_info
		sta	[info_addr]

		ldy	#1
		lda.l	<timer_val		; INFO+1
		sta	[info_addr], y
		iny
		lda	irq_cnt			; INFO+2
		sta	[info_addr], y
		iny
		lda	<frame_num_lo		; INFO+3
		sta	[info_addr], y
		iny
		lda	<tick_count		; INFO+4
		sta	[info_addr], y
		iny
		lda	<sectors_remain		; INFO+5
		sta	[info_addr], y
		iny
		lda	<video_wrong		; INFO+6
		sta	[info_addr], y
		iny
		lda	<delay_ticks		; INFO+7
		sta	[info_addr], y

next_info:	clc				; Next info slot.
		lda.l	<info_addr
		adc	#8
		sta.l	<info_addr
		bcc	done_info

		inc	<info_addr + 1		; Reached $E000?
		bbr5	<info_addr + 1, done_info

		rmb5	<info_addr + 1		; Reset to $C000.
		tma6
		inc	a
		tam6
		cmp	#$82
		bcc	done_info		; Bank $82?
		stz.h	<info_addr		; Stop recording!

done_info:	ply
		pla
		plp
		rts		

		;		
		;		
		;		

	.if	1

		;
		; This is the SCSI initialization code from cdrom.asm ripped
		; and included here so that we can see just how the PCE's CD
		; responds to an abort while in the middle of a read command.
		;
		; This is called when Sherlock has finished playing the intro
		; movie and just before it calls the System Card's cd_seek to
		; abort the movie's final cd_read command.
		;

IFU_SCSI_CTL	= $FF:1800	; _W : SCSI control signals out.
IFU_SCSI_FLG	= $FF:1800	; R_ : SCSI control signals in.
IFU_SCSI_DAT	= $FF:1801	; RW : SCSI data bus.

IFU_SCSI_ACK	= $FF:1802	; RW : Top bit of IFU_IRQ_MSK!
IFU_IRQ_MSK	= $FF:1802	; RW : Interrupt mask, 0=disabled.

IFU_IRQ_FLG	= $FF:1803	; R_ : Interrupt flags.

SCSI_MSK	=	$F8	; IFU_SCSI_FLG
SCSI_BSY	=	$80	; IFU_SCSI_FLG Bus is Busy or Free
SCSI_REQ	=	$40	; IFU_SCSI_FLG Target Requests next Xfer

SCSI_ACK	=	$80	; IFU_IRQ_MSK

		;	********

scsi_handshake:	lda	#SCSI_ACK		; Send SCSI_ACK signal.
		tsb	IFU_SCSI_ACK

!:		tst	#SCSI_REQ, IFU_SCSI_FLG	; Wait for drive to clear
		bne	!-			; the SCSI_REQ signal.

		trb	IFU_SCSI_ACK		; Clear SCSI_ACK signal.
		rts

		;	********

scsi_get_phase:	lda	IFU_SCSI_FLG
		and	#SCSI_MSK
		rts

		;	********

scsi_abort:	lda	#IFU_INT_DAT_IN + IFU_INT_MSG_IN
		trb	IFU_IRQ_MSK		; Just in case previously set.

		sta	IFU_SCSI_CTL		; Signal SCSI_RST to stop cmd.

		ldy	#30 * 2			; Wait 30ms.
		clx				; The inner loop takes 3584
.wait_500us:	sxy				; cycles, which is almost a
		sxy				; perfect 500 microseconds!
		nop
		dex
		bne	.wait_500us
		dey
		bne	.wait_500us

		lda	#$FF			; Send $FF on SCSI bus in case
		sta	IFU_SCSI_DAT		; the CD reads it as a command.

		tst	#SCSI_REQ, IFU_SCSI_FLG	; Does the CD-ROM want the PCE
		beq	scsi_initiate		; to send/recv some data?

.flush_data:	lda	IFU_SCSI_FLG		; Report the current state.
		and	#$F8
		inc	a
		inc	a
		jsr	track_event

		jsr	scsi_handshake		; Flush out stale data byte.

.still_busy:	tst	#SCSI_REQ, IFU_SCSI_FLG	; Keep on flushing data while
		bne	.flush_data		; the CD-ROM requests it.

		tst	#SCSI_BSY, IFU_SCSI_FLG	; Wait for the CD-ROM to drop
		bne	.still_busy		; the SCSI_BSY signal.

		lda	#INFO_NOTBUSY		; Report when the CD-ROM is
		jsr	track_event		; finally idle.

scsi_initiate:	lda	#$81			; Set the SCSI device ID bits
		sta	IFU_SCSI_DAT		; for the PCE and the CD-ROM.

		tst	#SCSI_BSY, IFU_SCSI_FLG	; Is the CD-ROM already busy
		bne	scsi_abort		; doing something?

		sta	IFU_SCSI_CTL		; Signal SCSI_SEL to CD-ROM.

.test_scsi_bus:	ldy	#18			; Wait for up to 20ms for the
		clx				; CD-ROM to signal SCSI_BSY.
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

		;	********

		;
		;
		;

	.endif
