/*	File io.h: 2.1 (83/03/20,16:02:07) */
/*% cc -O -c %
 *
 */

#ifndef _IO_H
#define _IO_H

#if defined(WIN32)
#define CR_LEN  2
#else
#define CR_LEN  1
#endif


intptr_t openin (char *p);
intptr_t openout (void);
void outfname (char *s);
void fixname (char *s);
intptr_t checkname (char *s);
#if defined(__APPLE__) || defined(__CYGWIN__)
void _kill (void);
#define kill _kill
#else
void kill (void);
#endif
void unget_line (void);
void readline (void);

/* could otherwise be char */
intptr_t inbyte (void);
intptr_t inchar (void);
intptr_t gch (void);
intptr_t nch (void);
intptr_t ch (void);

void pl (char *str);
void glabel (char *lab);
void gnlabel (intptr_t nlab);
void olprfix (void);
void col (void);
void comment (void);
void prefix (void);
void tab (void);
void ol (char *ptr);
void ot (char *ptr);
void nl (void);
void outsymbol (char *ptr);
void outlabel (intptr_t label);
void outdec (intptr_t number);
void outhex (intptr_t number);
void outhexfix (intptr_t number, intptr_t length);
char outbyte (char c);
void outstr (char *ptr);

#endif
