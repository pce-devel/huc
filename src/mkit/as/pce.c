#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "defs.h"
#include "externs.h"
#include "protos.h"
#include "pce.h"

/* bitmask defines for the number of arguments */

#define NARGS_0_1_2 0b11111000
#define NARGS_0_2_4 0b11101010
#define NARGS_1_3_5 0b11010101
#define NARGS_2_4_6 0b10101011
#define NARGS_0_1_2_3_5 0b11000000
#define NARGS_1_2_3_4_5_6 0b10000001

/* locals */
unsigned char workspace[65536];	/* buffer for .inc and .def directives */

t_blk blk_info[256];		/* meta-tile info table */
t_blk *blk_hash[HASH_COUNT];	/* meta-tile hash table */
t_symbol *blk_lablptr;		/* meta-tile symbol reference */


/* ----
 * write_header()
 * ----
 * generate and write rom header
 */

void
pce_write_header(FILE *f, int banks)
{
	/* setup header */
	memset(workspace, 0, 512);
	workspace[0] = banks;

	/* write */
	fwrite(workspace, 512, 1, f);
}


/* ----
 * scan_8x8_tile()
 * ----
 * scan an 8x8 tile for its palette number
 */

int
pce_scan_8x8_tile(unsigned int x, unsigned int y)
{
	int i, j;
	unsigned int pixel;
	unsigned char *ptr;

	/* scan the tile only in the last pass */
	if (pass != LAST_PASS)
		return (0);

	/* tile address */
	ptr = pcx_buf + x + (y * pcx_w);

	/* scan the tile for the 1st non-zero pixel */
	for (i = 0; i < 8; i++) {
		for (j = 0; j < 8; j++) {
			pixel = ptr[j];
			if (pixel & 0x0F) return (pixel >> 4);
		}
		ptr += pcx_w;
	}

	/* default palette */
	return (0);
}


/* ----
 * scan_16x16_tile()
 * ----
 * scan a 16x16 tile for its palette number
 */

int
pce_scan_16x16_tile(unsigned int x, unsigned int y)
{
	int i, j;
	unsigned int pixel;
	unsigned char *ptr;

	/* scan the tile only in the last pass */
	if (pass != LAST_PASS)
		return (0);

	/* tile address */
	ptr = pcx_buf + x + (y * pcx_w);

	/* scan the tile for the 1st non-zero pixel */
	for (i = 0; i < 16; i++) {
		for (j = 0; j < 16; j++) {
			pixel = ptr[j];
			if (pixel & 0x0F) return (pixel >> 4);
		}
		ptr += pcx_w;
	}

	/* default palette */
	return (0);
}


/* ----
 * pack_8x8_tile()
 * ----
 * encode a 8x8 tile
 *
 * return the tile's palette number, or -1 if multiple palettes used
 */

int
pce_pack_8x8_tile(unsigned char *buffer, void *data, int line_offset, int format)
{
	int i, j;
	int cnt;
	unsigned int pixel, mask;
	unsigned char *ptr;
	unsigned int *packed;
	int palette = -1;
	int fail = 0;

	/* clear buffer */
	memset(buffer, 0, 32);

	/* encode the tile */
	switch (format) {
	case CHUNKY_TILE:
		/* 8-bit chunky format - from a pcx */
		cnt = 0;
		ptr = (unsigned char *)data;

		for (i = 0; i < 8; i++) {
			for (j = 0; j < 8; j++) {
				mask = 1 << j;
				pixel = ptr[j ^ 0x07];
				buffer[cnt] |= (pixel & 0x01) ? mask : 0;
				buffer[cnt + 1] |= (pixel & 0x02) ? mask : 0;
				buffer[cnt + 16] |= (pixel & 0x04) ? mask : 0;
				buffer[cnt + 17] |= (pixel & 0x08) ? mask : 0;
				if (pixel & 0x0F) {
					if (palette < 0)
						palette = pixel >> 4;
					else
					if (palette != pixel >> 4)
						fail = -1;
				}
			}
			ptr += line_offset;
			cnt += 2;
		}
		break;

	case PACKED_TILE:
		/* 4-bit packed format - from an array */
		cnt = 0;
		packed = (unsigned int *)data;

		for (i = 0; i < 8; i++) {
			pixel = packed[i];

			for (j = 0; j < 8; j++) {
				mask = 1 << j;
				buffer[cnt] |= (pixel & 0x01) ? mask : 0;
				buffer[cnt + 1] |= (pixel & 0x02) ? mask : 0;
				buffer[cnt + 16] |= (pixel & 0x04) ? mask : 0;
				buffer[cnt + 17] |= (pixel & 0x08) ? mask : 0;
				pixel >>= 4;
			}
			cnt += 2;
		}
		break;

	default:
		/* other formats not supported */
		error("Unsupported format passed to 'pack_8x8_tile'!");
		break;
	}

	/* default for a completely emtpy tile */
	if (palette < 0)
		palette = 0;

	/* return palette number or -1 */
	return (palette | fail);
}


/* ----
 * pack_16x16_tile()
 * ----
 * encode a 16x16 tile
 *
 * return the tile's palette number, or -1 if multiple palettes used
 */

int
pce_pack_16x16_tile(unsigned char *buffer, void *data, int line_offset, int format)
{
	int i, j;
	int cnt;
	unsigned int pixel, mask;
	unsigned char *ptr;
	int palette = -1;
	int fail = 0;

	/* pack the tile only in the last pass */
	if (pass != LAST_PASS)
		return (0);

	/* clear buffer */
	memset(buffer, 0, 128);

	/* encode the tile */
	switch (format) {
	case CHUNKY_TILE:
		/* 8-bit chunky format - from a pcx */
		cnt = 0;
		ptr = (unsigned char *)data;

		for (i = 0; i < 16; i++) {
			for (j = 0; j < 8; j++) {
				mask = 1 << j;
				pixel = ptr[j ^ 0x07];
				buffer[cnt] |= (pixel & 0x01) ? mask : 0;
				buffer[cnt + 1] |= (pixel & 0x02) ? mask : 0;
				buffer[cnt + 16] |= (pixel & 0x04) ? mask : 0;
				buffer[cnt + 17] |= (pixel & 0x08) ? mask : 0;
				if (pixel & 0x0F) {
					if (palette < 0)
						palette = pixel >> 4;
					else
					if (palette != pixel >> 4)
						fail = -1;
				}

				pixel = ptr[(j + 8) ^ 0x07];
				buffer[cnt + 32] |= (pixel & 0x01) ? mask : 0;
				buffer[cnt + 33] |= (pixel & 0x02) ? mask : 0;
				buffer[cnt + 48] |= (pixel & 0x04) ? mask : 0;
				buffer[cnt + 49] |= (pixel & 0x08) ? mask : 0;
				if (pixel & 0x0F) {
					if (palette < 0)
						palette = pixel >> 4;
					else
					if (palette != pixel >> 4)
						fail = -1;
				}
			}
			if (i == 7)
				cnt += 48;
			ptr += line_offset;
			cnt += 2;
		}
		break;

	default:
		/* other formats not supported */
		error("Unsupported format passed to 'pack_16x16_tile'!");
		break;
	}

	/* default for a completely emtpy tile */
	if (palette < 0)
		palette = 0;

	/* return palette number or -1 */
	return (palette | fail);
}


/* ----
 * pack_16x16_sprite()
 * ----
 * encode a 16x16 sprite
 *
 * return the sprite's palette number, or -1 if multiple palettes used
 */

int
pce_pack_16x16_sprite(unsigned char *buffer, void *data, int line_offset, int format)
{
	int i, j;
	int cnt;
	unsigned int pixel, mask;
	unsigned char *ptr;
	unsigned int *packed;
	int palette = -1;
	int fail = 0;

	/* clear buffer */
	memset(buffer, 0, 128);

	/* encode the sprite */
	switch (format) {
	case CHUNKY_TILE:
		/* 8-bit chunky format - from pcx */
		cnt = 0;
		ptr = (unsigned char *)data;

		for (i = 0; i < 16; i++) {
			/* right column */
			for (j = 0; j < 8; j++) {
				mask = 1 << j;
				pixel = ptr[j ^ 0x0F];
				buffer[cnt] |= (pixel & 0x01) ? mask : 0;
				buffer[cnt + 32] |= (pixel & 0x02) ? mask : 0;
				buffer[cnt + 64] |= (pixel & 0x04) ? mask : 0;
				buffer[cnt + 96] |= (pixel & 0x08) ? mask : 0;
				if (pixel & 0x0F) {
					if (palette < 0)
						palette = pixel >> 4;
					else
					if (palette != pixel >> 4)
						fail = -1;
				}
			}

			/* left column */
			for (j = 0; j < 8; j++) {
				mask = 1 << j;
				pixel = ptr[j ^ 0x07];
				buffer[cnt + 1] |= (pixel & 0x01) ? mask : 0;
				buffer[cnt + 33] |= (pixel & 0x02) ? mask : 0;
				buffer[cnt + 65] |= (pixel & 0x04) ? mask : 0;
				buffer[cnt + 97] |= (pixel & 0x08) ? mask : 0;
				if (pixel & 0x0F) {
					if (palette < 0)
						palette = pixel >> 4;
					else
					if (palette != pixel >> 4)
						fail = -1;
				}
			}
			ptr += line_offset;
			cnt += 2;
		}
		break;

	case PACKED_TILE:
		/* 4-bit packed format - from array */
		cnt = 0;
		packed = (unsigned int *)data;

		for (i = 0; i < 16; i++) {
			/* left column */
			pixel = packed[cnt];

			for (j = 0; j < 8; j++) {
				mask = 1 << j;
				buffer[cnt + 1] |= (pixel & 0x01) ? mask : 0;
				buffer[cnt + 33] |= (pixel & 0x02) ? mask : 0;
				buffer[cnt + 65] |= (pixel & 0x04) ? mask : 0;
				buffer[cnt + 97] |= (pixel & 0x08) ? mask : 0;
				pixel >>= 4;
			}

			/* right column */
			pixel = packed[cnt + 1];

			for (j = 0; j < 8; j++) {
				mask = 1 << j;
				buffer[cnt] |= (pixel & 0x01) ? mask : 0;
				buffer[cnt + 32] |= (pixel & 0x02) ? mask : 0;
				buffer[cnt + 64] |= (pixel & 0x04) ? mask : 0;
				buffer[cnt + 96] |= (pixel & 0x08) ? mask : 0;
				pixel >>= 4;
			}
			cnt += 2;
		}
		break;

	default:
		/* other formats not supported */
		error("Unsupported format passed to 'pack_16x16_sprite'!");
		break;
	}

	/* default for a completely emtpy sprite */
	if (palette < 0)
		palette = 0;

	/* return palette number or -1 */
	return (palette | fail);
}


