; ***************************************************************************
; ***************************************************************************
;
; ted2-sd.asm
;
; Functions for using the SD card interface on the Turbo Everdrive v2.
;
; Copyright John Brandwood 2019-2022.
;
; Distributed under the Boost Software License, Version 1.0.
; (See accompanying file LICENSE_1_0.txt or copy at
;  http://www.boost.org/LICENSE_1_0.txt)
;
; ***************************************************************************
; ***************************************************************************
;
; For SD/MMC programming information, see ...
;
;   "How to Use MMC/SDC" at http://elm-chan.org/docs/mmc/mmc_e.html
;
; ***************************************************************************
; ***************************************************************************
;
; ONLY USE THESE PUBLIC INTERFACES ...
;
; sdc_initialize - Initialize the SD card.
; sdc_read_data	 - Read one or more 512-byte blocks.
; sdc_write_data - Write one or more 512-byte blocks.
;
; ***************************************************************************
; ***************************************************************************



; ***************************************************************************
; ***************************************************************************
;
; Definitions
;

; Return Codes for the SD card functions.

SDC_OK			= $00

SDC_ERR_TIMEOUT		= $FF

SDC_ERR_INIT		= $81
SDC_ERR_DSK_RD		= $82
SDC_ERR_DSK_WR		= $83
SDC_ERR_WR_CRC		= $8B	; From SD Data Response code.
SDC_ERR_WR_ERR		= $8D	; From SD Data Response code.

; MMC/SDC Command Codes.

SDC_GO_IDLE_STATE	= $40	; CMD0	 Software Reset.
SDC_SEND_OP_COND	= $41	; CMD1	 Initiate initialization process.
SDC_SEND_IF_COND	= $48	; CMD8	 SDC V2 only! Check voltage range.
SDC_SEND_CID		= $4A	; CMD10	 Read CID register.
SDC_STOP_TRANSMISSION	= $4C	; CMD12	 Stop reading blocks.
SDC_SEND_STATUS		= $4D	; CMD13	 Send Status.
SDC_SET_BLOCK_LENGTH	= $50	; CMD16	 Change R/W block size (not SDHC/SDXC).
SDC_READ_ONE_BLOCK	= $51	; CMD17	 Read a block.
SDC_READ_BLOCKS		= $52	; CMD18	 Read multiple blocks.
SDC_WRITE_ONE_BLOCK	= $58	; CMD24	 Write a block.
SDC_WRITE_BLOCKS	= $59	; CMD25	 Write multiple blocks.
SDC_APP_CMD		= $77	; CMD55	 Leading command of ACMD<n> command.
SDC_READ_OCR		= $7A	; CMD58	 Read OCR.

SDC_APP_SEND_NUM_WR_BLK = $56	; ACMD22 SDC only! # of blocks written OK.
SDC_APP_WRITE_ERASE_CNT = $57	; ACMD23 SDC only! Set pre-erase count.
SDC_APP_SEND_OP_COND	= $69	; ACMD41 SDC only! Begin initialization.

SDC_WRITE_MUL_TOKEN	= $FC	; Start of packet for write multiple blocks.
SDC_STOP_TRAN_TOKEN	= $FD	; Stop token for write multiple blocks.
SDC_DATA_XFER_TOKEN	= $FE	; Start of packet for read/write single block.

SDC_DUMMY_CRC		= $01	; 7-bit CRC plus 1-bit tail.

SDC_V2			= $40	; MUST be $40!!!
SDC_HC			= $80	; NZ if block addressing instead of byte.



; ***************************************************************************
; ***************************************************************************
;
;
; Data
;

		.zp

sdc_data_addr:	ds	2

		.bss

sdc_cmd_param:	ds	4		; 32-bits (big-endian).
sdc_block_num:	ds	4		; Current SD block number.
sdc_block_cnt:	ds	4		; Block count (32-bit, but 32MB max).
sdc_card_type:	ds	1		; card type bit0=SDHC bit1=v2
sdc_cid_value:	ds	16	        ; SD card's CID register.

		.code

; Print messages?

SDC_PRINT_MESSAGES	= 0	; Should functions print error messages?

	.if	SDC_PRINT_MESSAGES

_init_found_v2: db	" SDC init found V2 card.",$0D,0
_init_v2_sdhc:	db	" SDC init found SDHC card.",$0D,0
_init_ready:	db	" SDC card ready.",$0D,0

_init_ok:	db	" SDC init completed OK.",$0D,0
_init_error:	db	" SDC init failed!",$0D,0

_disk_read_err: db	" SDC read sector failed!",$0D,0
_disk_write_err:db	" SDC write sector failed!",$0D,0

	.endif	SDC_PRINT_MESSAGES


		.procgroup			; Group ted2-sd in 1 bank!

; Use const_0000 from core-kernal.asm if possible!

	.ifdef	const_0000
sdc_zero	=	const_0000
	.else
sdc_zero:	ds	2			; zero
	.endif



; ***************************************************************************
; ***************************************************************************
;
; sdc_initialize - Initialize the SD card.
;
; Returns: Y,A,Z-flag,N-flag = SDC_OK or an error code.
;

sdc_initialize	.proc

		tma4				; Preserve MPR4.
		pha
		cla				; Map the TED2 into MPR4.
		tam4

		lda	ted2_unlocked		; Is TED2 hardware unlocked?
		bne	.unlocked

		call	unlock_ted2		; If not, try unlocking it!
		beq	.unlocked
		jmp	.sdc_init_done		; Return the error code.

.unlocked:	stz	TED_BASE_ADDR + TED_REG_SPI_CFG

		stz	sdc_card_type

		stz	<sdc_data_addr + 0
		stz	<sdc_data_addr + 1

		tai	sdc_zero, sdc_block_num, 4 + 4

		; Send > 74 clock pulses with CS off to enter native mode.

		TED_SPI_CS_OFF
		TED_SPI_SPD_LO

		ldx	#24
.reset_native:	jsr	spi_recv_byte
		dex
		bne	.reset_native

		tai	sdc_zero, sdc_cmd_param, 4

		; Send CMD0 to turn SD card into SPI mode.

		lda	#SDC_GO_IDLE_STATE
		ldy	#$95			; Valid CRC
		jsr	sdc_send_cmd
		cpy	#$01	        	; "In Idle" state?
		beq	.card_idle

		lda	#SDC_GO_IDLE_STATE
		ldy	#$95			; Valid CRC
		jsr	sdc_send_cmd
		cpy	#$01	        	; "In Idle" state?
		bne	.card_error

		; Send CMD8 to check SDCv2 voltage range.

.card_idle:	lda	#$01			; Set interface voltage
		sta	sdc_cmd_param + 2	; range to 2.7-3.6V.
		lda	#$AA			; Set check pattern.
		sta	sdc_cmd_param + 3

		lda	#SDC_SEND_IF_COND
		ldy	#$87			; Valid CRC
		jsr	sdc_send_cmd
		cpy	#$01			; Was the command accepted?
		bne	.sdc_init

		jsr	spi_recv_byte		; Skip 1st byte of response.
		jsr	spi_recv_byte		; Skip 2nd byte of response.
		jsr	spi_recv_byte		; Get the voltage range.
		and	#$0F
		tay
		jsr	spi_recv_byte		; Get the check pattern.
		cmp	#$AA			; Confirm check pattern.
		bne	.card_error
		cpy	#$01			; Confirm voltage range.
		bne	.card_error

		lda	#SDC_V2	        	; We've got a Type 2 card!
		tsb	sdc_card_type

	.if	SDC_PRINT_MESSAGES
		PUTS	_init_found_v2
	.endif

.sdc_init:	ldx	#66			; Set timeout to > 1 second.

		tai	sdc_zero, sdc_cmd_param, 4

.sdc_init_loop: stz	sdc_cmd_param + 0

		lda	#SDC_APP_CMD		; Send part 1 of APP command.
		ldy	#SDC_DUMMY_CRC
		jsr	sdc_send_cmd
		bmi	.card_error		; Was there a timeout?
		cpy	#$02			; Was there an error?
		bcs	.sdc_init_wait

		lda	sdc_card_type		; SDCv2=$40 or SDCv1=$00
		sta	sdc_cmd_param + 0	; Because HCS flag is bit 30!

		lda	#SDC_APP_SEND_OP_COND	; Send part 2 of the command,
		ldy	#SDC_DUMMY_CRC		; to INITIALIZE the SD card.
		jsr	sdc_send_cmd
		beq	.sdc_ready		; Is the SD card initialized?
		bmi	.card_error		; Was there a timeout?
		cpy	#$01			; Was there an error?
		bne	.card_error

.sdc_init_wait: jsr	wait_vsync		; Delay before asking again.

		dex				; Still waiting for the
		bne	.sdc_init_loop		; card to initialize?

.card_error:	bsr	spi_recv_byte		; Wait 8clk before halting SPI.

		ldy	#SDC_ERR_INIT		; Timeout or Error.
		bra	.sdc_init_done

.sdc_ready:	stz	sdc_cmd_param + 0

	.if	SDC_PRINT_MESSAGES
		PUTS	_init_ready
	.endif

		tst	#SDC_V2,sdc_card_type
		beq	.sdc_blocklen

		lda	#SDC_READ_OCR		; Send CMD58 to read the OCR.
		ldy	#SDC_DUMMY_CRC
		bsr	sdc_send_cmd
		bne	.card_error

		bsr	spi_recv_byte		; Read top byte of OCR.
		and	#$40			; Save the HCS bit.
		tay
		bsr	spi_recv_byte		; Skip 2nd byte of OCR.
		bsr	spi_recv_byte		; Skip 3rd byte of OCR.
		bsr	spi_recv_byte		; Skip btm byte of OCR.
		tya				; Check the HCS bit.
		beq	.sdc_blocklen

		lda	#SDC_HC			; Card uses block addressing!
		tsb	sdc_card_type

	.if	SDC_PRINT_MESSAGES
		PUTS	_init_v2_sdhc
	.endif

		bra	.sdc_init_ok

.sdc_blocklen:	lda	#>512			; Set block len to 512 bytes
		sta	sdc_cmd_param + 2	; if not an HC card.
;		stz	sdc_cmd_param + 3

		lda	#SDC_SET_BLOCK_LENGTH
		ldy	#SDC_DUMMY_CRC
		bsr	sdc_send_cmd
		bne	.card_error

.sdc_init_ok:	stz	sdc_cmd_param + 2

		lda	#SDC_SEND_CID		; Send CMD4A to read the CID.
		ldy	#SDC_DUMMY_CRC
		bsr	sdc_send_cmd
		bne	.card_error

		clx				; Wait for the CID data block.
		cly
.sdc_wait_cid:	bsr	spi_recv_byte
		cmp	#SDC_DATA_XFER_TOKEN
		beq	.sdc_xfer_cid
		dey
		bne	.sdc_wait_cid
		bra	.card_error		; Timeout!

.sdc_xfer_cid:	bsr	spi_recv_byte		; Read 16-byte unique Card ID.
		sta	sdc_cid_value, x
		inx
		cpx	#16
		bne	.sdc_xfer_cid

		bsr	spi_recv_byte		; Skip 2-byte CRC.
		bsr	spi_recv_byte

		bsr	spi_recv_byte		; Wait 8clk before halting SPI.

		ldy	#SDC_OK

		TED_SPI_SPD_HI			; OK to do this now.

.sdc_init_done: TED_SPI_CS_OFF			; All done, deselect the card.

		pla				; Restore MPR4.
		tam4

;		tya				; Set the N & Z result flags.
		leave				; Return the result.

		.endp				; sdc_initialize



; ***************************************************************************
; ***************************************************************************
;
; spi_recv_byte - Get a byte from the SD card.
; spi_send_byte - Send a byte to the SD card.
;
; Preserves X & Y registers.
;
; N.B. FOR INTERNAL USE ONLY, THIS IS NOT A PUBLIC FUNCTION!
;

spi_recv_byte:	lda	#$FF
spi_send_byte:	sta	TED_BASE_ADDR + TED_REG_SPI
		TED_SPI_WAIT
		lda	TED_BASE_ADDR + TED_REG_SPI
		rts



; ***************************************************************************
; ***************************************************************************
;
; sdc_send_cmd - Send a command packet to the SD card.
;
; A = CMD value, Y = CRC value.
;
; Returns: Y = result code (and Z flag) or $FF if timeout.
;
; Preserves X register.
;
; N.B. FOR INTERNAL USE ONLY, THIS IS NOT A PUBLIC FUNCTION!
;

sdc_send_cmd:	pha				; Preserve CMD parameter.

		TED_SPI_WAIT			; Wait if SPI bus busy.

		TED_SPI_CS_OFF			; Cycle the CARD_SELECT.
		bsr	spi_recv_byte
		TED_SPI_CS_ON
		bsr	spi_recv_byte

		pla				; Send CMD.
		pha
		bsr	spi_send_byte

		lda	sdc_cmd_param + 0	; Send ARG (32-bit big-endian).
		bsr	spi_send_byte
		lda	sdc_cmd_param + 1
		bsr	spi_send_byte
		lda	sdc_cmd_param + 2
		bsr	spi_send_byte
		lda	sdc_cmd_param + 3
		bsr	spi_send_byte

		tya				; Send CRC.
		bsr	spi_send_byte

		cly				; Timeout count.

		; Discard the byte after SDC_STOP_TRANSMISSION.

		pla
		cmp	#SDC_STOP_TRANSMISSION
		bne	.wait
		bsr	spi_recv_byte

		; Wait for response (0..8 bytes in SD specification).

.wait:		bsr	spi_recv_byte
		cmp	#$FF
		bne	.done
		dey
		bne	.wait

		; Timeout waiting for response!

.done:		tay				; Return response code in Y.
		rts



; ***************************************************************************
; ***************************************************************************
;
; sdc_set_blk_arg - Copy the current block number to the SD cmd parameters.
;
; Args: sdc_block_num (little-endian)
; Sets: sdc_cmd_param (big-endian)
;
; Preserves X & Y registers.
;
; N.B. FOR INTERNAL USE ONLY, THIS IS NOT A PUBLIC FUNCTION!
;

sdc_set_blk_arg:lda	sdc_card_type		; Check the SDC_HC flag.
		bpl	.byte_address

.block_address: lda	sdc_block_num+0
		sta	sdc_cmd_param+3
		lda	sdc_block_num+1
		sta	sdc_cmd_param+2
		lda	sdc_block_num+2
		sta	sdc_cmd_param+1
		lda	sdc_block_num+3
		sta	sdc_cmd_param+0
		rts

		; Standard SD cards take a byte address.
		; So multiply sector number by 512.

.byte_address:	stz	sdc_cmd_param+3
		lda	sdc_block_num+0
		asl	a
		sta	sdc_cmd_param+2
		lda	sdc_block_num+1
		rol	a
		sta	sdc_cmd_param+1
		lda	sdc_block_num+2
		rol	a
		sta	sdc_cmd_param+0
		rts



; ***************************************************************************
; ***************************************************************************
;
; sdc_read_data - Read one or more 512-byte blocks.
;
; Args: sdc_block_num
; Args: sdc_block_cnt
; Args: sdc_data_addr
;
; Returns: Y,A,Z-flag,N-flag = SDC_OK or an error code.
;
; Notes:
;
;   The sdc_data_addr destination MUST be < $8000, and also 512-byte aligned
;   if using the auto-incrementing bank capability.
;
;   When the sdc_data_addr increments to >= $8000, then the next PCE bank
;   is mapped into MPR3, and reading continues.
;

sdc_read_data	.proc

		tma4				; Preserve MPR4.
		pha
		cla				; Map the TED2 into MPR4.
		tam4

		lda	ted2_unlocked		; Is TED2 hardware unlocked?
		bne	.unlocked

		call	unlock_ted2		; If not, try unlocking it!
		bne	.exit
;		beq	.unlocked
;		jmp	.exit			; Return the error code.

.unlocked:	jsr	sdc_set_blk_arg		; Set the block num parameter.

		lda	sdc_block_cnt + 0	; Check for zero blocks.
		cmp	#2
		ora	sdc_block_cnt + 1
		tay				; Zero returns SDC_OK.
		beq	.all_done
		lda	sdc_block_cnt + 1
		sbc	#0			; Set C if >= 2 blocks.

.send_cmd:	php				; Preserve C flag.

		cla				; If == 1, SDC_READ_ONE_BLOCK.
		adc	#SDC_READ_ONE_BLOCK	; If >= 2, SDC_READ_BLOCKS.
		ldy	#SDC_DUMMY_CRC
		jsr	sdc_send_cmd
		bne	.read_error

.read_data:	jsr	spi_rd_fast		; Read the blocks from SD card.

		plp				; Restore the C flag.
		bcc	.all_done		; Was this a multiple read?

.stop_data:	phy				; Preserve result.

		tai	sdc_zero, sdc_cmd_param, 4

		lda	#SDC_STOP_TRANSMISSION	; Send SDC_STOP_TRANSMISSION.
		ldy	#SDC_DUMMY_CRC
		jsr	sdc_send_cmd		; Returns garbage!

.busy:		jsr	spi_recv_byte		; Wait for the busy to be done.
		cmp	#$FF
		bne	.busy

		ply				; Restore result.

.all_done:	jsr	spi_recv_byte		; Wait 8clk before halting SPI.

		TED_SPI_CS_OFF

	.if	SDC_PRINT_MESSAGES
		tya				; Set flags for result.
		beq	.exit			; Report the error.
		phy
		PUTS	_disk_read_err
		ply
	.endif	SDC_PRINT_MESSAGES

.exit:		pla				; Restore MPR4.
		tam4

;		tya				; Set the N & Z result flags.
		leave				; Return the result.

.read_error:	plp
		bra	.all_done

		.endp				; sdc_read_data



; ***************************************************************************
; ***************************************************************************
;
; spi_rd_fast - Hardware-accelerated sector read from SD card to RAM.
;
; Args: sdc_block_cnt
; Args: sdc_data_addr
;
; Returns: Y,A,Z-flag,N-flag = SDC_OK or an error code.
;
; N.B. FOR INTERNAL USE ONLY, THIS IS NOT A PUBLIC FUNCTION!
;
; Notes:
;
;   The sdc_data_addr destination MUST be < $8000, and also 512-byte aligned
;   if using the auto-incrementing bank capability.
;
;   When the sdc_data_addr increments to >= $8000, then the next PCE bank
;   is mapped into MPR3, and reading continues.
;

spi_rd_fast:	TED_SPI_ARD_ON

		lda	<sdc_data_addr + 0
		sta	.start_xfer + 3

.sector_loop:	lda	<sdc_data_addr + 1
		sta	.start_xfer + 4

		ldy	#$80
		clx
.wait_start:	lda	TED_BASE_ADDR + TED_REG_SPI
		cmp	#SDC_DATA_XFER_TOKEN
		beq	.start_xfer
		dex
		bne	.wait_start
		dey
		bpl	.wait_start
		bra	.timeout

		; Read 512-byte sector.

.start_xfer:	tai	TED_BASE_ADDR + TED_REG_SPI, $0000, 512

		; Skip 2-byte CRC.

		lda	TED_BASE_ADDR + TED_REG_SPI
		lda	TED_BASE_ADDR + TED_REG_SPI

		; Update destination.

		lda	<sdc_data_addr + 1	; Increment addr in bank.
		inc	a
		inc	a
		cmp	#$80			; Wrap bank at $8000.
		bne	.keep_bank

		tma3				; Map in next PCE 8KB bank.
		inc	a
		tam3

		lda	#$60			; Reset destination pointer.
.keep_bank:	sta	<sdc_data_addr + 1

		lda	sdc_block_cnt + 0	; Decrement 16-bit block
		bne	.skip			; count.
		dec	sdc_block_cnt + 1
.skip:		dec	a
		sta	sdc_block_cnt + 0
		ora	sdc_block_cnt + 1
		bne	.sector_loop

		ldy	#SDC_OK

.timeout:	TED_SPI_ARD_OFF

		tya				; Set the N & Z result flags.
		rts



; ***************************************************************************
; ***************************************************************************
;
; sdc_write_data - Write one or more 512-byte blocks.
;
; Args: sdc_block_num
; Args: sdc_block_cnt
; Args: sdc_data_addr
;
; Returns: Y,A,Z-flag,N-flag = SDC_OK or an error code.
;
; Notes:
;
;   The sdc_data_addr destination MUST be < $8000, and also 512-byte aligned
;   if using the auto-incrementing bank capability.
;
;   When the sdc_data_addr increments to >= $8000, then the next PCE bank
;   is mapped into MPR3, and reading continues.
;

sdc_write_data	.proc

		tma4				; Preserve MPR4.
		pha
		cla				; Map the TED2 into MPR4.
		tam4

		lda	ted2_unlocked		; Is TED2 hardware unlocked?
		bne	.unlocked

		call	unlock_ted2		; If not, try unlocking it!
		beq	.unlocked
		jmp	.exit			; Return the error code.

.unlocked:	lda	sdc_block_cnt + 0	; Check for zero blocks.
		cmp	#2
		ora	sdc_block_cnt + 1
		bne	.non_zero

		tay				; Zero returns SDC_OK.
		jmp	.exit

.cmd_failed:	plp				; Discard the C flag.
		bra	.finished

.non_zero:	lda	sdc_block_cnt + 1
		sbc	#0			; Set C if >= 2 blocks.
		php				; Preserve C flag.
		bcc	.start_write

		; Tell the SD card how many blocks we're writing.

		tai	sdc_zero, sdc_cmd_param, 4

		lda	#SDC_APP_CMD		; Send part 1 of APP command.
		ldy	#SDC_DUMMY_CRC
		jsr	sdc_send_cmd
		bne	.cmd_failed

		lda	sdc_block_cnt + 1	; Send part 2 with block count.
		sta	sdc_cmd_param + 2
		lda	sdc_block_cnt + 0
		sta	sdc_cmd_param + 3

		lda	#SDC_APP_WRITE_ERASE_CNT
		ldy	#SDC_DUMMY_CRC
		jsr	sdc_send_cmd
		bne	.cmd_failed

		; Write the block(s).

.start_write:	jsr	sdc_set_blk_arg		; Set the block num parameter.

		ldx	#SDC_DATA_XFER_TOKEN	; Set token for data block.
		plp				; Is this a multiple write?
		bcc	.send_cmd
		ldx	#SDC_WRITE_MUL_TOKEN
.send_cmd:	php				; Preserve C flag.

		cla				; If == 1, SDC_WRITE_ONE_BLOCK.
		adc	#SDC_WRITE_ONE_BLOCK	; If >= 2, SDC_WRITE_BLOCKS.
		ldy	#SDC_DUMMY_CRC
		jsr	sdc_send_cmd
		bne	.cmd_failed

.write_data:	jsr	spi_recv_byte		; Delay write for 1 byte.

		jsr	spi_wr_slow		; Write the blocks to SD card.

		plp				; Restore the C flag.
		bcc	.check_write		; Was this a multiple write?

		lda	#SDC_STOP_TRAN_TOKEN	; Stop multiple block write.
		jsr	spi_send_byte
		jsr	spi_recv_byte		; Delay for 1 byte.
.busy:		jsr	spi_recv_byte		; Wait for the busy to be done.
		cmp	#$FF
		bne	.busy

.check_write:	tya				; Was there are error in the
		bne	.finished		; transfer to the SD card?

		tai	sdc_zero, sdc_cmd_param, 4

		lda	#SDC_SEND_STATUS	; Check the status of the
		ldy	#SDC_DUMMY_CRC		; actual write itself.
		jsr	sdc_send_cmd
		jsr	spi_recv_byte		; Get 2nd byte of status.
		bne	.write_failed
		tya				; Y == 0 == SDC_OK if no error.
		beq	.finished

.write_failed:	.if	0			; Do we want to know #written?

		stz	sdc_block_cnt + 0
		stz	sdc_block_cnt + 1

		lda	#SDC_APP_CMD		; Send part 1 of APP command.
		ldy	#SDC_DUMMY_CRC
		jsr	sdc_send_cmd
		bne	.count_done

		lda	#SDC_APP_SEND_NUM_WR_BLK; Send part 2 to get count.
		ldy	#SDC_DUMMY_CRC		; of successfully written blks.
		jsr	sdc_send_cmd
		bne	.count_done

		ldy	#$80			; Wait for the block count.
.wait_count:	jsr	spi_recv_byte		; (0..8 bytes per the SD
		cmp	#SDC_DATA_XFER_TOKEN	;  specification).
		beq	.xfer_count
		dey
		bne	.wait_count
		bra	.count_done		; Timeout!

.xfer_count:	jsr	spi_recv_byte		; Read 4-byte blk count.
		jsr	spi_recv_byte
		jsr	spi_recv_byte
		sta	sdc_block_cnt + 1
		jsr	spi_recv_byte
		sta	sdc_block_cnt + 0

		jsr	spi_recv_byte		; Skip 2-byte CRC.
		jsr	spi_recv_byte

.count_done:
		.endif

		ldy	#SDC_ERR_WR_ERR		; Signal that the write failed.

.finished:	jsr	spi_recv_byte		; Wait 8clk before halting SPI.

		TED_SPI_CS_OFF

		.if	SDC_PRINT_MESSAGES
		tya				; Set flags for result.
		beq	.exit			; Report the error.
		phy
		PUTS	_disk_write_err
		ply
		.endif	SDC_PRINT_MESSAGES

.exit:		pla				; Restore MPR4.
		tam4

;		tya				; Set the N & Z result flags.
		leave				; Return the result.

		.endp				; sdc_write_data



; ***************************************************************************
; ***************************************************************************
;
; spi_wr_slow - Write one or more data packets to the SPI.
;
; Args: X = SDC_DATA_XFER_TOKEN or SDC_WRITE_MUL_TOKEN
; Args: sdc_block_cnt
; Args: sdc_data_addr
;
; Returns: Y,A,Z-flag,N-flag = SDC_OK or an error code.
;
; N.B. FOR INTERNAL USE ONLY, THIS IS NOT A PUBLIC FUNCTION!
;
; Notes:
;
;   The sdc_data_addr destination MUST be < $8000, and also 512-byte aligned
;   if using the auto-incrementing bank capability.
;
;   When the sdc_data_addr increments to >= $8000, then the next PCE bank
;   is mapped into MPR3, and reading continues.
;

spi_wr_slow:	txa
;		phx				; Preserve data-start token.
		jsr	spi_send_byte		; Send data-start token.

		bsr	.write_page		; Send 1st 256 bytes of data.
		bsr	.write_page		; Send 2nd 256 bytes of data.

		jsr	spi_recv_byte		; Send 1st byte of dummy CRC.
		jsr	spi_recv_byte		; Send 2nd byte of dummy CRC.

		jsr	spi_recv_byte		; Get the response code.
		pha

.busy:		jsr	spi_recv_byte		; Wait for the busy to be done,
		cmp	#$FF			; and allow 8 clocks after xfer
		bne	.busy			; before halting SPI clock.

		pla				; Check the response code.
		and	#$1F
		ora	#$80			; Make any error negative.
		tay
;		plx				; Restore data-start token.

		cpy	#$85			; Was the data accepted?
		bne	.error

		lda	<sdc_data_addr + 1	; Wrap bank at $8000.
		cmp	#$80
		bne	.decrement

		tma3				; Map in next PCE 8KB bank.
		inc	a
		tam3

		lda	#$60			; Reset destination pointer.
		sta	<sdc_data_addr + 1

.decrement:	lda	sdc_block_cnt + 0	; Decrement 16-bit block
		bne	.skip			; count.
		dec	sdc_block_cnt + 1
.skip:		dec	a
		sta	sdc_block_cnt + 0
		ora	sdc_block_cnt + 1
		bne	spi_wr_slow

		ldy	#SDC_OK			; All finished OK!

.error:		rts

		; Write out 256 bytes.

.write_page:	cly
.write_byte:	lda	[sdc_data_addr], y
		sta	TED_BASE_ADDR + TED_REG_SPI
		TED_SPI_WAIT
		lda	TED_BASE_ADDR + TED_REG_SPI
		iny
		bne	.write_byte

		inc	<sdc_data_addr + 1
		rts

		.endprocgroup			; Group ted2-sd in 1 bank!
