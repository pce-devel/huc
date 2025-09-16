all: tutorial1.pce

include ../Make_ex.inc

SRC = tutorial1.c

CFLAGS ?= -v -O2 -fno-recursive -gC

tutorial1.pce: $(SRC)
	$(CC) $(CFLAGS) $(SRC) $(LIBS)
