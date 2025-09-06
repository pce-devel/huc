# HuCC Function Reference

## **Video Functions**

`cls( void );`
`cls( unsigned int tile );`
Clear the entire screen. 
- **Overload 1**: Without parameters, the screen is filled with a space character
- **Overload 2**: With `tile` parameter, the screen is filled with the specified BAT value

`disp_on( void );`
Enable display output.

`disp_off( void );`
Blank/Turn off the display output.

`vsync( void );`
`vsync( unsigned char count );`
Synchronize the program to the video Vertical Blanking Signal (VBL), which is around 1/60th of a second.

- **Overload 1**: Without parameters, returns as soon as a VBL signal has been received
- **Overload 2**: With `count` parameter, synchronizes to the number of frames requested (e.g., `vsync(3)` for 20fps)

`init_240x208( void );`
Initialize the screen to 240x208 resolution (30 characters wide by 26 characters tall).

`init_256x224( void );`
Initialize the screen to 256x224 resolution (32 characters wide by 28 characters tall).

`set_screen_size( unsigned char value );`
Change the virtual screen size. By default the startup code initializes a virtual screen of 64 characters wide and 32 character tall, but other values are possible, namely : 32x32, 128x32, 32x64, 64x64, or 128x64. The larger the virtual screen is the less video memory you will have for your graphics (font, tiles, sprites).

`set_xres( unsigned int x_pixels );`
`set_xres( unsigned int x_pixels, unsigned char blur_flag );`
Change the horizontal resolution to '*x_pixels*' (in pixels). This changes the video controller's registers to display more pixels on the screen; it does not affect any virtual calculations.

- **Overload 1**: Uses default blur setting (XRES_SOFT)
- **Overload 2**: With `blur_flag` parameter, specifies smoothing type: **XRES_SHARP** or **XRES_SOFT** (default)

**Note**: The 5MHz dot clock will be used up to a horizontal resolutions of 268 pixels. The 7MHz dot clock will be used up to 356 pixels and the 10MHz dot clock will be used beyond this. The maximum visible resolution seems to be around 512 pixels.

`vram_addr( unsigned char bat_x, unsigned char bat_y );`     
Return the VRAM address for the character at position (bat_x, bat_y).

`get_vram( unsigned int address );`
Read a 16-bit word from VRAM at the specified address.

`put_vram( unsigned int address, unsigned int data );`
Write a 16-bit word to VRAM at the specified address.

**Note:** These functions provide direct VRAM access for low-level operations. For most graphics operations, use the higher-level functions like `load_vram()`, `load_bat()`, etc.

`load_vram( unsigned int vram, unsigned char __far *data, unsigned int num_words );`
Generic function to load data (BAT, CG, sprites) in video memory, at address `vram`. '*num_words*' is the number of 16-bit words to load.

`vram2vram( unsigned int dest_vram, unsigned int src_vram, unsigned int num_words );`
Copy data from one VRAM location to another using DMA. Useful for sprite animation and tile updates. '*dest_vram*' is the destination address, '*src_vram*' is the source address, and '*num_words*' is the number of 16-bit words to copy.

`far_load_vram( unsigned int vram, unsigned int num_words );`
Load VRAM data from far memory. The data source must be set up using `set_far_base()` before calling this function.

`load_bat( unsigned int vram, unsigned char __far *data, unsigned char tiles_w, unsigned char tiles_h );`
Load a rectangular character attribute table (BAT) of width 'w' and of height 'h' in video memory at address '*vram*'.

`far_load_bat( unsigned int vram, unsigned char tiles_w, unsigned char tiles_h );`
Load BAT data from far memory. The data source must be set up using `set_far_base()` before calling this function.

`load_sprites( unsigned int vram, unsigned char __far *data, unsigned int num_groups );`
Load sprite pattern data to VRAM. '*num_groups*' specifies the number of 16x16 sprite groups to load.

`far_load_sprites( unsigned int vram, unsigned int num_groups );`
Load sprite data from far memory. The data source must be set up using `set_far_base()` before calling this function.

`vram2vram( unsigned int vram_dst, unsigned int vram_src, unsigned int word_len );`
Copy data within VRAM from source address to destination address. '*word_len*' is the number of 16-bit words to copy.

`peek( unsigned int addr );`
`peekw( unsigned int addr );`
Read the contents of memory location 'addr'. peek() is char-sized access, whereas peekw() is word-sized.

`poke( unsigned int addr, unsigned char with );`
`pokew( unsigned int addr, unsigned int with );`
Write '*val*' value at memory location 'addr'. `poke()` is char-sized access, whereas `pokew()` is word-sized. 
This function can be used to access the hardware I/O ports located at 0x0000 to 0x1FFF.

## **Memory Access Functions**

`farpeek( void __far *addr );`
Read a byte from far memory address. Used for accessing data across memory banks.

`farpeekw( void __far *addr );`
Read a word from far memory address. Used for accessing data across memory banks.

`set_far_base( unsigned char data_bank, unsigned char *data_addr );`
Set the base address for far memory operations. Used in conjunction with far memory functions like `far_load_vram()`, `far_load_bat()`, etc.

**Related functions:** `set_far_offset()`, `far_load_vram()`, `far_load_bat()`, `far_load_sprites()`, `far_load_palette()`, `far_load_font()`

`set_far_offset( unsigned int offset, unsigned char data_bank, unsigned char *data_addr );`
Set the offset for far memory operations. Allows fine-tuning of the far memory pointer.

`far_peek( void );`
Read a byte from the current far memory location (set by `set_far_base()`).

`far_peekw( void );`
Read a word from the current far memory location (set by `set_far_base()`).

## **Joypad Functions**

`joy( unsigned int which );`
Return the current status of joypad number 'which'. This function is 6 button compliant.

`joytrg( unsigned int which );`
Return informations about newly pressed buttons and directions of joypad number '*which*'. But beware of this function, these informations are refreshed every frame, so if your game loop is not fast enough you could miss some keypresses.

