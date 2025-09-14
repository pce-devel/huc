#include "huc.h"

#incspr(bonk,"bonkwalk.png",0,0,2,8);
#incpal(bonkpal,"bonkwalk.png",0);

#incchr(scene_chr,"scene.png");
#incpal(scene_pal,"scene.png");
#incbat(scene_bat,"scene.png",0x1000,32,28);

main()
{
	int j1, j2, bonkx;
	char tics, frame;
	bonkx = 104;
	init_satb();
	spr_make(0,104,153,0x5100,FLIP_MAS|SIZE_MAS,NO_FLIP|SZ_32x32,0,1);
	load_palette(16,bonkpal,1);
	set_color(256,0);
	load_vram(0x5000,bonk,0x400);
	satb_update();
	load_background(scene_chr,scene_pal,scene_bat,32,28);
	for (;;)
	{
		vsync();
		j1 = joy(0);
		j2 = joytrg(0);
		if (j1 & JOY_LEFT)
		{
			tics++;
			spr_ctrl(FLIP_X_MASK,FLIP_X);
			if (bonkx > -8) bonkx--;
		}
		if (j1 & JOY_RGHT)
		{
			tics++;
			spr_ctrl(FLIP_X_MASK,NO_FLIP_X);
			if (bonkx < 232) bonkx++;
		}
		if (j2 & JOY_STRT)
		{
			pause();
		}
		spr_x(bonkx);
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

void pause(void)
{
	for (;;)
	{
		vsync();
		if (joytrg(0) & JOY_STRT) return;
	}
}
