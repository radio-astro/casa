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
# Macros related to CASA's user interface
#


#
#  Rule for handling task XML
#
#     casa_add_tasks( module
#                     target
#                     source1.xml [source2.xml ...] )
#
#  - Generates python bindings
#  - Generates tasks.py taskinfo.py
#  - Defines install rules
#

macro( casa_add_tasks module _target )
  set( _xmls ${ARGN} )

  set( _out_py "" )
  set( _out_taskinfo "" )
  set( _out_latex "" )
  set( _out_html "" )
  set( _out_pdf "" )
  set( _all_tasks "" )

  foreach( _x ${_xmls} )
    
    get_filename_component( _base ${_x} NAME_WE )
    get_filename_component( _xml ${_x} ABSOLUTE )

    list( APPEND _all_tasks ${_base} )

    set( _cli ${CMAKE_CURRENT_BINARY_DIR}/${_base}_cli.py )
    set( _pg  ${CMAKE_CURRENT_BINARY_DIR}/${_base}_pg.py )
    set( _py  ${CMAKE_CURRENT_BINARY_DIR}/${_base}.py )

    # Create _cli.py
    set( _xsl ${CMAKE_SOURCE_DIR}/install/casa2pycli.xsl )
    add_custom_command(
      OUTPUT ${_cli}
      COMMAND ${SAXON} -o ${_cli} ${_xml} ${_xsl} 
      DEPENDS ${_xml} ${_xsl} )

    # Create _pg.py
    set( _xsl ${CMAKE_SOURCE_DIR}/install/casa2pypg.xsl )
    add_custom_command(
      OUTPUT ${_pg}
      COMMAND ${SAXON} -o ${_pg} ${_xml} ${_xsl} 
      DEPENDS ${_xml} ${_xsl} )

    # Create .py
    set( _xsl ${CMAKE_SOURCE_DIR}/install/casa2pyimp.xsl )
    add_custom_command(
      OUTPUT ${_py}
      COMMAND ${SAXON} -o ${_py} ${_xml} ${_xsl}
      DEPENDS ${_xml} ${_xsl} )

    # Create intermediate file for the generation of tasks.py and tasksinfo.py
    set( _xsl ${CMAKE_SOURCE_DIR}/install/casa2tsum2.xsl )
    set( _out ${_base}_tasksinfo.py )
    add_custom_command( 
      OUTPUT ${_out}
      COMMAND ${SAXON} -o ${_out} ${_xml} ${_xsl}
      COMMAND echo >> ${_out}
      DEPENDS ${_xml} ${_xsl}
      )
    
    set( _out_taskinfo ${_out_taskinfo} ${_out} )

    # Keep track of generated files
    set( _out_py ${_out_py} ${_py} ${_cli} ${_pg} )

    # Create task documentation
    casa_add_doc( ${_xml} ${CASA_DOC_DIR} task )

  endforeach()

  set( _tasksref ${CASA_DOC_DIR}/helpfiles/tasksref.htex )
  add_custom_target(    
   xmlcasa_tasksref
   COMMAND mkdir -p ${CASA_DOC_DIR}/helpfiles
   COMMAND echo > ${_tasksref}
   COMMAND for x in ${_all_tasks} \; do echo "\\\\input{$$x.htex}" >> ${_tasksref} \; done
   DEPENDS ${_xml}
  )

  add_custom_target( 
    ${_target}
    ALL 
    DEPENDS ${_out_py} 
    )
  add_dependencies( inst ${_target} )
  add_custom_target( ${_target}_fast ${CMAKE_BUILD_TOOL} ${_target}/fast )
  add_dependencies( ${module}_fast ${_target}_fast )

  install( 
    FILES ${_xmls}
    DESTINATION ${casaroot}/share/xml
    )

  install( 
    PROGRAMS ${_out_py}
    DESTINATION python/${PYTHONV} 
    )

  # Create tasksinfo.py
  set( _tasksinfo ${CMAKE_CURRENT_BINARY_DIR}/tasksinfo.py )
  add_custom_command( 
    OUTPUT ${_tasksinfo}
    COMMAND echo "from odict import odict" > ${_tasksinfo}
    COMMAND echo "mytasks = odict\\(\\)" >> ${_tasksinfo}
    COMMAND echo "tasksum = odict\\(\\)" >> ${_tasksinfo}
    COMMAND echo "taskcat = odict\\(\\)" >> ${_tasksinfo}
    COMMAND echo "taskvis = odict\\(\\)" >> ${_tasksinfo}

    COMMAND for x in ${_out_taskinfo} \; do grep -Ev "\"(^#?from|^\$\$)\"" $$x >> ${_tasksinfo} \; done
    COMMAND cat ${CMAKE_SOURCE_DIR}/install/tasksinfo.pytxt >> ${_tasksinfo}
    DEPENDS ${_out_taskinfo}
    )

  # Create tasks.py
  set( _tasks ${CMAKE_CURRENT_BINARY_DIR}/tasks.py )
  add_custom_command( 
    OUTPUT ${_tasks}
    COMMAND echo > ${_tasks}
    COMMAND for x in ${_out_taskinfo} \; do grep -E "\"^from\"" $$x >> ${_tasks} \; done
    DEPENDS ${_out_taskinfo}
    COMMAND echo >> ${_tasks}
    COMMAND echo "from tget import *" >> ${_tasks}
    COMMAND echo "from taskmanager import tm" >> ${_tasks}
    DEPENDS ${_out_taskinfo}
    )

  add_custom_target( tasks ALL DEPENDS ${_tasks} ${_tasksinfo} )
  add_dependencies( inst tasks )
  add_custom_target( tasks_fast ${CMAKE_BUILD_TOOL} tasks/fast )
  add_dependencies( ${module}_fast tasks_fast )

  install(
    PROGRAMS ${_tasks} ${_tasksinfo} DESTINATION python/${PYTHONV}
    )
