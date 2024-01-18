; ***************************************************************************
; ***************************************************************************
;
; movie.asm
;
; Functions for developers to play HuVIDEO (or similar) on the PC Engine.
;
; This is derived from Hudson's original HuVIDEO player code in Gulliver Boy.
;
; Significant improvements have been made to change the buffering process in
; order to allow for 12fps video.
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
; HuVIDEO PLAYBACK TIMELINE FOR A 5 FRAME LONG 10FPS VIDEO
;
; IRQ2-HALF | IRQ1-RCR | IRQ1-VBL
; ------------------------------------------------------------------
; ......... | draw 0.0 |
; ......... | draw 0.1 |
; ......... | draw 0.2 |
; ......... | draw 0.3 |
; ......... | draw 0.4 |
; ......... | draw 0.5 | palette 0, display frame 0, start audio 0.0
; audio 0.1 | draw 0.0 |
; audio 0.2 | draw 0.1 |
; audio 0.3 | draw 0.2 |
; audio 0.4 | draw 0.3 |
; audio 0.5 | draw 0.4 |
; audio 1.0 | draw 0.5 | palette 0, display frame 0
; audio 1.1 | draw 1.0 |
; audio 1.2 | draw 1.1 |
; audio 1.3 | draw 1.2 |
; audio 1.4 | draw 1.3 |
; audio 1.5 | draw 1.4 |
; audio 2.0 | draw 1.5 | palette 1, display frame 1
; audio 2.1 | draw 2.0 |
; audio 2.2 | draw 2.1 |
; audio 2.3 | draw 2.2 |
; audio 2.4 | draw 2.3 |
; audio 2.5 | draw 2.4 |
; audio 3.0 | draw 2.5 | palette 2, display frame 2
; audio 3.1 | draw 3.0 |
; audio 3.2 | draw 3.1 |
; audio 3.3 | draw 3.2 |
; audio 3.4 | draw 3.3 |
; audio 3.5 | draw 3.4 |
; audio 4.0 | draw 3.5 | palette 3, display frame 3
; audio 4.1 | draw 4.0 |
; audio 4.2 | draw 4.1 |
; audio 4.3 | draw 4.2 |
; audio 4.4 | draw 4.3 |
; audio 4.5 | draw 4.4 |
; ......... | draw 4.5 | palette 4, display frame 4
;
; When the HuVIDEO player buffer wraps for the 1st time, video is paused for
; 5 ticks to let the audio catch up to the same frame!
;
; video wraps at irq_cnt $AE->$AF
; irq1 skipped $AF-$B0 skipped
; irq1 skipped $B0-$B1 skipped
; irq1 skipped $B1-$B2 skipped
; irq1 skipped $B2-$B3 skipped
; irq1 skipped, then audio wraps and releases rmb3, $B3-$B4 skipped
; irq1 draws 0, $B4-$B5 skipped
; irq1 draws 1, $B5-$B6 skipped
; irq1 draws 2, $B6-$B7 skipped
; irq1 draws 3, $B7-$B8 skipped
; irq1 draws 4, $B8-$B9 skipped
; irq1 draws 5, $B9-$BA flip
;
; ***************************************************************************
; ***************************************************************************
;
; Suggested video sizes for new 8-frames-per-second HuVIDEO data streams ...
;
;
;  256x104 (approximately 2.39x1)
;   32x13 = 416 CHR
;    $3400 tiles ($0770 bytes-per-tick * 7 -> $3410)
;    $0200 palettes
;    $00D0 tile palettes
;    $03E8 adpcm (1000 bytes, 2000 samples)
;    $0028 adpcm resync
;    =
;    $3AE0 -> $3B00 * 8 -> 118KB/s
;
;
;  224x120 (approximately 16x9)
;   28x15 = 420 CHR
;    $3480 tiles ($0780 bytes-per-tick * 7 -> $3480)
;    $0200 palettes
;    $00D2 tile palettes
;    $03E8 adpcm (1000 bytes, 2000 samples)
;    $0028 adpcm resync
;    =
;    $3B62 -> $3C00 * 8 -> 120KB/s
;
;
;  192x144 (approximately 4x3)
;   24x18 = 432 CHR
;    $3600 tiles ($07C0 bytes-per-tick * 7 -> $3640)
;    $0200 palettes
;    $00D8 tile palettes
;    $03E8 adpcm (1000 bytes, 2000 samples)
;    $0028 adpcm resync
;    =
;    $3CE8 -> $3D00 * 8 -> 122KB/s
;
;
; Suggested video sizes for new 10-frames-per-second HuVIDEO data streams ...
;
;
;  224x96 (approximately 2.39x1)
;   28x12 = 336 CHR
;    $2A00 tiles ($0700 bytes-per-tick * 6 -> $2A00)
;    $0200 palettes
;    $00A8 tile palettes
;    $0320 adpcm (800 bytes, 1600 samples)
;    $0028 adpcm resync
;    =
;    $2FF0 -> $3000 * 10 -> 120KB/s
;
;
;  192x112 (approximately 16x9)
;   24x14 = 336 CHR
;    $2A00 tiles ($0700 bytes-per-tick * 6 -> $2A00)
;    $0200 palettes
;    $00A8 tile palettes
;    $0320 adpcm (800 bytes, 1600 samples)
;    $0028 adpcm resync
;    =
;    $2FF0 -> $3000 * 10 -> 120KB/s
;
;
;  176x120 (approximately 4x3)
;   22x15 = 330 CHR
;    $2940 tiles ($06E0 bytes-per-tick * 6 -> $2940)
;    $0200 palettes
;    $00A5 tile palettes
;    $0320 adpcm (800 bytes, 1600 samples)
;    $0028 adpcm resync
;    =
;    $2F2D -> $3000 * 10 -> 120KB/s
;
;
; Suggested video sizes for new 12-frames-per-second HuVIDEO data streams ...
;
;
;  208x88 (approximately 2.39x1)
;   26x11 = 286 CHR
;    $23C0 tiles ($0730 bytes-per-tick * 5 -> $23F0)
;    $01E0 palettes **NOTE**
;    $008F tile palettes
;    $02A0 adpcm (669 bytes, 1338 samples)
;    $0028 adpcm resync
;    =
;    $28F7 -> $2900 * 12 -> 123KB/s
;
;
;  176x104 (approximately 16x9)
;   22x13 = 286 CHR
;    $23C0 tiles ($0730 bytes-per-tick * 5 -> $23F0)
;    $01E0 palettes **NOTE**
;    $008F tile palettes
;    $02A0 adpcm (669 bytes, 1338 samples)
;    $0028 adpcm resync
;    =
;    $28F7 -> $2900 * 12 -> 123KB/s
;
;
;  184x96 (approximately 16x9)
;   23x12 = 276 CHR
;    $2280 tiles ($06F0 bytes-per-tick * 5 -> $22B0)
;    $0200 palettes
;    $0090 tile palettes
;    $02A0 adpcm (669 bytes, 1338 samples)
;    $0028 adpcm resync
;    =
;    $27D8 -> $2800 * 12 -> 120KB/s
;
;
;  160x112 (approximately 4x3)
;   20x14 = 280 CHR
;    $2300 tiles ($0700 bytes-per-tick * 5 -> $2300)
;    $0200 palettes
;    $008C tile palettes
;    $02A0 adpcm (669 bytes, 1338 samples)
;    $0028 adpcm resync
;    =
;    $2854 -> $2900 * 12 -> 123KB/s
;
; ***************************************************************************
; ***************************************************************************