/* ----
 * do_vram()
 * ----
 * .vram pseudo
 */

void
pce_vram(int *ip)
{
	/* define label */
	labldef(LOCATION);

	/* check if there's a label */
	if (lastlabl == NULL) {
		error("No label!");
		return;
	}

	/* get the VRAM address */
	if (!evaluate(ip, ';', 0))
		return;
	if (value >= 0x7F00) {
		error("Incorrect VRAM address!");
		return;
	}
	lastlabl->vram = value;

	/* output line */
	if (pass == LAST_PASS) {
		loadlc(value, 1);
		println();
	}
}


/* ----
 * do_pal()
 * ----
 * .pal pseudo
 */

void
pce_pal(int *ip)
{
	/* define label */
	labldef(LOCATION);

	/* check if there's a label */
	if (lastlabl == NULL) {
		error("No label!");
		return;
	}

	/* get the palette index */
	if (!evaluate(ip, ';', 0))
		return;
	if (value > 15) {
		error("Incorrect palette index!");
		return;
	}
	lastlabl->palette = value;

	/* output line */
	if (pass == LAST_PASS) {
		loadlc(value, 1);
		println();
	}
}


/* ----
 * do_defchr()
 * ----
 * .defchr pseudo
 */

void
pce_defchr(int *ip)
{
	unsigned int data[8];
	int i;

	/* output infos */
	data_loccnt = loccnt;
	data_size = 4;
	data_level = 3;

	/* check if there's a label */
	if (lablptr) {
		/* define label */
		labldef(LOCATION);

		/* get the VRAM address */
		if (!evaluate(ip, ',', 0))
			return;
		if (value >= 0x7F00) {
			error("Incorrect VRAM address!");
			return;
		}
		lablptr->vram = value;

		/* get the default palette */
		if (!evaluate(ip, ',', 0))
			return;
		if (value > 0x0F) {
			error("Incorrect palette index!");
			return;
		}
		lablptr->palette = value;
	}

	/* get tile data */
	for (i = 0; i < 8; i++) {
		/* get value */
		if (!evaluate(ip, (i < 7) ? ',' : ';', 0))
			return;

		/* store value */
		data[i] = value;
	}

	/* encode tile */
	if (pass == LAST_PASS)
		pce_pack_8x8_tile(workspace, data, 0, PACKED_TILE);

	/* store tile */
	putbuffer(workspace, 32);

	/* output line */
	if (pass == LAST_PASS)
		println();
}


/* ----
 * do_defpal()
 * ----
 * .defpal pseudo
 */

void
pce_defpal(int *ip)
{
	unsigned int color;
	unsigned int r, g, b;
	char c;

	/* define label */
	labldef(LOCATION);

	/* output infos */
	data_loccnt = loccnt;
	data_size = 2;
	data_level = 3;

	/* get data */
	for (;;) {
		/* get color */
		if (!evaluate(ip, 0, 0))
			return;

		/* store data on last pass */
		if (pass == LAST_PASS) {
			/* convert color */
			r = (value >> 8) & 0x7;
			g = (value >> 4) & 0x7;
			b = (value) & 0x7;
			color = (g << 6) + (r << 3) + (b);

			/* store color */
			putword(loccnt, color, DATA_OUT);
		}

		/* update location counter */
		loccnt += 2;

		/* check if there's another color */
		c = prlnbuf[(*ip)++];

		if (c != ',')
			break;
	}

	/* output line */
	if (pass == LAST_PASS)
		println();

	/* check errors */
	if (c != ';' && c != '\0')
		error("Syntax error!");
}


/* ----
 * do_defspr()
 * ----
 * .defspr pseudo
 */

void
pce_defspr(int *ip)
{
	unsigned int data[32];
	int i;

	/* output infos */
	data_loccnt = loccnt;
	data_size = 4;
	data_level = 3;

	/* check if there's a label */
	if (lablptr) {
		/* define label */
		labldef(LOCATION);

		/* get the VRAM address */
		if (!evaluate(ip, ',', 0))
			return;
		if (value >= 0x7F00) {
			error("Incorrect VRAM address!");
			return;
		}
		lablptr->vram = value;

		/* get the default palette */
		if (!evaluate(ip, ',', 0))
			return;
		if (value > 0x0F) {
			error("Incorrect palette index!");
			return;
		}
		lablptr->palette = value;
	}

	/* get sprite data */
	for (i = 0; i < 32; i++) {
		/* get value */
		if (!evaluate(ip, (i < 31) ? ',' : ';', 0))
			return;

		/* store value */
		data[i] = value;
	}

	/* encode sprite */
	pce_pack_16x16_sprite(workspace, data, 0, PACKED_TILE);

	/* store sprite */
	putbuffer(workspace, 128);

	/* output line */
	if (pass == LAST_PASS)
		println();
}


/* ----
 * do_incbat()
 * ----
 * build a BAT
 *
 * .incbat "filename", vram [[,x ,y] ,w ,h] [,tileref_label]
 */

void
pce_incbat(int *ip)
{
	int i, j;
	int x, y, w, h;
	int tx, ty;
	unsigned basechr, index;
	int tile_number;
	unsigned fail = 0;
	unsigned char tile_data[32];

	/* define label */
	labldef(LOCATION);

	/* allocate memory for the symbol's tags */
	if (lablptr && lablptr->tags == NULL) {
		if ((lablptr->tags = calloc(1, sizeof(t_tags))) == NULL) {
			error("Cannot allocate memory for tags!");
			return;
		}
	}

	/* output */
	if (pass == LAST_PASS)
		loadlc(loccnt, 0);

	/* assume no optional tile reference */
	tile_lablptr = NULL;

	/* get args */
	if (!pcx_get_args(ip, NARGS_1_2_3_4_5_6))
		return;

	/* check that last arg is a tile reference */
	if ((pcx_nb_args & 1) == 0) {
		if (expr_lablcnt == 0) {
			error("No tile table reference!");
			return;
		}
		if (expr_lablcnt > 1) {
			expr_lablcnt = 0;
			error("Too many tile table references!");
			return;
		}
		if (expr_lablptr->data_type != P_INCCHR) {
			error("Tile table reference is not a .INCCHR!");
			return;
		}
		if (pass == LAST_PASS && !pcx_set_tile(expr_lablptr, value)) {
			return;
		}
		--pcx_nb_args;
	}

	/* check that the VRAM arg is in range */
	if (pcx_arg[0] < 0x0400 || pcx_arg[0] > 0x7F00) {
		error("Tileset must begin at VRAM 0x0400..0x7F00!");
		return;
	}
	basechr = (pcx_arg[0] >> 4);

	/* set up x, y, w, h from the args */
	if (!pcx_parse_args(1, pcx_nb_args - 1, &x, &y, &w, &h, 8))
		return;

	/* build the BAT */
	if (pass == LAST_PASS) {
		index = 0;
		fail = 0;
		basechr = (pcx_arg[0] >> 4);

		for (i = 0; i < h; i++) {
			for (j = 0; j < w; j++) {
				/* tile coordinates */
				tx = x + (j << 3);
				ty = y + (i << 3);

				/* convert 8x8 tile data */
				int palette = pcx_pack_8x8_tile(tile_data, tx, ty);

				if (palette < 0) {
					error("Multiple palettes used in CHR at image (%d, %d)!", tx, ty);
					fail |= 1;
					palette = 0;
				}

				if (tile_lablptr == NULL) {
					/* Traditional conversion as a bitmap with wasted repeats */
					tile_number = (basechr & 0xFFF);
					basechr++;
				} else {
					/* Sensible conversion with repeats removed */
					tile_number = pcx_search_tile(tile_data, 32);
					if (tile_number == -1) {
						error("Unrecognized CHR at image (%d, %d)!", tx, ty);
						fail |= 2;
						tile_number = 0;
					}
					tile_number += (basechr & 0x0FFF);
				}

				tile_number |= palette << 12;
				workspace[2 * index + 0] = tile_number & 0xFF;
				workspace[2 * index + 1] = tile_number >> 8;
				index++;
			}
		}
	}

	/* store data */
	putbuffer(workspace, 2 * w * h);

	/* attach the BAT size to the label */
	if (lablptr) {
		lablptr->data_count = w;
		lablptr->data_type = P_INCBAT;
		lablptr->data_size = 2 * w * h;
		if (pass == LAST_PASS)
			lastlabl->size = 2;
	}

	/* output */
	if (pass == LAST_PASS)
		println();
}


