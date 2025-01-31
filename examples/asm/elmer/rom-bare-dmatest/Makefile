all: dmatest.pce

include ../Make_ex.inc

AFLAGS ?= --newproc --strip -m -l 2 -S -gA

SRC_INC	= pceas.inc pcengine.inc joypad.asm tty.asm vce.asm vdc.asm
SRC_OVL	= dmatest.asm

dmatest.pce: $(SRC_OVL) $(SRC_INC)
	$(AS) $(AFLAGS) --raw dmatest.asm
