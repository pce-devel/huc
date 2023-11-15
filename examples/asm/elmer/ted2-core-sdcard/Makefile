all: sdcard.pce

include ../Make_ex.inc

AFLAGS ?= -newproc -strip -m -l 2 -S

SRC_INC	= pceas.inc pcengine.inc core.inc core-startup.asm core-kernel.asm joypad.asm tty.asm crc32.asm ted2.asm ted2-sd.asm ted2-fat32.asm
SRC_OVL	= sdcard.asm core-config.inc

sdcard.pce: $(SRC_OVL) $(SRC_INC)
	$(AS) $(AFLAGS) -raw sdcard.asm
