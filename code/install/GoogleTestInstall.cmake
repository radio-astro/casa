# This script is intended to be run as part of the cmake for the code project.
# It will download, build and install the google test library for use in
# test written using that framework.
#
# The source directory for Google Test will be code/gtest.  It will contain
# a single subdirectory (googletest-release-1.7.0 at this writing) which is
# the is the root of the exploded tar file.  The source directory will be
# added as a subdirectory of the project and thus contain a CMakeLists.txt 
# file.  This is created from the template file GoogleTestInstall.in.
# The header files needed for unit test compilation will remain in 
# the include folder of the release (i.e., they are not installed as other
# CASA include files are).
#
# As configured CMakeLists.txt file will create a corresponding binary directory
# (e.g., code/build/gtest).  The static libraray, libgtest.a, will be located
# there for use in building unit tests.  The library is actually built as part
# of the "make" process and the compilation should be using all the same settings
# as the other C++ files in code.
#
# The google test configuration will only be performed if the code/gtest directory
# is not present; thus to get a clean redo, that directory must first be removed.
#
# This framework was constructed 8 October 2015 when the current version of
# google test was 1.7.0 and recently relocated onto github.  The archive is
# found in the "release" section of the google/googletest repo.
#
# (jjacobs)
#
###############################################################################

# Only the GtestVersion line below should require changing when upgrading
# to a later gtest release
set (GoogleTest_Version "1.7.0")

set (CasaCodeSourceRoot ${CMAKE_SOURCE_DIR})
set (GoogleTest_Root ${CMAKE_BINARY_DIR}/gtest)

set (GoogleTest_ReleaseArchive 
     https://svn.cv.nrao.edu/casa/devel/googletest/googletest-release-${GoogleTest_Version}.tar.gz)

######################
#                    #
# Exported variables #
#                    #
######################

set (GoogleTest_ArchiveFile "googletest-release-${GoogleTest_Version}.tar.gz")
set (GoogleTest_ReleaseRoot ${GoogleTest_Root}/googletest-release-${GoogleTest_Version} 
     CACHE STRING "Root of Google Test source")
set (GoogleTest_LibraryDir ${CMAKE_BINARY_DIR}/gtest CACHE STRING "Location of libgtest.a")
set (GoogleTest_Target "libgtest" CACHE STRING "Target building Google Test library")

message ("-- Google Test installation commencing ...")

if (NOT EXISTS ${GoogleTest_Root})
   execute_process (COMMAND mkdir ${GoogleTest_Root}
                    RESULT_VARIABLE status)

   if (NOT ${status} EQUAL 0)
       message (SEND_ERROR "*** Failed to create Google Test directory: ${GoogleTest_Root}")
   endif ()
endif ()

if (NOT EXISTS ${GoogleTest_Root}/${GoogleTest_ArchiveFile}) # download archive

   message ("-- File ${GoogleTest_Root}/${GoogleTest_ArchiveFile} not present; downloading")

   execute_process (COMMAND curl -L -O ${GoogleTest_ReleaseArchive}
                    WORKING_DIRECTORY ${GoogleTest_Root}
		    RESULT_VARIABLE status)
   if (NOT ${status} EQUAL 0)
       message (SEND_ERROR "*** Failed to download Google Test from ${GoogleTest_ReleaseArchive}.")
   endif ()

   message ("-- ... Downloaded source")

else ()
   message ("-- ... Using existing source")
endif ()

# Explode the archive
if( APPLE )
    execute_process (COMMAND tar xf ${GoogleTest_ArchiveFile}
	  	 WORKING_DIRECTORY ${GoogleTest_Root}
		 RESULT_VARIABLE status)
else ()
    execute_process (COMMAND tar xf ${GoogleTest_ArchiveFile} --overwrite
		 WORKING_DIRECTORY ${GoogleTest_Root}
		 RESULT_VARIABLE status)
endif ()

if (NOT ${status} EQUAL 0)
   message (SEND_ERROR "*** Failed to explode Google Test tar file.")
endif ()

# Create a root directory for GoogleTest and configure a CMakeLists.txt
# file in that directory.

message ("-- ... Configuring directory ")

configure_file (install/GoogleTestInstall.in ${GoogleTest_Root}/CMakeLists.txt @ONLY)

# Wire the Google Test subproject in by adding it as a subdirector and
# creating dependencies

add_subdirectory (${GoogleTest_Root} ${GoogleTest_Root})
message ("DEBUG GoogleTest_Root=${GoogleTest_Root}")

message ("-- ... GoogleTest installation complete")



 



