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


# Macros for defining libraries, executables, tests, etc


#
# In CASA, it does not make sense to build without also installing
# the build products. Therefore...
#
# Create a target which ensures that the install target is 
# always run as part of all. 
#
# For this to work, all other targets must be added as dependencies
# of this "inst" target. This is done in the macro's below using add_dependencies()
macro( casa_always_install )
  add_custom_target( inst ALL ${CMAKE_BUILD_TOOL} install/fast )
endmacro()


if (NOT TARGET unit_test)
    add_custom_target (unit_test COMMAND "cd" ${CMAKE_BINARY_DIR} COMMAND "ctest")
endif ()

#
# Setup definitions and include directories for this module,
# and create a dynamic library
#

macro( casa_add_library module )

  add_definitions( ${${module}_DEFINITIONS} )

  # Treat all but the first include dir as a "system" one which will
  # disable warnings form these often uneditable files.  The first one
  # is the path to code which is how CASA header files will be found
  # and there is no leniency for them.  Order of these files is set
  # in casa_add_module.

  list (GET ${module}_INCLUDE_DIRS 0 codeIncludeDir)
  list (REMOVE_ITEM ${module}_INCLUDE_DIRS ${codeIncludeDir})

  include_directories( SYSTEM ${${module}_INCLUDE_DIRS} )
  include_directories( ${codeIncludeDir})

  # Create the target lib<module>, but set the output library
  # filename to lib<module>.<suffix> (which would have defaulted
  # to liblib<module>.<suffix>). The target named <module> is the 
  # umbrella target for all targets (libraries, executables, ...) 
  # in this subdirectory.

  add_library( lib${module} ${ARGN} )
  set_target_properties( lib${module} PROPERTIES OUTPUT_NAME ${module} )

  if (${module}_WarningsAsErrors)
    set_property (TARGET lib${module} APPEND PROPERTY COMPILE_FLAGS "-Werror")
    message ("-- Library lib${module} has WarningsAsErrors")
  endif ()

  add_dependencies( inst lib${module} )
  add_custom_target( lib${module}_fast ${CMAKE_BUILD_TOOL} lib${module}/fast )
  add_dependencies( ${module}_fast lib${module}_fast )

  target_link_libraries( lib${module} ${${module}_LINK_TO} )

  if( NOT NO_SOVERSION )
      set_target_properties( lib${module} PROPERTIES SOVERSION ${casa_soversion} )
  endif()

  install( TARGETS lib${module} LIBRARY DESTINATION lib )

endmacro()

#
# casa_add_executable( module name source1 [source2...] )
#
#     Add an executable, and define the install rules.
#
macro( casa_add_executable module name )

  set( _sources ${ARGN})

  add_executable( ${name} ${_sources} )

  if (NOT ${module} STREQUAL NONE)

    if (${module}_WarningsAsErrors)
      set_property (TARGET ${name} APPEND PROPERTY COMPILE_FLAGS "-Werror")
      message ("-- Executable ${name} has WarningsAsErrors")
    endif ()

    target_link_libraries( ${name} lib${module} ${INTEL_LIBS} )
    add_dependencies( ${module}_fast ${name}_fast )

  endif ()

  add_dependencies( inst ${name} )
  add_custom_target( ${name}_fast ${CMAKE_BUILD_TOOL} ${name}/fast )

  install( TARGETS ${name} RUNTIME DESTINATION bin )

  if( APPLE )
    # On Mac, things are installed here too.
    install( TARGETS ${name} RUNTIME DESTINATION apps/${name}.app/Contents/MacOS )
  endif()

endmacro()



#
# Add a CASA subdirectory (module)
#
#  The following variables are set
#     <module>_INCLUDE_DIRS
#     <module>_DEFINITIONS
#     <module>_LINK_TO
# 
# Fixme: move qt4, dbus, alma special cases
# to client code