;
; Configure Library ...
;

	.ifndef	HUV_12FPS
HUV_12FPS	=	0			; Choose frame rate.
	.endif

	.ifndef	SUPPORT_HUVIDEO
SUPPORT_HUVIDEO	=	1
	.endif

;
; Include dependancies ...
;

		include "adpcm.asm"		; ADPCM functions.
		include "cdrom.asm"		; CDROM functions.



; ***************************************************************************
; ***************************************************************************
;
; Definitions
;

		; HuVIDEO File Header Offsets

HUV_HEADER	=	$6000			; Load the header in MPR3.
HUV_HEADER_BLKS	=	4			; #sectors in header.

		rsset	HUV_HEADER
HUV_SIGNATURE	rs	16      		; "HuVideo", then 9 x $00.
HUV_FRM_COUNT	rs	2			; #Frames in movie.
HUV_FRM_WIDTH	rs	2			; In pixels.
HUV_FRM_HEIGHT	rs	2			; In pixels.
HUV_FRM_COLORS	rs	1			; #Palettes.
HUV_MOVIE_FMT	rs	1			; 0=CHR, 1=SPR.
HUV_NUM_60THS	rs	1			; 5=12-FPS, 6=10-FPS.

		;

BUFFER_1ST_BANK	=	$6A
BUFFER_END_BANK	=	$88
BUFFER_BANKS	=	BUFFER_END_BANK - BUFFER_1ST_BANK
BUFFER_SECTORS	=	BUFFER_BANKS * 8192 / 2048

BUFFER_BANK	=	2

; ACD can buffer 42 frames @ $3000 = $7E000 = 504KB = 252 sectors.
; ACD can buffer 48 frames @ $2900 = $7B000 = 492KB = 246 sectors.
;
; SCD can buffer 20 frames @ $3000 = $3C000 = 240KB = 120 sectors.
; SCD can buffer 24 frames @ $2900 = $3D800 = 246KB = 123 sectors.

		;

	.if	HUV_12FPS
TICKS_PER_FRAME	=	5			; 12 FPS playback.
BYTES_PER_FRAME	=	$2900			; 123KB/second.
ADPCM_PER_FRAME	=	667			; #bytes at 12 FPS.

;BUFFER_FRAMES	=	(BUFFER_BANKS * 8192 / BYTES_PER_FRAME) ; = 23 @ 12fps = $3AF00
;BUFFER_FRAMES	=	(BUFFER_BANKS * 8192 / BYTES_PER_FRAME) ; = 24 @ 12fps = $3AF00

	.else
TICKS_PER_FRAME	=	6			; 10 FPS playback.
BYTES_PER_FRAME	=	$3000			; 120KB/second.
ADPCM_PER_FRAME	=	800			; #bytes at 10 FPS.

;BUFFER_FRAMES	=	(BUFFER_BANKS * 8192 / BYTES_PER_FRAME) ; = 20 @ 10fps = $3C000
	.endif

		;

HUV_BAT_X	=	4			; Playback X
HUV_BAT_Y	=	7			; Playback Y
HUV_BAT_W	=	24			; Playback W
HUV_BAT_H	=	14			; Playback H
HUV_BAT_CHR	=	(HUV_BAT_W * HUV_BAT_H)	; #tiles

;
;
;

; External Variables

abort_mask	=	$2044	; bit2 = 1 if user-abort is allowed
exit_code	=	$2B7E

;
; ZP Variables ...
;

;
; huv_state:
;
; bit 0 - 0 Buffer not loaded, set to 1 when data is available.
; bit 1 - 1 VIDEO frame drawn and ready to display.
; bit 2 - 1 AUDIO playing, 0 start AUDIO when frame ready.
; bit 3 - 1 AUDIO wrapped buffer, resync VIDEO.
; bit 4 - 1 VIDEO wrapped buffer early, pause VIDEO.
; bit 5 - 1 Ready to start fade down.
; bit 6 - 1 Ready to update fade down.
; bit 7 - UNUSED!
;

		; Put the workspace in zero-page.

		rsset	$2080

huv_stack_save:	rs	1

		; Movie information.

huv_movie_lba_h:rs	1	;
huv_movie_lba_m:rs	1	;
huv_movie_lba_l:rs	1	;

huv_movie_end_h:rs	1	;
huv_movie_end_m:rs	1	;
huv_movie_end_l:rs	1	;

huv_movie_frms:	rs	2	; Total #frames in movie.

huv_pxl_offset:	rs	2	; = $8000 for Gulliver
huv_pal_offset:	rs	2	; = $AA00 for Gulliver
huv_bat_offset:	rs	2	; = $AC00 for Gulliver
huv_pcm_offset:	rs	2	; = $ACA8 for Gulliver
huv_ini_offset:	rs	2	; = $AFC8 for Gulliver

		; Buffer management.

huv_block_cnt:	rs	2	; #sectors left to read in movie.

huv_read_len	=	_al	; Parameters for cdr_read_ram.
huv_read_lba_h	=	_cl	; These parameters are preserved by the call
huv_read_lba_m	=	_ch	; to cdr_read_ram, unlike the System Card!
huv_read_lba_l	=	_dl

		;

