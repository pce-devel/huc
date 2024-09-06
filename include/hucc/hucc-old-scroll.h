#ifndef _hucc_old_scroll_h
#define _hucc_old_scroll_h

/****************************************************************************
; ***************************************************************************
;
; hucc-old-scroll.h
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
; **************************************************************************/

// *************
// Functions in hucc-old-scroll.asm ...
// *************

#ifdef __HUCC__

#asmdef	HUCC_USES_OLD_SCROLL 1

extern void __fastcall __xsafe scroll( unsigned char num<_al>, unsigned int x<_cx>, unsigned int y<_dx>, unsigned char top<_ah>, unsigned char bottom<_bl>, unsigned char disp<acc> );
extern void __fastcall __xsafe scroll_disable( unsigned char num<acc> );

#endif // __HUCC__

#endif // _hucc_old_scroll_h