`joybuf( unsigned int which );`
Get joypad buffer for joypad 'which'. This function is only available if configured in hucc-config.inc.

`get_joy_events( unsigned int which );`
Get joypad events for joypad 'which'. This function is only available if configured in hucc-config.inc.

`clear_joy_events( unsigned char mask );`
Clear joypad events specified by 'mask'. This function is only available if configured in hucc-config.inc.

## **Color and Palette Functions**

`clear_palette( void );`
Clear all palette entries to black.

`set_color( unsigned int index, unsigned int value );`
Set the specified color `index` (0-511) to the given rgb-value.

`set_color_rgb( unsigned int index, unsigned char r, unsigned char g, unsigned char b );`
Set the specified color to the given rgb component values. This function is easier to use than `set_color()`, but it is slower.

`get_color( unsigned int index );`
Retrieve the rgb-value of the specified color.

`load_palette( unsigned char palette, unsigned char __far *data, unsigned int num_palettes );`
Load one or more 16-color palette-blocks at once. `palette` is the index of the first block (0-31) to load, and `num_palettes` the number of blocks. This function can be used to load palettes defined using `#defpal` or included with `#incpal` directives.

`far_load_palette( unsigned char palette, unsigned char num_palettes );`
Load palette data from far memory. The data source must be set up using `set_far_base()` before calling this function.

`read_palette( unsigned char palette, unsigned char num_palettes, unsigned int *destination );`
Read palette data back from the VCE into the specified destination buffer.

`set_sprpal( unsigned char palette, unsigned char __far *data );`
`set_sprpal( unsigned char palette, unsigned char __far *data, unsigned int num_palettes );`
Exactly the same function as `load_palette()`, but this function offers direct access to sprite palette-blocks. Sprite palette-blocks are standard block number 16 to 31, but with this function you can simply access them with indexes 0 to 15. This function and the `set_bgpal()` function make sprite and character palette-block manipulation easier; with them you don't have to know the real block indexes. Without the third arguments, the function loads only one block.

`set_bgpal( unsigned char palette, unsigned char __far *data );`
`set_bgpal( unsigned char palette, unsigned char __far *data, unsigned int num_palettes );`
Exactly the same function has `load_palette()`, but this function offers direct access to charachter palette-blocks. Character palette-blocks are standard block number 16 to 31, but with this function you can simply access them with indexes 0 to 15. This function and the `set_sprpal()` function make sprite and character palette-blocks manipulation easier; with them you don't have to know the real block indexes. Without the third arguments, the function loads only one block.

`fade_to_black( unsigned int __far *from, unsigned int *destination, unsigned char num_colors, unsigned char value_to_sub );`
Fade palette colors towards black by subtracting the specified value from each color component.

`fade_to_white( unsigned int __far *from, unsigned int *destination, unsigned char num_colors, unsigned char value_to_add );`
Fade palette colors towards white by adding the specified value to each color component.

`cross_fade_to( unsigned int __far *from, unsigned int *destination, unsigned char num_colors, unsigned char which_component );`
Cross-fade between two palettes, modifying only the specified color component (0=red, 1=green, 2=blue).

`far_fade_to_black( unsigned int *destination, unsigned char num_colors, unsigned char value_to_sub );`
Far memory version of fade_to_black. Uses the current far memory source.

`far_fade_to_white( unsigned int *destination, unsigned char num_colors, unsigned char value_to_add );`
Far memory version of fade_to_white. Uses the current far memory source.

`far_cross_fade_to( unsigned int *destination, unsigned char num_colors, unsigned char which_component );`
Far memory version of cross_fade_to. Uses the current far memory source.

## **Clock Functions**
***
`clock_hh( void );`
`clock_mm( void );`
`clock_ss( void );`
`clock_tt( void );`
Return the number of *hours, minutes, seconds*, or '*ticks* (one VSYNC interval, or 1/60th of a second) since the last `clock_reset()`.

Note: The accuracy of this clock will be "off" by about 2 seconds per whole hour. This is due to the fact that NTSC VSYNC frequency is actually 59.94Hz, rather than 60Hz (whilst the VCE "refreshes" at around 59.82Hz).

`clock_reset( void );`
Resets the clock timer.

## **Number Functions**

`abs( int value );`
Return the absolute value of the input.

`srand( unsigned char seed );`
Change the random seed. You can use this function to improve randomness by giving a value based on when the player presses start for the first time, for example.

`rand( void );`
Return a 16-bit random number.

`rand8( void );`
Return an 8-bit random number.

`random8( unsigned char limit );`
Return a random value between 0 and limit-1. Note: "limit" is 0..255.

`random( unsigned char limit );`
Return a random value between 0 and limit-1. Note: "limit" is 0..128, values 129..255 are treated as 128.

## **Tile & Map Functions**

`set_tile_address( unsigned int vram );`
Set the base VRAM address for tile operations.

`set_tile_data( unsigned char __far *tiles, unsigned char num_tiles, unsigned char __far *palette_table, unsigned char tile_type );`
Define an array of tiles to be used by all tile and map functions.

**Parameters:**
- `tiles`: Address of tile graphics data in memory
- `num_tiles`: Number of tiles (maximum 256)
- `palette_table`: Address of palette-index array for tiles (each tile has its own palette index)
- `tile_type`: Type of tiles (8x8 or 16x16)

**Important Notes:**
- Palette indexes must be shifted left by four bits (e.g., 0x40 for palette index 4)
- Used with `#inctile_ex()` or `#incchr_ex()` directives for automatic tile type recognition
- 8x8 tiles use `incchr_ex`, 16x16 tiles use `inctile_ex`

**Related functions:** `set_tile_address()`, `load_tile()`, `put_tile()`, `map_get_tile()`, `map_put_tile()`

`set_far_tile_data( unsigned char tile_bank, unsigned char *tile_addr, unsigned char num_tiles, unsigned char palette_table_bank, unsigned char *palette_table_addr, unsigned char tile_type );`
Set tile data from far memory. All parameters specify the bank and address for the tile data and palette table.

