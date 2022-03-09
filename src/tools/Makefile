#
# Makefile for HuC tools sources
#

SUBDIRS = pcxtool mod2mml mml wav2vox sym2inc pce2png

all clean:
	@$(MAKE) $(SUBDIRS) "COMMAND=$@"

.PHONY: $(SUBDIRS)

$(SUBDIRS):
	@echo " "
	@echo " -----> make $(COMMAND) in $@"
	$(MAKE) --directory=$@ $(COMMAND)

