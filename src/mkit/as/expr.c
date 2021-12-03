#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "defs.h"
#include "externs.h"
#include "protos.h"
#include "expr.h"

/* ----
 * pc_symbol
 * ----
 * pre-defined symbol used for the program counter in an expression
 */

t_symbol pc_symbol = {
	NULL, /* next */
	NULL, /* local */
	NULL, /* proc */
	DEFABS, /* type */
	0, /* value */
	0, /* bank */
	0, /* page */
	0, /* nb */
	0, /* size */
	0, /* vram */
	0, /* pal */
	0, /* defcnt */
	0, /* refcnt */
	0, /* reserved */
	0, /* data_type */
	0, /* data_size */
	"*" /* name */
};


/* ----
 * is_expr_symbol()
 * ----
 * is the next string a symbol?
 * this is complicated because a "multi-label" looks like a "not" or a "not-equal".
 */

inline int is_expr_symbol (char * pstr)
{
	unsigned char c = *pstr++;
	if (isalpha(c) || c == '_' || c == '.' || c == '@')
		return 1;
	if (c != '!')
		return 0;
	for (;;) {
		c = *pstr;
		if (isalnum(c) || (c == '_') || (c == '.')) {
			++pstr;
		} else {
			break;
		}
	}
	c = *pstr;
	return (c == '+' || c == '-') ? 1 : 0;
}


/* ----
 * evaluate()
 * ----
 * evaluate an expression
 */

