# $Id$

XLIBLIST := QT4 WCSLIB MIRLIB LAPACK F77 CFITSIO
#XLIBLIST:=QT4 Motif PGPLOT WCSLIB X11 XPM LAPACK F77 CFITSIO MIRLIB
AIPSROOT := $(word 1, $(CASAPATH))
AIPSARCH := $(AIPSROOT)/$(word 2, $(CASAPATH))
include $(AIPSARCH)/makedefs
SKIP_MAKEDEFS := YES

ifneq "$(wildcard $(QT4ROOT)/*)" ""
include $(word 1, $(CASAPATH))/code/install/makefile.tst
endif
