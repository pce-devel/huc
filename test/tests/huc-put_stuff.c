#include <huc.h>

int main()
{
	unsigned char i;

	load_default_font();
	set_color(1,0x1FF);
	put_string("Hi", 0, 0);
	
#ifdef __HUCC__

	/* HuCC only supports writing to an X,Y coordinate */
	for (i = 0; i < 10; i++)
		put_digit(i, i, 1);
	for (i = 0; i < 26; i++)
		put_char(i + 'A', i, 3);
	for (i = 0; i < 26; i++)
		put_vram(vram_addr(i, 6), i+129);
	for (i = 0; i < 15; i++)
		put_number(i, 2, i*2, 8);
	put_string("Done", 12, 17);

#else

	for (i = 0; i < 10; i++)
		put_digit(i, i, 1);
	for (i = 0; i < 10; i++)
		put_digit(i, 130 + i);
	for (i = 0; i < 26; i++)
		put_char(i + 'A', i, 3);
	for (i = 0; i < 26; i++)
		put_char(i + 'A', 258 + i);
	for (i = 0; i < 26; i++)
		put_raw(i+129, i, 6);
	for (i = 0; i < 26; i++)
		put_raw(i+129, 512-64+2+i);
	for (i = 0; i < 15; i++)
		put_number(i, 2, i*2, 8);
	for (i = 0; i < 15; i++)
		put_number(i, 2, 512 + 64 + 2 + i*2);
	put_string("Done", 1100);

#endif

	vsync();
	vsync();
	dump_screen();
	for(;;);
}
