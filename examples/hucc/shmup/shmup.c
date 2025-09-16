/* Sample game for HuC, using new language features (and NOT SimpleTracker).
 * Based on the sample game at http://obeybrew.com/tutorials.html
 */

#include <huc.h>

#incspr(bonk,"charwalk.pcx",0,0,2,8)
#incpal(bonkpal,"charwalk.pcx")
#incspr(bullet,"bullet.pcx",0,0,1,1)
#incpal(bulletpal,"bullet.pcx")
#incspr(ship,"ship.pcx",0,0,2,8)
#incpal(shippal,"ship.pcx")
#incspr(explosion,"explosion.pcx",0,0,2,16)
#incpal(explosionpal,"explosion.pcx")

const unsigned char * aSpriteDataAddr[] = {
	&bonk, &bullet, &ship, &explosion };

const unsigned char aSpriteDataBank[] = {
	^bonk, ^bullet, ^ship, ^explosion };

#incchr(scene_chr,"scene.png")
#incpal(scene_pal,"scene.png")
#incbat(scene_bat,"scene.png",0x1000,32,28)

#define SPEED_X 2
#define SPEED_Y 2

#define SPEED_BULLET 4
#define MAX_BULLETS 10
#define BULLET_SPRITE 1

struct bullet {
    int x, y;
    signed char active;
};

struct bullet bullets[MAX_BULLETS];

#define MAX_SHIPS 5
#define SPEED_SHIP 1
#define SHIP_SPRITE (BULLET_SPRITE + MAX_BULLETS)
#define SCORE_SHIP 100

struct ship {
    int x, y;
    char active;
    char vx, vy;
};

struct ship ships[MAX_SHIPS];

/* put some of these variables into zero-page memory */
unsigned int frames;
__zp unsigned int score, hiscore;
__zp struct ship *pship;
static __zp struct bullet *pshot;

void do_ships(void)
{
	unsigned int i,j;
	unsigned char r;
	static __zp struct ship *sp;
	static __zp struct bullet *bp;

        r = rand();
        if ((r & 0x7e) == 2) {
                for (i = 0, sp = ships; i < MAX_SHIPS; ++i, ++sp) {
                        if (!sp->active) {
                                sp->active = 1;
                                sgx_spr_set(SHIP_SPRITE + i);
                                if (r & 1) {
                                        sgx_spr_ctrl(FLIP_MAS|SIZE_MAS,FLIP_X|SZ_32x32);
                                        sp->vx = SPEED_SHIP;
                                        sp->vy = 0;
                                        sp->x = -32;
                                }
                                else {
                                        sgx_spr_ctrl(FLIP_MAS|SIZE_MAS,NO_FLIP_X|SZ_32x32);
                                        sp->vx = -SPEED_SHIP;
                                        sp->vy = 0;
                                        sp->x = 256;
                                }
                                sp->y = rand() % 210;
                                break;
                        }
                }
        }

        for (i = 0, sp = ships; i < MAX_SHIPS; ++i, ++sp) {
                if (sp->active) {
                        sgx_spr_set(SHIP_SPRITE + i);
                        sgx_spr_x(sp->x);
                        sgx_spr_y(sp->y);
                        if (sp->active > 1) {
                                sgx_spr_pattern(0x5900 + (sp->active >> 3) * 0x100);
                                sgx_spr_pal(3);
                                sp->active++;
                                if (sp->active == 64) {
					sp->active = 0;
					sgx_spr_x(-32);
					sgx_spr_y(16);
					sgx_spr_pal(2);
				}
			}
			else {
				sgx_spr_pattern(0x5500 + (((frames >> 4) & 3) * 0x100));
				for (j = 0, bp = bullets; j < MAX_BULLETS; ++j, ++bp) {
					if (bp->active) {
						if (bp->x > sp->x - 4 &&
						    bp->x < sp->x + 20 &&
						    bp->y > sp->y - 3 &&
						    bp->y < sp->y + 16) {
							/* explosion */
							sp->active = 2;
							sp->vx = 0;
							sp->vy = 0;
							bp->active = 0;
							sgx_spr_set(BULLET_SPRITE + j);
							sgx_spr_x(-16);
							sgx_spr_y(0);
							score += SCORE_SHIP;
						}
					}
				}
			}
			sp->x += sp->vx;
			sp->y += sp->vy;
			if (sp->x > 256 || sp->x < -32) {
				sp->active = 0;
				sgx_spr_x(-32);
				sgx_spr_y(16);
			}
		}
	}
#ifdef DEBUG
	for (i = 0, sp = ships; i < MAX_SHIPS; ++i, ++sp) {
		if (sp->active) {
		    put_number(sp->x, 4, 1, 4+i);
		    put_number(sp->y, 4, 5, 4+i);
		}
	}
#endif
}

