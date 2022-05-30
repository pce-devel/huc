all: sgx_test.iso sgx_test.sgx

include ../Make_ex.inc

SRC = sgx_test.c

CFLAGS ?= -sgx -fno-recursive -msmall
CDFLAGS = -scd

sgx_test.iso: $(SRC)
	$(CC) $(CFLAGS) $(CDFLAGS) $(SRC) $(LIBS)

sgx_test.sgx: $(SRC)
	$(CC) $(CFLAGS) $(SRC) $(LIBS)
	$(MV) sgx_test.pce $@
