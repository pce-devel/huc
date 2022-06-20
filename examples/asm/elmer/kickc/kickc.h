// **************************************************************************
// **************************************************************************
//
// kickc.h
//
// KickC interfaces to assembly-language library code.
//
// Copyright John Brandwood 2021-2022.
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
// **************************************************************************
// **************************************************************************

//
//
//

#pragma data_seg(zp)
__export byte random [4];
#pragma data_seg(bss)

//
//
//

inline void wait_vsync (void) {
	kickasm( clobbers "A" )
	{{ jsr wait_vsync }}
}

inline byte rand (void) {
	kickasm( clobbers "AY" )
	{{ jsr get_random }}
	return random[1];
}

inline byte rand210 (void) {
	kickasm( clobbers "AY" )
	{{ jsr _random210 }}
	return *__al;
}

//
//
//

inline void init_256x224(void) {
	kickasm( clobbers "AXY" )
	{{ jsr init_256x224 }}
}

#if 1

inline void dropfnt8x8_vdc (byte * font, word vram, byte count, byte plane2, byte plane3) {
	*__di = vram;
	*__al = plane2;
	*__ah = plane3;
	*__bl = count;
	*__si = (word) font;
	*__bank = (byte) (font >> 23);
	kickasm( clobbers "Y" )
	{{ ldy.z __bank }}
	kickasm( clobbers "AXY" )
	{{ jsr dropfnt8x8_vdc }}
}

#else

// #define ldy_bank( addr ) asm { ldy #addr >> 23 }
// #define ldy_bank( addr ) kickasm( clobbers "Y" ) {{ ldy #bank( addr ) }}

#define dropfnt8x8_vdc( font, vram, count, plane2, plane3 ) \
	*__di = (word) (vram); \
	*__al = (byte) (plane2); \
	*__ah = (byte) (plane3); \
	*__bl = (byte) (count); \
	*__si = (word) (font); \
	*__bank = (byte) ((font) >> 23); \
	kickasm( clobbers "Y" ) \
	{{ ldy.z __bank }} \
	kickasm( clobbers "AXY" ) \
	{{ jsr dropfnt8x8_vdc }}

#endif

#if 0

inline void vdc_di_to_mawr (word vram) {
	*__di = (word) vram;
	kickasm( clobbers "AXY" )
	{{ jsr vdc_di_to_mawr }}
}

#else

#define vdc_di_to_mawr( vram ) \
	*__di = (word) (vram); \
	kickasm( clobbers "AXY" ) \
	{{ jsr vdc_di_to_mawr }}

#endif

inline void set_dspon(void) {
	kickasm( clobbers "A" )
	{{ jsr set_dspon }}
}

#if 1

inline void load_palette (byte palnum, word * data, byte palcnt) {
	*__al = palnum;
	*__ah = palcnt;
	*__si = (word) data;
	*__bank = (byte) ((data) >> 23);
	kickasm( clobbers "Y" )
	{{ ldy.z __bank }}
	kickasm( clobbers "AXY" )
	{{ jsr load_palettes }}
}

#else

#define load_palette( palnum, data, palcnt ) \
	*__al = (palnum); \
	*__ah = (palcnt); \
	*__si = (word) (data); \
	*__bank = (byte) ((data) >> 23); \
	kickasm( clobbers "Y" ) \
	{{ ldy.z __bank }} \
	kickasm( clobbers "AXY" ) \
	{{ jsr load_palettes }}

#endif
