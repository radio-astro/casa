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
  QtAutoGui/QtAutoGui.qo.h
  QtAutoGui/QtGuiEntry.qo.h
  QtPlotter/QtCanvas.qo.h
  QtPlotter/QtMWCTools.qo.h
  QtPlotter/QtOldMWCTools.qo.h
  QtPlotter/QtPlotter.qo.h
  QtPlotter/QtProfile.qo.h
  QtPlotter/QtProfilePrefs.qo.h
  QtPlotter/QtDrawCanvas.qo.h
  QtPlotter/QtDrawColors.qo.h
  QtPlotter/QtDrawer.qo.h
  QtPlotter/GaussianEstimateWidget.qo.h
  QtPlotter/GaussianEstimateDialog.qo.h
  QtPlotter/MomentSettingsWidgetOptical.qo.h
  QtPlotter/MomentSettingsWidgetRadio.qo.h
  QtPlotter/SpecFitLogDialog.qo.h
  QtPlotter/SettingsWidget.qo.h
  QtPlotter/SearchMoleculesWidget.qo.h
  QtPlotter/SearchMoleculesResultsWidget.qo.h
  QtPlotter/SearchMoleculesDialog.qo.h
  QtPlotter/SearchRedshiftDialog.qo.h
  QtPlotter/LineOverlaysTab.qo.h
  QtPlotter/SmoothPreferences.qo.h
  QtPlotter/LineOverlaysSearchResultsDialog.qo.h
  QtPlotter/ColorSummaryWidget.qo.h
  QtPlotter/SpecFitSettingsFixedTableCell.qo.h
  QtPlotter/SpecFitSettingsWidgetOptical.qo.h
  QtPlotter/SpecFitSettingsWidgetRadio.qo.h
  QtPlotter/ThresholdingBinPlotDialog.qo.h
  QtPlotter/LegendPreferences.qo.h
  QtPlotter/CanvasHolder.qo.h
  QtPlotter/SpectralPositioningWidget.qo.h
  QtPlotter/annotations/AnnotationEditor.qo.h
  QtPlotter/annotations/AnnotationEditorText.qo.h
  QtViewer/AnimatorWidget.qo.h
  QtViewer/AnimatorHolder.qo.h
  QtViewer/CursorTrackingHolder.qo.h
  QtViewer/TrackBox.qo.h
  QtViewer/ColorHistogram.qo.h
  QtViewer/FileBox.qo.h
  QtViewer/InActiveDock.qo.h
  QtViewer/MakeMask.qo.h
  QtViewer/MakeRegion.qo.h
  QtViewer/QtAnnotatorGui.qo.h
  QtViewer/QtCanvasManager.qo.h
  QtViewer/QtCleanPanelGui.qo.h
  QtViewer/QtDataManager.qo.h
  QtViewer/QtExportManager.qo.h
  QtViewer/QtExportBrowser.qo.h
  QtViewer/QtDataOptionsPanel.qo.h
  QtViewer/QtDBusViewerAdaptor.qo.h
  QtViewer/QtDisplayDataGui.qo.h
  QtViewer/QtDisplayData.qo.h
  QtViewer/QtDisplayPanelGui.qo.h
  QtViewer/QtDisplayPanel.qo.h
  QtViewer/QtLELEdit.qo.h
  QtViewer/QtMouseToolBar.qo.h
  QtViewer/QtMouseTools.qo.h
  QtViewer/QtOldMouseTools.qo.h
  QtViewer/QtMouseToolState.qo.h
  QtViewer/RegionToolManager.qo.h
  QtViewer/QtPixelCanvas.qo.h
  QtViewer/QtRegionManager.qo.h
  QtViewer/QtViewerBase.qo.h
  QtViewer/QtViewerPrintGui.qo.h
  QtViewer/QtViewer.qo.h
  QtViewer/Preferences.qo.h
  QtViewer/ColorTransferWidget.qo.h
  QtViewer/ImageManager/ImageManagerDialog.qo.h
  QtViewer/ImageManager/DisplayOptionsDialog.h
  QtViewer/ImageManager/ImageScroll.qo.h
  QtViewer/ImageManager/ImageView.qo.h
  QtViewer/ImageManager/DisplayLabel.qo.h
  QtViewer/SlicerGen.qo.h
  Fit/FindSourcesDialog.qo.h
  Fit/ResidualHistogramDialog.qo.h
  Fit/Fit2DTool.qo.h
  Fit/PixelRangeDialog.qo.h
  Fit/Fit2DLogDialog.qo.h
  RegionShapes/DS9FileWriter.qo.h
  RegionShapes/QtNewRegionShape.qo.h
  RegionShapes/QtRegionShapeManager.qo.h
  RegionShapes/QtSingleRegionShape.qo.h
  RegionShapes/RSUtils.qo.h
  region/Region.qo.h
  region/Polyline.qo.h
  region/PVLine.qo.h
  region/QtRegionSource.qo.h
  region/QtRegionDock.qo.h
  region/QtRegionState.qo.h
  region/QtRegionStats.qo.h
  region/HistogramTab.qo.h
  region/HistogramGraph.qo.h
  Clean/CleanGui.qo.h
  Slicer/SlicePlot.qo.h
  Slicer/ImageSlice.qo.h
  Slicer/SliceSegment.qo.h
  Slicer/SlicerMainWindow.qo.h
  Slicer/SliceColorPreferences.qo.h
  Slicer/SlicePlotPreferences.qo.h
  )

