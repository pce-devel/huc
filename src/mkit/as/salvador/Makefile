# SALVADOR COMPRESSION LIBRARY MAKEFILE
#

#
# Defines
#
BASEDIR=..d ..d ..d ..
include ../../../Make_src.inc

OBJS    = divsufsort.o divsufsort_utils.o matchfinder.o shrink.o sssort.o trsort.o

LIB     = salvador.a
EXE     = $(LIB)

CFLAGS += -O3 -g -fomit-frame-pointer

#
#

all: $(LIB)

# RULES
#
divsufsort.o       : divsufsort.c
divsufsort_utils.o : divsufsort_utils.c
matchfinder.o      : matchfinder.c
shrink.o           : shrink.c
sssort.o           : sssort.c
trsort.o           : trsort.c

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
