; ***************************************************************************
; ***************************************************************************
;
; mb128-base.asm
;
; Base code for talking to an "MB128" or "Save Kun" backup-memory device.
;
; Copyright John Brandwood 2019-2021.
;
; Distributed under the Boost Software License, Version 1.0.
; (See accompanying file LICENSE_1_0.txt or copy at
;  http://www.boost.org/LICENSE_1_0.txt)
;
; ***************************************************************************
; ***************************************************************************
;
; The MB128 is built around an MSM6389 1Mbit (128KByte) serial memory chip.
;
; That memory chip is accessed by the PC Engine one bit at a time, through
; the joypad port.
;
; The MB128 normally allows the joypad port's signals to directly pass
; through to any attached joypad/mouse/multitap devices.
;
; When it detects a specific "wakeup" sequence on the joypad port, it takes
; control of the port and locks-out the other devices.
;
; You then send the MB128 a command, and when the command is finished, the
; MB128 releases the joypad port, and waits for the next "wakeup" sequence.
;
;
; The MB128 only responds to two commands, READ BITS and WRITE BITS.
;
; READ BITS
;
;  1 <addr> <size> ... MB128 then sends <data> ... PCE sends 3 zero-bits.
;
; WRITE BITS
;
;  0 <addr> <size> ... MB128 then receives <data> ... PCE sends 5 zero-bits.
;
; <addr> is a 10-bit value, in 128-byte increments within the 1Mbit range.
;
;	 The code normally uses an 8-bit value of nominal 512-byte "sectors".
;
; <size> is a 20-bit value, as the # of bits to transfer.
;
;	 The code normally uses an 8-bit value of nominal 512-byte "sectors".
;
; <addr>, <size>, and <data> are all little-endian values, sent and received
; low-bit first.
;
; The final bits of each command that the PCE sends, seem designed to flush
; out the MB128's internal shift-registers, and to prepare the MB128's
; sequence-detector for the next "wakeup" sequence.
;
; ***************************************************************************
; ***************************************************************************

; Compilation options.

MB1_TRY_UNJAM	=	1			; Try to unjam a stuck MB128.

; Error Codes.

MB1_OK		=	$00
MB1_ERR_INIT	=	$A0
MB1_ERR_CSUM	=	$A1
MB1_ERR_INVALID	=	$A2
MB1_ERR_READ	=	$A3
MB1_ERR_WRITE	=	$A4

;

		.bss

mb1_detected:	ds	1			; NZ if MB128 ever detected.

		.code



		.procgroup			; mb128-base core library.

; ***************************************************************************
; ***************************************************************************
;
; mb1_read_data - Read sectors of data into memory from the MB128.
;
; Args: __di, __di_bank = _farptr to page-aligned buffer memory in MPR3.
; Args: __al = Sector address.
; Args: __ah = Sector count.
;
; Uses: __temp
;
; Returns: X = MB1_OK (and Z flag) or an error code.
;

mb1_read_data	.proc

.wait_mutex:	lda	#$80			; Acquire port mutex to avoid
		tsb	port_mutex		; conflict with a joypad.
		bmi	.wait_mutex

		jsr	__di_to_mpr3		; Map data to read into MPR3.

;		php				; Disable interrupts during
;		sei				; this function.

		phx				; Preserve sector count.
		pha				; Preserve sector address.

		jsr	mb1_wakeup		; Wakeup the MB128 interface.
		bne	.finished		; Return error code.

		jsr	mb1_send_rd_cmd
		lda	<__al			; Get sector address.
		jsr	mb1_send_addr
		lda	<__ah			; Get sector count.
		jsr	mb1_send_size

		lda	<__ah			; Get sector count.
		jsr	mb1_xvert_size		; Xvert sectors to bytes.

		stz	IO_PORT			; CLR lo, SEL lo (buttons).
		cpx	#$00
		beq	.next_64kb
.64kb_loop:	phy
		cly
.page_loop:	phx
.byte_loop:	lda	#$80

.bit_loop:	pha
		lda	#2
		sta	IO_PORT			; CLR hi, SEL lo (reset).
		pha				; RWCLK lo for 29 cycles = 4us.
		pla
		nop
		nop
		lda	IO_PORT			; Read while in reset state.
		lsr	a
		pla
		ror	a
		stz	IO_PORT			; CLR lo, SEL lo (buttons).
		bcc	.bit_loop		; RWCLK hi for 14 cycles = 2us.

		sta	[__di], y		; Save the byte in memory.

.next_byte:	iny
		bne	.byte_loop
		inc	<__di + 1
		bpl	.next_page
		tma3
		inc	a
		tam3
		lda	#$60
		sta	<__di + 1
.next_page:	plx
		dex
		bne	.page_loop
		ply
.next_64kb:	dey
		bpl	.64kb_loop

		jsr	mb1_flush_data		; Prime wakeup buffer.

		ldx	#MB1_OK

;.finished:	lda	VDC_SR			; Skip any pending VDC irq.
;		nop
;		plp				; Restore interrupts.

.finished:	stz	port_mutex		; Release port mutex.

;		txa				; Set the N & Z result flags.
		leave				; All done, phew!

		.endp



; ***************************************************************************
; ***************************************************************************
;
; mb1_write_data - Write sectors of data from memory to the MB128.
;
; Args: __si, __si_bank = _farptr to page-aligned buffer memory in MPR3.
; Args: __al = Sector address.
; Args: __ah = Sector count.
;
; Uses: __temp
;
; Returns: X = MB1_OK (and Z flag) or an error code.
;

mb1_write_data	.proc

.wait_mutex:	lda	#$80			; Acquire port mutex to avoid
		tsb	port_mutex		; conflict with a joypad.
		bmi	.wait_mutex

;		php				; Disable interrupts during
;		sei				; this function.

		jsr	__si_to_mpr3		; Map data to check into MPR3.

		jsr	mb1_wakeup		; Wakeup the MB128 interface.
		bne	.finished		; Return error code.

		jsr	mb1_send_wr_cmd
		lda	<__al			; Get sector address.
		jsr	mb1_send_addr
		lda	<__ah			; Get sector count.
		jsr	mb1_send_size

		lda	<__ah			; Get sector count.
		jsr	mb1_xvert_size		; Xvert sectors to bytes.

		cpx	#$00
		beq	.next_64kb
.64kb_loop:	phy
		cly
.page_loop:	phx
		clx
		sec
.byte_loop:	lda	[__si], y
;		stx	IO_PORT			; RWCLK lo for 30 cycles > 4us.
		ror	a			; Put next bit in C.

.bit_loop:	pha
		cla				; Move C flag into bit 0.
		rol	a
		pha
		sta	IO_PORT			; CLR lo, SEL is bit.
		pla				; RWCLK hi for 14 cycles = 2us.
		pha
		ora	#2
		sta	IO_PORT			; CLR hi, SEL is bit (reset).
		plx				; RWCLK lo for 29 cycles = 4us.
		pla
		lsr	a
		bne	.bit_loop

.next_byte:	iny
		bne	.byte_loop
		inc	<__si + 1
		nop
		stx	IO_PORT			; RWCLK lo for 29 cycles = 4us.
		bpl	.next_page
		tma3
		inc	a
		tam3
		lda	#$60
		sta	<__si + 1
.next_page:	plx
		dex
		bne	.page_loop
		ply
.next_64kb:	dey
		bpl	.64kb_loop

		jsr	mb1_flush_data		; Prime wakeup buffer.

		ldx	#MB1_OK

;.finished:	lda	VDC_SR			; Skip any pending VDC irq.
;		nop
;		plp				; Restore interrupts.

.finished:	stz	port_mutex		; Release port mutex.

;		txa				; Set the N & Z result flags.
		leave				; All done, phew!

		.endp



; ***************************************************************************
; ***************************************************************************
;
; mb1_check_data - Check that sectors of data from the MB128 match memory.
;
; Args: __si, __si_bank = _farptr to page-aligned buffer memory in MPR3.
; Args: __al = Sector address.
; Args: __ah = Sector count.
;
; Uses: __temp
;
; Returns: X = MB1_OK (and Z flag) or an error code.
;

mb1_check_data	.proc

.wait_mutex:	lda	#$80			; Acquire port mutex to avoid
		tsb	port_mutex		; conflict with a joypad.
		bmi	.wait_mutex

;		php				; Disable interrupts during
;		sei				; this function.

		jsr	__si_to_mpr3		; Map data to check into MPR3.

		jsr	mb1_wakeup		; Wakeup the MB128 interface.
		bne	.finished		; Return error code.

		jsr	mb1_send_rd_cmd
		lda	<__al			; Get sector address.
		jsr	mb1_send_addr
		lda	<__ah			; Get sector count.
		jsr	mb1_send_size

		lda	<__ah			; Get sector count.
		jsr	mb1_xvert_size		; Xvert sectors to bytes.

		cld				; D flag indicates failure.
		stz	IO_PORT			; CLR lo, SEL lo (buttons).
		cpx	#$00
		beq	.next_64kb
.64kb_loop:	phy
		cly
.page_loop:	phx
.byte_loop:	lda	#$80

.bit_loop:	pha
		lda	#2
		sta	IO_PORT			; CLR hi, SEL lo (reset).
		pha				; RWCLK lo for 29 cycles = 4us.
		pla
		nop
		nop
		lda	IO_PORT			; Read while in reset state.
		lsr	a
		pla
		ror	a
		stz	IO_PORT			; CLR lo, SEL lo (buttons).
		bcc	.bit_loop		; RWCLK hi for 14 cycles = 2us.

		cmp	[__si], y		; Compare the byte in memory.
		beq	.next_byte
		sed				; Signal that the test failed!

.next_byte:	iny
		bne	.byte_loop
		inc	<__si + 1
		bpl	.next_page
		tma3
		inc	a
		tam3
		lda	#$60
		sta	<__si + 1
.next_page:	plx
		dex
		bne	.page_loop
		ply
.next_64kb:	dey
		bpl	.64kb_loop

		php				; Preserve the D flag.
		cld				; Reset D flag.

		jsr	mb1_flush_data		; Prime wakeup buffer.

		pla				; Get the D flag value.
		and	#$08			; Test the D flag.
		tax

;.finished:	lda	VDC_SR			; Skip any pending VDC irq.
;		nop
;		plp				; Restore interrupts.

.finished:	stz	port_mutex		; Release port mutex.

;		txa				; Set the N & Z result flags.
		leave				; All done, phew!

		.endp



; ***************************************************************************
; ***************************************************************************
;
; mb1_detect - Detect whether an MB128 is present.
;
; Uses: __temp
;
; Returns: X = MB1_OK (and Z flag) or an error code.
;

mb1_detect	.proc

.wait_mutex:	lda	#$80			; Acquire port mutex to avoid
		tsb	port_mutex		; conflict with a joypad.
		bmi	.wait_mutex

;		php				; Disable interrupts during
;		sei				; this function.

		jsr	mb1_wakeup		; Wakeup the MB128 interface.
		bne	.finished

.ready:		lda	#%00000001		; Send READ 1 BIT FROM
		jsr	mb1_send_byte		; ADDR 0 command.
		lda	#%00001000
		jsr	mb1_send_byte
;		lda	#%00000000
		jsr	mb1_send_byte
;		lda	#%00000000		; Send 7 bits, Recv 1 bit,
		jsr	mb1_send_byte		; but we ignore the value.

		ldx	#MB1_OK			; Return MB128_OK, i.e. found!

.finished:	jsr	mb1_flush_data		; Prime wakeup buffer.

;		lda	VDC_SR			; Skip any pending VDC irq.
;		nop
;		plp				; Restore interrupts.

		stz	port_mutex		; Release port mutex.

;		txa				; Set the N & Z result flags.
		leave				; All done, phew!

		.endp



; ***************************************************************************
; ***************************************************************************
;
; mb1_wakeup - Wake up the MB128 so that it is ready for a command.
;
; Returns: X = MB1_OK (and Z flag) or an error code.
;
; Uses: __temp
;
; N.B. For INTERNAL use, not for APPLICATION use!
;

mb1_wakeup:	ldy	#$80 + 1		; Max 128KB of data to "unjam".

		ldx	#3			; Retry 3 times (not needed).

.retry:		lda	#$A8			; Send "$A8" to MB128.
		jsr	mb1_send_byte		; Send a byte to MB128.

		lda	#%10			; Send '0' bit to MB128.
		jsr	mb1_send_bits		; Selects buttons.

		lda	IO_PORT			; Read buttons.
		and	#$0F
		sta	<__temp

		lda	#%11			; Send '1' bit to MB128.
		jsr	mb1_send_bits		; Selects direction-pad.

		lda	IO_PORT			; Read direction-pad.
		asl	a
		asl	a
		asl	a
		asl	a

.self_mod:	ora	<__temp			; Composite the buttons.
		cmp	#$40			; Magic value for detection.
		bne	.not_detected		; L, R, U, RUN, SEL, I & II.

		sta	mb1_detected		; Remember this for the future.

		ldx	#MB1_OK			; MB128_OK, found and ready!
		rts				;

.not_detected:	dex				; Timeout?
		bne	.retry			; Timeout!

	.if	MB1_TRY_UNJAM

		lda	mb1_detected		; Did we ever detect an MB128?
		beq	.fail			; Only try unjamming if "yes".

		dey				; Are there any more sectors
		beq	.fail			; to flush?

		cla				; Send/Recv 2 sectors worth
.unjam:		bsr	mb1_send_byte		; of data with the MB128 to
		bsr	mb1_send_byte		; unjam the interface.
		bsr	mb1_send_byte
		bsr	mb1_send_byte
		dex
		bne	.unjam

		ldx	#1			; Try wakeup again.
		bra	.retry

	.endif	MB1_TRY_UNJAM

.fail:		ldx	#MB1_ERR_INIT		; MB1_ERR_INIT, i.e. not found.
		rts



; ***************************************************************************
; ***************************************************************************
;
; mb1_send_byte - Send a single byte to the MB128.
; mb1_send_bits - Send a # of bits to the MB128.
;
; Args: A = data to send.
;
; Returns: A = 0 and CS.
;
; N.B. X,Y are preserved.
;
; N.B. For INTERNAL use, not for APPLICATION use!
;

mb1_send_bits:	clc
		db	$B0			; BCS instruction to skip SEC.
mb1_send_byte:	sec
		ror	a
		phx
.bit_loop:	pha
		cla				; Move C flag into bit 0.
		rol	a
		pha
		sta	IO_PORT			; CLR lo, SEL is bit.
		pla				; RWCLK hi for 14 cycles = 2us.
		pha
		ora	#2
		sta	IO_PORT			; CLR hi, SEL is bit (reset).
		plx				; RWCLK lo for 29 cycles = 4us.
		pla
		lsr	a
		bne	.bit_loop
		sax
		sax
		sax
		sax
		stx	IO_PORT			; Keep SEL as last bit sent so
		plx				; mb1_wakeup can read the port.
		rts				; Return with A = 0.

		;

mb1_send_rd_cmd:lda	#%00001001		; Send '1' followed by
		bra	mb1_send_bits		; low 2 address bits.

		;

mb1_send_wr_cmd:lda	#%00001000		; Send '0' followed by
		bra	mb1_send_bits		; low 2 address bits.

		;

mb1_flush_data:	lda	#%00100000		; Send five '0' bits
		bra	mb1_send_bits		; to flush MB128 buffer.

		;

mb1_send_addr:	bsr	mb1_send_byte		; Send top 8 bits of
		lda	#%00001000		; address followed by
		bra	mb1_send_bits		; #-of-bits count.

		;

mb1_send_size:	jsr	mb1_xvert_size		; Xvert sectors to bytes.
		bsr	mb1_send_byte		; Send lo-byte.
		txa
		bsr	mb1_send_byte		; Send hi-byte.
		tya
		and	#%00000001
		ora	#%00000010
		bra	mb1_send_bits		; Send bit-17.

		;

mb1_xvert_size:	asl	a			; Xvert #-of-sectors
		tax				; to #-of-bytes.
		cla
		rol	a
		tay
		cla
		rts

		.endprocgroup			; mb128-base core library.
