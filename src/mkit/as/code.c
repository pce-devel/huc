
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "defs.h"
#include "externs.h"
#include "protos.h"

unsigned char auto_inc;
unsigned char auto_tag;
unsigned int auto_tag_value;

static struct t_branch * getbranch(int opcode_length);


/* ----
 * classC()
 * ----
 * choose "call" or "jsr/jmp" processing for a "jsr/jmp" instruction
 */

void
classC(int *ip)
{
	/* Both ".kickc" and "-newproc" need to use the call trampolines */
	if (newproc_opt || kickc_mode)
	{
		/* skip spaces */
		while (isspace(prlnbuf[*ip]))
			(*ip)++;

		/* if the operand looks like a global label */
		if (isalpha(prlnbuf[*ip]) || (prlnbuf[*ip] == '_')) {
			if (opval == 0x40) { optype = 1; }
			do_call(ip);
			return;
		}
	}

	/* default to traditional "jsr" and "jmp" behavior */
	class4(ip);
}


/* ----
 * classR()
 * ----
 * choose "rts" or "leave" processing for an "rts" instruction
 */

void
classR(int *ip)
{
	/* Check for someone exiting a C function from within a #asm section. */
	if (hucc_mode && proc_ptr)
	{
		error("You cannot exit a C function with an RTS in HuCC!\nPlease read the documentation!");
		return;
	}

	/* Only if ".kickc" and currently inside a C function/procedure */
	if (kickc_mode && scopeptr && proc_ptr)
	{
		/* Replace the "rts" instruction with a "jmp leave_proc" */
		do_leave(ip);
		return;
	}

	/* default to traditional "rts" instruction */
	class1(ip);
}


/* ----
 * class1()
 * ----
 * 1 byte, no operand field
 */

void
class1(int *ip)
{
	check_eol(ip);

	/* update location counter */
	loccnt++;

	/* generate code */
	if (pass == LAST_PASS) {
		/* opcode */
		putbyte(data_loccnt, opval);

		/* output line */
		println();
	}
}


/* ----
 * class2()
 * ----
 * 2 bytes, relative addressing
 */

void
class2(int *ip)
{
	struct t_branch * branch;
	unsigned int addr;

	/* update location counter */
	loccnt += 2;

	/* get destination address */
	if (!evaluate(ip, ';', 0))
		return;

	/* all branches tracked for long-branch handling */
	branch = getbranch(2);

	/* need more space for a long-branch */
	if ((branch) && (branch->convert)) {
		if ((opval & 0x1F) == 0x10)
			/* conditional branch */
			loccnt += 3;
		else
			/* bra and bsr */
			loccnt += 1;
	}

	/* generate code */
	if (pass == LAST_PASS) {
		if ((branch) && (branch->convert)) {
			/* long-branch opcode */
			if ((opval & 0x1F) == 0x10) {
				/* conditional-branch */
				putbyte(data_loccnt + 0, opval ^ 0x20);
				putbyte(data_loccnt + 1, 0x03);

				putbyte(data_loccnt + 2, 0x4C);
				putword(data_loccnt + 3, value & 0xFFFF);
			} else {
				/* bra and bsr */
				putbyte(data_loccnt + 0, (opval == 0x80) ? 0x4C : 0x20);
				putword(data_loccnt + 1, value & 0xFFFF);
			}
		} else {
			/* short-branch opcode */
			putbyte(data_loccnt, opval);

			/* calculate branch offset */
			addr = (value & 0xFFFF) - (loccnt + (page << 13) + phase_offset);

			/* check range */
			if (addr > 0x7Fu && addr < ~0x7Fu) {
				error("Branch address out of range!");
				return;
			}

			/* offset */
			putbyte(data_loccnt + 1, addr);
		}

		/* output line */
		println();
	}
}


/* ----
 * class3()
 * ----
 * 2 bytes, inherent addressing
 */

