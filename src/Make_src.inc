# Include file for Makefile for HuC
# Written for unified version, October 16, 2001 by Dave Shadoff 
#
# This file set initial defines, both general and platform-specific
#


#
# general stuff
#
CDEFS =
CFLAGS = -Wall -g -O2
CFLAGS += -DGIT_VERSION="\"$(shell git describe --long --tags --dirty --always)\""
CFLAGS += -DGIT_DATE="\"$(shell git show -s --format=%ci|cut -d" " -f1)\""
LDFLAGS = -g

#
# platform-specific stuff
#
ifeq "$(shell uname -o)" "Cygwin"
CFLAGS += -Wno-char-subscripts
endif

# BASEDIR uses 'd' for path separator; we fix that here
#
ROOTDIR=$(subst d ,/,$(BASEDIR))
BINDIR=$(ROOTDIR)/bin
MAKETARG=$(ROOTDIR)/src/Maketarg.inc
EXESUFFIX = 
CP = cp
RM = rm -f
CC = gcc
