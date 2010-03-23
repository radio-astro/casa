#
# CASA - Common Astronomy Software Applications
# Copyright (C) 2010 by ESO (in the framework of the ALMA collaboration)
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

set( CASA_MAJOR_VERSION 3 )
set( CASA_MINOR_VERSION 0 )
set( CASA_PATCH_VERSION 1 )
set( CASA_API_VERSION "${CASA_MAJOR_VERSION}.${CASA_MINOR_VERSION}.${CASA_PATCH_VERSION}" )

# Define compiler paths on OSX 10.5. This must be done before defining the project()
if( APPLE AND EXISTS            /opt/casa/core2-apple-darwin8/3rd-party/bin/gfortran )
    set( CMAKE_Fortran_COMPILER /opt/casa/core2-apple-darwin8/3rd-party/bin/gfortran )
    set( CMAKE_CXX_COMPILER     /opt/casa/core2-apple-darwin8/3rd-party/bin/g++ )
endif()

project( CASA C CXX Fortran )
# C is enabled because 
# - FindQt4.cmake requires it (bug)
# - ccmtools uses the C preprocessor
if( CMAKE_Fortran_COMPILER STREQUAL CMAKE_Fortran_COMPILER-NOTFOUND )
  message( FATAL_ERROR "Could not find fortran compiler. Please check!" )
endif()

#
# Build options
#

# Build type (None, Release, Debug, RelWithDebInfo, MinSizeRel).
if( NOT CMAKE_BUILD_TYPE )
  set( CMAKE_BUILD_TYPE RelWithDebInfo )
endif()

if( CMAKE_BUILD_TYPE STREQUAL Debug )
  add_definitions( -DAIPS_DEBUG )
endif()

dump( CMAKE_HOST_SYSTEM_NAME
  CMAKE_HOST_SYSTEM
  CMAKE_HOST_UNIX
  CMAKE_HOST_APPLE
  CMAKE_HOST_WIN32
  CMAKE_SYSTEM_NAME
  CMAKE_SYSTEM
  CMAKE_SYSTEM_PROCESSOR
  CMAKE_SYSTEM_VERSION 
  CMAKE_GENERATOR 
  CMAKE_BUILD_TYPE )

set( BUILD_SHARED_LIBS ON )

set( DEFAULT_CXX_FLAGS 
     "-pipe -Wall -Wno-non-template-friend -Wcast-align -Wno-comment"
     )


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
    if( CMAKE_SYSTEM MATCHES ^Darwin-9 )
        set( casa_packages /opt/casa/core2-apple-darwin8/3rd-party )
        set( CMAKE_SHARED_LINKER_FLAGS "-Wl,-undefined -Wl,error" )
      elseif( CMAKE_SYSTEM MATCHES ^Darwin-10 )
        set( casa_packages /opt/casa/core2-apple-darwin10 )
        set( CMAKE_Fortran_FLAGS "${CMAKE_Fortran_FLAGS} -arch i386" )
        set( CMAKE_SHARED_LINKER_FLAGS "-Wl,-undefined -Wl,error -arch i386" )
        set( DEFAULT_CXX_FLAGS "${DEFAULT_CXX_FLAGS} -arch i386" )
    else()
        message( WARNING "Unsupported Mac: ${CMAKE_SYSTEM}" )
    endif()

    set( CMAKE_Fortran_FLAGS -fno-second-underscore )
    set( arch darwin )
    set( DEFAULT_CXX_FLAGS
      "${DEFAULT_CXX_FLAGS} -dynamic -mtune=nocona -ftree-vectorize -ffast-math -mfpmath=sse -fasm-blocks"
      )

