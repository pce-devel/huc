# Include file for Makefile for HuC
# Written for unified version, October 16, 2001 by Dave Shadoff 
#
# This file set initial defines, both general and platform-specific
#


#
# general stuff
#
GIT_VERSION := $(subst -,.,$(shell git describe --long --always))
GIT_DATE := $(shell git show -s --format=%ci|cut -d" " -f1)
CDEFS =
CFLAGS = -Wall -g -O2
CFLAGS += -DGIT_VERSION="\"$(GIT_VERSION)\""
CFLAGS += -DGIT_DATE="\"$(GIT_DATE)\""
LDFLAGS = -g

#
# platform-specific stuff
#

# BASEDIR uses 'd' for path separator; we fix that here
#
ROOTDIR=$(subst d ,/,$(BASEDIR))
BINDIR=$(ROOTDIR)/bin
MAKETARG=$(ROOTDIR)/src/Maketarg.inc
EXESUFFIX = 
CP = cp
RM = rm -f
CC = cc