`set_map_data( unsigned char __far *map, unsigned char w, unsigned char h );`
Set the map data for tile operations. '*w*' and '*h*' specify the map width and height in tiles.

`set_far_map_data( unsigned char map_bank, unsigned char *map_addr, unsigned char w, unsigned char h );`
Set map data from far memory. '*map_bank*' and '*map_addr*' specify the far memory location.

`load_tile( unsigned int vram );`
Load tile graphics data in video memory, at address '*vram*'. You must first have defined a tile array with `set_tile_data()` before using this function.

`put_tile( unsigned char tile, unsigned char bat_x, unsigned char bat_y );`
Put individual tiles on the screen, either directly at video memory location '*vaddr*', or at screen coordinates 'x/y' (in tile unit). '*tile*' is a tile index in the tile array defined by the most recent call to `set_tile_data()`.

`map_put_tile( unsigned char map_x, unsigned char map_y, unsigned char tile );`
Modifies the map data (sets a map element to a new tile ID), but works only when the map is stored in RAM - ie. a Super CDROM game which is loaded into RAM, and executes there. 'x' and 'y' are specified in the same units as `map_get_tile()` (ie. pixels, not tiles).

`map_get_tile( unsigned char map_x, unsigned char map_y );`
Return the tile index as defined in the tile array used in the most recent call to `set_tile_data()`. The 'x/y' argument is in pixel units, unlike the put_tile functions and thus this function is ideal for colision routines.

`load_map( unsigned char bat_x, unsigned char bat_y, int map_x, int map_y, unsigned char tiles_w, unsigned char tiles_h );`
Load a part of a map on the screen. 'sx' and 'sy' are screen coordinates (in tile unit; 16 pixels), 'mx' and 'my' are position in the map, and 'w' and 'h' are respectively the number of tile-index to load horizontaly and verticaly. This function doesn't do any screen clipping, so you must not pass incorrect or too big screen coordinates to it as that would corrupt the video memory.

`set_map_pals( unsigned char __far *palette_table );`

`set_map_tile_type( unsigned char tile_type );`

`set_map_tile_base( unsigned int vram );`

`load_background( unsigned char __far *tiles, unsigned char __far *palettes, unsigned char __far *bat, unsigned char w, unsigned char h );`
This function is an all-in-one function, it is used to display an entire background image on the screen, like a game title image. It will load BG character data, it will load the palette, and finaly if will load the BAT. Use it with directives *#incchr*, *#incpal* and *#incbat* to manage the different types of data. The BG character data will be stored at address 0x1000 to 0x5000 in video memory.

## **Block Map Functions**

Block map functions provide a map system based on 16x16 meta-tiles (blocks). This system is more efficient for large maps and supports collision detection.

`set_blocks( unsigned char __far *blk_def, unsigned char __far *flg_def, unsigned char number_of_blocks );`
Define the block definitions and collision flags for the block map system. `blk_def` contains the tile data for each block, `flg_def` contains collision flags, and `number_of_blocks` specifies how many blocks are defined.

`set_blkmap( unsigned char __far *blk_map, unsigned char blocks_w );`
Set the block map data. `blk_map` contains the map data using block indices, and `blocks_w` specifies the width of the map in blocks.

`set_multimap( unsigned char __far *multi_map, unsigned char screens_w );`
Set up a multi-screen map system. `multi_map` contains screen indices for large maps, and `screens_w` specifies the width in screens.

`draw_map( void );`
Draw the current block map to the screen based on the current scroll position.

`scroll_map( void );`
Update the block map display based on the current scroll position. Call this when the map position changes.

`blit_map( unsigned char tile_x, unsigned char tile_y, unsigned char tile_w, unsigned char tile_h );`
Draw a specific rectangular area of the block map to the screen. Parameters specify the tile coordinates and dimensions.

`get_map_block( unsigned int x, unsigned int y );`
Get the block index at the specified pixel coordinates. Returns the block index and sets global variables `map_blk_flag` and `map_blk_mask` with collision and mask information.

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

### **SuperGrafx Block Map Functions**

`sgx_set_blocks( unsigned char __far *blk_def, unsigned char __far *flg_def, unsigned char number_of_blocks );`
SuperGrafx version of `set_blocks()`.

`sgx_set_blkmap( unsigned char __far *blk_map, unsigned char blocks_w );`
SuperGrafx version of `set_blkmap()`.

`sgx_set_multimap( unsigned char __far *multi_map, unsigned char screens_w );`
SuperGrafx version of `set_multimap()`.

`sgx_draw_map( void );`
SuperGrafx version of `draw_map()`.

`sgx_scroll_map( void );`
SuperGrafx version of `scroll_map()`.

`sgx_blit_map( unsigned char tile_x, unsigned char tile_y, unsigned char tile_w, unsigned char tile_h );`
SuperGrafx version of `blit_map()`.

`sgx_get_map_block( unsigned int x, unsigned int y );`
SuperGrafx version of `get_map_block()`.

## **Character Map Functions**

Character map functions provide a simple map system based on 8x8 characters (tiles) in BAT format. This is useful for smaller maps or when you need more granular control.

`set_chrmap( unsigned char __far *chrmap, unsigned char tiles_w );`
Set the character map data. `chrmap` contains the map data using character indices, and `tiles_w` specifies the width of the map in tiles.

`draw_bat( void );`
Draw the current character map to the screen based on the current scroll position.

`scroll_bat( void );`
Update the character map display based on the current scroll position. Call this when the map position changes.

`blit_bat( unsigned char tile_x, unsigned char tile_y, unsigned char tile_w, unsigned char tile_h );`
Draw a specific rectangular area of the character map to the screen. Parameters specify the tile coordinates and dimensions.

### **SuperGrafx Character Map Functions**

`sgx_set_chrmap( unsigned char __far *chrmap, unsigned char tiles_w );`
SuperGrafx version of `set_chrmap()`.

