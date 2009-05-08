#
#   Copyright (C) 1992-2004
#   Associated Universities, Inc. Washington DC, USA.
#
#   This program is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 2 of the License, or
#   (at your option) any later version.
#
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#   along with this program; if not, write to the Free Software
#   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#

###
### ensure that these start out undefined...
###
CFLAGS :=
CXXFLAGS :=
FFLAGS :=

-include makedefs

###
### defaults setup below:
###
###   CC:       C compiler
###   C++:      C++ compiler
###   FC:       fortran compiler
###   OPT:      optimization level
###   DESTDIR:  root dir for where build will be stored
###   INSTDIR:  root dir for final installation
###

ifeq "$(OPT)" ""
OPT := -O2
endif

ifeq "$(C++)" ""
C++ := c++
endif

ifeq "$(CC)" ""
CC  := gcc
endif

os := $(shell uname | tr 'A-Z' 'a-z')
arch := $(shell uname -p)

ifeq "$(CFLAGS)" ""
CFLAGS := $(OPT) -DCASA_USECASAPATH -DCASACORE_NEEDS_RETHROW
ifeq "$(os)" "linux"
ifeq "$(arch)" "x86_64"
CFLAGS += -m64
endif
endif
endif
ifeq "$(CXXFLAGS)" ""
CXXFLAGS := $(OPT) -DCASA_USECASAPATH -DCASACORE_NEEDS_RETHROW
ifeq "$(os)" "linux"
ifeq "$(arch)" "x86_64"
CXXFLAGS += -m64
endif
endif
endif
ifeq "$(FFLAGS)" ""
FFLAGS := $(OPT) -DCASA_USECASAPATH -DCASACORE_NEEDS_RETHROW
ifeq "$(os)" "linux"
ifeq "$(arch)" "x86_64"
FFLAGS += -m64
endif
endif
endif

DEP := 1
ONELIB := 0

VERSION:=$(shell head -1 VERSION | perl -pe "s|^(\S+).*|\$$1|")

fastdep := $(shell which fastdep 2> /dev/null)
assay := $(shell which assay 2> /dev/null)

ifeq "$(os)" "darwin"
SO := dylib
SOV := $(VERSION).dylib
endif
ifeq "$(os)" "linux"
SO := so
SOV := so.$(VERSION)
INC += -I/usr/include/cfitsio
endif

##
## setup DESTDIR
##
ifeq "$(DESTDIR)" ""
DESTDIR := $(shell echo $$CASAPATH | perl -pe "s|(\S+).*|\$$1|")
ifeq "$(DESTDIR)" ""
DESTDIR := $(shell pwd)/build
else
ifeq "$(ARCH)" ""
ARCH := $(shell echo $$CASAPATH | perl -pe "s|\S+\s+(\S+).*|\$$1|")
endif
endif
endif

##
## setup INSTDIR
##
ifneq "$(INSTDIR)" ""
instlib_path := $(INSTDIR)/lib/
else
instlib_path :=
endif

INCDIR := $(DESTDIR)/include

##
## setup fortran compiler
##
ifeq "$(FC)" ""
FC  := $(shell type gfortran 2> /dev/null | perl -pe 's@^\S+\s+is\s+@@')
endif
ifeq "$(FC)" ""
FC  := $(shell type g77 2> /dev/null | perl -pe 's@^\S+\s+is\s+@@')
endif
ifneq "$(shell echo $(FC) | perl -pe 's|.*?gfortran.*|gfortran|')" "gfortran"
FC_LIB := -lgfortran
endif


ifeq "$(FC)" ""
allsys: 
	@echo "error could not find fortran compiler; please set the 'FC', GNUmakefile variable"
else
allsys: images msfits
endif

ifneq "$(ARCH)" ""
LIBDIR := $(DESTDIR)/$(ARCH)/lib
else
LIBDIR := $(DESTDIR)/lib
endif
LASTVERSION := $(LIBDIR)/.version.last

CASACC := $(shell find casa -type f -name '*.cc' | egrep -v '/test/|/apps/')
CASAOBJ := $(CASACC:%.cc=%.o)
TCASA := $(shell find casa -type f -name 't*.cc' | grep /test/)
CASAINC := $(shell find casa -type f \( -name '*.h' -o -name '*.tcc' \) | egrep -v '/test/|/apps/' | perl -pe "s@^casa/@$(INCDIR)/casacore/@g")
ifneq "$(ARCH)" ""
CASALIB := $(shell echo $(CASAOBJ) | perl -pe "s|(\w+\\.o)|$(ARCH)/\$$1|g")
CASADEP := $(shell echo $(CASAOBJ) | perl -pe "s|(\w+)\\.o|$(ARCH)/\$$1.dep|g")
else
CASALIB := $(CASAOBJ)
CASADEP := $(shell echo $(CASAOBJ) | perl -pe "s|(\w+)\\.o|\$$1.dep|g")
endif
CASALIB_PATH := $(LIBDIR)/libcasa_casa.$(SOV)
CASALNK_PATH := $(LIBDIR)/libcasa_casa.$(SO)
CORELIB_PATH := $(LIBDIR)/libcasacore.$(SOV)
CORELNK_PATH := $(LIBDIR)/libcasacore.$(SO)
### components   -> tables      -> casa
###              -> coordinates -> (cfitsio)
###                             -> (wcs)
###                             -> measures  -> scimath   -> casa
###                                                       -> (lapack)
###                                                       -> (blas)
COMPONENTSCC := $(shell find components -type f -name '*.cc' | egrep -v '/test/|/apps/')
COMPONENTSOBJ := $(COMPONENTSCC:%.cc=%.o)
TCOMPONENTS := $(shell find components -type f -name 't*.cc' | grep /test/)
COMPONENTSINC := $(shell find components -type f \( -name '*.h' -o -name '*.tcc' \) | egrep -v '/test/|/apps/' | perl -pe "s@^components/@$(INCDIR)/casacore/@g")
ifneq "$(ARCH)" ""
COMPONENTSLIB := $(shell echo $(COMPONENTSOBJ) | perl -pe "s|(\w+\\.o)|$(ARCH)/\$$1|g")
COMPONENTSDEP := $(shell echo $(COMPONENTSOBJ) | perl -pe "s|(\w+)\\.o|$(ARCH)/\$$1.dep|g")
else
COMPONENTSLIB := $(COMPONENTSOBJ)
COMPONENTSDEP := $(shell echo $(COMPONENTSOBJ) | perl -pe "s|(\w+)\\.o|\$$1.dep|g")
endif
COMPONENTSLIB_PATH := $(LIBDIR)/libcasa_components.$(SOV)
COMPONENTSLNK_PATH := $(LIBDIR)/libcasa_components.$(SO)
###  coordinates -> measures  -> scimath -> casa
###                                      -> (lapack)
###                                      -> (blas)
###              -> fits      -> tables  -> casa
###                           -> (cfitsio)
###              -> (wcslib)
COORDINATESCC := $(shell find coordinates -type f -name '*.cc' | egrep -v '/test/|/apps/')
COORDINATESOBJ := $(COORDINATESCC:%.cc=%.o)
TCOORDINATES := $(shell find coordinates -type f -name 't*.cc' | grep /test/)
COORDINATESINC := $(shell find coordinates -type f \( -name '*.h' -o -name '*.tcc' \) | egrep -v '/test/|/apps/' | perl -pe "s@^coordinates/@$(INCDIR)/casacore/@g")
ifneq "$(ARCH)" ""
COORDINATESLIB := $(shell echo $(COORDINATESOBJ) | perl -pe "s|(\w+\\.o)|$(ARCH)/\$$1|g")
COORDINATESDEP := $(shell echo $(COORDINATESOBJ) | perl -pe "s|(\w+)\\.o|$(ARCH)/\$$1.dep|g")
else
COORDINATESLIB := $(COORDINATESOBJ)
COORDINATESDEP := $(shell echo $(COORDINATESOBJ) | perl -pe "s|(\w+)\\.o|\$$1.dep|g")
endif
COORDINATESLIB_PATH := $(LIBDIR)/libcasa_coordinates.$(SOV)
COORDINATESLNK_PATH := $(LIBDIR)/libcasa_coordinates.$(SO)
###  lattices -> tables  -> casa
###           -> scimath -> casa
###                      -> (lapack)
###                      -> (blas)
LATTICESCC := $(shell find lattices -type f -name '*.cc' | egrep -v '/test/|/apps/')
LATTICESOBJ := $(LATTICESCC:%.cc=%.o)
TLATTICES := $(shell find lattices -type f -name 't*.cc' | grep /test/)
LATTICESINC := $(shell find lattices -type f \( -name '*.h' -o -name '*.tcc' \) | egrep -v '/test/|/apps/' | perl -pe "s@^lattices/@$(INCDIR)/casacore/@g")
ifneq "$(ARCH)" ""
LATTICESLIB := $(shell echo $(LATTICESOBJ) | perl -pe "s|(\w+\\.o)|$(ARCH)/\$$1|g")
LATTICESDEP := $(shell echo $(LATTICESOBJ) | perl -pe "s|(\w+)\\.o|$(ARCH)/\$$1.dep|g")
else
LATTICESLIB := $(LATTICESOBJ)
LATTICESDEP := $(shell echo $(LATTICESOBJ) | perl -pe "s|(\w+)\\.o|\$$1.dep|g")
endif
LATTICESLIB_PATH := $(LIBDIR)/libcasa_lattices.$(SOV)
LATTICESLNK_PATH := $(LIBDIR)/libcasa_lattices.$(SO)
###  images      -> mirlib
###              -> lattices -> tables         -> casa
###                          -> scimath        -> casa
###                                            -> (lapack)
###                                            -> (blas)
###              -> fits        -> measures    -> scimath -> casa
###                             -> tables      -> casa
###                             -> (cfitsio)
###              -> components  -> tables      -> casa
###                             -> coordinates -> (cfitsio)
###                                            -> (wcs)
###                                            -> measures  -> scimath   -> casa
###                                                                      -> (lapack)
###                                                                      -> (blas)
###         
IMAGESCC := $(shell find images -type f -name '*.cc' | egrep -v '/test/|/apps/')
IMAGESOBJ := $(IMAGESCC:%.cc=%.o)
TIMAGES := $(shell find images -type f -name 't*.cc' | grep /test/)
IMAGESINC := $(shell find images -type f \( -name '*.h' -o -name '*.tcc' \) | egrep -v '/test/|/apps/' | perl -pe "s@^images/@$(INCDIR)/casacore/@g")
ifneq "$(ARCH)" ""
IMAGESLIB := images/images/Images/ImageExprGram.lcc images/images/Images/ImageExprGram.ycc \
		$(shell echo $(IMAGESOBJ) | perl -pe "s|(\w+\\.o)|$(ARCH)/\$$1|g")
