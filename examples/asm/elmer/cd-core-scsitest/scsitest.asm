; ***************************************************************************
; ***************************************************************************
;
; scsitest.asm
;
; Testing program to investigate the PC Engine's SCSI CD-ROM behavior.
;
; This could be used by emulator authors to compare a real PCE vs emulation.
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
; When running on a HuCARD the setup is standard for the CORE library.
;
; The PC Engine's memory map is set to ...
;
;   MPR0 = bank $FF : PCE hardware
;   MPR1 = bank $F8 : PCE RAM with Stack & ZP
;   MPR2 = bank $00 : HuCard ROM
;   MPR3 = bank $01 : HuCard ROM
;   MPR4 = bank $02 : HuCard ROM
;   MPR5 = bank $03 : HuCard ROM
;   MPR6 = bank $xx : HuCard ROM banked ASM library procedures.
;   MPR7 = bank $00 : HuCard ROM with "CORE(not TM)" kernel and IRQ vectors.
;
; If it is running on an old CD-ROM2 System the overlay is loaded from the
; ISO into banks $80-$87 (64KB max).
;
; The PC Engine's memory map is set to ...
;
;   MPR0 = bank $FF : PCE hardware
;   MPR1 = bank $F8 : PCE RAM with Stack & ZP & "CORE(not TM)" kernel
;   MPR2 = bank $80 : CD RAM
;   MPR3 = bank $81 : CD RAM
;   MPR4 = bank $82 : CD RAM
;   MPR5 = bank $83 : CD RAM
;   MPR6 = bank $84 : CD RAM
;   MPR6 = bank $xx : CD RAM banked ASM library procedures.
;   MPR7 = bank $80 : CD RAM or System Card's bank $00
;
; ***************************************************************************
; ***************************************************************************


		;
		; Create some equates for a very generic VRAM layout, with a
		; 64*32 BAT, followed by the SAT, then followed by the tiles
		; for the ASCII character set.
		;
		; This uses the first 8KBytes of VRAM ($0000-$0FFF).
		;

BAT_LINE	=	64
BAT_SIZE	=	64 * 32
SAT_ADDR	=	BAT_SIZE		; SAT takes 16 tiles of VRAM.
CHR_ZERO	=	BAT_SIZE / 16		; 1st tile # after the BAT.
CHR_0x10	=	CHR_ZERO + 16		; 1st tile # after the SAT.
CHR_0x20	=	CHR_ZERO + 32		; ASCII ' ' CHR tile #.

		;
		; Include the library, reading the project's configuration
		; settings from the local "core-config.inc", if it exists.
		;

		include "core.inc"		; Basic includes.

		.list
		.mlist

		include	"common.asm"		; Common helpers.
		include	"vdc.asm"		; Useful VDC routines.
		include	"font.asm"		; Useful font routines.
		include	"tty.asm"		; Useful TTY print routines.
		include	"cdrom.asm"		; Fast CD-ROM routines.



; ***************************************************************************
; ***************************************************************************
;
; Constants and Variables.
;

		.zp

timer_val:	ds	2
data_relative:	ds	1			; $80=DATA_IN clears timer.

		.bss

		; Display variables.

count_was:	ds	2
timer_was:	ds	2
text_message:	ds	80

retry_count:	ds	2

lba_top:	ds	1
lba_add:	ds	1

		; Hacks to the CD-ROM test_read() to create bad behavior.
		;
		; Neither the System Card nor Hudson's "fast" code ever do
		; stupid things like these, but all the Sherlock Holmes CD
		; games rely on this behavior.
		;

exit_on_data:	ds	1			; Exit cd_read early?
exit_on_stat:	ds	1			; Exit cd_read early?
exit_on_mesg:	ds	1			; Exit cd_read early?

delay_data:	ds	1			; Add delay after DATA_IN?

		.code

; ***************************************************************************
; ***************************************************************************



; ***************************************************************************
; ***************************************************************************
;
; core_main - This is executed after "CORE(not TM)" library initialization.
;
; This is the first code assembled after the library includes, so we're still
; in the CORE_BANK, usually ".bank 0"; and because this is assembled with the
; default configuration from "include/core-config.inc", which sets the option
; "USING_MPR7", then we're running in MPR7 ($E000-$FFFF).
;

		.code

