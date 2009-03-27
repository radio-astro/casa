### Notice: 
###    This Makefile assumes CASA+CASACore enviroment. 
###    For who wants to build ASAP with older CASA without CASACore, 
###    1. Replace $(CASAPATH) with $(AIPSPATH). 
###    2. CASAINC should be 
###        '-I$(CASAROOT)/code/include -I$(CASAROOT)/code/casa'
###    3. Replase library names in G2CARCH '-lcasa_*' with '-l*'.
###    4. Comment-IN the definition of 'CXXOPTS'. 
TARGET   := /tmp/_asap.so

# the casa environment AIPSPATH has to be defined
CASAROOT  := $(word 1, $(CASAPATH))
CASAARCH  := $(word 2, $(CASAPATH))
CASAINC   := -I$(CASAROOT)/code/include -I$(CASAROOT)/$(CASAARCH)/include/casacore -I$(CASAROOT)/$(CASAARCH)/include
CASALIB   := $(CASAROOT)/$(CASAARCH)/lib
THIRDPARTY = /opt/casa/core2-apple-darwin8/3rd-party


# the compiler
ifndef CXX
   CXX      := g++ 
endif

# compiler flags
CXXFLAGS := -fPIC -O2 -g -DCASA_USECASAPATH
CXXFLAGS += -ansi -Wall

# darwin specific flags
ifeq "$(CASAARCH)" "darwin"
   CXXFLAGS +=-DAIPS_DARWIN
endif

# cfitsio
CFITSIOROOT := $(THIRDPARTY)
CFITSIOINC := -I$(CFITSIOROOT)/include/cfitsio
#CFITSIOLIB := -L$(CFITSIOROOT)/lib -lcfitsio
CFITSIOLIB := -lcfitsio

#rpfits
RPFITSROOT := $(THIRDPARTY)
#RPFITSLIB := $(RPFITSROOT)/lib/librpfits.a
RPFITSLIB := -lrpfits

G2CROOT := /usr
G2CARCH := $(G2CROOT)/lib/gcc/i386-apple-darwin8.7.1/4.2.0/libgcc.a
G2CARCH := $(G2CROOT)/lib/gcc/powerpc-apple-darwin8.7.0/4.2.0/libgcc.a
G2CARCH := $(G2CROOT)/lib/gcc/i386-redhat-linux/4.1.0/libgcc.a
#G2CLIB := $(G2CROOT)/lib/libgfortran.a
G2CLIB := -lgfortran

# This assumes all casa libs are static only (*.a)
# if not than there might by symbol resolution errors.
CASAPPLIB := -L$(CASALIB) \
             -latnf -lcasa_images -lcasa_ms -lcasa_components -lcasa_coordinates \
             -lcasa_lattices -lcasa_fits -lcasa_measures -lcasa_measures_f \
             -lcasa_tables -lcasa_scimath -lcasa_scimath_f -lcasa_casa  \
             $(CASALIB)/libwcs.a \
             $(RPFITSLIB) $(CFITSIOLIB) $(G2CLIB) -lstdc++

# darwin specific CASA flags
ifeq "$(CASAARCH)" "darwin"
   CASAPPLIB += -framework vecLib
else
   CASAPPLIB += -llapack -lblas
endif

# python
PYVERSION := 2.5
PYTHONROOT := /usr/lib/casapy
PYTHONINC := -I$(PYTHONROOT)/include/python$(PYVERSION)
PYTHONLIB := -L$(PYTHONROOT)/lib -lpython$(PYVERSION)

# use python universal build
ifeq "$(CASAARCH)" "darwin"
   PYVERSION := 2.5
   PYTHONROOT := $(THIRDPARTY)/lib/Python.framework/Versions/$(PYVERSION)
   PYTHONINC := -I$(PYTHONROOT)/include/python$(PYVERSION)
   PYTHONLIB := -F$(THIRDPARTY)/lib -framework Python
endif

# the linker
LD        := $(CXX)

# has to be build with same g++ version as casa
#BOOSTROOT := /home/casa/boost
BOOSTROOT := $(THIRDPARTY)
#BOOSTLIB  := -L$(BOOSTROOT)/lib -lboost_python-gcc34
BOOSTLIB  := -L$(BOOSTROOT)/lib -lboost_python-mt-1_35
BOOSTINC  := -I$(BOOSTROOT)/include/boost-1_35
#BOOSTINC  := -I$(BOOSTROOT)/include/boost

