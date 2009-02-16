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

##
## installation builds:
##
## gmake DESTDIR=`echo $CASAPATH | sed 's|\([^ ]*\)[ ][ ]*.*$|\1|'` ARCH=`echo $CASAPATH | sed 's|[^ ]*[ ][ ]*\([^ ]*\).*$|\1|'` setup
## gmake DESTDIR=`echo $CASAPATH | sed 's|\([^ ]*\)[ ][ ]*.*$|\1|'` ARCH=`echo $CASAPATH | sed 's|[^ ]*[ ][ ]*\([^ ]*\).*$|\1|'`
##

-include makedefs

C++ := c++
CXXFLAGS := -O2

CC  := gcc
CFLAGS := -O2

FC  := 
FFLAGS := -O2

DESTDIR := 
ARCH := 

INSTALLDIR := $(DESTDIR)

VERSION:=0.3.0

LIBDIR := $(INSTALLDIR)/lib

OS := $(shell uname | tr 'A-Z' 'a-z')
ifeq "$(OS)" "darwin"
SO := $(VERSION).dylib
#INC :=
endif
ifeq "$(OS)" "linux"
SO := so.$(VERSION)
INC += -I/usr/include/cfitsio

arch := $(shell uname -p)
ifeq "$(arch)" "x86_64"
FFLAGS += -m64
CFLAGS += -m64
CXXFLAGS += -m64
endif
endif

##
## setup DESTDIR
##
ifeq "$(DESTDIR)" ""
DESTDIR := $(CASAARCH)
ifeq "$(DESTDIR)" ""
DESTDIR := $(shell pwd)/build
else
INC += -I$(CASAARCH)/include
endif
endif

##
## setup fortran compiler
##
ifeq "$(FC)" ""
FC  := $(shell type gfortran 2> /dev/null | perl -pe 's@^\S+\s+is\s+@@')
ifneq "$(FC)" ""
FC_LIB := -lgfortran
endif
endif

ifeq "$(FC)" ""
FC  := $(shell type g77 2> /dev/null | perl -pe 's@^\S+\s+is\s+@@')
endif

ifeq "$(FC)" ""
all: 
	@echo "error could not find fortran compiler; please set the 'FC', GNUmakefile variable"
else
all: images msfits
endif

CASACC := $(shell find casa -type f -name '*.cc' | egrep -v '/test/|/apps/')
CASAOBJ := $(CASACC:%.cc=%.o)
ifneq "$(ARCH)" ""
CASALIB := $(shell echo $(CASAOBJ) | perl -pe "s|(\w+\\.o)|$(ARCH)/\$$1|g")
CASALIB_PATH := $(DESTDIR)/$(ARCH)/lib/libcasa_casa.$(SO)
CORELIB_PATH := $(DESTDIR)/$(ARCH)/lib/libcasacore.$(SO)
else
CASALIB := $(CASAOBJ)
CASALIB_PATH := $(DESTDIR)/lib/libcasa_casa.$(SO)
CORELIB_PATH := $(DESTDIR)/lib/libcasacore.$(SO)
endif
### components   -> tables      -> casa
###              -> coordinates -> (cfitsio)
###                             -> (wcs)
###                             -> measures  -> scimath   -> casa
###                                                       -> (lapack)
###                                                       -> (blas)
COMPONENTSCC := $(shell find components -type f -name '*.cc' | egrep -v '/test/|/apps/')
COMPONENTSOBJ := $(COMPONENTSCC:%.cc=%.o)
ifneq "$(ARCH)" ""
COMPONENTSLIB := $(shell echo $(COMPONENTSOBJ) | perl -pe "s|(\w+\\.o)|$(ARCH)/\$$1|g")
COMPONENTSLIB_PATH := $(DESTDIR)/$(ARCH)/lib/libcasa_components.$(SO)
else
COMPONENTSLIB := $(COMPONENTSOBJ)
COMPONENTSLIB_PATH := $(DESTDIR)/lib/libcasa_components.$(SO)
endif
###  coordinates -> measures  -> scimath -> casa
###                                      -> (lapack)
###                                      -> (blas)
###              -> fits      -> tables  -> casa
###                           -> (cfitsio)
###              -> (wcslib)
COORDINATESCC := $(shell find coordinates -type f -name '*.cc' | egrep -v '/test/|/apps/')
COORDINATESOBJ := $(COORDINATESCC:%.cc=%.o)
ifneq "$(ARCH)" ""
COORDINATESLIB := $(shell echo $(COORDINATESOBJ) | perl -pe "s|(\w+\\.o)|$(ARCH)/\$$1|g")
COORDINATESLIB_PATH := $(DESTDIR)/$(ARCH)/lib/libcasa_coordinates.$(SO)
else
COORDINATESLIB := $(COORDINATESOBJ)
COORDINATESLIB_PATH := $(DESTDIR)/lib/libcasa_coordinates.$(SO)
endif
###  lattices -> tables  -> casa
###           -> scimath -> casa
###                      -> (lapack)
###                      -> (blas)
LATTICESCC := $(shell find lattices -type f -name '*.cc' | egrep -v '/test/|/apps/')
LATTICESOBJ := $(LATTICESCC:%.cc=%.o)
ifneq "$(ARCH)" ""
LATTICESLIB := $(shell echo $(LATTICESOBJ) | perl -pe "s|(\w+\\.o)|$(ARCH)/\$$1|g")
LATTICESLIB_PATH := $(DESTDIR)/$(ARCH)/lib/libcasa_lattices.$(SO)
else
LATTICESLIB := $(LATTICESOBJ)
LATTICESLIB_PATH := $(DESTDIR)/lib/libcasa_lattices.$(SO)
endif
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
ifneq "$(ARCH)" ""
IMAGESLIB := images/images/Images/ImageExprGram.lcc images/images/Images/ImageExprGram.ycc \
		$(shell echo $(IMAGESOBJ) | perl -pe "s|(\w+\\.o)|$(ARCH)/\$$1|g")
