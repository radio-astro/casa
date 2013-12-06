###
# CMakeLists.txt for build with casa
###


# environment dependent settings
if( APPLE )
   if( CMAKE_SYSTEM MATCHES ^Darwin-10 OR
		CMAKE_SYSTEM MATCHES ^Darwin-11 )
      if ( NOT arch )
         set( arch darwin64 )
      endif()
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
      	 if( CMAKE_SYSTEM MATCHES ^Darwin-11 )
             set( casa_packages /opt/casa/darwin11 )
	 else()
	     set( casa_packages /opt/casa/darwin10-64b )
	 endif()
      else()
         set( casa_packages /opt/casa/core2-apple-darwin10 )
      endif()
   elseif( CMAKE_SYSTEM MATCHES ^Darwin-9 )
      if ( NOT arch )
         set( arch darwin )
      endif()
      set( casa_packages /opt/casa/core2-apple-darwin8/3rd-party )
   endif()         
elseif( CMAKE_SYSTEM_NAME STREQUAL Linux )
   if( CMAKE_SYSTEM_PROCESSOR STREQUAL x86_64 )
      if ( EXISTS /usr/lib64/casa/01 )
         set( casa_packages /usr/lib64/casa/01 )
      else( )
          set( casa_packages /usr/lib64/casapy )
      endif( )
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
IF ( NOT DEFINED CASA_CODE_PATH )
    IF ( EXISTS ${casaroot}/code/install )
        set( CASA_CODE_PATH ${casaroot}/code )
    ELSE()
        set( CASA_CODE_PATH ${CMAKE_SOURCE_DIR}/../code )
    ENDIF()
ENDIF()
message( STATUS "CASA_CODE_PATH = " ${CASA_CODE_PATH} )
set( CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CASA_CODE_PATH}/install" )
message( STATUS "CMAKE_MODULE_PATH = " ${CMAKE_MODULE_PATH} )

include( config )
include( CASA )

#
# casacore
#
set( CASACORE_PATHS "${casaroot}/${arch};${casaroot};/usr/local;/usr" )

set( epochdelta 1385403204 )
if ( EXISTS ${CMAKE_INSTALL_PREFIX}/${arch}/casa_sover.txt )
    execute_process( COMMAND perl -e "while (<>) { chomp and print if (! m/^\#/ ) }" ${CMAKE_INSTALL_PREFIX}/${arch}/casa_sover.txt
                     OUTPUT_VARIABLE __asap_soversion )
elseif( EXISTS ${CMAKE_INSTALL_PREFIX}/casa_sover.txt )
    execute_process( COMMAND perl -e "while (<>) { chomp and print if (! m/^#/ ) }" ${CMAKE_INSTALL_PREFIX}/casa_sover.txt
                     OUTPUT_VARIABLE __asap_soversion )
else( )
    execute_process( COMMAND perl -e "$t=time( )-${epochdelta};$z=$t & 0xff; $y=($t>>8)&0xff; $x=($t>>16)&0xffff; print \"$x.$y.$z\""
                     OUTPUT_VARIABLE __asap_soversion )
endif( )
set(asap_soversion ${__asap_soversion} CACHE STRING "version for shared objects")
message( STATUS "shared object version number ${asap_soversion}" )

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
# CASA (only alma/ASDM)
#
find_path( LIBXML2_INCLUDE_DIR libxml/xmlversion.h 
           PATH_SUFFIXES libxml2 )
if( LIBXML2_INCLUDE_DIR MATCHES "NOTFOUND$" )
   message( FATAL_ERROR "libxml/xmlversion.h could not be found. Please check!" )
endif()
message( STATUS "LIBXML2_INCLUDE_DIR = " ${LIBXML2_INCLUDE_DIR} )
find_path( LIBXML2_LIBRARY libxml2${CMAKE_SHARED_LIBRARY_SUFFIX}
           PATHS /usr
           PATH_SUFFIXES lib64 lib )
#find_path( LIBXML2_LIBRARY libxml2.so )
if ( LIBXML2_LIBRARY MATCHES "NOTFOUND$" )
   message( FATAL_ERROR "libxml2${CMAKE_SHARED_LIBRARY_SUFFIX} could not be found. Please check!" )
endif()
message( STATUS "LIBXML2_LIBRARY = " ${LIBXML2_LIBRARY} ) 

set( ASDM_INCLUDE_ROOT ${CASA_CODE_PATH} )
if ( EXISTS ${CASA_CODE_PATH}/include/alma )
   set( ASDM_INCLUDE_ROOT ${CASA_CODE_PATH}/include )
endif()
#set( ASDM_INCLUDE_DIR_OLD ${ASDM_INCLUDE_ROOT}/alma/ASDM
#                          ${ASDM_INCLUDE_ROOT}/alma/Enumerations
#                          ${ASDM_INCLUDE_ROOT}/alma/ASDMBinaries
#                          ${ASDM_INCLUDE_ROOT}/alma/Enumtcl
#                          ${LIBXML2_INCLUDE_DIR} )
#set( ASDM_LIBRARY_OLD ${casaroot}/${arch}/lib/libalma${CMAKE_SHARED_LIBRARY_SUFFIX} 
#                      ${LIBXML2_LIBRARY}/libxml2${CMAKE_SHARED_LIBRARY_SUFFIX} )
#set( ASDM_INCLUDE_DIR ${ASDM_INCLUDE_ROOT}/alma_v3/ASDM
#                      ${ASDM_INCLUDE_ROOT}/alma_v3/Enumerations
#                      ${ASDM_INCLUDE_ROOT}/alma_v3/ASDMBinaries
#                      ${ASDM_INCLUDE_ROOT}/alma_v3/Enumtcl
#                      ${LIBXML2_INCLUDE_DIR} )
#set( ASDM_LIBRARY ${casaroot}/${arch}/lib/libalma_v3${CMAKE_SHARED_LIBRARY_SUFFIX}
#                  ${LIBXML2_LIBRARY}/libxml2${CMAKE_SHARED_LIBRARY_SUFFIX} )
set( ASDM_INCLUDE_DIR ${ASDM_INCLUDE_ROOT}/alma/ASDM
                      ${ASDM_INCLUDE_ROOT}/alma/Enumerations
                      ${ASDM_INCLUDE_ROOT}/alma/ASDMBinaries
                      ${ASDM_INCLUDE_ROOT}/alma/Enumtcl
                      ${LIBXML2_INCLUDE_DIR} )
if ( NOT ALMA_LIB_PATH )
   set( ASDM_LIBRARY ${casaroot}/${arch}/lib/libalma${CMAKE_SHARED_LIBRARY_SUFFIX}
                     ${LIBXML2_LIBRARY}/libxml2${CMAKE_SHARED_LIBRARY_SUFFIX} )
else( )
   set( ASDM_LIBRARY ${ALMA_LIB_PATH}
                     ${LIBXML2_LIBRARY}/libxml2${CMAKE_SHARED_LIBRARY_SUFFIX} )
endif( )
message( STATUS "ASDM_LIBRARY = " ${ASDM_LIBRARY} ) 
add_definitions( -DWITHOUT_ACS )

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
   add_subdirectory( external-alma/asdm2ASAP )
#   add_subdirectory( external-alma/oldasdm2ASAP ) 
endmacro( asap_add_subdirectory )

