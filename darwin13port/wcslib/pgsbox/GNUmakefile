#-----------------------------------------------------------------------------
# GNU makefile for building PGSBOX 4.7
#
# Summary of the main targets
# ---------------------------
#   build:     Build the library.
#
#   clean:     Delete intermediate object files.
#
#   cleaner:   clean, and also delete the test executables.
#
#   cleanest (distclean, or realclean):  cleaner, and also delete the pgsbox
#              and WCSLIB object libraries.
#
#   check (or test): Compile and run the test programs.  By default they are
#              executed in batch mode; use
#
#                gmake MODE=interactive check
#
#              to run them interactively.
#
#   tests:     Compile the test programs (but don't run them).
#
# Notes:
#   1) If you need to make changes then preferably modify ../makedefs.in
#      instead and re-run configure.
#
#   2) In compiling the test programs, this makefile assumes that the
#      WCSLIB 4.7 sources reside in ../C and ../Fortran (as in the
#      distribution kit).
#
# Author: Mark Calabretta, Australia Telescope National Facility
# http://www.atnf.csiro.au/~mcalabre/index.html
# $Id: GNUmakefile,v 4.7 2011/02/07 07:03:43 cal103 Exp $
#-----------------------------------------------------------------------------
# Get configure settings.
include ../makedefs

PGSBOXLIB := libpgsbox-$(LIBVER).a
MODULES   := pgsbox.o \
             cpgsbox.o \
             pgwcsl.o \
             pgcrfn.o

WCSLIB   := ../C/$(WCSLIB)

# For building the sharable library.
PICLIB := libpgsbox-PIC.a
WCSSHR := ../C/$(SHRLIB)
SHRLIB := $(subst libwcs,libpgsbox,$(SHRLIB))
SHRLD  := $(subst libwcs,libpgsbox,$(SHRLD))
SHRLN  := $(subst libwcs,libpgsbox,$(SHRLN))

CPPFLAGS += -I. -I.. -I../C

vpath %.h   ..:../C
vpath %.in  ..
vpath %.inc ../Fortran

# Pattern rules
#--------------

$(PGSBOXLIB)(%.o) : %.f
	-@ echo ''
	   $(FC) -I../Fortran $(FFLAGS) -c $<
	   $(AR) r $(PGSBOXLIB) $%
	-@ $(RM) $%

$(PGSBOXLIB)(%.o) : %.c
	-@ echo ''
	   $(CC) $(CPPFLAGS) $(CFLAGS) -c $<
	   $(AR) r $(PGSBOXLIB) $%
	-@ $(RM) $%

$(PICLIB)(%.o) : %.f
	-@ echo ''
	   $(FC) -I../Fortran $(FFLAGS) $(SHRFLAGS) -c $<
	   $(AR) r $(PICLIB) $%
	-@ $(RM) $%

$(PICLIB)(%.o) : %.c
	-@ echo ''
	   $(CC) $(CPPFLAGS) $(CFLAGS) $(SHRFLAGS) -c $<
	   $(AR) r $(PICLIB) $%
	-@ $(RM) $%

%.i : %.c
	-@ echo ''
	-@ $(RM) $@
	   $(CPP) $(CPPFLAGS) $(CFLAGS) $< > $@

%.d : %.c
	-@ echo ''
	-@ $(CPP) $(CPPFLAGS) $(CFLAGS) $< | \
	   sed -n -e 's|^# 1 "\([^/].*\.h\)".*|\1|p' | \
	   sort -u

# Static and static pattern rules
#--------------------------------

.PHONY : build check clean cleaner cleanest distclean install realclean test \
         tests wcslib

build : lib

lib : $(PGSBOXLIB) $(SHRLIB)

$(PGSBOXLIB) : $(MODULES:%=$(PGSBOXLIB)(%))
	-@ echo ''
	   $(RANLIB) $@

$(SHRLIB) : $(PICLIB)
	-@ echo ''
	-@ $(RM) -r tmp
	-  mkdir tmp && \
	     cd tmp && \
	     trap 'cd .. ; $(RM) -r tmp' 0 1 2 3 15 ; \
	     $(AR) x ../$(PICLIB) && \
	     $(SHRLD) -o $@ *.o ../$(WCSSHR) $(LDFLAGS) $(PGPLOTLIB) && \
	     mv $@ ..

$(PICLIB) : $(MODULES:%.o=$(PICLIB)(%.o)) ;

