// **************************************************************************
// **************************************************************************
//
// shmup.c
//
// "hello, world" example of using KickC on the PC Engine.
//
// Sample game for HuC, using new language features (and NOT SimpleTracker).
// Based on the sample game at http://obeybrew.com/tutorials.html
//
// Copyright John Brandwood 2021.
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
// **************************************************************************
// **************************************************************************



// **************************************************************************
// **************************************************************************
//
// Some important #pragma settings for using KickC on the PCE.
//

// Set the target architecture.

#pragma target(pce)

// Use the __varcall calling method.
//
// This avoids passing parameters in registers, which is currently needed
// because our PCE call-trampolines use the A register for bank switching.

#pragma calling(__varcall)

// Put all C data/variables in the BSS section.
//
// This should *always* be set except for *small* sections of C code where
// you're specifically declaring some initialized data or graphics that go
// into the CODE or DATA sections.
//
// On the PCE, there is room for about 8KB of permanently-mapped const data
// in the CODE section, and the DATA section is reserved for graphics/sound
// data that gets banked into MPR3 & MPR4 as needed by the library routines
// which use far-data pointers.

#pragma data_seg(bss)

// Prefer using main memory for variables instead of zero-page?
//
// This can be set if the compiler runs out of ZP space.

#pragma var_model(mem)



// **************************************************************************
// **************************************************************************
//
// Include the basic PCE hardware definitions for KickC.
//

#include "pcengine.h"
#include "kickc.h"
#include "huc-gfx.h"



// **************************************************************************
// **************************************************************************
//
// Create some equates for a simple VRAM layout, with a 64*32 BAT, followed
// by the SAT, then followed by the tiles for the ASCII character set.
//

#define BAT_SIZE	(64 * 32)
#define SAT_ADDR	BAT_SIZE		// SAT takes 16 tiles of VRAM.
#define CHR_ZERO	(BAT_SIZE / 16)		// 1st tile # after the BAT.
#define CHR_0x10	(CHR_ZERO + 16)		// 1st tile # after the SAT.
#define CHR_0x20	(CHR_ZERO + 32)		// ASCII ' ' CHR tile #.
#define PALETTE1	(0x1 << 12)		// Palette # add for tile.



// **************************************************************************
// **************************************************************************
//
// Put the permanently-mapped const data into the CODE section ...
//

#pragma data_seg(code)

const char str_hi [] = "HI";
const char str_game_over [] = "GAME OVER";



// **************************************************************************
// **************************************************************************
//
// Put the banked-data into the DATA section ...
//
// This is used for C data that is banked into MPR3 & MPR4 as needed, and
// which library routines access with 24bit far-pointers.
//

#pragma data_seg(data)

char my_font [] = kickasm {{ .incbin "font8x8-ascii-bold-short.dat" }};

byte bonkspr [] = kickasm {{ .incspr "charwalk.pcx",0,0,2,8 }};
word bonkpal [] = kickasm {{ .incpal "charwalk.pcx" }};

byte bulletspr [] = kickasm {{ .incspr "bullet.pcx",0,0,1,1 }};
word bulletpal [] = kickasm {{ .incpal "bullet.pcx" }};

byte shipspr [] = kickasm {{ .incspr "ship.pcx",0,0,2,8 }};
word shippal [] = kickasm {{ .incpal "ship.pcx" }};

byte explosionspr [] = kickasm {{ .incspr "explosion.pcx",0,0,2,16 }};
word explosionpal [] = kickasm {{ .incpal "explosion.pcx" }};

byte scene_chr [] = kickasm {{ .incchr "scene.png" }};
word scene_pal [] = kickasm {{ .incpal "scene.png" }};

word __align(2) scene_bat [] = kickasm {{ .incbat "scene.png",4096,32,28 }};

unsigned int __align(2) textpal[] = {
	0x0000,0x0000,0x0000,0x0000,0x00C7,0x0001,0x01FF,0x0000,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
};

#pragma data_seg(bss)



// **************************************************************************
// **************************************************************************
//
// Prototypes for calling ASM library functions.
//
// Now *something* actually has to pull in the ASM code that these refer to!
//

