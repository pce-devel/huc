all: shmup.pce

include ../Make_ex.inc

CFLAGS = -msmall -fno-recursive
LIBS =

shmup.pce: shmup.c
	$(CC) -scd -fsigned-char $(CFLAGS) $(LIBS) $<
	$(CC) -fsigned-char $(CFLAGS) $(LIBS) $<
