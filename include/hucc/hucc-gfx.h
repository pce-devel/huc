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

extern void __fastcall __xsafe set_256x224( void );
//extern void __fastcall __xsafe set_352x224( void );
//extern void __fastcall __xsafe set_496x224( void );

extern void __fastcall __xsafe set_screen_size( unsigned char value<_al> );
extern void __fastcall __xsafe sgx_set_screen_size( unsigned char value<_al> );

extern void __fastcall __xsafe __macro set_xres( unsigned int x_pixels<_ax> );
extern void __fastcall __xsafe __macro sgx_set_xres( unsigned int x_pixels<_ax> );

extern void __fastcall __xsafe set_xres( unsigned int x_pixels<_ax>, unsigned char blur_flag<_bl> );
extern void __fastcall __xsafe sgx_set_xres( unsigned int x_pixels<_ax>, unsigned char blur_flag<_bl> );

extern unsigned int __fastcall __xsafe __macro vram_addr( unsigned char bat_x<_al>, unsigned char bat_y<_ah> );
extern unsigned int __fastcall __xsafe __macro sgx_vram_addr( unsigned char bat_x<_al>, unsigned char bat_y<_ah> );

extern unsigned int __fastcall __xsafe __macro get_vram( unsigned int address<_di> );
extern unsigned int __fastcall __xsafe __macro sgx_get_vram( unsigned int address<_di> );

extern void __fastcall __xsafe __macro put_vram( unsigned int address<_di>, unsigned int data<acc> );
extern void __fastcall __xsafe __macro sgx_put_vram( unsigned int address<_di>, unsigned int data<acc> );

extern void __fastcall set_tile_address( unsigned int vram<acc> );
extern void __fastcall sgx_set_tile_address( unsigned int vram<acc> );

extern void __fastcall __xsafe __nop set_tile_data( unsigned char __far *tiles<vdc_tile_bank:vdc_tile_addr>, unsigned char num_tiles<vdc_num_tiles>, unsigned char __far *palette_table<vdc_attr_bank:vdc_attr_addr>, unsigned char tile_type<vdc_tile_type> );
extern void __fastcall __xsafe __nop sgx_set_tile_data( unsigned char __far *tiles<sgx_tile_bank:sgx_tile_addr>, unsigned char num_tiles<sgx_num_tiles>, unsigned char __far *palette_table<sgx_attr_bank:sgx_attr_addr>, unsigned char tile_type<sgx_tile_type> );

extern void __fastcall __xsafe __nop set_far_tile_data( unsigned char tile_bank<vdc_tile_bank>, unsigned char *tile_addr<vdc_tile_addr>, unsigned char num_tiles<vdc_num_tiles>, unsigned char palette_table_bank<vdc_attr_bank>, unsigned char *palette_table_addr<vdc_attr_addr>, unsigned char tile_type<vdc_tile_type> );
extern void __fastcall __xsafe __nop sgx_set_far_tile_data( unsigned char tile_bank<vdc_tile_bank>, unsigned char *tile_addr<vdc_tile_addr>, unsigned char num_tiles<vdc_num_tiles>, unsigned char palette_table_bank<vdc_attr_bank>, unsigned char *palette_table_addr<vdc_attr_addr>, unsigned char tile_type<vdc_tile_type> );

extern void __fastcall __xsafe load_vram( unsigned int vram<_di>, unsigned char __far *data<_bp_bank:_bp>, unsigned int num_words<_ax> );
extern void __fastcall __xsafe sgx_load_vram( unsigned int vram<_di>, unsigned char __far *data<_bp_bank:_bp>, unsigned int num_words<_ax> );

extern void __fastcall __xsafe far_load_vram( unsigned int vram<_di>, unsigned char data_bank<_bp_bank>, unsigned char *data_addr<_bp>, unsigned int num_words<_ax> );
extern void __fastcall __xsafe sgx_far_load_vram( unsigned int vram<_di>, unsigned char data_bank<_bp_bank>, unsigned char *data_addr<_bp>, unsigned int num_words<_ax> );

extern void __fastcall __xsafe load_tile( unsigned int vram<_di> );
extern void __fastcall __xsafe sgx_load_tile( unsigned int vram<_di> );

