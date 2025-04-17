#ifndef _hucc_metamap_h
#define _hucc_metamap_h

/****************************************************************************
; ***************************************************************************
;
; hucc-metamap.h
;
; A map system based on 16x16 meta-tiles (aka "blocks", aka CHR/BLK/MAP).
;
; Copyright John Brandwood 2025.
;
; Distributed under the Boost Software License, Version 1.0.
; (See accompanying file LICENSE_1_0.txt or copy at
;  http://www.boost.org/LICENSE_1_0.txt)
;
; ***************************************************************************
; ***************************************************************************
;
; The maximum X and Y size for metatile maps is 128 tiles (2048 pixels).
;
; The maximum total size for a metamap is 16KBytes, which allows for maps up
; to 128x128 tiles (2048x2048 pixels).
;
; Huge multi-screen metatile maps are also supported (optionally).
;
; The maximum X and Y for multi-screen maps is 128 screens (32768 pixels).
;
; The maximum total size for a multi-screen map is 8KBytes, which allows for
; a total of 1024 screens.
;
; ***************************************************************************
; **************************************************************************/

// *************
// Functions in metamap.asm ...
// *************

#ifdef __HUCC__

#asmdef	HUCC_USES_METAMAP 1

// Current top-left of drawn map in pixels.
extern unsigned int   vdc_map_pxl_x;
extern unsigned int   vdc_map_pxl_y;

// Width and height to draw/scroll in characters (not tiles).
extern unsigned char  vdc_map_draw_w;
extern unsigned char  vdc_map_draw_h;

// Width of map (or multi-screen BAT) in tiles.
extern unsigned char  vdc_map_line_w;

// Width of map in screens (if multi-screen map).
extern unsigned char  vdc_map_scrn_w;

extern unsigned char  vdc_scr_bank;
extern unsigned char *vdc_scr_addr;

extern unsigned char  vdc_map_bank;
extern unsigned char *vdc_map_addr;

extern unsigned char  vdc_blk_bank;
extern unsigned char *vdc_blk_addr;

extern unsigned char  vdc_flg_bank;
extern unsigned char *vdc_flg_addr;

extern void __fastcall set_metatiles( unsigned char __far *tiledef<vdc_blk_bank:vdc_blk_addr>, unsigned char __far *flagdef<vdc_flg_bank:vdc_flg_addr>, unsigned char number_of_tiles<_al> );
extern void __fastcall __macro set_metamap( unsigned char __far *metamap<vdc_map_bank:vdc_map_addr>, unsigned char tiles_w<vdc_map_line_w> );
extern void __fastcall __macro set_multimap( unsigned char __far *multimap<vdc_scr_bank:vdc_scr_addr>, unsigned char screens_w<vdc_map_scrn_w> );

extern void __fastcall draw_map( void );
extern void __fastcall scroll_map( void );

extern void __fastcall blit_map( unsigned char tile_x<map_bat_x>, unsigned char tile_y<map_bat_y>, unsigned char tile_w<map_draw_x>, unsigned char tile_h<map_draw_y> );

#asm
		.macro	_set_metamap.2
		stz	vdc_scr_bank
		.endm

		.macro	_set_multimap.2
		lda	vdc_bat_width
		lsr	a
		sta	vdc_map_line_w
		.endm
#endasm

#ifdef _SGX

// Current top-left of drawn map in pixels.
extern unsigned int   sgx_map_pxl_x;
extern unsigned int   sgx_map_pxl_y;

// Width and height to draw/scroll in characters (not tiles).
extern unsigned char  sgx_map_draw_w;
extern unsigned char  sgx_map_draw_h;

// Width of map (or multi-screen BAT) in tiles.
extern unsigned char  sgx_map_line_w;

// Width of map in screens (if multi-screen map).
extern unsigned char  sgx_map_scrn_w;

extern unsigned char  sgx_scr_bank;
extern unsigned char *sgx_scr_addr;

extern unsigned char  sgx_map_bank;
extern unsigned char *sgx_map_addr;

extern unsigned char  sgx_blk_bank;
extern unsigned char *sgx_blk_addr;

extern unsigned char  sgx_flg_bank;
extern unsigned char *sgx_flg_addr;

extern void __fastcall sgx_set_metatiles( unsigned char __far *tiledef<sgx_blk_bank:sgx_blk_addr>, unsigned char __far *flagdef<sgx_flg_bank:sgx_flg_addr>, unsigned char number_of_tiles<_al> );
extern void __fastcall __macro sgx_set_metamap( unsigned char __far *metamap<sgx_map_bank:sgx_map_addr>, unsigned char tiles_w<sgx_map_line_w> );
extern void __fastcall __macro sgx_set_multimap( unsigned char __far *multimap<sgx_scr_bank:sgx_scr_addr>, unsigned char screens_w<sgx_map_scrn_w> );

extern void __fastcall sgx_init_metatiles( unsigned char number_of_tiles<_al> );

extern void __fastcall sgx_draw_map( void );
extern void __fastcall sgx_scroll_map( void );

extern void __fastcall sgx_blit_map( unsigned char tile_x<map_bat_x>, unsigned char tile_y<map_bat_y>, unsigned char tile_w<map_draw_x>, unsigned char tile_h<map_draw_y> );

#asm
		.macro	_sgx_set_metamap.2
		stz	sgx_scr_bank
		.endm

		.macro	_sgx_set_multimap.2
		lda	sgx_bat_width
		lsr	a
		sta	sgx_map_line_w
		.endm
#endasm

#endif

#endif // __HUCC__

#endif // _hucc_metamap_h
