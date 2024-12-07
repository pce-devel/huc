all: shmup.pce

include ../Make_ex.inc

AFLAGS ?= --kc --newproc --strip -m -l 2 -S -gA

SRC_INC	= pceas.inc pcengine.inc core.inc core-startup.asm core-kernel.asm joypad.asm kickc.asm kickc-final.asm
SRC_OVL	= shmup.asm core-config.inc

shmup.pce: $(SRC_OVL) $(SRC_INC)
	$(AS) $(AFLAGS) --raw shmup.asm
