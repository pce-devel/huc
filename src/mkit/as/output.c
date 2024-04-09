#include <stdio.h>
#include <string.h>
#include "defs.h"
#include "externs.h"
#include "protos.h"


/* ----
 * println()
 * ----
 * prints the contents of prlnbuf
 */

void
println(void)
{
	int nb, cnt;
	int i;

	/* check if output possible */
	if (list_level == 0)
		return;
	if (!xlist || !asm_opt[OPT_LIST] || (expand_macro && !asm_opt[OPT_MACRO]))
		return;
	if (cloaking_stripped)
		return;

	/* undo the pre-processor's modification to the line */
	if (preproc_modidx != 0) {
		prlnbuf[preproc_modidx] = '/';
	}

	/* only output the 1st line that was continued */
	if (continued_line)
		strcpy(prlnbuf, tmplnbuf);

	/* output */
	if (data_loccnt == -1)
		/* line buffer */
		fprintf(lst_fp, "%s\n", prlnbuf);
	else {
		/* line buffer + data bytes */
		loadlc(data_loccnt, 0);

		/* number of bytes */
		nb = loccnt - data_loccnt;

		/* check level */
		if ((data_level > list_level) && (nb > 4)) {
			/* doesn't match */
			fputs(prlnbuf, lst_fp); putc('\n', lst_fp);
		}
		else {
			/* ok */
			cnt = 0;
			for (i = 0; i < nb; i++) {
//				if (bank == UNDEFINED_BANK || bank == STRIPPED_BANK) {
				if (bank > bank_limit) {
					prlnbuf[18 + (3 * cnt)] = '-';
					prlnbuf[19 + (3 * cnt)] = '-';
				}
				else {
					hexcon(2, rom[bank][data_loccnt]);
					prlnbuf[18 + (3 * cnt)] = hex[1];
					prlnbuf[19 + (3 * cnt)] = hex[2];
				}
				data_loccnt++;
				cnt++;
				if (cnt == data_size) {
					cnt = 0;
					fputs(prlnbuf, lst_fp); putc('\n', lst_fp);
					clearln();
					loadlc(data_loccnt, 0);
				}
			}
			if (cnt) {
				fputs(prlnbuf, lst_fp); putc('\n', lst_fp);
			}
		}
	}

	/* redo the pre-processor's modification to the line */
	if (preproc_modidx != 0) {
		prlnbuf[preproc_modidx] = ';';
	}
}


/* ----
 * clearln()
 * ----
 * clear prlnbuf
 */

void
clearln(void)
{
	int i;

	for (i = 0; i < SFIELD; i++)
		prlnbuf[i] = ' ';
	prlnbuf[i] = 0;
}


/* ----
 * loadlc()
 * ----
 * load 16 bit value in printable form into prlnbuf
 */

void
loadlc(int offset, int pos)
{
	int i;

	if (pos)
		i = 20;
	else
		i = 7;

	if (pos == 0) {
		if (bank == UNDEFINED_BANK || bank == STRIPPED_BANK) {
			prlnbuf[i++] = ' ';
			prlnbuf[i++] = ' ';
			prlnbuf[i++] = '-';
			prlnbuf[i++] = '-';
		}
		else {
			if ((section_flags[section] & S_IS_SF2) && (bank > 127)) {
				hexcon(1, (bank / 64) - 1);
				prlnbuf[i++] = hex[1];
				prlnbuf[i++] = ':';
				hexcon(2, (bank & 63) + 64);
				prlnbuf[i++] = hex[1];
				prlnbuf[i++] = hex[2];

			} else {
				prlnbuf[i++] = ' ';
				prlnbuf[i++] = ' ';
				hexcon(2, bank);
				prlnbuf[i++] = hex[1];
				prlnbuf[i++] = hex[2];
			}
		}
		prlnbuf[i++] = ':';
		offset += page << 13;
	}
	hexcon(4, offset);
	prlnbuf[i++] = hex[1];
	prlnbuf[i++] = hex[2];
	prlnbuf[i++] = hex[3];
	prlnbuf[i] = hex[4];
}


/* ----
 * hexcon()
 * ----
 * convert number supplied as argument to hexadecimal in hex[digit]
 */

void
hexcon(int digit, int num)
{
	for (; digit > 0; digit--) {
		hex[digit] = (num & 0x0f) + '0';
		if (hex[digit] > '9')
			hex[digit] += 'A' - '9' - 1;
		num >>= 4;
	}
}


/* ----
 * putbyte()
 * ----
 * store a byte in the rom
 */

