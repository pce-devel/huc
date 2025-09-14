all: tutorial3.pce

include ../Make_ex.inc

SRC = tutorial3.c

CFLAGS ?= -v -O2 -fno-recursive -gC

tutorial3.pce: $(SRC)
	$(CC) $(CFLAGS) $(SRC) $(LIBS)
