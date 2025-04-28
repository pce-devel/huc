# PNGREADWRITE LIBRARY MAKEFILE
#

#
# Defines
#
BASEDIR=..d ..d ..d ..
include ../../../Make_src.inc

OBJS    = adler32.o compress.o crc32.o deflate.o inffast.o inflate.o inftrees.o trees.o zutil.o\
          png.o pngerror.o pngget.o pngmem.o pngread.o pngrio.o pngrtran.o pngrutil.o\
          pngset.o pngtrans.o pngwio.o pngwrite.o pngwtran.o pngwutil.o

LIB     = pngreadwrite.a
EXE     = $(LIB)

CFLAGS += -DPNG_USER_CONFIG -DNO_GZCOMPRESS -DZ_SOLO -DNO_GZIP

ifneq ($(shell $(CC) -v 2>&1 | grep -c "clang version"), 1)
CFLAGS += -Wno-multistatement-macros
endif

#
#

all: $(LIB)

# RULES
#
$(OBJS)    : pnglibconf.h
adler32.o  : adler32.c
compress.o : compress.c
crc32.o    : crc32.c
deflate.o  : deflate.c
inffast.o  : inffast.c
inflate.o  : inflate.c
inftrees.o : inftrees.c
trees.o    : trees.c
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
pngwio.o   : pngwio.c
pngwrite.o : pngwrite.c
pngwtran.o : pngwtran.c
pngwutil.o : pngwutil.c

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
