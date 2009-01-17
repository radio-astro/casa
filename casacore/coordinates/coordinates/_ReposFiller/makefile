# $Id: makefile 18826 2005-07-08 16:15:59Z ddebonis $

# Use the generic AIPS++ class implementation makefile.
#------------------------------------------------------
XLIBLIST := WCSLIB
include $(word 1, $(AIPSPATH))/code/install/makefile.imp

# Set up a parallel make if PARALLEL_MAKE is defined in makedefs.
ifdef PARALLEL_MAKE
  MAKE := $(PARALLEL_MAKE)
endif
