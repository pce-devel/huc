# PNGREAD LIBRARY MAKEFILE
#

#
# Defines
#
BASEDIR=..d ..d ..d ..
include ../../../Make_src.inc

OBJS    = adler32.o crc32.o inffast.o inflate.o inftrees.o zutil.o\
          png.o pngerror.o pngget.o pngmem.o pngread.o pngrio.o pngrtran.o pngrutil.o pngset.o pngtrans.o

LIB     = pngread.a
EXE     = $(LIB)

CFLAGS += -DPNG_USER_CONFIG -DNO_GZCOMPRESS -DZ_SOLO -DNO_GZIP

#
#

all: $(LIB)

# RULES
#
$(OBJS)    : pnglibconf.h
adler32.o  : adler32.c
crc32.o    : crc32.c
inffast.o  : inffast.c
inflate.o  : inflate.c
inftrees.o : inftrees.c
zutil.o    : zutil.c
png.o      : png.c
pngerror.o : pngerror.c
pngget.o   : pngget.c
pngmem.o   : pngmem.c
pngread.o  : pngread.c
pngrio.o   : pngrio.c
pngrtran.o : pngrtran.c
pngrutil.o : pngrutil.c
pngset.o   : pngset.c
pngtrans.o : pngtrans.c

# EXE
#
$(LIB): $(OBJS)
	ar rs $(LIB) $(OBJS)

#
# Targets
#
include $(MAKETARG)

clean::
	rm -f $(TARGETS)
