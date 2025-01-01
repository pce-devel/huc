#ifndef _hucc_scroll_h
#define _hucc_scroll_h

/****************************************************************************
; ***************************************************************************
;
; hucc-scroll.h
;
; Routines for a fast split-screen scrolling system.
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
; The maximum number of splits for each screen layer is set in your project's
; "hucc-config.inc" file, with the library having a limit of 128-per-layer.
;
; Your first active split must be defined to start at screen line 0, and then
; the rest of the active splits must be in increasing line order to match the
; way that the PC Engine displays the output image.
;
; You can have disabled splits interleaved with your active splits.
;
; Splits that are normally disabled can be used to create full screen effects
; such as bouncing the screen up and down by adding blank areas at the top or
; bottom of the screen, and then rapidly changing the height of those areas.
;
; ***************************************************************************
; **************************************************************************/

// *************
// VDC bits for display_flags ...
// *************

#ifndef BKG_ON
#define BKG_ON 0x80
#endif

#ifndef SPR_ON
#define SPR_ON 0x40
#endif

// *************
// Functions in hucc-scroll.asm ...
// *************

#ifdef __HUCC__

#asmdef	HUCC_USES_NEW_SCROLL 1

extern void __fastcall scroll_split( unsigned char index<_al>, unsigned char screen_line<_ah>, unsigned int bat_x<_bx>, unsigned int bat_y<_cx>, unsigned char display_flags<_dl> );
extern void __fastcall sgx_scroll_split( unsigned char index<_al>, unsigned char screen_line<_ah>, unsigned int bat_x<_bx>, unsigned int bat_y<_cx>, unsigned char display_flags<_dl> );

extern void __fastcall disable_split( unsigned char index<acc> );
extern void __fastcall sgx_disable_split( unsigned char index<acc> );

extern void __fastcall __macro disable_all_splits( void );
extern void __fastcall __macro sgx_disable_all_splits( void );

#asm
		.macro	_disable_all_splits
		ldy	#HUCC_PCE_SPLITS - 1
!loop:		tya
		call	_disable_split.1
		dey
		bpl	!loop-
		.endm

		.macro	_sgx_disable_all_splits
		ldy	#HUCC_SGX_SPLITS - 1
!loop:		tya
		call	_sgx_disable_split.1
		dey
		bpl	!loop-
		.endm
#endasm

#endif // __HUCC__

#endif // _hucc_scroll_h
