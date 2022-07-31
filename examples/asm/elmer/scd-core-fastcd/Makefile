all: core-fastcd.iso fastcd.pce

include ../Make_ex.inc

AFLAGS ?= -newproc -strip -m -l 2 -S

SRC_INC = pceas.inc pcengine.inc core.inc core-startup.asm core-kernel.asm joypad.asm adpcm.asm cdrom.asm
SRC_OVL = fastcd.asm core-config.inc

SRC_ISO = fastcd.ovl alice.vdc alice.ram umbrella-16k.vox

core-fastcd.iso: $(SRC_ISO)
	$(IL) core-fastcd.iso -boot="../include/ipl-scd.dat" -ipl="Fast-CD SuperCD" -asm $(SRC_ISO)

fastcd.ovl: $(SRC_OVL) $(SRC_INC) fastcd.pce
	$(AS) $(AFLAGS) -scd -overlay fastcd.asm

fastcd.pce: $(SRC_OVL) $(SRC_INC)
	$(AS) $(AFLAGS) fastcd.asm