endmacro()

#
#  Rules for handling tool XML
#
#        casa_add_tools( out_swig out_sources
#                        tool1.xml [tool2.xml ...]
#                      )
#
#  out_sources: generated .cc and .h files
#

macro( casa_add_tools out_swig out_sources )

  INCLUDE(${SWIG_USE_FILE})
  message (status  ${SWIG_USE_FILE} )
  set( _xmls ${ARGN} )

  foreach( _x ${_xmls} )
    
    get_filename_component( _base ${_x} NAME_WE )
    get_filename_component( _xml ${_x} ABSOLUTE )
    get_filename_component( _path ${_x} PATH )

    # Generate .xml from .xml
    set( _out_xml casa${_base}.xml )
    set( _xsl ${CMAKE_SOURCE_DIR}/install/casa2swigxml.xsl )
    set( _swigh ${CMAKE_CURRENT_BINARY_DIR}/${_base}_cmpt.h )
    set( _xsl2 ${CMAKE_SOURCE_DIR}/install/casa2c++h.xsl )
    add_custom_command(
      OUTPUT ${_swigh}
      COMMAND ${SAXON} ${_xml} ${_xsl} > ${_out_xml}_tmp1
      COMMAND sed -e \"s/exmlns/xmlns/\" ${_out_xml}_tmp1 > ${_out_xml}
      COMMAND ${SAXON} ${_out_xml} ${_xsl2} > ${_swigh}_tmp2
      COMMAND sed -e \"s/<?xml version=.*//\" ${_swigh}_tmp2 > ${_swigh}
      DEPENDS ${_xml} ${_xsl} ${_xsl2}
      )

    # Then generate the swig interface file
    set( _swig ${CMAKE_CURRENT_BINARY_DIR}/${_base}_cmpt.i )
    add_custom_command(
      OUTPUT ${_swig}
      COMMAND echo "%module " ${_base} > ${_swig}
      COMMAND echo "%include \\<tools/casa_typemaps.i\\> " >> ${_swig}
      COMMAND echo "%include \\\"${_base}_cmpt.h\\\"" >> ${_swig}
      COMMAND echo "%{" >> ${_swig}
      COMMAND echo "\\#include \\<${_base}_cmpt.h\\> " >> ${_swig}
      COMMAND echo "%}" >> ${_swig}
      DEPENDS ${_swigh} ${_xsl} 
      )


    set( _outputs
	    ${_base}_cmpt.h
	    #${_base}PYTHON_wrap.c
	    )

    SET_SOURCE_FILES_PROPERTIES(${_swig} PROPERTIES CPLUSPLUS 1 )
    SET_SOURCE_FILES_PROPERTIES(${_swig} PROPERTIES SWIG_FLAGS "-I${CMAKE_SOURCE_DIR}") 
    SWIG_ADD_MODULE(${_base} python ${_swig} ${_path}/${_base}_cmpt.cc)
    SWIG_LINK_LIBRARIES( ${_base} ${CASACODE_LIBRARIES} ${PYTHON_LIBRARIES} ${ATM_LIBRARIES} ${CMAKE_CURRENT_BINARY_DIR}/libtools.dylib )

    set( ${out_swig} ${${out_swig}} ${_swig} )

    set( ${out_sources} ${${out_sources}} ${_outputs} )

    install( FILES ${CMAKE_CURRENT_BINARY_DIR}/${_base}_cmpt.h
             DESTINATION include/casa/tools/${_base} )

    # Create tool documentation
    if ( NOT ${_base} STREQUAL plotms )    # because there is already a plotms task, and there would be a name conflict!
	casa_add_doc( ${_xml} ${CASA_DOC_DIR} tool )
    endif() 

  endforeach()
  
endmacro( casa_add_tools )


#