IMAGESLIB_PATH := $(DESTDIR)/$(ARCH)/lib/libcasa_images.$(SO)
else
IMAGESLIB := images/images/Images/ImageExprGram.lcc images/images/Images/ImageExprGram.ycc \
		$(IMAGESOBJ)
IMAGESLIB_PATH := $(DESTDIR)/lib/libcasa_images.$(SO)
endif
###  tables -> casa
TABLESCC := $(shell find tables -type f -name '*.cc' | egrep -v '/test/|/apps/')
TABLESOBJ := $(TABLESCC:%.cc=%.o)
ifneq "$(ARCH)" ""
TABLESLIB := tables/tables/Tables/RecordGram.lcc tables/tables/Tables/RecordGram.ycc \
		tables/tables/Tables/TableGram.lcc tables/tables/Tables/TableGram.ycc \
		$(shell echo $(TABLESOBJ) | perl -pe "s|(\w+\\.o)|$(ARCH)/\$$1|g")
TABLESLIB_PATH := $(DESTDIR)/$(ARCH)/lib/libcasa_tables.$(SO)
else
TABLESLIB := tables/tables/Tables/RecordGram.lcc tables/tables/Tables/RecordGram.ycc \
		tables/tables/Tables/TableGram.lcc tables/tables/Tables/TableGram.ycc \
		$(TABLESOBJ)
