# $Id: makefile,v 1.5 2006/09/22 21:36:52 hye Exp $


XLIBLIST :=QT4
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
#	$(QT4ROOT)/bin/rcc -name filecatalog -o filecatalog.qrc.cc filecatalog.qrc

#qt : ru
#	$(QT4ROOT)/bin/moc -o filecatalog.moc.cc filecatalog.qo.h
#	$(QT4ROOT)/bin/moc -o dirmodel.moc.cc dirmodel.qo.h
#	$(QT4ROOT)/bin/moc -o pictureviewer.moc.cc pictureviewer.qo.h
#	$(QT4ROOT)/bin/moc -o texteditor.moc.cc texteditor.qo.h

qtclean :
	rm -f *.ui.h *.qrc.cc *.moc.cc

