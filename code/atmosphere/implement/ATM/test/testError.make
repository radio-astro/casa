# This is a makefile allowing to rebuild C++ gipsy functions for an installation using the ACS setup
# Without this I have problems at the link when using the vector class (but no problem at the compilation)
# Le C++_OPTS doit pouvoir etre aussi simple que dans Axis.make (a simplifier donc ici!)
# J'ai garde le minimum necessaire de ACS_LIB et acs_lib compare a tstc2sdm.make pr que ca link!


Name  = Error

Main  =  test${Name}.cc
Ccsrc =  ${Name}.cc

C++_COMP  = g++ -D__cplusplus__
C++_OPTS  = -I. -I.. -I- -I/usr/include
LD        = g77

# library search paths for ACS in general
ACS_LIB   = -L${ACS_ROOT}/ACSSW/lib \
            -L/usr/X11R6/lib \
            -L${ACS_ROOT}/gnu/lib \
            -L/usr/local/lib

#  libraries for ACS in general
acs_lib   = -lmaci

CC_OPTS   = -D_BSD_SOURCE -Dntohl=ntohl -D__g77__=2 -I. -I$gip_inc -I- -I/usr/include -ansi -pedantic -Wall

C++_OPTS  = -D_POSIX_THREADS -D_POSIX_THREAD_SAFE_FUNCTIONS -D_REENTRANT \
            -fcheck-new  -fPIC   -DCCS_LIGHT \
            -fno-operator-names -fpermissive \
            -I. -I${gip_inc} -I- -I/usr/include

#----------------------------------------------------------------------------
# There should be nothing to edit beyond this line
#----------------------------------------------------------------------------

Ccobj = ${Ccsrc:.cc=.o} ${Main:.cc=.o}
Ccinc = ${Ccsrc:.cc=.h}

.SUFFIXES: .o .cc

.cc.o:
	@echo "Compilation of $<"
	$(C++_COMP) -D__cplusplus__ -c ${C++_OPTS}  $<


test${Name}: ${CCinc} ${Ccobj}
	@echo "Link"
	${LD} -o ${Name} $(Ccobj) ${gip_lib}/giplib.a \
	${ACS_LIB} ${acs_lib} \
	-lm  \
	-lstdc++ \
	-L/usr/X11R6/lib \
	-lgcc_s -lc
	${Name}