TABLESLIB_PATH := $(DESTDIR)/lib/libcasa_tables.$(SO)
endif
###  scimath -> casa
###          -> (lapack)
###          -> (blas)
SCIMATHCC := $(shell find scimath -type f -name '*.cc' | egrep -v '/test/|/apps/')
SCIMATHF := $(shell find scimath -type f -name '*.f' | egrep -v '/test/|/apps/')
SCIMATHOBJ := $(SCIMATHCC:%.cc=%.o)
SCIMATHFOBJ := $(SCIMATHF:%.f=%.o)
ifneq "$(ARCH)" ""
SCIMATHLIB := $(shell echo $(SCIMATHOBJ) | perl -pe "s|(\w+\\.o)|$(ARCH)/\$$1|g")
SCIMATHLIB_PATH := $(DESTDIR)/$(ARCH)/lib/libcasa_scimath.$(SO)
SCIMATHFLIB := $(shell echo $(SCIMATHFOBJ) | perl -pe "s|(\w+\\.o)|$(ARCH)/\$$1|g")
SCIMATHFLIB_PATH := $(DESTDIR)/$(ARCH)/lib/libcasa_scimath_f.$(SO)
else
SCIMATHLIB := $(SCIMATHOBJ)
SCIMATHLIB_PATH := $(DESTDIR)/lib/libcasa_scimath.$(SO)
SCIMATHFLIB := $(SCIMATHFOBJ)
SCIMATHFLIB_PATH := $(DESTDIR)/lib/libcasa_scimath_f.$(SO)
endif
###  measures -> scimath -> casa
###           -> tables  -> casa
MEASURESCC := $(shell find measures -type f -name '*.cc' | egrep -v '/test/|/apps/')
MEASURESF := $(shell find measures -type f -name '*.f' | egrep -v '/test/|/apps/')
MEASURESOBJ := $(MEASURESCC:%.cc=%.o)
MEASURESFOBJ := $(MEASURESF:%.f=%.o)
ifneq "$(ARCH)" ""
MEASURESLIB := $(shell echo $(MEASURESOBJ) | perl -pe "s|(\w+\\.o)|$(ARCH)/\$$1|g")
MEASURESLIB_PATH := $(DESTDIR)/$(ARCH)/lib/libcasa_measures.$(SO)
MEASURESFLIB := $(shell echo $(MEASURESFOBJ) | perl -pe "s|(\w+\\.o)|$(ARCH)/\$$1|g")
MEASURESFLIB_PATH := $(DESTDIR)/$(ARCH)/lib/libcasa_measures_f.$(SO)
else
MEASURESLIB := $(MEASURESOBJ)
MEASURESLIB_PATH := $(DESTDIR)/lib/libcasa_measures.$(SO)
MEASURESFLIB := $(MEASURESFOBJ)
MEASURESFLIB_PATH := $(DESTDIR)/lib/libcasa_measures_f.$(SO)
endif
###  fits -> measures  -> scimath -> casa
###       -> tables    -> casa
###       -> (cfitsio)
FITSCC := $(shell find fits -type f -name '*.cc' | egrep -v '/test/|/apps/')
FITSOBJ := $(FITSCC:%.cc=%.o)
ifneq "$(ARCH)" ""
FITSLIB := $(shell echo $(FITSOBJ) | perl -pe "s|(\w+\\.o)|$(ARCH)/\$$1|g")
FITSLIB_PATH := $(DESTDIR)/$(ARCH)/lib/libcasa_fits.$(SO)
else
FITSLIB := $(FITSOBJ)
FITSLIB_PATH := $(DESTDIR)/lib/libcasa_fits.$(SO)
endif
###  ms -> measures  -> scimath -> casa
###                             -> (lapack)
###                             -> (blas)
MSCC := $(shell find ms -type f -name '*.cc' | egrep -v '/test/|/apps/')
MSOBJ := $(MSCC:%.cc=%.o)
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
MSLIB_PATH := $(DESTDIR)/$(ARCH)/lib/libcasa_ms.$(SO)
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
MSLIB_PATH := $(DESTDIR)/lib/libcasa_ms.$(SO)
endif
###  msfits -> fits -> (cfitsio)
###                 -> measures  -> tables  -> casa
###                              -> scimath -> casa
###         -> ms   -> measures  -> scimath -> casa
###                              -> (lapack)
###                              -> (blas)
MSFITSCC := $(shell find msfits -type f -name '*.cc' | egrep -v '/test/|/apps/')
MSFITSOBJ := $(MSFITSCC:%.cc=%.o)
ifneq "$(ARCH)" ""
MSFITSLIB := $(shell echo $(MSFITSOBJ) | perl -pe "s|(\w+\\.o)|$(ARCH)/\$$1|g")
MSFITSLIB_PATH := $(DESTDIR)/$(ARCH)/lib/libcasa_msfits.$(SO)
else
MSFITSLIB := $(MSFITSOBJ)
MSFITSLIB_PATH := $(DESTDIR)/lib/libcasa_msfits.$(SO)
endif
###
MIRCC := $(shell find mirlib -type f -name '*.c' | egrep -v '/test/|/apps/')
MIROBJ := $(MIRCC:%.c=%.o)
ifneq "$(ARCH)" ""
MIRLIB := $(shell echo $(MIROBJ) | perl -pe "s|(\w+\\.o)|$(ARCH)/\$$1|g")
MIRLIB_PATH := $(DESTDIR)/$(ARCH)/lib/libcasa_mirlib.$(SO)
else
MIRLIB := $(MIROBJ)
MIRLIB_PATH := $(DESTDIR)/lib/libcasa_mirlib.$(SO)
endif