void
class3(int *ip)
{
	check_eol(ip);

	/* update location counter */
	loccnt += 2;

	/* generate code */
	if (pass == LAST_PASS) {
		/* opcodes */
		putbyte(data_loccnt, opval);
		putbyte(data_loccnt + 1, optype);

		/* output line */
		println();
	}
}


/* ----
 * class4()
 * ----
 * various addressing modes
 */

void
class4(int *ip)
{
	char buffer[32];
	char c;
	int len, mode;
	int i;

	/* skip spaces */
	while (isspace(prlnbuf[*ip]))
		(*ip)++;

	/* low/high byte prefix string */
	if (isalpha(prlnbuf[*ip])) {
		len = 0;
		i = *ip;

		/* extract string */
		for (;;) {
			c = prlnbuf[i];
			if (c == '\0' || c == ' ' || c == '\t' || c == ';')
				break;
			if ((!isalpha(c) && c != '_') || (len == 31)) {
				len = 0;
				break;
			}
			buffer[len++] = c;
			i++;
		}

		/* check */
		if (len) {
			buffer[len] = '\0';

			if (strcasecmp(buffer, "low_byte") == 0) {
				opext = 'L';
				*ip = i;
			}
			if (strcasecmp(buffer, "high_byte") == 0) {
				opext = 'H';
				*ip = i;
			}
		}
	}

	/* get operand */
	mode = getoperand(ip, opflg, ';');
	if (!mode)
		return;

	/* make opcode */
	if (pass == LAST_PASS) {
		for (i = 0; i < 32; i++) {
			if (mode & (1 << i))
				break;
		}
		opval += opvaltab[optype][i];
	}

	/* auto-tag */
	if (auto_tag) {
		if (pass == LAST_PASS) {
			putbyte(loccnt, 0xA0);
			putbyte(loccnt + 1, auto_tag_value);
		}
		loccnt += 2;
	}

	/* generate code */
	switch (mode) {
	case ACC:
		/* one byte */
		if (pass == LAST_PASS)
			putbyte(loccnt, opval);

		loccnt++;
		break;

	case IMM:
	case ZP:
	case ZP_X:
	case ZP_Y:
	case ZP_IND:
	case ZP_IND_X:
	case ZP_IND_Y:
		/* two bytes */
		if (pass == LAST_PASS) {
			putbyte(loccnt, opval);
			putbyte(loccnt + 1, value);
		}
		loccnt += 2;
		break;

	case ABS:
	case ABS_X:
	case ABS_Y:
	case ABS_IND:
	case ABS_IND_X:
		/* three bytes */
		if (pass == LAST_PASS) {
			putbyte(loccnt, opval);
			putword(loccnt + 1, value);
		}
		loccnt += 3;
		break;
	}

	/* auto-increment */
	if (auto_inc) {
		if (pass == LAST_PASS)
			putbyte(loccnt, auto_inc);

		loccnt += 1;
	}

	/* output line */
	if (pass == LAST_PASS)
		println();
}


/* ----
 * class5()
 * ----
 * 3 bytes, zp/relative addressing
 */

void
class5(int *ip)
{
	struct t_branch * branch;
	int zp;
	int mode;
	unsigned int addr;

	/* update location counter */
	loccnt += 3;

	/* get first operand */
	mode = getoperand(ip, ZP, ',');
	zp = value;
	if (!mode)
		return;

	/* get second operand */
	mode = getoperand(ip, ABS, ';');
	if (!mode)
		return;

	/* all branches tracked for long-branch handling */
	branch = getbranch(3);

	/* need more space for a long-branch */
	if ((branch) && (branch->convert))
		loccnt += 3;

	/* generate code */
	if (pass == LAST_PASS) {
		if ((branch) && (branch->convert)) {
			/* long-branch opcode */
			putbyte(data_loccnt, opval ^ 0x80);
			putbyte(data_loccnt + 1, zp);
			putbyte(data_loccnt + 2, 0x03);

			putbyte(data_loccnt + 3, 0x4C);
			putword(data_loccnt + 4, value & 0xFFFF);
		} else {
			/* short-branch opcode */
			putbyte(data_loccnt, opval);
			putbyte(data_loccnt + 1, zp);

			/* calculate branch offset */
			addr = (value & 0xFFFF) - (loccnt + (page << 13) + phase_offset);

			/* check range */
			if (addr > 0x7Fu && addr < ~0x7Fu) {
				error("Branch address out of range!");
				return;
			}

			/* offset */
			putbyte(data_loccnt + 2, addr);
		}

		/* output line */
		println();
	}
}