IMAGESDEP :=    $(shell echo $(IMAGESOBJ) | perl -pe "s|(\w+)\\.o|$(ARCH)/\$$1.dep|g")
else
IMAGESLIB := images/images/Images/ImageExprGram.lcc images/images/Images/ImageExprGram.ycc \
		$(IMAGESOBJ)
IMAGESDEP :=    $(shell echo $(IMAGESOBJ) | perl -pe "s|(\w+)\\.o|\$$1.dep|g")
endif
IMAGESLIB_PATH := $(LIBDIR)/libcasa_images.$(SOV)
IMAGESLNK_PATH := $(LIBDIR)/libcasa_images.$(SO)
###  tables -> casa
TABLESCC := $(shell find tables -type f -name '*.cc' | egrep -v '/test/|/apps/')
TABLESOBJ := $(TABLESCC:%.cc=%.o)
TTABLES := $(shell find tables -type f -name 't*.cc' | grep /test/)
TABLESINC := $(shell find tables -type f \( -name '*.h' -o -name '*.tcc' \) | egrep -v '/test/|/apps/' | perl -pe "s@^tables/@$(INCDIR)/casacore/@g")
ifneq "$(ARCH)" ""
TABLESLIB := tables/tables/Tables/RecordGram.lcc tables/tables/Tables/RecordGram.ycc \
		tables/tables/Tables/TableGram.lcc tables/tables/Tables/TableGram.ycc \
		$(shell echo $(TABLESOBJ) | perl -pe "s|(\w+\\.o)|$(ARCH)/\$$1|g")
TABLESDEP :=    $(shell echo $(TABLESOBJ) | perl -pe "s|(\w+)\\.o|$(ARCH)/\$$1.dep|g")
else
TABLESLIB := tables/tables/Tables/RecordGram.lcc tables/tables/Tables/RecordGram.ycc \
		tables/tables/Tables/TableGram.lcc tables/tables/Tables/TableGram.ycc \
		$(TABLESOBJ)
