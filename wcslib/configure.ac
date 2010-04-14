#-----------------------------------------------------------------------------
# Process this file with autoconf-2.53 or later to produce a configure script.
#-----------------------------------------------------------------------------
# Author: Mark Calabretta, Australia Telescope National Facility
# http://www.atnf.csiro.au/~mcalabre/index.html
# $Id: configure.ac,v 4.3 2007/12/27 05:57:06 cal103 Exp $
#-----------------------------------------------------------------------------

AC_INIT([WCSLIB], [4.3], [mcalabre@atnf.csiro.au], [wcslib-4.3])
AC_PREREQ([2.53])
AC_REVISION([$Revision: 4.3 $])
AC_SUBST([PACKAGE_VERSION])
AC_DEFINE_UNQUOTED([WCSLIB_VERSION], [$PACKAGE_VERSION])

AC_CONFIG_SRCDIR([C/wcs.h])
AC_CONFIG_AUX_DIR([config])

# Get the system type.
AC_CANONICAL_BUILD
ARCH=${build_cpu}-$build_os
AC_SUBST([ARCH])


# Look for Flex.
AC_CHECK_PROG([FLEX], [flex], [flex], [], [], [])
if test "x$FLEX" = xflex ; then
  # Version 2.5.9 or later is required.
  V=`flex --version | awk '{print $NF}'`
  W=`echo $V | awk -F. '{if ((($1*100 + $2)*100 + $3) < 20509) print "no"}'`
  if test "x$W" != x ; then
    AC_MSG_NOTICE([Flex version $V is too old, ignored.])
    FLEX=
  else
    AC_MSG_NOTICE([Using Flex version $V.])
  fi
fi

if test "x$FLEX" = x ; then
  AC_MSG_WARN([Flex version 2.5.9 or later does not appear to be
           available, will use pre-generated sources.])
fi


# Look for an ANSI C compiler.
AC_PROG_CPP
AC_PROG_CC
AC_PROG_CC_STDC
AC_C_CONST
AC_TYPE_SIZE_T
if test "x$ac_cv_prog_cc_stdc" = xno -o \
        "x$ac_cv_c_const"      = xno -o \
        "x$ac_cv_type_size_t"  = xno; then
  AC_MSG_ERROR([
    -------------------------------------------------------
    An ANSI standard C library is required to build WCSLIB.

    ERROR: WCSLIB configuration failure.
    -------------------------------------------------------], [1])

fi

# Check for ANSI C headers.
AC_HEADER_STDC
AC_CHECK_HEADERS([errno.h limits.h math.h stdio.h stdlib.h string.h])
if test "x$ac_cv_header_stdc" = xno; then
  AC_MSG_ERROR([
    -------------------------------------------------------------------
    An ANSI standard C library is required to build WCSLIB.  One of the
    ANSI C header files it requires is missing or unusable.

    ERROR: WCSLIB configuration failure.
    -------------------------------------------------------------------], [1])
fi

# Checks for ANSI C library functions (suggested by autoscan).
AC_CHECK_LIB([m], [floor])
AC_FUNC_MALLOC
AC_CHECK_FUNCS([floor memset pow sqrt strchr strstr])

# Check the size and availability of integer data types.
AC_CHECK_SIZEOF([int])
AC_CHECK_SIZEOF([long int])
AC_CHECK_SIZEOF([long long int])

# 64-bit integer data type; use long long int preferentially since that
# accords with "%lld" formatting used in fitshdr.l, e.g.
#                int   size_t  long int  long long int
#                ---   ------  --------  -------------
#   gcc x86:      32     32       32          64
#   gcc x86_64:   32     64       64          64
if test "x$ac_cv_sizeof_long_long_int" = x8; then
  AC_DEFINE([WCSLIB_INT64], [long long int], [64-bit integer data type.])
elif test "x$ac_cv_sizeof_long_int" = x8; then
  AC_DEFINE([WCSLIB_INT64], [long int], [64-bit integer data type.])
elif test "x$ac_cv_sizeof_int" = x8; then
  AC_DEFINE([WCSLIB_INT64], [int], [64-bit integer data type.])
fi

# Look for a Fortran compiler.
AC_PROG_F77([g77 f77 gfortran ifort xlf frt pgf77 fl32 af77 fort77 f90 xlf90 \
             pgf90 epcf90 f95 fort xlf95 lf95 g95])

