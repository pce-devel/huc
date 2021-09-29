; c:\Projects\PCE_DEV\Rick_HUC\huc\bin\mod2mml.exe -d 11=4,12=3,13=1,15=2 -a -n -v 3=28,1=28 -f 3=.25 -o bgm.asm ackerlight.mod 
_bgm:
	.dw _bgm_song
	.dw _bgm_wave_table
	.dw _bgm_vol_table
	.dw _bgm_chan_map

; Pattern 0
_bgm_pat0:
; ch 0
	.db $00, 7
	.dw 342
; ch 1
	.db $01, 255
	.dw 171
; ch 2 rest
	.db $ff
; ch 3
	.db $ec, 1
	.dw 287
; ch 0 rest
	.db $ff
; ch 1 rest
	.db $ff
; ch 2 rest
	.db $ff
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 14
	.dw 684
; ch 1 rest
	.db $ff
; ch 2
	.db $ec, 1
	.dw 287
; ch 3
	.db $ec, 1
	.dw 287
; ch 0 rest
	.db $ff
; ch 1 rest
	.db $ff
; ch 2 rest
	.db $ff
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 7
	.dw 342
; ch 1 rest
	.db $ff
; ch 2 rest
	.db $ff
; ch 3
	.db $ec, 1
	.dw 287
; ch 0 rest
	.db $ff
; ch 1 rest
	.db $ff
; ch 2 rest
	.db $ff
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 14
	.dw 684
; ch 1 rest
	.db $ff
; ch 2
	.db $ec, 1
	.dw 287
; ch 3
	.db $ec, 1
	.dw 287
; ch 0 rest
	.db $ff
; ch 1 rest
	.db $ff
; ch 2 rest
	.db $ff
; ch 3
	.db $ec, 1
	.dw 287
; ch 0
	.db $00, 7
	.dw 342
; ch 1 rest
	.db $ff
; ch 2 rest
	.db $ff
; ch 3
	.db $ec, 1
	.dw 287
; ch 0 rest
	.db $ff
; ch 1 rest
	.db $ff
; ch 2
	.db $ec, 1
	.dw 287
; ch 3
	.db $ec, 1
	.dw 287
; ch 0
	.db $00, 14
	.dw 684
; ch 1 rest
	.db $ff
; ch 2 rest
	.db $ff
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 7
	.dw 342
; ch 1 rest
	.db $ff
; ch 2 rest
	.db $ff
; ch 3
	.db $ec, 1
	.dw 287
; ch 0 rest
	.db $ff
; ch 1 rest
	.db $ff
; ch 2 rest
	.db $ff
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 14
	.dw 684
; ch 1 rest
	.db $ff
; ch 2 rest
	.db $ff
; ch 3
	.db $ec, 1
	.dw 287
; ch 0
	.db $00, 7
	.dw 342
; ch 1 rest
	.db $ff
; ch 2 rest
	.db $ff
; ch 3
	.db $ec, 1
	.dw 287
; ch 0
	.db $00, 14
	.dw 684
; ch 1 rest
	.db $ff
; ch 2 rest
	.db $ff
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 6
	.dw 287
; ch 1
	.db $01, 255
	.dw 144
; ch 2 rest
	.db $ff
; ch 3
	.db $ec, 1
	.dw 287
; ch 0 rest
	.db $ff
; ch 1 rest
	.db $ff
; ch 2 rest
	.db $ff
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 12
	.dw 575
; ch 1 rest
	.db $ff
; ch 2
	.db $ec, 1
	.dw 287
; ch 3
	.db $ec, 1
	.dw 287
; ch 0 rest
	.db $ff
; ch 1 rest
	.db $ff
; ch 2 rest
	.db $ff
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 6
	.dw 287
; ch 1 rest
	.db $ff
; ch 2 rest
	.db $ff
; ch 3
	.db $ec, 1
	.dw 287
; ch 0 rest
	.db $ff
; ch 1 rest
	.db $ff
; ch 2 rest
	.db $ff
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 12
	.dw 575
; ch 1 rest
	.db $ff
; ch 2
	.db $ec, 1
	.dw 287
; ch 3
	.db $ec, 1
	.dw 287
; ch 0 rest
	.db $ff
; ch 1 rest
	.db $ff
; ch 2 rest
	.db $ff
; ch 3
	.db $ec, 1
	.dw 287
; ch 0
	.db $00, 6
	.dw 304
; ch 1
	.db $01, 255
	.dw 152
; ch 2 rest
	.db $ff
; ch 3
	.db $ec, 1
	.dw 287
; ch 0 rest
	.db $ff
; ch 1 rest
	.db $ff
; ch 2
	.db $ec, 1
	.dw 287
; ch 3
	.db $ec, 1
	.dw 287
; ch 0
	.db $00, 13
	.dw 609
; ch 1 rest
	.db $ff
; ch 2 rest
	.db $ff
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 6
	.dw 304
; ch 1 rest
	.db $ff
; ch 2 rest
	.db $ff
; ch 3
	.db $ec, 1
	.dw 287
; ch 0 rest
	.db $ff
; ch 1 rest
	.db $ff
; ch 2 rest
	.db $ff
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 13
	.dw 609
; ch 1 rest
	.db $ff
; ch 2
	.db $ec, 1
	.dw 287
; ch 3
	.db $ec, 1
	.dw 287
; ch 0
	.db $00, 6
	.dw 304
; ch 1 rest
	.db $ff
; ch 2 rest
	.db $ff
; ch 3
	.db $ec, 1
	.dw 287
; ch 0
	.db $00, 13
	.dw 609
; ch 1 rest
	.db $ff
; ch 2 rest
	.db $ff
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 7
	.dw 342
; ch 1
	.db $01, 255
	.dw 171
; ch 2 rest
	.db $ff
; ch 3
	.db $ec, 1
	.dw 287
; ch 0 rest
	.db $ff
; ch 1 rest
	.db $ff
; ch 2 rest
	.db $ff
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 14
	.dw 684
; ch 1 rest
	.db $ff
; ch 2
	.db $ec, 1
	.dw 287
; ch 3
	.db $ec, 1
	.dw 287
; ch 0 rest
	.db $ff
; ch 1 rest
	.db $ff
; ch 2 rest
	.db $ff
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 7
	.dw 342
; ch 1 rest
	.db $ff
; ch 2 rest
	.db $ff
; ch 3
	.db $ec, 1
	.dw 287
; ch 0 rest
	.db $ff
; ch 1 rest
	.db $ff
; ch 2 rest
	.db $ff
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 14
	.dw 684
; ch 1 rest
	.db $ff
; ch 2
	.db $ec, 1
	.dw 287
; ch 3
	.db $ec, 1
	.dw 287
; ch 0 rest
	.db $ff
; ch 1 rest
	.db $ff
; ch 2 rest
	.db $ff
; ch 3
	.db $ec, 1
	.dw 287
; ch 0
	.db $00, 7
	.dw 342
; ch 1 rest
	.db $ff
; ch 2 rest
	.db $ff
; ch 3
	.db $ec, 1
	.dw 287
; ch 0 rest
	.db $ff
; ch 1 rest
	.db $ff
; ch 2
	.db $ec, 1
	.dw 287
; ch 3
	.db $ec, 1
	.dw 287
; ch 0
	.db $00, 14
	.dw 684
; ch 1 rest
	.db $ff
; ch 2 rest
	.db $ff
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 7
	.dw 342
; ch 1 rest
	.db $ff
; ch 2 rest
	.db $ff
; ch 3
	.db $ec, 1
	.dw 287
; ch 0 rest
	.db $ff
; ch 1 rest
	.db $ff
; ch 2 rest
	.db $ff
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 14
	.dw 684
; ch 1 rest
	.db $ff
; ch 2
	.db $ec, 1
	.dw 287
; ch 3
	.db $ec, 1
	.dw 287
; ch 0
	.db $00, 7
	.dw 342
; ch 1 rest
	.db $ff
; ch 2 rest
	.db $ff
; ch 3
	.db $ec, 1
	.dw 287
; ch 0
	.db $00, 14
	.dw 684
; ch 1 rest
	.db $ff
; ch 2
	.db $ec, 1
	.dw 287
; ch 3
	.db $ec, 1
	.dw 287
; ch 0
	.db $00, 8
	.dw 384
; ch 1
	.db $01, 255
	.dw 191
; ch 2 rest
	.db $ff
; ch 3
	.db $ec, 1
	.dw 287
; ch 0 rest
	.db $ff
; ch 1 rest
	.db $ff
; ch 2 rest
	.db $ff
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 16
	.dw 769
; ch 1 rest
	.db $ff
; ch 2 rest
	.db $ff
; ch 3
	.db $ec, 1
	.dw 287
; ch 0 rest
	.db $ff
; ch 1 rest
	.db $ff
; ch 2 rest
	.db $ff
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 8
	.dw 384
; ch 1 rest
	.db $ff
; ch 2
	.db $ec, 1
	.dw 287
; ch 3
	.db $ec, 1
	.dw 287
; ch 0 rest
	.db $ff
; ch 1 rest
	.db $ff
; ch 2 rest
	.db $ff
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 16
	.dw 769
; ch 1 rest
	.db $ff
; ch 2 rest
	.db $ff
; ch 3
	.db $ec, 1
	.dw 287
; ch 0 rest
	.db $ff
; ch 1 rest
	.db $ff
; ch 2
	.db $ec, 1
	.dw 287
; ch 3
	.db $ec, 1
	.dw 287
; ch 0
	.db $00, 7
	.dw 363
; ch 1
	.db $01, 255
	.dw 181
; ch 2 rest
	.db $ff
; ch 3
	.db $ec, 1
	.dw 287
; ch 0 rest
	.db $ff
; ch 1 rest
	.db $ff
; ch 2
	.db $ec, 1
	.dw 287
; ch 3
	.db $ec, 1
	.dw 287
; ch 0
	.db $00, 15
	.dw 726
; ch 1 rest
	.db $ff
; ch 2 rest
	.db $ff
; ch 3
	.db $ee, 1
	.dw 287
; ch 0
	.db $00, 7
	.dw 363
; ch 1 rest
	.db $ff
; ch 2
	.db $ee, 1
	.dw 287
; ch 3 rest
	.db $ff
; ch 0 rest
	.db $ff
; ch 1 rest
	.db $ff
; ch 2 rest
	.db $ff
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 15
	.dw 726
; ch 1 rest
	.db $ff
; ch 2 rest
	.db $ff
; ch 3
	.db $ee, 1
	.dw 287
; ch 0
	.db $00, 7
	.dw 363
; ch 1 rest
	.db $ff
; ch 2
	.db $ee, 1
	.dw 287
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 15
	.dw 726
; ch 1 rest
	.db $ff
; ch 2 rest
	.db $ff
; ch 3
	.db $ee, 1
	.dw 287
; ch 0 rest
	.db $ff
; ch 1 rest
	.db $ff
; ch 2 rest
	.db $ff
; ch 3 rest
	.db $ff

; Pattern 1
_bgm_pat1:
; ch 0
	.db $00, 7
	.dw 342
; ch 1
	.db $01, 255
	.dw 171
; ch 2
	.db $02, 52
	.dw 2739
; ch 3
	.db $ea, 2
	.dw 322
; ch 0 rest
	.db $ff
; ch 1 rest
	.db $ff
; ch 2 rest
	.db $ff
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 14
	.dw 684
; ch 1 rest
	.db $ff
; ch 2
	.db $02, 25
	.dw 1369
; ch 3 rest
	.db $ff
; ch 0 rest
	.db $ff
; ch 1 rest
	.db $ff