void
putbyte(int offset, int data)
{
	int addr;

	if (((section_flags[section] & S_IS_ROM) == 0) || (bank > bank_limit))
		return;

	addr = offset + 1 + (bank << 13);

	if (addr > rom_limit) {
		fatal_error("ROM overflow!");
		return;
	}

	if (((addr - 1) >> 13) > max_bank) {
		if (pass == LAST_PASS) {
			fatal_error("Cannot change ROM size in LAST_PASS!");
			return;
		}
		/* N.B. putbyte() is ONLY called in LAST_PASS, so this is redundant! */
		max_bank = ((addr - 1) >> 13);
	}

	rom[bank][offset] = 0xFF & (data);

	if (section == S_DATA && asm_opt[OPT_DATAPAGE] != 0)
		addr = (page << 13);
	else
		addr = (page << 13) + offset;

	map[bank][offset] = section + ((addr >> 8) & 0xE0);
}


/* ----
 * putword()
 * ----
 * store a word in the rom
 */

void
putword(int offset, int data)
{
	int addr;

	if (((section_flags[section] & S_IS_ROM) == 0) || (bank > bank_limit))
		return;

	addr = offset + 2 + (bank << 13);

	if (addr > rom_limit) {
		fatal_error("ROM overflow!");
		return;
	}

	if (((addr - 1) >> 13) > max_bank) {
		if (pass == LAST_PASS) {
			fatal_error("Cannot change ROM size in LAST_PASS!");
			return;
		}
		/* N.B. putword() is ONLY called in LAST_PASS, so this is redundant! */
		max_bank = ((addr - 1) >> 13);
	}

	rom[bank][offset + 0] = 0xFF & (data);
	rom[bank][offset + 1] = 0xFF & (data >> 8);

	if (section == S_DATA && asm_opt[OPT_DATAPAGE] != 0)
		addr = (page << 13);
	else
		addr = (page << 13) + offset;

	map[bank][offset + 0] = section + ((addr++ >> 8) & 0xE0);
	map[bank][offset + 1] = section + ((addr++ >> 8) & 0xE0);
}


/* ----
 * putdword()
 * ----
 * store a double word in the rom
 */

void
putdword(int offset, int data)
{
	int addr;

	if (((section_flags[section] & S_IS_ROM) == 0) || (bank > bank_limit))
		return;

	addr = offset + 4 + (bank << 13);

	if (addr > rom_limit) {
		fatal_error("ROM overflow!");
		return;
	}

	if (((addr - 1) >> 13) > max_bank) {
		if (pass == LAST_PASS) {
			fatal_error("Cannot change ROM size in LAST_PASS!");
			return;
		}
		/* N.B. putdword() is ONLY called in LAST_PASS, so this is redundant! */
		max_bank = ((addr - 1) >> 13);
	}

	rom[bank][offset + 0] = 0xFF & (data);
	rom[bank][offset + 1] = 0xFF & (data >> 8);
	rom[bank][offset + 2] = 0xFF & (data >> 16);
	rom[bank][offset + 3] = 0xFF & (data >> 24);

	if (section == S_DATA && asm_opt[OPT_DATAPAGE] != 0)
		addr = (page << 13);
	else
		addr = (page << 13) + offset;

	map[bank][offset + 0] = section + ((addr++ >> 8) & 0xE0);
	map[bank][offset + 1] = section + ((addr++ >> 8) & 0xE0);
	map[bank][offset + 2] = section + ((addr++ >> 8) & 0xE0);
	map[bank][offset + 3] = section + ((addr++ >> 8) & 0xE0);
}


/* ----
 * putbuffer()
 * ----
 * copy a buffer at the current location
 */