if test "x$F77" = x; then
  AC_MSG_WARN([
    ---------------------------------------------------------------------
    Fortran compiler not found, will skip PGSBOX and Fortran test suite.
    ---------------------------------------------------------------------])

  # Best guess at Fortran name mangling for use if a compiler does ever become
  # available.
  AC_DEFINE([F77_FUNC(name,NAME)], [name ## _])

else
  if test "x$ac_cv_f77_compiler_gnu" = xyes ; then
    # Beware of gfortran!
    if test "x$F77" = xg77 -o "x$F77" = xf77 ; then
      FFLAGS="$FFLAGS -Wno-globals"
    fi
  fi

  AC_MSG_CHECKING(whether $F77 accepts -I)
  AC_LANG_PUSH(Fortran 77)
  FFLAGS_save=$FFLAGS
  FFLAGS=-I.
  AC_TRY_COMPILE([], [], [FFLAGS="$FFLAGS_save -I."; AC_MSG_RESULT(yes)],
    [FFLAGS="$FFLAGS_save"; AC_MSG_RESULT(no)])
  AC_LANG_POP()

  # Libraries required by the Fortran compiler itself (sets FLIBS).  Required
  # by utilities and test programs written in C that link to the pgsbox object
  # library - this contains the only Fortran modules in the WCSLIB suite.
  AC_F77_LIBRARY_LDFLAGS

  # F77 name mangling (defines the F77_FUNC preprocessor macro).
  AC_F77_WRAPPERS
fi


# System-dependent system libraries (for building the sharable library).
#-----------------------------------------------------------------------
# Darwin (contains stubs for long double).
AC_CHECK_LIB([SystemStubs], [printf\$LDBLStub], [LIBS="$LIBS -lSystemStubs"],
             [], [])


# Library and installation utilities.
#------------------------------------
# Static library generation.
AC_PROG_RANLIB

# Shared library generation.
if test "x$ac_cv_c_compiler_gnu" = xyes ; then
  # Note that -fPIC is on by default for Macs, this just makes it obvious.
  SHRFLAGS="$CFLAGS -fPIC"
  SHRLD="\$(CC) \$(SHRFLAGS)"

  V=$PACKAGE_VERSION
  case "$build_os" in
  darwin*)
    SHRLIB=libwcs.$V.dylib
    SHRLD="$SHRLD -dynamiclib -single_module"
    SHRLD="$SHRLD -compatibility_version $V -current_version $V"
    SHRLN=

    case "$build_cpu" in
    powerpc*)
      # Switch off -fPIC (not applicable for Intel Macs).
      CFLAGS="$CFLAGS -mdynamic-no-pic"
      ;;
    esac
    ;;
  *)
    # Covers Linux and Solaris at least.
    SHRLIB=libwcs.so.$V
    SHRLD="$SHRLD -shared -Wl,-h\$(SHRLIB)"
    SHRLN=libwcs.so
    ;;
  esac

else
  SHRLIB=
  SHRFLAGS=
  SHRLD=
  SHRSFX=
  SHRLN=
fi

AC_SUBST([SHRLIB])
AC_SUBST([SHRFLAGS])
AC_SUBST([SHRLD])
AC_SUBST([SHRSFX])
AC_SUBST([SHRLN])

# Installation utilities.
AC_PROG_LN_S
AC_PROG_INSTALL


# The following are required to build utilities and test programs.
# ----------------------------------------------------------------
AC_MSG_NOTICE([

Looking for libraries etc. required to build utilities and test suite...])

# Check for other quasi-standard header files.
AC_CHECK_HEADERS([unistd.h])

# Large file support.
AC_FUNC_FSEEKO
AC_SYS_LARGEFILE


# System libraries that may be required by WCSLIB itself.
# SunOS, extra maths functions.
AC_CHECK_LIB([sunmath], [cosd], [LIBS="-lsunmath $LIBS"], [], [])


# Extra places to look for third-party libraries.
LIBDIRS="/local/pgplot      \
         /local/cfitsio     \
         /local/lib         \
         /usr/local/pgplot  \
         /usr/local/cfitsio \
         /usr/local/lib     \
         /opt/local/lib     \
         /opt/SUNWspro/lib  \
         /sw/lib"

for LIBDIR in $LIBDIRS ; do
  AC_CHECK_FILE([$LIBDIR], [LDFLAGS="$LDFLAGS -L$LIBDIR"], [continue])
done

# Extra places to look for third-party include files.
INCDIRS="/local/include \
         /usr/local/include \
         /opt/local/include \
         /sw/include        \
         /local             \
         /usr/include"

# Search for PGPLOT.
for INCDIR in $INCDIRS ; do
  AC_CHECK_FILE([$INCDIR/pgplot/cpgplot.h],
                [PGPLOTINC=-I$INCDIR/pgplot; break])
  AC_CHECK_FILE([$INCDIR/cpgplot.h], [PGPLOTINC=-I$INCDIR; break])
done

