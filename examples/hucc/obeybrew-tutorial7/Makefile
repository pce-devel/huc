all: tutorial7.pce

include ../Make_ex.inc

SRC = tutorial7.c

CFLAGS ?= -v -O2 -fno-recursive -gC

tutorial7.pce: $(SRC)
	$(CC) $(CFLAGS) $(SRC) $(LIBS)