core_main:	; Turn the display off and initialize the screen mode.

		call	init_512x224		; Initialize VDC & VRAM.

		; Upload the font to VRAM.

		stz	<_di + 0		; Destination VRAM address.
		lda	#>(CHR_0x10 * 16)
		sta	<_di + 1

		lda	#$FF			; Put font in colors 4-7,
		sta	<_al			; so bitplane 2 = $FF and
		stz	<_ah			; bitplane 3 = $00.

		lda	#16 + 96		; 16 box graphics + 96 ASCII.
		sta	<_bl

		lda	#<my_font		; Address of font data.
		sta	<_bp + 0
		lda	#>my_font
		sta	<_bp + 1
		ldy	#^my_font

		call	dropfntbox_vdc		; Upload font to VRAM.

		; Upload the 8x8 font to VRAM in colors 0-3, with box tiles
		; using colors 4-7.
		;
		; Thus we call dropfntbox_vdc() instead of dropfnt8x8_vdc()!

		stz	<_di + 0		; Destination VRAM address.
		lda	#>(CHR_0x10 * 16)
		sta	<_di + 1

		lda	#$FF			; Put font in colors 4-7,
		sta	<_al			; so bitplane 2 = $FF and
		stz	<_ah			; bitplane 3 = $00.

		lda	#16 + 96		; 16 graphics + 96 ASCII.
		sta	<_bl

		lda	#<my_font		; Address of font data.
		sta	<_bp + 0
		lda	#>my_font
		sta	<_bp + 1
		ldy	#^my_font

		call	dropfnt8x8_vdc		; Upload font to VRAM.

		; Upload the palette data to the VCE.

		stz	<_al			; Start at palette 0 (BG).
		lda	#3			; Copy 3 palettes of 16 colors.
		sta	<_ah
		lda	#<cpc464_colors		; Set the ptr to the palette
		sta.l	<_bp			; data.
		lda	#>cpc464_colors
		sta.h	<_bp
		ldy	#^cpc464_colors
		call	load_palettes		; Add to the palette queue.

		; Turn on the BG & SPR layers, then wait for a soft-reset.

		setvec	vsync_hook, .vblank_irq	; Enable vsync_hook.
		smb4	<irq_vec

		call	set_bgon		; Enable background.

		call	wait_vsync		; Wait for vsync.

		; Initialize the timer.

		tii	timer_code, timer_irq, timer_len

		stz	TIMER_CR		; Reset timer hardware.
		stz	IRQ_ACK
		stz	TIMER_DR		; 0 -> 1024 cycles -> 143.034939us

		lda.l	#timer_irq		; Install timer_irq vector.
		sta.l	timer_hook
		lda.h	#timer_irq
		sta.h	timer_hook
		smb2	<irq_vec		; Enable timer_hook.

		; Reset the TTY.

		PRINTF	"\e<\eP0\eXL1\eYT1\n"

		; Give the CD-ROM hardware a little time to wake up.

		ldy	#30			; Wait for CD drive to get
		call	wait_nvsync		; stable after power-on.

		; Run tests until the user reboots.

.main_loop:	jsr	test_number1
		jsr	test_number2
		jsr	test_number3
		jsr	test_number4
		jsr	test_number5
		jsr	test_number6
		jsr	test_number7
		jsr	test_number8
		jsr	test_number9
		jsr	test_number10
		jsr	test_number11

		PRINTF	"\fAll tests completed!\n\nExecuting CD_RESET to stop the CD.\n\n"
		call	cdr_reset
		jsr	test_finished

		jmp	.main_loop

		; Simple VBLANK IRQ handler for the CORE library.

.vblank_irq:	cli				; Allow RCR and TIMER IRQ.
		jmp	xfer_palettes		; Transfer queue to VCE now.

		;
		; 1024-cycle timer interrupt.
		;

timer_code:	stz	IRQ_ACK			; Clear timer interrupt.
		inc.l	<timer_val
		bne	!+
		inc.h	<timer_val
!:		rti

timer_len	=	* - timer_code

		.bss				; Run the timer code in RAM
timer_irq:	ds	timer_len		; because this bank is gone
		.code				; during System Card calls.

		;
		; Clear timer value.
		;

clear_timer:	stz	TIMER_CR
		lda	#1
		stz.l	<timer_val
		stz.h	<timer_val
		sta	TIMER_CR
		rts



; ***************************************************************************
; ***************************************************************************
;
; Event logging code, data and macro.
;

		; Circular buffer for log entries.

		.bss
logtbl_tag:	ds	256			; Tag# of entry.
logtbl_flg:	ds	256			; IFU_SCSI_FLG.
logtbl_rlo:	ds	256			; Count lo-byte.
logtbl_rhi:	ds	256			; Count hi-byte.
logtbl_tlo:	ds	256			; Timer lo-byte.
logtbl_thi:	ds	256			; Timer hi-byte.
		.zp
log_entry:	ds	1			; Input offset.
log_print:	ds	1			; Output offset.
		.code

		; Add an event to the log.

log_flg:	lda	IFU_SCSI_FLG
log_tag:	php
		sei
		nop
		ldx	<log_entry

		say
		cmp	logtbl_tag, x
		say
		bne	.unique
		cmp	logtbl_flg, x
		bne	.unique

.repeated:	inc	logtbl_rlo, x
		bne	!+
		inc	logtbl_rhi, x
		bra	!+

.unique:	inx
		stx	<log_entry
		sta	logtbl_flg, x
		tya
		sta	logtbl_tag, x
		stz	logtbl_rlo, x
		stz	logtbl_rhi, x

!:		lda.l	<timer_val
		sta	logtbl_tlo, x
		lda.h	<timer_val
		sta	logtbl_thi, x
		plp
		rts

		; Table of event tag strings to be printed.

MAX_TAGS	=	64

tagtbl_lo:	ds	MAX_TAGS
tagtbl_hi:	ds	MAX_TAGS
tagtbl_bk:	ds	MAX_TAGS

tag_number	.set	-1			; Next TAG will be 0.

		;
		; Macro to create a tag string and add a log entry.
		;

TAG		.macro
tag_number	.set	tag_number + 1
	.if	(tag_number >= MAX_TAGS)
		.fail	Too many TAGs, increase MAX_TAGS!
	.endif
		.data
!string:	db	(!end+ - !string-)	; PASCAL-style string.
		db	\1
