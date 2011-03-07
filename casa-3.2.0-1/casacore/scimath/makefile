# $Id: makefile 19881 2007-02-15 04:17:29Z Malte.Marquarding $

# Use the generic AIPS++ class implementation makefile.
#------------------------------------------------------
include $(word 1, $(AIPSPATH))/code/install/makefile.imp

# Set up a parallel make if PARALLEL_MAKE is defined in makedefs.
ifdef PARALLEL_MAKE
  MAKE := $(PARALLEL_MAKE)
endif
