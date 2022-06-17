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
  // **************************************************************************
  //
  // System Card's Zero Page Variables (6502-style zero-page addresses).
  //
  .label __bp = $ec
  .label __si = $ee
  .label __di = $f0
  .label __cdi_b = $f2
  .label __vdc_crl = $f3
  .label __vdc_crh = $f4
  .label __irq_vec = $f5
  .label __vdc_sr = $f6
  .label __vdc_reg = $f7
  .label __ax = $f8
  .label __al = $f8
  .label __ah = $f9
  .label __bx = $fa
  .label __bl = $fa
  .label __bh = $fb
  .label __cx = $fc
  .label __cl = $fc
  .label __ch = $fd
  .label __dx = $fe
  .label __dl = $fe
  .label __dh = $ff
  .label __temp = 0
  .label __bank = 2
  // **************************************************************************
  //
  // System Card's Main RAM Variables.
  //
  .label irq2_hook = $2200
.segment Code
__start: {
    // /mnt/huc/huc/examples/asm/elmer/kickc/pcengine.h:403
    lda #<0
    sta.z __bp
    sta.z __bp+1
    // /mnt/huc/huc/examples/asm/elmer/kickc/pcengine.h:404
    sta.z __si
    sta.z __si+1
    // /mnt/huc/huc/examples/asm/elmer/kickc/pcengine.h:405
    sta.z __di
    sta.z __di+1
    // /mnt/huc/huc/examples/asm/elmer/kickc/pcengine.h:406
    sta.z __cdi_b
    // /mnt/huc/huc/examples/asm/elmer/kickc/pcengine.h:407
    sta.z __vdc_crl
    // /mnt/huc/huc/examples/asm/elmer/kickc/pcengine.h:408
    sta.z __vdc_crh
    // /mnt/huc/huc/examples/asm/elmer/kickc/pcengine.h:409
    sta.z __irq_vec
    // /mnt/huc/huc/examples/asm/elmer/kickc/pcengine.h:410
    sta.z __vdc_sr
    // /mnt/huc/huc/examples/asm/elmer/kickc/pcengine.h:411
    sta.z __vdc_reg
    // /mnt/huc/huc/examples/asm/elmer/kickc/pcengine.h:413
    sta.z __ax
    sta.z __ax+1
    // /mnt/huc/huc/examples/asm/elmer/kickc/pcengine.h:414
    sta.z __al
    // /mnt/huc/huc/examples/asm/elmer/kickc/pcengine.h:415
    sta.z __ah
    // /mnt/huc/huc/examples/asm/elmer/kickc/pcengine.h:417
    sta.z __bx
    sta.z __bx+1
    // /mnt/huc/huc/examples/asm/elmer/kickc/pcengine.h:418
    sta.z __bl
    // /mnt/huc/huc/examples/asm/elmer/kickc/pcengine.h:419
    sta.z __bh
    // /mnt/huc/huc/examples/asm/elmer/kickc/pcengine.h:421
    sta.z __cx
    sta.z __cx+1
    // /mnt/huc/huc/examples/asm/elmer/kickc/pcengine.h:422
    sta.z __cl
    // /mnt/huc/huc/examples/asm/elmer/kickc/pcengine.h:423
    sta.z __ch
    // /mnt/huc/huc/examples/asm/elmer/kickc/pcengine.h:425
    sta.z __dx
    sta.z __dx+1
    // /mnt/huc/huc/examples/asm/elmer/kickc/pcengine.h:426
    sta.z __dl
    // /mnt/huc/huc/examples/asm/elmer/kickc/pcengine.h:427
    sta.z __dh
    // /mnt/huc/huc/examples/asm/elmer/kickc/pcengine.h:429
    sta.z __temp
    sta.z __temp+1
    // /mnt/huc/huc/examples/asm/elmer/kickc/pcengine.h:430
    sta.z __bank
    // /mnt/huc/huc/examples/asm/elmer/kickc/pcengine.h:441
    sta irq2_hook
    sta irq2_hook+1
    jsr main
    rts
}
main: {
    // /mnt/huc/huc/examples/asm/elmer/kickc/kickc.h:51
    jsr init_256x224 
    // /mnt/huc/huc/examples/asm/elmer/rom-kickc-hello/hello.c:130
    lda #<($40*$20/$10+$10)*$10
    sta.z __di
    lda #>($40*$20/$10+$10)*$10
    sta.z __di+1
    lda #$ff
    sta.z __al
    lda #0
    sta.z __ah
    lda #$10+$60
    sta.z __bl
    lda #<my_font
    sta.z __si
    lda #>my_font
    sta.z __si+1
    lda #$ff&my_font>>$17
    sta.z __bank
    ldy.z __bank 
    jsr dropfnt8x8_vdc 
    // /mnt/huc/huc/examples/asm/elmer/rom-kickc-hello/hello.c:133
    lda #0
    sta.z __al
    lda #1
    sta.z __ah
    lda #<cpc464_colors
    sta.z __si
    lda #>cpc464_colors
    sta.z __si+1
    lda #$ff&cpc464_colors>>$17
    sta.z __bank
    ldy.z __bank 
    jsr load_palettes 
    // /mnt/huc/huc/examples/asm/elmer/rom-kickc-hello/hello.c:136
    lda #<$c*$40+$a
    sta.z __di
    lda #>$c*$40+$a
    sta.z __di+1
    jsr vdc_di_to_mawr 
    // /mnt/huc/huc/examples/asm/elmer/rom-kickc-hello/hello.c:139
    // Display the classic "hello, world" on the screen.
    lda #<message1
    sta.z print_message.string
    lda #>message1
    sta.z print_message.string+1
    jsr print_message
    // /mnt/huc/huc/examples/asm/elmer/rom-kickc-hello/hello.c:142
    lda #<$e*$40+8
    sta.z __di
    lda #>$e*$40+8
    sta.z __di+1
    jsr vdc_di_to_mawr 
    // /mnt/huc/huc/examples/asm/elmer/rom-kickc-hello/hello.c:145
    // Display a 2nd message on the screen.
    lda #<message2
    sta.z print_message.string
    lda #>message2
    sta.z print_message.string+1
    jsr print_message
    // /mnt/huc/huc/examples/asm/elmer/kickc/kickc.h:109
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