TABLESDEP :=    $(shell echo $(TABLESOBJ) | perl -pe "s|(\w+)\\.o|\$$1.dep|g")
endif
TABLESLIB_PATH := $(LIBDIR)/libcasa_tables.$(SOV)
TABLESLNK_PATH := $(LIBDIR)/libcasa_tables.$(SO)
###  scimath -> casa
###          -> (lapack)
###          -> (blas)
SCIMATHCC := $(shell find scimath -type f -name '*.cc' | egrep -v '/test/|/apps/')
SCIMATHF := $(shell find scimath -type f -name '*.f' | egrep -v '/test/|/apps/')
SCIMATHOBJ := $(SCIMATHCC:%.cc=%.o)
SCIMATHFOBJ := $(SCIMATHF:%.f=%.o)
TSCIMATH := $(shell find scimath -type f -name 't*.cc' | grep /test/)
SCIMATHINC := $(shell find scimath -type f \( -name '*.h' -o -name '*.tcc' \) | egrep -v '/test/|/apps/' | perl -pe "s@^scimath/@$(INCDIR)/casacore/@g")
ifneq "$(ARCH)" ""
SCIMATHLIB := $(shell echo $(SCIMATHOBJ) | perl -pe "s|(\w+\\.o)|$(ARCH)/\$$1|g")
SCIMATHDEP := $(shell echo $(SCIMATHOBJ) | perl -pe "s|(\w+)\\.o|$(ARCH)/\$$1.dep|g")
SCIMATHFLIB := $(shell echo $(SCIMATHFOBJ) | perl -pe "s|(\w+\\.o)|$(ARCH)/\$$1|g")
else
SCIMATHLIB := $(SCIMATHOBJ)
SCIMATHDEP := $(shell echo $(SCIMATHOBJ) | perl -pe "s|(\w+)\\.o|\$$1.dep|g")
SCIMATHFLIB := $(SCIMATHFOBJ)
endif
SCIMATHLIB_PATH := $(LIBDIR)/libcasa_scimath.$(SOV)
SCIMATHLNK_PATH := $(LIBDIR)/libcasa_scimath.$(SO)
SCIMATHFLIB_PATH := $(LIBDIR)/libcasa_scimath_f.$(SOV)
SCIMATHFLNK_PATH := $(LIBDIR)/libcasa_scimath_f.$(SO)
###  measures -> scimath -> casa
###           -> tables  -> casa
MEASURESCC := $(shell find measures -type f -name '*.cc' | egrep -v '/test/|/apps/')
MEASURESF := $(shell find measures -type f -name '*.f' | egrep -v '/test/|/apps/')
MEASURESOBJ := $(MEASURESCC:%.cc=%.o)
MEASURESFOBJ := $(MEASURESF:%.f=%.o)
TMEASURES := $(shell find measures -type f -name 't*.cc' | grep /test/)
MEASURESINC := $(shell find measures -type f \( -name '*.h' -o -name '*.tcc' \) | egrep -v '/test/|/apps/' | perl -pe "s@^measures/@$(INCDIR)/casacore/@g")
ifneq "$(ARCH)" ""
MEASURESLIB := $(shell echo $(MEASURESOBJ) | perl -pe "s|(\w+\\.o)|$(ARCH)/\$$1|g")
MEASURESDEP := $(shell echo $(MEASURESOBJ) | perl -pe "s|(\w+)\\.o|$(ARCH)/\$$1.dep|g")
MEASURESFLIB := $(shell echo $(MEASURESFOBJ) | perl -pe "s|(\w+\\.o)|$(ARCH)/\$$1|g")
else
MEASURESLIB := $(MEASURESOBJ)
MEASURESDEP := $(shell echo $(MEASURESOBJ) | perl -pe "s|(\w+)\\.o|\$$1.dep|g")
MEASURESFLIB := $(MEASURESFOBJ)
endif
MEASURESLIB_PATH := $(LIBDIR)/libcasa_measures.$(SOV)
MEASURESLNK_PATH := $(LIBDIR)/libcasa_measures.$(SO)
MEASURESFLIB_PATH := $(LIBDIR)/libcasa_measures_f.$(SOV)
MEASURESFLNK_PATH := $(LIBDIR)/libcasa_measures_f.$(SO)
###  ms -> measures  -> scimath -> casa
###                             -> (lapack)
###                             -> (blas)
MSCC := $(shell find ms -type f -name '*.cc' | egrep -v '/test/|/apps/')
MSOBJ := $(MSCC:%.cc=%.o)
TMS := $(shell find ms -type f -name 't*.cc' | grep /test/)
MSINC := $(shell find ms -type f \( -name '*.h' -o -name '*.tcc' \) | egrep -v '/test/|/apps/' | perl -pe "s@^ms/@$(INCDIR)/casacore/@g")
ifneq "$(ARCH)" ""
MSLIB := ms/ms/MeasurementSets/MSAntennaGram.lcc ms/ms/MeasurementSets/MSAntennaGram.ycc \
	       ms/ms/MeasurementSets/MSArrayGram.lcc ms/ms/MeasurementSets/MSArrayGram.ycc \
	       ms/ms/MeasurementSets/MSCorrGram.lcc ms/ms/MeasurementSets/MSCorrGram.ycc \
	       ms/ms/MeasurementSets/MSFieldGram.lcc ms/ms/MeasurementSets/MSFieldGram.ycc \
	       ms/ms/MeasurementSets/MSScanGram.lcc ms/ms/MeasurementSets/MSScanGram.ycc \
	       ms/ms/MeasurementSets/MSSpwGram.lcc ms/ms/MeasurementSets/MSSpwGram.ycc \
	       ms/ms/MeasurementSets/MSTimeGram.lcc ms/ms/MeasurementSets/MSTimeGram.ycc \
	       ms/ms/MeasurementSets/MSUvDistGram.lcc ms/ms/MeasurementSets/MSUvDistGram.ycc \
		$(shell echo $(MSOBJ) | perl -pe "s|(\w+\\.o)|$(ARCH)/\$$1|g")
MSDEP :=	$(shell echo $(MSOBJ) | perl -pe "s|(\w+)\\.o|$(ARCH)/\$$1.dep|g")
else
MSLIB := ms/ms/MeasurementSets/MSAntennaGram.lcc ms/ms/MeasurementSets/MSAntennaGram.ycc \
	       ms/ms/MeasurementSets/MSArrayGram.lcc ms/ms/MeasurementSets/MSArrayGram.ycc \
	       ms/ms/MeasurementSets/MSCorrGram.lcc ms/ms/MeasurementSets/MSCorrGram.ycc \
	       ms/ms/MeasurementSets/MSFieldGram.lcc ms/ms/MeasurementSets/MSFieldGram.ycc \
	       ms/ms/MeasurementSets/MSScanGram.lcc ms/ms/MeasurementSets/MSScanGram.ycc \
	       ms/ms/MeasurementSets/MSSpwGram.lcc ms/ms/MeasurementSets/MSSpwGram.ycc \
	       ms/ms/MeasurementSets/MSTimeGram.lcc ms/ms/MeasurementSets/MSTimeGram.ycc \
	       ms/ms/MeasurementSets/MSUvDistGram.lcc ms/ms/MeasurementSets/MSUvDistGram.ycc \
		$(MSOBJ)
MSDEP :=	$(shell echo $(MSOBJ) | perl -pe "s|(\w+)\\.o|\$$1.dep|g")
endif
MSLIB_PATH := $(LIBDIR)/libcasa_ms.$(SOV)
MSLNK_PATH := $(LIBDIR)/libcasa_ms.$(SO)
###  fits -> measures  -> scimath -> casa
###       -> tables    -> casa
###       -> (cfitsio)
FITSCC := $(shell find fits -type f -name '*.cc' | egrep -v '/test/|/apps/')
FITSOBJ := $(FITSCC:%.cc=%.o)
TFITS := $(shell find fits -type f -name 't*.cc' | grep /test/)
FITSINC := $(shell find fits -type f \( -name '*.h' -o -name '*.tcc' \) | egrep -v '/test/|/apps/' | perl -pe "s@^fits/@$(INCDIR)/casacore/@g")
ifneq "$(ARCH)" ""
FITSLIB := $(shell echo $(FITSOBJ) | perl -pe "s|(\w+\\.o)|$(ARCH)/\$$1|g")
FITSDEP := $(shell echo $(FITSOBJ) | perl -pe "s|(\w+)\\.o|$(ARCH)/\$$1.dep|g")
else
FITSLIB := $(FITSOBJ)
FITSDEP := $(shell echo $(FITSOBJ) | perl -pe "s|(\w+)\\.o|\$$1.dep|g")
endif
FITSLIB_PATH := $(LIBDIR)/libcasa_fits.$(SOV)
FITSLNK_PATH := $(LIBDIR)/libcasa_fits.$(SO)
###  msfits -> fits -> (cfitsio)
###                 -> measures  -> tables  -> casa
###                              -> scimath -> casa
###         -> ms   -> measures  -> scimath -> casa
###                              -> (lapack)
###                              -> (blas)
MSFITSCC := $(shell find msfits -type f -name '*.cc' | egrep -v '/test/|/apps/')
MSFITSOBJ := $(MSFITSCC:%.cc=%.o)
TMSFITS := $(shell find msfits -type f -name 't*.cc' | grep /test/)
MSFITSINC := $(shell find msfits -type f \( -name '*.h' -o -name '*.tcc' \) | egrep -v '/test/|/apps/' | perl -pe "s@^msfits/@$(INCDIR)/casacore/@g")
ifneq "$(ARCH)" ""
MSFITSLIB := $(shell echo $(MSFITSOBJ) | perl -pe "s|(\w+\\.o)|$(ARCH)/\$$1|g")
MSFITSDEP := $(shell echo $(MSFITSOBJ) | perl -pe "s|(\w+)\\.o|$(ARCH)/\$$1.dep|g")
else
MSFITSLIB := $(MSFITSOBJ)
MSFITSDEP := $(shell echo $(MSFITSOBJ) | perl -pe "s|(\w+)\\.o|\$$1.dep|g")
endif
MSFITSLIB_PATH := $(LIBDIR)/libcasa_msfits.$(SOV)
MSFITSLNK_PATH := $(LIBDIR)/libcasa_msfits.$(SO)
###
MIRCC := $(shell find mirlib -type f -name '*.c' | egrep -v '/test/|/apps/')
MIROBJ := $(MIRCC:%.c=%.o)
MIRINC := $(shell find mirlib -type f \( -name '*.h' -o -name '*.tcc' \) | egrep -v '/test/|/apps/' | perl -pe "s@^@$(INCDIR)/casacore/@g")
ifneq "$(ARCH)" ""
MIRLIB := $(shell echo $(MIROBJ) | perl -pe "s|(\w+\\.o)|$(ARCH)/\$$1|g")
MIRDEP := $(shell echo $(MIROBJ) | perl -pe "s|(\w+)\\.o|$(ARCH)/\$$1.dep|g")
else
MIRLIB := $(MIROBJ)
MIRDEP := $(shell echo $(MIROBJ) | perl -pe "s|(\w+)\\.o|\$$1.dep|g")
endif
MIRLIB_PATH := $(LIBDIR)/libcasa_mirlib.$(SOV)
MIRLNK_PATH := $(LIBDIR)/libcasa_mirlib.$(SO)

