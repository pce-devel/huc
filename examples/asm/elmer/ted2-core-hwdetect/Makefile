all: hwdetect.pce hwdetect.sgx

include ../Make_ex.inc

AFLAGS ?= -newproc -strip -m -l 2 -S

SRC_INC	= pceas.inc pcengine.inc core.inc core-startup.asm core-kernel.asm joypad.asm tty.asm
SRC_OVL	= hwdetect.asm core-config.inc

hwdetect.pce: $(SRC_OVL) $(SRC_INC)
	$(AS) $(AFLAGS) -raw hwdetect.asm

hwdetect.sgx: hwdetect.pce
	$(CP) hwdetect.pce hwdetect.sgx
