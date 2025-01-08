all: hello.pce

include ../Make_ex.inc

AFLAGS ?= --newproc --strip -m -l 2 -S -gA

SRC_INC	= pceas.inc pcengine.inc core.inc core-startup.asm core-kernel.asm joypad.asm
SRC_OVL	= hello.asm core-config.inc

hello.pce: $(SRC_OVL) $(SRC_INC)
	$(AS) $(AFLAGS) --raw hello.asm
