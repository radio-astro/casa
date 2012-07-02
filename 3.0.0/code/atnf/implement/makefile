# $Id$

# Use the generic AIPS++ class implementation makefile.
#------------------------------------------------------
include $(word 1, $(CASAPATH))/code/install/makefile.imp

.PHONY : pks pksio

PKSSUBS := $(addprefix $(CODEDIR)/,PKSIO pks)

pks pksio : $(PKSSUBS) $(LIB)
