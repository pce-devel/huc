#include "huc.h"
#include "hucc-scroll.h"
#include "hucc-chrmap.h"
#include "hucc-blkmap.h"

// Get the full 256-color palette from the bitmap.

#incpal( seran_pal, "seran-map.png" );


// Extract all of the 8x8 characters (a.k.a. tiles) that are used in the
// bitmap, removing the repeated characters.

#incchr( seran_chr, "seran-map.png", 0, 0, 128, 128, _OPTIMIZE );


// Extract 16x16 pixel blocks (a.k.a. meta-tiles) from the bitmap using
// the optimized character set that has already been extracted.
//
// There are a maximum of 256 blocks in a BLK map, and the definitiuons
// are always 2KByte long and 2KByte aligned in memory.
//
// You need to tell the converter where in VRAM you are going to upload
// the character set.

#incblk( seran_blk, "seran-map.png", 0x1000, 0, 0, 64, 64, seran_chr );


// Extract the map in BLK format (max size is 128x128 blocks).
//
// This is used for medium sized scrolling backgrounds, and is also the
// format for the individual screens in a huge multi-screen background.

#incmap( seran_map, "seran-map.png", 0, 0, 64, 64, seran_blk );


// There is no support (yet) for extra game-specific information for each
// of the blocks.

unsigned char seran_flg[256];


//
// An example of scrolling a block map (a map with 16x16 meta-tiles).
//

test_blkmap()
{
	/*  disable display */
	disp_off();
	vsync();

	/*  clear display */
	cls();

	set_240x208();

	load_default_font();

	// You can use COUNTOF() to get the number of palettes in a .incpal.

	load_palette( 0, seran_pal, COUNTOF(seran_pal) );

//	// You can use COUNTOF() to get the number of characters in a .incchr.
//
//	load_vram( 0x1000, seran_chr, COUNTOF(seran_chr) << 4 );

	// You can also use SIZEOF() to get the number of bytes in a .incchr.

	load_vram( 0x1000, seran_chr, SIZEOF(seran_chr) >> 1 );

	// There are normally 256 blocks in a .incblk although there is a way
	// to store fewer, but that isn't the point of this example.

	set_blocks( seran_blk, seran_flg, 256 );

	// You can use COUNTOF() to get the width of a .incmap map.

	set_blkmap( seran_map, COUNTOF(seran_map) );

	vdc_map_pxl_x = 0;
	vdc_map_pxl_y = 0;
	vdc_map_draw_w = (240 / 8) + 1;
	vdc_map_draw_h = (208 / 8) + 1;

	draw_map();

	vsync();

	disp_on();

	/*  demo main loop */
	for (;;)
	{
		scroll_split(0, 0, vdc_map_pxl_x & 511, vdc_map_pxl_y & 255, BKG_ON | SPR_ON);

		vsync();

		/*  finish when START is pressed */
		if (joytrg(0) & JOY_STRT) {
			break;
		}

		/*  get move direction */
		if (joy(0) & JOY_LEFT) {
			if (vdc_map_pxl_x > 0)
				vdc_map_pxl_x -= 2;
		}
		else
		if (joy(0) & JOY_RIGHT) {
			if (vdc_map_pxl_x < (1024 - 256))
				vdc_map_pxl_x += 2;
		}

		if (joy(0) & JOY_UP) {
			if (vdc_map_pxl_y > 0)
				vdc_map_pxl_y -= 2;
		}
		else
		if (joy(0) & JOY_DOWN) {
			if (vdc_map_pxl_y < (1024 - 224))
				vdc_map_pxl_y += 2;
		}

		scroll_map();
	}

}


// Extract the map in BAT format (max size 256x32 or 128x64 characters).
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

#incbat( seran_bat, "seran-map.png", 0x1000, 0, 0, 128, 64, seran_chr );


//
// An example of scrolling a character map (in the VDC's BAT format).
//

test_chrmap()
{
	/*  disable display */
	disp_off();
	vsync();

	/*  clear display */
	cls();

	set_240x208();

	load_default_font();

	// You can just ignore the COUNTOF() if you already know the number of palettes.

	load_palette( 0, seran_pal, 16 );

	// You can use BANK() to get bank of a symbol, allowing this to work ...

	set_far_base( BANK(seran_chr), seran_chr );
	far_load_vram( 0x1000, SIZEOF(seran_chr) >> 1 );

	// You can use COUNTOF() to get the width of a .incbat map.

	set_chrmap( seran_bat, COUNTOF(seran_bat) );

	vdc_map_pxl_x = 0;
	vdc_map_pxl_y = 0;
	vdc_map_draw_w = (240 / 8) + 1;
	vdc_map_draw_h = (208 / 8) + 1;

	draw_bat();

	vsync();

	disp_on();

	/*  demo main loop */
	for (;;)
	{
		scroll_split(0, 0, vdc_map_pxl_x & 255, vdc_map_pxl_y & 255, BKG_ON | SPR_ON);

		vsync();

		/*  finish when START is pressed */
		if (joytrg(0) & JOY_STRT) {
			break;
		}

		/*  get move direction */
		if (joy(0) & JOY_LEFT) {
			if (vdc_map_pxl_x > 0)
				vdc_map_pxl_x -= 2;
		}
		else
		if (joy(0) & JOY_RIGHT) {
			if (vdc_map_pxl_x < (1024 - 256))
				vdc_map_pxl_x += 2;
		}

		if (joy(0) & JOY_UP) {
			if (vdc_map_pxl_y > 0)
				vdc_map_pxl_y -= 2;
		}
		else
		if (joy(0) & JOY_DOWN) {
			if (vdc_map_pxl_y < (512 - 224))
				vdc_map_pxl_y += 2;
		}

		scroll_bat();
	}
}

//
//
//

main()
{
	for (;;)
	{
		// An example of scrolling a block map (a map with 16x16 meta-tiles).

		test_blkmap();

		// An example of scrolling a character map (in the VDC's BAT format).

		test_chrmap();
	}
}