elseif( CMAKE_SYSTEM_NAME STREQUAL Linux )

  set( CMAKE_SHARED_LINKER_FLAGS "-Wl,--no-undefined")

  if( CMAKE_SYSTEM_PROCESSOR STREQUAL x86_64 )
    set( casa_packages /usr/lib64/casapy )
    set( arch linux_64b )
    set( DEFAULT_CXX_FLAGS "${DEFAULT_CXX_FLAGS} -Wno-deprecated" )
  else()
    set( casa_packages /usr/lib/casapy )
    set( arch linux_gnu )
    set( DEFAULT_CXX_FLAGS "${DEFAULT_CXX_FLAGS} -Wno-deprecated -Woverloaded-virtual" )
  endif()
else()
  message( WARNING "Unsupported system: ${CMAKE_SYSTEM_NAME}" )
endif()

set( CMAKE_MODULE_LINKER_FLAGS ${CMAKE_SHARED_LINKER_FLAGS} )

if( NOT CMAKE_CXX_FLAGS )
  set( CMAKE_CXX_FLAGS "${DEFAULT_CXX_FLAGS}" )
endif()


string( REGEX REPLACE /[^/]+/?$ "" _casaroot ${CMAKE_SOURCE_DIR} )

#
# aipshosts is supported for backwards compatibility
#
if( EXISTS ${_casaroot}/aipshosts )
  message( WARNING "Found ${_casaroot}/aipshosts. You should probably delete this file, and the name of the installation directory will be automatically set (to \"${arch}\") based on this machine's architecture." )
  execute_process( COMMAND uname -n
                   COMMAND awk -F. "{print $1}"
                   OUTPUT_VARIABLE _host OUTPUT_STRIP_TRAILING_WHITESPACE )
  execute_process( COMMAND egrep "^[\ ]*${_host}[\ ]" ${_casaroot}/aipshosts 
                   COMMAND awk "{print $2}"
    OUTPUT_VARIABLE _a
    OUTPUT_STRIP_TRAILING_WHITESPACE
    )

  if( _a )
    set( arch ${_a} )    
  endif()
endif()

#
# The layout of the source+install directory trees
# is rather hard-coded in much source code. Do not
# allow the user to install anywhere else.
#
set( CMAKE_INSTALL_PREFIX ${_casaroot}/${arch} )
dump( CMAKE_INSTALL_PREFIX )

#
# Create casainit.sh
# The environment defined here is not needed at build time, but at runtime.
# The definitions are architecture dependent, hence put them in
# the install directory
#
file( WRITE ${CMAKE_INSTALL_PREFIX}/casainit.sh
      "#!/bin/sh\n"
      "# Generated by CMake, do not edit\n"
      "  export CASAPATH=\"${_casaroot} ${arch}\"\n"
      "  export PATH=${casa_packages}/bin:$PATH:${CMAKE_INSTALL_PREFIX}/bin\n"
)

# Create a top-level casainit.sh which detects the architecture
# and sources the right <arch>/casainit.sh
file( WRITE ${_casaroot}/casainit.sh 
      "#!/bin/sh\n"
      "# Generated by CMake, do not edit\n"
      "a_root=${_casaroot}\n"
      "a_host=`uname -n | awk -F. '{ print $1 }'`\n"
      "a_temp=\"\"\n"
      "if [ -f \"$a_root/aipshosts\" ]\n"
      "then\n"
      "    a_temp=`egrep \"^[  ]*$a_host[ ]\" \"$a_root/aipshosts\"`\n"
      "    a_arch=`/bin/echo $a_temp | awk '{ print $2 }'`\n"
      "fi\n"
      "if [ \"$a_arch\" == \"\" ]\n"
      "then\n"
      "    case `uname -s` in\n"
      "    Linux)\n"
      "        case `uname -p` in\n"
      "        x86_64)\n"
      "            a_arch=linux_64b\n"
      "            ;;\n"
      "        *)\n"
      "            a_arch=linux_gnu\n"
      "            ;;\n"
      "        esac\n"
      "        ;;\n"
      "    Darwin)\n"
      "        a_arch=darwin\n"
      "        ;;\n"
      "    *)\n"
      "        echo \"Unknown uname -s: `uname -s`\"\n"
      "        ;;\n"
      "    esac\n"
      "fi\n"
      "if [ \"$a_arch\" != \"\" ]\n"
      "then\n"
      "    . $a_root/$a_arch/casainit.sh\n"
      "fi\n"
)


