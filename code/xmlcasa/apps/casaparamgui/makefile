XLIBLIST := QT4 XERCES PYTHON
AIPSROOT := $(word 1, $(CASAPATH))
AIPSARCH := $(AIPSROOT)/$(word 2, $(CASAPATH))
include $(AIPSARCH)/makedefs
SKIP_MAKEDEFS := YES
BINEXECS := casabrowser

ifneq "$(wildcard $(QT4ROOT)/*)" ""
# Use the generic AIPS++ app makefile.
#------------------------------------------------------
include $(word 1, $(CASAPATH))/code/install/makefile.app
endif
