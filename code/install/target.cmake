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
add_custom_target( inst ALL ${CMAKE_BUILD_TOOL} install/fast )


#
# Setup definitions and include directories for this module,
# and create a dynamic library
#

macro( casa_add_library module )

  add_definitions( ${${module}_DEFINITIONS} )
  include_directories( ${${module}_INCLUDE_DIRS} )

  add_library( ${module} ${ARGN} )
  add_dependencies( inst ${module} )

  target_link_libraries( ${module} ${${module}_LINK_TO} )

  set_target_properties( ${module} PROPERTIES SOVERSION ${CASA_API_VERSION} )

  install( TARGETS ${module} LIBRARY DESTINATION lib )

endmacro()

#
# casa_add_executable( module name source1 [source2...] )
#
#     Add an executable, and define the install rules.
#
macro( casa_add_executable module name )

  set( _sources ${ARGN} )

  add_executable( ${name} ${_sources} )
  add_dependencies( inst ${name} )
  
  target_link_libraries( ${name} ${module} )

  install( TARGETS ${name} RUNTIME DESTINATION bin )

  if( APPLE )
    # On Mac, things are installed here too.
    install( TARGETS ${name} RUNTIME DESTINATION apps/${name}.app/Contents/MacOS )
  endif()

endmacro()

#
# casa_add_test( module source )
#
#      Add a unit test. The name of the test will be the basename of
#      of the source file.
#
macro( casa_add_test module source )

  get_filename_component( _tname ${source} NAME_WE )

  add_executable( ${_tname} EXCLUDE_FROM_ALL ${source} )
  target_link_libraries( ${_tname} ${module} )
  add_test( ${_tname} ${CMAKE_CURRENT_BINARY_DIR}/${_tname} )
  add_dependencies( check ${_tname} )

endmacro()

#
# casa_add_assay( module source )
#
#      Add an assay unit test. The name of the test will be the basename of
#      of the source file.
#
macro( casa_add_assay module source )

  get_filename_component( _tname ${source} NAME_WE )

  add_executable( ${_tname} EXCLUDE_FROM_ALL ${source} )
  target_link_libraries( ${_tname} ${module} )
  add_test( ${_tname} ${CASA_assay} ${CMAKE_CURRENT_BINARY_DIR}/${_tname} )
  add_dependencies( check ${_tname} )

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

  set( _dependencies ${ARGN} )

  # Include path always include code/include/
  set( ${module}_INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/include )
  set( ${module}_DEFINITIONS "" )

  if( NOT EXISTS ${CMAKE_SOURCE_DIR}/include/${module} )
    execute_process( COMMAND ${CMAKE_COMMAND} -E create_symlink ../${module}/implement ${module}
                     WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/include )
  endif()

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

      list( FIND ${module}_INCLUDE_DIRS ${_dir} _exists )
      if( _exists EQUAL -1 )
        set( ${module}_INCLUDE_DIRS 
          ${${module}_INCLUDE_DIRS}
          ${_dir} )
      endif()
    endforeach()

    # DEFINITIONS 
    foreach( _def ${${_dep}_DEFINITIONS} )

      list( FIND ${module}_DEFINITIONS ${_def} _exists )
      if( _exists EQUAL -1 )
        set( ${module}_DEFINITIONS 
          ${${module}_DEFINITIONS}
          ${_def} )
      endif()
    endforeach()

    # LINK_TO 
    # Unlike include dirs, libraries do not have
    # to be transitively propagated.
    # E.g. if A links to B and B links to C
    # we just need to explicitly link A to B and
    # the linker will make sure that C is eventually
    # linked to A

    if( DEFINED ${_dep}_LIBRARIES )
      # External library

      set( ${module}_LINK_TO 
        ${${module}_LINK_TO}
        ${${_dep}_LIBRARIES} )

    else()
      # Another CASA module

      set( ${module}_LINK_TO 
        ${${module}_LINK_TO} 
        ${_dep} )

    endif()

    # Generated code need build dir in include path
    if( ${_dep} STREQUAL "DBUS" OR
        ${_dep} STREQUAL "QT4" )
      
      set( ${module}_INCLUDE_DIRS 
        ${${module}_INCLUDE_DIRS}
        ${CMAKE_BINARY_DIR} )

    endif()

    # Special case for (old)alma
    if( ${module} STREQUAL "alma" OR
        ${module} STREQUAL "oldalma" )

      set( ${module}_INCLUDE_DIRS 
        ${${module}_INCLUDE_DIRS}
        ${CMAKE_SOURCE_DIR}/include/${module}/ASDM
        ${CMAKE_SOURCE_DIR}/include/${module}/ASDMBinaries
        ${CMAKE_SOURCE_DIR}/include/${module}/Enumtcl
        ${CMAKE_SOURCE_DIR}/include/${module}/Enumerations
        )

    endif()

  endforeach()

  #dump( ${module}_DEFINITIONS ${module}_INCLUDE_DIRS ${module}_LINK_TO )

  add_subdirectory( ${module} )

