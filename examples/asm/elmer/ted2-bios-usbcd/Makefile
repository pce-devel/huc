all: ted2usbcd.pce

include ../Make_ex.inc

SRC = ted2usbcd.asm ../ted2-bios-romcd/syscard3-ted2.inc

AFLAGS ?= -raw -m -l 2 -S

ted2usbcd.pce: $(SRC)
	$(AS) $(AFLAGS) ted2usbcd.asm
	cp ted2usbcd.pce ../include/ted2usbcd.pce

clean:
	rm -f ../include/ted2usbcd.pce ted2usbcd.pce *.lst *.sym
