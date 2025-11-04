# HuCC Function Reference

## **Memory Access Functions**

`peek( unsigned int addr );`
`peekw( unsigned int addr );`
Reads the contents of memory location '*addr*'. `peek()` is char-sized access, whereas `peekw()` is word-sized.

`poke( unsigned int addr, unsigned char with );`
`pokew( unsigned int addr, unsigned int with );`
Writes '*val*' value at memory location '*addr*'. `poke()` is char-sized access, whereas `pokew()` is word-sized. 
This function can be used to access the hardware I/O ports located at 0x0000 to 0x1FFF.

`farpeek( void __far *addr );`
`farpeekw( void __far *addr );`
Reads the contents of far memory location '*addr*'. `farpeek()` is char-sized access, whereas `farpeekw()` is word-sized.

**Note**: This legacy function has been superseded by the newer `set_far_base()` function.

`set_far_base( unsigned char data_bank, unsigned char *data_addr );`
Sets the base address for far memory operations. Used in conjunction with far memory functions like `far_load_vram()`, `far_load_bat()`, etc.

`set_far_offset( unsigned int offset, unsigned char data_bank, unsigned char *data_addr );`
Sets the offset for far memory operations. Allows fine-tuning of the far memory pointer.

`far_peek( void );`
Reads a byte from the current far memory location set by `set_far_base()`.

`far_peekw( void );`
Reads a word from the current far memory location set by `set_far_base()`.

**Related functions:** `far_load_vram()`, `far_load_bat()`, `far_load_sprites()`, `far_load_palette()`, `far_load_font()`

## **Video Functions**

`disp_on( void );`
Enables display output. Controls both Video Display Controllers (VDC1 and VDC2).

`disp_off( void );`
Blanks/turns off the display output. Controls both VDC1 and VDC2.

`vsync( void );`
`vsync( unsigned char count );`
Synchronizes the program to the video Vertical Blanking Signal (VBL), which is around 1/60th of a second. Controls both VDC1 and VDC2.

- **Overload 1**: Without parameters, returns as soon as a VBL signal has been received.
- **Overload 2**: With '*count*' parameter, synchronizes to the number of frames requested (e.g. `vsync(3)` for 20fps).

`cls( void );`
`cls( unsigned int tile );`
Clears the entire screen.
- **Overload 1**: Without parameters, the screen is filled with a space character.
- **Overload 2**: With '*tile*' parameter, the screen is filled with the specified BAT value.

`init_240x208( void );`
Initializes the screen to 240x208 resolution (30 characters wide by 26 characters tall). Controls both VDC1 and VDC2. This is a useful resolution for games that need to scroll their map on a virtual screen restricted to 256x256 pixels.

**Note**: Because of a VDC limitation, a screen with a horizontal resolution of 240 pixels can not display more than 62 sprites. The last two sprites (numbers 62 & 63) will not be visible.

`init_256x224( void );`
Initializes the screen to a standard 256x224 resolution (32 characters wide by 28 characters tall). Controls both VDC1 and VDC2.

`set_xres( unsigned int x_pixels );`
`set_xres( unsigned int x_pixels, unsigned char blur_flag );`
Sets the horizontal resolution to a custom '*x_pixels*' value (in pixels). This changes the VDC's registers to display more pixels on the screen; it does not affect any virtual calculations.

- **Overload 1**: Uses default blur setting `XRES_SOFT`.
- **Overload 2**: With '*blur_flag*' parameter, specifies smoothing type `XRES_SHARP` or `XRES_SOFT` (default).

**Note 1**: The three regular (non-overscan) horizontal resolutions are 256 pixels (5MHz), 336 pixels (7MHz) and 512 pixels (10MHz). The 5MHz dot clock will be used up to a horizontal resolution of 296 pixels. The 7MHz dot clock will be used up to 384 pixels and the 10MHz dot clock will be used beyond this. The maximum usable (overscan) resolution seems to be around 528 pixels.

**Note 2**: The '*blur_flag*' parameter only works with a composite output.

**Note 3**: Because of a hardware limitation, a screen with a horizontal resolution of 240 pixels can not display more than 62 sprites. The last two sprites (numbers 62 & 63) will not be visible.

`set_screen_size( unsigned char value );`
Changes the virtual screen size. By default the startup code initializes a virtual screen of 64 characters wide and 32 characters tall, but other values are possible, namely: 32x32, 128x32, 32x64, 64x64, or 128x64. The larger the virtual screen is, the less VRAM you will have for your graphics (fonts, tiles, sprites).

`vram_addr( unsigned char bat_x, unsigned char bat_y );`     
Returns the VRAM address for the character at position ('*bat_x*', '*bat_y*').

`get_vram( unsigned int address );`
Reads a 16-bit word from VRAM at the specified address.

`put_vram( unsigned int address, unsigned int data );`
Writes a 16-bit word to VRAM at the specified address.

**Note:** These functions provide direct VRAM access for low-level operations. For most graphics operations, use the higher-level functions like `load_vram()`, `load_bat()`, etc.

`load_vram( unsigned int vram, unsigned char __far *data, unsigned int num_words );`
Generic function to load data (BAT, tiles, sprites) in VRAM, at address '*vram*'. '*num_words*' is the number of 16-bit words to load.

`far_load_vram( unsigned int vram, unsigned int num_words );`
Loads VRAM data from far memory. The data source must be set up using `set_far_base()` before calling this function.

`load_bat( unsigned int vram, unsigned char __far *data, unsigned char tiles_w, unsigned char tiles_h );`
Loads a rectangular character attribute table (BAT) of width '*w*' and of height '*h*' in VRAM at address '*vram*'.

`far_load_bat( unsigned int vram, unsigned char tiles_w, unsigned char tiles_h );`
Loads BAT data from far memory. The data source must be set up using `set_far_base()` before calling this function.

`load_sprites( unsigned int vram, unsigned char __far *data, unsigned int num_groups );`
Loads sprite pattern data to VRAM. '*num_groups*' specifies the number of 16x16 sprite groups to load.

`far_load_sprites( unsigned int vram, unsigned int num_groups );`
Loads sprite data from far memory. The data source must be set up using `set_far_base()` before calling this function.