extern void __fastcall __xsafe load_bat( unsigned int vram<_di>, unsigned char __far *data<_bp_bank:_bp>, unsigned char tiles_w<_al>, unsigned char tiles_h<_ah> );
extern void __fastcall __xsafe sgx_load_bat( unsigned int vram<_di>, unsigned char __far *data<_bp_bank:_bp>, unsigned char tiles_w<_al>, unsigned char tiles_h<_ah> );

extern void __fastcall __xsafe far_load_bat( unsigned int vram<_di>, unsigned char data_bank<_bp_bank>, unsigned char *data_addr<_bp>, unsigned char tiles_w<_al>, unsigned char tiles_h<_ah> );
extern void __fastcall __xsafe far_sgx_load_bat( unsigned int vram<_di>, unsigned char data_bank<_bp_bank>, unsigned char *data_addr<_bp>, unsigned char tiles_w<_al>, unsigned char tiles_h<_ah> );

extern void __fastcall __xsafe load_palette( unsigned char palette<_al>, unsigned char __far *data<_bp_bank:_bp>, unsigned char num_palettes<_ah> );

extern void __fastcall __xsafe far_load_palette( unsigned char palette<_al>, unsigned char data_bank<_bp_bank>, unsigned char *data_addr<_bp>, unsigned char num_palettes<_ah> );

extern void __fastcall __xsafe set_font_addr( unsigned int vram<acc> );
extern void __fastcall __xsafe set_font_pal( unsigned char palette<acc> );

extern void __fastcall __xsafe load_font( char __far *font<_bp_bank:_bp>, unsigned char count<_al> );
extern void __fastcall __xsafe load_font( char __far *font<_bp_bank:_bp>, unsigned char count<_al>, unsigned int vram<acc> );

extern void __fastcall __xsafe far_load_font( unsigned char font_bank<_bp_bank>, unsigned char *font_addr<_bp>, unsigned char count<_al>, unsigned int vram<acc> );

extern void __fastcall __xsafe __nop set_font_color( unsigned char foreground<monofont_fg>, unsigned char background<monofont_bg> );
extern void __fastcall __xsafe load_default_font( void );

extern void __fastcall __xsafe put_char( unsigned char digit<_bl>, unsigned char bat_x<_al>, unsigned char bat_y<_ah> );
extern void __fastcall __xsafe put_digit( unsigned char digit<_bl>, unsigned char bat_x<_al>, unsigned char bat_y<_ah> );
extern void __fastcall __xsafe put_hex( unsigned int number<_bx>, unsigned char length<_cl>, unsigned char bat_x<_al>, unsigned char bat_y<_ah> );
extern void __fastcall __xsafe put_number( unsigned int number<_bx>, unsigned char length<_cl>, unsigned char bat_x<_al>, unsigned char bat_y<_ah> );
extern void __fastcall __xsafe put_string( unsigned char *string<_bp>, unsigned char bat_x<_al>, unsigned char bat_y<_ah> );

extern void __fastcall cls( void ); /* NOT __xsafe! */
extern void __fastcall cls( unsigned int tile<acc> ); /* NOT __xsafe! */

extern void __fastcall __xsafe disp_on( void );
extern void __fastcall __xsafe disp_off( void );

extern void __fastcall __xsafe set_tile_data( unsigned char *tile_ex<_di> );

extern void __fastcall __xsafe load_background( unsigned char __far *tiles<_bp_bank:_bp>, unsigned char __far *palettes<__fbank:__fptr>, unsigned char __far *bat<_cl:_bx>, unsigned char w<_dl>, unsigned char h<_dh> );

// Deprecated functions ...

extern void __fastcall __xsafe __macro set_bgpal( unsigned char palette<_al>, unsigned char __far *data<_bp_bank:_bp> );
extern void __fastcall __xsafe __macro set_bgpal( unsigned char palette<_al>, unsigned char __far *data<_bp_bank:_bp>, unsigned int num_palettes<_ah> );
extern void __fastcall __xsafe __macro set_sprpal( unsigned char palette<_al>, unsigned char __far *data<_bp_bank:_bp> );
extern void __fastcall __xsafe __macro set_sprpal( unsigned char palette<_al>, unsigned char __far *data<_bp_bank:_bp>, unsigned int num_palettes<_ah> );

#endif // __HUCC__

#endif // _hucc_gfx_h