!end:		db	0
SAVED_BANK	.set	bank(*) - _bank_base	; Remember data bank.
SAVED_ADDR	.set	*			; Remember data addr.
		.bank	CORE_BANK
		.org	tagtbl_lo + tag_number
		db	<!string-
		.org	tagtbl_hi + tag_number
		db	>!string-
		.org	tagtbl_bk + tag_number
		db	^!string-
		.bank	SAVED_BANK		; Restore data bank.
		.org	SAVED_ADDR		; Restore data addr.
		.code
		.endm

TAG_FLG		.macro
		TAG	\1
		ldy	#tag_number
		jsr	log_flg
		.endm

TAG_A		.macro
		TAG	\1
		ldy	#tag_number
		jsr	log_tag
		.endm

		;
		; Clear the current log entries.
		;

clear_log:	php
		sei
		nop
		stz	<log_entry
		stz	<log_print
		lda	#$FF
		sta	logtbl_tag
		plp
		rts

		;
		; Display the current log entries.
		;

print_log	.proc

.next:		ldx	<log_print
		cpx	<log_entry
		bcc	.show

.exit:		jsr	clear_log

		inc	tty_ypos		; End with a blank line.

		leave

.show:		inx
		stx	<log_print

		lda	tty_ypos		; Stop at end of screen.
		cmp	#28
		bcs	.next

		lda	logtbl_rlo, x
		sta.l	count_was
		lda	logtbl_rhi, x
		sta.h	count_was
		lda	logtbl_tlo, x
		sta.l	timer_was
		lda	logtbl_thi, x
		sta.h	timer_was
		lda	logtbl_flg, x
		pha

		lda	logtbl_tag, x		; Print the tag string.
		tax
		lda	tagtbl_lo, x
		sta.l	<_bp
		lda	tagtbl_hi, x
		sta.h	<_bp
		ldy	tagtbl_bk, x
		call	tty_printf_huc

		lda.l	count_was		; Only repeated once?
		ora.h	count_was
		beq	.unique

.repeated:	inc.l	count_was		; Increment the count
		bne	!+			; for display.
		inc.h	count_was
!:		PRINTF	" \eP1(repeated %d times)\eP0", count_was

.unique:	PRINTF	"\n"

		pla
		and	#SCSI_MSK
		pha
		sta	<_temp

		clx
		cly

.bit_loop:	phy
		asl	<_temp
		bcs	!+
		ldy	#.nul - .bsy

!:		lda	.bsy, y
		sta	text_message, x
		inx
		iny
		lda	.bsy, y
		sta	text_message, x
		inx
		iny
		lda	.bsy, y
		sta	text_message, x
		inx
		lda	#'-'
		sta	text_message, x
		inx

		pla
		clc
		adc	#3
		tay
		cmp	#.nul - .bsy
		bcc	.bit_loop

		dex

		ldy	#8
		pla
		bpl	!+

		ldy	#9
		cmp	#$C0
		bcc	!+

		lsr	a
		lsr	a
		lsr	a
		and	#7
		tay
!:		lda	.table, y
		tay

.phase_loop:	lda	.data_out, y
		sta	text_message, x
		beq	!+
		inx
		iny
		bra	.phase_loop

!:		PRINTF	" \eP2Timer = %5u, SCSI flags = %s\eP0\n", timer_was, text_message

		jmp	.next

.bsy:		db	"BSY"
.req:		db	"REQ"
.msg:		db	"MSG"
.csd:		db	"CSD"
.ixo:		db	"IXO"
.nul:		db	"---"

.table:		db	.data_out - .data_out	; 0
		db	.data_in  - .data_out	; 1
		db	.command  - .data_out	; 2
		db	.stat_in  - .data_out	; 3
		db	.invalid  - .data_out	; 4
		db	.invalid  - .data_out	; 5
		db	.mesg_out - .data_out	; 6
		db	.mesg_in  - .data_out	; 7
		db	.idle - .data_out	; 8
		db	.busy - .data_out	; 9
		db	.invalid  - .data_out	; A

.data_out:	db	" = DATA_OUT",0
.data_in:	db	" = DATA_IN",0
.command:	db	" = COMMAND",0
.stat_in:	db	" = STAT_IN",0
.mesg_out:	db	" = MESG_OUT",0
.mesg_in:	db	" = MESG_IN",0
.idle:		db	" = IDLE",0
.busy:		db	" = BUSY",0
.invalid:	db	" = INVALID",0

		.endp



; ***************************************************************************
; ***************************************************************************
;
; test_finished - Wait for a button press.
;

test_finished:	PRINTF	"Test completed, press \eP1RUN\eP0 to continue.\n"

.loop:		jsr	wait_vsync

		lda	joytrg + 0		; Read the joypad.
		bit	#JOY_RUN + JOY_SEL + JOY_B1 + JOY_B2
		beq	.loop

		rts



; ***************************************************************************
; ***************************************************************************
;
; test_number1 - Initialize the CD drive and scan the CD TOC.
;

test_number1:	PRINTF	"\f\eP1SCSI TEST #1: Reset CD-ROM and Initialize Disc.\eP0\n\n"

		PRINTF	"SuperCD-ROM Internal (default hidden) HuCARD memory:\n"
		PRINTF	"$18C1=%0hx $18C2=%0hx $18C3=%0hx\n", $18C1, $18C2, $18C3
		PRINTF	"SuperCD-ROM External (always enabled) HuCARD memory:\n"
		PRINTF	"$18C5=%0hx $18C6=%0hx $18C7=%0hx\n\n", $18C5, $18C6, $18C7

		jsr	clear_timer

	.if	CDROM

		; System Card processing for changing a disc.

