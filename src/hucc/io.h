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


int openin (char *p);
int openout (void);
void outfname (char *s);
void fixname (char *s);
int checkname (char *s);
#if defined(__APPLE__) || defined(__CYGWIN__)
void _kill (void);
#define kill _kill
#else
void kill (void);
#endif
void unget_line (void);
void readline (void);

/* could otherwise be char */
int inbyte (void);
int inchar (void);
int gch (void);
int nch (void);
int ch (void);

void pl (char *str);
void glabel (char *lab);
void gnlabel (int nlab);
void olprfix (void);
void col (void);
void comment (void);
void prefix (void);
void tab (void);
void ol (char *ptr);
void ot (char *ptr);
void nl (void);
void outlocal (SYMBOL *ptr);
void outsymbol (SYMBOL *ptr);
void outconst (int label);
void outlabel (int label);
void outdec (int number);
void outhex (int number);
void outhexfix (int number, int length);
char outbyte (char c);
void outstr (const char *ptr);

#endif
