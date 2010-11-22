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

set( _base CasaRef )
set( prefix  ${CASA_DOC_DIR} )
set( _latex ${prefix}/CasaRef/${_base}.tex )
set( _latex ${_base}.latex )

set( _inputs .:${prefix}/helpfiles/:${CMAKE_SOURCE_DIR}/doc/texinputs.dir/:${CMAKE_SOURCE_DIR}/doc/casaref/:${CMAKE_SOURCE_DIR}/doc/casaref/CasaRef.dir/:/usr/share/texmf/tex/{latex,generic,}//:/usr/share/texmf/web2c//:/usr/share/texmf/tex4ht//:/usr/share/texmf/tex/:$ENV{TEXINPUTS} )
set( CASAGOOGLESEARCH 006707099318971825654:vqush9mvpes )


    # Create HTML
    if( LATEX2HTML_CONVERTER )

      set( _html ${prefix}/CasaRef/${_base}.html )

#set ( LATEX2HTML_CONVERTER echo )
#set( LATEX2HTML_OPTIONS htcasa,section+,next,p-width,5 )

#execute_process( COMMAND sed -e "s/CASADOCUMENT/CASA Toolkit Reference Manual/" -e "s/CASAGOOGLESEARCH/${CASAGOOGLESEARCH}/" ${casaroot}/code/doc/texinputs.dir/nraoheader.4ht
#OUTPUT_FILE  ${prefix}/nraoheader.4ht
#)
      add_custom_command(
        OUTPUT ${_html}
	COMMAND sed -e "s/CASADOCUMENT/CASA Toolkit Reference Manual/" -e "s/CASAGOOGLESEARCH/${CASAGOOGLESEARCH}/" ${casaroot}/code/doc/texinputs.dir/nraoheader.4ht > ${prefix}/nraoheader.4ht
        COMMAND mkdir -p ${prefix}/CasaRef
        COMMAND cd ${prefix}/CasaRef && TEXINPUTS=${_inputs} ${LATEX2HTML_CONVERTER} ${_latex} ${LATEX2HTML_OPTIONS}
        COMMAND cd ${prefix}/CasaRef && TEXINPUTS=${_inputs} ${LATEX2HTML_CONVERTER} ${_latex} ${LATEX2HTML_OPTIONS}
        COMMAND cd ${prefix}/CasaRef && TEXINPUTS=${_inputs} ${LATEX2HTML_CONVERTER} ${_latex} ${LATEX2HTML_OPTIONS}
        DEPENDS ${_latex}
        VERBATIM )

      add_custom_target( casaref_html DEPENDS ${_html} )

      add_dependencies( casaref_html doc_latex )

    else()
      add_custom_target( casaref_html COMMAND echo "LaTeX to HTML converter was not found." )
      
    endif()
      
    # Create PDF
    if( PDFLATEX_COMPILER )
      set( _pdf  ${prefix}/CasaRef/${_base}.pdf )
      add_custom_command(
        OUTPUT ${_pdf}
        COMMAND mkdir -p ${prefix}/CasaRef
        COMMAND cd ${prefix}/CasaRef && TEXINPUTS=${_inputs} ${PDFLATEX_COMPILER} ${_latex}
        COMMAND cd ${prefix}/CasaRef && TEXINPUTS=${_inputs} ${PDFLATEX_COMPILER} ${_latex}
        COMMAND cd ${prefix}/CasaRef && TEXINPUTS=${_inputs} ${PDFLATEX_COMPILER} ${_latex}
        DEPENDS ${_latex}
        VERBATIM )

	add_custom_target( casaref_pdf DEPENDS ${_pdf} ) 
        add_dependencies( casaref_pdf doc_latex )
   else()
      add_custom_target( casaref_pdf COMMAND echo "LaTeX to PDF converter was not found." )

   endif()
