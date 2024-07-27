#ifndef _hucc_old_line_h
#define _hucc_old_line_h

/****************************************************************************
; ***************************************************************************
;
; hucc-old-line.h
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
; **************************************************************************/

// *************
// Functions in hucc-old-line.asm ...
// *************

#ifdef __HUCC__

#asmdef	HUCC_USES_OLD_LINE 1

extern void __fastcall __xsafe gfx_init( unsigned int start_vram_addr<_ax> );
extern void __fastcall __xsafe gfx_clear( unsigned int start_vram_addr<_di> );
extern void __fastcall __xsafe gfx_plot( unsigned int x<_gfx_x1>, unsigned int y<_gfx_y1>, char color<_gfx_color> );
extern void __fastcall __xsafe gfx_line( unsigned int x1<_gfx_x1>, unsigned int y1<_gfx_y1>, unsigned int x2<_gfx_x2>, unsigned int y2<_gfx_y2>, unsigned char color<_gfx_color> );

#endif // __HUCC__

#endif // _hucc_old_line_h