; ch 2 rest
	.db $ff
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 7
	.dw 342
; ch 1 rest
	.db $ff
; ch 2
	.db $02, 52
	.dw 2739
; ch 3
	.db $eb, 1
	.dw 342
; ch 0 rest
	.db $ff
; ch 1 rest
	.db $ff
; ch 2 rest
	.db $ff
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 14
	.dw 684
; ch 1 rest
	.db $ff
; ch 2
	.db $02, 25
	.dw 1369
; ch 3 rest
	.db $ff
; ch 0 rest
	.db $ff
; ch 1 rest
	.db $ff
; ch 2
	.db $02, 52
	.dw 2739
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 7
	.dw 342
; ch 1 rest
	.db $ff
; ch 2 rest
	.db $ff
; ch 3
	.db $ea, 2
	.dw 322
; ch 0 rest
	.db $ff
; ch 1 rest
	.db $ff
; ch 2
	.db $02, 52
	.dw 2739
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 14
	.dw 684
; ch 1 rest
	.db $ff
; ch 2
	.db $02, 25
	.dw 1369
; ch 3
	.db $ea, 2
	.dw 322
; ch 0
	.db $00, 7
	.dw 342
; ch 1 rest
	.db $ff
; ch 2 rest
	.db $ff
; ch 3 rest
	.db $ff
; ch 0 rest
	.db $ff
; ch 1 rest
	.db $ff
; ch 2
	.db $02, 52
	.dw 2739
; ch 3
	.db $eb, 1
	.dw 342
; ch 0
	.db $00, 14
	.dw 684
; ch 1 rest
	.db $ff
; ch 2
	.db $02, 25
	.dw 1369
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 7
	.dw 342
; ch 1 rest
	.db $ff
; ch 2
	.db $02, 52
	.dw 2739
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 14
	.dw 684
; ch 1 rest
	.db $ff
; ch 2
	.db $02, 25
	.dw 1369
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 6
	.dw 287
; ch 1
	.db $01, 255
	.dw 144
; ch 2
	.db $02, 43
	.dw 2302
; ch 3
	.db $ea, 2
	.dw 322
; ch 0 rest
	.db $ff
; ch 1 rest
	.db $ff
; ch 2 rest
	.db $ff
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 12
	.dw 575
; ch 1 rest
	.db $ff
; ch 2
	.db $02, 21
	.dw 1150
; ch 3 rest
	.db $ff
; ch 0 rest
	.db $ff
; ch 1 rest
	.db $ff
; ch 2 rest
	.db $ff
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 6
	.dw 287
; ch 1 rest
	.db $ff
; ch 2
	.db $02, 43
	.dw 2302
; ch 3
	.db $eb, 1
	.dw 342
; ch 0 rest
	.db $ff
; ch 1 rest
	.db $ff
; ch 2 rest
	.db $ff
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 12
	.dw 575
; ch 1 rest
	.db $ff
; ch 2
	.db $02, 21
	.dw 1150
; ch 3 rest
	.db $ff
; ch 0 rest
	.db $ff
; ch 1 rest
	.db $ff
; ch 2 rest
	.db $ff
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 6
	.dw 304
; ch 1
	.db $01, 255
	.dw 152
; ch 2
	.db $02, 45
	.dw 2438
; ch 3
	.db $ea, 2
	.dw 322
; ch 0 rest
	.db $ff
; ch 1 rest
	.db $ff
; ch 2 rest
	.db $ff
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 13
	.dw 609
; ch 1 rest
	.db $ff
; ch 2
	.db $02, 22
	.dw 1219
; ch 3
	.db $eb, 1
	.dw 342
; ch 0
	.db $00, 6
	.dw 304
; ch 1 rest
	.db $ff
; ch 2
	.db $02, 45
	.dw 2438
; ch 3
	.db $eb, 1
	.dw 342
; ch 0 rest
	.db $ff
; ch 1 rest
	.db $ff
; ch 2 rest
	.db $ff
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 13
	.dw 609
; ch 1 rest
	.db $ff
; ch 2
	.db $02, 45
	.dw 2438
; ch 3
	.db $eb, 1
	.dw 342
; ch 0
	.db $00, 6
	.dw 304
; ch 1 rest
	.db $ff
; ch 2
	.db $02, 22
	.dw 1219
; ch 3
	.db $eb, 1
	.dw 342
; ch 0
	.db $00, 13
	.dw 609
; ch 1 rest
	.db $ff
; ch 2
	.db $02, 45
	.dw 2438
; ch 3
	.db $eb, 1
	.dw 342
; ch 0
	.db $00, 7
	.dw 342
; ch 1
	.db $01, 255
	.dw 171
; ch 2
	.db $02, 52
	.dw 2739
; ch 3
	.db $ea, 2
	.dw 322
; ch 0 rest
	.db $ff
; ch 1 rest
	.db $ff
; ch 2 rest
	.db $ff
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 14
	.dw 684
; ch 1 rest
	.db $ff
; ch 2
	.db $02, 25
	.dw 1369
; ch 3 rest
	.db $ff
; ch 0 rest
	.db $ff
; ch 1 rest
	.db $ff
; ch 2 rest
	.db $ff
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 7
	.dw 342
; ch 1 rest
	.db $ff
; ch 2
	.db $02, 52
	.dw 2739
; ch 3
	.db $eb, 1
	.dw 342
; ch 0 rest
	.db $ff
; ch 1 rest
	.db $ff
; ch 2 rest
	.db $ff
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 14
	.dw 684
; ch 1 rest
	.db $ff
; ch 2
	.db $02, 25
	.dw 1369
; ch 3 rest
	.db $ff
; ch 0 rest
	.db $ff
; ch 1 rest
	.db $ff
; ch 2
	.db $02, 52
	.dw 2739
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 7
	.dw 342
; ch 1 rest
	.db $ff
; ch 2 rest
	.db $ff
; ch 3
	.db $ea, 2
	.dw 322
; ch 0 rest
	.db $ff
; ch 1 rest
	.db $ff
; ch 2
	.db $02, 52
	.dw 2739
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 14
	.dw 684
; ch 1 rest
	.db $ff
; ch 2
	.db $02, 25
	.dw 1369
; ch 3
	.db $ea, 2
	.dw 322
; ch 0
	.db $00, 7
	.dw 342
; ch 1 rest
	.db $ff
; ch 2
	.db $02, 52
	.dw 2739
; ch 3 rest
	.db $ff
; ch 0 rest
	.db $ff
; ch 1 rest
	.db $ff
; ch 2 rest
	.db $ff
; ch 3
	.db $eb, 1
	.dw 342
; ch 0
	.db $00, 14
	.dw 684
; ch 1 rest
	.db $ff
; ch 2
	.db $02, 52
	.dw 2739
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 7
	.dw 342
; ch 1 rest
	.db $ff
; ch 2
	.db $02, 25
	.dw 1369
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 14
	.dw 684
; ch 1 rest
	.db $ff
; ch 2
	.db $02, 52
	.dw 2739
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 8
	.dw 384
; ch 1
	.db $01, 255
	.dw 191
; ch 2
	.db $02, 57
	.dw 3078
; ch 3
	.db $ea, 2
	.dw 322
; ch 0 rest
	.db $ff
; ch 1 rest
	.db $ff
; ch 2 rest
	.db $ff
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 16
	.dw 769
; ch 1 rest
	.db $ff
; ch 2
	.db $02, 28
	.dw 1538
; ch 3 rest
	.db $ff
; ch 0 rest
	.db $ff
; ch 1 rest
	.db $ff
; ch 2 rest
	.db $ff
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 8
	.dw 384
; ch 1 rest
	.db $ff
; ch 2
	.db $02, 57
	.dw 3078
; ch 3
	.db $eb, 1
	.dw 342
; ch 0 rest
	.db $ff
; ch 1 rest
	.db $ff
; ch 2 rest
	.db $ff
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 16
	.dw 769
; ch 1 rest
	.db $ff
; ch 2
	.db $02, 28
	.dw 1538
; ch 3 rest
	.db $ff
; ch 0 rest
	.db $ff
; ch 1 rest
	.db $ff
; ch 2 rest
	.db $ff
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 7
	.dw 363
; ch 1
	.db $01, 255
	.dw 181
; ch 2
	.db $02, 54
	.dw 2908
; ch 3
	.db $ea, 2
	.dw 322
; ch 0 rest
	.db $ff
; ch 1 rest
	.db $ff
; ch 2 rest
	.db $ff
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 15
	.dw 726
; ch 1 rest
	.db $ff
; ch 2
	.db $02, 26
	.dw 1453
; ch 3
	.db $ea, 2
	.dw 322
; ch 0
	.db $00, 7
	.dw 363
; ch 1 rest
	.db $ff
; ch 2
	.db $02, 54
	.dw 2908
; ch 3
	.db $eb, 1
	.dw 322
; ch 0 rest
	.db $ff
; ch 1 rest
	.db $ff
; ch 2 rest
	.db $ff
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 15
	.dw 726
; ch 1 rest
	.db $ff
; ch 2
	.db $02, 54
	.dw 2908
; ch 3
	.db $eb, 1
	.dw 342
; ch 0
	.db $00, 7
	.dw 363
; ch 1 rest
	.db $ff
; ch 2
	.db $02, 26
	.dw 1453
; ch 3
	.db $eb, 1
	.dw 342
; ch 0
	.db $00, 15
	.dw 726
; ch 1 rest
	.db $ff
; ch 2
	.db $02, 54
	.dw 2908
; ch 3
	.db $eb, 1
	.dw 342
; ch 0 rest
	.db $ff
; ch 1 rest
	.db $ff
; ch 2 rest
	.db $ff
; ch 3 rest
	.db $ff

; Pattern 2
_bgm_pat2:
; ch 0
	.db $00, 7
	.dw 342
; ch 1
	.db $03, 9
	.dw 342
; ch 2
	.db $02, 52
	.dw 2739
; ch 3
	.db $ea, 2
	.dw 322
; ch 0 rest
	.db $ff
; ch 1 rest
	.db $ff
; ch 2 rest
	.db $ff
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 14
	.dw 684
; ch 1
	.db $03, 9
	.dw 342
; ch 2
	.db $02, 25
	.dw 1369
; ch 3 rest
	.db $ff
; ch 0 rest
	.db $ff
; ch 1
	.db $03, 7
	.dw 287
; ch 2 rest
	.db $ff
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 7
	.dw 342
; ch 1 rest
	.db $ff
; ch 2
	.db $02, 52
	.dw 2739
; ch 3
	.db $eb, 1
	.dw 342
; ch 0 rest
	.db $ff
; ch 1
	.db $03, 9
	.dw 342
; ch 2 rest
	.db $ff
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 14
	.dw 684
; ch 1
	.db $03, 7
	.dw 287
; ch 2
	.db $02, 25
	.dw 1369
; ch 3 rest
	.db $ff
; ch 0 rest
	.db $ff
; ch 1
	.db $03, 6
	.dw 228
; ch 2
	.db $02, 52
	.dw 2739
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 7
	.dw 342
; ch 1
	.db $03, 9
	.dw 342
; ch 2 rest
	.db $ff
; ch 3
	.db $ea, 2
	.dw 322
; ch 0 rest
	.db $ff
; ch 1 rest
	.db $ff
; ch 2
	.db $02, 52
	.dw 2739
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 14
	.dw 684
; ch 1
	.db $03, 9
	.dw 342
; ch 2
	.db $02, 25
	.dw 1369
; ch 3
	.db $ea, 2
	.dw 322
; ch 0
	.db $00, 7
	.dw 342
