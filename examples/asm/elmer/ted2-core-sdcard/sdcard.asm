; ***************************************************************************
; ***************************************************************************
;
; sdcard.asm
;
; SDcard read/write example using the "CORE(not TM)" PC Engine library code.
;
; Copyright John Brandwood 2022.
;
; Distributed under the Boost Software License, Version 1.0.
; (See accompanying file LICENSE_1_0.txt or copy at
;  http://www.boost.org/LICENSE_1_0.txt)
;
; ***************************************************************************
; ***************************************************************************
;
; This is an example of a simple HuCARD ROM that uses the "CORE(not TM)"
; library to provide interrupt-handling and joypad reading.
;
; It is running on a HuCARD, with Turbo Everdrive support, so the setup is
; a bit more complicated than usual!
;
; The PC Engine's memory map is set to ...
;
;   MPR0 = bank $FF : PCE hardware
;   MPR1 = bank $F8 : PCE RAM with Stack & ZP
;   MPR2 = bank $02 : HuCard ROM
;   MPR3 = bank $03 : HuCard ROM
;   MPR4 = bank $04 : HuCard ROM
;   MPR5 = bank $05 : HuCard ROM
;   MPR6 = bank $xx : HuCard ROM banked ASM library procedures.
;   MPR7 = bank $02 : HuCard ROM with "CORE(not TM)" kernel and IRQ vectors.
;
; ***************************************************************************
; ***************************************************************************

		;
		; Do you want to overwrite OS.PCE with a new version of TEOS?
		;
		; !!! DO NOT SET THIS !!!
		;
		; IT IS HERE TO SHOW HOW EASY IS IS TO WRITE TO A FILE, BUT
		; ACTUALLY OVERWRITING OS.PCE CAN MAKE YOUR TED2 UNUSABLE!
		;

UPDATE_TEOS	=	0

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
		; You would normally just set this in your project's local
		; "core-config.inc", but this shows another way to achieve
		; the same result.
		;

SUPPORT_TED2	=	1	; Support the Turbo Everdrive's use of bank0.

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
		include	"ted2.asm"		; TED2 hardware routines.
		include	"crc32.asm"		; CRC32 routines.
		include	"tty.asm"		; Useful TTY print routines.



; ***************************************************************************
; ***************************************************************************
;
; Constants and Variables.
;

		.bss

crc_value:	ds	4			; CRC of OS.PCE

		.code



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
		sta	<_si + 0
		lda	#>my_font
		sta	<_si + 1
		ldy	#^my_font

		call	dropfnt8x8_vdc		; Upload font to VRAM.

		; Upload the palette data to the VCE.

		stz	<_al			; Start at palette 0 (BG).
		lda	#1			; Copy 1 palette of 16 colors.
		sta	<_ah
		lda	#<cpc464_colors		; Set the ptr to the palette
		sta	<_si + 0		; data.
		lda	#>cpc464_colors
		sta	<_si + 1
		ldy	#^cpc464_colors
		call	load_palettes		; Add to the palette queue.

		call	xfer_palettes		; Transfer queue to VCE now.

		; Turn on the BG & SPR layers, then wait for a soft-reset.

		call	set_dspon		; Enable background.

		call	wait_vsync		; Wait for vsync.

		; Execute the SDcard and MB128 example code.

		call	sdcard_example		;

		; Wait for user to reboot.

.hang:		call	wait_vsync_usb		; Wait for vsync, or xfer.

		bra	.hang			; Wait for user to reboot.



; ***************************************************************************
; ***************************************************************************
;
; sdcard_example - 
;
; Uses: _bp = Pointer to directory entry in cache.
;
; Returns: X = F32_OK (and Z flag) or an error code.
;

sdcard_example	.proc				; Let PCEAS move this code!

		PRINTF	"\x1B<\eP0\eXL1\nInitializing Turbo Everdrive v2.\n"

		; Unlock the TED2 hardware, which takes over bank $00.

		call	unlock_ted2		; Unlock the TED2 hardware.
		beq	.ted2_ok

.ted2_fail:	PRINTF	"Turbo Everdrive v2 not found!\n"
		jmp	.finished

.ted2_ok:	PRINTF	"Turbo Everdrive v2 found and unlocked!\n"

		lda	#1			; Use the 2nd bank as RAM now
		tam2				; that we know this is a TED2.

		; Initialize and mount the SDcard.

		PRINTF	"Initializing SD card.\n"

		call	f32_mount_vol		; Mount the SDcard FAT-32.
		beq	.fat32_ok

.fat32_fail:	PRINTF	"FAT32 parition not found!\n"
		jmp	.finished

.fat32_ok:	PRINTF	"FAT32 parition mounted.\n\n"

		; Select "/TBED/" directory.

		PRINTF	"Selecting SDcard /TBED/ directory.\n"

		call	f32_select_root		; Start back at the root.
		bne	.cd_fail

		lda	#<.dirname		; Locate the 'TBED' folder.
		sta.l	<_si
		lda	#>.dirname
		sta.h	<_si
		ldy	#^.dirname
		call	f32_find_name
		bne	.cd_fail

		call	f32_change_dir		; Select the directory.
		beq	.cd_ok

.cd_fail:	PRINTF	"Directory change failed!\n"
		jmp	.finished

.dirname:	db	"TBED",0

.cd_ok:		PRINTF	"Directory selected.\n"

	.if	UPDATE_TEOS

		; Open the "OS.PCE" file.

		PRINTF	"Writing file /TBED/OS.PCE from memory.\n"

		lda	#<.filename		; Locate the 'OS.PCE' file.
		sta.l	<_si
		lda	#>.filename
		sta.h	<_si
		ldy	#^.filename
		call	f32_find_name		; Locate the named file in
		bne	.write_fail		; the current directory.

		call	f32_open_file		; Open the file, after which
		bne	.write_fail		; f32_cache_buf free for use!

		; Overwrite the "OS.PCE" file from memory.

		lda	#32768 / 512		; Or look at f32_file_length!
		sta.l	<_ax			; Lo-byte of # blocks in file.
		stz.h	<_ax			; Hi-byte of # blocks in file.

		lda	#bank(teos_image)	; First unused bank, reported
		tam3				; by PCEAS!
		stz.l	<_bx			; Load into MPR3, which will
		lda	#$60			; autoincrement the bank# as
		sta.h	<_bx			; necessary.

		call	f32_file_write		; Read the file into memory.
		bne	.write_fail

		call	f32_close_file		; Close the file & set N & Z.
		beq	.write_ok

.write_fail:	pha				; Preserve error code.
		call	f32_close_file		; Close the file & set N & Z.

		PRINTF	"File write failed!\n"
		pla				; Restore error code.
		jmp	.finished

.write_ok:	PRINTF	"File write complete.\n"

	.endif	UPDATE_TEOS

		; Open the "OS.PCE" file.

		PRINTF	"Reading file /TBED/OS.PCE into memory.\n"

		lda	#<.filename		; Locate the 'OS.PCE' file.
		sta.l	<_si
		lda	#>.filename
		sta.h	<_si
		ldy	#^.filename
		call	f32_find_name		; Locate the named file in
		bne	.read_fail		; the current directory.

		call	f32_open_file		; Open the file, after which
		bne	.read_fail		; f32_cache_buf free for use!

		; Read the "OS.PCE" file into memory.

		lda	#32768 / 512		; Or look at f32_file_length!
		sta.l	<_ax			; Lo-byte of # blocks in file.
		stz.h	<_ax			; Hi-byte of # blocks in file.

		lda	#_bank_base + _nb_bank	; First unused bank, reported
		tam3				; by PCEAS!
		stz.l	<_bx			; Load into MPR3, which will
		lda	#$60			; autoincrement the bank# as
		sta.h	<_bx			; necessary.

		call	f32_file_read		; Read the file into memory.
		bne	.read_fail

		call	f32_close_file		; Close the file & set N & Z.
		beq	.read_ok

.read_fail:	pha				; Preserve error code.
		call	f32_close_file		; Close the file & set N & Z.

		PRINTF	"File read failed!\n"
		pla				; Restore error code.
		jmp	.finished

.filename:	db	"OS.PCE",0

.read_ok:	PRINTF	"File read complete.\n"

		; Checksum the "OS.PCE" file.

		PRINTF	"Calculating CRC32 of file in memory.\n"

		call	init_crc32		; Initialize the CRC32 tables.

		lda.h	#32768
		stz.l	<_ax			; L-byte of #bytes to checksum.
		sta.h	<_ax			; M-byte of #bytes to checksum.
		stz	<_ax + 2		; H-byte of #bytes to checksum. 
		stz.l	<_si			; Checksum 32KB from $6000.
		lda	#$60
		sta.h	<_si
		ldy	#_bank_base + _nb_bank	; Bank#, set by PCEAS!
		call	calc_crc32		; Calculate the CRC32.

		tii	_cx, crc_value, 4

		PRINTF	"The CRC32 of \"OS.PCE\" is $%0lx.\n\n", crc_value

		; Display what version of OS.PCE it is.

		clx				; Compare the CRC32 against a
.os_loop:	lda	crc_value + 0		; list of known OS versions.
		cmp	.byte0, x
		bne	.next_os
		lda	crc_value + 1
		cmp	.byte1, x
		bne	.next_os
		lda	crc_value + 2
		cmp	.byte2, x
		bne	.next_os
		lda	crc_value + 3
		cmp	.byte3, x
		beq	.show_os
.next_os:	inx
		cpx	#(.byte1 - .byte0)
		bcc	.os_loop

.show_os:	lda	.string_lo, x		; Self-modify the PRINTF
		sta.l	.msg_param		; STRING parameter with
		lda	.string_hi, x		; the result.
		sta.h	.msg_param

		PRINTF	.msg_string		; Display the string.

		; All Done!

.finished:	leave				; Exit the procedure.

		; CRC32 $8B4E444A = Krikzz OS.PCE Version 1
		; CRC32 $2AE10C6C = Krikzz OS.PCE Version 2
		; CRC32 $BDD9D4C3 = TEOS Beta1
		; CRC32 $5C7E1E84 = TEOS Beta2
		; CRC32 $0F9AA6BD = TEOS Beta3
		; CRC32 $8D5B4CCC = TEOS Beta4
		; CRC32 $6311C68D = TEOS Beta5
		; CRC32 $0E307498 = TEOS-GT Beta5
		; CRC32 $EF3B1CFF = TEOS 3.01 Release
		; CRC32 $4556851F = TEOS-GT 3.01 Release

.msg_string:	STRING	"You are running \"%s\"!\n"
.msg_param:	dw	0

.str_osv1:	db	"Krikzz OS.PCE Version 1",0
.str_osv2:	db	"Krikzz OS.PCE Version 2",0
.str_tob1:	db	"TEOS Beta 1",0
.str_tob2:	db	"TEOS Beta 2",0
.str_tob3:	db	"TEOS Beta 3",0
.str_tob4:	db	"TEOS Beta 4",0
.str_tob5:	db	"TEOS Beta 5",0
.str_tgb5:	db	"TEOS-GT Beta 5",0
.str_tor1:	db	"TEOS 3.01 Release",0
.str_tgr1:	db	"TEOS-GT 3.01 Release",0
.str_unkn:	db	"An Unknown OS.PCE!",0

.string_lo:	dwl	.str_osv1
		dwl	.str_osv2
		dwl	.str_tob1
		dwl	.str_tob2
		dwl	.str_tob3
		dwl	.str_tob4
		dwl	.str_tob5
		dwl	.str_tgb5
		dwl	.str_tor1
		dwl	.str_tgr1
		dwl	.str_unkn

.string_hi:	dwh	.str_osv1
		dwh	.str_osv2
		dwh	.str_tob1
		dwh	.str_tob2
		dwh	.str_tob3
		dwh	.str_tob4
		dwh	.str_tob5
		dwh	.str_tgb5
		dwh	.str_tor1
		dwh	.str_tgr1
		dwh	.str_unkn

.byte0:		db	<($8B4E444A >>  0)
		db	<($2AE10C6C >>  0)
		db	<($BDD9D4C3 >>  0)
		db	<($5C7E1E84 >>  0)
		db	<($0F9AA6BD >>  0)
		db	<($8D5B4CCC >>  0)
		db	<($6311C68D >>  0)
		db	<($0E307498 >>  0)
		db	<($EF3B1CFF >>  0)
		db	<($4556851F >>  0)

.byte1:		db	<($8B4E444A >>  8)
		db	<($2AE10C6C >>  8)
		db	<($BDD9D4C3 >>  8)
		db	<($5C7E1E84 >>  8)
		db	<($0F9AA6BD >>  8)
		db	<($8D5B4CCC >>  8)
		db	<($6311C68D >>  8)
		db	<($0E307498 >>  8)
		db	<($EF3B1CFF >>  8)
		db	<($4556851F >>  8)

.byte2:		db	<($8B4E444A >> 16)
		db	<($2AE10C6C >> 16)
		db	<($BDD9D4C3 >> 16)
		db	<($5C7E1E84 >> 16)
		db	<($0F9AA6BD >> 16)
		db	<($8D5B4CCC >> 16)
		db	<($6311C68D >> 16)
		db	<($0E307498 >> 16)
		db	<($EF3B1CFF >> 16)
		db	<($4556851F >> 16)

.byte3:		db	<($8B4E444A >> 24)
		db	<($2AE10C6C >> 24)
		db	<($BDD9D4C3 >> 24)
		db	<($5C7E1E84 >> 24)
		db	<($0F9AA6BD >> 24)
		db	<($8D5B4CCC >> 24)
		db	<($6311C68D >> 24)
		db	<($0E307498 >> 24)
		db	<($EF3B1CFF >> 24)
		db	<($4556851F >> 24)

		.endp				; sdcard_example



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
;  $0 = transparent
;  $1 = dark blue shadow
;  $2 = white font
;
;  $4 = dark blue background
;  $5 = light blue shadow
;  $6 = yellow font
;

none		=	$000

		align	2

cpc464_colors:	defpal	$000,$001,$662,none,$002,$114,$551,none
		defpal	none,none,none,none,none,none,none,none



; ***************************************************************************
; ***************************************************************************
;
; It's the font data, nothing exciting to see here!
;

my_font:	incbin	"font8x8-ascii-bold-short.dat"



; ***************************************************************************
; ***************************************************************************
;
; Include the image to write to OS.PCE ... this is dangerous!
;
; N.B. This *MUST* be page-aligned for the lo-level MB128 routines!
;

	.if	UPDATE_TEOS

		align	8192

teos_image:	incbin	"TEOS.PCE"

	.endif	UPDATE_TEOS
