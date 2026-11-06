all: cd-sfx.iso

include ../Make_ex.inc

SRC = cd-sfx.c

CFLAGS ?= -v -O2 -fno-recursive -cd -over -gC

cd-sfx.ovl: $(SRC)
	$(CC) $(CFLAGS) $(SRC) $(LIBS)

cd-sfx.iso: cd-sfx.ovl
	$(IL) $@ -ipl="Cd Sfx Test" $^ pickup.vox
