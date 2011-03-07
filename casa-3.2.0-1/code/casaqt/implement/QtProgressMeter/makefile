# $Id: makefile,v 1.5 2006/09/22 21:36:52 hye Exp $


XLIBLIST := WCSLIB QT4 PGPLOT CORE
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

#qt : ru
#	$(QT4ROOT)/bin/moc -o progress.moc.cc progress.qo.h
#	$(QT4ROOT)/bin/moc -o progresstimer.moc.cc progresstimer.qo.h

qtclean :
	rm -f *.ui.h *.qrc.cc *.moc.cc