int
evaluate(int *ip, char last_char)
{
	int end, level;
	int op, type;
	int arg;
	int i;
	unsigned char c;

	end = 0;
	level = 0;
	undef = 0;
	op_idx = 0;
	val_idx = 0;
	value = 0;
	val_stack[0] = 0;
	need_operator = 0;
	expr_lablptr = NULL;
	expr_lablcnt = 0;
	op = OP_START;
	func_idx = 0;

	/* array index to pointer */
	expr = &prlnbuf[*ip];

	/* skip spaces */
cont:
	while (isspace(*expr))
		expr++;

	/* search for a continuation char */
	if (*expr == '\\') {
		/* skip spaces */
		i = 1;
		while (isspace(expr[i]))
			i++;

		/* check if end of line */
		if (expr[i] == ';' || expr[i] == '\0') {
			/* output */
			if (!continued_line) {
				/* replace '\' with three dots */
				strcpy(expr, "...");

				/* remember the 1st line continued */
				strcpy(tmplnbuf, prlnbuf);
			}

			/* ok */
			continued_line++;

			/* read a new line */
			if (readline() == -1)
				return (0);

			/* rewind line pointer and continue */
			expr = &prlnbuf[preproc_sfield];
			goto cont;
		}
	}

	/* parser main loop */
	while (!end) {
		c = *expr;

		/* number */
		if (isdigit(c)) {
			if (need_operator)
				goto error;
			if (!push_val(T_DECIMAL))
				return (0);
		}

		/* symbol */
		else
//		if (isalpha(c) || c == '_' || c == '.' || c == '@') {
		if (is_expr_symbol(expr)) {
			if (need_operator)
				goto error;
			if (!push_val(T_SYMBOL))
				return (0);
		}

		/* operators */
		else {
			switch (c) {
			/* function arg */
			case '\\':
				if (func_idx == 0) {
					error("Syntax error in expression!");
					return (0);
				}
				expr++;
				c = *expr++;
				if (c < '1' || c > '9') {
					error("Invalid function argument index!");
					return (0);
				}
				arg = c - '1';
				expr_stack[func_idx++] = expr;
				expr = func_arg[func_idx - 2][arg];
				break;

			/* hexa prefix */
			case '$':
				if (need_operator)
					goto error;
				if (!push_val(T_HEXA))
					return (0);
				break;

			/* character prefix */
			case '\'':
				if (need_operator)
					goto error;
				if (!push_val(T_CHAR))
					return (0);
				break;

			/* round brackets */
			case '(':
				if (need_operator)
					goto error;
				if (!push_op(OP_OPEN))
					return (0);
				level++;
				expr++;
				break;
			case ')':
				if (!need_operator)
					goto error;
				if (level == 0) {
					if (last_char != ')')
						goto error;
					end = 3;
					break;
				}
				while (op_stack[op_idx] != OP_OPEN) {
					if (!do_op())
						return (0);
				}
				op_idx--;
				level--;
				expr++;
				break;

			/* low byte, not equal, left shift, lower, lower or equal */
			case '<':
				switch (*(expr+1)) {
				case '>':
					if (!need_operator)
						goto error;
					op = OP_NOT_EQUAL;
					expr += 2;
					break;
				case '<':
					if (!need_operator)
						goto error;
					op = OP_SHL;
					expr += 2;
					break;
				case '=':
					if (!need_operator)
						goto error;
					op = OP_LOWER_EQUAL;
					expr += 2;
					break;
				default:
					if (!need_operator)
						op = OP_LOW;
					else
						op = OP_LOWER;
					expr++;
					break;
				}
				if (!push_op(op))
					return (0);
				break;

			/* high byte, right shift, higher, higher or equal */
			case '>':
				switch (*(expr+1)) {
				case '>':
					if (!need_operator)
						goto error;
					op = OP_SHR;
					expr += 2;
					break;
				case '=':
					if (!need_operator)
						goto error;
					op = OP_HIGHER_EQUAL;
					expr += 2;
					break;
				default:
					if (!need_operator)
						op = OP_HIGH;
					else
						op = OP_HIGHER;
					expr++;
					break;
				}
				if (!push_op(op))
					return (0);
				break;

			/* equal */
			case '=':
				if (!need_operator)
					goto error;
				if (!push_op(OP_EQUAL))
					return (0);
				expr++;
				if (*expr == '=')
					expr++;
				break;

			/* one complement */
			case '~':
				if (need_operator)
					goto error;
				if (!push_op(OP_COM))
					return (0);
				expr++;
				break;

			/* sub, neg */
			case '-':
				if (need_operator)
					op = OP_SUB;
				else
					op = OP_NEG;
				if (!push_op(op))
					return (0);
				expr++;
				break;

			/* bank byte, xor */
			case '^':
				if (need_operator)
					op = OP_XOR;
				else
					op = OP_BANK;
				if (!push_op(op))
					return (0);
				expr++;
				break;

			/* not, not equal */
			case '!':
				if (!need_operator)
					op = OP_NOT;
				else {
					op = OP_NOT_EQUAL;
					expr++;
					if (*expr != '=')
						goto error;
				}
				if (!push_op(op))
					return (0);
				expr++;
				break;

			/* binary prefix, current PC */
			case '%':
			case '*':
				if (!need_operator) {
					if (c == '%')
						type = T_BINARY;
					else
						type = T_SYMBOL;
					if (!push_val(type))
						return (0);
					break;
				}

			/* modulo, mul, add, div, and, xor, or */
			case '+':
			case '/':
			case '&':
			case '|':
				if (!need_operator)
					goto error;
				switch (c) {
				case '%': op = OP_MOD; break;
				case '*': op = OP_MUL; break;
				case '+': op = OP_ADD; break;
				case '/': op = OP_DIV; break;
				case '&': op = OP_AND; break;
				case '|': op = OP_OR;  break;
				}
				if (!push_op(op))
					return (0);
				expr++;
				break;

			/* skip immediate operand prefix if in macro */
			case '#':
				if (expand_macro)
					expr++;
				else
					end = 3;
				break;

			/* space or tab */
			case ' ':
			case '\t':
				expr++;
				break;

			/* end of line */
			case '\0':
				if (func_idx) {
					func_idx--;
					expr = expr_stack[func_idx];
					break;
				}
			case ';':
				end = 1;
				break;
			case ',':
				end = 2;
				break;
			default:
				end = 3;
				break;
			}
		}
	}

	if (!need_operator)
		goto error;
	if (level != 0)
		goto error;
	while (op_stack[op_idx] != OP_START) {
		if (!do_op())
			return (0);
	}

	/* get the expression value */
	value = val_stack[val_idx];

	/* any undefined symbols? trap that if in the last pass */
	if (undef) {
		if (pass == LAST_PASS)
			error("Undefined symbol in operand field!");
	}

	/* check if the last char is what the user asked for */
	switch (last_char) {
	case ';':
		if (end != 1)
			goto error;
		expr++;
		break;
	case ',':
		if (end != 2) {
			error("Argument missing!");
			return (0);
		}
		expr++;
		break;
	}

	/* convert back the pointer to an array index */
	*ip = expr - prlnbuf;

	/* ok */
	return (1);

	/* syntax error */
error:
	error("Syntax error in expression!");
	return (0);
}


