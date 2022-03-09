# Makefile for pce2png
#
# Written for Linux development version, September 27, 2001 by Dave Shadoff 
#

#
# Defines
#

BASEDIR=..d ..d ..
include ../../Make_src.inc

HDRS = bmpfile.h errorcode.h lexer.h mappedfile.h pce2png.h pcxfile.h pngfile.h pxlmap.h sysid.h elmer.h targetver.h
OBJS = bmpfile.o errorcode.o lexer.o mappedfile.o pce2png.o pcxfile.o pngfile.o pxlmap.o sysid.o 

LIBS = pngwrite/pngwrite.a

CFLAGS  += -Ipngwrite

EXE  = pce2png$(EXESUFFIX)

#
# Subdirectories
#

all::
	$(MAKE) pngwrite "COMMAND=$@"
	$(MAKE) $(EXE)

.PHONY: pngwrite

pngwrite:
	$(MAKE) --directory=$@ $(COMMAND)

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
	$(MAKE) pngwrite "COMMAND=$@"
	rm -f $(EXE) $(BINDIR)/$(EXE) $(OBJS)
