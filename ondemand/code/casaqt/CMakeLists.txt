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
  QtBrowser/QCloseableWidget.qo.h
  QtBrowser/QProgressPanel.qo.h
  QtBrowser/TBBrowser.qo.h
  QtBrowser/TBConnection.qo.h
  QtBrowser/TBDataTab.qo.h
  QtBrowser/TBFilterRules.qo.h
  QtBrowser/TBFormat.qo.h
  QtBrowser/TBHelp.qo.h
  QtBrowser/TBKeywordsTab.qo.h
  QtBrowser/TBMain.qo.h
  QtBrowser/TBOptions.qo.h
  QtBrowser/TBPlotCanvas.qo.h
  QtBrowser/TBPlotter.qo.h
  QtBrowser/TBRowLocate.qo.h
  QtBrowser/TBSlicer.qo.h
  QtBrowser/TBSorter.qo.h
  QtBrowser/TBTableInfo.qo.h
  QtBrowser/TBTableTabs.qo.h
  QtBrowser/TBTaQL.qo.h
  QtBrowser/TBTypes.qo.h
  QtBrowser/TBValidityChecker.qo.h
  QtBrowser/TBViewArray.qo.h
  QtFileCatalog/dirmodel.qo.h
  QtFileCatalog/filecatalog.qo.h
  QtFileCatalog/pictureviewer.qo.h
  QtFileCatalog/texteditor.qo.h
  QtLogger/logmodel.qo.h
  QtLogger/logviewer.qo.h
  QtLogger/logwatcher.qo.h
  QtPlotServer/QtDBusPlotSvrAdaptor.qo.h
  QtPlotServer/QtPlotFrame.qo.h
  QtPlotServer/QtPlotServer.qo.h
  QtPlotServer/QtPlotSvrPanel.qo.h
  QtProgressMeter/progress.qo.h
  QtProgressMeter/progresstimer.qo.h
  QtUtilities/QtActionGroup.qo.h
  QtUtilities/QtButtonGroup.qo.h
  QtUtilities/QtComboCheckBox.qo.h
  QtUtilities/QtDBusXmlApp.qo.h
  QtUtilities/QtEditingWidget.qo.h
  QtUtilities/QtFileDialog.qo.h
  QtUtilities/QtIndexChooser.qo.h
  QtUtilities/QtPanelBase.qo.h
  QtUtilities/QtPlotWidget.qo.h
  QtUtilities/QtProgressWidget.qo.h
  QwtPlotter/QPAxis.qo.h
  QwtPlotter/QPCanvasHelpers.qo.h
  QwtPlotter/QPCanvas.qo.h
  QwtPlotter/QPLayeredCanvas.qo.h
  QwtPlotter/QPPanel.qo.h
  QwtPlotter/QPPlotItem.qo.h
  QwtPlotter/QPPlotter.qo.h
  QwtPlotter/QPTool.qo.h
  )

casa_qt4_wrap_ui( outfiles_ui
  " "
  QtBrowser/QCloseableWidget.ui
  QtBrowser/QProgressPanel.ui
  QtBrowser/TBArrayPanel.ui
  QtBrowser/TBBrowser.ui
  QtBrowser/TBConnection.ui
  QtBrowser/TBDataTab.ui
  QtBrowser/TBFieldKeywordsTab.ui
  QtBrowser/TBFilterRules.ui
  QtBrowser/TBFormat.ui
  QtBrowser/TBHelp.ui
  QtBrowser/TBMain.ui
  QtBrowser/TBNewKeyword.ui
  QtBrowser/TBOptions.ui
  QtBrowser/TBPlotCanvas.ui
  QtBrowser/TBPlotter.ui
  QtBrowser/TBRowLocate.ui
  QtBrowser/TBSlicer.ui
  QtBrowser/TBSorter.ui
  QtBrowser/TBTableInfo.ui
  QtBrowser/TBTableKeywordsTab.ui
  QtBrowser/TBTableTabs.ui
  QtBrowser/TBTaQL.ui
  QtBrowser/TBTypes.ui
  QtBrowser/TBValidityChecker.ui
  QtBrowser/TBViewArray.ui
  QtUtilities/PlotColorWidget.ui
  QtUtilities/PlotFileWidget.ui
  QtUtilities/PlotFillWidget.ui
  QtUtilities/PlotFontWidget.ui
  QtUtilities/PlotLabelWidget.ui
  QtUtilities/PlotLineWidget.ui
  QtUtilities/PlotSymbolWidget.ui
  QtUtilities/QtIndexChooser.ui
  QtUtilities/QtProgressCompactWidget.ui
  QtUtilities/QtProgressWidget.ui
  QwtPlotter/QPPlotter.ui
  )

