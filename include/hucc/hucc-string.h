#ifndef _hucc_string_h
#define _hucc_string_h

/****************************************************************************
; ***************************************************************************
;
; hucc-string.h
;
; Not-quite-standard, but fast, replacements for <string.h>.
;
; Copyright John Brandwood 2024.
;
; Distributed under the Boost Software License, Version 1.0.
; (See accompanying file LICENSE_1_0.txt or copy at
;  http://www.boost.org/LICENSE_1_0.txt)
;
; ***************************************************************************
; ***************************************************************************
;
; !!! WARNING : non-standard return values !!!
;
; Strings are limited to a maximum of 255 characters (+ the terminator)!
;
; The memcpy(), strcpy() and strcat() functions do NOT return the destination
; address, and they are declared "void" to check that the value is not used.
;
; mempcpy() is provided which returns the end address instead of the starting
; address, because this is typically more useful.
;
; Please note that both memcpy() and memset() are implemented using a TII for
; speed, and so the length should be < 16 bytes if used in time-critical bits
; of code (such as when using a split screen) because they delay interrupts.
;
; strncpy() and strncat() are not provided, because strncpy() was not created
; for the purpose of avoiding string overruns, and strncat() is just a poorly
; designed function IMHO.
;
; POSIX strlcpy() and strlcat() are provided instead, but once again they are
; slightly non-standard in that the return value when there is an overflow is
; the buffer size (so that the overflow can be detected), instead of the full
; size of the destination string that was too big to fit in the buffer.
;
; ***************************************************************************
; **************************************************************************/

// *************
// Functions in hucc-string.asm ...
// *************

#ifdef __HUCC__

#asmdef	HUCC_USES_STRING 1

#if 1

extern void __fastcall strcpy( char *destination<_di>, char *source<_bp> );
extern void __fastcall strcat( char *destination<_di>, char *source<_bp> );

extern unsigned int __fastcall strlen( char *source<_bp> );

extern unsigned int __fastcall strlcpy( char *destination<_di>, char *source<_bp>, unsigned char size<acc> );
extern unsigned int __fastcall strlcat( char *destination<_di>, char *source<_bp>, unsigned char size<acc> );

extern void __fastcall memcpy( unsigned char *destination<ram_tii_dst>, unsigned char *source<ram_tii_src>, unsigned int count<acc> );
extern unsigned char * __fastcall mempcpy( unsigned char *destination<ram_tii_dst>, unsigned char *source<ram_tii_src>, unsigned int count<acc> );

extern void __fastcall memset( unsigned char *destination<ram_tii_src>, unsigned char value<_al>, unsigned int count<acc> );

extern int __fastcall strcmp( char *destination<_di>, char *source<_bp> );
extern int __fastcall strncmp( char *destination<_di>, char *source<_bp>, unsigned int count<_ax> );
extern int __fastcall memcmp( unsigned char *destination<_di>, unsigned char *source<_bp>, unsigned int count<acc> );

#else

/* NOT WORKING YET (needs compiler changes) ... */

extern void __fastcall strcpy( char *destination<_di>, char __far *source<_bp_bank:_bp> );
extern void __fastcall strcat( char *destination<_di>, char __far *source<_bp_bank:_bp> );

extern unsigned int __fastcall strlen( char __far *source<_bp_bank:_bp> );

extern unsigned int __fastcall strlcpy( char *destination<_di>, char __far *source<_bp_bank:_bp>, unsigned char size<acc> );
extern unsigned int __fastcall strlcat( char *destination<_di>, char __far *source<_bp_bank:_bp>, unsigned char size<acc> );

extern void __fastcall memcpy( unsigned char *destination<ram_tii_dst>, unsigned char __far *source<_bp_bank:ram_tii_src>, unsigned int count<acc> );
extern unsigned char * __fastcall mempcpy( unsigned char *destination<ram_tii_dst>, unsigned char __far *source<_bp_bank:ram_tii_src>, unsigned int count<acc> );

extern void __fastcall memset( unsigned char *destination<ram_tii_src>, unsigned char value<_al>, unsigned int count<acc> );

extern int __fastcall strcmp( char *destination<_di>, char __far *source<_bp_bank:_bp> );
extern int __fastcall strncmp( char *destination<_di>, char __far *source<_bp_bank:_bp>, unsigned int count<_ax> );
extern int __fastcall memcmp( unsigned char *destination<_di>, unsigned char __far *source<_bp_bank:_bp>, unsigned int count<_ax> );

#endif

#endif // __HUCC__

#endif // _hucc_string_h
