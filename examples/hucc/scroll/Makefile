all: scroll.pce

include ../Make_ex.inc

SRC = scroll.c

CFLAGS ?= -v -O2 -fno-recursive -gC

scroll.pce: $(SRC)
	$(CC) $(CFLAGS) $(SRC) $(LIBS)