huv_abort_vec:	rs	2	;

huv_state:	rs	1	;

huv_play_frame:	rs	2	; Current frame number (playing).
huv_load_frame:	rs	2	; Current frame number (loading).

huv_load_pages:	rs	1	; #pages of buffer data loaded.

huv_fade_ptr:	rs	2	;

		;

huv_audio_bank:	rs	1	; Bank# of buffered audio frame.
huv_audio_page:	rs	1	; Page# of buffered audio frame ($60xx-$7Fxx).
huv_audio_tick:	rs	1	; Audio output step (0-5 for 10fps).

huv_audio_pcm:	rs	2	; Buffer pointer for PCM data output.

huv_video_bank:	rs	1	; Bank# of buffered video frame.
huv_video_page:	rs	1	; Page# of buffered video frame ($60xx-$7Fxx).
huv_video_tick:	rs	1	; Video output step (0-5 for 10fps).

huv_video_pxl:	rs	2	; Buffer pointer for PXL data output.
huv_video_bat:	rs	2	; Buffer pointer for BAT data output.

huv_pxl_vram:	rs	2	; Current VRAM addr to write PXL data.
huv_bat_vram:	rs	2	; Current VRAM addr to write BAT data.
huv_bat_tile:	rs	2	; Current VRAM tile# to write to BAT.

		;

huv_bat_x:	rs	1	; Playback movie at BAT X.
huv_bat_y:	rs	1	; Playback movie at BAT Y.

huv_rcr_top:	rs	2	; top playback line with RCR offset, ONLY EVER SET!
huv_rcr_btm:	rs	2	; btm playback line with RCR offset, ONLY USED ONCE!

huv_bat_y_count:rs	1	; Temporary while updating the BAT.
huv_bat_x_count:rs	1	; Temporary while updating the BAT.

		;

	.if	SUPPORT_TIMING
huv_buffer_fill:rs	1	; #Sectors into buffer when refill starts.
huv_buffer_full:rs	1	; #Sectors into buffer when refill complete.
huv_buffer_1st:	rs	1	; #ticks to load 1st sector in a refill.
huv_buffer_rest:rs	1	; #ticks to load rest of buffer in a refill.
	.endif	SUPPORT_TIMING

		.code



; ***************************************************************************
; ***************************************************************************
;
; Entry point for HuVideo playback.
;

play_huvideo:	lda	#HUV_BAT_X		; Set movie location
		sta	<huv_bat_x
		lda	#HUV_BAT_Y
		sta	<huv_bat_y

		lda	<huv_bat_y		; Calculate the BAT VRAM addr
		stz.l	huv_screen_bat		; for the 1st and 2nd screen.
		lsr	a			; Assumes a 64x32 BAT.
		ror.l	huv_screen_bat
		lsr	a
		ror.l	huv_screen_bat
		sta.h	huv_screen_bat
		sta.h	huv_screen_bat + 2
		lda	<huv_bat_x
		and	#$1F
		ora.l	huv_screen_bat
		sta.l	huv_screen_bat
		ora	#$20
		sta.l	huv_screen_bat + 2

	.if	1
		lda	<_al
		asl	a
		asl	a
		tax
		lda	gulliver_lba + 0, x	; lba_l
		sta	<_dl
		lda	gulliver_lba + 1, x	; lba_m
		sta	<_ch
		lda	gulliver_lba + 2, x	; lba_h
		sta	<_cl
	.else
		ldx	<_al
		jsr	get_file_info
	.endif

		tsx
		stx	huv_stack_save

		stz.l	<huv_movie_frms
		stz.h	<huv_movie_frms

		; Play the movie at the LBA location huv_movie_lba_l/huv_movie_lba_m/huv_movie_lba_h.

		stz	<huv_state		; Initialize HuVideo state.

		jsr	huv_proc_header		; Process movie header.

		jsr	huv_enable_rcr

		stz.l	<huv_play_frame		; Play from frame#.
		stz.h	<huv_play_frame
		jsr	huv_play_from

		call	adpcm_reset

		stz	<irq_vec

		jsr	set_rcroff
;		jsr	set_dspoff

exit_huvideo:	jsr	core_clr_hooks
		jsr	wait_vsync

		cly
		tya

		rts



; ***************************************************************************
; ***************************************************************************
;
; Check for user-abort button (RUN on 2nd joypad)
;

huv_chk_abort:	jsr	huv_read_pad			; supports a 6-button pad.

		tst	#$04, <abort_mask		; is skip allowed?
		beq	.done

		tst	#JOY_RUN, joytrg + 1		; RUN on (joytrg+1)
		bne	.abort

.done:		rts

.abort:		stz	<irq_vec			; disable IRQ hooks
		cli

	.if	SUPPORT_ADPCM				;
		call	cdr_ad_stop
	.else
		call	adpcm_stop
	.endif

		jsr	set_rcroff

		ldx	<huv_stack_save			; restore saved stack
		txs

		jmp	exit_huvideo



; ***************************************************************************
; ***************************************************************************
;
; Read and process the movie's header sector.
;