.reset_cd:	TAG_FLG	"Reset CD-ROM hardware (start) ..."

		system	cd_reset		; Reset the CD drive.

		TAG_FLG	"Reset CD-ROM hardware (finished)."

.wait_busy:	cla				; Check for CD drive busy.
		system	cd_stat
		cmp	#$00
		bne	.wait_busy

		TAG_FLG	"CD-ROM Drive idle but stopped."

.wait_ready:	lda	#1			; Check for CD drive ready.
		system	cd_stat
		cmp	#$00
		beq	.drive_ready

		ldy	#60			; Wait a second and try again.
		jsr	wait_nvsync
		bra	.wait_ready

.drive_ready:	TAG_FLG	"CD-ROM Drive spun up and ready."

		system	cd_contnts		; Read the CD-ROM disc TOC.
		cmp	#$00
		bne	.reset_cd

		TAG_FLG	"Got CD-ROM TOC information."

	.else

		; Or use the cdrom.asm code if on HuCARD.

;		call	cdr_init_disc		; Without messages!
		call	test_init_disc		; With messages!

	.endif

		call	print_log

		jmp	test_finished



; ***************************************************************************
; ***************************************************************************
;
; test_number2 - See what selects the CD-ROM
;

test_number2:	PRINTF	"\f\eP1SCSI TEST #2: What selects the CD-ROM?\eP0\n\n"

		; Does a simple write to IFU_SCSI_CTL do it?

		jsr	clear_timer

		TAG_FLG	"Checking for IDLE."

		lda	#$00
		sta	IFU_SCSI_CTL

		TAG_FLG	"Written $00 to $1800 SCSI Control bits."

		ldy	#20 * 2			; Wait 20ms.
		jsr	test_delay

		TAG_FLG	"Wait 20ms to see if state changes."

		call	print_log

		jsr	test_abort
		jsr	clear_log

		; Does an abort write to IFU_SCSI_CTL do it?

		jsr	clear_timer

		TAG_FLG	"Checking for IDLE."

		lda	#$81
		sta	IFU_SCSI_CTL

		TAG_FLG	"Written $81 to $1800 SCSI Control bits."

		ldy	#20 * 2			; Wait 20ms.
		jsr	test_delay

		TAG_FLG	"Wait 20ms to see if state changes."

		call	print_log

		jsr	test_abort
		jsr	clear_log

		; Let's see what MiSTer does!

		jsr	clear_timer

		TAG_FLG	"Checking for IDLE."

		stz	IFU_IRQ_MSK

		TAG_FLG	"Written $00 to $1802 SCSI IRQ mask."

		ldy	#20 * 2			; Wait 20ms.
		jsr	test_delay

		TAG_FLG	"Wait 20ms to see if state changes."

		call	print_log

		jsr	test_abort
		jsr	clear_log

		jmp	test_finished



; ***************************************************************************
; ***************************************************************************
;
; test_number3 - Begin communication, then don't send a command.
;

test_number3:	PRINTF	"\f\eP1SCSI TEST #3: Initiate communication with CD-ROM, then ...\eP0\n\n"

		jsr	clear_timer

		jsr	test_initiate
		call	print_log

		jmp	test_finished



; ***************************************************************************
; ***************************************************************************
;
; test_number4 - Restart communication, then don't send a command.
;

test_number4:	PRINTF	"\f\eP1SCSI TEST #4: ... Restart communication with CD-ROM.\eP0\n\n"

		jsr	clear_timer

		jsr	test_initiate
		call	print_log

		jsr	test_abort
		jsr	clear_log

		jmp	test_finished



; ***************************************************************************
; ***************************************************************************
;
; test_number5 - Read 2 sectors.
;

test_number5:	PRINTF	"\f\eP1SCSI TEST #5: Read 2 sectors.\eP0\n\n"

		jsr	clear_timer

		lda	#$04			; Read LBA #$000004
		stz	<_ch
		stz	<_cl
		sta	<_dl

		lda	#2			; Read 2 sectors.
		sta	<_al

		jsr	test_initiate		; Acquire the SCSI bus.

		jsr	test_cd_read		; Issue read command.

		call	print_log		; Display the states.

		jmp	test_finished



; ***************************************************************************
; ***************************************************************************
;
; test_number6 - Read 2 sectors, abort before 1st.
;

test_number6:	PRINTF	"\f\eP1SCSI TEST #6: Read 2 sectors, abort before 1st.\eP0\n\n"

		jsr	clear_timer

		lda	#1
		sta	exit_on_data

		lda	#$04			; Read LBA #$000004
		stz	<_ch
		stz	<_cl
		sta	<_dl

		lda	#2			; Read 2 sectors.
		sta	<_al

		jsr	test_initiate		; Acquire the SCSI bus.

		call	clear_log		; Hide early states.

		jsr	test_cd_read		; Issue read command.

		jsr	test_abort		; Abort the command.

		call	print_log		; Display the states.

		stz	exit_on_data

		jmp	test_finished



; ***************************************************************************
; ***************************************************************************
;
; test_number7 - Read 2 sectors, abort before 2nd.
;