/* ----
 * push_val()
 * ----
 * extract a number and push it on the value stack
 */

int
push_val(int type)
{
	unsigned int mul, val;
	int op;
	char c;

	val = 0;
	c = *expr;

	switch (type) {

	/* char ascii value */
	case T_CHAR:
		expr++;
		val = *expr++;
		if ((*expr != c) || (val == 0)) {
			error("Syntax Error!");
			return (0);
		}
		expr++;
		break;

	/* symbol */
	case T_SYMBOL:
		if (*expr == '*') {
			/* symbol for the program counter */
			symbol[0] = 1;
			symbol[1] = '*';
			symbol[2] = '\0';
			expr++;

			if (data_loccnt == -1)
				pc_symbol.value = (loccnt + (page << 13));
			else
				pc_symbol.value = (data_loccnt + (page << 13));
			pc_symbol.bank = bank + bank_base;
			pc_symbol.page = page;

			expr_lablptr = &pc_symbol;
		} else {
			/* extract it */
			if (!getsym())
				return (0);

			/* an user function? */
			if (func_look()) {
				if (!func_getargs())
					return (0);

				expr_stack[func_idx++] = expr;
				expr = func_ptr->line;
				return (1);
			}

			/* a predefined function? */
			op = check_keyword();
			if (op) {
				if (!push_op(op))
					return (0);
				else
					return (1);
			}

			/* search the symbol */
			expr_lablptr = stlook(1);
		}

		/* check if undefined, if not get its value */
		if (expr_lablptr == NULL)
			return (0);
		else if (expr_lablptr->type == UNDEF)
			undef++;
		else if (expr_lablptr->type == IFUNDEF)
			undef++;
		else if ((expr_lablptr->bank == STRIPPED_BANK) &&
			((proc_ptr == NULL) || (proc_ptr->bank != STRIPPED_BANK))) {
				error("Symbol from an unused procedure that was stripped out!");
				undef++;
			}
		else
			val = expr_lablptr->value;

		/* remember we have seen a symbol in the expression */
		expr_lablcnt++;
		break;

	/* binary number %1100_0011 */
	case T_BINARY:
		mul = 2;
		goto extract;

	/* hexa number $15AF */
	case T_HEXA:
		mul = 16;
		goto extract;

	/* decimal number 48 (or hexa 0x5F) */
	case T_DECIMAL:
		if ((c == '0') && (toupper(expr[1]) == 'X')) {
			mul = 16;
			expr++;
		}
		else {
			mul = 10;
			val = c - '0';
		}
		/* extract a number */
extract:
		for (;;) {
			expr++;
			c = *expr;

			if (isdigit(c))
				c -= '0';
			else if (isalpha(c)) {
				c = toupper(c);
				if (c < 'A' && c > 'F')
					break;
				else {
					c -= 'A';
					c += 10;
				}
			}
			else if (c == '_' && mul == 2)
				continue;
			else
				break;
			if (c >= mul)
				break;
			val = (val * mul) + c;
		}
		break;
	}

	/* check for too big expression */
	if (val_idx == 63) {
		error("Expression too complex!");
		return (0);
	}

	/* push the result on the value stack */
	val_idx++;
	val_stack[val_idx] = val;

	/* next must be an operator */
	need_operator = 1;

	/* ok */
	return (1);
}


/* ----
 * getsym()
 * ----
 * extract a symbol name from the input string
 */

