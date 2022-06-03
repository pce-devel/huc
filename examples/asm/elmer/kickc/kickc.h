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

inline void dropfnt8x8_vdc (byte * font, word vram, byte count, byte plane2, byte plane3) {
	*__si = (word) font;
	*__bank = (byte) (font >> 23);
	*__di = vram;
	*__al = plane2;
	*__ah = plane3;
	*__bl = count;
	kickasm( clobbers "AXY" )
	{{ jsr dropfnt8x8_vdc }}
}

inline void vdc_di_to_mawr (word vram) {
	*__di = (word) vram;
	kickasm( clobbers "AXY" )
	{{ jsr vdc_di_to_mawr }}
}

inline void set_dspon(void) {
	kickasm( clobbers "A" )
	{{ jsr set_dspon }}
}

inline void load_palette (byte palnum, word * data, byte palcnt) {
	*__si = (word) data;
	*__bank = (byte) (data >> 23);
	*__al = palnum;
	*__ah = palcnt;
	kickasm( clobbers "AXY" )
	{{ jsr load_palettes }}
}