`sgx_draw_bat( void );`
SuperGrafx version of `draw_bat()`.

`sgx_scroll_bat( void );`
SuperGrafx version of `scroll_bat()`.

`sgx_blit_bat( unsigned char tile_x, unsigned char tile_y, unsigned char tile_w, unsigned char tile_h );`
SuperGrafx version of `blit_bat()`.

## **Font Related Functions**

`set_font_addr( unsigned int vram );`
Set the font address in video memory. Use this function to change the current font; to use several font on the same screen, or when you load yourself a font and need to tell the library where it is.

`set_font_pal( unsigned char palette );`
Set the font palette index (0-15) to use.

`load_font( char __far *font, unsigned char count );`
`load_font( char __far *font, unsigned char count, unsigned int vram );`
Load a custom font in video memory. Used together with the *#incchr* directive it will allow you to load a font from a PCX file. 

Note: Custom fonts are "*colored*" fonts, so they won't be affected by any previous call to `set_font_color()`. The number of characters to load ranges from 0 to 224, ASCII characters 0 to 31 are never used, and can't be defined so you must start your font at the space character which is ASCII code 32. 

If you don't implicitely give a video memory address, the function will load your font just above the BAT (usualy it's address 0x800).

**Related functions:** `set_font_addr()`, `set_font_pal()`, `set_font_color()`, `load_default_font()`, `far_load_font()`

`far_load_font( unsigned char count, unsigned int vram );`
Load font data from far memory. The data source must be set up using `set_far_base()` before calling this function.

`set_font_color( unsigned char foreground, unsigned char background );`
Set the default font foreground and background colors (colors range from 0 to 15). Changes won't take effect immediately, you must re-load the font by calling `load_default_font()`.

`load_default_font( void );`
Loads a default font in video memory. Without parameters the first default font is loaded just above the BAT in video memory; usualy it's address 0x800. Otherwise you can select the font number, and eventualy the address in video memory. In its current implementation the library supports only one default font, but in the future more fonts could be made available.

## **Text Output Functions**
All the text output functions have two forms, one where you directly specify the video memory address, and another one where you specify x/y coordinates (in character units). The second form is a bit slower but more user-friendly.

`put_digit( unsigned char digit, unsigned char bat_x, unsigned char bat_y );`
Output a digit character '0'-'9' given its numeric value. Hexa digits ('A'-'F') are also supported, a value of 10 will output 'A', a value of 11 will output 'B', and so on...

`put_char( unsigned char digit, unsigned char bat_x, unsigned char bat_y );`
Output an ASCII character.          

`put_hex( unsigned int number, unsigned char length, unsigned char bat_x, unsigned char bat_y );`
Output a hexadecimal number. The 'length' argument is used to format the number. As many as 'length' hex digit(s) will be displayed. If the number has less than 'length' digit(s), blank spaces will be added to its left.

`put_number( unsigned int number, unsigned char length, unsigned char bat_x, unsigned char bat_y );`
Output a signed number. The 'length' argument is used to format the number. As many as 'length' digit(s) will be displayed. If the number has less than 'lgenth' digit(s), blank spaces will be added to its left. If the number is negative, a '-' sign is added.

`put_string( unsigned char *string, unsigned char bat_x, unsigned char bat_y );`
Output a null terminated ASCII string.

`put_raw( unsigned int data, unsigned char bat_x, unsigned char bat_y );`
Output raw data directly to the screen at the specified position.

## **Printf and Formatted Output Functions**

Formatted output functions with support for up to 4 variable arguments. All functions support virtual-terminal control codes and escape sequences.

`putchar( unsigned char ascii );`
Output a single ASCII character to the current cursor position.

`puts( unsigned char *string );`
Output a null-terminated string to the current cursor position.

`sprintf( unsigned char *string, unsigned char *format [, ...] );`
Format a string and store it in the destination buffer. Supports 0-4 variable arguments.
- **Warning**: Output must be less than 256 characters
- **Warning**: Width, precision, and escape sequence numbers must be ≤ 127

`printf( unsigned char *format [, ...] );`
Format and output a string to the current cursor position. Supports 0-4 variable arguments.
- **Warning**: Width, precision, and escape sequence numbers must be ≤ 127

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

## **Graphics Functions**
`gfx_init( unsigned int start_vram_addr );`
Initialize screen to point to sequential graphics tiles, located starting at address '*start_vram_addr*' in VRAM.

`gfx_clear( unsigned int start_vram_addr );`
Clear graphical screen. Starting at address '*start_vram_addr*' in VRAM, this function sets sequential tiles in VRAM to all zeroes, with a size based on the virtual map.

`gfx_plot( unsigned int x, unsigned int y, char color );`
Set a pixel at (x,y) to color listed. '*color*' should be avalue between 0 and 15.

`gfx_line( unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2, unsigned char color );`
Draw a line from (x1, y1) to (x2, y2) in color listed. '*color*' should be a value between 0 and 15.

## **Split Screen and Scrolling Functions**

`scroll_split( unsigned char index, unsigned char screen_line, unsigned int bat_x, unsigned int bat_y, unsigned char display_flags );`
Create a split screen window at the specified screen line. '*index*' is the split index (0-127), '*screen_line*' is where the split occurs, '*bat_x*' and '*bat_y*' are the background position, and '*display_flags*' control what's displayed (BKG_ON, SPR_ON, etc.).

**Example:**
```c
// Create a status bar at line 20 with background only
scroll_split(0, 20, 0, 0, BKG_ON);

// Create a split at line 100 showing both background and sprites
scroll_split(1, 100, 32, 16, BKG_ON | SPR_ON);
```

`disable_split( unsigned char index );`
Disable the split screen window specified by '*index*'.

`disable_all_splits( void );`
Disable all active split screen windows.

