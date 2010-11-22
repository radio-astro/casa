### Notice: 
###    This Makefile assumes CASA+CASACore enviroment. 
###    For who wants to build ASAP with older CASA without CASACore 
###    environment, replace $(CASAPATH) with $(AIPSPATH). 
EXTDIR := $(shell pwd)
ATNFDIR := $(EXTDIR)/atnf

# the casa environment CASAPATH has to be defined
CASAROOT  := $(word 1, $(CASAPATH))
CASAARCH  := $(word 2, $(CASAPATH))

CXXFLAGS := $(shell $(shell make -s -f $(CASAROOT)/$(CASAARCH)/makedefs VARS=CXXFLAGS eval_vars) && echo $$CXXFLAGS)
CXXOPTS := $(shell $(shell make -s -f $(CASAROOT)/$(CASAARCH)/makedefs VARS=COPTFLAGS eval_vars) && echo $$COPTFLAGS)
LDFLAGS := $(shell $(shell make -s -f $(CASAROOT)/$(CASAARCH)/makedefs VARS=LD_FLAGS eval_vars) && echo $$LD_FLAGS)
COREINCD := $(shell $(shell make -s -f $(CASAROOT)/$(CASAARCH)/makedefs VARS=COREINCD eval_vars) && echo $$COREINCD)
CORELIB := $(shell $(shell make -s -f $(CASAROOT)/$(CASAARCH)/makedefs VARS=CORELIB eval_vars) && echo $$CORELIB)
WCSLIBLIB := $(shell $(shell make -s -f $(CASAROOT)/$(CASAARCH)/makedefs VARS=WCSLIBLIB eval_vars) && echo $$WCSLIBLIB)
CFITSIOLIBD := $(shell $(shell make -s -f $(CASAROOT)/$(CASAARCH)/makedefs VARS=CFITSIOLIBD eval_vars) && echo $$CFITSIOLIBD)
CFITSIOINCD := $(shell $(shell make -s -f $(CASAROOT)/$(CASAARCH)/makedefs VARS=CFITSIOINCD eval_vars) && echo $$CFITSIOINCD)
RPFITSLIBD := $(shell $(shell make -s -f $(CASAROOT)/$(CASAARCH)/makedefs VARS=RPFITSLIBD eval_vars) && echo $$RPFITSLIBD)

CASAINC   := -I$(CASAROOT)/code/include -I$(COREINCD) -I$(CASAROOT)/$(CASAARCH)/include
CASALIB   := $(CASAROOT)/$(CASAARCH)/lib

CASALIB   := $(CASAROOT)/$(CASAARCH)/lib
#USELIB := lib64

ifeq ($(shell uname), Darwin)
	TARGETLIB   := libatnf.dylib
else
        TARGETLIB   := libatnf.so
endif

# the compiler
ifndef CXX
   CXX      := g++
endif

# compiler flags
CXXFLAGS += -fPIC

# cfitsio
CFITSIOLIB := -lcfitsio

#rpfits
RPFITSLIB := -lrpfits

# the linker
LD        := $(CXX)

LDFLAGS   += -shared
ifeq ($(shell uname), Darwin)
	   LDFLAGS += -headerpad_max_install_names -install_name $(CASAROOT)/$(CASAARCH)/lib/$(TARGETLIB)
endif 

INCDIRS   := -I$(COREINCD) -I$(CFITSIOINCD) -I$(EXTDIR)
LIBS      := -L$(CFITSIOLIBD) $(CFITSIOLIB) -L$(RPFITSLIBD) $(RPFITSLIB) -L$(CASALIB) $(CORELIB) -lgfortran

