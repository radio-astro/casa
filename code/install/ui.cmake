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

    # Create intermediate file for the generation of tasks.py and tasksinfo.py
    set( _xsl ${CMAKE_SOURCE_DIR}/xmlcasa/install/casa2tsum2.xsl )
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

    COMMAND for x in ${_out_taskinfo} \; do grep -Ev "\"(^#?from|^\$\$)\"" $$x >> ${_tasksinfo} \; done
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
#   casa_ccmtools( INPUT input1 [input2 ...]
#                  OUTPUT output1 [output2 ...]
#                  OPTIONS option1 [option2 ...] 
#                  DEPENDS depend1 [depend2 ...] )
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
#    successful, it is checked that all output files exist after
#    running ccmtools
#  - implicit dependencies from the generated C++ to IDL files
#    (which include each other).
#    The option IMPLICIT_DEPENDS traces the dependencies
#    from the output C++ to indirect IDL files included by
#    the direct dependency. Note, in CMake 2.8 this works only
#    for the Makefile backend.

macro( casa_ccmtools )
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

    DEPENDS ${_conversions} ${_depends} ${CCMTOOLS_ccmtools_EXECUTABLE}

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
#        casa_add_tools( out_idl out_sources
#                        tool1.xml [tool2.xml ...]
#                      )
#
#  out_idl: generated IDL
#  out_sources: generated .cc and .h files
#

macro( casa_add_tools out_idl out_sources )

  set( _xmls ${ARGN} )

  foreach( _x ${_xmls} )
    
    get_filename_component( _base ${_x} NAME_WE )
    get_filename_component( _xml ${_x} ABSOLUTE )

    # Generate .xml from .xml
    set( _out_xml casa${_base}.xml )
    set( _xsl ${CMAKE_SOURCE_DIR}/xmlcasa/install/casa2toolxml.xsl )
    add_custom_command(
      OUTPUT ${_out_xml}
      COMMAND ${SAXON} ${_xml} ${_xsl} > ${_out_xml}_tmp1
      COMMAND sed -e \"s/exmlns/xmlns/\" ${_out_xml}_tmp1 > ${_out_xml}
      DEPENDS ${_xml} ${_xsl} 
      )

    # Then xml -> idl
    set( _idl ${CMAKE_CURRENT_BINARY_DIR}/casa${_base}.idl )
    set( _xsl ${CMAKE_SOURCE_DIR}/xmlcasa/install/casa2idl3.xsl )
    add_custom_command(
      OUTPUT ${_idl}
      COMMAND ${SAXON} ${_out_xml} ${_xsl} > ${_out_xml}_tmp2
      COMMAND sed -e \"s/<?xml version=.*//\" ${_out_xml}_tmp2 > ${_idl}
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

    casa_ccmtools(
      INPUT ${_idl}
      OUTPUT ${_outputs}
      OPTIONS c++local c++python
      DEPENDS ${_idl}
      )

    set( ${out_idl} ${${out_idl}} ${_idl} )

    set( ${out_sources} ${${out_sources}} ${_outputs} )

    install( FILES ${CMAKE_CURRENT_BINARY_DIR}/impl/${_base}_cmpt.h
             DESTINATION include/casa/xmlcasa/${_base} )

    install( FILES ${CMAKE_CURRENT_BINARY_DIR}/impl/${_base}_cmpt.h
             DESTINATION include/casa/impl )

    # Create tool documentation
    if ( NOT ${_base} STREQUAL plotms )    # because there is already a plotms task, and there would be a name conflict!
	casa_add_doc( ${_xml} ${CASA_DOC_DIR} tool )
    endif() 

  endforeach()
  
endmacro( casa_add_tools )


#
#  Rules for handling source IDL
#
#       casa_idl( outfiles input
#                 prefix1 [prefix2 ...] )
#
#  outfiles: list of generated C++ sources
#  input   : source IDL
#  prefix* : basename of output C++ sources
#

macro( casa_idl outfiles input )

    set( _types ${ARGN} )  # Created output files

    get_filename_component( _idl ${input} ABSOLUTE )

    set( _outputs "" )

    foreach( _t ${_types} )     
      list( APPEND _outputs
        Python_Converter/${_t}_python.h
        Python_Converter/${_t}_python.cc
        )

      if( NOT ${_t} MATCHES "(^shape_type|Vec)$" )
          # This condition matches which files ccmtools outputs

          list( APPEND _outputs
                impl/casac/${_t}.h )
          install( FILES ${CMAKE_CURRENT_BINARY_DIR}/impl/casac/${_t}.h
                   DESTINATION include/casa/impl/casac )
          install( FILES ${CMAKE_CURRENT_BINARY_DIR}/impl/casac/${_t}.h
                   DESTINATION include/casa/casac )
      endif()

      list( APPEND _outputs
            CCM_Local/casac/${_t}.h )
      install( FILES ${CMAKE_CURRENT_BINARY_DIR}/CCM_Local/casac/${_t}.h
               DESTINATION include/casa/CCM_Local/casac )

    endforeach()

    casa_ccmtools(
      INPUT ${_idl}
      OUTPUT ${_outputs}
      OPTIONS c++local c++python
      DEPENDS ${_idl}
      )
    
    set( ${outfiles} ${${outfiles}} ${_outputs} )
    
endmacro()


#
#  casa_pybinding( outfiles
#                  source1.idl [source2.idl] )
#
#  outfiles: generated casac_python.* C++ files
#  source* : input IDLs
#

macro( casa_pybinding outfiles )

  set( _idls ${ARGN} )

  # Avoid name clashing between the casac_python.*
  # created here and elsewhere (by other ccmtools invocations) by 
  # using a different output directory
  set( ${outfiles} 
    pybinding/Python_Converter/casac_python.cc
    pybinding/Python_Converter/casac_python.h
    pybinding/impl/casac/casac.h
    )

  casa_ccmtools(
    INPUT ${_idls}
    OUTPUT ${${outfiles}}
    OPTIONS c++local c++python
    --output=${CMAKE_CURRENT_BINARY_DIR}/pybinding
    )
    # You would think that here is missing a DEPENDS ${_idls},
    # but in fact the output does not depend on the content of the
    # input files, and therefore does *not* need to be regenerated 
    # whenever an input file changes (which takes much time). The
    # output only depends on the input filenames.

  install( FILES ${CMAKE_CURRENT_BINARY_DIR}/pybinding/impl/casac/casac.h
           DESTINATION include/casa/impl/casac )

endmacro()



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
    set( _xsl ${CMAKE_SOURCE_DIR}/xmlcasa/install/casa2latex.xsl )

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
