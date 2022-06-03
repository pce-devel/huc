// **************************************************************************
// **************************************************************************
//
// shmup.c
//
// "hello, world" example of using KickC on the PC Engine.
//
// Sample game for HuC, using new language features (and NOT SimpleTracker).
// Based on the sample game at http://obeybrew.com/tutorials.html
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

  .const SIZEOF_STRUCT_SHIP = 7
  .const SIZEOF_STRUCT_BULLET = 5
  .const OFFSET_STRUCT_SHIP_ACTIVE = 4
  .const OFFSET_STRUCT_SHIP_VX = 5
  .const OFFSET_STRUCT_SHIP_VY = 6
  .const OFFSET_STRUCT_SHIP_Y = 2
  .const OFFSET_STRUCT_BULLET_ACTIVE = 4
  .const OFFSET_STRUCT_BULLET_Y = 2
  // base pointer
  .label __si = $ee
  // source address
  .label __di = $f0
  // shadow of VDC register index
  .label __ax = $f8
  .label __al = $f8
  .label __ah = $f9
  .label __bx = $fa
  .label __bl = $fa
  .label __cl = $fc
  // CORE(not TM) library variable!
  .label __bank = 2
  // 1
  .label joynow = $2228
  // 5	officially called joy
  .label joytrg = $222d
