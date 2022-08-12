all: gulliver.pce gulliver.ovl

include ../Make_ex.inc

AFLAGS ?= -newproc -strip -m -l 2 -S

SRC_INC = pceas.inc pcengine.inc core.inc core-startup.asm core-kernel.asm joypad.asm adpcm.asm cdrom.asm movie.asm
SRC_OVL = gulliver.asm core-config.inc

gulliver.ovl: $(SRC_OVL) $(SRC_INC) gulliver.pce
	$(AS) $(AFLAGS) -scd -overlay gulliver.asm
	cp gulliver.ovl /d/retrogamedev/tools/gulliverboy/

gulliver.pce: $(SRC_OVL) $(SRC_INC)
	$(AS) $(AFLAGS) gulliver.asm
