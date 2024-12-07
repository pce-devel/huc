all: sherlock-hack.ovl sherlock-ted2.ovl dat2csv

include ../Make_ex.inc

ifeq ($(OS),Windows_NT)
TARGETS += *.exe
else
TARGETS += dat2csv
endif

sherlock-hack.ovl: sherlock-hack.asm sherlock-boot.bin
	$(AS) --raw -m -l 2 -S --trim sherlock-hack.asm -o sherlock-hack.ovl
	cp sherlock-hack.ovl /d/retrogamedev/tools/sherlock/

sherlock-ted2.ovl: sherlock-ted2.asm sherlock-hack.ovl
	$(AS) --newproc --strip -m -l 2 -S --scd --overlay sherlock-ted2.asm
	cp sherlock-ted2.ovl /d/retrogamedev/tools/sherlock/

dat2csv: dat2csv.c
	cc dat2csv.c -o dat2csv
