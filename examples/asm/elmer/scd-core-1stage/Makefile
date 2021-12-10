include ../Make_ex.inc
VPATH = ../include ../font

AFLAGS ?= -newproc -strip -m -l 2 -S -overlay

SRC_INC = pceas.inc pcengine.inc core.inc core-startup.asm core-kernel.asm joypad.asm
SRC_OVL = stage1.asm core-config.inc

SRC_ISO = stage1.ovl

all: core-1stage.iso

core-1stage.iso: $(SRC_ISO)
	$(IL) core-1stage.iso -ipl,"../include/ipl-scd.bin" -asm $(SRC_ISO)

stage1.ovl: $(SRC_OVL) $(SRC_INC)
	$(AS) $(AFLAGS) -scd stage1.asm

clean:
	rm -f *.iso *.ovl *.bin *.lst *.sym *.s