void main(void)
{
	unsigned int j1;
	int bonkx, bonky;
	unsigned int tic;
	unsigned char i, j;
	unsigned char bullet_wait;
	signed char bonk_dir;
	char r;
	unsigned char dead;

	hiscore = 0;

	/* no goto yet, so we have to use this instead */
	for (;;) {

	tic = 0;
	frames = 0;
	score = 0;
	dead = 0;
	bonkx = 104;
	bonky = 153;
	bullet_wait = 0;
	bonk_dir = 1;

	sgx_init_satb();
	sgx_spr_set(0);
	sgx_spr_x(bonkx);
	sgx_spr_y(bonky);
	sgx_spr_pattern(0x5000);
	sgx_spr_ctrl(FLIP_MAS|SIZE_MAS,FLIP_X|SZ_32x32);
	sgx_spr_pal(0);
	sgx_spr_pri(1);

	for (i = 0; i < MAX_BULLETS; i++) {
		sgx_spr_set(BULLET_SPRITE + i);
		sgx_spr_x(-16);
		sgx_spr_y(0);
		sgx_spr_pattern(0x5400);
		sgx_spr_ctrl(FLIP_MAS|SIZE_MAS,NO_FLIP|SZ_16x16);
		sgx_spr_pal(1);
		sgx_spr_pri(1);
		bullets[i].active = 0;
	}

	for (i = 0; i < MAX_SHIPS; i++) {
		sgx_spr_set(SHIP_SPRITE + i);
		sgx_spr_x(-32);
		sgx_spr_y(16);
		sgx_spr_pattern(0x5500);
		sgx_spr_ctrl(FLIP_MAS|SIZE_MAS,NO_FLIP|SZ_32x32);
		sgx_spr_pal(2);
		sgx_spr_pri(1);
		ships[i].active = 0;
	}
	load_palette(16,bonkpal,1);
	load_palette(17,bulletpal,1);
	load_palette(18,shippal,1);
	load_palette(19,explosionpal,1);

	// you can either load vram using the label
	sgx_load_vram(0x5000,bonk,0x400);

	// or by using using the bank and address
	set_far_base(aSpriteDataBank[0],aSpriteDataAddr[0]);
	sgx_far_load_vram(0x5000,0x400);

	// or by using using the bank and address with an offset
	set_far_offset(0,aSpriteDataBank[0],aSpriteDataAddr[0]);
	sgx_far_load_vram(0x5000,0x400);

	sgx_load_vram(0x5400,bullet,0x40);
	sgx_load_vram(0x5500,ship,0x400);
	sgx_load_vram(0x5900,explosion,0x800);

	sgx_satb_update();

	load_background(scene_chr,scene_pal,scene_bat,32,28);

	set_font_color(15, 6);
	load_default_font();

	while(!dead)
	{
		vsync();
		j1 = joy(0);
		if (joytrg(0) & JOY_RUN) {
			vsync();
			while (!(joytrg(0) & JOY_RUN))
				vsync();
		}
		if (j1 & JOY_LEFT)
		{
			sgx_spr_ctrl(FLIP_X_MASK,NO_FLIP_X);
			if (bonkx > -8) bonkx -= SPEED_X;
			tic++;
			bonk_dir = -1;
		}
		if (j1 & JOY_RIGHT)
		{
			sgx_spr_ctrl(FLIP_X_MASK,FLIP_X);
			if (bonkx < 232) bonkx += SPEED_X;
			tic++;
			bonk_dir = 1;
		}
		if (j1 & JOY_UP)
		{
			if (bonky > -8) bonky -= SPEED_Y;
		}
		if (j1 & JOY_DOWN)
		{
			if (bonky < 212) bonky += SPEED_Y;
		}
		if ((j1 & JOY_II) && !bullet_wait) {
			for (pshot = bullets, i = 0; i < MAX_BULLETS; ++i, ++pshot) {
				if (!pshot->active) {
					pshot->active = bonk_dir;
					pshot->x = bonkx + 8 + bonk_dir * 16;
					pshot->y = bonky + 10;
					bullet_wait = 10;
					sgx_spr_set(BULLET_SPRITE + i);
					if (bonk_dir > 0)
						sgx_spr_ctrl(FLIP_X_MASK, FLIP_X);
					else
						sgx_spr_ctrl(FLIP_X_MASK, NO_FLIP_X);
					break;
				}
			}
		}

		if (bullet_wait)
			bullet_wait--;

		pshot = bullets;
		for (i = 0; i < MAX_BULLETS; i++) {
			if (pshot->active) {
				sgx_spr_set(BULLET_SPRITE + i);
				sgx_spr_x(pshot->x);
				sgx_spr_y(pshot->y);
				pshot->x += pshot->active * SPEED_BULLET;
				//put_number(pshot->x, 4, 0, 0);
				if (pshot->x > 256 || pshot->x < -16) {
					pshot->active = 0;
					sgx_spr_x(-16);
					sgx_spr_y(0);
				}
			}
			++pshot;
		}

		do_ships();

		pship = ships;
		for (i = 0; i < MAX_SHIPS; i++) {
			if (pship->active == 1 &&
			    ((pship->x > bonkx - 24 &&
			    pship->x < bonkx + 24 &&
			    pship->y > bonky - 20 &&
			    pship->y < bonky + 9) ||
			    (pship->x > bonkx - 18 &&
			    pship->x < bonkx + 18 &&
			    pship->y > bonky + 8 &&
			    pship->y < bonky + 25))) {
				put_string("GAME OVER", 11, 12);
				for (j = 0; j < 100; j++)
					vsync();
				dead = 1;
				if (score > hiscore)
				    hiscore = score;
			}
			pship++;
		}
		sgx_spr_set(0);
		sgx_spr_x(bonkx);
		//put_number(bonkx, 4, 0, 1);
		sgx_spr_y(bonky);
		sgx_spr_pattern(0x5000 + (((tic >> 2) & 3) * 0x100));
		sgx_satb_update();
		put_number(score, 5, 26, 1);
		put_string("HI", 1, 1);
		put_number(hiscore, 5, 4, 1);
		frames++;
	}
	}
}
