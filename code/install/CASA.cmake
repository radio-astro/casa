
macro( dump )
  foreach( _a ${ARGN} )
    message( STATUS "${_a} = ${${_a}}" )
  endforeach()
endmacro()

#
# Setup definitions and include directories for this module,
# and create a dynamic library
#
#    casa_add_library( module source1 [source2 ...] )
#

macro( casa_add_library mod )

  add_definitions( ${${mod}_DEFINITIONS} )

  include_directories( ${${mod}_INCLUDE_DIRS} )

  add_library( ${mod} ${ARGN} )

  target_link_libraries( ${mod} ${${mod}_LINK_TO} )

  set_target_properties( ${mod} PROPERTIES SOVERSION ${CASA_API_VERSION} )

  install( TARGETS ${mod} LIBRARY DESTINATION lib )

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
      message( FATAL_ERROR "${_dep} is listed as dependency for ${module}, but both ${_dep}_INCLUDE_DIRS and ${_dep}_LIBRARIES are undefined!" )
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

  #dump( ${module}_DEFINITIONS )
  #message("Set ${module}_INCLUDE_DIRS to ${${module}_INCLUDE_DIRS}" )
  #message("Set ${module}_DEFINITIONS to ${${module}_DEFINITIONS}" )
  #message("Set ${module}_LINK_TO to ${${module}_LINK_TO}" )

  add_subdirectory( ${module} )

endmacro()



#
# casa_add_python( target_name installation_directory )
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
          -e 's|/CASASUBST/casa_version|${CASAVERSION}|'
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

  install( PROGRAMS ${_out_all}
           DESTINATION ${_install_dir} )

endmacro()

#
#  Rules for handling task XML
#

