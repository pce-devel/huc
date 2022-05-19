/*	File while.c: 2.1 (83/03/20,16:02:22) */
/*% cc -O -c %
 *
 */

#include <stdint.h>
#include <stdio.h>
#include "defs.h"
#include "data.h"
#include "error.h"
#include "gen.h"
#include "io.h"
#include "while.h"

void addwhile (intptr_t *ptr)
/*intptr_t	ptr[];*/
{
	intptr_t k;

	if (wsptr == WSMAX) {
		error("too many active whiles");
		return;
	}
	k = 0;
	while (k < WSSIZ)
		*wsptr++ = ptr[k++];
}

void delwhile (void)
{
	if (readwhile())
		wsptr = wsptr - WSSIZ;
}

intptr_t *readwhile (void)
{
	if (wsptr == ws) {
		error("no active do/for/while/switch");
		return (0);
	}
	else
		return (wsptr - WSSIZ);
}

intptr_t *findwhile (void)
{
	intptr_t *ptr;

	for (ptr = wsptr; ptr != ws;) {
		ptr = ptr - WSSIZ;
		if (ptr[WSTYP] != WSSWITCH)
			return (ptr);
	}
	error("no active do/for/while");
	return (NULL);
}

intptr_t *readswitch (void)
{
	intptr_t *ptr;

	ptr = readwhile();
	if (ptr)
		if (ptr[WSTYP] == WSSWITCH)
			return (ptr);

	return (0);
}

void addcase (intptr_t val)
{
	intptr_t lab;

	if (swstp == SWSTSZ)
		error("too many case labels");
	else {
		swstcase[swstp] = val;
		swstlab[swstp++] = lab = getlabel();
		gnlabel(lab);
	}
}
