# $Id$

# Use the generic AIPS++ package makefile.
#-----------------------------------------
include $(word 1, $(CASAPATH))/code/install/makefile.pkg

.PHONY : pks

pks :
	 @ $(MAKE) -C $(MAKEIMPD) pks
	 @ $(MAKE) -C $(MAKEAPSD) pks
