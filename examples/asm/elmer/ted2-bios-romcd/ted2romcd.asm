; ***************************************************************************
; ***************************************************************************
;
; ted2romcd.asm
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

                include "syscard3-ted2.inc"


; ***************************************************************************
; ***************************************************************************
;
; Apply some extra patches to run an overlay from ROM instead of loading it
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

                .bank   0
                .org    RUNBOOT_ADR

                jsr     test_overlay    ; Patch System Card's jump to boot loader.

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
                ; Copy ROM banks $20-$3F (256KB) into SCD RAM banks $68-$87,
                ; then map banks $68-$6C to MPR2-6 and jump to bank $68:4000
                ; just as if the game/overlay had been loaded by IPL-SCD.
                ;

test_overlay:   php                     ; Disable interrupts to avoid
                sei                     ; screen-shake.

                ldx     #$20            ; Copy the SCD overlay program
                ldy     #$68            ; from ROM to RAM.
.loop:          txa
                tam2
                tya
                tam3
                tii     $4000, $6000, $2000
                inx
                iny
                cpy     #$88
                bne     .loop

                lda     VDC_SR          ; Throw away any pending VDC
                plp                     ; interrupt.

                jsr     ex_vsync        ; Wait for a stable screen.

                lda     #$68            ; Set up the same environment
                tam2                    ; as IPL-SCD.BIN
                inc     a
                tam3
                inc     a
                tam4
                inc     a
                tam5
                inc     a
                tam6

                ldx     #$FF            ; Execute the overlay program.
                txs
                jmp     $4000

                ;
                ; Change on-screen identification so that we know what this
                ; code has been built for.
                ;
                ; Originally "VER. 3.00".
                ;

                .bank   1
                .org    MESSAGE_ADR

                .db     "TED2ROMCD"
