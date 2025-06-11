#
# Makefile for examples
#

SUBDIRS = scroll sgx shmup metatile1-chrblkmap metatile2-multimap metatile3-multiblk

all clean:
	@$(MAKE) $(SUBDIRS) "COMMAND=$@"

.PHONY: $(SUBDIRS)

$(SUBDIRS):
	@echo " "
	@echo " -----> make $(COMMAND) in $@"
	$(MAKE) --directory=$@ $(COMMAND)