/* ----
 * do_incpal()
 * ----
 * extract the palette of a PCX file
 *
 * .incpal "filename" [,which-palette [,number-of-palettes]]
 */

void
pce_incpal(int *ip)
{
	unsigned int temp;
	int i, start, nb;
	int r, g, b;

	/* define label */
	labldef(LOCATION);

	/* output */
	if (pass == LAST_PASS)
		loadlc(loccnt, 0);

	/* get args */
	if (!pcx_get_args(ip, NARGS_0_1_2))
		return;

	start = 0;
	nb = pcx_nb_colors;

	if (pcx_nb_args) {
		start = pcx_arg[0] << 4;

		if (pcx_nb_args == 1)
			nb = 16;
		else {
			nb = pcx_arg[1] << 4;
		}
	}

	/* check args */
	if (((start + nb) > 256) || (nb == 0)) {
		error("Palette index out of range!");
		return;
	}

	/* make data */
	if (pass == LAST_PASS) {
		for (i = 0; i < nb; i++) {
			r = pcx_pal[start + i][0];
			g = pcx_pal[start + i][1];
			b = pcx_pal[start + i][2];
			temp = ((r & 0xE0) >> 2) | ((g & 0xE0) << 1) | ((b & 0xE0) >> 5);
			workspace[2 * i] = temp & 0xff;
			workspace[2 * i + 1] = temp >> 8;
		}
	}

	/* store data */
	putbuffer(workspace, nb << 1);

	/* attach the number of loaded palettes to the label */
	if (lablptr) {
		lablptr->data_type = P_INCPAL;
		lablptr->data_size = nb * 2;
		lablptr->data_count = nb >> 4;
	}
	else
	if (lastlabl && lastlabl->data_type == P_INCPAL) {
		lastlabl->data_size += nb * 2;
		lastlabl->data_count += nb >> 4;
	}

	/* set the size after conversion in the last pass */
	if (lastlabl && lastlabl->data_type == P_INCPAL) {
		if (pass == LAST_PASS)
			lastlabl->size = 32;
	}

	/* output */
	if (pass == LAST_PASS)
		println();
}


/* ----
 * do_incspr()
 * ----
 * PCX to sprites
 *
 * .incspr "filename", [[,x ,y] ,w ,h] [, optimize]
 */

void
pce_incspr(int *ip)
{
	int i, j;
	int x, y, w, h;
	int sx, sy;
	int nb_sprite = 0;
	int total = 0;
	unsigned char optimize;
	unsigned int crc;
	unsigned int hash;
	unsigned char spr_data[128];

	/* define label */
	labldef(LOCATION);

	/* allocate memory for the symbol's tags */
	if (lablptr && lablptr->tags == NULL) {
		if ((lablptr->tags = calloc(1, sizeof(t_tags))) == NULL) {
			error("Cannot allocate memory for tags!");
			return;
		}
		/* allocate memory for the sprite metadata */
		if ((lablptr->tags->metadata = calloc(1, sizeof(tile) / sizeof(struct t_tile))) == NULL) {
			error("Cannot allocate memory for metadata!");
			return;
		}
	}

	/* output */
	if (pass == LAST_PASS)
		loadlc(loccnt, 0);

	/* get args */
	if (!pcx_get_args(ip, NARGS_0_1_2_3_5))
		return;

	/* odd number of args after filename if there is an "optimize" flag */
	optimize = 0;
	if ((pcx_nb_args & 1) != 0) {
		optimize = (pcx_arg[pcx_nb_args - 1] != 0);
		--pcx_nb_args;
	}

	/* set up x, y, w, h from the args */
	if (!pcx_parse_args(0, pcx_nb_args, &x, &y, &w, &h, 16))
		return;

	/* shortcut if we already know how much data is produced */
	if (pass == EXTRA_PASS && lastlabl && lastlabl->data_type == P_INCSPR) {
		/* output the cumulative size from the first .incspr of a set */
		if (lastlabl && lastlabl == lablptr)
			putbuffer(workspace, lablptr->data_size);
		return;
	}

	/* are we expanding a set of sprites that was just created? */
	if (lablptr == NULL && lastlabl != NULL && lastlabl->data_type == P_INCSPR) {
		nb_sprite = lastlabl->data_count;
	} else {
		tile_lablptr = lastlabl = lablptr;
		if (lablptr)
			tile_offset = lablptr->value;
		memset(tile_tbl, 0, sizeof(tile_tbl));
	}

	/* pack sprites */
	for (i = 0; i < h; i++) {
		for (j = 0; j < w; j++) {
			/* sprite coordinates */
			sx = x + (j << 4);
			sy = y + (i << 4);

			/* encode sprite */
			int palette = pcx_pack_16x16_sprite(spr_data, sx, sy);

			/* warning not error because PCEAS hasn't previously cared about this */
			if (palette < 0)
				warning("Multiple palettes used in SPR at image (%d, %d)!", sx, sy);
			else
			if (lastlabl)
				lastlabl->tags->metadata[nb_sprite] = palette;

			/* calculate tile crc */
			crc = crc_calc(spr_data, 128);
			hash = crc & (HASH_COUNT - 1);

			if (optimize) {
				/* search tile */
				t_tile *test_tile = tile_tbl[hash];
				while (test_tile) {
					if (test_tile->crc == crc &&
					    memcmp(test_tile->data, spr_data, 128) == 0)
						break;
					test_tile = test_tile->next;
				}

				/* ignore repeated tiles */
				if (test_tile) {
					continue;
				}
			}

			/* insert the new tile in the tile table */
			if (nb_sprite == (sizeof(tile) / sizeof(struct t_tile))) {
				error("Too many sprites in image! The maximum is %d.", sizeof(tile) / sizeof(struct t_tile));
				return;
			}

			tile[nb_sprite].next = tile_tbl[hash];
			tile[nb_sprite].index = nb_sprite;
			tile[nb_sprite].data = &rom[bank][loccnt];
			tile[nb_sprite].crc = crc;
			tile_tbl[hash] = &tile[nb_sprite];

			/* putbuffer only copies on the LAST_PASS and we really need it */
			if (pass != LAST_PASS && !stop_pass) {
				memcpy(tile[nb_sprite].data, spr_data, 128);
			}

			putbuffer(spr_data, 128);

			/* store tile */
			nb_sprite += 1;
			total += 128;
		}
	}

	/* attach the number of loaded characters to the label */
	if (lablptr) {
		lablptr->data_type = P_INCSPR;
		lablptr->data_size = total;
		lablptr->data_count = nb_sprite;
	}
	else
	if (lastlabl && lastlabl->data_type == P_INCSPR) {
		lastlabl->data_size += total;
		lastlabl->data_count = nb_sprite;
	}

	/* set the size after conversion in the last pass */
	if (lastlabl && lastlabl->data_type == P_INCSPR) {
		if (pass == LAST_PASS)
			lastlabl->size = 128;
	}

	/* output */
	if (pass == LAST_PASS)
		println();
}


/* ----
 * do_incmask()
 * ----
 * PCX to 1bpp sprite masks
 *
 * .incmask "filename", [[,x ,y] ,w ,h] [, optimize]
 */

void
pce_incmask(int *ip)
{
	int i, j;
	int x, y, w, h;
	int sx, sy;
	int nb_mask = 0;
	int total = 0;
	unsigned char optimize;
	unsigned int crc;
	unsigned int hash;
	unsigned char spr_data[128];

	/* define label */
	labldef(LOCATION);

	/* output */
	if (pass == LAST_PASS)
		loadlc(loccnt, 0);

	/* get args */
	if (!pcx_get_args(ip, NARGS_0_1_2_3_5))
		return;

	/* odd number of args after filename if there is an "optimize" flag */
	optimize = 0;
	if ((pcx_nb_args & 1) != 0) {
		optimize = (pcx_arg[pcx_nb_args - 1] != 0);
		--pcx_nb_args;
	}

	/* set up x, y, w, h from the args */
	if (!pcx_parse_args(0, pcx_nb_args, &x, &y, &w, &h, 16))
		return;

	/* shortcut if we already know how much data is produced */
	if (pass == EXTRA_PASS && lastlabl && lastlabl->data_type == P_INCMASK) {
		/* output the cumulative size from the first .incmask of a set */
		if (lastlabl && lastlabl == lablptr)
			putbuffer(workspace, lablptr->data_size);
		return;
	}

	/* are we expanding a set of sprites that was just created? */
	if (lablptr == NULL && lastlabl != NULL && lastlabl->data_type == P_INCMASK) {
		nb_mask = lastlabl->data_count;
	} else {
		tile_lablptr = lablptr;
		if (lablptr)
			tile_offset = lablptr->value;
		memset(tile_tbl, 0, sizeof(tile_tbl));
	}

	/* pack sprites */
	for (i = 0; i < h; i++) {
		for (j = 0; j < w; j++) {
			/* sprite coordinates */
			sx = x + (j << 4);
			sy = y + (i << 4);

			/* encode sprite to get its 1st bitplane */
			pcx_pack_16x16_sprite(spr_data, sx, sy);

			/* calculate mask crc */
			crc = crc_calc(spr_data, 32);
			hash = crc & (HASH_COUNT - 1);

			if (optimize) {
				/* search tile */
				t_tile *test_tile = tile_tbl[hash];
				while (test_tile) {
					if (test_tile->crc == crc &&
					    memcmp(test_tile->data, spr_data, 32) == 0)
						break;
					test_tile = test_tile->next;
				}

				/* ignore repeated tiles */
				if (test_tile) {
					continue;
				}
			}

			/* insert the new tile in the tile table */
			if (nb_mask == 256) {
				error("Too many mask sprites in image! The maximum is 256.");
				return;
			}

			tile[nb_mask].next = tile_tbl[hash];
			tile[nb_mask].index = nb_mask;
			tile[nb_mask].data = &rom[bank][loccnt];
			tile[nb_mask].crc = crc;
			tile_tbl[hash] = &tile[nb_mask];

			/* putbuffer only copies on the LAST_PASS and we really need it */
			if (pass != LAST_PASS && !stop_pass) {
				memcpy(tile[nb_mask].data, spr_data, 32);
			}

			putbuffer(spr_data, 32);

			/* store tile */
			nb_mask += 1;
			total += 32;
		}
	}

	/* attach the number of loaded masks to the label */
	if (lablptr) {
		lablptr->data_type = P_INCMASK;
		lablptr->data_size = total;
		lablptr->data_count = nb_mask;
	}
	else
	if (lastlabl && lastlabl->data_type == P_INCMASK) {
		lastlabl->data_size += total;
		lastlabl->data_count = nb_mask;
	}

	/* set the size after conversion in the last pass */
	if (lastlabl && lastlabl->data_type == P_INCMASK) {
		if (pass == LAST_PASS)
			lastlabl->size = 32;
	}

	/* output */
	if (pass == LAST_PASS)
		println();
}


