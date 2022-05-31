all: ../include/ipl-scd.dat

include ../Make_ex.inc

SRC = ipl-scd.asm ../font/font8x8-ascii-exos.dat

AFLAGS ?= -raw -m -l 2 -S

ipl-scd.out: $(SRC)
	$(AS) $(AFLAGS) -ipl ipl-scd.asm -o ipl-scd.out

../include/ipl-scd.dat: ipl-scd.out
ifeq ($(CP),copy)
	$(CP) ipl-scd.out ..\include\ipl-scd.dat
else
	$(CP) ipl-scd.out ../include/ipl-scd.dat
endif
