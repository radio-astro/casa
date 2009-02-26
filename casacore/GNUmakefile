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

-include makedefs

C++ := c++
CXXFLAGS := -O2

CC  := gcc
CFLAGS := -O2

FC  := 
FFLAGS := -O2

DESTDIR := 
ARCH := 

DEP := 1

VERSION:=$(shell head -1 VERSION | perl -pe "s|^(\S+).*|\$$1|")

OS := $(shell uname | tr 'A-Z' 'a-z')
arch := $(shell uname -p)
fastdep := $(shell which fastdep 2> /dev/null)

ifeq "$(OS)" "darwin"
SO := $(VERSION).dylib
#INC :=
endif
ifeq "$(OS)" "linux"
SO := so.$(VERSION)
INC += -I/usr/include/cfitsio
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
allsys: 
	@echo "error could not find fortran compiler; please set the 'FC', GNUmakefile variable"
else
allsys: images msfits
endif

CASACC := $(shell find casa -type f -name '*.cc' | egrep -v '/test/|/apps/')
CASAOBJ := $(CASACC:%.cc=%.o)
ifneq "$(ARCH)" ""
CASALIB := $(shell echo $(CASAOBJ) | perl -pe "s|(\w+\\.o)|$(ARCH)/\$$1|g")
CASADEP := $(shell echo $(CASAOBJ) | perl -pe "s|(\w+)\\.o|$(ARCH)/\$$1.dep|g")
CASALIB_PATH := $(DESTDIR)/$(ARCH)/lib/libcasa_casa.$(SO)
CORELIB_PATH := $(DESTDIR)/$(ARCH)/lib/libcasacore.$(SO)
else
CASALIB := $(CASAOBJ)
CASADEP := $(shell echo $(CASAOBJ) | perl -pe "s|(\w+)\\.o|\$$1.dep|g")
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
COMPONENTSDEP := $(shell echo $(COMPONENTSOBJ) | perl -pe "s|(\w+)\\.o|$(ARCH)/\$$1.dep|g")
COMPONENTSLIB_PATH := $(DESTDIR)/$(ARCH)/lib/libcasa_components.$(SO)
else
COMPONENTSLIB := $(COMPONENTSOBJ)
COMPONENTSDEP := $(shell echo $(COMPONENTSOBJ) | perl -pe "s|(\w+)\\.o|\$$1.dep|g")
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
COORDINATESDEP := $(shell echo $(COORDINATESOBJ) | perl -pe "s|(\w+)\\.o|$(ARCH)/\$$1.dep|g")
COORDINATESLIB_PATH := $(DESTDIR)/$(ARCH)/lib/libcasa_coordinates.$(SO)
else
COORDINATESLIB := $(COORDINATESOBJ)
COORDINATESDEP := $(shell echo $(COORDINATESOBJ) | perl -pe "s|(\w+)\\.o|\$$1.dep|g")
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
LATTICESDEP := $(shell echo $(LATTICESOBJ) | perl -pe "s|(\w+)\\.o|$(ARCH)/\$$1.dep|g")
LATTICESLIB_PATH := $(DESTDIR)/$(ARCH)/lib/libcasa_lattices.$(SO)
else
LATTICESLIB := $(LATTICESOBJ)
LATTICESDEP := $(shell echo $(LATTICESOBJ) | perl -pe "s|(\w+)\\.o|\$$1.dep|g")
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
IMAGESDEP :=    $(shell echo $(IMAGESOBJ) | perl -pe "s|(\w+)\\.o|$(ARCH)/\$$1.dep|g")
IMAGESLIB_PATH := $(DESTDIR)/$(ARCH)/lib/libcasa_images.$(SO)
else
IMAGESLIB := images/images/Images/ImageExprGram.lcc images/images/Images/ImageExprGram.ycc \
		$(IMAGESOBJ)