# FLIBS (found above via AC_F77_LIBRARY_LDFLAGS) only helps if PGPLOT was
# built using the same Fortran compiler that we are using here.

# PGPLOT compiled by the SUN Fortran compiler but linked with something else.
AC_CHECK_LIB([M77],     [iand_],     [PGPLOTLIB="-lM77 $PGPLOTLIB"],
             [], [$PGPLOTLIB $LIBS])
AC_CHECK_LIB([F77],     [f77_init],  [PGPLOTLIB="-lF77 $PGPLOTLIB"],
             [], [$PGPLOTLIB $LIBS])

# PGPLOT compiled with g77 but linked with something else.
AC_CHECK_LIB([frtbegin], [main],     [PGPLOTLIB="-lfrtbegin $PGPLOTLIB"],
             [], [$PGPLOTLIB $LIBS])
AC_CHECK_LIB([g2c],      [gerror_],  [PGPLOTLIB="-lg2c $PGPLOTLIB"],
             [], [$PGPLOTLIB $LIBS])

# PGPLOT compiled with gfortran but linked with something else.
AC_CHECK_LIB([gfortran], [_gfortran_abort],
             [PGPLOTLIB="-lgfortran $PGPLOTLIB"], [], [$PGPLOTLIB $LIBS])

# Search for X11 includes and libraries.
AC_PATH_X
if test "x$no_x" = x; then
  if test "x$ac_x_libraries" != x ; then
    # Not needed for systems that keep the X11 libraries in /usr/lib.
    LDFLAGS="$LDFLAGS -L$ac_x_libraries"
  fi
  PGPLOTLIB="-lX11 $PGPLOTLIB"
fi

# It is possible that other libraries may be required depending on what
# graphics drivers were installed with PGPLOT.
AC_CHECK_LIB([z],       [deflate],   [PGPLOTLIB="-lz $PGPLOTLIB"],
             [], [$PGPLOTLIB $LIBS])
AC_CHECK_LIB([png],     [png_error], [PGPLOTLIB="-lpng $PGPLOTLIB"],
             [], [$PGPLOTLIB $LIBS])
AC_CHECK_LIB([pgplot],  [pgbeg_],    [PGPLOTLIB="-lpgplot $PGPLOTLIB"],
             [], [$PGPLOTLIB $LIBS])
AC_CHECK_LIB([cpgplot], [cpgbeg],    [PGPLOTLIB="-lcpgplot $PGPLOTLIB"],
             [PGPLOTLIB=], [$PGPLOTLIB $LIBS])

if test "x$PGPLOTLIB" = x; then
  AC_MSG_WARN([PGPLOT not found, skipping PGPLOT-dependent tests.])
else
  AC_MSG_NOTICE([PGPLOT appears to be available.])
fi

AC_SUBST([PGPLOTINC])
AC_SUBST([PGPLOTLIB])


# Search for CFITSIO.
for INCDIR in $INCDIRS ; do
  AC_CHECK_FILE([$INCDIR/cfitsio/fitsio.h],
                [CFITSIOINC=-I$INCDIR/cfitsio; break])
  AC_CHECK_FILE([$INCDIR/fitsio.h], [CFITSIOINC=-I$INCDIR; break])
done

AC_CHECK_LIB([socket],  [recv],   [CFITSIOLIB="-lsocket"], [], [$LIBS])
AC_CHECK_LIB([cfitsio], [ffopen], [CFITSIOLIB="-lcfitsio $CFITSIOLIB"], [],
             [$CFITSIOLIB $LIBS])

if test "x$CFITSIOINC" = x -o "x$CFITSIOLIB" = x; then
  AC_MSG_WARN([CFITSIO not found, skipping CFITSIO-dependent tests.])
else
  AC_MSG_NOTICE([CFITSIO appears to be available.])
  AC_DEFINE([HAVE_CFITSIO], [1], [Define to 1 if CFITSIO is available.])

  # Check for fits_read_wcstab, present in CFITSIO 3.004beta and later.
  AC_CHECK_LIB([cfitsio], [fits_read_wcstab], [GETWCSTAB=],
               [GETWCSTAB=getwcstab.o], [$CFITSIOLIB $LIBS])
  if test "x$GETWCSTAB" != x ; then
    AC_MSG_WARN([fits_read_wcstab not found in CFITSIO, will use
                    getwcstab.c to compile test programs.])
  fi
fi

AC_SUBST([CFITSIOINC])
AC_SUBST([CFITSIOLIB])
AC_SUBST([GETWCSTAB])


AC_CONFIG_FILES([makedefs])
AC_CONFIG_HEADERS([wcsconfig.h wcsconfig_f77.h wcsconfig_tests.h \
                   wcsconfig_utils.h])
AC_OUTPUT