`vram2vram( unsigned int vram_dst, unsigned int vram_src, unsigned int word_len );`
Copies data from one VRAM location to another using Direct Memory Access. Useful for fast sprite animations and tile updates. '*vram_dst*' is the destination address, '*vram_src*' is the source address, and '*word_len*' is the number of 16-bit words to copy.

#### **SuperGrafx Video Functions**

These functions control the additional VDC (VDC2) provided by the SuperGrafx.

`sgx_detect( void );`
Checks for SuperGrafx hardware. Returns **TRUE (1)** if exists; **FALSE(0)** if not.

`sgx_cls( void );`
`sgx_cls( unsigned int tile );`

`sgx_set_xres( unsigned int x_pixels, unsigned char blur_flag );`
Be careful to use the same resolution as `set_xres()`, or the VDC2 layer will be corrupted!

`sgx_set_screen_size( unsigned char value );`

`sgx_vram_addr( unsigned char bat_x, unsigned char bat_y );`

`sgx_get_vram( unsigned int address );`

`sgx_put_vram( unsigned int address, unsigned int data );`

`sgx_load_vram( unsigned int vram, unsigned char __far *data, unsigned int num_words );`

`sgx_far_load_vram( unsigned int vram, unsigned int num_words );`

`sgx_load_bat( unsigned int vram, unsigned char __far *data, unsigned char tiles_w, unsigned char tiles_h );`

`sgx_far_load_bat( unsigned int vram, unsigned char tiles_w, unsigned char tiles_h );`

`sgx_load_sprites( unsigned int vram, unsigned char __far *data, unsigned int num_groups );`

`sgx_far_load_sprites( unsigned int vram, unsigned int num_groups );`

`sgx_vram2vram( unsigned int vram_dst, unsigned int vram_src, unsigned int word_len );`

`vpc_set_ctl( unsigned int bits );`
Sets the control register of the SuperGrafx VPC (Video Priority Controller). Controls sprite and background priority settings between VDC1 and VDC2. The '*bits*' value can take one of the 3 parameters below.

**Priority Modes:**
- `VPC_SPR1_BKG1_SPR2_BKG2` - Mode 1
- `VPC_SPR1_SPR2_BKG1_BKG2` - Mode 2 (default)
- `VPC_BKG1_BKG2_SPR1_SPR2` - Mode 3

`vpc_set_win1( unsigned int width );`
Sets VPC window 1 width for split screen effects. Default '*width*' is 0x0000.

`vpc_set_win2( unsigned int width );`
Sets VPC window 2 width for split screen effects. Default '*width*' is 0x0000.

## **Split Screen and Scrolling Functions**

`scroll_split( unsigned char index, unsigned char screen_line, unsigned int bat_x, unsigned int bat_y, unsigned char display_flags );`
Creates a split screen window at the specified screen line. Up to 128 windows can be defined for VDC1. '*index*' is the split index (0-127), '*screen_line*' is where the split occurs, '*bat_x*' and '*bat_y*' are the background position, and the '*display_flags*' control what is displayed (BKG_ON, SPR_ON, BKG_OFF, SPR_OFF).

**Example:**
```c
// Create a status bar at line 20 with background only
scroll_split(0, 20, 0, 0, BKG_ON);

// Create a split at line 100 showing both background and sprites
scroll_split(1, 100, 32, 16, BKG_ON | SPR_ON);
```

`disable_split( unsigned char index );`
Disables the split screen window specified by '*index*'.

`disable_all_splits( void );`
Disables all active split screen windows.

`scroll( unsigned char num, unsigned int x, unsigned int y, unsigned char top, unsigned char bottom, unsigned char disp );`
Defines screen window '*num*'. Up to 4 windows can be defined. '*top*' and '*bottom*' are the screen top and bottom limits of the window (limits are included in the window area). '*disp*' controls the type of the window. If bit 7 is set, background graphics will be displayed in this window; and if bit 6 is set, sprites will also be displayed. If none of these bits are set, the window will stay blank. '*x*' and '*y*' are the top-left coordinates of the area in the virtual screen that will be displayed in the window.

**Note**: This legacy function has been superseded by the superior `scroll_split()` function.

`scroll_disable( unsigned char num );`
Disables scrolling for the screen window '*num*'. Only use it with the legacy `scroll()` function!

#### **SuperGrafx Split Screen Functions**

`sgx_scroll_split( unsigned char index, unsigned char screen_line, unsigned int bat_x, unsigned int bat_y, unsigned char display_flags );`
Up to 128 windows can be defined for VDC2.

`sgx_disable_split( unsigned char index );`

`sgx_disable_all_splits( void );`

## **Sprite Functions**

`init_satb( void );`
Initializes the internal Sprite Attribute Table (SATB) used by the HuCC library to manage sprites. This function must be called before any other sprite function is called.

**Related functions:** `reset_satb()`, `satb_update()`, `spr_set()`, `spr_hide()`, `spr_show()`

`reset_satb( void );`
Resets the SAT, this has the effect to disable and reset all the sprites.

`satb_update( void );`
Updates the SAT in VRAM. This will refresh sprites on the screen. Use this function regularly to update the sprite display. The best place to call `satb_update()` is after every `vsync()` call, but no need to call `satb_update()` if you didn't change any sprite attribute. '*nb*' specifies the number of sprites to refresh, starting from sprite 0. By default the library refreshes only the sprites you use; but if you need to explicitely refresh a certain number of sprites, then you can use '*nb*'.

`spr_set( unsigned char num );`
Selects sprite '*num*' (0-63) as the current sprite.

`spr_hide( void );`
Without parameters, this function will hide the current sprite. Use '*num*' to hide a different sprite than the current one.

`spr_show( void );`
Shows a sprite that has been hidden using the `spr_hide()` function.

`spr_x( unsigned int value );`
Sets the '*x*' coordinate of the current sprite. Negative values will make the sprite disappear under the left border, while values higher than the screen width will make it disappear under the right border.

`spr_y( unsigned int value );`
Sets the '*y*' coordinate of the current sprite. Negative values will make the sprite disappear under the top border, while values higher than the screen height will make it disappear under the bottom border.

