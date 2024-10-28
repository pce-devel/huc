all: shmup.sgx shmup.iso

include ../Make_ex.inc

CFLAGS = -msmall -fno-recursive
LIBS =

shmup.sgx: shmup.c
	$(CC) -sgx -fsigned-char $(CFLAGS) $(LIBS) $<
	$(MV) shmup.pce $@

shmup.ovl: shmup.c shmup.sgx
	$(CC) -sgx -scd -over -fsigned-char $(CFLAGS) $(LIBS) $<

shmup.iso: shmup.ovl
	$(IL) $@ -ipl="SuperGRAFX SHMUP" $^
