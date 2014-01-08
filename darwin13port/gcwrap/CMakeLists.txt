# CASA - Common Astronomy Software Applications
# Copyright (C) 2010 by ESO (in the framework of the ALMA collaboration)
# Copyright (C) 2010 Associated Universities, Inc. Washington DC, USA.
#
# This file is part of CASA.
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

cmake_minimum_required( VERSION 2.8 )

set( CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/install )
include( config )
include( ui )
include( target )
include( CASA )

set( CASA_MAJOR_VERSION 4 )
set( CASA_MINOR_VERSION 2 )
set( CASA_PATCH_VERSION 0 )
set( CASA_API_VERSION "${CASA_MAJOR_VERSION}.${CASA_MINOR_VERSION}.${CASA_PATCH_VERSION}" )

###
### default name for casa-maintained dbus C++ library... (but could also be called just "dbus-cpp")
###
set( dbus-cpp-library-name casa-dbus-cpp )
###
### by default, look for Qt4 in the standard places... (with casa01-* RPMs and linux, we avoid system places)...
###
set( qt4 system-qt )

enable_language (Fortran)

# Define compiler paths on OSX 10.5. This must be done before invoking project()
if( APPLE )
    set( SO dylib )
    if( EXISTS ${CMAKE_Fortran_COMPILER}) 
        message(STATUS "Use " ${CMAKE_Fortran_COMPILER})
    elseif( EXISTS            /opt/casa/core2-apple-darwin8/3rd-party/bin/gfortran )
        set( CMAKE_Fortran_COMPILER /opt/casa/core2-apple-darwin8/3rd-party/bin/gfortran )
	set( CMAKE_CXX_COMPILER     /opt/casa/core2-apple-darwin8/3rd-party/bin/g++ )
    elseif( EXISTS        /opt/local/bin/gfortran )
        set( CMAKE_Fortran_COMPILER /opt/local/bin/gfortran )
    elseif( EXISTS        /opt/local/bin/gfortran-mp-4.2 )
        set( CMAKE_Fortran_COMPILER /opt/local/bin/gfortran-mp-4.2 )
    elseif( EXISTS        /opt/local/bin/gfortran-mp-4.4 )
        set( CMAKE_Fortran_COMPILER /opt/local/bin/gfortran-mp-4.4 )
	if( EXISTS /opt/local/bin/g++-mp-4.4 )
	    set( CMAKE_CXX_COMPILER /opt/local/bin/g++-mp-4.4 )
	endif()
    endif()
else()
    set( SO so )
endif()

project( CASA C CXX Fortran )

# C is enabled because 
# - FindQt4.cmake requires it (bug)
# - ccmtools uses the C preprocessor
if( CMAKE_Fortran_COMPILER STREQUAL CMAKE_Fortran_COMPILER-NOTFOUND )
  message( FATAL_ERROR "Could not find fortran compiler. Please check!" )
endif()

casa_always_install()

#
# Build options
#

# force searching for libcasacore instead of libcasa_*
if( NOT USE_LIBCASACORE )
  set( USE_LIBCASACORE False )
endif()

# Build type (None, Release, Debug, RelWithDebInfo, MinSizeRel).
if( NOT CMAKE_BUILD_TYPE )
  set( CMAKE_BUILD_TYPE RelWithDebInfo )
endif()

if( INTERACTIVE_ITERATION )
	add_definitions( -DINTERACTIVE_ITERATION )
endif()

if( CMAKE_BUILD_TYPE STREQUAL Debug )
  add_definitions( -DAIPS_DEBUG )
  set( CMAKE_CXX_FLAGS_DEBUG "-ggdb3 -O0 -fno-omit-frame-pointer" )
endif()

include( UsePrivatePath )
if( NOT RE_CMAKE )
    string( REGEX REPLACE /[^/]+/?$ "" casaroot ${CMAKE_SOURCE_DIR} )
endif()
#string ( REGEX REPLACE /\\/\\.\\./ "" casaroot ${casaroot} )

#
# aipshosts is supported for backwards compatibility
#
if( NOT arch AND EXISTS ${casaroot}/aipshosts )
  message( WARNING "Found ${casaroot}/aipshosts. You probably don't need  this file unless you need to build multiple architectures. If you remove aipshosts the name of the installation directory will be automatically set  based on this machine's architecture, or defined by the cmake command line option -Darch=<arch name>. Specifying on the command line will override the aipshosts entry." )
  execute_process( COMMAND uname -n
                   COMMAND awk -F. "{print $1}"
                   OUTPUT_VARIABLE _host OUTPUT_STRIP_TRAILING_WHITESPACE )
  execute_process( COMMAND egrep "^[\ ]*${_host}[\ \t]*" ${casaroot}/aipshosts 
                   COMMAND awk "{print $2}"
    OUTPUT_VARIABLE _a
    OUTPUT_STRIP_TRAILING_WHITESPACE
    )

  if( _a )
    set( arch ${_a} )    
  endif()
endif()

set( CMAKE_CXX_FLAGS_PROFILE "-g -Ofast -pg -fprofile-arcs" )

