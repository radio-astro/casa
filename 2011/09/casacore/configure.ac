AC_INIT([casacore], [0.4], [casacore@maintainer.org])

AM_INIT_AUTOMAKE([-Wall -Werror foreign nostdinc])
# nostdinc removes -I. from the include paths, in order to avoid
# picking up casacore's versions of stdlib.h (etc) instead of the system
# ones. 
# (as a workaround for casacore's redefinition of standard headers, it shouldn't)

AC_CONFIG_MACRO_DIR([m4])
AC_PROG_LIBTOOL
AC_PROG_CC
AC_PROG_CXX
AM_PROG_LEX
AC_PROG_YACC

# Use the c++ compiler in the following tests for external libraries
AC_LANG([C++])

CASA_CHECK_LIB(wcslib,
              -lwcs,
              [#include <wcslib/wcs.h>
               #include <wcslib/wcsconfig.h>],
              WCSLIB_VERSION,
              4.3)
CASA_CHECK_LIB(cfitsio,
              -lcfitsio,
              [#include <fitsio.h>
               #include <fitsio2.h>],
              CFITSIO_VERSION,
              3.14)
CASA_CHECK_LIB_STR(fftw,
              -lfftw3 -lfftw3_threads -lfftw3f -lfftw3f_threads,
              [#include <fftw3.h>],
              fftw_version,
              3.1.2)

AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([
                 Makefile
                 mirlib/Makefile
                 casa/casa/Makefile
                 tables/tables/Makefile
                 scimath/scimath/Makefile
                 measures/measures/Makefile
                 fits/fits/Makefile
                 coordinates/coordinates/Makefile
                 components/components/Makefile
                 lattices/lattices/Makefile
                 images/images/Makefile
                 ms/ms/Makefile
                 msfits/msfits/Makefile
                 tables/tables/Tables/Makefile
                 images/images/Images/Makefile
                 ms/ms/MeasurementSets/Makefile
                ])
AC_OUTPUT