`scroll( unsigned char num, unsigned int x, unsigned int y, unsigned char top, unsigned char bottom, unsigned char disp );`
Define screen window '*num*'. Up to four window can be defined. '*top*' and '*bottom*' are the screen top and bottom limits of the window (limits are included in the window area). '*disp*' controls the type of the window, if bit 7 is set background graphics will be displayed in this window, and if bit 6 is set sprites will also be displayed. If none of these bits are set the window will stay blank. 'x' and 'y' are the top-left coordinates of the area in the virtual screen that will be displayed in the window.

`scroll_disable( unsigned char num );`
Disable scrolling for the screen window '*num*'.

## **Sprite Functions**

`init_satb( void );`
Initialize the internal Sprite Attribute Table (SATB) used by the library to handle sprites. This function must be called before any other sprite function is called.

**Related functions:** `reset_satb()`, `satb_update()`, `spr_set()`, `spr_show()`, `spr_hide()`

`sgx_init_satb( void );`
Initialize the SuperGrafx Sprite Attribute Table (SATB). Must be called before any SuperGrafx sprite functions are used.

`sgx_detect( void );`
Detect if the system is running on SuperGrafx hardware. Returns non-zero if SuperGrafx is detected, zero otherwise.

`init_240x208( void );`
Initialize the screen to 240x208 resolution with 32x32 BAT size. This is a common resolution for games that need more screen space than the default 256x224.

`reset_satb( void );`
Reset the internal SATB, this has the effect to disable and reset all the sprites.

`satb_update( void );`
Copy the internal sprite attribute table to the video ram. This will refresh sprites on the screen. Use this function regularly to update the sprite display. The best place to call `satb_update()` is after every `vsync()` call, but no need to call `satb_update()` if you didn't change any sprite attribute. '*nb*' specifys the number of sprite to refresh; starting from sprite 0. By default the library refreshes only the sprites you use, but if you need to implicitely refresh a certain number of sprites then you can use '*nb*'.

`spr_set( unsigned char num );`
Select sprite '*num*' (0-63) as the current sprite.

`spr_hide( void );`
Without parameters this function will hide the current sprite. Use '*num*' to hide a different sprite than the current one.

`spr_show( void );`
Show a sprite that has been hidden using the `spr_hide()` function.

`spr_x( unsigned int value );`
Set the x coordinate of the current sprite. Negative values will make the sprite disappear under the left border, while values higher than the screen width will make it disappear under the right border.

`spr_y( unsigned int value );`
Set the y coordinate of the current sprite.

`spr_pattern( unsigned int vaddr );`
Set the pattern address in video memory of the current sprite.

`spr_ctrl( unsigned char mask, unsigned char value );`
Set different attributes of the current sprite using bitwise operations.

**Sprite Sizes:**
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

**Sprite Control Masks:**
- `SIZE_MAS` (0x31) - Size mask for `spr_ctrl()`
- `FLIP_MAS` (0x88) - Flip mask for `spr_ctrl()`
- `FLIP_X_MASK` (0x08) - Horizontal flip mask
- `FLIP_Y_MASK` (0x80) - Vertical flip mask

`spr_pal( unsigned char palette );`
Set the palette-block index (0-15) of the current sprite.

`spr_pri( unsigned char priority );`
Set the priority of the current sprite. '0' will make it appear behind the background (through color 0), '1' will make it appear in front of the background.

`spr_get_x( void );`
Return the x coordinate of the current sprite.

`spr_get_y( void );`
Return the y coordinate of the current sprite.


## **String Manipulation Functions**

Standard string manipulation functions with support for both regular and far memory sources.

`strcpy( char *destination, char *source );`
`strcpy( char *destination, char __far *source );`
Copy the source string to the destination area.
- **Overload 1**: Regular memory source
- **Overload 2**: Far memory source (crosses memory banks)

`strcat( char *destination, char *source );`
`strcat( char *destination, char __far *source );`
Concatenate source string onto the end of destination string.
- **Overload 1**: Regular memory source
- **Overload 2**: Far memory source (crosses memory banks)

`strlen( char *source );`
`strlen( char __far *source );`
Compute the length of the specified string, up to but not including the terminating null character.
- **Overload 1**: Regular memory source
- **Overload 2**: Far memory source (crosses memory banks)

`strlcpy( char *destination, char *source, unsigned char size );`
`strlcpy( char *destination, char __far *source, unsigned char size );`
Copy the source string to the destination area with size limit (POSIX compliant).
- **Overload 1**: Regular memory source
- **Overload 2**: Far memory source (crosses memory banks)

`strlcat( char *destination, char *source, unsigned char size );`
`strlcat( char *destination, char __far *source, unsigned char size );`
Concatenate source string to destination with size limit (POSIX compliant).
- **Overload 1**: Regular memory source
- **Overload 2**: Far memory source (crosses memory banks)

`memcpy( unsigned char *destination, unsigned char *source, unsigned int count );`
`memcpy( unsigned char *destination, unsigned char __far *source, unsigned int count );`
Copy memory from source to destination.
- **Overload 1**: Regular memory source
- **Overload 2**: Far memory source (crosses memory banks)

`mempcpy( unsigned char *destination, unsigned char *source, unsigned int count );`
`mempcpy( unsigned char *destination, unsigned char __far *source, unsigned int count );`
Copy memory from source to destination, return pointer to end of destination.
- **Overload 1**: Regular memory source
- **Overload 2**: Far memory source (crosses memory banks)

`memset( unsigned char *destination, unsigned char value, unsigned int count );`
Set memory area to specified value.

`strcmp( char *destination, char *source );`
`strcmp( char *destination, char __far *source );`
Compare strings.
- **Overload 1**: Regular memory source
- **Overload 2**: Far memory source (crosses memory banks)

`strncmp( char *destination, char *source, unsigned int count );`
`strncmp( char *destination, char __far *source, unsigned int count );`
Compare strings with limited count.
- **Overload 1**: Regular memory source
- **Overload 2**: Far memory source (crosses memory banks)

`memcmp( unsigned char *destination, unsigned char *source, unsigned int count );`
`memcmp( unsigned char *destination, unsigned char __far *source, unsigned int count );`
Compare memory areas.
- **Overload 1**: Regular memory source
- **Overload 2**: Far memory source (crosses memory banks)

