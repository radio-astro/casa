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
  implement/QtBrowser/QCloseableWidget.qo.h
  implement/QtBrowser/QProgressPanel.qo.h
  implement/QtBrowser/TBBrowser.qo.h
  implement/QtBrowser/TBConnection.qo.h
  implement/QtBrowser/TBDataTab.qo.h
  implement/QtBrowser/TBFilterRules.qo.h
  implement/QtBrowser/TBFormat.qo.h
  implement/QtBrowser/TBHelp.qo.h
  implement/QtBrowser/TBKeywordsTab.qo.h
  implement/QtBrowser/TBMain.qo.h
  implement/QtBrowser/TBOptions.qo.h
  implement/QtBrowser/TBPlotCanvas.qo.h
  implement/QtBrowser/TBPlotter.qo.h
  implement/QtBrowser/TBRowLocate.qo.h
  implement/QtBrowser/TBSlicer.qo.h
  implement/QtBrowser/TBSorter.qo.h
  implement/QtBrowser/TBTableInfo.qo.h
  implement/QtBrowser/TBTableTabs.qo.h
  implement/QtBrowser/TBTaQL.qo.h
  implement/QtBrowser/TBTypes.qo.h
  implement/QtBrowser/TBValidityChecker.qo.h
  implement/QtBrowser/TBViewArray.qo.h
  implement/QtFileCatalog/dirmodel.qo.h
  implement/QtFileCatalog/filecatalog.qo.h
  implement/QtFileCatalog/pictureviewer.qo.h
  implement/QtFileCatalog/texteditor.qo.h
  implement/QtLogger/logmodel.qo.h
  implement/QtLogger/logviewer.qo.h
  implement/QtLogger/logwatcher.qo.h
  implement/QtParamGui/ParamPanel.qo.h
  implement/QtParamGui/QtParamGUI.qo.h
  implement/QtParamGui/ValuePanel.qo.h
  implement/QtPlotServer/QtDBusPlotSvrAdaptor.qo.h
  implement/QtPlotServer/QtPlotFrame.qo.h
  implement/QtPlotServer/QtPlotServer.qo.h
  implement/QtPlotServer/QtPlotSvrPanel.qo.h
  implement/QtProgressMeter/progress.qo.h
  implement/QtProgressMeter/progresstimer.qo.h
  implement/QtUtilities/QtActionGroup.qo.h
  implement/QtUtilities/QtButtonGroup.qo.h
  implement/QtUtilities/QtComboCheckBox.qo.h
  implement/QtUtilities/QtDBusXmlApp.qo.h
  implement/QtUtilities/QtEditingWidget.qo.h
  implement/QtUtilities/QtFileDialog.qo.h
  implement/QtUtilities/QtIndexChooser.qo.h
  implement/QtUtilities/QtPanelBase.qo.h
  implement/QtUtilities/QtPlotWidget.qo.h
  implement/QtUtilities/QtProgressWidget.qo.h
  implement/QwtPlotter/QPCanvasHelpers.qo.h
  implement/QwtPlotter/QPCanvas.qo.h
  implement/QwtPlotter/QPLayeredCanvas.qo.h
  implement/QwtPlotter/QPPanel.qo.h
  implement/QwtPlotter/QPPlotItem.qo.h
  implement/QwtPlotter/QPPlotter.qo.h
  implement/QwtPlotter/QPTool.qo.h
  )

