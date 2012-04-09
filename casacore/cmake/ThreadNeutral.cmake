################################################################
#
# ThreadNeutral.cmake
#
# Determines the location of the CASA distribution which will provide
# the Boost headers and libraries and setups things to allow
# thread-neutral build of both casacore and casa.
# 
# 1) Determine where the CASA distribution is located.
# 2) Find Boost in the distribution
# 3) Configure a header file that causes CASA build to
#    use thread neutrality when compiling.
#
################################################################

option (CASA_THREAD_NEUTRAL "" NO) 

if (CASA_THREAD_NEUTRAL)

    ################################################################
    #
    #   Step 1 - Locate CASA installation directory
    #
    #   Determine location of the Casa installed packages.  This
    #   is the only place that can be relied on to have the needed
    #   boost distribution.
    #
    # Sets the variable casa_packages to point to the appropriate
    # directory where casa-distributed packages are located.
    #
    # Apple
    #     Darwin 9
    #         All    --> /opt/casa/core2-apple-darwin8/3rd-party
    #     Darwin 10
    #         64-bit --> /opt/casa/darwin10-64b
    #     Darwin 11
    #         64-bit --> /opt/casa/darwin11
    #     Darwin 12
    #	      64-bit --> /opt/casa/darwin12
    # Linux
    #     32-bit --> /usr/lib/casapy
    #     64-bit --> /usr/lib64/casapy
    #
    #################################################################

    if (BOOST_ROOT)

        # A location for BOOST was explicitly provided so don't both
        # to look for the CASA distribution.

    else (BOOST_ROOT)

	if (APPLE)

	    # Determine location of packages on various Apple
	    # OS-version/processor combinations.

            # I believe this could be avoided by defining CMAKE_FIND_ROOT_PATH, wky

	    if (CMAKE_SYSTEM MATCHES ^Darwin-9)
		set (casa_packages /opt/casa/core2-apple-darwin8/3rd-party)
	    elseif (CMAKE_SYSTEM MATCHES ^Darwin-10)
		    set  (casa_packages /opt/casa/darwin10-64b) # Darwin 10 64-bit
	    elseif  (CMAKE_SYSTEM MATCHES ^Darwin-11)
		    set  (casa_packages /opt/casa/darwin11) # Darwin 11 64-bit
	    elseif  (CMAKE_SYSTEM MATCHES ^Darwin-12)
		    set  (casa_packages /opt/casa/darwin12) # Darwin 12 64-bit
	    else()
		message (FATAL_ERROR "Unsupported Mac: ${CMAKE_SYSTEM}")
	    endif()

	elseif (CMAKE_SYSTEM_NAME STREQUAL Linux)

	    # Determine location of packages on Linux

	    if (CMAKE_SYSTEM_PROCESSOR STREQUAL x86_64)

		set (casa_packages /usr/lib64/casapy) # Linux 64-bit

	    else()

		set (casa_packages /usr/lib/casapy) # Linux 32-bit

	    endif()

	else() # Neither Linux nor Apple so fail!!!

	      message (FATAL_ERROR "Unsupported system: ${CMAKE_SYSTEM_NAME}")

	endif()

	set( BOOST_ROOT ${casa_packages}/include)

    endif (BOOST_ROOT)

    #############################################################
    #
    #   Step 2 - Locate boost
    #
    #   Now that the distribution is located, find and set up the 
    #   boost-related variables
    #
    #############################################################

    set( boost_components thread serialization)
    set( min_boost_version 1.41.0)

    find_package( Boost ${min_boost_version} REQUIRED ${boost_components} )

    if (NOT Boost_FOUND)
	message( FATAL_ERROR "Boost with version >= ${min_boost_version} needed for CASA-thread-neutrality!" )
    endif (NOT Boost_FOUND)

    include_directories (${Boost_INCLUDE_DIRS})

    set (CASA_THREAD_NEUTRALITY "Yes") # for reporting only

    ######################################################
    #
    # Step 3 - Configure header file to capture thread neutrality for CASA.
    #
    # Now configure the include file so that it records the thread neutrality-related settings for use
    # by outside code using casacore.  When thread neutrality is not enabled the file will
    # end up having no defines it it.  However, if it is not present then the compilations will fail.

    # Set up these to variables to be used to flesh out a header file template later in
    # this file.
    #
    ######################################################

    set (CASA_THREAD_NEUTRAL_DEFINE "#define CASA_THREAD_NEUTRAL")
    set (USE_BOOST_SHARED_PTR_DEFINE "#define USE_BOOST_SHARED_PTR")

endif (CASA_THREAD_NEUTRAL)

# Now configure use the variables to create the header file.
#
# If thread neutrality is on then the header file will contain these two lines:
#
# #define CASA_THREAD_NEUTRAL
# #define USE_BOOST_SHARED_PTR
#
# Otherwise it will not define these two so the header file will be essentially 
# empty; this is necessary to allow switching back from enabled to disable builds.

configure_file (casa/Utilities/CasaThreadNeutrality.h.in ${CMAKE_BINARY_DIR}/casa/Utilities/CasaThreadNeutrality.h)
