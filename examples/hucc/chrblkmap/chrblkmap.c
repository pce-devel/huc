/****************************************************************************
; ***************************************************************************
;
; chrblkmap.c
;
; An example of scrolling a meta-tile (aka block) or BAT (aka character) map.
;
; Copyright John Brandwood 2025.
;
; Distributed under the Boost Software License, Version 1.0.
; (See accompanying file LICENSE_1_0.txt or copy at
;  http://www.boost.org/LICENSE_1_0.txt)
;
; ***************************************************************************
; ***************************************************************************
;
; This uses the Seran Village map from Falcom's "Legend of Xanadu 2" game.
;
; See the README.md file in the "../seran/" directory for more information.
;
; ***************************************************************************
; **************************************************************************/

#include "huc.h"
#include "hucc-scroll.h"
#include "hucc-chrmap.h"
#include "hucc-blkmap.h"



// **************************************************************************
// **************************************************************************
//
// An example of scrolling a block map (a map with 16x16 meta-tiles).
//
// This also shows one way to animate background characters in the map.
//


// Extract all of the 8x8 characters for the 8 frames of 32x32 pixel water animation.

#incchr( water_chr, "../seran/seran-water.png", 4, 4*8 ); /* Animated characters. */


// Extract all of the 8x8 characters for the 4 frames of 64x8 pixel shore animation.

#incchr( shore_chr, "../seran/seran-shore.png", 8, 1*4 ); /* Animated characters. */


// Extract all of the 8x8 characters (a.k.a. tiles) that are used in the
// bitmap, removing the repeated characters.
//
// This uses PCEAS's ability to add multiple images to a character set, which
// requires that they are all put one-after-the-other with no labels or blank
// lines or different commands between the multiple .inchr/.padchr statements.
//
// The characters that we're going to animate must be included somewhere that
// we can find them later on. In this case it is easiest to just include them
// first so that they're at the start of the character set, but that is not a
// a general requirement for background animation to work.

#asm
OPTIMIZE	=	1
		.data
_seran_chr:	incchr	"../seran/seran-water.png", 4, 4 ; 1st frame of the animation.
		incchr	"../seran/seran-shore.png", 8, 1 ; 1st frame of the animation.
		incchr	"../seran/seran-map.png", 128, 128, OPTIMIZE
		.code
#endasm

extern unsigned char seran_chr[];


// Extract 16x16 pixel blocks (a.k.a. meta-tiles) from the bitmap using
// the optimized character set that has already been extracted.
//
// There are a maximum of 256 blocks in a BLK map, and the definitiuons
// are always 2KByte long and 2KByte aligned in memory.
//
// You need to tell the converter where in VRAM you are going to upload
// the character set.

#incblk( seran_blk, "../seran/seran-map.png", 0x1000, seran_chr );


// Extract the map in BLK format (max size is 128x128 blocks).
//
// This is used for medium sized scrolling backgrounds, and is also the
// format for the individual screens in a huge multi-screen background.

#incmap( seran_map, "../seran/seran-map.png", seran_blk );


// Get the full 256-color palette from the bitmap.

#incpal( seran_pal, "../seran/seran-map.png" );


// This example doesn't use the extra .MASKMAP/.OVERMAP lookup table for
// the BLK, but the library routines expect *something* to be specified.
//
// This just gives the code an array of blank data to keep it happy.

unsigned char seran_msk[256];


// Define where the map's original copies of the animated characters are in VRAM
// so that they can be swapped out with the animated versions of the characters.
//
// We know what the VRAM addresses are because we specifically put the 1st frame
// of each animation at the start of the character set.

#define WATER_CHR 0x1000
#define SHORE_CHR 0x1100


// Define where to upload the animation characters in VRAM so that they're
// all in VRAM and can be used for the VDC chip's vram2vram DMA.

#define WATER_ANIM 0x3000
#define SHORE_ANIM 0x3800


// Define the size of each frame of the animation, an 8x8 character is 16 words.
//
// HuCC users please note ... putting the constant math in parentheses here lets
// the compiler optimize it to a single number when the #defines are used.

#define TILE_WORDS 16
#define WATER_SIZE (4 * 4 * TILE_WORDS)
#define SHORE_SIZE (8 * 1 * TILE_WORDS)


// Variables for the animation.

unsigned char water_frame;
unsigned char shore_frame;
unsigned char frame_cycle;
unsigned int  cycle_offset;

//

