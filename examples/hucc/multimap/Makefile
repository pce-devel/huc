all: multimap.pce

include ../Make_ex.inc

SRC = multimap.c

CFLAGS ?= -v -O2 -fno-recursive -gL

multimap.pce: $(SRC)
	$(CC) $(CFLAGS) $(SRC) $(LIBS)
