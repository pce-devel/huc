#include "huc.h"

const int paldata[] = { 0, 10, 20, 30, 40, 50, 60, 70, 80, 90, 0xA0, 0xB0, 0xC0, 0xD0, 0xE0, 0xF0 };
const int sprdata[] = { 1001, 2002, 3003, 4004, 5005, 6006, 7007, 8008, 9009, 10010, 11011, 12012, 13013, 14014, 15015, 16016 };

main()
{
	int posx, colorchange, result;
	init_satb();
	spr_set(0);
	spr_x(0);
	spr_y(0);
	spr_pal(0);
	spr_pri(1);
	spr_pattern(0x5000);
	spr_ctrl(FLIP_MAS|SIZE_MAS,NO_FLIP|SZ_16x16);
	load_vram(0x5000, sprdata, 0x40);
	load_palette(16,paldata,1);
	satb_update();
	for (posx = 0; posx < 256; posx++)
	{
		spr_set(0);
		spr_x(posx);
		spr_y(spr_get_y() + random(3));
		satb_update();
		for (colorchange = 257; colorchange < 272; colorchange++)
		{
			result = random(255) + random(255);
			set_color(colorchange,result);
		}
		vsync(3);
	}
}
