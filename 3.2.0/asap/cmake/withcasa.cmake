###
# CMakeLists.txt for build with casa
###


# environment dependent settings
if( APPLE )
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
         set( casa_packages /opt/casa/darwin10-64b )
      else()
         set( casa_packages /opt/casa/core2-apple-darwin10 )
      endif()
   elseif( CMAKE_SYSTEM MATCHES ^Darwin-9 )
      set( casa_packages /opt/casa/core2-apple-darwin8/3rd-party )
   endif()         
elseif( CMAKE_SYSTEM_NAME STREQUAL Linux )
   if( CMAKE_SYSTEM_PROCESSOR STREQUAL x86_64 )
      set( casa_packages /usr/lib64/casapy )
      if( NOT arch )
         set( arch linux_64b )
      endif()
   else()
      set( casa_packages /usr/lib/casapy )
      if( NOT arch )
         set( arch linux_gnu )
      endif()
   endif()
endif()
message( STATUS "arch = " ${arch} )

# install directory
#
# The layout of the source+install directory trees
# is rather hard-coded in much source code. However,
# with care CASA can be built and installed elsewhere...
#
IF(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    # the regular expression means '../'
    #  [^ ] Matches any character(s) not inside the brackets
    #  +    Matches preceding pattern one or more times
    #  ?    Matches preceding pattern zero or once only
    #  $    Mathces at end of a line
    string( REGEX REPLACE /[^/]+/?$ "" casaroot ${CMAKE_SOURCE_DIR} )
    set( CMAKE_INSTALL_PREFIX ${casaroot}/${arch} CACHE PATH "casa architecture directory" FORCE )
ELSE()
    set( casaroot ${CMAKE_INSTALL_PREFIX}/.. CACHE PATH "casa architecture directory" FORCE )
ENDIF(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)

message( STATUS "casaroot = " ${casaroot} )

# modules
IF ( EXISTS ${casaroot}/code/install )
    set( CMAKE_MODULE_PATH ${casaroot}/code/install )
ELSE()
    set( CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/../code/install )
ENDIF()


include( config )
include( CASA )

#
# casacore
#
set( CASACORE_PATHS "${casaroot}/${arch};${casaroot};/usr/local;/usr" )


#
# Boost
#
if( NOT BOOST_ROOT )
   set( BOOST_ROOT ${casa_packages} )
endif()


#
# wcslib
#
set( WCSLIB_PATHS "${casaroot}/${arch};/usr/local;/usr" )


#
# subdirectories
#  ASAP2TO3 asap2to3       apps
#  PYRAPLIB libpyrap.so    external/libpyrap
#  ATNFLIB  libatnf.so     external-alma/atnf
#  ASAPLIB  _asap.so       src
#  python modules          python
#  shared files            share
#
macro( asap_add_subdirectory )
   add_subdirectory( apps )
   add_subdirectory( external/libpyrap )
   add_subdirectory( external-alma/atnf )
   add_subdirectory( src )
   add_subdirectory( python )
   add_subdirectory( share )
endmacro( asap_add_subdirectory )

