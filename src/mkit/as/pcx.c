#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <ctype.h>
#include "defs.h"
#include "externs.h"
#include "protos.h"

/* globals */
char pcx_name[128];			/* pcx file name */
int pcx_w, pcx_h;			/* pcx dimensions */
int pcx_nb_colors;			/* number of colors in the pcx */
int pcx_nb_args;			/* number of argument */
unsigned int pcx_arg[8];		/* pcx args array */
unsigned char *pcx_buf;			/* pointer to the pcx buffer */
unsigned char pcx_pal[256][3];		/* palette */
unsigned char pcx_plane[2048][4];	/* plane buffer */
unsigned int tile_offset;		/* offset in the tile reference table */
struct t_tile tile[256];		/* tile info table */
struct t_tile *tile_tbl[256];		/* tile hash table */
struct t_symbol *tile_lablptr;		/* tile symbol reference */
struct PCX_HEADER {			/* pcx file header */
	unsigned char manufacturer, version;
	unsigned char encoding;
	unsigned char bpp;
	unsigned char xmin[2], ymin[2], xmax[2], ymax[2];
	unsigned char xdpi[2], ydpi[2];
	unsigned char colormap[16][3];
	unsigned char reserved;
	unsigned char np;
	unsigned char bytes_per_line[2];
	unsigned char palette_info[2];
	unsigned char xscreen[2], yscreen[2];
	unsigned char pad[54];
} pcx;

/* externs */
extern struct t_symbol *expr_lablptr;	/* pointer to the lastest label */
extern int expr_lablcnt;		/* number of label seen in an expression */

/* macros */
#define GET_SHORT(a) ((a[1] << 8) + a[0])


/* ----
 * pcx_pack_8x8_tile()
 * ----
 */

int
pcx_pack_8x8_tile(unsigned char *buffer, int x, int y)
{
	unsigned char *ptr;

	/* tile address */
	ptr = pcx_buf + x + (y * pcx_w);

	/* encode the tile */
	return (machine->pack_8x8_tile(buffer, ptr, pcx_w, CHUNKY_TILE));
}


/* ----
 * pcx_pack_16x16_tile()
 * ----
 */

int
pcx_pack_16x16_tile(unsigned char *buffer, int x, int y)
{
	unsigned char *ptr;

	/* tile address */
	ptr = pcx_buf + x + (y * pcx_w);

	/* encode the tile */
	return (machine->pack_16x16_tile(buffer, ptr, pcx_w, CHUNKY_TILE));
}


/* ----
 * pcx_pack_16x16_sprite()
 * ----
 */

int
pcx_pack_16x16_sprite(unsigned char *buffer, int x, int y)
{
	unsigned char *ptr;

	/* sprite address */
	ptr = pcx_buf + x + (y * pcx_w);

	/* encode the sprite */
	return (machine->pack_16x16_sprite(buffer, ptr, pcx_w, CHUNKY_TILE));
}


/* ----
 * pcx_set_tile()
 * ----
 */

