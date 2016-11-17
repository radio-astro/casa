DOCROOT ?= $(firstword $(CASAPATH))/$(word 2, $(CASAPATH))

makefile_path := $(abspath $(lastword $(MAKEFILE_LIST)))
current_dir := $(dir $(makefile_path))
DOXY = $(current_dir)bin/doxy
PLONE = $(current_dir)bin/fetch_plone

all: doxygen plone

doxygen: check
	if test -d "$(DOCROOT)/doc/doxygen"; then                              \
	    rm -rf "$(DOCROOT)/doc/doxygen";                                   \
    fi
	mkdir -p $(DOCROOT)/doc/doxygen &&                                     \
	$(DOXY) out=$(DOCROOT)/doc/doxygen

plone: check
	if test -d "$(DOCROOT)/doc/plone"; then                                \
	    rm -rf "$(DOCROOT)/doc/plone";                                     \
    fi
	mkdir -p $(DOCROOT)/doc/plone &&                                       \
	$(PLONE) out=$(DOCROOT)/doc/plone

check:
	@if test -d "$(DOCROOT)" -a "$(DOCROOT)" != "/"; then                  \
		echo "good";                                                       \
	else                                                                   \
		echo "please specify a DOCROOT where output should be placed";     \
		exit 1;                                                            \
	fi

