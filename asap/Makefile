### Notice: 
###    This Makefile assumes CASA+CASACore enviroment. 
###    For who wants to build ASAP with older CASA without CASACore 
###    environment, replace $(CASAPATH) with $(AIPSPATH). 
#for mac
#PREFIX := /opt/casa/darwin
#for linux
PREFIX := $(word 1, $(CASAPATH))/$(word 2,$(CASAPATH))
PYTHONVER := $(shell $(shell make -s -f $(PREFIX)/makedefs VARS=PYTHONVER eval_vars) && echo $$PYTHONVER)
PYDIR := $(PREFIX)/python/$(PYTHONVER)

ifndef ASAPROOT
   ASAPROOT := $(shell pwd)
endif
ATNFD := external-alma

#PY := $(wildcard python/*.py)
PY := $(wildcard python/*.py python/svninfo.txt)
LIBS := /tmp/_asap.so
BINS := bin/asap
APPS := apps/asap2to3

CASAROOT  := $(word 1, $(CASAPATH))
PLATFORM  := $(word 2, $(CASAPATH))
DISTDIR   := asap_$(PLATFORM)

all: module #doc

module:
	@if ( test -f getsvnrev.sh ) ; then /bin/bash getsvnrev.sh ; fi
	@cd $(ASAPROOT)/$(ATNFD); make
	@cd $(ASAPROOT)/src; make
	@cd $(ASAPROOT)/apps; make

doc:
	@cd $(ASAPROOT)/doc; make


install:
	@if ( test -f getsvnrev.sh ) ; then /bin/bash getsvnrev.sh ; fi
	@if ( test ! -d $(PYDIR)/asap ) ; then mkdir -p $(PYDIR)/asap ; fi
	@if ( test ! -d $(PREFIX)/bin ) ; then mkdir -p $(PREFIX)/bin ; fi
	@for file in $(LIBS) ; do cp -f $$file $(PYDIR)/asap ; done
	@for file in $(BINS) ; do cp -f $$file $(PREFIX)/bin ; done
	@for file in $(PY) ; do cp -f $$file $(PYDIR)/asap ; done
	@for file in $(APPS) ; do cp -f $$file $(PREFIX)/bin ; done
	@if ( test ! -d $(PREFIX)/share/asap ) ; then mkdir -p $(PREFIX)/share/asap ; fi
	@cp -f share/ipythonrc-asap $(PREFIX)/share/asap/
	@cd $(ASAPROOT)/$(ATNFD); make install
	@echo "Successfully installed asap module to" $(PYDIR)

clean:
	@cd $(ASAPROOT)/src; make clean
#	@cd $(ASAPROOT)/doc; make clean
	@cd $(ASAPROOT)/$(ATNFD); make realclean
	@cd $(ASAPROOT)/apps; make clean

datadist:
	@echo "Generating ASAP data archive from aips++ installation..."
	@cd $(CASAROOT); tar cfj $(ASAPROOT)/$(DISTDIR)/share/data.tar.bz2 data/ephemerides data/geodetic
	@echo "...done."


dist: module doc
	@cd $(ASAPROOT)
	@if ( test -d $(DISTDIR)  ) ; then rm -rf $(DISTDIR) ; fi
	@mkdir $(DISTDIR)
	@mkdir $(DISTDIR)/build $(DISTDIR)/bin $(DISTDIR)/share
	@for file in $(LIBS) ; do cp -f $$file $(DISTDIR)/build/ ; done
	@for file in $(PY) ; do cp -f $$file $(DISTDIR)/build/ ; done
	@for file in $(BINS) ; do cp -f $$file $(DISTDIR)/bin/ ; done
	@cp -f share/ipythonrc-asap $(DISTDIR)/share/
	make datadist
	@cp -f doc/README $(DISTDIR)/
	@cp -f doc/CHANGELOG $(DISTDIR)/
	@cp -f bin/install.sh $(DISTDIR)/bin/
	@echo "Creating compressed archive..."
	@tar jcf $(DISTDIR).tar.bz2 $(DISTDIR)
	@rm -rf $(DISTDIR)/
	@echo "Successfully created binary package" $(DISTDIR).tar.bz2

.PHONY: install clean
