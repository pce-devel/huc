all: ted2romcd.pce

include ../Make_ex.inc

SRC = ted2romcd.asm syscard3-ted2.inc

AFLAGS ?= -raw -m -l 2 -S

ted2romcd.pce: $(SRC)
	$(AS) $(AFLAGS) ted2romcd.asm
	cp ted2romcd.pce ../include/ted2romcd.pce

clean:
	rm -f ../include/ted2romcd.pce ted2romcd.pce *.lst *.sym
