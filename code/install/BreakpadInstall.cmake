# This script is intended to be run as part of the cmake for the code project.

# This script downloads, configures and builds the breakpad package.  CASA will use
# breakpad to capture crashes of the various CASA applications.  This file will put
# breakpad into a directory named "breakpad" that is directly below the build directory.

# The CASA application needs the breakpad header files and libbreakpad_client.a to
# build in the crash reporter functionality.  Part of this file defines two
# variables, Breakpad_IncludeRoot which is to be used in a "-I" option during
# compilation of breakpad using code, and Breakpad_Libray which will be the
# path to the created libbreakpad_client.a.

# The source code will come from the breakpad site (part of Google's chromium project)
# and then require some manual work (see below) to create zipped tar file.  This
# tar file will reside on a CASA server and be downloaded into the user's build tree
# during the cmake process.  After download, the archive will be exploded, the 
# package build configured, made and the library of interest copied.  These operations
# will only occur when the expected artifacts are not present.
#
# (jjacobs)
#
###############################################################################

##############################################################################
#
# How to create the breakpad archive file to put onto the CASA server
#
# 1) Download the depot tools package
#
# --> git clone https://chromium.googlesource.com/chromium/tools/depot_tools.git
#
# 2) Now download breakpad using a depot_tools script
#
# Apparently using "git" to clone the breakpad repo does not work correctly because
# some of the needed files are contained in unrelated git repos.  Some of the depot
# tools handle this problem.  If you try to use git directly, during the build step
# if will fail with some missing header files (paths will contain "/third_party/").
#
# --> PATH=../depot_tools:$PATH fetch --nohooks breakpad
#
# 3) Now change the name of the top-level breakpad directory (it will contain 
#    "configure") to "breakpad-distro".  Probably --> mv breakpad breakpad-distro
#  
# 4) Tar it up leaving out the .git tree
#
# --> tar czf breakpad-from-google-yymmdd breakpad-distro --exclude=.git
#
#     # yymmdd is the date of this operation (e.g., 160531)
#     # be sure to change the name timestamp in the lower part of this 
#     # CMakeLists.txt file.


macro (configure_breakpad Breakpad_Root)

    # Derived from BreakpadInstall.in

    message ("DEBUG BreakpadInstall-->CMakeLists.txt executing")

    # Define a variable so that Breakpad clients can establish a
    # dependency on this library.

    # Build Breakpad now

    message ("-- .. ")
    message ("-- .. +----------------------+")
    message ("-- .. | Configuring breakpad |")
    message ("-- .. +----------------------+")
    message ("-- .. ")

    # Perform the configuration step by executing "configure"

    execute_process (COMMAND "./configure"
	WORKING_DIRECTORY "breakpad/breakpad-distro"
	RESULT_VARIABLE status)
    if (NOT ${status} EQUAL 0)
	message (SEND_ERROR "*** Failed to configure breakpad: ${Breakpad_Root}")
    endif ()

    # Now make it

    message ("-- .. ")
    message ("-- .. +-------------------+")
    message ("-- .. | Building breakpad |")
    message ("-- .. +-------------------+")
    message ("-- .. ")

    execute_process (COMMAND "make"
	WORKING_DIRECTORY "breakpad/breakpad-distro"
	RESULT_VARIABLE status)
    if (NOT ${status} EQUAL 0)
	message (SEND_ERROR "*** Failed to make breakpad: ${Breakpad_Root}")
    endif ()

    # Copy the client library up to the breakpad root directory

    if (APPLE)
	set (OS "mac")
    else ()
	set (OS "linux")
    endif ()

    execute_process (COMMAND cp ./breakpad-distro/src/client/${OS}/libbreakpad_client.a .
	WORKING_DIRECTORY "breakpad"
	OUTPUT_VARIABLE error_message
	ERROR_VARIABLE error_message
	RESULT_VARIABLE status)
    if (NOT ${status} EQUAL 0)
	message (SEND_ERROR "*** Failed to copy: ${command}: ${error_message}")
    endif ()

endmacro (configure_breakpad)

