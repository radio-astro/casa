###
# CMakeLists.txt for ASAP module
###

# minimum requirement for cmake version
cmake_minimum_required( VERSION 2.8 )

# Define compiler paths on OSX 10.5. This must be done before invoking project()
if( APPLE )
    if( EXISTS            /opt/casa/core2-apple-darwin8/3rd-party/bin/gfortran )
    set( SO dylib )
        set( CMAKE_Fortran_COMPILER /opt/casa/core2-apple-darwin8/3rd-party/bin/gfortran )
	set( CMAKE_CXX_COMPILER     /opt/casa/core2-apple-darwin8/3rd-party/bin/g++ )
    elseif( EXISTS        /opt/local/bin/gfortran )
        set( CMAKE_Fortran_COMPILER /opt/local/bin/gfortran )
    elseif( EXISTS        /opt/local/bin/gfortran-mp-4.4 )
        set( CMAKE_Fortran_COMPILER /opt/local/bin/gfortran-mp-4.4 )
	if( EXISTS /opt/local/bin/g++-mp-4.4 )
	    set( CMAKE_CXX_COMPILER /opt/local/bin/g++-mp-4.4 )
	endif()
    endif()
endif()

# project name is ASAP
project( ASAP )

# install directory
# set casaroot
# the regular expression means '../'
#  [^ ] Matches any character(s) not inside the brackets
#  +    Matches preceding pattern one or more times
#  ?    Matches preceding pattern zero or once only
#  $    Mathces at end of a line
string( REGEX REPLACE /[^/]+/?$ "" casaroot ${CMAKE_SOURCE_DIR} )
message( STATUS "casaroot = " ${casaroot} )

# modules
set( CMAKE_MODULE_PATH ${casaroot}/code/install )
include( config )
include( CASA )

# flags
set( DEFAULT_CXX_FLAGS
     "-pipe -Wall -Wextra -Wno-non-template-friend -Wcast-align -Wno-comment -O3" )
find_package( OpenMP )
if( OPENMP_FOUND )
   set( DEFAULT_CXX_FLAGS "${DEFAULT_CXX_FLAGS} ${OpenMP_CXX_FLAGS}" )
endif()

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

add_definitions( ${CASACORE_DEFINITIONS} )

# environment dependent settings
message( STATUS "CMAKE_SYSTEM = " ${CMAKE_SYSTEM} )
if( APPLE )
   set( SO dylib )
   if( NOT arch )
      set( arch darwin )
   endif()
   if( CMAKE_SYSTEM MATCHES ^Darwin-10 )
      if( NOT archflag )
         if( EXISTS /opt/casa/darwin10-64b )
            set( archflag x86_64 )
         elseif( EXISTS /opt/casa/core2-apple-darwin10 )
            set( archflag i386 )
         else()
            set( archflag x86_64 )
         endif()
      endif()
      if( archflag STREQUAL x86_64 )
         add_definitions( -DAIPS_64B )
         set( casa_packages /opt/casa/darwin10-64b )
      else()
         set( casa_packages /opt/casa/core2-apple-darwin10 )
      endif()
      execute_process( COMMAND ${CMAKE_CXX_COMPILER} --version
                       COMMAND head -1
                       COMMAND perl -pe "s|.*?(\\d+\\.\\d+)\\.\\d+$|$1|"
                       OUTPUT_VARIABLE _cxx_version
                       OUTPUT_STRIP_TRAILING_WHITESPACE )
      if( NOT _cxx_version STREQUAL "4.4" )
         set( DEFAULT_CXX_FLAGS "${DEFAULT_CXX_FLAGS} -arch ${archflag}" )
      endif()
   elseif( CMAKE_SYSTEM MATCHES ^Darwin-9 )
      set( casa_packages /opt/casa/core2-apple-darwin8/3rd-party )
   endif()         
elseif( CMAKE_SYSTEM_NAME STREQUAL Linux )
   set( SO so )
   add_definitions( -DAIPS_LINUX )
   if( CMAKE_SYSTEM_PROCESSOR STREQUAL x86_64 )
      set( casa_packages /usr/lib64/casapy )
      if( NOT arch )
         set( arch linux_64b )
      endif()
      add_definitions( -DAIPS_64B )
      set( DEFAULT_CXX_FLAGS "${DEFAULT_CXX_FLAGS} -Wno-deprecated" )
   else()
      set( casa_packages /usr/lib/casapy )
      if( NOT arch )
         set( arch linux_gnu )
      endif()
      set( DEFAULT_CXX_FLAGS "${DEFAULT_CXX_FLAGS} -Wno-deprecated -Woverloaded-virtual" )
   endif()
endif()
message( STATUS "arch = " ${arch} )

# set root directory for installation
set( CMAKE_INSTALL_PREFIX ${casaroot}/${arch} )
message( STATUS "CMAKE_INSTALL_PREFIX = " ${CMAKE_INSTALL_PREFIX} )

# set flags for cpp compiler
set( CMAKE_CXX_FLAGS ${DEFAULT_CXX_FLAGS} )