; ch 1
	.db $03, 7
	.dw 287
; ch 2 rest
	.db $ff
; ch 3 rest
	.db $ff
; ch 0 rest
	.db $ff
; ch 1 rest
	.db $ff
; ch 2
	.db $02, 52
	.dw 2739
; ch 3
	.db $eb, 1
	.dw 342
; ch 0
	.db $00, 14
	.dw 684
; ch 1
	.db $03, 9
	.dw 342
; ch 2
	.db $02, 25
	.dw 1369
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 7
	.dw 342
; ch 1
	.db $03, 7
	.dw 287
; ch 2
	.db $02, 52
	.dw 2739
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 14
	.dw 684
; ch 1
	.db $03, 6
	.dw 228
; ch 2
	.db $02, 25
	.dw 1369
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 6
	.dw 287
; ch 1
	.db $03, 6
	.dw 256
; ch 2
	.db $02, 43
	.dw 2302
; ch 3
	.db $ea, 2
	.dw 322
; ch 0 rest
	.db $ff
; ch 1
	.db $03, 7
	.dw 287
; ch 2 rest
	.db $ff
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 12
	.dw 575
; ch 1
	.db $03, 10
	.dw 384
; ch 2
	.db $02, 21
	.dw 1150
; ch 3 rest
	.db $ff
; ch 0 rest
	.db $ff
; ch 1 rest
	.db $ff
; ch 2 rest
	.db $ff
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 6
	.dw 287
; ch 1 rest
	.db $ff
; ch 2
	.db $02, 43
	.dw 2302
; ch 3
	.db $eb, 1
	.dw 342
; ch 0 rest
	.db $ff
; ch 1 rest
	.db $ff
; ch 2 rest
	.db $ff
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 12
	.dw 575
; ch 1 rest
	.db $ff
; ch 2
	.db $02, 21
	.dw 1150
; ch 3 rest
	.db $ff
; ch 0 rest
	.db $ff
; ch 1 rest
	.db $ff
; ch 2 rest
	.db $ff
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 6
	.dw 304
; ch 1
	.db $03, 8
	.dw 304
; ch 2
	.db $02, 45
	.dw 2438
; ch 3
	.db $ea, 2
	.dw 322
; ch 0 rest
	.db $ff
; ch 1 rest
	.db $ff
; ch 2 rest
	.db $ff
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 13
	.dw 609
; ch 1
	.db $03, 7
	.dw 287
; ch 2
	.db $02, 22
	.dw 1219
; ch 3
	.db $eb, 1
	.dw 342
; ch 0
	.db $00, 6
	.dw 304
; ch 1 rest
	.db $ff
; ch 2
	.db $02, 45
	.dw 2438
; ch 3
	.db $eb, 1
	.dw 342
; ch 0 rest
	.db $ff
; ch 1
	.db $03, 6
	.dw 256
; ch 2 rest
	.db $ff
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 13
	.dw 609
; ch 1 rest
	.db $ff
; ch 2
	.db $02, 45
	.dw 2438
; ch 3
	.db $eb, 1
	.dw 342
; ch 0
	.db $00, 6
	.dw 304
; ch 1
	.db $03, 6
	.dw 256
; ch 2
	.db $02, 22
	.dw 1219
; ch 3
	.db $eb, 1
	.dw 342
; ch 0
	.db $00, 13
	.dw 609
; ch 1
	.db $03, 7
	.dw 287
; ch 2
	.db $02, 45
	.dw 2438
; ch 3
	.db $eb, 1
	.dw 342
; ch 0
	.db $00, 7
	.dw 342
; ch 1
	.db $03, 9
	.dw 342
; ch 2
	.db $02, 52
	.dw 2739
; ch 3
	.db $ea, 2
	.dw 322
; ch 0 rest
	.db $ff
; ch 1 rest
	.db $ff
; ch 2 rest
	.db $ff
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 14
	.dw 684
; ch 1
	.db $03, 10
	.dw 384
; ch 2
	.db $02, 25
	.dw 1369
; ch 3 rest
	.db $ff
; ch 0 rest
	.db $ff
; ch 1
	.db $03, 9
	.dw 342
; ch 2 rest
	.db $ff
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 7
	.dw 342
; ch 1 rest
	.db $ff
; ch 2
	.db $02, 52
	.dw 2739
; ch 3
	.db $eb, 1
	.dw 342
; ch 0 rest
	.db $ff
; ch 1 rest
	.db $ff
; ch 2 rest
	.db $ff
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 14
	.dw 684
; ch 1 rest
	.db $ff
; ch 2
	.db $02, 25
	.dw 1369
; ch 3 rest
	.db $ff
; ch 0 rest
	.db $ff
; ch 1
	.db $03, 10
	.dw 384
; ch 2
	.db $02, 52
	.dw 2739
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 7
	.dw 342
; ch 1
	.db $03, 9
	.dw 342
; ch 2 rest
	.db $ff
; ch 3
	.db $ea, 2
	.dw 322
; ch 0 rest
	.db $ff
; ch 1
	.db $03, 10
	.dw 384
; ch 2
	.db $02, 52
	.dw 2739
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 14
	.dw 684
; ch 1
	.db $03, 9
	.dw 342
; ch 2
	.db $02, 25
	.dw 1369
; ch 3
	.db $ea, 2
	.dw 322
; ch 0
	.db $00, 7
	.dw 342
; ch 1 rest
	.db $ff
; ch 2
	.db $02, 52
	.dw 2739
; ch 3 rest
	.db $ff
; ch 0 rest
	.db $ff
; ch 1
	.db $03, 10
	.dw 384
; ch 2 rest
	.db $ff
; ch 3
	.db $eb, 1
	.dw 342
; ch 0
	.db $00, 14
	.dw 684
; ch 1
	.db $03, 9
	.dw 342
; ch 2
	.db $02, 52
	.dw 2739
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 7
	.dw 342
; ch 1
	.db $03, 12
	.dw 457
; ch 2
	.db $02, 25
	.dw 1369
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 14
	.dw 684
; ch 1
	.db $03, 10
	.dw 384
; ch 2
	.db $02, 52
	.dw 2739
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 8
	.dw 384
; ch 1 rest
	.db $ff
; ch 2
	.db $02, 57
	.dw 3078
; ch 3
	.db $ea, 2
	.dw 322
; ch 0 rest
	.db $ff
; ch 1 rest
	.db $ff
; ch 2 rest
	.db $ff
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 16
	.dw 769
; ch 1 rest
	.db $ff
; ch 2
	.db $02, 28
	.dw 1538
; ch 3 rest
	.db $ff
; ch 0 rest
	.db $ff
; ch 1
	.db $03, 6
	.dw 256
; ch 2 rest
	.db $ff
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 8
	.dw 384
; ch 1 rest
	.db $ff
; ch 2
	.db $02, 57
	.dw 3078
; ch 3
	.db $eb, 1
	.dw 342
; ch 0 rest
	.db $ff
; ch 1
	.db $03, 6
	.dw 256
; ch 2 rest
	.db $ff
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 16
	.dw 769
; ch 1
	.db $03, 7
	.dw 287
; ch 2
	.db $02, 28
	.dw 1538
; ch 3 rest
	.db $ff
; ch 0 rest
	.db $ff
; ch 1 rest
	.db $ff
; ch 2 rest
	.db $ff
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 7
	.dw 363
; ch 1
	.db $03, 8
	.dw 304
; ch 2
	.db $02, 54
	.dw 2908
; ch 3
	.db $ea, 2
	.dw 322
; ch 0 rest
	.db $ff
; ch 1 rest
	.db $ff
; ch 2 rest
	.db $ff
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 15
	.dw 726
; ch 1
	.db $03, 7
	.dw 287
; ch 2
	.db $02, 26
	.dw 1453
; ch 3
	.db $ea, 2
	.dw 322
; ch 0
	.db $00, 7
	.dw 363
; ch 1
	.db $03, 8
	.dw 304
; ch 2
	.db $02, 54
	.dw 2908
; ch 3
	.db $eb, 1
	.dw 322
; ch 0 rest
	.db $ff
; ch 1 rest
	.db $ff
; ch 2 rest
	.db $ff
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 15
	.dw 726
; ch 1
	.db $03, 7
	.dw 287
; ch 2
	.db $02, 54
	.dw 2908
; ch 3
	.db $eb, 1
	.dw 342
; ch 0
	.db $00, 7
	.dw 363
; ch 1
	.db $03, 8
	.dw 304
; ch 2
	.db $02, 26
	.dw 1453
; ch 3
	.db $eb, 1
	.dw 342
; ch 0
	.db $00, 15
	.dw 726
; ch 1
	.db $03, 10
	.dw 384
; ch 2
	.db $02, 54
	.dw 2908
; ch 3
	.db $eb, 1
	.dw 342
; ch 0 rest
	.db $ff
; ch 1 rest
	.db $ff
; ch 2 rest
	.db $ff
; ch 3 rest
	.db $ff

; Pattern 3
_bgm_pat3:
; ch 0
	.db $00, 7
	.dw 342
; ch 1
	.db $03, 9
	.dw 342
; ch 2
	.db $02, 52
	.dw 2739
; ch 3
	.db $ea, 2
	.dw 322
; ch 0 rest
	.db $ff
; ch 1 rest
	.db $ff
; ch 2 rest
	.db $ff
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 14
	.dw 684
; ch 1
	.db $03, 9
	.dw 342
; ch 2
	.db $02, 25
	.dw 1369
; ch 3 rest
	.db $ff
; ch 0 rest
	.db $ff
; ch 1
	.db $03, 7
	.dw 287
; ch 2 rest
	.db $ff
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 7
	.dw 342
; ch 1 rest
	.db $ff
; ch 2
	.db $02, 52
	.dw 2739
; ch 3
	.db $eb, 1
	.dw 342
; ch 0 rest
	.db $ff
; ch 1
	.db $03, 9
	.dw 342
; ch 2 rest
	.db $ff
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 14
	.dw 684
; ch 1
	.db $03, 7
	.dw 287
; ch 2
	.db $02, 25
	.dw 1369
; ch 3 rest
	.db $ff
; ch 0 rest
	.db $ff
; ch 1
	.db $03, 6
	.dw 228
; ch 2
	.db $02, 52
	.dw 2739
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 7
	.dw 342
; ch 1
	.db $03, 9
	.dw 342
; ch 2 rest
	.db $ff
; ch 3
	.db $ea, 2
	.dw 322
; ch 0 rest
	.db $ff
; ch 1 rest
	.db $ff
; ch 2
	.db $02, 52
	.dw 2739
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 14
	.dw 684
; ch 1
	.db $03, 9
	.dw 342
; ch 2
	.db $02, 25
	.dw 1369
; ch 3
	.db $ea, 2
	.dw 322
; ch 0
	.db $00, 7
	.dw 342
; ch 1
	.db $03, 7
	.dw 287
; ch 2 rest
	.db $ff
; ch 3 rest
	.db $ff
; ch 0 rest
	.db $ff
; ch 1 rest
	.db $ff
; ch 2
	.db $02, 52
	.dw 2739
; ch 3
	.db $eb, 1
	.dw 342
; ch 0
	.db $00, 14
	.dw 684
; ch 1
	.db $03, 9
	.dw 342
; ch 2
	.db $02, 25
	.dw 1369
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 7
	.dw 342
; ch 1
	.db $03, 7
	.dw 287
; ch 2
	.db $02, 52
	.dw 2739
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 14
	.dw 684
; ch 1
	.db $03, 6
	.dw 228
; ch 2
	.db $02, 25
	.dw 1369
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 6
	.dw 287
