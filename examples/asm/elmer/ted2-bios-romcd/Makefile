all: ted2romcd.pce

include ../Make_ex.inc

SRC = ted2romcd.asm syscard3-ted2.inc

AFLAGS ?= --raw -m -l 2 -S

ted2romcd.pce: $(SRC)
	$(AS) $(AFLAGS) ted2romcd.asm
ifeq ($(CP),copy)
	$(CP) ted2romcd.pce ..\include\ted2romcd.pce
else
	$(CP) ted2romcd.pce ../include/ted2romcd.pce
endif

# Add this to the list of TARGETS to clean.
#
# Windows CMD.EXE will fail if the specified file does not exist,
# and so any files added here *MUST* be in the form of wildcards!

TARGETS += ../include/ted2romcd.p*