/* ----
 * class6()
 * ----
 * 7 bytes, src/dest/length
 */

void
class6(int *ip)
{
	int i;
	int addr[3];

	/* update location counter */
	loccnt += 7;

	/* get operands */
	for (i = 0; i < 3; i++) {
		if (!evaluate(ip, (i < 2) ? ',' : ';', 0))
			return;
		if (pass == LAST_PASS) {
			if (value & ~0xFFFF) {
				error("Operand size error!");
				return;
			}
		}
		addr[i] = value;
	}

	/* generate code */
	if (pass == LAST_PASS) {
		/* opcodes */
		putbyte(data_loccnt, opval);
		putword(data_loccnt + 1, addr[0]);
		putword(data_loccnt + 3, addr[1]);
		putword(data_loccnt + 5, addr[2]);

		/* output line */
		println();
	}
}


/* ----
 * class7()
 * ----
 * TST instruction
 */

void
class7(int *ip)
{
	int mode;
	int addr, imm;

	/* get first operand */
	mode = getoperand(ip, IMM, ',');
	imm = value;
	if (!mode)
		return;

	/* get second operand */
	mode = getoperand(ip, (ZP | ZP_X | ABS | ABS_X), ';');
	addr = value;
	if (!mode)
		return;

	/* make opcode */
	if (mode & (ZP | ZP_X))
		opval = 0x83;
	if (mode & (ABS | ABS_X))
		opval = 0x93;
	if (mode & (ZP_X | ABS_X))
		opval += 0x20;

	/* generate code */
	if (pass == LAST_PASS) {
		/* opcodes */
		putbyte(loccnt, opval);
		putbyte(loccnt + 1, imm);

		if (mode & (ZP | ZP_X))
			/* zero page */
			putbyte(loccnt + 2, addr);
		else
			/* absolute */
			putword(loccnt + 2, addr);
	}

	/* update location counter */
	if (mode & (ZP | ZP_X))
		loccnt += 3;
	else
		loccnt += 4;

	/* auto-increment */
	if (auto_inc) {
		if (pass == LAST_PASS)
			putbyte(loccnt, auto_inc);

		loccnt += 1;
	}

	/* output line */
	if (pass == LAST_PASS)
		println();
}


/* ----
 * class8()
 * ----
 * TAM/TMA instruction
 */

void
class8(int *ip)
{
	int mode;

	/* update location counter */
	loccnt += 2;

	/* get operand */
	mode = getoperand(ip, IMM, ';');
	if (!mode)
		return;

	/* generate code */
	if (pass == LAST_PASS) {
		/* check page index */
		if (value & 0xF8) {
			error("Incorrect page index!");
			return;
		}

		/* opcodes */
		putbyte(data_loccnt, opval);
		putbyte(data_loccnt + 1, (1 << value));

		/* output line */
		println();
	}
}


/* ----
 * class9()
 * ----
 * RMB/SMB instructions
 */

void
class9(int *ip)
{
	int bit;
	int mode;

	/* update location counter */
	loccnt += 2;

	/* get the bit index */
	mode = getoperand(ip, IMM, ',');
	bit = value;
	if (!mode)
		return;

	/* get the zero page address */
	mode = getoperand(ip, ZP, ';');
	if (!mode)
		return;

	/* generate code */
	if (pass == LAST_PASS) {
		/* check bit number */
		if (bit > 7) {
			error("Incorrect bit number!");
			return;
		}

		/* opcodes */
		putbyte(data_loccnt, opval + (bit << 4));
		putbyte(data_loccnt + 1, value);

		/* output line */
		println();
	}
}