int
pcx_set_tile(struct t_symbol *ref, unsigned int offset)
{
	int i;
	int hash;
	int size, start;
	unsigned int crc;
	unsigned char *data;
	int nb;

	/* do nothing in first passes */
	if (pass != LAST_PASS)
		return (1);

	/* same tile set? */
	if (ref == NULL)
		return (1);
	if ((ref == tile_lablptr) && (offset == tile_offset))
		return (1);

	/* check symbol */
	if (ref->nb == 0) {
		if ((ref->type == IFUNDEF) || (ref->type == UNDEF))
			error("Tile table undefined!");
		else
			error("Incorrect tile table reference!");

		/* no tile table */
		tile_lablptr = NULL;
		return (1);
	}
	if (ref->size == 0) {
		error("Tile table has not been compiled yet!");
		tile_lablptr = NULL;
		return (1);
	}

	/* adjust offset */
	start = (offset - ref->value);

	if ((start < 0))
		goto err;
	if ((start % ref->size) != 0)
		goto err;
	if ((start / ref->size) >= ref->nb)
		goto err;

	/* reset tile hash table */
	for (i = 0; i < 256; i++)
		tile_tbl[i] = NULL;

	/* get infos */
	nb = ref->nb - (start / ref->size);
	size = ref->size;
	data = &rom[ref->bank - bank_base][ref->value & 0x1FFF] + start;

	/* 256 tiles max */
	if (nb > 256)
		nb = 256;

	/* parse tiles */
	for (i = 0; i < nb; i++) {
		/* calculate tile crc */
		crc = crc_calc(data, size);
		hash = (crc & 0xFF);

		/* insert the tile in the tile table */
		tile[i].next = tile_tbl[hash];
		tile[i].index = i;
		tile[i].data = data;
		tile[i].crc = crc;
		tile_tbl[hash] = &tile[i];

		/* next */
		data += size;
	}

	/* ok */
	tile_lablptr = ref;
	tile_offset = offset;
	return (1);

	/* error */
err:
	tile_lablptr = NULL;
	error("Incorrect tile table reference!");
	return (1);
}


/* ----
 * pcx_search_tile()
 * ----
 */

int
pcx_search_tile(unsigned char *data, int size)
{
	struct t_tile *tile;
	unsigned int crc;

	/* do nothing in first passes */
	if (pass != LAST_PASS)
		return (0);

	/* quick check */
	if (tile_lablptr == NULL)
		return (-1);
	if (tile_lablptr->size != size)
		return (-1);

	/* calculate tile crc */
	crc = crc_calc(data, size);
	tile = tile_tbl[crc & 0xFF];

	/* search tile */
	while (tile) {
		if (tile->crc == crc) {
			if (memcmp(tile->data, data, size) == 0)
				return (tile->index);
		}
		tile = tile->next;
	}

	/* not found */
	return (-1);
}


/* ----
 * pcx_get_args()
 * ----
 * get arguments in pcx pseudo instructions (.incchr/spr/tile/pal/bat/chrpal/sprpal/tilepal)
 */

int
pcx_get_args(int *ip)
{
	char name[128];
	char c;

	/* get pcx file name */
	if (!getstring(ip, name, 127))
		return (0);

	/* reset args counter */
	pcx_nb_args = 0;

	/* get args */
	for (;;) {
		/* skip spaces */
		while (isspace(c = prlnbuf[(*ip)++])) ;

		/* check syntax */
		if ((c != ',') && (c != ';') && (c != 0)) {
			error("Syntax error!");
			return (0);
		}
		if (c != ',')
			break;

		/* get arg */
		if (!evaluate(ip, 0))
			return (0);

		/* store arg */
		pcx_arg[pcx_nb_args++] = value;

		/* check number of args */
		if (pcx_nb_args == 7)
			break;
	}

	/* check number of args */
	if (optype & (1 << pcx_nb_args)) {
		error("Invalid number of arguments!");
		return (0);
	}

	/* load and unpack the pcx */
	if (!pcx_load(name))
		return (0);

	/* parse tiles */
	if (opval == P_INCMAP) {
		if (expr_lablcnt == 0)
			error("No tile table reference!");
		if (expr_lablcnt > 1) {
			expr_lablcnt = 0;
			error("Too many tile table references!");
		}
		if (!pcx_set_tile(expr_lablptr, value))
			return (0);
	}

	/* ok */
	return (1);
}


/* ----
 * pcx_parse_args()
 * ----
 * parse arguments of pcx pseudo directive
 */

