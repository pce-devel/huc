; ***************************************************************************
; ***************************************************************************
;
; hucc-old-scroll.asm
;
; Based on the original HuC and MagicKit functions by David Michel and the
; other original HuC developers.
;
; Modifications copyright John Brandwood 2024.
;
; Distributed under the Boost Software License, Version 1.0.
; (See accompanying file LICENSE_1_0.txt or copy at
;  http://www.boost.org/LICENSE_1_0.txt)
;
; ***************************************************************************
; ***************************************************************************
;
; HuC's old scrolling library is provided for use with existing HuC projects,
; but it should preferably be avoided in new projects because it is slow and
; forever limited by its choice to handle gaps between areas, and sorting.
;
; HuCC's new scrolling library puts the responibility for defining both gaps
; and the display order into the project's hands, but in return it runs much
; faster, using less than 1/6 of the processing time in VBLANK, and far less
; time in the RCR interrputs themselves. It also supports the 2nd SuperGRAFX
; screen layer, which HuC's old scrolling library ignores.
;
; ***************************************************************************
; ***************************************************************************

	; Signal that the old scrolling library is used.

HUCC_OLD_SCROLL	= 1

	; Use the old name to configure the #scroll regions.

	.ifndef	HUC_NUM_SCROLLS
HUC_NUM_SCROLLS = 4
	.endif

	.if	HUC_NUM_SCROLLS > 8
		.fail	Number of defined scrolls cannot exceed 8
	.endif

; [ 28] user scrolling vars
		.bss
scroll_top:	.ds	HUC_NUM_SCROLLS ; top
scroll_btm:	.ds	HUC_NUM_SCROLLS ; btm
scroll_crl:	.ds	HUC_NUM_SCROLLS ; control
scroll_xl:	.ds	HUC_NUM_SCROLLS ; x
scroll_xh:	.ds	HUC_NUM_SCROLLS ;
scroll_yl:	.ds	HUC_NUM_SCROLLS ; y
scroll_yh:	.ds	HUC_NUM_SCROLLS ; scrolling table

; [ 69] display list
		.bss

active_index	.ds	1
active_list	.ds	HUC_NUM_SCROLLS + 1

active_top	.ds	HUC_NUM_SCROLLS + 1
active_btm	.ds	HUC_NUM_SCROLLS
active_crl	.ds	HUC_NUM_SCROLLS
active_xl	.ds	HUC_NUM_SCROLLS
active_xh	.ds	HUC_NUM_SCROLLS
active_yl	.ds	HUC_NUM_SCROLLS
active_yh	.ds	HUC_NUM_SCROLLS

		.code

SCROLL_IRQ	=	$0C			; VDC CR for VBL and RCR IRQ.
HUC_SCR_HEIGHT	=	224
HUC_1ST_RCR	=	$144



; ***************************************************************************
; ***************************************************************************
;
; void __fastcall scroll( unsigned char num<_al>, unsigned int x<_cx>, unsigned int y<_dx>, unsigned char top<_ah>, unsigned char bottom<_bl>, unsigned char disp<acc> );
;
; set screen scrolling

_scroll.6:	ldy	<_al			; Region number.

		and	#$C0			; Flags (mark it as enabled).
		ora	#SCROLL_IRQ
		sta	scroll_crl, y

		lda	<_bl			; BTM+1 so that it's easier to
		inc	a			; calcute gaps between regions.
		sta	scroll_btm, y

		lda	<_ah			; TOP
		sta	scroll_top, y

		sec				; Either Y at top of the frame
		beq	!+			; or Y-1 because the RCR code
		clc				; sets it on the line before.
!:		lda.l	<_dx
		sbc	#0
		sta	scroll_yl, y
		lda.h	<_dx
		sbc	#0
		sta	scroll_yh, y

		lda.l	<_cx
		sta	scroll_xl, y
		lda.h	<_cx
		sta	scroll_xh, y

		rts



; ***************************************************************************
; ***************************************************************************
;
; void __fastcall scroll_disable( unsigned char num<acc> );
;
; disable screen scrolling for a scroll region

_scroll_disable.1:
		tay
		lda	scroll_crl, y
		and	#~SCROLL_IRQ
		sta	scroll_crl, y
		rts



; ***************************************************************************
; ***************************************************************************
;
; vbl_init_scroll
;
; N.B. Y of $FF is the first "set-in-vblank" region.
;
; Y ... $FF, $BF, $F0 for 2 regions at $00 and $C0.
;
; From Charles MacDonald's pcetech.txt ...
;
;  Raster Compare Register (RCR):
;
;  The range of the RCR is 263 lines, relative to the start of the active
;  display period. (defined by VSW, VDS, and VCR) The VDC treats the first
;  scanline of the active display period as $0040, so the valid ranges for
;  the RCR register are $0040 to $0146.
;
;  For example, assume VSW=$02, VDS=$17. This positions the first line of
;  the active display period at line 25 of the frame. An RCR value of $0040
;  (zero) causes an interrupt at line 25, and a value of $0146 (262) causes an
;  interrupt at line 24 of the next frame.
;
;  Any other RCR values that are out of range ($00-$3F, $147-$3FF) will never
;  result in a successful line compare.

vbl_init_scroll	.proc

		clx				; Active List index
		ldy	#$FF			; Scroll List index

.region_loop:	iny				; Scroll List index
		cpy	#HUC_NUM_SCROLLS
		bcs	.last_region

.next_region:	lda	scroll_crl, y		; Add all the enabled Scroll
		and	#SCROLL_IRQ		; regions to the Active list.
		beq	.region_loop

		lda	scroll_top, y		; Ignore the region if it is
		cmp	#HUC_SCR_HEIGHT		; off-screen.
		bcs	.region_loop

		phx				; Preserve Active List index.

.check_unique:	dex				; Check all Active List entries
		bmi	.is_unique		; to see if the top is the same.
		cmp	active_top, x
		bne	.check_unique

		plx				; This region has the same
		bra	.region_loop		; active_top as another!

.is_unique:	plx				; Restore Active List index.

		sta	active_top, x		; Copy top scanline.
		lda	scroll_btm, y		; Copy btm scanline.
		sta	active_btm, x
		lda	scroll_crl, y		; Copy control bits.
		sta	active_crl, x
		lda	scroll_xl, y		; Copy BAT coordinates.
		sta	active_xl, x
		lda	scroll_xh, y
		sta	active_xh, x
		lda	scroll_yl, y
		sta	active_yl, x
		lda	scroll_yh, y
		sta	active_yh, x

		txa				; Initialize the index.
		sta	active_list, x

		inx
		bra	.region_loop

.last_region:	lda	#HUC_SCR_HEIGHT		; Terminate active list.
		sta	active_top, x

		txa				; Initialize the index.
		sta	active_list, x

		pha				; Number of active regions.

		cmp	#2			; No sort needed if there are
		bcc	.sort_done		; less than 2 active regions.

		; Bubble-sort the order of indexes in the Active List.

.sort_pass:	sta	<__temp			; #regions left to sort.

		clv				; Reset swap flag.

		ldy	#1

.pass_loop:	ldx	active_list - 1, y	; Index of previous.
		lda	active_top, x

		ldx	active_list + 0, y	; Index of current.
		cmp	active_top, x
		bcc	.no_swap

		lda	active_list - 1, y	; Swap them in the list.
		sta	active_list + 0, y
		txa
		sta	active_list - 1, y

		bit	#$40			; Set V flag to signal a swap.

.no_swap:	iny				; Reached the end of the list?
		cpy	<__temp			; #regions left to sort.
		bcc	.pass_loop
		bvc	.sort_done		; Was there a swap last pass?

		lda	<__temp			; #regions left to sort.
		dec	a
		cmp	#2			; Is there only 1 active region
		bcs	.sort_pass		; left to sort?

.sort_done:	pla				; Number of active regions.
		beq	.finished		; No RCR if no active regions.

		; Set up the first RCR and scroll coordinates.

		ldx	active_list		; 1st region in active_list.
		lda	active_top, x		; Is 1st region top > line 0?
		beq	!+
		lda	#$FF			; Start display with a gap.
!:		sta	active_index		; Init RCR active_list index.

		ldy	active_xl, x		; Set VBL bg_x1 and bg_y1 from
		sty.l	bg_x1			; the 1st scroll region.
		ldy	active_xh, x
		sty.h	bg_x1
		ldy	active_yl, x
		sty.l	bg_y1
		ldy	active_yh, x
		sty.h	bg_y1

		smb7	<vdc_crl		; Ensure BURST MODE is off.

		st0	#VDC_RCR		; 1st RCR always happens just
		st1	#<HUC_1ST_RCR		; before the display starts.
		st2	#>HUC_1ST_RCR

.finished:	leave				; All done!

		.endp



; ***************************************************************************
; ***************************************************************************
;
; rcr_next_scroll
;
; BTM	189 GAP
; BTM+1 190 CMP TOP -> CC
; TOP	192 CMP BTM+1 -> CS
;
; BTM	190 GAP
; BTM+1 191 CMP TOP -> CC
; TOP	192 CMP BTM+1 -> CS
;
; BTM	191 NO GAP
; BTM+1 192 CMP TOP -> CS
; TOP	192 CMP BTM+1 -> CS & Z
;
; BTM	192 NO GAP
; BTM+1 193 CMP TOP -> CS
; TOP	192 CMP BTM+1 -> CC
;
; BTM	193 NO GAP
; BTM+1 194 CMP TOP -> CS
; TOP	192 CMP BTM+1 -> CC

;		;;;				; 8 (cycles for the INT)
;		bbs1	<irq_vec, .hook		; 8
;		jmp	[irq1_hook]		; 7
;
;irq1_handler:	pha				; 3 Save all registers.
;		phx				; 3
;		phy				; 3
;
;	.if	CDROM
;	.if	USING_MPR7
;		tma7				; 4 Preserve MPR7.
;		pha				; 3
;		lda	<core_1stbank		; 4 Allow users to put IRQ
;		tam7				; 5 handlers in CORE_BANK.
;	.endif
;	.endif	CDROM
;
;		lda	VDC_SR			; 6 Acknowledge the VDC's IRQ.
;		sta	<vdc_sr			; 4 Remember what caused it.
;
;		; HSYNC ?
;
;.check_hsync:	bbr2	<vdc_sr, .check_vsync	; 6 Is this an HSYNC interrupt?
;
;		bbr6	<irq_vec, .skip_hookh	; 6 Is a driver registered?
;
;		bsr	.user_hsync		; 8 Call game's HSYNC code.
;.user_hsync:	jmp	[hsync_hook]		; 7

hucc_rcr:	ldy	active_index		; 5 -1 if 1st region is > line 0.
		bmi	.top_is_blank		; 2

		ldx	active_list, y		; 5 X = region# of this RCR.
		iny				; 2 Remember index of next RCR.
		sty	active_index		; 5
		lda	active_list, y		; 5 Y = region# of next RCR.
		tay				; 2

		st0	#VDC_CR			; 5
		lda	active_crl, x		; 5 = 121 cycles from RCR
		sta	VDC_DL			; 6

		lda	active_top, y		; Is next region off-screen?
		cmp	#HUC_SCR_HEIGHT
		bcs	.end_of_screen

		cmp	active_btm, x		; Is there a gap before the
		beq	.set_next_rcr		; next region starts?
		bcs	.gap_next_rcr

.set_next_rcr:	st0	#VDC_RCR		; Set next RCR 1 line before
		clc				; the region begins.
		adc	#64-1
		sta	VDC_DL
		cla
		rol	a
		sta	VDC_DH

.set_scroll:	st0	#VDC_BXR
		lda	active_xl, x
		sta	VDC_DL
		lda	active_xh, x
		sta	VDC_DH

		st0	#VDC_BYR
		lda	active_yl, x
		sta	VDC_DL
		lda	active_yh, x
		sta	VDC_DH
		rts

.top_is_blank:	stz	active_index		; Start at top next RCR.

		st0	#VDC_CR			; Disable BG & SPR before the
		st1	#SCROLL_IRQ		; top of the screen.

		ldx	active_list		; Set 1st RCR when the region
		lda	active_top, x		; actually begins.
		bra	.set_next_rcr

.gap_next_rcr:	dec	active_index		; Repeat the current region.
		lda	#SCROLL_IRQ		; Disable BG & SPR in the gap.
		sta	active_crl, x

		lda	#HUC_SCR_HEIGHT		; No gap when region repeated.
		ldy	active_btm, x
		sta	active_btm, x
		tya				; Set next RCR at the start of
		bra	.set_next_rcr		; the gap.

.end_of_screen:	ldy	active_btm, x		; Does the current region stop
		cpy	#HUC_SCR_HEIGHT		; before the screen ends?
		bcc	.gap_next_rcr

		st0	#VDC_RCR		; Stop further RCRs by setting
		st1	#0			; a value that is never taken.
		st2	#0
		bra	.set_scroll