void
putbuffer(void *data, int size)
{
	int addr;
	int step;

	/* check size */
	if (size == 0)
		return;

	/* check if the buffer will fit in the rom */
	if ((section_flags[section] & S_IS_ROM) && (bank <= bank_limit)) {
		addr = (bank << 13) + loccnt;

		if ((addr + size) > rom_limit) {
			fatal_error("ROM overflow!");
			return;
		}

		/* copy the buffer */
		if (pass == LAST_PASS) {
			if (data)
				memcpy(&rom[bank][loccnt], data, size);
			else
				memset(&rom[bank][loccnt], 0, size);

			if (section == S_DATA && asm_opt[OPT_DATAPAGE] != 0)
				memset(&map[bank][loccnt], section + (page << 5), size);
			else {
				int temp = (page + (loccnt >> 13)) & 7;
				int left = size;

				while (left != 0) {
					step = 0x2000 - (addr & 0x1FFF);
					if (step > left) { step = left; }
					memset(&map[0][0] + addr, section + (temp << 5), step);
					temp = (temp + 1) & 7;
					addr += step;
					left -= step;
				}
			}
		}
	} else {
		if ((loccnt + size) > section_limit[section]) {
			fatal_error("Too large to fit in the current section!");
			return;
		}
	}

	/* update the location counter */
	step = (loccnt + size) >> 13;
	bank = (bank + step);
	if (section != S_DATA || asm_opt[OPT_DATAPAGE] == 0)
		page = (page + step) & 7;

	loccnt = (loccnt + size) & 0x1FFF;

	if (loccnt == 0 && step != 0) {
		loccnt = 0x2000;
		bank = (bank - 1);
		if (section != S_DATA || asm_opt[OPT_DATAPAGE] == 0)
			page = (page - 1) & 7;
	}

	/* update rom size */
	if ((section_flags[section] & S_IS_ROM) && (bank < UNDEFINED_BANK)) {
		if (bank > max_bank) {
			if (loccnt)
				max_bank = bank;
			else
				max_bank = bank - 1;
		}
	}
}


/* ----
 * write_srec()
 * ----
 */

void
write_srec(char *file, char *ext, int base)
{
	unsigned char data, chksum;
	char fname[128];
	int addr, dump, cnt, pos, i, j;
	FILE *fp;

	/* status message */
	if (!strcmp(ext, "mx"))
		printf("writing mx file... ");
	else
		printf("writing s-record file... ");

	/* flush output */
	fflush(stdout);

	/* add the file extension */
	strcpy(fname, file);
	strcat(fname, ".");
	strcat(fname, ext);

	/* open the file */
	if ((fp = fopen(fname, "w")) == NULL) {
		printf("can not open file '%s'!\n", fname);
		return;
	}

	/* dump the rom */
	dump = 0;
	cnt = 0;
	pos = 0;

	for (i = 0; i <= max_bank; i++) {
		for (j = 0; j < 8192; j++) {
			if (map[i][j] != 0xFF) {
				/* data byte */
				if (cnt == 0)
					pos = j;
				cnt++;
				if (cnt == 32)
					dump = 1;
			}
			else {
				/* free byte */
				if (cnt)
					dump = 1;
			}
			if (j == 8191)
				if (cnt)
					dump = 1;

			/* dump */
			if (dump) {
				dump = 0;
				addr = base + (i << 13) + pos;
				chksum = cnt + ((addr >> 16) & 0xFF) +
					 ((addr >> 8) & 0xFF) +
					 ((addr) & 0xFF) +
					 4;

				/* number, address */
				fprintf(fp, "S2%02X%06X", cnt + 4, addr);

				/* code */
				while (cnt) {
					data = rom[i][pos++];
					chksum += data;
					fprintf(fp, "%02X", data);
					cnt--;
				}

				/* chksum */
				fprintf(fp, "%02X\n", (~chksum) & 0xFF);
			}
		}
	}

	/* starting address */
	addr = ((map[0][0] >> 5) << 13);
	chksum = ((addr >> 8) & 0xFF) + (addr & 0xFF) + 4;
	fprintf(fp, "S804%06X%02X", addr, (~chksum) & 0xFF);

	/* ok */
	fclose(fp);
	printf("OK\n");
}


/* ----
 * fatal_error()
 * ----
 * stop compilation
 */

void
fatal_error(char *stptr)
{
	error(stptr);
	stop_pass = 1;
}


/* ----
 * error()
 * ----
 * error printing routine
 */

void
error(char *stptr)
{
	warning(stptr);
	errcnt++;
}


/* ----
 * warning()
 * ----
 * warning printing routine
 */

void
warning(char *stptr)
{
	int i, temp;

	/* put the source line number into prlnbuf */
	i = 4;
	temp = slnum;
	while (temp != 0) {
		prlnbuf[i--] = temp % 10 + '0';
		temp /= 10;
	}

	/* update the current file name */
	if (infile_error != infile_num) {
		infile_error = infile_num;
		printf("#[%i]   %s\n", infile_num, input_file[infile_num].file->name);
	}

	/* undo the pre-processor's modification to the line */
	if (preproc_modidx != 0) {
		prlnbuf[preproc_modidx] = '/';
	}

	/* output the line and the error message */
	loadlc(loccnt, 0);
	printf("%s\n", prlnbuf);
	printf("       %s\n", stptr);

	/* redo the pre-processor's modification to the line */
	if (preproc_modidx != 0) {
		prlnbuf[preproc_modidx] = ';';
	}
}
