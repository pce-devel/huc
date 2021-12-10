include ../Make_ex.inc
VPATH = ../include ../font

AFLAGS ?= -newproc -strip -m -l 2 -S

SRC_INC	= pceas.inc pcengine.inc core.inc core-startup.asm core-kernel.asm joypad.asm
SRC_OVL	= hello.asm core-config.inc

all: hello.pce

hello.pce: $(SRC_OVL) $(SRC_INC)
	$(AS) $(AFLAGS) -raw hello.asm

clean:
	rm -f *.pce *.sgx *.bin *.lst *.sym *.s