# Do not verify the contents of CASAPATH, it is not used by the build system
#message( STATUS "$CASAPATH = $ENV{CASAPATH}" )
#if( NOT $ENV{CASAPATH} MATCHES "^${_casaroot}/? +${arch}" )
#  message( FATAL_ERROR "The environment variable CASAPATH must start with \"${_casaroot} ${arch}\". You should source ${_casaroot}/casainit.sh" )
#endif()

# Choose to have correct RPATHs both in the build tree and
# in the install tree (at the cost of having to change the
# rpath when installing)
set( CMAKE_INSTALL_RPATH ${CMAKE_INSTALL_PREFIX}/lib )
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
    -DMACOSXBUILT
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
  INCLUDES_HINTS /usr/include/cfitsio
  LIBS cfitsio
  CPP_VERSION CFITSIO_VERSION
  RUN_VERSION "(ffvers(&v), v)" )


# X11.
find_package( X11 REQUIRED )
# Rename
set( X11_INCLUDE_DIRS ${X11_INCLUDE_DIR} )

#X11LIB   := -lXt -lXmu -lX11 -lXext

# PGPLOT (FORTRAN plotting package).
# depends on X11
casa_find( PGPLOT
  VERSION 5.3.1
  INCLUDES cpgplot.h
  LIBS pgplot cpgplot
  DEPENDS X11 )

# WCSLIB
casa_find( WCSLIB
  VERSION 4.3
  INCLUDES wcslib/wcsconfig.h
  LIBS wcs
       pgsbox
  CPP_VERSION WCSLIB_VERSION
  DEFINITIONS -DSIGNBIT 
  DEPENDS PGPLOT )

# pgsbox is a Fortran library. Since we are using the C++ linker,
# the Fortran runtime libraries must be explicitly linked in.
# There must be a better way of doing this...
if( APPLE )
  if( CMAKE_SYSTEM MATCHES ^Darwin-9 )
    set( WCSLIB_LIBRARIES ${WCSLIB_LIBRARIES} ${casa_packages}/lib/gcc42/libgfortran.dylib )
  endif()
elseif( CMAKE_SYSTEM_PROCESSOR STREQUAL x86_64 )
  if( EXISTS /usr/lib64/libg2c.so.0 )
    set( WCSLIB_LIBRARIES ${WCSLIB_LIBRARIES} /usr/lib64/libg2c.so.0 )
  endif()
  if( EXISTS /usr/lib64/libgfortran.so.1 )
    set( WCSLIB_LIBRARIES ${WCSLIB_LIBRARIES} /usr/lib64/libgfortran.so.1 )
  endif()
else()
  if( EXISTS /usr/lib/libg2c.so.0 )
    set( WCSLIB_LIBRARIES ${WCSLIB_LIBRARIES} /usr/lib/libg2c.so.0 )
  endif()
  if( EXISTS /usr/lib/libgfortran.so.1 )
    set( WCSLIB_LIBRARIES ${WCSLIB_LIBRARIES} /usr/lib/libgfortran.so.1 )
  endif()
endif()


# BLAS
casa_find( BLAS
           LIBS blas )

# LAPACK
casa_find( LAPACK
           LIBS lapack 
           DEPENDS BLAS )

