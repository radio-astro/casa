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

if (APPLE)

    # Determine location of packages on various Apple
    # OS-version/processor combinations.

    if (CMAKE_SYSTEM MATCHES ^Darwin-9)

        set (casa_packages /opt/casa/core2-apple-darwin8/3rd-party)

    elseif (CMAKE_SYSTEM MATCHES ^Darwin-10)

        if (CASA_SYSTEM_PROCESSOR MATCHES x86_64)

            set  (casa_packages /opt/casa/darwin10-64b) # Darwin 10 64-bit

        else (CASA_SYSTEM_PROCESSOR MATCHES x86_64)

            set (casa_packages /opt/casa/core2-apple-darwin10) # Darwin 10 32-bit

        endif (CASA_SYSTEM_PROCESSOR MATCHES x86_64)

    elseif  (CMAKE_SYSTEM MATCHES ^Darwin-11)

        if (CASA_SYSTEM_PROCESSOR MATCHES x86_64)

	    set  (casa_packages /opt/casa/darwin11) # Darwin 11 64-bit

        else (CASA_SYSTEM_PROCESSOR MATCHES x86_64)

	    messge (FATAL_ERROR "Not expecting Darwin 11 with 32-bit processor")

        endif (CASA_SYSTEM_PROCESSOR MATCHES x86_64)

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