macro( casa_add_tasks _target )
  set( _xmls ${ARGN} )

  set( _out_all "" )

  foreach( _x ${_xmls} )
    
    get_filename_component( _base ${_x} NAME_WE )
    get_filename_component( _xml ${_x} ABSOLUTE )

    set( _cli ${CMAKE_CURRENT_BINARY_DIR}/${_base}_cli.py )
    set( _pg  ${CMAKE_CURRENT_BINARY_DIR}/${_base}_pg.py )
    set( _py  ${CMAKE_CURRENT_BINARY_DIR}/${_base}.py )

    # Create _cli.py
    set( _xsl ${CMAKE_SOURCE_DIR}/xmlcasa/install/casa2pycli.xsl )
    add_custom_command(
      OUTPUT ${_cli}
      COMMAND ${SAXON} -o ${_cli} ${_xml} ${_xsl} 
      DEPENDS ${_xml} ${_xsl} )

    # Create _pg.py
    set( _xsl ${CMAKE_SOURCE_DIR}/xmlcasa/install/casa2pypg.xsl )
    add_custom_command(
      OUTPUT ${_pg}
      COMMAND ${SAXON} -o ${_pg} ${_xml} ${_xsl} 
      DEPENDS ${_xml} ${_xsl} )

    # Create .py
    set( _xsl ${CMAKE_SOURCE_DIR}/xmlcasa/install/casa2pyimp.xsl )
    add_custom_command(
      OUTPUT ${_py}
      COMMAND ${SAXON} -o ${_py} ${_xml} ${_xsl}
      DEPENDS ${_xml} ${_xsl} )

    # Keep track of generated files
    set( _out_all ${_out_all} ${_py} ${_cli} ${_pg} )

  endforeach()

  add_custom_target( 
    ${_target}
    ALL 
    DEPENDS ${_out_all} )

  install( 
    FILES ${_xmls}
    DESTINATION ../share/xml
    )

  install( 
    PROGRAMS ${_out_all}
    DESTINATION python/${PYTHONV} 
    )


  # Create tasks.py, tasksinfo.py
  # This is a lousy implementation, which
  # runs saxon on every .xml file, if any of them changed.
  set( _tasks     ${CMAKE_CURRENT_BINARY_DIR}/tasks.py )
  set( _tasksinfo ${CMAKE_CURRENT_BINARY_DIR}/tasksinfo.py )
  set( _xsl ${CMAKE_SOURCE_DIR}/xmlcasa/install/casa2tsum2.xsl )
  add_custom_command( 
    OUTPUT ${_tasks} ${_tasksinfo}
    COMMAND echo > taskinfo
    COMMAND for task in ${CMAKE_CURRENT_SOURCE_DIR}/tasks/*.xml \; do echo \$\$task \;  ${SAXON} \$\$task ${_xsl} >> taskinfo \;  done
    COMMAND echo >> taskinfo
    COMMAND echo "from tget import *" >> taskinfo
    COMMAND echo "from taskmanager import tm" >> taskinfo
    COMMAND grep "^from" taskinfo > ${_tasks}
    COMMAND echo "from odict import odict" > ${_tasksinfo}
    COMMAND echo "mytasks = odict\\(\\)" >> ${_tasksinfo}
    COMMAND echo "tasksum = odict\\(\\)" >> ${_tasksinfo}
    COMMAND grep -Ev "^#?from" taskinfo >> ${_tasksinfo}
    DEPENDS ${_xmls}
    )

  add_custom_target( tasks ALL DEPENDS ${_tasks} ${_tasksinfo} )

  install(
    PROGRAMS ${_tasks} ${_tasksinfo} DESTINATION python/${PYTHONV}
    )
endmacro()


#
#   casa_add_ccmtools( INPUT input1 [input2 ...]
#                      OUTPUT output1 [output2 ...]
#                      OPTIONS option1 [option2 ...] 
#                      DEPENDS depend1 [depend2 ...] )
#
#  There are a few issues involved with invoking ccmtools.
#  The solution to these are factored out in this macro.
#
#  - ccmtools sometimes skips writing an output file. 
#    This has the consequence that if the file was considered
#    out of date, it will still be considered out of date.
#    This is resolved by removing all expected output files
#    before invoking ccmtools
#  - ccmtools always returns zero, even in case of error. In
#    order for the build system to check if the invocation was
#    successful it is checked that all output files exist after
#    running ccmtools
#  - implicit dependencies to IDL files.
#    The option IMPLICIT_DEPENDS traces the dependencies
#    from the output C++ to indirect IDL files included by
#    the direct dependency. Note, in CMake 2.8 this works only
#    for the Makefile backend.

macro( casa_add_ccmtools )
  # Careful!: variable names have global scope.
  # The names must not conflict with 'temporary' 
  # variables used in the caller macro. This is
  # the responsibility of the calling code.

  # Parse options
  set( _opt false )
  set( _in false )
  set( _out false )
  set( _dep false )
  set( _input "" )
  set( _output "" )
  set( _options "" )
  set( _depends "" )
  foreach( _arg ${ARGN} )
    if( _arg STREQUAL OPTIONS)
      set( _opt true )
      set( _in false )
      set( _out false )
      set( _dep false )
    elseif( _arg STREQUAL INPUT)
      set( _opt false )
      set( _in true )
      set( _out false )
      set( _dep false )
    elseif( _arg STREQUAL OUTPUT)
      set( _opt false )
      set( _in false )
      set( _out true )
      set( _dep false )
    elseif( _arg STREQUAL DEPENDS)
      set( _opt false )
      set( _in false )
      set( _out false )
      set( _dep true )
    elseif( _opt )
      set( _options ${_options} ${_arg} )
    elseif( _in )
      set( _input ${_input} ${_arg} )
    elseif( _out )
      set( _output ${_output} ${_arg} )
    elseif( _dep )
      set( _depends ${_depends} ${_arg} )
    else(
        message( FATAL_ERROR "Illegal options: ${ARGN}" )
        )
    endif()
  endforeach()
  # Done parsing

  #message(" OPTIONS = ${_options} ")
  #message(" INPUT = ${_input} ")
  #message(" OUTPUT = ${_output} ")

  set( _conversions ${CMAKE_SOURCE_DIR}/xmlcasa/xml/conversions.xml )

  add_custom_command(
    OUTPUT ${_output}

    COMMAND rm -f ${_output}  # otherwise CCMTOOLS sometimes skips writing

    COMMAND ${CCMTOOLS_ccmtools_EXECUTABLE} ${CCMTOOLS_ccmtools_FLAGS} ${_options}
    --overwrite 
    --coda=${_conversions}
    -I${CMAKE_SOURCE_DIR}/xmlcasa/idl
    -I${CMAKE_CURRENT_BINARY_DIR}
    ${_input}

    # Now check if the outputs were created and return an error if not
    COMMAND ${PERL_EXECUTABLE} -le 'for (@ARGV) { ( -e ) or die \"$$_ missing!\"\; }' --
    ARGS ${_output}

    DEPENDS ${_conversions} ${_depends}

    # Indirect dependencies to included IDL files
    IMPLICIT_DEPENDS C ${_depends}  # Use the C preprocessor because
                                    # that is what ccmtools uses
    )
  
  if( NOT CMAKE_GENERATOR STREQUAL "Unix Makefiles")
    message( WARNING "Dependency tracking of generated IDL files may not work when CMAKE_GENERATOR is ${CMAKE_GENERATOR}" )
  endif()

endmacro()


#
#  Rules for handling tool XML
#
#  out_idl: generated IDL
#  out_sources: generated .cc and .h files
#

macro( casa_tool out_idl out_sources )
  set( _xmls ${ARGN} )


  foreach( _x ${_xmls} )
    
    get_filename_component( _base ${_x} NAME_WE )
    get_filename_component( _xml ${_x} ABSOLUTE )

    # Generate .xml from .xml
    set( _out_xml casa${_base}.xml )
    set( _xsl ${CMAKE_SOURCE_DIR}/xmlcasa/install/casa2toolxml.xsl )
    add_custom_command(
      OUTPUT ${_out_xml}
      COMMAND ${SAXON} ${_xml} ${_xsl} | sed -e \"s/exmlns/xmlns/\" > ${_out_xml}
      DEPENDS ${_xml} ${_xsl} 
      )

    # Then xml -> idl
    set( _idl ${CMAKE_CURRENT_BINARY_DIR}/casa${_base}.idl )
    set( _xsl ${CMAKE_SOURCE_DIR}/xmlcasa/install/casa2idl3.xsl )
    add_custom_command(
      OUTPUT ${_idl}
      COMMAND ${SAXON} ${_out_xml} ${_xsl} | sed -e \"s/<?xml version=.*//\" > ${_idl}
      DEPENDS ${_out_xml} ${_xsl} 
      )

    # CCMTools create C++ bindings from IDL
    set( _outputs 
      Python_Converter/casa${_base}_python.cc
      Python_Converter/CCM_Local/casac/CCM_Session_${_base}/${_base}Home_python.cc
      Python_Converter/CCM_Local/casac/CCM_Session_${_base}/${_base}_python.cc
      Python_Converter/CCM_Local/casac/CCM_Session_${_base}/${_base}_python.h
      Python_Converter/CCM_Local/casac/CCM_Session_${_base}/${_base}Home_python.h
      CCM_Local/casac/CCM_Session_${_base}/${_base}_gen.cc
      CCM_Local/casac/CCM_Session_${_base}/${_base}Home_gen.cc
      CCM_Local/casac/CCM_Session_${_base}/${_base}Home_share.h
      CCM_Local/casac/CCM_Session_${_base}/${_base}_gen.h
      CCM_Local/casac/CCM_Session_${_base}/${_base}Home_gen.h
      CCM_Local/casac/CCM_Session_${_base}/${_base}_share.h
      impl/${_base}_impl.cc
      impl/${_base}Home_impl.cc
      impl/${_base}_impl.h
      impl/${_base}Home_impl.h
      impl/${_base}_cmpt.h
      impl/${_base}Home_entry.h
      )

    casa_add_ccmtools(
      INPUT ${_idl}
      OUTPUT ${_outputs}
      OPTIONS c++local c++python
      DEPENDS ${_idl}
      )

    set( ${out_idl} ${${out_idl}} ${_idl} )

    set( ${out_sources} ${${out_sources}} ${_outputs} )

  endforeach()
  