macro( casa_add_module module )

  set (options WarningsAsErrors) 
  set (oneValueArgs ) # None
  set (multiValueArgs )

  cmake_parse_arguments (casa_add_module "${options}" "${oneValueArgs}" 
                         "${multiValueArgs}" ${ARGN})

  set( _dependencies ${casa_add_module_UNPARSED_ARGUMENTS})

  # Internal target to update this module including dependencies,
  # then install this module, excluding dependencies
  add_custom_target( 
    ${module}_install
    COMMAND ${CMAKE_BUILD_TOOL} install 
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/${module}
    )

  # Target to update and install this module, excluding dependencies
  add_custom_target( 
    ${module}_fast
    COMMAND ${CMAKE_BUILD_TOOL} install/fast
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/${module}
    COMMENT "Updating ${module} (fast)..."
    )

  # Target to update and install this module, including dependency modules
  add_custom_target( ${module} COMMENT "Updating ${module}..." )
  add_dependencies( ${module} ${module}_install )

  # If WarningsAsErrors was specified, then add a compiler flag to treat 
  # warnings as error for this module

  if ( casa_add_module_WarningsAsErrors )
    if (APPLE)
        message ("**** WARNING: Module ${module} marked as locked against warnings ")
        message ("****          but inadequate hardware support for Apple platforms ")
        message ("****          makes implementation impractical.")
    else ()
        set (${module}_WarningsAsErrors true)
    endif ()
 endif ()

  # Include path always include code/
  set( ${module}_INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/ )
  set( ${module}_DEFINITIONS "" )

  # Which libraries to link against (internally used, not exported, by the module)
  set( ${module}_LINK_TO "" )

  foreach( _dep ${_dependencies} )

    if( NOT DEFINED ${_dep}_INCLUDE_DIRS AND NOT DEFINED ${_dep}_LIBRARIES )
      if( NOT _dep STREQUAL DL )  # the dl libraries may be empty on Mac
        message( FATAL_ERROR "${_dep} is listed as dependency for ${module}, but both ${_dep}_INCLUDE_DIRS and ${_dep}_LIBRARIES are undefined!" )
      endif()
    endif()
    
    # INCLUDE_DIRS 
    # Add each of the dependent's include dirs
    # to this module's include dirs
    # (if not already contained in _INCLUDE_DIRS)
    foreach( _dir ${${_dep}_INCLUDE_DIRS} )

      casa_append( ${module}_INCLUDE_DIRS ${_dir} )

    endforeach()

    # DEFINITIONS 
    foreach( _def ${${_dep}_DEFINITIONS} )

      casa_append( ${module}_DEFINITIONS ${_def} )

    endforeach()

    # LINK_TO 
    # Unlike include dirs, libraries do not have
    # to be transitively propagated.
    # E.g. if A links to (depends on) B and B links to C
    # we just need to explicitly link A to B and
    # the linker will make sure that C is eventually
    # linked to A

    if( DEFINED ${_dep}_LIBRARIES OR _dep STREQUAL DL )

      # External library

      set( ${module}_LINK_TO 
        ${${module}_LINK_TO}
        ${${_dep}_LIBRARIES} )

    else()
      # Another CASA module

      set(
        ${module}_LINK_TO 
        ${${module}_LINK_TO} 
        lib${_dep} )

      add_dependencies( ${module} ${_dep}_install )

    endif()

    # Generated code need build dir in include path
    if( ${_dep} STREQUAL "DBUS" OR
        ${_dep} STREQUAL "QT4" )
      
      set( ${module}_INCLUDE_DIRS 
        ${${module}_INCLUDE_DIRS}
        ${CMAKE_BINARY_DIR} )

    endif()

    # Special case for (old)alma(_v3)
    if( ${module} STREQUAL "alma" OR
        ${module} STREQUAL "oldalma" OR
	${module} STREQUAL "alma_v3")

      set( ${module}_INCLUDE_DIRS 
        ${${module}_INCLUDE_DIRS}
        ${CMAKE_SOURCE_DIR}/${module}/ASDM
        ${CMAKE_SOURCE_DIR}/${module}/ASDMBinaries
        ${CMAKE_SOURCE_DIR}/${module}/Enumtcl
        ${CMAKE_SOURCE_DIR}/${module}/Enumerations
        )

    endif()

  endforeach()

  #dump( ${module}_DEFINITIONS ${module}_INCLUDE_DIRS ${module}_LINK_TO )

  set( all_modules ${all_modules} ${module} )

  add_subdirectory( ${module} )

endmacro()



#
# casa_add_python( module
#                  target_name
#                  installation_directory 
#                  source1 [source2 ...]
#                )
#
# Creates target for python files
#