test_number7:	PRINTF	"\f\eP1SCSI TEST #7: Read 2 sectors, abort before 2nd.\eP0\n\n"

		jsr	clear_timer

		lda	#2
		sta	exit_on_data

		lda	#$04			; Read LBA #$000004
		stz	<_ch
		stz	<_cl
		sta	<_dl

		lda	#2			; Read 2 sectors.
		sta	<_al

		jsr	test_initiate		; Acquire the SCSI bus.

		call	clear_log		; Hide early states.

		jsr	test_cd_read		; Issue read command.

		jsr	test_abort		; Abort the command.

		call	print_log		; Display the states.

		stz	exit_on_data

		jmp	test_finished



; ***************************************************************************
; ***************************************************************************
;
; test_number8 - Read 2 sectors, abort during STAT_IN.
;

test_number8:	PRINTF	"\f\eP1SCSI TEST #8: Read 2 sectors, abort during STAT_IN.\eP0\n\n"

		jsr	clear_timer

		inc	exit_on_stat

		lda	#$04			; Read LBA #$000004
		stz	<_ch
		stz	<_cl
		sta	<_dl

		lda	#2			; Read 2 sectors.
		sta	<_al

		jsr	test_initiate		; Acquire the SCSI bus.

		call	clear_log		; Hide early messages.

		jsr	test_cd_read		; Issue read command.

		jsr	test_abort		; Abort the command.

		call	print_log		; Display the states.

		stz	exit_on_stat

		jmp	test_finished



; ***************************************************************************
; ***************************************************************************
;
; test_number9 - Read 2 sectors, abort during MESG_IN.
;

test_number9:	PRINTF	"\f\eP1SCSI TEST #9: Read 2 sectors, abort during MESG_IN.\eP0\n\n"

		jsr	clear_timer

		dec	exit_on_mesg

		lda	#$04			; Read LBA #$000004
		stz	<_ch
		stz	<_cl
		sta	<_dl

		lda	#2			; Read 2 sectors.
		sta	<_al

		jsr	test_initiate		; Acquire the SCSI bus.

		call	clear_log		; Hide early states.

		jsr	test_cd_read		; Issue read command.

		jsr	test_abort		; Abort the command.

		call	print_log		; Display the states.

		stz	exit_on_mesg

		jmp	test_finished



; ***************************************************************************
; ***************************************************************************
;
; test_number10 - Read 5 sectors, delaying after each read.
;

test_number10:	stz	delay_data		; No delay while positioning.

		lda	#$04			; Read LBA #$000004
		stz	<_ch
		stz	<_cl
		sta	<_dl

		lda	#5			; Read 5 sectors.
		sta	<_al

		jsr	test_initiate		; Acquire the SCSI bus.

		jsr	test_cd_read		; Perform a cd_seek.

		call	clear_log		; Hide results.

.retry:		lda	delay_data		; No point in a zero delay!
		bne	!+
		inc	delay_data

!:		PRINTF	"\f\eP1SCSI TEST #10: Read 5 sectors, delay %hdms after each.\eP0\n\n", delay_data

		smb7	<data_relative		; PHASE_DATA_IN clears timer.

		lda	#$04			; Read LBA #$000004
		stz	<_ch
		stz	<_cl
		sta	<_dl

		lda	#5			; Read 5 sectors.
		sta	<_al

		jsr	test_initiate		; Acquire the SCSI bus.

		jsr	test_cd_read		; Issue read command.

		call	clear_log		; Hide end states.

		PRINTF	"Press \eP1\x1E\x1F\eP0 to change delay, \eP1RUN\eP0 to continue.\n"

.loop:		jsr	wait_vsync

		lda	joytrg + 0		; Read the joypad.

		bit	#JOY_L + JOY_R
		bne	.retry

		bit	#JOY_U
		beq	!+
		lda	delay_data
		inc	a
		and	#$1F
		sta	delay_data
		bra	.retry

!:		bit	#JOY_D
		beq	!+
		lda	delay_data
		dec	a
		and	#$1F
		sta	delay_data
		bra	.retry

!:		bit	#JOY_RUN + JOY_SEL + JOY_B1 + JOY_B2
		beq	.loop

		stz	delay_data		; Reset back to 0.

		rts



; ***************************************************************************
; ***************************************************************************
;
; test_number11 - Seek then read.
;

test_number11:	stz	lba_top			; Reset back to 0.
		stz	lba_add			; Reset back to 0.

.retry:		PRINTF	"\f\eP1SCSI TEST #11: Seek to LBA $%02hx0004, then read LBA+%hd.\eP0\n\n", lba_top, lba_add

		lda	#$04			; Read LBA #$000004
		sta	<_dl
		stz	<_ch
		lda	lba_top
		sta	<_cl

		lda	#1			; Read 1 sector, i.e. cd_seek.
		sta	<_al

		jsr	test_initiate		; Acquire the SCSI bus.

		jsr	test_cd_read		; Issue read command.

		jsr	clear_timer

		lda	#$04			; Read LBA #$000004 + lba_add
		clc
		adc	lba_add
		sta	<_dl
		stz	<_ch
		lda	lba_top
		sta	<_cl

		lda	#3			; Read 3 sectors.
		sta	<_al

		jsr	test_initiate		; Acquire the SCSI bus.

		call	clear_log		; Hide early states.

		jsr	test_cd_read		; Issue read command.

		call	print_log		; Display the states.

	.if	CDROM
		PRINTF	"Press \eP1\x1E\x1F\eP0 to change offset, \eP1RUN\eP0 to continue.\n"
	.else
		PRINTF	"Press \eP1\x1E\x1F\eP0 to change offset, \eP1\x1C\x1D\eP0 to change LBA, \eP1RUN\eP0 to continue.\n"
	.endif