int
getsym(void)
{
	int i;
	char c;

	i = 0;

	/* get the symbol, stop at the first 'non symbol' char */
	for (;;) {
		c = *expr;
		if (i == 0 && isdigit(c))
			break;
		if (isalnum(c) || (c == '_') || (c == '.') || (i == 0 && c == '@') || (i == 0 && c == '!')) {
			if (i < (SBOLSZ - 1)) { symbol[++i] = c; }
			expr++;
		} else {
			break;
		}
	}

	/* store symbol length */
	symbol[0] = i;
	symbol[i + 1] = '\0';

	/* is it a reserved symbol? */
	if (i == 1) {
		switch (toupper(symbol[1])) {
		case 'A':
		case 'X':
		case 'Y':
			error("Symbol is reserved (A, X or Y)!");
			i = 0;
		}
	}

	/* is it a multi-label? */
	if (symbol[1] == '!') {
		struct t_symbol * baselabl;
		int whichlabl = 0;
		char tail [10];

		/* find the base multi-label instance */
		if ((baselabl = stlook(1)) == NULL) {
			fatal_error("Out of memory!");
			return (0);
		}

		/* sanity check */
		if (baselabl->type != UNDEF) {
			fatal_error("How did this multi-label get defined!");
			return (0);
		}

		/* find out how many labels +/- to jump */
		c = *expr;
		if (c != '+' && c != '-') {
			fatal_error("References to a multi-label must include at least one \'+\' or \'-\'!");
			return (0);
		}
		while (*expr == c) {
			whichlabl += ((c == '+') ? 1 : -1);
			expr++;
		}
		if (whichlabl < 0) { ++whichlabl; }

		/* add the current multi-label instance */
		whichlabl += baselabl->defcnt;
		if ((whichlabl < 0) || (whichlabl > 0x7FFFF)) {
			/* illegal value */
			whichlabl = 0;
		}

		/* create the target multi-label name */
		sprintf(tail, "!%d", 0x7FFFF & whichlabl);
		strncat(symbol, tail, SBOLSZ - 1 - strlen(symbol));
		i = symbol[0] = strlen(&symbol[1]);
	}

	if (i >= SBOLSZ - 1) {
		char errorstr[512];
		snprintf(errorstr, 512, "Symbol name too long ('%s' is %d chars long, max is %d)", symbol + 1, i, SBOLSZ - 2);
		fatal_error(errorstr);
		return (0);
	}

	return (i);
}


/* ----
 * check_keyword()
 * ----
 * verify if the current symbol is a reserved function
 */

int
check_keyword(void)
{
	int op = 0;

	/* check if its an assembler function */
	if (symbol[0] == keyword[0][0] && !strcasecmp(symbol, keyword[0]))
		op = OP_DEFINED;
	else if (symbol[0] == keyword[1][0] && !strcasecmp(symbol, keyword[1]))
		op = OP_HIGH;
	else if (symbol[0] == keyword[2][0] && !strcasecmp(symbol, keyword[2]))
		op = OP_LOW;
	else if (symbol[0] == keyword[3][0] && !strcasecmp(symbol, keyword[3]))
		op = OP_PAGE;
	else if (symbol[0] == keyword[4][0] && !strcasecmp(symbol, keyword[4]))
		op = OP_BANK;
	else if (symbol[0] == keyword[7][0] && !strcasecmp(symbol, keyword[7]))
		op = OP_SIZEOF;
	else {
		if (machine->type == MACHINE_PCE) {
			/* PCE specific functions */
			if (symbol[0] == keyword[5][0] && !strcasecmp(symbol, keyword[5]))
				op = OP_VRAM;
			else if (symbol[0] == keyword[6][0] && !strcasecmp(symbol, keyword[6]))
				op = OP_PAL;
		}
	}

	/* extra setup for functions that send back symbol infos */
	switch (op) {
	case OP_DEFINED:
	case OP_HIGH:
	case OP_LOW:
	case OP_PAGE:
	case OP_BANK:
	case OP_VRAM:
	case OP_PAL:
	case OP_SIZEOF:
		expr_lablptr = NULL;
		expr_lablcnt = 0;
		break;
	}

	/* ok */
	return (op);
}


/* ----
 * push_op()
 * ----
 * push an operator on the stack
 */

int
push_op(int op)
{
	if (op != OP_OPEN) {
		while (op_pri[op_stack[op_idx]] >= op_pri[op]) {
			if (!do_op())
				return (0);
		}
	}
	if (op_idx == 63) {
		error("Expression too complex!");
		return (0);
	}
	op_idx++;
	op_stack[op_idx] = op;
	need_operator = 0;
	return (1);
}


/* ----
 * do_op()
 * ----
 * apply an operator to the value stack
 */

