#
# CASA - Common Astronomy Software Applications
# Copyright (C) 2010 by ESO (in the framework of the ALMA collaboration)
#
# This file is part of CASA.
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#


# - Macros for verifying the system configuration


#  casa_check_version( error p actual [EXACT] desired )
#
#        error : output error message, or empty if ok
#            p : package name
#       actual : given version number
#      desired : nominal version number(s)
#
# If each major/minor/micro number uses integer comparison, then
#
#     23.2  <  23.03    ( same as 23.3 )
#
# If float comparison is used, then
#
#     23.2  >  23.03
#
# Use the heuristics, that float comparison is used only for CFITSIO

macro( casa_check_version error p actual )
  set( desired ${ARGN} )

  list( GET desired 0 _v )
  if( _v STREQUAL EXACT )
    list( GET desired 1 _v )
    if( ${actual} VERSION_EQUAL ${_v} )
      set( ${error} "" )
    else()
      set( ${error} "${p} version is not ${_v}. Please check!" )
    endif()
  else()
    if( ${p} MATCHES "^CFITSIO" )
      set( _cmp GREATER )
    else()
      set( _cmp VERSION_GREATER )
    endif()
    if( ${actual} VERSION_EQUAL ${_v} OR
        ${actual} ${_cmp} ${_v} )
      set( ${error} "" )
    else()
      set( ${error} "${p} version is not ${_v} or later. Please check!" )
    endif()
  endif()

endmacro()





#   casa_find - Generic package finder.
#
#   Synopsis:
#
#      casa_find(  package
#               [ VERSION version ]
#               [ INCLUDES include1 ... ]
#               [ INCLUDES_SUFFIX suffix1 ... ]
#               [ PREFIX_HINTS dir1 ... ]
#               [ LIBS lib1 ... ]
#               [ PROGRAMS program1 ... ]
#               [ CPP_VERSION cpp_version ]
#               [ RUN_VERSION run_version ]
#               [ PROG_VERSOIN prog_version ]
#               [ EXTRA_TEST extra_test ]
#               [ DEFINITIONS flag1 ... ]
#               [ DEPENDS package1 ... ]
#               [ NO_REQUIRE ]
#               [ NO_CHECK ]
#               [ NO_LINK ]
#               [ IGNORE ]
#             )
#
#   Sets the following variables
#   (naming follows CMake-2.8 guidelines)
#
#     <package>_FOUND
#     <package>_INCLUDE_DIRS
#     <package>_LIBRARIES
#     <package>_DEFINITIONS
#     <package>_<program>_EXECUTABLE   for each program
#     <package>_VERSION   (if version info is available)
#
#   All detected files are added to the variable casa_find_found.
#
#      INCLUDES: Header file names to search for
#
# INCLUDES_SUFFIXES: Possible extra suffix directory which are appended to the 
#                    candidate include paths.
#                    Passed to CMake's find_path as PATH_SUFFIXES
#
#  PREFIX_HINTS: Extra (package-specific) prefix directories to search for
#                libraries and headers. Passed to find_path() as PATHS after
#                appending "/include". Passed to find_library() as PATHS after
#                appending "/lib".
#
#          LIBS: Library names to search for (without prefix and suffix)
#
#   CPP_VERSION: A piece of C++ code, which when plugged into
#                "std::cout << ${cpp_version};" will
#                cause the compile time version number
#                to be printed on standard output.
#                cpp_version is typically a preprocessor
#                symbol defined in a header file.
#
#   RUN_VERSION: C++ code, which when plugged into
#                "std::cout << ${run_version}" will
#                cause the runtime version number
#                to be printed on standard output.
#
#    EXTRA_TEST: C+ code, which when plugged into
#                "int main(int argc, char *argv[]) { ${extra_test} }"
#                will cause the program to return zero if
#                this additional runtime test passed.
#
#  PROG_VERSION: command line options which will cause the executable
#                to print on standard output its version number. This is
#                not fully implemented: specifying PROG_VERSION causes a
#                check to be made that the executable can execute and
#                returns non-zero with the given arguments
#
#       DEPENDS: Include directories and libraries from dependency
#                packages will be used when compiling and linking
#
#    NO_REQUIRE: If defined, it is not a fatal error if the package
#                is not found
#
#      NO_CHECK: If defined, the check on whether the headers compile
#                is skipped
#
#       NO_LINK: If defined, the check on whether the libraries link
#                is skipped
#
#        IGNORE: do not consider these directories
#  
#  In addition to the given PREFIX_HINTS, this macro also searches in
#  the build directory ${CMAKE_INSTALL_PREFIX}, and
#  in /usr/lib(64)/casapy (Linux) or /opt/casa/core2-apple-darwin8/3rd-party (Mac)
#
#  For example:    casa_find( CFITSIO
#                             INCLUDES fits.h
#                             INCLUDES_SUFFIXES cfitsio somewhere
#                             PREFIX_HINTS /usr2
#                           )
#
#  will search for header "fits.h" in an exponential number of locations:
#      /usr2/include/fits.h
#      /usr2/include/cfitsio/fits.h
#      /usr2/include/somewhere/fits.h
#      ${CMAKE_INSTALL_PREFIX}/include/fits.h
#      ${CMAKE_INSTALL_PREFIX}/cfitsio/fits.h
#      ${CMAKE_INSTALL_PREFIX}/somewhere/fits.h
#      /usr/lib/casapy/include/fits.h
#      /usr/lib/casapy/include/cfitsio/fits.h
#      /usr/lib/casapy/include/somewhere/fits.h
#      /sw/include/fits.h
#      /sw/include/cfitsio/fits.h
#      /sw/include/somewhere/fits.h
#      /opt/local/include/fits.h
#      /opt/local/include/cfitsio/fits.h
#      /opt/local/include/somewhere/fits.h
#      /opt/include/fits.h
#      /opt/include/cfitsio/fits.h
#      /opt/include/somewhere/fits.h
#   and at CMake's default search paths:
#      /usr/include/fits.h
#      /usr/include/cfitsio/fits.h
#      /usr/include/somewhere/fits.h
#      /usr/local/include/fits.h
#      /usr/local/include/cfitsio/fits.h
#      /usr/local/include/somewhere/fits.h
#      ... cut, more CMake built-ins ...
#