; ch 1
	.db $03, 6
	.dw 256
; ch 2
	.db $02, 43
	.dw 2302
; ch 3
	.db $ea, 2
	.dw 322
; ch 0 rest
	.db $ff
; ch 1
	.db $03, 7
	.dw 287
; ch 2 rest
	.db $ff
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 12
	.dw 575
; ch 1
	.db $03, 10
	.dw 384
; ch 2
	.db $02, 21
	.dw 1150
; ch 3 rest
	.db $ff
; ch 0 rest
	.db $ff
; ch 1 rest
	.db $ff
; ch 2 rest
	.db $ff
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 6
	.dw 287
; ch 1 rest
	.db $ff
; ch 2
	.db $02, 43
	.dw 2302
; ch 3
	.db $eb, 1
	.dw 342
; ch 0 rest
	.db $ff
; ch 1 rest
	.db $ff
; ch 2 rest
	.db $ff
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 12
	.dw 575
; ch 1 rest
	.db $ff
; ch 2
	.db $02, 21
	.dw 1150
; ch 3 rest
	.db $ff
; ch 0 rest
	.db $ff
; ch 1 rest
	.db $ff
; ch 2 rest
	.db $ff
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 6
	.dw 304
; ch 1
	.db $03, 8
	.dw 304
; ch 2
	.db $02, 45
	.dw 2438
; ch 3
	.db $ea, 2
	.dw 322
; ch 0 rest
	.db $ff
; ch 1 rest
	.db $ff
; ch 2 rest
	.db $ff
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 13
	.dw 609
; ch 1
	.db $03, 7
	.dw 287
; ch 2
	.db $02, 22
	.dw 1219
; ch 3
	.db $eb, 1
	.dw 342
; ch 0
	.db $00, 6
	.dw 304
; ch 1 rest
	.db $ff
; ch 2
	.db $02, 45
	.dw 2438
; ch 3
	.db $eb, 1
	.dw 342
; ch 0 rest
	.db $ff
; ch 1
	.db $03, 6
	.dw 256
; ch 2 rest
	.db $ff
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 13
	.dw 609
; ch 1 rest
	.db $ff
; ch 2
	.db $02, 45
	.dw 2438
; ch 3
	.db $eb, 1
	.dw 342
; ch 0
	.db $00, 6
	.dw 304
; ch 1
	.db $03, 6
	.dw 256
; ch 2
	.db $02, 22
	.dw 1219
; ch 3
	.db $eb, 1
	.dw 342
; ch 0
	.db $00, 13
	.dw 609
; ch 1
	.db $03, 7
	.dw 287
; ch 2
	.db $02, 45
	.dw 2438
; ch 3
	.db $eb, 1
	.dw 342
; ch 0
	.db $00, 7
	.dw 342
; ch 1
	.db $03, 9
	.dw 342
; ch 2
	.db $02, 52
	.dw 2739
; ch 3
	.db $ea, 2
	.dw 322
; ch 0 rest
	.db $ff
; ch 1
	.db $03, 10
	.dw 384
; ch 2 rest
	.db $ff
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 14
	.dw 684
; ch 1
	.db $03, 12
	.dw 457
; ch 2
	.db $02, 25
	.dw 1369
; ch 3 rest
	.db $ff
; ch 0 rest
	.db $ff
; ch 1 rest
	.db $ff
; ch 2 rest
	.db $ff
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 7
	.dw 342
; ch 1 rest
	.db $ff
; ch 2
	.db $02, 52
	.dw 2739
; ch 3
	.db $eb, 1
	.dw 342
; ch 0 rest
	.db $ff
; ch 1 rest
	.db $ff
; ch 2 rest
	.db $ff
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 14
	.dw 684
; ch 1 rest
	.db $ff
; ch 2
	.db $02, 25
	.dw 1369
; ch 3 rest
	.db $ff
; ch 0 rest
	.db $ff
; ch 1
	.db $03, 12
	.dw 457
; ch 2
	.db $02, 52
	.dw 2739
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 7
	.dw 342
; ch 1
	.db $03, 12
	.dw 457
; ch 2 rest
	.db $ff
; ch 3
	.db $ea, 2
	.dw 322
; ch 0 rest
	.db $ff
; ch 1 rest
	.db $ff
; ch 2
	.db $02, 52
	.dw 2739
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 14
	.dw 684
; ch 1
	.db $03, 10
	.dw 384
; ch 2
	.db $02, 25
	.dw 1369
; ch 3
	.db $ea, 2
	.dw 322
; ch 0
	.db $00, 7
	.dw 342
; ch 1
	.db $03, 12
	.dw 457
; ch 2
	.db $02, 52
	.dw 2739
; ch 3 rest
	.db $ff
; ch 0 rest
	.db $ff
; ch 1 rest
	.db $ff
; ch 2 rest
	.db $ff
; ch 3
	.db $eb, 1
	.dw 342
; ch 0
	.db $00, 14
	.dw 684
; ch 1
	.db $03, 10
	.dw 384
; ch 2
	.db $02, 52
	.dw 2739
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 7
	.dw 342
; ch 1
	.db $03, 12
	.dw 457
; ch 2
	.db $02, 25
	.dw 1369
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 14
	.dw 684
; ch 1
	.db $03, 15
	.dw 575
; ch 2
	.db $02, 52
	.dw 2739
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 8
	.dw 384
; ch 1
	.db $03, 13
	.dw 512
; ch 2
	.db $02, 57
	.dw 3078
; ch 3
	.db $ea, 2
	.dw 322
; ch 0 rest
	.db $ff
; ch 1 rest
	.db $ff
; ch 2 rest
	.db $ff
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 16
	.dw 769
; ch 1
	.db $03, 13
	.dw 512
; ch 2
	.db $02, 28
	.dw 1538
; ch 3 rest
	.db $ff
; ch 0 rest
	.db $ff
; ch 1
	.db $03, 12
	.dw 457
; ch 2 rest
	.db $ff
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 8
	.dw 384
; ch 1 rest
	.db $ff
; ch 2
	.db $02, 57
	.dw 3078
; ch 3
	.db $eb, 1
	.dw 342
; ch 0 rest
	.db $ff
; ch 1
	.db $03, 13
	.dw 512
; ch 2 rest
	.db $ff
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 16
	.dw 769
; ch 1
	.db $03, 15
	.dw 575
; ch 2
	.db $02, 28
	.dw 1538
; ch 3 rest
	.db $ff
; ch 0 rest
	.db $ff
; ch 1
	.db $03, 20
	.dw 769
; ch 2 rest
	.db $ff
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 7
	.dw 342
; ch 1
	.db $03, 18
	.dw 684
; ch 2
	.db $02, 52
	.dw 2739
; ch 3
	.db $ea, 2
	.dw 322
; ch 0 rest
	.db $ff
; ch 1 rest
	.db $ff
; ch 2 rest
	.db $ff
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 16
	.dw 769
; ch 1
	.db $03, 20
	.dw 769
; ch 2
	.db $02, 57
	.dw 3078
; ch 3
	.db $ea, 2
	.dw 322
; ch 0
	.db $00, 14
	.dw 684
; ch 1
	.db $03, 18
	.dw 684
; ch 2
	.db $02, 52
	.dw 2739
; ch 3
	.db $eb, 1
	.dw 322
; ch 0 rest
	.db $ff
; ch 1 rest
	.db $ff
; ch 2 rest
	.db $ff
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 16
	.dw 769
; ch 1
	.db $03, 20
	.dw 769
; ch 2
	.db $02, 57
	.dw 3078
; ch 3
	.db $eb, 1
	.dw 342
; ch 0
	.db $00, 14
	.dw 684
; ch 1
	.db $03, 18
	.dw 684
; ch 2
	.db $02, 52
	.dw 2739
; ch 3
	.db $eb, 1
	.dw 342
; ch 0
	.db $00, 16
	.dw 769
; ch 1
	.db $03, 20
	.dw 769
; ch 2
	.db $02, 57
	.dw 3078
; ch 3
	.db $eb, 1
	.dw 342
; ch 0 rest
	.db $ff
; ch 1 rest
	.db $ff
; ch 2 rest
	.db $ff
; ch 3 rest
	.db $ff

; Pattern 4
_bgm_pat4:
; ch 0
	.db $00, 5
	.dw 256
; ch 1
	.db $03, 6
	.dw 256
; ch 2
	.db $02, 37
	.dw 2051
; ch 3
	.db $ea, 2
	.dw 322
; ch 0 rest
	.db $ff
; ch 1 rest
	.db $ff
; ch 2 rest
	.db $ff
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 11
	.dw 512
; ch 1
	.db $03, 7
	.dw 287
; ch 2
	.db $02, 18
	.dw 1025
; ch 3 rest
	.db $ff
; ch 0 rest
	.db $ff
; ch 1 rest
	.db $ff
; ch 2 rest
	.db $ff
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 5
	.dw 256
; ch 1
	.db $03, 6
	.dw 256
; ch 2
	.db $02, 37
	.dw 2051
; ch 3
	.db $eb, 1
	.dw 342
; ch 0 rest
	.db $ff
; ch 1 rest
	.db $ff
; ch 2 rest
	.db $ff
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 7
	.dw 342
; ch 1
	.db $03, 9
	.dw 342
; ch 2
	.db $02, 18
	.dw 1025
; ch 3 rest
	.db $ff
; ch 0 rest
	.db $ff
; ch 1 rest
	.db $ff
; ch 2
	.db $02, 37
	.dw 2051
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 5
	.dw 256
; ch 1
	.db $03, 6
	.dw 256
; ch 2 rest
	.db $ff
; ch 3
	.db $ea, 2
	.dw 322
; ch 0 rest
	.db $ff
; ch 1 rest
	.db $ff
; ch 2
	.db $02, 37
	.dw 2051
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 11
	.dw 512
; ch 1
	.db $03, 7
	.dw 287
; ch 2
	.db $02, 18
	.dw 1025
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 5
	.dw 256
; ch 1
	.db $03, 6
	.dw 256
; ch 2
	.db $02, 37
	.dw 2051
; ch 3 rest
	.db $ff
; ch 0 rest
	.db $ff
; ch 1 rest
	.db $ff
; ch 2 rest
	.db $ff
; ch 3
	.db $eb, 1
	.dw 342
; ch 0
	.db $00, 7
	.dw 342
; ch 1
	.db $03, 6
	.dw 256
; ch 2
	.db $02, 37
	.dw 2051
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 6
	.dw 287
; ch 1
	.db $03, 9
	.dw 342
; ch 2
	.db $02, 18
	.dw 1025
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 5
	.dw 256
; ch 1 rest
	.db $ff
; ch 2
	.db $02, 37
	.dw 2051
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 12
	.dw 575
; ch 1
	.db $03, 7
	.dw 287
; ch 2
	.db $02, 43
	.dw 2302
; ch 3
	.db $ea, 2
	.dw 322
; ch 0 rest
	.db $ff
; ch 1 rest
	.db $ff
; ch 2 rest
	.db $ff
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 6
	.dw 287
; ch 1
	.db $03, 8
	.dw 322
; ch 2
	.db $02, 21
	.dw 1150
; ch 3 rest
	.db $ff
; ch 0 rest
	.db $ff
; ch 1 rest
	.db $ff
; ch 2 rest
	.db $ff
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 12
	.dw 575
; ch 1
	.db $03, 7
	.dw 287
; ch 2
	.db $02, 43
	.dw 2302
; ch 3
	.db $eb, 1
	.dw 342
; ch 0 rest
	.db $ff
; ch 1 rest
	.db $ff
; ch 2 rest
	.db $ff
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 6
	.dw 287