MACRO( casa_add_python module  _target _install_dir )

  install( PROGRAMS ${ARGN}
           DESTINATION ${_install_dir} )

endmacro()





#
#   casa_add_pymodule( module name source1 [source2 ...] )
#
# Creates a python module. 
# The module is always linked to libxmlcasa and ${xmlcasa_LINK_TO}.
#

macro( casa_add_pymodule name )

  set( _sources ${ARGN} )

  add_library( ${name} MODULE ${_sources} )
  add_dependencies( inst ${name} )
  add_custom_target( ${name}_fast ${CMAKE_BUILD_TOOL} ${name}/fast )
  add_dependencies( xmlcasa_fast ${name}_fast )

  set_target_properties( ${name} PROPERTIES PREFIX "" )
  target_link_libraries( ${name} libxmlcasa ${xmlcasa_LINK_TO} )
  install( TARGETS ${name} LIBRARY DESTINATION python/${PYTHONV} )

endmacro()

##############################################################################
##############################################################################
#
# Unit test related macros


#
# casa_add_test( module source1 [source2 ...])
#
# Legacy macro for adding a unit test; used for already defined tests.
# When the test is ready for prime-time the macro cass_add_unit_test
# should be used instead.
#
macro( casa_add_test)

    set (sources ${ARGN})
    list (GET sources 0 module)
    list (REMOVE_AT sources 0)
    casa_add_unit_test (MODULES ${module} SOURCES ${sources} NOT_READY)

#  get_filename_component( _tname ${source} NAME_WE )

#  add_executable( ${_tname} EXCLUDE_FROM_ALL ${source} )
#  target_link_libraries( ${_tname} lib${module} )
#  add_test( ${_tname} ${CMAKE_CURRENT_BINARY_DIR}/${_tname} )
#  add_dependencies( check ${_tname} )

endmacro()

#
# casa_add_check_module (MODULES module1 [module2] ... TEST test)
#
# Internal macro that creates a check_MODULE make target if it doesn't exist.
# Adds dependency on "test" to that make target
#
macro( casa_add_check_module)

    set (options ) 
    set (oneValueArgs TEST) # None
    set (multiValueArgs MODULES)

    cmake_parse_arguments (casa_add_check "${options}" "${oneValueArgs}" 
                           "${multiValueArgs}" ${ARGN})
  
    set (modules ${casa_add_check_MODULES})
    set (test ${casa_add_check_TEST})

    set (targetSuffix "")
    set (pathSuffix "")
    list (GET modules 0 moduleName)
    set (moduleDirectory "${CMAKE_BINARY_DIR}/${moduleName}")

    foreach (source ${modules})

        set (targetSuffix "${targetSuffix}_${source}")
	set (pathSuffix "${pathSuffix}${source}/")
        set (target  "unit_test${targetSuffix}")

        if (NOT TARGET ${target})

            add_custom_target (${target}
                               "echo" "--- Checking module ${targetSuffix}"
                               COMMAND "cd" ${moduleDirectory}
                               COMMAND ctest)
            message ("INFO added custom target: ${target}")
        endif ()

        add_dependencies (${target} ${test})

    endforeach ()

endmacro ()

#
# casa_add_assay (module source [source1 ...])
#
# Legacy macro for adding a unit test; used for already defined tests.
# When the test is ready for prime-time the macro cass_add_unit_test
# should be used instead.
#
macro (casa_add_assay)

    set (sources ${ARGN})
    list (GET sources 0 module)
    list (REMOVE_AT sources 0)
    casa_add_unit_test (MODULES ${module} SOURCES ${sources} NOT_READY)

endmacro ()