int
do_op(void)
{
	int val[2];
	int op;

	/* operator */
	op = op_stack[op_idx--];

	/* first arg */
	val[0] = val_stack[val_idx];

	/* second arg */
	if (op_pri[op] < 9)
		val[1] = val_stack[--val_idx];
	else
		val[1] = 0;

	switch (op) {
	/* BANK */
	case OP_BANK:
		if (!check_func_args("BANK"))
			return (0);
		if (pass == LAST_PASS) {
			if (expr_lablptr->bank >= RESERVED_BANK) {
				if ((expr_lablptr->bank != STRIPPED_BANK) ||
					(proc_ptr == NULL) || (proc_ptr->bank != STRIPPED_BANK)) {
					error("No BANK index for this symbol!");
					val[0] = 0;
					break;
				}
			}
		}
		val[0] = expr_lablptr->bank + (val[0] / 8192) - (expr_lablptr->value / 8192);
		break;

	/* PAGE */
	case OP_PAGE:
		if (!check_func_args("PAGE"))
			return (0);
		val[0] = expr_lablptr->page + (val[0] / 8192) - (expr_lablptr->value / 8192);
		break;

	/* VRAM */
	case OP_VRAM:
		if (!check_func_args("VRAM"))
			return (0);
		if (pass == LAST_PASS) {
			if (expr_lablptr->vram == -1)
				error("No VRAM address for this symbol!");
		}
		val[0] = expr_lablptr->vram;
		break;

	/* PAL */
	case OP_PAL:
		if (!check_func_args("PAL"))
			return (0);
		if (pass == LAST_PASS) {
			if (expr_lablptr->pal == -1)
				error("No palette index for this symbol!");
		}
		val[0] = expr_lablptr->pal;
		break;

	/* DEFINED */
	case OP_DEFINED:
		if (!check_func_args("DEFINED"))
			return (0);
		if ((expr_lablptr->type != IFUNDEF) && (expr_lablptr->type != UNDEF))
			val[0] = 1;
		else {
			val[0] = 0;
			undef--;
		}
		break;

	/* SIZEOF */
	case OP_SIZEOF:
		if (!check_func_args("SIZEOF"))
			return (0);
		if (pass == LAST_PASS) {
			if (expr_lablptr->data_type == -1) {
				error("No size attributes for this symbol!");
				return (0);
			}
		}
		val[0] = expr_lablptr->data_size;
		break;

	/* HIGH */
	case OP_HIGH:
		val[0] = (val[0] & 0xFF00) >> 8;
		break;

	/* LOW */
	case OP_LOW:
		val[0] = val[0] & 0xFF;
		break;

	case OP_ADD:
		val[0] = val[1] + val[0];
		break;

	case OP_SUB:
		val[0] = val[1] - val[0];
		break;

	case OP_MUL:
		val[0] = val[1] * val[0];
		break;

	case OP_DIV:
		if (val[0] == 0) {
			error("Divide by zero!");
			return (0);
		}
		val[0] = val[1] / val[0];
		break;

	case OP_MOD:
		if (val[0] == 0) {
			error("Divide by zero!");
			return (0);
		}
		val[0] = val[1] % val[0];
		break;

	case OP_NEG:
		val[0] = -val[0];
		break;

	case OP_SHL:
		val[0] = val[1] << (val[0] & 0x1F);
		break;

	case OP_SHR:
		val[0] = val[1] >> (val[0] & 0x1f);
		break;

	case OP_OR:
		val[0] = val[1] | val[0];
		break;

	case OP_XOR:
		val[0] = val[1] ^ val[0];
		break;

	case OP_AND:
		val[0] = val[1] & val[0];
		break;

	case OP_COM:
		val[0] = ~val[0];
		break;

	case OP_NOT:
		val[0] = !val[0];
		break;

	case OP_EQUAL:
		val[0] = (val[1] == val[0]);
		break;

	case OP_NOT_EQUAL:
		val[0] = (val[1] != val[0]);
		break;

	case OP_LOWER:
		val[0] = (val[1] < val[0]);
		break;

	case OP_LOWER_EQUAL:
		val[0] = (val[1] <= val[0]);
		break;

	case OP_HIGHER:
		val[0] = (val[1] > val[0]);
		break;

	case OP_HIGHER_EQUAL:
		val[0] = (val[1] >= val[0]);
		break;

	default:
		error("Invalid operator in expression!");
		return (0);
	}

	/* result */
	val_stack[val_idx] = val[0];
	return (1);
}


/* ----
 * check_func_args()
 * ----
 * check BANK/PAGE/VRAM/PAL function arguments
 */

int
check_func_args(char *func_name)
{
	char string[64];

	if (expr_lablcnt == 1)
		return (1);
	else if (expr_lablcnt == 0)
		sprintf(string, "No symbol in function %s!", func_name);
	else {
		sprintf(string, "Too many symbols in function %s!", func_name);
	}

	/* output message */
	error(string);
	return (0);
}
