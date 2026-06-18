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
	@cd test ; /bin/sh ./test_compiler.sh
	@cd test ; /bin/sh ./test_hucc.sh

check:
	@cd test ; /bin/sh ./test_examples.sh


DATE = $(shell date +%F)
ifneq ($(OS),Windows_NT)
	PLATFORMSUFFIX = $(shell uname)
else
	PLATFORMSUFFIX = Win64
endif


package:
	mkdir -p tmp
	cd bin ; find . -type f -exec strip {} +
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
ifeq ($(OS),Windows_NT)
	wget https://mirror.msys2.org/mingw/ucrt64/mingw-w64-ucrt-x86_64-make-4.4.1-3-any.pkg.tar.zst
	tar --zstd -xf mingw-w64-ucrt-x86_64-make-4.4.1-3-any.pkg.tar.zst ucrt64/bin/mingw32-make.exe
	cp ucrt64/bin/mingw32-make.exe bin/
	rm -f mingw-w64-ucrt-x86_64-make-4.4.1-3-any.pkg.tar.zst
	rm -fr ucrt64
endif
	rm -f huc-$(DATE)-$(PLATFORMSUFFIX).zip
	cd .. ; zip -r huc/huc-$(DATE)-$(PLATFORMSUFFIX).zip huc/* -x *.zip -x .*

examples: src
