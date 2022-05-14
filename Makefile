#
# Makefile for HuC sources
#

SUBDIRS = src tgemu examples

all clean: bin
	@$(MAKE) $(SUBDIRS) "COMMAND=$@"

bin:
	mkdir -p bin

.PHONY: $(SUBDIRS)

$(SUBDIRS):
	@echo " "
	@echo " -----> make $(COMMAND) in $@"
	$(MAKE) --directory=$@ $(COMMAND)

install:
	cp -p bin/* /usr/local/bin
	mkdir -p /usr/include/pce
	cp -pr include/pce/* /usr/include/pce/

package:
	mkdir -p tmp
	strip bin/*
	mv bin/* tmp/
	$(MAKE) --directory=src   clean > /dev/null
	$(MAKE) --directory=tgemu clean > /dev/null
	cd examples
	find . -type f -name '*.s'   -delete
	find . -type f -name '*.lst' -delete
	find . -type f -name '*.sym' -delete
	find . -type f -name '*.bin' -delete
	find . -type f -name '*.ovl' -delete
	cd ..
	mv tmp/* bin/
	rm -d tmp
	rm -f huc.zip
	zip -r huc * -x .*

check:
	md5sum -c < examples/checksum.txt

examples: src