# CASACore
# - Some header files exported by CASACore include WCSLIB headers
# - CASACore header files declare symbols which are defined in LAPACK and BLAS
casa_find( CASACORE
           VERSION 8.1
           INCLUDES casa/aips.h
                    scimath/Mathematics/MatrixMathLA.h  # declares symbols defined in LAPACK
           INCLUDES_HINTS ${CMAKE_INSTALL_PREFIX}/include/casacore
           LIBS casa_casa
                casa_components
                casa_coordinates
                casa_fits
                casa_images
                casa_lattices
                casa_measures
                casa_ms
                casa_msfits
                casa_scimath
                casa_scimath_f
                casa_tables 
           DEPENDS LAPACK CFITSIO WCSLIB )
              
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



# Python
#
# Using different versions of Python depending on platform!
# The shared libraries have version dependent names.
set( PV 25 )
set( PYTHONV 2.5 )
set( PYTHONVERS 2.5.2 )
if( APPLE )
  if( CMAKE_SYSTEM MATCHES ^Darwin-9 )
    set( python_library python2.5.2 )
  else()
    set( PV 26 )
    set( PYTHONV 2.6 )
    set( PYTHONVERS 2.6.4 )
    set( python_library python2.6 )  
  endif()
else()
  set( python_library python2.5 )
endif()

casa_find( PYTHON
  VERSION EXACT ${PYTHONVERS}
  INCLUDES Python.h 
     numpy/npy_interrupt.h   # for numpy
    # Does not exist on 10.5.5: numpy/numpyconfig.h
  INCLUDES_HINTS
    ${casa_packages}/include/python${PYTHONV} 
    ${casa_packages}/lib/python${PYTHONV}/site-packages/numpy/core/include
    ${casa_packages}/Library/Frameworks/Python.framework/Versions/${PYTHONV}/include/python${PYTHONV}
    ${casa_packages}/Library/Frameworks/Python.framework/Versions/${PYTHONV}/lib/python${PYTHONV}/site-packages/numpy/core/include
  LIBS ${python_library}
  CPP_VERSION PY_VERSION )


if( APPLE )
    if( CMAKE_SYSTEM MATCHES ^Darwin-9 )
        set( pyroot ${casa_packages}/lib/Python.framework/Versions/${PYTHONV} )
    else()
       set( pyroot ${casa_packages}/Library/Frameworks/Python.framework/Versions/${PYTHONV} )
    endif()
else()
    set( pyroot ${casa_packages} )
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
  PROG_VERSION "${Java_java} -version")

set( SAXON ${Java_java_EXECUTABLE} -jar ${CMAKE_SOURCE_DIR}/xmlcasa/install/saxon8.jar )

# C++ DBus
if( APPLE )
  set( DBUSARCH_INCLUDE_DIRS
    ${casa_packages}/lib/dbus-1.0/include
    )
else()
  set( DBUSARCH_INCLUDE_DIRS 
    /usr/lib/qt-4.3.4/dbus/lib/dbus-1.0/include
    /usr/lib/dbus-1.0/include
    )
endif()

casa_find( DBUS
  1.0
  INCLUDES dbus/dbus.h
           dbus-c++/dbus.h # dbus/dbus-arch-deps.h
  INCLUDES_HINTS
    /usr/lib/qt-4.3.4/dbus/include/dbus-1.0
    /usr/lib/qt-4.3.4/dbus/include/dbus-c++-1
    /usr/include/dbus-1.0
    /usr/include/dbus-c++-1
    ${casa_packages}/dbus-1.0/include
    ${casa_packages}/include/dbus-1.0
    ${casa_packages}/include/dbus-c++-1
  LIBS dbus-c++-1
  LIBS_HINTS /usr/lib/qt-4.3.4/dbus/lib
  PROGRAMS dbusxx-xml2cpp
  DEPENDS DBUSARCH
)

# ATM
casa_find( ATM
  VERSION 0.3
  INCLUDES ATMEnumerations.h
  INCLUDES_HINTS /usr/include/ATM
  LIBS aatm
  #CPP_VERSION ATM_VERSION
  #RUN_VERSION "atm::getVersion()" 
)

