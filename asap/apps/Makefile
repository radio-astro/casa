### Notice: 
###    This Makefile assumes CASA+CASACore enviroment. 
###    For who wants to build ASAP with older CASA without CASACore 
###    environment, replace $(CASAPATH) with $(AIPSPATH). 

# the casa environment AIPSPATH has to be defined
CASAROOT  := $(word 1, $(CASAPATH))
CASAARCH  := $(word 2, $(CASAPATH))

COREINCD := $(shell $(shell make -s -f $(CASAROOT)/$(CASAARCH)/makedefs VARS=COREINCD eval_vars) && echo $$COREINCD)
CORELIB := $(shell $(shell make -s -f $(CASAROOT)/$(CASAARCH)/makedefs VARS=CORELIB eval_vars) && echo $$CORELIB)
CASALIB   := $(CASAROOT)/$(CASAARCH)/lib

# the compiler
ifndef CXX
   CXX      := g++
endif

# compiler flags
CXXFLAGS := -fPIC -O3 -g
CXXFLAGS += -ansi -Wno-long-long -Wall
CXXFLAGS := -Xlinker -rpath -Xlinker $(CASALIB)


INCDIRS   := -I$(COREINCD)
LIBS      := -L$(CASALIB) $(CORELIB)

OBJECTS := asap2to3 

.SUFFIXES: .cpp

.cpp:
	$(CXX) $(CXXFLAGS) $(INCDIRS) $(LIBS) -o $@ $<

all : $(OBJECTS)

$(OBJECTS):

clean :
	-rm -f $(OBJECTS)
