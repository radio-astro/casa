# $Id: makefile,v 1.5 2006/09/22 21:36:52 hye Exp $


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
#	$(QT4ROOT)/bin/rcc -name logviewer -o logviewer.qrc.cc logviewer.qrc

#qt : ru
#	$(QT4ROOT)/bin/moc -o logviewer.moc.cc logviewer.qo.h
#	$(QT4ROOT)/bin/moc -o logmodel.moc.cc logmodel.qo.h
#	$(QT4ROOT)/bin/moc -o logwatcher.moc.cc logwatcher.qo.h

qtclean :
	rm -f *.ui.h *.qrc.cc *.moc.cc