IMAGESDEP :=    $(shell echo $(IMAGESOBJ) | perl -pe "s|(\w+)\\.o|\$$1.dep|g")
IMAGESLIB_PATH := $(DESTDIR)/lib/libcasa_images.$(SO)
endif
###  tables -> casa
TABLESCC := $(shell find tables -type f -name '*.cc' | egrep -v '/test/|/apps/')
TABLESOBJ := $(TABLESCC:%.cc=%.o)
ifneq "$(ARCH)" ""
TABLESLIB := tables/tables/Tables/RecordGram.lcc tables/tables/Tables/RecordGram.ycc \
		tables/tables/Tables/TableGram.lcc tables/tables/Tables/TableGram.ycc \
		$(shell echo $(TABLESOBJ) | perl -pe "s|(\w+\\.o)|$(ARCH)/\$$1|g")
TABLESDEP :=    $(shell echo $(TABLESOBJ) | perl -pe "s|(\w+)\\.o|$(ARCH)/\$$1.dep|g")
TABLESLIB_PATH := $(DESTDIR)/$(ARCH)/lib/libcasa_tables.$(SO)
else
TABLESLIB := tables/tables/Tables/RecordGram.lcc tables/tables/Tables/RecordGram.ycc \
		tables/tables/Tables/TableGram.lcc tables/tables/Tables/TableGram.ycc \
		$(TABLESOBJ)
TABLESDEP :=    $(shell echo $(TABLESOBJ) | perl -pe "s|(\w+)\\.o|\$$1.dep|g")
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
SCIMATHDEP := $(shell echo $(SCIMATHOBJ) | perl -pe "s|(\w+)\\.o|$(ARCH)/\$$1.dep|g")
SCIMATHLIB_PATH := $(DESTDIR)/$(ARCH)/lib/libcasa_scimath.$(SO)
SCIMATHFLIB := $(shell echo $(SCIMATHFOBJ) | perl -pe "s|(\w+\\.o)|$(ARCH)/\$$1|g")
SCIMATHFLIB_PATH := $(DESTDIR)/$(ARCH)/lib/libcasa_scimath_f.$(SO)
else
SCIMATHLIB := $(SCIMATHOBJ)
SCIMATHDEP := $(shell echo $(SCIMATHOBJ) | perl -pe "s|(\w+)\\.o|\$$1.dep|g")
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
MEASURESDEP := $(shell echo $(MEASURESOBJ) | perl -pe "s|(\w+)\\.o|$(ARCH)/\$$1.dep|g")
MEASURESLIB_PATH := $(DESTDIR)/$(ARCH)/lib/libcasa_measures.$(SO)
MEASURESFLIB := $(shell echo $(MEASURESFOBJ) | perl -pe "s|(\w+\\.o)|$(ARCH)/\$$1|g")
MEASURESFLIB_PATH := $(DESTDIR)/$(ARCH)/lib/libcasa_measures_f.$(SO)
else
MEASURESLIB := $(MEASURESOBJ)
MEASURESDEP := $(shell echo $(MEASURESOBJ) | perl -pe "s|(\w+)\\.o|\$$1.dep|g")
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
FITSDEP := $(shell echo $(FITSOBJ) | perl -pe "s|(\w+)\\.o|$(ARCH)/\$$1.dep|g")
FITSLIB_PATH := $(DESTDIR)/$(ARCH)/lib/libcasa_fits.$(SO)
else
FITSLIB := $(FITSOBJ)
FITSDEP := $(shell echo $(FITSOBJ) | perl -pe "s|(\w+)\\.o|\$$1.dep|g")
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
MSDEP :=	$(shell echo $(MSOBJ) | perl -pe "s|(\w+)\\.o|$(ARCH)/\$$1.dep|g")
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
MSDEP :=	$(shell echo $(MSOBJ) | perl -pe "s|(\w+)\\.o|\$$1.dep|g")
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
MSFITSDEP := $(shell echo $(MSFITSOBJ) | perl -pe "s|(\w+)\\.o|$(ARCH)/\$$1.dep|g")
MSFITSLIB_PATH := $(DESTDIR)/$(ARCH)/lib/libcasa_msfits.$(SO)
else
MSFITSLIB := $(MSFITSOBJ)
MSFITSDEP := $(shell echo $(MSFITSOBJ) | perl -pe "s|(\w+)\\.o|\$$1.dep|g")
MSFITSLIB_PATH := $(DESTDIR)/lib/libcasa_msfits.$(SO)
endif
###
MIRCC := $(shell find mirlib -type f -name '*.c' | egrep -v '/test/|/apps/')
MIROBJ := $(MIRCC:%.c=%.o)
ifneq "$(ARCH)" ""
MIRLIB := $(shell echo $(MIROBJ) | perl -pe "s|(\w+\\.o)|$(ARCH)/\$$1|g")
MIRDEP := $(shell echo $(MIROBJ) | perl -pe "s|(\w+)\\.o|$(ARCH)/\$$1.dep|g")
MIRLIB_PATH := $(DESTDIR)/$(ARCH)/lib/libcasa_mirlib.$(SO)
else
MIRLIB := $(MIROBJ)
MIRDEP := $(shell echo $(MIROBJ) | perl -pe "s|(\w+)\\.o|\$$1.dep|g")
MIRLIB_PATH := $(DESTDIR)/lib/libcasa_mirlib.$(SO)
endif

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

