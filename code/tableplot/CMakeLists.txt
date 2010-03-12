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

add_definitions( ${tableplot_DEFINITIONS} )
include_directories( ${tableplot_INCLUDE_DIRS} )

# Dynamic library
add_library( tableplot
  implement/TablePlot/BasePlot.cc
  implement/TablePlot/CasaPyInterpreter.cc
  implement/TablePlot/CrossPlot.cc
  implement/TablePlot/FlagVersion.cc
  implement/TablePlot/PanelParams.cc
  implement/TablePlot/PlotterGlobals.cc
  implement/TablePlot/SLog.cc
  implement/TablePlot/TablePlot.cc
  implement/TablePlot/TPPlotter.cc
 )

target_link_libraries( tableplot ${tableplot_LINK_TO} )

set_target_properties( tableplot PROPERTIES SOVERSION ${CASA_API_VERSION} )

install( TARGETS tableplot LIBRARY DESTINATION lib )

# Executables
# Unit tests
