all: ted2usbcd.pce

include ../Make_ex.inc

SRC = ted2usbcd.asm ../ted2-bios-romcd/syscard3-ted2.inc

AFLAGS ?= -raw -m -l 2 -S

ted2usbcd.pce: $(SRC)
	$(AS) $(AFLAGS) ted2usbcd.asm
ifeq ($(CP),copy)
	$(CP) ted2usbcd.pce ..\include\ted2usbcd.pce
else
	$(CP) ted2usbcd.pce ../include/ted2usbcd.pce
endif

# ../include/ted2usbcd.pce: ./ted2usbcd.pce

# Add this to the list of TARGETS to clean.
#
# Windows CMD.EXE will fail if the specified file does not exist,
# and so any files added here *MUST* be in the form of wildcards!

TARGETS += ../include/ted2usbcd.p*