define orphan-objects
  perl -e 'use File::Find; %source = ( ); sub find_source { if ( -f $$_ && m/\.(?:cc|c|f)$$/ ) { my $$file = $$_; s/\.(?:cc|c|f)$$//; $$source{$$_} = "$$File::Find::dir/$$file"; } } sub find_orphan { if ( -f $$_ && m/\.o$$/ ) { my $$file = $$_; s/\.o$$//; if ( ! defined $$source{$$_} ) { unlink($$file); } } } find( { wanted => \&find_source }, "$1" ); find( { wanted => \&find_orphan }, "$1" );'
endef

INCDIR := $(DESTDIR)/include
MODULES := casa components coordinates fits images lattices \
		measures mirlib ms msfits scimath tables
MODULESRE := $(shell echo $(MODULES) | perl -pe '$$_=join("|",split())')
VPATH := . $(MODULES)

$(INCDIR)/casacore/%.h: %.h
	@if test ! -d $(dir $@); then mkdir -p $(dir $@); fi
	cp $< $@

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

t% : t%.cc
	$(C++) $(COREINC) $(INC) -o $@ $< -L$(DESTDIR)/lib -lcasa_casa

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
libcasacore: $(CORELIB_PATH)

$(CORELIB_PATH): $(CASALIB) $(COMPONENTSLIB) $(COORDINATESLIB) $(LATTICESLIB) $(IMAGESLIB) $(TABLESLIB) $(SCIMATHLIB) \
			$(SCIMATHFLIB) $(MEASURESLIB) $(MEASURESFLIB) $(FITSLIB) $(MSLIB) $(MSFITSLIB) $(MIRLIB) \
			$(shell find . -type f \( -name '*.h' -o -name '*.tcc' \) | egrep -v '/test/|/apps/' | perl -pe "s@^\./\w+/@$(INCDIR)/casacore/@g")
	@if test ! -d $(dir $@); then mkdir -p $(dir $@); fi
	@$(call orphan-objects,.)
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
casa: $(CASALIB_PATH) $(shell find casa -type f \( -name '*.h' -o -name '*.tcc' \) | egrep -v '/test/|/apps/' | perl -pe "s@^casa/@$(INCDIR)/casacore/@g")

$(CASALIB_PATH): $(CASALIB)
	@if test ! -d $(dir $@); then mkdir -p $(dir $@); fi
	@$(call orphan-objects,casa)
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
components: coordinates tables $(COMPONENTSLIB_PATH) $(shell find components -type f \( -name '*.h' -o -name '*.tcc' \) | egrep -v '/test/|/apps/' | perl -pe "s@^components/@$(INCDIR)/casacore/@g")

$(COMPONENTSLIB_PATH): $(COORDINATESLIB_PATH) $(TABLESLIB_PATH) $(COMPONENTSLIB)
	@if test ! -d $(dir $@); then mkdir -p $(dir $@); fi
	@$(call orphan-objects,components)
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
coordinates: measures fits $(COORDINATESLIB_PATH) $(shell find coordinates -type f \( -name '*.h' -o -name '*.tcc' \) | egrep -v '/test/|/apps/' | perl -pe "s@^coordinates/@$(INCDIR)/casacore/@g")

$(COORDINATESLIB_PATH): $(MEASURESLIB_PATH) $(FITSLIB_PATH) $(COORDINATESLIB)
	@if test ! -d $(dir $@); then mkdir -p $(dir $@); fi
	@$(call orphan-objects,coordinates)
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