; ch 1
	.db $03, 10
	.dw 384
; ch 2
	.db $02, 21
	.dw 1150
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 12
	.dw 575
; ch 1
	.db $03, 7
	.dw 287
; ch 2
	.db $02, 43
	.dw 2302
; ch 3 rest
	.db $ff
; ch 0 rest
	.db $ff
; ch 1 rest
	.db $ff
; ch 2 rest
	.db $ff
; ch 3
	.db $ea, 2
	.dw 322
; ch 0
	.db $00, 12
	.dw 575
; ch 1
	.db $03, 7
	.dw 287
; ch 2
	.db $02, 43
	.dw 2302
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 7
	.dw 322
; ch 1
	.db $03, 8
	.dw 322
; ch 2
	.db $02, 21
	.dw 1150
; ch 3
	.db $ee, 1
	.dw 342
; ch 0
	.db $00, 6
	.dw 287
; ch 1
	.db $03, 7
	.dw 287
; ch 2
	.db $02, 43
	.dw 2302
; ch 3
	.db $ee, 1
	.dw 342
; ch 0 rest
	.db $ff
; ch 1 rest
	.db $ff
; ch 2 rest
	.db $ff
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 7
	.dw 322
; ch 1
	.db $03, 8
	.dw 322
; ch 2
	.db $02, 43
	.dw 2302
; ch 3
	.db $ee, 1
	.dw 342
; ch 0 rest
	.db $ff
; ch 1
	.db $03, 9
	.dw 342
; ch 2
	.db $02, 21
	.dw 1150
; ch 3
	.db $ee, 1
	.dw 342
; ch 0
	.db $00, 9
	.dw 432
; ch 1
	.db $03, 11
	.dw 432
; ch 2
	.db $02, 47
	.dw 2584
; ch 3
	.db $ee, 1
	.dw 342
; ch 0
	.db $00, 8
	.dw 384
; ch 1
	.db $03, 10
	.dw 384
; ch 2
	.db $02, 57
	.dw 3078
; ch 3
	.db $ea, 2
	.dw 322
; ch 0 rest
	.db $ff
; ch 1 rest
	.db $ff
; ch 2 rest
	.db $ff
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 16
	.dw 769
; ch 1
	.db $03, 11
	.dw 432
; ch 2
	.db $02, 28
	.dw 1538
; ch 3 rest
	.db $ff
; ch 0 rest
	.db $ff
; ch 1 rest
	.db $ff
; ch 2 rest
	.db $ff
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 8
	.dw 384
; ch 1
	.db $03, 10
	.dw 384
; ch 2
	.db $02, 57
	.dw 3078
; ch 3
	.db $eb, 1
	.dw 342
; ch 0 rest
	.db $ff
; ch 1 rest
	.db $ff
; ch 2 rest
	.db $ff
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 16
	.dw 769
; ch 1
	.db $03, 13
	.dw 512
; ch 2
	.db $02, 28
	.dw 1538
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 8
	.dw 384
; ch 1
	.db $03, 10
	.dw 384
; ch 2
	.db $02, 57
	.dw 3078
; ch 3 rest
	.db $ff
; ch 0 rest
	.db $ff
; ch 1 rest
	.db $ff
; ch 2 rest
	.db $ff
; ch 3
	.db $ea, 2
	.dw 322
; ch 0
	.db $00, 8
	.dw 384
; ch 1
	.db $03, 10
	.dw 384
; ch 2
	.db $02, 57
	.dw 3078
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 16
	.dw 769
; ch 1
	.db $03, 11
	.dw 432
; ch 2
	.db $02, 28
	.dw 1538
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 8
	.dw 384
; ch 1
	.db $03, 10
	.dw 384
; ch 2
	.db $02, 57
	.dw 3078
; ch 3 rest
	.db $ff
; ch 0 rest
	.db $ff
; ch 1 rest
	.db $ff
; ch 2 rest
	.db $ff
; ch 3
	.db $eb, 1
	.dw 342
; ch 0
	.db $00, 16
	.dw 769
; ch 1
	.db $03, 11
	.dw 432
; ch 2
	.db $02, 57
	.dw 3078
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 8
	.dw 384
; ch 1
	.db $03, 10
	.dw 384
; ch 2
	.db $02, 28
	.dw 1538
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 16
	.dw 769
; ch 1
	.db $03, 13
	.dw 512
; ch 2
	.db $02, 57
	.dw 3078
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 7
	.dw 342
; ch 1
	.db $03, 9
	.dw 342
; ch 2
	.db $02, 52
	.dw 2739
; ch 3
	.db $ea, 2
	.dw 322
; ch 0 rest
	.db $ff
; ch 1 rest
	.db $ff
; ch 2 rest
	.db $ff
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 14
	.dw 684
; ch 1
	.db $03, 10
	.dw 384
; ch 2
	.db $02, 25
	.dw 1369
; ch 3 rest
	.db $ff
; ch 0 rest
	.db $ff
; ch 1 rest
	.db $ff
; ch 2 rest
	.db $ff
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 7
	.dw 342
; ch 1
	.db $03, 9
	.dw 342
; ch 2
	.db $02, 52
	.dw 2739
; ch 3
	.db $eb, 1
	.dw 342
; ch 0 rest
	.db $ff
; ch 1 rest
	.db $ff
; ch 2 rest
	.db $ff
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 14
	.dw 684
; ch 1
	.db $03, 10
	.dw 384
; ch 2
	.db $02, 25
	.dw 1369
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 7
	.dw 342
; ch 1
	.db $03, 7
	.dw 287
; ch 2
	.db $02, 52
	.dw 2739
; ch 3 rest
	.db $ff
; ch 0 rest
	.db $ff
; ch 1 rest
	.db $ff
; ch 2 rest
	.db $ff
; ch 3
	.db $ea, 2
	.dw 322
; ch 0
	.db $00, 14
	.dw 684
; ch 1
	.db $03, 9
	.dw 342
; ch 2
	.db $02, 52
	.dw 2739
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 7
	.dw 342
; ch 1 rest
	.db $ff
; ch 2
	.db $02, 25
	.dw 1369
; ch 3
	.db $ee, 1
	.dw 342
; ch 0
	.db $00, 14
	.dw 684
; ch 1
	.db $03, 7
	.dw 287
; ch 2
	.db $02, 52
	.dw 2739
; ch 3 rest
	.db $ff
; ch 0 rest
	.db $ff
; ch 1 rest
	.db $ff
; ch 2 rest
	.db $ff
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 14
	.dw 684
; ch 1
	.db $03, 9
	.dw 342
; ch 2
	.db $02, 25
	.dw 1369
; ch 3
	.db $ee, 1
	.dw 342
; ch 0
	.db $00, 7
	.dw 342
; ch 1
	.db $03, 7
	.dw 287
; ch 2
	.db $02, 52
	.dw 2739
; ch 3
	.db $ee, 1
	.dw 342
; ch 0
	.db $00, 14
	.dw 684
; ch 1
	.db $03, 8
	.dw 322
; ch 2
	.db $02, 25
	.dw 1369
; ch 3
	.db $ee, 1
	.dw 342
; ch 0 rest
	.db $ff
; ch 1 rest
	.db $ff
; ch 2 rest
	.db $ff
; ch 3 rest
	.db $ff

; Pattern 5
_bgm_pat5:
; ch 0
	.db $00, 5
	.dw 256
; ch 1
	.db $03, 6
	.dw 256
; ch 2
	.db $02, 37
	.dw 2051
; ch 3
	.db $ea, 2
	.dw 322
; ch 0 rest
	.db $ff
; ch 1 rest
	.db $ff
; ch 2 rest
	.db $ff
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 11
	.dw 512
; ch 1
	.db $03, 7
	.dw 287
; ch 2
	.db $02, 18
	.dw 1025
; ch 3 rest
	.db $ff
; ch 0 rest
	.db $ff
; ch 1 rest
	.db $ff
; ch 2 rest
	.db $ff
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 5
	.dw 256
; ch 1
	.db $03, 6
	.dw 256
; ch 2
	.db $02, 37
	.dw 2051
; ch 3
	.db $eb, 1
	.dw 342
; ch 0 rest
	.db $ff
; ch 1 rest
	.db $ff
; ch 2 rest
	.db $ff
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 7
	.dw 342
; ch 1
	.db $03, 9
	.dw 342
; ch 2
	.db $02, 18
	.dw 1025
; ch 3 rest
	.db $ff
; ch 0 rest
	.db $ff
; ch 1 rest
	.db $ff
; ch 2
	.db $02, 37
	.dw 2051
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 5
	.dw 256
; ch 1
	.db $03, 6
	.dw 256
; ch 2 rest
	.db $ff
; ch 3
	.db $ea, 2
	.dw 322
; ch 0 rest
	.db $ff
; ch 1 rest
	.db $ff
; ch 2
	.db $02, 37
	.dw 2051
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 11
	.dw 512
; ch 1
	.db $03, 7
	.dw 287
; ch 2
	.db $02, 18
	.dw 1025
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 5
	.dw 256
; ch 1
	.db $03, 6
	.dw 256
; ch 2
	.db $02, 37
	.dw 2051
; ch 3 rest
	.db $ff
; ch 0 rest
	.db $ff
; ch 1 rest
	.db $ff
; ch 2 rest
	.db $ff
; ch 3
	.db $eb, 1
	.dw 342
; ch 0
	.db $00, 7
	.dw 342
; ch 1
	.db $03, 6
	.dw 256
; ch 2
	.db $02, 37
	.dw 2051
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 6
	.dw 287
; ch 1
	.db $03, 9
	.dw 342
; ch 2
	.db $02, 18
	.dw 1025
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 5
	.dw 256
; ch 1 rest
	.db $ff
; ch 2
	.db $02, 37
	.dw 2051
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 12
	.dw 575
; ch 1
	.db $03, 7
	.dw 287
; ch 2
	.db $02, 43
	.dw 2302
; ch 3
	.db $ea, 2
	.dw 322
; ch 0 rest
	.db $ff
; ch 1 rest
	.db $ff
; ch 2 rest
	.db $ff
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 6
	.dw 287
; ch 1
	.db $03, 8
	.dw 322
; ch 2
	.db $02, 21
	.dw 1150
; ch 3 rest
	.db $ff
; ch 0 rest
	.db $ff
; ch 1 rest
	.db $ff
; ch 2 rest
	.db $ff
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 12
	.dw 575
; ch 1
	.db $03, 7
	.dw 287
; ch 2
	.db $02, 43
	.dw 2302
; ch 3
	.db $eb, 1
	.dw 342
; ch 0 rest
	.db $ff
; ch 1 rest
	.db $ff
; ch 2 rest
	.db $ff
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 6
	.dw 287
; ch 1
	.db $03, 10
	.dw 384
; ch 2
	.db $02, 21
	.dw 1150
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 12
	.dw 575
; ch 1
	.db $03, 7
	.dw 287
; ch 2
	.db $02, 43
	.dw 2302
; ch 3 rest
	.db $ff
; ch 0 rest
	.db $ff
; ch 1 rest
	.db $ff
; ch 2 rest
	.db $ff
; ch 3
	.db $ea, 2
	.dw 322
; ch 0
	.db $00, 12
	.dw 575
; ch 1
	.db $03, 7
	.dw 287
; ch 2
	.db $02, 43
	.dw 2302
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 7
	.dw 322
; ch 1
	.db $03, 8
	.dw 322
; ch 2
	.db $02, 21
	.dw 1150
; ch 3
	.db $ee, 1
	.dw 342
; ch 0
	.db $00, 6
	.dw 287
