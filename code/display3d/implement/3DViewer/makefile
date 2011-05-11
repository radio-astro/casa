
XLIBLIST := WCSLIB QT4 PGPLOT CFITSIO OPENGL
# Use the generic AIPS++ class implementation makefile.
#------------------------------------------------------

AIPSROOT := $(word 1, $(CASAPATH))
AIPSARCH := $(AIPSROOT)/$(word 2, $(CASAPATH))
include $(AIPSARCH)/makedefs
SKIP_MAKEDEFS := YES

ifneq "$(wildcard $(QT4ROOT)/*)" ""
include $(word 1, $(CASAPATH))/code/install/makefile.imp
endif



ru :
	#$(QT4ROOT)/bin/rcc -name 3DV -o 3DV.qrc.cc 3DV.qrc

qt : ru
	$(QT4ROOT)/bin/moc-qt4 -o Colorbar.moc.cc Colorbar.qo.h
	$(QT4ROOT)/bin/moc-qt4 -o 3DDisplayData.moc.cc 3DDisplayData.qo.h
	$(QT4ROOT)/bin/moc-qt4 -o 3DWindow.moc.cc 3DWindow.qo.h
	$(QT4ROOT)/bin/moc-qt4 -o 3DController.moc.cc 3DController.qo.h
	$(QT4ROOT)/bin/moc-qt4 -o SlideEditor.moc.cc SlideEditor.qo.h

qtclean :
	rm -f *.ui.h *.qrc.cc *.moc.cc

