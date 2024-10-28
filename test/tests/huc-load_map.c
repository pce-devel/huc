#include "huc.h"

#define C1 129
#define C2 130
#define C3 131

const char map[] = {
	C1, C1, C1, C3, C3, C1, C1, C1,
	C2, C2, C2, C2, C2, C2, C2, C2,
	C1, C1, C1, C1, C1, C1, C1, C1,
	C3, C3, C3, C1, C2, C3, C3, C3,
	C1, C1, C1, C1, C1, C1, C1, C1,
	C2, C2, C2, C2, C2, C2, C2, C2,
	C1, C1, C1, C1, C1, C1, C1, C1
};

const char ctable[0x100] = { 0 };

int main()
{
	unsigned char i;
	load_default_font();
	set_color_rgb(1,7,7,7);

#ifdef __HUCC__
	put_vram(0, 129);
#else
	put_raw(129, 0);
#endif

	set_map_data(map, 8, 7);

	set_map_tile_type(8);
	set_map_tile_base(0);
	set_map_pals(ctable);

	load_map(1, 1, 0, 0, 8, 7);
	load_map(5, 11, 1, 1, 7, 6);
	load_map(20, 11, 0, 3, 6, 1);
	load_map(20, 13, 2, 0, 5, 1);

#ifdef __HUCC__
	for (i = 0; i < 3; i++)
		put_hex(get_vram(i), 4, i*5, 9);
#else
	for (i = 0; i < 3; i++)
		put_hex(vram[i], 4, i*5, 9);
#endif

	vsync();
	vsync();
	dump_screen();
	for(;;);
}
