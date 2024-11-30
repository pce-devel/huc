all: shmup.sgx shmup.iso

include ../Make_ex.inc

CFLAGS = --sgx -fno-recursive -fsigned-char -gC
CDFLAGS = --scd
LIBS =

shmup.iso: shmup.c
	$(CC) $(CFLAGS) $(CDFLAGS) $< $(LIBS)

shmup.sgx: shmup.c shmup.iso
	$(CC) $(CFLAGS) $(LIBS) $<