#
# DL
#
set( DL_LIBRARIES ${CMAKE_DL_LIBS} CACHE STRING "dl libraries" FORCE )
if( DL_LIBRARIES STREQUAL "dl" )
  set( DL_LIBRARIES "-ldl" CACHE STRING "dl libraries" FORCE )
endif()
message( STATUS "DL_LIBRARIES = " ${DL_LIBRARIES} )


#
# BLAS
#
find_library( BLAS_LIBRARIES libblas.${SO} )
message( STATUS "BLAS_LIBRARIES = " ${BLAS_LIBRARIES} )


#
# LAPACK
#
find_library( LAPACK_LIBRARIES liblapack.${SO} )
message( STATUS "LAPACK_LIBRARIES = " ${LAPACK_LIBRARIES} ) 


#
# casacore
#
if( NOT USE_LIBCASACORE )
   # use casacore libraries 
   set( _includename casa/aipsdef.h )
   find_path( CASACORE_INCLUDE_DIR ${_includename} 
              HINTS ${casaroot}/${arch}/include/
              PATH_SUFFIXES casacore ) 
   if( CASACORE_INCLUDE_DIR STREQUAL CASACORE_INCLUDE_DIR-NOTFOUND )
     message( FATAL_ERROR "${_includename} could not be found. Please check!" )
   endif()
   set( CASACORE_LIBS casa
                      components
                      coordinates
                      fits
                      images
                      lattices
                      measures
                      mirlib
                      ms
                      msfits
                      scimath
                      scimath_f
                      tables )
   set( _casacore_libs "" )
   foreach( _a ${CASACORE_LIBS} )
      set( _libname libcasa_${_a}.${SO} )
      unset( _casacore_lib CACHE )
      find_library( _casacore_lib ${_libname}
                    PATHS ${CMAKE_INSTALL_PREFIX} PATH_SUFFIXES lib )
      if( NOT _casacore_lib-NOTFOUND )
         #list( APPEND _casacore_libs casa_${_a} )
         list( APPEND _casacore_libs ${_casacore_lib} )
      endif()
   endforeach()
   set( CASACORE_LIBRARIES ${_casacore_libs} )
else()
   # use libcasacore
   set( _libname libcasacore.${SO} )
   find_library( CASACORE_LIBRARIES ${_libname} )
   if( CASACORE_LIBRARIES STREQUAL CASACORE_LIBRARIES-NOTFOUND )
      message( FATAL_ERROR "${_libname} could not be found. Please check!" )
   endif()
   set( _includename casa/aipsdef.h )
   find_path( CASACORE_INCLUDE_DIR ${_includename} 
              PATH_SUFFIXES casacore ) 
   if( CASACORE_INCLUDE_DIR STREQUAL CASACORE_INCLUDE_DIR-NOTFOUND )
     message( FATAL_ERROR "${_includename} could not be found. Please check!" )
   endif()
endif()
message( STATUS "CASACORE_LIBRARIES = " ${CASACORE_LIBRARIES} )
message( STATUS "CASACORE_INCLUDE_DIR = " ${CASACORE_INCLUDE_DIR} )