`spr_pattern( unsigned int vaddr );`
Sets the pattern address in VRAM of the current sprite.

`spr_ctrl( unsigned char mask, unsigned char value );`
Sets different attributes of the current sprite using bitwise operations.

**Sprite Control Masks:**
- `SIZE_MAS` (0x31) - Size mask for `spr_ctrl()`
- `FLIP_MAS` (0x88) - Flip mask for `spr_ctrl()`
- `FLIP_X_MASK` (0x08) - Horizontal flip mask
- `FLIP_Y_MASK` (0x80) - Vertical flip mask

**Sprite Size:**
- `SZ_16x16` (0x00) - 16x16 pixels
- `SZ_16x32` (0x10) - 16x32 pixels  
- `SZ_16x64` (0x30) - 16x64 pixels
- `SZ_32x16` (0x01) - 32x16 pixels
- `SZ_32x32` (0x11) - 32x32 pixels
- `SZ_32x64` (0x31) - 32x64 pixels

**Sprite Flipping:**
- `NO_FLIP` (0x00) - No flipping
- `NO_FLIP_X` (0x00) - No horizontal flip
- `NO_FLIP_Y` (0x00) - No vertical flip
- `FLIP_X` (0x08) - Horizontal flip
- `FLIP_Y` (0x80) - Vertical flip
- `FLIP_MAS` (0x88) - Both flips

`spr_pal( unsigned char palette );`
Sets the palette block index (0-15) of the current sprite.

`spr_pri( unsigned char priority );`
Sets the priority of the current sprite. '*0*' will make it appear behind the background (through color 0), '*1*' will make it appear in front of the background.

`spr_get_x( void );`
Returns the '*x*' coordinate of the current sprite.

`spr_get_y( void );`
Returns the '*y*' coordinate of the current sprite.

#### **SuperGrafx Sprite Functions**

`sgx_init_satb( void );`

`sgx_reset_satb( void );`

`sgx_satb_update( void );`

`sgx_spr_set( unsigned char num );`

`sgx_spr_hide( void );`

`sgx_spr_show( void );`

`sgx_spr_x( unsigned int value );`

`sgx_spr_y( unsigned int value );`

`sgx_spr_pattern( unsigned int vaddr );`

`sgx_spr_ctrl( unsigned char mask, unsigned char value );`

`sgx_spr_pal( unsigned char palette );`

`sgx_spr_pri( unsigned char priority );`

`sgx_spr_get_x( void );`

`sgx_spr_get_y( void );`

## **Legacy Graphics Functions**

`gfx_init( unsigned int start_vram_addr );`
Initializes screen to point to sequential graphics tiles, located starting at address '*start_vram_addr*' in VRAM.

`gfx_clear( unsigned int start_vram_addr );`
Clears graphical screen. Starting at address '*start_vram_addr*' in VRAM, this function sets sequential tiles in VRAM to all zeroes, with a size based on the virtual map.

`gfx_plot( unsigned int x, unsigned int y, char color );`
Sets a pixel at ('*x*', '*y*') to color listed. '*color*' should be a value between 0 and 15.

`gfx_line( unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2, unsigned char color );`
Draws a line from ('*x1*', '*y1*') to ('*x2*', '*y2*') in color listed. '*color*' should be a value between 0 and 15.

## **Color and Palette Functions**

`clear_palette( void );`
Clears all palette entries to black.

`set_color( unsigned int index, unsigned int value );`
Sets the specified color '*index*' (0-511) to a given color number from the 9-bit global palette. Requires an appropriate PC Engine color chart to be useful!

`get_color( unsigned int index );`
Retrieves the 9-bit RGB value of the specified color '*index*' (one 3-bit RGB component at a time, via bitshifting: blue -> red -> green).

`set_color_rgb( unsigned int index, unsigned char r, unsigned char g, unsigned char b );`
Sets the specified color to the given RGB component values (brightness ranges from 0 to 7). This function is much easier to use than `set_color()`, but it is noticeably slower.

`load_palette( unsigned char palette, unsigned char __far *data, unsigned int num_palettes );`
Loads one or more 16-color palette blocks at once. '*palette*' is the index of the first block (0-31) to load, and '*num_palettes*' the number of blocks. This function can be used to load palettes defined using `#defpal` or included with `#incpal` directives.

`far_load_palette( unsigned char palette, unsigned char num_palettes );`
Loads palette data from far memory. The data source must be set up using `set_far_base()` before calling this function.

`set_bgpal( unsigned char palette, unsigned char __far *data );`
`set_bgpal( unsigned char palette, unsigned char __far *data, unsigned int num_palettes );`
This legacy function is exactly the same as `load_palette()`, but it is limited to character palette block numbers 0 to 15. Without the third argument, the function loads only one block.

`set_sprpal( unsigned char palette, unsigned char __far *data );`
`set_sprpal( unsigned char palette, unsigned char __far *data, unsigned int num_palettes );`
This legacy function is exactly the same as `load_palette()`, except it offers direct access to sprite palette blocks. Sprite palette blocks are standard block numbers 16 to 31, but with this function you can simply access them with indices 0 to 15. This can make sprite palette block manipulation easier, since you don't have to remember the real sprite palette block indices. Without the third argument, the function loads only one block.

`read_palette( unsigned char palette, unsigned char num_palettes, unsigned int *destination );`
Reads palette data back from the VCE into the specified destination buffer.

`fade_to_black( unsigned int __far *from, unsigned int *destination, unsigned char num_colors, unsigned char value_to_sub );`
Fades palette colors towards black by subtracting the specified value from each color component.

`fade_to_white( unsigned int __far *from, unsigned int *destination, unsigned char num_colors, unsigned char value_to_add );`
Fades palette colors towards white by adding the specified value to each color component.

`cross_fade_to( unsigned int __far *from, unsigned int *destination, unsigned char num_colors, unsigned char which_component );`
Crossfades between two palettes, modifying only the specified color component (0=red, 1=green, 2=blue).

`far_fade_to_black( unsigned int *destination, unsigned char num_colors, unsigned char value_to_sub );`
Far memory version of `fade_to_black()`. Uses the current far memory source set by `set_far_base()`.

