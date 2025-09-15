#
# Makefile for examples
#

SUBDIRS = scroll sgx shmup \
	metatile1-chrblkmap metatile2-multimap metatile3-multiblk \
	obeybrew-tutorial1 obeybrew-tutorial2 obeybrew-tutorial3 obeybrew-tutorial4 \
	obeybrew-tutorial5 obeybrew-tutorial6 obeybrew-tutorial7

all clean:
	@$(MAKE) $(SUBDIRS) "COMMAND=$@"

.PHONY: $(SUBDIRS)

$(SUBDIRS):
	@echo " "
	@echo " -----> make $(COMMAND) in $@"
	$(MAKE) --directory=$@ $(COMMAND)
