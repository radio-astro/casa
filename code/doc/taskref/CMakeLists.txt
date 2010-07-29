#
# CASA - Common Astronomy Software Applications
# Copyright (C) 2010 by ESO (in the framework of the ALMA collaboration)
# Copyright (C) 2010 Associated Universities, Inc. Washington DC, USA.
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

set( _base TaskRef )
set( prefix  ${CASA_DOC_DIR} )
set( _latex ${prefix}/htmlfiles/${_base}.tex )
set( _latex ${_base}.latex )

set( _inputs .:${prefix}/helpfiles/:${CMAKE_SOURCE_DIR}/doc/texinputs.dir/:${CMAKE_SOURCE_DIR}/doc/taskref/:${CMAKE_SOURCE_DIR}/doc/taskref/TaskRef.dir/:/usr/share/texmf/tex/{latex,generic,}//:/usr/share/texmf/web2c//:/usr/share/texmf/tex4ht//:/usr/share/texmf/tex/:$ENV{TEXINPUTS} )


    # Create HTML
    if( LATEX2HTML_CONVERTER )

      set( _html ${prefix}/htmlfiles/${_base}.html )

#set ( LATEX2HTML_CONVERTER echo )
#set( LATEX2HTML_OPTIONS htcasa,section+,next,p-width,5 )

      add_custom_command(
        OUTPUT ${_html}
        COMMAND mkdir -p ${prefix}/htmlfiles
        COMMAND cd ${prefix}/htmlfiles && TEXINPUTS=${_inputs} ${LATEX2HTML_CONVERTER} ${_latex} ${LATEX2HTML_OPTIONS}
        COMMAND cd ${prefix}/htmlfiles && TEXINPUTS=${_inputs} ${LATEX2HTML_CONVERTER} ${_latex} ${LATEX2HTML_OPTIONS}
        COMMAND cd ${prefix}/htmlfiles && TEXINPUTS=${_inputs} ${LATEX2HTML_CONVERTER} ${_latex} ${LATEX2HTML_OPTIONS}
        DEPENDS ${_latex}
        VERBATIM )

      add_custom_target( taskref_html DEPENDS ${_html} )

      add_dependencies( taskref_html xmlcasa_tasksref doc_latex )

    else()
      add_custom_target( taskref_html COMMAND echo "LaTeX to HTML converter was not found." )
    endif()
      
    # Create PDF
    if( PDFLATEX_COMPILER )
      set( _pdf  ${prefix}/pdf_files/${_base}.pdf )
      add_custom_command(
        OUTPUT ${_pdf}
        COMMAND mkdir -p ${prefix}/pdf_files
        COMMAND cd ${prefix}/pdf_files && TEXINPUTS=${_inputs} ${PDFLATEX_COMPILER} ${_latex}
        COMMAND cd ${prefix}/pdf_files && TEXINPUTS=${_inputs} ${PDFLATEX_COMPILER} ${_latex}
        COMMAND cd ${prefix}/pdf_files && TEXINPUTS=${_inputs} ${PDFLATEX_COMPILER} ${_latex}
        DEPENDS ${_latex}
        VERBATIM )

	add_custom_target( taskref_pdf DEPENDS ${_pdf} ) 
        add_dependencies( taskref_pdf xmlcasa_tasksref doc_latex )
   else()
      add_custom_target( taskref_pdf COMMAND echo "LaTeX to PDF converter was not found." )

   endif()