COREINC := -Iimages -Icasa -Ifits -Icomponents -Icoordinates -Ilattices \
		-Imeasures -I. -Ims -Imsfits -Iscimath -Itables

ifneq "$(fastdep)" ""
ifneq "$(ARCH)" ""
define make-depend
  $(fastdep) $(COREINC) +O $(ARCH) $1 > $2
endef
else
define make-depend
  $(fastdep) $(COREINC) $1 > $2
endef
endif
else
define make-depend
  gcc -MM -MT $(subst .dep,.o,$2) $(COREINC) $1 > $2
endef
endif

ifneq "$(assay)" ""
define run-test
  (cd $(dir $1) && ($(assay) ./$(notdir $1) | perl -pe 's@\./$(notdir $1)$$@$1@')  || exit 0)
endef
else
define run-test
  echo "run test on:" $1
endef
endif

###
### object files whose source files have been removed or renamed
###
define orphan-objects
  perl -e 'use File::Find; %source = ( ); @removed = ( ); sub find_source { if ( -f $$_ && m/\.(?:cc|c|f)$$/ ) { s/\.(?:cc|c|f)$$//; $$source{"$$File::Find::dir/$$_"} = 1; } } sub find_orphan { return unless ("$1" eq "" || $$File::Find::dir =~ m|/$1|); if ( -f $$_ && m/\.o$$/ ) { my $$file = $$_; my $$src = $$File::Find::dir; $$src =~ s|/$1$$|| if "$1" ne ""; $$src = "$$src/$$file"; $$src =~ s|\.o$$||; if ( ! defined $$source{$$src} ) { push(@removed, "$$File::Find::dir/$$file"); unlink($$file); my $$dep = $$file; $$dep =~ s|\.o$$|.dep|; if ( -f $$dep ) { push(@removed, "$$File::Find::dir/$$dep"); unlink($$dep); } } } } find( { wanted => \&find_source }, "$2" ); find( { wanted => \&find_orphan }, "$2" ); if ( scalar(@removed) > 0 ) { print "removed object files which no longer have a source file:\n"; print "\t" . join("\n\t",@removed) . "\n"; }'
endef

###
### header file dependencies in *.dep files which have been removed or renamed
###
define orphan-deps
  perl -e 'use File::Find; %headers = ( ); $$scrubre = ""; sub find_scrub { if ( -f $$_ && m/\.dep$$/ ) { my $$file = $$_; my @out = ( ); my $$dump = 0; open( DEP, "< $$file" ); while ( <DEP> ) { $$dump = 1 if s@(?:$$scrubre)@@g; push( @out, $$_ ); } close( DEP ); if ( $$dump ) { open( OUT, "> $$file" ); print OUT join("", @out); close( OUT ); } } } sub find_dep { if ( -f $$_ && m/\.dep$$/ ) { open( DEP, "< $$_ " ); while (<DEP>) { if ( m@(\S+\.h)@ ) { $$headers{$$1} = 1; } } close( DEP ); } } find( { wanted => \&find_dep }, "$1" ); @scrub = ( ); foreach ( keys %headers ) { if ( ! -f $$_ ) { push( @scrub, $$_ ); } } if ( scalar(@scrub) > 0 ) { print "removing out-of-date dependencies:\n"; print "\t" . join("\n\t",@scrub) . "\n"; $$scrubre = join("|",@scrub); find( { wanted => \&find_scrub }, "$1" ); }'
endef

###
### installed header files which have been removed or renamed
###
define orphan-headers
  perl -e 'use File::Find; %headers = ( ); @removed = ( ); sub find_hdrs { if ( -f $$_ && ( m/\.h$$/ || m/\.tcc$$/ )) { $$headers{"$$File::Find::dir/$$_"} = 1; } } sub find_orphans { if ( -f $$_ && ( m/\.h$$/ || m/\.tcc$$/ )) { my $$file = "$$File::Find::dir/$$_"; my $$found = 0; $$file =~ s@^\./@@; foreach $$key (keys %headers) { if ( $$key =~ m|\Q$$file\E$$| ) { $$found = 1; last; } } unless ( $$found ) { push( @removed, "$$File::Find::dir/$$_"); unlink($$_); } } } find( { wanted => \&find_hdrs }, "$2" ); chdir("$1/casacore"); find( { wanted => \&find_orphans }, "$2" ); if ( scalar(@removed) > 0 ) { print "removed out-of-date, installed header files from $1:\n"; print "\t" . join("\n\t",@removed) . "\n"; }'
endef

define install-header
	if test ! -d $(dir $2); then mkdir -p $(dir $2); fi
	cp $1 $2
endef

#--- --- --- --- --- --- --- --- --- --- --- --- --- --- --- ---
$(INCDIR)/casacore/casa/%.h: casa/casa/%.h
	$(call install-header,$<,$@)

$(INCDIR)/casacore/components/%.h: components/components/%.h
	$(call install-header,$<,$@)

$(INCDIR)/casacore/coordinates/%.h: coordinates/coordinates/%.h
	$(call install-header,$<,$@)

$(INCDIR)/casacore/fits/%.h: fits/fits/%.h
	$(call install-header,$<,$@)

$(INCDIR)/casacore/images/%.h: images/images/%.h
	$(call install-header,$<,$@)

$(INCDIR)/casacore/lattices/%.h: lattices/lattices/%.h
	$(call install-header,$<,$@)

$(INCDIR)/casacore/measures/%.h: measures/measures/%.h
	$(call install-header,$<,$@)

$(INCDIR)/casacore/ms/%.h: ms/ms/%.h
	$(call install-header,$<,$@)

$(INCDIR)/casacore/msfits/%.h: msfits/msfits/%.h
	$(call install-header,$<,$@)

$(INCDIR)/casacore/scimath/%.h: scimath/scimath/%.h
	$(call install-header,$<,$@)

$(INCDIR)/casacore/tables/%.h: tables/tables/%.h
	$(call install-header,$<,$@)
