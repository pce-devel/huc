; ***************************************************************************
; ***************************************************************************
;
; Conversion script for the Seran Village graphics.
;
; ***************************************************************************
; ***************************************************************************
;
; This is an example script, assembled with PCEAS, that shows how to use its
; graphics conversion capabilities to process multi-layered image files that
; can be created with graphics tools such as Cosmigo's Pro Motion or Grafx2.
;
; Layers are typically used to add collision information, masking sprites or
; event flag/trigger information to map images for a developer's game.
;
; ***************************************************************************
; ***************************************************************************
;
; SYNTAX FOR PCEAS GRAPHICS CONVERSION COMMANDS:
;
;   [label]  .INCPAL  "filename"  [,which-palette [,number-of-palettes]]
;   [label]  .INCCHR  "filename"  [[,x, y] ,w, h] [,optimize]
;   [label]  .INCTILE "filename"  [[,x ,y] ,w ,h] [,optimize]
;   [label]  .INCMASK "filename"  [[,x ,y] ,w ,h] [,optimize]
;   [label]  .INCSPR  "filename"  [[,x ,y] ,w ,h] [,optimize]
;
;   [label]  .INCBAT  "filename" ,vram [[,x ,y] ,w ,h] [,chrset_label]
;   [label]  .INCBLK  "filename" ,vram [[,x ,y] ,w ,h] ,chrset_label
;
;   [label]  .INCMAP  "filename"  [[,x ,y] ,w ,h] ,tileset_label
;   [label]  .FLAGMAP "filename"  [[,x ,y] ,w ,h] ,map_label
;   [label]  .MASKMAP "filename"  [[,x ,y] ,w ,h] ,map_label ,mask_label
;   [label]  .OVERMAP "filename"  [[,x ,y] ,w ,h] ,map_label ,spr_label
;
;   [label]  .PADCHR  chrset_label, to_number_of_characters
;
;   [label]  .SWIZZLE map_label ,w ,h
;
; Note: Labels declared as "extern" in C need to start with an '_' in ASM.
; Note: "[" and "]" mark the start and end of optional parameters.
; Note: "x" and "y" are the starting offset in pixels.
; Note: "w" and "h" are in characters (1/8), tiles (1/16), or blocks (1/16).
; Note: "optimize" is a '0' or '1', use '1' to ignore repeats.
; Note: The case of the commands is unimportant, and the '.' is optional.
;
; ***************************************************************************
;
; SYNTAX FOR PCEAS DATA OUTPUT COMMANDS:
;
;   [label]  .OUTBIN  rom_offset, length [, "filename"]
;   [label]  .OUTZX0  rom_offset, length, window_size [, "filename"]
;
; The label, if present, is set to the size of the output data file.
;
; The rom_offset is not an HuC6280 address, it is an offset into the ROM image
; that PCEAS is building. Use "linear(label)" to get a label's offset.
;
; The ZX0 window_size must either match the ZX0_WINLEN in your hucc_config.inc
; file or be 0 to use no window, ZX0_WINLEN defaults to 2048 if undefined.
;
; If a filename is present then a new file is opened, else the data is written
; to the end of the previous file that was opened by the last either ".OUTBIN"
; or ".OUTZX0".
;
; ***************************************************************************

		.data

; Defined this as a name so that we don't use unexplained numbers later!

OPTIMIZE	=	1

;PAL_SIZE	=	32
;CHR_SIZE	=	32
;SPR_SIZE	=	128



; Extract the 16 frames of the Beach waves color-cycling for palettes 12 and 13.

_waves_pal:	incpal	"../seran/beach-waves-00.png", 12, 2
		incpal	"../seran/beach-waves-01.png", 12, 2
		incpal	"../seran/beach-waves-02.png", 12, 2
		incpal	"../seran/beach-waves-03.png", 12, 2
		incpal	"../seran/beach-waves-04.png", 12, 2
		incpal	"../seran/beach-waves-05.png", 12, 2
		incpal	"../seran/beach-waves-06.png", 12, 2
		incpal	"../seran/beach-waves-07.png", 12, 2
		incpal	"../seran/beach-waves-08.png", 12, 2
		incpal	"../seran/beach-waves-09.png", 12, 2
		incpal	"../seran/beach-waves-10.png", 12, 2
		incpal	"../seran/beach-waves-11.png", 12, 2
		incpal	"../seran/beach-waves-12.png", 12, 2
		incpal	"../seran/beach-waves-13.png", 12, 2
		incpal	"../seran/beach-waves-14.png", 12, 2
		incpal	"../seran/beach-waves-15.png", 12, 2



