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
INSTALLDIR := $(DESTDIR)

VERSION:=0.3.0

LIBDIR := $(INSTALLDIR)/lib

OS := $(shell uname | tr 'A-Z' 'a-z')
ARCH := $(shell uname -p)
ifeq "$(OS)" "darwin"
SO := $(VERSION).dylib
#INC :=
endif
ifeq "$(OS)" "linux"
SO := so.$(VERSION)
INC += -I/usr/include/cfitsio
ifeq "$(ARCH)" "x86_64"
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
CASALIB := $(CASAOBJ)
### components   -> tables      -> casa
###              -> coordinates -> (cfitsio)
###                             -> (wcs)
###                             -> measures  -> scimath   -> casa
###                                                       -> (lapack)
###                                                       -> (blas)
COMPONENTSCC := $(shell find components -type f -name '*.cc' | egrep -v '/test/|/apps/')
COMPONENTSOBJ := $(COMPONENTSCC:%.cc=%.o)
COMPONENTSLIB := $(COMPONENTSOBJ)
###  coordinates -> measures  -> scimath -> casa
###                                      -> (lapack)
###                                      -> (blas)
###              -> fits      -> tables  -> casa
###                           -> (cfitsio)
###              -> (wcslib)
COORDINATESCC := $(shell find coordinates -type f -name '*.cc' | egrep -v '/test/|/apps/')
COORDINATESOBJ := $(COORDINATESCC:%.cc=%.o)
COORDINATESLIB := $(COORDINATESOBJ)
###  lattices -> tables  -> casa
###           -> scimath -> casa
###                      -> (lapack)
###                      -> (blas)
LATTICESCC := $(shell find lattices -type f -name '*.cc' | egrep -v '/test/|/apps/')
LATTICESOBJ := $(LATTICESCC:%.cc=%.o)
LATTICESLIB := $(LATTICESOBJ)
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
IMAGESLIB := images/images/Images/ImageExprGram.lcc images/images/Images/ImageExprGram.ycc $(IMAGESOBJ)
###  tables -> casa
TABLESCC := $(shell find tables -type f -name '*.cc' | egrep -v '/test/|/apps/')
TABLESOBJ := $(TABLESCC:%.cc=%.o)
TABLESLIB := tables/tables/Tables/RecordGram.lcc tables/tables/Tables/RecordGram.ycc \
		tables/tables/Tables/TableGram.lcc tables/tables/Tables/TableGram.ycc $(TABLESOBJ)
###  scimath -> casa
###          -> (lapack)
###          -> (blas)
SCIMATHCC := $(shell find scimath -type f -name '*.cc' | egrep -v '/test/|/apps/')
SCIMATHF := $(shell find scimath -type f -name '*.f' | egrep -v '/test/|/apps/')
SCIMATHOBJ := $(SCIMATHCC:%.cc=%.o)
SCIMATHFOBJ := $(SCIMATHF:%.f=%.o)
SCIMATHLIB := $(SCIMATHOBJ)
SCIMATHFLIB := $(SCIMATHFOBJ)
###  measures -> scimath -> casa
###           -> tables  -> casa
MEASURESCC := $(shell find measures -type f -name '*.cc' | egrep -v '/test/|/apps/')
MEASURESF := $(shell find measures -type f -name '*.f' | egrep -v '/test/|/apps/')
MEASURESOBJ := $(MEASURESCC:%.cc=%.o)
MEASURESFOBJ := $(MEASURESF:%.f=%.o)
MEASURESLIB := $(MEASURESOBJ)
MEASURESFLIB := $(MEASURESFOBJ)
###  fits -> measures  -> scimath -> casa
###       -> tables    -> casa
###       -> (cfitsio)
FITSCC := $(shell find fits -type f -name '*.cc' | egrep -v '/test/|/apps/')
FITSOBJ := $(FITSCC:%.cc=%.o)
FITSLIB := $(FITSOBJ)
###  ms -> measures  -> scimath -> casa
###                             -> (lapack)
###                             -> (blas)
MSCC := $(shell find ms -type f -name '*.cc' | egrep -v '/test/|/apps/')
MSOBJ := $(MSCC:%.cc=%.o)
MSLIB := ms/ms/MeasurementSets/MSAntennaGram.lcc ms/ms/MeasurementSets/MSAntennaGram.ycc \
	       ms/ms/MeasurementSets/MSArrayGram.lcc ms/ms/MeasurementSets/MSArrayGram.ycc \
	       ms/ms/MeasurementSets/MSCorrGram.lcc ms/ms/MeasurementSets/MSCorrGram.ycc \
	       ms/ms/MeasurementSets/MSFieldGram.lcc ms/ms/MeasurementSets/MSFieldGram.ycc \
	       ms/ms/MeasurementSets/MSScanGram.lcc ms/ms/MeasurementSets/MSScanGram.ycc \
	       ms/ms/MeasurementSets/MSSpwGram.lcc ms/ms/MeasurementSets/MSSpwGram.ycc \
	       ms/ms/MeasurementSets/MSTimeGram.lcc ms/ms/MeasurementSets/MSTimeGram.ycc \
	       ms/ms/MeasurementSets/MSUvDistGram.lcc ms/ms/MeasurementSets/MSUvDistGram.ycc $(MSOBJ)
