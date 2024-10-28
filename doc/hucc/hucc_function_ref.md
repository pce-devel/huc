# HuCC Function Reference

## **Video Functions**

`cls( void );`
`cls( unsigned int tile );`
Clear the entire screen. Without parameters the screen is filled with a space character otherwise it's filled with the bat-value given as the argument.

`disp_on( void );`
Enable display output.

`disp_off( void );`
Blank/Turn off the display output.

`vsync( void );`
`vsync( unsigned char count );`
Synchronize the program to the video Vertical Blanking Signal (VBL), which is around 1/60th of a second.

Without parameters this function returns as soon as a VBL signal has been received, else your program will be synchronized to the number of frames you requested. So for example to run your game at 20fps, just use vsync(3) at the end of your game loop.

`set_256x224( void );`
Sets the screen to 32 characters wide and 28 characters tall.

`set_screen_size( unsigned char value );`
Change the virtual screen size. By default the startup code initializes a virtual screen of 64 characters wide and 32 character tall, but other values are possible, namely : 32x32, 128x32, 32x64, 64x64, or 128x64. The larger the virtual screen is the less video memory you will have for your graphics (font, tiles, sprites).

`set_xres( unsigned int x_pixels );`
Change the horizontal resolution to '*x_pixels*' (in pixels). This changes the video controller's registers to display more pixels on the screen; it does not affect any virtual calculations.

`set_xres( unsigned int x_pixels, unsigned char blur_flag );`
Change the horizontal resolution to '*x_pixels*' (in pixels). This changes the video controller's registers to display more pixels on the screen; it does not affect any virtual calculations. 

`'blur_flag'` indicates what kind of smoothing will be used: **XRES_SHARP** or **XRES_SOFT** (default). 

Note: The 5MHz dot clock will be used up to a horizontal resolutions of 268 pixels. The 7MHz dot clock will be used up to 356 pixels and the 10MHz dot clock will be used beyond this. The maximum visible resolution seems to be around 512 pixels.

`vram_addr( unsigned char bat_x, unsigned char bat_y );`     

`get_vram( unsigned int address );`

`put_vram( unsigned int address, unsigned int data );`

`peek( unsigned int addr );`
`peekw( unsigned int addr );`
Read the contents of memory location 'addr'. peek() is char-sized access, whereas peekw() is word-sized.

`poke( unsigned int addr, unsigned char with );`
`pokew( unsigned int addr, unsigned int with );`
Write '*val*' value at memory location 'addr'. `poke()` is char-sized access, whereas `pokew()` is word-sized. 
This function can be used to access the hardware I/O ports located at 0x0000 to 0x1FFF.

## **Joypad Functions**

`joy( unsigned int which );`
Return the current status of joypad number 'which'. This function is 6 button compliant.

`joytrg( unsigned int which );`
Return informations about newly pressed buttons and directions of joypad number '*which*'. But beware of this function, these informations are refreshed every frame, so if your game loop is not fast enough you could miss some keypresses.

## **Color and Palette Functions**

`set_color( unsigned int index, unsigned int value );`
Set the specified color `index` (0-511) to the given rgb-value.

`set_color_rgb( unsigned int index, unsigned char r, unsigned char g, unsigned char b );`
Set the specified color to the given rgb component values. This function is easier to use than `set_color()`, but it is slower.

`get_color( unsigned int index );`
Retrieve the rgb-value of the specified color.

`load_palette( unsigned char palette, unsigned char __far *data, unsigned int num_palettes );`
Load one or more 16-color palette-blocks at once. `palette` is the index of the first block (0-31) to load, and `num_palettes` the number of blocks. This function can be used to load palettes defined using `#defpal` or included with `#incpal` directives.

`set_sprpal( unsigned char palette, unsigned char __far *data );`
`set_sprpal( unsigned char palette, unsigned char __far *data, unsigned int num_palettes );`
Exactly the same function as `load_palette()`, but this function offers direct access to sprite palette-blocks. Sprite palette-blocks are standard block number 16 to 31, but with this function you can simply access them with indexes 0 to 15. This function and the `set_bgpal()` function make sprite and character palette-block manipulation easier; with them you don't have to know the real block indexes. Without the third arguments, the function loads only one block.

`set_bgpal( unsigned char palette, unsigned char __far *data );`
`set_bgpal( unsigned char palette, unsigned char __far *data, unsigned int num_palettes );`
Exactly the same function has `load_palette()`, but this function offers direct access to charachter palette-blocks. Character palette-blocks are standard block number 16 to 31, but with this function you can simply access them with indexes 0 to 15. This function and the `set_sprpal()` function make sprite and character palette-blocks manipulation easier; with them you don't have to know the real block indexes. Without the third arguments, the function loads only one block.

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