## **CD Overlay functions**

`cd_execoverlay( unsigned char ovl_index );`
Loads a program overlay specified by '*ovl_index*', and execute it. If an error occurs during loading, the previous context (ie. the overlay running until that moment) is reloaded and an error value is returned to the program.

## **CDROM functions**

`cd_boot( void );`
Boot the CD-ROM system.

`cd_getver( void );`
Returns CDROM system card version number in BCD. (ie. Japanese Super System card = 0x0300, American Duo = 0x301)

`cd_reset( void );`
Reset the CDROM drive, and stop the motor

`cd_pause( void );`
Pause the CDROM drive during play of an audio track. Probably most useful if the player pauses the game in the middle of a level which is synchronized to music.

`cd_unpause( void );`
Continue the CDROM audio audio track after pause.

`cd_fade( unsigned char type );`
Fade CD audio with the specified fade type.

`cd_fastvram( unsigned char ovl_index, unsigned int sect_offset, unsigned int vramaddr, unsigned int sectors );`
Fast VRAM loading from CD-ROM. Loads the specified number of sectors directly to VRAM.

`cd_playtrk( unsigned char start_track, unsigned char end_track, unsigned char mode );`
Play one or more CDROM audio tracks in a few different modes. This will not play '*end*' track, so '*end*' >= 'start'+1. If you wish to play until end of disc (or if 'start' track is final track), then set 'end' to value '**CDPLAY_ENDOFDISC**'.

    Attempts to play data tracks will not play, and will return non-zero error code.

    Valid modes        Meaning
    -----------        -------
    CDPLAY_MUTE        Plays audio without sound (muted)
    CDPLAY_REPEAT      Plays audio, repeating when track(s) complete
    CDPLAY_NORMAL      Plays audio only until completion of track(s)

`cd_playmsf( unsigned char start_minute,  unsigned char start_second,  unsigned char start_frame, unsigned char end_minute,  unsigned char end_second,  unsigned char end_frame,  unsigned char mode );`
Play CDROM audio in a few different modes, as above.
M/S/F = minute/second/frame indexing technique.
(Note: there are 75 frames per second)

(See `cd_playtrk()` for valid values of '*mode*')

`cd_loadvram( unsigned char ovl_index, unsigned int sect_offset, unsigned int vramaddr, unsigned int bytes );`
Read data from the CDROM directly into video RAM at address specified by '*vramaddr*', for a length of '*bytes*'.  Note that 2 bytes are required to fill one VRAM word.

Read it from the overlay segment specified by '*ovl_index*', with sector offset (ie. multiples of 2048 bytes) of '*sect_offset*'.

Non-zero return values indicate errors.

`cd_loaddata( unsigned char ovl_index, unsigned int sect_offset, unsigned char __far *buffer, unsigned int bytes );`
Read data from the CDROM into area (or overlay 'const' or other data) specified by '*destaddr*', for a length of '*bytes*'.

Read it from the overlay segment specified by '*ovl_index*', with sector offset (ie. multiples of 2048 bytes) of '*sect_offset*'.

Non-zero return values indicate errors.

`cd_loadbank( unsigned char ovl_index, unsigned int sect_offset, unsigned char bank, unsigned int sectors );`

`cd_status( unsigned char mode );`
Checks the status of the CDROM device.

    Valid Mode   Meaning           Return value & meaning
    ----------   -------           ----------------------
       0         Drive Busy Check   0     = Drive Not Busy
                                    other = Drive Busy
       other     Drive Ready Check  0     = Drive Ready
                                    other = Sub Error Code

Non-zero return values indicate errors.

## **ADPCM Hardware Functions**

`ad_reset( void );`
Resets ADPCM hardware.

`ad_trans( unsigned char ovl_index, unsigned int sect_offset, unsigned char nb_sectors, unsigned int ad_addr );`
Transfer data from CDROM into ADPCM buffer. Read it from the overlay segment specified by '*ovl_index*', with sector offset (ie. multiples of 2048 bytes) of '*sect_offset*'. Read '*nb_sectors*' sectors and store data into ADPCM buffer starting from offset '*ad_addr*'. Non-zero return values indicate errors.

`ad_read( unsigned int ad_addr, unsigned char mode, unsigned int buf, unsigned int bytes );`
Read data from the ADPCM buffer. Read '*bytes*' bytes starting at offset '*ad_addr*' in ADPCM buffer. The mode '*mode*' determines the meaning of the '*buf*' offset.

    Valid modes        Meaning of 'buf'
    -----------        ----------------
    0                  Offset in the mapped ram.
    2-6                Directly into memory mapped to MMR #'mode'
    0xFF               Offset in video ram.

Non-zero return values indicate errors.

`ad_write( unsigned int ad_addr, unsigned char mode, unsigned int buf, unsigned int bytes );`
Write data into the ADPCM buffer. Write '*bytes*' bytes starting at offset '*ad_addr*' in ADPCM buffer. The mode '*mode*' determines the meaning of the '*buf*' offset.

    Valid modes        Meaning of 'buf'
    -----------        ----------------
    0                  Offset in the mapped ram.
    2-6                Directly into memory mapped to MMR #'mode'
    0xFF               Offset in video ram.

Non-zero return values indicate errors.

`ad_play( unsigned int ad_addr, unsigned int bytes, unsigned char freq, unsigned char mode );`
Play ADPCM sound from data loaded in the ADPCM buffer. Start playing from '*ad_addr*' offset in the ADPCM buffer, for '*bytes*' bytes. The frequency used for playback is computed from '*freq*' using the following formula : `real_frequency_in_khz = 32 / (16 - 'freq')`. Valid range for '*freq*' is 0-15. If bit 0 of 'mode' is on, values of '*ad_addr*', '*bytes*' and '*freq*' from the previous `ad_play()` call are reused. If bit 7 of '*mode*' is on, playback loops instead of stopping at the end of the range.