#--- --- --- --- --- --- --- --- --- --- --- --- --- --- --- ---
$(INCDIR)/casacore/casa/%.tcc: casa/casa/%.tcc
	$(call install-header,$<,$@)

$(INCDIR)/casacore/components/%.tcc: components/components/%.tcc
	$(call install-header,$<,$@)

$(INCDIR)/casacore/coordinates/%.tcc: coordinates/coordinates/%.tcc
	$(call install-header,$<,$@)

$(INCDIR)/casacore/fits/%.tcc: fits/fits/%.tcc
	$(call install-header,$<,$@)

$(INCDIR)/casacore/images/%.tcc: images/images/%.tcc
	$(call install-header,$<,$@)

$(INCDIR)/casacore/lattices/%.tcc: lattices/lattices/%.tcc
	$(call install-header,$<,$@)

$(INCDIR)/casacore/measures/%.tcc: measures/measures/%.tcc
	$(call install-header,$<,$@)

$(INCDIR)/casacore/ms/%.tcc: ms/ms/%.tcc
	$(call install-header,$<,$@)

$(INCDIR)/casacore/msfits/%.tcc: msfits/msfits/%.tcc
	$(call install-header,$<,$@)

$(INCDIR)/casacore/scimath/%.tcc: scimath/scimath/%.tcc
	$(call install-header,$<,$@)

$(INCDIR)/casacore/tables/%.tcc: tables/tables/%.tcc
	$(call install-header,$<,$@)
#--- --- --- --- --- --- --- --- --- --- --- --- --- --- --- ---

$(INCDIR)/casacore/mirlib/%.h: mirlib/%.h
	$(call install-header,$<,$@)

$(INCDIR)/casacore/%.tcc: %.tcc
	@if test ! -d $(dir $@); then mkdir -p $(dir $@); fi
	cp $< $@

%.o : %.cc
	@$(call make-depend,$<,$(subst .o,.dep,$@))
	$(C++) $(CXXFLAGS) -fPIC $(COREINC) $(INC) -c $< -o $@

%.o : %.c
	@$(call make-depend,$<,$(subst .o,.dep,$@))
	$(CC) $(CLAGS) -fPIC $(COREINC) $(INC) -c $< -o $@

%.o : %.f
	$(FC) $(FFLAGS) -fPIC $(COREINC) $(INC) -c $< -o $@

%.lcc : %.ll
	flex -P$(basename $(notdir $<)) -t $< > $@

%.ycc : %.yy
	bison -p $(basename $(notdir $<)) -o $@ $<

%.trd: %
	@$(call run-test,$<)

t% : t%.cc
	@if test -e "$(LIBDIR)/libcasacore.$(SO)"; then \
	    $(C++) $(CXXFLAGS) -I$(dir $<) $(COREINC) $(INC) -o $@ $< -L$(LIBDIR) -lcasacore -lcfitsio -lwcs -llapack -lblas -lcfitsio; \
	else \
	    $(C++) $(CXXFLAGS) -I$(dir $<) $(COREINC) $(INC) -o $@ $< -L$(LIBDIR) -lcasa_images -lcasa_msfits -lcasa_components -lcasa_coordinates -lcasa_ms -lcasa_measures -lcasa_measures_f -lcasa_scimath -lcasa_scimath_f -lcasa_fits -lcasa_lattices -lcasa_tables -lcasa_casa -lcfitsio -lcasa_mirlib -lwcs -llapack -lblas -lcfitsio; \
	fi

%.$(SO) : %.$(SOV)
	rm -f $@
	cd $(dir $<) && ln -fs $(notdir $<) $(notdir $@)

.SECONDEXPANSION:

%.o : $$(shell echo $$< | perl -pe "s|(.*?)/$(ARCH)/(\w+).o|\$$$$1/\$$$$2.cc|")
	@if test ! -d $(dir $@); then mkdir $(dir $@); fi
	@$(call make-depend,$<,$(subst .o,.dep,$@))
	$(C++) $(CXXFLAGS) -fPIC $(COREINC) $(INC) -c $< -o $@

%.o : $$(shell echo $$< | perl -pe "s|(.*?)/$(ARCH)/(\w+).o|\$$$$1/\$$$$2.c|")
	@if test ! -d $(dir $@); then mkdir $(dir $@); fi
	@$(call make-depend,$<,$(subst .o,.dep,$@))
	$(CC) $(CLAGS) -fPIC $(COREINC) $(INC) -c $< -o $@

%.o : $$(shell echo $$< | perl -pe "s|(.*?)/$(ARCH)/(\w+).o|\$$$$1/\$$$$2.f|")
	@if test ! -d $(dir $@); then mkdir $(dir $@); fi
	$(FC) $(FFLAGS) -fPIC $(COREINC) $(INC) -c $< -o $@

##
###  libcasacore
###
libcasacore: $(CORELNK_PATH)

$(CORELIB_PATH): $(LASTVERSION) $(CASALIB) $(COMPONENTSLIB) $(COORDINATESLIB) $(LATTICESLIB) $(IMAGESLIB) $(TABLESLIB) $(SCIMATHLIB) \
			$(SCIMATHFLIB) $(MEASURESLIB) $(MEASURESFLIB) $(MSLIB) $(FITSLIB) $(MSFITSLIB) $(MIRLIB) \
			$(CASAINC) $(COMPONENTSINC) $(COORDINATESINC) $(LATTICESINC) $(IMAGESINC) $(TABLESINC) $(SCIMATHINC) \
			$(MEASURESINC) $(MSINC) $(FITSINC) $(MSFITSINC) $(MIRINC)
	@if test ! -d $(dir $@); then mkdir -p $(dir $@); fi
	@$(call orphan-objects,$(ARCH),.)
	@$(call orphan-headers,$(INCDIR),.)
ifeq "$(os)" "darwin"
	$(C++) -dynamiclib -install_name $(instlib_path)$(notdir $@) -o $@ $(filter %.o,$^) -lcfitsio -lwcs -llapack -lblas $(FC_LIB) -lfftw3 -lfftw3f -lfftw3_threads -lfftw3f_threads
endif
ifeq "$(os)" "linux"
	$(C++) -shared -Wl,-soname,$(notdir $@) -o $@ $(filter %.o,$^) -lcfitsio -lwcs -llapack -lblas $(FC_LIB) -lfftw3 -lfftw3f -lfftw3_threads -lfftw3f_threads
endif

###
###  casa
###
casa: $(CASALNK_PATH)

$(CASALIB_PATH): $(LASTVERSION) $(CASALIB) $(CASAINC)
	@if test ! -d $(dir $@); then mkdir -p $(dir $@); fi
	@$(call orphan-objects,$(ARCH),casa)
	@$(call orphan-headers,$(INCDIR),casa)
ifeq "$(os)" "darwin"
	$(C++) -dynamiclib -install_name $(instlib_path)$(notdir $@) -o $@ $(filter %.o,$^)
endif
ifeq "$(os)" "linux"
	$(C++) -shared -Wl,-soname,$(notdir $@) -o $@ $(filter %.o,$^)
endif

###
###  components
###
components: coordinates tables $(COMPONENTSLNK_PATH)

$(COMPONENTSLIB_PATH): $(LASTVERSION) $(COORDINATESLIB_PATH) $(TABLESLIB_PATH) $(COMPONENTSLIB) $(COMPONENTSINC)
	@if test ! -d $(dir $@); then mkdir -p $(dir $@); fi
	@$(call orphan-objects,$(ARCH),components)
	@$(call orphan-headers,$(INCDIR),components)
