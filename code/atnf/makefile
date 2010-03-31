# $Id$

# Use the generic AIPS++ package makefile.
#-----------------------------------------
include $(word 1, $(CASAPATH))/code/install/makefile.pkg
ifdef PARALLEL_MAKE
    MAKE := $(PARALLEL_MAKE)
endif


.PHONY : pks

pks :
	 @ $(MAKE) -C $(MAKEIMPD) pks
	 @ $(MAKE) -C $(MAKEAPSD) pks
