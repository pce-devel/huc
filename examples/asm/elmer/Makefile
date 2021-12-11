#
# Makefile for examples/as/elmer
#

SUBDIRS = ipl-scd scd-bios-hello rom-core-hello scd-core-1stage scd-core-2stage

all clean:
	@$(MAKE) $(SUBDIRS) "COMMAND=$@"

.PHONY: $(SUBDIRS)

$(SUBDIRS):
	@echo " "
	@echo " -----> make $(COMMAND) in $@"
	$(MAKE) --directory=$@ $(COMMAND)