/* ----
 * do_inctile()
 * ----
 * PCX to 16x16 tiles (max 256 tiles per inctile, wraps around if 257)
 *
 * .inctile "filename", [[,x ,y] ,w ,h]
 */

void
pce_inctile(int *ip)
{
	int i, j;
	int x, y, w, h;
	int tx, ty;
	int nb_tile = 0;

	/* define label */
	labldef(LOCATION);

	/* allocate memory for the symbol's tags */
	if (lablptr && lablptr->tags == NULL) {
		if ((lablptr->tags = calloc(1, sizeof(t_tags))) == NULL) {
			error("Cannot allocate memory for tags!");
			return;
		}
		/* allocate memory for the tile metadata */
		if ((lablptr->tags->metadata = calloc(1, sizeof(tile) / sizeof(struct t_tile))) == NULL) {
			error("Cannot allocate memory for metadata!");
			return;
		}
	}

	/* output */
	if (pass == LAST_PASS)
		loadlc(loccnt, 0);

	/* get args */
	if (!pcx_get_args(ip, NARGS_0_2_4))
		return;

	/* set up x, y, w, h from the args */
	if (!pcx_parse_args(0, pcx_nb_args, &x, &y, &w, &h, 16))
		return;

	/* pack tiles */
	for (i = 0; i < h; i++) {
		for (j = 0; j < w; j++) {
			/* tile coordinates */
			tx = x + (j << 4);
			ty = y + (i << 4);

			/* encode tile */
			int palette = pcx_pack_16x16_tile(workspace + 128 * (nb_tile % 256), tx, ty);

			/* warning not error because PCEAS hasn't previously cared about this */
			if (palette < 0)
				warning("Multiple palettes used in TILE at image (%d, %d)!", tx, ty);
			else
			if (lablptr)
				lablptr->tags->metadata[nb_tile] = palette;

			/* no optimization for old MagicKit 16x16 tiles */
			nb_tile++;

			/* max 256 tiles, with number 257 wrapping around to */
			/* tile 0 to deal with art programs (like ProMotion) */
			/* that always store a blank tile 0. */
			if (nb_tile == 257) {
				warning("Using the graphics from tile 256 as tile 0.");
			} else
			if (nb_tile >= 258) {
				error("Too many tiles in image! The maximum is 256.");
				return;
			}
		}
	}

	/* store a maximum of 256 tiles (32KB) */
	if (nb_tile > 256) nb_tile = 256;
	if (nb_tile)
		putbuffer(workspace, 128 * nb_tile);

	/* attach the number of loaded tiles to the label */
	if (lablptr) {
		lablptr->data_type = P_INCTILE;
		lablptr->data_size = 128 * nb_tile;
		lablptr->data_count = nb_tile;
	}
	else
	if (lastlabl && lastlabl->data_type == P_INCTILE) {
		lastlabl->data_size += 128 * nb_tile;
		lastlabl->data_count += nb_tile;
	}

	/* set the size after conversion in the last pass */
	if (lastlabl && lastlabl->data_type == P_INCTILE) {
		if (pass == LAST_PASS)
			lastlabl->size = 128;
	}

	/* output */
	if (pass == LAST_PASS)
		println();
}


/* ----
 * do_incblk()
 * ----
 * PCX to 16x16 blocks (aka 16x16 meta-tiles) (max 256 tiles per .incblk)
 *
 * .incblk "filename", vram [[,x ,y] ,w ,h] ,tileref_label
 */

void
pce_incblk(int *ip)
{
	int i, j;
	int x, y, w, h;
	int tx, ty, cx, cy;
	unsigned basechr, index;
	int nb_blks = 0;
	int tile_number;
	unsigned hash;
	unsigned fail = 0;
	unsigned char tile_data[32];
	uint64_t data;
	t_blk *blk;

	/* are we expanding a set of blocks that was just created? */
	if (lablptr != NULL || lastlabl == NULL || lastlabl != blk_lablptr) {
		lastlabl = NULL;
		blk_lablptr = NULL;
	}

	/* align new block definitions to a 2KByte boundary */
	if (blk_lablptr == NULL) {
		if (loccnt >= 0x2000) {
			loccnt &= 0x1FFF;
			bank = (bank + 1);
			if (section != S_DATA || asm_opt[OPT_DATAPAGE] == 0)
				page = (page + 1) & 7;
		}
		if (loccnt & 0x07FF) {
			/* update location counter */
			int oldloc = loccnt;
			loccnt = (loccnt + 0x0800) & 0x1800;

			if (loccnt == 0) {
				/* signal discontiguous change in loccnt */
				discontiguous = 1;
				bank = (bank + 1);
				if (section != S_DATA || asm_opt[OPT_DATAPAGE] == 0)
					page = (page + 1) & 7;
			} else {
				if ((section_flags[section] & S_IS_ROM) && (bank < UNDEFINED_BANK)) {
					uint32_t info, *fill_a;
					uint8_t *fill_b;
					int offset;

					memset(&rom[bank][oldloc], 0, loccnt - oldloc);
					memset(&map[bank][oldloc], section + (page << 5), loccnt - oldloc);

					info = debug_info(DATA_OUT);
					fill_a = &dbg_info[bank][oldloc];
					fill_b = &dbg_column[bank][oldloc];
					offset = loccnt - oldloc;
					while (offset--) {
						*fill_a++ = info;
						*fill_b++ = debug_column;
					}
				}
			}
		}
	}

	labldef(LOCATION);

	/* allocate memory for the symbol's tags */
	if (lablptr && lablptr->tags == NULL) {
		if ((lablptr->tags = calloc(1, sizeof(t_tags))) == NULL) {
			error("Cannot allocate memory for tags!");
			return;
		}
	}

	/* output */
	if (pass == LAST_PASS)
		loadlc(loccnt, 0);

	/* get args */
	if (!pcx_get_args(ip, NARGS_2_4_6))
		return;

	/* check that last arg is a tile reference */
	if (expr_lablcnt == 0) {
		error("No character set reference!");
		return;
	}
	if (expr_lablcnt > 1) {
		expr_lablcnt = 0;
		error("Too many character set references!");
		return;
	}
	if (expr_lablptr->data_type != P_INCCHR) {
		error("Character set reference is not a .INCCHR!");
		return;
	}
	if (pass == LAST_PASS && !pcx_set_tile(expr_lablptr, value)) {
		return;
	}
	--pcx_nb_args;

	/* check that the VRAM arg is in range and then subtract 0x1000 */
	if (pcx_arg[0] & 15) {
		error("Character set VRAM address must 16-word aligned!");
		return;
	}
	if (pcx_arg[0] < 0x1000 || pcx_arg[0] > 0x7000) {
		error("Character set VRAM address must be 0x1000..0x7000!");
		return;
	}
	basechr = (pcx_arg[0] >> 4) - 0x0100;

	/* set up x, y, w, h from the args */
	if (!pcx_parse_args(1, pcx_nb_args - 1, &x, &y, &w, &h, 16))
		return;

	if (pass == LAST_PASS) {
		/* pack blocks */
		unsigned char *packed;
		if (blk_lablptr) {
			/* expand an existing set of blocks */
			nb_blks = blk_lablptr->data_count;
			packed = &rom[blk_lablptr->rombank][blk_lablptr->value & 0x1FFF];
		} else {
			/* create a new set of blocks */
			nb_blks = 0;
			memset(blk_hash, 0, sizeof(blk_hash));
			memset(workspace, 0, 2048);
			packed = &workspace[0];
		}

		for (i = 0; i < h; i++) {
			for (j = 0; j < w; j++) {
				/* convert 16x16 block data */
				data = 0;
				hash = 0;
				index = 0;
				for (cy = 0; cy < 16; cy += 8) {
					for (cx = 0; cx < 16; cx += 8) {
						/* tile coordinates */
						tx = x + (j << 4) + cx;
						ty = y + (i << 4) + cy;

						/* convert 8x8 tile data */
						int palette = pcx_pack_8x8_tile(tile_data, tx, ty);

						if (palette < 0) {
							error("Multiple palettes used in CHR at image (%d, %d)!", tx, ty);
							fail |= 1;
							palette = 0;
						}

						tile_number = pcx_search_tile(tile_data, 32);
						if (tile_number == -1) {
							error("Unrecognized CHR at image (%d, %d)!", tx, ty);
							fail |= 2;
							tile_number = 0;
						}
						else
						if ((tile_number + basechr) >= 0x0700) {
							fail |= 4;
							tile_number = 0;
						}
						tile_number += basechr;
						tile_number |= palette << 12;

						/* save the tile number in PCE interleaved format */
						packed[index * 256 + nb_blks + 0x0000] = tile_number;
						packed[index * 256 + nb_blks + 0x0400] = tile_number >> 8;

						/* save the tile number in 64-bit linear format */
						data |= ((uint64_t) tile_number) << (index * 16);

						/* update a janky hash value */
						hash += (tile_number ^ ((tile_number >> 8) & 0xFF));

						index += 1;
					}
				}

				/* has the meta-tile already been defined */
				hash &= (HASH_COUNT - 1);
				blk = blk_hash[hash];
				while (blk) {
					if (blk->data == data) break;
					blk = blk->next;
				}

				/* add a new meta-tile */
				if (!blk) {
					if (nb_blks == 256) {
						fail |= 8;
						blk = &blk_info[0];
					} else {
						blk = &blk_info[0] + nb_blks++;
						blk->data = data;
						blk->next = blk_hash[hash];
						blk_hash[hash] = blk;
					}
				}
			}
		}
	}

	/* write out the meta-tile definitions */
	if (blk_lablptr == NULL)
		putbuffer(workspace, 2048);

	/* errors */
	if (fail & 4)
		error("One or more 8x8 tiles are located beyond 64KBytes of VRAM!");
	if (fail & 8)
		error("Too many meta-tiles in image! The maximum is 256.");
	if (fail)
		return;

	/* attach the number of blocks to the label */
	if (lastlabl) {
		blk_lablptr = lastlabl;
		lastlabl->tags->uses_sym = expr_lablptr;
		lastlabl->vram = pcx_arg[0];
		lastlabl->data_type = P_INCBLK;
		lastlabl->data_size = 2048;
		lastlabl->data_count = nb_blks;
		if (pass == LAST_PASS)
			lastlabl->size = 8;
	}

	/* output */
	if (pass == LAST_PASS)
		println();
}


