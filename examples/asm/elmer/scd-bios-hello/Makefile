all: bios-hello.iso

include ../Make_ex.inc

AFLAGS ?= -newproc -strip -m -l 2 -S -overlay

SRC_INC = pceas.inc pcengine.inc
SRC_OVL = hello.asm

SRC_ISO = hello.ovl

bios-hello.iso: $(SRC_ISO)
	$(IL) bios-hello.iso -boot="../include/ipl-scd.dat" -ipl="Hello SuperCD" -asm $(SRC_ISO)

hello.ovl: $(SRC_OVL) $(SRC_INC)
	$(AS) $(AFLAGS) -scd hello.asm