// #pragma var_model(mem)

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
    signed char active;
    signed char vx, vy;
};

struct ship ships[MAX_SHIPS];

unsigned int frames;
unsigned int score, hiscore;



// **************************************************************************
// **************************************************************************
//
// do_ships()
//

void do_ships(void)
{
	unsigned int i,j;
	unsigned char r;
	struct ship *sp;
	struct bullet *bp;

	r = rand();
	if ((r & 0x7e) == 2) {
		for (i = 0, sp = ships; i < MAX_SHIPS; ++i, ++sp) {
			if (!sp->active) {
				sp->active = 1;
				spr_set(SHIP_SPRITE + i);
				if (r & 1) {
					spr_ctrl(FLIP_MAS|SIZE_MAS,FLIP_X|SZ_32x32);
					sp->vx = SPEED_SHIP;
					sp->vy = 0;
					sp->x = -32;
				}
				else {
					spr_ctrl(FLIP_MAS|SIZE_MAS,NO_FLIP_X|SZ_32x32);
					sp->vx = -SPEED_SHIP;
					sp->vy = 0;
					sp->x = 256;
				}
// CHANGED!			sp->y = rand() % 210;
				sp->y = (int) rand210();
				break;
			}
		}
	}

	for (i = 0, sp = ships; i < MAX_SHIPS; ++i, ++sp) {
		if (sp->active) {
			spr_set(SHIP_SPRITE + i);
			spr_x(sp->x);
			spr_y(sp->y);
			if (sp->active > 1) {
// CHANGED!			spr_pattern(0x5900 + (sp->active >> 3) * 0x100);
				spr_pattern(0x5900 + (((word) (sp->active) >> 3)) * 0x100);
				spr_pal(3);
				sp->active++;
				if (sp->active == 64) {
					sp->active = 0;
					spr_x(-32);
					spr_y(16);
					spr_pal(2);
				}
			}
			else {
				spr_pattern(0x5500 + (((frames >> 4) & 3) * 0x100));
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
							spr_set(BULLET_SPRITE + j);
							spr_x(-16);
							spr_y(0);
							score += SCORE_SHIP;
//							st_effect_noise(5, 15, 20);
//							st_set_vol(5, 15, 15);
//							st_set_env(5, snd_bullet_env);
						}
					}
				}
			}
			sp->x += sp->vx;
			sp->y += sp->vy;
			if (sp->x > 256 || sp->x < -32) {
				sp->active = 0;
				spr_x(-32);
				spr_y(16);
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



// **************************************************************************
// **************************************************************************
//
// main()
//

void main (void)
{
	unsigned int j1;
	int bonkx, bonky;
	unsigned int tic;
	unsigned char i, j;
	unsigned char bullet_wait;
	signed char bonk_dir;
	signed char r;
	unsigned char dead;
	struct ship *sp;
	struct bullet *bp;

	hiscore = 0;

	// Initialize VDC & VRAM.
	init_256x224();

//	st_init();
//	st_set_song(bank(bgm), bgm);

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

//	st_reset();
//	st_set_env(3, snd_bullet_env);
//	st_load_wave(3, snd_bullet_wave);
//	st_set_vol(3, 15, 15);
//	st_play_song();

	init_satb();
	spr_set(0);
	spr_x(bonkx);
	spr_y(bonky);
	spr_pattern(0x5000);
	spr_ctrl(FLIP_MAS|SIZE_MAS,FLIP_X|SZ_32x32);
	spr_pal(0);
	spr_pri(1);

	for (i = 0; i < MAX_BULLETS; i++) {
		spr_set(BULLET_SPRITE + i);
		spr_x(-16);
		spr_y(0);
		spr_pattern(0x5400);
		spr_ctrl(FLIP_MAS|SIZE_MAS,NO_FLIP|SZ_16x16);
		spr_pal(1);
		spr_pri(1);
		bullets[i].active = 0;
	}

	for (i = 0; i < MAX_SHIPS; i++) {
		spr_set(SHIP_SPRITE + i);
		spr_x(-32);
		spr_y(16);
		spr_pattern(0x5500);
		spr_ctrl(FLIP_MAS|SIZE_MAS,NO_FLIP|SZ_32x32);
		spr_pal(2);
		spr_pri(1);
		ships[i].active = 0;
	}
	load_palette(16,bonkpal,1);
	load_palette(17,bulletpal,1);
	load_palette(18,shippal,1);
	load_palette(19,explosionpal,1);

	load_vram(0x5000,bonkspr,0x400);
	load_vram(0x5400,bulletspr,0x40);
	load_vram(0x5500,shipspr,0x400);
	load_vram(0x5900,explosionspr,0x800);

	satb_update();

	load_background(scene_chr,scene_pal,scene_bat,32,28);

//	set_font_color(15, 6);
//	load_default_font();

	dropfnt8x8_vdc( my_font, (CHR_0x10 * 16), 16+96, 0xFF, 0x00 );
	load_palette(1,textpal,1);

	// Turn on the BG & SPR layers, then wait for a soft-reset.
	set_dspon();

	while(!dead)
	{
		wait_vsync();
		j1 = get_joy(0);
		if (get_joytrg(0) & JOY_RUN) {
			wait_vsync();
			while (!(get_joytrg(0) & JOY_RUN))
				wait_vsync();
		}
		if (j1 & JOY_LEFT)
		{
			spr_ctrl(FLIP_X_MASK,NO_FLIP_X);
			if (bonkx > -8) bonkx -= SPEED_X;
			tic++;
			bonk_dir = -1;
		}
		if (j1 & JOY_RIGHT)
		{
			spr_ctrl(FLIP_X_MASK,FLIP_X);
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
			for (bp = bullets, i = 0; i < MAX_BULLETS; ++i, ++bp) {
				if (!bp->active) {
					bp->active = bonk_dir;
					bp->x = bonkx + 8 + bonk_dir * 16;
					bp->y = bonky + 10;
					bullet_wait = 10;
					spr_set(BULLET_SPRITE + i);
					if (bonk_dir > 0) {
						spr_ctrl(FLIP_X_MASK, FLIP_X);
					} else {
						spr_ctrl(FLIP_X_MASK, NO_FLIP_X);
					}
//					st_effect_wave(3, 800, 16);
					break;
				}
			}
		}

		if (bullet_wait)
			bullet_wait--;

		bp = bullets;
		for (i = 0; i < MAX_BULLETS; i++) {
			if (bp->active) {
				spr_set(BULLET_SPRITE + i);
				spr_x(bp->x);
				spr_y(bp->y);
// CHANGED!			bp->x += bp->active * SPEED_BULLET;
				bp->x += (bp->active >= 0) ? (int) SPEED_BULLET : (int) -SPEED_BULLET;
				//put_number(bp->x, 4, 0, 0);
				if (bp->x > 256 || bp->x < -16) {
					bp->active = 0;
					spr_x(-16);
					spr_y(0);
				}
			}
			++bp;
		}

		do_ships();

		sp = ships;
		for (i = 0; i < MAX_SHIPS; i++) {
			if (sp->active == 1) {
				if ((sp->x > bonkx - 24 &&
				    sp->x < bonkx + 24 &&
				    sp->y > bonky - 20 &&
				    sp->y < bonky + 9) ||
				    (sp->x > bonkx - 18 &&
				    sp->x < bonkx + 18 &&
				    sp->y > bonky + 8 &&
				    sp->y < bonky + 25))
				{
// CHANGED!				put_string("GAME OVER", 11, 4);
					put_string(str_game_over, 11, 4);
					for (j = 0; j < 100; j++)
						wait_vsync();
					dead = 1;
					if (score > hiscore)
					    hiscore = score;
				}
			}
			sp++;
		}
		spr_set(0);
		spr_x(bonkx);
		//put_number(bonkx, 4, 0, 1);
		spr_y(bonky);
		spr_pattern(0x5000 + (((tic >> 2) & 3) * 0x100));
		satb_update();
		put_number(score, 5, 26, 1);
// CHANGED!	put_string("HI", 1, 1);
		put_string(str_hi, 1, 1);
		put_number(hiscore, 5, 4, 1);
		frames++;
	}
	}
}