casa_qt4_add_resources( outfiles_qrc " " QtBrowser/actions.qrc OPTIONS -name QtBrowser )
casa_qt4_add_resources( outfiles_qrc " " QtFileCatalog/filecatalog.qrc OPTIONS -name QtFileCatalog )
casa_qt4_add_resources( outfiles_qrc " " QtLogger/logviewer.qrc OPTIONS -name QtLogger )

casa_add_library( casaqt
  ${outfiles_qoh} ${outfiles_ui} ${outfiles_qrc}
  PlotterImplementations/PlotterImplementations.cc
  QtBrowser/QCloseableWidget.cc
  QtBrowser/QProgressPanel.cc
  QtBrowser/TBAction.cc
  QtBrowser/TBArray.cc
  QtBrowser/TBBrowser.cc
  QtBrowser/TBConnection.cc
  QtBrowser/TBConstants.cc
  QtBrowser/TBData.cc
  QtBrowser/TBDataTab.cc
  QtBrowser/TBExportThread.cc
  QtBrowser/TBField.cc
  QtBrowser/TBFilterRules.cc
  QtBrowser/TBFormat.cc
  QtBrowser/TBHelp.cc
  QtBrowser/TBKeyword.cc
  QtBrowser/TBKeywordsTab.cc
  QtBrowser/TBMain.cc
  QtBrowser/TBOptions.cc
  QtBrowser/TBParser.cc
  QtBrowser/TBPlotCanvas.cc
  QtBrowser/TBPlotter.cc
  QtBrowser/TBRowLocate.cc
  QtBrowser/TBSlicer.cc
  QtBrowser/TBSorter.cc
  QtBrowser/TBTable.cc
  QtBrowser/TBTableDriver.cc
  QtBrowser/TBTableInfo.cc
  QtBrowser/TBTableTabs.cc
  QtBrowser/TBTaQL.cc
  QtBrowser/TBTest.cc
  QtBrowser/TBTypes.cc
  QtBrowser/TBValidityChecker.cc
  QtBrowser/TBViewArray.cc
  QtBrowser/TBView.cc
  QtBrowser/TBXMLDriver.cc
  QtBrowser/XMLtoken.cc
  QtFileCatalog/diritem.cc
  QtFileCatalog/dirmodel.cc
  QtFileCatalog/filecatalog.cc
  QtFileCatalog/pictureviewer.cc
  QtFileCatalog/texteditor.cc
  QtLogger/logitem.cc
  QtLogger/logmodel.cc
  QtLogger/logviewer.cc
  QtPlotServer/QtDBusPlotSvrAdaptor.cc
  QtPlotServer/QtPlotFrame.cc
  QtPlotServer/QtPlotServer.cc
  QtPlotServer/QtPlotHistogram.cc
  QtPlotServer/QtPlotSvrPanel.cc
  QtPlotServer/QtRasterData.cc
  QtProgressMeter/progress.cc
  QtProgressMeter/progresstimer.cc
  QtUtilities/QtActionGroup.cc
  QtUtilities/QtButtonGroup.cc
  QtUtilities/QtComboCheckBox.cc
  QtUtilities/QtId.cc
  QtUtilities/QtDBusApp.cc
  QtUtilities/QtDBusXmlApp.cc
  QtUtilities/QtDBusXML.cc
  QtUtilities/QtEditingWidget.cc
  QtUtilities/QtFileDialog.cc
  QtUtilities/QtIndexChooser.cc
  QtUtilities/QtLayeredLayout.cc
  QtUtilities/QtPanelBase.cc
  QtUtilities/QtPlotWidget.cc
  QtUtilities/QtProgressWidget.cc
  QtUtilities/QtUtilities.cc
  QtUtilities/Axis/ExternalAxisWidget.cc
  QtUtilities/Axis/ExternalAxisWidgetLeft.cc
  QtUtilities/Axis/ExternalAxisWidgetRight.cc
  QtUtilities/Axis/ExternalAxisWidgetTop.cc
  QtUtilities/Axis/ExternalAxisWidgetBottom.cc
  QwtPlotter/QPAnnotation.cc
  QwtPlotter/QPAxis.cc
  QwtPlotter/QPBarPlot.cc
  QwtPlotter/QPCanvas.cc
  QwtPlotter/QPCanvasHelpers.cc
  QwtPlotter/QPExporter.cc
  QwtPlotter/QPData.cc
  QwtPlotter/QPFactory.cc
  QwtPlotter/QPImageCache.cc
  QwtPlotter/QPLayeredCanvas.cc
  QwtPlotter/QPOptions.cc
  QwtPlotter/QPPanel.cc
  QwtPlotter/QPPlotItem.cc
  QwtPlotter/QPPlotter.cc
  QwtPlotter/QPRasterPlot.cc
  QwtPlotter/QPScatterPlot.cc
  QwtPlotter/QPShape.cc
  QwtPlotter/QPTool.cc
  )
