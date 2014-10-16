###
# CMakeLists.txt for ASAP module
###

# minimum requirement for cmake version
cmake_minimum_required( VERSION 2.8 )

# options
option( USE_LIBCASACORE 
        "set ON to use libcasacore.so instead of libcasa_*.so"
        OFF )
option( STANDALONE 
        "set ON to build standalone mode"
        OFF )
option( PKSMS
        "set ON to diable to build PKSMS2reader/writer"
        OFF )
message( STATUS "USE_LIBCASACORE = " ${USE_LIBCASACORE} )
message( STATUS "STANDALONE = " ${STANDALONE} )
message( STATUS "PKSMS = " ${PKSMS} )

# Use casapy's fork of casacore
if ( NOT STANDALONE )
  add_definitions( -DUSE_CASAPY )
endif ()

# Define compiler paths on OSX 10.5. This must be done before invoking project()
if( APPLE )
    if( EXISTS            /opt/casa/core2-apple-darwin8/3rd-party/bin/gfortran )
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


#
# build type and their flags
#
# CMAKE_BUILD_TYPE (Release, Debug, RelWithDebInfo, MinSizeRel, Profile)
# default is RelWithDebInfo
#


if ( NOT CMAKE_BUILD_TYPE )
   set( CMAKE_BUILD_TYPE RelWithDebInfo )
endif()
message( STATUS "CMAKE_BUILD_TYPE = " ${CMAKE_BUILD_TYPE} )

# flags for Debug build
if ( CMAKE_BUILD_TYPE STREQUAL Debug )
   add_definitions( -DAIPS_DEBUG )
endif()
set( CMAKE_CXX_FLAGS_DEBUG "-ggdb3 -O0 -fno-omit-frame-pointer" )

# flags for Profile build
set( CMAKE_CXX_FLAGS_PROFILE "-g -O2 -pg -fprofile-arcs -fno-omit-frame-pointer" )

# flags for Release build
set( CMAKE_SHARED_LINKER_FLAGS_RELEASE "-Wl,-s" )
set( CMAKE_EXE_LINKER_FLAGS_RELEASE "-Wl,-s" )

# flags for MinSizeRel build
set( CMAKE_SHARED_LINKER_FLAGS_MINSIZEREL "-Wl,-s" )
set( CMAKE_EXE_LINKER_FLAGS_MINSIZEREL "-Wl,-s" )

