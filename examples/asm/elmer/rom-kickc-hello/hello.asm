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
.cpu _huc6280
  ; PCEAS setup for assembling a KickC program.
	.include "kickc.asm"
	.kickc

  // Data (Read/Write) High
  .label VDC_DW = $202
  // base pointer
  .label __si = $ee
  // source address
  .label __di = $f0
  .label __al = $f8
  .label __ah = $f9
  .label __bl = $fa
  // CORE(not TM) library variable!
  .label __bank = 2
.segment Code
__start: {
    jsr main
    rts
}
main: {
    .const dropfnt8x8_vdc1_vram = ($40*$20/$10+$10)*$10
    .const dropfnt8x8_vdc1_count = $10+$60
    .const dropfnt8x8_vdc1_plane2 = $ff
    .const dropfnt8x8_vdc1_plane3 = 0
    .const load_palette1_palnum = 0
    .const load_palette1_palcnt = 1
    .const vdc_di_to_mawr1_vram = $c*$40+$a
    .const vdc_di_to_mawr2_vram = $e*$40+8
    // /mnt/huc/huc/examples/asm/elmer/kickc/kickc.h:51
    jsr init_256x224 
    // /mnt/huc/huc/examples/asm/elmer/kickc/kickc.h:56
    lda #<my_font
    sta.z __si
    lda #>my_font
    sta.z __si+1
    // /mnt/huc/huc/examples/asm/elmer/kickc/kickc.h:57
    lda #$ff&my_font>>$17
    sta.z __bank
    // /mnt/huc/huc/examples/asm/elmer/kickc/kickc.h:58
    lda #<dropfnt8x8_vdc1_vram
    sta.z __di
    lda #>dropfnt8x8_vdc1_vram
    sta.z __di+1
    // /mnt/huc/huc/examples/asm/elmer/kickc/kickc.h:59
    lda #dropfnt8x8_vdc1_plane2
    sta.z __al
    // /mnt/huc/huc/examples/asm/elmer/kickc/kickc.h:60
    lda #dropfnt8x8_vdc1_plane3
    sta.z __ah
    // /mnt/huc/huc/examples/asm/elmer/kickc/kickc.h:61
    lda #dropfnt8x8_vdc1_count
    sta.z __bl
    // /mnt/huc/huc/examples/asm/elmer/kickc/kickc.h:62
    jsr dropfnt8x8_vdc 
    // /mnt/huc/huc/examples/asm/elmer/kickc/kickc.h:78
    lda #<cpc464_colors
    sta.z __si
    lda #>cpc464_colors
    sta.z __si+1
    // /mnt/huc/huc/examples/asm/elmer/kickc/kickc.h:79
    lda #$ff&cpc464_colors>>$17
    sta.z __bank
    // /mnt/huc/huc/examples/asm/elmer/kickc/kickc.h:80
    lda #load_palette1_palnum
    sta.z __al
    // /mnt/huc/huc/examples/asm/elmer/kickc/kickc.h:81
    lda #load_palette1_palcnt
    sta.z __ah
    // /mnt/huc/huc/examples/asm/elmer/kickc/kickc.h:82
    jsr load_palettes 
    // /mnt/huc/huc/examples/asm/elmer/kickc/kickc.h:67
    lda #<vdc_di_to_mawr1_vram
    sta.z __di
    lda #>vdc_di_to_mawr1_vram
    sta.z __di+1
    // /mnt/huc/huc/examples/asm/elmer/kickc/kickc.h:68
    jsr vdc_di_to_mawr 
    // /mnt/huc/huc/examples/asm/elmer/rom-kickc-hello/hello.c:139
    // Display the classic "hello, world" on the screen.
    lda #<message1
    sta.z print_message.string
    lda #>message1
    sta.z print_message.string+1
    jsr print_message
    // /mnt/huc/huc/examples/asm/elmer/kickc/kickc.h:67
    lda #<vdc_di_to_mawr2_vram
    sta.z __di
    lda #>vdc_di_to_mawr2_vram
    sta.z __di+1
    // /mnt/huc/huc/examples/asm/elmer/kickc/kickc.h:68
    jsr vdc_di_to_mawr 
    // /mnt/huc/huc/examples/asm/elmer/rom-kickc-hello/hello.c:145
    // Display a 2nd message on the screen.
    lda #<message2
    sta.z print_message.string
    lda #>message2
    sta.z print_message.string+1
    jsr print_message
    // /mnt/huc/huc/examples/asm/elmer/kickc/kickc.h:73
    jsr set_dspon 
  wait_vsync1:
    // /mnt/huc/huc/examples/asm/elmer/kickc/kickc.h:30
    jsr wait_vsync 
    jmp wait_vsync1
}
// *************************************************************************
// *************************************************************************
//
// print_message() - Write an ASCII text string directly to the VDC.
//
// void print_message(__zp($40) const char *string)
print_message: {
    .label string = $40
    ldy #0
  __b1:
    // /mnt/huc/huc/examples/asm/elmer/rom-kickc-hello/hello.c:103
    lda (string),y
    iny
    cmp #0
    bne __b2
    // /mnt/huc/huc/examples/asm/elmer/rom-kickc-hello/hello.c:106
    rts
  __b2:
    // /mnt/huc/huc/examples/asm/elmer/rom-kickc-hello/hello.c:104
    clc
    adc #$40*$20/$10
    sta VDC_DW
    lda #0
    sta VDC_DW+1
    jmp __b1
}
.segment zp
  random: .fill 4, 0
.segment code
.encoding "ascii"
  message1: .text "hello, world"
  .byte 0
  message2: .text "welcome to KickC"
  .byte 0
.segment data
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
  .align 2
  cpc464_colors: .word 0, 1, $1b2, $1b2, 2, $4c, $169, $1b2, 0, 0, 0, 0, 0, 0, 0, 0
// **************************************************************************
// **************************************************************************
//
// Errr ... it's the font data.
//
// This is the syntax for using a PCEAS ".incbin" to fill a C array.
//
my_font:
.incbin "font8x8-ascii-bold-short.dat" 
