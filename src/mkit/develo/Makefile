# DEVELO TOOLS MAKEFILE [MSDOS]
#

#
# Defines
#
BASEDIR=..d ..d ..
include ../../Make_src.inc

LIBDIR   += -L./lib
LIB      += -ldevel

EXECMX   = execmx$(EXESUFFIX)
PERUN    = perun$(EXESUFFIX)
GETROM   = getrom$(EXESUFFIX)
TARGETS  = $(EXECMX) $(PERUN) $(GETROM)

#
#

all::
ifneq ($(shell uname -s),Linux)
	@echo 'Skipping Develo directories on this platform'
else
	$(MAKE) lib "COMMAND=$@"
	$(MAKE) $(TARGETS)
endif

clean::
ifneq ($(shell uname -s),Linux)
	@echo 'Skipping Develo directories on this platform'
else
	$(MAKE) lib "COMMAND=$@"
endif

.PHONY: lib

lib:
	$(MAKE) --directory=$@ $(COMMAND)

# EXE
#
$(EXECMX) : execmx.c lib/libdevel.a
	$(CC) $(CFLAGS) -o $(EXECMX) execmx.c $(LIBDIR) $(LIB)
	$(CP) $(EXECMX) $(BINDIR)

$(PERUN)  : perun.c  lib/libdevel.a
	$(CC) $(CFLAGS) -o $(PERUN)  perun.c  $(LIBDIR) $(LIB)
	$(CP) $(PERUN) $(BINDIR)

$(GETROM) : getrom.c lib/libdevel.a
	$(CC) $(CFLAGS) -o $(GETROM) getrom.c $(LIBDIR) $(LIB)
	$(CP) $(GETROM) $(BINDIR)

indent:
	uncrustify -l c -q -c ../as/uncrustify.cfg --replace *.c */*.c */*.h

#
# Targets
#
include $(MAKETARG)

clean::
	rm -f $(TARGETS) $(BINDIR)/$(EXECMX) $(BINDIR)/$(PERUN) $(BINDIR)/$(GETROM)
