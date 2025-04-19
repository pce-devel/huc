all: chrblkmap.pce

include ../Make_ex.inc

SRC = chrblkmap.c

CFLAGS ?= -v -O2 -fno-recursive -gL

chrblkmap.pce: $(SRC)
	$(CC) $(CFLAGS) $(SRC) $(LIBS)