`srand( unsigned char seed );`
Change the random seed. You can use this function to improve randomness by giving a value based on when the player presses start for the first time, for example.

`rand( void );`
Return a 16-bit random number.

`rand8( void );`

`random8( unsigned char limit );`

`random( unsigned char limit );`
Return a random value between 0 and A-1.

## **Tile & Map Functions**
`set_tile_address( unsigned int vram );`

`set_tile_data( unsigned char __far *tiles, unsigned char num_tiles, unsigned char __far *palette_table, unsigned char tile_type );`
Define an array of tile to be used by all the tile and map functions. `tile_data` is the address of the tile graphics data in memory, `nb_tile` is the number of tile (max. 256), and `pal_ref` is the address of a palette-index array to use for tiles; each tile has its own palette index attached to it (note that palette indexes must be shifted to the left by four bits, ie. 0x40 for palette index 4).

The new 1-parameter form is used together with `#inctile_ex()` or `#incchr_ex()` directives, where all other relevant data is already conveyed in the `#incxxx_ex()` directive. Also, this form automatically recognizes whether it is being used with 8x8 (`incchr_ex`) or 16x16 (`inctile_ex`) tiles.

`load_vram( unsigned int vram, unsigned char __far *data, unsigned int num_words );`
Generic function to load data (BAT, CG, sprites) in video memory, at address `vram`. '*num_words*' is the number of 16-bit words to load.

`load_tile( unsigned int vram );`
Load tile graphics data in video memory, at address '*vram*'. You must first have defined a tile array with `set_tile_data()` before using this function.

`load_bat( unsigned int vram, unsigned char __far *data, unsigned char tiles_w, unsigned char tiles_h );`
Load a rectangular character attribute table (BAT) of width 'w' and of height 'h' in video memory at address '*vram*'.

`put_tile( unsigned char tile, unsigned char bat_x, unsigned char bat_y );`
Put individual tiles on the screen, either directly at video memory location '*vaddr*', or at screen coordinates 'x/y' (in tile unit). '*tile*' is a tile index in the tile array defined by the most recent call to `set_tile_data()`.

`map_put_tile( unsigned char map_x, unsigned char map_y, unsigned char tile );`
Modifies the map data (sets a map element to a new tile ID), but works only when the map is stored in RAM - ie. a Super CDROM game which is loaded into RAM, and executes there. 'x' and 'y' are specified in the same units as `map_get_tile()` (ie. pixels, not tiles).

`map_get_tile( unsigned char map_x, unsigned char map_y );`
Return the tile index as defined in the tile array used in the most recent call to `set_tile_data()`. The 'x/y' argument is in pixel units, unlike the put_tile functions and thus this function is ideal for colision routines.

`load_map( unsigned char bat_x, unsigned char bat_y, int map_x, int map_y, unsigned char tiles_w, unsigned char tiles_h );`
Load a part of a map on the screen. 'sx' and 'sy' are screen coordinates (in tile unit; 16 pixels), 'mx' and 'my' are position in the map, and 'w' and 'h' are respectively the number of tile-index to load horizontaly and verticaly. This function doesn't do any screen clipping, so you must not pass incorrect or too big screen coordinates to it as that would corrupt the video memory.

`set_map_data( unsigned char __far *map, unsigned char w, unsigned char h );`

`set_map_pals( unsigned char __far *palette_table );`

`set_map_tile_type( unsigned char tile_type );`

`set_map_tile_base( unsigned int vram );`

`load_background( unsigned char __far *tiles, unsigned char __far *palettes, unsigned char __far *bat, unsigned char w, unsigned char h );`
This function is an all-in-one function, it is used to display an entire background image on the screen, like a game title image. It will load BG character data, it will load the palette, and finaly if will load the BAT. Use it with directives *#incchr*, *#incpal* and *#incbat* to manage the different types of data. The BG character data will be stored at address 0x1000 to 0x5000 in video memory.

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

`set_font_color( unsigned char foreground, unsigned char background );`
Set the default font foreground and background colors (colors range from 0 to 15). Changes won't take effect immediately, you must re-load the font by calling `load_default_font()`.

`load_default_font( void );  char num, int vaddr)`
Loads a default font in video memory. Without parameters the first default font is loaded just above the BAT in video memory; usualy it's address 0x800. Otherwise you can select the font number, and eventualy the address in video memory. In its current implementation the library supports only one default font, but in the future more fonts could be made available.