###  msfits -> fits -> (cfitsio)
###                 -> measures  -> tables  -> casa
###                              -> scimath -> casa
###         -> ms   -> measures  -> scimath -> casa
###                              -> (lapack)
###                              -> (blas)
MSFITSCC := $(shell find msfits -type f -name '*.cc' | egrep -v '/test/|/apps/')
MSFITSOBJ := $(MSFITSCC:%.cc=%.o)
MSFITSLIB := $(MSFITSOBJ)
###
MIRCC := $(shell find mirlib -type f -name '*.c' | egrep -v '/test/|/apps/')
MIROBJ := $(MIRCC:%.c=%.o)
MIRLIB := $(MIROBJ)

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

###
###  libcasacore
###
libcasacore: $(DESTDIR)/lib/libcasacore.$(SO)

$(DESTDIR)/lib/libcasacore.$(SO): $(CASALIB) $(COMPONENTSLIB) $(COORDINATESLIB) $(LATTICESLIB) \
					$(IMAGESLIB) $(TABLESLIB) $(SCIMATHLIB) $(SCIMATHFLIB) \
					$(MEASURESLIB) $(MEASURESFLIB) $(FITSLIB) $(MSLIB) \
					$(MSFITSLIB) $(MIRLIB)
ifeq "$(OS)" "darwin"
	$(C++) -dynamiclib -install_name $(notdir $@) -o $@ $(filter %.o,$^) -lcfitsio -lwcs -llapack -lblas $(FC_LIB)
endif
ifeq "$(OS)" "linux"
	$(C++) -shared -Wl,-soname,$(notdir $@) -o $@ $(filter %.o,$^) -lcfitsio -lwcs -llapack -lblas $(FC_LIB)
endif
	cd $(DESTDIR)/lib && ln -fs $(notdir $@) $(subst .$(VERSION),,$(notdir $@))

###
###  casa
###
casa: $(DESTDIR)/lib/libcasa_casa.$(SO)

$(DESTDIR)/lib/libcasa_casa.$(SO): $(CASALIB)
ifeq "$(OS)" "darwin"
	$(C++) -dynamiclib -install_name $(notdir $@) -o $@ $(filter %.o,$^)
endif
ifeq "$(OS)" "linux"
	$(C++) -shared -Wl,-soname,$(notdir $@) -o $@ $(filter %.o,$^)
endif
	cd $(DESTDIR)/lib && ln -fs $(notdir $@) $(subst .$(VERSION),,$(notdir $@))

###
###  components
###
components: coordinates tables $(DESTDIR)/lib/libcasa_components.$(SO)

$(DESTDIR)/lib/libcasa_components.$(SO): $(COMPONENTSLIB)
ifeq "$(OS)" "darwin"
	$(C++) -dynamiclib -install_name $(notdir $@) -o $@ $(filter %.o,$^) -L$(DESTDIR)/lib -lcasa_coordinates -lcasa_measures -lcasa_measures_f -lcasa_scimath -lcasa_scimath_f -lcasa_fits -lcasa_tables -lcasa_casa -lcfitsio -lwcs -llapack -lblas $(FC_LIB)
