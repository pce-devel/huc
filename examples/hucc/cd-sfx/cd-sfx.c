#include "huc.h"

main()
{
	unsigned char result;
	set_color_rgb(1, 7, 7, 7);
	set_font_color(1, 0);
	set_font_pal(0);
	load_default_font();
	put_string("Playing cd sfx...", 8, 13);

	// our sound effect is loaded from cd overlay index 2
	// we read 3 sectors of 2048 byts for a file size of 6144
	result = ad_trans(2, 0x0000, 3, 0x0000);
	if (result == 0)
		put_string("success!", 8, 16);
	else
		put_string("fail!", 8, 19);
	
	// play 6144 bytes of data from the address where the
	// file was loaded
	ad_play(0x0000, 6144, ADPLAY_FREQ_16KHZ, ADPLAY_AUTOSTOP);
}
