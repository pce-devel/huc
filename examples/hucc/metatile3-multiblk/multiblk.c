/****************************************************************************
; ***************************************************************************
;
; multiblk.c
;
; An example of using multiple sets of blocks in a multi-screen block map.
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
; This expands upon the previous example to show how to use two map sections
; in a multi-screen map in order to work around the limited number of blocks
; (256 at most) in a single map section.
;
; This example also shows how to do background animation using color-cycling
; instead of uploading animated 8x8 characters.
;
; It also shows how to put a project's graphics conversion steps into a .ASM
; file in order to keep the C source code cleaner and easier to follow.
;
; Don't worry, you do not need to learn how to program in assembly-language!
;
; Finally, this example adds palette fading at the start and end of the demo
; to show how to create faded palettes from a reference palette.
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
// **************************************************************************
// **************************************************************************


// Do all of the graphics conversion in a separate file.

#asm
.include "graphics.asm"
#endasm


// Inform the C compiler about the data that was created in "graphics.asm".

extern unsigned char waves_pal[];
extern unsigned char seran_pal[];
extern unsigned char seran_water[];
extern unsigned char seran_shore[];
extern unsigned char seran_chr[];
extern unsigned char multi_map[];


// Define the screen size that we're going to use to reduce the number of
// unexplained "magic" numbers later on.

#define VIEW_W 256
#define VIEW_H 224

#define BAT_W 512
#define BAT_H 256

#define SPEED 2


// Define where the map's original copies of the animated characters are in VRAM
// so that they can be swapped out with the animated versions of the characters.
//
// We know what the VRAM addresses are because we specifically put the 1st frame
// of each animation at the start of the character set.

#define WATER_CHR 0x1000
#define SHORE_CHR 0x1100


// Define where to upload the animation characters in VRAM so that they are all
// in VRAM and can be used for the VDC chip's vram2vram DMA.
//
// Using 2 sets of blocks in a single character set has increased the number of
// of characters to just over 0x300 so we're moving the animations around a bit
// so that the total usage is 32KBytes (VRAM $1000..$4FFF).

#define SHORE_ANIM 0x4600
#define WATER_ANIM 0x4800


// Define the size of each frame of the animation, an 8x8 character is 16 words.
//
// HuCC users please note ... putting the constant math in parentheses helps the
// compiler optimize it to a single number when the #defines are used.

#define PALETTE_BYTES 32
#define TILE_WORDS 16
#define WATER_SIZE (4 * 4 * TILE_WORDS)
#define SHORE_SIZE (8 * 1 * TILE_WORDS)


// Variables for the animation.

unsigned char water_delay;
unsigned char water_cycle;
unsigned char water_frame;
unsigned char shore_frame;

unsigned char waves_delay;
unsigned int  waves_frame;

// Buffers for fading palettes.

unsigned char fade_val;      // 0..7
unsigned int  fade_pal[256]; // Buffer for 256 faded colors.
unsigned int  base_pal[256]; // Buffer for the original un-faded colors.

//

test_multiblk()
{
//	// This is the HuCC default, so it isn't actually needed.
//
//	init_256x224();

	// So just disable the display and clear the palette before uploading the graphics.

	disp_off();
	vsync();
	clear_palette();

	// Upload the default HuCC font.

	set_font_color( 4, 0 );
	set_font_pal( 15 );
	load_default_font();

	// You can use SIZEOF() to get the number of bytes in a .incchr.

	load_vram( 0x1000, seran_chr, SIZEOF(seran_chr) >> 1 );

	// The animations need to be in VRAM to use vram2vram() DMA.

	load_vram( WATER_ANIM, seran_water, SIZEOF(seran_water) >> 1 );
	load_vram( SHORE_ANIM, seran_shore, SIZEOF(seran_shore) >> 1 );

	// We built the multi-screen map earlier, so we know that it is 2 screens wide.
	//
	// A multi-screen map defines the BLK used, so no call to "set_blocks()".

	set_multimap( multi_map, 2 );

	// The multi-screen map is drawn using global variables for the top-left
	// coordinate in pixels, and the width and height in 8x8 characters.

	vdc_map_pxl_x = 0;
	vdc_map_pxl_y = 0;
	vdc_map_draw_w = (VIEW_W / 8) + 1; // + 1 to allow for scrolling.
	vdc_map_draw_h = (VIEW_H / 8) + 1; // + 1 to allow for scrolling.

	draw_map();

	put_string( " Use 2 sets of blocks in a map. ", 0, 0 );
	put_string( " LRUD to scroll.  RUN for next. ", 0, 1 );

	scroll_split( 0, 0, vdc_map_pxl_x & (BAT_W - 1), vdc_map_pxl_y & (BAT_H - 1), BKG_ON | SPR_ON );

	vsync();

	disp_on();

	// Fade in the screen before starting the main loop, ending at -1 which is 255 as unsigned.

	for (fade_val = 7; fade_val != 255; --fade_val)
	{
		// Create a 256 color faded palette in the fade_pal[] array by subtracting
		// the fade_val (7..0) from each of the RGB color components in seran_pal.

		fade_to_black( seran_pal, fade_pal, 256, fade_val );

		// Load the background palette from the fade_pal[] arrary.

		load_palette( 0, fade_pal, 16 );

		// Don't go too fast, there are only 8 steps!

		vsync( 5 );
	}

	// Demo main loop.

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
				vdc_map_pxl_x -= SPEED;
		}
		else
		if (joy(0) & JOY_RIGHT) {
			if (vdc_map_pxl_x < (1024 - VIEW_W))
				vdc_map_pxl_x += SPEED;
		}

		if (joy(0) & JOY_UP) {
			if (vdc_map_pxl_y > 0)
				vdc_map_pxl_y -= SPEED;
		}
		else
		if (joy(0) & JOY_DOWN) {
			if (vdc_map_pxl_y < (1792 - VIEW_H))
				vdc_map_pxl_y += SPEED;
		}

		// Update the newly-visible edges of the map.

		scroll_map();

		scroll_split( 0, 0, vdc_map_pxl_x & (BAT_W - 1), vdc_map_pxl_y & (BAT_H - 1), BKG_ON | SPR_ON );

		// Animate the Seran water by overwriting background characters (tiles) from
		// copies of the animation that are already in VRAM using VRAM-to-VRAM DMA.

		if (++water_delay == 6) {
			water_delay = 0;

			// Use vram2vram DMA to update the background animation in the next vblank.

			if (water_cycle ^= 1) {
				// 4 animation frames of 8x1 characters per frame,
				vram2vram( SHORE_CHR, SHORE_ANIM + (shore_frame * SHORE_SIZE), ( 8 * TILE_WORDS) );
				shore_frame = ++shore_frame & 3;
			} else {
				// 8 animation frames of 4x4 characters per frame,
				vram2vram( WATER_CHR, WATER_ANIM + (water_frame * WATER_SIZE), (16 * TILE_WORDS) );
				water_frame = ++water_frame & 7;
			}
		}

		// Animate the Beach waves using color-cycled palette data.
		//
		// The wave characters are drawn in palettes 12 and 13, and the color-cycle is
		// stored in memory as 16 frames of 2 palettes for each frame.

		if (++waves_delay == 8)
		{
			waves_delay = 0;
			set_far_offset( waves_frame, BANK(waves_pal), waves_pal );
			far_load_palette( 12, 2 );
			waves_frame = (waves_frame + PALETTE_BYTES * 2) & (PALETTE_BYTES * 2 * 16 - 1);
		}
	}

	// We could just use "seran_pal" as the reference for fading out the 
	// screen since in contains the entire background palette, but it is
	// useful to show how to read the existing VCE palette contents into
	// a buffer in RAM instead.

	read_palette( 0, 16, base_pal );

	// Flash the screen to white, but stop before the final step (7) because that is
	// the first step of the next fade.

	for (fade_val = 0; fade_val != 7; ++fade_val)
	{
		// Create a 256 color faded palette in the fade_pal[] array by adding the
		// fade_val (0..6) to each of the RGB color components in base_pal[].

		fade_to_white( base_pal, fade_pal, 256, fade_val );

		// Load the background palette from the fade_pal[] arrary.

		load_palette( 0, fade_pal, 16 );

		// Don't go too fast, there are only 8 steps!

		vsync( 5 );
	}

	// Fade back down from white, but stop before the final step (0) because that is
	// the first step of the next fade.

	for (fade_val = 7; fade_val != 0; --fade_val)
	{
		// Create a 256 color faded palette in the fade_pal[] array by adding the
		// fade_val (7..1) to each of the RGB color components in base_pal[].

		fade_to_white( base_pal, fade_pal, 256, fade_val );

		// Load the background palette from the fade_pal[] arrary.

		load_palette( 0, fade_pal, 16 );

		// Don't go too fast, there are only 8 steps!

		vsync( 5 );
	}

	// Finally fade down from the regular palette to black.

	for (fade_val = 0; fade_val != 8; ++fade_val)
	{
		// Create a 256 color faded palette in the fade_pal[] array by subtracting
		// the fade_val (0..7) from each of the RGB color components in base_pal[].

		fade_to_black( base_pal, fade_pal, 256, fade_val );

		// Load the background palette from the fade_pal[] arrary.

		load_palette( 0, fade_pal, 16 );

		// Don't go too fast, there are only 8 steps!

		vsync( 5 );
	}

	// Leave the screen black for 1/2 second, just to create a pause before the next demo.

	vsync( 30 );
}


// **************************************************************************
// **************************************************************************
//
// Switch between different demos (if there are different demos) ...
//
// **************************************************************************
// **************************************************************************

main()
{
	for (;;)
	{
		test_multiblk();
	}
}