/* ----
 * class10()
 * ----
 * BBR/BBS instructions
 */

void
class10(int *ip)
{
	struct t_branch * branch;
	int bit;
	int zp;
	int mode;
	unsigned int addr;

	/* update location counter */
	loccnt += 3;

	/* get the bit index */
	mode = getoperand(ip, IMM, ',');
	bit = value;
	if (!mode)
		return;

	/* get the zero page address */
	mode = getoperand(ip, ZP, ',');
	zp = value;
	if (!mode)
		return;

	/* get the jump address */
	mode = getoperand(ip, ABS, ';');
	if (!mode)
		return;

	/* all branches tracked for long-branch handling */
	branch = getbranch(3);

	/* need more space for a long-branch */
	if ((branch) && (branch->convert))
		loccnt += 3;

	/* generate code */
	if (pass == LAST_PASS) {
		/* check bit number */
		if (bit > 7) {
			error("Incorrect bit number!");
			return;
		}

		if ((branch) && (branch->convert)) {
			/* long-branch opcode */
			putbyte(data_loccnt, (opval + (bit << 4)) ^ 0x80);
			putbyte(data_loccnt + 1, zp);
			putbyte(data_loccnt + 2, 0x03);

			putbyte(data_loccnt + 3, 0x4C);
			putword(data_loccnt + 4, value & 0xFFFF);
		} else {
			/* short-branch opcode */
			putbyte(data_loccnt, opval + (bit << 4));
			putbyte(data_loccnt + 1, zp);

			/* calculate branch offset */
			addr = (value & 0xFFFF) - (loccnt + (page << 13) + phase_offset);

			/* check range */
			if (addr > 0x7Fu && addr < ~0x7Fu) {
				error("Branch address out of range!");
				return;
			}

			/* offset */
			putbyte(data_loccnt + 2, addr);
		}

		/* output line */
		println();
	}
}


/* ----
 * getoperand()
 * ----
 */

