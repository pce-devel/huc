all: rcrtest.pce

include ../Make_ex.inc

AFLAGS ?= -newproc -strip -m -l 2 -S

SRC_INC	= pceas.inc pcengine.inc joypad.asm tty.asm vce.asm vdc.asm
SRC_OVL	= rcrtest.asm

rcrtest.pce: $(SRC_OVL) $(SRC_INC)
	$(AS) $(AFLAGS) -raw rcrtest.asm
