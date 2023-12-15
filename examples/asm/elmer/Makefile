# Makefile for examples/as/elmer
#

PREREQS = ipl-scd
SUBDIRS = rom-bare-rcrtest rom-bare-tiatest \
          rom-core-hello rom-core-okitest \
          scd-bios-hello scd-bios-hello-error \
          cd-core-1stage cd-core-2stage scd-core-1stage scd-core-1stage-error \
          scd-core-2stage scd-core-2stage-error scd-core-fastcd \
          rom-kickc-hello rom-kickc-shmup ted2-core-hwdetect ted2-core-sdcard \
          ted2-core-gulliver

all clean:
	@$(MAKE) $(PREREQS) "COMMAND=$@"
	@$(MAKE) $(SUBDIRS) "COMMAND=$@"

.PHONY: $(PREREQS) $(SUBDIRS)

$(PREREQS) $(SUBDIRS):
	@echo " "
	@echo " -----> make $(COMMAND) in $@"
	$(MAKE) --directory=$@ $(COMMAND)
