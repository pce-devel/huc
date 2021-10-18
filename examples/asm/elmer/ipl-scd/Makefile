all: ipl-scd.bin

include ../Make_ex.inc

SRC = ipl-scd.s font8x8-exos-elmer-supercd.bin

AFLAGS ?= -raw -ipl -m -l 2 -S

ipl-scd.bin: ipl-scd.s
	$(AS) $(AFLAGS) -o ipl-scd.bin ipl-scd.s
	cp ipl-scd.bin ../include/ipl-scd.bin

clean:
	rm -f ../include/ipl-scd.bin ipl-scd.bin *.lst *.sym
