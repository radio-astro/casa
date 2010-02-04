# $Id: makefile 17904 2004-09-30 05:44:56Z gvandiep $

# Use the generic AIPS++ class implementation makefile.
#------------------------------------------------------
XLIBLIST := WCSLIB CFITSIO
include $(word 1, $(AIPSPATH))/code/install/makefile.imp

# Set up a parallel make if PARALLEL_MAKE is defined in makedefs
ifdef PARALLEL_MAKE
  MAKE := $(PARALLEL_MAKE)
endif
