#ifndef _hucc_gfx_h
#define _hucc_gfx_h

/****************************************************************************
; ***************************************************************************
;
; huc-gfx.h
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
// Screen defines ...
// *************

#define	SCR_SIZE_32x32	0
#define	SCR_SIZE_64x32	1
#define	SCR_SIZE_128x32	2
#define	SCR_SIZE_32x64	4
#define	SCR_SIZE_64x64	5
#define	SCR_SIZE_128x64	6

#define	XRES_SHARP	0
#define	XRES_SOFT	4
#define	XRES_KEEP	128

#define VPC_WIN_A	0x00
#define VPC_WIN_B	0x01
#define	VPC_WIN_AB	0x02
#define	VPC_WIN_NONE	0x03
#define	VPC_NORM	0x00
#define	VPC_SPR		0x04
#define	VPC_INV_SPR	0x08
#define VDC1_ON		0x01
#define	VDC1_OFF	0x00
#define VDC2_ON		0x02
#define	VDC2_OFF	0x00
#define VDC_ON		0x03
#define	VDC_OFF		0x00

// *************
// Functions in hucc-gfx.asm ...
// *************

#ifdef __HUCC__

#asmdef	HUCC_USES_GFX 1

extern void __fastcall set_240x208( void );
extern void __fastcall set_256x224( void );

extern void __fastcall set_screen_size( unsigned char value<_al> );
extern void __fastcall sgx_set_screen_size( unsigned char value<_al> );

extern void __fastcall set_xres( unsigned int x_pixels<_ax>, unsigned char blur_flag<_bl> );
extern void __fastcall sgx_set_xres( unsigned int x_pixels<_ax>, unsigned char blur_flag<_bl> );

extern unsigned int __fastcall __macro vram_addr( unsigned char bat_x<_al>, unsigned char bat_y<_ah> );
extern unsigned int __fastcall __macro sgx_vram_addr( unsigned char bat_x<_al>, unsigned char bat_y<_ah> );

extern unsigned int __fastcall __macro get_vram( unsigned int address<_di> );
extern unsigned int __fastcall __macro sgx_get_vram( unsigned int address<_di> );

extern void __fastcall __macro put_vram( unsigned int address<_di>, unsigned int data<acc> );
extern void __fastcall __macro sgx_put_vram( unsigned int address<_di>, unsigned int data<acc> );

extern void __fastcall load_palette( unsigned char palette<_al>, unsigned char __far *data<_bp_bank:_bp>, unsigned char num_palettes<_ah> );

extern void __fastcall __macro load_vram( unsigned int vram<_di>, unsigned char __far *data<_bp_bank:_bp>, unsigned int num_words<_ax> );
extern void __fastcall __macro load_vram_bank(unsigned int vram<_di>, unsigned int data<__bp>, unsigned char bank<_bp_bank>, unsigned int num_words<__cx>);
extern void __fastcall __macro sgx_load_vram( unsigned int vram<_di>, unsigned char __far *data<_bp_bank:_bp>, unsigned int num_words<_ax> );

extern void __fastcall load_bat( unsigned int vram<_di>, unsigned char __far *data<_bp_bank:_bp>, unsigned char tiles_w<_al>, unsigned char tiles_h<_ah> );
extern void __fastcall sgx_load_bat( unsigned int vram<_di>, unsigned char __far *data<_bp_bank:_bp>, unsigned char tiles_w<_al>, unsigned char tiles_h<_ah> );

extern void __fastcall __macro load_sprites( unsigned int vram<_di>, unsigned char __far *data<_bp_bank:_bp>, unsigned int num_groups<acc> );
extern void __fastcall __macro sgx_load_sprites( unsigned int vram<_di>, unsigned char __far *data<_bp_bank:_bp>, unsigned int num_groups<acc> );

extern void __fastcall set_font_addr( unsigned int vram<acc> );
extern void __fastcall sgx_set_font_addr( unsigned int vram<acc> );

extern void __fastcall set_font_pal( unsigned char palette<acc> );
extern void __fastcall sgx_set_font_pal( unsigned char palette<acc> );

extern void __fastcall load_font( char __far *font<_bp_bank:_bp>, unsigned char count<_al>, unsigned int vram<acc> );
extern void __fastcall sgx_load_font( char __far *font<_bp_bank:_bp>, unsigned char count<_al>, unsigned int vram<acc> );

extern void __fastcall __nop set_font_color( unsigned char foreground<monofont_fg>, unsigned char background<monofont_bg> );

extern void __fastcall __macro load_default_font( void );
extern void __fastcall __macro sgx_load_default_font( void );

extern void __fastcall cls( void );
extern void __fastcall sgx_cls( void );

extern void __fastcall cls( unsigned int tile<acc> );
extern void __fastcall sgx_cls( unsigned int tile<acc> );

extern void __fastcall disp_on( void );
extern void __fastcall disp_off( void );

// extern void __fastcall __nop set_far_base( unsigned char data_bank<_bp_bank>, unsigned char *data_addr<_bp> );
// extern void __fastcall set_far_offset( unsigned int offset<_bp>, unsigned char data_bank<_bp_bank>, unsigned char *data_addr<acc> );

extern void __fastcall far_load_palette( unsigned char palette<_al>, unsigned char num_palettes<_ah> );

extern void __fastcall __macro far_load_vram( unsigned int vram<_di>,  unsigned int num_words<_ax> );
extern void __fastcall __macro sgx_far_load_vram( unsigned int vram<_di>, unsigned int num_words<_ax> );

extern void __fastcall far_load_bat( unsigned int vram<_di>, unsigned char tiles_w<_al>, unsigned char tiles_h<_ah> );
extern void __fastcall sgx_far_load_bat( unsigned int vram<_di>, unsigned char tiles_w<_al>, unsigned char tiles_h<_ah> );

extern void __fastcall __macro far_load_sprites( unsigned int vram<_di>, unsigned int num_groups<acc> );
extern void __fastcall __macro sgx_far_load_sprites( unsigned int vram<_di>, unsigned int num_groups<acc> );

extern void __fastcall far_load_font( unsigned char count<_al>, unsigned int vram<acc> );
extern void __fastcall sgx_far_load_font( unsigned char count<_al>, unsigned int vram<acc> );

extern void __fastcall vram2vram( unsigned int vram_src<_ax>, unsigned int vram_dst<_bx>, unsigned int word_len<_cx> );
extern void __fastcall sgx_vram2vram( unsigned int vram_src<_ax>, unsigned int vram_dst<_bx>, unsigned int word_len<_cx> );

// *************
// Deprecated functions ...
// *************

extern void __fastcall __macro set_xres( unsigned int x_pixels<_ax> );
extern void __fastcall __macro sgx_set_xres( unsigned int x_pixels<_ax> );

extern void __fastcall load_background( unsigned char __far *tiles<_bp_bank:_bp>, unsigned char __far *palettes<__fbank:__fptr>, unsigned char __far *bat<_cl:_bx>, unsigned char w<_dl>, unsigned char h<_dh> );

extern void __fastcall __macro set_bgpal( unsigned char palette<_al>, unsigned char __far *data<_bp_bank:_bp> );
extern void __fastcall __macro set_bgpal( unsigned char palette<_al>, unsigned char __far *data<_bp_bank:_bp>, unsigned int num_palettes<_ah> );
extern void __fastcall __macro set_sprpal( unsigned char palette<_al>, unsigned char __far *data<_bp_bank:_bp> );
extern void __fastcall __macro set_sprpal( unsigned char palette<_al>, unsigned char __far *data<_bp_bank:_bp>, unsigned int num_palettes<_ah> );

extern void __fastcall load_font( char __far *font<_bp_bank:_bp>, unsigned char count<_al> );

extern void __fastcall put_char( unsigned char digit<_bl>, unsigned char bat_x<_dil>, unsigned char bat_y<_dih> );
extern void __fastcall put_digit( unsigned char digit<_bl>, unsigned char bat_x<_dil>, unsigned char bat_y<_dih> );
extern void __fastcall put_hex( unsigned int number<_bx>, unsigned char length<_cl>, unsigned char bat_x<_dil>, unsigned char bat_y<_dih> );
extern void __fastcall put_number( unsigned int number<_bx>, unsigned char length<_cl>, unsigned char bat_x<_dil>, unsigned char bat_y<_dih> );
extern void __fastcall put_raw( unsigned int data<_bx>, unsigned char bat_x<_dil>, unsigned char bat_y<_dih> );
extern void __fastcall put_string( unsigned char *string<_bp>, unsigned char bat_x<_dil>, unsigned char bat_y<_dih> );

#endif // __HUCC__

#endif // _hucc_gfx_h
