#
# Makefile for HuC sources
#

SUBDIRS = huc sdcc mcpp mkit isolink tools
NJOBS := $(shell nproc)

all clean:
	@$(MAKE) $(SUBDIRS) "COMMAND=$@"

.PHONY: $(SUBDIRS)

$(SUBDIRS):
	@echo " "
	@echo " -----> make $(COMMAND) in $@"
	$(MAKE) -j $(NJOBS) --directory=$@ $(COMMAND)

