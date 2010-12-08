###
# CMakeLists.txt for ASAP module
###

# minimum requirement for cmake version
cmake_minimum_required( VERSION 2.8 )

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

# flags
set( DEFAULT_CXX_FLAGS
     "-pipe -Wall -Wextra -Wno-non-template-friend -Wcast-align -Wno-comment" )
find_package( OpenMP )
if( OPENMP_FOUND )
   set( DEFAULT_CXX_FLAGS "${DEFAULT_CXX_FLAGS} ${OpenMP_CXX_FLAGS}" )
endif()

# environment dependent settings
if( APPLE )
   set( SO dylib )
   if( NOT arch )
      set( arch darwin )
   endif()
   if( CMAKE_SYSTEM MATHCES ^Darwin-10 )
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
# casacore
#
set( CASACORE_INCLUDE_DIR ${casaroot}/${arch}/include/casacore )

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
      list( APPEND _casacore_libs casa_${_a} )
   endif()
endforeach()
set( CASACORE_LIBRARIES ${_casacore_libs} )
message( STATUS "CASACORE_LIBRARIES = " ${CASACORE_LIBRARIES} )

#
# Python
#
# tentative
set( PYVERSION 2.6 )
if (NOT PYTHON_ROOT )
   set( PYTHON_ROOT ${casa_packages} )
endif()
find_package( PythonLibs REQUIRED python${PYVERSION} )

if( NOT PYTHONLIBS_FOUND )
   message( FATAL_ERROR "Python could not be found. Please check!" )
endif()
message( STATUS "PYTHON_INCLUDE_PATH = " ${PYTHON_INCLUDE_PATH} )
message( STATUS "PYTHON_LIBRARIES = " ${PYTHON_LIBRARIES} )

set( PYTHON_INCLUDE_DIR ${PYTHON_INCLUDE_PATH} )
find_path( NUMPY_INC_DIR numpy/npy_interrupt.h
           PATHS ${casa_packages}/lib/python${PYVERSION}/site-packages/numpy/core/include )
message( STATUS "NUMPY_INC_DIR = " ${NUMPY_INC_DIR} )
if( NOT NUMPY_INC_DIR STREQUAL NUMPY_INC_DIR-NOTFOUND ) 
   list( APPEND PYTHON_INCLUDE_DIR ${NUMPY_INC_DIR} )
endif()
message( STATUS "PYTHON_INCLUDE_DIR = " ${PYTHON_INCLUDE_DIR} )

#
# Boost
#
if( NOT BOOST_ROOT )
   set( BOOST_ROOT ${casa_packages} )
endif()

set( boost_components regex program_options filesystem )
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
endif() 

if( NOT Boost_VERSION GREATER 103199 AND NOT CASA_IGNORE_VERSION )
  message( FATAL_ERROR "Boost version (${Boost_VERSION}) is too old! Must be 103200 (1.32.0) or newer. Please check!" )
endif()



# include path
message( STATUS "Boost_INCLUDE_DIR = " ${Boost_INCLUDE_DIR} )
include_directories( ${CASACORE_INCLUDE_DIR} 
                     ${Boost_INCLUDE_DIR} 
                     ${PYTHON_INCLUDE_DIR} )

# link path
set( CASACORE_LIBRARY_DIR ${casaroot}/${arch}/lib )
link_directories( ${CASACORE_LIBRARY_DIR} )


# install directory
set( EXEC_INSTALL_DIR bin )
set( LIB_INSTALL_DIR lib )
set( PYTHON_INSTALL_DIR python/${PYVERSION}/asap )
set( SHARED_INSTALL_DIR share/asap )

# subdirectories
#  asap2to3       apps
#  libpyrap.so    external/libpyrap
#  libatnf.so     external-alma/atnf
#  _asap.so       src
#  python modules python
add_subdirectory( apps )
add_subdirectory( external/libpyrap )
add_subdirectory( external-alma/atnf )
add_subdirectory( src )
add_subdirectory( python )
add_subdirectory( share )