int
pcx_parse_args(int i, int nb, int *a, int *b, int *c, int *d, int size)
{
	int x, y, w, h;

	x = 0;
	y = 0;

	/* get coordinates */
	if (nb == 0) {			/* no arg */
		w = (pcx_w / size);
		h = (pcx_h / size);
	}
	else if (nb == 2) {		/* 2 args */
		w = pcx_arg[i];
		h = pcx_arg[i + 1];
	}
	else {					/* 4 args */
		x = pcx_arg[i];
		y = pcx_arg[i + 1];
		w = pcx_arg[i + 2];
		h = pcx_arg[i + 3];
	}

	/* if w is zero, then calculate from image width */
	if ((w == 0) && (x < pcx_w))
		w = (pcx_w - x) / size;

	/* if h is zero, then calculate from image height */
	if ((h == 0) && (y < pcx_h))
		h = (pcx_h - y) / size;

	/* check */
	if (((x + w * size) > pcx_w) || ((y + h * size) > pcx_h)) {
		error("Coordinates out of range!");
		return (0);
	}

	/* write back the value */
	*a = x;
	*b = y;
	*c = w;
	*d = h;

	/* ok */
	return (1);
}


/* ----
 * pcx_load()
 * ----
 * load a PCX file and unpack it
 */

int
pcx_load(char *name)
{
	FILE *f;
	size_t l;

	/* check if the file is the same as the previously loaded one;
	 * if this is the case do not reload it
	 */
	if ((name[0] != '\0') && (strcasecmp(pcx_name, name) == 0))
		return (1);

	/* do we want to load a png file instead of a pcx file? */
	if (((l = strlen(name)) > 4) && (strcasecmp(".png", (name + l - 4)) == 0))
		return (png_load(name));

	/* do we want to load a bmp file instead of a pcx file? */
	if (((l = strlen(name)) > 4) && (strcasecmp(".bmp", (name + l - 4)) == 0))
		return (bmp_load(name));

	/* no it's a new file - ok let's prepare loading */
	if (pcx_buf)
		free(pcx_buf);
	pcx_buf = NULL;
	pcx_name[0] = '\0';

	/* open the file */
	if ((f = open_file(name, "rb")) == NULL) {
		error("Can not open file!");
		return (0);
	}

	/* get the picture size */
	fread(&pcx, 128, 1, f);
	pcx_w = (GET_SHORT(pcx.xmax) - GET_SHORT(pcx.xmin) + 1);
	pcx_h = (GET_SHORT(pcx.ymax) - GET_SHORT(pcx.ymin) + 1);

	/* adjust picture width */
	if (pcx_w & 0x01)
		pcx_w++;

	/* check size range */
	if ((pcx_w > 16384) || (pcx_h > 4096)) {
		error("Picture size too big, max. 16384x4096!");
		return (0);
	}
	if ((pcx_w < 16) || (pcx_h < 16)) {
		error("Picture size too small, min. 16x16!");
		return (0);
	}

	/* malloc a buffer */
	pcx_buf = malloc(pcx_w * pcx_h);
	if (pcx_buf == NULL) {
		error("Can not load file, not enough memory!");
		return (0);
	}

	/* decode the picture */
	if ((pcx.bpp == 8) && (pcx.np == 1))
		decode_256(f, pcx_w, pcx_h);
	else if ((pcx.bpp == 1) && (pcx.np <= 4))
		decode_16(f, pcx_w, pcx_h);
	else {
		error("Unsupported or invalid PCX format!");
		return (0);
	}

	fclose(f);
	strcpy(pcx_name, name);
	return (1);
}


/* ----
 * decode_256()
 * ----
 * decode a 256 colors PCX file
 */

