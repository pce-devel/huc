all: ac_test.iso

include ../Make_ex.inc

SRC = ac_test.c

CFLAGS ?= -v -acd -over -sgx -fno-recursive -msmall

ac_test.ovl: $(SRC)
	$(CC) $(CFLAGS) $(SRC) $(LIBS)

ac_test.iso: ac_test.ovl
	$(IL) $@ -sgx -ipl="ArcadeCard Test" $^ pce_tile.bin pce_bat.bin pce_pal.bin
