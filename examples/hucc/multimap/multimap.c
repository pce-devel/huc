/****************************************************************************
; ***************************************************************************
;
; multimap.c
;
; An example of building and scrolling a multi-screen block map.
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
#include "hucc-blkmap.h"



// **************************************************************************
// **************************************************************************
//
// An example of scrolling a block map (a map with 16x16 meta-tiles).
//
// This also shows one way to animate background characters in the map.
//


// Get the full 256-color palette from the bitmap.

#incpal( seran_pal, "../seran/seran-map.png" );


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
//
// This uses PCEAS's ability to add multiple images to a block set in order to
// put the two blocks that make up the entrance to each house at the beginning
// of the set of blocks so that they can be detected as BLK 0 and 1 in the map
// when the player tries to enter a building.
//
// This show how to force specific BLK graphics to use specific BLK numbers so
// the programmer can write their game code to detect them.

#asm
		.data
_seran_blk:	incblk	"../seran/seran-enter.png", 0x1000, _seran_chr ; House entrance.
		incblk	"../seran/seran-map.png", 0x1000, _seran_chr
		.code
#endasm

extern unsigned char seran_blk[];


// Extract the map in BLK format.
//
// This is used for medium sized scrolling backgrounds, and is also the
// format for the individual screens in a huge multi-screen background.
//
// A map can be transformed later on for use as a multi-screen map with
// the ".SWIZZLE" command even if it is larger than the normal limit of
// 128x128 blocks in size.
//
// Note that the map data for a multi-screen map *must* be aligned to a
// 256-byte boundary in memory.
//
// This shows how to accomplish that in HuCC, but as you must be seeing
// by now all of these separate "#asm/#endasm" sections are a sign that
// it is usually simpler to handle all of this map data conversion in a
// large "#asm/#endasm" section, or in a separate asm file.

#asm
		.data
		.align	256
		.code
#endasm

#incmap( seran_map, "../seran/seran-map.png", seran_blk );


// Convert the map into BAT-sized chunks so that it can be used as parts
// of a multi-screen map.
//
// Since this example uses a BAT size of 32x32 characters instead of the
// HuCC default of 64x32 characters, the width and height parameters are
// both 16 (i.e. 32 characters / 2 characters-per-16x16-block).
//
// Each resulting BAT-sized "screen" is therefore 256 bytes long (16x16)
// or 512 bytes long (32x16) for the most common BAT sizes, which allows
// for the easy calculation of the offsets to each screen's map data.
//
// Note that the .SWIZZLE command is only available in a "#asm" section!

#asm
		swizzle	_seran_map, 16, 16
#endasm



// Define a 4x4 multi-screen map using the 16 BAT-sized chunks that were
// created by the .SWIZZLE command, together with the "SCREEN" macro.
//
// Each screen's data in the multi-screen map tells the map library code
// where to find the map chunk, the BLK data, and the 256-byte MASK/OVER
// table used to associate mask or overlay sprites with the BLK.
//
// The final parameter is not used yet at this time, so set it to zero.
//
// A map this small does not need to be a multi-screen map, but it shows
// how to build up a multi-screen map data structure.
//
// Both the map data and the MASK/OVER table must be 256-byte aligned in
// memory, and BLK definitions must be 2048-byte aligned.
//
// The multi-screen map data structure can hold a limit of 1024 screens,
// and it does not need any particular alignment.
//
// It is stored as an array of lines of screens from the top the bottom,
// with each line defining the screens in left to the right order.

#asm
		.data
_multi_map:	; 1st line of 4 screens, left-to-right, each screen is 16x16=256 bytes long.
		SCREEN	_seran_map + $0000, _seran_blk, _seran_msk, 0
		SCREEN	_seran_map + $0100, _seran_blk, _seran_msk, 0
		SCREEN	_seran_map + $0200, _seran_blk, _seran_msk, 0
		SCREEN	_seran_map + $0300, _seran_blk, _seran_msk, 0
		; 2nd line of 4 screens, each screen is 16x16=256 bytes long.
		SCREEN	_seran_map + $0400, _seran_blk, _seran_msk, 0
		SCREEN	_seran_map + $0500, _seran_blk, _seran_msk, 0
		SCREEN	_seran_map + $0600, _seran_blk, _seran_msk, 0
		SCREEN	_seran_map + $0700, _seran_blk, _seran_msk, 0
		; 3rd line of 4 screens, each screen is 16x16=256 bytes long.
		SCREEN	_seran_map + $0800, _seran_blk, _seran_msk, 0
		SCREEN	_seran_map + $0900, _seran_blk, _seran_msk, 0
		SCREEN	_seran_map + $0A00, _seran_blk, _seran_msk, 0
		SCREEN	_seran_map + $0B00, _seran_blk, _seran_msk, 0
		; 4th line of 4 screens, each screen is 16x16=256 bytes long.
		SCREEN	_seran_map + $0C00, _seran_blk, _seran_msk, 0
		SCREEN	_seran_map + $0D00, _seran_blk, _seran_msk, 0
		SCREEN	_seran_map + $0E00, _seran_blk, _seran_msk, 0
		SCREEN	_seran_map + $0F00, _seran_blk, _seran_msk, 0
		.code
#endasm

extern unsigned char multi_map[];


// This example doesn't use the extra .MASKMAP/.OVERMAP lookup table for
// the BLK, but the library routines expect *something* to be specified.
//
// This just gives the code an array of blank data to keep it happy.

const unsigned char seran_msk[256];


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
// HuCC users please note ... putting the constant math in parentheses helps the
// compiler optimize it to a single number when the #defines are used.

#define TILE_WORDS 16
#define WATER_SIZE (4 * 4 * TILE_WORDS)
#define SHORE_SIZE (8 * 1 * TILE_WORDS)


// Variables for the animation.

unsigned char water_frame;
unsigned char shore_frame;
unsigned char frame_cycle;
unsigned int  cycle_offset;

//

test_multimap()
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

	// We built the multi-screen map earlier, so we know that it is 4 screens wide.
	set_multimap( multi_map, 4 );

	vdc_map_pxl_x = 0;
	vdc_map_pxl_y = 0;
	vdc_map_draw_w = (240 / 8) + 1;
	vdc_map_draw_h = (208 / 8) + 1;

	draw_map();

	put_string( "How to use a Multi-Screen map.", 0, 0 );
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
// Switch between two different demos ...
//

main()
{
	for (;;)
	{
		// An example of scrolling a block map with animation.

		test_multimap();
	}
}
