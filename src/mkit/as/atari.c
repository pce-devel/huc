#include <stdio.h>
#include <string.h>
#include "defs.h"
#include "externs.h"
#include "protos.h"
#include "atari.h"

/* locals */
static int cart_type = 1;	/* cartridge type */
static struct ICART {		/* ICAR Atari cartridge header */
	unsigned char id[4];
	unsigned char type[4];
	unsigned char csum[4];
	unsigned char zero[4];
} fuji_header;


/* ----
 * fuji_cartsize
 * ----
 * Cartridge size as # of 8KB banks, maximum 1MB size.
 *
 * Cartridges larger than 1MB use zero to mark them as unsupported by fujias.
 *
 * See "https://github.com/atari800/atari800/blob/master/DOC/cart.txt"
 *
 * Final entry here is cart type 70 ... "aDawliah 64 KB cartridge"
 */

const unsigned char fuji_cartsize [] = {
	0x00, 0x01, 0x02, 0x02, 0x04, 0x04, 0x02, 0x05,
	0x08, 0x08, 0x08, 0x08, 0x04, 0x08, 0x10, 0x02,
	0x02, 0x10, 0x05, 0x01, 0x00, 0x01, 0x04, 0x20,
	0x40, 0x80, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40,
	0x80, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x01,
	0x02, 0x10, 0x80, 0x10, 0x01, 0x02, 0x00, 0x04,
	0x08, 0x10, 0x08, 0x10, 0x04, 0x01, 0x10, 0x20,
	0x40, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x08, 0x10, 0x04, 0x08
};	


/* ----
 * fuji_write_header()
 * ----
 * generate and write cartridge header
 */

void
fuji_write_header(FILE *f, int banks)
{
	int num_banks, bank, offset, checksum;

	/* check the number of banks */
	if ((cart_type < 1) || (cart_type >= sizeof(fuji_cartsize))) {
		error("Cartridge type out of range!");
		return;
	}

	num_banks = fuji_cartsize[cart_type];

	if (num_banks == 0) {
		error("Cartridge > 1MByte unsupported!");
		return;
	}
	if (num_banks != (max_bank + 1)) {
		error("Too many banks for cartridge!");
		return;
	}

	/* calculate cartridge checksum */
	checksum = 0;
	for (bank = 0; bank <= max_bank; ++bank) {
		for (offset = 0; offset < 0x2000; ++offset) {
			checksum += rom[bank][offset];
		}
	}

	/* setup CAR header */
	memset(&fuji_header, 0, sizeof(fuji_header));
	fuji_header.id[0] = 'C';
	fuji_header.id[1] = 'A';
	fuji_header.id[2] = 'R';
	fuji_header.id[3] = 'T';
	fuji_header.type[3] = cart_type;
	fuji_header.csum[0] = (checksum >> 24) & 255;
	fuji_header.csum[1] = (checksum >> 16) & 255;
	fuji_header.csum[2] = (checksum >>  8) & 255;
	fuji_header.csum[3] = (checksum >>  0) & 255;

	/* write */
	fwrite(&fuji_header, sizeof(fuji_header), 1, f);
}


/* ----
 * fuji_carttype()
 * ----
 * .cartridge pseudo
 */

void
fuji_carttype(int *ip)
{
	int num_banks;

	/* define label */
//	labldef(0, 0, LOCATION);

	/* get cartridge type value */
	if (!evaluate(ip, ';', 0))
//	if (!evaluate(ip, 0, 0))
		return;
	if ((value < 1) || (value >= sizeof(fuji_cartsize))) {
		error("Cartridge type out of range!");
		return;
	}

	num_banks = fuji_cartsize[value];

	if (num_banks == 0) {
		error("Cartridge > 1MByte unsupported!");
		return;
	}

	/* get new bank infos */
	cart_type = value;

	/* update the max bank counter */
	if (max_bank < (num_banks - 1))
		max_bank = num_banks - 1;

	/* output line */
	if (pass == LAST_PASS) {
		loadlc(value, 1);
		println();
	}
}


/* ----
 * pack_8x8_tile()
 * ----
 * encode a 8x8 tile for the Atari
 */

int
fuji_pack_8x8_tile(unsigned char *buffer, void *data, int line_offset, int format)
{
	int i, j;
	int cnt, err;
	unsigned int pixel;
	unsigned char *ptr;
	unsigned int *packed;

	/* pack the tile only in the last pass */
	if (pass != LAST_PASS)
		return (0);

	/* clear buffer */
	memset(buffer, 0, 16);

	/* encode the tile */
	switch (format) {
	case CHUNKY_TILE:
		/* 8-bit chunky format */
		cnt = 0;
		ptr = data;

		for (i = 0; i < 8; i++) {
			for (j = 0; j < 8; j++) {
				pixel = ptr[j ^ 0x07];
				buffer[cnt] |= (pixel & 0x01) ? (1 << j) : 0;
				buffer[cnt + 8] |= (pixel & 0x02) ? (1 << j) : 0;
			}
			ptr += line_offset;
			cnt += 1;
		}
		break;

	case PACKED_TILE:
		/* 4-bit packed format */
		cnt = 0;
		err = 0;
		packed = data;

		for (i = 0; i < 8; i++) {
			pixel = packed[i];

			for (j = 0; j < 8; j++) {
				/* check for errors */
				if (pixel & 0x0C)
					err++;

				/* convert the tile */
				buffer[cnt] |= (pixel & 0x01) ? (1 << j) : 0;
				buffer[cnt + 8] |= (pixel & 0x02) ? (1 << j) : 0;
				pixel >>= 4;
			}
			cnt += 1;
		}

		/* error message */
		if (err)
			error("Incorrect pixel color index!");
		break;

	default:
		/* other formats not supported */
		error("Internal error: unsupported format passed to 'pack_8x8_tile'!");
		break;
	}

	/* ok */
	return (0);
}

