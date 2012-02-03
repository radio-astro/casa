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
install (FILES
	implement/GenericPlotter/Plot.h
	implement/GenericPlotter/PlotAnnotation.h
	implement/GenericPlotter/PlotCanvas.h
	implement/GenericPlotter/PlotCanvasLayout.h
	implement/GenericPlotter/PlotData.h
	implement/GenericPlotter/PlotEvent.h
	implement/GenericPlotter/PlotEventHandler.h
	implement/GenericPlotter/PlotFactory.h
	implement/GenericPlotter/PlotItem.h
	implement/GenericPlotter/PlotLogger.h
	implement/GenericPlotter/PlotOperation.h
	implement/GenericPlotter/PlotOptions.h
	implement/GenericPlotter/PlotPanel.h
	implement/GenericPlotter/PlotShape.h
	implement/GenericPlotter/PlotTool.h
	implement/GenericPlotter/Plotter.h
	implement/GenericPlotter/SimplePlotter.h
	DESTINATION include/casacode/graphics/GenericPlotter
	)
install (FILES
	implement/Graphics/PGPLOT.h
	implement/Graphics/PGPlotterLocal.h
	implement/Graphics/X11Intrinsic.h
	DESTINATION include/casacode/graphics/Grpaphics
	)
install (FILES
	implement/X11/X11Util.h
	implement/X11/X_enter.h
	implement/X11/X_exit.h
	DESTINATION include/casacode/graphics/X11
	)
