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


# This target is executed unconditionally and before any other target that
# depends on the definitions file
add_custom_target( tools_definitions COMMAND ${CMAKE_COMMAND} -E remove -f definitions )


# This is delegated to the cmake script code/install/casadef.cmake in
# order to allow setting environment variables portably.
add_custom_command( OUTPUT definitions
	  COMMAND ${CMAKE_COMMAND} -Dcasadef_perl=${PERL_EXECUTABLE} -Dcasadef_source_dir=${CMAKE_SOURCE_DIR} -P ${CMAKE_SOURCE_DIR}/install/casadef.cmake > definitions_tmp
	    COMMAND ${CMAKE_COMMAND} -E copy definitions_tmp definitions
	    )
    # Using only shell redirection (>) to generate the definitions file, will cause
    # the file to be empty until the command has finished executing (which is a
    # feature of the shell), and thus trigger other make rules before the file is
    # complete.

add_custom_target(
    tools_python_casadef
    COMMAND echo "import os" > casadef.py
    COMMAND echo "if os.environ.has_key\\\(\\'__CASAPY_TASKDIR\\'\\\):" >> casadef.py
    COMMAND echo "    task_directory = os.environ[\\'__CASAPY_TASKDIR\\']" >> casadef.py
    COMMAND echo "else:" >> casadef.py
    COMMAND echo "    task_directory = \\\"${PYTHON_TASKD_CASADEF}\\\"" >> casadef.py
    COMMAND echo "if os.environ.has_key\\\(\\'__CASAPY_PYTHONDIR\\'\\\):" >> casadef.py
    COMMAND echo "    python_library_directory = os.environ[\\'__CASAPY_PYTHONDIR\\']" >> casadef.py
    COMMAND echo "else:" >> casadef.py
    COMMAND echo "    python_library_directory = \\\"${PYTHON_LIBD_CASADEF}\\\"" >> casadef.py
    COMMAND echo "casa_version = \\\"${CASA_MAJOR_VERSION}.${CASA_MINOR_VERSION}.${CASA_PATCH_VERSION}\\\"" >> casadef.py
    COMMAND cat definitions >> casadef.py
    DEPENDS definitions
    )

add_dependencies( tools_python_casadef tools_definitions )

add_dependencies( inst tools_python_casadef )
add_dependencies( tools_fast tools_python_casadef )
install( PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/casadef.py DESTINATION python/${PYTHONV} )

# Make sure that compile time definitions (build time, svn revision number)
# are recomputed in every build.

# set SVNREVISION and BUILDTIME
set( casadef_quiet TRUE )
set( casadef_perl ${PERL_EXECUTABLE} )
set( casadef_source_dir ${CMAKE_SOURCE_DIR} )
#include( ${CMAKE_SOURCE_DIR}/install/casadef.cmake )
#include( ${CMAKE_SOURCE_DIR}/install/ui.cmake )

