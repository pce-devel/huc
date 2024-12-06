all: core-1stage.iso

include ../Make_ex.inc

AFLAGS ?= --overlay --newproc --strip -m -l 2 -S -gL

SRC_INC = pceas.inc pcengine.inc core.inc core-startup.asm core-kernel.asm joypad.asm
SRC_OVL = stage1.asm core-config.inc

SRC_ISO = stage1.ovl

core-1stage.iso: $(SRC_ISO)
	$(IL) core-1stage.iso --ipl="1-Stage CD" $(SRC_ISO)

stage1.ovl: $(SRC_OVL) $(SRC_INC)
	$(AS) $(AFLAGS) --cd stage1.asm
