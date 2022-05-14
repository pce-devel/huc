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
	mkdir -p /usr/include/pce
	cp -pr include/pce/* /usr/include/pce/

test:
ifneq ($(shell uname -s),Linux)
	@echo 'Note: "make test" only runs on a linux platform.'
else
	cd test ; ./mk
endif

check:
	md5sum -c < examples/checksum.txt

DATE = $(shell date +%F)

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
	find test         -type f -name '*.s'   -delete
	find test         -type f -name '*.pce' -delete
	find test         -type f -name '*.sym' -delete
	mv tmp/* bin/
	rm -d tmp
	rm -f huc-$(DATE).zip
	zip -r huc-$(DATE) * -x *.zip -x .*

examples: src
