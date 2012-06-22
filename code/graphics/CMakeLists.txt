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
  GenericPlotter/PlotCanvas.cc
  GenericPlotter/PlotCanvasLayout.cc
  GenericPlotter/PlotData.cc
  GenericPlotter/PlotEvent.cc
  GenericPlotter/PlotFactory.cc
  GenericPlotter/PlotItem.cc
  GenericPlotter/PlotLogger.cc
  GenericPlotter/PlotOperation.cc
  GenericPlotter/PlotOptions.cc
  GenericPlotter/Plotter.cc
  GenericPlotter/PlotTool.cc
  GenericPlotter/SimplePlotter.cc
  Graphics/PGPlotterLocal.cc
  ${casa_graphics_pgplot_obj}
  ${casa_graphics_pgplot_src}
  )
install (FILES
	GenericPlotter/Plot.h
	GenericPlotter/PlotAnnotation.h
	GenericPlotter/PlotCanvas.h
	GenericPlotter/PlotCanvasLayout.h
	GenericPlotter/PlotData.h
	GenericPlotter/PlotEvent.h
	GenericPlotter/PlotEventHandler.h
	GenericPlotter/PlotFactory.h
	GenericPlotter/PlotItem.h
	GenericPlotter/PlotLogger.h
	GenericPlotter/PlotOperation.h
	GenericPlotter/PlotOptions.h
	GenericPlotter/PlotPanel.h
	GenericPlotter/PlotShape.h
	GenericPlotter/PlotTool.h
	GenericPlotter/Plotter.h
	GenericPlotter/SimplePlotter.h
	DESTINATION include/casacode/graphics/GenericPlotter
	)
install (FILES
	Graphics/PGPLOT.h
	Graphics/PGPlotterLocal.h
	Graphics/X11Intrinsic.h
	DESTINATION include/casacode/graphics/Graphics
	)
install (FILES
	X11/X11Util.h
	X11/X_enter.h
	X11/X_exit.h
	DESTINATION include/casacode/graphics/X11
	)
