# pce2png

This utility converts memory dumps from an emulator into a PCX/BMP/PNG bitmap.

Both the VDC and the VCE memory need to be dumped.

When not using a BAT, raw memory can be rendered into any one of the VCE's 32
palettes (0..15 background, 16..31 sprite).

Usage      : pce2png [<inopt>] [<outopt>]

<inopt>    : Option........Description....................................

             -vdc <file>   Binary file of VDC data
             -vde <file>   Binary file of VCE data
             -pal <n>      Write bitmap in palette N from VCE data
             -bat <w> <h>  Set BAT size in tiles and dump screen image
             -spr <w> <h>  Set SPR size and dump VDC data as sprites
             -vdcbegin <n> Starting offset into VDC data (in bytes)
             -vdcdelta <n> Amount to add to get to next CHR or SPR

<outopt>   : OutOpt........Description....................................

             -w <n>        Pixel width of output bitmap (a power of 2)
             -h <n>        Pixel height of output bitmap (a power of 2)
             -out <file>   Filename to save output bitmap
