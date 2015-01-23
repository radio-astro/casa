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

#
# Various macros.
#


#
# Print the contents of one or more CMake variables
#
macro( dump )
  foreach( _a ${ARGN} )
    message( STATUS "${_a} = ${${_a}}" )
  endforeach()
endmacro()

#
# Append element to list, if that element does not already exist in the list
#
macro( casa_append l x )

  list( FIND ${l} ${x} _exists )
  if( _exists EQUAL -1 )
    set( ${l} ${${l}} ${x} ${ARGN} )
  endif()

endmacro()


#
# How to generate .h from .xml for DBUS
#

macro( casa_add_dbus_proxy _header _interface)
    get_filename_component(_infile ${_interface} ABSOLUTE)
    get_filename_component(_out_path ${_header} PATH)

    # The output directory needs to exist, 
    # or dbus-xml2cpp will silently fail
    add_custom_command( 
      OUTPUT ${_header}
      COMMAND mkdir -p ${_out_path}
      COMMAND ${dbus-xml-2-cxx} ${_infile} --proxy=${_header}
      DEPENDS ${_infile}
      )
endmacro()

macro( casa_add_dbus_adaptor _header _interface)
    get_filename_component(_infile ${_interface} ABSOLUTE)
    get_filename_component(_out_path ${_header} PATH)

    # The output directory needs to exist, 
    # or dbus-xml2cpp will silently fail
    add_custom_command( 
      OUTPUT ${_header}
      COMMAND mkdir -p ${_out_path}
      COMMAND ${dbus-xml-2-cxx} ${_infile} --adaptor=${_header}
      DEPENDS ${_infile}
      )
endmacro()

macro( casa_qt4_add_dbus_proxy qohfiles ccfiles _header _interface)
    get_filename_component(_out_path ${_header} PATH)
    get_filename_component(_out_name ${_header} NAME_WE)
    get_filename_component(_infile ../casadbus/xml/${_interface} ABSOLUTE)
    get_filename_component(_out_header ${CMAKE_CURRENT_BINARY_DIR}/${_header} ABSOLUTE)
    get_filename_component(_out_source ${CMAKE_CURRENT_BINARY_DIR}/${_out_path}/${_out_name}.cc ABSOLUTE)

    # The output directory needs to exist, 
    # or dbus-xml2cpp will silently fail
    add_custom_command( 
      OUTPUT ${_out_header} ${_out_source}
      COMMAND mkdir -p ${_out_path}
      COMMAND ${QT_DBUSXML2CPP_EXECUTABLE} -v -c ${_out_name} -i "casaqt/QtUtilities/QtDBusMeta.h" -p ${_out_header}:${_out_source} ${_infile}
      DEPENDS ${_infile}
    )
    SET(${qohfiles} ${${qohfiles}} ${_out_header})
    SET(${ccfiles} ${${ccfiles}} ${_out_source})
endmacro()


# Generate .ui.h files from .ui
# The same as CMAKE's built-in macro, but using a different naming convention
# for the generated file
#
MACRO (CASA_QT4_WRAP_UI outfiles )

# Later version of Qt4Macros.cmake added an output argument to
# qt4_extract_options.  This causes the first .ui file to be
# lost (and the corresponding .ui.h file to not be generated)
# if the old signature is used (jjacobs).  It's known that 2.8.5
# uses the old signature and 2.8.12.2 uses the new signature;
# the exact version where this cuts over could be somewhere 
# in between; it's hard to track down.

  if (CMAKE_VERSION VERSION_LESS 2.8.12)
    QT4_EXTRACT_OPTIONS(ui_files ui_options ${ARGN})
  else ()
    QT4_EXTRACT_OPTIONS(ui_files ui_options ui_target_ignored ${ARGN})
  endif (CMAKE_VERSION VERSION_LESS 2.8.12)

  FOREACH (it ${ui_files})
    GET_FILENAME_COMPONENT(outfile ${it} NAME_WE)
    GET_FILENAME_COMPONENT(infile ${it} ABSOLUTE)
    GET_FILENAME_COMPONENT(_path ${it} PATH)

    string(REGEX REPLACE "^implement/" "" _path ${_path})

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

# Later version of Qt4Macros.cmake added an output argument to
# qt4_extract_options.  This causes the first resource file to be
# lost if the old signature is used (jjacobs).  It's known that 2.8.5
# uses the old signature and 2.8.12.2 uses the new signature;
# the exact version where this cuts over could be somewhere 
# in between; it's hard to track down.  The symptom here is
# a link error complaiing about a missing symbol containing
# "resource" in its signature.

  if (CMAKE_VERSION VERSION_LESS 2.8.12)
    QT4_EXTRACT_OPTIONS(rcc_files rcc_options ${ARGN})
  else ()
    QT4_EXTRACT_OPTIONS(rcc_files rcc_options rcc_target ${ARGN})
  endif (CMAKE_VERSION VERSION_LESS 2.8.12)

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
