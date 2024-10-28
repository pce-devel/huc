/*	File while.c: 2.1 (83/03/20,16:02:22) */
/*% cc -O -c %
 *
 */

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "defs.h"
#include "data.h"
#include "error.h"
#include "gen.h"
#include "io.h"
#include "while.h"

void addwhile (int *ptr)
{
	int k;

	if (wsptr == WS_LIMIT) {
		error("too many active whiles");
		return;
	}
	k = 0;
	while (k < WS_COUNT)
		*wsptr++ = ptr[k++];
}

void delwhile (void)
{
	if (readwhile())
		wsptr = wsptr - WS_COUNT;
}

int *readwhile (void)
{
	if (wsptr == ws) {
		error("no active do/for/while/switch");
		return (0);
	}
	else
		return (wsptr - WS_COUNT);
}

int *findwhile (void)
{
	int *ptr;

	for (ptr = wsptr; ptr != ws;) {
		ptr = ptr - WS_COUNT;
		if (ptr[WS_TYPE] != WS_SWITCH)
			return (ptr);
	}
	error("no active do/for/while");
	return (NULL);
}

int *readswitch (void)
{
	int *ptr;

	ptr = readwhile();
	if (ptr)
		if (ptr[WS_TYPE] == WS_SWITCH)
			return (ptr);

	return (0);
}

void addcase (int val)
{
	int lab;

	if (swstp == SWST_COUNT)
		error("too many case labels");
	else {
		swstcase[swstp] = val;
		swstlabel[swstp++] = lab = getlabel();
		gcase(lab, val);
	}
}
