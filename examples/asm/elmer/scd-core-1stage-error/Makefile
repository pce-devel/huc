all: core-1stage-error.iso

include ../Make_ex.inc

AFLAGS ?= -newproc -strip -m -l 2 -S -overlay

SRC_INC = pceas.inc pcengine.inc core.inc core-startup.asm core-kernel.asm joypad.asm
SRC_OVL = stage1.asm core-config.inc

ERR_INC = pceas.inc pcengine.inc core.inc core-startup.asm core-kernel.asm joypad.asm
ERR_OVL = error.asm core-config.inc saz_vdc.zx0 saz_vce.zx0

SRC_ISO = stage1.ovl

core-1stage-error.iso: $(SRC_ISO) error.ovl
	$(IL) core-1stage-error.iso -boot="../include/ipl-scd.dat" -ipl="1-Stage-cderr CD" -asm $(SRC_ISO) -cderr error.ovl

stage1.ovl: $(SRC_OVL) $(SRC_INC)
	$(AS) $(AFLAGS) -scd stage1.asm

# Note that the error overlay is assembled as "-cd" and NOT "-scd"!

error.ovl: $(ERR_OVL) $(ERR_INC)
	$(AS) $(AFLAGS) -cd error.asm
