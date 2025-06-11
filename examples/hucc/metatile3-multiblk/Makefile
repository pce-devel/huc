all: multiblk.pce

include ../Make_ex.inc

SRC = multiblk.c graphics.asm

CFLAGS ?= -v -O2 -fno-recursive -gL

multiblk.pce: $(SRC)
	$(CC) $(CFLAGS) $< $(LIBS)