casa_qt4_wrap_ui( outfiles_ui
  QtAutoGui/adjustmentBottom.ui
  QtAutoGui/checkButtonItem.ui
  QtAutoGui/comboItem.ui
  QtAutoGui/lineEditorItem.ui
  QtAutoGui/minMaxEditorItem.ui
  QtAutoGui/pairItem.ui
  QtAutoGui/pushButtonItem.ui
  QtAutoGui/sliderEditorItem.ui
  QtAutoGui/sliderLabelItem.ui
  QtViewer/AnimatorWidget.ui
  QtViewer/AnimatorHolder.ui
  QtViewer/CursorTrackingHolder.ui
  QtViewer/ColorHistogram.ui
  QtViewer/printControl.ui
  QtViewer/QtAnnotatorGui.ui
  QtViewer/QtDataManager.ui
  QtViewer/SlicerGen.ui
  QtViewer/QtExportManager.ui
  QtViewer/QtExportBrowser.ui
  QtViewer/QtDataOptionsPanel.ui
  QtViewer/QtRegionMgr.ui
  QtViewer/QtDataMgrMsSelect.ui
  QtViewer/Preferences.ui
  QtViewer/ColorTransferWidget.ui
  QtViewer/ImageManager/ImageManagerDialog.ui
  QtViewer/ImageManager/ImageScroll.ui
  QtViewer/ImageManager/ImageView.ui
  QtViewer/QtCanvasManager.ui
  QtViewer/CursorLink.ui
  QtPlotter/QtProfileGUI.ui
  QtPlotter/QtProfilePrefsGUI.ui
  QtPlotter/CanvasHolder.ui
  QtPlotter/GaussianEstimateDialog.ui
  QtPlotter/GaussianEstimateWidget.ui
  QtPlotter/SpecFitLogDialog.ui
  QtPlotter/MomentSettingsWidgetOptical.ui
  QtPlotter/MomentSettingsWidgetRadio.ui
  QtPlotter/SettingsWidget.ui
  QtPlotter/SearchMoleculesWidget.ui
  QtPlotter/SearchMoleculesResultsWidget.ui
  QtPlotter/SearchRedshiftDialog.ui
  QtPlotter/LineOverlaysTab.ui
  QtPlotter/LineOverlaysSearchResultsDialog.ui
  QtPlotter/SearchMoleculesDialog.ui
  QtPlotter/ColorSummaryWidget.ui
  QtPlotter/SpecFitSettingsWidgetRadio.ui
  QtPlotter/SpecFitSettingsWidgetOptical.ui
  QtPlotter/SpecFitSettingsFixedTableCell.ui
  QtPlotter/ThresholdingBinPlotDialog.ui
  QtPlotter/LegendPreferences.ui
  QtPlotter/SmoothPreferences.ui
  QtPlotter/SpectralPositioningWidget.ui
  QtPlotter/annotations/AnnotationEditor.ui
  QtPlotter/annotations/AnnotationEditorText.ui
  Fit/FindSourcesDialog.ui
  Fit/Fit2DTool.ui
  Fit/Fit2DLogDialog.ui
  Fit/PixelRangeDialog.ui
  Fit/ResidualHistogramDialog.ui
  RegionShapes/QtDS9WriteOptions.ui
  RegionShapes/QtEditRegionShape.ui
  RegionShapes/QtNewRegionShape.ui
  RegionShapes/QtRegionShapeManager.ui
  RegionShapes/QtRSFileLoader.ui
  RegionShapes/QtRSFileSaver.ui
  RegionShapes/QtSingleRegionShape.ui
  region/QtRegionDock.ui
  region/QtRegionState.ui
  region/QtImageRegionStats.ui
  region/QtMsRegionStats.ui
  region/HistogramTab.ui
  region/HistogramGraph.ui
  region/SlicerStats.ui
  region/PVLineStats.ui
  Slicer/SlicerMainWindow.ui
  Slicer/ImageSlice.ui
  Slicer/SliceSegment.ui
  Slicer/SliceColorPreferences.ui
  Slicer/SlicePlotPreferences.ui
  Clean/CleanGui.ui
  Clean/ConfirmStop.ui
  )

casa_qt4_add_resources( outfiles_qrc QtAutoGui/QtAutoGui.qrc OPTIONS -name QtAutoGui )
casa_qt4_add_resources( outfiles_qrc QtPlotter/QtPlotter.qrc OPTIONS -name QtPlotter )
casa_qt4_add_resources( outfiles_qrc QtPlotter/QtDrawer.qrc OPTIONS -name QtDrawer )
casa_qt4_add_resources( outfiles_qrc QtViewer/QtViewer.qrc OPTIONS -name QtViewer )
casa_qt4_add_resources( outfiles_qrc Slicer/Slicer.qrc OPTIONS -name Slicer )

include_directories( ${CMAKE_CURRENT_BINARY_DIR}/QtAutoGui ) # for generated code