.loop:		jsr	wait_vsync

		lda	joytrg + 0		; Read the joypad.

	.if	CDROM
	.else
		; Only a HuCARD runs with a real 384MB+ CD large enough to change lba_top!

!:		bit	#JOY_L
		beq	!+
		lda	lba_top
		dec	a
		and	#3
		sta	lba_top
		bra	.retry

!:		bit	#JOY_R
		beq	!+
		lda	lba_top
		inc	a
		and	#3
		sta	lba_top
		bra	.retry

	.endif

!:		bit	#JOY_U
		beq	!+
		lda	lba_add
		inc	a
		and	#$1F
		sta	lba_add
		bra	.retry

!:		bit	#JOY_D
		beq	!+
		lda	lba_add
		dec	a
		and	#$1F
		sta	lba_add
		jmp	.retry

!:		bit	#JOY_RUN + JOY_SEL + JOY_B1 + JOY_B2
		beq	.loop

		stz	lba_top			; Reset back to 0.
		stz	lba_add			; Reset back to 0.

		rts



; ***************************************************************************
; ***************************************************************************
;
; test_init_disc - Initialize the CD drive and scan the CD TOC.
;
; Returns: Y,Z-flag,N-flag = $00 or an error code.
;

test_init_disc	.proc

		; Reset the CD-ROM drive.

.reset:		TAG_FLG	"Reset CD-ROM hardware (start) ..."

		call	cdr_reset

		TAG_FLG	"Reset CD-ROM hardware (finished)."

		; Wait for the CD-ROM drive to release SCSI_BSY.

		stz.l	retry_count		; Retry 1024 times, which is
		lda.h	#1024			; *far* more than needed, if
		sta.h	retry_count		; the drive is attached.

.wait_busy:	lda.l	retry_count
		bne	!+
		dec.h	retry_count
		bpl	!+			; Check for timeout!

		ldy	#CDERR_NO_DRIVE
		jmp	.finished

!:		dec.l	retry_count

		cly				; Is the drive busy?
		call	cdr_stat
		bne	.wait_busy

		; Wait for the CD-ROM to spin-up and become ready.

		TAG_FLG	"CD-ROM Drive idle but stopped."

		stz.l	retry_count
		stz.h	retry_count

.wait_ready:	ldy	#1			; Is the drive ready? Calling
		call	cdr_stat		; this is what actually makes
		beq	.drive_ready		; the drive start spinning up
		cmp	#CDERR_NOT_READY	; after reset.
		bne	.error

		ldy	#60			; Wait a second and try again.
		jsr	wait_nvsync

		inc.l	retry_count
		bne	.wait_ready
		inc.h	retry_count
		bra	.wait_ready

.error:		sta	retry_count
		ldy	retry_count
		bra	.finished

		; Get the CD-ROM disc's Table Of Contents info.

.drive_ready:	TAG_FLG	"CD-ROM Drive spun up and ready."

		call	cdr_contents
		bne	.finished

		TAG_FLG	"Got CD-ROM TOC information."

		cly				; Set return-code.

.finished:	leave

		.endp



; ***************************************************************************
; ***************************************************************************
;
; test_cd_read - with CPU memory as destination (like CD_READ to RAM).
;
; Returns: Y,Z-flag,N-flag = $00 or an error code.
;

		; Initiate CD-ROM command.

test_cd_retry:	jsr	test_initiate		; Acquire the SCSI bus.


		; Test cd_read, assume already initiated.

test_cd_read:

		; Process SCSI phases.

.proc_scsi_loop:jsr	test_get_phase
		cmp	#PHASE_COMMAND		; 1st phase.
		beq	.send_command
		cmp	#PHASE_DATA_IN		; 2nd phase.
		beq	.get_response
		cmp	#PHASE_STAT_IN		; 3rd phase.
		beq	.get_status
		cmp	#PHASE_MESG_IN		; End phase.
		bne	.proc_scsi_loop

		; SCSI command done.

.read_mesg:	phy

		lda	exit_on_mesg		; Test aborting when we get to
		beq	!+			; PHASE_MESG_IN?

		call	clear_log		; Hide previous messages.

!:		TAG_FLG	"cd_read: Got PHASE_MESG_IN"
		ply

		lda	exit_on_mesg		; Test aborting when we get to
		bne	.test_exit		; PHASE_MESG_IN?

		bit	IFU_SCSI_DAT		; Flush message byte.
		jsr	test_handshake

.wait_exit:	bit	IFU_SCSI_FLG		; Wait for the CD-ROM to
		bmi	.wait_exit		; release SCSI_BSY.

		cpy	#CDSTS_CHECK
		beq	.read_error

		phy
		TAG_FLG	"cd_read: Finished"
		ply

		tya				; Returns code in Y & A.
		rts

.test_exit:	TAG_FLG	"cd_read: Quitting early as a test!"

		cly				; Fake return code for test.

		tya				; Returns code in Y & A.
		rts