test_blkmap()
{
	// Use a reduced screen size and 32x32 BAT size to save VRAM.
	// This also clears and enables the display.

	set_240x208();

	// Disable the display before uploading the graphics.
	disp_off();
	vsync();

	// Upload the default HuCC font.
	set_font_color(4, 0);
	set_font_pal(15);
	load_default_font();

	// You can use COUNTOF() to get the number of palettes in a .incpal.
	load_palette( 0, seran_pal, COUNTOF(seran_pal) );

	// You can use SIZEOF() to get the number of bytes in a .incchr.
	load_vram( 0x1000, seran_chr, SIZEOF(seran_chr) >> 1 );

	// The animations need to be in VRAM to use vram2vram() DMA.
	load_vram( WATER_ANIM, water_chr, SIZEOF(water_chr) >> 1 );
	load_vram( SHORE_ANIM, shore_chr, SIZEOF(shore_chr) >> 1 );

	// There are normally 256 blocks in a .incblk although there is a way
	// to store fewer, but that isn't the point of this example.
	set_blocks( seran_blk, seran_msk, 256 );

	// You can use COUNTOF() to get the width of a .incmap map.
	set_blkmap( seran_map, COUNTOF(seran_map) );

	vdc_map_pxl_x = 0;
	vdc_map_pxl_y = 0;
	vdc_map_draw_w = (240 / 8) + 1;
	vdc_map_draw_h = (208 / 8) + 1;

	draw_map();

	put_string( "An animated scrolling BLK map.", 0, 0 );
	put_string( "LRUD to scroll.  RUN for next.", 0, 1 );

	scroll_split(0, 0, vdc_map_pxl_x & 511, vdc_map_pxl_y & 255, BKG_ON | SPR_ON);

	vsync();

	disp_on();

	//  demo main loop
	for (;;)
	{
		// It is usually best to put vsync() at the top of the game loop.
		vsync();

		// Finish when START is pressed.
		if (joytrg(0) & JOY_STRT) {
			break;
		}

		// Get movement direction.
		if (joy(0) & JOY_LEFT) {
			if (vdc_map_pxl_x > 0)
				vdc_map_pxl_x -= 2;
		}
		else
		if (joy(0) & JOY_RIGHT) {
			if (vdc_map_pxl_x < (1024 - 240))
				vdc_map_pxl_x += 2;
		}

		if (joy(0) & JOY_UP) {
			if (vdc_map_pxl_y > 0)
				vdc_map_pxl_y -= 2;
		}
		else
		if (joy(0) & JOY_DOWN) {
			if (vdc_map_pxl_y < (1024 - 208))
				vdc_map_pxl_y += 2;
		}

		scroll_map();

		scroll_split(0, 0, vdc_map_pxl_x & 511, vdc_map_pxl_y & 255, BKG_ON | SPR_ON);

		// Slow the animation by only uploading on every other video frame (each 1/60th second).
		// Note that "irq_cnt" is a system variable that the HuCC runtime increments every video frame.
		if (irq_cnt & 1)
		{
			// Upload a water frame over 4/60th, then upload a shore frame over 2/60th.
			++frame_cycle;
			if (frame_cycle == 6) {
				frame_cycle = 0;
				water_frame = ++water_frame & 7;
				shore_frame = ++shore_frame & 3;
			}

			// Each frame of animation uploads 4 characters per cycle.
			cycle_offset = ((frame_cycle & 3) * (4 * TILE_WORDS));

			// Use vram2vram DMA to update the background animation in the next vblank.
			if (frame_cycle < 4) {
				// 8 animation frames of 4x4 characters per frame,
				// each frame split into 4 uploads of 4 characters.
				vram2vram( WATER_ANIM + (water_frame * WATER_SIZE) + cycle_offset, WATER_CHR + cycle_offset, (4 * TILE_WORDS) );
			} else {
				// 4 animation frames of 8x1 characters per frame,
				// each frame split into 2 uploads of 4 characters.
				vram2vram( SHORE_ANIM + (shore_frame * SHORE_SIZE) + cycle_offset, SHORE_CHR + cycle_offset, (4 * TILE_WORDS) );
			}
		}
	}
}



// **************************************************************************
// **************************************************************************
//
// An example of scrolling a character map (in the VDC's BAT format).
//


// Extract the map in BAT format, max size 256x32 or 128x64 characters so
// we only convert half of the original Seran Village map.
//
// This uses 8 times the memory of a tile map or block map, but you can
// use more than 32KB of character data, and you are not limited to the
// maximum of 256 tiles/blocks.
//
// This format is best used for small logos, title screens, status bars
// and reward screens, but can also be used for scrolling backgrounds.
//
// You need to tell the converter where in VRAM you are going to upload
// the character set.

