; ***************************************************************************
; ***************************************************************************
;
; sherlock-ted2.asm
;
; Wrapper for sherlock-hack.asm to save the results to the TED2 afterwards.
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
; This is a SuperCD overlay program that contains Sherlock's hacked CD game
; code so that Sherlock's intro video can be profiled and logged on a Turbo
; Everdrive v2, with the log then written to the file /TBED/SHERLOCK.DAT on
; the Everdrive's SDcard.
;
; The file /TBED/SHERLOCK.DAT *must* already exist on the SDcard before you
; can run this program (copy the one in this directory to your SDcard)!
;
; To use this program ...
;
; 1) Put the US release of the "Sherlock Holmes Consulting Detective" CD in
;    your PC Engine's CD-ROM drive.
;
; 2) Use your Turbo Everdrive's USB uploader software to upload and run the
;    modified "ted2usbcd.pce" System Card image to your PC Engine.
;    This is a hacked System Card that uploads a CD overlay through USB.
;    Please see the "../ted2-bios-usbcd" directory for how to create it.
;
; 3) After "ted2usbcd.pce" has initialized the CD it will prompt you to use
;    your Turbo Everdrive's USB uploader software to upload a CD overlay.
;    Upload the "sherlock-ted2.ovl".
;
; 4) Sherlock's intro movie will play, and then the data will be saved over
;    the current contents of the /TBED/SHERLOCK.DAT file on your SDcard.
;    You can confirm that the data was saved correctly by comparing the CRC
;    values displayed on the screen.
;
; 5) Turn off the PC Engine and copy the SHERLOCK.DAT file from SDcard onto
;    your PC.
;
; 6) Run the "dat2csv" program in this directory to convert SHERLOCK.DAT to
;    a human-readable SHERLOCK.CSV spreadsheet file.
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

SAVED_PC	.set	*			; Remember core_main address.

		.org	$FFFC			; Overwrite CORE NMI vector
		dw	main_nmi		; which is normally unused.

		.org	SAVED_PC		; Restore core_main address.

		;
		; Execute Sherlock's game code now.
		;

core_main:	lda	#$6A			; Results stored in
		tam6				; bank $6A..$81

		lda	$C000			; Has the Sherlock
		cmp	#$FF			; code already run?
		bne	main_save

		sei				; The log data is cleared
		ldx	#$FF			; so exec Sherlock's code
		txs				; to create the log data.

		lda	#$81			; Copy Sherlock's
		tam2				; code to bank $F8.
		tii	$4A00, $2A00, $4000 - $2A00

		lda	#$FF			; Clear the Sherlock code
		sta	$4A00			; out of bank $81.
		tii	$4A00, $4A01, $6000 - $4A01

		tii	ram_goto_game, $2100, len_goto_game
		jmp	$2100

ram_goto_game:	lda	#$82
		tam2
		inc	a
		tam3
		inc	a
		tam4
		inc	a
		tam5
		inc	a
		tam6
		cla
		tam7
		bit	VDC_SR
		cli
		jmp	$2A00			; Execute game code.

len_goto_game	=	* - ram_goto_game

		;
		; Sherlock's video has finished, so it executes this program's
		; NMI vector to return control here so we can save the results.
		;

main_nmi:	stz	core_ram1st		; Clean restart CORE kernel.
		jmp	core_boot

		;
		; Sherlock's video has finished, so now do a normal startup and
		; then save the results to SDcard.
		;

main_save:	; Turn the display off and initialize the screen mode.

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

		call	dropfnt8x8_vdc		; Upload font to VRAM.

		; Upload the palette data to the VCE.

		stz	<_al			; Start at palette 0 (BG).
		lda	#1			; Copy 1 palette of 16 colors.
		sta	<_ah
		lda	#<cpc464_colors		; Set the ptr to the palette
		sta	<_bp + 0		; data.
		lda	#>cpc464_colors
		sta	<_bp + 1
		ldy	#^cpc464_colors
		call	load_palettes		; Add to the palette queue.

		call	xfer_palettes		; Transfer queue to VCE now.

		; Turn on the BG & SPR layers, then wait for a soft-reset.

		call	set_dspon		; Enable background.

		call	wait_vsync		; Wait for vsync.

		; Reset the CD-ROM, because Sherlock leaves it in a mess!

		system	cd_reset

		; Save Sherlock's log data to the SDcard.

		jsr	save_data		;

		; Wait for user to reboot.

		lda	ted2_unlocked		; If running on a TED2, then
		bne	.hang_usb		; allow for a USB upload.

.hang:		call	wait_vsync		; Wait for vsync, or xfer.

		bra	.hang			; Wait for user to reboot.

.hang_usb:	call	wait_vsync_usb		; Wait for vsync, or xfer.

		bra	.hang_usb		; Wait for user to reboot.



; ***************************************************************************
; ***************************************************************************
;
; save_data - Save the data in memory to /TBED/SHERLOCK.DAT on the SDcard
;

LOG_BANK	=	$6A
LOG_SIZE	=	($82 - $6A) * $2000

save_data:	PRINTF	"\x1B<\eP0\eXL1\nInitializing Turbo Everdrive v2.\n"

		; Unlock the TED2 hardware, which takes over bank $00.

		call	unlock_ted2		; Unlock the TED2 hardware.
		beq	.ted2_ok

