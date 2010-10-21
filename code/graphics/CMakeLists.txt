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

if( EXISTS ${CMAKE_SOURCE_DIR}/graphics/pgplot/CMakeLists.txt )
    include( ${CMAKE_SOURCE_DIR}/graphics/pgplot/CMakeLists.txt )
endif()

casa_add_library( graphics
  implement/GenericPlotter/PlotCanvas.cc
  implement/GenericPlotter/PlotCanvasLayout.cc
  implement/GenericPlotter/PlotData.cc
  implement/GenericPlotter/PlotEvent.cc
  implement/GenericPlotter/PlotFactory.cc
  implement/GenericPlotter/PlotItem.cc
  implement/GenericPlotter/PlotLogger.cc
  implement/GenericPlotter/PlotOperation.cc
  implement/GenericPlotter/PlotOptions.cc
  implement/GenericPlotter/Plotter.cc
  implement/GenericPlotter/PlotTool.cc
  implement/GenericPlotter/SimplePlotter.cc
  implement/Graphics/PGPlotterLocal.cc
  ${casa_graphics_pgplot_obj}
  ${casa_graphics_pgplot_src}
  )
