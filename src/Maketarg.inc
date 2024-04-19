# Include file for Makefile for HuC
# Written for unified version, October 16, 2001 by Dave Shadoff 
#
# This file defines default targets
#

clean::
	$(RM) core
	$(RM) *.o *.gcno *.gcda *.gcov
	$(RM) -rf --one-file-system msbuild/.vs msbuild/.x86 msbuild/.x64
ifneq "$(EXE)" ""
	$(RM) $(EXE) $(BINDIR)/$(EXE) $(BINDIR)/*.pdb
endif

%.o: %.c
	$(CC) $(CDEFS) $(CFLAGS) -c $< -o $@
%.o: %.cc
	$(CXX) $(CDEFS) $(CXXFLAGS) -c $< -o $@