endif
ifeq "$(OS)" "linux"
	$(C++) -shared -Wl,-soname,$(notdir $@) -o $@ $(filter %.o,$^) -L$(DESTDIR)/lib -lcasa_coordinates -lcasa_measures -lcasa_measures_f -lcasa_scimath -lcasa_scimath_f -lcasa_fits -lcasa_tables -lcasa_casa -lcfitsio -lwcs -llapack -lblas $(FC_LIB)
endif
	cd $(DESTDIR)/lib && ln -fs $(notdir $@) $(subst .$(VERSION),,$(notdir $@))

###
###  coordinates
###
coordinates: measures fits $(DESTDIR)/lib/libcasa_coordinates.$(SO)

$(DESTDIR)/lib/libcasa_coordinates.$(SO): $(COORDINATESLIB)
ifeq "$(OS)" "darwin"
	$(C++) -dynamiclib -install_name $(notdir $@) -o $@ $(filter %.o,$^) -L$(DESTDIR)/lib -lcasa_measures -lcasa_measures_f -lcasa_scimath -lcasa_scimath_f -lcasa_fits -lcasa_tables -lcasa_casa -lcfitsio -lwcs -llapack -lblas $(FC_LIB)
endif
ifeq "$(OS)" "linux"
	$(C++) -shared -Wl,-soname,$(notdir $@) -o $@ $(filter %.o,$^) -L$(DESTDIR)/lib -lcasa_measures -lcasa_measures_f -lcasa_scimath -lcasa_scimath_f -lcasa_fits -lcasa_tables -lcasa_casa -lcfitsio -lwcs -llapack -lblas $(FC_LIB)
endif
	cd $(DESTDIR)/lib && ln -fs $(notdir $@) $(subst .$(VERSION),,$(notdir $@))

###
###  lattices
###
lattices: scimath $(DESTDIR)/lib/libcasa_lattices.$(SO)

$(DESTDIR)/lib/libcasa_lattices.$(SO): $(LATTICESLIB)
ifeq "$(OS)" "darwin"
	$(C++) -dynamiclib -install_name $(notdir $@) -o $@ $(filter %.o,$^) -L$(DESTDIR)/lib -lcasa_tables -lcasa_scimath -lcasa_scimath_f -lcasa_casa -llapack -lblas $(FC_LIB)
endif
ifeq "$(OS)" "linux"
	$(C++) -shared -Wl,-soname,$(notdir $@) -o $@ $(filter %.o,$^) -L$(DESTDIR)/lib -lcasa_tables -lcasa_scimath -lcasa_scimath_f -lcasa_casa -llapack -lblas $(FC_LIB)
endif
	cd $(DESTDIR)/lib && ln -fs $(notdir $@) $(subst .$(VERSION),,$(notdir $@))

###
###  images
###
images: components lattices fits mirlib $(DESTDIR)/lib/libcasa_images.$(SO)

$(DESTDIR)/lib/libcasa_images.$(SO): $(IMAGESLIB)
ifeq "$(OS)" "darwin"
	$(C++) -dynamiclib -install_name $(notdir $@) -o $@ $(filter %.o,$^) -L$(DESTDIR)/lib -lcasa_components -lcasa_coordinates -lcasa_measures -lcasa_measures_f -lcasa_scimath -lcasa_scimath_f -lcasa_fits -lcasa_lattices -lcasa_tables -lcasa_casa -lcfitsio -lcasa_mirlib -lwcs -llapack -lblas -lcfitsio $(FC_LIB)
endif
ifeq "$(OS)" "linux"
	$(C++) -shared -Wl,-soname,$(notdir $@) -o $@ $(filter %.o,$^) -L$(DESTDIR)/lib -lcasa_components -lcasa_coordinates -lcasa_measures -lcasa_measures_f -lcasa_scimath -lcasa_scimath_f -lcasa_fits -lcasa_lattices -lcasa_tables -lcasa_casa -lcfitsio -lcasa_mirlib -lwcs -llapack -lblas -lcfitsio $(FC_LIB)
endif
	cd $(DESTDIR)/lib && ln -fs $(notdir $@) $(subst .$(VERSION),,$(notdir $@))

