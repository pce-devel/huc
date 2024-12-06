all: core-fastcd.iso fastcd.pce

include ../Make_ex.inc

AFLAGS ?= --newproc --strip -m -l 2 -S -gA

SRC_INC = pceas.inc pcengine.inc core.inc core-startup.asm core-kernel.asm joypad.asm adpcm.asm cdrom.asm
SRC_OVL = fastcd.asm core-config.inc

SRC_ISO = fastcd.ovl ../data/alice.vdc ../data/alice.ram ../data/umbrella-16k.vox

core-fastcd.iso: $(SRC_ISO)
	$(IL) core-fastcd.iso --ipl="Fast-CD SuperCD" $(SRC_ISO)

fastcd.pce: $(SRC_OVL) $(SRC_INC)
	$(AS) $(AFLAGS) --raw fastcd.asm

fastcd.ovl: $(SRC_OVL) $(SRC_INC) fastcd.pce
	$(AS) $(AFLAGS) --scd --overlay fastcd.asm