%.o : %.cc
	$(C++) $(CXXFLAGS) -fPIC -I$(DESTDIR)/include/casacore $(INC) -c $< -o $@

%.o : %.c
	$(CC) $(CLAGS) -fPIC -I$(DESTDIR)/include/casacore $(INC) -c $< -o $@

%.o : %.f
	$(FC) $(FFLAGS) -fPIC -I$(DESTDIR)/include/casacore $(INC) -c $< -o $@

%.lcc : %.ll
	flex -P$(basename $(notdir $<)) -t $< > $@

%.ycc : %.yy
	bison -p $(basename $(notdir $<)) -o $@ $<

t% : t%.cc
	$(C++) -I$(DESTDIR)/include/casacore $(INC) -o $@ $< -L$(DESTDIR)/lib -lcasa_casa

.SECONDEXPANSION:

%.o : $$(shell echo $$< | perl -pe "s|(.*?)/$(ARCH)/(\w+).o|\$$$$1/\$$$$2.cc|")
	@if test ! -d $(dir $@); then mkdir $(dir $@); fi
	$(C++) $(CXXFLAGS) -fPIC -I$(DESTDIR)/include/casacore $(INC) -c $< -o $@

%.o : $$(shell echo $$< | perl -pe "s|(.*?)/$(ARCH)/(\w+).o|\$$$$1/\$$$$2.c|")
	@if test ! -d $(dir $@); then mkdir $(dir $@); fi
	$(CC) $(CLAGS) -fPIC -I$(DESTDIR)/include/casacore $(INC) -c $< -o $@

%.o : $$(shell echo $$< | perl -pe "s|(.*?)/$(ARCH)/(\w+).o|\$$$$1/\$$$$2.f|")
	@if test ! -d $(dir $@); then mkdir $(dir $@); fi
	$(FC) $(FFLAGS) -fPIC -I$(DESTDIR)/include/casacore $(INC) -c $< -o $@

###
###  libcasacore
###
libcasacore: $(CORELIB_PATH)

$(CORELIB_PATH): $(CASALIB) $(COMPONENTSLIB) $(COORDINATESLIB) $(LATTICESLIB) \
					$(IMAGESLIB) $(TABLESLIB) $(SCIMATHLIB) $(SCIMATHFLIB) \
					$(MEASURESLIB) $(MEASURESFLIB) $(FITSLIB) $(MSLIB) \
					$(MSFITSLIB) $(MIRLIB)
ifeq "$(OS)" "darwin"
	$(C++) -dynamiclib -install_name $(notdir $@) -o $@ $(filter %.o,$^) -lcfitsio -lwcs -llapack -lblas $(FC_LIB)
endif
ifeq "$(OS)" "linux"
	$(C++) -shared -Wl,-soname,$(notdir $@) -o $@ $(filter %.o,$^) -lcfitsio -lwcs -llapack -lblas $(FC_LIB)
endif
	cd $(dir $@) && ln -fs $(notdir $@) $(subst .$(VERSION),,$(notdir $@))

###
###  casa
###
casa: $(CASALIB_PATH)

$(CASALIB_PATH): $(CASALIB)
ifeq "$(OS)" "darwin"
	$(C++) -dynamiclib -install_name $(notdir $@) -o $@ $(filter %.o,$^)
endif
ifeq "$(OS)" "linux"
	$(C++) -shared -Wl,-soname,$(notdir $@) -o $@ $(filter %.o,$^)
endif
	cd $(dir $@) && ln -fs $(notdir $@) $(subst .$(VERSION),,$(notdir $@))

###
###  components
###
components: coordinates tables $(COMPONENTSLIB_PATH)

$(COMPONENTSLIB_PATH): $(COMPONENTSLIB)
ifeq "$(OS)" "darwin"
	$(C++) -dynamiclib -install_name $(notdir $@) -o $@ $(filter %.o,$^) -L$(dir $@) -lcasa_coordinates -lcasa_measures -lcasa_measures_f -lcasa_scimath -lcasa_scimath_f -lcasa_fits -lcasa_tables -lcasa_casa -lcfitsio -lwcs -llapack -lblas $(FC_LIB)