install (FILES
	PlotterImplementations/PlotterImplementations.h
	DESTINATION include/casacode/casaqt/QwtPlotter
	)
install (FILES
	QtBrowser/QCloseableWidget.qo.h
	QtBrowser/QProgressPanel.qo.h
	QtBrowser/TBAction.h
	QtBrowser/TBArray.h
	QtBrowser/TBBrowser.qo.h
	QtBrowser/TBConnection.qo.h
	QtBrowser/TBConstants.h
	QtBrowser/TBData.h
	QtBrowser/TBDataTab.qo.h
	QtBrowser/TBExportThread.h
	QtBrowser/TBField.h
	QtBrowser/TBFilterRules.qo.h
	QtBrowser/TBFormat.qo.h
	QtBrowser/TBHelp.qo.h
	QtBrowser/TBKeyword.h
	QtBrowser/TBKeywordsTab.qo.h
	QtBrowser/TBMain.qo.h
	QtBrowser/TBOptions.qo.h
	QtBrowser/TBParser.h
	QtBrowser/TBPlotCanvas.qo.h
	QtBrowser/TBPlotter.qo.h
	QtBrowser/TBRowLocate.qo.h
	QtBrowser/TBSlicer.qo.h
	QtBrowser/TBSorter.qo.h
	QtBrowser/TBTaQL.qo.h
	QtBrowser/TBTable.h
	QtBrowser/TBTableDriver.h
	QtBrowser/TBTableInfo.qo.h
	QtBrowser/TBTableTabs.qo.h
	QtBrowser/TBTest.h
	QtBrowser/TBTypes.qo.h
	QtBrowser/TBValidityChecker.qo.h
	QtBrowser/TBView.h
	QtBrowser/TBViewArray.qo.h
	QtBrowser/TBXMLDriver.h
	QtBrowser/XMLtoken.h
	DESTINATION include/casacode/casaqt/QtBrowser
	)
install (FILES
	QtFileCatalog/diritem.h
	QtFileCatalog/dirmodel.qo.h
	QtFileCatalog/filecatalog.qo.h
	QtFileCatalog/pictureviewer.qo.h
	QtFileCatalog/texteditor.qo.h
	DESTINATION include/casacode/casaqt/QtFileCatalog
	)
install (FILES
	QtLogger/logitem.h
	QtLogger/logmodel.qo.h
	QtLogger/logviewer.qo.h
	QtLogger/logwatcher.qo.h
	DESTINATION include/casacode/casaqt/QtLogger
	)
