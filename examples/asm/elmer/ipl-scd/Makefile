all: ipl-scd.bin

include ../Make_ex.inc

SRC = ipl-scd.asm ../font/font8x8-ascii-exos.dat

AFLAGS ?= -raw -m -l 2 -S

ipl-scd.bin: $(SRC)
	$(AS) $(AFLAGS) -ipl ipl-scd.asm
	cp ipl-scd.bin ../include/ipl-scd.bin

clean:
	rm -f ../include/ipl-scd.bin *.bin *.lst *.sym
