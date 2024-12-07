all: okitest.pce okitest.ovl core-okitest.iso

include ../Make_ex.inc

AFLAGS ?= --newproc --strip -m -l 2 -S -gA

SRC_INC	= pceas.inc pcengine.inc core.inc core-startup.asm core-kernel.asm joypad.asm tty.asm vce.asm vdc.asm adpcm.asm
SRC_OVL	= okitest.asm core-config.inc

SRC_ISO = okitest.ovl

okitest.pce: $(SRC_OVL) $(SRC_INC)
	$(AS) $(AFLAGS) --raw okitest.asm

okitest.ovl: $(SRC_OVL) $(SRC_INC) okitest.pce
	$(AS) $(AFLAGS) --cd --overlay okitest.asm

core-okitest.iso: $(SRC_ISO)
	$(IL) core-okitest.iso -ipl="OKI Test CD" $(SRC_ISO)