.segment Code
__start: {
    // /mnt/huc/huc/examples/asm/elmer/rom-kickc-shmup/shmup.c:177
    lda #<0
    sta frames
    sta frames+1
    // /mnt/huc/huc/examples/asm/elmer/rom-kickc-shmup/shmup.c:178
    sta score
    sta score+1
    sta hiscore
    sta hiscore+1
    jsr main
    rts
}
// **************************************************************************
// **************************************************************************
//
// main()
//
main: {
    .const load_palette1_palnum = $10
    .const load_palette1_palcnt = 1
    .const load_palette2_palnum = $11
    .const load_palette2_palcnt = 1
    .const load_palette3_palnum = $12
    .const load_palette3_palcnt = 1
    .const load_palette4_palnum = $13
    .const load_palette4_palcnt = 1
    .const load_palette5_palnum = 0
    .const load_palette5_palcnt = $10
    .const dropfnt8x8_vdc1_vram = ($40*$20/$10+$10)*$10
    .const dropfnt8x8_vdc1_count = $10+$60
    .const dropfnt8x8_vdc1_plane2 = $ff
    .const dropfnt8x8_vdc1_plane3 = 0
    .const load_palette6_palnum = 1
    .const load_palette6_palcnt = 1
    .label bp = $46
    .label bp_1 = $48
    .label sp = $4a
    // /mnt/huc/huc/examples/asm/elmer/rom-kickc-shmup/shmup.c:301
    lda #<0
    sta hiscore
    sta hiscore+1
    // /mnt/huc/huc/examples/asm/elmer/kickc/kickc.h:51
    jsr init_256x224 
  __b1:
    // /mnt/huc/huc/examples/asm/elmer/rom-kickc-shmup/shmup.c:313
    lda #<0
    sta frames
    sta frames+1
    // /mnt/huc/huc/examples/asm/elmer/rom-kickc-shmup/shmup.c:314
    sta score
    sta score+1
    // /mnt/huc/huc/examples/asm/elmer/kickc/huc-gfx.h:117
    jsr _init_satb 
    // /mnt/huc/huc/examples/asm/elmer/rom-kickc-shmup/shmup.c:328
    lda #0
    sta.z __al
    jsr _spr_set 
    // /mnt/huc/huc/examples/asm/elmer/rom-kickc-shmup/shmup.c:329
    lda #<$68
    sta.z __ax
    lda #>$68
    sta.z __ax+1
    jsr _spr_x 
    // /mnt/huc/huc/examples/asm/elmer/rom-kickc-shmup/shmup.c:330
    lda #<$99
    sta.z __ax
    lda #>$99
    sta.z __ax+1
    jsr _spr_y 
    // /mnt/huc/huc/examples/asm/elmer/rom-kickc-shmup/shmup.c:331
    lda #<$5000
    sta.z __ax
    lda #>$5000
    sta.z __ax+1
    jsr _spr_pattern 
    // /mnt/huc/huc/examples/asm/elmer/rom-kickc-shmup/shmup.c:332
    lda #($88|$31)^$ff
    sta.z __al
    lda #(8|$11)&($88|$31)
    sta.z __ah
    jsr _spr_ctrl 
    // /mnt/huc/huc/examples/asm/elmer/rom-kickc-shmup/shmup.c:333
    lda #0
    sta.z __al
    jsr _spr_pal 
    // /mnt/huc/huc/examples/asm/elmer/rom-kickc-shmup/shmup.c:334
    lda #1
    sta.z __al
    jsr _spr_pri 
    ldx #0
  __b2:
    // /mnt/huc/huc/examples/asm/elmer/rom-kickc-shmup/shmup.c:336
    cpx #$a
    bcc __b3
    ldx #0
  __b4:
    // /mnt/huc/huc/examples/asm/elmer/rom-kickc-shmup/shmup.c:347
    cpx #5
    bcc __b5
    // /mnt/huc/huc/examples/asm/elmer/kickc/kickc.h:78
    lda #<bonkpal
    sta.z __si
    lda #>bonkpal
    sta.z __si+1
    // /mnt/huc/huc/examples/asm/elmer/kickc/kickc.h:79
    lda #$ff&bonkpal>>$17
    sta.z __bank
    // /mnt/huc/huc/examples/asm/elmer/kickc/kickc.h:80
    lda #load_palette1_palnum
    sta.z __al
    // /mnt/huc/huc/examples/asm/elmer/kickc/kickc.h:81
    lda #load_palette1_palcnt
    sta.z __ah
    // /mnt/huc/huc/examples/asm/elmer/kickc/kickc.h:82
    jsr load_palettes 
    // /mnt/huc/huc/examples/asm/elmer/kickc/kickc.h:78
    lda #<bulletpal
    sta.z __si
    lda #>bulletpal
    sta.z __si+1
    // /mnt/huc/huc/examples/asm/elmer/kickc/kickc.h:79
    lda #$ff&bulletpal>>$17
    sta.z __bank
    // /mnt/huc/huc/examples/asm/elmer/kickc/kickc.h:80
    lda #load_palette2_palnum
    sta.z __al
    // /mnt/huc/huc/examples/asm/elmer/kickc/kickc.h:81
    lda #load_palette2_palcnt
    sta.z __ah
    // /mnt/huc/huc/examples/asm/elmer/kickc/kickc.h:82
    jsr load_palettes 
    // /mnt/huc/huc/examples/asm/elmer/kickc/kickc.h:78
    lda #<shippal
    sta.z __si
    lda #>shippal
    sta.z __si+1
    // /mnt/huc/huc/examples/asm/elmer/kickc/kickc.h:79
    lda #$ff&shippal>>$17
    sta.z __bank
    // /mnt/huc/huc/examples/asm/elmer/kickc/kickc.h:80
    lda #load_palette3_palnum
    sta.z __al
    // /mnt/huc/huc/examples/asm/elmer/kickc/kickc.h:81
    lda #load_palette3_palcnt
    sta.z __ah
    // /mnt/huc/huc/examples/asm/elmer/kickc/kickc.h:82
    jsr load_palettes 
    // /mnt/huc/huc/examples/asm/elmer/kickc/kickc.h:78
    lda #<explosionpal
    sta.z __si
    lda #>explosionpal
    sta.z __si+1
    // /mnt/huc/huc/examples/asm/elmer/kickc/kickc.h:79
    lda #$ff&explosionpal>>$17
    sta.z __bank
    // /mnt/huc/huc/examples/asm/elmer/kickc/kickc.h:80
    lda #load_palette4_palnum
    sta.z __al
    // /mnt/huc/huc/examples/asm/elmer/kickc/kickc.h:81
    lda #load_palette4_palcnt
    sta.z __ah
    // /mnt/huc/huc/examples/asm/elmer/kickc/kickc.h:82
    jsr load_palettes 
    // /mnt/huc/huc/examples/asm/elmer/rom-kickc-shmup/shmup.c:362
    lda #<bonkspr
    sta.z __si
    lda #>bonkspr
    sta.z __si+1
    lda #$ff&bonkspr>>$17
    sta.z __bank
    lda #<$5000
    sta.z __di
    lda #>$5000
    sta.z __di+1
    lda #<$400
    sta.z __ax
    lda #>$400
    sta.z __ax+1
    jsr _load_vram 
    // /mnt/huc/huc/examples/asm/elmer/rom-kickc-shmup/shmup.c:363
    lda #<bulletspr
    sta.z __si
    lda #>bulletspr
    sta.z __si+1
    lda #$ff&bulletspr>>$17
    sta.z __bank
    lda #<$5400
    sta.z __di
    lda #>$5400
    sta.z __di+1
    lda #<$40
    sta.z __ax
    lda #>$40
    sta.z __ax+1
    jsr _load_vram 
    // /mnt/huc/huc/examples/asm/elmer/rom-kickc-shmup/shmup.c:364
    lda #<shipspr
    sta.z __si
    lda #>shipspr
    sta.z __si+1
    lda #$ff&shipspr>>$17
    sta.z __bank
    lda #<$5500
    sta.z __di
    lda #>$5500
    sta.z __di+1
    lda #<$400
    sta.z __ax
    lda #>$400
    sta.z __ax+1
    jsr _load_vram 
    // /mnt/huc/huc/examples/asm/elmer/rom-kickc-shmup/shmup.c:365
    lda #<explosionspr
    sta.z __si
    lda #>explosionspr
    sta.z __si+1
    lda #$ff&explosionspr>>$17
    sta.z __bank
    lda #<$5900
    sta.z __di
    lda #>$5900
    sta.z __di+1
    lda #<$800
    sta.z __ax
    lda #>$800
    sta.z __ax+1
    jsr _load_vram 
    // /mnt/huc/huc/examples/asm/elmer/kickc/huc-gfx.h:127
    jsr _satb_update 
    // /mnt/huc/huc/examples/asm/elmer/rom-kickc-shmup/shmup.c:369
    lda #<scene_chr
    sta.z __si
    lda #>scene_chr
    sta.z __si+1
    lda #$ff&scene_chr>>$17
    sta.z __bank
    lda #<$1000
    sta.z __di
    lda #>$1000
    sta.z __di+1
    lda #<$4000
    sta.z __ax
    lda #>$4000
    sta.z __ax+1
    jsr _load_vram 
    // /mnt/huc/huc/examples/asm/elmer/kickc/kickc.h:30
    jsr wait_vsync 
    // /mnt/huc/huc/examples/asm/elmer/kickc/kickc.h:78
    lda #<scene_pal
    sta.z __si
    lda #>scene_pal
    sta.z __si+1
    // /mnt/huc/huc/examples/asm/elmer/kickc/kickc.h:79
    lda #$ff&scene_pal>>$17
    sta.z __bank
    // /mnt/huc/huc/examples/asm/elmer/kickc/kickc.h:80
    lda #load_palette5_palnum
    sta.z __al
    // /mnt/huc/huc/examples/asm/elmer/kickc/kickc.h:81
    lda #load_palette5_palcnt
    sta.z __ah
    // /mnt/huc/huc/examples/asm/elmer/kickc/kickc.h:82
    jsr load_palettes 
    // /mnt/huc/huc/examples/asm/elmer/rom-kickc-shmup/shmup.c:369
    lda #<scene_bat
    sta.z __si
    lda #>scene_bat
    sta.z __si+1
    lda #$ff&scene_bat>>$17
    sta.z __bank
    lda #<0
    sta.z __di
    sta.z __di+1
    lda #$20
    sta.z __al
    lda #$1c
    sta.z __ah
    jsr _load_bat 
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
    lda #<textpal
    sta.z __si
    lda #>textpal
    sta.z __si+1
    // /mnt/huc/huc/examples/asm/elmer/kickc/kickc.h:79
    lda #$ff&textpal>>$17
    sta.z __bank
    // /mnt/huc/huc/examples/asm/elmer/kickc/kickc.h:80
    lda #load_palette6_palnum
    sta.z __al
    // /mnt/huc/huc/examples/asm/elmer/kickc/kickc.h:81
    lda #load_palette6_palcnt
    sta.z __ah
    // /mnt/huc/huc/examples/asm/elmer/kickc/kickc.h:82
    jsr load_palettes 
    // /mnt/huc/huc/examples/asm/elmer/kickc/kickc.h:73
    jsr set_dspon 
    lda #1
    sta bonk_dir
    lda #0
    sta bullet_wait
    lda #<$99
    sta bonky
    lda #>$99
    sta bonky+1
    lda #<0
    sta tic
    sta tic+1
    lda #<$68
    sta bonkx
    lda #>$68
    sta bonkx+1
    lda #0
    sta dead
  __b6:
    // /mnt/huc/huc/examples/asm/elmer/rom-kickc-shmup/shmup.c:380
    lda dead
    beq wait_vsync2
    jmp __b1
  wait_vsync2:
    // /mnt/huc/huc/examples/asm/elmer/kickc/kickc.h:30
    jsr wait_vsync 
    // /mnt/huc/huc/examples/asm/elmer/kickc/huc-gfx.h:36
    lda joynow
    sta j1
    lda #0
    sta j1+1
    // /mnt/huc/huc/examples/asm/elmer/kickc/huc-gfx.h:40
    lda joytrg
    // /mnt/huc/huc/examples/asm/elmer/rom-kickc-shmup/shmup.c:384
    and #8
    cmp #0
    beq __b7
    // /mnt/huc/huc/examples/asm/elmer/kickc/kickc.h:30
    jsr wait_vsync 
  get_joytrg2:
    // /mnt/huc/huc/examples/asm/elmer/kickc/huc-gfx.h:40
    lda joytrg
    // /mnt/huc/huc/examples/asm/elmer/rom-kickc-shmup/shmup.c:386
    and #8
    cmp #0
    beq wait_vsync4
  __b7:
    // /mnt/huc/huc/examples/asm/elmer/rom-kickc-shmup/shmup.c:389
    lda #$80
    and j1
    cmp #0
    beq __b8
    // /mnt/huc/huc/examples/asm/elmer/rom-kickc-shmup/shmup.c:391
    lda #8^$ff
    sta.z __al
    lda #0
    sta.z __ah
    jsr _spr_ctrl 
    // /mnt/huc/huc/examples/asm/elmer/rom-kickc-shmup/shmup.c:392
    lda #<-8
    cmp bonkx
    lda #>-8
    sbc bonkx+1
    bvc !+
    eor #$80
  !:
    bpl __b9
    lda bonkx
    sec
    sbc #2
    sta bonkx
    lda bonkx+1
    sbc #>2
    sta bonkx+1
  __b9:
    // /mnt/huc/huc/examples/asm/elmer/rom-kickc-shmup/shmup.c:393
    inc tic
    bne !+
    inc tic+1
  !:
    lda #-1
    sta bonk_dir
  __b8:
    // /mnt/huc/huc/examples/asm/elmer/rom-kickc-shmup/shmup.c:396
    lda #$20
    and j1
    cmp #0
    beq __b10
    // /mnt/huc/huc/examples/asm/elmer/rom-kickc-shmup/shmup.c:398
    lda #8^$ff
    sta.z __al
    lda #8&8
    sta.z __ah
    jsr _spr_ctrl 
    // /mnt/huc/huc/examples/asm/elmer/rom-kickc-shmup/shmup.c:399
    lda bonkx
    cmp #<$e8
    lda bonkx+1
    sbc #>$e8
    bvc !+
    eor #$80
  !:
    bpl __b11
    lda bonkx
    clc
    adc #<2
    sta bonkx
    lda bonkx+1
    adc #>2
    sta bonkx+1
  __b11:
    // /mnt/huc/huc/examples/asm/elmer/rom-kickc-shmup/shmup.c:400
    inc tic
    bne !+
    inc tic+1
  !:
    lda #1
    sta bonk_dir
  __b10:
    // /mnt/huc/huc/examples/asm/elmer/rom-kickc-shmup/shmup.c:403
    lda #$10
    and j1
    cmp #0
    beq __b12
    // /mnt/huc/huc/examples/asm/elmer/rom-kickc-shmup/shmup.c:405
    lda #<-8
    cmp bonky
    lda #>-8
    sbc bonky+1
    bvc !+
    eor #$80
  !:
    bpl __b12
    lda bonky
    sec
    sbc #2
    sta bonky
    lda bonky+1
    sbc #>2
    sta bonky+1
  __b12:
    // /mnt/huc/huc/examples/asm/elmer/rom-kickc-shmup/shmup.c:407
    lda #$40
    and j1
    cmp #0
    beq __b13
    // /mnt/huc/huc/examples/asm/elmer/rom-kickc-shmup/shmup.c:409
    lda bonky
    cmp #<$d4
    lda bonky+1
    sbc #>$d4
    bvc !+
    eor #$80
  !:
    bpl __b13
    lda bonky
    clc
    adc #<2
    sta bonky
    lda bonky+1
    adc #>2
    sta bonky+1
  __b13:
    // /mnt/huc/huc/examples/asm/elmer/rom-kickc-shmup/shmup.c:411
    lda #2
    and j1
    cmp #0
    beq __b14
    lda bullet_wait
    beq __b19
    jmp __b14
  __b19:
    lda #<bullets
    sta.z bp
    lda #>bullets
    sta.z bp+1
    ldx #0
  __b15:
    // /mnt/huc/huc/examples/asm/elmer/rom-kickc-shmup/shmup.c:412
    cpx #$a
    bcc __b16
  __b14:
    // /mnt/huc/huc/examples/asm/elmer/rom-kickc-shmup/shmup.c:430
    lda bullet_wait
    beq __b21
    // /mnt/huc/huc/examples/asm/elmer/rom-kickc-shmup/shmup.c:431
    dec bullet_wait
  __b21:
    lda #<bullets
    sta.z bp_1
    lda #>bullets
    sta.z bp_1+1
    ldx #0
  __b22:
    // /mnt/huc/huc/examples/asm/elmer/rom-kickc-shmup/shmup.c:434
    cpx #$a
    bcc __b23
    // /mnt/huc/huc/examples/asm/elmer/rom-kickc-shmup/shmup.c:451
    jsr do_ships
    lda #<ships
    sta.z sp
    lda #>ships
    sta.z sp+1
    lda #0
    sta i
  __b30:
    // /mnt/huc/huc/examples/asm/elmer/rom-kickc-shmup/shmup.c:454
    lda i
    cmp #5
    bcc __b31
    // /mnt/huc/huc/examples/asm/elmer/rom-kickc-shmup/shmup.c:476
    lda #0
    sta.z __al
    jsr _spr_set 
    // /mnt/huc/huc/examples/asm/elmer/rom-kickc-shmup/shmup.c:477
    lda bonkx
    sta.z __ax
    lda bonkx+1
    sta.z __ax+1
    jsr _spr_x 
    // /mnt/huc/huc/examples/asm/elmer/rom-kickc-shmup/shmup.c:479
    lda bonky
    sta.z __ax
    lda bonky+1
    sta.z __ax+1
    jsr _spr_y 
    // /mnt/huc/huc/examples/asm/elmer/rom-kickc-shmup/shmup.c:480
    lda tic+1
    lsr
    sta __100+1
    lda tic
    ror
    sta __100
    lsr __100+1
    ror __100
    lda #3
    and __100
    sta __102+1
    lda #0
    sta __102
    lda __103
    clc
    adc #<$5000
    sta __103
    lda __103+1
    adc #>$5000
    sta __103+1
    lda __103
    sta.z __ax
    lda __103+1
    sta.z __ax+1
    jsr _spr_pattern 
    // /mnt/huc/huc/examples/asm/elmer/kickc/huc-gfx.h:127
    jsr _satb_update 
    // /mnt/huc/huc/examples/asm/elmer/rom-kickc-shmup/shmup.c:482
    lda #$1a
    sta.z __al
    lda #1
    sta.z __ah
    lda score
    sta.z __bx
    lda score+1
    sta.z __bx+1
    lda #5
    sta.z __cl
    jsr _put_number 
    // /mnt/huc/huc/examples/asm/elmer/rom-kickc-shmup/shmup.c:484
    lda #1
    sta.z __al
    sta.z __ah
    lda #<str_hi
    sta.z __si
    lda #>str_hi
    sta.z __si+1
    jsr _put_string 
    // /mnt/huc/huc/examples/asm/elmer/rom-kickc-shmup/shmup.c:485
    lda #4
    sta.z __al
    lda #1
    sta.z __ah
    lda hiscore
    sta.z __bx
    lda hiscore+1
    sta.z __bx+1
    lda #5
    sta.z __cl
    jsr _put_number 
    // /mnt/huc/huc/examples/asm/elmer/rom-kickc-shmup/shmup.c:486
    inc frames
    bne !+
    inc frames+1
  !:
    jmp __b6
  __b31:
    // /mnt/huc/huc/examples/asm/elmer/rom-kickc-shmup/shmup.c:455
    ldy #OFFSET_STRUCT_SHIP_ACTIVE
    lda (sp),y
    cmp #1
    bne __b33
    // /mnt/huc/huc/examples/asm/elmer/rom-kickc-shmup/shmup.c:456
    lda bonkx
    sec
    sbc #$18
    sta __72
    lda bonkx+1
    sbc #>$18
    sta __72+1
    // /mnt/huc/huc/examples/asm/elmer/rom-kickc-shmup/shmup.c:457
    lda bonkx
    clc
    adc #<$18
    sta __74
    lda bonkx+1
    adc #>$18
    sta __74+1
    // /mnt/huc/huc/examples/asm/elmer/rom-kickc-shmup/shmup.c:458
    lda bonky
    sec
    sbc #$14
    sta __77
    lda bonky+1
    sbc #>$14
    sta __77+1
    // /mnt/huc/huc/examples/asm/elmer/rom-kickc-shmup/shmup.c:459
    lda bonky
    clc
    adc #<9
    sta __80
    lda bonky+1
    adc #>9
    sta __80+1
    // /mnt/huc/huc/examples/asm/elmer/rom-kickc-shmup/shmup.c:460
    lda bonkx
    sec
    sbc #$12
    sta __83
    lda bonkx+1
    sbc #>$12
    sta __83+1
    // /mnt/huc/huc/examples/asm/elmer/rom-kickc-shmup/shmup.c:461
    lda bonkx
    clc
    adc #<$12
    sta __85
    lda bonkx+1
    adc #>$12
    sta __85+1
    // /mnt/huc/huc/examples/asm/elmer/rom-kickc-shmup/shmup.c:462
    lda bonky
    clc
    adc #<8
    sta __88
    lda bonky+1
    adc #>8
    sta __88+1
    // /mnt/huc/huc/examples/asm/elmer/rom-kickc-shmup/shmup.c:463
    lda bonky
    clc
    adc #<$19
    sta __91
    lda bonky+1
    adc #>$19
    sta __91+1
    // /mnt/huc/huc/examples/asm/elmer/rom-kickc-shmup/shmup.c:456
    ldy #0
    lda __72
    cmp (sp),y
    iny
    lda __72+1
    sbc (sp),y
    bvc !+
    eor #$80
  !:
    bpl __b58
    ldy #0
    lda (sp),y
    cmp __74
    iny
    lda (sp),y
    sbc __74+1
    bvc !+
    eor #$80
  !:
    bmi __b60
  __b58:
    ldy #0
    lda __83
    cmp (sp),y
    iny
    lda __83+1
    sbc (sp),y
    bvc !+
    eor #$80
  !:
    bpl __b33
    ldy #0
    lda (sp),y
    cmp __85
    iny
    lda (sp),y
    sbc __85+1
    bvc !+
    eor #$80
  !:
    bmi __b63
    jmp __b33
  __b29:
    lda #1
    sta dead
  __b33:
    // /mnt/huc/huc/examples/asm/elmer/rom-kickc-shmup/shmup.c:474
    lda #SIZEOF_STRUCT_SHIP
    clc
    adc.z sp
    sta.z sp
    bcc !+
    inc.z sp+1
  !:
    // /mnt/huc/huc/examples/asm/elmer/rom-kickc-shmup/shmup.c:454
    inc i
    jmp __b30
  __b63:
    // /mnt/huc/huc/examples/asm/elmer/rom-kickc-shmup/shmup.c:456
    ldy #OFFSET_STRUCT_SHIP_Y
    lda __88
    cmp (sp),y
    iny
    lda __88+1
    sbc (sp),y
    bvc !+
    eor #$80
  !:
    bpl __b33
    ldy #OFFSET_STRUCT_SHIP_Y
    lda (sp),y
    cmp __91
    iny
    lda (sp),y
    sbc __91+1
    bvc !+
    eor #$80
  !:
    bmi __b38
    jmp __b33
  __b38:
    // /mnt/huc/huc/examples/asm/elmer/rom-kickc-shmup/shmup.c:466
    lda #$b
    sta.z __al
    lda #4
    sta.z __ah
    lda #<str_game_over
    sta.z __si
    lda #>str_game_over
    sta.z __si+1
    jsr _put_string 
    ldx #0
  __b34:
    // /mnt/huc/huc/examples/asm/elmer/rom-kickc-shmup/shmup.c:467
    cpx #$64
    bcc wait_vsync5
    // /mnt/huc/huc/examples/asm/elmer/rom-kickc-shmup/shmup.c:470
    lda score+1
    cmp hiscore+1
    bne !+
    lda score
    cmp hiscore
    beq __b29
  !:
    bcc __b29
    // /mnt/huc/huc/examples/asm/elmer/rom-kickc-shmup/shmup.c:471
    lda score
    sta hiscore
    lda score+1
    sta hiscore+1
    jmp __b29
  wait_vsync5:
    // /mnt/huc/huc/examples/asm/elmer/kickc/kickc.h:30
    jsr wait_vsync 
    // /mnt/huc/huc/examples/asm/elmer/rom-kickc-shmup/shmup.c:467
    inx
    jmp __b34
  __b60:
    // /mnt/huc/huc/examples/asm/elmer/rom-kickc-shmup/shmup.c:456
    ldy #OFFSET_STRUCT_SHIP_Y
    lda __77
    cmp (sp),y
    iny
    lda __77+1
    sbc (sp),y
    bvc !+
    eor #$80
  !:
    bpl __b58
    ldy #OFFSET_STRUCT_SHIP_Y
    lda (sp),y
    cmp __80
    iny
    lda (sp),y
    sbc __80+1
    bvc !+
    eor #$80
  !:
    bmi __b38
    jmp __b58
  __b23:
    // /mnt/huc/huc/examples/asm/elmer/rom-kickc-shmup/shmup.c:435
    ldy #OFFSET_STRUCT_BULLET_ACTIVE
    lda (bp_1),y
    cmp #0
    beq __b25
    // /mnt/huc/huc/examples/asm/elmer/rom-kickc-shmup/shmup.c:436
    txa
    clc
    adc #1
    sta.z __al
    jsr _spr_set 
    // /mnt/huc/huc/examples/asm/elmer/rom-kickc-shmup/shmup.c:437
    ldy #0
    lda (bp_1),y
    sta.z __ax
    iny
    lda (bp_1),y
    sta.z __ax+1
    jsr _spr_x 
    // /mnt/huc/huc/examples/asm/elmer/rom-kickc-shmup/shmup.c:438
    ldy #OFFSET_STRUCT_BULLET_Y
    lda (bp_1),y
    sta.z __ax
    iny
    lda (bp_1),y
    sta.z __ax+1
    jsr _spr_y 
    // /mnt/huc/huc/examples/asm/elmer/rom-kickc-shmup/shmup.c:440
    ldy #OFFSET_STRUCT_BULLET_ACTIVE
    lda (bp_1),y
    cmp #0
    bpl __b26
    lda #<-4
    sta __63
    lda #>-4
    sta __63+1
    jmp __b27
  __b26:
    // /mnt/huc/huc/examples/asm/elmer/rom-kickc-shmup/shmup.c:440
    lda #<4
    sta __63
    lda #>4
    sta __63+1
  __b27:
    // CHANGED!			bp->x += bp->active * SPEED_BULLET;
    ldy #0
    clc
    lda (bp_1),y
    adc __63
    sta (bp_1),y
    iny
    lda (bp_1),y
    adc __63+1
    sta (bp_1),y
    // /mnt/huc/huc/examples/asm/elmer/rom-kickc-shmup/shmup.c:442
    ldy #0
    lda #<$100
    cmp (bp_1),y
    iny
    lda #>$100
    sbc (bp_1),y
    bvc !+
    eor #$80
  !:
    bmi __b28
    ldy #0
    lda (bp_1),y
    cmp #<-$10
    iny
    lda (bp_1),y
    sbc #>-$10
    bvc !+
    eor #$80
  !:
    bpl __b25
  __b28:
    // /mnt/huc/huc/examples/asm/elmer/rom-kickc-shmup/shmup.c:443
    lda #0
    ldy #OFFSET_STRUCT_BULLET_ACTIVE
    sta (bp_1),y
    // /mnt/huc/huc/examples/asm/elmer/rom-kickc-shmup/shmup.c:444
    lda #<-$10
    sta.z __ax
    lda #>-$10
    sta.z __ax+1
    jsr _spr_x 
    // /mnt/huc/huc/examples/asm/elmer/rom-kickc-shmup/shmup.c:445
    lda #<0
    sta.z __ax
    sta.z __ax+1
    jsr _spr_y 
  __b25:
    // /mnt/huc/huc/examples/asm/elmer/rom-kickc-shmup/shmup.c:448
    lda #SIZEOF_STRUCT_BULLET
    clc
    adc.z bp_1
    sta.z bp_1
    bcc !+
    inc.z bp_1+1
  !:
    // /mnt/huc/huc/examples/asm/elmer/rom-kickc-shmup/shmup.c:434
    inx
    jmp __b22
  __b16:
    // /mnt/huc/huc/examples/asm/elmer/rom-kickc-shmup/shmup.c:413
    ldy #OFFSET_STRUCT_BULLET_ACTIVE
    lda (bp),y
    cmp #0
    bne __b17
    // /mnt/huc/huc/examples/asm/elmer/rom-kickc-shmup/shmup.c:414
    lda bonk_dir
    sta (bp),y
    // /mnt/huc/huc/examples/asm/elmer/rom-kickc-shmup/shmup.c:415
    lda bonkx
    clc
    adc #<8
    sta __50
    lda bonkx+1
    adc #>8
    sta __50+1
    lda bonk_dir
    asl
    asl
    asl
    asl
    sta.z $ff
    clc
    adc __52
    sta __52
    lda.z $ff
    ora #$7f
    bmi !+
    lda #0
  !:
    adc __52+1
    sta __52+1
    ldy #0
    lda __52
    sta (bp),y
    iny
    lda __52+1
    sta (bp),y
    // /mnt/huc/huc/examples/asm/elmer/rom-kickc-shmup/shmup.c:416
    lda bonky
    clc
    adc #<$a
    sta __53
    lda bonky+1
    adc #>$a
    sta __53+1
    ldy #OFFSET_STRUCT_BULLET_Y
    lda __53
    sta (bp),y
    iny
    lda __53+1
    sta (bp),y
    // /mnt/huc/huc/examples/asm/elmer/rom-kickc-shmup/shmup.c:418
    txa
    clc
    adc #1
    sta.z __al
    jsr _spr_set 
    // /mnt/huc/huc/examples/asm/elmer/rom-kickc-shmup/shmup.c:419
    lda bonk_dir
    cmp #0
    beq !+
    bpl __b18
  !:
    // /mnt/huc/huc/examples/asm/elmer/rom-kickc-shmup/shmup.c:422
    lda #8^$ff
    sta.z __al
    lda #0
    sta.z __ah
    jsr _spr_ctrl 
  __b32:
    lda #$a
    sta bullet_wait
    jmp __b14
  __b18:
    // /mnt/huc/huc/examples/asm/elmer/rom-kickc-shmup/shmup.c:420
    lda #8^$ff
    sta.z __al
    lda #8&8
    sta.z __ah
    jsr _spr_ctrl 
    jmp __b32
  __b17:
    // /mnt/huc/huc/examples/asm/elmer/rom-kickc-shmup/shmup.c:412
    inx
    lda #SIZEOF_STRUCT_BULLET
    clc
    adc.z bp
    sta.z bp
    bcc !+
    inc.z bp+1
  !:
    jmp __b15
  wait_vsync4:
    // /mnt/huc/huc/examples/asm/elmer/kickc/kickc.h:30
    jsr wait_vsync 
    jmp get_joytrg2
  __b5:
    // /mnt/huc/huc/examples/asm/elmer/rom-kickc-shmup/shmup.c:348
    txa
    clc
    adc #1+$a
    sta.z __al
    jsr _spr_set 
    // /mnt/huc/huc/examples/asm/elmer/rom-kickc-shmup/shmup.c:349
    lda #<-$20
    sta.z __ax
    lda #>-$20
    sta.z __ax+1
    jsr _spr_x 
    // /mnt/huc/huc/examples/asm/elmer/rom-kickc-shmup/shmup.c:350
    lda #<$10
    sta.z __ax
    lda #>$10
    sta.z __ax+1
    jsr _spr_y 
    // /mnt/huc/huc/examples/asm/elmer/rom-kickc-shmup/shmup.c:351
    lda #<$5500
    sta.z __ax
    lda #>$5500
    sta.z __ax+1
    jsr _spr_pattern 
    // /mnt/huc/huc/examples/asm/elmer/rom-kickc-shmup/shmup.c:352
    lda #($88|$31)^$ff
    sta.z __al
    lda #$11&($88|$31)
    sta.z __ah
    jsr _spr_ctrl 
    // /mnt/huc/huc/examples/asm/elmer/rom-kickc-shmup/shmup.c:353
    lda #2
    sta.z __al
    jsr _spr_pal 
    // /mnt/huc/huc/examples/asm/elmer/rom-kickc-shmup/shmup.c:354
    lda #1
    sta.z __al
    jsr _spr_pri 
    // /mnt/huc/huc/examples/asm/elmer/rom-kickc-shmup/shmup.c:355
    txa
    asl
    stx.z $ff
    clc
    adc.z $ff
    asl
    stx.z $ff
    clc
    adc.z $ff
    tay
    lda #0
    sta ships+OFFSET_STRUCT_SHIP_ACTIVE,y
    // /mnt/huc/huc/examples/asm/elmer/rom-kickc-shmup/shmup.c:347
    inx
    jmp __b4
  __b3:
    // /mnt/huc/huc/examples/asm/elmer/rom-kickc-shmup/shmup.c:337
    txa
    clc
    adc #1
    sta.z __al
    jsr _spr_set 
    // /mnt/huc/huc/examples/asm/elmer/rom-kickc-shmup/shmup.c:338
    lda #<-$10
    sta.z __ax
    lda #>-$10
    sta.z __ax+1
    jsr _spr_x 
    // /mnt/huc/huc/examples/asm/elmer/rom-kickc-shmup/shmup.c:339
    lda #<0
    sta.z __ax
    sta.z __ax+1
    jsr _spr_y 
    // /mnt/huc/huc/examples/asm/elmer/rom-kickc-shmup/shmup.c:340
    lda #<$5400
    sta.z __ax
    lda #>$5400
    sta.z __ax+1
    jsr _spr_pattern 
    // /mnt/huc/huc/examples/asm/elmer/rom-kickc-shmup/shmup.c:341
    lda #($88|$31)^$ff
    sta.z __al
    lda #0
    sta.z __ah
    jsr _spr_ctrl 
    // /mnt/huc/huc/examples/asm/elmer/rom-kickc-shmup/shmup.c:342
    lda #1
    sta.z __al
    jsr _spr_pal 
    // /mnt/huc/huc/examples/asm/elmer/rom-kickc-shmup/shmup.c:343
    lda #1
    sta.z __al
    jsr _spr_pri 
    // /mnt/huc/huc/examples/asm/elmer/rom-kickc-shmup/shmup.c:344
    txa
    asl
    asl
    stx.z $ff
    clc
    adc.z $ff
    tay
    lda #0
    sta bullets+OFFSET_STRUCT_BULLET_ACTIVE,y
    // /mnt/huc/huc/examples/asm/elmer/rom-kickc-shmup/shmup.c:336
    inx
    jmp __b2
  .segment bss
    __50: .word 0
    .label __52 = __50
    __53: .word 0
    __63: .word 0
    __72: .word 0
    __74: .word 0
    __77: .word 0
    __80: .word 0
    __83: .word 0
    __85: .word 0
    __88: .word 0
    __91: .word 0
    __100: .word 0
    __102: .word 0
    .label __103 = __102
    j1: .word 0
    tic: .word 0
    bonkx: .word 0
    bonky: .word 0
    bullet_wait: .byte 0
    i: .byte 0
    dead: .byte 0
    bonk_dir: .byte 0
}
.segment Code
// **************************************************************************
// **************************************************************************
//
// do_ships()
//
do_ships: {
    .label sp = $42
    .label sp_1 = $44
    .label bp = $40
    // /mnt/huc/huc/examples/asm/elmer/kickc/kickc.h:35
    jsr get_random 
    // /mnt/huc/huc/examples/asm/elmer/kickc/kickc.h:37
    ldx random+1
    // /mnt/huc/huc/examples/asm/elmer/rom-kickc-shmup/shmup.c:196
    txa
    and #$7e
    cmp #2
    bne __b5
    lda #<ships
    sta.z sp
    lda #>ships
    sta.z sp+1
    lda #<0
    sta i
    sta i+1
  __b1:
    // /mnt/huc/huc/examples/asm/elmer/rom-kickc-shmup/shmup.c:197
    lda i+1
    bne !+
    lda i
    cmp #5
    bcc __b2
  !:
  __b5:
    lda #<ships
    sta.z sp_1
    lda #>ships
    sta.z sp_1+1
    lda #<0
    sta i_1
    sta i_1+1
  __b7:
    // /mnt/huc/huc/examples/asm/elmer/rom-kickc-shmup/shmup.c:220
    lda i_1+1
    bne !+
    lda i_1
    cmp #5
    bcc __b8
  !:
    // /mnt/huc/huc/examples/asm/elmer/rom-kickc-shmup/shmup.c:278
    rts
  __b8:
    // /mnt/huc/huc/examples/asm/elmer/rom-kickc-shmup/shmup.c:221
    ldy #OFFSET_STRUCT_SHIP_ACTIVE
    lda (sp_1),y
    cmp #0
    beq __b9
    // /mnt/huc/huc/examples/asm/elmer/rom-kickc-shmup/shmup.c:222
    lda #1+$a
    clc
    adc i_1
    sta __12
    lda #0
    adc i_1+1
    sta __12+1
    lda __12
    sta.z __al
    jsr _spr_set 
    // /mnt/huc/huc/examples/asm/elmer/rom-kickc-shmup/shmup.c:223
    ldy #0
    lda (sp_1),y
    sta.z __ax
    iny
    lda (sp_1),y
    sta.z __ax+1
    jsr _spr_x 
    // /mnt/huc/huc/examples/asm/elmer/rom-kickc-shmup/shmup.c:224
    ldy #OFFSET_STRUCT_SHIP_Y
    lda (sp_1),y
    sta.z __ax
    iny
    lda (sp_1),y
    sta.z __ax+1
    jsr _spr_y 
    // /mnt/huc/huc/examples/asm/elmer/rom-kickc-shmup/shmup.c:225
    ldy #OFFSET_STRUCT_SHIP_ACTIVE
    lda (sp_1),y
    sec
    sbc #1+1
    bvc !+
    eor #$80
  !:
    bpl __b10
    // /mnt/huc/huc/examples/asm/elmer/rom-kickc-shmup/shmup.c:238
    lda frames+1
    lsr
    sta __14+1
    lda frames
    ror
    sta __14
    lsr __14+1
    ror __14
    lsr __14+1
    ror __14
    lsr __14+1
    ror __14
    lda #3
    and __14
    sta __16+1
    lda #0
    sta __16
    lda __17
    clc
    adc #<$5500
    sta __17
    lda __17+1
    adc #>$5500
    sta __17+1
    lda __17
    sta.z __ax
    lda __17+1
    sta.z __ax+1
    jsr _spr_pattern 
    lda #<bullets
    sta.z bp
    lda #>bullets
    sta.z bp+1
    lda #<0
    sta j
    sta j+1
  __b11:
    // /mnt/huc/huc/examples/asm/elmer/rom-kickc-shmup/shmup.c:239
    lda j+1
    bne !+
    lda j
    cmp #$a
    bcc __b12
  !:
  __b16:
    // /mnt/huc/huc/examples/asm/elmer/rom-kickc-shmup/shmup.c:261
    ldy #OFFSET_STRUCT_SHIP_VX
    lda (sp_1),y
    sta.z $ff
    ldy #0
    clc
    adc (sp_1),y
    sta (sp_1),y
    iny
    lda.z $ff
    ora #$7f
    bmi !+
    lda #0
  !:
    adc (sp_1),y
    sta (sp_1),y
    // /mnt/huc/huc/examples/asm/elmer/rom-kickc-shmup/shmup.c:262
    ldy #OFFSET_STRUCT_SHIP_VY
    lda (sp_1),y
    sta.z $ff
    ldy #OFFSET_STRUCT_SHIP_Y
    clc
    adc (sp_1),y
    sta (sp_1),y
    iny
    lda.z $ff
    ora #$7f
    bmi !+
    lda #0
  !:
    adc (sp_1),y
    sta (sp_1),y
    // /mnt/huc/huc/examples/asm/elmer/rom-kickc-shmup/shmup.c:263
    ldy #0
    lda #<$100
    cmp (sp_1),y
    iny
    lda #>$100
    sbc (sp_1),y
    bvc !+
    eor #$80
  !:
    bmi __b18
    ldy #0
    lda (sp_1),y
    cmp #<-$20
    iny
    lda (sp_1),y
    sbc #>-$20
    bvc !+
    eor #$80
  !:
    bpl __b9
  __b18:
    // /mnt/huc/huc/examples/asm/elmer/rom-kickc-shmup/shmup.c:264
    lda #0
    ldy #OFFSET_STRUCT_SHIP_ACTIVE
    sta (sp_1),y
    // /mnt/huc/huc/examples/asm/elmer/rom-kickc-shmup/shmup.c:265
    lda #<-$20
    sta.z __ax
    lda #>-$20
    sta.z __ax+1
    jsr _spr_x 
    // /mnt/huc/huc/examples/asm/elmer/rom-kickc-shmup/shmup.c:266
    lda #<$10
    sta.z __ax
    lda #>$10
    sta.z __ax+1
    jsr _spr_y 
  __b9:
    // /mnt/huc/huc/examples/asm/elmer/rom-kickc-shmup/shmup.c:220
    inc i_1
    bne !+
    inc i_1+1
  !:
    lda #SIZEOF_STRUCT_SHIP
    clc
    adc.z sp_1
    sta.z sp_1
    bcc !+
    inc.z sp_1+1
  !:
    jmp __b7
  __b12:
    // /mnt/huc/huc/examples/asm/elmer/rom-kickc-shmup/shmup.c:240
    ldy #OFFSET_STRUCT_BULLET_ACTIVE
    lda (bp),y
    cmp #0
    beq __b13
    // /mnt/huc/huc/examples/asm/elmer/rom-kickc-shmup/shmup.c:241
    ldy #0
    lda (sp_1),y
    sec
    sbc #<4
    sta __20
    iny
    lda (sp_1),y
    sbc #>4
    sta __20+1
    // /mnt/huc/huc/examples/asm/elmer/rom-kickc-shmup/shmup.c:242
    ldy #0
    lda (sp_1),y
    clc
    adc #<$14
    sta __22
    iny
    lda (sp_1),y
    adc #>$14
    sta __22+1
    // /mnt/huc/huc/examples/asm/elmer/rom-kickc-shmup/shmup.c:243
    ldy #OFFSET_STRUCT_SHIP_Y
    lda (sp_1),y
    sec
    sbc #<3
    sta __25
    iny
    lda (sp_1),y
    sbc #>3
    sta __25+1
    // /mnt/huc/huc/examples/asm/elmer/rom-kickc-shmup/shmup.c:244
    ldy #OFFSET_STRUCT_SHIP_Y
    lda (sp_1),y
    clc
    adc #<$10
    sta __28
    iny
    lda (sp_1),y
    adc #>$10
    sta __28+1
    // /mnt/huc/huc/examples/asm/elmer/rom-kickc-shmup/shmup.c:241
    ldy #0
    lda __20
    cmp (bp),y
    iny
    lda __20+1
    sbc (bp),y
    bvc !+
    eor #$80
  !:
    bpl __b13
    ldy #0
    lda (bp),y
    cmp __22
    iny
    lda (bp),y
    sbc __22+1
    bvc !+
    eor #$80
  !:
    bpl __b13
    ldy #OFFSET_STRUCT_BULLET_Y
    lda __25
    cmp (bp),y
    iny
    lda __25+1
    sbc (bp),y
    bvc !+
    eor #$80
  !:
    bmi __b23
  __b13:
    // /mnt/huc/huc/examples/asm/elmer/rom-kickc-shmup/shmup.c:239
    inc j
    bne !+
    inc j+1
  !:
    lda #SIZEOF_STRUCT_BULLET
    clc
    adc.z bp
    sta.z bp
    bcc !+
    inc.z bp+1
  !:
    jmp __b11
  __b23:
    // /mnt/huc/huc/examples/asm/elmer/rom-kickc-shmup/shmup.c:241
    ldy #OFFSET_STRUCT_BULLET_Y
    lda (bp),y
    cmp __28
    iny
    lda (bp),y
    sbc __28+1
    bvc !+
    eor #$80
  !:
    bpl __b13
    // /mnt/huc/huc/examples/asm/elmer/rom-kickc-shmup/shmup.c:246
    /* explosion */
    lda #2
    ldy #OFFSET_STRUCT_SHIP_ACTIVE
    sta (sp_1),y
    // /mnt/huc/huc/examples/asm/elmer/rom-kickc-shmup/shmup.c:247
    lda #0
    ldy #OFFSET_STRUCT_SHIP_VX
    sta (sp_1),y
    // /mnt/huc/huc/examples/asm/elmer/rom-kickc-shmup/shmup.c:248
    ldy #OFFSET_STRUCT_SHIP_VY
    sta (sp_1),y
    // /mnt/huc/huc/examples/asm/elmer/rom-kickc-shmup/shmup.c:249
    ldy #OFFSET_STRUCT_BULLET_ACTIVE
    sta (bp),y
    // /mnt/huc/huc/examples/asm/elmer/rom-kickc-shmup/shmup.c:250
    lda #1
    clc
    adc j
    sta __32
    lda #0
    adc j+1
    sta __32+1
    lda __32
    sta.z __al
    jsr _spr_set 
    // /mnt/huc/huc/examples/asm/elmer/rom-kickc-shmup/shmup.c:251
    lda #<-$10
    sta.z __ax
    lda #>-$10
    sta.z __ax+1
    jsr _spr_x 
    // /mnt/huc/huc/examples/asm/elmer/rom-kickc-shmup/shmup.c:252
    lda #<0
    sta.z __ax
    sta.z __ax+1
    jsr _spr_y 
    // /mnt/huc/huc/examples/asm/elmer/rom-kickc-shmup/shmup.c:253
    lda #$64
    clc
    adc score
    sta score
    bcc !+
    inc score+1
  !:
    jmp __b13
  __b10:
    // /mnt/huc/huc/examples/asm/elmer/rom-kickc-shmup/shmup.c:227
    ldy #OFFSET_STRUCT_SHIP_ACTIVE
    lda (sp_1),y
    sta __98
    ora #$7f
    bmi !+
    lda #0
  !:
    sta __98+1
    lsr __33+1
    ror __33
    lsr __33+1
    ror __33
    lsr __33+1
    ror __33
    lda __34
    sta __34+1
    lda #0
    sta __34
    lda __35
    clc
    adc #<$5900
    sta __35
    lda __35+1
    adc #>$5900
    sta __35+1
    lda __35
    sta.z __ax
    lda __35+1
    sta.z __ax+1
    jsr _spr_pattern 
    // /mnt/huc/huc/examples/asm/elmer/rom-kickc-shmup/shmup.c:228
    lda #3
    sta.z __al
    jsr _spr_pal 
    // /mnt/huc/huc/examples/asm/elmer/rom-kickc-shmup/shmup.c:229
    ldy #OFFSET_STRUCT_SHIP_ACTIVE
    lda (sp_1),y
    inc
    sta (sp_1),y
    // /mnt/huc/huc/examples/asm/elmer/rom-kickc-shmup/shmup.c:230
    lda (sp_1),y
    cmp #$40
    bne __b16
    // /mnt/huc/huc/examples/asm/elmer/rom-kickc-shmup/shmup.c:231
    lda #0
    sta (sp_1),y
    // /mnt/huc/huc/examples/asm/elmer/rom-kickc-shmup/shmup.c:232
    lda #<-$20
    sta.z __ax
    lda #>-$20
    sta.z __ax+1
    jsr _spr_x 
    // /mnt/huc/huc/examples/asm/elmer/rom-kickc-shmup/shmup.c:233
    lda #<$10
    sta.z __ax
    lda #>$10
    sta.z __ax+1
    jsr _spr_y 
    // /mnt/huc/huc/examples/asm/elmer/rom-kickc-shmup/shmup.c:234
    lda #2
    sta.z __al
    jsr _spr_pal 
    jmp __b16
  __b2:
    // /mnt/huc/huc/examples/asm/elmer/rom-kickc-shmup/shmup.c:198
    ldy #OFFSET_STRUCT_SHIP_ACTIVE
    lda (sp),y
    cmp #0
    bne __b3
    // /mnt/huc/huc/examples/asm/elmer/rom-kickc-shmup/shmup.c:199
    lda #1
    sta (sp),y
    // /mnt/huc/huc/examples/asm/elmer/rom-kickc-shmup/shmup.c:200
    lda #1+$a
    clc
    adc __7
    sta __7
    bcc !+
    inc __7+1
  !:
    lda __7
    sta.z __al
    jsr _spr_set 
    // /mnt/huc/huc/examples/asm/elmer/rom-kickc-shmup/shmup.c:201
    txa
    and #1
    cmp #0
    bne __b4
    // /mnt/huc/huc/examples/asm/elmer/rom-kickc-shmup/shmup.c:208
    lda #($88|$31)^$ff
    sta.z __al
    lda #$11&($88|$31)
    sta.z __ah
    jsr _spr_ctrl 
    // /mnt/huc/huc/examples/asm/elmer/rom-kickc-shmup/shmup.c:209
    lda #-1
    ldy #OFFSET_STRUCT_SHIP_VX
    sta (sp),y
    // /mnt/huc/huc/examples/asm/elmer/rom-kickc-shmup/shmup.c:210
    lda #0
    ldy #OFFSET_STRUCT_SHIP_VY
    sta (sp),y
    // /mnt/huc/huc/examples/asm/elmer/rom-kickc-shmup/shmup.c:211
    tay
    lda #<$100
    sta (sp),y
    iny
    lda #>$100
    sta (sp),y
  rand2101:
    // /mnt/huc/huc/examples/asm/elmer/kickc/kickc.h:41
    jsr _random210 
    // /mnt/huc/huc/examples/asm/elmer/kickc/kickc.h:43
    lda.z __al
    // /mnt/huc/huc/examples/asm/elmer/rom-kickc-shmup/shmup.c:214
    // CHANGED!			sp->y = rand() % 210;
    ldy #OFFSET_STRUCT_SHIP_Y
    sta (sp),y
    lda #0
    iny
    sta (sp),y
    jmp __b5
  __b4:
    // /mnt/huc/huc/examples/asm/elmer/rom-kickc-shmup/shmup.c:202
    lda #($88|$31)^$ff
    sta.z __al
    lda #(8|$11)&($88|$31)
    sta.z __ah
    jsr _spr_ctrl 
    // /mnt/huc/huc/examples/asm/elmer/rom-kickc-shmup/shmup.c:203
    lda #1
    ldy #OFFSET_STRUCT_SHIP_VX
    sta (sp),y
    // /mnt/huc/huc/examples/asm/elmer/rom-kickc-shmup/shmup.c:204
    lda #0
    ldy #OFFSET_STRUCT_SHIP_VY
    sta (sp),y
    // /mnt/huc/huc/examples/asm/elmer/rom-kickc-shmup/shmup.c:205
    tay
    lda #<-$20
    sta (sp),y
    iny
    lda #>-$20
    sta (sp),y
    jmp rand2101
  __b3:
    // /mnt/huc/huc/examples/asm/elmer/rom-kickc-shmup/shmup.c:197
    inc i
    bne !+
    inc i+1
  !:
    lda #SIZEOF_STRUCT_SHIP
    clc
    adc.z sp
    sta.z sp
    bcc !+
    inc.z sp+1
  !:
    jmp __b1
  .segment bss
    .label __7 = i
    __12: .word 0
    __14: .word 0
    __16: .word 0
    .label __17 = __16
    __20: .word 0
    __22: .word 0
    __25: .word 0
    __28: .word 0
    __32: .word 0
    .label __33 = __98
    .label __34 = __98
    .label __35 = __98
    __98: .word 0
    i: .word 0
    i_1: .word 0
    j: .word 0
}
.segment zp
  random: .fill 4, 0
.segment code
.encoding "ascii"
  str_hi: .text "HI"
  .byte 0
  str_game_over: .text "GAME OVER"
  .byte 0
.segment data
my_font:
.incbin "font8x8-ascii-bold-short.dat" 
bonkspr:
.incspr "charwalk.pcx",0,0,2,8 
bonkpal:
.incpal "charwalk.pcx" 
bulletspr:
.incspr "bullet.pcx",0,0,1,1 
bulletpal:
.incpal "bullet.pcx" 
shipspr:
.incspr "ship.pcx",0,0,2,8 
shippal:
.incpal "ship.pcx" 
explosionspr:
.incspr "explosion.pcx",0,0,2,16 
explosionpal:
.incpal "explosion.pcx" 
scene_chr:
.incchr "scene.png" 
scene_pal:
.incpal "scene.png" 
  .align 2
scene_bat:
.incbat "scene.png",4096,32,28 
  .align 2
  textpal: .word 0, 0, 0, 0, $c7, 1, $1ff, 0, 0, 0, 0, 0, 0, 0, 0, 0
.segment bss
  bullets: .fill 5*$a, 0
  ships: .fill 7*5, 0
  frames: .word 0
  score: .word 0
  hiscore: .word 0