###
###  tables
###
tables: casa $(DESTDIR)/lib/libcasa_tables.$(SO)

$(DESTDIR)/lib/libcasa_tables.$(SO): $(TABLESLIB)
ifeq "$(OS)" "darwin"
	$(C++) -dynamiclib -install_name $(notdir $@) -o $@ $(filter %.o,$^) -L$(DESTDIR)/lib -lcasa_casa
endif
ifeq "$(OS)" "linux"
	$(C++) -shared -Wl,-soname,$(notdir $@) -o $@ $(filter %.o,$^) -L$(DESTDIR)/lib -lcasa_casa
endif
	cd $(DESTDIR)/lib && ln -fs $(notdir $@) $(subst .$(VERSION),,$(notdir $@))

###
###  scimath
###
scimath: casa $(DESTDIR)/lib/libcasa_scimath.$(SO)

$(DESTDIR)/lib/libcasa_scimath.$(SO): $(SCIMATHLIB) $(DESTDIR)/lib/libcasa_scimath_f.$(SO)
ifeq "$(OS)" "darwin"
	$(C++) -dynamiclib -install_name $(notdir $@) -o $@ $(filter %.o,$^) -L$(DESTDIR)/lib -lcasa_scimath_f -lcasa_casa -llapack -lblas $(FC_LIB)
endif
ifeq "$(OS)" "linux"
	$(C++) -shared -Wl,-soname,$(notdir $@) -o $@ $(filter %.o,$^) -L$(DESTDIR)/lib -lcasa_scimath_f -lcasa_casa -llapack -lblas $(FC_LIB)
endif
	cd $(DESTDIR)/lib && ln -fs $(notdir $@) $(subst .$(VERSION),,$(notdir $@))

$(DESTDIR)/lib/libcasa_scimath_f.$(SO): $(SCIMATHFLIB)
ifeq "$(OS)" "darwin"
	$(C++) -dynamiclib -install_name $(notdir $@) -o $@ $(filter %.o,$^) -L$(DESTDIR)/lib -lcasa_casa -llapack -lblas $(FC_LIB)
endif
ifeq "$(OS)" "linux"
	$(C++) -shared -Wl,-soname,$(notdir $@) -o $@ $(filter %.o,$^) -L$(DESTDIR)/lib -lcasa_casa -llapack -lblas $(FC_LIB)
endif
	cd $(DESTDIR)/lib && ln -fs $(notdir $@) $(subst .$(VERSION),,$(notdir $@))


###
###  measures
###
measures: scimath tables $(DESTDIR)/lib/libcasa_measures.$(SO)

$(DESTDIR)/lib/libcasa_measures.$(SO):  $(MEASURESLIB) $(DESTDIR)/lib/libcasa_measures_f.$(SO)
ifeq "$(OS)" "darwin"
	$(C++) -dynamiclib -install_name $(notdir $@) -o $@ $(filter %.o,$^) -L$(DESTDIR)/lib -lcasa_measures_f -lcasa_scimath -lcasa_scimath_f -lcasa_tables -lcasa_casa $(FC_LIB)
endif
ifeq "$(OS)" "linux"
	$(C++) -shared -Wl,-soname,$(notdir $@) -o $@ $(filter %.o,$^) -L$(DESTDIR)/lib -lcasa_measures_f -lcasa_scimath -lcasa_scimath_f -lcasa_tables -lcasa_casa $(FC_LIB)
endif
	cd $(DESTDIR)/lib && ln -fs $(notdir $@) $(subst .$(VERSION),,$(notdir $@))

$(DESTDIR)/lib/libcasa_measures_f.$(SO):  $(MEASURESFLIB)
ifeq "$(OS)" "darwin"
	$(C++) -dynamiclib -install_name $(notdir $@) -o $@ $(filter %.o,$^) -L$(DESTDIR)/lib -lcasa_scimath -lcasa_scimath_f -lcasa_tables -lcasa_casa $(FC_LIB)
endif
ifeq "$(OS)" "linux"
	$(C++) -shared -Wl,-soname,$(notdir $@) -o $@ $(filter %.o,$^) -L$(DESTDIR)/lib -lcasa_scimath -lcasa_scimath_f -lcasa_tables -lcasa_casa $(FC_LIB)