`far_fade_to_white( unsigned int *destination, unsigned char num_colors, unsigned char value_to_add );`
Far memory version of `fade_to_white()`. Uses the current far memory source set by `set_far_base()`.

`far_cross_fade_to( unsigned int *destination, unsigned char num_colors, unsigned char which_component );`
Far memory version of `cross_fade_to()`. Uses the current far memory source set by `set_far_base()`.

## **Character Map Functions**

Character map functions provide a simple map system based on 8x8 characters (tiles) in BAT format. This is useful for small maps or when you need granular control.

`set_chrmap( unsigned char __far *chrmap, unsigned char tiles_w );`
Sets the character map data. '*chrmap*' contains the map data using character indices, and '*tiles_w*' specifies the width of the map in tiles.

`draw_bat( void );`
Draws the current character map to the screen based on the current scroll position.

`scroll_bat( void );`
Updates the character map display based on the current scroll position. Call this when the map position changes.

`blit_bat( unsigned char tile_x, unsigned char tile_y, unsigned char tile_w, unsigned char tile_h );`
Draws a specific rectangular area of the character map to the screen. Parameters specify the tile coordinates and dimensions.

#### **SuperGrafx Character Map Functions**

`sgx_set_chrmap( unsigned char __far *chrmap, unsigned char tiles_w );`

`sgx_draw_bat( void );`

`sgx_scroll_bat( void );`

`sgx_blit_bat( unsigned char tile_x, unsigned char tile_y, unsigned char tile_w, unsigned char tile_h );`

## **Block Map Functions**

Block map functions provide a map system based on 16x16 blocks (meta-tiles). This system is more efficient for large maps and supports collision detection.

`set_blocks( unsigned char __far *blk_def, unsigned char __far *flg_def, unsigned char number_of_blocks );`
Defines the block definitions and collision flags for the block map system. '*blk_def*' contains the tile data for each block, '*flg_def*' contains collision flags, and '*number_of_blocks*' specifies how many blocks are defined.

`set_blkmap( unsigned char __far *blk_map, unsigned char blocks_w );`
Sets the block map data. '*blk_map*' contains the map data using block indices, and '*blocks_w*' specifies the width of the map in blocks.

`set_multimap( unsigned char __far *multi_map, unsigned char screens_w );`
Sets up a multi-screen map system. '*multi_map*' contains screen indices for large maps, and '*screens_w*' specifies the width in screens.

`draw_map( void );`
Draws the current block map to the screen based on the current scroll position.

`scroll_map( void );`
Updates the block map display based on the current scroll position. Call this when the map position changes.

`blit_map( unsigned char tile_x, unsigned char tile_y, unsigned char tile_w, unsigned char tile_h );`
Draws a specific rectangular area of the block map to the screen. Parameters specify the tile coordinates and dimensions.

`get_map_block( unsigned int x, unsigned int y );`
Gets the block index at the specified pixel coordinates. Returns the block index and sets global variables '*map_blk_flag*' and '*map_blk_mask*' with collision and mask information.

**Example:**
```c
// Set up a simple block map
set_blocks(block_definitions, collision_flags, 16);
set_blkmap(map_data, 32);  // 32 blocks wide

// Check collision at player position
unsigned char block = get_map_block(player_x, player_y);
if (map_blk_flag & COLLISION_MASK) {
    // Handle collision
}
```

#### **SuperGrafx Block Map Functions**

`sgx_set_blocks( unsigned char __far *blk_def, unsigned char __far *flg_def, unsigned char number_of_blocks );`

`sgx_set_blkmap( unsigned char __far *blk_map, unsigned char blocks_w );`

`sgx_set_multimap( unsigned char __far *multi_map, unsigned char screens_w );`

`sgx_draw_map( void );`

`sgx_scroll_map( void );`

`sgx_blit_map( unsigned char tile_x, unsigned char tile_y, unsigned char tile_w, unsigned char tile_h );`

`sgx_get_map_block( unsigned int x, unsigned int y );`

## **Legacy Tile and Map Functions**

This is an older tilemapping library from HuC3/4.

`set_tile_address( unsigned int vram );`
Sets the base VRAM address for legacy tile operations.

`set_tile_data( unsigned char __far *tiles, unsigned char num_tiles, unsigned char __far *palette_table, unsigned char tile_type );`
Defines an array of tiles to be used by all legacy tile and map functions.

`set_far_tile_data( unsigned char tile_bank, unsigned char *tile_addr, unsigned char num_tiles, unsigned char palette_table_bank, unsigned char *palette_table_addr, unsigned char tile_type );`
Sets tile data from far memory. All parameters specify the bank and address for the tile data and palette table.

**Parameters:**
- '*tiles'* : Address of tile graphics data in memory.
- '*num_tiles'* : Number of tiles (maximum 256).
- '*palette_table'* : Address of palette-index array for tiles (each tile has its own palette index).
- '*tile_type'* : Type of tiles (8x8 or 16x16).

**Important notes:**
- Palette indices must be shifted left by four bits (e.g., 0x40 for palette index 4).
- Used with `#inctile_ex()` or `#incchr_ex()` legacy directives for automatic tile type recognition.
- 8x8 tiles use `incchr_ex()`, 16x16 tiles use `inctile_ex()`.

`load_tile( unsigned int vram );`
Loads tile graphics data in VRAM, at address '*vram*'. You must first have defined a tile array with `set_tile_data()` before using this function.

`set_map_tile_base( unsigned int vram );`
Sets map tile base address.

`set_map_tile_type( unsigned char tile_type );`
Sets map tile type.

`set_map_pals( unsigned char __far *palette_table );`
Sets map palette table.

`set_map_data( unsigned char __far *map, unsigned char w, unsigned char h );`
Sets the map data for tile operations. '*w*' and '*h*' specify the map width and height in tiles.

`set_far_map_data( unsigned char map_bank, unsigned char *map_addr, unsigned char w, unsigned char h );`
Sets map data from far memory. '*map_bank*' and '*map_addr*' specify the far memory location.