/* ----
 * do_incchrpal()
 * ----
 * PCX to palette array for 8x8 tiles
 *
 * .incchrpal "filename", [[,x ,y] ,w ,h]
 */

void
pce_incchrpal(int *ip)
{
	int i, j;
	int x, y, w, h;
	int tx, ty;
	int nb_chr = 0;

	/* define label */
	labldef(LOCATION);

	/* output */
	if (pass == LAST_PASS)
		loadlc(loccnt, 0);

	/* get args */
	if (!pcx_get_args(ip, NARGS_0_2_4))
		return;

	/* set up x, y, w, h from the args */
	if (!pcx_parse_args(0, pcx_nb_args, &x, &y, &w, &h, 8))
		return;

	/* scan tiles */
	for (i = 0; i < h; i++) {
		for (j = 0; j < w; j++) {
			/* tile coordinates */
			tx = x + (j << 3);
			ty = y + (i << 3);

			/* get chr palette */
			workspace[0] = pce_scan_8x8_tile(tx, ty) << 4;
			/* store palette number */
			putbuffer(workspace, 1);
			nb_chr++;
		}
	}

	/* size */
	if (lablptr) {
		lablptr->data_type = P_INCCHRPAL;
		lablptr->data_size = nb_chr;
	}
	else
	if (lastlabl && lastlabl->data_type == P_INCCHRPAL) {
		lastlabl->data_size += nb_chr;
	}

	/* attach the number of tile palette bytes to the label */
	if (lastlabl && lastlabl->data_type == P_INCCHRPAL) {
		if (nb_chr) {
			lastlabl->data_count = nb_chr;
			if (pass == LAST_PASS)
				lastlabl->size = 1;
		}
	}

	/* output */
	if (pass == LAST_PASS)
		println();
}


/* ----
 * do_incsprpal()
 * ----
 * PCX to palette array for 16x16 sprites
 *
 * .incsprpal "filename", [[,x ,y] ,w ,h]
 */

void
pce_incsprpal(int *ip)
{
	int i, j;
	int x, y, w, h;
	int tx, ty;
	int nb_sprite = 0;
	unsigned char palette;

	/* define label */
	labldef(LOCATION);

	/* output */
	if (pass == LAST_PASS)
		loadlc(loccnt, 0);

	/* get args */
	if (!pcx_get_args(ip, NARGS_0_2_4))
		return;

	/* set up x, y, w, h from the args */
	if (!pcx_parse_args(0, pcx_nb_args, &x, &y, &w, &h, 16))
		return;

	/* scan sprites */
	for (i = 0; i < h; i++) {
		for (j = 0; j < w; j++) {
			/* sprite coordinates */
			tx = x + (j << 4);
			ty = y + (i << 4);

			/* get sprite palette */
			palette = pce_scan_16x16_tile(tx, ty);

			/* store palette number */
			putbuffer(&palette, 1);
			nb_sprite++;
		}
	}

	/* size */
	if (lablptr) {
		lablptr->data_type = P_INCSPRPAL;
		lablptr->data_size = nb_sprite;
	}
	else
	if (lastlabl && lastlabl->data_type == P_INCSPRPAL) {
		lastlabl->data_size += nb_sprite;
	}

	/* attach the number of sprite palette bytes to the label */
	if (lastlabl && lastlabl->data_type == P_INCSPRPAL) {
		if (nb_sprite) {
			lastlabl->data_count = nb_sprite;
			if (pass == LAST_PASS)
				lastlabl->size = 1;
		}
	}

	/* output */
	if (pass == LAST_PASS)
		println();
}


/* ----
 * do_inctilepal()
 * ----
 * PCX to palette array for 16x16 tiles
 *
 * .inctilepal "filename", [[,x ,y] ,w ,h]
 */

void
pce_inctilepal(int *ip)
{
	int i, j;
	int x, y, w, h;
	int tx, ty;
	int nb_tile = 0;

	/* define label */
	labldef(LOCATION);

	/* output */
	if (pass == LAST_PASS)
		loadlc(loccnt, 0);

	/* get args */
	if (!pcx_get_args(ip, NARGS_0_2_4))
		return;

	/* set up x, y, w, h from the args */
	if (!pcx_parse_args(0, pcx_nb_args, &x, &y, &w, &h, 16))
		return;

	/* tilepal must not cross a bank boundary */
	if ((loccnt & 0x1FFF) > 0x1F00) {
		error("Tile palette must not cross a bank boundary!");
		return;
	}

	/* scan tiles */
	for (i = 0; i < h; i++) {
		for (j = 0; j < w; j++) {
			/* tile coordinates */
			tx = x + (j << 4);
			ty = y + (i << 4);

			/* get tile palette */
			workspace[(nb_tile % 256)] = pce_scan_16x16_tile(tx, ty) << 4;
			nb_tile++;

			/* max 256 tiles, with number 257 wrapping around to */
			/* tile 0 to deal with art programs (like ProMotion) */
			/* that always store a blank tile 0. */
			if (nb_tile == 257) {
				warning("Using the palette from tile 256 for tile 0.");
			} else
			if (nb_tile >= 258) {
				error("Too many tiles in image! The maximum is 256.");
				return;
			}
		}
	}

	/* store a maximum of 256 tile palettes */
	if (nb_tile > 256) nb_tile = 256;
	if (nb_tile)
		putbuffer(workspace, nb_tile);

	/* size */
	if (lablptr) {
		lablptr->data_type = P_INCTILEPAL;
		lablptr->data_size = nb_tile;
	}
	else
	if (lastlabl && lastlabl->data_type == P_INCTILEPAL) {
		lastlabl->data_size += nb_tile;
	}

	/* attach the number of tile palette bytes to the label */
	if (lastlabl && lastlabl->data_type == P_INCTILEPAL) {
		if (nb_tile) {
			lastlabl->data_count = nb_tile;
			if (pass == LAST_PASS)
				lastlabl->size = 1;
		}
	}

	/* output */
	if (pass == LAST_PASS)
		println();
}


/* ----
 * do_incmap()
 * ----
 * .incmap pseudo - convert a 16x16 tiled PCX into a map
 *
 * .incmap "filename", [[,x ,y] ,w ,h] ,tileref_label
 */

