; ***************************************************************************
; ***************************************************************************
;
; TURBO-GRAFX / PC-ENGINE SUPER SYSTEM CARD 3.00 PATCH FOR TURBO EVERDRIVE 2
;
; Copyright John Brandwood 2015-2024.
;
; Distributed under the Boost Software License, Version 1.0.
; (See accompanying file LICENSE_1_0.txt or copy at
;   http://www.boost.org/LICENSE_1_0.txt)
;
; ***************************************************************************
; ***************************************************************************
;
; * Fixes problem with the Turbo Everdrive 2 and DUO/DUO-R/SuperCDROM RAM.
; * Allows the TED2 to be used as a Super System Card on a TurboGrafx CD.
; * Expands SCD RAM memory to 512KB for translations/homebrew.
; * 1MB RAM using a TED2CARD header supported by Mednafen-HappyEyes.
;
; ***************************************************************************
; ***************************************************************************
;
; Release History :
;
; 2015-11-24 - Initial Release 3.01
; 2019-01-07 - Release 3.02
;              Add patch for Dragon Slayer II crash.
; 2024-01-05 - Release 3.03
;              Add patch to fix System Card's CD_READ-to-MPR bug.
;
; ***************************************************************************
; ***************************************************************************
;
; It is important to use a patch like this to avoid CPU bus-fighting on the
; Turbo Everdrive 2 when both the TED and the DUO/DUO-R/SuperCDROM RAM are
; trying to respond to CPU read requests at the same time.
;
; This occurs beacuse the built-in SCD RAM on the DUO/DUO-R/SuperCDROM does
; not disable itself on the CARD-DETECT signal that the Turbo Everdrive uses
; to disable the built-in System Card.
;
; An unpatched System Card unlocks the SCD RAM by doing 2 writes to the SCD
; hardware register at $18c0.
;
; This patch changes the System Card's SCD RAM detection routine to remove
; that unlocking sequence and so avoids the damaging bus-fighting.
;
; It also changes the TED2 into a 1MB RAM configuration, and adds some
; code to repair the System Card after the "Gate of Thunder" game writes
; to System Card memory.
;
; GATE OF THUNDER problem with System Card in RAM:
;
;   "Gate Of Thunder" calls EX_MEMOPEN and then overwrites $FFF5 with the
;   SCD RAM starting bank number. $FFF5 normally contains $80.
;
;   "Gate Of Thunder" then immediately calls EX_SCRSIZE which is a good
;   place to patch in code to fix the damage.
;
;   EX_SCRSIZE is vector $e069, which points to code at $e280.
;

                .include  "pcengine.inc"
                .list

                ;
                ; Decide which platform to build for.
                ;
                ; Pick one, and only one!
                ;

JPN_SYSCARD     =       1

                ;
                ; Build on top of an original Super System Card image.
                ;

                .bank   0
                .org    $0000

                .if     JPN_SYSCARD

                ; SYSCARD3.JPN.PCE - MD5: 38179df8f4ac870017db21ebcbf53114

                .incbin "syscard3.jpn.pce"

SCRSIZ_OLDPATCH =       $e28e ; Patch STA $0002 in TED2 v3.02 patch!!!
SCRSIZ_NEWPATCH =       $e288 ; Patch STA $0000 in TED2 v3.03 patch!!!

MEMOPEN_ADR     =       $fe92
UNLOCK_TEST     =       $c86b
MESSAGE_ADR     =       $c9d1
RUNBOOT_ADR     =       $e206

CDREAD_PATCH    =       $ed8e
RESTORE_MPRx    =       $ed91
RETRY_CDREAD    =       $ecf1

SCRSIZ_FUNC     =       $e267 ; bank 0 ; repair this from TED2 v3.01 patch!!!
HACKINIT_ADR    =       $c887 ; bank 1 ; repair this from TED2 v3.01 patch!!!
DISPLAY_SUPER   =       $c950

                .else

                ; SYSCARD3.USA.PCE - MD5: 0754f903b52e3b3342202bdafb13efa5

                .incbin "syscard3.usa.pce"

SCRSIZ_OLDPATCH =       $e2a7 ; Patch STA $0002 in TED2 v3.02 patch!!!
SCRSIZ_NEWPATCH =       $e2a1 ; Patch STA $0000 in TED2 v3.03 patch!!!

MEMOPEN_ADR     =       $feab
UNLOCK_TEST     =       $c86b ; bank 1
MESSAGE_ADR     =       $c9c4 ; bank 1
RUNBOOT_ADR     =       $e21f

CDREAD_PATCH    =       $eda7
RESTORE_MPRx    =       $edaa
RETRY_CDREAD    =       $ed0a

SCRSIZ_FUNC     =       $e280 ; bank 0 ; repair this from TED2 v3.01 patch!!!
HACKINIT_ADR    =       $c887 ; bank 1 ; repair this from TED2 v3.01 patch!!!
DISPLAY_SUPER   =       $c943

                .endif

                ;
                ; If we're building for real hardware with the System
                ; Card in RAM then keep copies of the first and last
                ; 16-bytes of the original System Card so that we can
                ; restore any overwrites to registers or IRQ vectors.
                ;
                ; The idea is that these 2 backup areas and the signature
                ; area are all going to be at fixed locations in this and
                ; any future versions of the expanded-ram System Card so
                ; that programs can rely on them.
                ;

                .bank   0

                .if     JPN_SYSCARD

                .org    $ffb0

original_e000:  .db     $4c,$f3,$e0,$4c ; Original JPN ROM $e000-$e00f.
                .db     $e3,$e8,$4c,$8f
                .db     $eb,$4c,$05,$ec
                .db     $4c,$cb,$ed,$4c

                .org    $ffc0

original_fff0:  .db     $e9,$4c,$dc,$e9 ; Original JPN ROM $fff0-$ffff.
                .db     $68,$80,$36,$e7
                .db     $70,$e8,$b3,$e6
                .db     $a9,$e6,$f3,$e0

                .else

                .org    $ffb0

original_e000:  .db     $4c,$f3,$e0,$4c ; Original USA ROM $e000-$e00f.
                .db     $fc,$e8,$4c,$a8
                .db     $eb,$4c,$1e,$ec
                .db     $4c,$e4,$ed,$4c

                .org    $ffc0

original_fff0:  .db     $e9,$4c,$f5,$e9 ; Original USA ROM $fff0-$ffff.
                .db     $68,$80,$4f,$e7
                .db     $89,$e8,$cc,$e6
                .db     $c2,$e6,$f3,$e0

                .endif

                .org    $ffd0

                ;
                ; Signature (same address as McGenjin card).
                ;
                ; My patched version of Mednafen recognizes this signature
                ; as a sign to enable 1MB of RAM in banks $00-$7F, simulating
                ; the environment that we have when running on a TED2.
                ;
                ; See https://github.com/jbrandwood/mednafen-happyeyes
                ;

card_signature: .db     "TED2CARD"
                .db     $00             ; Capabilities byte 0.
                .db     $00             ; Capabilities byte 1.
                .db     $00             ; Capabilities byte 2.
                .db     $00             ; Capabilities byte 3.

                ;
                ; Hack EX_MEMOPEN to say that SCD RAM exists and is unlocked.
                ;

                .bank   0
                .org    MEMOPEN_ADR

my_ex_memopen:  lda     #$68            ; Return that 192KB of external SCD RAM
                ldx     #$03            ; exists and thus avoid unlocking the
                clc                     ; internal SCD RAM chip.
                rts

fix_ex_scrsiz:  sta     $0000           ; Exec ex_scrsiz instr that was patched.
                lda     #$80            ; Repair "Gate of Thunder" damage.
                sta     $FFF5
                rts

fix_ex_cdread:  jsr     RESTORE_MPRx    ; Fix Hudson's bug by restoring the old
                jmp     RETRY_CDREAD    ; MPRx value before the CD_READ retries.

scd_unlock_test:php                     ; Disable interrupts for the next bit.
                sei
                lda     #$A5            ; Unlock TED2 registers.
                sta     $E005
                lda     #$04            ; Select normal TED2 512KB banks.
                sta     $E007
                lda     #%00001011      ; FPGA=off+locked, RAM=writable.
                sta     $E006
                tii     $FFB0,$E000,8   ; Restore bytes damaged if the memory
                plp                     ; was already writable.
                bra     my_ex_memopen   ; Initialization complete.

                ; The original function is 48 bytes long.

                ds      my_ex_memopen + 48 - *

                ;
                ; If we're building for a TED2 or 1MB-RAM card then patch
                ; EX_SCRSIZ to repair the damage that "Gate of Thunder"
                ; does on startup.
                ;

                .bank   0
                .org    $e069
                jmp     SCRSIZ_FUNC

                .org    SCRSIZ_OLDPATCH
                sta     $0002

                .org    SCRSIZ_NEWPATCH
                jsr     fix_ex_scrsiz

                ;
                ; If we're building for a TED2 or 1MB-RAM card then patch the
                ; System Card's SCD RAM detect to do our initialization and
                ; to restore the original System Card contents in case it has
                ; been overwritten by the last program that ran.
                ;

                .bank   1
                .org    UNLOCK_TEST
                jsr     scd_unlock_test

                .org    HACKINIT_ADR
                jsr     DISPLAY_SUPER   ; Restore original code.

                ;
                ; Patch the System Card to check for game-specific fixes
                ; when it runs the IPL's boot-loader code.
                ;
                ; Bank $1F is mapped into MPR6 to perform the check for
                ; Hudson's licensed IPL boot-loader.
                ;

                .bank   0
                .org    RUNBOOT_ADR
                jsr     fix_bootload    ; Patch System Card's jump to boot loader.

                ;
                ; Fix Hudson's System Card bug where ex_cdread to an MPR fails
                ; to restore the original bank before retrying after an error.
                ;

                .bank   0
                .org    CDREAD_PATCH
                jmp     fix_ex_cdread

                ;
                ; Check if the IPL of the CD (loaded at $3000) matches
                ; any of the games that need to be fixed.
                ;

                .bank   31
                .org    $da58

fix_bootload:   ldx     #(game_fix - game_list)

.loop0:         ldy     #21             ; Compare IPL program name.
.loop1:         dex
                lda     $306A,y
                cmp     game_list,x
                bne     .wrong_game
                dey
                bpl     .loop1

                ldy     #9              ; Compare IPL boot location.
.loop2:         dex
                lda     $3000,y
                cmp     game_list,x
                bne     .wrong_game
                dey
                bpl     .loop2

                txa                     ; Found a matching game!
                lsr     a
                lsr     a
                lsr     a
                lsr     a
                tax
                jmp     [game_fix,x]    ; Game-specific patch for boot loader.

.wrong_game:    txa                     ; Try the next game.
                and     #$E0
                tax
                bne     .loop0

                jmp     $2B26           ; Goto the original code.

                ;
                ; Identifiers for a maximum of 8 Game-Specific Patches.
                ;
                ; 32-bytes per game ...
                ; ... 10 bytes of IPL boot location. ($3000..$3009)
                ; ... 22 bytes of IPL program name.  ($306A..$307F)
                ;

game_list:      .db  $00, $00, $02, $03, $00, $30, $00, $3A, $00, $01
                .db  "EIYU_DENSETSU_2 921107"

game_fix:       .dw     fix_dslayer2

                ;
                ; Fix for Dragon Slayer II.
                ;
                ; The game is testing for the extended RAM on a Development
                ; System and enabling some debugging code if it finds it.
                ;
                ; Having RAM in bank $4C fools the game into enabling that
                ; code, which then crashes the game just after the intro.
                ;

fix_dslayer2:   tii     .patch,$2CD1,(.done - .patch)
                jmp     $2B26           ; Execute the boot loader in RAM.

.patch:         bra     .code           ; This needs to be at $2CD1.
                nop
                rts                     ; This needs to be at $2CD4.

.code:          ldy     #15             ; Start of the the check code.
.loop:          lda     $412F,y
                cmp     $2CD1+(.test - .patch),y
                bne     .exec           ; Do not patch if different.
                dey
                bpl     .loop

                lda     #$80            ; Patch "BNE $415F" instruction
                sta     $4137           ; into  "BRA $415F".

.exec:          jmp     [$2007]         ; Execute the game's boot program.

                ; This is the game's boot code that is expected (@ $412F).

.test:          .db     $B2, $10, $49, $FF, $92, $10, $D2, $10
                .db     $D0, $26, $F7, $20, $AE, $11, $E0, $AD

.done:

                ;
                ; Make sure that the game-specific patches do not overrun
                ; the original ROM contents.
                ;

                .ds     $dae0-*         ; Generate error if too long.

                ;
                ; Change on-screen identification so that we know what this
                ; code has been built for.
                ;
                ; Originally "VER. 3.00".
                ;

                .bank   1
                .org    MESSAGE_ADR

                .db     "TED2 3.03"