`load_map( unsigned char bat_x, unsigned char bat_y, int map_x, int map_y, unsigned char tiles_w, unsigned char tiles_h );`
Loads a part of a map on the screen. '*sx*' and '*sy*' are screen coordinates (in tile units, i.e. 16 pixels), '*mx*' and '*my*' are positions in the map, and '*w*' and '*h*' are respectively the number of tile indices to load horizontally and vertically. This function doesn't do any screen clipping, so you must not pass incorrect or too big screen coordinates to it, as that would corrupt the VRAM!

`put_tile( unsigned char tile, unsigned char bat_x, unsigned char bat_y );`
Puts individual tiles on the screen, either directly at VRAM location '*vaddr*', or at screen coordinates '*x*' and '*y*' (in tile units). '*tile*' is a tile index in the tile array defined by the most recent call to `set_tile_data()`.

`map_get_tile( unsigned char map_x, unsigned char map_y );`
Returns the tile index as defined in the tile array used in the most recent call to `set_tile_data()`. The '*x*' and '*y*' arguments are in pixel units, unlike the put_tile functions. So this function is ideal for collision routines.

`map_put_tile( unsigned char map_x, unsigned char map_y, unsigned char tile );`
Modifies the map data (sets a map element to a new tile ID), but works only when the map is stored in RAM (i.e. a Super CD-ROM game which is loaded into System Card RAM, and executes there). '*x*' and '*y*' are specified in pixel units, not tiles.

`load_background( unsigned char __far *tiles, unsigned char __far *palettes, unsigned char __far *bat, unsigned char w, unsigned char h );`
This legacy all-in-one function is used to display an entire background image on the screen, like a game title image. It will load background character data, it will load the palette, and finally it will load the BAT. Use it with directives *#incchr*, *#incbat* and *#incpal* to manage the different types of data. The character data will be stored at fixed address 0x1000 to 0x5000 in VRAM.

**Note**: This basic function is hardcoded for a resolution of 256x224 pixels. The tileset (character data) is **not** optimized for duplicates! Each tile occupies its own VRAM space.

#### **SuperGrafx Legacy Tile and Map Functions**

`sgx_set_tile_address( unsigned int vram );`

`sgx_set_tile_data( unsigned char __far *tiles, unsigned char num_tiles, unsigned char __far *palette_table, unsigned char tile_type );`

`sgx_set_far_tile_data( unsigned char tile_bank, unsigned char *tile_addr, unsigned char num_tiles, unsigned char palette_table_bank, unsigned char *palette_table_addr, unsigned char tile_type );`

`sgx_load_tile( unsigned int vram );`

`sgx_set_map_tile_base( unsigned int vram );`

`sgx_set_map_tile_type( unsigned char tile_type );`

`sgx_set_map_pals( unsigned char __far *palette_table );`

`sgx_set_map_data( unsigned char __far *map, unsigned char w, unsigned char h );`

`sgx_set_far_map_data( unsigned char map_bank, unsigned char *map_addr, unsigned char w, unsigned char h );`

`sgx_load_map( unsigned char bat_x, unsigned char bat_y, int map_x, int map_y, unsigned char tiles_w, unsigned char tiles_h );`

`sgx_put_tile( unsigned char tile, unsigned char bat_x, unsigned char bat_y );`

`sgx_map_get_tile( unsigned char map_x, unsigned char map_y );`

`sgx_map_put_tile( unsigned char map_x, unsigned char map_y, unsigned char tile );`

## **Font Functions**

`load_default_font( void );`
Loads a default font in VRAM. Without parameters, the first default font is loaded just above the BAT in VRAM; usually it's address 0x0800. Otherwise you can select the font number, and eventually the address in VRAM. In its current implementation the library supports only one default font, but in the future more fonts could be made available.

`set_font_pal( unsigned char palette );`
Selects the tile palette block index (0-15) to use for the font foreground and background colors.

`set_font_color( unsigned char foreground, unsigned char background );`
Sets the default font foreground and background colors (colors range from 0 to 15). Changes won't take effect immediately, you must re-load the font by calling `load_default_font()`.

`set_font_addr( unsigned int vram );`
Sets the font address in VRAM. Use this function to change the current font, to use several fonts on the same screen, or when you load a custom font and need to tell the library where it is.

`load_font( char __far *font, unsigned char count );`
`load_font( char __far *font, unsigned char count, unsigned int vram );`
Loads a custom font in VRAM. When used together with the *#incchr* directive, it will allow you to load a font from a picture file.

`far_load_font( unsigned char count, unsigned int vram );`
Loads font data from far memory. The data source must be set up using `set_far_base()` before calling this function.

**Note 1**: Custom fonts are "*hard-colored*" fonts (i.e. colors come from your picture file), so they won't be affected by any previous call to `set_font_color()`. The number of characters to load ranges from 0 to 224; ASCII characters 0 to 31 are never used and can't be defined, so you must start your font at the space character which is ASCII code 32.

**Note 2**: If you don't implicitely give a VRAM address, the function will load your font just above the BAT (usually it's address 0x0800).

#### **SuperGrafx Font Functions**

`sgx_load_default_font( void );`

`sgx_set_font_pal( unsigned char palette );`

`sgx_set_font_addr( unsigned int vram );`

`sgx_load_font( char __far *font, unsigned char count, unsigned int vram );`

`sgx_far_load_font( unsigned char count, unsigned int vram );`

## **Printf and Formatted Output Functions**

These formatted output functions support up to 4 optional variable arguments. All functions support virtual-terminal control codes and escape sequences.

`putchar( unsigned char ascii );`
Outputs a single ASCII character to the current cursor position.

`puts( unsigned char *string );`
Outputs a null-terminated string to the current cursor position.

`printf( unsigned char *format, unsigned int vararg1, unsigned int vararg2, unsigned int vararg3, unsigned int vararg4 );`
Formats and outputs a string to the current cursor position. Supports 0-4 variable arguments.
- **Warning**: Width, precision, and escape sequence numbers must be ≤ 127.

`sprintf( unsigned char *string, unsigned char *format, unsigned int vararg1, unsigned int vararg2, unsigned int vararg3, unsigned int vararg4 );`
Formats a string and store it in the destination buffer. Supports 0-4 variable arguments.
- **Warning**: Output must be less than 256 characters.
- **Warning**: Width, precision, and escape sequence numbers must be ≤ 127.

