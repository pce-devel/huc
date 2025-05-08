all: sgx_vdc_scroll.sgx sgx_vdc_scroll.iso

include ../Make_ex.inc

SRC = sgx_vdc_scroll.c

CFLAGS ?= -O2 --sgx -fno-recursive -gA
CDFLAGS = --scd
LIBS =

sgx_vdc_scroll.iso: $(SRC)
	$(CC) $(CFLAGS) $(CDFLAGS) $< $(LIBS)

sgx_vdc_scroll.sgx: $(SRC) sgx_vdc_scroll.iso
	$(CC) $(CFLAGS) $< $(LIBS)
