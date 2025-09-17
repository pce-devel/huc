all: overlay.iso

include ../Make_ex.inc

SRC = prog1.c prog2.c prog3.c
OBJ = prog1.ovl prog2.ovl prog3.ovl
DATA = data.bin data1.bin

CFLAGS ?= -v -O2 -fno-recursive -gL -over

cderr.ovl: cderr.c
	$(CC) $(CFLAGS) -cd $<

%.ovl: %.c
	$(CC) $(CFLAGS) -scd $<

overlay.iso: $(OBJ) cderr.ovl
	$(IL) $@ $(OBJ) $(DATA) -cderr cderr.ovl
