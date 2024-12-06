all: hwdetect.sgx

include ../Make_ex.inc

AFLAGS ?= --newproc --strip -m -l 2 -S -gA

SRC_INC	= pceas.inc pcengine.inc core.inc core-startup.asm core-kernel.asm joypad.asm tty.asm
SRC_OVL	= hwdetect.asm core-config.inc

hwdetect.sgx: $(SRC_OVL) $(SRC_INC)
	$(AS) $(AFLAGS) --sgx --raw hwdetect.asm
