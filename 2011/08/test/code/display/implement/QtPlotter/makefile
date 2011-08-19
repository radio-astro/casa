# $Id$


XLIBLIST := WCSLIB QT4 PGPLOT 
# Use the generic AIPS++ class implementation makefile.
#------------------------------------------------------

AIPSROOT := $(word 1, $(CASAPATH))
AIPSARCH := $(AIPSROOT)/$(word 2, $(CASAPATH))
include $(AIPSARCH)/makedefs
SKIP_MAKEDEFS := YES

ifneq "$(wildcard $(QT4ROOT)/*)" ""
include $(word 1, $(CASAPATH))/code/install/makefile.imp
endif


#ru :
#	$(QT4ROOT)/bin/rcc -name plotter -o QtPlotter.qrc.cc QtPlotter.qrc

#qt : ru
#	$(QT4ROOT)/bin/moc -o QtCanvas.moc.cc QtCanvas.qo.h
#	$(QT4ROOT)/bin/moc -o QtPlotter.moc.cc QtPlotter.qo.h
#	$(QT4ROOT)/bin/moc -o QtProfile.moc.cc QtProfile.qo.h
#	$(QT4ROOT)/bin/moc -o QtMWCTools.moc.cc QtMWCTools.qo.h

qtclean :
	rm -f *.ui.h *.qrc.cc *.moc.cc