.read_error:	TAG_FLG	"cd_read: Read error!"

;		jsr	scsi_req_sense		; Clear error, get code in Y.
		bra	test_cd_retry

		; Get SCSI post-command status.

.get_status:	lda	exit_on_stat		; Test aborting when we get to
		beq	!+			; PHASE_STAT_IN?

		call	clear_log		; Hide previous messages.

!:		lda	delay_data
		beq	!+

		call	print_log		; Display the states.

!:		TAG_FLG	"cd_read: Got PHASE_STAT_IN"

		lda	exit_on_stat		; Test aborting when we get to
		bne	.test_exit		; PHASE_STAT_IN?

		ldy	IFU_SCSI_DAT		; Read status code.
		jsr	test_handshake
		jmp	.proc_scsi_loop

		; Send SCSI command.

.send_command:	TAG_FLG	"cd_read: Got PHASE_COMMAND"

		jsr	test_read_cmd

		lda	exit_on_data		; Test aborting when we get to
		ora	delay_data		; PHASE_DATA_IN?
		beq	.sent_command

		call	clear_log		; Hide previous messages.

.sent_command:	jmp	.proc_scsi_loop

		; Read SCSI reply.
		;
		; Fast-copy 2048 byte sector to RAM.
		;
		; It takes approx 55,000 cycles to read a sector.

.get_response:	lda	#$80			; Reset the timer?
		trb	<data_relative
		bpl	!+
		jsr	clear_timer

!:		TAG_FLG	"cd_read: Got PHASE_DATA_IN"

		lda	exit_on_data		; Test aborting when we get to
		beq	.read			; PHASE_DATA_IN?

		dec	exit_on_data		; Abort on 1st, 2nd, 3rd, etc
		beq	.test_exit		; sector?

.read:		ldx	#$07			; Read a sector's data.
		cly
!loop:		lda	IFU_SCSI_AUTO

;		sta	[scsi_ram_ptr], y	; Replace write-to-RAM with a
		pha				; harmless delay operation.
		pla

		nop
		nop
		nop
		iny
		bne	!loop-

;		inc.h	<scsi_ram_ptr		; Replace write-to-RAM with a
		sxy				; harmless delay operation.
		sxy

		dex
		bne	!loop-

!loop:		lda	IFU_SCSI_AUTO

;		sta	[scsi_ram_ptr], y	; Replace write-to-RAM with a
		pha				; harmless delay operation.
		pla

		nop
		nop
		iny
		cpy	#$F6
		bne	!loop-

		php
		sei
		lda	IFU_SCSI_AUTO

;		sta	[scsi_ram_ptr], y	; Replace write-to-RAM with a
		pha				; harmless delay operation.
		pla

		iny
		nop
		nop
		nop
		nop
		nop
		lda	IFU_SCSI_AUTO
		plp

;		sta	[scsi_ram_ptr], y	; Replace write-to-RAM with a
		pha				; harmless delay operation.
		pla

		iny
		nop
		nop
		nop
		nop

!loop:		lda	IFU_SCSI_AUTO

;		sta	[scsi_ram_ptr], y	; Replace write-to-RAM with a
		pha				; harmless delay operation.
		pla

		nop
		nop
		nop
		iny
		bne	!loop-
		inc.h	<scsi_ram_ptr
		cla

		; Add an artificial delay between sectors?

		lda	delay_data
		beq	!+

		asl	a
		tay
		jsr	test_delay		; Delay Y * 500us.

		TAG_FLG	"cd_read: Finished reading sector data and delay."

		jmp	.proc_scsi_loop

!:		TAG_FLG	"cd_read: Finished reading sector data."

		jmp	.proc_scsi_loop

		;
		;	********
		;

test_handshake:	lda	#SCSI_ACK		; Send SCSI_ACK signal.
		tsb	IFU_SCSI_ACK

!:		tst	#SCSI_REQ, IFU_SCSI_FLG	; Wait for drive to clear
		bne	!-			; the SCSI_REQ signal.

		trb	IFU_SCSI_ACK		; Clear SCSI_ACK signal.
		rts

		;
		;	********
		;

test_get_phase:	lda	IFU_SCSI_FLG
		and	#SCSI_MSK
		rts

		;
		;	********
		;

test_delay:	clx				; The inner loop takes 3584
.wait_500us:	sxy				; cycles, which is almost a
		sxy				; perfect 500 microseconds!
		nop
		dex
		bne	.wait_500us
		dey
		bne	.wait_500us
		rts

		;
		;	********
		;

test_abort:	TAG_FLG	"Writing $00 to $1800 to send abort signal to CD-ROM."

		lda	#IFU_INT_DAT_IN + IFU_INT_MSG_IN
		trb	IFU_IRQ_MSK		; Just in case previously set.

		stz	IFU_SCSI_CTL		; Signal SCSI_RST to stop cmd.
;		sta	IFU_SCSI_CTL		; Signal SCSI_RST to stop cmd.

		ldy	#30 * 2			; Wait 30ms.
		jsr	test_delay

		TAG_FLG	"Finished 30ms wait."

		lda	#$FF			; Send $FF on SCSI bus in case
		sta	IFU_SCSI_DAT		; the CD reads it as a command.

		tst	#SCSI_REQ, IFU_SCSI_FLG	; Does the CD-ROM want the PCE
		beq	.abort_done		; to send/recv some data?