; ch 1
	.db $03, 7
	.dw 287
; ch 2
	.db $02, 43
	.dw 2302
; ch 3
	.db $ee, 1
	.dw 342
; ch 0 rest
	.db $ff
; ch 1 rest
	.db $ff
; ch 2 rest
	.db $ff
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 7
	.dw 322
; ch 1
	.db $03, 8
	.dw 322
; ch 2
	.db $02, 43
	.dw 2302
; ch 3
	.db $ee, 1
	.dw 342
; ch 0 rest
	.db $ff
; ch 1
	.db $03, 9
	.dw 342
; ch 2
	.db $02, 21
	.dw 1150
; ch 3
	.db $ee, 1
	.dw 342
; ch 0
	.db $00, 9
	.dw 432
; ch 1
	.db $03, 11
	.dw 432
; ch 2
	.db $02, 47
	.dw 2584
; ch 3
	.db $ee, 1
	.dw 342
; ch 0
	.db $00, 8
	.dw 384
; ch 1
	.db $03, 5
	.dw 191
; ch 2
	.db $02, 57
	.dw 3078
; ch 3
	.db $ea, 2
	.dw 322
; ch 0 rest
	.db $ff
; ch 1 rest
	.db $ff
; ch 2 rest
	.db $ff
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 16
	.dw 769
; ch 1
	.db $03, 5
	.dw 216
; ch 2
	.db $02, 28
	.dw 1538
; ch 3 rest
	.db $ff
; ch 0 rest
	.db $ff
; ch 1 rest
	.db $ff
; ch 2 rest
	.db $ff
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 8
	.dw 384
; ch 1
	.db $03, 5
	.dw 191
; ch 2
	.db $02, 57
	.dw 3078
; ch 3
	.db $eb, 1
	.dw 342
; ch 0 rest
	.db $ff
; ch 1 rest
	.db $ff
; ch 2 rest
	.db $ff
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 16
	.dw 769
; ch 1
	.db $03, 6
	.dw 256
; ch 2
	.db $02, 28
	.dw 1538
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 8
	.dw 384
; ch 1
	.db $03, 5
	.dw 191
; ch 2
	.db $02, 57
	.dw 3078
; ch 3 rest
	.db $ff
; ch 0 rest
	.db $ff
; ch 1 rest
	.db $ff
; ch 2 rest
	.db $ff
; ch 3
	.db $ea, 2
	.dw 322
; ch 0
	.db $00, 8
	.dw 384
; ch 1
	.db $03, 5
	.dw 191
; ch 2
	.db $02, 57
	.dw 3078
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 16
	.dw 769
; ch 1
	.db $03, 5
	.dw 216
; ch 2
	.db $02, 28
	.dw 1538
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 8
	.dw 384
; ch 1
	.db $03, 5
	.dw 191
; ch 2
	.db $02, 57
	.dw 3078
; ch 3 rest
	.db $ff
; ch 0 rest
	.db $ff
; ch 1 rest
	.db $ff
; ch 2 rest
	.db $ff
; ch 3
	.db $eb, 1
	.dw 342
; ch 0
	.db $00, 16
	.dw 769
; ch 1
	.db $03, 5
	.dw 216
; ch 2
	.db $02, 57
	.dw 3078
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 8
	.dw 384
; ch 1
	.db $03, 5
	.dw 191
; ch 2
	.db $02, 28
	.dw 1538
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 16
	.dw 769
; ch 1
	.db $03, 6
	.dw 256
; ch 2
	.db $02, 57
	.dw 3078
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 7
	.dw 342
; ch 1
	.db $03, 9
	.dw 342
; ch 2
	.db $02, 52
	.dw 2739
; ch 3
	.db $ea, 2
	.dw 322
; ch 0 rest
	.db $ff
; ch 1 rest
	.db $ff
; ch 2 rest
	.db $ff
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 14
	.dw 684
; ch 1
	.db $03, 10
	.dw 384
; ch 2
	.db $02, 25
	.dw 1369
; ch 3 rest
	.db $ff
; ch 0 rest
	.db $ff
; ch 1 rest
	.db $ff
; ch 2 rest
	.db $ff
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 7
	.dw 342
; ch 1
	.db $03, 9
	.dw 342
; ch 2
	.db $02, 52
	.dw 2739
; ch 3
	.db $eb, 1
	.dw 342
; ch 0 rest
	.db $ff
; ch 1 rest
	.db $ff
; ch 2 rest
	.db $ff
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 14
	.dw 684
; ch 1
	.db $03, 10
	.dw 384
; ch 2
	.db $02, 25
	.dw 1369
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 7
	.dw 342
; ch 1
	.db $03, 9
	.dw 342
; ch 2
	.db $02, 52
	.dw 2739
; ch 3 rest
	.db $ff
; ch 0 rest
	.db $ff
; ch 1
	.db $03, 10
	.dw 384
; ch 2 rest
	.db $ff
; ch 3
	.db $ea, 2
	.dw 322
; ch 0
	.db $00, 14
	.dw 684
; ch 1 rest
	.db $ff
; ch 2
	.db $02, 52
	.dw 2739
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 5
	.dw 256
; ch 1
	.db $03, 13
	.dw 512
; ch 2
	.db $02, 18
	.dw 1025
; ch 3
	.db $ee, 1
	.dw 342
; ch 0
	.db $00, 6
	.dw 287
; ch 1 rest
	.db $ff
; ch 2
	.db $02, 21
	.dw 1150
; ch 3 rest
	.db $ff
; ch 0 rest
	.db $ff
; ch 1
	.db $03, 13
	.dw 512
; ch 2 rest
	.db $ff
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 6
	.dw 287
; ch 1
	.db $03, 15
	.dw 575
; ch 2
	.db $02, 21
	.dw 1150
; ch 3
	.db $ee, 1
	.dw 342
; ch 0
	.db $00, 6
	.dw 287
; ch 1 rest
	.db $ff
; ch 2
	.db $02, 21
	.dw 1150
; ch 3
	.db $ee, 1
	.dw 342
; ch 0
	.db $00, 6
	.dw 304
; ch 1
	.db $03, 16
	.dw 609
; ch 2
	.db $02, 22
	.dw 1219
; ch 3
	.db $ee, 1
	.dw 342
; ch 0 rest
	.db $ff
; ch 1 rest
	.db $ff
; ch 2 rest
	.db $ff
; ch 3 rest
	.db $ff

; Pattern 6
_bgm_pat6:
; ch 0
	.db $00, 14
	.dw 684
; ch 1
	.db $04, 3
	.dw 171
; ch 2
	.db $02, 52
	.dw 2739
; ch 3
	.db $ea, 2
	.dw 322
; ch 0 rest
	.db $ff
; ch 1 rest
	.db $ff
; ch 2 rest
	.db $ff
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 7
	.dw 342
; ch 1
	.db $04, 2
	.dw 144
; ch 2
	.db $02, 25
	.dw 1369
; ch 3 rest
	.db $ff
; ch 0 rest
	.db $ff
; ch 1 rest
	.db $ff
; ch 2 rest
	.db $ff
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 14
	.dw 684
; ch 1
	.db $04, 2
	.dw 114
; ch 2
	.db $02, 52
	.dw 2739
; ch 3
	.db $eb, 1
	.dw 342
; ch 0 rest
	.db $ff
; ch 1 rest
	.db $ff
; ch 2
	.db $02, 25
	.dw 1369
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 7
	.dw 342
; ch 1
	.db $04, 3
	.dw 171
; ch 2 rest
	.db $ff
; ch 3 rest
	.db $ff
; ch 0 rest
	.db $ff
; ch 1
	.db $04, 2
	.dw 144
; ch 2
	.db $02, 52
	.dw 2739
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 14
	.dw 684
; ch 1 rest
	.db $ff
; ch 2 rest
	.db $ff
; ch 3
	.db $ea, 2
	.dw 322
; ch 0 rest
	.db $ff
; ch 1
	.db $04, 2
	.dw 114
; ch 2
	.db $02, 52
	.dw 2739
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 7
	.dw 342
; ch 1 rest
	.db $ff
; ch 2
	.db $02, 25
	.dw 1369
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 14
	.dw 684
; ch 1
	.db $04, 3
	.dw 171
; ch 2
	.db $02, 52
	.dw 2739
; ch 3 rest
	.db $ff
; ch 0 rest
	.db $ff
; ch 1
	.db $04, 2
	.dw 144
; ch 2 rest
	.db $ff
; ch 3
	.db $eb, 1
	.dw 342
; ch 0
	.db $00, 14
	.dw 684
; ch 1
	.db $04, 2
	.dw 114
; ch 2
	.db $02, 52
	.dw 2739
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 7
	.dw 342
; ch 1
	.db $04, 2
	.dw 144
; ch 2
	.db $02, 25
	.dw 1369
; ch 3 rest
	.db $ff
; ch 0 rest
	.db $ff
; ch 1
	.db $04, 3
	.dw 171
; ch 2
	.db $02, 52
	.dw 2739
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 16
	.dw 769
; ch 1
	.db $04, 3
	.dw 191
; ch 2
	.db $02, 57
	.dw 3078
; ch 3
	.db $ea, 2
	.dw 322
; ch 0 rest
	.db $ff
; ch 1 rest
	.db $ff
; ch 2 rest
	.db $ff
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 8
	.dw 384
; ch 1
	.db $04, 2
	.dw 152
; ch 2
	.db $02, 28
	.dw 1538
; ch 3 rest
	.db $ff
; ch 0 rest
	.db $ff
; ch 1 rest
	.db $ff
; ch 2 rest
	.db $ff
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 16
	.dw 769
; ch 1
	.db $04, 2
	.dw 128
; ch 2
	.db $02, 57
	.dw 3078
; ch 3
	.db $eb, 1
	.dw 342
; ch 0 rest
	.db $ff
; ch 1 rest
	.db $ff
; ch 2 rest
	.db $ff
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 8
	.dw 384
; ch 1
	.db $04, 3
	.dw 191
; ch 2
	.db $02, 28
	.dw 1538
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 16
	.dw 769
; ch 1
	.db $04, 2
	.dw 152
; ch 2
	.db $02, 57
	.dw 3078
; ch 3 rest
	.db $ff
; ch 0 rest
	.db $ff
; ch 1 rest
	.db $ff
; ch 2 rest
	.db $ff
; ch 3
	.db $ea, 2
	.dw 322
; ch 0
	.db $00, 8
	.dw 384
; ch 1
	.db $04, 2
	.dw 128
; ch 2
	.db $02, 57
	.dw 3078
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 16
	.dw 769
; ch 1 rest
	.db $ff
; ch 2
	.db $02, 28
	.dw 1538
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 8
	.dw 384
; ch 1
	.db $04, 3
	.dw 191
; ch 2
	.db $02, 57
	.dw 3078
; ch 3 rest
	.db $ff
; ch 0 rest
	.db $ff
; ch 1
	.db $04, 2
	.dw 152
; ch 2 rest
	.db $ff
; ch 3
	.db $eb, 1
	.dw 342
; ch 0
	.db $00, 16
	.dw 769
; ch 1
	.db $04, 2
	.dw 128
; ch 2
	.db $02, 28
	.dw 1538
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 8
	.dw 384
; ch 1
	.db $04, 2
	.dw 152
; ch 2
	.db $02, 57
	.dw 3078
; ch 3
	.db $eb, 1
	.dw 342
; ch 0
	.db $00, 16
	.dw 769
; ch 1
	.db $04, 3
	.dw 191
; ch 2
	.db $02, 57
	.dw 3078
; ch 3
	.db $eb, 1
	.dw 342