ifeq "$(os)" "darwin"
	$(C++) -dynamiclib -install_name $(instlib_path)$(notdir $@) -o $@ $(filter %.o,$^) -L$(dir $@) -lcasa_coordinates -lcasa_measures -lcasa_measures_f -lcasa_scimath -lcasa_scimath_f -lcasa_fits -lcasa_tables -lcasa_casa -lcfitsio -lwcs -llapack -lblas $(FC_LIB)
endif
ifeq "$(os)" "linux"
	$(C++) -shared -Wl,-soname,$(notdir $@) -o $@ $(filter %.o,$^) -L$(dir $@) -lcasa_coordinates -lcasa_measures -lcasa_measures_f -lcasa_scimath -lcasa_scimath_f -lcasa_fits -lcasa_tables -lcasa_casa -lcfitsio -lwcs -llapack -lblas $(FC_LIB)
endif

###
###  coordinates
###
coordinates: measures fits $(COORDINATESLNK_PATH)

$(COORDINATESLIB_PATH): $(LASTVERSION) $(MEASURESLIB_PATH) $(FITSLIB_PATH) $(COORDINATESLIB) $(COORDINATESINC)
	@if test ! -d $(dir $@); then mkdir -p $(dir $@); fi
	@$(call orphan-objects,$(ARCH),coordinates)
	@$(call orphan-headers,$(INCDIR),coordinates)
ifeq "$(os)" "darwin"
	$(C++) -dynamiclib -install_name $(instlib_path)$(notdir $@) -o $@ $(filter %.o,$^) -L$(dir $@) -lcasa_measures -lcasa_measures_f -lcasa_scimath -lcasa_scimath_f -lcasa_fits -lcasa_tables -lcasa_casa -lcfitsio -lwcs -llapack -lblas $(FC_LIB)
endif
ifeq "$(os)" "linux"
	$(C++) -shared -Wl,-soname,$(notdir $@) -o $@ $(filter %.o,$^) -L$(dir $@) -lcasa_measures -lcasa_measures_f -lcasa_scimath -lcasa_scimath_f -lcasa_fits -lcasa_tables -lcasa_casa -lcfitsio -lwcs -llapack -lblas $(FC_LIB)
endif

###
###  lattices
###
lattices: scimath $(LATTICESLNK_PATH) 

$(LATTICESLIB_PATH): $(LASTVERSION) $(SCIMATHFLIB_PATH) $(LATTICESLIB) $(LATTICESINC)
	@if test ! -d $(dir $@); then mkdir -p $(dir $@); fi
	@$(call orphan-objects,$(ARCH),lattices)
	@$(call orphan-headers,$(INCDIR),lattices)
ifeq "$(os)" "darwin"
	$(C++) -dynamiclib -install_name $(instlib_path)$(notdir $@) -o $@ $(filter %.o,$^) -L$(dir $@) -lcasa_tables -lcasa_scimath -lcasa_scimath_f -lcasa_casa -llapack -lblas $(FC_LIB)
endif
ifeq "$(os)" "linux"
	$(C++) -shared -Wl,-soname,$(notdir $@) -o $@ $(filter %.o,$^) -L$(dir $@) -lcasa_tables -lcasa_scimath -lcasa_scimath_f -lcasa_casa -llapack -lblas $(FC_LIB)
endif

###
###  images
###
images: components lattices fits mirlib $(IMAGESLNK_PATH)

$(IMAGESLIB_PATH): $(LASTVERSION) $(COMPONENTSLIB_PATH) $(LATTICESLIB_PATH) $(FITSLIB_PATH) $(MIRLIB_PATH) $(IMAGESLIB) $(IMAGESINC)
	@if test ! -d $(dir $@); then mkdir -p $(dir $@); fi
	@$(call orphan-objects,$(ARCH),images)
	@$(call orphan-headers,$(INCDIR),images)
ifeq "$(os)" "darwin"
	$(C++) -dynamiclib -install_name $(instlib_path)$(notdir $@) -o $@ $(filter %.o,$^) -L$(dir $@) -lcasa_components -lcasa_coordinates -lcasa_measures -lcasa_measures_f -lcasa_scimath -lcasa_scimath_f -lcasa_fits -lcasa_lattices -lcasa_tables -lcasa_casa -lcfitsio -lcasa_mirlib -lwcs -llapack -lblas -lcfitsio $(FC_LIB)
endif
ifeq "$(os)" "linux"
	$(C++) -shared -Wl,-soname,$(notdir $@) -o $@ $(filter %.o,$^) -L$(dir $@) -lcasa_components -lcasa_coordinates -lcasa_measures -lcasa_measures_f -lcasa_scimath -lcasa_scimath_f -lcasa_fits -lcasa_lattices -lcasa_tables -lcasa_casa -lcfitsio -lcasa_mirlib -lwcs -llapack -lblas -lcfitsio $(FC_LIB)
endif

###
###  tables
###
tables: casa $(TABLESLNK_PATH)

$(TABLESLIB_PATH): $(LASTVERSION) $(CASALIB_PATH) $(TABLESLIB) $(TABLESINC)
	@if test ! -d $(dir $@); then mkdir -p $(dir $@); fi
	@$(call orphan-objects,$(ARCH),tables)
	@$(call orphan-headers,$(INCDIR),tables)
ifeq "$(os)" "darwin"
	$(C++) -dynamiclib -install_name $(instlib_path)$(notdir $@) -o $@ $(filter %.o,$^) -L$(dir $@) -lcasa_casa
endif
ifeq "$(os)" "linux"
	$(C++) -shared -Wl,-soname,$(notdir $@) -o $@ $(filter %.o,$^) -L$(dir $@) -lcasa_casa
endif

###
###  scimath
###
scimath: casa $(SCIMATHLNK_PATH)

$(SCIMATHLIB_PATH): $(LASTVERSION) $(CASALNK_PATH) $(SCIMATHLIB) $(SCIMATHFLNK_PATH) $(SCIMATHINC)
	@if test ! -d $(dir $@); then mkdir -p $(dir $@); fi
	@$(call orphan-objects,$(ARCH),scimath)
	@$(call orphan-headers,$(INCDIR),scimath)
ifeq "$(os)" "darwin"
	$(C++) -dynamiclib -install_name $(instlib_path)$(notdir $@) -o $@ $(filter %.o,$^) -L$(dir $@) -lcasa_scimath_f -lcasa_casa -llapack -lblas -lfftw3 -lfftw3f -lfftw3_threads -lfftw3f_threads $(FC_LIB)
endif
ifeq "$(os)" "linux"
	$(C++) -shared -Wl,-soname,$(notdir $@) -o $@ $(filter %.o,$^) -L$(dir $@) -lcasa_scimath_f -lcasa_casa -llapack -lblas -lfftw3 -lfftw3f -lfftw3_threads -lfftw3f_threads $(FC_LIB)
endif

$(SCIMATHFLIB_PATH): $(SCIMATHFLIB)
	@if test ! -d $(dir $@); then mkdir -p $(dir $@); fi
	@$(call orphan-objects,$(ARCH),scimath)
ifeq "$(os)" "darwin"
	$(C++) -dynamiclib -install_name $(instlib_path)$(notdir $@) -o $@ $(filter %.o,$^) -L$(dir $@) -lcasa_casa -llapack -lblas -lfftw3 -lfftw3f -lfftw3_threads -lfftw3f_threads $(FC_LIB)
