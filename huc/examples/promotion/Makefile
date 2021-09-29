all: promotion.pce

include ../Make_ex.inc

SRC = promotion.c

CFLAGS ?= -fno-recursive -msmall

promotion.pce: $(SRC)
	$(CC) $(CFLAGS) $(SRC) $(LIBS)

clean:
	rm -f *.pce *.lst *.sym *.s