huv_proc_header:lda	#BUFFER_1ST_BANK	; Map the workspace into MPR3.
		tam3

		lda.l	#HUV_HEADER		; Read the 1st movie sector
		sta.l	<_bx			; from the LBA that was set
		lda.h	#HUV_HEADER		; previously.
		sta.h	<_bx
		lda	#1
		sta	<_al
		call	cdr_read_ram
		bne	huv_proc_header		; Just try again if error.

		clc				; Save the LBA of 1st frame.
		lda	#HUV_HEADER_BLKS
		adc	<_dl
		sta	<huv_movie_lba_l
		cla
		adc	<_ch
		sta	<huv_movie_lba_m
		cla
		adc	<_cl
		sta	<huv_movie_lba_h

		lda.l	HUV_FRM_COUNT		; Save the #frames in
		sta.l	<huv_movie_frms		; movie.
		sta.l	<_cx
		lda.h	HUV_FRM_COUNT
		sta.h	<huv_movie_frms
		sta.h	<_cx

		jsr	huv_frm_to_lba		; Calc the movie end.
		tii	huv_read_lba_h, huv_movie_end_h, 3

		lda	HUV_FRM_COLORS		; HUV_FRM_COLORS (0..16)
		stz.h	huv_vce_tia + 5		; = #palettes per frame
		asl	a			; This self-mods the TIA
		asl	a			; length in huv_irq1_vbl.
		asl	a
		asl	a
		rol.h	huv_vce_tia + 5
		asl	a
		rol.h	huv_vce_tia + 5
		sta.l	huv_vce_tia + 5

		stz.l	<huv_pxl_offset		; Calc frame offset of PXL.
		stz.h	<huv_pxl_offset

		lda.l	#(HUV_BAT_CHR * 32)	; Calc frame offset of PAL.
		sta.l	<huv_pal_offset
		ldx.h	#(HUV_BAT_CHR * 32)
		stx.h	<huv_pal_offset

		clc				; Calc frame offset of BAT.
		adc.l	huv_vce_tia + 5
		sta.l	<huv_bat_offset
		sax
		adc.h	huv_vce_tia + 5
		sta.h	<huv_bat_offset
		sax

		clc				; Calc frame offset of PCM.
		adc.l	#(HUV_BAT_W * HUV_BAT_H) / 2
		sta.l	<huv_pcm_offset
		sax
		adc.h	#(HUV_BAT_W * HUV_BAT_H) / 2
		sta.h	<huv_pcm_offset
		sax

		clc				; HuVIDEO stores APDCM fade-in
		adc.l	#ADPCM_PER_FRAME	; data after the audio bytes.
		sta.l	<huv_ini_offset
		sax
		adc.h	#ADPCM_PER_FRAME
		sta.h	<huv_ini_offset

		rts



; ***************************************************************************
; ***************************************************************************
;
; huv_frm_to_lba - Calculate a frame's LBA and page-offset into the sector.
;
; ARGS: _cx = frame#
;

huv_frm_to_lba:	lda.h	#BYTES_PER_FRAME	; A frame is page-aligned so
		sta	<_al    		; we don't need the lo-byte.
		call	mul_16x8u		; 16-bit x 8-bit -> 24-bit

		lda	<_ax + 0		; Preserve page-offset into
		and	#7			; the sector.
		pha

		ldx	#3			; Divide by 8 for the sector
!:		lsr	<_ax + 2		; offset.
		ror	<_ax + 1
		ror	<_ax + 0
		dex
		bne	!-

		clc				; Add the movie starting LBA
		lda	<_ax + 0		; and save the result in the
		adc	<huv_movie_lba_l	; cdr_read_ram params, i.e.
		sta	<huv_read_lba_l		; the normal _cl, _ch, _dl,
		lda	<_ax + 1		; but with a name that makes
		adc	<huv_movie_lba_m	; the code easier to read.
		sta	<huv_read_lba_m
		lda	<_ax + 2
		adc	<huv_movie_lba_h
		sta	<huv_read_lba_h

		pla				; Return page-offset.
		rts



; ***************************************************************************
; ***************************************************************************
;
; Play movie from huv_play_frame to end of movie.
;

huv_play_from:	stz.l	bg_x1			; Reset screen flip.
		stz.h	bg_x1

		stz	<huv_state
		stz	<huv_video_tick

		lda.l	<huv_play_frame		; Get starting frame's LBA.
		sta.l	<huv_load_frame
		sta.l	<_cx
		lda.h	<huv_play_frame
		sta.h	<huv_load_frame
		sta.h	<_cx
		jsr	huv_frm_to_lba

		and	#7			; Convert the page-offset to
		pha				; be a negative value to add
		eor	#$FF			; to the #sectors loaded, so
		inc	a			; that the #frames loaded is
		sta	<huv_load_pages		; trackable.
		pla
		ora.h	#$6000			; Convert the page-offset to
		sta	<huv_audio_page		; it's playback buffer addr.
		sta	<huv_video_page
		sta.h	<huv_video_pxl		; Checked by buffer reload!

		lda	#BUFFER_1ST_BANK
		sta	<huv_audio_bank
		sta	<huv_video_bank

		sec				; Calculate the #sectors left
		lda	<huv_movie_end_l	; until the end-of-file.
		sbc	<_dl			; The 16-bit count allows for
		sta.l	<huv_block_cnt		; a 15-minute movie.
		lda	<huv_movie_end_m
		sbc	<_ch
		sta.h	<huv_block_cnt

		; Loop around, continually reloading the buffer.

.reload_buffer:	lda.l	<huv_block_cnt		; #sectors to read
		cmp.l	#BUFFER_SECTORS		; = min( BUFFER_SECTORS, huv_block_cnt )
		lda.h	<huv_block_cnt
		sbc.h	#BUFFER_SECTORS
		lda	#BUFFER_SECTORS
		bcs	!+
		lda.l	<huv_block_cnt
!:		sta	<huv_read_len

	.if	0
		call	cdr_test_bnk		; TEST BACK-TO-BACK CD SPEED!
	.else
		call	cdr_stream_ram		; Refill the buffer.
		bne	.final_frames		; Stop on errors.

		smb0	<huv_state		; Signal buffered data ready.
	.endif

		lda	<huv_load_pages		; Acknowledge the last sectors
		cmp.h	#BYTES_PER_FRAME	; in the read, now we are sure
		bcc	!+			; they loaded OK.
		sei				; Disable interrupts while
		sbc.h	#BYTES_PER_FRAME	; changing huv_load_frame!
		sta	<huv_load_pages
		inc.l	<huv_load_frame
		bne	!+
		inc.h	<huv_load_frame
!:		cli

	.if	SUPPORT_TIMING
		jsr	.find_position		; Track when buffer fill was
		sta	<huv_buffer_full	; completed.
		jsr	huv_show_stats		; Display the timing stats.
	.endif	SUPPORT_TIMING

		clc				; Update LBA for next buffer
		lda	<huv_read_len		; load.
		adc	<huv_read_lba_l
		sta	<huv_read_lba_l
		cla
		adc	<huv_read_lba_m
		sta	<huv_read_lba_m
		cla
		adc	<huv_read_lba_h
		sta	<huv_read_lba_h

		lda.l	<huv_block_cnt		; Have we loaded up all of
		sec				; sectors in the movie?
		sbc	<huv_read_len
		sta.l	<huv_block_cnt
		bcs	!+
		dec.h	<huv_block_cnt
!:		ora.h	<huv_block_cnt
		beq	.final_frames		; Branch if finished loading.

;		jmp	.reload_buffer		; TEST BACK-TO-BACK CD SPEED!

