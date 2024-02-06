; ***************************************************************************
; ***************************************************************************
;
; ted2usbcd.asm
;
; Create a Turbo Everdrive 2 version of the Super System Card that can be
; used to test homebrew SCD overlay programs created by PCEAS.
;
; Copyright John Brandwood 2021-2022.
;
; Distributed under the Boost Software License, Version 1.0.
; (See accompanying file LICENSE_1_0.txt or copy at
;  http://www.boost.org/LICENSE_1_0.txt)
;
; ***************************************************************************
; ***************************************************************************
;
; NOTE: This is not compatible with programs written in HuC!
;
; NOTE: This *is* compatible wtih programs written in KickC!
;
; ***************************************************************************
; ***************************************************************************


; ***************************************************************************
; ***************************************************************************
;
; Use the original TED2 v3.02 Super System Card patch as the basis for this.
;

        	include "../ted2-bios-romcd/syscard3-ted2.inc"


; ***************************************************************************
; ***************************************************************************
;
; Apply some extra patches to run an overlay from USB instead of loading it
; from the CD, but try to keep the environment as similar as possible.
;

		.list
		.mlist

		;
		; Patch the System Card to run our code just after the CD IPL
		; has been loaded into memory and verified, and just before
        	; it would normally run the IPL's boot-loader code.
		;
		; Bank $1F is mapped into MPR6 to perform the check for
        	; Hudson's licensed IPL boot-loader.
		;

		.bank	0
		.org	RUNBOOT_ADR

        	jsr	usb_download	; Patch System Card's jump to boot loader.

                ;
                ; Autorun the CD, but not instantly, because that causes problems.
                ;

                .if     JPN_SYSCARD
AUTORUN_ADR	=	$C8C4
                .else
AUTORUN_ADR	=	$C8B7
                .endif

		.bank	1
		.org	AUTORUN_ADR

		jsr	autorun_cd

                .bank   0
                .org    $FF00

autorun_cd:	lda	irq_cnt
		cmp	#30		; Wait 1/2 second.
		cla
		bcc	.set_pad1
		lda	#$08
.set_pad1:	sta	joynow
		sta	joytrg
		rts

		;
		; Change on-screen identification so that we know what this
		; code has been built for.
		;
        	; Originally "VER. 3.00".
		;

		.bank	1
		.org	MESSAGE_ADR

        	.db	"TED2USBCD"

		; TED2 hardware locations when mapped into MPR7.

TED2_SPI	=	$E000
TED2_SPI2	=	$E001
TED2_FIFO	=	$E003
TED2_STATE	=	$E004
TED2_KEY1	=	$E005
TED2_CFG	=	$E006
TED2_MAP	=	$E007
TED2_KRIKZZ	=	$FFE5

		;
		; Allow Krikzz's turbo-usb2.exe to send a ROM that is really
		; a SuperCD overlay to load into banks $68-$87.
		;

		.bank	31
		.org	$D900		; BIOS free memory to $DAE0!

		; Confirm that the TED2 hardware is available.

usb_download:	php				; Disable interrupts.
		sei

		lda	#$A5			; Unlock TED2 registers.
		sta	TED2_KEY1
		lda	#%00001000		; FPGA=on+writable, RAM=writable.
		sta	TED2_CFG

        	ldx	#5			; Check that KRIKzz's FPGA
.check:		lda	TED2_KRIKZZ, x		; ROM is now in bank 0.
		cmp	.str_krikzz, x
		bne	.no_ted2
		dex
		bpl	.check
		bra	.got_ted2

.no_ted2:	plp				; Execute the CD's boot code.
		jmp	$2B26

.str_krikzz:	db	"KRIKZZ"		; Ident in FPGA ROM.

		; Found TED2, start waiting for USB download.

.got_ted2:	stz	irq_cnt			; Reset message color.
		stz	.had_star + 1		; Self-modifying code!

		; Keep flashing the "WAITING" message.

.next_frame:	lda	#%00001011		; FPGA=off+locked, RAM=writable.
		sta	TED2_CFG

		tii	$FFB0, $E000, 8		; Restore damaged BIOS code.

		plp				; Restore interrupts.

		lda	irq_cnt			; Wait for the next vblank.
.wait_vblank:	cmp	irq_cnt
		beq	.wait_vblank

		php				; Disable interrupts.
		sei

		jsr	print_waiting		; Print "WAITING" message.

		lda	#$A5			; Unlock TED2 registers.
		sta	TED2_KEY1
		lda	#%00001000		; FPGA=on+writable, RAM=writable.
		sta	TED2_CFG

.wait_usb:	tst	#$08, TED2_STATE	; Have we received a USB byte?
		beq	.next_frame

		; turbo-usb2.exe is sending us a command!

.got_usb:	lda	TED2_FIFO
        	cmp	#'*'
		beq	.got_star
.had_star:	ldx	#0			; Was this preceded by a '*'?
		beq	.ignore
        	cmp	#'t'
		beq	.got_t
        	cmp	#'g'
		beq	.got_g
.ignore:	stz	.had_star + 1		; Self-modifying code!
		bra	.wait_usb

		; turbo-usb2.exe is sending us a command!

.got_star:	sta	.had_star + 1		; Self-modifying code!
		bra	.wait_usb

        	; turbo-usb2.exe wants to know if we're listening!

.got_t:		tst	#$04, TED2_STATE
		beq	.got_t
		lda	#'k'
		sta	TED2_FIFO
		bra	.wait_usb

		; turbo-usb2.exe wants to send a HuCard ROM image!

.got_g:		jsr	print_loading		; Print "LOADING" message.

		lda	#$68			; Select first SuperCD bank.
		tam3

.bank_loop:	clx
		lda	#$60			; Reset the desination page.
		sta	.save_page + 2

.byte_loop:	lda	#$08			; Copy a byte from USB.
.wait_byte:	bit	TED2_STATE
		beq	.wait_byte
		lda	TED2_FIFO
.save_page:	sta	$6000, x		; Self-modifying code!
		inx
		bne	.byte_loop		; Same page?
		inc	.save_page + 2
		bpl	.byte_loop		; Same bank?

		tma3				; Map in next PCE 8KB bank.
		inc	a
		cmp	#$88			; Just loaded bank $87?
		bcc	.what_next
		lda	#$40			; Wrap around to bank $40.

.what_next:	tam3				; Set the next bank to load.

		tst	#$08, TED2_STATE	; What to do next?
		beq	.what_next
		lda	TED2_FIFO
        	cmp	#'+'			; Load another bank!
		beq	.bank_loop

.card_type:	tst	#$08, TED2_STATE	; Is this a special HuCARD?
		beq	.card_type
		lda	TED2_FIFO		; Ignore the HuCARD type.

		lda	#%00001011		; FPGA=off+locked, RAM=writable.
		sta	TED2_CFG

		tii	$FFB0, $E000, 8		; Restore damaged BIOS code.

		lda	VDC_SR			; Throw away any pending VDC
		plp				; interrupt.

		jsr	ex_vsync		; Wait for a stable screen.
		jsr	print_finished		; Print "COMPLETE!" message.

		ldx	#$FF			; Execute the overlay program.
		txs

		lda	rndseed			; Reset irq_cnt if any program
		sta	irq_cnt			; wants something non-zero.

		lda	#$68			; Set up the same environment
		tam2				; as IPL-SCD.BIN
		inc	a
		tam3
		inc	a
		tam4
		inc	a
		tam5
		inc	a
;		tam6				; We're still running in MPR6!

		ldy	$4000			; HuC/MagicKit starts overlay
		cpy	#$A9                    ; with a "LDA #1" overlay num.
		bne	.exec_core
		ldy	$4001
		cpy	#$01
		bne	.exec_core

		stz	$4001			; Pretend it's not overlay #1.

		tma3				; HuC sets MPR5 to $69.
		tam5
		tma2				; HuC sets MPR6 to $68.

.exec_huc:	tii	.exec_C070, $2100, 5
		jmp	$2100
.exec_C070:	tam6				; MPR6 can be set now!
		jmp	$C070			; Execute the overlay.

.exec_core:	tii	.exec_4000, $2100, 5
		jmp	$2100
.exec_4000:	tam6				; MPR6 can be set now!
		jmp	$4000			; Execute the overlay.

		; Print the "WAITING" message.

print_waiting:	jsr	prepare_print
.loop:		lda	str_waiting, x
		sta	VDC_DL
		sty	VDC_DH
		inx
		cpx	#28
		bcc	.loop
		lda	<vdc_reg
		sta	VDC_AR
		rts

		; Print the "LOADING" message.

print_loading:	stz	irq_cnt
		jsr	prepare_print
.loop:		lda	str_loading, x
		sta	VDC_DL
		sty	VDC_DH
		inx
		cpx	#28
		bcc	.loop
		lda	<vdc_reg
		sta	VDC_AR
		rts

		; Print the "COMPLETE!" message.

print_finished:	jsr	prepare_print
.loop:		lda	str_finished, x
		sta	VDC_DL
		sty	VDC_DH
		inx
		cpx	#28
		bcc	.loop
		lda	<vdc_reg
		sta	VDC_AR
		rts

str_waiting:	db	"  WAITING FOR USB DOWNLOAD  "
str_loading:	db	"DOWNLOADING FROM COMPUTER..."
str_finished:	db	"     DOWNLOAD COMPLETE!     "

		; Select VDC BAT location and palette.

prepare_print:	st0	#VDC_MAWR
		st1.l	#$0242			; Screen BAT loaction.
		st2.h	#$0242
		st0	#VDC_VWR
		lda	irq_cnt			; Flash palette 1 or 2.
		and	#$40
		lsr	a
		lsr	a
		adc	#$13
		tay
		clx
		rts

		ds	$DAE0-*, 255		; Generate error if too long.

		.bank	32			; Expand beyond 256KB so that
		nop				; TEOS will not patch it!