void
pce_incmap(int *ip)
{
	int i, j;
	int x, y, w, h;
	int tx, ty, cx, cy;
	unsigned basechr, index;
	int tile_number;
	unsigned hash;
	unsigned fail = 0;
	uint64_t data;
	t_blk *this_blk;
	unsigned char tile_data[128];

	/* define label */
	labldef(LOCATION);

	/* allocate memory for the symbol's tags */
	if (lablptr && lablptr->tags == NULL) {
		if ((lablptr->tags = calloc(1, sizeof(t_tags))) == NULL) {
			error("Cannot allocate memory for tags!");
			return;
		}
	}

	/* output */
	if (pass == LAST_PASS)
		loadlc(loccnt, 0);

	/* get args */
	if (!pcx_get_args(ip, NARGS_1_3_5))
		return;

	/* check that last arg is a tile reference */
	if (expr_lablcnt == 0) {
		error("No tile table reference!");
		return;
	}
	if (expr_lablcnt > 1) {
		expr_lablcnt = 0;
		error("Too many tile table references!");
		return;
	}
	if ((expr_lablptr->data_type != P_INCTILE) &&
	    (expr_lablptr->data_type != P_INCBLK)) {
		error("Tile table reference must be a .INCTILE or .INCBLK!");
		return;
	}

	/* set up x, y, w, h from the args */
	if (!pcx_parse_args(0, pcx_nb_args - 1, &x, &y, &w, &h, 16))
		return;

	/* attach the map size to the label */
	if (lablptr) {
		lastlabl->tags->uses_sym = expr_lablptr;
		lablptr->data_count = w;
		lablptr->data_type = P_INCMAP;
		lablptr->data_size = w * h;
		if (pass == LAST_PASS)
			lastlabl->size = 1;
	}

	/* shortcut until we need to generate the final data */
	if (pass != LAST_PASS) {
		putbuffer(workspace, w * h);
		return;
	}

	/* pack map */
	if (expr_lablptr->data_type == P_INCTILE) {
		/* pack a 16x16 bitmap tile map */

		if (!pcx_set_tile(expr_lablptr, value))
			return;

		for (i = 0; i < h; i++) {
			for (j = 0; j < w; j++) {
				/* tile coordinates */
				tx = x + (j << 4);
				ty = y + (i << 4);

				/* get tile */
				int palette = pcx_pack_16x16_tile(tile_data, tx, ty);

				if (palette < 0) {
					error("Multiple palettes used in TILE at image (%d, %d)!", tx, ty);
					fail |= 1;
					palette = 0;
				}

				/* search tile */
				tile_number = pcx_search_tile(tile_data, 128);

				/* didn't find the tile */
				if (tile_number == -1) {
					error("Unrecognized TILE at image (%d, %d)!", tx, ty);
					fail |= 8;
					tile_number = 0;
				}

				/* store tile index */
				if (pass == LAST_PASS)
					putbyte(loccnt, tile_number & 0xFF, DATA_OUT);

				/* update location counter */
				loccnt++;
			}
		}
	} else {
		/* pack a 16x16 meta-tile map */

		if (!expr_lablptr->tags)
			return;
		if (!pcx_set_tile(expr_lablptr->tags->uses_sym, expr_lablptr->tags->uses_sym->value))
			return;

		basechr = (expr_lablptr->vram >> 4) - 0x0100;

		if (blk_lablptr != expr_lablptr) {
			/* regenerate the meta-tile hash table from the data in rom */
			unsigned char *packed = &rom[expr_lablptr->rombank][expr_lablptr->value & 0x1FFF];

			if (expr_lablptr->data_count == -1) {
				error("Incorrect meta-tile label reference!");
				return;
			}
			if (pass == LAST_PASS && expr_lablptr->size == 0) {
				error("Meta-tiles have not been compiled yet!");
				return;
			}

			blk_lablptr = expr_lablptr;
			memset(blk_hash, 0, sizeof(blk_hash));
			for (i = 0; i != blk_lablptr->data_count; i++) {
				data = 0;
				hash = 0;
				for (j = 0; j != 4; j++) {
					tile_number =
						(packed[j * 256 + i + 0x0000] << 0) +
						(packed[j * 256 + i + 0x0400] << 8);
					data |= ((uint64_t) tile_number) << (j * 16);
					hash += (tile_number ^ ((tile_number >> 8) & 0xFF));
				}
				hash &= (HASH_COUNT - 1);
				this_blk = &blk_info[i];
				this_blk->data = data;
				this_blk->next = blk_hash[hash];
				blk_hash[hash] = this_blk;
			}
		}

		for (i = 0; i < h; i++) {
			for (j = 0; j < w; j++) {
				/* convert 16x16 block data */
				data = 0;
				hash = 0;
				index = 0;
				for (cy = 0; cy < 16; cy += 8) {
					for (cx = 0; cx < 16; cx += 8) {
						/* tile coordinates */
						tx = x + (j << 4) + cx;
						ty = y + (i << 4) + cy;

						/* convert 8x8 tile data */
						int palette = pcx_pack_8x8_tile(tile_data, tx, ty);

						if (palette < 0) {
							error("Multiple palettes used in CHR at image (%d, %d)!", tx, ty);
							fail |= 1;
							palette = 0;
						}

						tile_number = pcx_search_tile(tile_data, 32);
						if (tile_number == -1) {
							error("Unrecognized CHR at image (%d, %d)!", tx, ty);
							fail |= 2;
							tile_number = 0;
						}
						else
						if ((tile_number + basechr) >= 0x0700) {
							fail |= 4;
							tile_number = 0;
						}
						tile_number += basechr;
						tile_number |= palette << 12;

						/* save the tile number in 64-bit linear format */
						data |= ((uint64_t) tile_number) << (index * 16);

						/* update a janky hash value */
						hash += (tile_number ^ ((tile_number >> 8) & 0xFF));

						index += 1;
					}
				}

				/* search tile */
				hash &= (HASH_COUNT - 1);
				this_blk = blk_hash[hash];
				while (this_blk) {
					if (this_blk->data == data) break;
					this_blk = this_blk->next;
				}

				/* didn't find the tile */
				if (!this_blk) {
					error("Unrecognized BLK at image (%d, %d)!", tx - 8, ty - 8);
					fail |= 8;
					this_blk = &blk_info[0];
				}

				tile_number = this_blk - &blk_info[0];

				/* store tile index */
				if (pass == LAST_PASS)
					putbyte(loccnt, tile_number & 0xFF, DATA_OUT);

				/* update location counter */
				loccnt++;
			}
		}
	}

	/* errors */
	if (fail & 4)
		error("One or more 8x8 tiles are located beyond 64KBytes of VRAM!");
	if (fail)
		return;

	/* output */
	if (pass == LAST_PASS)
		println();
}


/* ----
 * do_haltmap()
 * ----
 * PCX to add collision data flags to 16x16 blocks (aka 16x16 meta-tiles)
 *
 * .haltmap "filename" [[,x ,y] ,w ,h] ,map_label
 */

void
pce_haltmap(int *ip)
{
	int i, j, k, l;
	int x, y, w, h;
	int tx, ty, cx, cy;
	int duplicated = 0;

	t_symbol *maplabl;
	t_symbol *blklabl;
	t_symbol *chrlabl;

	labldef(LOCATION);

	/* output */
	if (pass == LAST_PASS)
		loadlc(loccnt, 0);

	/* get args */
	if (!pcx_get_args(ip, NARGS_1_3_5))
		return;

	/* verify the MAP reference */
	if (pcx_nb_args < 1 || pcx_lbl[pcx_nb_args - 1] == NULL) {
		error("No MAP reference!");
		return;
	}
	maplabl = pcx_lbl[--pcx_nb_args];
	if (maplabl->data_type != P_INCMAP) {
		error("MAP reference is not a .INCMAP!");
		return;
	}
	if (maplabl->tags == NULL) {
		error("MAP reference has no tags structure!");
		return;
	}
	blklabl = maplabl->tags->uses_sym;
	if (blklabl == NULL || blklabl->data_type != P_INCBLK) {
		error("MAP reference must be a block (meta-tile) map!");
		return;
	}
	if (blklabl->tags == NULL) {
		error("BLK reference has no tags structure!");
		return;
	}
	chrlabl = blklabl->tags->uses_sym;
	if (chrlabl == NULL || chrlabl->data_type != P_INCCHR) {
		error(".INCBLK reference does not itself reference a .INCCHR!");
		return;
	}

	/* set up x, y, w, h from the args */
	if (!pcx_parse_args(0, pcx_nb_args, &x, &y, &w, &h, 16))
		return;
	if (w != (maplabl->data_count)) {
		error(".HALTMAP image is not the same width as the .INCMAP!");
		return;
	}
	if (h != (maplabl->data_size / maplabl->data_count)) {
		error(".HALTMAP image is not the same height as the .INCMAP!");
		return;
	}

	/* only do the time-consuming stuff on the last pass */
	if (pass == LAST_PASS) {
		/* sanity checks */
		if (blklabl->size == 0) {
			error(".INCBLK reference has not been compiled yet!");
			return;
		}
		if (maplabl->size == 0) {
			error(".INCMAP reference has not been compiled yet!");
			return;
		}
		if (blklabl->flags & (FLG_MASK | FLG_OVER)) {
			warning(".HALTMAP after a .MASKMAP/.OVERMAP may change the BLK definitions!");
		}

		/* remember that masks have been generated for this .INCBLK/.INTILE */
		blklabl->flags |= FLG_HALT;

		/* allocate memory for tracking the BLK's collision status */
		if (blklabl->tags->metadata == NULL) {
			if ((blklabl->tags->metadata = calloc(512, 1)) == NULL) {
				error("Cannot allocate memory for .HALTMAP tracking!");
				return;
			}
		}

		/* table of BLK that have had their collision flags set */
		unsigned char *blkseen = blklabl->tags->metadata;

		/* links to identical BLK with different collision flags */
		unsigned char *nextblk = blklabl->tags->metadata + 256;

		unsigned char *blkdata = &rom[blklabl->rombank][blklabl->value & 0x1FFF];
		unsigned char *mapdata = &rom[maplabl->rombank][maplabl->value & 0x1FFF];

		unsigned char mask = (chrlabl->data_count > 1024) ? 0x08 : 0x0C;

		for (i = 0; i < h; i++) {
			for (j = 0; j < w; j++, mapdata++) {
				unsigned char blkindx = *mapdata;
				unsigned char flag[4];

				unsigned index = 0;
				for (cy = 0; cy < 16; cy += 8) {
					for (cx = 0; cx < 16; cx += 8) {
						/* tile coordinates */
						tx = x + (j << 4) + cx;
						ty = y + (i << 4) + cy;

						/* extract color used by collision image 8x8 character */
						unsigned char *ppixel = pcx_buf + tx + pcx_w * ty;
						int color = -1;
						for (k = 0; k < 8; k++) {
							for (l = 0; l < 8; l++) {
								if ((ppixel[l] & 0x0F) != 0) {
									if (color < 0)
										color = ppixel[l] & 0x0F;
									else
									if ((ppixel[l] & 0x0F) != color) {
										error("Multiple collision colors in CHR at image (%d, %d)!", tx, ty);
										k = 7;
										break;
									}
								}
							}
							ppixel += pcx_w;
						}
						if (color < 0) color = 0;

						/* swizzle color bits so b3=color1 and b2=color2or3 */
						/* to allow a character set >= 32KB to use a 1 bit flag */
						color = (((color & 1) << 3) | ((color & 2) << 1)) & mask;

						/* calculate where the collision flags are stored in the BLK data */
						unsigned char *blkflag = &blkdata[index * 256 + blkindx + 0x0400];

						/* set the collision flags */
						flag[index] = (*blkflag & ~mask) | color;

						index += 1;
					}
				}

				if (blkseen[blkindx] == 0) {
					/* this is the 1st time that we've seen this BLK */
					blkseen[blkindx] = 1;
					blkdata[0x0400 + 0 * 256 + blkindx] = flag[0];
					blkdata[0x0400 + 1 * 256 + blkindx] = flag[1];
					blkdata[0x0400 + 2 * 256 + blkindx] = flag[2];
					blkdata[0x0400 + 3 * 256 + blkindx] = flag[3];
					continue;
				}

				unsigned char found = 0;

				for (;;) {
					/* have we seen this BLK with these flags before */
					if (blkdata[0x0400 + 0 * 256 + blkindx] == flag[0] &&
					    blkdata[0x0400 + 1 * 256 + blkindx] == flag[1] &&
					    blkdata[0x0400 + 2 * 256 + blkindx] == flag[2] &&
					    blkdata[0x0400 + 3 * 256 + blkindx] == flag[3]) {
						found = 1;
						break;
						}
					if (nextblk[blkindx] == 0)
						break;
					blkindx = nextblk[blkindx];
				}

				if (found) {
					/* this could be different to the original BLK in the MAP! */
					*mapdata = blkindx;
				} else
				if (blklabl->data_count < 256) {
					/* add a duplicate BLK with the new collision flags */
					blkdata[0x0000 + 0 * 256 + blklabl->data_count] = blkdata[0x0000 + 0 * 256 + blkindx];
					blkdata[0x0000 + 1 * 256 + blklabl->data_count] = blkdata[0x0000 + 1 * 256 + blkindx];
					blkdata[0x0000 + 2 * 256 + blklabl->data_count] = blkdata[0x0000 + 2 * 256 + blkindx];
					blkdata[0x0000 + 3 * 256 + blklabl->data_count] = blkdata[0x0000 + 3 * 256 + blkindx];
					blkdata[0x0400 + 0 * 256 + blklabl->data_count] = flag[0];
					blkdata[0x0400 + 1 * 256 + blklabl->data_count] = flag[1];
					blkdata[0x0400 + 2 * 256 + blklabl->data_count] = flag[2];
					blkdata[0x0400 + 3 * 256 + blklabl->data_count] = flag[3];
					nextblk[blkindx] = blklabl->data_count;
					*mapdata = blklabl->data_count++;
					if (!duplicated++)
						warning("Collision flag differences are causing BLK to be duplicated!");
					fprintf(ERROUT, "       Warning: BLK duplicated for collision flags at image (%4d, %4d).\n", tx - 8, ty - 8);
				} else {
					/* no room for a duplicate BLK with the new collision flags */
					error("No room for new collision BLK at image (%4d, %4d)!", tx - 8, ty - 8);
				}
			}
		}
	}

	if (duplicated)
		fprintf(ERROUT, "       Warning: A total of %d BLK were duplicated for collision flags.\n", duplicated);

	/* output */
	if (pass == LAST_PASS)
		println();
}