.wait_to_reload:jsr	.find_position
		cmp	#30     		; Start the reload when we're
		bcc	.wait_to_reload		; xx sectors or more into the
		sta	<huv_buffer_fill	; buffer.
		jmp	.reload_buffer

		;

.final_frames:
;		bbs4	<huv_state, .done_playing

		lda.l	<huv_play_frame		; Wait while there are still
		cmp.l	<huv_load_frame		; frames to play.
		lda.h	<huv_play_frame
		sbc.h	<huv_load_frame
		bcc	.final_frames
;		bbs2	<huv_state, .final_frames; Wait for AUDIO to stop.
		lda	<huv_video_tick		; Wait for VIDEO to stop.
		bne	.final_frames

		;

.done_playing:	jsr	set_rcroff

		setvec	irq2_hook, huv_irq1_vbl	; ???

		rmb0	<huv_state		; Signal buffer empty.
		smb5	<huv_state		; Trigger fade down.

		sei
	.if	SUPPORT_ADPCM			;
		call	cdr_ad_stop
	.else
		call	adpcm_stop
	.endif
		cli

		lda	#8			; fade out the screen
		sta	.step_count

.fade_out:	lda	#8
		sta	.step_delay

.wait_step:	lda	irq_cnt
.wait_vbl:	cmp	irq_cnt
		beq	.wait_vbl

		dec	.step_delay
		bne	.wait_step

		jsr	huv_fade_colors

		dec	.step_count
		bpl	.fade_out
		rts

.step_count:	db	0			; #steps in fade
.step_delay:	db	0			; delay per step

.find_position:	sec				; Calculate how many sectors
		lda	<huv_video_bank		; into the buffer that video
		sbc	#BUFFER_1ST_BANK	; data is being read from.
		asl	a
		asl	a
		sta	<_temp
		sec
		lda.h	<huv_video_pxl
		sbc.h	#$6000
		lsr	a
		lsr	a
		lsr	a
		clc
		adc	<_temp
		rts

;
; Calculate RCR line and enable RCR interrupt.
;

huv_enable_rcr:	jsr	wait_vsync

		lda	#VDC_RCR
		sta	<vdc_reg
		st0	#VDC_RCR

		lda	<huv_bat_y		; convert CHR to PXL
		asl	a
		asl	a
		asl	a
		clc
		adc	#64			; RCR IRQ starts at 64
		sta	VDC_DL
		stz	VDC_DH

		setvec	irq1_hook, huv_irq1_rcr

		smb1	<irq_vec

		rts



	.if	SUPPORT_TIMING

; ***************************************************************************
; ***************************************************************************
;
; huv_show_stats - Hack to display the 4 timing stats on the screen.
;

huv_show_stats:	php
		sei

		lda	scsi_stat_time + 0	;
		sta	<huv_buffer_1st
		eor	#$FF
		sec
		adc	scsi_stat_time + BUFFER_SECTORS - 1
		sta	<huv_buffer_rest

		clx
		cly

.byte_loop:	st0	#VDC_MAWR
		lda	.where, y
		sta	VDC_DL
		iny
		lda	.where, y
		sta	VDC_DH
		iny
		st0	#VDC_VWR

		lda	<huv_buffer_fill, x
		lsr	a
		lsr	a
		lsr	a
		lsr	a
		ora.l	#CHR_ZERO + '0'
		cmp.l	#CHR_ZERO + '0' + 10
		bcc	.skip_lo
		adc	#6
.skip_lo:	sta	VDC_DL
		st2.h	#CHR_ZERO + (15 << 12)

		lda	<huv_buffer_fill, x
		and	#$0F
		ora.l	#CHR_ZERO + '0'
		cmp.l	#CHR_ZERO + '0' + 10
		bcc	.skip_hi
		adc	#6
.skip_hi:	sta	VDC_DL
		st2.h	#CHR_ZERO + (15 << 12)

		tya
		bit	#2
		bne	.byte_loop

		inx
		cpx	#4
		bne	.byte_loop

		plp
		rts

.where:		dw	28 + (23 * 64), 60 + (23 * 64)
		dw	28 + (24 * 64), 60 + (24 * 64)
		dw	28 + (25 * 64), 60 + (25 * 64)
		dw	28 + (26 * 64), 60 + (26 * 64)

	.endif	SUPPORT_TIMING



; ***************************************************************************
; ***************************************************************************
;
; HuVideo IRQ1 - RCR
;
; This should be located in permanently-accessible memory!
;

huv_irq1_rcr:	pha
		phx
		phy

		lda	VDC_SR

		bbr0	<huv_state, .rcr_done	; Wait until buffer loaded.
		bbs4	<huv_state, .rcr_done	; Pause to sync to audio?

	.if	1
		lda.l	<huv_play_frame		; Are we trying to play a
		cmp.l	<huv_load_frame		; frame that has not been
		lda.h	<huv_play_frame		; loaded yet?
		sbc.h	<huv_load_frame
		bcc	.check_tick

		stz	<huv_video_tick		; Buffer starvation!
	.else
		bra	.check_tick
	.endif

.rcr_done:	lda.l	#huv_irq1_vbl
		sta.l	irq1_hook
		lda.h	#huv_irq1_vbl
		sta.h	irq1_hook

		ply
		plx
		pla
		rti

		; Upload the graphics for this tick.

.check_tick:	lda	<huv_video_tick		; Is this the 1st tick of a
		bne	.update_video		; new frame?

.new_frame:	lda.l	<huv_play_frame		; Are we trying to play a
		cmp.l	<huv_load_frame		; frame that has not been
		lda.h	<huv_play_frame		; loaded yet?
		sbc.h	<huv_load_frame
		bcs	.rcr_done

		lda.h	bg_x1			; Find the screen that is not
		eor	#1			; being displayed.
		asl	a
		tax

		lda.l	huv_screen_pxl, x	; Choose the screen to draw.
		sta.l	<huv_pxl_vram
		lda.h	huv_screen_pxl, x
		sta.h	<huv_pxl_vram

		lda.l	huv_screen_bat, x
		sta.l	<huv_bat_vram
		lda.h	huv_screen_bat, x
		sta.h	<huv_bat_vram

		lda.l	huv_screen_chr, x
		sta.l	<huv_bat_tile
		lda.h	huv_screen_chr, x
		sta.h	<huv_bat_tile

		clc
		lda.l	<huv_pxl_offset
		sta.l	<huv_video_pxl
		lda.h	<huv_pxl_offset
		adc	<huv_video_page
		sta.h	<huv_video_pxl

		clc
		lda.l	<huv_bat_offset
		sta.l	<huv_video_bat
		lda.h	<huv_bat_offset
		adc	<huv_video_page
		sta.h	<huv_video_bat

		lda	<huv_video_bank		; Map the video data to MPR3,
		tam3				; MPR4, and MPR5.
		inc	a
		cmp	#BUFFER_END_BANK
		bcc	!+
		lda	#BUFFER_1ST_BANK
