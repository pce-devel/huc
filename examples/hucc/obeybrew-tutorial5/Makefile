all: tutorial5.pce

include ../Make_ex.inc

SRC = tutorial5.c

CFLAGS ?= -v -O2 -fno-recursive -gC

tutorial5.pce: $(SRC)
	$(CC) $(CFLAGS) $(SRC) $(LIBS)