casa_add_library( display
  ${outfiles_qoh} ${outfiles_ui} ${outfiles_qrc}
  Display/AttributeBuffer.cc
  Display/Attribute.cc
  Display/Options.cc
  Display/AttValBase.cc
  Display/Position.cc
  Display/DisplayCoordinateSystem.cc
  DisplayCanvas/DefaultWCCoordinateHandler.cc
  DisplayCanvas/DefaultWCSizeControlHandler.cc
  DisplayCanvas/WCAxisLabeller.cc
  DisplayCanvas/WCCoordinateHandler.cc
  DisplayCanvas/WCCSAxisLabeller.cc
  DisplayCanvas/WCCSNLAxisLabeller.cc
  DisplayCanvas/WCDataScaleHandler.cc
  DisplayCanvas/WCLinearScaleHandler.cc
  DisplayCanvas/WCPowerScaleHandler.cc
  DisplayCanvas/WCResampleHandler.cc
  DisplayCanvas/WCSimpleResampleHandler.cc
  DisplayCanvas/WCSizeControlHandler.cc
  Display/ColorConversion.cc
  Display/ColorDistribution.cc
  Display/Colormap.cc
  Display/ColormapDefinition.cc
  Display/ColormapInfo.cc
  Display/ColormapManager.cc
  DisplayDatas/ActiveCaching2dDD.cc
  DisplayDatas/AxesDisplayData.cc
  DisplayDatas/AxesDisplayMethod.cc
  DisplayDatas/CachingDisplayData.cc
  DisplayDatas/CachingDisplayMethod.cc
  DisplayDatas/DDDEllipse.cc
  DisplayDatas/DDDHandle.cc
  DisplayDatas/DDDObject.cc
  DisplayDatas/DDDPolygon.cc
  DisplayDatas/DDDRectangle.cc
  DisplayDatas/DisplayData.cc
  DisplayDatas/DisplayMethod.cc
  DisplayDatas/DrawingDisplayData.cc
  DisplayDatas/DrawingDisplayMethod.cc
  DisplayDatas/Galaxy.cc
  DisplayDatas/Histogram2dDD.cc
  DisplayDatas/Histogram2dDM.cc
  DisplayDatas/LatticeAsVector2.cc
  DisplayDatas/LatticePADD2.cc
  DisplayDatas/LattPADMVector2.cc
  DisplayDatas/MSAsRaster.cc
  DisplayDatas/NBody.cc
  DisplayDatas/PassiveCachingDD.cc
  DisplayDatas/PassiveTableDD.cc
  DisplayDatas/PrincipalAxesDD.cc
  DisplayDatas/PrincipalAxesDM.cc
  DisplayDatas/Profile2dDD.cc
  DisplayDatas/Profile2dDM.cc
  DisplayDatas/ScrollingRasterDD.cc
  DisplayDatas/ScrollingRasterDM.cc
  DisplayDatas/SkyCatOverlayDD.cc
  DisplayDatas/SkyCatOverlayDM.cc
  DisplayDatas/TblAsContour.cc
  DisplayDatas/TblAsContourDM.cc
  DisplayDatas/TblAsRasterDD.cc
  DisplayDatas/TblAsRasterDM.cc
  DisplayDatas/TblAsXYDD.cc
  DisplayDatas/TblAsXYDM.cc
  DisplayDatas/WedgeDD.cc
  DisplayDatas/WedgeDM.cc
  DisplayDatas/WorldAxesDD.cc
  DisplayDatas/WorldAxesDM.cc
  Display/DisplayEnums.cc
  Display/DisplayMarkerDefinitions.cc
  Display/DisplayParameter.cc
  Display/DLFont.cc
  Display/DParameterButton.cc
  Display/DParameterChoice.cc
  Display/DParameterColorChoice.cc
  Display/DParameterFontChoice.cc
  Display/DParameterMapKeyChoice.cc
  Display/DParameterString.cc
  Display/DParameterSwitch.cc
  DisplayEvents/Animator.cc
  DisplayEvents/AniPosEH.cc
  DisplayEvents/DDModEvent.cc
  DisplayEvents/DisplayDataEvent.cc
  DisplayEvents/DisplayEvent.cc
  DisplayEvents/DisplayTool.cc
  DisplayEvents/DTVisible.cc
  DisplayEvents/MultiWCTool.cc
  DisplayEvents/RegionTool.cc
  DisplayEvents/MWCAnimator.cc
  DisplayEvents/MWCCrosshairTool.cc
  DisplayEvents/MWCPannerTool.cc
  DisplayEvents/MWCPolylineTool.cc
  DisplayEvents/MWCRulerlineTool.cc
  DisplayEvents/MWCPositionVelocityTool.cc
  DisplayEvents/MWCPolyTool.cc
  DisplayEvents/MWCPTRegion.cc
  DisplayEvents/MWCEllipseTool.cc
  DisplayEvents/MWCRectTool.cc
  DisplayEvents/MultiRectTool.cc
  DisplayEvents/MultiRectToolImpl.cc
  DisplayEvents/MultiPVTool.cc
  DisplayEvents/MultiPVToolImpl.cc
  DisplayEvents/MultiEllipseTool.cc
  DisplayEvents/MultiEllipseToolImpl.cc
  DisplayEvents/MultiPointTool.cc
  DisplayEvents/MultiPointToolImpl.cc
  DisplayEvents/MultiPolyTool.cc
  DisplayEvents/MultiPolylineTool.cc
  DisplayEvents/MultiPolyToolImpl.cc
  DisplayEvents/MultiPolylineToolImpl.cc
  DisplayEvents/MWCETRegion.cc
  DisplayEvents/MWCRTRegion.cc
  DisplayEvents/MWCRTZoomer.cc
  DisplayEvents/PCInvisTool.cc
  DisplayEvents/PCITFiddler.cc
  DisplayEvents/PCMotionEH.cc
  DisplayEvents/PCMotionEvent.cc
  DisplayEvents/PCPositionEH.cc
  DisplayEvents/PCPositionEvent.cc
  DisplayEvents/PCRefreshEH.cc
  DisplayEvents/PCRefreshEvent.cc
  DisplayEvents/PCTestPattern.cc
  DisplayEvents/PCTool.cc
  DisplayEvents/PixelCanvasEvent.cc
  DisplayEvents/WCCrosshairTool.cc
  DisplayEvents/WCInvisTool.cc
  DisplayEvents/WCMotionEH.cc
  DisplayEvents/WCMotionEvent.cc
  DisplayEvents/WCPolyTool.cc
  DisplayEvents/WCPositionEH.cc
  DisplayEvents/WCPositionEvent.cc
  DisplayEvents/WCPTRegion.cc
  DisplayEvents/WCRectTool.cc
  DisplayEvents/WCRefreshEH.cc
  DisplayEvents/WCRefreshEvent.cc
  DisplayEvents/WCRTRegion.cc
  DisplayEvents/WCRTZoomer.cc
  DisplayEvents/WCTool.cc
  DisplayEvents/WorldCanvasEvent.cc
  Display/GLPCDisplayList.cc
  Display/GLPCTexture.cc
  Display/GLPixelCanvas.cc
  Display/GLPixelCanvasColorTable.cc
  Display/GLSupport.cc
  Display/MultiWCHolder.cc
  Display/PanelDisplay.cc
  Display/PCVGBuffer.cc
  Display/PixelCanvas.cc
  Display/PixelCanvasColorTable.cc
  Display/PSDriver.cc
  Display/PSPixelCanvas.cc
  Display/PSPixelCanvasColorTable.cc
  Display/PSWorldCanvasApp.cc
  DisplayShapes/Annotations.cc
  DisplayShapes/DisplayShape.cc
  DisplayShapes/DisplayShapeInterface.cc
  DisplayShapes/DisplayShapeWithCoords.cc
  DisplayShapes/DSArrow.cc
  DisplayShapes/DSBasic.cc
  DisplayShapes/DSCircle.cc
  DisplayShapes/DSClosed.cc
  DisplayShapes/DSEllipse.cc
  DisplayShapes/DSLine.cc
  DisplayShapes/DSMarker.cc
  DisplayShapes/DSPixelArrow.cc
  DisplayShapes/DSPixelEllipse.cc
  DisplayShapes/DSPixelMarker.cc
  DisplayShapes/DSPixelPoly.cc
  DisplayShapes/DSPixelPolyLine.cc
  DisplayShapes/DSPixelRectangle.cc
  DisplayShapes/DSPixelText.cc
  DisplayShapes/DSPoly.cc
  DisplayShapes/DSPolyLine.cc
  DisplayShapes/DSRectangle.cc
  DisplayShapes/DSScreenArrow.cc
  DisplayShapes/DSScreenEllipse.cc
  DisplayShapes/DSScreenMarker.cc
  DisplayShapes/DSScreenPoly.cc
  DisplayShapes/DSScreenPolyLine.cc
  DisplayShapes/DSScreenRectangle.cc
  DisplayShapes/DSScreenText.cc
  DisplayShapes/DSSquare.cc
  DisplayShapes/DSText.cc
  DisplayShapes/DSWorldArrow.cc
  DisplayShapes/DSWorldEllipse.cc
  DisplayShapes/DSWorldMarker.cc
  DisplayShapes/DSWorldPoly.cc
  DisplayShapes/DSWorldPolyLine.cc
  DisplayShapes/DSWorldRectangle.cc
  DisplayShapes/DSWorldText.cc
  Display/SlicePanelDisplay.cc
  Display/State.cc
  Display/WCPGFilter.cc
  Display/WorldCanvas.cc
  Display/WorldCanvasHolder.cc
  Display/WorldCanvasPGPlotDriver.cc
  Fit/FindSourcesDialog.cc
  Fit/Fit2DTool.cc
  Fit/Fit2DLogDialog.cc
  Fit/Gaussian2DFitter.cc
  Fit/ComponentListWrapper.cc
  Fit/ColorComboDelegate.cc
  Fit/PixelRangeDialog.cc
  Fit/ResidualHistogramDialog.cc
  Fit/RegionBox.cc
  QtAutoGui/QtAutoGui.cc
  QtAutoGui/QtGuiEntry.cc
  QtAutoGui/QtLayout.cc
  QtAutoGui/QtXmlRecord.cc
  QtPlotter/QtCanvas.cc
  QtPlotter/ProfileTaskFacilitator.cc
  QtPlotter/QtMWCTools.cc
  QtPlotter/QtOldMWCTools.cc
  QtPlotter/QtPlotSettings.cc
  QtPlotter/QtPlotter.cc
  QtPlotter/QtProfile.cc
  QtPlotter/QtProfilePrefs.cc
  QtPlotter/QtDrawCanvas.cc
  QtPlotter/QtDrawColors.cc
  QtPlotter/QtDrawSettings.cc
  QtPlotter/QtDrawer.cc
  QtPlotter/CanvasCurve.cc
  QtPlotter/CanvasHolder.cc
  QtPlotter/ColorSummaryWidget.cc
  QtPlotter/ColorSummaryDelegate.cc
  QtPlotter/Util.cc
  QtPlotter/MolecularLine.cc
  QtPlotter/ProfileFitMarker.cc
  QtPlotter/WorldCanvasTranslator.cc
  QtPlotter/SpecFitLogDialog.cc
  QtPlotter/GaussianEstimateWidget.cc
  QtPlotter/GaussianEstimateDialog.cc
  QtPlotter/SearchMoleculesWidget.cc
  QtPlotter/SearchMoleculesResultsWidget.cc
  QtPlotter/SearchMoleculesResultDisplayer.cc
  QtPlotter/LineOverlaysTab.cc
  QtPlotter/SmoothPreferences.cc
  QtPlotter/SmoothSpinBox.cc
  QtPlotter/LineOverlaysSearchResultsDialog.cc
  QtPlotter/SearchMoleculesDialog.cc
  QtPlotter/SearchRedshiftDialog.cc
  QtPlotter/MomentSettingsWidgetOptical.cc
  QtPlotter/MomentSettingsWidgetRadio.cc
  QtPlotter/SettingsWidget.cc
  QtPlotter/SpecFit.cc
  QtPlotter/SpecFitPolynomial.cc
  QtPlotter/SpecFitGaussian.cc
  QtPlotter/SpecFitSettingsWidgetOptical.cc
  QtPlotter/SpecFitSettingsWidgetRadio.cc
  QtPlotter/SpecFitSettingsFixedTableCell.cc
  QtPlotter/ProfileTaskMonitor.cc
  QtPlotter/ThresholdingBinPlotDialog.cc
  QtPlotter/LegendPreferences.cc
  QtPlotter/SpectralPositioningWidget.cc 
  QtPlotter/conversion/Converter.cc
  QtPlotter/conversion/ConverterFrequency.cc
  QtPlotter/conversion/ConverterFrequencyVelocity.cc
  QtPlotter/conversion/ConverterFrequencyWavelength.cc
  QtPlotter/conversion/ConverterVelocity.cc
  QtPlotter/conversion/ConverterVelocityFrequency.cc
  QtPlotter/conversion/ConverterVelocityWavelength.cc
  QtPlotter/conversion/ConverterWavelength.cc
  QtPlotter/conversion/ConverterWavelengthFrequency.cc
  QtPlotter/conversion/ConverterWavelengthVelocity.cc
  QtPlotter/conversion/ConverterChannel.cc
  QtPlotter/conversion/ConverterIntensity.cc
  QtPlotter/annotations/Annotation.cc
  QtPlotter/annotations/AnnotationText.cc
  QtPlotter/annotations/AnnotationEditor.cc
  QtPlotter/annotations/AnnotationEditorText.cc
  QtPlotter/annotations/PropertyListener.cc
  QtPlotter/canvasMode/CanvasMode.cc
  QtPlotter/canvasMode/CanvasModeAnnotation.cc
  QtPlotter/canvasMode/CanvasModeChannel.cc
  QtPlotter/canvasMode/CanvasModeContextMenu.cc
  QtPlotter/canvasMode/CanvasModeRangeSelection.cc
  QtPlotter/canvasMode/CanvasModeZoom.cc
  QtPlotter/canvasMode/CanvasModeFactory.cc
  QtViewer/AnimatorWidget.cc
  QtViewer/AnimatorHolder.cc
  QtViewer/CursorTrackingHolder.cc
  QtViewer/TrackBox.cc
  QtViewer/QtOptions.cc
  QtViewer/ColorHistogram.cc
  QtViewer/ColorHistogramScale.cc
  QtViewer/FileBox.cc
  QtViewer/InvisibleAxis.cc
  QtViewer/QtWCBox.cc
  QtViewer/MakeMask.cc
  QtViewer/MakeRegion.cc
  QtViewer/QtAnnotatorGui.cc
  QtViewer/QtCanvasManager.cc
  QtViewer/QtCleanPanelGui.cc
  QtViewer/QtDataManager.cc
  QtViewer/QtExportManager.cc
  QtViewer/QtExportBrowser.cc
  QtViewer/QtDataOptionsPanel.cc
  QtViewer/QtDBusViewerAdaptor.cc
  QtViewer/QtDisplayData.cc
  QtViewer/QtDisplayPanel.cc
  QtViewer/QtDisplayPanelGui.cc
  QtViewer/QtMouseToolBar.cc
  QtViewer/QtMouseTools.cc
  QtViewer/QtOldMouseTools.cc
  QtViewer/QtMouseToolState.cc
  QtViewer/RegionToolManager.cc
  QtViewer/QtPCColorTable.cc
  QtViewer/QtPixelCanvas.cc
  QtViewer/QtRegionManager.cc
  QtViewer/QtViewerBase.cc
  QtViewer/QtViewer.cc
  QtViewer/QtViewerPrintGui.cc
  QtViewer/Preferences.cc
  QtViewer/ColorTransferWidget.cc
  QtViewer/DisplayDataHolder.cc
  QtViewer/InActiveDock.cc
  QtViewer/ImageManager/ImageScroll.cc
  QtViewer/ImageManager/ImageManagerDialog.cc
  QtViewer/ImageManager/ImageView.cc
  QtViewer/ImageManager/DisplayLabel.cc
  QtViewer/ImageManager/DisplayOptionsDialog.cc
  QtViewer/SlicerGen.cc
  RegionShapes/DS9FileReader.cc
  RegionShapes/DS9FileWriter.cc
  RegionShapes/QtNewRegionShape.cc
  RegionShapes/QtRegionShapeManager.cc
  RegionShapes/QtSingleRegionShape.cc
  RegionShapes/RegionShape.cc
  RegionShapes/RegionShapes.cc
  RegionShapes/RSFileReaderWriter.cc
  RegionShapes/RSUtils.cc
  RegionShapes/XMLFileReaderWriter.cc
  Slicer/SlicerMainWindow.cc
  Slicer/SliceWorker.cc
  Slicer/SliceColorPreferences.cc
  Slicer/SliceZoomer.cc
  Slicer/ImageSlice.cc
  Slicer/SlicePlot.cc
  Slicer/SliceAxisDraw.cc
  Slicer/ImageSlice.cc
  Slicer/ImageSliceColorBar.cc
  Slicer/SliceSegment.cc
  Slicer/SliceStatistics.cc
  Slicer/SliceStatisticsFactory.cc
  Slicer/SliceStatisticsDistance.cc
  Slicer/SliceStatisticsPosition.cc
  Slicer/SlicePlotPreferences.cc
  Slicer/SegmentTracer.cc
  Utilities/DisplayOptions.cc
  Utilities/DlHandle.cc
  Utilities/DlTarget.cc
  Utilities/Lowlevel.cc
  Utilities/dtor.cc
  Utilities/ImageProperties.cc
  region/RegionCreator.cc
  region/RegionSource.cc
  region/QtRegionSource.cc
  region/QtRegionSourceFactory.cc
  region/QtRegionDock.cc
  region/QtRegionState.cc
  region/QtRegionStats.cc
  region/HistogramTab.cc
  region/HistogramGraph.cc
  region/Rectangle.cc
  region/PVLine.cc
  region/Ellipse.cc
  region/Point.cc
  region/Polygon.cc
  region/Polyline.cc
  region/Region.cc
  ds9/ds9context.cc
  ds9/ds9parser.cc
  ds9/ds9writer.cc
#  ${BISON_ds9parse_OUTPUTS}
  ds9/ds9parse.cc
#  ${FLEX_ds9lex_OUTPUTS}
  ds9/ds9lex.cc
  Clean/CleanGui.cc
  )