; Extract the 256-color palette used by both of the map sections in this example.
;
; The "Seran" section uses palettes

_seran_pal:	incpal	"../seran/rpg-west-map.png", 0, 16



; Extract all of the 8x8 characters for the 8 frames of 32x32 pixel water animation.

_seran_water:	incchr	"../seran/seran-water.png", 4, 4*8 ; /* Animated characters. */



; Extract all of the 8x8 characters for the 4 frames of 64x8 pixel shore animation.

_seran_shore:	incchr	"../seran/seran-shore.png", 8, 1*4 ; /* Animated characters. */



; Extract the mask sprites from the bitmap that are going to be used as
; low-priority sprites to push other high-priority sprite pixels behind
; the background layer to create the effect of a 2-layer map while only
; actually using a single map layer.
;
; Only 1 bitplane of the sprite is stored, using only 25% of the memory
; that would be needed for full 16-color sprites.
;
; In this case both Seran and the Beach share the same mask sprites, so
; we're going to add the masks from both the sections of the map layer.

seran_msk:	incmask "../seran/blank.png", 1, 1 ; Make sure 1st mask is blank.
		incmask	"../seran/rpg-west-msk.png", 1024,    0, 64, 64, OPTIMIZE
		incmask	"../seran/rpg-west-msk.png", 1024, 1024, 64, 48, OPTIMIZE

seran_msk_count	=	countof(seran_msk)



; Extract all of the 8x8 characters (a.k.a. tiles) that are used in the
; bitmap, removing the repeated characters.
;
; This uses PCEAS's ability to add multiple images to a character set, which
; requires that they are all put one-after-the-other with no labels or blank
; lines or different commands between the multiple .inchr/.padchr statements.
;
; The characters that we're going to animate must be included somewhere that
; we can find them later on. In this case it is easiest to just include them
; first so that they're at the start of the character set, but that is not a
; a general requirement for background animation to work.

_seran_chr:	incchr	"../seran/seran-water.png", 4, 4 ; 1st frame of the animation.
		incchr	"../seran/seran-shore.png", 8, 1 ; 1st frame of the animation.
		incchr	"../seran/rpg-west-map.png", 1024,    0, 128, 128, OPTIMIZE
		incchr	"../seran/rpg-west-map.png", 1024, 1024, 128,  96, OPTIMIZE

num_seran_chr	=	COUNTOF(_seran_chr)



; Extract 16x16 pixel blocks (a.k.a. meta-tiles) from the bitmap using
; the optimized character set that has already been extracted.
;
; There are a maximum of 256 blocks in a BLK map, and the definitions
; are always 2KByte long and 2KByte aligned in memory.
;
; You need to tell the converter where in VRAM you are going to upload
; the character set.
;
; This uses PCEAS's ability to add multiple images to a set of blocks in
; order to put the two blocks that make up the entrance to each house at
; the beginning of the set of blocks so that they can be detected as BLK
; 0 and 1 in the map when the player tries to enter a building.
;
; This show how to force specific graphics to use specific block numbers
; so the programmer can write their game code to detect them.

seran_blk:	incblk	"../seran/seran-enter.png", 0x1000, _seran_chr ; House entrance.
		incblk	"../seran/rpg-west-map.png", 0x1000, 1024,    0, 64, 64, _seran_chr



; Create a 2nd set of blocks for the "beach" section of the map with the
; expanded set of characters that was made with both the Seran and Beach
; graphics together in a single character set so that the shared graphic
; characters are not repeated in VRAM.

beach_blk:	incblk	"../seran/rpg-west-map.png", 0x1000, 1024, 1024, 64, 48, _seran_chr