.flush_data:	TAG_FLG	"Flushing byte from bus."

		jsr	test_handshake		; Flush out stale data byte.

.still_busy:	tst	#SCSI_REQ, IFU_SCSI_FLG	; Keep on flushing data while
		bne	.flush_data		; the CD-ROM requests it.

		tst	#SCSI_BSY, IFU_SCSI_FLG	; Wait for the CD-ROM to drop
		bne	.still_busy		; the SCSI_BSY signal.

.abort_done:	TAG_FLG	"Abort completed, CD-ROM Idle."

		rts

		;
		;	********
		;

test_initiate:	TAG_FLG	"Attempting to select CD-ROM."

		lda	#$81			; Set the SCSI device ID bits
		sta	IFU_SCSI_DAT		; for the PCE and the CD-ROM.

		tst	#SCSI_BSY, IFU_SCSI_FLG	; Is the CD-ROM already busy
		beq	.not_busy		; doing something?

		jsr	test_abort
		bra	test_initiate

.not_busy:	stz	IFU_SCSI_CTL		; Signal SCSI_SEL to CD-ROM.

;		lda	#$81
;		sta	IFU_SCSI_CTL		; Signal SCSI_SEL to CD-ROM.

		lda	IFU_SCSI_FLG		; Read IFU_SCSI_FLG ASAP!
		TAG_A	"CD-ROM Idle, written $00 to $1800 to select CD-ROM."

.test_scsi_bus:	ldy	#18			; Wait for up to 20ms for the
		clx				; CD-ROM to signal SCSI_BSY.
.wait_scsi_bus:	jsr	test_get_phase
		and	#SCSI_BSY
		bne	.ready
		dex
		bne	.wait_scsi_bus
		dey
		bne	.wait_scsi_bus

		TAG_FLG	"CD-ROM selection timeout."

		; Timeout, try again.

		bra	test_initiate

		; The normal PC Engine code just returns as soon
		; as it gets BSY, but let's see just how long it
		; takes to get a valid COMMAND phase ...

.ready:		jsr	test_get_phase
		and	#SCSI_REQ
		beq	.ready

		TAG_FLG	"CD-ROM selected and got REQ."

		rts

		;
		;	********
		;

test_read_cmd:	stz	scsi_send_buf + 5	; scsi_ctrl

		lda	<_al
		sta	scsi_send_buf + 4	; scsi_len_l =	bios_len_l

		clc
		lda	<_dl
		adc	recbase0_l		; Start of data track.
		sta	scsi_send_buf + 3	; scsi_lba_l = bios_rec_l + data_lba_l

		lda	<_ch
		adc	recbase0_m		; Start of data track.
		sta	scsi_send_buf + 2	; scsi_lba_m = bios_rec_m + data_lba_m

		lda	<_cl
		adc	recbase0_h		; Start of data track.
		sta	scsi_send_buf + 1	; scsi_lba_h = bios_rec_h + data_lba_h

		lda	#CDROM_READ		; SCSI read command.
		sta	scsi_send_buf + 0	; scsi_opcode

		clx
.send_byte:	lda	IFU_SCSI_FLG
		and	#SCSI_MSK
		cmp	#PHASE_COMMAND
		bne	.send_byte

		lda	scsi_send_buf, x
		sta	IFU_SCSI_DAT
		jsr	test_handshake

		inx
		cpx	#6			; 6-byte SCSI command.
		bne	.send_byte

		rts

		;
		;	********
		;

test_get_status:ldy	IFU_SCSI_DAT		; Read status code.
		jsr	test_handshake

.wait_mesg:	jsr	test_get_phase		; Do the final phases of the
		cmp	#PHASE_STAT_IN		; SCSI transaction sequence.
		beq	test_get_status
		cmp	#PHASE_MESG_IN
		bne	.wait_mesg

.read_mesg:	lda	IFU_SCSI_DAT		; Flush message byte.
		jsr	test_handshake

.wait_exit:	bit	IFU_SCSI_FLG		; Wait for the CD-ROM to
		bmi	.wait_exit		; release SCSI_BSY.

		tya				; Return status & set flags.
		rts



; ***************************************************************************
; ***************************************************************************
;
; The "CORE(not TM)" library initializes .DATA, so we don't do it again!
;
; ***************************************************************************
; ***************************************************************************

		.data



; ***************************************************************************
; ***************************************************************************
;
; cpc464_colors - Palette data (a blast-from-the-past!)
;
; Note: DEFPAL palette data is in RGB format, 4-bits per value.
; Note: Packed palette data is in GRB format, 3-bits per value.
;
;  $4 = dark blue background
;  $5 = light blue shadow
;  $6 = yellow font
;

		align	2

none		=	$000

cpc464_colors:	defpal	$000,none,none,none,$002,$114,$662,none
		defpal	none,none,none,none,none,none,none,none

		defpal	$000,none,none,none,$002,$114,$666,none
		defpal	none,none,none,none,none,none,none,none

		defpal	$000,none,none,none,$002,$114,$266,none
		defpal	none,none,none,none,none,none,none,none



; ***************************************************************************
; ***************************************************************************
;
; It's the font data, nothing exciting to see here!
;

my_font:	incbin	"font8x8-ascii-bold-short.dat"