casa_add_library( tools
 atmosphere/atmosphere_cmpt.cc
 calanalysis/calanalysis_cmpt.cc
 calibrater/calplot_cmpt.cc
 casa/logsink_cmpt.cc
 casa/quanta_cmpt.cc
 casa/TSLogSink.cc
 components/componentlist_cmpt.cc
 #flagging/flagger_cmpt.cc
 flagging/agentflagger_cmpt.cc
 images/coordsys_cmpt.cc
 images/image_cmpt.cc
 images/imagemetadata_cmpt.cc
 images/imagepol_cmpt.cc
 images/regionmanager_cmpt.cc
 measures/measures_cmpt.cc
 ms/ms_cmpt.cc
 ms/msmetadata_cmpt.cc
 ms/msplot_cmpt.cc
 ms/MSPlot/MsPlot.cc
 mstransformer/mstransformer_cmpt.cc
 nrao/vlafillertask_cmpt.cc
 plotms/plotms_cmpt.cc
 scimath/fitter_cmpt.cc
 scimath/functional_cmpt.cc
 StdCasa/conversions_python.cc
 StdCasa/interrupt.cc
 spectralline/spectralline_cmpt.cc
 synthesis/calibrater_cmpt.cc
 synthesis/deconvolver_cmpt.cc
 synthesis/imager_cmpt.cc
 synthesis/imagertask_cmpt.cc
 synthesis/CalTables/PlotCal.cc
 synthesis/simulator_cmpt.cc
 synthesis/synthesisimager_cmpt.cc
 synthesis/synthesisdeconvolver_cmpt.cc
 synthesis/synthesisparsync_cmpt.cc
 synthesis/synthesisimstore_cmpt.cc
 synthesis/synthesisiterbot_cmpt.cc
 synthesis/synthesisutils_cmpt.cc
 synthesis/vpmanager_cmpt.cc
 tables/table_cmpt.cc
 tables/tableindex_cmpt.cc
 tables/tableiterator_cmpt.cc
 tables/tableplot_cmpt.cc
 tables/tablerow_cmpt.cc
 tables/TablePlot/BasePlot.cc
 tables/TablePlot/CasaPyInterpreter.cc
 tables/TablePlot/CrossPlot.cc
 tables/TablePlot/PanelParams.cc
 tables/TablePlot/PlotterGlobals.cc
 tables/TablePlot/TablePlot.cc
 tables/TablePlot/TPPlotter.cc
 utils/BaseInterface.cc
 utils/CasapyWatcher.cc
 utils/dbus_cmpt.cc
 utils/stdBaseInterface.cc
 utils/utils_cmpt.cc
 xerces/asdmCasaSaxHandler.cc
 xerces/asdmCasaXMLUtil.cc
 xerces/stdcasaXMLUtil.cc
  )
add_dependencies( libtools_fast tools_definitions )
add_dependencies( libtools      tools_definitions )

#casa_add_assay( bindingxerces/test/tXMLToRecord.cc )

casa_add_tools( tools_swig tools_sources tools_py
 atmosphere/atmosphere.xml
 calanalysis/calanalysis.xml
 calibrater/calplot.xml
 casa/logsink.xml
 casa/quanta.xml
 components/componentlist.xml
 #flagging/flagger.xml
 flagging/agentflagger.xml
 images/coordsys.xml
 images/image.xml
 images/imagemetadata.xml
 images/imagepol.xml
 images/regionmanager.xml
 measures/measures.xml
 ms/msplot.xml
 ms/ms.xml
 ms/msmetadata.xml
 mstransformer/mstransformer.xml
 nrao/vlafillertask.xml
 plotms/plotms.xml
 scimath/fitter.xml
 scimath/functional.xml
 spectralline/spectralline.xml
 synthesis/calibrater.xml
 synthesis/deconvolver.xml
 synthesis/imagertask.xml
 synthesis/imager.xml
 synthesis/simulator.xml
 synthesis/synthesisimager.xml
 synthesis/synthesisdeconvolver.xml
 synthesis/synthesisparsync.xml
 synthesis/synthesisimstore.xml
 synthesis/synthesisiterbot.xml
 synthesis/synthesisutils.xml
 synthesis/vpmanager.xml
 tables/tableindex.xml
 tables/tableiterator.xml
 tables/tableplot.xml
 tables/tablerow.xml
 tables/table.xml
 utils/dbus.xml
 utils/utils.xml )

include_directories( ${CMAKE_CURRENT_BINARY_DIR} )
# for generated C++ bindings

add_definitions( -pthread -fno-strict-aliasing -DNDEBUG ) # may or may not be necessary for compiling python modules

add_custom_target( generated_sources SOURCES ${tools_sources} ${tools_swig} )
add_custom_target( generated_py SOURCES ${tools_py} )

foreach(_tool ${tools_swig} )
   get_filename_component( _base ${_tool} NAME_WE )
   add_dependencies( _${_base} libtools)
   add_dependencies( inst  _${_base} )
   install( TARGETS _${_base}
	            DESTINATION python/${PYTHONV}/__casac__ )
endforeach()

add_dependencies( inst  libtools generated_sources generated_py )
add_dependencies( libtools  generated_sources )
add_dependencies( libtools_fast  generated_sources )

foreach(_tool ${tools_py} )
   get_filename_component( _base ${_tool} NAME_WE )
   install( FILES ${CMAKE_CURRENT_BINARY_DIR}/${_base}.py
	               DESTINATION python/${PYTHONV}/__casac__ )
endforeach()

