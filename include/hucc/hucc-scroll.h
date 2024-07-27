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
; **************************************************************************/

// *************
// Functions in hucc-scroll.asm ...
// *************

#ifdef __HUCC__

#asmdef	HUCC_USES_NEW_SCROLL 1

extern void __fastcall __xsafe scroll_split( unsigned char index<_al>, unsigned char top<_ah>, unsigned int x<_bx>, unsigned int y<_cx>, unsigned char disp<_dl> );
extern void __fastcall __xsafe sgx_scroll_split( unsigned char index<_al>, unsigned char top<_ah>, unsigned int x<_bx>, unsigned int y<_cx>, unsigned char disp<_dl> );

extern void __fastcall __xsafe disable_split( unsigned char index<acc> );
extern void __fastcall __xsafe sgx_disable_split( unsigned char index<acc> );

#endif // __HUCC__

#endif // _hucc_scroll_h