$(LATTICESLIB_PATH): $(SCIMATHFLIB_PATH) $(LATTICESLIB) $(shell find lattices -type f \( -name '*.h' -o -name '*.tcc' \) | egrep -v '/test/|/apps/' | perl -pe "s@^lattices/@$(INCDIR)/casacore/@g")
	@if test ! -d $(dir $@); then mkdir -p $(dir $@); fi
	@$(call orphan-objects,lattices)
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
images: components lattices fits mirlib $(IMAGESLIB_PATH) $(shell find images -type f \( -name '*.h' -o -name '*.tcc' \) | egrep -v '/test/|/apps/' | perl -pe "s@^images/@$(INCDIR)/casacore/@g")

$(IMAGESLIB_PATH): $(COMPONENTSLIB_PATH) $(LATTICESLIB_PATH) $(FITSLIB_PATH) $(MIRLIB_PATH) $(IMAGESLIB)
	@if test ! -d $(dir $@); then mkdir -p $(dir $@); fi
	@$(call orphan-objects,images)
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
tables: casa $(TABLESLIB_PATH) $(shell find tables -type f \( -name '*.h' -o -name '*.tcc' \) | egrep -v '/test/|/apps/' | perl -pe "s@^tables/@$(INCDIR)/casacore/@g")

$(TABLESLIB_PATH): $(CASALIB_PATH) $(TABLESLIB)
	@if test ! -d $(dir $@); then mkdir -p $(dir $@); fi
	@$(call orphan-objects,tables)
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

$(SCIMATHLIB_PATH): $(CASALIB_PATH) $(SCIMATHLIB) $(SCIMATHFLIB_PATH) $(shell find scimath -type f \( -name '*.h' -o -name '*.tcc' \) | egrep -v '/test/|/apps/' | perl -pe "s@^scimath/@$(INCDIR)/casacore/@g")
	@if test ! -d $(dir $@); then mkdir -p $(dir $@); fi
	@$(call orphan-objects,scimath)
ifeq "$(OS)" "darwin"
	$(C++) -dynamiclib -install_name $(notdir $@) -o $@ $(filter %.o,$^) -L$(dir $@) -lcasa_scimath_f -lcasa_casa -llapack -lblas -lfftw3 -lfftw3f -lfftw3_threads -lfftw3f_threads $(FC_LIB)
endif
ifeq "$(OS)" "linux"
	$(C++) -shared -Wl,-soname,$(notdir $@) -o $@ $(filter %.o,$^) -L$(dir $@) -lcasa_scimath_f -lcasa_casa -llapack -lblas -lfftw3 -lfftw3f -lfftw3_threads -lfftw3f_threads $(FC_LIB)
endif
	cd $(dir $@) && ln -fs $(notdir $@) $(subst .$(VERSION),,$(notdir $@))

$(SCIMATHFLIB_PATH): $(SCIMATHFLIB)
	@if test ! -d $(dir $@); then mkdir -p $(dir $@); fi
	@$(call orphan-objects,scimath)
ifeq "$(OS)" "darwin"
	$(C++) -dynamiclib -install_name $(notdir $@) -o $@ $(filter %.o,$^) -L$(dir $@) -lcasa_casa -llapack -lblas -lfftw3 -lfftw3f -lfftw3_threads -lfftw3f_threads $(FC_LIB)
endif
ifeq "$(OS)" "linux"
	$(C++) -shared -Wl,-soname,$(notdir $@) -o $@ $(filter %.o,$^) -L$(dir $@) -lcasa_casa -llapack -lblas -lfftw3 -lfftw3f -lfftw3_threads -lfftw3f_threads $(FC_LIB)
endif
	cd $(dir $@) && ln -fs $(notdir $@) $(subst .$(VERSION),,$(notdir $@))


###
###  measures
###
measures: scimath tables $(MEASURESLIB_PATH) $(shell find measures -type f \( -name '*.h' -o -name '*.tcc' \) | egrep -v '/test/|/apps/' | perl -pe "s@^measures/@$(INCDIR)/casacore/@g")

$(MEASURESLIB_PATH):  $(SCIMATHFLIB_PATH) $(TABLESLIB_PATH) $(MEASURESLIB) $(MEASURESFLIB_PATH)
	@if test ! -d $(dir $@); then mkdir -p $(dir $@); fi
	@$(call orphan-objects,measures)
