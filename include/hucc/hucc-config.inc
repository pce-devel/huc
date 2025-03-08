; ***************************************************************************
; ***************************************************************************
;
; hucc-config.inc
;
; Configuration settings for the HuCC projects"CORE(not TM)" PC Engine library code.
;
; Copyright John Brandwood 2021-2024.
;
; Distributed under the Boost Software License, Version 1.0.
; (See accompanying file LICENSE_1_0.txt or copy at
;  http://www.boost.org/LICENSE_1_0.txt)
;
; ***************************************************************************
; ***************************************************************************
;
; The idea is that you, a PCE developer, copy this file from the ../include/
; directory and into your project's directory, and then customize the values
; to suit your particular project.
;
; Because PCEAS searches the current (i.e. project) directory for an include
; file first, then it will find your customized copy of the file rather than
; the original copy in the ../include/ directory.
;
; That means that all of the different overlay programs in your project will
; share the same overall library configuration for your game.
;
; ***************************************************************************
; ***************************************************************************

		; Get the sound driver's configuration settings, this will
		; be read from the current directory, if it exists, or the
		; "../include/" directory if not.

		include	"hucc-sound.inc"

;
; Add optional debugging code?
;

	.ifndef	HUCC_DEBUG_SP
	.ifdef	_DEBUG		; HuCC sets _DEBUG when called with "-g" flag.
HUCC_DEBUG_SP	=	1	; (0 or 1)
	.else
HUCC_DEBUG_SP	=	0	; (0 or 1)
	.endif
	.endif

;
; Is the last track of the CD a duplicate of the main ISO data track?
;
; Most CD games put two copies of the same ISO track on the CD, so that data
; can be read from the secondary track if the drive has a problem reading it
; from the primary location.
;
; If set, the secondary track will be enabled when the library first starts.
;

	.ifndef	SUPPORT_2ISO
SUPPORT_2ISO	=	0	; (0 or 1)
	.endif

;
; Maximum number of directory entries to use from the ISO.
;
; The fewer that we choose, the less RAM memory we need. A setting of 64 is
; a good compromise to use until you are putting lots of files on the ISO.
;

	.ifndef	MAX_DIRSIZE
MAX_DIRSIZE	=	64	; (4..256)
	.endif

;
; Use the System Card's PSG driver code instead of a modern alternative
; sound driver like a DefleMask or Furnace player?
;

	.ifndef	USING_PSGDRIVER
USING_PSGDRIVER	=	0	; (0 or 1)
	.endif

;
; Include a 2KByte data table to support super-fast multiplication?
;
; This data table is also needed by sound drivers that can play DefleMask or
; Furnace tunes, because those trackers use multiplies quite extensively.
;
; Note that enabling this option also implies creating a HOME_BANK that HuCC
; will map into MPR5 ($A000..$BFFF).
;

	.ifndef FAST_MULTIPLY
FAST_MULTIPLY	=	0	; (0 or 1)
	.endif

;
; Support development for the Turbo EverDrive v2?
;
; This only applies to HuCARD programs, but when chosen, the "CORE(not TM)"
; kernel is moved from bank 0 to bank 2, and a minimal set of startup code
; is put in bank 0.
;
; This is because the TED2 hardware occupies the whole of bank 0 when it is
; activated, and so we need to run with a different bank in MPR7.
;

	.ifndef	SUPPORT_TED2
	.ifdef	_TED2		; HuCC sets _TED2 when called with "-ted2" flag.
SUPPORT_TED2	=	1	; (0 or 1)
	.else
SUPPORT_TED2	=	0	; (0 or 1)
	.endif
	.endif

;
; Support development for the SuperGRAFX?
;
; This enables SuperGRAFX support in certain library functions.
;

	.ifndef	SUPPORT_SGX
	.ifdef	_SGX		; HuCC sets _SGX when called with "-sgx" flag.
SUPPORT_SGX	=	1	; (0 or 1)
	.else
SUPPORT_SGX	=	0	; (0 or 1)
	.endif
	.endif

;
; Choose SuperGRAFX VPC initialization mode.
;
; SGX_PARALLAX=0 (useful when VDC #1 is a fullscreen HUD)
;
;  FRONT
;   SP1 = VDC #1 (pce) sprite pixels
;   BG1 = VDC #1 (pce) background pixels
;   SP2 = VDC #2 (sgx) sprite pixels
;   BG2 = VDC #2 (sgx) background pixels
;  BACK
;
; SGX_PARALLAX=1
;
;  FRONT
;   SP1 = VDC #1 (pce) sprite pixels
;   SP2 = VDC #2 (sgx) sprite pixels
;   BG1 = VDC #1 (pce) background pixels
;   BG2 = VDC #2 (sgx) background pixels
;  BACK
;

	.ifndef	SGX_PARALLAX
SGX_PARALLAX	=	1	; The most common default.
	.endif

;
; Support development for the ArcadeCard?
;
; This enables ArcadeCard support in certain library functions.
;

	.ifndef	SUPPORT_ACD
	.ifdef	_ACD		; HuCC sets _ACD when called with "-acd" flag.
SUPPORT_ACD	=	1	; (0 or 1)
	.else
SUPPORT_ACD	=	0	; (0 or 1)
	.endif
	.endif

;
; Support development for the IFU's ADPCM hardware?
;
; Developers would normally just use the System Card functions for ADPCM, but
; enabling this adds alternative functions, and allows ADPCM use on a HuCARD.
;

	.ifndef	SUPPORT_ADPCM
SUPPORT_ADPCM	=	0	; (0 or 1)
	.endif

;
; Select which version of the joystick library code to include, only one of
; these can be set to '1' ...
;
; SUPPORT_2BUTTON : Only returns buttons I and II.
; SUPPORT_6BUTTON : Read buttons III-VI, but ignore a mouse.
; SUPPORT_MOUSE	  : Read mouse, but ignore buttons III-VI.
;
; It doesn't make sense to design a game the relies on both the 6-button and
; the mouse, so the joystick library is optimized for one or the other.
;
; Note that both those devices are always detected and no conflicts occur,
; this just controls reading either buttons III-VI or the Mouse Y-movement.
;

	.ifndef	SUPPORT_6BUTTON
	.ifndef	SUPPORT_MOUSE
SUPPORT_2BUTTON	=	0	; (0 or 1)
SUPPORT_6BUTTON	=	1	; (0 or 1)
SUPPORT_MOUSE	=	0	; (0 or 1)
	.endif
	.endif

;
; How many joypad/mouse devices should be supported?
;
; This is normally 5, but can be set to 3 (or lower) in order to speed up
; the processing and free up CPU time for other code, which is especially
; useful for mouse games.
;

	.ifndef	MAX_PADS
MAX_PADS	=	5	; (1..5)
	.endif

;
; Implement a HuC-compatible seperate array for accumulating joypad presses?
;
; This is only really needed if your game is not expected to run at 60Hz and
; so you will sometimes miss joypad presses.
;
; Alternatively you can set ACCUMULATE_JOY so that joypad presses are always
; accumulated in the normal joytrg array, which is how asm programmers would
; normally deal with this situation.
;

	.ifndef HUC_JOY_EVENTS
HUC_JOY_EVENTS	=	0
	.endif

;
; Should "joytrg" accumulate presses, and so need to be explicitely cleared
; after reading, or should it just be the result of the last read?
;
; This option has no effect if HUC_JOY_EVENTS is non-zero since that option
; takes precedence over this one!
;

	.ifndef ACCUMULATE_JOY
ACCUMULATE_JOY	=	0
	.endif

;
; Choose how much zero-page memory to allocate for the HuCC stack.
;
; This should normally be 128 bytes, but if compiling with "-fno-recursive"
; you can usually get it down to 16 or 32 bytes.
;
; Some sound drivers, such as HuTrack, use a lot of ZP memory, and so you
; will need to set a stack size of 96 or less bytes if using one of those.
;

	.ifndef HUCC_STACK_SZ
HUCC_STACK_SZ	=	128	; (16 .. 128)
	.endif

;
; Choose how many split-screen scrolling regions to allow if you decide to
; use the old HuC scrolling library instead of the new HuCC library.
;
; The old HuC library is very very slow, and not recommended for new games.
;

	.ifndef	HUC_NUM_SCROLLS
HUC_NUM_SCROLLS	=	4	; (4 .. 8)
	.endif	HUC_NUM_SCROLLS

;
; Choose how many split-screen scrolling regions to allow if you decide to
; use the new HuCC scrolling library instead of the old HuC library.
;
; The new HuCC library is 6x faster, and it supports both screen layers if
; developing for the SuperGRAFX, with different numbers of splits for each
; layer.
;

	.ifndef	HUCC_PCE_SPLITS
HUCC_PCE_SPLITS =	8	; (4 .. 128) PCE VDC layer.
	.endif

	.ifndef	HUCC_SGX_SPLITS
HUCC_SGX_SPLITS =	8	; (4 .. 128) SGX VDC layer.
	.endif

;
; Support ZX0 decompression ring-buffer?
;
; If you decompress directly to VRAM, then you need to define a ring-buffer
; in RAM, both sized and aligned to a power-of-two (i.e. 256, 512, 1KB, 2KB).
;
; You also need to make sure that you tell the compressor that it needs to
; limit the window size with its "-w" option.
;
; Note that CD-ROM developers should really just decompress to RAM, and then
; use a TIA to copy the data to VRAM, because that is faster, you get better
; compression without a window, and you save code memory by not needing both
; versions of the decompression routine.
;
; HuCARD developers who either can't afford the RAM, or who prefer execution
; speed over compression, can choose a ring-buffer size of 256 bytes so that
; a faster version of the decompression code is used.
;

	.ifndef ZX0_WINBUF
ZX0_WINBUF	=	($3800)	; Default to a 2KB window in RAM at $3800.
ZX0_WINMSK	=	($0800 - 1)
	.endif

;
; The DATA_BANK location needs to be set as early as possible so that library
; code is able to put data in there before the total overall size of the code
; is known.
;
; By default, DATA_BANK is the next bank after the CORE_BANK.
;
; HUC_RESERVE_BANKS allows developers to reserve space between the CORE_BANK
; and the DATA_BANK that they can use however they wish.
;
; One use for HUC_RESERVE_BANKS is to allocate space that PCEAS can use when
; it relocates procedures. This provides a way for a developer to group code
; together at the start of memory, and leave the maximum number of banks for
; loading dynamic data from CD-ROM.
;
; The HuCC environment sets RESERVE_BANKS=1 (or higher) so that there is a
; a bank for the permanent C code and static constants.
;

	.ifndef	HUC_RESERVE_BANKS
HUC_RESERVE_BANKS =	0
	.endif	HUC_RESERVE_BANKS
