all: tutorial2.pce

include ../Make_ex.inc

SRC = tutorial2.c

CFLAGS ?= -v -O2 -fno-recursive -gC

tutorial2.pce: $(SRC)
	$(CC) $(CFLAGS) $(SRC) $(LIBS)