# XML2
casa_find( LIBXML2
  VERSION 2.6.16
  INCLUDES libxml/xmlversion.h
  INCLUDES_HINTS /usr/include/libxml2
  LIBS xml2
  CPP_VERSION LIBXML_DOTTED_VERSION 
  RUN_VERSION "LIBXML_DOTTED_VERSION; LIBXML_TEST_VERSION;" 
  )

# Do not reuse FindLibXml2.cmake, except this excerpt
IF (NOT WIN32)
   # use pkg-config to get the directories and then use these values
   # in the FIND_PATH() and FIND_LIBRARY() calls
   FIND_PACKAGE(PkgConfig)
   PKG_CHECK_MODULES(PC_LIBXML libxml-2.0)
   SET(LIBXML2_DEFINITIONS ${LIBXML2_DEFINITIONS} ${PC_LIBXML_CFLAGS_OTHER})
ENDIF (NOT WIN32)

# CCMTools
file( WRITE ${CMAKE_BINARY_DIR}/ccmtools_test.py
      "import sys\n"
      "print 'Successfully launched IPython from C++.'\n" 
      "sys.exit(0)\n")
#
# The names of CCMTools libraries changed.
# Need to support both naming conventions, until the new
# version is universally available. (This implementation
# would be cleaner if CCM exported a version number.)
#
if( EXISTS ${casa_packages}/lib/libCCM_Python.so OR
    EXISTS ${casa_packages}/lib64/libCCM_Python.so OR
    EXISTS ${casa_packages}/lib/libCCM_Python.dylib )
  set( _ccmtools_libs CCM_Python CCM_Local_HomeFinder WX_Utils_error WX_Utils_code WX_Utils_types )
else()
  set( _ccmtools_libs ccmtools_local )
endif()