endmacro()



#
# casa_add_python( target_name
#                  installation_directory 
#                  source1 [source2 ...]
#                )
#
# Creates target for python files
#

MACRO( casa_add_python _target _install_dir )
  set( _pyexecs ${ARGN} )

  set( _out_all "" )

  FOREACH (_py ${_pyexecs} )

    GET_FILENAME_COMPONENT(_infile ${_py} ABSOLUTE)
    GET_FILENAME_COMPONENT(_outfile ${_py} NAME)

    # Retain paths for scripts in usecases/ and regressions/
    # strip path for everything else
    string( REGEX MATCH "usecases|regressions" _o ${_infile} )
    if( _o )
      set( _outfile ${CMAKE_CURRENT_BINARY_DIR}/${_py} )
    else()
      set( _outfile ${CMAKE_CURRENT_BINARY_DIR}/${_outfile} )      
    endif()

    GET_FILENAME_COMPONENT(_outpath ${_outfile} PATH)
    
    set( _out_all ${_out_all} ${_outfile} )

    ADD_CUSTOM_COMMAND( OUTPUT ${_outfile}
      COMMAND mkdir -p ${_outpath}
      COMMAND 
      sed -e 's|/CASASUBST/task_directory/|${PYTHON_TASKD}|' 
          -e 's|/CASASUBST/python_library_directory/|${PYTHON_LIBD}|'
          -e 's|/CASASUBST/build_time|${BUILDTIME}|'
          -e 's|/CASASUBST/subversion_revision|${SVNREVISION}|'
          -e 's|/CASASUBST/subversion_url|${SVNURL}|'
          -e 's|/CASASUBST/casa_version|${CASA_MAJOR_VERSION}.${CASA_MINOR_VERSION}.${CASA_PATCH_VERSION}|'
          -e 's|/CASASUBST/casa_build|${CASABUILD}|'
          ${_infile} > ${_outfile} 
      DEPENDS ${_infile} 
      COMMENT "Creating ${_outfile}"
      )

  endforeach()


  add_custom_target( 
    ${_target}
    ALL 
    DEPENDS ${_out_all} )
    #COMMENT "Creating python files")

  add_dependencies( inst ${_target} )

  install( PROGRAMS ${_out_all}
           DESTINATION ${_install_dir} )

endmacro()





#
#   casa_add_pymodule( name source1 [source2 ...] )
#
# Creates a python module. 
# The module is always linked to xmlcasa and ${xmlcasa_LINK_TO}.
#

macro( casa_add_pymodule name )

  set( _sources ${ARGN} )

  add_library( ${name} MODULE ${_sources} )
  add_dependencies( inst ${name} )

  set_target_properties( ${name} PROPERTIES PREFIX "" )
  target_link_libraries( ${name} xmlcasa ${xmlcasa_LINK_TO} )
  install( TARGETS ${name} LIBRARY DESTINATION python/${PYTHONV} )

endmacro()
