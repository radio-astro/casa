#-----------------------------------------------------------------------------
#   GNU makefile for building WCSLIB 4.3.1.
#
#   Summary of the main targets
#   ---------------------------
#      all:       Do 'make all' in each subdirectory.
#      check:     Do 'make check' in each subdirectory.
#      tests:     Do 'make tests' in each subdirectory.
#      install:   Do 'make install' in each subdirectory.
#      realclean (or cleanest): Do 'make realclean' in each subdirectory.
#      show_all:  Print the values of all variables used in this GNUmakefile.
#      writable:  Run chmod recursively to make all sources writable.
#
#   Notes:
#      1) If you need to make changes then preferably modify makedefs.in
#         instead.
#
#      2) Refer also to the GNUmakefiles in subdirectories, particularly
#         C/GNUmakefile.
#
#   Author: Mark Calabretta, Australia Telescope National Facility
#   http://www.atnf.csiro.au/~mcalabre/index.html
#   $Id: GNUmakefile,v 4.3.1.3 2008/02/20 01:00:07 cal103 Exp cal103 $
#-----------------------------------------------------------------------------
# Get configure settings.
include makedefs

SUBDIRS := C Fortran pgsbox utils
TSTDIRS := C Fortran pgsbox

# Top of the 'make install' hierarchy: psgbox -> Fortran -> C.
INSTDIR := pgsbox

# Is a Fortran compiler available?
ifeq "$(FC)" ""
  SUBDIRS := C Fortran utils
  TSTDIRS := C
  INSTDIR := Fortran
endif

# Is PGPLOT available?
ifeq "$(PGPLOTINC)" ""
  SUBDIRS := $(filter-out pgsbox, $(SUBDIRS))
  INSTDIR := Fortran
endif

ifeq "$(PGPLOTLIB)" ""
  TSTDIRS := $(filter-out pgsbox, $(TSTDIRS))
endif


.PHONY : build check chmod cleanest install realclean show_all tests writable

build :
	-@ for DIR in $(SUBDIRS) ; do \
	     echo '' ; \
	     $(TIMER) ; \
	     $(MAKE) -k -C $$DIR build ; \
	   done

check tests : show_all
	-@ echo ''
	-@ $(TIMER)
	 @ for DIR in $(SUBDIRS) ; do \
	     echo '' ; \
	     $(MAKE) -i -C $$DIR cleaner ; \
	   done
	-@ echo ''
	 @ for DIR in $(TSTDIRS) ; do \
	     echo '' ; \
	     $(TIMER) ; \
	     $(MAKE) -k -C $$DIR $@ ; \
	   done 

install :
	$(MAKE) -k -C $(INSTDIR) $@
	$(MAKE) -k -C utils $@
	$(INSTALL) -m 444 wcsconfig.h wcsconfig_f77.h $(INCDIR)
	$(INSTALL) -m 444 COPYING $(INCDIR)/COPYING
	$(INSTALL) -m 444 README  $(INCDIR)/README

realclean cleanest :
	for DIR in $(SUBDIRS) ; do \
	   $(MAKE) -C $$DIR realclean ; \
	done
	$(RM) -r autom4te.cache autoscan.log
	$(RM) confdefs.h conftest.*
	$(RM) config.log config.status configure.lineno
	$(RM) wcsconfig.h wcsconfig_*.h makedefs
	$(RM) wcslib-*.tar.gz

show_all ::
	-@ echo 'Subdirectories to be built...'
	-@ echo '  SUBDIRS     := $(SUBDIRS)'
	-@ echo '  TSTDIRS     := $(TSTDIRS)'
	-@ echo ''

writable :
	  chmod -R u+w .

GNUmakefile : makedefs ;

makedefs : makedefs.in config.status
	-@ echo ''
	-@ $(TIMER)
	   ./config.status

config.status : configure
	-@ echo ''
	-@ $(TIMER)
	-@ echo ''
	-@ echo "Environment variables that affect 'configure':"
	-@ echo "  CC       = $${CC-(undefined)}"
	-@ echo "  CFLAGS   = $${CFLAGS-(undefined)}"
	-@ echo "  CPP      = $${CPP-(undefined)}"
	-@ echo "  CPPFLAGS = $${CPPFLAGS-(undefined)}"
	-@ echo "  F77      = $${F77-(undefined)}"
	-@ echo "  FFLAGS   = $${FFLAGS-(undefined)}"
	-@ echo "  LDFLAGS  = $${LDFLAGS-(undefined)}"
	-@ echo ''
	   ./configure --no-create


#-----------------------------------------------------------------------------
# These are for code management.

.PHONY : dist

dist : realclean
	-@ echo $(WCSLIBV)/C/RCS        >  wcslib.X
	-@ echo $(WCSLIBV)/C/flexed/RCS >> wcslib.X
	-@ echo $(WCSLIBV)/C/test/RCS   >> wcslib.X
	-@ echo $(WCSLIBV)/Fortran/RCS  >> wcslib.X
	-@ echo $(WCSLIBV)/Fortran/test/RCS >> wcslib.X
	-@ echo $(WCSLIBV)/makedefs     >> wcslib.X
	-@ echo $(WCSLIBV)/other        >> wcslib.X
	-@ echo $(WCSLIBV)/pgsbox/RCS   >> wcslib.X
	-@ echo $(WCSLIBV)/RCS          >> wcslib.X
	-@ echo $(WCSLIBV)/utils/RCS    >> wcslib.X
	-@ echo $(WCSLIBV)/wcslib.X     >> wcslib.X
	   tar cvfX ../$(WCSLIBV).tar wcslib.X -C .. $(WCSLIBV)
	   mv ../$(WCSLIBV).tar ./$(WCSLIBV).tar
	   rm -f wcslib.X $(WCSLIBV).tar.gz
	   gzip --best $(WCSLIBV).tar
	   chmod 444 $(WCSLIBV).tar.gz

configure : configure.ac
	-@ echo ''
	-@ $(TIMER)
	   autoconf

# Code development overrides must be included specifically before 'configure'
# generates makedefs.
-include flavours
