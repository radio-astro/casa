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
  implement/QtAutoGui/QtAutoGui.qo.h
  implement/QtAutoGui/QtGuiEntry.qo.h
  implement/QtPlotter/QtCanvas.qo.h
  implement/QtPlotter/QtMWCTools.qo.h
  implement/QtPlotter/QtPlotter.qo.h
  implement/QtPlotter/QtProfile.qo.h
  implement/QtViewer/FileBox.qo.h
  implement/QtViewer/MakeMask.qo.h
  implement/QtViewer/MakeRegion.qo.h
  implement/QtViewer/QtAnnotatorGui.qo.h
  implement/QtViewer/QtCanvasManager.qo.h
  implement/QtViewer/QtCleanPanelGui.qo.h
  implement/QtViewer/QtDataManager.qo.h
  implement/QtViewer/QtDataOptionsPanel.qo.h
  implement/QtViewer/QtDBusViewerAdaptor.qo.h
  implement/QtViewer/QtDisplayDataGui.qo.h
  implement/QtViewer/QtDisplayData.qo.h
  implement/QtViewer/QtDisplayPanelGui.qo.h
  implement/QtViewer/QtDisplayPanel.qo.h
  implement/QtViewer/QtLELEdit.qo.h
  implement/QtViewer/QtMouseToolBar.qo.h
  implement/QtViewer/QtMouseTools.qo.h
  implement/QtViewer/QtMouseToolState.qo.h
  implement/QtViewer/QtPixelCanvas.qo.h
  implement/QtViewer/QtRegionManager.qo.h
  implement/QtViewer/QtViewerBase.qo.h
  implement/QtViewer/QtViewerPrintGui.qo.h
  implement/QtViewer/QtViewer.qo.h
  implement/RegionShapes/DS9FileWriter.qo.h
  implement/RegionShapes/QtNewRegionShape.qo.h
  implement/RegionShapes/QtRegionShapeManager.qo.h
  implement/RegionShapes/QtSingleRegionShape.qo.h
  implement/RegionShapes/RSUtils.qo.h
  )

casa_qt4_wrap_ui( outfiles_ui
  implement/QtAutoGui/adjustmentBottom.ui
  implement/QtAutoGui/checkButtonItem.ui
  implement/QtAutoGui/comboItem.ui
  implement/QtAutoGui/lineEditorItem.ui
  implement/QtAutoGui/minMaxEditorItem.ui
  implement/QtAutoGui/pairItem.ui
  implement/QtAutoGui/pushButtonItem.ui
  implement/QtAutoGui/sliderEditorItem.ui
  implement/QtAutoGui/sliderLabelItem.ui
  implement/QtViewer/printControl.ui
  implement/QtViewer/QtAnimatorGui.ui
  implement/QtViewer/QtAnnotatorGui.ui
  implement/QtViewer/QtDataManager.ui
  implement/QtViewer/QtDataOptionsPanel.ui
  implement/QtViewer/QtRegionMgr.ui
  implement/RegionShapes/QtDS9WriteOptions.ui
  implement/RegionShapes/QtEditRegionShape.ui
  implement/RegionShapes/QtNewRegionShape.ui
  implement/RegionShapes/QtRegionShapeManager.ui
  implement/RegionShapes/QtRSFileLoader.ui
  implement/RegionShapes/QtRSFileSaver.ui
  implement/RegionShapes/QtSingleRegionShape.ui
  )

casa_qt4_add_resources( outfiles_qrc implement/QtAutoGui/QtAutoGui.qrc OPTIONS -name QtAutoGui )
casa_qt4_add_resources( outfiles_qrc implement/QtPlotter/QtPlotter.qrc OPTIONS -name QtPlotter )
casa_qt4_add_resources( outfiles_qrc implement/QtViewer/QtViewer.qrc OPTIONS -name QtViewer )


include_directories( ${CMAKE_CURRENT_BINARY_DIR}/QtAutoGui ) # for generated code