if ( DEFINED CXX_FLAGS_TAIL_END )
   set( CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} ${CXX_FLAGS_TAIL_END}" )
   set( CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} ${CXX_FLAGS_TAIL_END}" )
   set( CMAKE_CXX_FLAGS_MINSIZEREL "${CMAKE_CXX_FLAGS_MINSIZEREL} ${CXX_FLAGS_TAIL_END}" )
   set( CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${CXX_FLAGS_TAIL_END}" )
   set( CMAKE_CXX_FLAGS_PROFILE "${CMAKE_CXX_FLAGS_PROFILE} ${CXX_FLAGS_TAIL_END}" )
endif()

# default flags
set( DEFAULT_CXX_FLAGS
     "-pipe -Wall -Wextra -Wno-non-template-friend -Wcast-align -Wno-comment" )

find_package( OpenMP )
if( OPENMP_FOUND )
   set( DEFAULT_CXX_FLAGS "${DEFAULT_CXX_FLAGS} ${OpenMP_CXX_FLAGS}" )
endif()

# environment dependent settings
message( STATUS "CMAKE_SYSTEM = " ${CMAKE_SYSTEM} )
message( STATUS "CMAKE_SYSTEM_PROCESSOR = " ${CMAKE_SYSTEM_PROCESSOR} )
if( APPLE )
   set( SO dylib )
   if( CMAKE_SYSTEM MATCHES ^Darwin-10 OR
		CMAKE_SYSTEM MATCHES ^Darwin-11 )
      if( NOT archflag )
         if( EXISTS /opt/casa/darwin11 )
            set( archflag x86_64 )
	 elseif( EXISTS /opt/casa/darwin10-64b )
            set( archflag x86_64 )
         elseif( EXISTS /opt/casa/core2-apple-darwin10 )
            set( archflag i386 )
         else()
            set( archflag x86_64 )
         endif()
      endif()
      if( archflag STREQUAL x86_64 )
         add_definitions( -DAIPS_64B )
      endif()
      execute_process( COMMAND ${CMAKE_CXX_COMPILER} --version
                       COMMAND head -1
                       COMMAND perl -pe "s|.*?(\\d+\\.\\d+)\\.\\d+$|$1|"
                       OUTPUT_VARIABLE _cxx_version
                       OUTPUT_STRIP_TRAILING_WHITESPACE )
      if( NOT _cxx_version STREQUAL "4.4" )
         set( DEFAULT_CXX_FLAGS "${DEFAULT_CXX_FLAGS} -arch ${archflag}" )
      endif()
   endif()
elseif( CMAKE_SYSTEM_NAME STREQUAL Linux )
   set( SO so )
   add_definitions( -DAIPS_LINUX )
   if( CMAKE_SYSTEM_PROCESSOR STREQUAL x86_64 )
      add_definitions( -DAIPS_64B )
      set( DEFAULT_CXX_FLAGS "${DEFAULT_CXX_FLAGS} -Wno-deprecated" )
   else()
      set( DEFAULT_CXX_FLAGS "${DEFAULT_CXX_FLAGS} -Wno-deprecated -Woverloaded-virtual" )
   endif()
endif()

# set flags for cpp compiler
set( CMAKE_CXX_FLAGS ${DEFAULT_CXX_FLAGS} )


#
# casacore definitions
#
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

#
# mode specific settings
#
# STANDALONE=ON   standalone build
# STANDALONE=OFF  build with casa
#
set( CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_SOURCE_DIR}/cmake" )
message( STATUS "CMAKE_MODULE_PATH = " ${CMAKE_MODULE_PATH} )
if( STANDALONE )
   include( standalone )
else()
   include( withcasa )
endif()

message( STATUS "CMAKE_INSTALL_PREFIX = " ${CMAKE_INSTALL_PREFIX} )


#
# casacore
#
unset( CASACORE_INCLUDE_DIR CACHE )
unset( CASACORE_LIBRARIES CACHE )
if( NOT USE_LIBCASACORE )
   # use casacore libraries 
   set( _includename casa/aipsdef.h )
   find_path( CASACORE_INCLUDE_DIR ${_includename}
              PATHS ${CASACORE_PATHS}
              PATH_SUFFIXES include include/casacore
              NO_DEFAULT_PATH )
   find_path( CASACORE_INCLUDE_DIR ${_includename} ) 
   if( CASACORE_INCLUDE_DIR MATCHES "NOTFOUND$" )
     message( FATAL_ERROR "${_includename} could not be found. Please check!" )
   endif()
   set( CASACORE_LIBS casa
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
                    PATHS ${CASACORE_PATHS}
                    PATH_SUFFIXES lib64 lib )
      if( _casacore_lib MATCHES "NOTFOUND$" )
         message( FATAL_ERROR "${_libname} could not be found. Please check!" )
      else()
         #list( APPEND _casacore_libs casa_${_a} )
         list( APPEND _casacore_libs ${_casacore_lib} )
      endif()
   endforeach()
   set( CASACORE_LIBRARIES ${_casacore_libs} )
else()
   # use libcasacore
   set( _libname libcasacore.${SO} )
   find_library( CASACORE_LIBRARIES ${_libname}
                 PATHS ${CASACORE_PATHS} 
                 PATH_SUFFIXES lib64 lib )
   if( CASACORE_LIBRARIES MATCHES "NOTFOUND$" )
      message( FATAL_ERROR "${_libname} could not be found. Please check!" )
   endif()
   set( _includename casa/aipsdef.h )
   string( REGEX REPLACE /[^/]+/[^/]+/?$ "" _coreroot ${CASACORE_LIBRARIES} )
   find_path( CASACORE_INCLUDE_DIR ${_includename}
              PATHS ${_coreroot}
              PATH_SUFFIXES include include/casacore
              NO_DEFAULT_PATH )
   find_path( CASACORE_INCLUDE_DIR ${_includename}
              PATHS ${CASACORE_PATHS}
              PATH_SUFFIXES include include/casacore
              NO_DEFAULT_PATH )
   find_path( CASACORE_INCLUDE_DIR ${_includename} )
   if( CASACORE_INCLUDE_DIR MATCHES "NOTFOUND$" )
     message( FATAL_ERROR "${_includename} could not be found. Please check!" )
   endif()
endif()
message( STATUS "CASACORE_LIBRARIES = " ${CASACORE_LIBRARIES} )
message( STATUS "CASACORE_INCLUDE_DIR = " ${CASACORE_INCLUDE_DIR} )
unset( USE_LIBCASACORE CACHE )


#
# Python
#
if( STANDALONE )
   include( FindPythonLibs )
   if( NOT PYTHONLIBS_FOUND )
      message( FATAL_ERROR "Python could not be found. Please check!" )
   endif()

   # Define pyroot
   string( REGEX REPLACE "/[^/]+/[^/]+/?$" "" pyroot ${PYTHON_INCLUDE_DIRS} )

   # find numpy
   string( REGEX MATCH [0-9].[0-9] PYTHONV ${PYTHON_INCLUDE_DIRS} )
   string( REPLACE "." "" PV ${PYTHONV} )
   find_path( NUMPY_INCLUDE_DIR numpy/npy_interrupt.h
              PATHS 
                 ${pyroot}/lib/python${PYTHONV}/site-packages/numpy/core
                 ${pyroot}/Library/Frameworks/Python.framework/Versions/${PYTHONV}
                 ${pyroot}/Library/Frameworks/Python.framework/Versions/${PYTHONV}/lib/python${PYTHONV}/site-packages/numpy/core
              PATH_SUFFIXES include )
   if( NUMPY_INCLUDE_DIR MATCHES "NOTFOUND$" )
      message( FATAL_ERROR "numpy/npy_interrupt.h could not be found. Please check!" )
   endif()
   list( APPEND PYTHON_INCLUDE_DIRS ${NUMPY_INCLUDE_DIR} )

   set( PYTHON_DEFINITIONS ${PYTHON_DEFINITIONS}
      -DPYTHONROOT=\"${pyroot}\"
      -DPYTHONVER=\"${PYTHONV}\"
      -DPYVERSION=${PV} )
else()
   # with CASA
   if( NOT PYTHON_FOUND )
      if ( NOT PYTHON_LIBNAME )
         #set( _names 2.9 2.8 2.7 2.6 2.5.2 2.5 )
         set( _names 2.7 2.6 2.5.2 2.5 )
	 # OSX 10.7 has Python2.7 by default. CASA has't yet supported python > 2.6 anyway.
         # (The library named libpython.2.5.2.dylib seems to exist only in the CASA world.)
      else()
         set( _names ${PYTHON_LIBNAME} )
      endif()

      set( _found False )
      foreach( _v ${_names} )

         casa_find(
            PYTHON${_v}
	    #PREFIX_HINTS ${PYTHON_ROOT_DIR}
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
   # For some unknown reason cmake sets the /usr for Lion
   #
   if( APPLE )
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

   set( PYTHON_DEFINITIONS ${PYTHON_DEFINITIONS}
      -DAIPSROOT=\"${CMAKE_SOURCE_DIR}\"
      -DAIPSARCH=\"${arch}\"
      -DAIPSSITE=\"garching\"
      -DPYTHONROOT=\"${pyroot}\"
      -DPYTHONVER=\"${PYTHONV}\"
      -DPYVERSION=${PV} )
endif()
message( STATUS "PYTHON_INCLUDE_DIRS = " ${PYTHON_INCLUDE_DIRS} )
message( STATUS "PYTHON_LINRARIES = " ${PYTHON_LIBRARIES} )
message( STATUS "PYTHONV = " ${PYTHONV} )


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
if ( BLAS_LIBRARIES MATCHES "NOTFOUND$" )
   message( FATAL_ERROR "blas could not be found. Please check!" )
endif()
message( STATUS "BLAS_LIBRARIES = " ${BLAS_LIBRARIES} )


#
# LAPACK
#
find_library( LAPACK_LIBRARIES liblapack.${SO} )
if ( LAPACK_LIBRARIES MATCHES "NOTFOUND$" )
   message( FATAL_ERROR "lapack could not be found. Please check!" )
endif()
message( STATUS "LAPACK_LIBRARIES = " ${LAPACK_LIBRARIES} )


#
# Boost
#
set( boost_components python )
if( NOT STANDALONE )
  set( boost_components ${boost_components} system )
endif()
find_package( Boost REQUIRED ${boost_components} )
if( NOT Boost_FOUND )
  message( FATAL_ERROR "Boost could not be found. Please check!" )
endif()
message( STATUS "BOOST_INCLUDE_DIR = " ${Boost_INCLUDE_DIR} )
message( STATUS "BOOST_LIBRARIES = " ${Boost_LIBRARIES} )


#
# cfitsio
#
if( STANDALONE )
   find_path( CFITSIO_INCLUDE_DIRS fitsio.h
              PATH_SUFFIXES cfitsio )
   if( CFITSIO_INCLUDE_DIRS MATCHES "NOTFOUND$" )
      message( FATAL_ERROR "fitsio.h could not be found. Please check!" )
   endif()
   string( REGEX REPLACE "/[^/]+/?$" "" CFITSIO_ROOT ${CFITSIO_INCLUDE_DIRS} )
   message( STATUS "CFITSIO_ROOT=" ${CFITSIO_ROOT} )
   find_library( CFITSIO_LIBRARIES libcfitsio.${SO}
                 PATHS ${CFITSIO_ROOT}
                 PATH_SUFFIXES lib64 lib
                 NO_DEFAULT_PATH )
   if( CFITSIO_LIBRARIES MATCHES "NOTFOUND$" )
      find_library( CFITSIO_LIBRARIES libcfitsio.a
                    PATHS ${CFITSIO_ROOT}
                    PATH_SUFFIXES lib64 lib
                    NO_DEFAULT_PATH )
   endif()
   find_library( CFITSIO_LIBRARIES libcfitsio.${SO}
                 PATHS /usr/local /usr
                 PATH_SUFFIXES lib64 lib )
   if( CFITSIO_LIBRARIES MATCHES "NOTFOUND$" )
      message( FATAL_ERROR "libcfitsio.so could not be found. Please check!" )
   endif()
else()
   casa_find( CFITSIO
              VERSION 3.006
              INCLUDES fitsio.h fitsio2.h
              INCLUDES_SUFFIXES cfitsio
              LIBS cfitsio
              CPP_VERSION CFITSIO_VERSION
              RUN_VERSION "(ffvers(&v), v)" )
endif()
message( STATUS "CFITSIO_INCLUDE_DIRS = " ${CFITSIO_INCLUDE_DIRS} )
message( STATUS "CFITSIO_LIBRARIES = " ${CFITSIO_LIBRARIES} )


#
# Fortran
#
if( NOT FORTRAN_LIBRARIES )

  message( STATUS "Looking for Fortran runtime libraries" )
  set( _try  ${CMAKE_BINARY_DIR}/try_fortran.cc )
  file( WRITE ${_try}
    "int main() { return 0; }\n"
    )

  execute_process( COMMAND ${CMAKE_Fortran_COMPILER} -print-search-dirs
                   COMMAND perl -e "$found = 0; while(<>) {  s|\n||g; if( m|^libraries:\\s*=(.*?$)| ) { foreach $d (split(':',$1) ) { if ( -e \"$d/libgfortran${CMAKE_SHARED_LIBRARY_SUFFIX}\" ) { print \"$d/libgfortran${CMAKE_SHARED_LIBRARY_SUFFIX}\"; $found = 1; break; }} break if $found; } break if $found;}"
                   OUTPUT_VARIABLE _gfortran_lib_path
                   OUTPUT_STRIP_TRAILING_WHITESPACE )

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

# X11.
if( NOT APPLE )
  find_package( X11 REQUIRED )
  # Rename
  set( X11_INCLUDE_DIRS ${X11_INCLUDE_DIR} )
endif()

# PGPLOT (FORTRAN plotting package).
# depends on X11
# not require in standalone mode
if( NOT STANDALONE )
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

  add_definitions( -DENABLE_PLOTTER2 )
endif()

#
# RPFITS
#
if ( STANDALONE )
   find_path( RPFITS_INCLUDE_DIRS RPFITS.h
              PATHS /usr/local /usr
              PATH_SUFFIXES include )
   if( RPFITS_INCLUDE_DIRS MATCHES "NOTFOUND$" )
      message( FATAL_ERROR "RPFITS.h could not be found. Please check!" )
   endif()
   find_library( RPFITS_LIBRARIES librpfits.so
                 PATHS /usr/local /usr
                 PATH_SUFFIXES lib64 lib )
   if( RPFITS_LIBRARIES MATCHES "NOTFOUND$" )
      message( FATAL_ERROR "librpfits.so could not be found. Please check!" )
   endif()
   list( APPEND RPFITS_LIBRARIES ${FORTRAN_LIBRARIES} )
else()
   casa_find( RPFITS
              VERSION 2.11
              INCLUDES RPFITS.h
              LIBS rpfits
              RUN_VERSION names_.rpfitsversion
              DEPENDS FORTRAN )
endif()
message( STATUS "RPFITS_INCLUDE_DIRS = " ${RPFITS_INCLUDE_DIRS} )
message( STATUS "RPFITS_LIBRARIES = " ${RPFITS_LIBRARIES} )


#
# wcslib
#
set( _wcslib libwcs.${SO} )
set( _wcs_version 4.3 )
find_library( WCSLIB ${_wcslib} 
              PATHS ${WCSLIB_PATHS} 
              PATH_SUFFIXES lib64 lib )
if( WCSLIB MATCHES "NOTFOUND$" )
   message( STATUS "${_wcslib} could not be found." )
   unset( _wcslib CACHE )
   unset( WCSLIB CACHE )
   set( _wcslib libwcs.${_wcs_version}.${SO} )
   message( STATUS "Try to find ${_wcslib}..." )
   find_library( WCSLIB ${_wcslib}
                 PATHS ${WCSLIB_PATHS}
                 PATH_SUFFIXES lib64 lib )
   if( WCSLIB MATCHES "NOTFOUND$" )
      message( FATAL_ERROR "${_wcslib} could not be found. Please check!" )
   endif()
endif()
message( STATUS "WCSLIB = " ${WCSLIB} )
find_path( WCSLIB_INCLUDE_DIR wcslib/wcs.h
           PATHS ${WCSLIB_PATHS}
           PATH_SUFFIXES include )
if( WCSLIB_INCLUDE_DIR MATCHES "NOTFOUND$" )
   message( FATAL_ERROR "wcs.h could not be found. Please check!" )
endif()
message( STATUS "WCSLIB_INCLUDE_DIR = " ${WCSLIB_INCLUDE_DIR} )


#
# common include path
#
include_directories( ${CASACORE_INCLUDE_DIR}
                     ${Boost_INCLUDE_DIR}
                     ${X11_INCLUDE_DIRS}
                     ${PGPLOT_INCLUDE_DIRS}
                     ${PYTHON_INCLUDE_DIRS}
                     ${WCSLIB_INCLUDE_DIR}
                     ${CFITSIO_INCLUDE_DIRS}
                     ${RPFITS_INCLUDE_DIRS} )


#
# install directory
#
set( EXEC_INSTALL_DIR bin )
set( LIB_INSTALL_DIR lib )
set( PYTHON_INSTALL_DIR python/${PYTHONV}/asap )
set( SHARED_INSTALL_DIR share/asap )

#
# execute getsvnrev.sh to create/update svninfo.txt
# run chmod to make sure the script has executable 
#
exec_program( chmod ARGS a+x ${ASAP_SOURCE_DIR}/getsvnrev.sh )
exec_program( chmod ARGS a+x ${ASAP_SOURCE_DIR}/cmake/exec_getsvnrev.sh )
exec_program( ${ASAP_SOURCE_DIR}/cmake/exec_getsvnrev.sh ARGS ${ASAP_SOURCE_DIR})

# Add the modules to be built.
#
# Choose to have correct RPATHs both in the build tree and
# in the install tree (at the cost of having to change the
# rpath when installing)
#
set( CMAKE_INSTALL_RPATH ${CMAKE_INSTALL_PREFIX}/lib )
set( CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
set( CMAKE_INSTALL_NAME_DIR ${CMAKE_INSTALL_PREFIX}/lib )

#
# PKSMS
#
if ( NOT PKSMS )
   add_definitions( -DNOPKSMS )
endif()

#
# libraries and executables
#
set( ASAPPYLIB _asap )
set( ASAPLIB asap )
set( PYRAPLIB pyrap )
set( ATNFLIB atnf )
set( ASAP2TO3 asap2to3 )
if ( NOT STANDALONE )
#   set( ASDM2ASAP_OLD oldasdm2ASAP )
   set( ASDM2ASAP asdm2ASAP )
endif()

#
# always install by default
#
add_custom_target( inst ALL ${CMAKE_BUILD_TOOL} install/fast )

#
# subdirectories
#
asap_add_subdirectory()
