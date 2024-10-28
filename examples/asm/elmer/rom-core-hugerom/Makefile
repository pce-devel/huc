all: hugerom.pce hugerom.s

include ../Make_ex.inc

AFLAGS ?= -newproc -strip -m -l 2 -S

SRC_INC	= pceas.inc pcengine.inc core.inc core-startup.asm core-kernel.asm joypad.asm common.asm vdc.asm unpack-zx0.asm 
SRC_OVL	= hugerom.asm core-config.inc

hugerom.pce hugerom.sym: $(SRC_OVL) $(SRC_INC)
	$(AS) $(AFLAGS) -sf2 -raw hugerom.asm

hugerom.s: hugerom.s2i hugerom.sym
	$(S2I) hugerom.s2i