casa_add_library( display
  ${outfiles_qoh} ${outfiles_ui} ${outfiles_qrc}
  implement/Display/AttributeBuffer.cc
  implement/Display/Attribute.cc
  implement/Display/AttValBase.cc
  implement/DisplayCanvas/DefaultWCCoordinateHandler.cc
  implement/DisplayCanvas/DefaultWCSizeControlHandler.cc
  implement/DisplayCanvas/WCAxisLabeller.cc
  implement/DisplayCanvas/WCCoordinateHandler.cc
  implement/DisplayCanvas/WCCSAxisLabeller.cc
  implement/DisplayCanvas/WCCSNLAxisLabeller.cc
  implement/DisplayCanvas/WCDataScaleHandler.cc
  implement/DisplayCanvas/WCLinearScaleHandler.cc
  implement/DisplayCanvas/WCPowerScaleHandler.cc
  implement/DisplayCanvas/WCResampleHandler.cc
  implement/DisplayCanvas/WCSimpleResampleHandler.cc
  implement/DisplayCanvas/WCSizeControlHandler.cc
  implement/Display/ColorConversion.cc
  implement/Display/ColorDistribution.cc
  implement/Display/Colormap.cc
  implement/Display/ColormapDefinition.cc
  implement/Display/ColormapInfo.cc
  implement/Display/ColormapManager.cc
  implement/DisplayDatas/ActiveCaching2dDD.cc
  implement/DisplayDatas/AxesDisplayData.cc
  implement/DisplayDatas/AxesDisplayMethod.cc
  implement/DisplayDatas/CachingDisplayData.cc
  implement/DisplayDatas/CachingDisplayMethod.cc
  implement/DisplayDatas/DDDEllipse.cc
  implement/DisplayDatas/DDDHandle.cc
  implement/DisplayDatas/DDDObject.cc
  implement/DisplayDatas/DDDPolygon.cc
  implement/DisplayDatas/DDDRectangle.cc
  implement/DisplayDatas/DisplayData.cc
  implement/DisplayDatas/DisplayMethod.cc
  implement/DisplayDatas/DrawingDisplayData.cc
  implement/DisplayDatas/DrawingDisplayMethod.cc
  implement/DisplayDatas/Galaxy.cc
  implement/DisplayDatas/Histogram2dDD.cc
  implement/DisplayDatas/Histogram2dDM.cc
  implement/DisplayDatas/LatticeAsVector2.cc
  implement/DisplayDatas/LatticePADD2.cc
  implement/DisplayDatas/LattPADMVector2.cc
  implement/DisplayDatas/MSAsRaster.cc
  implement/DisplayDatas/NBody.cc
  implement/DisplayDatas/PassiveCachingDD.cc
  implement/DisplayDatas/PassiveTableDD.cc
  implement/DisplayDatas/PrincipalAxesDD.cc
  implement/DisplayDatas/PrincipalAxesDM.cc
  implement/DisplayDatas/Profile2dDD.cc
  implement/DisplayDatas/Profile2dDM.cc
  implement/DisplayDatas/ScrollingRasterDD.cc
  implement/DisplayDatas/ScrollingRasterDM.cc
  implement/DisplayDatas/SkyCatOverlayDD.cc
  implement/DisplayDatas/SkyCatOverlayDM.cc
  implement/DisplayDatas/TblAsContour.cc
  implement/DisplayDatas/TblAsContourDM.cc
  implement/DisplayDatas/TblAsRasterDD.cc
  implement/DisplayDatas/TblAsRasterDM.cc
  implement/DisplayDatas/TblAsXYDD.cc
  implement/DisplayDatas/TblAsXYDM.cc
  implement/DisplayDatas/WedgeDD.cc
  implement/DisplayDatas/WedgeDM.cc
  implement/DisplayDatas/WorldAxesDD.cc
  implement/DisplayDatas/WorldAxesDM.cc
  implement/Display/DisplayEnums.cc
  implement/Display/DisplayMarkerDefinitions.cc
  implement/Display/DisplayParameter.cc
  implement/Display/DLFont.cc
  implement/Display/DParameterButton.cc
  implement/Display/DParameterChoice.cc
  implement/Display/DParameterColorChoice.cc
  implement/Display/DParameterFontChoice.cc
  implement/Display/DParameterMapKeyChoice.cc
  implement/Display/DParameterString.cc
  implement/Display/DParameterSwitch.cc
  implement/DisplayEvents/Animator.cc
  implement/DisplayEvents/AniPosEH.cc
  implement/DisplayEvents/DDModEvent.cc
  implement/DisplayEvents/DisplayDataEvent.cc
  implement/DisplayEvents/DisplayEvent.cc
  implement/DisplayEvents/DisplayTool.cc
  implement/DisplayEvents/DTVisible.cc
  implement/DisplayEvents/MultiWCTool.cc
  implement/DisplayEvents/MWCAnimator.cc
  implement/DisplayEvents/MWCCrosshairTool.cc
  implement/DisplayEvents/MWCPannerTool.cc
  implement/DisplayEvents/MWCPolylineTool.cc
  implement/DisplayEvents/MWCPolyTool.cc
  implement/DisplayEvents/MWCPTRegion.cc
  implement/DisplayEvents/MWCRectTool.cc
  implement/DisplayEvents/MWCRTRegion.cc
  implement/DisplayEvents/MWCRTZoomer.cc
  implement/DisplayEvents/PCInvisTool.cc
  implement/DisplayEvents/PCITFiddler.cc
  implement/DisplayEvents/PCMotionEH.cc
  implement/DisplayEvents/PCMotionEvent.cc
  implement/DisplayEvents/PCPositionEH.cc
  implement/DisplayEvents/PCPositionEvent.cc
  implement/DisplayEvents/PCRefreshEH.cc
  implement/DisplayEvents/PCRefreshEvent.cc
  implement/DisplayEvents/PCTestPattern.cc
  implement/DisplayEvents/PCTool.cc
  implement/DisplayEvents/PixelCanvasEvent.cc
  implement/DisplayEvents/WCCrosshairTool.cc
  implement/DisplayEvents/WCInvisTool.cc
  implement/DisplayEvents/WCMotionEH.cc
  implement/DisplayEvents/WCMotionEvent.cc
  implement/DisplayEvents/WCPolyTool.cc
  implement/DisplayEvents/WCPositionEH.cc
  implement/DisplayEvents/WCPositionEvent.cc
  implement/DisplayEvents/WCPTRegion.cc
  implement/DisplayEvents/WCRectTool.cc
  implement/DisplayEvents/WCRefreshEH.cc
  implement/DisplayEvents/WCRefreshEvent.cc
  implement/DisplayEvents/WCRTRegion.cc
  implement/DisplayEvents/WCRTZoomer.cc
  implement/DisplayEvents/WCTool.cc
  implement/DisplayEvents/WorldCanvasEvent.cc
  implement/Display/GLPCDisplayList.cc
  implement/Display/GLPCTexture.cc
  implement/Display/GLPixelCanvas.cc
  implement/Display/GLPixelCanvasColorTable.cc
  implement/Display/GLSupport.cc
  implement/Display/MultiWCHolder.cc
  implement/Display/PanelDisplay.cc
  implement/Display/PCVGBuffer.cc
  implement/Display/PixelCanvas.cc
  implement/Display/PixelCanvasColorTable.cc
  implement/Display/PSDriver.cc
  implement/Display/PSPixelCanvas.cc
  implement/Display/PSPixelCanvasColorTable.cc
  implement/Display/PSWorldCanvasApp.cc
  implement/DisplayShapes/Annotations.cc
  implement/DisplayShapes/DisplayShape.cc
  implement/DisplayShapes/DisplayShapeInterface.cc
  implement/DisplayShapes/DisplayShapeWithCoords.cc
  implement/DisplayShapes/DSArrow.cc
  implement/DisplayShapes/DSBasic.cc
  implement/DisplayShapes/DSCircle.cc
  implement/DisplayShapes/DSClosed.cc
  implement/DisplayShapes/DSEllipse.cc
  implement/DisplayShapes/DSLine.cc
  implement/DisplayShapes/DSMarker.cc
  implement/DisplayShapes/DSPixelArrow.cc
  implement/DisplayShapes/DSPixelEllipse.cc
  implement/DisplayShapes/DSPixelMarker.cc
  implement/DisplayShapes/DSPixelPoly.cc
  implement/DisplayShapes/DSPixelPolyLine.cc
  implement/DisplayShapes/DSPixelRectangle.cc
  implement/DisplayShapes/DSPixelText.cc
  implement/DisplayShapes/DSPoly.cc
  implement/DisplayShapes/DSPolyLine.cc
  implement/DisplayShapes/DSRectangle.cc
  implement/DisplayShapes/DSScreenArrow.cc
  implement/DisplayShapes/DSScreenEllipse.cc
  implement/DisplayShapes/DSScreenMarker.cc
  implement/DisplayShapes/DSScreenPoly.cc
  implement/DisplayShapes/DSScreenPolyLine.cc
  implement/DisplayShapes/DSScreenRectangle.cc
  implement/DisplayShapes/DSScreenText.cc
  implement/DisplayShapes/DSSquare.cc
  implement/DisplayShapes/DSText.cc
  implement/DisplayShapes/DSWorldArrow.cc
  implement/DisplayShapes/DSWorldEllipse.cc
  implement/DisplayShapes/DSWorldMarker.cc
  implement/DisplayShapes/DSWorldPoly.cc
  implement/DisplayShapes/DSWorldPolyLine.cc
  implement/DisplayShapes/DSWorldRectangle.cc
  implement/DisplayShapes/DSWorldText.cc
  implement/Display/SlicePanelDisplay.cc
  implement/Display/State.cc
  implement/Display/WCPGFilter.cc
  implement/Display/WorldCanvas.cc
  implement/Display/WorldCanvasHolder.cc
  implement/Display/WorldCanvasPGPlotDriver.cc
  implement/QtAutoGui/QtAutoGui.cc
  implement/QtAutoGui/QtGuiEntry.cc
  implement/QtAutoGui/QtLayout.cc
  implement/QtAutoGui/QtXmlRecord.cc
  implement/QtPlotter/QtCanvas.cc
  implement/QtPlotter/QtMWCTools.cc
  implement/QtPlotter/QtPlotSettings.cc
  implement/QtPlotter/QtPlotter.cc
  implement/QtPlotter/QtProfile.cc
  implement/QtViewer/FileBox.cc
  implement/QtViewer/MakeMask.cc
  implement/QtViewer/MakeRegion.cc
  implement/QtViewer/QtAnnotatorGui.cc
  implement/QtViewer/QtCleanPanelGui.cc
  implement/QtViewer/QtDataManager.cc
  implement/QtViewer/QtDataOptionsPanel.cc
  implement/QtViewer/QtDBusViewerAdaptor.cc
  implement/QtViewer/QtDisplayData.cc
  implement/QtViewer/QtDisplayPanel.cc
  implement/QtViewer/QtDisplayPanelGui.cc
  implement/QtViewer/QtMouseToolBar.cc
  implement/QtViewer/QtMouseTools.cc
  implement/QtViewer/QtMouseToolState.cc
  implement/QtViewer/QtPCColorTable.cc
  implement/QtViewer/QtPixelCanvas.cc
  implement/QtViewer/QtRegionManager.cc
  implement/QtViewer/QtViewerBase.cc
  implement/QtViewer/QtViewer.cc
  implement/QtViewer/QtViewerPrintGui.cc
  implement/RegionShapes/DS9FileReader.cc
  implement/RegionShapes/DS9FileWriter.cc
  implement/RegionShapes/QtNewRegionShape.cc
  implement/RegionShapes/QtRegionShapeManager.cc
  implement/RegionShapes/QtSingleRegionShape.cc
  implement/RegionShapes/RegionShape.cc
  implement/RegionShapes/RegionShapes.cc
  implement/RegionShapes/RSFileReaderWriter.cc
  implement/RegionShapes/RSUtils.cc
  implement/RegionShapes/XMLFileReaderWriter.cc
  implement/Utilities/DisplayOptions.cc
  )

casa_add_executable( display casaplotter apps/casaplotter/casaplotter.cc )
casa_add_executable( display casaviewer apps/casaviewer/casaviewer.cc )
casa_add_assay( display implement/DisplayEvents/test/tDisplayEvent.cc )
casa_add_assay( display implement/DisplayEvents/test/tDisplayTool.cc )
casa_add_assay( display implement/Display/test/dGLDemo.cc )
casa_add_assay( display implement/Display/test/dPSLatticeAsRaster.cc )
casa_add_assay( display implement/Display/test/dPSWorldCanvasApp.cc )
casa_add_assay( display implement/Display/test/tAttribute.cc )
casa_add_assay( display implement/Display/test/tAttVal.cc )
casa_add_assay( display implement/Display/test/tAttValPoi.cc )
casa_add_assay( display implement/QtViewer/test/dQtDataManager.cc )
casa_add_assay( display implement/Utilities/test/tDisplayOptions.cc )
