# Makefile for examples/as/elmer
#

PREREQS = ipl-scd
SUBDIRS = scd-bios-hello scd-bios-hello-error rom-bare-vdctest rom-core-hello \
          cd-core-1stage cd-core-2stage scd-core-1stage scd-core-1stage-error \
          scd-core-2stage scd-core-2stage-error rom-kickc-hello rom-kickc-shmup \
          ted2-core-hwdetect ted2-core-sdcard

all clean:
	@$(MAKE) $(PREREQS) "COMMAND=$@"
	@$(MAKE) $(SUBDIRS) "COMMAND=$@"

.PHONY: $(PREREQS) $(SUBDIRS)

$(PREREQS) $(SUBDIRS):
	@echo " "
	@echo " -----> make $(COMMAND) in $@"
	$(MAKE) --directory=$@ $(COMMAND)
