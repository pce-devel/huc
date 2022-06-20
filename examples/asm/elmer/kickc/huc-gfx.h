// **************************************************************************
// **************************************************************************
//
// huc-gfx.h
//
// Compatibility stuff for HuC's library functions.
//
// Copyright John Brandwood 2021-2022.
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
// **************************************************************************
// **************************************************************************

/*
 * joypad defines
 */

#define	JOY_A		0x01
#define	JOY_I		0x01
#define	JOY_B		0x02
#define	JOY_II		0x02
#define	JOY_SLCT	0x04
#define	JOY_SEL		0x04
#define	JOY_STRT	0x08
#define	JOY_RUN		0x08
#define	JOY_UP		0x10
#define	JOY_RGHT	0x20
#define	JOY_RIGHT	0x20
#define	JOY_DOWN	0x40
#define	JOY_LEFT	0x80

inline byte get_joy (byte which) {
	return *(joynow + which);
}

inline byte get_joytrg (byte which) {
	return *(joytrg + which);
}



/*
 * VRAM access
 */

#define load_vram(addr, data, size) \
	*__di = (word) (addr); \
	*__ax = (word) (size); \
	*__si = (word) (data); \
	*__si = (word) (data); \
	*__bank = (byte) ((data) >> 23); \
	kickasm( clobbers "Y" ) \
	{{ ldy.z __bank }} \
	kickasm( clobbers "AXY" ) \
	{{ jsr _load_vram }}

#define load_bat(addr, data, w, h) \
	*__di = (word) (addr); \
	*__al = (byte) (w); \
	*__ah = (byte) (h); \
	*__si = (word) (data); \
	*__bank = (byte) ((data) >> 23); \
	kickasm( clobbers "Y" ) \
	{{ ldy.z __bank }} \
	kickasm( clobbers "AXY" ) \
	{{ jsr _load_bat }}

#define load_background(chrdata, paldata, batdata, batw, bath) \
	load_vram(0x1000, chrdata, 0x4000); \
	wait_vsync(); \
	load_palette(0, paldata, 16); \
	load_bat(0, batdata, batw, bath);



/*
 * text output
 */

#define put_string(addr, x, y) \
	*__al = (byte) (x); \
	*__ah = (byte) (y); \
	*__si = (word) (addr); \
	kickasm( clobbers "AXY" ) \
	{{ jsr _put_string }}

#define put_number(value, n, x, y) \
	*__al = (byte) (x); \
	*__ah = (byte) (y); \
	*__bx = (word) (value); \
	*__cl = (byte) (n); \
	kickasm( clobbers "AXY" ) \
	{{ jsr _put_number }}



/*
 * sprite functions
 */

#define  FLIP_X_MASK 0x08
#define  FLIP_Y_MASK 0x80
#define  FLIP_MAS   0x88
#define  SIZE_MAS   0x31

#define  NO_FLIP    0x00
#define  NO_FLIP_X  0x00
#define  NO_FLIP_Y  0x00
#define  FLIP_X     0x08
#define  FLIP_Y     0x80
#define  SZ_16x16   0x00
#define  SZ_16x32   0x10
#define  SZ_16x64   0x30
#define  SZ_32x16   0x01
#define  SZ_32x32   0x11
#define  SZ_32x64   0x31

inline void init_satb (void) {
	kickasm( clobbers "AXY" )
	{{ jsr _init_satb }}
} 

inline void reset_satb (void) {
	kickasm( clobbers "AXY" )
	{{ jsr _init_satb }}
} 

inline void satb_update (void) {
	kickasm( clobbers "AXY" )
	{{ jsr _satb_update }}
} 

#define spr_set(num) { \
	*__al = (byte) (num); \
	kickasm( clobbers "A" ) \
	{{ jsr _spr_set }} } 

#define spr_hide() { \
	kickasm( clobbers "AY" ) \
	{{ jsr _spr_hide }} }

#define spr_show() { \
	kickasm( clobbers "AY" ) \
	{{ jsr _spr_show }} }

#define spr_x(x_val) { \
	*__ax = (word) (x_val); \
	kickasm( clobbers "AY" ) \
	{{ jsr _spr_x }} }

#define spr_y(y_val) { \
	*__ax = (word) (y_val); \
	kickasm( clobbers "AY" ) \
	{{ jsr _spr_y }} }

#define spr_pattern(vaddr) { \
	*__ax = (word) (vaddr); \
	kickasm( clobbers "AY" ) \
	{{ jsr _spr_pattern }} }

#define spr_pal(pal) { \
	*__al = (byte) (pal); \
	kickasm( clobbers "AY" ) \
	{{ jsr _spr_pal }} }

#define spr_pri(pri) { \
	*__al = (byte) (pri); \
	kickasm( clobbers "AY" ) \
	{{ jsr _spr_pri }} }

#define spr_ctrl(mask, value) { \
	*__al = (byte) ~(mask); \
	*__ah = (byte) ((value) & (mask)); \
	kickasm( clobbers "AY" ) \
	{{ jsr _spr_ctrl }} }