endif
ifeq "$(OS)" "linux"
	$(C++) -shared -Wl,-soname,$(notdir $@) -o $@ $(filter %.o,$^) -L$(dir $@) -lcasa_coordinates -lcasa_measures -lcasa_measures_f -lcasa_scimath -lcasa_scimath_f -lcasa_fits -lcasa_tables -lcasa_casa -lcfitsio -lwcs -llapack -lblas $(FC_LIB)
endif
	cd $(dir $@) && ln -fs $(notdir $@) $(subst .$(VERSION),,$(notdir $@))

###
###  coordinates
###
coordinates: measures fits $(COORDINATESLIB_PATH)

$(COORDINATESLIB_PATH): $(COORDINATESLIB)
ifeq "$(OS)" "darwin"
	$(C++) -dynamiclib -install_name $(notdir $@) -o $@ $(filter %.o,$^) -L$(dir $@) -lcasa_measures -lcasa_measures_f -lcasa_scimath -lcasa_scimath_f -lcasa_fits -lcasa_tables -lcasa_casa -lcfitsio -lwcs -llapack -lblas $(FC_LIB)
endif
ifeq "$(OS)" "linux"
	$(C++) -shared -Wl,-soname,$(notdir $@) -o $@ $(filter %.o,$^) -L$(dir $@) -lcasa_measures -lcasa_measures_f -lcasa_scimath -lcasa_scimath_f -lcasa_fits -lcasa_tables -lcasa_casa -lcfitsio -lwcs -llapack -lblas $(FC_LIB)
endif
	cd $(dir $@) && ln -fs $(notdir $@) $(subst .$(VERSION),,$(notdir $@))

###
###  lattices
###
lattices: scimath $(LATTICESLIB_PATH)

$(LATTICESLIB_PATH): $(LATTICESLIB)
ifeq "$(OS)" "darwin"
	$(C++) -dynamiclib -install_name $(notdir $@) -o $@ $(filter %.o,$^) -L$(dir $@) -lcasa_tables -lcasa_scimath -lcasa_scimath_f -lcasa_casa -llapack -lblas $(FC_LIB)
endif
ifeq "$(OS)" "linux"
	$(C++) -shared -Wl,-soname,$(notdir $@) -o $@ $(filter %.o,$^) -L$(dir $@) -lcasa_tables -lcasa_scimath -lcasa_scimath_f -lcasa_casa -llapack -lblas $(FC_LIB)
endif
	cd $(dir $@) && ln -fs $(notdir $@) $(subst .$(VERSION),,$(notdir $@))

###
###  images
###
images: components lattices fits mirlib $(IMAGESLIB_PATH)

$(IMAGESLIB_PATH): $(IMAGESLIB)
ifeq "$(OS)" "darwin"
	$(C++) -dynamiclib -install_name $(notdir $@) -o $@ $(filter %.o,$^) -L$(dir $@) -lcasa_components -lcasa_coordinates -lcasa_measures -lcasa_measures_f -lcasa_scimath -lcasa_scimath_f -lcasa_fits -lcasa_lattices -lcasa_tables -lcasa_casa -lcfitsio -lcasa_mirlib -lwcs -llapack -lblas -lcfitsio $(FC_LIB)
endif
ifeq "$(OS)" "linux"
	$(C++) -shared -Wl,-soname,$(notdir $@) -o $@ $(filter %.o,$^) -L$(dir $@) -lcasa_components -lcasa_coordinates -lcasa_measures -lcasa_measures_f -lcasa_scimath -lcasa_scimath_f -lcasa_fits -lcasa_lattices -lcasa_tables -lcasa_casa -lcfitsio -lcasa_mirlib -lwcs -llapack -lblas -lcfitsio $(FC_LIB)
endif
	cd $(dir $@) && ln -fs $(notdir $@) $(subst .$(VERSION),,$(notdir $@))

###
###  tables
###
tables: casa $(TABLESLIB_PATH)

