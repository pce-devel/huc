all: sfx.iso

include ../Make_ex.inc

SRC = sfx.c

CFLAGS ?= -v -O2 -fno-recursive -cd -over -gC

sfx.ovl: $(SRC)
	$(CC) $(CFLAGS) $(SRC) $(LIBS)

sfx.iso: sfx.ovl
	$(IL) $@ -ipl="Sfx Test" $^ pickup.vox