install ( FILES
          Clean/CleanGui.qo.h
          DESTINATION include/casacode/display/Clean )
install ( FILES
          Display/Attribute.h
          Display/AttributeBuffer.h
          Display/AttVal.h
          Display/AttValBase.h
          Display/AttValPoi.h
          Display/AttValPoiTol.h
          Display/AttValTol.h
          Display/ColorConversion.h
          Display/ColorDistribution.h
          Display/Colormap.h
          Display/ColormapDefinition.h
          Display/ColormapInfo.h
          Display/ColormapManager.h
          Display/DisplayCoordinateSystem.h
          Display/DisplayEnums.h
          Display/DisplayParameter.h
          Display/DLFont.h
          Display/DParameterButton.h
          Display/DParameterChoice.h
          Display/DParameterColorChoice.h
          Display/DParameterFontChoice.h
          Display/DParameterMapKeyChoice.h
          Display/DParameterRange.h
          Display/DParameterString.h
          Display/DParameterSwitch.h
          Display/GLPCDisplayList.h
          Display/GLPCTexture.h
          Display/GLPixelCanvas.h
          Display/GLPixelCanvasColorTable.h
          Display/GLSupport.h
          Display/MouseToolState.h
          Display/MultiWCHolder.h
          Display/Options.h
          Display/PanelDisplay.h
          Display/PCVGBuffer.h
          Display/PixelCanvas.h
          Display/PixelCanvasColorTable.h
          Display/Position.h
          Display/PSDriver.h
          Display/PSPixelCanvas.h
          Display/PSPixelCanvasColorTable.h
          Display/PSWorldCanvasApp.h
          Display/SlicePanelDisplay.h
          Display/StandAloneDisplayApp.h
          Display/State.h
          Display/WCPGFilter.h
          Display/WorldCanvas.h
          Display/WorldCanvasHolder.h
          Display/WorldCanvasPGPlotDriver.h
          DESTINATION include/casacode/display/Display )