casa_qt4_wrap_ui( outfiles_ui
  implement/QtBrowser/QCloseableWidget.ui
  implement/QtBrowser/QProgressPanel.ui
  implement/QtBrowser/TBArrayPanel.ui
  implement/QtBrowser/TBBrowser.ui
  implement/QtBrowser/TBConnection.ui
  implement/QtBrowser/TBDataTab.ui
  implement/QtBrowser/TBFieldKeywordsTab.ui
  implement/QtBrowser/TBFilterRules.ui
  implement/QtBrowser/TBFormat.ui
  implement/QtBrowser/TBHelp.ui
  implement/QtBrowser/TBMain.ui
  implement/QtBrowser/TBNewKeyword.ui
  implement/QtBrowser/TBOptions.ui
  implement/QtBrowser/TBPlotCanvas.ui
  implement/QtBrowser/TBPlotter.ui
  implement/QtBrowser/TBRowLocate.ui
  implement/QtBrowser/TBSlicer.ui
  implement/QtBrowser/TBSorter.ui
  implement/QtBrowser/TBTableInfo.ui
  implement/QtBrowser/TBTableKeywordsTab.ui
  implement/QtBrowser/TBTableTabs.ui
  implement/QtBrowser/TBTaQL.ui
  implement/QtBrowser/TBTypes.ui
  implement/QtBrowser/TBValidityChecker.ui
  implement/QtBrowser/TBViewArray.ui
  implement/QtParamGui/ParamPanel.ui
  implement/QtParamGui/QtParamGUI.ui
  implement/QtParamGui/ValuePanel.ui
  implement/QtUtilities/PlotColorWidget.ui
  implement/QtUtilities/PlotFileWidget.ui
  implement/QtUtilities/PlotFillWidget.ui
  implement/QtUtilities/PlotFontWidget.ui
  implement/QtUtilities/PlotLabelWidget.ui
  implement/QtUtilities/PlotLineWidget.ui
  implement/QtUtilities/PlotSymbolWidget.ui
  implement/QtUtilities/QtIndexChooser.ui
  implement/QtUtilities/QtProgressCompactWidget.ui
  implement/QtUtilities/QtProgressWidget.ui
  implement/QwtPlotter/QPPlotter.ui
  )

casa_qt4_add_resources( outfiles_qrc implement/QtBrowser/actions.qrc OPTIONS -name QtBrowser )
casa_qt4_add_resources( outfiles_qrc implement/QtFileCatalog/filecatalog.qrc OPTIONS -name QtFileCatalog )
casa_qt4_add_resources( outfiles_qrc implement/QtLogger/logviewer.qrc OPTIONS -name QtLogger )