endif
ifeq "$(os)" "linux"
	$(C++) -shared -Wl,-soname,$(notdir $@) -o $@ $(filter %.o,$^) -L$(dir $@) -lcasa_casa -llapack -lblas -lfftw3 -lfftw3f -lfftw3_threads -lfftw3f_threads $(FC_LIB)
endif


###
###  measures
###
measures: scimath tables $(MEASURESLNK_PATH)

$(MEASURESLIB_PATH):  $(LASTVERSION) $(SCIMATHFLNK_PATH) $(TABLESLNK_PATH) $(MEASURESLIB) $(MEASURESFLNK_PATH) $(MEASURESINC)
	@if test ! -d $(dir $@); then mkdir -p $(dir $@); fi
	@$(call orphan-objects,$(ARCH),measures)
	@$(call orphan-headers,$(INCDIR),measures)
ifeq "$(os)" "darwin"
	$(C++) -dynamiclib -install_name $(instlib_path)$(notdir $@) -o $@ $(filter %.o,$^) -L$(dir $@) -lcasa_measures_f -lcasa_scimath -lcasa_scimath_f -lcasa_tables -lcasa_casa $(FC_LIB)
endif
ifeq "$(os)" "linux"
	$(C++) -shared -Wl,-soname,$(notdir $@) -o $@ $(filter %.o,$^) -L$(dir $@) -lcasa_measures_f -lcasa_scimath -lcasa_scimath_f -lcasa_tables -lcasa_casa $(FC_LIB)
endif

$(MEASURESFLIB_PATH):  $(MEASURESFLIB)
	@if test ! -d $(dir $@); then mkdir -p $(dir $@); fi
	@$(call orphan-objects,$(ARCH),measures)
ifeq "$(os)" "darwin"
	$(C++) -dynamiclib -install_name $(instlib_path)$(notdir $@) -o $@ $(filter %.o,$^) -L$(dir $@) -lcasa_scimath -lcasa_scimath_f -lcasa_tables -lcasa_casa $(FC_LIB)
endif
ifeq "$(os)" "linux"
	$(C++) -shared -Wl,-soname,$(notdir $@) -o $@ $(filter %.o,$^) -L$(dir $@) -lcasa_scimath -lcasa_scimath_f -lcasa_tables -lcasa_casa $(FC_LIB)
endif

###
###  ms
###
ms: measures tables $(MSLNK_PATH)

$(MSLIB_PATH): $(LASTVERSION) $(MEASURESLNK_PATH) $(TABLESLNK_PATH) $(MSLIB) $(MSINC)
	@if test ! -d $(dir $@); then mkdir -p $(dir $@); fi
	@$(call orphan-objects,$(ARCH),ms)
	@$(call orphan-headers,$(INCDIR),ms)
ifeq "$(os)" "darwin"
	$(C++) -dynamiclib -install_name $(instlib_path)$(notdir $@) -o $@ $(filter %.o,$^) -L$(dir $@) -lcasa_measures -lcasa_measures_f -lcasa_tables -lcasa_scimath -lcasa_scimath_f -lcasa_casa -llapack -lblas $(FC_LIB)
endif
ifeq "$(os)" "linux"
	$(C++) -shared -Wl,-soname,$(notdir $@) -o $@ $(filter %.o,$^) -L$(dir $@) -lcasa_measures -lcasa_measures_f -lcasa_tables -lcasa_scimath -lcasa_scimath_f -lcasa_casa -llapack -lblas $(FC_LIB)
endif


###
###  fits
###
fits: scimath tables $(FITSLNK_PATH)

$(FITSLIB_PATH): $(LASTVERSION) $(SCIMATHLNK_PATH) $(TABLESLNK_PATH) $(FITSLIB) $(FITSINC)
	@if test ! -d $(dir $@); then mkdir -p $(dir $@); fi
	@$(call orphan-objects,$(ARCH),fits)
	@$(call orphan-headers,$(INCDIR),fits)
ifeq "$(os)" "darwin"
	$(C++) -dynamiclib -install_name $(instlib_path)$(notdir $@) -o $@ $(filter %.o,$^) -L$(dir $@) -lcasa_measures -lcasa_measures_f -lcasa_tables -lcasa_casa -lcfitsio $(FC_LIB)
endif
ifeq "$(os)" "linux"
	$(C++) -shared -Wl,-soname,$(notdir $@) -o $@ $(filter %.o,$^) -L$(dir $@) -lcasa_measures -lcasa_measures_f -lcasa_tables -lcasa_casa -lcfitsio $(FC_LIB)
endif

###
###  msfits
###
msfits: ms fits $(MSFITSLNK_PATH)

$(MSFITSLIB_PATH): $(LASTVERSION) $(MSLNK_PATH) $(FITSLNK_PATH) $(MSFITSLIB) $(MSFITSINC)
	@if test ! -d $(dir $@); then mkdir -p $(dir $@); fi
	@$(call orphan-objects,$(ARCH),msfits)
	@$(call orphan-headers,$(INCDIR),msfits)
ifeq "$(os)" "darwin"
	$(C++) -dynamiclib -install_name $(instlib_path)$(notdir $@) -o $@ $(filter %.o,$^) -L$(dir $@) -lcasa_ms -lcasa_measures -lcasa_measures_f -lcasa_tables -lcasa_fits -lcasa_casa -lcfitsio -llapack -lblas $(FC_LIB)
endif
ifeq "$(os)" "linux"
	$(C++) -shared -Wl,-soname,$(notdir $@) -o $@ $(filter %.o,$^) -L$(dir $@) -lcasa_ms -lcasa_measures -lcasa_measures_f -lcasa_tables -lcasa_fits -lcasa_casa -lcfitsio -llapack -lblas $(FC_LIB)
endif


###
###  mirlib
###
mirlib: $(MIRLNK_PATH) 

$(MIRLIB_PATH): $(LASTVERSION) $(MIRLIB) $(MIRINC)
	@if test ! -d $(dir $@); then mkdir -p $(dir $@); fi
	@$(call orphan-objects,$(ARCH),mirlib)
	@$(call orphan-headers,$(INCDIR),mirlib)
ifeq "$(os)" "darwin"
	$(C++) -dynamiclib -install_name $(instlib_path)$(notdir $@) -o $@ $(filter %.o,$^)
endif
ifeq "$(os)" "linux"
	$(C++) -shared -Wl,-soname,$(notdir $@) -o $@ $(filter %.o,$^)
endif


cleandep:
	@$(call orphan-deps,.)

cleandeps:
	@$(call orphan-deps,.)

cleancasa:
	@rm -f $(CASALIB_PATH)
	@rm -f $(subst .$(VERSION),,$(CASALIB_PATH))

cleancore:
	@rm -f $(CORELIB_PATH)
	@rm -f $(subst .$(VERSION),,$(CORELIB_PATH))

cleancomponents:
	@rm -f $(COMPONENTSLIB_PATH)
	@rm -f $(subst .$(VERSION),,$(COMPONENTSLIB_PATH))

cleancoordinates:
	@rm -f $(COORDINATESLIB_PATH)
	@rm -f $(subst .$(VERSION),,$(COORDINATESLIB_PATH))

cleanlattices:
	@rm -f $(LATTICESLIB_PATH)
	@rm -f $(subst .$(VERSION),,$(LATTICESLIB_PATH))

cleanimages:
	@rm -f $(IMAGESLIB_PATH)
	@rm -f $(subst .$(VERSION),,$(IMAGESLIB_PATH))

cleantables:
	@rm -f $(TABLESLIB_PATH)
	@rm -f $(subst .$(VERSION),,$(TABLESLIB_PATH))