void
decode_256(FILE *f, int w, int h)
{
	unsigned int i, c, x, y;
	unsigned char *ptr;

	ptr = pcx_buf;
	x = 0;
	y = 0;

	/* decode */
	switch (pcx.encoding) {
	case 0:
		/* raw */
		fread(pcx_buf, w, h, f);
		c = fgetc(f);
		return;

	case 1:
		/* simple run-length encoding */
		do {
			c = fgetc(f);
			if (c == EOF)
				break;
			if ((c & 0xC0) != 0xC0)
				i = 1;
			else {
				i = (c & 0x3F);
				c = fgetc(f);
			}
			do {
				x++;
				*ptr++ = c;
				if (x == w) {
					x = 0;
					y++;
				}
			} while (--i);
		} while (y < h);
		break;

	default:
		error("Unsupported PCX encoding scheme!");
		return;
	}

	/* get the palette */
	if (c != EOF)
		c = fgetc(f);
	while ((c != 12) && (c != EOF))
		c = fgetc(f);
	if (c == 12)
		fread(pcx_pal, 768, 1, f);

	/* number of colors */
	pcx_nb_colors = 256;
}


/* ----
 * decode_16()
 * ----
 * decode a 16 (or less) colors PCX file
 */

void
decode_16(FILE *f, int w, int h)
{
	int i, j, k, n;
	int x, y, p;
	unsigned int c, pix;
	unsigned char *ptr;

	ptr = pcx_buf;
	x = 0;
	y = 0;
	p = 0;

	/* decode */
	switch (pcx.encoding) {
	case 0:
		/* raw */
		error("Unsupported PCX encoding scheme!");
		break;

	case 1:
		/* simple run-length encoding */
		do {
			/* get a char */
			c = fgetc(f);
			if (c == EOF)
				break;

			/* check if it's a repeat command */
			if ((c & 0xC0) != 0xC0)
				i = 1;
			else {
				i = (c & 0x3F);
				c = fgetc(f);
			}

			/* unpack */
			do {
				pcx_plane[x >> 3][p] = c;
				x += 8;

				/* end of line */
				if (x >= w) {
					x = 0;
					p++;

					/* plane to chunky conversion */
					if (p == pcx.np) {
						p = 0;
						n = (w + 7) >> 3;
						y++;

						/* loop */
						for (j = 0; j < n; j++) {
							for (k = 7; k >= 0; k--) {
								/* get pixel index */
								pix = 0;

								switch (pcx.np) {
								case 4:
									pix |= ((pcx_plane[j][3] >> k) & 0x01) << 3;
								case 3:
									pix |= ((pcx_plane[j][2] >> k) & 0x01) << 2;
								case 2:
									pix |= ((pcx_plane[j][1] >> k) & 0x01) << 1;
								case 1:
									pix |= ((pcx_plane[j][0] >> k) & 0x01);
									break;
								}

								/* store pixel */
								if (x < w)
									*ptr++ = pix;

								x++;
							}
						}
						x = 0;
					}
				}
			} while (--i);
		} while (y < h);
		break;

	default:
		error("Unsupported PCX encoding scheme!");
		return;
	}

	/* get the palette */
	memset(pcx_pal, 0, 768);
	memcpy(pcx_pal, pcx.colormap, (1 << pcx.np) * 3);

	/* number of colors */
	pcx_nb_colors = 16;
}

#include <stdint.h>
#pragma pack(1)
typedef struct
{
	uint16_t magic;
	uint32_t fileSize;
	uint32_t reserved0;
	uint32_t bitmapDataOffset;
	uint32_t bitmapHeaderSize;
	uint32_t width;
	uint32_t height;
	uint16_t planes;
	uint16_t bitsPerPixel;
	uint32_t compression;
	uint32_t bitmapDataSize;
	uint32_t hRes;
	uint32_t vRes;
	uint32_t colors;
	uint32_t importantColors;
} BMPHeader_t;