`ad_cplay( unsigned char ovl_index, unsigned int sect_offset, unsigned int nb_sectors, unsigned char freq );`
Compressed play - load and play ADPCM data in one operation.

`ad_stop( void );`
Stop playing ADPCM.

`ad_stat( void );`
Returns current ADPCM status. Return **FALSE(0)** if ADPCM playing isn't in progress else returns a non zero value.

## **Backup Ram Functions**

`bm_check( void );`
Return whether the backup ram is available.

`bm_format( void );`
Format the whole backup ram.  Does not format if data is already formatted. Actually, data isn't erased but the header data reports so. You should be able to find old data in the bram using raw reads in the backup ram bank but not through the usual HuC functions. Return backup ram status as defined in the hucc.h file.

`bm_free( void );`
Return the number of free bytes available for user data.  The amount required for the data header and the necessary 2-byte termination are already deducted from this amount.  The value returned is the number of bytes free for user data.

`bm_read( unsigned char *buffer, unsigned char *name, unsigned int offset, unsigned int length );`
Read '*nb*' bytes from file named '*name*' and put them into '*buf*'. I'm not sure whether the '*offset*' is relative to the buffer or the file ... Return backup ram status as defined in the hucc.h file.

`bm_write( unsigned char *buffer, unsigned char *name, unsigned int offset, unsigned int length );`
Write into the file named '*name*'. Data to write are in the buffer '*buf*' and '*nb*' bytes are written from offset '*offset*' in the buffer. Return backup ram status as defined in the hucc.h file.

`bm_delete( unsigned char *name );`
Delete BRAM entry with a given name

`bm_exist( unsigned char *name );`
Check whether a backup ram file exists. This returns **TRUE (!= 0)** if good; **FALSE (0)** if bad. The name structure is not just an ASCII name; it begins with a 2-byte "uniqueness ID" which is almost always 00 00, followed by 10 bytes of ASCII name - which should be padded with trailing spaces.

`bm_create( unsigned char *name, unsigned int length );`
Create a new file names '*name*' with a size of '*size*' bytes. Return backup ram status as defined in the hucc.h file.

## **ZX0 Compression Functions**

ZX0 "modern" format is not supported, because it costs an extra 4 bytes of code in this decompressor, and it runs slower.
Use Emmanuel Marty's SALVADOR ZX0 compressor which can be found here ...  https://github.com/emmanuel-marty/salvador

To create a ZX0 file to decompress to RAM
*salvador -classic <infile> <outfile>*

To create a ZX0 file to decompress to VRAM, using a 2KB ring-buffer in RAM
*salvador -classic -w 2048 <infile> <outfile>*

`zx0_to_ram( unsigned char *ram, char far *compressed );`
Decompress ZX0 compressed data directly to RAM.

`zx0_to_vdc( unsigned int vaddr, char far *compressed );`
Decompress ZX0 compressed data directly to VDC VRAM.

`zx0_to_sgx( unsigned int vaddr, char far *compressed );`
Decompress ZX0 compressed data directly to SuperGrafx VRAM.

## **SuperGrafx Functions**

`sgx_detect( void );`
Returns **TRUE (1)** if SuperGrafx hardware is detected; **FALSE(0)** if not.

### **SuperGrafx Video Functions**

`sgx_set_screen_size( unsigned char value );`
Set SuperGrafx virtual screen size. Same values as `set_screen_size()`.

`sgx_set_xres( unsigned int x_pixels, unsigned char blur_flag );`
Set SuperGrafx horizontal resolution. Same as `set_xres()` but for SuperGrafx hardware.

`sgx_cls( void );`
`sgx_cls( unsigned int tile );`
Clear the SuperGrafx screen. Same as `cls()` but for SuperGrafx hardware.

`sgx_vram_addr( unsigned char bat_x, unsigned char bat_y );`
Return the SuperGrafx VRAM address for the character at position (bat_x, bat_y).

`sgx_get_vram( unsigned int address );`
Read a 16-bit word from SuperGrafx VRAM at the specified address.

`sgx_put_vram( unsigned int address, unsigned int data );`
Write a 16-bit word to SuperGrafx VRAM at the specified address.

`sgx_load_vram( unsigned int vram, unsigned char __far *data, unsigned int num_words );`
Load data into SuperGrafx VRAM.

`sgx_far_load_vram( unsigned int vram, unsigned int num_words );`
Load SuperGrafx VRAM data from far memory.

`sgx_load_bat( unsigned int vram, unsigned char __far *data, unsigned char tiles_w, unsigned char tiles_h );`
Load a BAT into SuperGrafx VRAM.

`sgx_far_load_bat( unsigned int vram, unsigned char tiles_w, unsigned char tiles_h );`
Load SuperGrafx BAT data from far memory.

`sgx_load_sprites( unsigned int vram, unsigned char __far *data, unsigned int num_groups );`
Load sprite data into SuperGrafx VRAM.

`sgx_far_load_sprites( unsigned int vram, unsigned int num_groups );`
Load SuperGrafx sprite data from far memory.

`sgx_vram2vram( unsigned int vram_dst, unsigned int vram_src, unsigned int word_len );`
Copy data within SuperGrafx VRAM.

`vpc_set_ctl( unsigned int bits );`
Set VPC (Video Priority Controller) control register. Controls sprite and background priority settings.

`vpc_set_win1( unsigned int width );`
Set VPC window 1 width for split screen effects.

`vpc_set_win2( unsigned int width );`
Set VPC window 2 width for split screen effects.

### **SuperGrafx Tile and Map Functions**

`sgx_set_tile_address( unsigned int vram );`
Set the base VRAM address for SuperGrafx tile operations.

`sgx_set_tile_data( unsigned char __far *tiles, unsigned char num_tiles, unsigned char __far *palette_table, unsigned char tile_type );`
Define SuperGrafx tile data. Same as `set_tile_data()` but for SuperGrafx hardware.

