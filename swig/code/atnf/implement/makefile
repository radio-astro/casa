# $Id$

# Use the generic AIPS++ class implementation makefile.
#------------------------------------------------------
include $(word 1, $(CASAPATH))/code/install/makefile.imp
ifdef PARALLEL_MAKE
   MAKE := $(PARALLEL_MAKE)
endif

.PHONY : pks pksio

PKSSUBS := $(addprefix $(CODEDIR)/,PKSIO pks)

pks pksio : $(PKSSUBS) $(LIB)
