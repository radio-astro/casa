 # $Id$

# HELPACKS ought to be a sed script that generates the package list from the
# Refman.latex source.
DOCSUSER  := $(word 1, $(CASAPATH))/docs/taskref
HELPAREA  := $(word 1, $(CASAPATH))/docs/xmlcasa/tasks/helpfiles

EXTRA_TEXINPUTS := $(HELPAREA)

# Use the generic AIPS++ documentation makefile
#----------------------------------------------
include $(word 1, $(CASAPATH))/code/install/makefile.doc

HTLATEXOPTS := "htcasa,section+,next,p-width,5"
CASAGOOGLESEARCH := 006707099318971825654:0jfwitmloig

$(DOCSAREA)/TaskRef.pdf  : $(HELPAREA)/*.htex TaskRef.latex


