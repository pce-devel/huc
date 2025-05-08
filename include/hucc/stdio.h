#ifndef _hucc_stdio_h
#define _hucc_stdio_h

/****************************************************************************
; ***************************************************************************
;
; stdio.h
;
; A small and simple printf() and sprintf() implementation for HuCC.
;
; Copyright John Brandwood 2019-2025.
;
; Distributed under the Boost Software License, Version 1.0.
; (See accompanying file LICENSE_1_0.txt or copy at
;  http://www.boost.org/LICENSE_1_0.txt)
;
; ***************************************************************************
; ***************************************************************************
;
; WARNING: This only supports a subset of POSIX printf()/sprintf() behavior!
; WARNING: <width>, <precision> and numbers in ESC-sequences must be <= 127!
; WARNING: This only supports a maximum of 4 data arguments!
; WARNING: sprintf() must output less than 256 characters!
;
; ***************************************************************************
; ***************************************************************************
;
; A format specifier follows this prototype:
;
;   '%' [<flags>] [<width>] ['.' <precision>] [<length>] <specifier>
;
; <flags>
;
;   ('-' | '0' | '+' | ' ') = one or more of the following ...
;
;    '-' to left-justify output within the <width>
;    '0' to pad to <width> with zero instead of space
;    '+' to prepend positive integers with a '+' when "%d", "%i" or "%u"
;    ' ' to prepend positive integers with a ' ' when "%d", "%i" or "%u"
;
; <width>
;
;   (<decimal-number> | '*') = "1..127" or read value from an argument
;
; <precision>
;
;   (<decimal-number> | '*') = "1..127" or read value from an argument
;
; <length>
;
;   ('l') = only "%x" and "%X" : read two 16-bit arguments as a 32-bit value
;
; <specifier>
;
;   '%' = just print a '%'
;   'c' = print an ASCII character
;   'd' = print a signed integer variable in decimal
;   'i' = print a signed integer variable in decimal
;   's' = print string, stop at <precision> characters
;   'u' = print an unsigned integer variable in decimal
;   'x' = print an unsigned binary (or BCD) variable in hex
;   'X' = print an unsigned binary (or BCD) variable in hex
;
; Note:
;
;   HuCC follows GCC's behavior, except for "%u" with a '+' ...
;
;     If both '+' and ' ' flags are used, then '+' overrides the ' ' flag.
;     "%u" is unsigned-by-definition and treats '+' as ' ' (GCC ignores '+').
;     "%x", "%X", "%c" and "%s" ignore the '+' and ' ' flags.
;
; ***************************************************************************
;
; printf() and puts() output accept virtual-terminal control codes:
;
; <control-code>
;
;   '\n' = move cursor-X to XL, cursor-Y down 1 line.
;   '\r' = move cursor-X to XL.
;   '\f' = clear-screen + home cursor-X and cursor-Y to (XL, YT)
;
; ***************************************************************************
;
; printf() and puts() output accept virtual-terminal escape sequences:
;
;   ('\e' | '\x1B') <decimal-number> <escape-code>
;
; <escape-code>
;
;   'X' = set current X coordinate (0..127)
;   'Y' = set current Y coordinate (0..63)
;   'L' = set minimum X coordinate (0..127)
;   'T' = set minimum Y coordinate (0..63)
;   'P' = set current palette (0..15)
;
; ***************************************************************************
; **************************************************************************/

#ifndef NULL
#define NULL (0)
#endif

// *************
// TTY-related variables ...
// *************

extern unsigned char vdc_tty_x_lhs;
extern unsigned char vdc_tty_y_top;
extern unsigned char vdc_tty_x;
extern unsigned char vdc_tty_y;

extern unsigned char sgx_tty_x_lhs;
extern unsigned char sgx_tty_y_top;
extern unsigned char sgx_tty_x;
extern unsigned char sgx_tty_y;

// *************
// Functions in hucc-printf.asm ...
// *************

#ifdef __HUCC__

#asmdef	HUCC_USES_STDIO 1

extern void __fastcall __macro putchar( unsigned char ascii<acc> );
extern void __fastcall __macro sgx_putchar( unsigned char ascii<acc> );

extern void __fastcall __macro puts( unsigned char *string<_bp> );
extern void __fastcall __macro sgx_puts( unsigned char *string<_bp> );

// This is HuCC/HuC's way of simulating varargs!

extern int __fastcall sprintf( unsigned char *string<_di>, unsigned char *format<_bp> );
extern int __fastcall sprintf( unsigned char *string<_di>, unsigned char *format<_bp>, unsigned int vararg1<__vararg1> );
extern int __fastcall sprintf( unsigned char *string<_di>, unsigned char *format<_bp>, unsigned int vararg1<__vararg1>, unsigned int vararg2<__vararg2> );
extern int __fastcall sprintf( unsigned char *string<_di>, unsigned char *format<_bp>, unsigned int vararg1<__vararg1>, unsigned int vararg2<__vararg2>, unsigned int vararg3<__vararg3> );
extern int __fastcall sprintf( unsigned char *string<_di>, unsigned char *format<_bp>, unsigned int vararg1<__vararg1>, unsigned int vararg2<__vararg2>, unsigned int vararg3<__vararg3>, unsigned int vararg4<__vararg3> );

extern int __fastcall printf( unsigned char *format<_bp> );
extern int __fastcall printf( unsigned char *format<_bp>, unsigned int vararg1<__vararg1> );
extern int __fastcall printf( unsigned char *format<_bp>, unsigned int vararg1<__vararg1>, unsigned int vararg2<__vararg2> );
extern int __fastcall printf( unsigned char *format<_bp>, unsigned int vararg1<__vararg1>, unsigned int vararg2<__vararg2>, unsigned int vararg3<__vararg3> );
extern int __fastcall printf( unsigned char *format<_bp>, unsigned int vararg1<__vararg1>, unsigned int vararg2<__vararg2>, unsigned int vararg3<__vararg3>, unsigned int vararg4<__vararg3> );

extern int __fastcall sgx_printf( unsigned char *format<_bp> );
extern int __fastcall sgx_printf( unsigned char *format<_bp>, unsigned int vararg1<__vararg1> );
extern int __fastcall sgx_printf( unsigned char *format<_bp>, unsigned int vararg1<__vararg1>, unsigned int vararg2<__vararg2> );
extern int __fastcall sgx_printf( unsigned char *format<_bp>, unsigned int vararg1<__vararg1>, unsigned int vararg2<__vararg2>, unsigned int vararg3<__vararg3> );
extern int __fastcall sgx_printf( unsigned char *format<_bp>, unsigned int vararg1<__vararg1>, unsigned int vararg2<__vararg2>, unsigned int vararg3<__vararg3>, unsigned int vararg4<__vararg3> );

#asm
		include	"hucc-printf.asm"
#endasm

#endif // __HUCC__

#endif // _hucc_stdio_h