endif
	cd $(DESTDIR)/lib && ln -fs $(notdir $@) $(subst .$(VERSION),,$(notdir $@))

###
###  ms
###
ms: measures tables $(DESTDIR)/lib/libcasa_ms.$(SO)

$(DESTDIR)/lib/libcasa_ms.$(SO): $(MSLIB)
ifeq "$(OS)" "darwin"
	$(C++) -dynamiclib -install_name $(notdir $@) -o $@ $(filter %.o,$^) -L$(DESTDIR)/lib -lcasa_measures -lcasa_measures_f -lcasa_tables -lcasa_scimath -lcasa_scimath_f -lcasa_casa -llapack -lblas $(FC_LIB)
endif
ifeq "$(OS)" "linux"
	$(C++) -shared -Wl,-soname,$(notdir $@) -o $@ $(filter %.o,$^) -L$(DESTDIR)/lib -lcasa_measures -lcasa_measures_f -lcasa_tables -lcasa_scimath -lcasa_scimath_f -lcasa_casa -llapack -lblas $(FC_LIB)
endif
	cd $(DESTDIR)/lib && ln -fs $(notdir $@) $(subst .$(VERSION),,$(notdir $@))


###
###  fits
###
fits: scimath tables $(DESTDIR)/lib/libcasa_fits.$(SO)

$(DESTDIR)/lib/libcasa_fits.$(SO): $(FITSLIB)
ifeq "$(OS)" "darwin"
	$(C++) -dynamiclib -install_name $(notdir $@) -o $@ $(filter %.o,$^) -L$(DESTDIR)/lib -lcasa_measures -lcasa_measures_f -lcasa_tables -lcasa_casa -lcfitsio $(FC_LIB)
endif
ifeq "$(OS)" "linux"
	$(C++) -shared -Wl,-soname,$(notdir $@) -o $@ $(filter %.o,$^) -L$(DESTDIR)/lib -lcasa_measures -lcasa_measures_f -lcasa_tables -lcasa_casa -lcfitsio $(FC_LIB)
endif
	cd $(DESTDIR)/lib && ln -fs $(notdir $@) $(subst .$(VERSION),,$(notdir $@))

###
###  msfits
###
msfits: ms fits $(DESTDIR)/lib/libcasa_msfits.$(SO)

$(DESTDIR)/lib/libcasa_msfits.$(SO): $(MSFITSLIB)
ifeq "$(OS)" "darwin"
	$(C++) -dynamiclib -install_name $(notdir $@) -o $@ $(filter %.o,$^) -L$(DESTDIR)/lib -lcasa_ms -lcasa_measures -lcasa_measures_f -lcasa_tables -lcasa_fits -lcasa_casa -lcfitsio -llapack -lblas $(FC_LIB)
endif
ifeq "$(OS)" "linux"
	$(C++) -shared -Wl,-soname,$(notdir $@) -o $@ $(filter %.o,$^) -L$(DESTDIR)/lib -lcasa_ms -lcasa_measures -lcasa_measures_f -lcasa_tables -lcasa_fits -lcasa_casa -lcfitsio -llapack -lblas $(FC_LIB)
endif
	cd $(DESTDIR)/lib && ln -fs $(notdir $@) $(subst .$(VERSION),,$(notdir $@))


###
###  mirlib
###
mirlib: $(DESTDIR)/lib/libcasa_mirlib.$(SO)

$(DESTDIR)/lib/libcasa_mirlib.$(SO): $(MIRLIB)
ifeq "$(OS)" "darwin"
	$(C++) -dynamiclib -install_name $(notdir $@) -o $@ $(filter %.o,$^)
endif
ifeq "$(OS)" "linux"
	$(C++) -shared -Wl,-soname,$(notdir $@) -o $@ $(filter %.o,$^)
endif
	cd $(DESTDIR)/lib && ln -fs $(notdir $@) $(subst .$(VERSION),,$(notdir $@))

clean:
	rm -f `find . -type f -name '*.lcc'` `find . -type f -name '*.ycc'`

setup:
	mkdir -p $(DESTDIR)/lib
	mkdir -p $(DESTDIR)/include/casacore
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