install (FILES
	QtPlotServer/QtDBusPlotSvrAdaptor.qo.h
	QtPlotServer/QtPlotFrame.qo.h
	QtPlotServer/QtPlotHistogram.h
	QtPlotServer/QtPlotServer.qo.h
	QtPlotServer/QtPlotSvrPanel.qo.h
	QtPlotServer/QtRasterData.h
	DESTINATION include/casacode/casaqt/QtPlotServer
	)
install (FILES
	QtProgressMeter/progress.qo.h
	QtProgressMeter/progresstimer.qo.h
	DESTINATION include/casacode/casaqt/QtProgressMeter
	)
install (FILES
	QtUtilities/QtActionGroup.qo.h
	QtUtilities/QtButtonGroup.qo.h
	QtUtilities/QtComboCheckBox.qo.h
	QtUtilities/QtDBusApp.h
	QtUtilities/QtDBusXML.h
	QtUtilities/QtDBusXmlApp.qo.h
	QtUtilities/QtEditingWidget.qo.h
	QtUtilities/QtFileDialog.qo.h
	QtUtilities/QtId.h
	QtUtilities/QtIndexChooser.qo.h
	QtUtilities/QtLayeredLayout.h
	QtUtilities/QtPanelBase.qo.h
	QtUtilities/QtPlotWidget.qo.h
	QtUtilities/QtProgressWidget.qo.h
	QtUtilities/QtUtilities.h
	QtUtilities/Axis/ExternalAxisWidget.h
	QtUtilities/Axis/ExternalAxisWidgetLeft.h
	QtUtilities/Axis/ExternalAxisWidgetRight.h
	QtUtilities/Axis/ExternalAxisWidgetTop.h
	QtUtilities/Axis/ExternalAxisWidgetBottom.h
	DESTINATION include/casacode/casaqt/QtUtilities
	)
install (FILES
	QwtPlotter/QPAnnotation.h
	QwtPlotter/QPAxis.qo.h
	QwtPlotter/QPBarPlot.h
	QwtPlotter/QPCanvas.qo.h
	QwtPlotter/QPCanvasHelpers.qo.h
	QwtPlotter/QPData.h
	QwtPlotter/QPExportCanvas.h
	QwtPlotter/QPFactory.h
	QwtPlotter/QPImageCache.h
	QwtPlotter/QPLayeredCanvas.qo.h
	QwtPlotter/QPOptions.h
	QwtPlotter/QPExporter.h
	QwtPlotter/QPPanel.qo.h
	QwtPlotter/QPPlotItem.qo.h
	QwtPlotter/QPPlotter.qo.h
	QwtPlotter/QPRasterPlot.h
	QwtPlotter/QPScatterPlot.h
	QwtPlotter/QPShape.h
	QwtPlotter/QPTool.qo.h
	DESTINATION include/casacode/casaqt/QwtPlotter
	)

casa_add_executable( casaqt casafilecatalog apps/casafilecatalog/casafilecatalog.cc )
casa_add_executable( casaqt casalogger apps/casalogger/casalogger.cc )
casa_add_executable( casaqt casapictureviewer apps/casapictureviewer/casapictureviewer.cc )
casa_add_executable( casaqt casaplotserver apps/casaplotserver/casaplotserver.cc )
casa_add_executable( casaqt casaprogresstimer apps/casaprogresstimer/casaprogresstimer.cc )
casa_add_executable( casaqt qcasabrowser apps/qcasabrowser/qcasabrowser.cc )
casa_add_executable( casaqt qwtplottertest apps/qwtplottertest/qwtplottertest.cc )
casa_add_executable( casaqt mergeparams apps/mergeparams/mergeparams.cc )
#casa_add_executable( casaqt genparams apps/genparams/genparams.cc )

install( PROGRAMS apps/qcasabrowser/casabrowser DESTINATION bin )
casa_add_assay( casaqt QtProgressMeter/test/tProgressMeter.cc )