casa_add_library( casaqt
  ${outfiles_qoh} ${outfiles_ui} ${outfiles_qrc}
  implement/PlotterImplementations/PlotterImplementations.cc
  implement/QtBrowser/QCloseableWidget.cc
  implement/QtBrowser/QProgressPanel.cc
  implement/QtBrowser/TBAction.cc
  implement/QtBrowser/TBArray.cc
  implement/QtBrowser/TBBrowser.cc
  implement/QtBrowser/TBConnection.cc
  implement/QtBrowser/TBConstants.cc
  implement/QtBrowser/TBData.cc
  implement/QtBrowser/TBDataTab.cc
  implement/QtBrowser/TBExportThread.cc
  implement/QtBrowser/TBField.cc
  implement/QtBrowser/TBFilterRules.cc
  implement/QtBrowser/TBFormat.cc
  implement/QtBrowser/TBHelp.cc
  implement/QtBrowser/TBKeyword.cc
  implement/QtBrowser/TBKeywordsTab.cc
  implement/QtBrowser/TBMain.cc
  implement/QtBrowser/TBOptions.cc
  implement/QtBrowser/TBParser.cc
  implement/QtBrowser/TBPlotCanvas.cc
  implement/QtBrowser/TBPlotter.cc
  implement/QtBrowser/TBRowLocate.cc
  implement/QtBrowser/TBSlicer.cc
  implement/QtBrowser/TBSorter.cc
  implement/QtBrowser/TBTable.cc
  implement/QtBrowser/TBTableDriver.cc
  implement/QtBrowser/TBTableInfo.cc
  implement/QtBrowser/TBTableTabs.cc
  implement/QtBrowser/TBTaQL.cc
  implement/QtBrowser/TBTest.cc
  implement/QtBrowser/TBTypes.cc
  implement/QtBrowser/TBValidityChecker.cc
  implement/QtBrowser/TBViewArray.cc
  implement/QtBrowser/TBView.cc
  implement/QtBrowser/TBXMLDriver.cc
  implement/QtBrowser/XMLtoken.cc
  implement/QtFileCatalog/diritem.cc
  implement/QtFileCatalog/dirmodel.cc
  implement/QtFileCatalog/filecatalog.cc
  implement/QtFileCatalog/pictureviewer.cc
  implement/QtFileCatalog/texteditor.cc
  implement/QtLogger/logitem.cc
  implement/QtLogger/logmodel.cc
  implement/QtLogger/logviewer.cc
  implement/QtParamGui/ParamPanel.cc
  implement/QtParamGui/PythonInterpreter.cc
  implement/QtParamGui/QtParamGUI.cc
  implement/QtParamGui/ValuePanel.cc
  implement/QtPlotServer/QtDBusPlotSvrAdaptor.cc
  implement/QtPlotServer/QtPlotFrame.cc
  implement/QtPlotServer/QtPlotServer.cc
  implement/QtPlotServer/QtPlotSvrPanel.cc
  implement/QtPlotServer/QtRasterData.cc
  implement/QtProgressMeter/progress.cc
  implement/QtProgressMeter/progresstimer.cc
  implement/QtUtilities/QtActionGroup.cc
  implement/QtUtilities/QtButtonGroup.cc
  implement/QtUtilities/QtComboCheckBox.cc
  implement/QtUtilities/QtDBusApp.cc
  implement/QtUtilities/QtDBusXmlApp.cc
  implement/QtUtilities/QtDBusXML.cc
  implement/QtUtilities/QtEditingWidget.cc
  implement/QtUtilities/QtFileDialog.cc
  implement/QtUtilities/QtIndexChooser.cc
  implement/QtUtilities/QtLayeredLayout.cc
  implement/QtUtilities/QtPanelBase.cc
  implement/QtUtilities/QtPlotWidget.cc
  implement/QtUtilities/QtProgressWidget.cc
  implement/QtUtilities/QtUtilities.cc
  implement/QwtPlotter/QPAnnotation.cc
  implement/QwtPlotter/QPBarPlot.cc
  implement/QwtPlotter/QPCanvas.cc
  implement/QwtPlotter/QPCanvasHelpers.cc
  implement/QwtPlotter/QPData.cc
  implement/QwtPlotter/QPFactory.cc
  implement/QwtPlotter/QPImageCache.cc
  implement/QwtPlotter/QPLayeredCanvas.cc
  implement/QwtPlotter/QPOptions.cc
  implement/QwtPlotter/QPPanel.cc
  implement/QwtPlotter/QPPlotItem.cc
  implement/QwtPlotter/QPPlotter.cc
  implement/QwtPlotter/QPRasterPlot.cc
  implement/QwtPlotter/QPScatterPlot.cc
  implement/QwtPlotter/QPShape.cc
  implement/QwtPlotter/QPTool.cc
  )

casa_add_executable( casaqt casafilecatalog apps/casafilecatalog/casafilecatalog.cc )
casa_add_executable( casaqt casalogger apps/casalogger/casalogger.cc )
casa_add_executable( casaqt casapictureviewer apps/casapictureviewer/casapictureviewer.cc )
casa_add_executable( casaqt casaplotserver apps/casaplotserver/casaplotserver.cc )
casa_add_executable( casaqt casaprogresstimer apps/casaprogresstimer/casaprogresstimer.cc )
casa_add_executable( casaqt qcasabrowser apps/qcasabrowser/qcasabrowser.cc )
casa_add_executable( casaqt qwtplottertest apps/qwtplottertest/qwtplottertest.cc )

install( PROGRAMS apps/qcasabrowser/casabrowser DESTINATION bin )
casa_add_assay( casaqt implement/QtProgressMeter/test/tProgressMeter.cc )
