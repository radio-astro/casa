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

include( ${QT_USE_FILE} )
set( QT4_LIBRARIES ${QT_LIBRARIES} )

qt4_wrap_cpp( outfiles_qoh
  implement/Actions/PlotMSCacheThread.qo.h
  implement/Actions/PlotMSDrawThread.qo.h
  implement/Actions/PlotMSExportThread.qo.h
  implement/Actions/PlotMSThread.qo.h
  implement/Gui/PlotMSAveragingWidget.qo.h
  implement/Gui/PlotMSAxisWidget.qo.h
  implement/Gui/PlotMSFlaggingWidget.qo.h
  implement/Gui/PlotMSLoggerWidget.qo.h
  implement/Gui/PlotMSPlotter.qo.h
  implement/Gui/PlotMSSelectionWidget.qo.h
  implement/Gui/PlotMSTransformationsWidget.qo.h
  implement/Gui/PlotRangeWidget.qo.h
  implement/GuiTabs/PlotMSAnnotatorTab.qo.h
  implement/GuiTabs/PlotMSAxesTab.qo.h
  implement/GuiTabs/PlotMSCacheTab.qo.h
  implement/GuiTabs/PlotMSCanvasTab.qo.h
  implement/GuiTabs/PlotMSDataTab.qo.h
  implement/GuiTabs/PlotMSDisplayTab.qo.h
  implement/GuiTabs/PlotMSExportTab.qo.h
  implement/GuiTabs/PlotMSFlaggingTab.qo.h
  implement/GuiTabs/PlotMSMultiAxesTab.qo.h
  implement/GuiTabs/PlotMSOptionsTab.qo.h
  implement/GuiTabs/PlotMSPlotTab.qo.h
  implement/GuiTabs/PlotMSTab.qo.h
  implement/GuiTabs/PlotMSToolsTab.qo.h
  implement/GuiTabs/PlotMSTransformationsTab.qo.h
  )

casa_qt4_wrap_ui( outfiles_ui
  implement/Gui/PlotMSAveragingWidget.ui
  implement/Gui/PlotMSAxisWidget.ui
  implement/Gui/PlotMSFlaggingWidget.ui
  implement/Gui/PlotMSPlotter.ui
  implement/Gui/PlotMSTransformationsWidget.ui
  implement/Gui/PlotRangeWidget.ui
  implement/GuiTabs/PlotMSAnnotatorTab.ui
  implement/GuiTabs/PlotMSAxesTab.ui
  implement/GuiTabs/PlotMSCacheTab.ui
  implement/GuiTabs/PlotMSCanvasTab.ui
  implement/GuiTabs/PlotMSDataTab.ui
  implement/GuiTabs/PlotMSDisplayTab.ui
  implement/GuiTabs/PlotMSExportTab.ui
  implement/GuiTabs/PlotMSFlaggingTab.ui
  implement/GuiTabs/PlotMSMultiAxesTab.ui
  implement/GuiTabs/PlotMSOptionsTab.ui
  implement/GuiTabs/PlotMSPlotTab.ui
  implement/GuiTabs/PlotMSToolsTab.ui
  implement/GuiTabs/PlotMSTransformationsTab.ui
  )

casa_qt4_add_resources( outfiles_qrc implement/Gui/PlotMSPlotter.qrc OPTIONS -name Gui )

include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/implement/Gui ) 
# because the generated code contain relative paths to the source tree

casa_add_library( plotms
  ${outfiles_qoh} ${outfiles_ui} ${outfiles_qrc}
  implement/Actions/PlotMSAction.cc
  implement/Actions/PlotMSCacheThread.cc
  implement/Actions/PlotMSDrawThread.cc
  implement/Actions/PlotMSExportThread.cc
  implement/Actions/PlotMSThread.cc
  implement/Data/PlotMSCache.cc
  implement/Data/PlotMSCacheIndexer.cc
  implement/Data/PlotMSData.cc
  implement/Data/PlotMSVBAverager.cc
  implement/Gui/PlotMSAnnotator.cc
  implement/Gui/PlotMSAveragingWidget.cc
  implement/Gui/PlotMSAxisWidget.cc
  implement/Gui/PlotMSFlaggingWidget.cc
  implement/Gui/PlotMSLoggerWidget.cc
  implement/Gui/PlotMSPlotter.cc
  implement/Gui/PlotMSSelectionWidget.cc
  implement/Gui/PlotMSTransformationsWidget.cc
  implement/Gui/PlotRangeWidget.cc
  implement/GuiTabs/PlotMSAnnotatorTab.cc
  implement/GuiTabs/PlotMSAxesTab.cc
  implement/GuiTabs/PlotMSCacheTab.cc
  implement/GuiTabs/PlotMSCanvasTab.cc
  implement/GuiTabs/PlotMSDataTab.cc
  implement/GuiTabs/PlotMSDisplayTab.cc
  implement/GuiTabs/PlotMSExportTab.cc
  implement/GuiTabs/PlotMSFlaggingTab.cc
  implement/GuiTabs/PlotMSMultiAxesTab.cc
  implement/GuiTabs/PlotMSOptionsTab.cc
  implement/GuiTabs/PlotMSPlotTab.cc
  implement/GuiTabs/PlotMSTab.cc
  implement/GuiTabs/PlotMSToolsTab.cc
  implement/GuiTabs/PlotMSTransformationsTab.cc
  implement/PlotMS/PlotMSAveraging.cc
  implement/PlotMS/PlotMS.cc
  implement/PlotMS/PlotMSConstants.cc
  implement/PlotMS/PlotMSDBusApp.cc
  implement/PlotMS/PlotMSFlagging.cc
  implement/PlotMS/PlotMSLabelFormat.cc
  implement/PlotMS/PlotMSParameters.cc
  implement/PlotMS/PlotMSRegions.cc
  implement/PlotMS/PlotMSSelection.cc
  implement/PlotMS/PlotMSTransformations.cc
  implement/PlotMS/PlotMSWatchedParameters.cc
  implement/Plots/PlotMSMultiPlot.cc
  implement/Plots/PlotMSPage.cc
  implement/Plots/PlotMSPlot.cc
  implement/Plots/PlotMSPlotManager.cc
  implement/Plots/PlotMSPlotParameterGroups.cc
  implement/Plots/PlotMSPlotParameters.cc
  implement/Plots/PlotMSSinglePlot.cc
  )

casa_add_executable( plotms casaplotms apps/casaplotms/casaplotms.cc )