#
# Python
#
if( NOT PYTHON_FOUND )

  if ( NOT PYTHON_LIBNAME )
    set( _names 2.9 2.8 2.7 2.6 2.5.2 2.5 )
    # (The library named libpython.2.5.2.dylib seems to exist only in the CASA world.)
  else()
    set( _names ${PYTHON_LIBNAME} )
  endif()

  set( _found False )
  foreach( _v ${_names} )

    casa_find( 
      PYTHON${_v} 
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

# The python version and prefix is known, do the actual search
if( NOT PYTHON_FOUND )
  message( STATUS "Looking for PYTHON version ${PYTHONV}.x in ${python_prefix}" )
endif()

casa_find( PYTHON
  VERSION 2.5    # minimum required
  INCLUDES Python.h 
     numpy/npy_interrupt.h   # for numpy
  INCLUDES_SUFFIXES python${PYTHONV}
  PREFIX_HINTS 
     ${python_prefix}/lib/python${PYTHONV}/site-packages/numpy/core
     ${python_prefix}/Library/Frameworks/Python.framework/Versions/${PYTHONV}
     ${python_prefix}/Library/Frameworks/Python.framework/Versions/${PYTHONV}/lib/python${PYTHONV}/site-packages/numpy/core
  LIBS ${python_library}
  CPP_VERSION PY_VERSION )

# Store PYTHON_LIBNAME in the cache
set( PYTHON_LIBNAME ${PYTHON_LIBNAME} CACHE STRING "Python major and minor version to use" FORCE )

# Define pyroot to two directory levels above Python.h.
string( REGEX REPLACE "/[^/]+/[^/]+/?$" "" pyroot ${PYTHON_Python.h} )

message( STATUS "PYTHON_INCLUDE_DIR = " ${PYTHON_INCLUDE_DIR} )
message( STATUS "PYTHON_LINRARIES = " ${PYTHON_LIBRARIES} )
message( STATUS "PYTHONV = " ${PYTHONV} )

set( PYTHON_DEFINITIONS ${PYTHON_DEFINITIONS}
  -DAIPSROOT=\"${CMAKE_SOURCE_DIR}\"
  -DAIPSARCH=\"${arch}\"
  -DAIPSSITE=\"garching\"
  -DPYTHONROOT=\"${pyroot}\"
  -DPYTHONVER=\"${PYTHONV}\"
  -DPYVERSION=${PV} )


#
# Boost
#
if( NOT BOOST_ROOT )
   set( BOOST_ROOT ${casa_packages} )
endif()

set( boost_components python )
find_package( Boost REQUIRED ${boost_components} )
if( NOT Boost_FOUND )
  message( FATAL_ERROR "Boost could not be found. Please check!" )
endif()
message( STATUS "BOOST_INCLUDE_DIR = " ${Boost_INCLUDE_DIR} )
message( STATUS "BOOST_LIBRARIES = " ${Boost_LIBRARIES} )


#
# cfitsio
#
casa_find( CFITSIO
           VERSION 3.006
           INCLUDES fitsio.h fitsio2.h
           INCLUDES_SUFFIXES cfitsio
           LIBS cfitsio
           CPP_VERSION CFITSIO_VERSION
           RUN_VERSION "(ffvers(&v), v)" )
if( NOT CFITSIO_FOUND )
   message( FATAL_ERROR "CFITSIO could not be found. Please check!" )
endif()
message( STATUS "CFITSIO_INCLUDE_DIRS = " ${CFITSIO_INCLUDE_DIRS} )
message( STATUS "CFITSIO_LIBRARIES = " ${CFITSIO_LIBRARIES} )


#
# wcslib
#
set( _wcslib libwcs.${SO} )
set( _wcs_version 4.3 )
find_library( WCSLIB ${_wcslib} 
              HINTS /usr/lib64 /usr/lib ${casaroot}/${arch}/lib )
if( WCSLIB STREQUAL WCSLIB-NOTFOUND )
   message( STATUS "${_wcslib} could not be found." )
   unset( _wcslib CACHE )
   unset( WCSLIB CACHE ) 
   set( _wcslib libwcs.${_wcs_version}.${SO} )
   message( STATUS "Try to find ${_wcslib}..." )
   find_library( WCSLIB ${_wcslib} 
                 HINTS /usr/lib64 /usr/lib ${casaroot}/${arch}/lib )
   if( WCSLIB STREQUAL WCSLIB-NOTFOUND )
      message( FATAL_ERROR "${_wcslib} could not be found. Please check!" )
   endif()
endif()
message( STATUS "WCSLIB = " ${WCSLIB} )
find_path( WCSLIB_INCLUDE_DIR wcslib/wcs.h
           HINTS /usr/include ${casaroot}/${arch}/include )
if( WCSLIB_INCLUDE_DIR STREQUAL WCSLIB_INCLUDE_DIR-NOTFOUND )
   message( FATAL_ERROR "wcs.h could not be found. Please check!" )
endif()
message( STATUS "WCSLIB_INCLUDE_DIR = " ${WCSLIB_INCLUDE_DIR} )


#
# RPFITS (Fortran)
#
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

casa_find( RPFITS
  VERSION 2.11
  INCLUDES RPFITS.h
  LIBS rpfits
  RUN_VERSION names_.rpfitsversion
  DEPENDS FORTRAN 
)
if( NOT RPFITS_FOUND )
   message( FATAL_ERROR "${_wcslib} could not be found. Please check!" )
endif()
message( STATUS "RPFITS_INCLUDE_DIR = " ${RPFITS_INCLUDE_DIR} )
message( STATUS "RPFITS_LIBRARIES = " ${RPFITS_LIBRARIES} )


#
# common include path
#
message( STATUS "Boost_INCLUDE_DIR = " ${Boost_INCLUDE_DIR} )
include_directories( ${CASACORE_INCLUDE_DIR} 
                     ${Boost_INCLUDE_DIR} 
                     ${PYTHON_INCLUDE_DIR} 
                     ${WCSLIB_INCLUDE_DIR} 
                     ${CFITSIO_INCLUDE_DIRS} 
                     ${RPFITS_INCLUDE_DIR} )

#
# link path
#
#set( CASACORE_LIBRARY_DIR ${casaroot}/${arch}/lib )
#link_directories( ${CASACORE_LIBRARY_DIR} )


#
# install directory
#
set( EXEC_INSTALL_DIR bin )
set( LIB_INSTALL_DIR lib )
set( PYTHON_INSTALL_DIR python/${PYTHONV}/asap )
set( SHARED_INSTALL_DIR share/asap )


#
# libraries and executables
#
set( ASAPLIB asap )
set( PYRAPLIB pyrap )
set( ATNFLIB atnf )
set( ASAP2TO3 asap2to3 )


#
# subdirectories
#  ASAP2TO3 asap2to3       apps
#  PYRAPLIB libpyrap.so    external/libpyrap
#  ATNFLIB  libatnf.so     external-alma/atnf
#  ASAPLIB  _asap.so       src
#  python modules           python
#  shared files             share
#
add_subdirectory( apps )
add_subdirectory( external/libpyrap )
add_subdirectory( external-alma/atnf )
add_subdirectory( src )
add_subdirectory( python )
add_subdirectory( share )