endmacro()


#
#  Rules for handling source IDL
#
#  outfiles: list of generated C++ sources
#  input   : source IDL
#  *       : expected output files
#

macro( CASA_idl outfiles input )

    set( _types ${ARGN} )  # Created output files

    get_filename_component( _idl ${input} ABSOLUTE )

    set( _outputs "" )

    foreach( _t ${_types} )     
      set( _outputs ${_outputs}
        Python_Converter/${_t}_python.h
        Python_Converter/${_t}_python.cc
        )

    endforeach()
     
    casa_add_ccmtools(
      INPUT ${_idl}
      OUTPUT ${_outputs}
      OPTIONS c++local c++python
      DEPENDS ${_idl}
      )
    
    set( ${outfiles} ${${outfiles}} ${_outputs} )
    
endmacro( CASA_idl )


macro( casa_binding outfiles )

  set( _idls ${ARGN} )

  # Avoid name clashing between the casac_python.*
  # created here and elsewhere by 
  # putting them in a subdirectory
  set( ${outfiles} 
    binding/Python_Converter/casac_python.cc
    binding/Python_Converter/casac_python.h
    )

  casa_add_ccmtools(
    INPUT ${_idls}
    OUTPUT ${${outfiles}}
    OPTIONS c++local c++python
    --output=${CMAKE_CURRENT_BINARY_DIR}/binding
    )
    # You would think that here is missing a
    # DEPENDS ${_idl}, but in fact the output does not
    # depend on the content of the input files, and therefore
    # do *not* need to be regenerated whenever an input file
    # changes (which takes much time). The output only depends
    # on the input filenames.
    #
    # on the other hand, running this command without idl existing is also bad...