int
bmp_load(char *name)
{
	BMPHeader_t header;
	int i;
	FILE *      pFile      = NULL;
	/* no it's a new file - ok let's prepare loading */
	if (pcx_buf)
		free(pcx_buf);
	pcx_buf = NULL;
	pcx_name[0] = '\0';

	/* open the file */
	if ((pFile = open_file(name, "rb")) == NULL) {
		error("Can not open file!");
		goto errorCleanup;
	}

	fread(&header,1,sizeof(BMPHeader_t),pFile);

	/* adjust picture size to 8-pixel character boundaries */
	pcx_w = (header.width + 7) & ~7;
	pcx_h = (header.height + 7) & ~7;

	/* check size range */
	if ((pcx_w > 16384) || (pcx_h > 4096)) {
		error("Picture size too big, max. 16384x4096!");
		goto errorCleanup;
	}
	if ((pcx_w < 16) || (pcx_h < 16)) {
		error("Picture size too small, min. 16x16!");
		goto errorCleanup;
	}

	/* malloc a buffer */
	pcx_buf = malloc(pcx_w * pcx_h);
	if (pcx_buf == NULL) {
		error("Can not load file, not enough memory!");
		goto errorCleanup;
	}
	for (i=0;i<header.colors;i++)
	{
		fread(&pcx_pal[i][2],1,1,pFile);
		fread(&pcx_pal[i][1],1,1,pFile);
		fread(&pcx_pal[i][0],1,1,pFile);
		fseek(pFile,1,SEEK_CUR);
	}
	for (i = header.colors; i < 256; i += 1) {
		pcx_pal[i][0] = 0;
		pcx_pal[i][1] = 0;
		pcx_pal[i][2] = 0;
	}
	pcx_nb_colors = 256;

	if (header.bitsPerPixel==4)
	{
		for (uint32_t y=0;y<pcx_h;y++)
		{
			for (uint32_t x=0;x<pcx_w;x+=2)
			{
				uint8_t byte;
				fread(&byte,1,1,pFile);
				pcx_buf[x+(((header.height-1-y))*header.width)]=byte>>4;
				pcx_buf[1+x+(((header.height-1-y))*header.width)]=byte&0xf;
			}
		}
	}
	else 
	{
//					uint8_t *src = &buffer[bmp_header->bitmapDataOffset];
		fseek(pFile,header.bitmapDataOffset,SEEK_SET);
		for (uint32_t y=0;y<pcx_h;y++)
		{

			for (uint32_t x=0;x<pcx_w;x++)
			{
				uint8_t byte;
				fread(&byte,1,1,pFile);
				pcx_buf[x+(((header.height-1-y))*header.width)]=byte;
			}
		}
	}

/*
	aImageRows = (png_byte **) malloc(uHeight * sizeof(png_byte *));
	if (aImageRows == NULL) {
		error("Can not load file, not enough memory!");
		goto errorCleanup;
	}
	for (i = 0; i < uHeight; i++)
		aImageRows[i] = (png_byte *) (pcx_buf + pcx_w * i);
	png_read_image(pPngStruct, aImageRows);
	png_read_end(pPngStruct, NULL);
	png_destroy_read_struct(&pPngStruct, &pPngInfo, NULL);
	free(aImageRows);
*/
	fclose(pFile);
	strcpy(pcx_name, name);
	return (1);

	/* error handlers (reached via the dreaded goto) */
	errorCleanup:
	if (pFile)
		fclose(pFile);

	return (0);
}


/* ----
 * png_load()
 * ----
 * load a PNG file and unpack it
 */

#include "png.h"