!:		tam4
		inc	a
		cmp	#BUFFER_END_BANK
		bcc	!+
		lda	#BUFFER_1ST_BANK
!:		tam5

.update_video:	jsr	huv_update_chr
		jsr	huv_update_bat

		lda	<huv_video_tick		; Have we finished drawing a
		inc	a			; new frame?
		cmp	#TICKS_PER_FRAME
		bne	!+

		smb1	<huv_state		; Signal frame OK to display.
		cla

!:		sta	<huv_video_tick
		jmp	.rcr_done

		;

huv_screen_pxl:	dw	$5000			; 1st screen PXL address.
		dw	$6800			; 2nd screen PXL address.

huv_screen_chr:	dw	$5000 / 16		; 1st screen CHR number.
		dw	$6800 / 16		; 2nd screen CHR number.

huv_screen_bat:	dw	0			; 1st screen BAT address.
		dw	0			; 2nd screen BAT address.



; ***************************************************************************
; ***************************************************************************
;
; HuVideo IRQ1 - VBLANK
;

huv_irq1_vbl:	pha
		phx
		phy

		lda	VDC_SR

.chk_begin_fade:bbr5	<huv_state, .chk_fading	; Start a fade down?

		call	huv_get_colors		; Get palette from VCE.
		jmp	.finished

.chk_fading:	bbr6	<huv_state, .chk_playing; Continue a fade down?
		beq	.chk_playing

		call	huv_put_colors		; Put updated VCE palette.
		jmp	.finished

		; Have we finished drawing a frame?
		;
		; NZ after RCR when the frame finishes drawing.

.chk_playing:	bbs1	<huv_state, .frame_ready
		jmp	.vbl_done

.frame_ready:	rmb1	<huv_state

		tma3
		pha
		tma4
		pha
		tma5
		pha

		lda	<huv_video_bank
		tam3
		inc	a
		cmp	#BUFFER_END_BANK
		bcc	!+
		lda	#BUFFER_1ST_BANK
!:		tam4
		inc	a
		cmp	#BUFFER_END_BANK
		bcc	!+
		lda	#BUFFER_1ST_BANK
!:		tam5

		clc
		lda.l	<huv_pal_offset
		sta.l	huv_vce_tia + 1
		lda.h	<huv_pal_offset
		adc	<huv_video_page
		sta.h	huv_vce_tia + 1

		stz	VCE_CTA + 0		; Update HuVideo palette.
		stz	VCE_CTA + 1

huv_vce_tia	=	*			; Self-modifying code!
		tia	$8000, VCE_CTW, $0200	; #palettes per frame.

		pla
		tam5
		pla
		tam4
		pla
		tam3

.flip_display:	st0	#VDC_BXR		; Swap screen to display.
		stz	VDC_DL
		lda.h	bg_x1
		eor	#1
		sta.h	bg_x1
		sta	VDC_DH

		; Do we need to start/restart audio playback?

		bbs2	<huv_state, .next_frame	; Is AUDIO playing?

.begin_adpcm:	smb2	<huv_state		; Signal AUDIO playing.
		jsr	huv_reset_adpcm		; Reset ADPCM, init IRQ2.
		jsr	huv_start_adpcm		; Write first ADPCM and start.

		bra	.vbl_done		; Repeat the video frame.

		; Con

.next_frame:	bbs3	<huv_state, .sync_frame	; Has audio just wrapped?

		ldx	<huv_video_bank		; Advance the video buffer ptr
		lda	<huv_video_page		; by one frame.
		inx				; This is at-least 1 bank, and
		clc				; sometimes 2 banks.
		adc.h	#BYTES_PER_FRAME - $2000
		bpl	!+
		inx				; Advance a 2nd bank.
		adc.h	#$E000			; Put the ptr back in MPR3.
!:		sta	<huv_video_page
		sta.h	<huv_video_pxl		; Checked by buffer reload!
		stx	<huv_video_bank
		cpx	#BUFFER_END_BANK	; Do we need to wrap the bank
		bcc	.vbl_done		; back to the buffer start?

		; The video has wrapped before the audio, so delay the video
		; to let the audio catch up.
		;
		; This keeps the audio and video in sync over time.

		txa
		sbc	#BUFFER_BANKS		; Preserves C flag!
		sta	<huv_video_bank

		smb4	<huv_state		; VIDEO wrapped before AUDIO!
;		rmb3	<huv_state		; Clear signal from AUDIO task.

		bra	.vbl_done

		; If the audio has wrapped, the video needs to catch up.
		;
		; This keeps the audio and video in sync over time.

.sync_frame:	ldx	<huv_audio_bank		; Resync video frame to audio
		lda	<huv_audio_page		; frame.
		stx	<huv_video_bank
		sta	<huv_video_page

		rmb3	<huv_state		; Clear signal from AUDIO task.

;		stz	<huv_video_tick		; Restart VIDEO drawing.

		;

.vbl_done:	call	huv_chk_abort		; execute the abort check

.skip_check:	lda.l	#huv_irq1_rcr		; huv_irq1_rcr
		sta.l	irq1_hook
		lda.h	#huv_irq1_rcr
		sta.h	irq1_hook

.finished:	inc	irq_cnt			; keep clock ticking

		ply
		plx
		pla
		rti



; ***************************************************************************
; ***************************************************************************
;
; Update HuVideo CHR (which are $2A00 bytes)
;
; $0700 bytes = $2A00 bytes / 6
;
; Note: TIA of 1KB takes  8328 cycles if no sprites. = 19 scanlines
; Note: TIA of 1KB takes 10010 cycles if 16 sprites. = 22 scanlines
;

