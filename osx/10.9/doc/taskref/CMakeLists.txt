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
set( _latex ${prefix}/TaskRef/${_base}.tex )
set( _latex ${_base}.latex )

#set( _inputs .:${prefix}/helpfiles/:${CMAKE_SOURCE_DIR}/doc/texinputs.dir/:${CMAKE_SOURCE_DIR}/doc/taskref/:${CMAKE_SOURCE_DIR}/doc/taskref/TaskRef.dir/:/usr/share/texmf/tex/{latex,generic,}//:/usr/share/texmf/web2c//:/usr/share/texmf/tex4ht//:/usr/share/texmf/tex/:$ENV{TEXINPUTS} )
if( APPLE )
   set( TEXROOT /Library/TeX/Root )
else()
   set( TEXROOT /usr/share )
endif()
set( _inputs .:${prefix}/helpfiles/:${CMAKE_SOURCE_DIR}/doc/texinputs.dir/:${CMAKE_SOURCE_DIR}/doc/taskref/:${CMAKE_SOURCE_DIR}/doc/taskref/TaskRef.dir/:${TEXROOT}/texmf/tex/{latex,generic,}//:${TEXROOT}/texmf/web2c//:${TEXROOT}/texmf/tex4ht//:${TEXROOT}/texmf/tex/:$ENV{TEXINPUTS} )
set( CASAGOOGLESEARCH 006707099318971825654:0jfwitmloig )


    # Create HTML
    if( LATEX2HTML_CONVERTER )

      set( _html ${prefix}/TaskRef/${_base}.html )

#set ( LATEX2HTML_CONVERTER echo )
#set( LATEX2HTML_OPTIONS htcasa,section+,next,p-width,5 )

      add_custom_command(
        OUTPUT ${_html}
	COMMAND sed -e "s/CASADOCUMENT/CASA Task Reference Manual/" -e "s/CASAGOOGLESEARCH/${CASAGOOGLESEARCH}/" ${casaroot}/code/doc/texinputs.dir/nraoheader.4ht > ${prefix}/nraoheader.4ht
	COMMAND sed -e "s/%\\\\NextFile/\\\\NextFile/" ${prefix}/helpfiles/simobserve.htex > /tmp/simobserve.htex
	COMMAND cp /tmp/simobserve.htex ${prefix}/helpfiles
	COMMAND sed -e "s/%\\\\NextFile/\\\\NextFile/" ${prefix}/helpfiles/simanalyze.htex > /tmp/simanalyze.htex
	COMMAND cp /tmp/simanalyze.htex ${prefix}/helpfiles
        COMMAND mkdir -p ${prefix}/TaskRef
        COMMAND cd ${prefix}/TaskRef && TEXINPUTS=${_inputs} ${LATEX2HTML_CONVERTER} ${_latex} ${LATEX2HTML_OPTIONS}
        COMMAND cd ${prefix}/TaskRef && TEXINPUTS=${_inputs} ${LATEX2HTML_CONVERTER} ${_latex} ${LATEX2HTML_OPTIONS}
        COMMAND cd ${prefix}/TaskRef && TEXINPUTS=${_inputs} ${LATEX2HTML_CONVERTER} ${_latex} ${LATEX2HTML_OPTIONS}
        DEPENDS ${_latex}
        VERBATIM )

      add_custom_target( taskref_html DEPENDS ${_html} )

      add_dependencies( taskref_html doc_latex xmlcasa_tasksref )

    else()
      add_custom_target( taskref_html COMMAND echo "LaTeX to HTML converter was not found." )
    endif()
      
    # Create PDF
    if( PDFLATEX_COMPILER )
      set( _pdf  ${prefix}/TaskRef/${_base}.pdf )
      add_custom_command(
        OUTPUT ${_pdf}
	COMMAND sed -e "s/\\\\NextFile/%\\\\NextFile/" ${prefix}/helpfiles/simobserve.htex > /tmp/simobserve.htex
	COMMAND cp /tmp/simobserve.htex ${prefix}/helpfiles
	COMMAND sed -e "s/\\\\NextFile/%\\\\NextFile/" ${prefix}/helpfiles/simanalyze.htex > /tmp/simanalyze.htex
	COMMAND cp /tmp/simanalyze.htex ${prefix}/helpfiles
        COMMAND mkdir -p ${prefix}/TaskRef
        COMMAND cd ${prefix}/TaskRef && TEXINPUTS=${_inputs} ${PDFLATEX_COMPILER} ${_latex}
        COMMAND cd ${prefix}/TaskRef && TEXINPUTS=${_inputs} ${PDFLATEX_COMPILER} ${_latex}
        COMMAND cd ${prefix}/TaskRef && TEXINPUTS=${_inputs} ${PDFLATEX_COMPILER} ${_latex}
        DEPENDS ${_latex}
        VERBATIM )

	add_custom_target( taskref_pdf DEPENDS ${_pdf} ) 
        add_dependencies( taskref_pdf doc_latex xmlcasa_tasksref )
   else()
      add_custom_target( taskref_pdf COMMAND echo "LaTeX to PDF converter was not found." )

   endif()
