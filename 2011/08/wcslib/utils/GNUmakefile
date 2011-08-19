#-----------------------------------------------------------------------------
# GNU makefile for WCSLIB 4.7 utilities: fitshdr, HPXcvt, wcsgrid and wcsware.
#
# Summary of the main targets
# ---------------------------
#   build:     Build all utilities.
#   clean (or cleaner): Delete intermediate object files.
#   cleanest (distclean, or realclean): cleaner, and also delete the
#              executables.
#
# Notes:
#   1: If you need to make changes then preferably modify ../makedefs.in
#      instead and re-run configure.
#
#   2: In compiling these utilities, this makefile assumes that the WCSLIB 4.7
#      sources reside in ../{pgsbox,C} (as in the distribution kit).
#
# Author: Mark Calabretta, Australia Telescope National Facility
# http://www.atnf.csiro.au/~mcalabre/index.html
# $Id: GNUmakefile,v 4.7 2011/02/07 07:03:43 cal103 Exp $
#-----------------------------------------------------------------------------
# Get configure settings.
include ../makedefs

UTILS := fitshdr

ifneq "$(CFITSIOINC)" ""
ifneq "$(CFITSIOLIB)" ""
  UTILS += HPXcvt wcsware

  ifneq "$(PGPLOTINC)" ""
  ifneq "$(PGPLOTLIB)" ""
    UTILS += wcsgrid
  endif
  endif
endif
endif

WCSLIB    := ../C/$(WCSLIB)
PGSBOXLIB := ../pgsbox/libpgsbox-$(LIBVER).a

ifneq "$(GETWCSTAB)" ""
  GETWCSTAB := ../C/$(GETWCSTAB)
endif

CPPFLAGS += -I.. -I../C

vpath %.h  ..:../C:../pgsbox
vpath %.in ..


# Static and static pattern rules
#--------------------------------

.PHONY : build clean cleaner cleanest distclean install realclean

build : $(UTILS)

fitshdr : fitshdr.c
	-@ echo ''
	   $(CC) $(CPPFLAGS) $(CFLAGS) -c -o $@.o $<
	   $(LD) -o $@ $@.o $(LDFLAGS) $(LIBS)
	-@ $(RM) $@.o

HPXcvt : HPXcvt.c
	-@ echo ''
	   $(CC) $(CPPFLAGS) $(CFITSIOINC) $(CFLAGS) -c -o $@.o $<
	   $(LD) -o $@ $@.o $(LDFLAGS) $(CFITSIOLIB) $(LIBS)
	-@ $(RM) $@.o

wcsware : wcsware.c $(GETWCSTAB) $(WCSLIB)
	-@ echo ''
	   $(CC) -DDO_CFITSIO $(CPPFLAGS) -I../pgsbox -I../C $(CFITSIOINC) \
	     $(CFLAGS) -c -o $@.o $<
	   $(LD) $(LDFLAGS) -o $@ $@.o $(GETWCSTAB) $(CFITSIOLIB) $(WCSLIB) \
	     $(FLIBS) $(LIBS)
	-@ $(RM) $@.o

wcsgrid : wcsgrid.c $(PGSBOXLIB) $(GETWCSTAB) $(WCSLIB)
	-@ echo ''
	   $(CC) -DDO_CFITSIO $(CPPFLAGS) -I../pgsbox -I../C $(PGPLOTINC) \
	     $(CFITSIOINC) $(CFLAGS) -c -o $@.o $<
	   $(LD) $(LDFLAGS) -o $@ $@.o $(PGSBOXLIB) $(PGPLOTLIB) \
	     $(GETWCSTAB) $(CFITSIOLIB) $(WCSLIB) $(FLIBS) $(LIBS)
	-@ $(RM) $@.o

../C/getwcstab.o :
	   $(MAKE) -C ../C $(@F)

clean cleaner :
	-  $(RM) -r *.o *.i a.out core *.dSYM $(EXTRA_CLEAN)

cleanest distclean realclean : cleaner
	-  $(RM) $(UTILS)

$(PGSBOXLIB) :
	-@ echo ''
	   $(MAKE) -C ../pgsbox lib

$(WCSLIB) :
	-@ echo ''
	   $(MAKE) -C ../C lib

install : build
	-  if [ ! -d "$(BINDIR)" ] ; then \
	     $(INSTALL) -d -m 2775 $(BINDIR) ; \
	   fi
	   $(INSTALL) -m 755 $(UTILS) $(BINDIR)

GNUmakefile : ../makedefs ;

../makedefs ../wcsconfig.h ../wcsconfig_utils.h : makedefs.in wcsconfig.h.in \
    wcsconfig_utils.h.in ../config.status
	-@ $(RM) ../wcsconfig.h ../wcsconfig_utils.h
	   cd .. && config.status

fitshdr : wcsconfig.h wcsconfig_utils.h
wcsware : getwcstab.h wcs.h wcsfix.h wcshdr.h
wcsgrid : cpgsbox.h getwcstab.h wcs.h wcsfix.h wcshdr.h
