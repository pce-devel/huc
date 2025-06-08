#ifndef _hucc_blkmap_h
#define _hucc_blkmap_h

/****************************************************************************
; ***************************************************************************
;
; hucc-blkmap.h
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
; The maximum X and Y size for blkmaps is 128 blocks (2048 pixels).
;
; The maximum total size for a blkmap is 16KBytes, which allows for maps up
; to 128x128 blocks (2048x2048 pixels).
;
; Huge multi-screen blkmaps are also supported (optionally).
;
; The maximum X and Y for multi-screen maps is 128 screens (32768 pixels).
;
; The maximum total size for a multi-screen map is 8KBytes, which allows for
; a total of 1024 screens.
;
; ***************************************************************************
; **************************************************************************/

// *************
// Functions in blkmap.asm ...
// *************

#ifdef __HUCC__

#asmdef	HUCC_USES_BLKMAP 1

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

// Extra information returned by get_map_block() and sgx_get_map_block(),
// which are the collision flags and the mask/overlay sprite for the BLK.
extern unsigned char  map_blk_flag;
extern unsigned char  map_blk_mask;

extern unsigned char  vdc_scr_bank;
extern unsigned char *vdc_scr_addr;

extern unsigned char  vdc_map_bank;
extern unsigned char *vdc_map_addr;

extern unsigned char  vdc_blk_bank;
extern unsigned char *vdc_blk_addr;

extern unsigned char  vdc_tbl_bank;
extern unsigned char *vdc_tbl_addr;

extern void __fastcall set_blocks( unsigned char __far *blk_def<vdc_blk_bank:vdc_blk_addr>, unsigned char __far *flg_def<vdc_tbl_bank:vdc_tbl_addr>, unsigned char number_of_blocks<_al> );
extern void __fastcall __macro set_blkmap( unsigned char __far *blk_map<vdc_map_bank:vdc_map_addr>, unsigned char blocks_w<vdc_map_line_w> );
extern void __fastcall __macro set_multimap( unsigned char __far *multi_map<vdc_scr_bank:vdc_scr_addr>, unsigned char screens_w<vdc_map_scrn_w> );

extern void __fastcall draw_map( void );
extern void __fastcall scroll_map( void );

extern void __fastcall blit_map( unsigned char tile_x<map_bat_x>, unsigned char tile_y<map_bat_y>, unsigned char tile_w<map_draw_x>, unsigned char tile_h<map_draw_y> );

extern unsigned char __fastcall get_map_block( unsigned int x<map_pxl_x>, unsigned int y<map_pxl_y> );

#asm
		.macro	_set_blkmap.2
		stz	vdc_scr_bank
		.endm

		.macro	_set_multimap.2
		lda	vdc_bat_width
		lsr	a
		sta	vdc_map_line_w
		.endm

		.macro	SCREEN
		db	(((\1) & $1FFF) | $6000) >> 8, bank(\1)
		db	(((\2) & $1FFF) | $4000) >> 8, bank(\2)
		db	(((\3) & $1FFF) | $6000) >> 8, bank(\3)
		dw	(\4)
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

extern unsigned char  sgx_tbl_bank;
extern unsigned char *sgx_tbl_addr;

extern void __fastcall sgx_set_blocks( unsigned char __far *blk_def<sgx_blk_bank:sgx_blk_addr>, unsigned char __far *flg_def<sgx_tbl_bank:sgx_tbl_addr>, unsigned char number_of_blocks<_al> );
extern void __fastcall __macro sgx_set_blkmap( unsigned char __far *blk_map<sgx_map_bank:sgx_map_addr>, unsigned char blocks_w<sgx_map_line_w> );
extern void __fastcall __macro sgx_set_multimap( unsigned char __far *multi_map<sgx_scr_bank:sgx_scr_addr>, unsigned char screens_w<sgx_map_scrn_w> );

extern void __fastcall sgx_draw_map( void );
extern void __fastcall sgx_scroll_map( void );

extern void __fastcall sgx_blit_map( unsigned char tile_x<map_bat_x>, unsigned char tile_y<map_bat_y>, unsigned char tile_w<map_draw_x>, unsigned char tile_h<map_draw_y> );

extern unsigned char __fastcall sgx_get_map_block( unsigned int x<map_pxl_x>, unsigned int y<map_pxl_y> );

#asm
		.macro	_sgx_set_blkmap.2
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

#endif // _hucc_blkmap_h
