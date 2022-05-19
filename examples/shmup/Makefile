include ../Make_ex.inc

CFLAGS = -msmall -fno-recursive
LIBS =

all: shmup.pce

shmup.pce: shmup.c
	$(CC) -scd -fsigned-char $(CFLAGS) $(LIBS) $<
	$(CC) -fsigned-char $(CFLAGS) $(LIBS) $<

clean:
	rm -f *.sym *.lst *.s *.pce *.iso
