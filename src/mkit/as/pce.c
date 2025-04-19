#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include "defs.h"
#include "externs.h"
#include "protos.h"
#include "pce.h"

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
 */

int
pce_pack_8x8_tile(unsigned char *buffer, void *data, int line_offset, int format)
{
	int i, j;
	int cnt;
	unsigned int pixel, mask;
	unsigned char *ptr;
	unsigned int *packed;

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
				pixel = ptr[j ^ 0x07];
				mask = 1 << j;
				buffer[cnt] |= (pixel & 0x01) ? mask : 0;
				buffer[cnt + 1] |= (pixel & 0x02) ? mask : 0;
				buffer[cnt + 16] |= (pixel & 0x04) ? mask : 0;
				buffer[cnt + 17] |= (pixel & 0x08) ? mask : 0;
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

	/* ok */
	return (0);
}


/* ----
 * pack_16x16_tile()
 * ----
 * encode a 16x16 tile
 */

int
pce_pack_16x16_tile(unsigned char *buffer, void *data, int line_offset, int format)
{
	int i, j;
	int cnt;
	unsigned int pixel, mask;
	unsigned char *ptr;

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
				pixel = ptr[j ^ 0x07];
				mask = 1 << j;
				buffer[cnt] |= (pixel & 0x01) ? mask : 0;
				buffer[cnt + 1] |= (pixel & 0x02) ? mask : 0;
				buffer[cnt + 16] |= (pixel & 0x04) ? mask : 0;
				buffer[cnt + 17] |= (pixel & 0x08) ? mask : 0;

				pixel = ptr[(j + 8) ^ 0x07];
				buffer[cnt + 32] |= (pixel & 0x01) ? mask : 0;
				buffer[cnt + 33] |= (pixel & 0x02) ? mask : 0;
				buffer[cnt + 48] |= (pixel & 0x04) ? mask : 0;
				buffer[cnt + 49] |= (pixel & 0x08) ? mask : 0;
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

	/* ok */
	return (0);
}


/* ----
 * pack_16x16_sprite()
 * ----
 * encode a 16x16 sprite
 */