**Example:**
```c
int score = 1234;
int lives = 3;
printf("Score: %d Lives: %d\n", score, lives);
printf("Position: \eX%d\eY%d", 10, 5);  // Set cursor position
```

**Format Specifiers:**
- `%c` - Character
- `%d`, `%i` - Signed integer
- `%u` - Unsigned integer  
- `%x`, `%X` - Hexadecimal
- `%s` - String
- `%%` - Literal '%'

**Escape Sequences:**
- `\n` - Newline (move to next line)
- `\r` - Carriage return (move to start of line)
- `\f` - Form feed (clear screen and home cursor)
- `\eX<num>` - Set X coordinate
- `\eY<num>` - Set Y coordinate
- `\eL<num>` - Set minimum X coordinate
- `\eT<num>` - Set minimum Y coordinate
- `\eP<num>` - Set current palette

#### **SuperGrafx Printf and Formatted Output Functions**

`sgx_putchar( unsigned char ascii );`

`sgx_puts( unsigned char *string );`

`sgx_printf( unsigned char *format, unsigned int vararg1, unsigned int vararg2, unsigned int vararg3, unsigned int vararg4 );`

## **Legacy Text Output Functions**

All these legacy text output functions have two forms: one where you directly specify the VRAM address, and another one where you specify '*x*' and '*y*' coordinates (in character units). The second form is a bit slower but more user-friendly.

**Note**: These text functions only work on VDC1.

`put_digit( unsigned char digit, unsigned char bat_x, unsigned char bat_y );`
Outputs a digit character '0'-'9' given its numeric value. Hexa digits ('A'-'F') are also supported, a value of 10 will output 'A', a value of 11 will output 'B', and so on.

`put_char( unsigned char digit, unsigned char bat_x, unsigned char bat_y );`
Outputs an ASCII character.          

`put_hex( unsigned int number, unsigned char length, unsigned char bat_x, unsigned char bat_y );`
Outputs a hexadecimal number. The '*length*' argument is used to format the number. As many as '*length*' hex digit(s) will be displayed. If the number has less than '*length*' digit(s), blank spaces will be added to its left.

`put_number( unsigned int number, unsigned char length, unsigned char bat_x, unsigned char bat_y );`
Outputs a signed number. The '*length*' argument is used to format the number. As many as '*length*' digit(s) will be displayed. If the number has less than '*length*' digit(s), blank spaces will be added to its left. If the number is negative, a '-' sign is added.

`put_string( unsigned char *string, unsigned char bat_x, unsigned char bat_y );`
Outputs a null terminated ASCII string.

`put_raw( unsigned int data, unsigned char bat_x, unsigned char bat_y );`
Outputs raw data directly to the screen at the specified position.

## **Number Functions**

`abs( int value );`
Returns the absolute value of the input.

`srand( unsigned char seed );`
Changes the random seed. You can use this function to improve randomness by giving a value based on when the player presses `RUN` for the first time, for example.

`rand( void );`
Returns a 16-bit random number.

`rand8( void );`
Returns an 8-bit random number.

`random8( unsigned char limit );`
Returns a random value between 0 and '*limit*' -1. Note: '*limit*' is 0...255.

`random( unsigned char limit );`
Returns a random value between 0 and '*limit*' -1. Note: '*limit*' is 0...128, values 129...255 are treated as 128. This legacy function is basically a 7-bit random number generator.

## **String Manipulation Functions**

Standard string manipulation functions with support for both regular and far memory sources.

`strcpy( char *destination, char *source );`
`strcpy( char *destination, char __far *source );`
Copies the source string to the destination area.
- **Overload 1**: Regular memory source.
- **Overload 2**: Far memory source (crosses memory banks).

`strcat( char *destination, char *source );`
`strcat( char *destination, char __far *source );`
Concatenates source string onto the end of destination string.
- **Overload 1**: Regular memory source.
- **Overload 2**: Far memory source (crosses memory banks).

`strlen( char *source );`
`strlen( char __far *source );`
Computes the length of the specified string, up to but not including the terminating null character.
- **Overload 1**: Regular memory source.
- **Overload 2**: Far memory source (crosses memory banks).

`strlcpy( char *destination, char *source, unsigned char size );`
`strlcpy( char *destination, char __far *source, unsigned char size );`
Copies the source string to the destination area with size limit (POSIX compliant).
- **Overload 1**: Regular memory source.
- **Overload 2**: Far memory source (crosses memory banks).

`strlcat( char *destination, char *source, unsigned char size );`
`strlcat( char *destination, char __far *source, unsigned char size );`
Concatenates source string to destination with size limit (POSIX compliant).
- **Overload 1**: Regular memory source.
- **Overload 2**: Far memory source (crosses memory banks).

`memcpy( unsigned char *destination, unsigned char *source, unsigned int count );`
`memcpy( unsigned char *destination, unsigned char __far *source, unsigned int count );`
Copies memory from source to destination.
- **Overload 1**: Regular memory source.
- **Overload 2**: Far memory source (crosses memory banks).

`mempcpy( unsigned char *destination, unsigned char *source, unsigned int count );`
`mempcpy( unsigned char *destination, unsigned char __far *source, unsigned int count );`
Copies memory from source to destination and returns pointer to end of destination.
- **Overload 1**: Regular memory source.
- **Overload 2**: Far memory source (crosses memory banks).

`memset( unsigned char *destination, unsigned char value, unsigned int count );`
Sets memory area to specified value.

`strcmp( char *destination, char *source );`
`strcmp( char *destination, char __far *source );`
Compares strings.
- **Overload 1**: Regular memory source.
- **Overload 2**: Far memory source (crosses memory banks).

`strncmp( char *destination, char *source, unsigned int count );`
`strncmp( char *destination, char __far *source, unsigned int count );`
Compares strings with limited count.
- **Overload 1**: Regular memory source.
- **Overload 2**: Far memory source (crosses memory banks).