huv_update_chr:	st0	#VDC_MAWR

		clc
		lda.l	<huv_pxl_vram
		sta	VDC_DL
		adc.l	#$0700 / 2
		sta.l	<huv_pxl_vram
		lda.h	<huv_pxl_vram
		sta	VDC_DH
		adc.h	#$0700 / 2
		sta.h	<huv_pxl_vram

		st0	#VDC_VWR

		clc
		lda.l	<huv_video_pxl
		sta	.selfmod + 1
		adc.l	#$0700
		sta.l	<huv_video_pxl
		lda.h	<huv_video_pxl
		sta	.selfmod + 2
		adc.h	#$0700
		sta.h	<huv_video_pxl

.selfmod:	tia	$0000, VDC_DL, $0700

		rts



; ***************************************************************************
; ***************************************************************************
;
; Update HuVideo BAT
;

huv_update_bat:	lda.l	<huv_video_bat		; self-mod address!
		sta	.bat_pair + 1
		lda.h	<huv_video_bat
		sta	.bat_pair + 2

		ldx	<huv_video_tick
		lda	.lines_per_step, x
		sta	<huv_bat_y_count
		cly

.bat_line:	st0	#VDC_MAWR
		lda.l	<huv_bat_vram
		sta	VDC_DL
		lda.h	<huv_bat_vram
		sta	VDC_DH
		st0	#VDC_VWR

		lda	#HUV_BAT_W
		sta	<huv_bat_x_count
		ldx.l	<huv_bat_tile

.bat_pair:	lda	$0000, y
		iny
		pha
		and	#$F0
		ora.h	<huv_bat_tile
		stx	VDC_DL
		sta	VDC_DH
		inx
		bne	!+
		inc.h	<huv_bat_tile

!:		pla
		asl	a
		asl	a
		asl	a
		asl	a
		ora.h	<huv_bat_tile
		stx	VDC_DL
		sta	VDC_DH
		inx
		bne	!+
		inc.h	<huv_bat_tile

!:		dec	<huv_bat_x_count
		dec	<huv_bat_x_count
		bne	.bat_pair

		stx.l	<huv_bat_tile

		lda.l	<huv_bat_vram
		clc
		adc	#64			; BAT width
		sta.l	<huv_bat_vram
		bcc	!+
		inc.h	<huv_bat_vram

!:		dec	<huv_bat_y_count
		bne	.bat_line

		tya
		clc
		adc.l	<huv_video_bat
		sta.l	<huv_video_bat
		bcc	.finished
		inc.h	<huv_video_bat

.finished:	rts

.lines_per_step:db	2, 3, 2, 3, 2, 2



; ***************************************************************************
; ***************************************************************************
;
; huv_start_adpcm -
;

huv_start_adpcm:jsr	huv_first_adpcm

		stz	<huv_audio_tick

		lda	huv_pcm_per_frm + 0
		lsr	a
		tax
		ldy	#$80
		bsr	set_adpcm_reg
		bsr	set_adpcm_sz

		lda	#14			; 16KHz playback
		sta	IFU_ADPCM_SPD
		lda	#IFU_INT_HALF
		sta	IFU_IRQ_MSK
		lda	#ADPCM_PLAY
		sta	IFU_ADPCM_CTL
		rts



; ***************************************************************************
; ***************************************************************************
;
; huv_stop_adpcm -
;

huv_stop_adpcm:	rmb2	<huv_state		; Signal AUDIO stopped.

		call	adpcm_reset
		rts

;

set_adpcm_sz:	lda	#ADPCM_SET_SZ
		tsb	IFU_ADPCM_CTL
		lda	#ADPCM_SET_SZ

!:		trb	IFU_ADPCM_CTL
		rts

set_adpcm_reg:	stx	IFU_ADPCM_LSB
		sty	IFU_ADPCM_MSB
		rts



; ***************************************************************************
; ***************************************************************************
;
; huv_first_adpcm -
;

huv_first_adpcm:clc
		lda.l	<huv_ini_offset
		sta.l	<huv_audio_pcm
		lda.h	<huv_ini_offset
		adc	<huv_audio_page
		sta.h	<huv_audio_pcm

		ldx	#$28
		bsr	huv_write_adpcm

		clc
		lda.l	<huv_pcm_offset
		adc	#$28
		sta.l	<huv_audio_pcm
		lda.h	<huv_pcm_offset
		adc	#$00
		adc	<huv_audio_page
		sta.h	<huv_audio_pcm

		lda	huv_pcm_per_frm + 0
		sec
		sbc	#$28
		tax
;		bra	huv_write_adpcm



; ***************************************************************************
; ***************************************************************************
;
; huv_write_adpcm -
;

huv_write_adpcm:tma3
		pha
		tma4
		pha
		tma5
		pha

		lda	<huv_audio_bank		; Map the audio data to MPR3,
		tam3				; MPR4, and MPR5.
		inc	a
		cmp	#BUFFER_END_BANK
		bcc	!+
		lda	#BUFFER_1ST_BANK
!:		tam4
		inc	a
		cmp	#BUFFER_END_BANK
		bcc	!+
		lda	#BUFFER_1ST_BANK
!:		tam5

		cly
.write_loop:	lda	[huv_audio_pcm], y
.write_wait:	tst	#ADPCM_WR_BSY, IFU_ADPCM_FLG
		bne	.write_wait		; 30-cycles-per-byte is the
		sta	IFU_ADPCM_DAT		; maximum safe rate without
		iny				; triggering a BSY delay.
		dex
		bne	.write_loop

		tya
		clc
		adc.l	<huv_audio_pcm
		sta.l	<huv_audio_pcm
		bcc	.finished
		inc.h	<huv_audio_pcm

.finished:	pla
		tam5
		pla
		tam4
		pla
		tam3

		rts



; ***************************************************************************
; ***************************************************************************
;
; huv_reset_adpcm -
;

huv_reset_adpcm:call	adpcm_reset

.wait_busy:	call	adpcm_stat
		ora	#$00
		bne	.wait_busy

		setvec	irq2_hook, huv_irq2	; IRQ2 vector

		smb0	<irq_vec		; Enable IRQ2 vector.
		rts



; ***************************************************************************
; ***************************************************************************
;
; HuVideo IRQ2
;
; Check the ADPCM hardware playback status
;
; bit 2 ($04) - INTHALF	- ADPCM buffer at less than 50% left
; bit 3 ($08) - INTSTOP - ADPCM finished
; bit 4 ($10) - INTSUB	- ???
; bit 5 ($20) - INTMSG	- CD data finished
; bit 6 ($40) - INTDAT	- CD data ready
;

