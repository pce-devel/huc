all: overlay.iso

include ../Make_ex.inc

SRC = prog1.c prog2.c prog3.c
OBJ = prog1.ovl prog2.ovl prog3.ovl
DATA = data.bin data1.bin


#CFLAGS ?= -v -fno-recursive -msmall -over
CFLAGS ?= -v -msmall -over

cderr.ovl: cderr.c
	$(CC) $(CFLAGS) -cd $<

%.ovl: %.c
	$(CC) $(CFLAGS) -scd $<

overlay.iso: $(OBJ) cderr.ovl
	../../../bin/isolink $@ $(OBJ) $(DATA) -cderr cderr.ovl

clean:
	rm -f *.pce *.lst *.sym *.s *.iso *.ovl
