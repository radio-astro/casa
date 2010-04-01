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
# Use the heuristics, that if the version string contains ".0"
# then float comparison is used, otherwise integer

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
    if( ${actual} MATCHES "\\.0" OR ${_v} MATCHES "\\.0" )
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
#               [ LIBS lib1 ... ]
#               [ LIBS_HINTS dir1 ... ]
#               [ PROGRAMS program1 ... ]
#               [ CPP_VERSION cpp_version ]
#               [ RUN_VERSION run_version ]
#               [ PROG_VERSOIN prog_version ]
#               [ EXTRA_TEST extra_test ]
#               [ DEFINITIONS flag1 ... ]
#               [ DEPENDS package1 ... ]
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
#      INCLUDES: Header file names to search for
#
# INCLUDES_HINTS: (Optional) extra directories where to search for headers
#
#          LIBS: Library names to search for (without prefix and suffix)
#
#    LIBS_HINTS: (Optional) Extra directories where to search for libraries
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
#  PROG_VERSION: shell command which prints on standard output
#                the package version
#
#       DEPENDS: Include directories and libraries from dependency
#                packages will be used when compiling and linking
macro( casa_find package )
  
  # Parse arguments
  set( _current "illegal" )
  set( _version "" )
  set( _programs "" )
  set( _prog_version "" )
  set( _includes "" )
  set( _includes_hints "" )
  set( _libs "" )
  set( _libs_hints "" )
  set( _cpp_version "" )
  set( _run_version "" )
  set( _extra_test "" )
  set( _definitions "" )
  set( _depends "" )
  foreach ( _a ${ARGN} )
    
    if(${_a} STREQUAL VERSION)
      set( _current _version )
    elseif (${_a} STREQUAL PROGRAMS)
      set( _current _programs )
    elseif (${_a} STREQUAL PROG_VERSION)
      set( _current _prog_version )
    elseif (${_a} STREQUAL INCLUDES)
      set( _current _includes )
    elseif (${_a} STREQUAL INCLUDES_HINTS )
      set( _current _includes_hints )
    elseif (${_a} STREQUAL LIBS)
      set( _current _libs )
    elseif (${_a} STREQUAL LIBS_HINTS)
      set( _current _libs_hints )
    elseif (${_a} STREQUAL CPP_VERSION)
      set( _current _cpp_version )
    elseif (${_a} STREQUAL RUN_VERSION)
      set( _current _run_version )
    elseif( ${_a} STREQUAL EXTRA_TEST )
      set( _current _extra_test )
    elseif (${_a} STREQUAL DEFINITIONS)
      set( _current _definitions )
    elseif (${_a} STREQUAL DEPENDS)
      set( _current _depends )
    elseif( ${_a} STREQUAL "illegal" )
      message( FATAL_ERROR "Illegal macro invocation ${ARGN}" )
    else()
      # Append argument to the current variable
      set( ${_current} ${${_current}} ${_a} )
    endif()

  endforeach()

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
  foreach( _d ${_depends} )
    #dump( ${_d}_INCLUDE_DIRS )
    #dump( ${_d}_LIBRARIES )
    if( NOT ${_d}_INCLUDE_DIRS AND NOT ${_d}_LIBRARIES )
      message( FATAL_ERROR "${_d}_INCLUDE_DIRS and ${_d}_LIBRARIES are undefined! Cannot detect ${package}" )
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
      set( _hints ${${package}_INCLUDE_DIRS} )
      message( STATUS "Looking for ${package} headers in ${_hints}" )
    else()
      set( _hints
        ${_includes_hints}
	${CMAKE_INSTALL_PREFIX}/include
        ${casa_packages}/include
        /usr/local/include
        /usr/include
       )
    endif()
    set( ${package}_INCLUDE_DIRS "" )
 
    foreach( _include ${_includes} )
      
      message( STATUS "Looking for ${package} header ${_include}" )

      unset( ${package}_${_include} CACHE )

      find_path( ${package}_${_include}
        NAMES ${_include}
        PATHS ${_hints}
        NO_DEFAULT_PATH
        )
      if( ${package}_${_include} MATCHES "NOTFOUND$" )
        message( STATUS "Looking for ${package} header ${_include} -- NOT FOUND" )
        set( _found FALSE )
        unset( ${package}_INCLUDE_DIRS CACHE )
      else()
        message( STATUS "Looking for ${package} header ${_include} -- ${${package}_${_include}}/${_include}" )
      endif()

      casa_append( ${package}_INCLUDE_DIR ${${package}_${_include}} )
      
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
      if( ${package} STREQUAL CCMTOOLS )
        message( STATUS "Checking whether ${package} headers compile -- skipped, they never do")
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
      set( _hints
        ${_libs_hints}
	${CMAKE_INSTALL_PREFIX}/lib
        ${casa_packages}/lib
        /usr/local/lib
        /usr/lib
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
          PATHS ${_hints}
          NO_DEFAULT_PATH
          )
    
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
      
      unset( ${package}_${_program}_EXECUTABLE CACHE )
      find_program( ${package}_${_program}_EXECUTABLE
        ${_program}
        PATHS 
        /usr/lib/qt-4.3.4/dbus/bin/
        ${casa_packages}/bin
        )
      
      if( ${package}_${_program}_EXECUTABLE MATCHES "NOTFOUND$" )
        message( STATUS "Looking for ${package} program ${_program} -- NOT FOUND" )
        set( _found FALSE )
      else()
        message( STATUS "Looking for ${package} program ${_program} -- ${${package}_${_program}_EXECUTABLE}" )
      endif()
      
      #if( TRUE )
      #  execute_process( COMMAND ${_prog_version} )
      #endif()
      
    endforeach()
  endif()

  set( ${package}_FOUND ${_found} CACHE BOOL "Was ${package} found?" FORCE )

  if( NOT ${package}_FOUND)
    unset( ${package}_INCLUDE_DIRS CACHE )
    unset( ${package}_LIBRARIES CACHE )
    message( FATAL_ERROR "${package} could not be found. Please check!" )
  endif()

  else()
    #message( STATUS "${package} already found" )
  endif()
  
  #dump( ${package}_FOUND     ${package}_INCLUDE_DIRS    ${package}_LIBRARIES    ${package}_DEFINITIONS )
  #foreach( _p ${_programs} )
  #  dump( ${package}_${_p}_EXECUTABLE )
  #endforeach()
   
endmacro( casa_find )