`sgx_set_far_tile_data( unsigned char tile_bank, unsigned char *tile_addr, unsigned char num_tiles, unsigned char palette_table_bank, unsigned char *palette_table_addr, unsigned char tile_type );`
Set SuperGrafx tile data from far memory.

`sgx_load_tile( unsigned int vram );`
Load tile graphics data into SuperGrafx VRAM.

`sgx_set_map_data( unsigned char __far *map, unsigned char w, unsigned char h );`
Set SuperGrafx map data.

`sgx_set_far_map_data( unsigned char map_bank, unsigned char *map_addr, unsigned char w, unsigned char h );`
Set SuperGrafx map data from far memory.

`sgx_load_map( unsigned char bat_x, unsigned char bat_y, int map_x, int map_y, unsigned char tiles_w, unsigned char tiles_h );`
Load a map into SuperGrafx VRAM.

`sgx_map_get_tile( unsigned char map_x, unsigned char map_y );`
Get tile from SuperGrafx map data.

`sgx_map_put_tile( unsigned char map_x, unsigned char map_y, unsigned char tile );`
Put tile into SuperGrafx map data.

`sgx_put_tile( unsigned char tile, unsigned char bat_x, unsigned char bat_y );`
Put tile on SuperGrafx screen.

`sgx_set_map_pals( unsigned char __far *palette_table );`
Set SuperGrafx map palette table.

`sgx_set_map_tile_type( unsigned char tile_type );`
Set SuperGrafx map tile type.

`sgx_set_map_tile_base( unsigned int vram );`
Set SuperGrafx map tile base address.

### **SuperGrafx Font Functions**

`sgx_set_font_addr( unsigned int vram );`
Set SuperGrafx font address.

`sgx_set_font_pal( unsigned char palette );`
Set SuperGrafx font palette.

`sgx_load_font( char __far *font, unsigned char count, unsigned int vram );`
Load font into SuperGrafx VRAM.

`sgx_far_load_font( unsigned char count, unsigned int vram );`
Load SuperGrafx font from far memory.

`sgx_load_default_font( void );`
Load default font into SuperGrafx VRAM.

### **SuperGrafx Text Output Functions**

`sgx_putchar( unsigned char ascii );`
Output character to SuperGrafx screen.

`sgx_puts( unsigned char *string );`
Output string to SuperGrafx screen.

`sgx_printf( unsigned char *format );`
`sgx_printf( unsigned char *format, unsigned int vararg1 );`
`sgx_printf( unsigned char *format, unsigned int vararg1, unsigned int vararg2 );`
`sgx_printf( unsigned char *format, unsigned int vararg1, unsigned int vararg2, unsigned int vararg3 );`
`sgx_printf( unsigned char *format, unsigned int vararg1, unsigned int vararg2, unsigned int vararg3, unsigned int vararg4 );`
Formatted output to SuperGrafx screen. Same as `printf()` but for SuperGrafx hardware.

### **SuperGrafx Sprite Functions**

`sgx_init_satb( void );`
Initialize SuperGrafx Sprite Attribute Table (SATB).

`sgx_reset_satb( void );`
Reset SuperGrafx SATB, disabling all sprites.

`sgx_satb_update( void );`
Update SuperGrafx sprites on screen.

`sgx_spr_set( unsigned char num );`
Select SuperGrafx sprite number (0-63).

`sgx_spr_hide( void );`
Hide the current SuperGrafx sprite.

`sgx_spr_show( void );`
Show the current SuperGrafx sprite.

`sgx_spr_x( unsigned int value );`
Set x coordinate of current SuperGrafx sprite.

`sgx_spr_y( unsigned int value );`
Set y coordinate of current SuperGrafx sprite.

`sgx_spr_pattern( unsigned int vaddr );`
Set pattern address for current SuperGrafx sprite.

`sgx_spr_ctrl( unsigned char mask, unsigned char value );`
Set attributes of current SuperGrafx sprite (size, flipping).

`sgx_spr_pal( unsigned char palette );`
Set palette for current SuperGrafx sprite.

`sgx_spr_pri( unsigned char priority );`
Set priority for current SuperGrafx sprite.

`sgx_spr_get_x( void );`
Get x coordinate of current SuperGrafx sprite.

`sgx_spr_get_y( void );`
Get y coordinate of current SuperGrafx sprite.

### **SuperGrafx Split Screen Functions**

`sgx_scroll_split( unsigned char index, unsigned char screen_line, unsigned int bat_x, unsigned int bat_y, unsigned char display_flags );`
Create SuperGrafx split screen window.

`sgx_disable_split( unsigned char index );`
Disable SuperGrafx split screen window.

`sgx_disable_all_splits( void );`
Disable all SuperGrafx split screen windows.

## **Arcade Card Functions**

`ac_exists( void );`
Returns **TRUE (1)** if exists; **FALSE (0)** if not.



## **Debug and Test Functions**

These functions are primarily for debugging and testing purposes:

`dump_screen( void );`
Dump screen contents for testing purposes. Only available in TGEMU emulator.

`abort( void );`
Abort program execution. Only available in TGEMU emulator.

`exit( int value );`
Exit program with specified exit code. Only available in TGEMU emulator.

`__builtin_ffs( unsigned int value );`
"Find first set bit". Counts the number of bits in an int until it finds one that is set.

## **Special cases, for compatibility with existing apps ONLY, not to be used**

These functions are provided for compatibility with existing applications but should not be used in new code:

*Note: No deprecated functions are currently documented here.*

## **HUC Functions currently NOT Supported in HuCC (as of 25/09/04)**
`get_font_pal`
`get_font_addr`
`strncpy`
`get_tile`
`gfx_setbgpal`
`gfx_point`
`spr_get_pal`
`spr_get_pattern`
`set_joy_callback`
`mouse_exists`
`mouse_enable`
`mouse_disable`
`mouse_x`
`mouse_y`
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
`cd_numtrk`
`cd_trktype`
`cd_trkinfo`