#
# casa_add_unit_test( MODULES module [submodule [subsubmodule]  ...]
#                     SOURCES source ... 
#                     [LIBRARIES library1 [library2] ...] 
#                     [INCLUDE_DIRS dir1 [dir2] ...] 
#                     [NOT_READY])
#
#      Add a unit test. The name of the test will be the basename of
#      of the first source file.  Usually there will be a single source
#      file and not libraries or include directories added (feature used
#      for other macros such as casa_add_google_test.  The NOT_READY option
#      is used to keep an unproven unit test out of the production unit test
#      build and test process (usually used for legacy unit tests awaiting
#      vetting).
#
# Example: 
#
# casa_add_unit_test (msvis VisibilityIterator_Test.cc)
#
macro( casa_add_unit_test)

  set (options NOT_READY COMMIT_ONLY) 
  set (oneValueArgs) # None
  set (multiValueArgs LIBRARIES COMMAND_ARGUMENTS INCLUDE_DIRS SOURCES MODULES) # None

  cmake_parse_arguments (casa_unit_test "${options}" "${oneValueArgs}" 
                         "${multiValueArgs}" ${ARGN})
  
  list (GET casa_unit_test_SOURCES 0 firstSource)
  get_filename_component( testName ${firstSource} NAME_WE )
  get_filename_component(path ${firstSource} PATH)

  # If any of the other sources are missing a path provide them the
  # path from the first source file.

  set (sources "")
  foreach (source ${casa_unit_test_SOURCES})
     get_filename_component(thePath ${source} PATH)
     if (thePath)
         list (APPEND sources ${source})
     else ()
         list (APPEND sources ${path}/${source})
     endif ()
  endforeach ()

  list (GET casa_unit_test_MODULES 0 module) # First element is the module

  if (casa_unit_test_NOT_READY)
    add_executable( ${testName} EXCLUDE_FROM_ALL ${sources} ) # not part of main build
    target_link_libraries( ${testName} lib${module} ${casa_unit_test_LIBRARIES})
  else ()
    add_executable( ${testName} EXCLUDE_FROM_ALL ${sources} )
    target_link_libraries( ${testName} lib${module} ${casa_unit_test_LIBRARIES})

    if (${module}_WarningsAsErrors)
      set_property (TARGET ${testName} APPEND PROPERTY COMPILE_FLAGS "-Werror")
    endif ()
  endif ()
  
  # add_executable( ${testName} EXCLUDE_FROM_ALL ${sources} ) # not part of main build
  # target_link_libraries( ${testName} lib${module} ${casa_unit_test_LIBRARIES})

  set (moduleDirectory "${CMAKE_BINARY_DIR}/${module}")
  set (unit_test_target unit_test_${testName})
  add_custom_target (${unit_test_target}
                     COMMAND "cd" ${moduleDirectory}
                     COMMAND ctest -R ${testName})
  add_dependencies (${unit_test_target} ${testName})

  if (NOT ${casa_unit_test_INCLUDE_DIRS})
    target_include_directories (${testName} PRIVATE "${casa_unit_test_INCLUDE_DIRS}")
  endif ()

  if (NOT casa_unit_test_NOT_READY)

    # This unit test is ready for normal use.  Add it as a runnable unit test both
    # globally and within its module.
    
    add_test( ${testName} ${CASA_assay} ${CMAKE_CURRENT_BINARY_DIR}/${testName} 
              ${casa_unit_test_COMMAND_ARGUMENTS})

    add_dependencies( unit_test ${testName} )

    if (NOT casa_unit_test_COMMIT_ONLY)
        casa_add_check_module (MODULES ${casa_unit_test_MODULES} TEST ${testName})
    endif ()

    if (CASA_SHOW_TESTS)
      message ("INFO Unit test ${testName} defined for module ${module}")
    endif ()

  else ()

    # This is a legacy unit test that hasn't been examined to see whether it will
    # build or provide meaningful test results.  Add it to a separate unit_test_unready
    # and unit_test_unready_MODULE build target so that it can be built.

    message ("WARNING Unit test ${firstSource} not ready for normal unit test usage.")

    if (NOT TARGET unit_test_unready) # create the unit_test_unready target if it doesn't exist
 
       add_custom_target (unit_test_unready)
    
    endif (NOT TARGET unit_test_unready)

    if (NOT TARGET unit_test_unready_${module}) # create unit_test_unready_MODULE if needed

       add_custom_target (unit_test_unready_${module})
    
    endif (NOT TARGET unit_test_unready_${module})

    # Add the test so that unit_test_unready and unit_test_unready_MODULE targets will depend on it.

    add_dependencies( unit_test_unready ${testName} )
    add_dependencies( unit_test_unready_${module} ${testName} )

  endif ()

  set (CasaTestName ${testName})

