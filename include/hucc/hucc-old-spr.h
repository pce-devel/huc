#ifndef _hucc_old_spr_h
#define _hucc_old_spr_h

/****************************************************************************
; ***************************************************************************
;
; hucc-old-spr.h
;
; Based on the original HuC and MagicKit functions by David Michel and the
; other original HuC developers.
;
; Modifications copyright John Brandwood 2024.
;
; Distributed under the Boost Software License, Version 1.0.
; (See accompanying file LICENSE_1_0.txt or copy at
;  http://www.boost.org/LICENSE_1_0.txt)
;
; ***************************************************************************
; **************************************************************************/

// *************
// Sprite defines ...
// *************

#define FLIP_X_MASK 0x08
#define FLIP_Y_MASK 0x80
#define FLIP_MAS    0x88
#define SIZE_MAS    0x31

#define NO_FLIP     0x00
#define NO_FLIP_X   0x00
#define NO_FLIP_Y   0x00
#define FLIP_X      0x08
#define FLIP_Y      0x80
#define SZ_16x16    0x00
#define SZ_16x32    0x10
#define SZ_16x64    0x30
#define SZ_32x16    0x01
#define SZ_32x32    0x11
#define	SZ_32x64    0x31

// *************
// Functions in hucc-old-spr.asm ...
// *************

#ifdef __HUCC__

#asmdef	HUCC_USES_OLD_SPR 1

extern void __fastcall init_satb( void );
extern void __fastcall reset_satb( void );
extern void __fastcall satb_update( void );
extern void __fastcall spr_set( unsigned char num<acc> );
extern void __fastcall spr_hide( void );
extern void __fastcall spr_show( void );
extern void __fastcall spr_x( unsigned int value<acc> );
extern void __fastcall spr_y( unsigned int value<acc> );
extern void __fastcall spr_pattern( unsigned int vaddr<acc> );
extern void __fastcall spr_ctrl( unsigned char mask<_al>, unsigned char value<acc> );
extern void __fastcall spr_pal( unsigned char palette<acc> );
extern void __fastcall spr_pri( unsigned char priority<acc> );

extern unsigned int __fastcall spr_get_x( void );
extern unsigned int __fastcall spr_get_y( void );

extern void __fastcall sgx_init_satb( void );
extern void __fastcall sgx_reset_satb( void );
extern void __fastcall sgx_satb_update( void );
extern void __fastcall sgx_spr_set( unsigned char num<acc> );
extern void __fastcall sgx_spr_hide( void );
extern void __fastcall sgx_spr_show( void );
extern void __fastcall sgx_spr_x( unsigned int value<acc> );
extern void __fastcall sgx_spr_y( unsigned int value<acc> );
extern void __fastcall sgx_spr_pattern( unsigned int vaddr<acc> );
extern void __fastcall sgx_spr_ctrl( unsigned char mask<_al>, unsigned char value<acc> );
extern void __fastcall sgx_spr_pal( unsigned char palette<acc> );
extern void __fastcall sgx_spr_pri( unsigned char priority<acc> );

extern unsigned int __fastcall sgx_spr_get_x( void );
extern unsigned int __fastcall sgx_spr_get_y( void );

#endif // __HUCC__

#endif // _hucc_old_spr_h
