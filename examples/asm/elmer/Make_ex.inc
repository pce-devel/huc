# Set up the PCEDEV tools location.
#
# This can be overriden in a user's project.

# ifeq ($(PCE_BINPATH),)
BINDIR := ../../../../bin
# else
# ifeq ($(OS),Windows_NT)
# BINDIR := $(subst \,/,$(PCE_BINPATH))
# else
# BINDIR := $(PCE_BINPATH)
# endif
# endif

export PATH := $(PWD)/../../../../bin:$(PATH)

CC     = $(BINDIR)/sdcc
AS     = $(BINDIR)/pceas
IL     = $(BINDIR)/isolink
S2I    = $(BINDIR)/sym2inc
W2V    = $(BINDIR)/wav2vox
CP     = cp
MV     = mv
RM     = rm -f

# Override "cp" and "rm" if using default Windows CMD.

ifeq ($(OS),Windows_NT)
ifeq ($(MSYSTEM),)
# Use CMD.EXE and Windows pathing.
CP     = copy
MV     = ren
RM     = del
else
# Use UNIX tools if Windows MSYS2.
endif
endif

# GNU Make wants UNIX path separators in the makefile.

VPATH = ../include:../font:../data:../../../../include/hucc

# PCE_INCLUDE needs platform-specific path separators.

ifeq ($(OS),Windows_NT)
export PCE_INCLUDE = $(subst :,;,$(subst /,\,$(VPATH)))
else
export PCE_INCLUDE = $(VPATH)
endif

# Default "clean" that works on both Unix shell and Windows_NT CMD.
#
# User projects can just add to the list of TARGETS to clean!
#
# Windows CMD.EXE will fail if the specified file does not exist,
# and so any user-added files *MUST* be in the form of wildcards!

TARGETS = *.pce *.sgx *.iso *.ovl *.out *.lst *.sym *.s

clean::
ifeq ($(RM),del)
	for %i in ( $(subst /,\,$(TARGETS)) ) do del "%i"
else
	$(RM) $(TARGETS)
endif