set( casa_find_found "" )  # Accumulated list of libraries and executables found by casa_find

macro( casa_find package )
  
  # Parse arguments
  set( _current "illegal" )
  set( _version "" )
  set( _programs "" )
  set( _prog_version "" )
  set( _includes "" )
  set( _includes_suffixes "" )
  set( _libs "" )
  set( _prefix_hints "" )
  set( _cpp_version "" )
  set( _run_version "" )
  set( _extra_test "" )
  set( _definitions "" )
  set( _depends "" )
  set( _ignore "" )
  set( _no_require False )
  set( _no_check False )
  set( _no_link False )
  foreach ( _a ${ARGN} )
    
    if(${_a} STREQUAL VERSION)
      set( _current _version )
    elseif (${_a} STREQUAL PROGRAMS )
      set( _current _programs )
    elseif (${_a} STREQUAL PROG_VERSION )
      set( _current _prog_version )
    elseif (${_a} STREQUAL INCLUDES )
      set( _current _includes )
    elseif (${_a} STREQUAL INCLUDES_SUFFIXES )
      set( _current _includes_suffixes )
    elseif (${_a} STREQUAL LIBS)
      set( _current _libs )
    elseif (${_a} STREQUAL PREFIX_HINTS )
      set( _current _prefix_hints )
    elseif (${_a} STREQUAL IGNORE )
      set( _current _ignore )
    elseif (${_a} STREQUAL CPP_VERSION )
      set( _current _cpp_version )
    elseif (${_a} STREQUAL RUN_VERSION )
      set( _current _run_version )
    elseif( ${_a} STREQUAL EXTRA_TEST )
      set( _current _extra_test )
    elseif (${_a} STREQUAL DEFINITIONS )
      set( _current _definitions )
    elseif (${_a} STREQUAL DEPENDS )
      set( _current _depends )
    elseif (${_a} STREQUAL NO_REQUIRE )
      set( _no_require True )
      set( _current "illegal" )
    elseif (${_a} STREQUAL NO_CHECK )
      set( _no_check True )
      set( _current "illegal" )
    elseif (${_a} STREQUAL NO_LINK )
      set( _no_link True )
      set( _current "illegal" )
    elseif( ${_a} STREQUAL "illegal" )
      message( FATAL_ERROR "Illegal macro invocation ${ARGN}" )
    else()
      # Append argument to the current variable
      set( ${_current} ${${_current}} ${_a} )
    endif()

  endforeach()

  if( casa_config_is_over )
    message( FATAL_ERROR "Internal error! casa_find() must not be called after casa_config_end()" )
  endif()

  # Done parsing

  #dump( package _version _programs _prog_version _includes _libs _cpp_version _run_version _extra_test _definitions )

  #message(STATUS "Looking for ${package} version ${_version}, must have ${_includes} and ${_libs} ...................")
  
  # Dependencies

  # pkg-config
  #find_package(PkgConfig)
  #if( PKG_CONFIG_FOUND )
  #  pkg_check_modules( ${package}_PKGCONF ${package} )
  #endif()

  if( _version )
    list( GET _version 0 _v )
    if( _v STREQUAL EXACT )
      list( GET _version 1 _v )
      set( _version_txt "is exactly ${_v}" )
    else()
      set( _version_txt "is ${_v} or later" )
    endif()
  endif()

  if( NOT "${${package}_VERSION_SPEC}" STREQUAL "${_version}" )
    set( ${package}_VERSION_SPEC "${_version}" CACHE STRING "${package} version requirement" FORCE )
    set( ${package}_FOUND False CACHE BOOL "Was ${package} found?" FORCE )
    # Remember the version specification so that the detection can be repeated if the version specification has changed
    # in-between cmake runs (even if the package was already found)
  endif()

  if( NOT ${package}_FOUND )

  # Check that dependencies are detected before proceeding
  # DL libraries may be undefined on Mac
  foreach( _d ${_depends} )
    #dump( ${_d}_INCLUDE_DIRS )
    #dump( ${_d}_LIBRARIES )
    if( NOT ${_d}_INCLUDE_DIRS AND NOT ${_d}_LIBRARIES AND NOT ${_d} STREQUAL DL )
      message( FATAL_ERROR "${_d}_INCLUDE_DIRS and ${_d}_LIBRARIES are undefined! Cannot detect ${package}" )
    endif()
  endforeach()


  ##
  ## strip out ignored header prefixes...
  ##
  set( _trimmed_prefix_hints "" )
  foreach( _p ${_prefix_hints} )
    set(_ok "yes")
    foreach( _ix ${_ignore} )
      if( ${_p} MATCHES "^${_ix}" )
        set(_ok "no")
        message( STATUS "        ignoring ${_p} when looking for ${package}")
      endif()
    endforeach()
    if( ${_ok} STREQUAL "yes" )
      list( APPEND _trimmed_prefix_hints "${_p}" )
    endif()
  endforeach()
  set( _prefix_hints ${_trimmed_prefix_hints} )
  ##
  ## strip out ignored paths for standard paths...
  ##
  set( _standard_include_paths "" )
  foreach( _p ${CMAKE_INSTALL_PREFIX}/include ${casa_packages}/include
              /opt/casa/01/include /opt/local/include /sw/include /opt/include )
    set(_ok "yes")
    foreach( _ix ${_ignore} )
      if( ${_p} MATCHES "^${_ix}" )
        set(_ok "no")
        message( STATUS "        ignoring standard path ${_p} when looking for ${package}")
      endif()
    endforeach()
    if( ${_ok} STREQUAL "yes" )
      list( APPEND _standard_include_paths "${_p}" )
    endif()
  endforeach()

  set( _standard_library_paths "" )
  foreach( _p  ${CMAKE_INSTALL_PREFIX}/lib ${casa_packages}/lib
               /opt/casa/01/lib /opt/local/lib /sw/lib /opt/lib )
    set(_ok "yes")
    foreach( _ix ${_ignore} )
      if( ${_p} MATCHES "^${_ix}" )
        set(_ok "no")
        message( STATUS "        ignoring standard path ${_p} when looking for ${package}")
      endif()
    endforeach()
    if( ${_ok} STREQUAL "yes" )
      list( APPEND _standard_library_paths "${_p}" )
    endif()
  endforeach()
    
 
  set( _try ${CMAKE_BINARY_DIR}/try_run.cc )

  set( _found TRUE )

  # Set compile definitions
  set( ${package}_DEFINITIONS ${_definitions} CACHE STRING "${package} preprocessor flags" FORCE )

  # Find headers
  if( _includes )

    # If user has already defined _INCLUDE_DIRS (but not _FOUND), then
    # look for headers only there
    if( ${package}_INCLUDE_DIRS ) 
      set( _paths ${${package}_INCLUDE_DIRS} )
      message( STATUS "Looking for ${package} headers in ${_paths}" )
    else()
      set( _prefix_hints_include "" )
      foreach( _p ${_prefix_hints} )
        list( APPEND _prefix_hints_include "${_p}/include" )
      endforeach()

      set( _paths
        ${_prefix_hints_include}
        ${_standard_include_paths}
       )
    endif()
    set( ${package}_INCLUDE_DIRS "" )
 
    foreach( _include ${_includes} )
      
      message( STATUS "Looking for ${package} header ${_include}" )

      unset( ${package}_${_include} CACHE )

      find_path( ${package}_${_include}
        NAMES ${_include}
        PATHS ${_paths}
        PATH_SUFFIXES ${_includes_suffixes}
        NO_DEFAULT_PATH
        )

      # If not found, search CMake's default paths
      if( ${package}_${_include} MATCHES "NOTFOUND$" )
        find_path( ${package}_${_include} 
          NAMES ${_include}
          PATH_SUFFIXES ${_includes_suffixes} 
          )
      endif()

      if( ${package}_${_include} MATCHES "NOTFOUND$" )
        message( STATUS "Looking for ${package} header ${_include} -- NOT FOUND" )
        set( _found FALSE )
        unset( ${package}_INCLUDE_DIRS CACHE )
      else()
        message( STATUS "Looking for ${package} header ${_include} -- ${${package}_${_include}}/${_include}" )
      endif()

      casa_append( ${package}_INCLUDE_DIR ${${package}_${_include}} )
      ## also include the base-path for added base-path + suffix paths...	 
      ##   (needed on osx for <ATM/*.h> includes)	 
      foreach ( _s ${_includes_suffixes} )		  
        string( REPLACE "." "\\." _r ${_s} )		   
        string( REPLACE "*" "\\*" _r ${_r} )		    
        string( REPLACE "+" "\\+" _r ${_r} )		     
        string( REPLACE "?" "\\?" _r ${_r} )		      
        if( ${${package}_${_include}} MATCHES "${_r}$" )      
            string( REGEX REPLACE "/${_r}$" "" ${package}_${_include}_root ${${package}_${_include}} )	 
            casa_append( ${package}_INCLUDE_DIR  ${${package}_${_include}_root} )		       
        endif()	   
      endforeach()
    endforeach()

    if( _found )
      # Add dependencies to include dirs
      set( ${package}_INCLUDE_DIRS ${${package}_INCLUDE_DIR} CACHE PATH "${package} include directories" FORCE )
      
      foreach( _d ${_depends} )
        foreach( _di ${${_d}_INCLUDE_DIRS} )
          casa_append( ${package}_INCLUDE_DIRS ${_di} CACHE PATH "${package} include directories" FORCE )
        endforeach()
      endforeach()
      
      # Try to compile program, and check compile version, if applicable
      message( STATUS "Checking whether ${package} headers compile")
        
      # CCMTOOLS headers define (not declare) symbols (!) and thus do not compile without linking.
      if( _no_check )
        message( STATUS "Checking whether ${package} headers compile -- skipped")
      else()
        
        file( WRITE ${_try}
          "/* try_run.cc */\n"
          "#include <cstdio>  /* Needed for readline/readline.h */\n"
          )
        foreach( _i ${_includes} )
          
          file( APPEND ${_try}
            "#include <${_i}>\n"
            )
        endforeach()
        file( APPEND ${_try}
          "#include <iostream>\n"
          "int main()\n"
          "{\n"
          )
        
        if( _cpp_version )
          file( APPEND ${_try}
            "    std::cout << ${_cpp_version};\n"
            )
        endif()
        file( APPEND ${_try}
          "    return 0;\n"
          "}\n"
          )
        
        try_run( ${package}_RUN ${package}_COMPILE ${CMAKE_BINARY_DIR} ${_try}
          CMAKE_FLAGS -Wdev
          "-DINCLUDE_DIRECTORIES=${${package}_INCLUDE_DIRS}"
          COMPILE_DEFINITIONS ${${package}_DEFINITIONS}
          COMPILE_OUTPUT_VARIABLE _compile_out
          RUN_OUTPUT_VARIABLE _run_out )
        message( STATUS "Checking whether ${package} headers compile -- ${${package}_COMPILE}" )
        
        if( NOT ${package}_COMPILE )
          file( READ ${_try} _prog )
	  message( SEND_ERROR "Could not compile ${package} headers:" )
          message( "${_compile_out}" )
          message( "Failed program was:" )
          message( "${_prog}" )
          unset( ${package}_INCLUDE_DIRS CACHE )
          message( FATAL_ERROR "Could not compile ${package} headers. Please check!" )
        endif()
        
        # Compile time version check
        if( _cpp_version ) 
          message( STATUS "Checking that ${package} compile version ${_version_txt}" )
          if( ${package}_RUN )
            file( READ ${_try} _prog )
	    message( SEND_ERROR "Could compile but not run ${package}:" )
            message(" ${_compile_out}" )
            message(" ${_run_out}" )
            message( "exit code: ${${package}_RUN}" )
            message( "Failed program was:" )
            message( "${_prog}" )
            unset( ${package}_INCLUDE_DIRS CACHE )
            message( FATAL_ERROR "Could compile but not run ${package}. Please check!" )
          endif()
          
          if( _run_out STREQUAL "" )
            file( READ ${_try} _prog )
	    message( SEND_ERROR "Illegal version number from ${package}:" )
            message(" ${_compile_out}" )
            message(" ${_run_out}" )
            message( "exit code: ${${package}_RUN}" )
            message( "Program produced no output (version number expected):" )
            message( "${_prog}" )
            unset( ${package}_INCLUDE_DIRS CACHE )
            message( FATAL_ERROR "Illegal version number from ${package}. Please check!" )
          endif()
          
          casa_check_version( _error_msg "${package} compile" "${_run_out}" ${_version} )

          if( _error_msg )
            if( CASA_IGNORE_VERSION )
              message( STATUS "Checking that ${package} compile version ${_version_txt} -- found ${_run_out} -- ignored" )
            else()
              message( STATUS "Checking that ${package} compile version ${_version_txt} -- found ${_run_out}" )
              unset( ${package}_INCLUDE_DIRS CACHE )
              message( FATAL_ERROR ${_error_msg} )
          endif()
          else()
            message( STATUS "Checking that ${package} compile version ${_version_txt} -- found ${_run_out} -- ok" )
            set( ${package}_VERSION ${_run_out} CACHE STRING "${package} version" FORCE )
          endif()
        endif()
        # end header version check
        
      endif()
      #end if not CCMTOOLS

    endif()
    #end if all headers were found

  endif()
  # end looking for headers

  # Find libraries
  if( _found AND _libs )

    if( ${package}_LIBRARIES )
      set( ${package}_LIBRARY ${${package}_LIBRARIES} )
      # ... and add any dependency libraries below
      message( STATUS "Using ${package} libraries ${${package}_LIBRARY}" )
    else()

      set( _prefix_hints_lib "" )
      foreach( _p ${_prefix_hints} )
        list( APPEND _prefix_hints_lib "${_p}/lib" )
      endforeach()

      set( _paths
        ${_prefix_hints_lib}       # append lib to each?
        ${_standard_library_paths}
        )
      # Note: find_library() automatically searches in and prefers
      # a lib64 variant of these paths, if it exists (see man cmake).

      set( ${package}_LIBRARY "" )

      foreach ( _lib ${_libs} )
        message( STATUS "Looking for ${package} library ${_lib}" )

        # Clear variable in order to force new search
        unset( ${package}_${_lib} CACHE )

        find_library( ${package}_${_lib}
          NAMES ${_lib} 
          PATHS ${_paths}
          NO_DEFAULT_PATH
          )
        
        # If not found, search CMake's default paths
        if( ${package}_${_lib} MATCHES "NOTFOUND$" )
          find_library( ${package}_${_lib}
            NAMES ${_lib} )
        endif()

        if( ${package}_${_lib} MATCHES "NOTFOUND$" )
          message( STATUS "Looking for ${package} library ${_lib} -- NOT FOUND" )
          set( _found FALSE )
          unset( ${package}_LIBRARIES CACHE )
        else()
          message( STATUS "Looking for ${package} library ${_lib} -- ${${package}_${_lib}}" )
        endif()

        casa_append( ${package}_LIBRARY ${${package}_${_lib}} )

      endforeach()

    endif()

    if( _found )
      # Add dependencies to libs
      set( ${package}_LIBRARIES ${${package}_LIBRARY} CACHE FILEPATH "${package} libraries" FORCE )
      foreach( _d ${_depends} )
        foreach( _dl ${${_d}_LIBRARIES} )
          casa_append( ${package}_LIBRARIES ${_dl} CACHE FILEPATH "${package} libraries" FORCE )
        endforeach()
      endforeach()

      # Link to program and check runtime version
      if( _no_link )
        message( STATUS "Checking whether ${package} links -- skipped")
      else()
        message( STATUS "Checking whether ${package} links")
      
      file( WRITE ${_try}
        "/* try_run.cc */\n"
        "#include <cstdio>  /* Needed for readline/readline.h */\n"
        )
      foreach( _i ${_includes} )
        
        file( APPEND ${_try}
          "#include <${_i}>\n"
          )
      endforeach()
      file( APPEND ${_try}
        "#include <iostream>\n"
        "int main() {\n"
        #      "    float v;\n"
        )
      
      if( _run_version )
        file( APPEND ${_try}
          "    float v;\n"
          "    v = 0; /* Suppress warning about unused variable (needed for CFITSIO) */\n"
          "    std::cout << ${_run_version};\n"
          )
      endif()
      file( APPEND ${_try}
        "    return 0;\n"
        "}\n"
        )

      # Same program as before, but this time link the libraries in
      try_run( ${package}_RUN ${package}_COMPILE ${CMAKE_BINARY_DIR} ${_try}
        CMAKE_FLAGS -Wdev
        "-DINCLUDE_DIRECTORIES=${${package}_INCLUDE_DIRS}"
        "-DLINK_LIBRARIES=${${package}_LIBRARIES}"
        "-DCMAKE_EXE_LINKER_FLAGS=${CMAKE_SHARED_LINKER_FLAGS}"
        COMPILE_DEFINITIONS ${${package}_DEFINITIONS}
        COMPILE_OUTPUT_VARIABLE _compile_out
        RUN_OUTPUT_VARIABLE _run_out )

      if( NOT ${package}_COMPILE )
        file( READ ${_try} _prog )
	message( SEND_ERROR "Could not link to ${package} libraries:" )
        message( "${_compile_out}" )
        message( "Failed program was:" )
        message( "${_prog}" )
        unset( ${package}_LIBRARIES CACHE )
        message( FATAL_ERROR "Could not link to ${package} libraries. Please check!" )
      endif()

      message( STATUS "Checking whether ${package} links -- ok")
      
      message( STATUS "Checking whether ${package} links and runs" )
      
      if( ${package}_RUN )
        file( READ ${_try} _prog )
	message( SEND_ERROR "Could compile and link to, but not run ${package}:" )
        message(" ${_compile_out}" )
        message(" ${_run_out}" )
        message( "exit code: ${${package}_RUN}" )
        message( "Failed program was:" )
        message( "${_prog}" )
        unset( ${package}_LIBRARIES CACHE )
        message( FATAL_ERROR "Could compile and link to, but not run ${package}. Please check!" )
      endif()

      message( STATUS "Checking whether ${package} links and runs -- ok" )
      
      # Runtime version check
      if( _run_version ) 
        message( STATUS "Checking that ${package} runtime version ${_version_txt}" )
        
        if( _run_out STREQUAL "" )
          file( READ ${_try} _prog )
	  message( SEND_ERROR "Illegal version number from ${package}:" )
          message(" ${_compile_out}" )
          message(" ${_run_out}" )
          message( "exit code: ${${package}_RUN}" )
          message( "Program produced no output (version number expected):" )
          message( "${_prog}" )
          unset( ${package}_LIBRARIES CACHE )
          message( FATAL_ERROR "Illegal version number from ${package}. Please check!" )
        endif()
        
        casa_check_version( _error_msg "${package} runtime" "${_run_out}" ${_version} )
        
        if( _error_msg )
          if( CASA_IGNORE_VERSION )
            message( STATUS "Checking that ${package} runtime version ${_version_txt} -- found ${_run_out} -- ignored" )
          else()
            message( STATUS "Checking that ${package} runtime version ${_version_txt} -- found ${_run_out}" )
            unset( ${package}_LIBRARIES CACHE )
            message( FATAL_ERROR ${_error_msg} )
          endif()
        else()
          message( STATUS "Checking that ${package} runtime version ${_version_txt} -- found ${_run_out} -- ok" )
          set( ${package}_VERSION ${_run_out} CACHE STRING "${package} version" FORCE )
          # runtime version takes precedence over compile time version
        endif()
      endif()
      # end runtime version check
      endif()
      # end no linkage check
    endif()
    # end if all libraries were found

  endif()
  # End looking for libraries

  # Additional runtime checks
  if( _found AND _extra_test )
  
      message( STATUS "Checking that ${package} works" )

      # Use any previously found headers and libraries
      file( WRITE ${_try}
        "/* try_run.cc */\n"
        )
      foreach( _i ${_includes} )

        file( APPEND ${_try}
          "#include <${_i}>\n"
          )
      endforeach()
      file( APPEND ${_try}
        "#include <iostream>\n"
        "int main(int argc, char *argv[]) {\n"
        "${_extra_test} }\n"
        )

      try_run( ${package}_TEST _compile_status ${CMAKE_BINARY_DIR} ${_try}
      CMAKE_FLAGS -Wdev
        "-DINCLUDE_DIRECTORIES=${${package}_INCLUDE_DIRS}"
        "-DLINK_LIBRARIES=${${package}_LIBRARIES}"
        "-DCMAKE_EXE_LINKER_FLAGS=${CMAKE_SHARED_LINKER_FLAGS}"
        COMPILE_DEFINITIONS ${${package}_DEFINITIONS}
        COMPILE_OUTPUT_VARIABLE _compile_out
        RUN_OUTPUT_VARIABLE _run_out
 	)

      if( NOT _compile_status OR ${package}_TEST )
        file( READ ${_try} _prog )
        message( SEND_ERROR "${package} seems to exist (in the correct version) but does not work:" )
        message( "${_compile_out}}" )
        message( "${_run_out}" )
        message( "compile status: ${_compile_status}" )
        message( "exit code: ${${package}_TEST}" )
        message( "Failed program was:" )
        message( "${_prog}" )
        unset( ${package}_LIBRARIES CACHE )
        message( FATAL_ERROR "${package} seems to exist (in the correct version) but does not work. Please check!" )
      endif()

      message( STATUS "Checking that ${package} works -- ok")
  endif()
  # end extra tests

  # Programs
  if( _found )
    foreach ( _program ${_programs} )
      message( STATUS "Looking for ${package} program ${_program}" )
      
      # Append /bin to given PREFIX_HINTS 
      set( _paths "" )
      foreach( _p ${_prefix_hints} )
        list( APPEND _paths "${_p}/bin" )
      endforeach()
      list( APPEND _paths ${CMAKE_INSTALL_PREFIX}/bin )
      list( APPEND _paths ${casa_packages}/bin )
      list( APPEND _paths /opt/local/bin )
      list( APPEND _paths /sw/bin )
      list( APPEND _paths /opt/bin )

      unset( ${package}_${_program}_EXECUTABLE CACHE )
      find_program( ${package}_${_program}_EXECUTABLE
        ${_program}
        PATHS ${_paths}
        )
      
      if( ${package}_${_program}_EXECUTABLE MATCHES "NOTFOUND$" )
        message( STATUS "Looking for ${package} program ${_program} -- NOT FOUND" )
        set( _found FALSE )
      else()
        message( STATUS "Looking for ${package} program ${_program} -- ${${package}_${_program}_EXECUTABLE}" )
      endif()

      # Optionally, check if the program is executable
      if( _prog_version )
        message( STATUS "Checking that ${${package}_${_program}_EXECUTABLE} works" )
                
        execute_process( 
          COMMAND ${${package}_${_program}_EXECUTABLE} ${_prog_version}
          RESULT_VARIABLE _run_status
          OUTPUT_VARIABLE _run_out
          ERROR_VARIABLE _run_out
          )
      
        if( _run_status )
          message( STATUS "Checking that ${${package}_${_program}_EXECUTABLE} works -- no" )
          message( "Failing command was: ${${package}_${_program}_EXECUTABLE} ${_prog_version}" )
          message( ${_run_out} )
          set( _found FALSE )
          unset( ${package}_${_program}_EXECUTABLE CACHE )
        else()
          message( STATUS "Checking that ${${package}_${_program}_EXECUTABLE} works -- ok" )
        endif()
      endif()

    endforeach()
  endif()

  set( ${package}_FOUND ${_found} CACHE BOOL "Was ${package} found?" FORCE )

  if( NOT ${package}_FOUND AND NOT ${_no_require} )
    unset( ${package}_INCLUDE_DIRS CACHE )
    unset( ${package}_LIBRARIES CACHE )
    message( FATAL_ERROR "${package} could not be found. Please check!" )
  endif()

  else()
    message( STATUS "Looking for ${package} -- (cached) ok " )
  endif()

  #
  # Append to casa_find_found all libraries + programs files for this package
  #
  foreach( _f ${${package}_LIBRARIES} )
    if( NOT ${_f} MATCHES "^-l" ) # ignore libraries given as -l
      if( NOT EXISTS ${_f} )
        message( WARNING "The file \"${_f}\" was detected by casa_find() but does not exist" )
      endif()
      list( APPEND casa_find_found ${_f} )
    endif()
  endforeach()
  
  foreach ( _program ${_programs} )
    set( _f ${${package}_${_program}_EXECUTABLE} )
    if( NOT EXISTS ${_f} )
      message( WARNING "The file \"${_f}\" was detected by casa_find() but does not exist" )
    endif()
    list( APPEND casa_find_found ${_f} )
  endforeach()

  #dump( ${package}_FOUND     ${package}_INCLUDE_DIRS    ${package}_LIBRARIES    ${package}_DEFINITIONS )
  #foreach( _p ${_programs} )
  #  dump( ${package}_${_p}_EXECUTABLE )
  #endforeach()
   
endmacro( casa_find )


#
# This macro is to be run after invocations of casa_find().
#
# It causes a compilation error to happen if files listed in
# ${casa_find_found} no longer exist.
#
macro( casa_config_end )

  set( casa_config_is_over TRUE )
  # Flag used to enforce that casa_find() must not be called
  # after this macro

  list( REMOVE_DUPLICATES casa_find_found )

  add_custom_target( config_check ALL
      ${PERL_EXECUTABLE} -le 'foreach (@ARGV) { if ( ! -e ) { print STDERR \"ERROR: The file $$_ has disappeared since it was detected by CMake. Cannot continue. It might help to clear CMake s cache (rm CMakeCache.txt) and rerun CMake\"\; exit 1;} }' -- ${casa_find_found}
      )

endmacro( casa_config_end )