## **Text Output Functions**
All the text output functions have two forms, one where you directly specify the video memory address, and another one where you specify x/y coordinates (in character units). The second form is a bit slower but more user-friendly.

`put_digit( unsigned char digit, unsigned char bat_x, unsigned char bat_y );`
Output a digit character '0'-'9' given its numeric value. Hexa digits ('A'-'F') are also supported, a value of 10 will output 'A', a value of 11 will output 'B', and so on...

`put_char( unsigned char digit, unsigned char bat_x, unsigned char bat_y );`
Output an ASCII character.          

`put_hex( unsigned int number, unsigned char length, unsigned char bat_x, unsigned char bat_y );`
Output an hexa number. The 'length' argument is used to format the number. As many as 'length' digit(s) will be displayed. If the number has less than 'lgenth' digit(s), blank spaces will be added to its left. 

`put_number( unsigned int number, unsigned char length, unsigned char bat_x, unsigned char bat_y );`
Output a signed number. The 'length' argument is used to format the number. As many as 'length' digit(s) will be displayed. If the number has less than 'lgenth' digit(s), blank spaces will be added to its left. If the number is negative, a '-' sign is added.

`put_string( unsigned char *string, unsigned char bat_x, unsigned char bat_y );`
Output a null terminated ASCII string.

## **Graphics Functions**
`gfx_init( unsigned int start_vram_addr );`
Initialize screen to point to sequential graphics tiles, located starting at address '*start_vram_addr*' in VRAM.

`gfx_clear( unsigned int start_vram_addr );`
Clear graphical screen. Starting at address '*start_vram_addr*' in VRAM, this function sets sequential tiles in VRAM to all zeroes, with a size based on the virtual map.

`gfx_plot( unsigned int x, unsigned int y, char color );`
Set a pixel at (x,y) to color listed. '*color*' should be avalue between 0 and 15.

`gfx_line( unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2, unsigned char color );`
Draw a line from (x1, y1) to (x2, y2) in color listed. '*color*' should be a value between 0 and 15.

## **Scroll Functions**

`scroll( unsigned char num, unsigned int x, unsigned int y, unsigned char top, unsigned char bottom, unsigned char disp );`
Define screen window '*num*'. Up to four window can be defined. '*top*' and '*bottom*' are the screen top and bottom limits of the window (limits are included in the window area). '*disp*' controls the type of the window, if bit 7 is set background graphics will be displayed in this window, and if bit 6 is set sprites will also be displayed. If none of these bits are set the window will stay blank. 'x' and 'y' are the top-left coordinates of the area in the virtual screen that will be displayed in the window.

`scroll_disable( unsigned char num );`
Disable scrolling for the screen window '*num*'.

`scroll_split( unsigned char index, unsigned char top, unsigned int x, unsigned int y, unsigned char disp );`

`disable_split( unsigned char index );`

## **Sprite Functions**

`init_satb( void );`
Initialize the internal Sprite Attribute Table (SATB) used by the library to handle sprites. This function must be called before any other sprite function is called.

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
Set different attributes of the current sprite. With this function you can change the sprite size (16x16, 32x32, ...) and the sprite orientation (horizontal/vertical flipping).

`spr_pal( unsigned char palette );`
Set the palette-block index (0-15) of the current sprite.

`spr_pri( unsigned char priority );`
Set the priority of the current sprite. '0' will make it appear behind the background (through color 0), '1' will make it appear in front of the background.

`spr_get_x( void );`
Return the x coordinate of the current sprite.

`spr_get_y( void );`
Return the y coordinate of the current sprite.


## **String Manipulation functions**

`strcpy( char *destination, char *source );`
Copy the source information to the destination area, as in ANSI 'C'.

`strcat( char *destination, char *source );`
Concatenate source string onto the end od destination string, as in ANSI 'C'.

`strlen( char *source );`
Computes the length of the specified string, up to but not including the terminating null character.

`strlcpy( char *destination, char *source, unsigned char size );`
Copy the source information to the destination area, as in ANSI 'C'.

`strlcat( char *destination, char *source, unsigned char size );`
Concatenate source string onto the end od destination string, as in ANSI 'C'.

`memcpy( unsigned char *destination, unsigned char *source, unsigned int count );`
Copy the source information to the destination area, as in ANSI 'C'.

`mempcpy( unsigned char *destination, unsigned char *source, unsigned int count );`
Copy the source information to the destination area, as in ANSI 'C'.

`memset( unsigned char *destination, unsigned char value, unsigned int count );`

