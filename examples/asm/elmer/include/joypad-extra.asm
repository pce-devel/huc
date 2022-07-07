; ***************************************************************************
; ***************************************************************************
;
; joypad-extra.asm
;
; Optional extra functions for using joypads/mice on the PC Engine.
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
; Control the "slow" and "fast" delays for auto-repeating the UP and DOWN
; button on the joypads (only if SUPPORT_6BUTTON or SUPPORT_MOUSE).
;
; The "slow" setting is the delay from when the direction is held to the
; first auto-repeat.
;
; The "fast" setting is the delay from when the first auto-repeat to all
; subsequent auto-repeats until the direction is released.
;
; Set to "0" to disable auto-repeat.
;

	.ifndef SLOW_RPT_UDLR
SLOW_RPT_UDLR	=	30
	.endif

	.ifndef FAST_RPT_UDLR
FAST_RPT_UDLR	=	10
	.endif



	.if	SLOW_RPT_UDLR

; ***************************************************************************
; ***************************************************************************
;
; do_autorepeat - auto-repeat UDLR button presses, useful for menu screens.
;

do_autorepeat	.proc

		; Do auto-repeat processing on the d-pad.

		ldx	#MAX_PADS - 1

.loop:		lda	joynow, x		; Auto-Repeat UDLR buttons
		ldy	#SLOW_RPT_UDLR		; while they are held.
		and	#$F0
		beq	.set_delay
		dec	joyrpt, x
		bne	.no_repeat
		ora	joytrg, x
		sta	joytrg, x
		ldy	#FAST_RPT_UDLR
.set_delay:	tya
		sta	joyrpt, x

.no_repeat:	dex				; Check the next pad from the
		bpl	.loop			; multitap.

		leave				; All done, phew!

		.endp

	.if	(* >= $4000)			; If not running in RAM, then
		.bss				; put the variables in RAM.
	.endif

joyrpt:		ds	MAX_PADS

		.code

	.endif	SLOW_RPT_UDLR



; ***************************************************************************
; ***************************************************************************
;
; detect_ports - Detect #ports attached to the console.
;
; Returns: Y,A,Z-flag,N-flag = #ports attached: 0,1 or N if a multitap.
;
; N.B. Zero is returned if the number of ports cannot be determined.
;
; This is NOT a definitive test, because 2 people pressing the same button
; at the same instant will confuse it, and so will a button changing state
; in the middle of a read.
;
; The detect_1port() function is much safer function, but it should only be
; used to detect if there is NO multitap present (i.e. 1 port), because its
; detection of more-than-1-port suffers from the same problem when a joypad
; button changes state in the middle of a read.
;

	.if	MAX_PADS != 1

detect_ports	.proc

		ldy	num_ports		; Don't check again if we've
		cpy	#MAX_PADS		; already reduced the number
		bne	.finished		; of ports.

		bit	mouse_flg		; If NEC MultiTap was found
		bvs	.finished		; then all ports are unique.

		clx				; Check whether a button has
	.if	SUPPORT_6BUTTON			; just been pressed.
.chk_pressed:	lda	joy6trg, x
		and	#$0F
		ora	joytrg, x
		bne	.check_port1
	.else
.chk_pressed:	lda	joytrg, x
		bne	.check_port1
	.endif
		inx
		cpx	#MAX_PADS
		bcc	.chk_pressed

		cly				; No buttons just pressed so
		bra	.finished		; result is undetermined!

		; A button has been pressed, so check for mirrors.

	.if	SUPPORT_6BUTTON

.check_port1:	lda	joy6trg			; Has a button been pressed
		and	#$0F			; on port 1?
		ora	joytrg
		beq	.check_port2

		ldx	#1			; Test if port 1 is mirrored
		lda	joytrg			; as port 2,3,4 or 5.
.test_loop:	cmp	joytrg, x		
		bne	.next_test
		ldy	joy6trg, x
		cpy	joy6trg
		beq	.got_result
.next_test:	inx
		cpx	#MAX_PADS
		bcc	.test_loop

.check_port2:	lda	joy6trg + 2		; Has a button been pressed
		and	#$0F			; on port 2?
		ora	joytrg + 2
		beq	.got_result

		lda	joytrg + 2		; Test if port 2 is mirrored
		cmp	joytrg + 4		; as port 4.
		bne	.got_result
		lda	joy6trg + 2
		cmp	joy6trg + 4
		bne	.got_result
		ldx	#2			; It's a 2-Port multitap!

	.else

.test_port1:	lda	joytrg			; Has a button been pressed
		beq	.test_port2		; on port 1?

		ldx	#1			; Test if port 1 is mirrored
.test_loop:	cmp	joytrg, x		; as port 2,3,4 or 5.
		bne	.next_test
.next_test:	inx
		cpx	#MAX_PADS
		bcc	.test_loop

.test_port2:	lda	joytrg + 2		; Has a button been pressed
		beq	.got_result		; on port 2?

		cmp	joytrg + 4		; Test if port 2 is mirrored
		bne	.got_result		; as port 4.
		lda	joynow + 2
		cmp	joynow + 4
		bne	.got_result
		ldx	#2			; It's a 2-Port multitap!

	.endif

.got_result:	sxy				; Return # of multitap ports.
;		cpy	#1
;		bne	.finished
;		sty	num_ports		; Limit future joypad reads.

.finished:	leave				; All done, phew!

		.endp

	.endif	MAX_PADS != 1



; ***************************************************************************
; ***************************************************************************
;
; detect_1port - Detect if no multitap is attached to the console.
;
; Returns: Y,A,Z-flag,N-flag = #ports attached: 0,1 or N if a multitap.
;
; N.B. Zero is returned if the number of ports cannot be determined.
;
; This is a much better function than detect_ports(), but it should only be
; used to detect if there is NO multitap present (i.e. 1 port), because its
; detection of more-than-1-port suffers from the same problem when a joypad
; button changes state in the middle of a read.
;

	.if	MAX_PADS != 1

detect_1port	.proc

		ldy	num_ports		; Don't check again if we've
		cpy	#MAX_PADS		; already reduced the number
		bne	.finished		; of ports.

		bit	mouse_flg		; If NEC MultiTap was found
		bvs	.finished		; then all ports are unique.

		clx				; Test if port 1 is mirrored
		lda	joytrg			; as port 2,3,4 and 5.
.loop:		cmp	joytrg, x
		bne	.different
	.if	SUPPORT_6BUTTON
		ldy	joy6trg, x
		cmp	joy6trg
		bne	.different
	.endif	SUPPORT_6BUTTON
		inx
		cpx	#MAX_PADS
		bcc	.loop

	.if	SUPPORT_6BUTTON
		tya				
		and	#$0F			
		ora	joytrg
	.endif	SUPPORT_6BUTTON
		tay				; If no buttons just pressed
		beq	.finished		; then report "0".

		ldy	#1			; One port if all identical!
		sty	num_ports		; Limit future joypad reads.

.finished:	leave

.different:	ldy	#MAX_PADS		; If not identical, then there
		bra	.finished		; is most-likely a multitap.

		.endp

	.endif	MAX_PADS != 1
