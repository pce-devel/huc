; ***************************************************************************
; ***************************************************************************
;
; core-config.inc
;
; Configuration settings for the "CORE(not TM)" PC Engine library code.
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

;
; Are we going to take over MPR7?
;
; While this is the default for the "CORE(not TM)" library, it is definitely
; possible to use the library to build CD-ROM code that runs with the System
; Card in MPR7, and with the 1st bank remaining in MPR2.
;
; N.B. The CD-ROM Stage1 loader runs the library with USING_MPR7=0 while it
;      installs the kernel into PCE RAM in MPR1.
;

	.ifndef	USING_MPR7
USING_MPR7	=	1	; (0 or 1)
	.endif

;
; Are we building a CD-ROM overlay that uses an already-built CD-ROM Stage1
; loader to install the library's kernel code into PCE RAM in MPR1?
;
; If this is set, the library's kernel code will be excluded from the build,
; which will save about 1KB-3KB of code space in every overlay program!
;
; N.B. The CD-ROM Stage1 loader sets USING_STAGE1=0 so it builds correctly.
;

	.ifndef	USING_STAGE1
USING_STAGE1	=	0	; (0 or 1)
	.endif

;
; Are we currently building the CD-ROM Stage1 loader?
;
; If this is set, the library's startup code does not call the __sound_init
; macro because the sound driver code is not usually a part of the kernel.
;
; N.B. The CD-ROM Stage1 loader sets BUILDING_STAGE1=1 so it builds correctly.
;

	.ifndef	BUILDING_STAGE1
BUILDING_STAGE1	=	0	; (0 or 1)
	.endif

;
; Is the last track of the CD a duplicate of the main ISO data track?
;
; Most CD games put two copies of the same ISO track on the CD, so that data
; can be read from the secondary track if the drive has a problem reading it
; it from the primary location.
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
; Support development for the Turbo EverDrive v2?
;
; This only applies to HuCard programs, but when chosen, the "CORE(not TM)"
; kernel is moved from bank 0 to bank 2, and a minimal set of startup code
; is put in bank 0.
;
; This is because the TED2 hardware occupies the whole of bank 0 when it is
; activated, and so we need to run with a different bank in MPR7.
;

	.ifndef	SUPPORT_TED2
SUPPORT_TED2	=	0	; (0 or 1)
	.endif

;
; Support development for the SuperGRAFX?
;
; This enables SuperGRAFX support in certain library functions.
;

	.ifndef	SUPPORT_SGX
SUPPORT_SGX	=	0	; (0 or 1)
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
SUPPORT_ACD	=	0	; (0 or 1)
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
SUPPORT_2BUTTON	=	1	; (0 or 1)
SUPPORT_6BUTTON	=	0	; (0 or 1)
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
; The DATA_BANK location needs to be set as early as possible so that library
; code is able to put data in there before the total overall size of the code
; is known.
;
; By default, DATA_BANK is the next bank after the CORE_BANK.
;
; RESERVE_BANKS allows developers to reserve some banks between the CORE_BANK
; and the DATA_BANK that they can use however they wish.
;
; One use for RESERVE_BANKS is to create empty space that PCEAS can use when
; it relocates procedures. This provides a way for a developer to group code
; together at the start of memory, and leave the maximum number of banks for
; loading dynamic data from CD-ROM.
;
; The KickC environment sets RESERVE_BANKS=1 (or higher) so that there is a
; a bank for the permanent C code and static constants.
;

	.ifndef	RESERVE_BANKS
	.ifdef	_KICKC
RESERVE_BANKS	=	1
	.else
RESERVE_BANKS	=	0
	.endif	_KICKC
	.endif	RESERVE_BANKS
