all: shmup.pce shmup.iso

include ../Make_ex.inc

CFLAGS = -msmall -fno-recursive
LIBS =

shmup.pce: shmup.c
	$(CC) -fsigned-char $(CFLAGS) $(LIBS) $<

shmup.iso: shmup.c shmup.pce
	$(CC) -scd -fsigned-char $(CFLAGS) $(LIBS) $<