LDFLAGS   := -shared -Wl$(TARGET) -s -Xlinker -rpath -Xlinker $(BOOSTROOT)/lib:$(PYTHONROOT)/lib:$(CASALIB)
ifeq "$(CASAARCH)" "darwin"
   LDFLAGS   := -bundle
endif


# DO NOT MODIFY AFTER THIS
INCDIRS   := -I. $(CASAINC)  $(CFITSIOINC) $(BOOSTINC) $(PYTHONINC)
LIBS      := $(PYTHONLIB)  $(BOOSTLIB) $(CASAPPLIB)

CCOBJECTS := MathUtils.o \
             RowAccumulator.o \
             Logger.o \
	     LineCatalog.o \
             STAttr.o \
             STHeader.o \
             STFiller.o \
             STMath.o \
             STSubTable.o \
             STMolecules.o \
             STTcal.o \
             STFocus.o \
             STWeather.o \
             STFrequencies.o \
             STHistory.o \
             STSelector.o \
	     STLineFinder.o \
             STFitter.o \
             STFitEntry.o \
             STFit.o \
             STPol.o \
             STPolLinear.o \
             STPolCircular.o \
             STPolStokes.o \
             STWriter.o \
             STAsciiWriter.o \
             Scantable.o
PYOBJECTS := python_Scantable.o \
             python_STFiller.o \
             python_STSelector.o \
             python_STMath.o \
             python_Fitter.o \
             python_Logger.o \
             python_STLineFinder.o \
             python_STFitEntry.o \
             python_STWriter.o \
             python_LineCatalog.o \
	     python_asap.o

OBJECTS := $(CCOBJECTS) $(PYOBJECTS)

HEADERS   := MathUtils.h \
             Logger.h \
             STAttr.h \
             STHeader.h \
             Scantable.h \
             STFiller.h \
             STMath.h \
             RowAccumulator.h \
             STSubTable.h \
             STMolecules.h \
             STTcal.h \
             STFocus.h \
             STWeather.h \
             STHistory.h \
             STFrequencies.h \
             STDefs.h \
             STWriter.h \
             STSelector.h \
	     STFitter.h \
	     STFit.h \
	     STFitEntry.h \
	     STLineFinder.h \
             Factory.h \
             FactoryBase.h \
	     STPol.h \
	     STPolStokes.h \
	     STPolCircular.h \
	     STPolLinear.h \
	     STWriter.h \
	     STAsciiWriter.h

STATICCCLIB := libasap.a
MEMBERS := $(patsubst %,$(STATICCCLIB)(%),$(CCOBJECTS))

.SUFFIXES: .c .o .cpp .cc .cxx .C

.cpp.o:
	-@ echo
	  $(CXX) $(CXXOPTS) -c $(CXXFLAGS) $(INCDIRS) -o $@ $<

.cc.o:
	-@ echo
	  $(CXX) $(CXXOPTS) -c $(CXXFLAGS) $(INCDIRS) -o $@ $<

.cxx.o:
	-@ echo
	  $(CXX) $(CXXOPTS) -c $(CXXFLAGS) $(INCDIRS) -o $@ $<

.C.o:
	-@ echo
	  $(CXX) $(CXXOPTS) -c $(CXXFLAGS) $(INCDIRS) -o $@ $<


.PHONY : all clean objects realclean relink

all : $(TARGET)

relink : cleanso $(TARGET)

objects : $(OBJECTS)

$(TARGET) : $(OBJECTS)
	-@ echo
	  $(LD) $(LDFLAGS) -o $(TARGET) $(OBJECTS) $(LIBS)

$(OBJECTS) : $(HEADERS)

staticlib: $(MEMBERS)

clean :
	-rm -f $(OBJECTS)

cleanso :
	-rm -f $(TARGET)

realclean : clean cleanso

test: objects tasap2.o
	-@ echo
	  $(LD) -g -o /tmp/tasap2 $(OBJECTS) $(LIBS) tasap2.o
