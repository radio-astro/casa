# CasaDistributionFinder.cmake
#
# Sets the variable casa_packages to point to the appropriate
# directory where casa-distributed packages are located.
#
# Apple
#     Darwin 9
#         All    --> /opt/casa/core2-apple-darwin8/3rd-party
#     Darwin 10
#         32-bit --> /opt/casa/core2-apple-darwin10
#         64-bit --> /opt/casa/darwin10-64b
#     Darwin 11
#         32-bit --> *Not Supported*
#         64-bit --> /opt/casa/darwin11
# Linux
#     32-bit --> /usr/lib/casapy
#     64-bit --> /usr/lib64/casapy

if (CASA_THREAD_NEUTRAL)
   if (APPLE)

    # I blieve this could be avoided by defining CMAKE_FIND_ROOT_PATH, wky

    # Determine location of packages on various Apple
    # OS-version/processor combinations.

       if (CMAKE_SYSTEM MATCHES ^Darwin-9)
           set (casa_packages /opt/casa/core2-apple-darwin8/3rd-party)
       elseif (CMAKE_SYSTEM MATCHES ^Darwin-10)
               set  (casa_packages /opt/casa/darwin10-64b) # Darwin 10 64-bit
       elseif  (CMAKE_SYSTEM MATCHES ^Darwin-11)
	       set  (casa_packages /opt/casa/darwin11) # Darwin 11 64-bit
       elseif  (CMAKE_SYSTEM MATCHES ^Darwin-12)
	       set  (casa_packages /opt/casa/darwin12) # Darwin 11 64-bit
       else()
           message (WARNING "Unsupported Mac: ${CMAKE_SYSTEM}")
       endif()

   elseif (CMAKE_SYSTEM_NAME STREQUAL Linux)
 
       # Determine location of packages on Linux

       if (CMAKE_SYSTEM_PROCESSOR STREQUAL x86_64)
           set (casa_packages /usr/lib64/casapy) # Linux 64-bit
       else()
           set (casa_packages /usr/lib/casapy) # Linux 32-bit
       endif()

   else()
         message (WARNING "Unsupported system: ${CMAKE_SYSTEM_NAME}")
   endif()
   
# CASA_THREAD_NEUTRAL
#
# Sets up the build configuration to support CASA thread neutrality
# by finding the appropriate boost headers and libraries. There are
# two steps: 1) If the user has not provided the location to start 
# looking for boost (via BOOST_ROOT) then the location of the CASA
# distributed packages is determined and made to be the first place
# searched by the find_package module

# Determine the location of CASA-distributed packages if a primary 
# directory was not provided.


# if (NOT BOOST_ROOT)
#
#   # Normally this file will determine where to start looking for the boost
#   # packages however, the user can override that via the BOOST_ROOT variable.
#   # N.B.: In theory the user could also specify it using the shell environment but
#   # by if a value is set before calling find_package, the environmental
#   # variable is ignored by the FindBoost module.
#
#   if (APPLE)
#
#     if (CASA_THREAD_NEUTRAL)
#
#       # The Apple logic for determining the location is available in code's
#       # CMakeLists.txt, but there are lots of other quirky things in that which
#       # ought to be considered before dropping it in here.  So for now just
#       # issue a warning and ignore this feature on the Apple.
#
#       message( WARNING "CASA_THREAD_NEUTRAL not yet supported on Apple hosts." )
#       set (CASA_THREAD_NEUTRAL NO)
#
#     endif (CASA_THREAD_NEUTRAL)
#
#   elseif( CMAKE_SYSTEM_NAME STREQUAL Linux )
#
#     if( CMAKE_SYSTEM_PROCESSOR STREQUAL x86_64 )
#       set( casa_packages /usr/lib64/casapy )
#     else()
#       set( casa_packages /usr/lib/casapy )
#
#   else()
#     message( WARNING "Unsupported system: ${CMAKE_SYSTEM_NAME}" )
#   endif()

# endif (NOT BOOST_ROOT)

   if (NOT BOOST_ROOT)
      set( BOOST_ROOT ${casa_packages}/include)
   endif(NOT BOOST_ROOT)
   set( boost_components thread serialization)
   set( min_boost_version 1.41.0)
   find_package( Boost ${min_boost_version} REQUIRED ${boost_components} )
   if (NOT Boost_FOUND)
      message( FATAL_ERROR "Boost with version >= ${min_boost_version} could not be found. Please check!" )
   endif (NOT Boost_FOUND)
   # include_directories{$(Boost_INCLUDE_DIR})
   set (CASA_THREAD_NEUTRALITY "Yes") # for reporting only
   # Set up these to variables to be used to flesh out a header file template later in
   # this file.
   set (CASA_THREAD_NEUTRAL_DEFINE "#define CASA_THREAD_NEUTRAL")
   set (USE_BOOST_SHARED_PTR_DEFINE "#define USE_BOOST_SHARED_PTR")
else (CASA_THREAD_NEUTRAL)
   set (CASA_THREAD_NEUTRALITY "No") # for reporting only
endif (CASA_THREAD_NEUTRAL)

# Now configure the include file so that it records the thread neutrality-related settings for use
# by outside code using casacore.  When thread neutrality is not enabled the file will
# end up having no defines it it.  However, if it is not present then the compilations will fail.

configure_file (casa/Utilities/CasaThreadNeutrality.h.in ${CMAKE_BINARY_DIR}/casa/Utilities/CasaThreadNeutrality.h)


