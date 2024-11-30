all: sgx_test.sgx sgx_test.iso

include ../Make_ex.inc

SRC = sgx_test.c

CFLAGS ?= --sgx -fno-recursive -gA
CDFLAGS = --scd
LIBS =

sgx_test.iso: $(SRC)
	$(CC) $(CFLAGS) $(CDFLAGS) $< $(LIBS)

sgx_test.sgx: $(SRC) sgx_test.iso
	$(CC) $(CFLAGS) $< $(LIBS)