install ( FILES
          DisplayCanvas/DefaultWCCoordinateHandler.h
          DisplayCanvas/DefaultWCSizeControlHandler.h
          DisplayCanvas/WCAxisLabeller.h
          DisplayCanvas/WCCoordinateHandler.h
          DisplayCanvas/WCCSAxisLabeller.h
          DisplayCanvas/WCCSNLAxisLabeller.h
          DisplayCanvas/WCDataScaleHandler.h
          DisplayCanvas/WCLinearScaleHandler.h
          DisplayCanvas/WCPowerScaleHandler.h
          DisplayCanvas/WCResampleHandler.h
          DisplayCanvas/WCSimpleResampleHandler.h
          DisplayCanvas/WCSizeControlHandler.h
          DESTINATION include/casacode/display/DisplayCanvas )
install ( FILES
          DisplayDatas/ActiveCaching2dDD.h
          DisplayDatas/AxesDisplayData.h
          DisplayDatas/AxesDisplayMethod.h
          DisplayDatas/CachingDisplayData.h
          DisplayDatas/CachingDisplayMethod.h
          DisplayDatas/DDDEllipse.h
          DisplayDatas/DDDHandle.h
          DisplayDatas/DDDObject.h
          DisplayDatas/DDDPolygon.h
          DisplayDatas/DDDRectangle.h
          DisplayDatas/DisplayData.h
          DisplayDatas/DisplayDataOptions.h
          DisplayDatas/DisplayMethod.h
          DisplayDatas/DrawingDisplayData.h
          DisplayDatas/DrawingDisplayMethod.h
          DisplayDatas/Galaxy.h
          DisplayDatas/Histogram2dDD.h
          DisplayDatas/Histogram2dDM.h
          DisplayDatas/LatticeAsContour.h
          DisplayDatas/LatticeAsMarker.h
          DisplayDatas/LatticeAsRaster.h
          DisplayDatas/LatticeAsVector.h
          DisplayDatas/LatticePADD.h
          DisplayDatas/LatticePADM.h
          DisplayDatas/LatticePADMContour.h
          DisplayDatas/LatticePADMMarker.h
          DisplayDatas/LatticePADMRaster.h
          DisplayDatas/LatticePADMVector.h
          DisplayDatas/MSAsRaster.h
          DisplayDatas/NBody.h
          DisplayDatas/PassiveCachingDD.h
          DisplayDatas/PassiveTableDD.h
          DisplayDatas/PrincipalAxesDD.h
          DisplayDatas/PrincipalAxesDM.h
          DisplayDatas/Profile2dDD.h
          DisplayDatas/Profile2dDM.h
          DisplayDatas/ScrollingRasterDD.h
          DisplayDatas/ScrollingRasterDM.h
          DisplayDatas/SkyCatOverlayDD.h
          DisplayDatas/SkyCatOverlayDM.h
          DisplayDatas/TblAsContourDD.h
          DisplayDatas/TblAsContourDM.h
          DisplayDatas/TblAsRasterDD.h
          DisplayDatas/TblAsRasterDM.h
          DisplayDatas/TblAsXYDD.h
          DisplayDatas/TblAsXYDM.h
          DisplayDatas/WedgeDD.h
          DisplayDatas/WedgeDM.h
          DisplayDatas/WorldAxesDD.h
          DisplayDatas/WorldAxesDM.h
          DESTINATION include/casacode/display/DisplayDatas )
