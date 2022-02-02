// **************************************************************************
// **************************************************************************
//
// hello.c
//
// "hello, world" example of using KickC on the PC Engine.
//
// Copyright John Brandwood 2021.
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
// **************************************************************************
// **************************************************************************



// **************************************************************************
// **************************************************************************
//
// Some important #pragma settings for using KickC on the PCE.
//

// Set the target architecture.

#pragma target(pce)

// Use the __varcall calling method.
//
// This avoids passing parameters in registers, which is currently needed
// because our PCE call-trampolines use the A register for bank switching.

#pragma calling(__varcall)

// Put all C data/variables in the BSS section.
//
// This should *always* be set except for *small* sections of C code where
// you're specifically declaring some initialized data or graphics that go
// into the CODE or DATA sections.
//
// On the PCE, there is room for about 8KB of permanently-mapped const data
// in the CODE section, and the DATA section is reserved for graphics/sound
// data that gets banked into MPR3 & MPR4 as needed by the library routines
// which use far-data pointers.

#pragma data_seg(bss)

// Prefer using main memory for variables instead of zero-page?
//
// This can be set if the compiler runs out of ZP space.

// #pragma var_model(mem)



// **************************************************************************
// **************************************************************************
//
// Include the basic PCE hardware definitions for KickC.
//

#include "pcengine.h"
#include "kickc.h"



// **************************************************************************
// **************************************************************************
//
// Prototypes for calling ASM library functions.
//
// Now *something* actually has to pull in the ASM code that these refer to!
//




// **************************************************************************
// **************************************************************************
//
// Create some equates for a simple VRAM layout, with a 64*32 BAT, followed
// by the SAT, then followed by the tiles for the ASCII character set.
//

#define BAT_SIZE	(64 * 32)
#define SAT_ADDR	BAT_SIZE		// SAT takes 16 tiles of VRAM.
#define CHR_ZERO	(BAT_SIZE / 16)		// 1st tile # after the BAT.
#define CHR_0x10	(CHR_ZERO + 16)		// 1st tile # after the SAT.
#define CHR_0x20	(CHR_ZERO + 32)		// ASCII ' ' CHR tile #.



// *************************************************************************
// *************************************************************************
//
// print_message() - Write an ASCII text string directly to the VDC.
//

void print_message (const char * string) {
	byte i = 0;
	char c;
	while ((c = string[i++]) != 0) {
		*VDC_DW = c + CHR_ZERO;
	}
}



// *************************************************************************
// *************************************************************************
//
// main() - This is executed after the initialization in "kickc.inc".
//
// Force the message into the ".code" section, so that it doesn't need to
// be paged into memory.
//

#pragma data_seg(code)
const char message1 [] = "hello, world";
const char message2 [] = "welcome to KickC";
#pragma data_seg(bss)

void main()
{
	// Initialize VDC & VRAM.
	init_256x224();

	// Upload the font to VRAM.
	dropfnt8x8_vdc( my_font, (CHR_0x10 * 16), 16+96, 0xFF, 0x00 );

	// Upload the palette data to the VCE.
	load_palette( 0, cpc464_colors, 1 );

	// Destination VRAM address.
	__di_to_vdc( (12*64 + 10) );

	// Display the classic "hello, world" on the screen.
	print_message( message1 );

	// Destination VRAM address.
	__di_to_vdc( (14*64 + 8) );

	// Display a 2nd message on the screen.
	print_message( message2 );

	// Turn on the BG & SPR layers, then wait for a soft-reset.
	set_dspon();
	for (;;) {
		wait_vsync();
	}
}



// **************************************************************************
// **************************************************************************
//
// Put the banked-data into the DATA section ...
//
// This is used for C data that is banked into MPR3 & MPR4 as needed, and
// which library routines use 24bit far-pointers for.
//

#pragma data_seg(data)



// **************************************************************************
// **************************************************************************
//
// cpc464_colors - Palette data (a blast-from-the-past!)
//
//  0 = transparent
//  1 = dark blue shadow
//  2 = white font
//
//  4 = dark blue background
//  5 = light blue shadow
//  6 = yellow font
//

unsigned int __align(2) cpc464_colors[] = {
	0x0000,0x0001,0x01B2,0x01B2,0x0002,0x004C,0x0169,0x01B2,
	0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,
};



// **************************************************************************
// **************************************************************************
//
// Errr ... it's the font data.
//
// This is the syntax for using a PCEAS ".incbin" to fill a C array.
//

char my_font [] = kickasm {{ .incbin "font8x8-ascii-bold-short.dat" }};
