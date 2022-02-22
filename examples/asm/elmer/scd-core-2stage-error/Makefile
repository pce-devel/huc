include ../Make_ex.inc
VPATH = ../include ../font ../data

AFLAGS ?= -newproc -strip -m -l 2 -S -overlay

SRC1_INC = pceas.inc pcengine.inc core.inc core-startup.asm core-kernel.asm joypad.asm
SRC1_OVL = core-stage1.asm core-config.inc

SRC2_INC = pceas.inc pcengine.inc core.inc core-startup.asm core-kernel.asm joypad.asm
SRC2_OVL = stage2.asm core-config.inc

ERR_INC = pceas.inc pcengine.inc core.inc core-startup.asm core-kernel.asm joypad.asm
ERR_OVL = error.asm core-config.inc saz_vdc.zx0 saz_vce.zx0

SRC_ISO  = core-stage1.bin stage2.ovl

all: core-2stage-error.iso

core-2stage-error.iso: $(SRC_ISO) error.ovl
	$(IL) core-2stage-error.iso -ipl="2-Stage cderr CD",0x4000,0x4000,0,1,2,3,4 -asm $(SRC_ISO) -cderr error.ovl

core-stage1.bin core-stage1.sym: $(SRC1_OVL) $(SRC1_INC)
	$(AS) $(AFLAGS) -scd -trim core-stage1.asm

core-stage1.s: core-stage1.s2i core-stage1.sym
	$(S2I) ../include/core-stage1.s2i

stage2.ovl: $(SRC2_OVL) $(SRC2_INC) core-stage1.s
	$(AS) $(AFLAGS) -scd stage2.asm

# Note that the error overlay is assembled as "-cd" and NOT "-scd"!

error.ovl: $(ERR_OVL) $(ERR_INC)
	$(AS) $(AFLAGS) -cd error.asm

clean:
	rm -f *.iso *.ovl *.bin *.lst *.sym *.s
