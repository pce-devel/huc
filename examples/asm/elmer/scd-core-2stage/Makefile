all: core-2stage.iso

include ../Make_ex.inc

AFLAGS ?= -newproc -strip -m -l 2 -S -overlay

SRC1_INC = pceas.inc pcengine.inc core.inc core-startup.asm core-kernel.asm joypad.asm
SRC1_OVL = core-stage1.asm core-config.inc

SRC2_INC = pceas.inc pcengine.inc core.inc core-startup.asm core-kernel.asm joypad.asm unpack-zx0.asm
SRC2_OVL = stage2.asm core-config.inc

SRC_ISO  = core-stage1.bin stage2.ovl

core-2stage.iso: $(SRC_ISO)
	$(IL) core-2stage.iso -ipl="2-Stage SuperCD",0x4000,0x4000,0,1,2,3,4 -asm $(SRC_ISO)

core-stage1.bin core-stage1.sym: $(SRC1_OVL) $(SRC1_INC)
	$(AS) $(AFLAGS) -scd -trim core-stage1.asm

core-stage1.s: core-stage1.s2i core-stage1.sym
	$(S2I) ../include/core-stage1.s2i

stage2.ovl: $(SRC2_OVL) $(SRC2_INC) core-stage1.s
	$(AS) $(AFLAGS) -scd stage2.asm
