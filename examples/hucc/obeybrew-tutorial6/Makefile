all: tutorial6.pce

include ../Make_ex.inc

SRC = tutorial6.c

CFLAGS ?= -v -O2 -fno-recursive -gC

tutorial6.pce: $(SRC)
	$(CC) $(CFLAGS) $(SRC) $(LIBS)
