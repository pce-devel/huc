include ../Make_ex.inc
VPATH = ../include ../font

AFLAGS ?= -kc -newproc -strip -m -l 2 -S

SRC_INC	= pceas.inc pcengine.inc core.inc core-startup.asm core-kernel.asm joypad.asm kickc.asm kickc-final.asm
SRC_OVL	= shmup.asm core-config.inc

all: shmup.pce

shmup.pce: $(SRC_OVL) $(SRC_INC)
	$(AS) $(AFLAGS) -raw shmup.asm

clean:
	rm -f *.pce *.sgx *.bin *.lst *.sym *.s