endmacro( casa_binding )


#
# Define a macro to define how to generate .h from .xml DBUS
#

MACRO(CASA_ADD_DBUS_INTERFACE _header _interface)
    GET_FILENAME_COMPONENT(_infile ${_interface} ABSOLUTE)
    GET_FILENAME_COMPONENT(_out_path ${_header} PATH)

    # The output directory needs to exist, 
    # or dbus-xml2cpp will silently fail
    ADD_CUSTOM_COMMAND(OUTPUT ${_header}
        COMMAND mkdir -p ${_out_path}
        COMMAND ${DBUS_dbusxx-xml2cpp_EXECUTABLE} ${_infile} --proxy=${_header}
        DEPENDS ${_infile})

ENDMACRO(CASA_ADD_DBUS_INTERFACE)


# Generate .ui.h files from .ui
# The builtin CMAKE macro uses a different naming convention
#
MACRO (CASA_QT4_WRAP_UI outfiles )
  QT4_EXTRACT_OPTIONS(ui_files ui_options ${ARGN})

  FOREACH (it ${ui_files})
    GET_FILENAME_COMPONENT(outfile ${it} NAME_WE)
    GET_FILENAME_COMPONENT(infile ${it} ABSOLUTE)
    GET_FILENAME_COMPONENT(_path ${it} PATH)

    string(REGEX REPLACE "^implement/" "" _path ${_path})

#    SET(outfile ${CMAKE_CURRENT_BINARY_DIR}/${outfile}.ui.h)
    SET(outfile ${CMAKE_CURRENT_BINARY_DIR}/${_path}/${outfile}.ui.h)

    GET_FILENAME_COMPONENT(_path ${outfile} PATH)

    ADD_CUSTOM_COMMAND(OUTPUT ${outfile}
      COMMAND mkdir -p ${_path}
      COMMAND ${QT_UIC_EXECUTABLE}
      ARGS ${ui_options} -o ${outfile} ${infile}
      MAIN_DEPENDENCY ${infile})
    SET(${outfiles} ${${outfiles}} ${outfile})
  ENDFOREACH (it)
ENDMACRO (CASA_QT4_WRAP_UI)




