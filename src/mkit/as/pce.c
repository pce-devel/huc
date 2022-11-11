#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "defs.h"
#include "externs.h"
#include "protos.h"
#include "pce.h"

/* locals */
static unsigned char buffer[128 * 256];	/* buffer for .inc and .def directives */
static unsigned char header[512];	/* rom header */


/* ----
 * write_header()
 * ----
 * generate and write rom header
 */

void
pce_write_header(FILE *f, int banks)
{
	/* setup header */
	memset(header, 0, 512);
	header[0] = banks;

	/* write */
	fwrite(header, 512, 1, f);
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

	/* pack the tile only in the last pass */
	if (pass != LAST_PASS)
		return (0);

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
		error("Internal error: unsupported format passed to 'pack_8x8_tile'!");
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
		error("Internal error: unsupported format passed to 'pack_16x16_tile'!");
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
		error("Internal error: unsupported format passed to 'pack_16x16_sprite'!");
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
	labldef(0, 0, LOCATION);

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
	labldef(0, 0, LOCATION);

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
	lastlabl->pal = value;

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
		labldef(0, 0, LOCATION);

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
		lablptr->pal = value;
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
	pce_pack_8x8_tile(buffer, data, 0, PACKED_TILE);

	/* store tile */
	putbuffer(buffer, 32);

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
	labldef(0, 0, LOCATION);

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
			putword(loccnt, color);
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
		labldef(0, 0, LOCATION);

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
		lablptr->pal = value;
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
	pce_pack_16x16_sprite(buffer, data, 0, PACKED_TILE);

	/* store sprite */
	putbuffer(buffer, 128);

	/* output line */
	if (pass == LAST_PASS)
		println();
}


/* ----
 * do_incbat()
 * ----
 * build a BAT
 */

void
pce_incbat(int *ip)
{
	unsigned char *ptr, ref;
	int i, j, k, l, x, y, w, h;
	unsigned int base, index, flag;
	unsigned int temp;

	/* define label */
	labldef(0, 0, LOCATION);

	/* output */
	if (pass == LAST_PASS)
		loadlc(loccnt, 0);

	/* get args */
	if (!pcx_get_args(ip))
		return;
	if (!pcx_parse_args(1, pcx_nb_args - 1, &x, &y, &w, &h, 8))
		return;

	/* build the BAT */
	if (pass == LAST_PASS) {
		index = 0;
		flag = 0;
		base = (pcx_arg[0] >> 4);

		for (i = 0; i < h; i++) {
			for (j = 0; j < w; j++) {
				ptr = pcx_buf + (x + (j * 8)) + ((y + (i * 8)) * pcx_w);
				ref = ptr[0] & 0xF0;

				/* check colors */
				for (k = 0; k < 8; k++) {
					for (l = 0; l < 8; l++) {
						if ((ptr[l] & 0xF0) != ref)
							flag = 1;
					}
					ptr += pcx_w;
				}
				temp = (base & 0xFFF) | ((ref & 0xF0) << 8);
				buffer[2 * index] = temp & 0xff;
				buffer[2 * index + 1] = temp >> 8;

				index++;
				base++;
			}
		}

		/* errors */
		if (flag)
			error("Invalid color index found!");
	}

	/* store data */
	putbuffer(buffer, 2 * w * h);

	/* output */
	if (pass == LAST_PASS)
		println();
}


/* ----
 * do_incpal()
 * ----
 * extract the palette of a PCX file
 */

void
pce_incpal(int *ip)
{
	unsigned int temp;
	int i, start, nb;
	int r, g, b;

	/* define label */
	labldef(0, 0, LOCATION);

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
			buffer[2 * i] = temp & 0xff;
			buffer[2 * i + 1] = temp >> 8;
		}
	}

	/* store data */
	putbuffer(buffer, nb << 1);

	/* output */
	if (pass == LAST_PASS)
		println();
}