# casa_add_doc( XML prefix type )
#
# - extracts PDF + HTML + LATEX documentation from the given XML
#   and adds the names of generated pdf/html/latex output to the cmake variables
#   casa_out_pdf, casa_out_html and casa_out_latex.
# type: "task" or "tool"
#
macro( casa_add_doc xml prefix type )

    # Create htex
    set( _htex  ${prefix}/helpfiles/${_base}.htex )
    set( _xsl ${CMAKE_SOURCE_DIR}/install/casa2tlatex.xsl )

    add_custom_command(
      OUTPUT ${_htex}
      COMMAND mkdir -p ${prefix}/helpfiles
      COMMAND ${SAXON} ${xml} ${_xsl} > ${_base}.htex_tmp1
      COMMAND sed -e "s/<?xml version.*//" ${_base}.htex_tmp1 > ${_base}.htex_tmp2
      COMMAND awk -f ${CMAKE_SOURCE_DIR}/install/docutils/xml2latex.awk ${_base}.htex_tmp2 >  ${_htex}
      DEPENDS ${xml} ${_xsl}
      VERBATIM
      )

    # Create latex
    set( _latex ${prefix}/htmlfiles/${_base}.latex )
    
    add_custom_command(
      OUTPUT ${_latex}
      COMMAND mkdir -p ${prefix}/htmlfiles
      COMMAND cat ${CMAKE_SOURCE_DIR}/install/docutils/tmpheader4tex > ${_latex}
      COMMAND cat ${_htex} >> ${_latex}
      COMMAND cat ${CMAKE_SOURCE_DIR}/install/docutils/tmptail4tex >> ${_latex}
      DEPENDS ${CMAKE_SOURCE_DIR}/install/docutils/tmpheader4tex
      ${_htex}                
      ${CMAKE_SOURCE_DIR}/install/docutils/tmptail4tex
      VERBATIM
      )

    if (${type} STREQUAL tool)
      add_custom_target( ${_base}_tool_latex DEPENDS ${_latex} )
    else()
      add_custom_target( ${_base}_task_latex DEPENDS ${_latex} )
    endif()
    
    set( casa_out_latex ${casa_out_latex} ${_latex} )

    # Create HTML
    if( LATEX2HTML_CONVERTER )

      set( _html ${prefix}/htmlfiles/${_base}/${_base}.html )

      add_custom_command(
        OUTPUT ${_html}
        COMMAND mkdir -p ${prefix}/htmlfiles
        COMMAND cd ${prefix}/htmlfiles && TEXINPUTS=.:${CMAKE_SOURCE_DIR}/doc/texinputs.dir//:$ENV{TEXINPUTS} ${LATEX2HTML_CONVERTER} ${_latex} ${LATEX2HTML_OPTIONS}
        COMMAND cd ${prefix}/htmlfiles && TEXINPUTS=.:${CMAKE_SOURCE_DIR}/doc/texinputs.dir//:$ENV{TEXINPUTS} ${LATEX2HTML_CONVERTER} ${_latex} ${LATEX2HTML_OPTIONS}
        COMMAND cd ${prefix}/htmlfiles && TEXINPUTS=.:${CMAKE_SOURCE_DIR}/doc/texinputs.dir//:$ENV{TEXINPUTS} ${LATEX2HTML_CONVERTER} ${_latex} ${LATEX2HTML_OPTIONS}
        DEPENDS ${_latex}
        VERBATIM )

      if (${type} STREQUAL tool)
        add_custom_target( ${_base}_tool_html DEPENDS ${_html} )
      else()
        add_custom_target( ${_base}_task_html DEPENDS ${_html} )
      endif()
      
      set( casa_out_html ${casa_out_html} ${_html} )
    endif()
      
    # Create PDF
    if( PDFLATEX_COMPILER )
      set( _pdf  ${prefix}/pdf_files/${_base}.pdf )
      add_custom_command(
        OUTPUT ${_pdf}
        COMMAND mkdir -p ${prefix}/pdf_files
        COMMAND cd ${prefix}/pdf_files && TEXINPUTS=.:${CMAKE_SOURCE_DIR}/doc/texinputs.dir//:$ENV{TEXINPUTS} ${PDFLATEX_COMPILER} ${_latex}
        COMMAND cd ${prefix}/pdf_files && TEXINPUTS=.:${CMAKE_SOURCE_DIR}/doc/texinputs.dir//:$ENV{TEXINPUTS} ${PDFLATEX_COMPILER} ${_latex}
        COMMAND cd ${prefix}/pdf_files && TEXINPUTS=.:${CMAKE_SOURCE_DIR}/doc/texinputs.dir//:$ENV{TEXINPUTS} ${PDFLATEX_COMPILER} ${_latex}
        DEPENDS ${_latex}
        VERBATIM )

      if (${type} STREQUAL tool)
	add_custom_target( ${_base}_tool_pdf DEPENDS ${_pdf} ) 
      else()
        add_custom_target( ${_base}_task_pdf DEPENDS ${_pdf} )
      endif()

      set( casa_out_pdf ${casa_out_pdf} ${_pdf} )

    endif()
endmacro()
