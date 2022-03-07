#
# Makefile for examples/as/elmer
#

SUBDIRS = ipl-scd scd-bios-hello scd-bios-hello-error rom-core-hello cd-core-1stage cd-core-2stage scd-core-1stage scd-core-2stage scd-core-1stage-error scd-core-2stage-error rom-kickc-hello rom-kickc-shmup

all clean:
	@$(MAKE) $(SUBDIRS) "COMMAND=$@"

.PHONY: $(SUBDIRS)

$(SUBDIRS):
	@echo " "
	@echo " -----> make $(COMMAND) in $@"
	$(MAKE) --directory=$@ $(COMMAND)