`strcmp( char *destination, char *source );`
Compare the source information against the destination information, as in ANSI 'C'.

`strncmp( char *destination, char *source, unsigned int count );`
Compare the source information against the destination information, as in ANSI 'C'.

`memcmp( unsigned char *destination, unsigned char *source, unsigned int count );`
Compare the source information against the destination information, as in ANSI 'C'.

## **CD Overlay functions**

`cd_execoverlay( unsigned char ovl_index );`
Loads a program overlay specified by '*ovl_index*', and execute it. If an error occurs during loading, the previous context (ie. the overlay running until that moment) is reloaded and an error value is returned to the program.

## **CDROM functions**

`cd_boot( void );`

`cd_getver( void );`
Returns CDROM system card version number in BCD. (ie. Japanese Super System card = 0x0300, American Duo = 0x301)

`cd_reset( void );`
Reset the CDROM drive, and stop the motor

`cd_pause( void );`
Pause the CDROM drive during play of an audio track. Probably most useful if the player pauses the game in the middle of a level which is synchronized to music.

`cd_unpause( void );`
Continue the CDROM audio audio track after pause.

`cd_fade( unsigned char type );`

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

## **ZX0 COMPRESSION FUNCTIONS**
ZX0 "modern" format is not supported, because it costs an extra 4 bytes of code in this decompressor, and it runs slower.
Use Emmanuel Marty's SALVADOR ZX0 compressor which can be found here ...  https://github.com/emmanuel-marty/salvador

To create a ZX0 file to decompress to RAM
*salvador -classic <infile> <outfile>*

To create a ZX0 file to decompress to VRAM, using a 2KB ring-buffer in RAM
*salvador -classic -w 2048 <infile> <outfile>*

`zx0_to_vdc( unsigned int vaddr, char far *compressed );`
`zx0_to_sgx( unsigned int vaddr, char far *compressed );`

## **SuperGrafx Equivalent Functions**

`sgx_detect( void );`
Returns **TRUE (1)** if exists; **FALSE(0)** if not.

`sgx_set_screen_size( unsigned char value );`
`sgx_set_xres( unsigned int x_pixels, unsigned char blur_flag );`

`sgx_vram_addr( unsigned char bat_x, unsigned char bat_y );`
Simple function to return the screen video memory address of the character located at position x/y.

`sgx_get_vram( unsigned int address );`

`sgx_put_vram( unsigned int address, unsigned int data );`

`sgx_set_tile_address( unsigned int vram );`

`sgx_set_tile_data( unsigned char __far *tiles, unsigned char num_tiles, unsigned char __far *palette_table, unsigned char tile_type );`

`sgx_load_vram( unsigned int vram, unsigned char __far *data, unsigned int num_words );`

`sgx_load_tile( unsigned int vram );`

`sgx_load_bat( unsigned int vram, unsigned char __far *data, unsigned char tiles_w, unsigned char tiles_h );`

`sgx_set_map_data( unsigned char __far *map, unsigned char w, unsigned char h );`

`sgx_load_map( unsigned char bat_x, unsigned char bat_y, int map_x, int map_y, unsigned char tiles_w, unsigned char tiles_h );`

`sgx_map_get_tile( unsigned char map_x, unsigned char map_y );`

`sgx_map_put_tile( unsigned char map_x, unsigned char map_y, unsigned char tile );`

`sgx_put_tile( unsigned char tile, unsigned char bat_x, unsigned char bat_y );`

`sgx_set_map_pals( unsigned char __far *palette_table );`

`sgx_set_map_tile_type( unsigned char tile_type );`

`sgx_set_map_tile_base( unsigned int vram );`

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

`sgx_scroll_split( unsigned char index, unsigned char top, unsigned int x, unsigned int y, unsigned char disp );`

`sgx_disable_split( unsigned char index );`

## **Arcade Card Functions**

`ac_exists( void );`
Returns **TRUE (1)** if exists; **FALSE (0)** if not.



## **Special cases, for compatibility with existing apps ONLY, not to be used**
`__builtin_ffs`
"find first set bit". It counts the number of bits in an int until it finds one that is set.

`abort()`

`error()`

## **HUC Functions currently NOT Supported in HuCC (as of 09/29/24)**
`get_font_pal`
`get_font_addr`
`put_raw`
`strncpy`
`get_tile`
`set_map_data`
`gfx_setbgpal`
`gfx_point`
`load_sprites`
`spr_get_pal`
`spr_get_pattern`
`set_joy_callback`
`get_joy_events`
`clear_joy_events`
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