/* ----
 * pce_maskmap()
 * ----
 * PCX to add a flag lookup table to a 16x16 tile or block (meta-tile) map
 *
 * .maskmap "filename" [[,x ,y] ,w ,h] ,map_label, mask_label
 */

void
pce_maskmap(int *ip)
{
	int i, j;
	int x, y, w, h;
	int sx, sy;
	unsigned fail = 0;
	unsigned crc;
	unsigned hash;
	unsigned char *masktable;
	unsigned char base_tile;

	t_symbol *maplabl;
	t_symbol *blklabl;
	t_symbol *msklabl;
	unsigned char spr_data[128];

	labldef(LOCATION);

	/* allocate memory for the symbol's tags */
	if (lablptr && lablptr->tags == NULL) {
		if ((lablptr->tags = calloc(1, sizeof(t_tags))) == NULL) {
			error("Cannot allocate memory for tags!");
			return;
		}
	}

	/* output */
	if (pass == LAST_PASS)
		loadlc(loccnt, 0);

	/* get args */
	if (!pcx_get_args(ip, NARGS_2_4_6))
		return;

	/* verify the SPR reference */
	if (pcx_nb_args < 2 || pcx_lbl[pcx_nb_args - 1] == NULL) {
		error("No SPR reference!");
		return;
	}
	msklabl = pcx_lbl[--pcx_nb_args];
	if (msklabl->data_type != P_INCMASK) {
		error("MASK reference is not a .INCMASK!");
		return;
	}

	/* verify the MAP reference */
	if (pcx_nb_args < 1 || pcx_lbl[pcx_nb_args - 1] == NULL) {
		error("No MAP reference!");
		return;
	}
	maplabl = pcx_lbl[--pcx_nb_args];
	if (maplabl->data_type != P_INCMAP) {
		error("MAP reference is not a .INCMAP!");
		return;
	}
	if (maplabl->tags == NULL) {
		error("MAP reference has no tags structure!");
		return;
	}
	blklabl = maplabl->tags->uses_sym;

	/* set up x, y, w, h from the args */
	if (!pcx_parse_args(0, pcx_nb_args, &x, &y, &w, &h, 16))
		return;

	if (w != (maplabl->data_count)) {
		error(".MASKMAP image is not the same width as the .INCMAP!");
		return;
	}
	if (h != (maplabl->data_size / maplabl->data_count)) {
		error(".MASKMAP image is not the same height as the .INCMAP!");
		return;
	}

	/* shortcut if we already know how much data is produced */
	if (pass == EXTRA_PASS && lastlabl && lastlabl->data_type == P_MASKMAP) {
		/* output the cumulative size from the first .maskmap of a set */
		if (lastlabl && lastlabl == lablptr)
			putbuffer(workspace, lablptr->data_size);
		return;
	}

	/* are we expanding a table of flags that was just created? */
	if (lablptr == NULL && lastlabl != NULL && lastlabl->data_type == P_MASKMAP) {
		masktable = &rom[lastlabl->rombank][lastlabl->value & 0x1FFF];
	} else {
		masktable = workspace;
		memset(masktable, 0, 256);
	}

	/* only do the time-consuming stuff on the last pass */
	if (pass == LAST_PASS) {
		/* sanity checks */
		if (maplabl->size == 0) {
			error(".INCMAP reference has not been compiled yet!");
			return;
		}
		if (msklabl->size == 0) {
			error(".INCMASK reference has not been compiled yet!");
			return;
		}

		/* remember that masks have been generated for this .INCBLK/.INTILE */
		blklabl->flags |= FLG_MASK;

		/* setup the hash table for the sprites */
		if (!pcx_set_tile(msklabl, msklabl->value))
			return;

		unsigned char *mapdata = &rom[maplabl->rombank][maplabl->value & 0x1FFF];

		for (i = 0; i < h; i++) {
			for (j = 0; j < w; j++) {
				/* get the map layer's tile for this coordinate */
				base_tile = *mapdata++;

				/* sprite coordinates */
				sx = x + (j << 4);
				sy = y + (i << 4);

				/* encode sprite to get its 1st bitplane */
				pcx_pack_16x16_sprite(spr_data, sx, sy);

				/* calculate mask crc */
				crc = crc_calc(spr_data, 32);
				hash = crc & (HASH_COUNT - 1);

				/* search for the mask */
				t_tile *test_tile = tile_tbl[hash];
				while (test_tile) {
					if (test_tile->crc == crc &&
					    memcmp(test_tile->data, spr_data, 32) == 0)
						break;
					test_tile = test_tile->next;
				}

				/* raise an error if there's no match */
				if (!test_tile) {
					error("Unrecognized mask sprite at image (%d, %d)!", sx, sy);
					fail |= 1;
					continue;
				}

				/* raise an error if the undlying tile already has a different mask */
				if (masktable[base_tile] != 0 && masktable[base_tile] != test_tile->index) {
					error("BLK already using a different mask sprite at image (%d, %d)!", sx, sy);
					fail |= 2;
					continue;
				}

				masktable[base_tile] = test_tile->index;
			}
		}
	}

	/* output the table of mask information */
	if (masktable == workspace)
		putbuffer(workspace, 256);

	/* errors */
	if (fail)
		return;

	/* size */
	if (lablptr) {
		lablptr->data_type = P_MASKMAP;
		lablptr->data_size = 256;
		lablptr->data_count = 256;
	}

	/* set the "size" when the mask table contains valid data */
	if (lastlabl && lastlabl->data_type == P_MASKMAP) {
		if (pass == LAST_PASS)
			lastlabl->size = 1;
	}

	/* output */
	if (pass == LAST_PASS)
		println();
}


/* ----
 * pce_overmap()
 * ----
 * PCX to add a sprite lookup table to a 16x16 tile or block (meta-tile) map
 *
 * .overmap "filename" [[,x ,y] ,w ,h] ,map_label, spr_label
 */

