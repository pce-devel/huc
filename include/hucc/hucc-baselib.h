#ifndef _hucc_baselib_h
#define _hucc_baselib_h

/****************************************************************************
; ***************************************************************************
;
; hucc-baselib.h
;
; Basic library functions provided as macros.
;
; Copyright John Brandwood 2024.
;
; Distributed under the Boost Software License, Version 1.0.
; (See accompanying file LICENSE_1_0.txt or copy at
;  http://www.boost.org/LICENSE_1_0.txt)
;
; ***************************************************************************
; **************************************************************************/

// *************
// Joypad defines ...
// *************

#define	JOY_A		0x01
#define	JOY_I		0x01
#define	JOY_B		0x02
#define	JOY_II		0x02
#define	JOY_SLCT	0x04
#define	JOY_SEL		0x04
#define	JOY_STRT	0x08
#define	JOY_RUN		0x08
#define	JOY_UP		0x10
#define	JOY_RGHT	0x20
#define	JOY_RIGHT	0x20
#define	JOY_DOWN	0x40
#define	JOY_LEFT	0x80

#define	JOY_C		0x0100
#define	JOY_III		0x0100
#define	JOY_D		0x0200
#define	JOY_IV		0x0200
#define	JOY_E		0x0400
#define	JOY_V		0x0400
#define	JOY_F		0x0800
#define	JOY_VI		0x0800

#define JOY_SIXBUT	0x8000
#define JOY_TYPE6	0x8000

// *************
// SuperGRAFX VPC settings for set_vpc_ctl() ...
// *************

#define VPC_SPR1_BKG1_SPR2_BKG2	0x3000 // same as SGX_PARALLAX=0
#define VPC_SPR1_SPR2_BKG1_BKG2	0x7000 // same as SGX_PARALLAX=1
#define VPC_BKG1_BKG2_SPR1_SPR2	0xB000

// *************
// System Card variables ...
// *************

extern unsigned int si;
extern unsigned int di;
extern unsigned int bp;

extern unsigned int ax;
extern unsigned int bx;
extern unsigned int cx;
extern unsigned int dx;

extern unsigned char al;
extern unsigned char ah;
extern unsigned char bl;
extern unsigned char bh;
extern unsigned char cl;
extern unsigned char ch;
extern unsigned char dl;
extern unsigned char dh;

extern unsigned char irq_cnt;
extern unsigned char joynow[5];
extern unsigned char joytrg[5];
extern unsigned char joy6now[5];
extern unsigned char joy6trg[5];
extern unsigned int  bg_x1;
extern unsigned int  bg_y1;
extern unsigned int  bg_x2;
extern unsigned int  bg_y2;

// *************
// Special macros to get information from PCEAS ...
// *************

#define BANK( datasym ) ((unsigned) (&__bank__ ## datasym))
#define SIZEOF( datasym ) ((unsigned) (&__sizeof__ ## datasym))
#define COUNTOF( datasym ) ((unsigned) (&__countof__ ## datasym))
#define OVERLAY( datasym ) ((unsigned) (&__overlay__ ## datasym))


// *************
// Functions in hucc-baselib.asm ...
// *************

#ifdef __HUCC__

#asmdef	HUCC_USES_BASELIB 1

#define	_OPTIMIZE 1

// *************
// Hardware Detection
// *************

extern unsigned char __fastcall __macro sgx_detect( void );
extern unsigned char __fastcall __macro ac_exists( void );


// *************
// Memory Access
// *************

extern unsigned char __fastcall __macro peek( unsigned int addr<__ptr> );
extern unsigned int  __fastcall __macro peekw( unsigned int addr<__ptr> );

extern void __fastcall __macro poke( unsigned int addr<__poke>, unsigned char with<acc> );
extern void __fastcall __macro pokew( unsigned int addr<__poke>, unsigned int with<acc> );

extern unsigned char __fastcall farpeek( void __far *addr<_bp_bank:_bp> );
extern unsigned int  __fastcall farpeekw( void __far *addr<_bp_bank:_bp> );

extern void __fastcall __nop set_far_base( unsigned char data_bank<_bp_bank>, unsigned char *data_addr<_bp> );
extern void __fastcall set_far_offset( unsigned int offset<_bp>, unsigned char data_bank<_bp_bank>, unsigned char *data_addr<acc> );

extern unsigned char __fastcall far_peek( void );
extern unsigned int  __fastcall far_peekw( void );


// *************
// Clock Functions
// *************

extern unsigned char __fastcall __macro clock_hh( void );
extern unsigned char __fastcall __macro clock_mm( void );
extern unsigned char __fastcall __macro clock_ss( void );
extern unsigned char __fastcall __macro clock_tt( void );
extern void __fastcall __macro clock_reset( void );


// *************
// Joypad Functions
// *************

extern unsigned int __fastcall __macro joy( unsigned char which<acc> );
extern unsigned int __fastcall __macro joytrg( unsigned char which<acc> );


// *************
// Number Functions
// *************

extern int __fastcall abs( int value<acc> );

extern void __fastcall __macro srand( unsigned char seed<acc> );
extern unsigned int __fastcall rand( void );
extern unsigned char __fastcall rand8( void );

// Note: "limit" is 0..255.
extern unsigned char __fastcall random8( unsigned char limit<acc> );

// Note: "limit" is 0..128, 129..255 are treated as 128!
extern unsigned char __fastcall random( unsigned char limit<acc> );


// *************
// Overlay Execution
// *************

extern unsigned char __fastcall __macro cd_execoverlay( unsigned char ovl_index<acc> );


// *************
// Functions that are only optionally available if configured in your hucc-config.inc
// *************

extern unsigned int __fastcall __macro joybuf( unsigned char which<acc> );
extern unsigned int __fastcall __macro get_joy_events( unsigned char which<acc> );
extern void __fastcall __macro clear_joy_events( unsigned char mask<acc> );


// *************
// Functions that are only implemented in the TGEMU emulator for unit-testing
// the compiler and which should never be used in normal HuCC projects ...
// *************

extern void __fastcall dump_screen( void );
extern void __fastcall abort( void );
extern void __fastcall exit( int value<acc> );

extern unsigned char __fastcall __builtin_ffs( unsigned int value<__temp> );

#endif // __HUCC__

#endif // _hucc_baselib_h