#
# Create .qrc.cc from .qrc
# Same as CMake's built-in, except
# the "-name" option to rcc is parametrized
#
MACRO (CASA_QT4_ADD_RESOURCES outfiles )
  QT4_EXTRACT_OPTIONS(rcc_files rcc_options ${ARGN})
  
  FOREACH (it ${rcc_files})
    GET_FILENAME_COMPONENT(outfilename ${it} NAME_WE)
    GET_FILENAME_COMPONENT(infile ${it} ABSOLUTE)
    GET_FILENAME_COMPONENT(rc_path ${infile} PATH)
    SET(outfile ${CMAKE_CURRENT_BINARY_DIR}/qrc_${outfilename}.cxx)
    #  parse file for dependencies
    #  all files are absolute paths or relative to the location of the qrc file
    FILE(READ "${infile}" _RC_FILE_CONTENTS)
    STRING(REGEX MATCHALL "<file[^<]+" _RC_FILES "${_RC_FILE_CONTENTS}")
    SET(_RC_DEPENDS)
    FOREACH(_RC_FILE ${_RC_FILES})
      STRING(REGEX REPLACE "^<file[^>]*>" "" _RC_FILE "${_RC_FILE}")
      STRING(REGEX MATCH "^/|([A-Za-z]:/)" _ABS_PATH_INDICATOR "${_RC_FILE}")
      IF(NOT _ABS_PATH_INDICATOR)
        SET(_RC_FILE "${rc_path}/${_RC_FILE}")
      ENDIF(NOT _ABS_PATH_INDICATOR)
      SET(_RC_DEPENDS ${_RC_DEPENDS} "${_RC_FILE}")
    ENDFOREACH(_RC_FILE)
    ADD_CUSTOM_COMMAND(OUTPUT ${outfile}
      COMMAND ${QT_RCC_EXECUTABLE}
      ARGS ${rcc_options} -o ${outfile} ${infile}
      MAIN_DEPENDENCY ${infile}
      DEPENDS ${_RC_DEPENDS})
    SET(${outfiles} ${${outfiles}} ${outfile})
  ENDFOREACH (it)
  
ENDMACRO (CASA_QT4_ADD_RESOURCES)





#
#  casa_check_version( error p actual desired )
#
#        error : output error message, or empty if ok
#            p : package name
#       actual : given version number
#      desired : nominal version number
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
        ${casa_packages}/include
	${CMAKE_INSTALL_PREFIX}/include
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

      set( ${package}_INCLUDE_DIR ${${package}_INCLUDE_DIR} ${${package}_${_include}} )
      
    endforeach()

    if( _found )
      # Add dependencies to include dirs
      set( ${package}_INCLUDE_DIRS ${${package}_INCLUDE_DIR} CACHE PATH "${package} include directories" FORCE )
      
      foreach( _d ${_depends} )
        set( ${package}_INCLUDE_DIRS ${${package}_INCLUDE_DIRS} ${${_d}_INCLUDE_DIRS} CACHE PATH "${package} include directories" FORCE )
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
        ${casa_packages}/lib
	${CMAKE_INSTALL_PREFIX}/lib
        /usr/local/lib
        /usr/lib
      )

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

        set( ${package}_LIBRARY ${${package}_LIBRARY} ${${package}_${_lib}} )

      endforeach()

    endif()

    if( _found )
      # Add dependencies to libs
      set( ${package}_LIBRARIES ${${package}_LIBRARY} CACHE FILEPATH "${package} libraries" FORCE )
      foreach( _d ${_depends} )
        set( ${package}_LIBRARIES ${${package}_LIBRARIES} ${${_d}_LIBRARIES} CACHE FILEPATH "${package} libraries" FORCE )
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
    
    #dump( ${package}_FOUND     ${package}_INCLUDE_DIRS    ${package}_LIBRARIES    ${package}_DEFINITIONS )
    #foreach( _p ${_programs} )
    #  dump( ${package}_${_p}_EXECUTABLE )
    #endforeach()

  endif() # end if not package found...
    
endmacro()
# casa_find