.ted2_fail:	PRINTF	"Turbo Everdrive v2 not found!\n"
		jmp	.finished

.ted2_ok:	PRINTF	"Turbo Everdrive v2 found and unlocked!\n"

		; Checksum the log data.

		PRINTF	"Calculating CRC32 of data.\n"

		call	init_crc32		; Initialize the CRC32 tables.

		lda.l	#LOG_SIZE
		sta.l	<_ax			; L-byte of #bytes to checksum.
		lda.h	#LOG_SIZE
		sta.h	<_ax			; M-byte of #bytes to checksum.
		lda	#LOG_SIZE >> 16
		sta	<_ax + 2		; H-byte of #bytes to checksum. 
		stz.l	<_bp			; Checksum the log data.
		lda	#$60
		sta.h	<_bp
		ldy	#LOG_BANK
		call	calc_crc32		; Calculate the CRC32.

		tii	_cx, crc_value, 4

		PRINTF	"The CRC32 of the data is $%0lx.\n\n", crc_value

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
		sta.l	<_bp
		lda	#>.dirname
		sta.h	<_bp
		ldy	#^.dirname
		call	f32_find_name
		bne	.cd_fail

		call	f32_change_dir		; Select the directory.
		beq	.cd_ok

.cd_fail:	PRINTF	"Directory change failed!\n"
		jmp	.finished

.dirname:	db	"TBED",0

.cd_ok:		PRINTF	"Directory selected.\n"

		; Open the "SHERLOCK.DAT" file.

		PRINTF	"Writing file /TBED/SHERLOCK.DAT from memory.\n"

		lda	#<.filename		; Locate the file.
		sta.l	<_bp
		lda	#>.filename
		sta.h	<_bp
		ldy	#^.filename
		call	f32_find_name		; Locate the named file in
		bne	.write_fail		; the current directory.

		call	f32_open_file		; Open the file, after which
		bne	.write_fail		; f32_cache_buf free for use!

		; Overwrite the "SHERLOCK.DAT" file from memory.

		lda.l	#LOG_SIZE / 512		; Or look at f32_file_length!
		sta.l	<_ax			; Lo-byte of # blocks in file.
		lda.h	#LOG_SIZE / 512
		sta.h	<_ax			; Hi-byte of # blocks in file.

		lda	#LOG_BANK		; First unused bank, reported
		tam3				; by PCEAS!
		stz.l	<_bx			; Save from MPR3, which will
		lda.h	#$6000			; autoincrement the bank# as
		sta.h	<_bx			; necessary.

		call	f32_file_write		; Write the file from memory.
		bne	.write_fail

		call	f32_close_file		; Close the file & set N & Z.
		beq	.write_ok

.write_fail:	pha				; Preserve error code.
		call	f32_close_file		; Close the file & set N & Z.

		PRINTF	"File write failed!\n"
		pla				; Restore error code.
		jmp	.finished

.write_ok:	PRINTF	"File write complete.\n"

		; Open the "SHERLOCK.DAT" file.

		PRINTF	"Reading file /TBED/SHERLOCK.DAT into memory.\n"

		lda	#<.filename		; Locate the file.
		sta.l	<_bp
		lda	#>.filename
		sta.h	<_bp
		ldy	#^.filename
		call	f32_find_name		; Locate the named file in
		bne	.read_fail		; the current directory.

		call	f32_open_file		; Open the file, after which
		bne	.read_fail		; f32_cache_buf free for use!

		; Read the "SHERLOCK.DAT" file into memory.

		lda.l	#LOG_SIZE / 512		; Or look at f32_file_length!
		sta.l	<_ax			; Lo-byte of # blocks in file.
		lda.h	#LOG_SIZE / 512
		sta.h	<_ax			; Hi-byte of # blocks in file.

		lda	#$40			; Read file into TED2 memory.
		tam3
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

.filename:	db	"SHERLOCK.DAT",0

.read_ok:	PRINTF	"File read complete.\n"

		; Checksum the log file.

		PRINTF	"Calculating CRC32 of file.\n"

		call	init_crc32		; Initialize the CRC32 tables.

		lda.l	#LOG_SIZE
		sta.l	<_ax			; L-byte of #bytes to checksum.
		lda.h	#LOG_SIZE
		sta.h	<_ax			; M-byte of #bytes to checksum.
		lda	#LOG_SIZE >> 16
		sta	<_ax + 2		; H-byte of #bytes to checksum. 
		stz.l	<_bp			; Checksum 32KB from $6000.
		lda	#$60
		sta.h	<_bp
		ldy	#$40			; Test file from TED2 memory.
		call	calc_crc32		; Calculate the CRC32.

		tii	_cx, crc_value, 4

		PRINTF	"The CRC32 of the file is $%0lx.\n\n", crc_value

		; All Done!

.finished:	rts



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
; Finally, include the hacked Sherlock CD boot-program image in the correct
; bank/address so that it can be executed just by paging it in to memory.
;

		.bank	$81 - $68		; Sherlock runs in bank $81.

		.org	$2A00
		incbin	"sherlock-hack.ovl"

		ds	$2000 - (* & $1FFF)	; Clear to end of bank.