ifeq "$(OS)" "darwin"
	$(C++) -dynamiclib -install_name $(notdir $@) -o $@ $(filter %.o,$^) -L$(dir $@) -lcasa_measures_f -lcasa_scimath -lcasa_scimath_f -lcasa_tables -lcasa_casa $(FC_LIB)
endif
ifeq "$(OS)" "linux"
	$(C++) -shared -Wl,-soname,$(notdir $@) -o $@ $(filter %.o,$^) -L$(dir $@) -lcasa_measures_f -lcasa_scimath -lcasa_scimath_f -lcasa_tables -lcasa_casa $(FC_LIB)
endif
	cd $(dir $@) && ln -fs $(notdir $@) $(subst .$(VERSION),,$(notdir $@))

$(MEASURESFLIB_PATH):  $(MEASURESFLIB)
	@if test ! -d $(dir $@); then mkdir -p $(dir $@); fi
	@$(call orphan-objects,measures)
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
ms: measures tables $(MSLIB_PATH) $(shell find ms -type f \( -name '*.h' -o -name '*.tcc' \) | egrep -v '/test/|/apps/' | perl -pe "s@^ms/@$(INCDIR)/casacore/@g")

$(MSLIB_PATH): $(MEASURESLIB_PATH) $(TABLESLIB_PATH) $(MSLIB)
	@if test ! -d $(dir $@); then mkdir -p $(dir $@); fi
	@$(call orphan-objects,ms)
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
fits: scimath tables $(FITSLIB_PATH) $(shell find fits -type f \( -name '*.h' -o -name '*.tcc' \) | egrep -v '/test/|/apps/' | perl -pe "s@^fits/@$(INCDIR)/casacore/@g")

$(FITSLIB_PATH): $(SCIMATHLIB_PATH) $(TABLESLIB_PATH) $(FITSLIB)
	@if test ! -d $(dir $@); then mkdir -p $(dir $@); fi
	@$(call orphan-objects,fits)
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
msfits: ms fits $(MSFITSLIB_PATH) $(shell find msfits -type f \( -name '*.h' -o -name '*.tcc' \) | egrep -v '/test/|/apps/' | perl -pe "s@^msfits/@$(INCDIR)/casacore/@g")

$(MSFITSLIB_PATH): $(MSLIB_PATH) $(FITSLIB_PATH) $(MSFITSLIB)
	@if test ! -d $(dir $@); then mkdir -p $(dir $@); fi
	@$(call orphan-objects,msfits)
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

$(MIRLIB_PATH): $(MIRLIB) $(shell find msfits -type f \( -name '*.h' -o -name '*.tcc' \) | egrep -v '/test/|/apps/' | perl -pe "s@^@$(INCDIR)/casacore/@g")
	@if test ! -d $(dir $@); then mkdir -p $(dir $@); fi
	@$(call orphan-objects,mirlib)
ifeq "$(OS)" "darwin"
	$(C++) -dynamiclib -install_name $(notdir $@) -o $@ $(filter %.o,$^)
endif
ifeq "$(OS)" "linux"
	$(C++) -shared -Wl,-soname,$(notdir $@) -o $@ $(filter %.o,$^)
endif
	cd $(dir $@) && ln -fs $(notdir $@) $(subst .$(VERSION),,$(notdir $@))

installinc: $(shell find . -type f \( -name '*.h' -o -name '*.tcc' \) | egrep -v '/test/|/apps/' | perl -pe "s@\\./(?:$(MODULESRE))/@$(INCDIR)/casacore/@g")

TCASACC := $(shell find casa -type f -name 't*.cc' | grep /test/)
TCASA := $(basename $(TCASACC | perl -pe 's|^[^/]+/||'))

casatest: $(TCASA)

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
	@rm -rf $(DESTDIR)/include/casacore

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

ifeq "$(DEP)" "1"
-include $(CASADEP) $(COMPONENTSDEP) $(COORDINATESDEP) $(LATTICESDEP) $(IMAGESDEP) \
	$(SCIMATHDEP) $(MEASURESDEP) $(FITSDEP) $(MSDEP) $(MSFITSDEP) $(MIRDEP)
endif