cleanscimath:
	@rm -f $(SCIMATHLIB_PATH)
	@rm -f $(subst .$(VERSION),,$(SCIMATHLIB_PATH))
	@rm -f $(SCIMATHFLIB_PATH)
	@rm -f $(subst .$(VERSION),,$(SCIMATHFLIB_PATH))

cleanmeasures:
	@rm -f $(MEASURESLIB_PATH)
	@rm -f $(subst .$(VERSION),,$(MEASURESLIB_PATH))
	@rm -f $(MEASURESFLIB_PATH)
	@rm -f $(subst .$(VERSION),,$(MEASURESFLIB_PATH))

cleanfits:
	@rm -f $(FITSLIB_PATH)
	@rm -f $(subst .$(VERSION),,$(FITSLIB_PATH))

cleanms:
	@rm -f $(MSLIB_PATH)
	@rm -f $(subst .$(VERSION),,$(MSLIB_PATH))

cleanmsfits:
	@rm -f $(MSFITSLIB_PATH)
	@rm -f $(subst .$(VERSION),,$(MSFITSLIB_PATH))

cleanmir:
	@rm -f $(MIRLIB_PATH)
	@rm -f $(subst .$(VERSION),,$(MIRLIB_PATH))

cleaninc:
	@rm -rf $(INCDIR)/casacore

ifneq "$(ARCH)" ""
clean: cleanmir cleanmsfits cleanms cleanfits cleanmeasures \
	cleanscimath cleantables cleanimages cleanlattices \
	cleancoordinates cleancomponents cleancore cleancasa \
	cleaninc
	@rm -f $(shell find . -type f -name '*.lcc') $(shell find . -type f -name '*.ycc')
	@rm -rf $(shell find . -type f -name '*.o' | grep $(ARCH) | perl -pe "s|/[^/]+\\.o$$||" | sort -u)
	@echo finished cleaning up...
else
clean: cleanmir cleanmsfits cleanms cleanfits cleanmeasures \
	cleanscimath cleantables cleanimages cleanlattices \
	cleancoordinates cleancomponents cleancore cleancasa \
	cleaninc
	@rm -f `find . -type f -name '*.lcc'` `find . -type f -name '*.ycc'`
	@rm -rf $(shell find . -type f -name '*.o')
	@echo finished cleaning up...
endif

$(LASTVERSION): VERSION
	@if test -e $(LASTVERSION); then \
		version=$(shell head -1 $(LASTVERSION) 2> /dev/null | perl -pe "s|^(\S+).*|\$$1|"); \
		echo "removing old libraries... new VERSION file."; \
	else \
		version=$(shell head -1 VERSION | perl -pe "s|^(\S+).*|\$$1|"); \
		echo "removing libraries... for relink."; \
	fi; \
	for i in `ls -1 $(LIBDIR)/lib*$$version* 2> /dev/null || exit 0` \
		 `(ls -1 $(LIBDIR)/lib*$$version* 2> /dev/null | sed "s|\.$$version||") || exit 0`; do \
		rm $$i; \
	done
	@mkdir -p $(dir $(LASTVERSION))
	@cp VERSION $(LASTVERSION)

###
### handle building & running unit tests
###
.precious: $(TCASA:%.cc=%)
ifeq "$(ONELIB)" "0"
$(TCASA:%.cc=%): $(CASALNK_PATH)
else
$(TCASA:%.cc=%): $(CORELNK_PATH)
endif
test-casa: $(TCASA:%.cc=%.trd)

.precious: $(TCOMPONENTS:%.cc=%)
ifeq "$(ONELIB)" "0"
$(TCOMPONENTS:%.cc=%): $(COMPONENTSLNK_PATH)
else
$(TCOMPONENTS:%.cc=%): $(CORELNK_PATH)
endif
test-components: $(TCOMPONENTS:%.cc=%.trd)

.precious: $(TCOORDINATES:%.cc=%)
ifeq "$(ONELIB)" "0"
$(TCOORDINATES:%.cc=%): $(COORDINATESLNK_PATH)
else
$(TCOORDINATES:%.cc=%): $(CORELNK_PATH)
endif
test-coordinates: $(TCOORDINATES:%.cc=%.trd)

.precious: $(TLATTICES:%.cc=%)
ifeq "$(ONELIB)" "0"
$(TLATTICES:%.cc=%): $(LATTICESLNK_PATH)
else
$(TLATTICES:%.cc=%): $(CORELNK_PATH)
endif
test-lattices: $(TLATTICES:%.cc=%.trd)

.precious: $(TIMAGES:%.cc=%)
ifeq "$(ONELIB)" "0"
$(TIMAGES:%.cc=%): $(IMAGESLNK_PATH)
else
$(TIMAGES:%.cc=%): $(CORELNK_PATH)
endif
test-images: $(TIMAGES:%.cc=%.trd)

.precious: $(TTABLES:%.cc=%)
ifeq "$(ONELIB)" "0"
$(TTABLES:%.cc=%): $(TABLESLNK_PATH)
else
$(TTABLES:%.cc=%): $(CORELNK_PATH)
endif
test-tables: $(TTABLES:%.cc=%.trd)

.precious: $(TSCIMATH:%.cc=%)
ifeq "$(ONELIB)" "0"
$(TSCIMATH:%.cc=%): $(SCIMATHLNK_PATH)
else
$(TSCIMATH:%.cc=%): $(CORELNK_PATH)
endif
test-scimath: $(TSCIMATH:%.cc=%.trd)

.precious: $(TMEASURES:%.cc=%)
ifeq "$(ONELIB)" "0"
$(TMEASURES:%.cc=%): $(MEASURESLNK_PATH)
else
$(TMEASURES:%.cc=%): $(CORELNK_PATH)
endif
test-measures: $(TMEASURES:%.cc=%.trd)

.precious: $(TFITS:%.cc=%)
ifeq "$(ONELIB)" "0"
$(TFITS:%.cc=%): $(FITSLNK_PATH)
else
$(TFITS:%.cc=%): $(CORELNK_PATH)
endif
test-fits: $(TFITS:%.cc=%.trd)

.precious: $(TMS:%.cc=%)
ifeq "$(ONELIB)" "0"
$(TMS:%.cc=%): $(MSLNK_PATH)
else
$(TMS:%.cc=%): $(CORELNK_PATH)
endif
test-ms: $(TMS:%.cc=%.trd)

.precious: $(TMSFITS:%.cc=%)
ifeq "$(ONELIB)" "0"
$(TMSFITS:%.cc=%): $(MSFITSLNK_PATH)
else
$(TMSFITS:%.cc=%): $(CORELNK_PATH)
endif
test-msfits: $(TMSFITS:%.cc=%.trd)

test:  $(TCASA:%.cc=%.trd) $(TCOMPONENTS:%.cc=%.trd) $(TCOORDINATES:%.cc=%.trd) $(TLATTICES:%.cc=%.trd) \
	$(TIMAGES:%.cc=%.trd) $(TTABLES:%.cc=%.trd) $(TSCIMATH:%.cc=%.trd) $(TMEASURES:%.cc=%.trd) \
	$(TFITS:%.cc=%.trd) $(TMS:%.cc=%.trd) $(TMSFITS:%.cc=%.trd)

ifeq "$(DEP)" "1"
-include $(CASADEP) $(COMPONENTSDEP) $(COORDINATESDEP) $(LATTICESDEP) $(IMAGESDEP) \
	$(SCIMATHDEP) $(MEASURESDEP) $(FITSDEP) $(MSDEP) $(MSFITSDEP) $(MIRDEP)
endif
