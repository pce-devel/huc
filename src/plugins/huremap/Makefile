# Makefile for huremap
#
# Written for Linux development version, September 27, 2001 by Dave Shadoff
#

#
# Defines
#

BASEDIR=..d ..d ..
include ../../Make_src.inc

HDRS =
OBJS = huremap.o

LIBS =

LDFLAGS += -Wl,-Bstatic -Wl,-subsystem,windows

EXE  = huremap$(EXESUFFIX)

#
# Subdirectories
#

all::
	$(MAKE) $(EXE)

#
# Executable
#

$(EXE): $(OBJS) $(LIBS) $(HDRS)
	$(CC) $(LDFLAGS) $(OBJS) $(LIBS) -o $@
	$(CP) $(EXE) $(BINDIR)/plugins/

#
# Targets
#

include $(MAKETARG)

clean::
	rm -f $(EXE) $(BINDIR)/plugins/$(EXE) $(OBJS)
