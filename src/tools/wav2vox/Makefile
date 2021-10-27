# Makefile for wav2vox
#
# Written for Linux development version, September 27, 2001 by Dave Shadoff 
#

#
# Defines
#
BASEDIR=..d ..d ..
include ../../Make_src.inc

HDRS = wav2vox.h adpcmoki.h riffio.h
OBJS = wav2vox.o adpcmoki.o riffio.o
EXE = wav2vox$(EXESUFFIX)

all: $(EXE)


$(EXE): $(OBJS) $(LIBS) $(HDRS)
	$(CC) $(LDFLAGS) $(OBJS) $(LIBS) -o $@
	$(CP) $(EXE) $(BINDIR)

#
# Targets
#
include $(MAKETARG)
