#
# Makefile for plugins sources for Cosmigo's Pro Motion NG.
#
# https://www.cosmigo.com/
#

ifeq ($(OS),Windows_NT)
	SUBDIRS = huremap husetpalette huzerozero

all clean: plugins
	@$(MAKE) $(SUBDIRS) "COMMAND=$@"

plugins:
	mkdir -p ../../bin/plugins

.PHONY: $(SUBDIRS)

$(SUBDIRS):
	@echo " "
	@echo " -----> make $(COMMAND) in $@"
	$(MAKE) --directory=$@ $(COMMAND)

else
all clean:

endif