$(TABLESLIB_PATH): $(TABLESLIB)
ifeq "$(OS)" "darwin"
	$(C++) -dynamiclib -install_name $(notdir $@) -o $@ $(filter %.o,$^) -L$(dir $@) -lcasa_casa
endif
ifeq "$(OS)" "linux"
	$(C++) -shared -Wl,-soname,$(notdir $@) -o $@ $(filter %.o,$^) -L$(dir $@) -lcasa_casa
endif
	cd $(dir $@) && ln -fs $(notdir $@) $(subst .$(VERSION),,$(notdir $@))

###
###  scimath
###
scimath: casa $(SCIMATHLIB_PATH)

$(SCIMATHLIB_PATH): $(SCIMATHLIB) $(SCIMATHFLIB_PATH)
ifeq "$(OS)" "darwin"
	$(C++) -dynamiclib -install_name $(notdir $@) -o $@ $(filter %.o,$^) -L$(dir $@) -lcasa_scimath_f -lcasa_casa -llapack -lblas $(FC_LIB)
endif
ifeq "$(OS)" "linux"
	$(C++) -shared -Wl,-soname,$(notdir $@) -o $@ $(filter %.o,$^) -L$(dir $@) -lcasa_scimath_f -lcasa_casa -llapack -lblas $(FC_LIB)
endif
	cd $(dir $@) && ln -fs $(notdir $@) $(subst .$(VERSION),,$(notdir $@))

$(SCIMATHFLIB_PATH): $(SCIMATHFLIB)
ifeq "$(OS)" "darwin"
	$(C++) -dynamiclib -install_name $(notdir $@) -o $@ $(filter %.o,$^) -L$(dir $@) -lcasa_casa -llapack -lblas $(FC_LIB)
endif
ifeq "$(OS)" "linux"
	$(C++) -shared -Wl,-soname,$(notdir $@) -o $@ $(filter %.o,$^) -L$(dir $@) -lcasa_casa -llapack -lblas $(FC_LIB)
endif
	cd $(dir $@) && ln -fs $(notdir $@) $(subst .$(VERSION),,$(notdir $@))


###
###  measures
###
measures: scimath tables $(MEASURESLIB_PATH)

$(MEASURESLIB_PATH):  $(MEASURESLIB) $(MEASURESFLIB_PATH)
ifeq "$(OS)" "darwin"
	$(C++) -dynamiclib -install_name $(notdir $@) -o $@ $(filter %.o,$^) -L$(dir $@) -lcasa_measures_f -lcasa_scimath -lcasa_scimath_f -lcasa_tables -lcasa_casa $(FC_LIB)
endif
ifeq "$(OS)" "linux"
	$(C++) -shared -Wl,-soname,$(notdir $@) -o $@ $(filter %.o,$^) -L$(dir $@) -lcasa_measures_f -lcasa_scimath -lcasa_scimath_f -lcasa_tables -lcasa_casa $(FC_LIB)
endif
	cd $(dir $@) && ln -fs $(notdir $@) $(subst .$(VERSION),,$(notdir $@))

$(MEASURESFLIB_PATH):  $(MEASURESFLIB)
ifeq "$(OS)" "darwin"
	$(C++) -dynamiclib -install_name $(notdir $@) -o $@ $(filter %.o,$^) -L$(dir $@) -lcasa_scimath -lcasa_scimath_f -lcasa_tables -lcasa_casa $(FC_LIB)
endif
ifeq "$(OS)" "linux"
	$(C++) -shared -Wl,-soname,$(notdir $@) -o $@ $(filter %.o,$^) -L$(dir $@) -lcasa_scimath -lcasa_scimath_f -lcasa_tables -lcasa_casa $(FC_LIB)
endif
	cd $(dir $@) && ln -fs $(notdir $@) $(subst .$(VERSION),,$(notdir $@))

###
###  ms
###
ms: measures tables $(MSLIB_PATH)

$(MSLIB_PATH): $(MSLIB)
ifeq "$(OS)" "darwin"
	$(C++) -dynamiclib -install_name $(notdir $@) -o $@ $(filter %.o,$^) -L$(dir $@) -lcasa_measures -lcasa_measures_f -lcasa_tables -lcasa_scimath -lcasa_scimath_f -lcasa_casa -llapack -lblas $(FC_LIB)