install ( FILES
          DisplayEvents/Animator.h
          DisplayEvents/AniPosEH.h
          DisplayEvents/CrosshairEvent.h
          DisplayEvents/DDModEvent.h
          DisplayEvents/DisplayDataEvent.h
          DisplayEvents/DisplayEH.h
          DisplayEvents/DisplayEvent.h
          DisplayEvents/DisplayTool.h
          DisplayEvents/DTVisible.h
          DisplayEvents/MultiEllipseTool.h
          DisplayEvents/MultiEllipseToolImpl.h
          DisplayEvents/MultiPointTool.h
          DisplayEvents/MultiPointToolImpl.h
          DisplayEvents/MultiPolylineTool.h
          DisplayEvents/MultiPolylineToolImpl.h
          DisplayEvents/MultiPolyTool.h
          DisplayEvents/MultiPolyToolImpl.h
          DisplayEvents/MultiPVTool.h
          DisplayEvents/MultiPVToolImpl.h
          DisplayEvents/MultiRectTool.h
          DisplayEvents/MultiRectToolImpl.h
          DisplayEvents/MultiWCTool.h
          DisplayEvents/MWCAnimator.h
          DisplayEvents/MWCCrosshairTool.h
          DisplayEvents/MWCEllipseTool.h
          DisplayEvents/MWCETRegion.h
          DisplayEvents/MWCEvents.h
          DisplayEvents/MWCPannerTool.h
          DisplayEvents/MWCPolylineTool.h
          DisplayEvents/MWCPolyTool.h
          DisplayEvents/MWCPositionVelocityTool.h
          DisplayEvents/MWCPTRegion.h
          DisplayEvents/MWCRectTool.h
          DisplayEvents/MWCRTRegion.h
          DisplayEvents/MWCRTZoomer.h
          DisplayEvents/MWCRulerlineTool.h
          DisplayEvents/PCInvisTool.h
          DisplayEvents/PCITFiddler.h
          DisplayEvents/PCMotionEH.h
          DisplayEvents/PCMotionEvent.h
          DisplayEvents/PCPositionEH.h
          DisplayEvents/PCPositionEvent.h
          DisplayEvents/PCRefreshEH.h
          DisplayEvents/PCRefreshEvent.h
          DisplayEvents/PCTestPattern.h
          DisplayEvents/PCTool.h
          DisplayEvents/PixelCanvasEvent.h
          DisplayEvents/RectRegionEvent.h
          DisplayEvents/RegionTool.h
          DisplayEvents/WCCrosshairTool.h
          DisplayEvents/WCInvisTool.h
          DisplayEvents/WCMotionEH.h
          DisplayEvents/WCMotionEvent.h
          DisplayEvents/WCPolyTool.h
          DisplayEvents/WCPositionEH.h
          DisplayEvents/WCPositionEvent.h
          DisplayEvents/WCPTRegion.h
          DisplayEvents/WCRectTool.h
          DisplayEvents/WCRefreshEH.h
          DisplayEvents/WCRefreshEvent.h
          DisplayEvents/WCRTRegion.h
          DisplayEvents/WCRTZoomer.h
          DisplayEvents/WCTool.h
          DisplayEvents/WorldCanvasEvent.h
          DESTINATION include/casacode/display/DisplayEvents )
install ( FILES
          ds9/ds9context.h
          ds9/ds9FlexLexer.h
          ds9/ds9lex.h
          ds9/ds9parser.h
          ds9/ds9writer.h
          DESTINATION include/casacode/display/ds9 )
install ( FILES
          Fit/ColorComboDelegate.h
          Fit/ComponentListWrapper.h
          Fit/FindSourcesDialog.qo.h
          Fit/Fit2DLogDialog.qo.h
          Fit/Fit2DTool.qo.h
          Fit/Gaussian2DFitter.h
          Fit/PixelRangeDialog.qo.h
          Fit/RegionBox.h
          Fit/ResidualHistogramDialog.qo.h
          DESTINATION include/casacode/display/Fit )
