all: core-scsitest.iso scsitest.pce

include ../Make_ex.inc

AFLAGS ?= -newproc -strip -m -l 2 -S

SRC_INC = pceas.inc pcengine.inc core.inc core-startup.asm core-kernel.asm joypad.asm adpcm.asm cdrom.asm
SRC_OVL = scsitest.asm core-config.inc

SRC_ISO = scsitest.ovl ../data/alice.vdc ../data/alice.ram ../data/umbrella-16k.vox

core-scsitest.iso: $(SRC_ISO)
	$(IL) core-scsitest.iso -ipl="test-CD CD-ROM2",0x4000,0x4000,0,1,2,3,4 -asm $(SRC_ISO)

scsitest.ovl: $(SRC_OVL) $(SRC_INC) scsitest.pce
	$(AS) $(AFLAGS) -cd -overlay scsitest.asm

scsitest.pce: $(SRC_OVL) $(SRC_INC)
	$(AS) $(AFLAGS) -raw scsitest.asm