`memcmp( unsigned char *destination, unsigned char *source, unsigned int count );`
`memcmp( unsigned char *destination, unsigned char __far *source, unsigned int count );`
Compares memory areas.
- **Overload 1**: Regular memory source.
- **Overload 2**: Far memory source (crosses memory banks).

## **ZX0 Compression Functions**

ZX0 "modern" format is not supported, because it costs an extra 4 bytes of code in this decompressor, and it runs slower.
Use Emmanuel Marty's **Salvador** ZX0 compressor, which can be found here: https://github.com/emmanuel-marty/salvador

*`salvador -classic <infile> <outfile>`*
Creates a ZX0 file to be decompressed to RAM.

*`salvador -classic -w 2048 <infile> <outfile>`*
Creates a ZX0 file to be decompressed to VRAM, using a 2KB ring-buffer in RAM.

`zx0_to_ram( unsigned char *ram, char far *compressed );`
Decompresses ZX0 compressed data directly to RAM.

`zx0_to_vdc( unsigned int vaddr, char far *compressed );`
Decompresses ZX0 compressed data directly to VDC1 VRAM.

`zx0_to_sgx( unsigned int vaddr, char far *compressed );`
Decompresses ZX0 compressed data directly to VDC2 VRAM.

## **Joypad Functions**

`joy( unsigned int which );`
Returns the current status of joypad number '*which*'. This function is 6 button compliant.

`joytrg( unsigned int which );`
Returns informations about newly pressed buttons and directions of joypad number '*which*'. But beware of this function: these informations are only refreshed every frame, so if your game loop is not fast enough you could miss some keypresses.

`joybuf( unsigned int which );`
Gets joypad buffer for joypad '*which*'. This function is only available if configured in hucc-config.inc.

`get_joy_events( unsigned int which );`
Gets joypad events for joypad '*which*'. This function is only available if configured in hucc-config.inc.

`clear_joy_events( unsigned char mask );`
Clears joypad events specified by '*mask*'. This function is only available if configured in hucc-config.inc.

## **Clock Functions**
***
`clock_hh( void );`
`clock_mm( void );`
`clock_ss( void );`
`clock_tt( void );`
Returns the number of *hours, minutes, seconds*, or *ticks* (one VSync interval, or 1/60th of a second) since the last `clock_reset()`.

**Note**: The accuracy of this clock will be "off" by about 2 seconds per whole hour. This is due to the fact that NTSC VSync frequency is actually 59.94Hz, rather than 60Hz (whilst the VCE "refreshes" at around 59.82Hz).

`clock_reset( void );`
Resets the clock timer.

## **CD-ROM Overlay Functions**

`cd_execoverlay( unsigned char ovl_index );`
Loads a program overlay specified by '*ovl_index*', and executes it. If an error occurs during loading, the previous context (i.e. the overlay running until that moment) is reloaded and an error value is returned to the program.

## **CD-ROM Functions**

`ac_exists( void );`
Checks for Arcade System Card. Returns **TRUE (1)** if exists; **FALSE (0)** if not.

`cd_boot( void );`
Boots the CD-ROM drive.

`cd_getver( void );`
Returns CD-ROM System Card version number in BCD (i.e. Japanese Super System Card = 0x0300, American Duo = 0x301).

`cd_reset( void );`
Resets the CD-ROM drive, and stops the motor.

`cd_pause( void );`
Pauses the CD-ROM drive during play of an audio track. Probably most useful if the player pauses the game in the middle of a level which is synchronized to music.

`cd_unpause( void );`
Continues the CD-ROM audio track after pause.

`cd_fade( unsigned char type );`
Fades CD-ROM audio with the specified fade type.

`cd_fastvram( unsigned char ovl_index, unsigned int sect_offset, unsigned int vramaddr, unsigned int sectors );`
Fast VRAM loading from CD-ROM. Loads the specified number of sectors directly to VRAM.

`cd_playtrk( unsigned char start_track, unsigned char end_track, unsigned char mode );`
Plays one or more CD-ROM audio tracks in a few different modes. This will not play '*end*' track, so '*end*' >= '*start*' +1. If you wish to play until end of disc (or if '*start*' track is the final track), then set '*end*' to value `CDPLAY_ENDOFDISC`.

    Attempts to play data tracks will not work, and will return non-zero error code.

    Valid modes        Meaning
    -----------        -------
    CDPLAY_MUTE        Plays audio without sound (muted)
    CDPLAY_REPEAT      Plays audio, repeating when track(s) complete
    CDPLAY_NORMAL      Plays audio only until completion of track(s)

`cd_playmsf( unsigned char start_minute,  unsigned char start_second,  unsigned char start_frame, unsigned char end_minute,  unsigned char end_second,  unsigned char end_frame,  unsigned char mode );`
Plays CD-ROM audio in a few different modes, as above. M/S/F = minute/second/frame indexing technique (there are 75 frames per second).

**Note**: See `cd_playtrk()` for valid values of '*mode*'.

`cd_loadvram( unsigned char ovl_index, unsigned int sect_offset, unsigned int vramaddr, unsigned int bytes );`
Reads data from the CD-ROM directly into VRAM at address specified by '*vramaddr*', for a length of '*bytes*'. Note that 2 bytes are required to fill one VRAM word. Reads it from the overlay segment specified by '*ovl_index*', with sector offset (i.e. multiples of 2048 bytes) of '*sect_offset*'. Non-zero return values indicate errors.

`cd_loaddata( unsigned char ovl_index, unsigned int sect_offset, unsigned char __far *buffer, unsigned int bytes );`
Reads data from the CD-ROM into area (or overlay 'const' or other data) specified by '*destaddr*', for a length of '*bytes*'. Reads it from the overlay segment specified by '*ovl_index*', with sector offset (i.e. multiples of 2048 bytes) of '*sect_offset*'. Non-zero return values indicate errors.

`cd_loadbank( unsigned char ovl_index, unsigned int sect_offset, unsigned char bank, unsigned int sectors );`

`cd_status( unsigned char mode );`
Checks the status of the CD-ROM drive. Non-zero return values indicate errors.

    Valid Mode   Meaning           Return value & meaning
    ----------   -------           ----------------------
       0         Drive Busy Check   0     = Drive Not Busy
                                    other = Drive Busy
       other     Drive Ready Check  0     = Drive Ready
                                    other = Sub Error Code