OBJECTS := $(ATNFDIR)/pks/pks_maths.o \
	$(ATNFDIR)/PKSIO/ASTEDataset.o \
	$(ATNFDIR)/PKSIO/ASTEFXDataset.o \
	$(ATNFDIR)/PKSIO/ASTEFXReader.o \
	$(ATNFDIR)/PKSIO/ASTEReader.o \
	$(ATNFDIR)/PKSIO/FITSreader.o \
	$(ATNFDIR)/PKSIO/GBTFITSreader.o \
	$(ATNFDIR)/PKSIO/MBFITSreader.o \
	$(ATNFDIR)/PKSIO/MBrecord.o \
	$(ATNFDIR)/PKSIO/NRO45FITSReader.o \
	$(ATNFDIR)/PKSIO/NRO45Reader.o \
	$(ATNFDIR)/PKSIO/NRODataset.o \
	$(ATNFDIR)/PKSIO/NROFITSDataset.o \
	$(ATNFDIR)/PKSIO/NROOTFDataset.o \
	$(ATNFDIR)/PKSIO/NROReader.o \
	$(ATNFDIR)/PKSIO/PKSFITSreader.o \
	$(ATNFDIR)/PKSIO/PKSMS2reader.o \
	$(ATNFDIR)/PKSIO/PKSMS2writer.o \
	$(ATNFDIR)/PKSIO/PKSreader.o \
	$(ATNFDIR)/PKSIO/PKSSDwriter.o \
	$(ATNFDIR)/PKSIO/SDFITSreader.o \
	$(ATNFDIR)/PKSIO/SDFITSwriter.o #\
#	$(ATNFDIR)/PKSIO/PKSmsg.o

HEADERS := $(ATNFDIR)/pks/pks_maths.h \
	$(ATNFDIR)/PKSIO/ASTEDataset.h \
	$(ATNFDIR)/PKSIO/ASTEFXDataset.h \
	$(ATNFDIR)/PKSIO/ASTEFXReader.h \
	$(ATNFDIR)/PKSIO/ASTEReader.h \
	$(ATNFDIR)/PKSIO/FITSreader.h \
	$(ATNFDIR)/PKSIO/GBTFITSreader.h \
	$(ATNFDIR)/PKSIO/MBFITSreader.h \
	$(ATNFDIR)/PKSIO/MBrecord.h \
	$(ATNFDIR)/PKSIO/NRO45FITSReader.h \
	$(ATNFDIR)/PKSIO/NRO45Reader.h \
	$(ATNFDIR)/PKSIO/NRODataRecord.h \
	$(ATNFDIR)/PKSIO/NRODataset.h \
	$(ATNFDIR)/PKSIO/NROFITSDataset.h \
	$(ATNFDIR)/PKSIO/NROOTFDataset.h \
	$(ATNFDIR)/PKSIO/NROReader.h \
	$(ATNFDIR)/PKSIO/PKSFITSreader.h \
	$(ATNFDIR)/PKSIO/PKSMS2reader.h \
	$(ATNFDIR)/PKSIO/PKSMS2writer.h \
	$(ATNFDIR)/PKSIO/PKSreader.h \
	$(ATNFDIR)/PKSIO/PKSrecord.h \
	$(ATNFDIR)/PKSIO/PKSSDwriter.h \
	$(ATNFDIR)/PKSIO/PKSwriter.h \
	$(ATNFDIR)/PKSIO/SDFITSreader.h \
	$(ATNFDIR)/PKSIO/SDFITSwriter.h \
	$(ATNFDIR)/PKSIO/SrcType.h #\
#	$(ATNFDIR)/PKSIO/PKSmsg.h

.SUFFIXES: .o .cc
.cc.o:
	-@ echo
	  $(CXX) $(CXXOPTS) -c $(CXXFLAGS) $(INCDIRS) -o $@ $<

.PHONY : all clean objects realclean install

all : $(TARGETLIB)

objects : $(OBJECTS)

$(TARGETLIB) : $(OBJECTS)
	-@ echo
	  $(LD) $(CXXFLAGS) $(LDFLAGS) -o $(TARGETLIB) $(OBJECTS) $(LIBS)

$(OBJECTS) : $(HEADERS)

install : 
	@if ( test -f $(TARGETLIB) ) ; then cp -f $(TARGETLIB) $(CASALIB)/. ; fi

clean :
	-rm -f $(OBJECTS)

cleanso :
	-rm -f $(TARGETLIB)

realclean : clean cleanso