; ch 0
	.db $00, 9
	.dw 432
; ch 1
	.db $04, 4
	.dw 216
; ch 2
	.db $02, 64
	.dw 3458
; ch 3
	.db $ea, 2
	.dw 322
; ch 0 rest
	.db $ff
; ch 1 rest
	.db $ff
; ch 2 rest
	.db $ff
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 4
	.dw 216
; ch 1
	.db $04, 3
	.dw 171
; ch 2
	.db $02, 32
	.dw 1728
; ch 3 rest
	.db $ff
; ch 0 rest
	.db $ff
; ch 1 rest
	.db $ff
; ch 2 rest
	.db $ff
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 9
	.dw 432
; ch 1
	.db $04, 2
	.dw 144
; ch 2
	.db $02, 64
	.dw 3458
; ch 3
	.db $eb, 1
	.dw 342
; ch 0 rest
	.db $ff
; ch 1 rest
	.db $ff
; ch 2
	.db $02, 32
	.dw 1728
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 4
	.dw 216
; ch 1
	.db $04, 4
	.dw 216
; ch 2 rest
	.db $ff
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 9
	.dw 432
; ch 1
	.db $04, 3
	.dw 171
; ch 2
	.db $02, 64
	.dw 3458
; ch 3 rest
	.db $ff
; ch 0 rest
	.db $ff
; ch 1 rest
	.db $ff
; ch 2 rest
	.db $ff
; ch 3
	.db $ea, 2
	.dw 322
; ch 0
	.db $00, 9
	.dw 432
; ch 1
	.db $04, 2
	.dw 144
; ch 2
	.db $02, 64
	.dw 3458
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 4
	.dw 216
; ch 1 rest
	.db $ff
; ch 2
	.db $02, 32
	.dw 1728
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 9
	.dw 432
; ch 1
	.db $04, 4
	.dw 216
; ch 2
	.db $02, 64
	.dw 3458
; ch 3 rest
	.db $ff
; ch 0 rest
	.db $ff
; ch 1
	.db $04, 3
	.dw 171
; ch 2 rest
	.db $ff
; ch 3
	.db $eb, 1
	.dw 342
; ch 0
	.db $00, 9
	.dw 432
; ch 1
	.db $04, 2
	.dw 144
; ch 2
	.db $02, 64
	.dw 3458
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 4
	.dw 216
; ch 1
	.db $04, 3
	.dw 171
; ch 2
	.db $02, 32
	.dw 1728
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 9
	.dw 432
; ch 1
	.db $04, 4
	.dw 216
; ch 2
	.db $02, 64
	.dw 3458
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 5
	.dw 256
; ch 1
	.db $04, 5
	.dw 256
; ch 2
	.db $02, 37
	.dw 2051
; ch 3
	.db $ea, 2
	.dw 322
; ch 0 rest
	.db $ff
; ch 1 rest
	.db $ff
; ch 2 rest
	.db $ff
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 11
	.dw 512
; ch 1
	.db $04, 4
	.dw 216
; ch 2
	.db $02, 18
	.dw 1025
; ch 3 rest
	.db $ff
; ch 0 rest
	.db $ff
; ch 1 rest
	.db $ff
; ch 2 rest
	.db $ff
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 5
	.dw 256
; ch 1
	.db $04, 3
	.dw 171
; ch 2
	.db $02, 37
	.dw 2051
; ch 3
	.db $eb, 1
	.dw 342
; ch 0 rest
	.db $ff
; ch 1 rest
	.db $ff
; ch 2
	.db $02, 18
	.dw 1025
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 11
	.dw 512
; ch 1
	.db $04, 5
	.dw 256
; ch 2 rest
	.db $ff
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 2
	.dw 114
; ch 1
	.db $04, 4
	.dw 228
; ch 2
	.db $02, 16
	.dw 912
; ch 3 rest
	.db $ff
; ch 0 rest
	.db $ff
; ch 1 rest
	.db $ff
; ch 2 rest
	.db $ff
; ch 3
	.db $ea, 2
	.dw 322
; ch 0
	.db $00, 4
	.dw 228
; ch 1
	.db $04, 4
	.dw 228
; ch 2
	.db $02, 34
	.dw 1829
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 2
	.dw 114
; ch 1
	.db $04, 3
	.dw 191
; ch 2
	.db $02, 16
	.dw 912
; ch 3
	.db $eb, 1
	.dw 342
; ch 0 rest
	.db $ff
; ch 1 rest
	.db $ff
; ch 2
	.db $02, 34
	.dw 1829
; ch 3
	.db $eb, 1
	.dw 342
; ch 0
	.db $00, 3
	.dw 152
; ch 1
	.db $04, 2
	.dw 152
; ch 2
	.db $02, 16
	.dw 912
; ch 3 rest
	.db $ff
; ch 0 rest
	.db $ff
; ch 1 rest
	.db $ff
; ch 2 rest
	.db $ff
; ch 3
	.db $eb, 1
	.dw 342
; ch 0
	.db $00, 3
	.dw 171
; ch 1
	.db $04, 2
	.dw 144
; ch 2
	.db $02, 8
	.dw 456
; ch 3
	.db $eb, 1
	.dw 342
; ch 0
	.db $00, 4
	.dw 191
; ch 1
	.db $04, 2
	.dw 152
; ch 2
	.db $02, 16
	.dw 912
; ch 3
	.db $eb, 1
	.dw 342
; ch 0 rest
	.db $ff
; ch 1 rest
	.db $ff
; ch 2 rest
	.db $ff
; ch 3 rest
	.db $ff

; Pattern 7
_bgm_pat7:
; ch 0
	.db $00, 14
	.dw 684
; ch 1
	.db $05, 6
	.dw 342
; ch 2
	.db $02, 52
	.dw 2739
; ch 3
	.db $ea, 2
	.dw 322
; ch 0 rest
	.db $ff
; ch 1 rest
	.db $ff
; ch 2 rest
	.db $ff
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 7
	.dw 342
; ch 1
	.db $05, 5
	.dw 287
; ch 2
	.db $02, 25
	.dw 1369
; ch 3 rest
	.db $ff
; ch 0 rest
	.db $ff
; ch 1 rest
	.db $ff
; ch 2 rest
	.db $ff
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 14
	.dw 684
; ch 1
	.db $05, 4
	.dw 228
; ch 2
	.db $02, 52
	.dw 2739
; ch 3
	.db $eb, 1
	.dw 342
; ch 0 rest
	.db $ff
; ch 1 rest
	.db $ff
; ch 2
	.db $02, 25
	.dw 1369
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 7
	.dw 342
; ch 1
	.db $05, 6
	.dw 342
; ch 2 rest
	.db $ff
; ch 3 rest
	.db $ff
; ch 0 rest
	.db $ff
; ch 1
	.db $05, 5
	.dw 287
; ch 2
	.db $02, 52
	.dw 2739
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 14
	.dw 684
; ch 1 rest
	.db $ff
; ch 2 rest
	.db $ff
; ch 3
	.db $ea, 2
	.dw 322
; ch 0 rest
	.db $ff
; ch 1
	.db $05, 4
	.dw 228
; ch 2
	.db $02, 52
	.dw 2739
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 7
	.dw 342
; ch 1 rest
	.db $ff
; ch 2
	.db $02, 25
	.dw 1369
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 14
	.dw 684
; ch 1
	.db $03, 4
	.dw 171
; ch 2
	.db $02, 52
	.dw 2739
; ch 3 rest
	.db $ff
; ch 0 rest
	.db $ff
; ch 1
	.db $03, 3
	.dw 144
; ch 2 rest
	.db $ff
; ch 3
	.db $eb, 1
	.dw 342
; ch 0
	.db $00, 14
	.dw 684
; ch 1
	.db $03, 2
	.dw 114
; ch 2
	.db $02, 52
	.dw 2739
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 7
	.dw 342
; ch 1
	.db $03, 3
	.dw 144
; ch 2
	.db $02, 25
	.dw 1369
; ch 3 rest
	.db $ff
; ch 0 rest
	.db $ff
; ch 1
	.db $03, 4
	.dw 171
; ch 2
	.db $02, 52
	.dw 2739
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 16
	.dw 769
; ch 1
	.db $05, 7
	.dw 384
; ch 2
	.db $02, 57
	.dw 3078
; ch 3
	.db $ea, 2
	.dw 322
; ch 0 rest
	.db $ff
; ch 1 rest
	.db $ff
; ch 2 rest
	.db $ff
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 8
	.dw 384
; ch 1
	.db $05, 5
	.dw 304
; ch 2
	.db $02, 28
	.dw 1538
; ch 3 rest
	.db $ff
; ch 0 rest
	.db $ff
; ch 1 rest
	.db $ff
; ch 2 rest
	.db $ff
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 16
	.dw 769
; ch 1
	.db $05, 4
	.dw 256
; ch 2
	.db $02, 57
	.dw 3078
; ch 3
	.db $eb, 1
	.dw 342
; ch 0 rest
	.db $ff
; ch 1 rest
	.db $ff
; ch 2 rest
	.db $ff
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 8
	.dw 384
; ch 1
	.db $05, 7
	.dw 384
; ch 2
	.db $02, 28
	.dw 1538
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 16
	.dw 769
; ch 1
	.db $05, 5
	.dw 304
; ch 2
	.db $02, 57
	.dw 3078
; ch 3 rest
	.db $ff
; ch 0 rest
	.db $ff
; ch 1 rest
	.db $ff
; ch 2 rest
	.db $ff
; ch 3
	.db $ea, 2
	.dw 322
; ch 0
	.db $00, 8
	.dw 384
; ch 1
	.db $05, 4
	.dw 256
; ch 2
	.db $02, 57
	.dw 3078
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 16
	.dw 769
; ch 1 rest
	.db $ff
; ch 2
	.db $02, 28
	.dw 1538
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 8
	.dw 384
; ch 1
	.db $03, 10
	.dw 384
; ch 2
	.db $02, 57
	.dw 3078
; ch 3 rest
	.db $ff
; ch 0 rest
	.db $ff
; ch 1
	.db $03, 8
	.dw 304
; ch 2 rest
	.db $ff
; ch 3
	.db $eb, 1
	.dw 342
; ch 0
	.db $00, 16
	.dw 769
; ch 1
	.db $03, 10
	.dw 384
; ch 2
	.db $02, 28
	.dw 1538
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 8
	.dw 384
; ch 1
	.db $03, 8
	.dw 304
; ch 2
	.db $02, 57
	.dw 3078
; ch 3
	.db $eb, 1
	.dw 342
; ch 0
	.db $00, 16
	.dw 769
; ch 1
	.db $03, 6
	.dw 256
; ch 2
	.db $02, 57
	.dw 3078
; ch 3
	.db $eb, 1
	.dw 342
; ch 0
	.db $00, 9
	.dw 432
; ch 1
	.db $05, 5
	.dw 287
; ch 2
	.db $02, 64
	.dw 3458
; ch 3
	.db $ea, 2
	.dw 322
; ch 0 rest
	.db $ff
; ch 1 rest
	.db $ff
; ch 2 rest
	.db $ff
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 4
	.dw 216
; ch 1
	.db $05, 8
	.dw 432
; ch 2
	.db $02, 32
	.dw 1728
; ch 3 rest
	.db $ff
; ch 0 rest
	.db $ff
; ch 1 rest
	.db $ff
; ch 2 rest
	.db $ff
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 9
	.dw 432
; ch 1
	.db $05, 8
	.dw 432
; ch 2
	.db $02, 64
	.dw 3458
; ch 3
	.db $eb, 1
	.dw 342