# On Linux x86_64 add the (gcc specific) -fno-omit-frame-pointer
# flag to debug builds
if( CMAKE_SYSTEM_NAME STREQUAL Linux AND
    CMAKE_SYSTEM_PROCESSOR STREQUAL x86_64 )
  foreach( _c CXX C FORTRAN )
    set( CMAKE_${_c}_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-omit-frame-pointer")
    set( CMAKE_${_c}_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -fno-omit-frame-pointer")
  endforeach()
endif()

if (CMAKE_CROSSCOMPILING)
  dump( CMAKE_HOST_SYSTEM )
endif()
dump( CMAKE_SYSTEM
      CMAKE_SYSTEM_PROCESSOR
      CMAKE_GENERATOR 
      CMAKE_BUILD_TYPE )

set( BUILD_SHARED_LIBS ON )

set( DEFAULT_CXX_FLAGS 
     "-pipe -Ofast -Wall -Wextra -Wno-non-template-friend -Wcast-align -Wno-comment"
     )

set( DEFAULT_C_FLAGS 
     "-pipe -Ofast -Wall -Wextra -Wcast-align -Wno-comment"
     )

# OpenMP
find_package( OpenMP )
if( OPENMP_FOUND )
  set( DEFAULT_CXX_FLAGS "${DEFAULT_CXX_FLAGS} ${OpenMP_CXX_FLAGS}" )
  set( DEFAULT_C_FLAGS   "${DEFAULT_C_FLAGS}   ${OpenMP_C_FLAGS}" )
endif()

# Test driver which requires that test programs are up-to-date
# before running. (The built-in "test" does not.)
add_custom_target( check ${CMAKE_CTEST_COMMAND} )
enable_testing()
set( CASA_assay ${CMAKE_SOURCE_DIR}/install/assay )

#
# Define paths for development packages.
# Check for unresolved symbols at link time.
# Set compiler flags.
# 
if( APPLE )
    set( CMAKE_Fortran_FLAGS -fno-second-underscore )
    execute_process( COMMAND ${CMAKE_Fortran_COMPILER} -print-search-dirs
	                 COMMAND perl -e "$found = 0; while(<>) { if( m|^libraries:\\s*=(.*?$)| ) { foreach $d (split(':',$1) ) { if ( -e \"$d/libgfortran.${CMAKE_SHARED_LIBRARY_SUFFIX}\" ) { print \"$d/libgfortran.${CMAKE_SHARED_LIBRARY_SUFFIX}\"; $found = 1; break;}; if ( -e \"$d/libgfortran.a\" and !$found ) { print \"$d/libgfortran.a \"; $found = 1; break;}break if $found;}break if $found;} break if $found; }"
                     OUTPUT_VARIABLE _gfortran_lib_path
                     OUTPUT_STRIP_TRAILING_WHITESPACE )

    if( CMAKE_SYSTEM MATCHES ^Darwin-9 )
        set( casa_packages /opt/casa/core2-apple-darwin8/3rd-party )
        set( CMAKE_SHARED_LINKER_FLAGS "-Wl,-undefined -Wl,error" )
        if( NOT arch )
        	set( arch darwin )
        endif()
    elseif( CMAKE_SYSTEM MATCHES ^Darwin-10 OR
		CMAKE_SYSTEM MATCHES ^Darwin-11 OR
		CMAKE_SYSTEM MATCHES ^Darwin-12 OR
		CMAKE_SYSTEM MATCHES ^Darwin-13)
        if( NOT arch )
        	set( arch darwin64 )
        endif()
        if( NOT archflag )
            set( archflag x86_64 )
            if( EXISTS /opt/casa/darwin11 )
                set( archflag x86_64 )
                if( EXISTS /opt/casa/darwin10-64b )
                    set( archflag x86_64 )
                elseif( EXISTS /opt/casa/core2-apple-darwin10 )
                    set( archflag i386 )
                else()
                    ## Apple no longer supplies fortran compilers, checking for x86_64 with fortran
	                ## compiler will be a good indication if x86_64 is available or not...
	                execute_process( COMMAND ${CMAKE_Fortran_COMPILER} -print-libgcc-file-name
        	                         COMMAND perl -e "while(<>){ print if s|^.*?(x86_64).*|$1| }"
                                     OUTPUT_VARIABLE archflag
                                     OUTPUT_STRIP_TRAILING_WHITESPACE )
                    if( NOT archflag )
                        set( archflag x86_64 )
                    endif()
                endif()
            endif()
            if( archflag STREQUAL x86_64 )
	            if(CMAKE_SYSTEM MATCHES ^Darwin-11)
                    set ( casa_packages /opt/casa/darwin11 )
                    set( PYTHON_ROOT_DIR /opt/casa/darwin11 )
	            elseif(CMAKE_SYSTEM MATCHES ^Darwin-12)
                    set ( casa_packages /opt/casa/darwin12 )
                    set( PYTHON_ROOT_DIR /opt/casa/darwin12 )
	            elseif(CMAKE_SYSTEM MATCHES ^Darwin-13)
                    set ( casa_packages /opt/casa/darwin13 )
                    set( PYTHON_ROOT_DIR /opt/casa/darwin13 )
	            else()
                    set ( casa_packages /opt/casa/darwin10-64b )
                    set( PYTHON_ROOT_DIR /opt/casa/darwin10-64b )
	            endif()
                add_definitions( -DAIPS_64B )
	        endif()
        else()
            set( casa_packages /opt/casa/core2-apple-darwin10 )
        endif()
	    execute_process( COMMAND ${CMAKE_Fortran_COMPILER} --version
                         COMMAND head -1
                         COMMAND perl -pe "s|.*?(\\d+\\.\\d+)\\.\\d+$|$1|"
                         OUTPUT_VARIABLE _gfortran_version
                         OUTPUT_STRIP_TRAILING_WHITESPACE )
	    execute_process( COMMAND ${CMAKE_CXX_COMPILER} -dumpmachine
                         COMMAND head -1
                         COMMAND perl -pe "s|.*?(\\d+\\.\\d+)\\.\\d+$|$1|"
                         OUTPUT_VARIABLE _gcc_machine
                         OUTPUT_STRIP_TRAILING_WHITESPACE )
	    execute_process( COMMAND ${CMAKE_CXX_COMPILER} --version
                         COMMAND head -1
                         COMMAND perl -pe "s|.*?(\\d+\\.\\d+)\\.\\d+$|$1|"
                         OUTPUT_VARIABLE _cxx_version
                         OUTPUT_STRIP_TRAILING_WHITESPACE )
	    set (INTEL_CXX_COMPILER "false" )
	    if( _cxx_version MATCHES "ICC")
		    set (INTEL_CXX_COMPILER "true" )
        endif()
	    set (INTEL_FORTRAN_COMPILER "false" )
	    if( _gfortran__version MATCHES "IFORT")
		    set (INTEL_FORTRAN_COMPILER "true" )
	    endif()

	    message( STATUS "$_gfortran_version = ${_gfortran_version}" )
	    message( STATUS "$_gfortran_lib_path = ${_gfortran_lib_path}" )
	    message( STATUS "$_gcc_machine = ${_gcc_machine}" )
        message( STATUS "cxxversion (${_cxx_version})" )
	    if( NOT _gfortran_version STREQUAL "4.4" AND
			NOT _gfortran_version STREQUAL "4.2" AND
			NOT LLVMCOMPILER )
	        set( CMAKE_Fortran_FLAGS "${CMAKE_Fortran_FLAGS}" )
	        set( CMAKE_SHARED_LINKER_FLAGS "-Wl,-undefined -Wl,error -framework AppKit -lgfortran")
	    else()
	        set( CMAKE_SHARED_LINKER_FLAGS "-Wl,-undefined -Wl,error -framework AppKit"  )
        endif()
        # Which part of CASA needs -framework AppKit to run? This should probably *not* be added
        # as a generic linker flag.
	    if( NOT _cxx_version STREQUAL "4.4" )
	        set( DEFAULT_CXX_FLAGS "${DEFAULT_CXX_FLAGS}" )
	    endif()
    else()
        message( WARNING "Unsupported Mac: ${CMAKE_SYSTEM}" )
    endif()

    if( NOT INTEL_CXX_COMPILER STREQUAL "true" )
        if( _cxx_version STREQUAL "4.4" )
	        set( DEFAULT_CXX_FLAGS "${DEFAULT_CXX_FLAGS} -dynamic -mtune=nocona -ftree-vectorize -ffast-math -mfpmath=sse" )
        else()
#	        set( DEFAULT_CXX_FLAGS "${DEFAULT_CXX_FLAGS} -dynamic -mtune=nocona -ftree-vectorize -ffast-math -mfpmath=sse -fasm-blocks" )
	        set( DEFAULT_CXX_FLAGS "${DEFAULT_CXX_FLAGS} -dynamic -mtune=nocona -ftree-vectorize -ffast-math -mfpmath=sse" )
        endif()
    endif()

    find_program( DBUS_FLAVOR_dbuspp-xml2cpp_EXECUTABLE dbuspp-xml2cpp PATHS ${casa_packages}/bin /opt/local/bin /usr/local/bin )
    if ( ${DBUS_FLAVOR_dbuspp-xml2cpp_EXECUTABLE} MATCHES "NOTFOUND$" )
        set( dbusxx dbus-c++ )
    else( )
        set( dbusxx dbus-cpp )
        add_definitions( -DDBUS_CPP )
        STRING( REGEX REPLACE "(^.*)/bin.*" "\\1" dbus-root-path  ${DBUS_FLAVOR_dbuspp-xml2cpp_EXECUTABLE} )
        if( EXISTS ${dbus-root-path}/lib/dbus-1.0 )
            set( dbus-arch-path-guess ${dbus-root-path}/lib/dbus-1.0 )
        endif( )
        if( EXISTS ${dbus-root-path}/lib/libdbus-cpp.dylib )
            set( dbus-cpp-library-name dbus-cpp )
        endif( )
    endif( )

    set( python_version 2.7 )

elseif( CMAKE_SYSTEM_NAME STREQUAL Linux )

    set( CMAKE_SHARED_LINKER_FLAGS "-Wl,--no-undefined")

    if( CMAKE_SYSTEM_PROCESSOR STREQUAL x86_64 )
        if ( EXISTS /usr/lib64/casa/01/bin/qmake )
            set( casa_packages /usr/lib64/casa/01 )
	        set( dbusxx dbus-cpp )
	        add_definitions( -DDBUS_CPP )
	        set( qt4 casa-qt )
            set( python_version 2.7 )
        else( )
	        set( dbusxx dbus-c++ )
            set( casa_packages /usr/lib64/casapy )
	        set( qt4 system-qt )
            set( python_version 2.7 )
        endif( )
        if( NOT arch )
    	    set( arch linux_64b )
        endif()
        set( DEFAULT_CXX_FLAGS "${DEFAULT_CXX_FLAGS} -Wno-deprecated" )
    else()
        set( casa_packages /usr/lib/casapy )
        if( NOT arch )
            set( arch linux_gnu )
        endif()
        set( DEFAULT_CXX_FLAGS "${DEFAULT_CXX_FLAGS} -Wno-deprecated -Woverloaded-virtual" )

        set( dbusxx dbus-c++ )
        set( casa_packages /usr/lib/casapy )
        set( qt4 system-qt )
        set( python_version 2.7 )

    endif()
else()
    message( WARNING "Unsupported system: ${CMAKE_SYSTEM_NAME}" )
endif()

set( CMAKE_MODULE_LINKER_FLAGS ${CMAKE_SHARED_LINKER_FLAGS} )

if( NOT CMAKE_CXX_FLAGS )
  set( CMAKE_CXX_FLAGS "${DEFAULT_CXX_FLAGS} ${EXTRA_CXX_FLAGS}" )
endif()

if( NOT CMAKE_C_FLAGS )
  set( CMAKE_C_FLAGS "${DEFAULT_C_FLAGS} ${EXTRA_C_FLAGS}" )
endif()


if( NOT RE_CMAKE )
    if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
        set( CMAKE_INSTALL_PREFIX ${casaroot}/${arch} CACHE PATH "casa architecture directory" FORCE )
	set( casaroot ${casaroot} CACHE PATH "toplevel casa directory" FORCE )
    else()
        set( CMAKE_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX} CACHE PATH "casa architecture directory" FORCE )
	set( casaroot ${CMAKE_INSTALL_PREFIX}/.. CACHE PATH "toplevel casa directory" FORCE )
   endif(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
endif()

dump( casaroot )
dump( CMAKE_INSTALL_PREFIX )

set( epochdelta 1385614800 )
if ( EXISTS ${CMAKE_INSTALL_PREFIX}/${arch}/casa_sover.txt )
    execute_process( COMMAND perl -e "while (<>) { chomp and print if (! m/^\#/ ) }" ${CMAKE_INSTALL_PREFIX}/${arch}/casa_sover.txt
                     OUTPUT_VARIABLE __casa_soversion )
elseif( EXISTS ${CMAKE_INSTALL_PREFIX}/casa_sover.txt )
    execute_process( COMMAND perl -e "while (<>) { chomp and print if (! m/^#/ ) }" ${CMAKE_INSTALL_PREFIX}/casa_sover.txt
                     OUTPUT_VARIABLE __casa_soversion )
else( )
    execute_process( COMMAND perl -e "$t=time( )-${epochdelta};$z=$t & 0xff; $y=($t>>8)&0xff; $x=($t>>16)&0xffff; print \"$x.$y.$z\""
                     OUTPUT_VARIABLE __casa_soversion )
endif( )
set(casa_soversion ${__casa_soversion} CACHE STRING "version for shared objects")
message( STATUS "shared object version number ${casa_soversion}" )

set( RE_CMAKE "YES" CACHE STRING "are we rerunning cmake?" FORCE )

#
# Targets for creating doxygen documentation
#

# Use htlatex, do not support latex2html
find_program( LATEX2HTML_CONVERTER NAMES htlatex )
set( LATEX2HTML_OPTIONS "htcasa,section+,next,p-width,5"
      CACHE STRING "LATEX2HTML_CONVERTER options" )

# Use pdflatex
find_program( PDFLATEX_COMPILER NAMES pdflatex )
#set( LATEX2PDF_OPTIONS "htcasa,section+,next,p-width,3" 
#CACHE STRING "LATEX2PDF_CONVERTER options" )

set( CASA_DOC_DIR ${casaroot}/docs ) # Where to put documentation

find_package( Doxygen )

if( DOXYGEN_FOUND )
  add_custom_target( doc_doxy
    COMMAND ${DOXYGEN_EXECUTABLE} ${casaroot}/install/docutils/doxygen.cfg 
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    )
else()
  add_custom_target( doc_doxy
    COMMAND echo "doxygen was not found."
    )
endif()

# Choose to have correct RPATHs both in the build tree and
# in the install tree (at the cost of having to change the
# rpath when installing)
set( CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib:arbitraryExtraPaddingToAllowForRpathGrowth" )
set( CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
set( CMAKE_INSTALL_NAME_DIR ${CMAKE_INSTALL_PREFIX}/lib )

# Preprocessor definitions
add_definitions( 
# seen in CASA code
        -DAIPS_HAS_QWT
# seen nowhere
#        -DHAVE_QT4
#        -DAIPS_USEATM         # only in synthesis test
#        -DAIPS_USEMATPLOTLIB  # commented out in tableplot
)

if( CMAKE_SYSTEM_NAME STREQUAL Linux )
  add_definitions( -DAIPS_LINUX )
  if( CMAKE_SYSTEM_PROCESSOR STREQUAL x86_64 )
    add_definitions(
      -DAIPS_64B
      )
  else()
    # Linux, 32 bit
  endif()
elseif( CMAKE_SYSTEM_NAME STREQUAL Darwin )
  add_definitions(
    -DAIPS_DARWIN 
    )
else(
    message( WARNING "Not on Linux nor Darwin!" )
)
endif()

include( TestBigEndian )
test_big_endian( is_big_endian )
if( NOT is_big_endian )
  add_definitions(
    -DAIPS_LITTLE_ENDIAN
    )
endif()

#
# External packages
#

# Perl
find_package( Perl )

# CFITSIO
casa_find( CFITSIO
  VERSION 3.006
  INCLUDES fitsio.h fitsio2.h
  INCLUDES_SUFFIXES cfitsio
  LIBS cfitsio
  PREFIX_HINTS ${CFITSIO_ROOT_DIR}
  CPP_VERSION CFITSIO_VERSION
  RUN_VERSION "(ffvers(&v), v)" )

# X11.
if( NOT APPLE )
  find_package( X11 REQUIRED )
  # Rename
  set( X11_INCLUDE_DIRS ${X11_INCLUDE_DIR} )
endif()

# Since we are using the C++ linker, the Fortran runtime libraries must be explicitly
# linked in.

if( NOT FORTRAN_LIBRARIES )

  message( STATUS "Looking for Fortran runtime libraries" )
  set( _try  ${CMAKE_BINARY_DIR}/try_fortran.cc )
  file( WRITE ${_try}
    "int main() { return 0; }\n"
    )
  
  if( _gfortran_lib_path )
    try_compile( _have_gfortran ${CMAKE_BINARY_DIR} ${_try}
      CMAKE_FLAGS -Wdev "-DCMAKE_EXE_LINKER_FLAGS=${_gfortran_lib_path}"
      )
  else()
    try_compile( _have_gfortran ${CMAKE_BINARY_DIR} ${_try}
      CMAKE_FLAGS -Wdev "-DCMAKE_EXE_LINKER_FLAGS=-lgfortran"
      )
  endif()
  try_compile( _have_g2c ${CMAKE_BINARY_DIR} ${_try}
    CMAKE_FLAGS -Wdev "-DCMAKE_EXE_LINKER_FLAGS=-lg2c"
    )
 
  if( _have_gfortran )
    if( _gfortran_lib_path )
      set( FORTRAN_LIBRARIES ${_gfortran_lib_path}
	CACHE STRING "Fortran library linker option" FORCE )
    else()
      set( FORTRAN_LIBRARIES -lgfortran 
        CACHE STRING "Fortran library linker option" FORCE )
    endif()
    message( STATUS "Looking for Fortran runtime libraries -- ${FORTRAN_LIBRARIES}" )
  elseif( _have_g2c )
    set( FORTRAN_LIBRARIES -lg2c
      CACHE STRING "Fortran library linker option" FORCE )
    message( STATUS "Looking for Fortran runtime libraries -- ${FORTRAN_LIBRARIES}" )
  else()
    set( FORTRAN_LIBRARIES ""
      CACHE STRING "Fortran library linker option" FORCE )
    message( STATUS "Looking for Fortran runtime libraries -- <none>" )
    # Not a fatal error because it might work, if all Fortran dependencies were
    # already linked statically to the Fortran runtime...
  endif()
endif()

# PGPLOT (FORTRAN plotting package).
# depends on X11
if( NOT SKIP_PGPLOT )
  if( APPLE )
    set( _deps FORTRAN )
  else()
    set( _deps X11 FORTRAN )
  endif()
  if (CMAKE_SYSTEM MATCHES ^Darwin-11 AND NOT LLVMCOMPILER )
     set( CMAKE_SHARED_LINKER_FLAGS "-Wl,-undefined -Wl,error -framework AppKit -lgfortran" )
  else()
     if (APPLE) 
        set( CMAKE_SHARED_LINKER_FLAGS "-Wl,-undefined -Wl,error -framework AppKit" )
     else()
        set( CMAKE_SHARED_LINKER_FLAGS "-Wl,-undefined -Wl,error" )
     endif()
  endif()
  casa_find( PGPLOT
    VERSION 5.3.1
    PREFIX_HINTS ${PGPLOT_ROOT_DIR}
    INCLUDES cpgplot.h
    LIBS pgplot${pgplot_ext} cpgplot${pgplot_ext}
    DEPENDS ${_deps} )
endif()

# WCSLIB
if( NOT SKIP_PGPLOT )
  casa_find( WCSLIB
    VERSION 4.3
    PREFIX_HINTS ${WCSLIB_ROOT_DIR}
    INCLUDES wcslib/wcsconfig.h
    LIBS wcs
         pgsbox${pgplot_ext}
    # WCSLIB does "#define WCSLIB_VERSION 4.3.4" without quotes,
    # hence "cout << WCSLIB_VERSION" would not work. The following
    # code makes a string out of WCSLIB_VERSION.
    CPP_VERSION "\"\";
#define CASA_STRINGIFY(x) #x
#define CASA_TOSTRING(x) CASA_STRINGIFY(x)
    std::cout << CASA_TOSTRING(WCSLIB_VERSION)"
    DEFINITIONS -DSIGNBIT 
    DEPENDS PGPLOT )
else()
  casa_find( WCSLIB
    VERSION 4.3
    INCLUDES wcslib/wcsconfig.h
    LIBS wcs
    # WCSLIB does "#define WCSLIB_VERSION 4.3.4" without quotes,
    # hence "cout << WCSLIB_VERSION" would not work. The following
    # code makes a string out of WCSLIB_VERSION.
    CPP_VERSION "\"\";
#define CASA_STRINGIFY(x) #x
#define CASA_TOSTRING(x) CASA_STRINGIFY(x)
    std::cout << CASA_TOSTRING(WCSLIB_VERSION)"
    DEFINITIONS -DSIGNBIT )
endif()

# FFTW
# Does not provide its version number in a header nor
# in the library, which is a pity because CASACore needs
# version >= 3.2.2 and is subject to a bug in fftw 3.2.1
casa_find( FFTW3
           PREFIX_HINTS ${FFTW3_ROOT_DIR}
           INCLUDES fftw3.h
           LIBS fftw3_threads fftw3f_threads fftw3f fftw3 )

# BLAS
casa_find( BLAS
           LIBS blas )

# LAPACK
casa_find( LAPACK
           LIBS lapack 
           DEPENDS BLAS )

# DL
set( DL_LIBRARIES ${CMAKE_DL_LIBS} CACHE STRING "dl libraries" FORCE )
if( DL_LIBRARIES STREQUAL "dl" )
  set( DL_LIBRARIES "-ldl" CACHE STRING "dl libraries" FORCE )
endif()

# CASACore
# - Some header files exported by CASACore include WCSLIB headers
# - CASACore header files declare symbols which are defined in LAPACK and BLAS
# - There are several, incompatible, ways of building CASACore.

if( EXISTS ${CMAKE_INSTALL_PREFIX}/lib/libcasacore.dylib OR
    EXISTS ${CMAKE_INSTALL_PREFIX}/lib/libcasacore.so OR
    EXISTS ${CMAKE_INSTALL_PREFIX}/lib64/libcasacore.so OR USE_LIBCASACORE )
  set( casacore_libs casacore )
  ##
  ## by default the GNUmakefile for casacore installs include
  ## files into ${casaroot}.
  ##
  set( casacore_prefix ${CMAKE_INSTALL_PREFIX}/.. )
else()
  if( EXISTS ${CMAKE_INSTALL_PREFIX}/../include/casacore/casa/BasicMath.h )
      set( casacore_prefix ${CMAKE_INSTALL_PREFIX}/.. )
  endif()
  set( casacore_libs
       casa_casa 
       casa_components
       casa_coordinates 
       casa_fits
       casa_images
       casa_lattices
       casa_measures
       casa_mirlib
       casa_ms
       casa_msfits
       casa_scimath
       casa_scimath_f
       casa_tables )
endif()

casa_find( CASACORE
           VERSION 1.0.226
           PREFIX_HINTS ${CASACORE_ROOT_DIR}
           INCLUDES casa/aips.h
                    casa/version.h
                    scimath/Mathematics/MatrixMathLA.h  # declares symbols defined in LAPACK
           INCLUDES_SUFFIXES casacore
           LIBS ${casacore_libs}
           PREFIX_HINTS ${casacore_prefix}
           CPP_VERSION CASACORE_VERSION
           RUN_VERSION "casa::getVersionCASA()"
           DEPENDS LAPACK CFITSIO WCSLIB FFTW3 DL )

# Listing explicitly CASACore's dependency libraries is not necessary if the CASACore libraries 
# were built with correct linking information, which they are not.

set( CASACORE_DEFINITIONS ${CASACORE_DEFINITIONS}
  -DCASA_USECASAPATH
  -DCASACORE_NEEDS_RETHROW
  -DAIPS_STDLIB
  -DAIPS_AUTO_STL
  -D_GNU_SOURCE )

if( CMAKE_SYSTEM_NAME STREQUAL Linux )
  set( CASACORE_DEFINITIONS ${CASACORE_DEFINITIONS}
    -D_FILE_OFFSET_BITS=64
    -D_LARGEFILE_SOURCE 
    )
endif()

#
# casa code stuff
#

if( EXISTS ${CMAKE_INSTALL_PREFIX}/lib/libcasa.dylib OR
    EXISTS ${CMAKE_INSTALL_PREFIX}/lib/libcasa.so OR
    EXISTS ${CMAKE_INSTALL_PREFIX}/lib64/libcasa.so OR USE_LIBCASACODE )
  set( casacode_libs casacode )
  ##
  ## by default the GNUmakefile for casacore installs include
  ## files into ${casaroot}.
  ##
  set( casacode_prefix ${CMAKE_INSTALL_PREFIX}/.. )
else()
  if( EXISTS ${CMAKE_INSTALL_PREFIX}/../include/casacode/casa/BasicMath.h )
      set( casacode_prefix ${CMAKE_INSTALL_PREFIX}/.. )
  endif()
  set( casacode_libs
       alma 
       calanalysis
       casadbus
       casaqt
       components
       dish
       display
       flagging
       graphics
       imageanalysis
       miriad
       mstransform
       nrao
       plotms
       spectrallines
       stdcasa
       synthesis
       )
endif()

#
# CASACode
#

casa_find( CASACODE
           VERSION 1.0.145
           PREFIX_HINTS ${CASACODE_ROOT_DIR}
           INCLUDES stdcasa/version.h
           INCLUDES_SUFFIXES casacode
           LIBS ${casacode_libs}
           PREFIX_HINTS ${casacode_prefix}
	   DEPENDS CASACORE LAPACK CFITSIO WCSLIB FFTW3 DL )


# ALMAWVR
if( USE_ALMAWVR )
  casa_find( ALMAWVR
             VERSION 0.23.2
             PREFIX_HINTS ${ALMWVR_ROOT_DIR}
             LIBS almawvr
           )
endif()

# Python

# First determine which python version to search for,
# then do the actual search
if( NOT PYTHON_FOUND )

  if ( NOT PYTHON_LIBNAME )
    set( _names 2.7 2.6 2.5.2 2.5 )
    # (The library named libpython.2.5.2.dylib seems to exist only in the CASA world.)
  else()
    set( _names ${PYTHON_LIBNAME} )
  endif()

  set( _found False )
  foreach( _v ${_names} )

    casa_find( 
      PYTHON${_v} 
      PREFIX_HINTS ${PYTHON_ROOT_DIR}
      LIBS python${_v} 
      NO_REQUIRE 
      )

    if( PYTHON${_v}_FOUND )
      set( PYTHON_LIBNAME ${_v} )
      set( _found True )
      break()
    endif()

  endforeach()
  
  if( NOT _found )
    message( FATAL_ERROR "Could not find any PYTHON library with version one of ${_names}. Please check!" )
  endif()

endif()

if( NOT PYTHON_LIBNAME )
  # Found Python, but PYTHON_LIBNAME is undefined, that is impossible.
  message( FATAL_ERROR "The variable PYTHON_LIBNAME is undefined. Most likely, CMake's cache is out of date and you need to redetect your PYTHON installation (\"cmake -U PYTHON*\")")
endif()

string( SUBSTRING ${PYTHON_LIBNAME} 0 3 PYTHONV )
string( REPLACE "." "" PV ${PYTHONV} )
set( python_library python${PYTHON_LIBNAME} )

# Form the Python install prefix by stripping away "lib/libpython2.5.2.dylib" (example) from 
# the full python library path
string( REGEX MATCH "/lib(64)?/lib${python_library}" _match ${PYTHON${PYTHON_LIBNAME}_LIBRARIES} )
if( _match )
  string( REGEX REPLACE "/lib(64)?/lib${python_library}.*" "" python_prefix ${PYTHON${PYTHON_LIBNAME}_LIBRARIES} )
else()
  # Python library was not in a lib(64) directory!
  message( WARNING "Python library path \"${PYTHON${PYTHON_LIBNAME}_LIBRARIES}\" does not contain \"/lib(64)/lib${python_library}\"" )
  set( python_prefix ${casa_packages} )
endif()

#
# For some unknown reason cmake sets the /usr for Lion...
# some people do not use the version of python in ${casa_packages}...
# setting python_path unilaterally prevents the user from configuring
# the location of python...
#
if( APPLE )
       set( python_prefix ${casa_packages} )
endif()


# The python version and prefix is known, do the actual search
if( NOT PYTHON_FOUND )
  message( STATUS "Looking for PYTHON version ${PYTHONV}.x in ${python_prefix}" )
endif()

message( "${python_prefix}/Library/Frameworks/Python.framework/Versions/${PYTHONV}/lib/python${PYTHONV}/site-packages/numpy/core")
message( "${NUMPY_ROOT_DIR}" )
casa_find( PYTHON
  VERSION 2.5    # minimum required
  INCLUDES Python.h 
     numpy/npy_interrupt.h   # for numpy
  INCLUDES_SUFFIXES python${PYTHONV}
  PREFIX_HINTS 
     ${NUMPY_ROOT_DIR}     
     ${python_prefix}/lib/python${PYTHONV}/site-packages/numpy/core
     ${python_prefix}/Library/Frameworks/Python.framework/Versions/${PYTHONV}
     ${python_prefix}/Library/Frameworks/Python.framework/Versions/${PYTHONV}/lib/python${PYTHONV}/site-packages/numpy/core
     ${python_prefix}/Library/Frameworks/Python.framework/Versions/${PYTHONV}/lib/python${PYTHONV}/site-packages/numpy/core/include
     ${python_prefix} # Support the ALMA ACS special case.
  LIBS ${python_library}
  CPP_VERSION PY_VERSION )

# Store PYTHON_LIBNAME in the cache
set( PYTHON_LIBNAME ${PYTHON_LIBNAME} CACHE STRING "Python major and minor version to use" FORCE )

# Define pyroot to two directory levels above Python.h.
string( REGEX REPLACE "/[^/]+/[^/]+/?$" "" pyroot ${PYTHON_Python.h} )

if( APPLE )
  if( CMAKE_SYSTEM MATCHES ^Darwin-9 )
    set( pyroot ${pyroot}/lib/Python.framework/Versions/${PYTHONV} )
  else()
    set( pyroot ${pyroot}/Library/Frameworks/Python.framework/Versions/${PYTHONV} )
  endif()
endif()

set( PYTHON_DEFINITIONS ${PYTHON_DEFINITIONS}
  -DAIPSROOT=\"${CMAKE_SOURCE_DIR}\"
  -DAIPSARCH=\"${arch}\"
  -DAIPSSITE=\"garching\"
  -DPYTHONROOT=\"${pyroot}\"
  -DPYTHONVER=\"${PYTHONV}\"
  -DPYVERSION=${PV} )

# Java
casa_find( Java
  VERSION 1.5
  PROGRAMS java
  PROG_VERSION "-version")

set( SAXON ${Java_java_EXECUTABLE} -jar ${CMAKE_SOURCE_DIR}/install/saxon8.jar )

if ( ${dbusxx} STREQUAL dbus-c++ )
  # C++ DBus
  #
  # The header file dbus/dbus-arch-deps.h exists on some but not all systems, in a directory
  # for itself
  #
  # Form all possible DBus prefixes and use them as hints to casa_find()
  #
  set( dbus_prefix "" )
  #foreach( _p ${casa_packages} ${CMAKE_INSTALL_PREFIX} /sw /opt/local /usr/local /opt /usr )
  foreach( _p ${casa_packages} ${CMAKE_INSTALL_PREFIX} /sw /opt/local /usr/local /opt /usr )
    foreach( _l lib lib64 )
      list( APPEND dbus_prefix ${_p} )
      list( APPEND dbus_prefix ${_p}/include )
      list( APPEND dbus_prefix ${_p}/${_l}/dbus-1.0 )
      list( APPEND dbus_prefix ${_p}/${_l}/qt-4.3.4/dbus )
      list( APPEND dbus_prefix ${_p}/${_l}/qt-4.3.4/dbus/${_l}/dbus-1.0 )
    endforeach()
  endforeach()

  casa_find( DBUSXX_ARCH
    INCLUDES dbus/dbus-arch-deps.h
    PREFIX_HINTS /usr/lib64/dbus-1.0 ${DBUS_ARCH_DIR} ${dbus_prefix}
    NO_CHECK   # This header does not compile if included directly
    NO_REQUIRE
    IGNORE /usr/lib64/casapy
  )
  if( DBUSXX_ARCH_FOUND )
    set( dbus_depends DBUSXX_ARCH )
  endif()

  casa_find( DBUSXX
    INCLUDES dbus/dbus.h
             dbus-c++/dbus.h
    INCLUDES_SUFFIXES dbus-1.0 dbus-c++-1
    PREFIX_HINTS ${DBUS_ROOT_DIR} ${dbus_prefix}
    LIBS dbus-c++-1
    PROGRAMS dbusxx-xml2cpp
    DEPENDS ${dbus_depends}
    IGNORE /usr/lib64/casapy
  )
  set( dbus-xml-2-cxx ${DBUSXX_dbusxx-xml2cpp_EXECUTABLE} )
  set( DBUS_INCLUDE_DIRS ${DBUSXX_INCLUDE_DIRS} )
  set( DBUS_LIBRARIES ${DBUSXX_LIBRARIES} )
else( )
  casa_find( DBUSPP_ARCH
    INCLUDES dbus/dbus-arch-deps.h
    PREFIX_HINTS ${DBUS_ARCH_DIR} ${casa_packages}/lib/dbus-1.0 ${dbus-arch-path-guess}
    NO_CHECK   # This header does not compile if included directly
    NO_REQUIRE
  )
  if( DBUSPP_ARCH_FOUND )
    set( dbus_depends DBUSPP_ARCH )
  endif()

  casa_find( DBUSPP
    INCLUDES dbus/dbus.h
             dbus-cpp/dbus.h
    INCLUDES_SUFFIXES dbus-1.0 dbus-cpp
    PREFIX_HINTS ${DBUS_ROOT_DIR} ${casa_packages}
    LIBS ${dbus-cpp-library-name}
    PROGRAMS dbuspp-xml2cpp
    DEPENDS ${dbus_depends}
  )
  set( dbus-xml-2-cxx ${DBUSPP_dbuspp-xml2cpp_EXECUTABLE} )
  set( DBUS_INCLUDE_DIRS ${DBUSPP_INCLUDE_DIRS} )
  set( DBUS_LIBRARIES ${DBUSPP_LIBRARIES} )
endif( )

# ATM
casa_find( ATM
  VERSION 0.5
  INCLUDES ATMEnumerations.h ATMVersion.h
  INCLUDES_SUFFIXES ATM
  LIBS aatm
  CPP_VERSION ATM_VERSION
  RUN_VERSION "atm::getVersion()" 
  PREFIX_HINTS ${ATM_ROOT_DIR}
)

# XML2
casa_find( LIBXML2
  VERSION 2.6.16
  PREFIX_HINTS ${LIBXML2_ROOT_DIR}
  INCLUDES libxml/xmlversion.h
  INCLUDES_SUFFIXES libxml2
  LIBS xml2
  CPP_VERSION LIBXML_DOTTED_VERSION 
  RUN_VERSION "LIBXML_DOTTED_VERSION; LIBXML_TEST_VERSION;" 
  )

# XSLT
casa_find( LIBXSLT
  VERSION 1.1.17
  PREFIX_HINTS ${LIBXSLT_ROOT_DIR}
  INCLUDES libxslt/xsltconfig.h
  INCLUDES_SUFFIXES libxslt
  LIBS xslt
  CPP_VERSION LIBXSLT_DOTTED_VERSION 
  RUN_VERSION "LIBXSLT_DOTTED_VERSION; LIBXSLT_VERSION;" 
  )

# Do not reuse FindLibXml2.cmake, except this excerpt
IF (NOT WIN32)
   # use pkg-config to get the directories and then use these values
   # in the FIND_PATH() and FIND_LIBRARY() calls
   FIND_PACKAGE(PkgConfig)
   PKG_CHECK_MODULES(PC_LIBXML libxml-2.0)
   SET(LIBXML2_DEFINITIONS ${LIBXML2_DEFINITIONS} ${PC_LIBXML_CFLAGS_OTHER})
ENDIF (NOT WIN32)

# Find SWIG
find_package( SWIG  2.0.1 )

if( NOT SWIG_FOUND )
	message( FATAL_ERROR "Swig could not be found. Please check!" )
endif()

#find_package( OpenGL REQUIRED )
#set( OPENGL_INCLUDE_DIRS ${OPENGL_INCLUDE_DIR} CACHE PATH "OpenGL include directories" FORCE )


# QT4  4.3.4 or 4.5.1

if( ${qt4} STREQUAL casa-qt )
  ### cmake is so lame...
  set( QT_QMAKE_EXECUTABLE /usr/lib64/casa/01/bin/qmake )
  message( STATUS "Explicitly setting qmake: ${QT_QMAKE_EXECUTABLE}" )
  set( qt4-version 4.8.5 )
else()
  set( qt4-version 4.3.4 )
endif()

find_package(
  Qt4 4.3.4
  COMPONENTS QtCore QtGui QtDBus QtXml QtUiTools ) #QtOpenGL )

if( NOT QT4_FOUND )
  message( FATAL_ERROR "Qt4 could not be found. Please check!" )
endif()

set( QT4_INCLUDE_DIRS ${QT_INCLUDES} CACHE PATH "QT4 include directories" FORCE )
set( QT4_DEFINITIONS ${QT_DEFINITIONS} -DQT_NO_DEBUG CACHE STRING "QT4 preprocessor flags" FORCE )
set( QT4_LIBRARIES 
  ${QT_QTCORE_LIBRARY} 
  ${QT_QTGUI_LIBRARY} 
  ${QT_QTDBUS_LIBRARY} 
  ${QT_QTXML_LIBRARY} ${QT_QTUITOOLS_LIBRARY}
  # ${QT_QTOPENGL_LIBRARY}
  CACHE FILEPATH "QT4 libraries" FORCE )
list( REMOVE_ITEM QT4_LIBRARIES "optimized" )
list( REMOVE_ITEM QT4_LIBRARIES "debug" )
set( QT4_FOUND TRUE CACHE BOOL "Was QT4 found?" )

# Detecting QT4 takes a bit of time, but does not seem possible to skip.
# If doing so, then "include( ${QT_USE_FILE} )" does not work (because QT_USE_FILE is not cached?).


# QWT (requires Qt)
casa_find( QWT
  VERSION 5.1.1
  INCLUDES qwt_global.h
  INCLUDES_SUFFIXES qwt qwt-qt4
  PREFIX_HINTS 
    ${QWT_ROOT_DIR}
    ${casa_packages}/qwt-5.2.0
    ${casa_packages}/qwt-5.2.1-svn
    ${casa_packages}/qwt-5.2.3-svn
    ${casa_packages}/qwt-5.2.4-svn
  LIBS qwt
  CPP_VERSION QWT_VERSION_STR
  DEPENDS QT4 )

# Xerces
casa_find( XERCES
  VERSION 2.7.0
  PREFIX_HINTS ${XERCES_ROOT_DIR}
  INCLUDES xercesc/util/XercesVersion.hpp
  LIBS xerces-c
  CPP_VERSION XERCES_FULLVERSIONDOT 
  RUN_VERSION "gXercesMajVersion << '.' << gXercesMinVersion << '.' << gXercesRevision" 
)
if( XERCES_VERSION VERSION_EQUAL   3.2 OR
    XERCES_VERSION VERSION_GREATER 3.2 )
  message( FATAL_ERROR "Xerces version (${XERCES_VERSION}) is too recent! Must be earlier than 3.0." )
endif()

#
# Boost
#
# Use the Boost finder that ships with CMake. It needs to be told where boost is.
# If the version is >= 1.35, require also boost-system.
#
# Note, find_package( Boost ... ) must be called for every cmake invocation
# in order to setup non-cached variables
#
if( NOT BOOST_ROOT )
  set( BOOST_ROOT ${casa_packages} )
endif()

set( boost_components regex program_options filesystem thread)
find_package( Boost REQUIRED ${boost_components} )

if( NOT Boost_FOUND )
  message( FATAL_ERROR "Boost could not be found. Please check!" )
endif()

# For boost >= 1.35, we need to link also to boost-system
if( Boost_VERSION GREATER 103499 )

  # Force redetection by resetting these variables
  if( NOT Boost_SYSTEM_FOUND )
    message( STATUS "Boost version (${Boost_VERSION}) is 1.35.0 or newer" )
    unset( Boost_FOUND )
    unset( Boost_INCLUDE_DIR CACHE )
  endif()
  set( boost_components regex program_options filesystem system )
  find_package( Boost REQUIRED ${boost_components} )

  if( NOT Boost_FOUND )
    message( FATAL_ERROR "Boost-system could not be found. Please check!" )
  endif()
  set(Boost_INCLUDE_DIRS ${Boost_INCLUDE_DIR}/boost)
  include_directories(${Boost_INCLUDE_DIRS})
endif() 

if( NOT Boost_VERSION GREATER 104099 AND NOT CASA_IGNORE_VERSION )
  message( FATAL_ERROR "Boost version (${Boost_VERSION}) is too old! Must be 104100 (1.41.0) or newer. Please check!" )
endif()


#
# RPFITS (Fortran)
#
casa_find( RPFITS
  VERSION 2.11
  INCLUDES RPFITS.h
  LIBS rpfits
  #RUN_VERSION names_.rpfitsversion
  PREFIX_HINTS ${RPFITS_ROOT_DIR}
  DEPENDS FORTRAN 
)

# Readline
casa_find( READLINE 
  VERSION 4.3
  INCLUDES readline/readline.h #depends on <cstdio> to compile
  LIBS readline ncurses
  PREFIX_HINTS ${READLINE_ROOT_DIR}
  CPP_VERSION "RL_VERSION_MAJOR << '.' << RL_VERSION_MINOR"
  RUN_VERSION rl_library_version
)

set( DL_LIBRARIES ${CMAKE_DL_LIBS} )

casa_config_end()

#
# The following variables are substitued into python scripts
#
set( PYTHON_LIBD ${CMAKE_INSTALL_PREFIX}/python/${PYTHONV} )
set( PYTHON_TASKD ${PYTHON_LIBD} )
#
# allow user to override contents of casadef.py (needed for linux rpms)
#
if( NOT PYTHONLIBD )
    set( PYTHON_LIBD_CASADEF ${PYTHON_LIBD} )
else()
    set( PYTHON_LIBD_CASADEF ${PYTHONLIBD} )
endif()
if( NOT PYTHONTASKD )
    set( PYTHON_TASKD_CASADEF ${PYTHON_TASKD} )
else()
    set( PYTHON_TASKD_CASADEF ${PYTHONTASKD} )
endif()

# The modules must be defined in dependency order!
# This will set up include paths, and which libraries to link to

casa_add_module( tools CASACORE Boost PYTHON QT4 DBUS ATM READLINE DL XERCES CASACODE )

#
# ASAP's build depends on something called a makedefs file. Fake one of those.
#

# Get library paths from the first of the rpfits/cfitsio/python library full paths.
list( GET RPFITS_LIBRARIES 0 rpfits_lib )
get_filename_component(rpfitslibd ${rpfits_lib} PATH)

list( GET CFITSIO_LIBRARIES 0 cfitsio_lib )
get_filename_component(cfitsiolibd ${cfitsio_lib} PATH)

list( GET PYTHON_LIBRARIES 0 python_lib )
get_filename_component(pythonlibd ${python_lib} PATH)

# convert semicolons to spaces...
set( _ii "" )
foreach( _i ${PYTHON_INCLUDE_DIRS} )
  set( _ii "${_ii} ${_i}" )
endforeach()

set( _cc "" )
foreach( _c ${CASACORE_LIBRARIES} )
  if( _c MATCHES Framework )
    # skip
  else()
    set( _cc "${_cc} ${_c}" )
  endif()
endforeach()

get_property( MYDEFINES DIRECTORY tools PROPERTY COMPILE_DEFINITIONS )
# convert semicolons to -Dmydefine...
set( _dd "" )
foreach( _d ${MYDEFINES} )
  set( _dd "${_dd} -D${_d}" )
endforeach()


#file( WRITE ${CMAKE_INSTALL_PREFIX}/makedefs
#"# This file exists only for ASAP\n"
#PYTHONVER=${PYTHONV}\n
#CXXFLAGS = \"${CMAKE_CXX_FLAGS}\"\n
#COPTFLAGS = \"${_dd}\"\n
#LD_FLAGS = \"${CMAKE_SHARED_LINKER_FLAGS}\"\n
#COREINCD = ${CASACORE_casa/aips.h}\n
#CORELIB = ${_cc}\n
#WCSLIBLIB = ${WCSLIB_wcs}\n
#WCSLIBLIBD = .\n
#PYTHONROOT = ${pyroot}\n
#PYTHONLIBD = ${pythonlibd}\n
#PYTHONINCD = ${_ii}\n
#PYTHONLIB = ${PYTHON_LIBRARIES}\n
#BOOSTROOT = ${BOOST_ROOT}\n
#RPFITSLIBD = ${rpfitslibd}\n
#CFITSIOLIBD = ${cfitsiolibd}\n
#CFITSIOINCD =  ${CFITSIO_INCLUDE_DIRS}\n
#eval_vars :\n
#\t@\ echo\ \"$(foreach\ V,$(VARS),$V=\\\"$($V)\\\")\ \;\ export\ $(VARS)\"\n
#"
#)
#
add_subdirectory(doc/taskref)
add_subdirectory(doc/casaref)
add_subdirectory(tasks)
add_subdirectory(python)

# All documentation
add_custom_target( doc DEPENDS taskref_html taskref_pdf casaref_html casaref_pdf )
#add_custom_target( doc )

configure_file( apps/casapy/casapy ${CMAKE_CURRENT_BINARY_DIR}/preprocessed/casapy @ONLY )
configure_file( apps/casapy/casa ${CMAKE_CURRENT_BINARY_DIR}/preprocessed/casa @ONLY )

install( PROGRAMS
         install/buildmytasks
         install/xmlgenpy
	 ${CMAKE_CURRENT_BINARY_DIR}/preprocessed/casa
	 ${CMAKE_CURRENT_BINARY_DIR}/preprocessed/casapy
         DESTINATION bin )

install( FILES 
         install/casa2c++h.xsl
         install/casa2py.xsl
         install/casa2pyimp.xsl
         install/casa2summary.xsl
	 install/casa2swigxml.xsl
         install/casa2tsum2.xsl
         install/casa2tsum3.xsl
         install/casa2latex.xsl
         install/casa2pycli.xsl
         install/casa2pypg.xsl
         install/casa2tlatex.xsl
         install/casa2tsum.xsl
         install/casatlist.xsl
         DESTINATION ${CMAKE_INSTALL_PREFIX}/xml )

install( FILES install/saxon8.jar DESTINATION lib )

