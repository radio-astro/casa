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

###
### with RHEL6 we can switch back to building with flex/bison...
###
#find_package (BISON REQUIRED)
#find_package (FLEX REQUIRED)

#file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/ds9)
#INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})
#BISON_TARGET( ds9parse ds9/ds9parse.yy ${CMAKE_CURRENT_BINARY_DIR}/ds9parse.cc
#              COMPILE_FLAGS "--file-prefix=${CMAKE_CURRENT_BINARY_DIR}" )
#FLEX_TARGET(  ds9lex ds9/ds9lex.ll ${CMAKE_CURRENT_BINARY_DIR}/ds9lex.cc
#              COMPILE_FLAGS "-+" )
#ADD_FLEX_BISON_DEPENDENCY(ds9lex ds9parse)


include( ${QT_USE_FILE} )
set( QT4_LIBRARIES ${QT_LIBRARIES} )

qt4_wrap_cpp( outfiles_qoh
  Feather/PlotHolder.qo.h
  Feather/FeatherMain.qo.h
  Feather/FeatherManager.qo.h
  Feather/FileLoader.qo.h
  Feather/FeatherPlotWidget.qo.h
  Feather/PreferencesFunction.qo.h
  Feather/Preferences.qo.h
  Feather/PreferencesColor.qo.h
  Feather/OverWriteFileDialog.qo.h
  Feather/AboutDialog.qo.h
  Histogram/BinPlotWidget.qo.h
  Histogram/ImageLoader.qo.h
  Histogram/ColorPreferences.qo.h
  Histogram/HistogramMain.qo.h
  Histogram/FitWidget.qo.h
  Histogram/RangeControlsWidget.qo.h
  Histogram/SaveHistogramWidget.qo.h
  Histogram/BinCountWidget.qo.h
  Histogram/ChannelRangeWidget.qo.h
  Histogram/ZoomWidget.qo.h
  Histogram/FootPrintWidget.qo.h
  )

casa_qt4_wrap_ui( outfiles_ui
  Feather/PlotHolder.ui
  Feather/FeatherMain.ui
  Feather/FileLoader.ui
  Feather/FeatherPlotWidget.ui
  Feather/PreferencesFunction.ui
  Feather/Preferences.ui
  Feather/PreferencesColor.ui
  Feather/OverWriteFileDialog.ui
  Feather/AboutDialog.ui
  Histogram/BinPlotWidget.ui
  Histogram/ImageLoader.ui
  Histogram/ColorPreferences.ui
  Histogram/HistogramMain.ui
  Histogram/FitWidget.ui
  Histogram/RangeControlsWidget.ui
  Histogram/SaveHistogramWidget.ui
  Histogram/BinCountWidget.ui
  Histogram/ChannelRangeWidget.ui
  Histogram/ZoomWidget.ui
  Histogram/FootPrintWidget.ui
  )

casa_qt4_add_resources( outfiles_qrc Feather/Feather.qrc OPTIONS -name Feather )
casa_qt4_add_resources( outfiles_qrc Histogram/Histogram.qrc OPTIONS -name Histogram )
#include_directories( ${CMAKE_CURRENT_BINARY_DIR}/guitools ) # for generated code

casa_add_library( guitools
  ${outfiles_qoh} ${outfiles_ui} ${outfiles_qrc}
  Feather/PlotHolder.cc
  Feather/FeatherMain.cc
  Feather/FeatherPlotWidget.cc
  Feather/FeatherPlotWidgetSlice.cc
  Feather/FeatherPlotWidgetScatter.cc
  Feather/FileLoader.cc
  Feather/LegendCurve.cc
  Feather/LegendItemRect.cc
  Feather/Preferences.cc
  Feather/PreferencesFunction.cc
  Feather/PreferencesColor.cc
  Feather/FeatherPlot.cc
  Feather/FeatherCurve.cc
  Feather/CurveDisplay.cc
  Feather/ExternalAxis.cc
  Feather/ExternalAxisWidget.cc
  Feather/ExternalAxisWidgetLeft.cc
  Feather/ExternalAxisWidgetRight.cc
  Feather/ExternalAxisWidgetBottom.cc
  Feather/FeatherManager.cc
  Feather/FeatheredData.cc
  Feather/FeatherThread.cc
  Feather/OverWriteFileDialog.cc
  Feather/AboutDialog.cc
  Histogram/ColorPreferences.cc
  Histogram/HistogramMain.cc
  Histogram/BinPlotWidget.cc
  Histogram/ImageLoader.cc
  Histogram/FitWidget.cc
  Histogram/HistogramMarkerGaussian.cc
  Histogram/HistogramMarkerPoisson.cc
  Histogram/Fitter.cc
  Histogram/FitterGaussian.cc
  Histogram/FitterPoisson.cc
  Histogram/RangeControlsWidget.cc
  Histogram/RangePicker.cc
  Histogram/SaveHistogramWidget.cc
  Histogram/ToolTipPicker.cc
  Histogram/ColorDelegate.cc
  Histogram/Histogram.cc
  Histogram/BinCountWidget.cc
  Histogram/ChannelRangeWidget.cc
  Histogram/ZoomWidget.cc
  Histogram/FootPrintWidget.cc
  )
	
casa_add_executable( guitools casafeather apps/casafeather/casafeather.cc )
casa_add_executable( guitools casahistogram apps/casahistogram/casahistogram.cc)