huv_irq2:	pha				; Ignore everything except an
		lda	IFU_IRQ_MSK		; IFU_INT_HALF from the ADPCM
		and	IFU_IRQ_FLG		; output.
		bit	#IFU_INT_HALF
		bne	.got_int_half

		pla
		rti

.got_int_half:	phx
		phy

		lda.l	<huv_play_frame
		cmp.l	<huv_load_frame
		lda.h	<huv_play_frame
		sbc.h	<huv_load_frame
		bcc	.pcm_playing

.pcm_starved:	jsr	huv_stop_adpcm		; rmb2 <huv_state
		bra	.irq2_done		; calls ad_reset()

.pcm_playing:	ldx	<huv_audio_tick		; 0..5 out of 6
		inx
		cpx	#TICKS_PER_FRAME
		bcc	.update_pcm

		; Increment audio playback to next frame.

.next_frame:	inc.l	<huv_play_frame		; Increment the current frame
		bne	!+			; (playing).
		inc.h	<huv_play_frame

!:		ldx	<huv_audio_bank		; Advance the audio buffer ptr
		inx				; by one frame.
		clc				; This is at-least 1 bank, and
		lda	<huv_audio_page		; sometimes 2 banks.
		adc.h	#BYTES_PER_FRAME - $2000
		bpl	!+
		inx				; Advance a 2nd bank.
		adc.h	#$E000			; Put the ptr back in MPR3.
!:		sta	<huv_audio_page
		txa				; Do we need to wrap the bank
		cmp	#BUFFER_END_BANK	; back to the buffer start?
		bcc	.done
		sbc	#BUFFER_BANKS		; Preserves C flag!

		smb3	<huv_state		; Signal AUDIO wrapped.
		rmb4	<huv_state		; Unpause VIDEO if waiting.

.done:		sta	<huv_audio_bank

		ldx.l	<huv_play_frame		; If we've reached the end of
		cpx.l	<huv_movie_frms		; the movie, stop writing any
		lda.h	<huv_play_frame		; new ADPCM.
		sbc.h	<huv_movie_frms
		bcs	.pcm_finished

		cpx.l	<huv_load_frame		; If we've caught up with the
		lda.h	<huv_play_frame		; loading, then panic!
		sbc.h	<huv_load_frame
		bcs	.pcm_starved

		clc
		lda.l	<huv_pcm_offset
		sta.l	<huv_audio_pcm
		lda.h	<huv_pcm_offset
		adc	<huv_audio_page
		sta.h	<huv_audio_pcm

		clx				; 1st tick of new frame.
.update_pcm:	stx	<huv_audio_tick
		lda	huv_pcm_per_frm, x
		tax

		jsr	huv_write_adpcm

.irq2_done:	ply
		plx
		pla
		rti

.pcm_finished:	lda	#IFU_INT_HALF
		trb	IFU_IRQ_MSK
		bra	.irq2_done

		;

	.if	HUV_12FPS
		; 667 bytes over 5 frames.
huv_pcm_per_frm:db	134, 134, 133, 133, 133
	.else
		; 800 bytes over 6 frames.
huv_pcm_per_frm:db	134, 134, 133, 133, 133, 133
	.endif



; ***************************************************************************
; ***************************************************************************
;
; Called during VBL to get color palette before a fade-out.
;

huv_get_colors:	stz	VCE_CTA + 0
		stz	VCE_CTA + 1
		tai	VCE_CTR, huv_vce_colors, $0200

		rmb5	<huv_state		; Signal palette stored.
		smb6	<huv_state		; Trigger next fade.

		lda	#$01
		sta	huv_vce_saved
		rts



; ***************************************************************************
; ***************************************************************************
;
; Called during VBL to put color palette during a fade-out.
;

huv_put_colors:	lda	huv_vce_saved
		beq	.skip

		stz	VCE_CTA + 0
		stz	VCE_CTA + 1
		tia	huv_vce_colors, VCE_CTW, $0200
		stz	huv_vce_saved
.skip:		rts



; ***************************************************************************
; ***************************************************************************
;
; huv_fade_colors -
;

huv_fade_colors:lda.l	#huv_vce_colors
		sta.l	<huv_fade_ptr
		lda.h	#huv_vce_colors
		sta.h	<huv_fade_ptr

;		jsr	.fade_page		; Fade 512 colors.
;		inc.h	<huv_fade_ptr
;		jsr	.fade_page
;		inc.h	<huv_fade_ptr

		jsr	.fade_page		; Fade 256 colors.
		inc.h	<huv_fade_ptr
		jsr	.fade_page

		inc	huv_vce_saved
		rts

		;

.fade_page:	cly

.test_b:	lda	[huv_fade_ptr], y	; Is BLUE non-zero?
		bit	#$07
		beq	.test_r

.fade_b:	lda	[huv_fade_ptr], y	; Fade down a step.
		dec	a
		sta	[huv_fade_ptr], y

.test_r:	bit	#$38			; Is RED non-zero?
		beq	.test_g

.fade_r:	lda	[huv_fade_ptr], y	; Fade down a step.
		sec
		sbc	#$08
		sta	[huv_fade_ptr], y

.test_g:	iny				; Is GREEN non-zero?
		bit	#$C0
		bne	.fade_g
		lda	[huv_fade_ptr], y
		bit	#$01
		beq	.next_color

.fade_g:	dey				; Fade down a step.
		lda	[huv_fade_ptr], y
		sec
		sbc	#$40
		sta	[huv_fade_ptr], y
		iny
		lda	[huv_fade_ptr], y
		sbc	#$00
		sta	[huv_fade_ptr], y

.next_color:	iny
		bne	.test_b
		rts

huv_vce_colors:	ds	$400			; 00
huv_vce_saved:	ds	1			; 00



; ***************************************************************************
; ***************************************************************************
;
; Joypad read (only reads port #1, puts it in joypad #2!)
;
; Handles 6-button joypad OK.
;

huv_read_pad:	lda	#$01
		;	...
;.soft_reset:	lda	#$03
;		sta	<_al
;		;system	psg_bios
;
;		;system	cd_boot
;
;		stz	$5582

		rts
