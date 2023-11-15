# Hack to run gmake if using bmake on a BSD system.

.if "$(.MAKE.JOBS)" != ""
JOBS = -j$(.MAKE.JOBS)
.endif

.PHONY: all clean examples test check package

all clean examples test check package:
	gmake $(JOBS) $@