endmacro()

# casa_add_google_test (MODULES module [submodule [subsubmodule] ...] SOURCES source1 [source2] ... [LIB library1 [library2] ...])
# 
# Adds a google-test unit test to the module.
#
# module - The name of the module (e.g., msvis, synthesis, etc.); the module is used to name the
#          library containing the module (e.g., libmsvis, libsynthesis)
#
# sources - The source files that make up the test.  
#           The first source file is taken as the name of
#           the test executable (e.g., msvis/MSVis/MyTest.cc -->
#           executable MyTest).  Paths of the source files are usually
#           relative to the directory holding the module's
#           CMakeLists.txt file.
#
# libraries - These are any additional libraries needed to link the executable.
#             CMake usually finds the expected ones and the macro
#             provides the google-test library; thus only special
#             libraries (usually test-only) need to be provided.  Not
#             providing any libraries explicitly is the more usual
#             case.  Libraries can be specified using an absolute path
#             or in a form compatible with ld's -l switch:
#             /home/orion/lib/libSomeLib.so or SomeLib (assumes that
#             the normal library search path contains libSomeLib.so).
#             # # Example: # # casa_add_google_test (Module msvis
#             SOURCES MSVis/test/VisibilityIterator_Gtest.cc
#             MSVis/test/MSFactory.cc) # # The test is invoked via the
#             code/install/assay script.  # # An XML file testName.xml
#             will be produced when the test is run.  # macro
#             (casa_add_google_test)
macro (casa_add_google_test)

    # Parse the arguments

    set (options NOT_ON_APPLE)
    set (oneValueArgs ) 
    set (multiValueArgs SOURCES LIBS MODULES) # the lists of source files and libraries
    cmake_parse_arguments (google_test "${options}" "${oneValueArgs}"
                           "${multiValueArgs}"  ${ARGN})

    list(GET google_test_SOURCES 0 testName)
    get_filename_component (testName ${testName} NAME_WE)

    if (DEFINED APPLE AND ${google_test_NOT_ON_APPLE})

        message ("WARNING:: Test ${testName} disabled on Apple platform.")

    else ()

        # The Google Test install logic in cmake will put the needed include and library
        # below the directory stored in the variable GtestRoot.

        set (gtestIncludeDirectory ${GoogleTest_ReleaseRoot}/include)
        set (gtestLibrary ${GoogleTest_LibraryDir}/libgtest.a)

        set (libraries ${gtestLibrary})
        list (APPEND gtestLibrary ${google_test_LIBS})

        casa_add_unit_test (MODULES ${google_test_MODULES} SOURCES ${google_test_SOURCES} 
                            LIBRARIES ${libraries} # gtest + provide libs
                            INCLUDE_DIRS ${gtestIncludeDirectory}) # gtest include dirs

        add_dependencies (${CasaTestName} ${GoogleTest_Target})

    endif ()

endmacro ()

# casa_add_demo (module sourceFile1 [sourceFile2 ...])
#
# Adds a demo application to the project.  A demo program is a small
# program demonstrating the use of some aspects of the module and is
# intended to serve as an example to other developers.
#
# The demo program will be built as part of the normal casa and module builds.
# This will help reduce the incidence of bit-rot; however, since the application 
# will not be executed as part of any of CASA's automatic processes there is
# some chance that the program could become unintentionally obsolete.
#
# Example:
#
# -- In code/components/CMakeLists.txt:
#
#   casa_add_demo( components ComponentModels/test/dPointShape.cc )
#
macro (casa_add_demo module)

  set (rawSources "${ARGN}")

  list (GET rawSources 0 firstSource)
  get_filename_component( executableName ${firstSource} NAME_WE )
  get_filename_component(path ${firstSource} PATH)

  set (sources "")
  foreach (source ${rawSources})
     get_filename_component(thePath ${source} PATH)
     if (thePath)
         list (APPEND sources ${source})
     else ()
         list (APPEND sources ${path}/${source})
     endif ()
  endforeach ()

  add_executable ( ${executableName} EXCLUDE_FROM_ALL ${sources})
  target_link_libraries( ${executableName} lib${module})

  # Ensure that make module builds this demo.

  add_dependencies (${executableName} lib${module})

endmacro ()