install : build
	   $(MAKE) -C ../Fortran install
	   $(INSTALL) -m 644 $(PGSBOXLIB) $(LIBDIR)
	   $(RANLIB) $(LIBDIR)/$(PGSBOXLIB)
	   $(RM) $(LIBDIR)/libpgsbox.a
	   $(LN_S) $(LIBDIR)/$(PGSBOXLIB) $(LIBDIR)/libpgsbox.a
	-  if [ "$(SHRLIB)" != "" ] ; then \
	     $(INSTALL) -m 644 $(SHRLIB) $(LIBDIR) ; \
	     if [ "$(SHRLN)" != "" ] ; then \
	       if [ -h "$(LIBDIR)/$(SHRLN)" ] ; then \
	         $(RM) $(LIBDIR)/$(SHRLN) ; \
	       fi ; \
	       $(LN_S) $(SHRLIB) $(LIBDIR)/$(SHRLN) ; \
	     fi ; \
	   fi
	   $(INSTALL) -m 444 *.h $(INCDIR)

clean :
	-  $(RM) -r *.o *.i a.out core *.dSYM $(EXTRA_CLEAN)

cleaner : clean
	-  $(RM) pgtest cpgtest

cleanest distclean realclean : cleaner
	-  $(RM) $(PICLIB) libpgsbox-*.a libpgsbox.so.* libpgsbox.*.dylib

check test : tests
	-@ echo ''
	-@ $(TIMER)
	-@ echo 'Running the PGSBOX test program, PGTEST:'
	-@ if [ '$(MODE)' = interactive ] ; then \
	     ./pgtest ; \
	   else \
	     ./pgtest < /dev/null ; \
	   fi
	-@ echo ''
	-@ echo ''
	-@ $(TIMER)
	-@ echo 'Running the cpgsbox() test program, cpgtest:'
	-@ if [ '$(MODE)' = interactive ] ; then \
	     ./cpgtest ; \
	   else \
	     ./cpgtest < /dev/null ; \
	   fi

tests : wcslib pgtest cpgtest

wcslib : $(WCSLIB)

$(WCSLIB) :
	-@ echo ''
	   $(MAKE) -C ../Fortran lib

pgtest : pgtest.f fscan.f lngvel.f $(PGSBOXLIB) $(WCSLIB)
	-@ $(RM) $@
	-@ echo ''
	   $(FC) -I../Fortran $(FFLAGS) $(LDFLAGS) -o $@ $^ \
	     $(PGPLOTLIB) $(LIBS)

cpgtest : cpgtest.c fscan.f lngvel.f $(PGSBOXLIB) $(WCSLIB)
	-@ $(RM) $@
	-@ echo ''
	   $(FC) -I../Fortran $(FFLAGS) -c fscan.f lngvel.f
	   $(CC) $(CPPFLAGS) $(PGPLOTINC) $(CFLAGS) $(LDFLAGS) -o $@ \
	     $(^:.f=.o) $(PGPLOTLIB) $(FLIBS) $(LIBS)
	-@ $(RM) fscan.o lngvel.o

GNUmakefile : ../makedefs ;

../makedefs ../wcsconfig_f77.h : makedefs.in wcsconfig_f77.h.in \
    ../config.status
	-@ $(RM) ../wcsconfig_f77.h
	   cd .. && config.status

show ::
	-@ echo 'For building libpgsbox...'
	-@ echo '  PGSBOXLIB   := $(PGSBOXLIB)'
	-@ echo '  PICLIB      := $(PICLIB)'
	-@ echo '  WCSSHR      := $(WCSSHR)'
	-@ echo '  SHRLIB      := $(SHRLIB)'
	-@ echo '  SHRFLAGS    := $(SHRFLAGS)'
	-@ echo '  SHRLD       := $(SHRLD)'
	-@ echo '  SHRLN       := $(SHRLN)'
	-@ echo ''
	-@ echo '  MODULES     := $(MODULES)'

# Dependencies
#-------------

$(PGSBOXLIB)(cpgsbox.o) : cpgsbox.h wcsconfig_f77.h
$(PGSBOXLIB)(pgwcsl.o)  : cel.h lin.h prj.h spc.h sph.h spx.h tab.h wcs.h \
                          wcsconfig_f77.h

pgtest.o  : wcs.inc
cpgtest.o : cel.h cpgsbox.h lin.h prj.h spc.h spx.h tab.h wcs.h wcsfix.h \
            wcsconfig_f77.h