casa_find( CCMTOOLS
  VERSION 0.5.5
  DEPENDS PYTHON
  INCLUDES CCM_Local/HomeFinder.h
           CCM_Python/BasicTypes.h
           CCM_Python/Py.h
           WX/Utils/error.h
           WX/Utils/type.h
  LIBS ${_ccmtools_libs}
  PROGRAMS ccmtools
  PROG_VERSION "--version" 
  EXTRA_TEST " 
    CCM_Python::Py::init(argc, argv, \"${pyroot}/site-packages:${CMAKE_BINARY_DIR}\");
    CCM_Python::Py py;
    py.evalFile(\"ccmtools_test.py\");
    /* This code is not reached, if IPython was launched */
    return 1;
    "
)

set( CCMTOOLS_ccmtools_FLAGS -Xms1024M -Xmx1024M )

# QT4  4.3.4 or 4.5.1

find_package(
  Qt4 4.3.4
  COMPONENTS QtCore QtGui QtDBus QtXml )
# Version checking not supported...

set( QT4_INCLUDE_DIRS ${QT_INCLUDES} CACHE PATH "QT4 include directories" FORCE )
set( QT4_DEFINITIONS ${QT_DEFINITIONS} -DQT_NO_DEBUG CACHE STRING "QT4 preprocessor flags" FORCE )
set( QT4_LIBRARIES 
  ${QT_QTCORE_LIBRARY} 
  ${QT_QTGUI_LIBRARY} 
  ${QT_QTDBUS_LIBRARY} 
  ${QT_QTXML_LIBRARY} 
  CACHE FILEPATH "QT4 libraries" FORCE )
set( QT4_FOUND TRUE CACHE BOOL "Was QT4 found?" )

# Detecting QT4 takes a bit of time, but does not seem possible to skip.
# If doing so, then "include( ${QT_USE_FILE} )" does not work (because QT_USE_FILE is not cached?).


# QWT (requires Qt)
casa_find( QWT
  VERSION 5.1.1
  INCLUDES qwt_global.h
  INCLUDES_HINTS 
    /usr/include/qwt
    ${casa_packages}/qwt-5.2.0/include
    ${casa_packages}/qwt-5.2.1-svn/include
  LIBS qwt
  LIBS_HINTS ${casa_packages}/qwt-5.2.0/lib 
             ${casa_packages}/qwt-5.2.1-svn/lib
  CPP_VERSION QWT_VERSION_STR
  DEPENDS QT4 )

# Xerces
casa_find( XERCES
  VERSION 2.7.0
  INCLUDES xercesc/util/XercesVersion.hpp
  LIBS xerces-c
  CPP_VERSION XERCES_FULLVERSIONDOT 
  RUN_VERSION "gXercesMajVersion << '.' << gXercesMinVersion << '.' << gXercesRevision" )

# Boost
if( APPLE )
  set( boost_components regex program_options filesystem system )
else()
  set( boost_components regex program_options filesystem )
endif()
# Use the Boost finder that ships with CMake. It needs to be told where boost is...
set( BOOST_ROOT ${casa_packages} )
find_package( Boost 1.32 REQUIRED ${boost_components} )
# The macro does not implement version check
if( NOT Boost_FOUND )
  message( FATAL_ERROR "Boost could not be found. Please check!" )
endif()

# RPFITS (Fortran)
# On OSX 10.6 only, libgfortran needs to be linked in.
set( RPFITS_FORTRAN_LIBRARIES "" )
set( RPFITS_FORTRAN_INCLUDE_DIRS /usr/include )
if( APPLE )
  if( CMAKE_SYSTEM MATCHES ^Darwin-10 )
    set( RPFITS_FORTRAN_LIBRARIES -lgfortran )
  endif()
endif()
casa_find( RPFITS
  VERSION 2.11
  INCLUDES RPFITS.h
  LIBS rpfits
  DEFINITIONS ${defs}
  RUN_VERSION names_.rpfitsversion
  DEPENDS RPFITS_FORTRAN 
)

# Readline
casa_find( READLINE 
  VERSION 4.3
  INCLUDES readline/readline.h #depends on <cstdio> to compile
  LIBS readline ncurses
  CPP_VERSION "RL_VERSION_MAJOR << '.' << RL_VERSION_MINOR"
  RUN_VERSION rl_library_version )

set( DL_LIBRARIES ${CMAKE_DL_LIBS} )

#
# ASAP depends on something called a makedefs file. Fake one of those.
#

# Get path from first of rpfits libraries.
list( GET ${RPFITS_LIBRARIES} 0 rpfits_lib )
get_filename_component(rpfitslibd ${rpfits_lib} PATH)

# convert semicolons to spaces...
set( _ii "" )
foreach( _i ${PYTHON_INCLUDE_DIRS} )
  set( _ii "${_ii} ${_i}" )
endforeach()

set( _cc "" )
foreach( _c ${CASACORE_LIBRARIES} )
  if (_c STREQUAL dl)
    set( _cc "${_cc} -ldl" )
  elseif( _c MATCHES Framework )
    # skip
  else()
    set( _cc "${_cc} ${_c}" )
  endif()
endforeach()

file( WRITE ${CMAKE_INSTALL_PREFIX}/makedefs
  "# This file exists only for ASAP\n"
  PYTHONVER=${PYTHONV}\n
  COREINCD = ${CASACORE_casa/aips.h}\n
  CORELIB = ${_cc}\n
  WCSLIBLIBD = .\n
  WCSLIBLIB = ${WCSLIB_wcs}\n
  PYTHONROOT = ${pyroot}\n
  PYTHONLIBD = ${casa_packages}/lib\n
  PYTHONINCD = ${_ii}\n
  PYTHONLIB = ${PYTHON_LIBRARIES}\n
  BOOSTROOT = ${BOOST_ROOT}\n
  RPFITSLIBD = ${rpfitslibd}\n
  eval_vars :\n
  \t@\ echo\ \"$(foreach\ V,$(VARS),$V=\\\"$($V)\\\")\ \;\ export\ $(VARS)\"\n
)


# The following variables are substitued into python scripts
set( PYTHON_LIBD ${CMAKE_INSTALL_PREFIX}/python/${PYTHONV} )
set( PYTHON_TASKD ${PYTHON_LIBD} )

# Temporarily set LC_TIME to a value which causes
# the date command to speak English.
# There was a problem with LC_TIME=Japanese causing
# a runtime crash
set( _lc_time_old $ENV{LC_TIME} )
set( ENV{LC_TIME} en_US.UTF-8 )
execute_process( COMMAND
  date -u "+%a %Y/%m/%d %H:%M:%S UTC"
  OUTPUT_VARIABLE BUILDTIME 
  OUTPUT_STRIP_TRAILING_WHITESPACE
  )
set( ENV{LC_TIME} ${_lc_time_old} )

execute_process(
  COMMAND ${PERL_EXECUTABLE} -e "open(INFO, 'svn info |') ; while (<INFO>){ if ( s/^Revision:\\s+//) { print; } }"
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  OUTPUT_VARIABLE SVNREVISION 
  OUTPUT_STRIP_TRAILING_WHITESPACE
  )

execute_process( COMMAND
  ${PERL_EXECUTABLE} -e "open(INFO, 'svn info |') ; while (<INFO>){ if ( s/^URL:\\s+//) { print; } }"
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  OUTPUT_VARIABLE SVNURL
  OUTPUT_STRIP_TRAILING_WHITESPACE
  )

execute_process( COMMAND
  ${PERL_EXECUTABLE} -e "open(VERSION, '< VERSION'); $_ = <VERSION>; s/^\\d+?\\d\\.\\d+\\.(\\d+).*/$1/; print;"
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  OUTPUT_VARIABLE CASABUILD
  OUTPUT_STRIP_TRAILING_WHITESPACE
)


# The modules must be defined in dependency order!
# This will set up include paths, and which libraries to link to
casa_add_module( graphics CASACORE PGPLOT X11 )
casa_add_module( casadbus CASACORE DBUS )
casa_add_module( tableplot CASACORE PYTHON )
casa_add_module( msvis CASACORE graphics )
casa_add_module( casaqt CASACORE QT4 QWT PYTHON XERCES graphics )
casa_add_module( plotms CASACORE QT4 casaqt msvis )
casa_add_module( display CASACORE WCSLIB QT4 casaqt msvis )
casa_add_module( flagging CASACORE Boost tableplot msvis )
casa_add_module( calibration CASACORE tableplot msvis )
casa_add_module( synthesis CASACORE ATM casadbus msvis calibration )
casa_add_module( alma CASACORE LIBXML2 Boost )
casa_add_module( oldalma CASACORE LIBXML2 Boost )
casa_add_module( atnf CASACORE RPFITS CFITSIO )
casa_add_module( dish CASACORE )
casa_add_module( nrao CASACORE )
casa_add_module( xmlcasa CASACORE CCMTOOLS PYTHON ATM READLINE DL plotms display flagging synthesis atnf dish nrao )


# demo atmosphere ionosphere jive nfra simulators vlbi vo

# Create a target which ensures that the install target is 
# always run as part of all.
add_custom_target( inst ALL ${CMAKE_BUILD_TOOL} install/fast )
add_dependencies( inst
  casac interrupt paramgui
          casapy 
          casaparamgui 
          casasplit
          casafilecatalog
          casalogger
          casapictureviewer
          casaplotserver
          casaprogresstimer
          qcasabrowser
          qwtplottertest
          casaplotms
          casaplotter
          casaviewer
          tasks
          xmlcasa_tasks
          xmlcasa_python
          xmlcasa_regressions_python
          xmlcasa_regressions_tests_python
          xmlcasa_usecases_python
          xmlcasa_demos_python
          alma
          asdm2MS
          oldalma
          oldasdm2MS )