; ch 0 rest
	.db $ff
; ch 1 rest
	.db $ff
; ch 2
	.db $02, 32
	.dw 1728
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 4
	.dw 216
; ch 1
	.db $05, 8
	.dw 432
; ch 2 rest
	.db $ff
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 9
	.dw 432
; ch 1
	.db $05, 5
	.dw 287
; ch 2
	.db $02, 64
	.dw 3458
; ch 3 rest
	.db $ff
; ch 0 rest
	.db $ff
; ch 1 rest
	.db $ff
; ch 2 rest
	.db $ff
; ch 3
	.db $ea, 2
	.dw 322
; ch 0
	.db $00, 9
	.dw 432
; ch 1
	.db $05, 5
	.dw 287
; ch 2
	.db $02, 64
	.dw 3458
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 4
	.dw 216
; ch 1 rest
	.db $ff
; ch 2
	.db $02, 32
	.dw 1728
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 9
	.dw 432
; ch 1
	.db $05, 8
	.dw 432
; ch 2
	.db $02, 64
	.dw 3458
; ch 3 rest
	.db $ff
; ch 0 rest
	.db $ff
; ch 1 rest
	.db $ff
; ch 2 rest
	.db $ff
; ch 3
	.db $eb, 1
	.dw 342
; ch 0
	.db $00, 9
	.dw 432
; ch 1
	.db $03, 5
	.dw 216
; ch 2
	.db $02, 64
	.dw 3458
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 4
	.dw 216
; ch 1
	.db $03, 3
	.dw 144
; ch 2
	.db $02, 32
	.dw 1728
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 9
	.dw 432
; ch 1
	.db $03, 5
	.dw 216
; ch 2
	.db $02, 64
	.dw 3458
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 5
	.dw 256
; ch 1
	.db $05, 4
	.dw 256
; ch 2
	.db $02, 37
	.dw 2051
; ch 3
	.db $ea, 2
	.dw 322
; ch 0 rest
	.db $ff
; ch 1 rest
	.db $ff
; ch 2 rest
	.db $ff
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 11
	.dw 512
; ch 1
	.db $05, 7
	.dw 384
; ch 2
	.db $02, 18
	.dw 1025
; ch 3 rest
	.db $ff
; ch 0 rest
	.db $ff
; ch 1 rest
	.db $ff
; ch 2 rest
	.db $ff
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 5
	.dw 256
; ch 1
	.db $05, 7
	.dw 384
; ch 2
	.db $02, 37
	.dw 2051
; ch 3
	.db $eb, 1
	.dw 342
; ch 0 rest
	.db $ff
; ch 1 rest
	.db $ff
; ch 2
	.db $02, 18
	.dw 1025
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 11
	.dw 512
; ch 1
	.db $05, 7
	.dw 384
; ch 2 rest
	.db $ff
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 2
	.dw 114
; ch 1
	.db $05, 4
	.dw 228
; ch 2
	.db $02, 16
	.dw 912
; ch 3 rest
	.db $ff
; ch 0 rest
	.db $ff
; ch 1 rest
	.db $ff
; ch 2 rest
	.db $ff
; ch 3
	.db $ea, 2
	.dw 322
; ch 0
	.db $00, 4
	.dw 228
; ch 1
	.db $05, 6
	.dw 342
; ch 2
	.db $02, 34
	.dw 1829
; ch 3 rest
	.db $ff
; ch 0
	.db $00, 2
	.dw 114
; ch 1
	.db $05, 4
	.dw 228
; ch 2
	.db $02, 16
	.dw 912
; ch 3
	.db $eb, 1
	.dw 342
; ch 0 rest
	.db $ff
; ch 1 rest
	.db $ff
; ch 2
	.db $02, 34
	.dw 1829
; ch 3
	.db $eb, 1
	.dw 342
; ch 0
	.db $00, 3
	.dw 152
; ch 1
	.db $03, 4
	.dw 171
; ch 2
	.db $02, 16
	.dw 912
; ch 3 rest
	.db $ff
; ch 0 rest
	.db $ff
; ch 1
	.db $03, 4
	.dw 152
; ch 2 rest
	.db $ff
; ch 3
	.db $eb, 1
	.dw 342
; ch 0
	.db $00, 3
	.dw 171
; ch 1
	.db $03, 2
	.dw 114
; ch 2
	.db $02, 8
	.dw 456
; ch 3
	.db $eb, 1
	.dw 342
; ch 0
	.db $00, 4
	.dw 191
; ch 1
	.db $03, 4
	.dw 152
; ch 2
	.db $02, 16
	.dw 912
; ch 3
	.db $eb, 1
	.dw 342
; ch 0 rest
	.db $ff
; ch 1 rest
	.db $ff
; ch 2 rest
	.db $ff
; ch 3 rest
	.db $ff

_bgm_song:
	.dw _bgm_pat0
	.dw _bgm_pat1
	.dw _bgm_pat2
	.dw _bgm_pat3
	.dw _bgm_pat4
	.dw _bgm_pat5
	.dw _bgm_pat6
	.dw _bgm_pat7
	.dw _bgm_pat1
	.dw _bgm_pat3
	.dw _bgm_pat5
	.dw _bgm_pat6
	.dw 0
_bgm_wave0:	; custom
	.db 16, 17, 16, 22, 24, 25, 24, 26
	.db 24, 24, 22, 17, 16, 18, 18, 21
	.db 19, 8, 10, 0, 0, 3, 4, 8
	.db 9, 11, 12, 12, 13, 14, 15, 18
_bgm_vol0: ; avg 44 slen 7798
	.db 24, 24, 24, 23, 24, 26, 27, 28
	.db 27, 27, 27, 27, 26, 25, 24, 24
_bgm_wave1:	; std wave 1
	.db 16, 13, 10, 7, 5, 3, 2, 1
	.db 0, 1, 2, 3, 5, 7, 9, 13
	.db 15, 18, 21, 23, 25, 27, 29, 30
	.db 30, 30, 29, 27, 25, 23, 21, 19
_bgm_vol1: ; avg 38 slen 8798
	.db 19, 18, 19, 21, 23, 24, 23, 22
	.db 22, 22, 22, 23, 25, 27, 30, 31
_bgm_wave2:	; custom
	.db 15, 16, 18, 20, 21, 23, 25, 27
	.db 28, 30, 31, 31, 31, 31, 29, 27
	.db 25, 22, 19, 16, 13, 10, 8, 6
	.db 5, 5, 5, 6, 8, 10, 12, 14
_bgm_vol2: ; avg 58 slen 6598
	.db 21, 22, 25, 27, 28, 27, 26, 24
	.db 22, 20, 18, 17, 18, 17, 16, 16
_bgm_wave3:	; custom
	.db 21, 26, 28, 31, 31, 31, 30, 26
	.db 23, 20, 20, 19, 18, 15, 16, 14
	.db 14, 11, 10, 8, 9, 7, 6, 3
	.db 1, 3, 5, 7, 8, 7, 8, 15
_bgm_vol3: ; avg 18 slen 9398
	.db 31, 27, 22, 20, 17, 14, 12, 12
	.db 13, 13, 11, 10, 9, 9, 8, 7
_bgm_wave4:	; custom
	.db 17, 24, 31, 25, 20, 23, 27, 22
	.db 18, 22, 25, 23, 21, 22, 22, 21
	.db 19, 18, 17, 16, 15, 15, 14, 14
	.db 14, 14, 12, 8, 5, 10, 14, 12
_bgm_vol4: ; avg 42 slen 6998
	.db 24, 24, 25, 26, 29, 31, 30, 28
	.db 26, 25, 25, 25, 23, 21, 19, 18
_bgm_wave5:	; custom
	.db 21, 29, 30, 29, 28, 25, 22, 22
	.db 21, 18, 17, 18, 17, 16, 16, 15
	.db 13, 12, 11, 8, 6, 6, 5, 1
	.db 0, 2, 2, 2, 6, 7, 8, 14
_bgm_vol5: ; avg 38 slen 6898
	.db 30, 29, 28, 26, 24, 23, 22, 21
	.db 20, 19, 18, 18, 17, 16, 17, 17
_bgm_wave10:	; std wave 10
	.db 31, 31, 31, 31, 31, 31, 31, 31
	.db 31, 31, 31, 31, 31, 31, 31, 31
	.db 31, 31, 31, 31, 31, 31, 31, 31
	.db 1, 1, 1, 1, 1, 1, 1, 1
_bgm_vol10: ; avg 20 slen 2998
	.db 31, 30, 28, 26, 26, 25, 21, 17
	.db 15, 13, 12, 11, 10, 10, 10, 10
_bgm_wave11:	; std wave 11
	.db 31, 0, 4, 6, 8, 10, 12, 14
	.db 16, 18, 20, 22, 24, 26, 28, 30
	.db 31, 30, 28, 26, 24, 22, 20, 18
	.db 16, 14, 12, 10, 8, 6, 4, 2
_bgm_vol11: ; avg 43 slen 1984
	.db 28, 29, 31, 29, 27, 25, 22, 20
	.db 17, 15, 13, 11, 10, 8, 7, 6
_bgm_wave12:	; std wave 12
	.db 18, 16, 26, 31, 24, 13, 20, 28
	.db 24, 18, 24, 28, 16, 1, 5, 11
	.db 5, 1, 9, 16, 5, 0, 3, 13
	.db 9, 7, 16, 26, 13, 3, 16, 20
_bgm_vol12: ; avg 53 slen 1394
	.db 28, 30, 31, 29, 27, 25, 24, 22
	.db 23, 24, 22, 19, 19, 19, 18, 17
_bgm_wave14:	; std wave 14
	.db 31, 30, 27, 23, 19, 15, 11, 7
	.db 3, 15, 12, 9, 7, 4, 2, 1
	.db 0, 0, 1, 2, 4, 7, 9, 12
	.db 31, 27, 23, 19, 15, 11, 7, 3
_bgm_vol14: ; avg 104 slen 1398
	.db 30, 30, 30, 30, 30, 30, 30, 30
	.db 30, 30, 30, 30, 30, 30, 28, 27
_bgm_wave_table:
	.dw _bgm_wave0
	.dw _bgm_wave1
	.dw _bgm_wave2
	.dw _bgm_wave3
	.dw _bgm_wave4
	.dw _bgm_wave5
	.dw 0
	.dw 0
	.dw 0
	.dw 0
	.dw _bgm_wave10
	.dw _bgm_wave11
	.dw _bgm_wave12
	.dw 0
	.dw _bgm_wave14
	.dw 0
	.dw 0
	.dw 0
	.dw 0
	.dw 0
	.dw 0
	.dw 0
	.dw 0
	.dw 0
	.dw 0
	.dw 0
	.dw 0
	.dw 0
	.dw 0
	.dw 0
	.dw 0
	.dw 0
_bgm_chan_map:
	.db 0, 1, 5, 4
_bgm_vol_table:
	.dw _bgm_vol0
	.dw _bgm_vol1
	.dw _bgm_vol2
	.dw _bgm_vol3
	.dw _bgm_vol4
	.dw _bgm_vol5
	.dw 0
	.dw 0
	.dw 0
	.dw 0
	.dw _bgm_vol10
	.dw _bgm_vol11
	.dw _bgm_vol12
	.dw 0
	.dw _bgm_vol14
	.dw 0
	.dw 0
	.dw 0
	.dw 0
	.dw 0
	.dw 0
	.dw 0
	.dw 0
	.dw 0
	.dw 0
	.dw 0
	.dw 0
	.dw 0
	.dw 0
	.dw 0
	.dw 0
	.dw 0