int
getoperand(int *ip, int flag, int last_char)
{
	unsigned int tmp;
	char c;
	int paren;
	int code;
	int mode;
	int pos;
	int end;

	/* init */
	paren = 0;
	auto_inc = 0;
	auto_tag = 0;

	/* skip spaces */
	while (isspace(prlnbuf[*ip]))
		(*ip)++;

	/* check addressing mode */
	switch (prlnbuf[*ip]) {
	case '\0':
	case ';':
		/* no operand */
		if (flag & ACC) {
			mode = ACC;
			break;
		}
		error("Operand missing!");
		return (0);

	case 'A':
	case 'a':
		/* accumulator */
		c = prlnbuf[(*ip) + 1];
		if (isspace(c) || c == '\0' || c == ';' || c == ',') {
			mode = ACC;
			(*ip)++;
			break;
		}

	default:
		/* other */
		c = prlnbuf[*ip];
		if (c == '*' && sdcc_mode) {
			/* change SDCC's '*' for ZP to '<' */
			c = '<';
		}

		switch (c) {
		case '#':
			/* immediate */
			mode = IMM;
			(*ip)++;
			break;

		case '<':
			/* zero page */
			mode = ZP | ZP_X | ZP_Y;
			(*ip)++;
			break;

		case '>':
			/* absolute */
			mode = ABS | ABS_X | ABS_Y;
			(*ip)++;
			break;

		case '[':
			/* indirect */
			mode = ABS_IND | ABS_IND_X | ZP_IND | ZP_IND_X | ZP_IND_Y;
			(*ip)++;
			if (prlnbuf[*ip] == '*' && sdcc_mode) {
				/* skip SDCC's redundant '*' */
				(*ip)++;
			}
			break;

		case '(':
			/* allow () as an alternative to [] for indirect? */
			if (asm_opt[OPT_INDPAREN]) {
				/* indirect */
				mode = ABS_IND | ABS_IND_X | ZP_IND | ZP_IND_X | ZP_IND_Y;
				(*ip)++;
				paren = 1;
				break;
			}
			/* fall through */

		default:
			if (opext == 'A')
				/* absolute */
				mode = ABS | ABS_X | ABS_Y;
			else
			if (opext == 'Z')
				/* zero page */
				mode = ZP | ZP_X | ZP_Y;
			else
			if (asm_opt[OPT_ZPDETECT])
				/* absolute OR zero page */
				mode = ABS | ABS_X | ABS_Y | ZP | ZP_X | ZP_Y;
			else
				/* absolute */
				mode = ABS | ABS_X | ABS_Y;
			break;
		}

		/* get value */
		if (!evaluate(ip, (paren == 1) ? ')' : 0, 0))
			return (0);

		/* traditional 6502 assembler auto-detect between ZP and ABS */
		if (mode == (ABS | ABS_X | ABS_Y | ZP | ZP_X | ZP_Y)) {
			/* was there an undefined or undefined-this-pass symbol? */
			if (undef || notyetdef ||
				((value & ~0xFF) != machine->ram_base)) {
				/* use ABS addressing, if available */
				if (flag & ABS)
					mode &= ~ZP;
				if (flag & ABS_X)
					mode &= ~ZP_X;
				if (flag & ABS_Y)
					mode &= ~ZP_Y;
			} else {
				/* use ZP addressing, if available */
				if (flag & ZP)
					mode &= ~ABS;
				if (flag & ZP_X)
					mode &= ~ABS_X;
				if (flag & ZP_Y)
					mode &= ~ABS_Y;
			}
		}

		/* check addressing mode */
		code = 0;
		end = 0;
		pos = 0;

		while (!end) {
			c = prlnbuf[*ip];
			if (c == ';' || c == '\0')
				break;
			switch (toupper(c)) {
			case ',':		/* , = 5 */
				if (!pos)
					pos = *ip;
				else {
					end = 1;
					break;
				}
				code++;
			case '+':		/* + = 4 */
				code++;
			case ')':		/* ] = 3 */
			case ']':		/* ] = 3 */
				code++;
				if (prlnbuf[*ip + 1] == '.') {
					end = 1;
					break;
				}
			case 'X':		/* X = 2 */
				code++;
			case 'Y':		/* Y = 1 */
				code++;
				code <<= 4;
			case ' ':
			case '\t':
				(*ip)++;
				break;
			default:
				code = 0xFFFFFF;
				end = 1;
				break;
			}
		}

		/* absolute, zp, or immediate */
		if (code == 0x000000)
			mode &= (ABS | ZP | IMM);

		/* indirect */
		else if (code == 0x000030)
			mode &= (ZP_IND | ABS_IND);		// ]

		/* indexed modes */
		else if (code == 0x000510)
			mode &= (ABS_Y | ZP_Y);			// ,Y
		else if (code == 0x000520)
			mode &= (ABS_X | ZP_X);			// ,X
		else if (code == 0x005230)
			mode &= (ZP_IND_X | ABS_IND_X);	// ,X]
		else if (code == 0x003510)
			mode &= (ZP_IND_Y);				// ],Y
		else if (code == 0x000001) {
			mode &= (ZP_IND_Y);				// ].tag
			(*ip) += 2;

			/* get tag */
			tmp = value;

			if (!evaluate(ip, 0, 0))
				return (0);

			/* ok */
			auto_tag = 1;
			auto_tag_value = value;
			value = tmp;
		}
		/* indexed modes with post-increment */
		else if (code == 0x051440) {
			mode &= (ABS_Y | ZP_Y);			// ,Y++
			auto_inc = 0xC8;
		}
		else if (code == 0x052440) {
			mode &= (ABS_X | ZP_X);			// ,X++
			auto_inc = 0xE8;
		}
		else if (code == 0x351440) {
			mode &= (ZP_IND_Y);				// ],Y++
			auto_inc = 0xC8;
		}

		/* absolute, zp, or immediate (or error) */
		else {
			mode &= (ABS | ZP | IMM);
			if (pos)
				*ip = pos;
		}

		/* check value on last pass */
		if (pass == LAST_PASS) {
			/* zp modes */
			if (mode & (ZP | ZP_X | ZP_Y | ZP_IND | ZP_IND_X | ZP_IND_Y) & flag) {
				/* extension stuff */
				if (opext && !auto_inc) {
					if (mode & (ZP_IND | ZP_IND_X | ZP_IND_Y))
						error("Instruction extension not supported in indirect modes!");
					if (opext == 'H')
						value++;
				}
				/* check address validity (accept $00xx as well for 6502-compatibility) */
				if ((value & ~0xFF) && ((value & ~0xFF) != machine->ram_base))
					error("Incorrect zero page address!");
			}

			/* immediate mode */
			else if (mode & (IMM)&flag) {
				/* extension stuff */
				if (opext == 'L')
					value = (value & 0xFF);
				else if (opext == 'H')
					value = (value & 0xFF00) >> 8;
				else if (opext != 0)
					error("Instruction extension not supported in immediate mode!");
				else {
					/* check for overflow, except in SDCC code (-256..255 are ok) */
					/* SDCC's code generator assumes that the assembler doesn't care */
					if ((sdcc_mode == 0) && (value & ~0xFF) && ((value & ~0xFF) != ~0xFF))
						error("Operand too large to fit in a byte!");
				}
			}

			/* absolute modes */
			else if (mode & (ABS | ABS_X | ABS_Y | ABS_IND | ABS_IND_X) & flag) {
				/* extension stuff */
				if (opext && !auto_inc) {
					if (mode & (ABS_IND | ABS_IND_X))
						error("Instruction extension not supported in indirect modes!");
					if (opext == 'H')
						value++;
				}
				/* check address validity */
				if (value & ~0xFFFF)
					error("Incorrect absolute address!");

				/* if HuC6280 and currently inside a ".kickc" C function/procedure */
				if (machine->ram_base && kickc_mode && scopeptr && proc_ptr) {
					if ((value & 0xFF00) == 0x0000) {
						error("Incorrect absolute address!");
					}
				}
			}
		}
		break;
	}

	/* compare addressing mode */
	mode &= flag;
	if (!mode) {
		error("Incorrect addressing mode!");
		return (0);
	}

	/* skip spaces */
	while (isspace(prlnbuf[*ip]))
		(*ip)++;

	/* get last char */
	c = prlnbuf[*ip];

	/* check if it's what the user asked for */
	switch (last_char) {
	case ';':
		/* last operand */
		if (c != ';' && c != '\0') {
			error("Syntax error!");
			return (0);
		}
		(*ip)++;
		break;

	case ',':
		/* need more operands */
		if (c != ',') {
			error("Operand missing!");
			return (0);
		}
		(*ip)++;
		break;
	}

	/* ok */
	return (mode);
}