int
png_load(char *name)
{
	FILE *      pFile      = NULL;
	png_structp pPngStruct = NULL;
	png_infop   pPngInfo   = NULL;
	png_byte ** aImageRows = NULL;
	int         iPngRGB    = 0;
	png_color * pPngRGB    = NULL;
	png_uint_32 uWidth;
	png_uint_32 uHeight;
	int         iBitDepth;
	int         iColorType;
	unsigned    i;

	#if 0
	/* check if the file is the same as the previously loaded one;
	 * if this is the case do not reload it
	 */
	if ((name[0] != '\0') && (strcasecmp(pcx_name, name) == 0))
		return (1);
	#endif

	/* no it's a new file - ok let's prepare loading */
	if (pcx_buf)
		free(pcx_buf);
	pcx_buf = NULL;
	pcx_name[0] = '\0';

	/* open the file */
	if ((pFile = open_file(name, "rb")) == NULL) {
		error("Can not open file!");
		goto errorCleanup;
	}

	/* initialize libpng for reading the file */
	pPngStruct = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!pPngStruct) {
		error("Can not load file, not enough memory!");
		goto errorCleanup;
	}

	pPngInfo = png_create_info_struct(pPngStruct);
	if (!pPngInfo) {
		error("Can not load file, not enough memory!");
		goto errorCleanup;
	}

	if (setjmp(png_jmpbuf(pPngStruct))) {
		error("Unsupported or invalid PNG file!");
		goto errorCleanup;
	}

	png_init_io(pPngStruct, pFile);
	png_read_info(pPngStruct, pPngInfo);

	/* get the picture info */
	png_get_IHDR(pPngStruct, pPngInfo, &uWidth, &uHeight, &iBitDepth, &iColorType, NULL, NULL, NULL);

	if (iColorType != PNG_COLOR_TYPE_PALETTE) {
		error("Unsupported or invalid PNG picture format!");
		goto errorCleanup;
	}

	if (iBitDepth < 8)
		png_set_packing(pPngStruct);

	png_read_update_info(pPngStruct, pPngInfo);

	/* adjust picture size to 8-pixel character boundaries */
	pcx_w = (uWidth + 7) & ~7;
	pcx_h = (uHeight + 7) & ~7;

	/* check size range */
	if ((pcx_w > 16384) || (pcx_h > 4096)) {
		error("Picture size too big, max. 16384x4096!");
		goto errorCleanup;
	}
	if ((pcx_w < 16) || (pcx_h < 16)) {
		error("Picture size too small, min. 16x16!");
		goto errorCleanup;
	}

	/* malloc a buffer */
	pcx_buf = malloc(pcx_w * pcx_h);
	if (pcx_buf == NULL) {
		error("Can not load file, not enough memory!");
		goto errorCleanup;
	}

	/* get the palette */
	if (png_get_valid(pPngStruct, pPngInfo, PNG_INFO_PLTE)) {
		png_get_PLTE(pPngStruct, pPngInfo, &pPngRGB, &iPngRGB);

		for (i = 0; i < (unsigned) iPngRGB; i += 1) {
			pcx_pal[i][0] = pPngRGB[i].red;
			pcx_pal[i][1] = pPngRGB[i].green;
			pcx_pal[i][2] = pPngRGB[i].blue;
		}

		for (i = iPngRGB; i < 256; i += 1) {
			pcx_pal[i][0] = 0;
			pcx_pal[i][1] = 0;
			pcx_pal[i][2] = 0;
		}

		pcx_nb_colors = (iBitDepth <= 4) ? 16 : 256;
	}

	/* create a list of each row's starting point in memory */
	aImageRows = (png_byte **) malloc(uHeight * sizeof(png_byte *));
	if (aImageRows == NULL) {
		error("Can not load file, not enough memory!");
		goto errorCleanup;
	}

	for (i = 0; i < uHeight; i++)
		aImageRows[i] = (png_byte *) (pcx_buf + pcx_w * i);

	/* decode the picture */
	png_read_image(pPngStruct, aImageRows);
	png_read_end(pPngStruct, NULL);
	png_destroy_read_struct(&pPngStruct, &pPngInfo, NULL);

	free(aImageRows);

	fclose(pFile);
	strcpy(pcx_name, name);
	return (1);

	/* error handlers (reached via the dreaded goto) */
	errorCleanup:

	if (pPngInfo)
		png_destroy_read_struct(&pPngStruct, &pPngInfo, NULL);
	else if (pPngStruct)
		png_destroy_read_struct(&pPngStruct, NULL, NULL);

	if (aImageRows)
		free(aImageRows);

	if (pFile)
		fclose(pFile);

	return (0);
}
