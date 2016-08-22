###
# CMakeLists.txt for standalone build
###


#
# casacore
#
# always use libcasacore.so
set( USE_LIBCASACORE ON )
set( CASACORE_PATHS "/usr/local;/usr" )


#
# wcslib
#
set( WCSLIB_PATHS "/usr/local;/usr" )


#
# subdirectories
#  ASAP2TO3 asap2to3       apps
#  PYRAPLIB libpyrap.so    external/libpyrap
#  ATNFLIB  libatnf.so     external/atnf
#  ASAPLIB  _asap.so       src
#  python modules          python
#  shared files            share
#  executables             bin
#
macro( asap_add_subdirectory )
   add_subdirectory( apps )
#   add_subdirectory( external/libpyrap )
   add_subdirectory( external/atnf )
   add_subdirectory( src )
   add_subdirectory( python )
   add_subdirectory( share )
   add_subdirectory( bin )
   add_subdirectory( external-alma/components )
endmacro( asap_add_subdirectory )