## **ADPCM Functions**

`ad_reset( void );`
Resets ADPCM hardware.

`ad_trans( unsigned char ovl_index, unsigned int sect_offset, unsigned char nb_sectors, unsigned int ad_addr );`
Transfers data from CD-ROM into ADPCM buffer. Reads it from the overlay segment specified by '*ovl_index*', with sector offset (i.e. multiples of 2048 bytes) of '*sect_offset*'. Reads '*nb_sectors*' sectors and stores data into ADPCM buffer starting from offset '*ad_addr*'. Non-zero return values indicate errors.

`ad_read( unsigned int ad_addr, unsigned char mode, unsigned int buf, unsigned int bytes );`
Reads data from the ADPCM buffer. Reads '*bytes*' bytes starting at offset '*ad_addr*' in ADPCM buffer. The mode '*mode*' determines the meaning of the '*buf*' offset. Non-zero return values indicate errors.

    Valid modes        Meaning of '*buf*'
    -----------        ----------------
    0                  Offset in the mapped RAM
    2-6                Directly into memory mapped to MMR #'mode'
    0xFF               Offset in VRAM

`ad_write( unsigned int ad_addr, unsigned char mode, unsigned int buf, unsigned int bytes );`
Writes data into the ADPCM buffer. Writes '*bytes*' bytes starting at offset '*ad_addr*' in ADPCM buffer. The mode '*mode*' determines the meaning of the '*buf*' offset. Non-zero return values indicate errors.

    Valid modes        Meaning of '*buf*'
    -----------        ----------------
    0                  Offset in the mapped RAM
    2-6                Directly into memory mapped to MMR #'mode'
    0xFF               Offset in VRAM

`ad_play( unsigned int ad_addr, unsigned int bytes, unsigned char freq, unsigned char mode );`
Plays ADPCM sound from data loaded in the ADPCM buffer. Starts playing from '*ad_addr*' offset in the ADPCM buffer, for '*bytes*' bytes. The frequency used for playback is computed from '*freq*' using the following formula: `real_frequency_in_khz = 32 / (16 - '*freq*')`. Valid range for '*freq*' is 0-15. If bit 0 of '*mode*' is on, values of '*ad_addr*', '*bytes*' and '*freq*' from the previous `ad_play()` call are reused. If bit 7 of '*mode*' is on, playback loops instead of stopping at the end of the range.

`ad_cplay( unsigned char ovl_index, unsigned int sect_offset, unsigned int nb_sectors, unsigned char freq );`
Compressed playing — loads and plays ADPCM data in one operation.

`ad_stop( void );`
Stops playing ADPCM.

`ad_stat( void );`
Returns current ADPCM status. Returns **FALSE(0)** if ADPCM playing isn't in progress, else returns a non-zero value.

## **Backup RAM Functions**

`bm_check( void );`
Checks for Backup RAM hardware. Returns **TRUE (1)** if exists; **FALSE (0)** if not.

`bm_format( void );`
Formats the whole Backup RAM.  Does not format if data is already formatted. Actually, data isn't erased but the header data reports so. You should be able to find old data in the BRAM using raw reads in the Backup RAM bank, but not through the usual HuC functions. Returns Backup RAM status as defined in the hucc.h file.

`bm_free( void );`
Returns the number of free bytes available for user data. The amount required for the data header and the necessary 2-byte termination are already deducted from this amount. The value returned is the number of bytes free for user data.

`bm_create( unsigned char *name, unsigned int length );`
Creates a new file named '*name*' with a size of '*size*' bytes. Returns Backup RAM status as defined in the hucc.h file.

`bm_exist( unsigned char *name );`
Checks whether a Backup RAM file exists. This returns **TRUE (!= 0)** if good; **FALSE (0)** if bad. The name structure is not just an ASCII name; it begins with a 2-byte "uniqueness ID" which is almost always 00 00, followed by 10 bytes of ASCII name, which should be padded with trailing spaces.

`bm_delete( unsigned char *name );`
Deletes a BRAM entry with a given name.

`bm_read( unsigned char *buffer, unsigned char *name, unsigned int offset, unsigned int length );`
Reads '*nb*' bytes from file named '*name*' and put them into '*buf*'. I'm not sure whether the '*offset*' is relative to the buffer or the file... Returns Backup RAM status as defined in the hucc.h file.

`bm_write( unsigned char *buffer, unsigned char *name, unsigned int offset, unsigned int length );`
Writes into the file named '*name*'. Data to write are in the buffer '*buf*' and '*nb*' bytes are written from offset '*offset*' in the buffer. Returns Backup RAM status as defined in the hucc.h file.

## **Debug and Test Functions**

**Important note**: These functions are primarily for debugging and testing purposes!

`dump_screen( void );`
Dumps screen contents for testing purposes. Only available in TGemu emulator.

`abort( void );`
Aborts program execution. Only available in TGemu emulator.

`exit( int value );`
Exits program with specified exit code. Only available in TGemu emulator.

`__builtin_ffs( unsigned int value );`
"Find First Set Bit". Counts the number of bits in an int until it finds one that is set.

## **Special cases, for compatibility with existing apps ONLY**

These functions are provided for compatibility with existing applications, but **should not be used in new code**.

***Note**: No deprecated functions are currently documented here.*

## **HuC3/4 Functions currently NOT Supported in HuCC (as of 2025/09/04)**

`spr_get_pattern`
`spr_get_pal`
`get_tile`
`gfx_point`
`gfx_setbgpal`
`get_font_addr`
`get_font_pal`
`strncpy`
`set_joy_callback`
`mouse_exists`
`mouse_enable`
`mouse_disable`
`mouse_x`
`mouse_y`
`cd_numtrk`
`cd_trktype`
`cd_trkinfo`
`bm_size`
`bm_rawread`
`bm_rawwrite`
`bm_errno`
`bm_sizeof`
`bm_getptr`
`bm_open`
`bm_enable`
`bm_disable`
`bm_checksum`
`bm_setup_ptr`