void
pce_overmap(int *ip)
{
	int i, j;
	int x, y, w, h;
	int sx, sy;
	unsigned fail = 0;
	unsigned crc;
	unsigned hash;
	unsigned char *flagtable;
	unsigned char base_tile;

	t_symbol *maplabl;
	t_symbol *blklabl;
	t_symbol *sprlabl;
	unsigned char spr_data[128];

	labldef(LOCATION);

	/* allocate memory for the symbol's tags */
	if (lablptr && lablptr->tags == NULL) {
		if ((lablptr->tags = calloc(1, sizeof(t_tags))) == NULL) {
			error("Cannot allocate memory for tags!");
			return;
		}
	}

	/* output */
	if (pass == LAST_PASS)
		loadlc(loccnt, 0);

	/* get args */
	if (!pcx_get_args(ip, NARGS_2_4_6))
		return;

	/* verify the SPR reference */
	if (pcx_nb_args < 2 || pcx_lbl[pcx_nb_args - 1] == NULL) {
		error("No SPR reference!");
		return;
	}
	sprlabl = pcx_lbl[--pcx_nb_args];
	if (sprlabl->data_type != P_INCSPR) {
		error("SPR reference is not a .INCSPR!");
		return;
	}

	/* verify the MAP reference */
	if (pcx_nb_args < 1 || pcx_lbl[pcx_nb_args - 1] == NULL) {
		error("No MAP reference!");
		return;
	}
	maplabl = pcx_lbl[--pcx_nb_args];
	if (maplabl->data_type != P_INCMAP) {
		error("MAP reference is not a .INCMAP!");
		return;
	}
	if (maplabl->tags == NULL) {
		error("MAP reference has no tags structure!");
		return;
	}
	blklabl = maplabl->tags->uses_sym;

	/* set up x, y, w, h from the args */
	if (!pcx_parse_args(0, pcx_nb_args, &x, &y, &w, &h, 16))
		return;

	if (w != (maplabl->data_count)) {
		error(".OVERMAP image is not the same width as the .INCMAP!");
		return;
	}
	if (h != (maplabl->data_size / maplabl->data_count)) {
		error(".OVERMAP image is not the same height as the .INCMAP!");
		return;
	}

	/* shortcut if we already know how much data is produced */
	if (pass == EXTRA_PASS && lastlabl && lastlabl->data_type == P_OVERMAP) {
		/* output the cumulative size from the first .flagmap of a set */
		if (lastlabl && lastlabl == lablptr)
			putbuffer(workspace, lablptr->data_size);
		return;
	}

	/* are we expanding a table of overlays that was just created? */
	if (lablptr == NULL && lastlabl != NULL && lastlabl->data_type == P_OVERMAP) {
		flagtable = &rom[lastlabl->rombank][lastlabl->value & 0x1FFF];
	} else {
		flagtable = workspace;
		memset(flagtable, 0, 256);
	}

	/* only do the time-consuming stuff on the last pass */
	if (pass == LAST_PASS) {
		/* sanity checks */
		if (maplabl->size == 0) {
			error(".INCMAP reference has not been compiled yet!");
			return;
		}
		if (sprlabl->size == 0) {
			error(".INCSPR reference has not been compiled yet!");
			return;
		}

		/* remember that overlays have been generated for this .INCBLK/.INTILE */
		blklabl->flags |= FLG_OVER;

		/* setup the hash table for the sprites */
		if (!pcx_set_tile(sprlabl, sprlabl->value))
			return;

		unsigned char *mapdata = &rom[maplabl->rombank][maplabl->value & 0x1FFF];

		for (i = 0; i < h; i++) {
			for (j = 0; j < w; j++) {
				/* get the map layer's tile for this coordinate */
				base_tile = *mapdata++;

				/* sprite coordinates */
				sx = x + (j << 4);
				sy = y + (i << 4);

				/* encode sprite */
				pcx_pack_16x16_sprite(spr_data, sx, sy);

				/* calculate sprite crc */
				crc = crc_calc(spr_data, 128);
				hash = crc & (HASH_COUNT - 1);

				/* search for the sprite */
				t_tile *test_tile = tile_tbl[hash];
				while (test_tile) {
					if (test_tile->crc == crc &&
					    memcmp(test_tile->data, spr_data, 128) == 0)
						break;
					test_tile = test_tile->next;
				}

				/* raise an error if there's no match */
				if (!test_tile) {
					error("Unrecognized overlay sprite at image (%d, %d)!", sx, sy);
					fail |= 1;
					continue;
				}

				/* raise an error if the undlying tile already has different flags */
				if (flagtable[base_tile] != 0 && flagtable[base_tile] != test_tile->index) {
					error("BLK already uses a different overlay sprite at image (%d, %d)!", sx, sy);
					fail |= 2;
					continue;
				}

				flagtable[base_tile] = test_tile->index;
			}
		}
	}

	/* output the table of flag information */
	if (flagtable == workspace)
		putbuffer(workspace, 256);

	/* errors */
	if (fail)
		return;

	/* size */
	if (lablptr) {
		lablptr->data_type = P_OVERMAP;
		lablptr->data_size = 256;
		lablptr->data_count = 256;
	}

	/* set the "size" when the flag table contains valid data */
	if (lastlabl && lastlabl->data_type == P_OVERMAP) {
		if (pass == LAST_PASS)
			lastlabl->size = 1;
	}

	/* output */
	if (pass == LAST_PASS)
		println();
}


/* ----
 * do_mml()
 * ----
 * .mml pseudo - music/sound macro language
 */

void
pce_mml(int *ip)
{
	int offset, bufsize, size;
	char mml[PATHSZ];
	char c;

	/* define label */
	labldef(LOCATION);

	/* output */
	if (pass == LAST_PASS)
		loadlc(loccnt, 0);

	/* start */
	bufsize = 8192;
	offset = mml_start(workspace);
	bufsize -= offset;

	/* extract and parse mml string(s) */
	for (;;) {
		/* get string */
		if (!getstring(ip, mml, PATHSZ - 1))
			return;

		/* parse string */
		size = mml_parse(workspace + offset, bufsize, mml);

		if (size == -1)
			return;

		/* adjust buffer size */
		offset += size;
		bufsize -= size;

		/* next string */
		c = prlnbuf[(*ip)++];

		if ((c != ',') && (c != ';') && (c != '\0')) {
			error("Syntax error!");
			return;
		}
		if (c != ',')
			break;

		/* skip spaces */
		while (isspace(prlnbuf[*ip]))
			(*ip)++;

		/* continuation char */
		if (prlnbuf[*ip] == '\\') {
			(*ip)++;

			/* skip spaces */
			while (isspace(prlnbuf[*ip]))
				(*ip)++;

			/* check */
			if (prlnbuf[*ip] == ';' || prlnbuf[*ip] == '\0') {
				/* output line */
				if (pass == LAST_PASS) {
					println();
					clearln();
				}

				/* read a new line */
				if (readline() == -1)
					return;

				/* rewind line pointer and continue */
				*ip = preproc_sfield;
			}
			else {
				error("Syntax error!");
				return;
			}
		}
	}

	/* stop */
	offset += mml_stop(workspace + offset);

	/* store data */
	putbuffer(workspace, offset);

	/* output */
	if (pass == LAST_PASS)
		println();
}


/* ----
 * pce_outpng()
 * ----
 * PCX to add collision data flags to 16x16 blocks (aka 16x16 meta-tiles)
 *
 * .outpng "filename" , palette_label ,map_label
 */

void
pce_outpng(int *ip)
{
	unsigned i;
	char c;
	char name[PATHSZ];
	t_symbol *arg[4];

	/* ignore this until the last pass */
	if (pass != LAST_PASS)
		return;

	/* disable ROM output when using this */
	no_rom_file = 1;

	/* get png output file name */
	if (!getstring(ip, name, PATHSZ - 1))
		return;

	/* get args */
	for (i = 0; i < 4; ++i) {
		arg[i] = NULL;

		/* skip spaces */
		while (isspace(c = prlnbuf[(*ip)++])) ;

		/* check syntax */
		if ((c != ',') && (c != ';') && (c != 0)) {
			error("Syntax error!");
			return;
		}
		if (c != ',')
			break;

		/* get label reference argument */
		if (!evaluate(ip, 0, 0))
			return;
		if (expr_lablcnt == 0) {
			error("Argument is not a label name!");
			return;
		}
		if (expr_lablcnt > 1) {
			error("Argument contains more than one label name!");
			return;
		}
		if (expr_lablptr->value != value) {
			error("Argument contains more than just a label name!");
			return;
		}

		/* store arg */
		arg[i] = expr_lablptr;
	}

	/* check end of line */
	if (!check_eol(ip))
		return;

	/* check the palette symbol */
	if (arg[0] == NULL) {
		error("Palette reference is missing!");
		return;
	}
	if ((arg[0]->data_type != P_INCPAL) &&
	    (arg[0]->data_type != P_INCBIN)) {
		error("Palette reference must be a .INCPAL or .INCBIN!");
		return;
	}

	/* check the data type */
	if (arg[1] == NULL) {
		error("Data reference is missing!");
		return;
	}

	/* a .incmap reference outputs the "solid" layer */
	if (arg[1]->data_type == P_INCMAP) {
		error("Not implemented, yet!");
		return;
	} else

	/* a .incblk reference outputs the optimized blocks */
	if (arg[1]->data_type == P_INCBLK) {
		error("Not implemented, yet!");
		return;
	} else

	{
		error("Unsupported data reference label type!");
		return;
	}

	/* output line */
	if (pass == LAST_PASS) {
		println();
	}
}
