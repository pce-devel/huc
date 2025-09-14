#include "huc.h"

#incspr(bonk,"bonk.png",0,0,2,2);
#incpal(bonkpal,"bonk.png",0);

#incchr(scene_chr,"scene.png");
#incpal(scene_pal,"scene.png");
#incbat(scene_bat,"scene.png",0x1000,32,28);

main()
{
	int j1, bonkx;
	bonkx = 104;
	init_satb();
	spr_make(0,104,153,0x5000,FLIP_MAS|SIZE_MAS,NO_FLIP|SZ_32x32,0,1);
	load_palette(16,bonkpal,1);
	load_vram(0x5000,bonk,0x100);
	satb_update();
	load_background(scene_chr,scene_pal,scene_bat,32,28);
	for (;;)
	{
		vsync();
		j1 = joy(0);
		if (j1 & JOY_LEFT)
		{
			spr_ctrl(FLIP_X_MASK,FLIP_X);
			if (bonkx > -8) bonkx--;
		}
		if (j1 & JOY_RGHT)
		{
			spr_ctrl(FLIP_X_MASK,NO_FLIP_X);
			if (bonkx < 232) bonkx++;
		}
		spr_x(bonkx);
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