macro (install_breakpad Breakpad_Root Breakpad_ArchiveFile Breakpad_ArchiveUrl)

    message ("-- Breakpad installation commencing ...")

    set (rebuild FALSE)

    # Create the breakpad directory under the root of the build tree

    if (NOT EXISTS ${Breakpad_Root})
	message ("-- ... Creating Breakpad source directory")
	execute_process (COMMAND mkdir ${Breakpad_Root}
	    RESULT_VARIABLE status)

	if (NOT ${status} EQUAL 0)
	    message (SEND_ERROR "*** Failed to create Breakpad directory: ${Breakpad_Root}")
	endif ()
    endif ()

    # If the required archive file is not present then download it.

    if (NOT EXISTS ${Breakpad_Root}/${Breakpad_ArchiveFile}) # download archive

	message ("-- ... Source archive file ${Breakpad_Root}/${Breakpad_ArchiveFile} not present; downloading")

	execute_process (COMMAND curl -L -O ${Breakpad_ArchiveUrl}
            WORKING_DIRECTORY ${Breakpad_Root}
            RESULT_VARIABLE status)
	if (NOT ${status} EQUAL 0)
	    message (SEND_ERROR "*** Failed to download Breakpad source archive from ${Breakpad_ArchiveUrl}.")
	else ()

	    # New archive file so wipe out the existing distro area (contains contents of archive file)

	    execute_process (COMMAND rm -r breakpad-distro
                WORKING_DIRECTORY ${Breakpad_Root}
                RESULT_VARIABLE status)
	endif ()

	message ("-- ... Downloaded source archive.")

	message ("-- ... Exploding source archive")
	
	# Explode the archive

	if( APPLE )
	    execute_process (COMMAND tar xf ${Breakpad_ArchiveFile}
    		WORKING_DIRECTORY ${Breakpad_Root}
    		RESULT_VARIABLE status)
	else ()
	    execute_process (COMMAND tar xf ${Breakpad_ArchiveFile} --exclude=".git" --overwrite
    		WORKING_DIRECTORY ${Breakpad_Root}
    		RESULT_VARIABLE status)
	endif ()
	
	if (NOT ${status} EQUAL 0)
	    message (SEND_ERROR "*** Failed to explode Breakpad tar file (${Breakpad_Root}/${Breakpad_ArchiveFile})")
	else ()
	    set (rebuild TRUE)
	endif ()
	
    else ()
	message ("-- ... Using existing source archive.")
    endif ()

    if (NOT EXISTS ${Breakpad_Root}/libbreakpad_client.a)
	set (rebuild true)
    endif ()

    if (rebuild)

	message ("-- ... Configuring directory ")

	configure_breakpad (${Breakpad_Root})

    else ()
	message ("-- ... Using existing library ")
    endif()

    message ("-- ... Breakpad installation complete")

endmacro (install_breakpad)

if (UseCrashReporter)

    # Only the Breakpad_Timestamp line below should require changing when upgrading
    # to a later breakpad release

    set (Breakpad_Timestamp "160426")
    set (Breakpad_Url_At_Casa "https://svn.cv.nrao.edu/casa/devel/breakpad")

    ################################

    set (Breakpad_Root ${CMAKE_BINARY_DIR}/breakpad) # Root of breakpad within the build tree.
    set (Breakpad_ArchiveFile "breakpad-from-google-${Breakpad_Timestamp}.tgz")
    set (Breakpad_ArchiveUrl 
	"${Breakpad_Url_At_Casa}/${Breakpad_ArchiveFile}")

    ######################
    #                    #
    # Exported variables #
    #                    #
    ######################

    install_breakpad (${Breakpad_Root} ${Breakpad_ArchiveFile} ${Breakpad_ArchiveUrl})

    set (Breakpad_Library "${Breakpad_Root}/libbreakpad_client.a" CACHE STRING "Full filespec for libbreakpad.a")
    set (Breakpad_IncludeRoot "${Breakpad_Root}/breakpad-distro/src" CACHE STRING "Location of breakpad include file root")

endif ()


 



