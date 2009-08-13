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

COREINCD := $(shell $(shell make -s -f $(CASAROOT)/$(CASAARCH)/makedefs VARS=COREINCD eval_vars) && echo $$COREINCD)
CORELIB := $(shell $(shell make -s -f $(CASAROOT)/$(CASAARCH)/makedefs VARS=CORELIB eval_vars) && echo $$CORELIB)
WCSLIBLIBD := $(shell $(shell make -s -f $(CASAROOT)/$(CASAARCH)/makedefs VARS=WCSLIBLIBD eval_vars) && echo $$WCSLIBLIBD)
WCSLIBLIB := $(shell $(shell make -s -f $(CASAROOT)/$(CASAARCH)/makedefs VARS=WCSLIBLIB eval_vars) && echo $$WCSLIBLIB)
PYTHONROOT := $(shell $(shell make -s -f $(CASAROOT)/$(CASAARCH)/makedefs VARS=PYTHONROOT eval_vars) && echo $$PYTHONROOT)
PYTHONLIBD := $(shell $(shell make -s -f $(CASAROOT)/$(CASAARCH)/makedefs VARS=PYTHONLIBD eval_vars) && echo $$PYTHONLIBD)
PYTHONINCD := $(shell $(shell make -s -f $(CASAROOT)/$(CASAARCH)/makedefs VARS=PYTHONINCD eval_vars) && echo $$PYTHONINCD)
PYTHONLIB := $(shell $(shell make -s -f $(CASAROOT)/$(CASAARCH)/makedefs VARS=PYTHONLIB eval_vars) && echo $$PYTHONLIB)
BOOSTROOT := $(shell $(shell make -s -f $(CASAROOT)/$(CASAARCH)/makedefs VARS=BOOSTROOT eval_vars) && echo $$BOOSTROOT)
RPFITSLIBD := $(shell $(shell make -s -f $(CASAROOT)/$(CASAARCH)/makedefs VARS=RPFITSLIBD eval_vars) && echo $$RPFITSLIBD)

CASAINC   := -I$(CASAROOT)/code/include -I$(COREINCD) -I$(CASAROOT)/$(CASAARCH)/include
CASALIB   := $(CASAROOT)/$(CASAARCH)/lib
USELIB := lib

# the compiler
ifndef CXX
   CXX      := g++
endif

# compiler flags
CXXFLAGS := -fPIC -O3 -g
CXXFLAGS += -ansi -Wno-long-long -Wall

# darwin specific flags
ifeq "$(CASAARCH)" "darwin"
   CXXFLAGS +=-DAIPS_DARWIN
endif

# cfitsio
CFITSIOROOT := /usr
CFITSIOINC := -I$(CFITSIOROOT)/include/cfitsio
CFITSIOLIB := -lcfitsio

#rpfits
RPFITSLIB := -lrpfits

G2CROOT := /usr
G2CLIB := -lgfortran

# This assumes all casa libs are static only (*.a)
# if not than there might by symbol resolution errors.
CASAPPLIB := -L$(CASALIB) -latnf $(CORELIB) \
             -L$(WCSLIBLIBD) $(WCSLIBLIB) \
             -L$(RPFITSLIBD) $(RPFITSLIB) $(CFITSIOLIB) $(G2CLIB) -lstdc++

# darwin specific CASA flags
ifeq "$(CASAARCH)" "darwin"
   CASAPPLIB += -framework vecLib
else
   CASAPPLIB += -llapack -lblas
endif

# python
PYTHONINC := $(addprefix -I,$(PYTHONINCD))
PYTHONLIB := -L$(PYTHONLIBD) $(PYTHONLIB)

# use python universal build
ifeq "$(CASAARCH)" "darwin"
   PYVERSION := 2.5
   #PYTHONROOT := /Library/Frameworks/Python.framework/Versions/$(PYVERSION)
   PYTHONINC := -I$(PYTHONROOT)/include/python$(PYVERSION)
   #PYTHONLIB := -F$(PYTHONROOT)/lib -framework Python
endif

# the linker
LD        := $(CXX)

# has to be build with same g++ version as casa
BOOSTLIB  := -L$(BOOSTROOT)/lib -lboost_python
BOOSTINC  := -I$(BOOSTROOT)/include/boost -I$(BOOSTROOT)/include

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
             STFITSImageWriter.o \
             Scantable.o \
             Templates.o 
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
	     STAsciiWriter.h \
             STFITSImageWriter.h

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