install ( FILES
          functional/elements.h
          DESTINATION include/casacode/display/functional )
install ( FILES
          QtAutoGui/QtAutoGui.qo.h
          QtAutoGui/QtGuiEntry.qo.h
          QtAutoGui/QtLayout.h
          QtAutoGui/QtXmlRecord.h
          DESTINATION include/casacode/display/QtAutoGui )
install ( FILES
          QtPlotter/annotations/Annotation.h
          QtPlotter/annotations/AnnotationEditor.qo.h
          QtPlotter/annotations/AnnotationEditorText.qo.h
          QtPlotter/annotations/AnnotationText.h
          QtPlotter/annotations/PropertyListener.h
          QtPlotter/CanvasCurve.h
          QtPlotter/CanvasHolder.qo.h
          QtPlotter/canvasMode/CanvasMode.h
          QtPlotter/canvasMode/CanvasModeAnnotation.h
          QtPlotter/canvasMode/CanvasModeChannel.h
          QtPlotter/canvasMode/CanvasModeContextMenu.h
          QtPlotter/canvasMode/CanvasModeFactory.h
          QtPlotter/canvasMode/CanvasModeRangeSelection.h
          QtPlotter/canvasMode/CanvasModeZoom.h
          QtPlotter/ColorSummaryDelegate.h
          QtPlotter/ColorSummaryWidget.qo.h
          QtPlotter/conversion/Converter.h
          QtPlotter/conversion/ConverterChannel.h
          QtPlotter/conversion/ConverterFrequency.h
          QtPlotter/conversion/ConverterFrequencyVelocity.h
          QtPlotter/conversion/ConverterFrequencyWavelength.h
          QtPlotter/conversion/ConverterIntensity.h
          QtPlotter/conversion/ConverterVelocity.h
          QtPlotter/conversion/ConverterVelocityFrequency.h
          QtPlotter/conversion/ConverterVelocityWavelength.h
          QtPlotter/conversion/ConverterWavelength.h
          QtPlotter/conversion/ConverterWavelengthFrequency.h
          QtPlotter/conversion/ConverterWavelengthVelocity.h
          QtPlotter/GaussianEstimateDialog.qo.h
          QtPlotter/GaussianEstimateWidget.qo.h
          QtPlotter/LegendPreferences.qo.h
          QtPlotter/LineOverlaysSearchResultsDialog.qo.h
          QtPlotter/LineOverlaysTab.qo.h
          QtPlotter/MolecularLine.h
          QtPlotter/MomentSettingsWidgetOptical.qo.h
          QtPlotter/MomentSettingsWidgetRadio.qo.h
          QtPlotter/ProfileFitMarker.h
          QtPlotter/ProfileTaskFacilitator.h
          QtPlotter/ProfileTaskMonitor.h
          QtPlotter/QtCanvas.qo.h
          QtPlotter/QtDrawCanvas.qo.h
          QtPlotter/QtDrawColors.qo.h
          QtPlotter/QtDrawer.qo.h
          QtPlotter/QtDrawSettings.h
          QtPlotter/QtMWCTools.qo.h
          QtPlotter/QtOldMWCTools.qo.h
          QtPlotter/QtPlotSettings.h
          QtPlotter/QtPlotter.qo.h
          QtPlotter/QtProfile.qo.h
          QtPlotter/QtProfilePrefs.qo.h
          QtPlotter/SearchMoleculesDialog.qo.h
          QtPlotter/SearchMoleculesResultDisplayer.h
          QtPlotter/SearchMoleculesResultsWidget.qo.h
          QtPlotter/SearchMoleculesWidget.qo.h
          QtPlotter/SearchRedshiftDialog.qo.h
          QtPlotter/SettingsWidget.qo.h
          QtPlotter/SmoothPreferences.qo.h
          QtPlotter/SmoothSpinBox.h
          QtPlotter/SpecFit.h
          QtPlotter/SpecFitGaussian.h
          QtPlotter/SpecFitLogDialog.qo.h
          QtPlotter/SpecFitPolynomial.h
          QtPlotter/SpecFitSettingsFixedTableCell.qo.h
          QtPlotter/SpecFitSettingsWidgetOptical.qo.h
          QtPlotter/SpecFitSettingsWidgetRadio.qo.h
          QtPlotter/SpectralPositioningWidget.qo.h
          QtPlotter/ThresholdingBinPlotDialog.qo.h
          QtPlotter/Util.h
          QtPlotter/WorldCanvasTranslator.h
          DESTINATION include/casacode/display/QtPlotter )
install ( FILES
          QtViewer/AnimatorHolder.qo.h
          QtViewer/AnimatorWidget.qo.h
          QtViewer/ColorHistogram.qo.h
          QtViewer/ColorHistogramScale.qo.h
          QtViewer/ColorTransferWidget.qo.h
          QtViewer/CursorTrackingHolder.qo.h
          QtViewer/DisplayDataHolder.h
          QtViewer/FileBox.qo.h
          QtViewer/ImageManager/DisplayLabel.qo.h
          QtViewer/ImageManager/DisplayOptionsDialog.h
          QtViewer/ImageManager/ImageManagerDialog.qo.h
          QtViewer/ImageManager/ImageScroll.qo.h
          QtViewer/ImageManager/ImageTracker.h
          QtViewer/ImageManager/ImageView.qo.h
          QtViewer/InActiveDock.qo.h
          QtViewer/InvisibleAxis.h
          QtViewer/MakeMask.qo.h
          QtViewer/MakeRegion.qo.h
          QtViewer/Preferences.qo.h
          QtViewer/QtAnnotatorGui.qo.h
          QtViewer/QtApp.h
          QtViewer/QtCanvasManager.qo.h
          QtViewer/QtCleanPanelGui.qo.h
          QtViewer/QtDataManager.qo.h
          QtViewer/QtDataOptionsPanel.qo.h
          QtViewer/QtDBusViewerAdaptor.qo.h
          QtViewer/QtDisplayData.qo.h
          QtViewer/QtDisplayDataGui.qo.h
          QtViewer/QtDisplayPanel.qo.h
          QtViewer/QtDisplayPanelGui.qo.h
          QtViewer/QtExportBrowser.qo.h
          QtViewer/QtExportManager.qo.h
          QtViewer/QtLELEdit.qo.h
          QtViewer/QtMouseToolBar.qo.h
          QtViewer/QtMouseTools.qo.h
          QtViewer/QtMouseToolState.qo.h
          QtViewer/QtOldMouseTools.qo.h
          QtViewer/QtOptions.h
          QtViewer/QtPCColorTable.h
          QtViewer/QtPixelCanvas.qo.h
          QtViewer/QtRegionManager.qo.h
          QtViewer/QtViewer.qo.h
          QtViewer/QtViewerBase.qo.h
          QtViewer/QtViewerPrintGui.qo.h
          QtViewer/QtWCBox.h
          QtViewer/RegionToolManager.qo.h
          QtViewer/SlicerGen.qo.h
          QtViewer/TrackBox.qo.h
          DESTINATION include/casacode/display/QtViewer )
