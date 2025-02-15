all: shmup.sgx shmup.iso

include ../Make_ex.inc

CFLAGS = -O2 --sgx -fno-recursive -gC -v
CDFLAGS = --scd
LIBS =

shmup.iso: shmup.c
	$(CC) $(CFLAGS) $(CDFLAGS) $< $(LIBS)

shmup.sgx: shmup.c shmup.iso
	$(CC) $(CFLAGS) $(LIBS) $<
