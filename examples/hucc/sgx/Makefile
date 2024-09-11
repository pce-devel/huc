all: sgx_test.iso sgx_test.sgx

include ../Make_ex.inc

SRC = sgx_test.c

CFLAGS ?= -sgx -fno-recursive
CDFLAGS = -scd -over

sgx_test.sgx: $(SRC)
	$(CC) $(CFLAGS) $(SRC) $(LIBS)
	$(MV) sgx_test.pce $@

sgx_test.ovl: $(SRC) sgx_test.sgx
	$(CC) $(CFLAGS) $(CDFLAGS) $(SRC) $(LIBS)

sgx_test.iso: sgx_test.ovl
	$(IL) $@ -ipl="SuperGRAFX Test" $^