#incbat( seran_bat, "../seran/seran-map.png", 0x1000, 128, 64, seran_chr );

//

test_chrmap()
{
	// Use a reduced screen size and 32x32 BAT size to save VRAM.
	// This also clears and enables the display.

	set_240x208();

	// Disable the display before uploading the graphics.
	disp_off();
	vsync();

	// Upload the default HuCC font.
	set_font_color(4, 0);
	set_font_pal(15);
	load_default_font();

	// You can just ignore the COUNTOF() if you already know the number of palettes.
	load_palette( 0, seran_pal, 16 );

	// You can use BANK() to get bank of a symbol, allowing this to work ...
	set_far_base( BANK(seran_chr), seran_chr );
	far_load_vram( 0x1000, SIZEOF(seran_chr) >> 1 );

	// The animations need to be in VRAM to use vram2vram() DMA.
	load_vram( WATER_ANIM, water_chr, SIZEOF(water_chr) >> 1 );
	load_vram( SHORE_ANIM, shore_chr, SIZEOF(shore_chr) >> 1 );

	// You can use COUNTOF() to get the width of a .incbat map.
	set_chrmap( seran_bat, COUNTOF(seran_bat) );

	vdc_map_pxl_x = 0;
	vdc_map_pxl_y = 0;
	vdc_map_draw_w = (240 / 8) + 1;
	vdc_map_draw_h = (208 / 8) + 1;

	draw_bat();

	put_string( "An animated scrolling CHR map.", 0, 0 );
	put_string( "LRUD to scroll.  RUN for next.", 0, 1 );

	scroll_split(0, 0, vdc_map_pxl_x & 511, vdc_map_pxl_y & 255, BKG_ON | SPR_ON);

	vsync();

	disp_on();

	// Demo main loop.
	for (;;)
	{
		// It is usually best to put vsync() at the top of the game loop.
		vsync();

		// Finish when START is pressed.
		if (joytrg(0) & JOY_STRT) {
			break;
		}

		// Get move direction.
		if (joy(0) & JOY_LEFT) {
			if (vdc_map_pxl_x > 0)
				vdc_map_pxl_x -= 2;
		}
		else
		if (joy(0) & JOY_RIGHT) {
			if (vdc_map_pxl_x < (1024 - 240))
				vdc_map_pxl_x += 2;
		}

		if (joy(0) & JOY_UP) {
			if (vdc_map_pxl_y > 0)
				vdc_map_pxl_y -= 2;
		}
		else
		if (joy(0) & JOY_DOWN) {
			if (vdc_map_pxl_y < (512 - 208))
				vdc_map_pxl_y += 2;
		}

		scroll_bat();

		scroll_split(0, 0, vdc_map_pxl_x & 255, vdc_map_pxl_y & 255, BKG_ON | SPR_ON);

		// Slow the animation by only uploading on every other video frame (each 1/60th second).
		// Note that "irq_cnt" is a system variable that the HuCC runtime increments every video frame.
		if (irq_cnt & 1)
		{
			// Upload a water frame over 4/60th, then upload a shore frame over 2/60th.
			++frame_cycle;
			if (frame_cycle == 6) {
				frame_cycle = 0;
				water_frame = ++water_frame & 7;
				shore_frame = ++shore_frame & 3;
			}

			// Each frame of animation uploads 4 characters per cycle.
			cycle_offset = ((frame_cycle & 3) * (4 * TILE_WORDS));

			// Use vram2vram DMA to update the background animation in the next vblank.
			if (frame_cycle < 4) {
				// 8 animation frames of 4x4 characters per frame,
				// each frame split into 4 uploads of 4 characters.
				vram2vram( WATER_ANIM + (water_frame * WATER_SIZE) + cycle_offset, WATER_CHR + cycle_offset, (4 * TILE_WORDS) );
			} else {
				// 4 animation frames of 8x1 characters per frame,
				// each frame split into 2 uploads of 4 characters.
				vram2vram( SHORE_ANIM + (shore_frame * SHORE_SIZE) + cycle_offset, SHORE_CHR + cycle_offset, (4 * TILE_WORDS) );
			}
		}
	}
}



// **************************************************************************
// **************************************************************************
//
// Switch between two different demos ...
//

main()
{
	for (;;)
	{
		// An example of scrolling a block map with animation.

		test_blkmap();

		// An example of scrolling a character map (in the VDC's BAT format).

		test_chrmap();
	}
}