install ( FILES
          region/Ellipse.h
          region/HistogramGraph.qo.h
          region/HistogramTab.qo.h
          region/Point.h
          region/Polygon.h
          region/Polyline.qo.h
          region/PVLine.qo.h
          region/QtRegionDock.qo.h
          region/QtRegionSource.qo.h
          region/QtRegionSourceFactory.h
          region/QtRegionState.qo.h
          region/QtRegionStats.qo.h
          region/Rectangle.h
          region/Region.qo.h
          region/RegionCreator.h
          region/RegionEnums.h
          region/RegionInfo.h
          region/RegionSource.h
          region/RegionSourceFactory.h
          DESTINATION include/casacode/display/region )
install ( FILES
          Slicer/ImageSlice.qo.h
          Slicer/ImageSliceColorBar.h
          Slicer/SegmentTracer.h
          Slicer/SliceAxisDraw.h
          Slicer/SliceColorPreferences.qo.h
          Slicer/SlicePlot.qo.h
          Slicer/SlicePlotPreferences.qo.h
          Slicer/SlicerMainWindow.qo.h
          Slicer/SliceSegment.qo.h
          Slicer/SliceStatistics.h
          Slicer/SliceStatisticsDistance.h
          Slicer/SliceStatisticsFactory.h
          Slicer/SliceStatisticsPosition.h
          Slicer/SliceWorker.h
          Slicer/SliceZoomer.h
          DESTINATION include/casacode/display/Slicer )
install ( FILES
          Utilities/DisplayOptions.h
          Utilities/DlHandle.h
          Utilities/DlTarget.h
          Utilities/dtor.h
          Utilities/ImageProperties.h
          Utilities/Lowlevel.h
          Utilities/StatusSink.h
          Utilities/StringUtil.h
          Utilities/VOID.h
          DESTINATION include/casacode/display/Utilities )
install ( FILES
          Display.h
          DisplayDatas.h
          DisplayErrors.h
          DisplayEvents.h
          DESTINATION include/casacode/display )

#          DisplayShapes/Annotations.h
#          DisplayShapes/DisplayShape.h
#          DisplayShapes/DisplayShapeInterface.h
#          DisplayShapes/DisplayShapeWithCoords.h
#          DisplayShapes/DSArrow.h
#          DisplayShapes/DSBasic.h
#          DisplayShapes/DSCircle.h
#          DisplayShapes/DSClosed.h
#          DisplayShapes/DSEllipse.h
#          DisplayShapes/DSLine.h
#          DisplayShapes/DSMarker.h
#          DisplayShapes/DSPixelArrow.h
#          DisplayShapes/DSPixelEllipse.h
#          DisplayShapes/DSPixelMarker.h
#          DisplayShapes/DSPixelPoly.h
#          DisplayShapes/DSPixelPolyLine.h
#          DisplayShapes/DSPixelRectangle.h
#          DisplayShapes/DSPixelText.h
#          DisplayShapes/DSPoly.h
#          DisplayShapes/DSPolyLine.h
#          DisplayShapes/DSRectangle.h
#          DisplayShapes/DSScreenArrow.h
#          DisplayShapes/DSScreenEllipse.h
#          DisplayShapes/DSScreenMarker.h
#          DisplayShapes/DSScreenPoly.h
#          DisplayShapes/DSScreenPolyLine.h
#          DisplayShapes/DSScreenRectangle.h
#          DisplayShapes/DSScreenText.h
#          DisplayShapes/DSSquare.h
#          DisplayShapes/DSText.h
#          DisplayShapes/DSWorldArrow.h
#          DisplayShapes/DSWorldEllipse.h
#          DisplayShapes/DSWorldMarker.h
#          DisplayShapes/DSWorldPoly.h
#          DisplayShapes/DSWorldPolyLine.h
#          DisplayShapes/DSWorldRectangle.h
#          DisplayShapes/DSWorldText.h
#          DisplayShapes.h
#          RegionShapes/DS9FileReader.h
#          RegionShapes/DS9FileWriter.qo.h
#          RegionShapes/QtNewRegionShape.qo.h
#          RegionShapes/QtRegionShapeManager.qo.h
#          RegionShapes/QtSingleRegionShape.qo.h
#          RegionShapes/RegionShape.h
#          RegionShapes/RegionShapes.h
#          RegionShapes/RSFileReaderWriter.h
#          RegionShapes/RSUtils.qo.h
#          RegionShapes/XMLFileReaderWriter.h

casa_add_executable( display casaplotter apps/casaplotter/casaplotter.cc )
#casa_add_executable( display casadrawer apps/casadrawer/casadrawer.cc )
casa_add_executable( display casaviewer apps/casaviewer/casaviewer.cc )
casa_add_assay( display DisplayEvents/test/tDisplayEvent.cc )
casa_add_assay( display DisplayEvents/test/tDisplayTool.cc )
casa_add_assay( display Display/test/dGLDemo.cc )
casa_add_assay( display Display/test/dPSLatticeAsRaster.cc )
casa_add_assay( display Display/test/dPSWorldCanvasApp.cc )
casa_add_assay( display Display/test/tAttribute.cc )
casa_add_assay( display Display/test/tAttVal.cc )
casa_add_assay( display Display/test/tAttValPoi.cc )
casa_add_assay( display QtViewer/test/dQtDataManager.cc )
casa_add_assay( display Utilities/test/tDisplayOptions.cc )
casa_add_assay( display Utilities/test/tDlHandle.cc )
