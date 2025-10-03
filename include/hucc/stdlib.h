#ifndef _hucc_stdlib_h
#define _hucc_stdlib_h

#define RAND_MAX 32767

#ifdef __HUCC__
extern int __fastcall abs( int value<acc> );
#endif

#endif // _hucc_stdlib_h