/* ----
 * getstring()
 * ----
 * get a string from prlnbuf
 */

int
getstring(int *ip, char *buffer, int size)
{
	char c;
	int i;

	/* skip spaces */
	while (isspace(prlnbuf[*ip]))
		(*ip)++;

	/* string must be enclosed */
	if (prlnbuf[(*ip)++] != '\"') {
		error("Incorrect string syntax!");
		return (0);
	}

	/* get string */
	i = 0;
	for (;;) {
		c = prlnbuf[(*ip)++];
		if (c == '\"')
			break;
		if (c == '\0') {
			error("String terminator missing!");
			return (0);
		}
		if (i >= size) {
			error("String too long!");
			return (0);
		}
		buffer[i++] = c;
	}

	/* end the string */
	buffer[i] = '\0';

	/* skip spaces */
	while (isspace(prlnbuf[*ip]))
		(*ip)++;

	/* ok */
	return (1);
}


/* ----
 * getbranch()
 * ----
 * return tracking structure for the current branch
 */

static struct t_branch *
getbranch(int opcode_length)
{
	struct t_branch * branch;
	unsigned int addr;

#if 1
	/* do not track yet, because .ifref can change after the first */
	/* pass which can then change the sequence of tracked branches */
	if (pass == FIRST_PASS)
		return (NULL);

	/* no tracking info if transitioned from FIRST_PASS to LAST_PASS */
	if ((branchlst == NULL) && (pass == LAST_PASS))
		return (NULL);

	/* track all branches for long-branch handling */
	if (pass_count == 2) {
#else
	if (pass == FIRST_PASS) {
#endif
		/* remember this branch instruction */
		if ((branch = malloc(sizeof(struct t_branch))) == NULL) {
			error("Out of memory!");
			return NULL;
		}
		if (branchlst == NULL)
			branchlst = branch;
		if (branchptr != NULL)
			branchptr->next = branch;
		branchptr = branch;

		branch->next = NULL;
		branch->convert = 0;

		if (asm_opt[OPT_LBRANCH] && !complex_expr) {
			/* enable expansion to long-branch */
			branch->label = expr_toplabl;
		} else {
			/* assume short, error if out-of-range */
			branch->label = NULL;
		}
	} else {
		/* update this branch instruction */
		if (branchptr == NULL) {
			fatal_error("Untracked branch instruction!");
			return NULL;
		}

		branch = branchptr;
		branchptr = branchptr->next;

		/* sanity check */
		if ((branch->label != NULL) && (branch->label != expr_lablptr)) {
#if 0
			if (branch->label == expr_toplabl) {
				/* resolve branch outside of label-scope */
				/* disable for now, because this is more */
				/* likely to be an error than deliberate */
//				branch->label = expr_lablptr;
			} else {
				fatal_error("Branch label mismatch!");
				return NULL;
			}
#else
			fatal_error("Branch label mismatch!");
			return NULL;
#endif
		}
	}

	branch->addr = (loccnt + (page << 13) + phase_offset) & 0xFFFF;
	branch->checked = 0;

	if (pass == LAST_PASS) {
		/* display the branches that have been converted */
		if (branch->convert && asm_opt[OPT_WARNING]) {
			loccnt -= opcode_length;
			warning("Converted to long-branch!\n");
			loccnt += opcode_length;
		}
	} else {
		/* check if it is outside short-branch range */
		if ((branch->label != NULL) &&
		    (branch->label->defthispass) &&
		    (branch->label->type == DEFABS)) {
			addr = (branch->label->value & 0xFFFF) - branch->addr;

			if (addr > 0x7Fu && addr < ~0x7Fu) {
				if (branch->convert == 0)
					++branches_changed;
				branch->convert = 1;
			}

			branch->checked = 1;
		}
	}

	return branch;
}


/* ----
 * branchopt()
 * ----
 * convert out-of-range short-branches into long-branches, also
 * convert incorrectly-guessed long-branches back to short-branches
 */

int
branchopt(void)
{
	struct t_branch * branch;
	unsigned int addr;
	int just_changed = 0;

	/* look through the entire list of branch instructions */
	for (branch = branchlst; branch != NULL; branch = branch->next) {

		/* check to see if a branch needs to be converted */
		if ((branch->checked == 0) &&
		    (branch->label != NULL) &&
		    (branch->label->type == DEFABS)) {
			/* check if it is outside short-branch range */
			addr = (branch->label->value & 0xFFFF) - branch->addr;

			if (addr > 0x7Fu && addr < ~0x7Fu) {
				if (branch->convert == 0)
					++just_changed;
				branch->convert = 1;
#if 0 /* This saving just doesn't seem to be worth an extra pass */
			} else {
				if (branch->convert == 1)
					++just_changed;
				branch->convert = 0;
#endif
			}
		}
	}

	/* report total changes this pass */
	branches_changed += just_changed;
	if (branches_changed)
		printf("     Changed %4d branches from short to long.\n", branches_changed);

	/* do another pass if anything just changed, except if KickC because */
	/* any changes during the pass itself can change a forward-reference */
	return ((kickc_opt) ? branches_changed : just_changed);
}