; Create two separate maps, with each using their own different set of
; BLK definitions.
;
; The two map sections will be joined together by the multi-screen map
; data structure.
;
; A map can be transformed later on for use as a multi-screen map with
; the ".SWIZZLE" command even if it is larger than the normal limit of
; 128x128 blocks in size.
;
; Note that the map data for a multi-screen map *must* be aligned to a
; 256-byte boundary in memory.

		align	256
seran_map:	incmap	"../seran/rpg-west-map.png", 1024,    0, 64, 64, seran_blk

		align	256
beach_map:	incmap	"../seran/rpg-west-map.png", 1024, 1024, 64, 48, beach_blk



; This example doesn't use the extra .MASKMAP/.OVERMAP lookup table for
; the BLK, but the library routines expect *something* to be specified.
;
; This just gives the code an array of blank data to keep it happy.

dummy_tbl:	ds	256



; Convert the map into BAT-sized chunks so that it can be used as parts
; of a multi-screen map.
;
; This example uses the default BAT size of 64x32 characters instead of
; the "multimap" example's use a 32x32 character BAT, and therefore the
; width and height parameters are 32 and 16 (in terms of 16x16-blocks).
;
; Each resulting BAT-sized "screen" is therefore 512 bytes long (32x16)
; or 256 bytes long (16x16) for the most common BAT sizes, which allows
; for the easy calculation of the offsets to each screen's map data.

		swizzle	seran_map, 32, 16
		swizzle	beach_map, 32, 16



; Define a 2x7 multi-screen map using the 14 BAT-sized chunks that were
; created by the .SWIZZLE command, together with the "SCREEN" macro.
;
; Each screen's data in the multi-screen map tells the map library code
; where to find the map chunk, the BLK data, and the 256-byte MASK/OVER
; table used to associate mask or overlay sprites with the BLK.
;
; The final parameter is not used yet at this time, so set it to zero.
;
; Both the map data and the MASK/OVER table must be 256-byte aligned in
; memory, and BLK definitions must be 2048-byte aligned.
;
; The multi-screen map data structure can hold a limit of 1024 screens,
; and it does not need any particular alignment.
;
; It is stored as an array of map lines from the top to the bottom, and
; each line defining the screens in left to the right order.

_multi_map:	; 1st line of 2 screens, left-to-right, each screen is 32x16=$0200 bytes long.
		SCREEN	seran_map + $0000, seran_blk, dummy_tbl, 0
		SCREEN	seran_map + $0200, seran_blk, dummy_tbl, 0
		; 2nd line of 2 screens, left-to-right, each screen is 32x16=$0200 bytes long.
		SCREEN	seran_map + $0400, seran_blk, dummy_tbl, 0
		SCREEN	seran_map + $0600, seran_blk, dummy_tbl, 0
		; 3rd line of 2 screens, left-to-right, each screen is 32x16=$0200 bytes long.
		SCREEN	seran_map + $0800, seran_blk, dummy_tbl, 0
		SCREEN	seran_map + $0A00, seran_blk, dummy_tbl, 0
		; 4th line of 2 screens, left-to-right, each screen is 32x16=$0200 bytes long.
		SCREEN	seran_map + $0C00, seran_blk, dummy_tbl, 0
		SCREEN	seran_map + $0E00, seran_blk, dummy_tbl, 0

		; 5th line of 2 screens, left-to-right, each screen is 32x16=$0200 bytes long.
		SCREEN	beach_map + $0000, beach_blk, dummy_tbl, 0
		SCREEN	beach_map + $0200, beach_blk, dummy_tbl, 0
		; 6th line of 2 screens, left-to-right, each screen is 32x16=$0200 bytes long.
		SCREEN	beach_map + $0400, beach_blk, dummy_tbl, 0
		SCREEN	beach_map + $0600, beach_blk, dummy_tbl, 0
		; 7th line of 2 screens, left-to-right, each screen is 32x16=$0200 bytes long.
		SCREEN	beach_map + $0800, beach_blk, dummy_tbl, 0
		SCREEN	beach_map + $0A00, beach_blk, dummy_tbl, 0



; Return to the .CODE section before the end of the file!

		.code
