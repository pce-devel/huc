# Hack to run gmake if using bmake on a BSD system.

.if "$(.MAKE.JOBS)" != ""
JOBS = -j$(.MAKE.JOBS)
.endif

all clean examples test check package:
	gmake $(JOBS) $@
