#include "huc.h"

main()
{
	unsigned char result;
	set_color_rgb(1, 7, 7, 7);
	set_font_color(1, 0);
	set_font_pal(0);
	load_default_font();
	put_string("Playing sfx...", 8, 13);

	result = ad_trans(2, 0x0, 3, 0x0);
	if (result == 0)
		put_string("success!", 8, 16);
	else
		put_string("fail!", 8, 19);
	
	ad_play(0, 6144, ADPLAY_FREQ_16KHZ, ADPLAY_AUTOSTOP);
}
