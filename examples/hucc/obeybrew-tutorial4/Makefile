all: tutorial4.pce

include ../Make_ex.inc

SRC = tutorial4.c

CFLAGS ?= -v -O2 -fno-recursive -gC

tutorial4.pce: $(SRC)
	$(CC) $(CFLAGS) $(SRC) $(LIBS)
