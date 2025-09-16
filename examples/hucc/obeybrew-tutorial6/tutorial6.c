#include "huc.h"

#incspr(bonk,"bonkwalk.png",0,0,2,8);
#incpal(bonkpal,"bonkwalk.png",0);

#incbin(levelmap,"tut6map.stm");
#incchr(leveltiles,"tut6chr.png");
#incchrpal(tilecolors,"tut6chr.png");
#incpal(levelpal0,"tut6chr.png",0);
#incpal(levelpal1,"tut6chr.png",1);

int bonkx;
char tics, frame;

void main(void)
{
	init_satb();
	tics = 0;
	frame = 0;
	bonkx = 104;
	spr_make(0,104,153,0x5100,FLIP_MAS|SIZE_MAS,NO_FLIP|SZ_32x32,0,1);
	load_palette(16,bonkpal,1);
	set_color(256,0);
	load_vram(0x5000,bonk,0x400);
	satb_update();  
	set_map_data(levelmap,64,28);
	set_tile_data(leveltiles,COUNTOF(leveltiles),tilecolors,8);
	load_tile(0x1000);
	load_map(0,0,0,0,64,28);
	load_palette(0,levelpal0,1);
	load_palette(1,levelpal1,1);
	for (bonkx = 0; bonkx < 256; bonkx ++)
	{
		scroll(0,bonkx,0,0,223,0xC0);
		tics++;
		if (tics > 4)
		{
			tics = 0;
			frame++;
			if (frame > 5) frame = 0;
			if (frame == 0) spr_pattern(0x5100);
			if (frame == 1) spr_pattern(0x5200);
			if (frame == 2) spr_pattern(0x5300);
			if (frame == 3) spr_pattern(0x5200);
			if (frame == 4) spr_pattern(0x5100);
			if (frame == 5) spr_pattern(0x5000);
		}
		satb_update();
		vsync();
	}
}

void spr_make(int spriteno,int spritex,int spritey,int spritepattern,int ctrl1,int ctrl2,int sprpal,int sprpri)
{
	spr_set(spriteno);
	spr_x(spritex);
	spr_y(spritey);
	spr_pattern(spritepattern);
	spr_ctrl(ctrl1,ctrl2);
	spr_pal(sprpal);
	spr_pri(sprpri);
}