endif
ifeq "$(OS)" "linux"
	$(C++) -shared -Wl,-soname,$(notdir $@) -o $@ $(filter %.o,$^) -L$(dir $@) -lcasa_measures -lcasa_measures_f -lcasa_tables -lcasa_scimath -lcasa_scimath_f -lcasa_casa -llapack -lblas $(FC_LIB)
endif
	cd $(dir $@) && ln -fs $(notdir $@) $(subst .$(VERSION),,$(notdir $@))


###
###  fits
###
fits: scimath tables $(FITSLIB_PATH)

$(FITSLIB_PATH): $(FITSLIB)
ifeq "$(OS)" "darwin"
	$(C++) -dynamiclib -install_name $(notdir $@) -o $@ $(filter %.o,$^) -L$(dir $@) -lcasa_measures -lcasa_measures_f -lcasa_tables -lcasa_casa -lcfitsio $(FC_LIB)
endif
ifeq "$(OS)" "linux"
	$(C++) -shared -Wl,-soname,$(notdir $@) -o $@ $(filter %.o,$^) -L$(dir $@) -lcasa_measures -lcasa_measures_f -lcasa_tables -lcasa_casa -lcfitsio $(FC_LIB)
endif
	cd $(dir $@) && ln -fs $(notdir $@) $(subst .$(VERSION),,$(notdir $@))

###
###  msfits
###
msfits: ms fits $(MSFITSLIB_PATH)

$(MSFITSLIB_PATH): $(MSFITSLIB)
ifeq "$(OS)" "darwin"
	$(C++) -dynamiclib -install_name $(notdir $@) -o $@ $(filter %.o,$^) -L$(dir $@) -lcasa_ms -lcasa_measures -lcasa_measures_f -lcasa_tables -lcasa_fits -lcasa_casa -lcfitsio -llapack -lblas $(FC_LIB)
endif
ifeq "$(OS)" "linux"
	$(C++) -shared -Wl,-soname,$(notdir $@) -o $@ $(filter %.o,$^) -L$(dir $@) -lcasa_ms -lcasa_measures -lcasa_measures_f -lcasa_tables -lcasa_fits -lcasa_casa -lcfitsio -llapack -lblas $(FC_LIB)
endif
	cd $(dir $@) && ln -fs $(notdir $@) $(subst .$(VERSION),,$(notdir $@))


###
###  mirlib
###
mirlib: $(MIRLIB_PATH)

$(MIRLIB_PATH): $(MIRLIB)
ifeq "$(OS)" "darwin"
	$(C++) -dynamiclib -install_name $(notdir $@) -o $@ $(filter %.o,$^)
endif
ifeq "$(OS)" "linux"
	$(C++) -shared -Wl,-soname,$(notdir $@) -o $@ $(filter %.o,$^)
endif
	cd $(dir $@) && ln -fs $(notdir $@) $(subst .$(VERSION),,$(notdir $@))

clean:
	rm -f `find . -type f -name '*.lcc'` `find . -type f -name '*.ycc'`

setup:
	mkdir -p $(DESTDIR)/include/casacore
ifneq "$(ARCH)" ""
	mkdir -p $(DESTDIR)/$(ARCH)/lib
else
	mkdir -p $(DESTDIR)/lib
