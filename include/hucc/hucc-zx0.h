#ifndef _hucc_zx0_h
#define _hucc_zx0_h

/****************************************************************************
; ***************************************************************************
;
; hucc-zx0.h
;
; HuC6280 decompressor for Einar Saukas's "classic" ZX0 format.
;
; The code length is 205 bytes for RAM, plus 222 (shorter) or 262 (faster)
; bytes for direct-to-VRAM, plus some generic utility code.
;
; Copyright John Brandwood 2021-2024.
;
; Distributed under the Boost Software License, Version 1.0.
; (See accompanying file LICENSE_1_0.txt or copy at
;  http://www.boost.org/LICENSE_1_0.txt)
;
; ***************************************************************************
; ***************************************************************************
;
; ZX0 "modern" format is not supported, because it costs an extra 4 bytes of
; code in this decompressor, and it runs slower.
;
; Use Emmanuel Marty's SALVADOR ZX0 compressor which can be found here ...
;  https://github.com/emmanuel-marty/salvador
;
; To create a ZX0 file to decompress to RAM
;
;  salvador -classic <infile> <outfile>
;
; To create a ZX0 file to decompress to VRAM, using a 2KB ring-buffer in RAM
;
;  salvador -classic -w 2048 <infile> <outfile>
;
; ***************************************************************************
; **************************************************************************/

// *************
// Functions in unpack-zx0.asm ...
// *************

#ifdef __HUCC__

#asmdef	HUCC_USES_ZX0 1

extern void __fastcall __macro zx0_to_vdc( unsigned int vaddr<_di>, char far *compressed<_bp_bank:_bp> );
extern void __fastcall __macro zx0_to_sgx( unsigned int vaddr<_di>, char far *compressed<_bp_bank:_bp> );

#asm
_zx0_to_vdc.2	.macro
		ldy	<_bp_bank
		call	zx0_to_vdc
		.endm

_zx0_to_sgx.2	.macro
		ldy	<_bp_bank
		call	zx0_to_sgx
		.endm
#endasm

#endif // __HUCC__

#endif // _hucc_zx0_h
