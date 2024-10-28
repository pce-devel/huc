# Makefile for hulz
#
# Written for Linux development version, September 27, 2001 by Dave Shadoff 
#

#
# Defines
#

BASEDIR=..d ..d ..
include ../../Make_src.inc

HDRS = elmer.h targetver.h
OBJS = hulz.o

LIBS =

EXE  = hulz$(EXESUFFIX)

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
	$(CP) $(EXE) $(BINDIR)

#
# Targets
#

include $(MAKETARG)

clean::
	rm -f $(EXE) $(BINDIR)/$(EXE) $(OBJS)
