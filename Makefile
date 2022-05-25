#
# Makefile for HuC sources
#

SUBDIRS = src tgemu examples

all clean: bin
	@$(MAKE) $(SUBDIRS) "COMMAND=$@"

bin:
	mkdir -p bin

.PHONY: $(SUBDIRS) test

$(SUBDIRS):
	@echo " "
	@echo " -----> make $(COMMAND) in $@"
	$(MAKE) --directory=$@ $(COMMAND)

install:
	cp -p bin/* /usr/local/bin
	mkdir -p /usr/include/huc
	cp -pr include/huc/* /usr/include/huc/

test:
	cd test ; ./mk

check:
	@echo ''
	md5sum --strict -c < examples/huc/checksum.txt

DATE = $(shell date +%F)
ifneq ($(OS),Windows_NT)
	PLATFORMSUFFIX = $(shell uname)
else
	PLATFORMSUFFIX = Win64
endif


package:
	mkdir -p tmp
	strip bin/*
	mv bin/* tmp/
	$(MAKE) --directory=src   clean > /dev/null
	$(MAKE) --directory=tgemu clean > /dev/null
	find examples     -type f -name '*.s'   -delete
	find examples     -type f -name '*.lst' -delete
	find examples     -type f -name '*.sym' -delete
	find examples     -type f -name '*.ovl' -delete
	find examples/asm -type f -name '*.bin' -delete
	mv tmp/* bin/
	rm -d tmp
	rm -f huc-$(DATE)-$(PLATFORMSUFFIX).zip
	zip -r huc-$(DATE)-$(PLATFORMSUFFIX).zip * -x *.zip -x .*

examples: src
