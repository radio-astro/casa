# $Id: makefile,v 1.5 2006/09/22 21:36:52 hye Exp $


XLIBLIST := XERCES QT4 # WCSLIB PGPLOT

# Use the generic AIPS++ class implementation makefile.
#------------------------------------------------------
AIPSROOT := $(word 1, $(CASAPATH))
AIPSARCH := $(AIPSROOT)/$(word 2, $(CASAPATH))
include $(AIPSARCH)/makedefs
SKIP_MAKEDEFS := YES

ifneq "$(wildcard $(QT4ROOT)/*)" ""
include $(word 1, $(CASAPATH))/code/install/makefile.imp
endif

qtclean :
	rm -f *.ui.h *.qrc.cc *.moc.cc