endif
	(cd casa && tar -cf - $(shell find casa -type f -name '*.h' | egrep -v '/test/|/apps/' | perl -pe 's|^[^/]+/||')) | tar -C $(DESTDIR)/include/casacore -xf -
	(cd casa && tar -cf - $(shell find casa -type f -name '*.tcc' | egrep -v '/test/|/apps/' | perl -pe 's|^[^/]+/||')) | tar -C $(DESTDIR)/include/casacore -xf -
	(cd components && tar -cf - $(shell find components -type f -name '*.h' | egrep -v '/test/|/apps/' | perl -pe 's|^[^/]+/||')) | tar -C $(DESTDIR)/include/casacore -xf -
	(cd components && tar -cf - $(shell find components -type f -name '*.tcc' | egrep -v '/test/|/apps/' | perl -pe 's|^[^/]+/||')) | tar -C $(DESTDIR)/include/casacore -xf -
	(cd coordinates && tar -cf - $(shell find coordinates -type f -name '*.h' | egrep -v '/test/|/apps/' | perl -pe 's|^[^/]+/||')) | tar -C $(DESTDIR)/include/casacore -xf -
	(cd coordinates && tar -cf - $(shell find coordinates -type f -name '*.tcc' | egrep -v '/test/|/apps/' | perl -pe 's|^[^/]+/||')) | tar -C $(DESTDIR)/include/casacore -xf -
	(cd scimath && tar -cf - $(shell find scimath -type f -name '*.h' | egrep -v '/test/|/apps/' | perl -pe 's|^[^/]+/||')) | tar -C $(DESTDIR)/include/casacore -xf -
	(cd scimath && tar -cf - $(shell find scimath -type f -name '*.tcc' | egrep -v '/test/|/apps/' | perl -pe 's|^[^/]+/||')) | tar -C $(DESTDIR)/include/casacore -xf -
	(cd tables && tar -cf - $(shell find tables -type f -name '*.h' | egrep -v '/test/|/apps/' | perl -pe 's|^[^/]+/||')) | tar -C $(DESTDIR)/include/casacore -xf -
	(cd tables && tar -cf - $(shell find tables -type f -name '*.tcc' | egrep -v '/test/|/apps/' | perl -pe 's|^[^/]+/||')) | tar -C $(DESTDIR)/include/casacore -xf -
	(cd lattices && tar -cf - $(shell find lattices -type f -name '*.h' | egrep -v '/test/|/apps/' | perl -pe 's|^[^/]+/||')) | tar -C $(DESTDIR)/include/casacore -xf -
	(cd lattices && tar -cf - $(shell find lattices -type f -name '*.tcc' | egrep -v '/test/|/apps/' | perl -pe 's|^[^/]+/||')) | tar -C $(DESTDIR)/include/casacore -xf -
	(cd measures && tar -cf - $(shell find measures -type f -name '*.h' | egrep -v '/test/|/apps/' | perl -pe 's|^[^/]+/||')) | tar -C $(DESTDIR)/include/casacore -xf -
	(cd measures && tar -cf - $(shell find measures -type f -name '*.tcc' | egrep -v '/test/|/apps/' | perl -pe 's|^[^/]+/||')) | tar -C $(DESTDIR)/include/casacore -xf -
	(cd images && tar -cf - $(shell find images -type f -name '*.h' | egrep -v '/test/|/apps/' | perl -pe 's|^[^/]+/||')) | tar -C $(DESTDIR)/include/casacore -xf -
	(cd images && tar -cf - $(shell find images -type f -name '*.tcc' | egrep -v '/test/|/apps/' | perl -pe 's|^[^/]+/||')) | tar -C $(DESTDIR)/include/casacore -xf -
	(cd ms && tar -cf - $(shell find ms -type f -name '*.h' | egrep -v '/test/|/apps/' | perl -pe 's|^[^/]+/||')) | tar -C $(DESTDIR)/include/casacore -xf -
	(cd ms && tar -cf - $(shell find ms -type f -name '*.tcc' | egrep -v '/test/|/apps/' | perl -pe 's|^[^/]+/||')) | tar -C $(DESTDIR)/include/casacore -xf -
	(cd fits && tar -cf - $(shell find fits -type f -name '*.h' | egrep -v '/test/|/apps/' | perl -pe 's|^[^/]+/||')) | tar -C $(DESTDIR)/include/casacore -xf -
	(cd fits && tar -cf - $(shell find fits -type f -name '*.tcc' | egrep -v '/test/|/apps/' | perl -pe 's|^[^/]+/||')) | tar -C $(DESTDIR)/include/casacore -xf -
	(cd msfits && tar -cf - $(shell find msfits -type f -name '*.h' | egrep -v '/test/|/apps/' | perl -pe 's|^[^/]+/||')) | tar -C $(DESTDIR)/include/casacore -xf -
	tar -cf - $(shell find mirlib -type f -name '*.h' | egrep -v '/test/|/apps/') | tar -C $(DESTDIR)/include/casacore -xf -


TCASACC := $(shell find casa -type f -name 't*.cc' | grep /test/)
TCASA := $(basename $(TCASACC | perl -pe 's|^[^/]+/||'))

casatest: $(TCASA)

