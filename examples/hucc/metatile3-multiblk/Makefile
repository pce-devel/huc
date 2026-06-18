all: multiblk.pce multiblk.iso

include ../Make_ex.inc

SRC = multiblk.c graphics.asm
ISO = multiblk.ovl datafile.bin

CFLAGS ?= -v -O2 -fno-recursive -gL
CDFLAGS ?= --scd --overlay

multiblk.ovl: $(SRC)
	$(CC) $(CFLAGS) $(CDFLAGS) $< $(LIBS)

multiblk.iso: $(ISO)
	$(IL) $@ --ipl="MultiBLK" $+

multiblk.pce: $(SRC)
	$(CC) $(CFLAGS) $< $(LIBS)
