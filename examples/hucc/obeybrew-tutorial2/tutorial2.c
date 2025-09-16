#include "huc.h"

main()
{
	int myint;
	char mychar;
	myint = 15000;
	mychar = 5;
	set_color_rgb(1, 7, 7, 7);
	set_font_color(1, 0);
	set_font_pal(0);
	load_default_font();
	put_number(myint, 5, 0, 0);
	put_number(mychar, 1, 0, 1);
	myint = myint + mychar;
	put_number(myint, 5, 0, 2);
	myint += mychar;
	put_number(myint, 5, 0, 3);
	myint++;
	put_number(myint, 5, 0, 4);
	myint = mychar - myint;
	myint -= mychar;
	myint--;
	put_number(myint, 6, 0, 5);
}
