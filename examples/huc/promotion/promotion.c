#include "huc.h"

/* vram allocation */
#define TILE_VRAM    0x1000

/* promotion's exported tileset */
#define TILE_WIDTH 16
#incpal(testpal,"tilemap#tiles.png");
#inctile(testtiles,"tilemap#tiles.png");
#inctilepal(testtable,"tilemap#tiles.png");

/* promotion's exported map */
#define MAP_WIDTH  16
#define MAP_HEIGHT 16
#incbin(testmap,"tilemap.pmp#layer~Layer 1#map001.stm");

main()
{
	int sy = 0;

	/* disable display */
	disp_off();

	/* disable display */
	set_screen_size(SCR_SIZE_32x32);

	/* clear display */
	cls();

	/* init map */
	set_map_data(testmap,MAP_WIDTH,MAP_HEIGHT);
	set_tile_data(testtiles,256,testtable,TILE_WIDTH);
	load_tile(TILE_VRAM);
	load_map(0,0,0,0,MAP_WIDTH,MAP_HEIGHT);
	load_palette(0,testpal,16);

	/* init scroll */
	scroll(0, 0, (sy >> 1), 0, 223, 0xC0);

	/* enable display */
	disp_on();
	vsync();

	for (;;)
	{
		sy = ++sy & 511;
		scroll(0, 0, (sy >> 1), 0, 223, 0xC0);
		vsync();
	}

}