/* ----
 * do_incspr()
 * ----
 * PCX to sprites
 */

void
pce_incspr(int *ip)
{
	unsigned int i, j;
	int x, y, w, h;
	unsigned int sx, sy;
	int nb_sprite = 0;

	/* define label */
	labldef(0, 0, LOCATION);

	/* output */
	if (pass == LAST_PASS)
		loadlc(loccnt, 0);

	/* get args */
	if (!pcx_get_args(ip))
		return;
	if (!pcx_parse_args(0, pcx_nb_args, &x, &y, &w, &h, 16))
		return;

	/* pack sprites */
	for (i = 0; i < h; i++) {
		for (j = 0; j < w; j++) {
			/* sprite coordinates */
			sx = x + (j << 4);
			sy = y + (i << 4);

			/* encode sprite */
			pcx_pack_16x16_sprite(buffer + 128 * (nb_sprite % 256), sx, sy);
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
		putbuffer(buffer, 128 * nb_sprite);

	/* size */
	if (lablptr) {
		lablptr->data_type = P_INCSPR;
		lablptr->data_size = 128 * nb_sprite;
	}
	else {
		if (lastlabl) {
			if (lastlabl->data_type == P_INCSPR)
				lastlabl->data_size += 128 * nb_sprite;
		}
	}

	/* attach the number of loaded sprites to the label */
	if (lastlabl) {
		if (nb_sprite) {
			lastlabl->nb = nb_sprite;
			if (pass == LAST_PASS)
				lastlabl->size = 128;
		}
	}

	/* output */
	if (pass == LAST_PASS)
		println();
}


/* ----
 * do_inctile()
 * ----
 * PCX to 16x16 tiles (max 256 tiles per inctile, wraps around if 257)
 */

void
pce_inctile(int *ip)
{
	unsigned int i, j;
	int x, y, w, h;
	unsigned int tx, ty;
	int nb_tile = 0;

	/* define label */
	labldef(0, 0, LOCATION);

	/* output */
	if (pass == LAST_PASS)
		loadlc(loccnt, 0);

	/* get args */
	if (!pcx_get_args(ip))
		return;
	if (!pcx_parse_args(0, pcx_nb_args, &x, &y, &w, &h, 16))
		return;

	/* pack tiles */
	for (i = 0; i < h; i++) {
		for (j = 0; j < w; j++) {
			/* tile coordinates */
			tx = x + (j << 4);
			ty = y + (i << 4);

			/* encode tile */
			pcx_pack_16x16_tile(buffer + 128 * (nb_tile % 256), tx, ty);
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
		putbuffer(buffer, 128 * nb_tile);

	/* size */
	if (lablptr) {
		lablptr->data_type = P_INCTILE;
		lablptr->data_size = 128 * nb_tile;
	}
	else {
		if (lastlabl) {
			if (lastlabl->data_type == P_INCTILE)
				lastlabl->data_size += 128 * nb_tile;
		}
	}

	/* attach the number of loaded tiles to the label */
	if (lastlabl) {
		if (nb_tile) {
			lastlabl->nb = nb_tile;
			if (pass == LAST_PASS)
				lastlabl->size = 128;
		}
	}

	/* output */
	if (pass == LAST_PASS)
		println();
}


/* ----
 * do_incchrpal()
 * ----
 * PCX to palette array for 8x8 tiles
 */

void
pce_incchrpal(int *ip)
{
	unsigned int i, j;
	int x, y, w, h;
	unsigned int tx, ty;
	int nb_chr = 0;

	/* define label */
	labldef(0, 0, LOCATION);

	/* output */
	if (pass == LAST_PASS)
		loadlc(loccnt, 0);

	/* get args */
	if (!pcx_get_args(ip))
		return;
	if (!pcx_parse_args(0, pcx_nb_args, &x, &y, &w, &h, 16))
		return;

	/* scan tiles */
	for (i = 0; i < h; i++) {
		for (j = 0; j < w; j++) {
			/* tile coordinates */
			tx = x + (j << 3);
			ty = y + (i << 3);

			/* get chr palette */
			buffer[0] = pce_scan_8x8_tile(tx, ty) << 4;

			/* store palette number */
			putbuffer(buffer, 1);
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
			lastlabl->nb = nb_chr;
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
 */

void
pce_incsprpal(int *ip)
{
	unsigned int i, j;
	int x, y, w, h;
	unsigned int tx, ty;
	int nb_sprite = 0;

	/* define label */
	labldef(0, 0, LOCATION);

	/* output */
	if (pass == LAST_PASS)
		loadlc(loccnt, 0);

	/* get args */
	if (!pcx_get_args(ip))
		return;
	if (!pcx_parse_args(0, pcx_nb_args, &x, &y, &w, &h, 16))
		return;

	/* scan sprites */
	for (i = 0; i < h; i++) {
		for (j = 0; j < w; j++) {
			/* sprite coordinates */
			tx = x + (j << 4);
			ty = y + (i << 4);

			/* get sprite palette */
			buffer[0] = pce_scan_16x16_tile(tx, ty);

			/* store palette number */
			putbuffer(buffer, 1);
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
			lastlabl->nb = nb_sprite;
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
 */

void
pce_inctilepal(int *ip)
{
	unsigned int i, j;
	int x, y, w, h;
	unsigned int tx, ty;
	int nb_tile = 0;

	/* define label */
	labldef(0, 0, LOCATION);

	/* output */
	if (pass == LAST_PASS)
		loadlc(loccnt, 0);

	/* get args */
	if (!pcx_get_args(ip))
		return;
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
			buffer[(nb_tile % 256)] = pce_scan_16x16_tile(tx, ty) << 4;
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
		putbuffer(buffer, nb_tile);

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
			lastlabl->nb = nb_tile;
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
 * .incmap pseudo - convert a tiled PCX into a map
 */

void
pce_incmap(int *ip)
{
	unsigned int i, j;
	int x, y, w, h;
	unsigned int tx, ty;
	int tile;
	int err = 0;

	/* define label */
	labldef(0, 0, LOCATION);

	/* output */
	if (pass == LAST_PASS)
		loadlc(loccnt, 0);

	/* get args */
	if (!pcx_get_args(ip))
		return;
	if (!pcx_parse_args(0, pcx_nb_args - 1, &x, &y, &w, &h, 16))
		return;

	/* pack map */
	for (i = 0; i < h; i++) {
		for (j = 0; j < w; j++) {
			/* tile coordinates */
			tx = x + (j << 4);
			ty = y + (i << 4);

			/* get tile */
			pcx_pack_16x16_tile(buffer, tx, ty);

			/* search tile */
			tile = pcx_search_tile(buffer, 128);

			if (tile == -1) {
				/* didn't find the tile */
				tile = 0;
				err++;
			}

			/* store tile index */
			if (pass == LAST_PASS)
				putbyte(loccnt, tile & 0xFF);

			/* update location counter */
			loccnt++;
		}
	}

	/* error */
	if (err)
		error("One or more tiles didn't match!");

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
	char mml[128];
	char c;

	/* define label */
	labldef(0, 0, LOCATION);

	/* output */
	if (pass == LAST_PASS)
		loadlc(loccnt, 0);

	/* start */
	bufsize = 8192;
	offset = mml_start(buffer);
	bufsize -= offset;

	/* extract and parse mml string(s) */
	for (;;) {
		/* get string */
		if (!getstring(ip, mml, 127))
			return;

		/* parse string */
		size = mml_parse(buffer + offset, bufsize, mml);

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
	offset += mml_stop(buffer + offset);

	/* store data */
	putbuffer(buffer, offset);

	/* output */
	if (pass == LAST_PASS)
		println();
}