int
pce_pack_16x16_sprite(unsigned char *buffer, void *data, int line_offset, int format)
{
	int i, j;
	int cnt;
	unsigned int pixel, mask;
	unsigned char *ptr;
	unsigned int *packed;

	/* pack the sprite only in the last pass */
	if (pass != LAST_PASS)
		return (0);

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
				pixel = ptr[j ^ 0x0F];
				mask = 1 << j;
				buffer[cnt] |= (pixel & 0x01) ? mask : 0;
				buffer[cnt + 32] |= (pixel & 0x02) ? mask : 0;
				buffer[cnt + 64] |= (pixel & 0x04) ? mask : 0;
				buffer[cnt + 96] |= (pixel & 0x08) ? mask : 0;
			}

			/* left column */
			for (j = 0; j < 8; j++) {
				pixel = ptr[j ^ 0x07];
				mask = 1 << j;
				buffer[cnt + 1] |= (pixel & 0x01) ? mask : 0;
				buffer[cnt + 33] |= (pixel & 0x02) ? mask : 0;
				buffer[cnt + 65] |= (pixel & 0x04) ? mask : 0;
				buffer[cnt + 97] |= (pixel & 0x08) ? mask : 0;
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

	/* ok */
	return (0);
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
	int i, j, k, l;
	int x, y, w, h;
	unsigned basechr, index;
	int tile_number;
	unsigned fail = 0;
	unsigned char tile_data[32];

	/* define label */
	labldef(LOCATION);

	/* output */
	if (pass == LAST_PASS)
		loadlc(loccnt, 0);

	/* assume no optional tile reference */
	tile_lablptr = NULL;

	/* get args */
	if (!pcx_get_args(ip))
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
		if (!pcx_set_tile(expr_lablptr, value)) {
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
				/* extract palette */
				unsigned char *ppixel = pcx_buf + (x + j * 8) + pcx_w * (y + i * 8);
				int palette = -1;
				for (k = 0; k < 8; k++) {
					for (l = 0; l < 8; l++) {
						if ((ppixel[l] & 0x0F) != 0) {
							if (palette < 0)
								palette = ppixel[l] & 0xF0;
							else
							if ((ppixel[l] & 0xF0) != palette)
								fail |= 1;
						}
					}
					ppixel += pcx_w;
				}
				if (palette < 0) palette = 0;

				/* convert 8x8 tile data */
				if (tile_lablptr == NULL) {
					/* Traditional conversion as a bitmap with wasted repeats */
					tile_number = (basechr & 0xFFF);
					basechr++;
				} else {
					/* Sensible conversion with repeats removed */
					pcx_pack_8x8_tile(tile_data, x + (j << 3), y + (i << 3));
					tile_number = pcx_search_tile(tile_data, 32);
					if (tile_number == -1) {
						/* didn't find the tile */
						tile_number = 0;
						fail |= 2;
					}
					tile_number += (basechr & 0x0FFF);
				}

				tile_number |= palette << 8;
				workspace[2 * index + 0] = tile_number & 0xFF;
				workspace[2 * index + 1] = tile_number >> 8;
				index++;
			}
		}

		/* errors */
		if (fail & 1)
			error("One or more 8x8 tiles contain pixels in multiple palettes!");
		if (fail & 2)
			error("One or more 8x8 tiles are not in the referenced tileset!");
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
	if (!pcx_get_args(ip))
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
		lablptr->data_size = 32 * nb;
		lablptr->data_count = nb >> 4;
	}
	else
	if (lastlabl && lastlabl->data_type == P_INCPAL) {
		lastlabl->data_size += 32 * nb;
		lastlabl->data_count += nb >> 4;
	}

	/* set the size after conversion in the last pass */
	if (lastlabl) {
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
 * .incspr "filename", [[,x ,y] ,w ,h]
 */

void
pce_incspr(int *ip)
{
	int i, j;
	int x, y, w, h;
	int sx, sy;
	int nb_sprite = 0;

	/* define label */
	labldef(LOCATION);

	/* output */
	if (pass == LAST_PASS)
		loadlc(loccnt, 0);

	/* get args */
	if (!pcx_get_args(ip))
		return;

	/* set up x, y, w, h from the args */
	if (!pcx_parse_args(0, pcx_nb_args, &x, &y, &w, &h, 16))
		return;

	/* pack sprites */
	for (i = 0; i < h; i++) {
		for (j = 0; j < w; j++) {
			/* sprite coordinates */
			sx = x + (j << 4);
			sy = y + (i << 4);

			/* encode sprite */
			pcx_pack_16x16_sprite(workspace + 128 * (nb_sprite % 256), sx, sy);
			nb_sprite++;

			/* max 256 sprites */
			if (nb_sprite >= 257) {
				error("Too many sprites in image! The maximum is 256.");
				return;
			}
		}
	}

	/* store a maximum of 256 sprites (32KB) */
	if (nb_sprite > 256) nb_sprite = 256;
	if (nb_sprite)
		putbuffer(workspace, 128 * nb_sprite);

	/* attach the number of loaded sprites to the label */
	if (lablptr) {
		lablptr->data_type = P_INCSPR;
		lablptr->data_size = 128 * nb_sprite;
		lablptr->data_count = nb_sprite;
	}
	else
	if (lastlabl && lastlabl->data_type == P_INCSPR) {
		lastlabl->data_size += 128 * nb_sprite;
		lastlabl->data_count += nb_sprite;
	}

	/* set the size after conversion in the last pass */
	if (lastlabl) {
		if (pass == LAST_PASS)
			lastlabl->size = 128;
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

	/* output */
	if (pass == LAST_PASS)
		loadlc(loccnt, 0);

	/* get args */
	if (!pcx_get_args(ip))
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
			pcx_pack_16x16_tile(workspace + 128 * (nb_tile % 256), tx, ty);
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
	if (lastlabl) {
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
	int i, j, k, l;
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

	/* output */
	if (pass == LAST_PASS)
		loadlc(loccnt, 0);

	/* get args */
	if (!pcx_get_args(ip))
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
	if (!pcx_set_tile(expr_lablptr, value)) {
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
				/* tile coordinates */
				tx = x + (j << 4);
				ty = y + (i << 4);

				data = 0;
				hash = 0;
				index = 0;
				for (cy = 0; cy < 16; cy += 8) {
					for (cx = 0; cx < 16; cx += 8) {
						/* extract palette */
						unsigned char *ppixel = pcx_buf + (x + tx + cx) + pcx_w * (y + ty + cy);
						int palette = -1;
						for (k = 0; k < 8; k++) {
							for (l = 0; l < 8; l++) {
								if ((ppixel[l] & 0x0F) != 0) {
									if (palette < 0)
										palette = ppixel[l] & 0xF0;
									else
									if ((ppixel[l] & 0xF0) != palette)
										fail |= 1;
								}
							}
							ppixel += pcx_w;
						}
						if (palette < 0) palette = 0;

						/* convert 8x8 tile data */
						pcx_pack_8x8_tile(tile_data, x + tx + cx, y + ty + cy);
						tile_number = pcx_search_tile(tile_data, 32);
						if (tile_number == -1) {
							fail |= 2;
							tile_number = 0;
						}
						else
						if ((tile_number + basechr) >= 0x0700) {
							fail |= 4;
							tile_number = 0;
						}
						tile_number += basechr;
						tile_number |= palette << 8;

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
	if (fail & 1)
		error("One or more 8x8 tiles contain pixels in multiple palettes!");
	if (fail & 2)
		error("One or more 8x8 tiles are not in the referenced tileset!");
	if (fail & 4)
		error("One or more 8x8 tiles are located beyond 64KBytes of VRAM!");
	if (fail & 8)
		error("Too many meta-tiles in image! The maximum is 256.");
	if (fail)
		return;

	/* attach the number of blocks to the label */
	if (lastlabl) {
		blk_lablptr = lastlabl;
		lastlabl->uses = tile_lablptr;
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
	if (!pcx_get_args(ip))
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
	else {
		if (lastlabl) {
			if (lastlabl->data_type == P_INCCHRPAL)
				lastlabl->data_size += nb_chr;
		}
	}

	/* attach the number of tile palette bytes to the label */
	if (lastlabl) {
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
	if (!pcx_get_args(ip))
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
	else {
		if (lastlabl) {
			if (lastlabl->data_type == P_INCSPRPAL)
				lastlabl->data_size += nb_sprite;
		}
	}

	/* attach the number of sprite palette bytes to the label */
	if (lastlabl) {
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
	if (!pcx_get_args(ip))
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
	else {
		if (lastlabl) {
			if (lastlabl->data_type == P_INCTILEPAL)
				lastlabl->data_size += nb_tile;
		}
	}

	/* attach the number of tile palette bytes to the label */
	if (lastlabl) {
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
	int i, j, k, l;
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

	/* output */
	if (pass == LAST_PASS)
		loadlc(loccnt, 0);

	/* get args */
	if (!pcx_get_args(ip))
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
		lastlabl->uses = expr_lablptr;
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
				pcx_pack_16x16_tile(tile_data, tx, ty);

				/* search tile */
				tile_number = pcx_search_tile(tile_data, 128);

				/* didn't find the tile */
				if (tile_number == -1) {
					tile_number = 0;
					fail |= 8;
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

		if (!pcx_set_tile(expr_lablptr->uses, expr_lablptr->uses->value))
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
				/* tile coordinates */
				tx = x + (j << 4);
				ty = y + (i << 4);

				data = 0;
				hash = 0;
				index = 0;
				for (cy = 0; cy < 16; cy += 8) {
					for (cx = 0; cx < 16; cx += 8) {
						/* extract palette */
						unsigned char *ppixel = pcx_buf + (x + tx + cx) + pcx_w * (y + ty + cy);
						int palette = -1;
						for (k = 0; k < 8; k++) {
							for (l = 0; l < 8; l++) {
								if ((ppixel[l] & 0x0F) != 0) {
									if (palette < 0)
										palette = ppixel[l] & 0xF0;
									else
									if ((ppixel[l] & 0xF0) != palette)
										fail |= 1;
								}
							}
							ppixel += pcx_w;
						}
						if (palette < 0) palette = 0;

						/* convert 8x8 tile data */
						pcx_pack_8x8_tile(tile_data, x + tx + cx, y + ty + cy);
						tile_number = pcx_search_tile(tile_data, 32);
						if (tile_number == -1) {
							fail |= 2;
							tile_number = 0;
						}
						else
						if ((tile_number + basechr) >= 0x0700) {
							fail |= 4;
							tile_number = 0;
						}
						tile_number += basechr;
						tile_number |= palette << 8;

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
	if (fail & 1)
		error("One or more 8x8 tiles contain pixels in multiple palettes!");
	if (fail & 2)
		error("One or more 8x8 tiles are not in the referenced tileset!");
	if (fail & 4)
		error("One or more 8x8 tiles are located beyond 64KBytes of VRAM!");
	if (fail & 8)
		error("One or more 16x16 tiles are not in the referenced tileset!");
	if (fail)
		return;

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
