//# Copyright (C) 1994,1995,1996,1997,1998,1999,2000
//# Associated Universities, Inc. Washington DC, USA.
//#
//# This library is free software; you can redistribute it and/or modify it
//# under the terms of the GNU Library General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or (at your
//# option) any later version.
//#
//# This library is distributed in the hope that it will be useful, but WITHOUT
//# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
//# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
//# License for more details.
//#
//# You should have received a copy of the GNU Library General Public License
//# along with this library; if not, write to the Free Software Foundation,
//# Inc., 675 Massachusetts Ave, Cambridge, MA 02139, USA.
//#
//# Correspondence concerning AIPS++ should be addressed as follows:
//#        Internet email: aips2-request@nrao.edu.
//#        Postal address: AIPS++ Project Office
//#                        National Radio Astronomy Observatory
//#                        520 Edgemont Road
//#                        Charlottesville, VA 22903-2475 USA
//#

#include "SlicePlot.qo.h"

#include <images/Images/ImageInterface.h>
#include <display/Slicer/SliceAxisDraw.h>
#include <imageanalysis/ImageAnalysis/ImageAnalysis.h>
#include <display/Display/DisplayCoordinateSystem.h>
#include <synthesis/MSVis/UtilJ.h>
#include <qwt_plot_curve.h>
#include <QFile>
#include <QDebug>

namespace casa {

	const QString SlicePlot::DISTANCE_AXIS = "Distance";
	const QString SlicePlot::POSITION_X_AXIS = "Position X";
	const QString SlicePlot::POSITION_Y_AXIS = "Position Y";
	const QString SlicePlot::UNIT_X_PIXEL = "Pixels";
	const QString SlicePlot::UNIT_X_ARCSEC = "Arc Seconds";
	const QString SlicePlot::UNIT_X_ARCMIN = "Arc Minutes";
	const QString SlicePlot::UNIT_X_ARCDEG = "Arc Degrees";

	SlicePlot::SlicePlot(QWidget *parent, bool allFunctionality ) :QwtPlot(parent),
		image(  ), imageAnalysis( NULL ), AXIS_FONT_SIZE(8),
		statLayout(NULL), factory(NULL) {
		setCanvasBackground( Qt::white );

		accumulateSlices = false;
		viewerColors = true;
		segmentMarkers = false;
		polylineColorUnit = true;
		sampleCount = 0;
		curveWidth = 1;
		markerSize = 5;
		fullVersion = allFunctionality;
		interpolationMethod = "";
		currentRegionId = -1;
		xAxis = DISTANCE_AXIS;
		xAxisUnits = UNIT_X_PIXEL;
		factory = new SliceStatisticsFactory();
		initAxisFont( QwtPlot::xBottom, getAxisLabel() );

		//Set up the axis
		SliceAxisDraw* xAxisDraw = new SliceAxisDraw();
		SliceAxisDraw* yAxisDraw = new SliceAxisDraw();
		if ( fullVersion ) {
			xAxisDraw->setTickFontSize( AXIS_FONT_SIZE );
			yAxisDraw->setTickFontSize( AXIS_FONT_SIZE );
		}
		setAxisScaleDraw( QwtPlot::yLeft, yAxisDraw );
		setAxisScaleDraw( QwtPlot::xBottom, xAxisDraw );

		axes.resize( 2 );
		axes[0] = 0;
		axes[1] = 1;

		setMouseTracking( true );
	}

	void SlicePlot::initAxisFont( int axisId, const QString& axisTitle ) {
		if ( fullVersion ) {
			QwtText axisLabel( axisTitle );
			QFont axisFont = this->axisFont( axisId );
			axisFont.setPointSize( AXIS_FONT_SIZE );
			axisFont.setBold( true );
			axisLabel.setFont( axisFont );
			setAxisTitle( axisId, axisLabel );
		}
	}

//-----------------------------------------------------------------------
//                      Color Management
//-----------------------------------------------------------------------

	void SlicePlot::setViewerCurveColor( int regionId, const QString& colorName) {
		//We need to store the colors in the map so we have them if we
		//are toggling back and forth between viewer/non-viewer color mode.
		ImageSlice* slice = getSlicerFor( regionId);
		slice->setViewerCurveColor( colorName );
		if ( viewerColors ) {
			replot();
		}
	}

	void SlicePlot::resetCurveColors( bool viewerColors, bool polylineColorUnit,
	                                  QList<QColor> accumulateCurveColors ) {
		//Store the new colors
		int accumulatedColorCount = accumulateCurveColors.size();
		curveColors.clear();
		for ( int i = 0; i < accumulatedColorCount; i++ ) {
			curveColors.append( accumulateCurveColors[i] );
		}

		this->polylineColorUnit = polylineColorUnit;
		for ( QMap<int,ImageSlice*>::iterator it = sliceMap.begin(); it != sliceMap.end(); ++it ) {
			(*it)->setPolylineColorUnit( polylineColorUnit );
		}
		setUseViewerColors( viewerColors );
	}

	void SlicePlot::setUseViewerColors( bool viewerColors ) {
		this->viewerColors = viewerColors;
		for ( QMap<int,ImageSlice*>::iterator it = sliceMap.begin(); it != sliceMap.end(); ++it ) {
			(*it)->setUseViewerColors( viewerColors );
		}
		resetExistingCurveColors();
	}

	int SlicePlot::assignCurveColors( int initialColorIndex, int regionId ) {
		int curveColorCount = sliceMap[regionId]->getColorCount();
		int colorCount = curveColors.size();
		if ( colorCount > 0 ) {
			QList<QColor> colorList;
			if ( initialColorIndex >= 0 ) {
				for ( int j = 0; j < curveColorCount; j++ ) {
					int colorIndex = j + initialColorIndex;
					colorIndex = colorIndex % colorCount;
					colorList.append( curveColors[colorIndex]);
				}
			}
			sliceMap[regionId]->setCurveColor( colorList );
		}
		return curveColorCount;
	}

	void SlicePlot::resetExistingCurveColors() {
		//Update the colors in the curves;
		int keyCount = sliceMap.size();
		QList<int> keys = sliceMap.keys();
		int usedColorCount = 0;
		for ( int i = 0; i < keyCount; i++ ) {
			int curveColorCount = assignCurveColors( usedColorCount, keys[i]);
			usedColorCount = usedColorCount + curveColorCount;
		}
		if ( keyCount >= 1 ) {
			replot();
		}
	}

	int SlicePlot::getColorIndex( int regionId ) const {
		int usedColors = -1;
		if ( !sliceMap.isEmpty()) {
			QList<int> keys = sliceMap.keys();
			int colorCount = curveColors.size();
			if ( colorCount > 0 ) {
				usedColors = 0;
				//Note:  we are traversing the map backward because new slices
				//get inserted at the front of the map.  We want the old slices
				//to retain their colors, and this also ensures newly inserted
				//slices don't get the same color as the last inserted slice.
				QListIterator<int> iter(keys);
				iter.toBack();
				while ( iter.hasPrevious() ) {
					int key = iter.previous();
					if ( key == regionId ) {
						break;
					}
					usedColors = usedColors + sliceMap[key]->getColorCount();
				}
				usedColors = usedColors % colorCount;
			}
		}
		return usedColors;
	}

//--------------------------------------------------------------------------------------
//              Properties
//--------------------------------------------------------------------------------------


	void SlicePlot::setAccumulateSlices( bool accumulate ) {
		if ( accumulateSlices != accumulate ) {
			accumulateSlices = accumulate;
			clearCurvesAll();
			resetCurves();
		}
	}

	void SlicePlot::segmentMarkerVisibilityChanged( bool visible ) {
		segmentMarkers = visible;
		for ( QMap<int,ImageSlice*>::iterator it = sliceMap.begin(); it != sliceMap.end(); ++it ) {
			(*it)->setShowCorners( segmentMarkers );
		}
		replot();
	}

	void SlicePlot::clearCurvesAll() {
		QList<int> keys = sliceMap.keys();
		int keyCount = keys.size();
		for( int i = 0; i < keyCount; i++ ) {
			sliceMap[keys[i]]->clearCurve();
			removeStatistic( keys[i]);
		}
		replot();
	}

	void SlicePlot::clearCurves( ) {
		QList<int> keys = sliceMap.keys();
		int keyCount = keys.size();
		for( int i = 0; i < keyCount; i++ ) {
			if ( ! accumulateSlices || !sliceMap[keys[i]]->isSelected()) {
				sliceMap[keys[i]]->clearCurve();
				removeStatistic( keys[i]);
			}
		}
		replot();
	}

	void SlicePlot::setPlotPreferences( int lineWidth, int markerSize ) {
		this->curveWidth = lineWidth;
		this->markerSize = markerSize;
		for ( QMap<int,ImageSlice*>::iterator it = sliceMap.begin(); it != sliceMap.end(); ++it ) {
			(*it)->setPlotPreferences( lineWidth, markerSize );
		}
		replot();
	}

	void SlicePlot::setSampleCount( int count ) {
		if ( sampleCount != count ) {
			sampleCount = count;
			for ( QMap<int,ImageSlice*>::iterator it = sliceMap.begin(); it != sliceMap.end(); ++it ) {
				(*it)->setSampleCount( sampleCount );
			}
			replot();
		}
	}

	void SlicePlot::setInterpolationMethod( const String& method ) {
		if ( interpolationMethod != method ) {
			interpolationMethod = method;
			for ( QMap<int,ImageSlice*>::iterator it = sliceMap.begin(); it != sliceMap.end(); ++it ) {
				(*it)->setInterpolationMethod( method );
			}
			replot();
		}
	}

//-----------------------------------------------------------------------
//             Changing what is displayed on the x-axis
//-----------------------------------------------------------------------

	QString SlicePlot::getAxisLabel() const {
		QString axisLabel = xAxis + " ("+xAxisUnits+")";
		return axisLabel;
	}

	void SlicePlot::setXAxis( const QString& newAxis ) {
		if ( newAxis != xAxis ) {
			xAxis = newAxis;
			initAxisFont( QwtPlot::xBottom, getAxisLabel() );

			SliceStatisticsFactory::AxisXChoice choice = getXAxis();
			factory->setAxisXChoice( choice );
			SliceStatistics* statistics = factory->getStatistics();
			QList<int> regionIds = this->sliceMap.keys();
			QList<int>::iterator idIter = regionIds.begin();
			while ( idIter != regionIds.end() ) {
				sliceMap[ *idIter ]->setStatistics( statistics );
				idIter++;
			}
			replot();
		}
	}


	SliceStatisticsFactory::AxisXUnits SlicePlot::getUnitMode() const {
		SliceStatisticsFactory::AxisXUnits unitMode = SliceStatisticsFactory::PIXEL_UNIT;
		if ( xAxisUnits == UNIT_X_ARCMIN ) {
			unitMode = SliceStatisticsFactory::ARCMIN_UNIT;
		} else if ( xAxisUnits == UNIT_X_ARCSEC ) {
			//Arcseconds.
			unitMode = SliceStatisticsFactory::ARCSEC_UNIT;
		} else if ( xAxisUnits == UNIT_X_ARCDEG ) {
			unitMode = SliceStatisticsFactory::ARCDEG_UNIT;
		}
		return unitMode;
	}

	void SlicePlot::xAxisUnitsChanged( const QString& units ) {
		if ( xAxisUnits != units ) {
			xAxisUnits = units;
			initAxisFont( QwtPlot::xBottom, getAxisLabel() );
			SliceStatisticsFactory::AxisXUnits unitMode = getUnitMode();
			factory->setXUnits( unitMode );
			SliceStatistics* statistics = factory->getStatistics();
			for ( QMap<int,ImageSlice*>::iterator it = sliceMap.begin(); it != sliceMap.end(); ++it ) {
				(*it)->setStatistics( statistics );
			}
			replot();
		}

	}

//-------------------------------------------------------------------
//                      Data Processing
//-------------------------------------------------------------------

	void SlicePlot::setImage( ImageInterface<float>* img ) {
		if ( img != NULL && image.get() != img ) {
			//Reset the image.  The units the image is using may have changed.
			image.reset(img);
			Unit brightnessUnit = image->units();
			String yAxisLabelStr = brightnessUnit.getName();
			initAxisFont( QwtPlot::yLeft, yAxisLabelStr.c_str());

			//Get a new image imageanalysis for calculating slices
			delete imageAnalysis;
			imageAnalysis = new ImageAnalysis( image );
			for ( QMap<int,ImageSlice*>::iterator it = sliceMap.begin(); it != sliceMap.end(); ++it ) {
				(*it)->setImageAnalysis( imageAnalysis );
			}

			//Reset the channel.
			updateChannel( 0 );
		}
		else if ( img == NULL ){
			image.reset();
			delete imageAnalysis;
			imageAnalysis = NULL;
			clearCurvesAll();
			QList<int> sliceKeys = sliceMap.keys();
			for ( int i = 0; i < sliceKeys.size(); i++ ) {
				ImageSlice* slice = sliceMap.take( sliceKeys[i]);
				delete slice;
			}

		}
	}

	ImageSlice* SlicePlot::getSlicerFor( int regionId ) {
		ImageSlice* slice = NULL;
		if ( sliceMap.contains( regionId )) {
			slice = sliceMap[regionId];
		} else {
			slice = new ImageSlice( regionId );
			slice->setImageAnalysis( imageAnalysis );
			slice->setInterpolationMethod( interpolationMethod );
			slice->setSampleCount( sampleCount );
			slice->setAxes( axes );
			slice->setCoords( coords );
			slice->setPlotPreferences( curveWidth, markerSize );
			slice->setUseViewerColors( viewerColors );
			slice->setPolylineColorUnit( polylineColorUnit );
			slice->setShowCorners( segmentMarkers );

			slice->setStatistics( factory->getStatistics());
			sliceMap.insert( regionId, slice );
			if ( !viewerColors ) {
				int colorIndex = getColorIndex( regionId );
				assignCurveColors( colorIndex, regionId );
			}
		}
		return slice;
	}

	void SlicePlot::updatePolyLine(  int regionId, viewer::region::RegionChanges regionChanges,
	                                 const QList<double> & worldX, const QList<double> & worldY,
	                                 const QList<int> &pixelX, const QList<int> & pixelY ) {
		if ( regionChanges == viewer::region::RegionChangeDelete ) {
			deletePolyLine( regionId );
		} else {
			if ( regionChanges == viewer::region::RegionChangeSelected ||
			        regionChanges == viewer::region::RegionChangeCreate ) {
				//Redraw the current selected region in a different color reserving the
				//selected color for this one.
				updateSelectedRegionId( regionId );
			}
			updatePolyLine( regionId, worldX, worldY, pixelX, pixelY );
		}
	}

	void SlicePlot::updatePolyLine(  int regionId, const QList<double>& worldX,
	                                 const QList<double>& worldY, const QList<int> &pixelX,
	                                 const QList<int> & pixelY) {
		if ( imageAnalysis != NULL ) {
			ImageSlice* slice = getSlicerFor( regionId );
			slice->updatePolyLine( pixelX, pixelY, worldX, worldY );
			sliceFinished( regionId);
		}
	}

	void SlicePlot::setRegionSelected( int regionId, bool selected ) {
		if ( sliceMap.contains( regionId )) {
			bool oldSelected = sliceMap[regionId]->isSelected();
			if (oldSelected != selected  ) {
				sliceMap[regionId]->setSelected( selected );
				if ( accumulateSlices  ) {
					resetCurves();
				}
			}
		}
	}

	void SlicePlot::updateSelectedRegionId( int selectedRegionId ) {
		if ( currentRegionId != selectedRegionId ) {
			currentRegionId = selectedRegionId;
		}
	}

	SliceStatisticsFactory::AxisXChoice SlicePlot::getXAxis() const {
		SliceStatisticsFactory::AxisXChoice choice = SliceStatisticsFactory::DISTANCE;
		if ( xAxis == SlicePlot::POSITION_X_AXIS ) {
			choice = SliceStatisticsFactory::X_POSITION;
		} else if ( xAxis == SlicePlot::POSITION_Y_AXIS ) {
			choice = SliceStatisticsFactory::Y_POSITION;
		}
		return choice;
	}

	void SlicePlot::resetCurves() {
		clearCurvesAll( );
		QList<int> keys = sliceMap.keys();
		int keyCount = keys.count();
		for ( int i = 0; i < keyCount; i++ ) {
			addPlotCurve( keys[i]);
		}
		replot();
	}

	void SlicePlot::addPlotCurve( int regionId ) {
		if ( ( accumulateSlices && sliceMap[regionId]->isSelected() ) ||
		        (!accumulateSlices)) {
			sliceMap[regionId]->addPlotCurve( this );
			addStatistic( regionId );
			int colorIndex = getColorIndex ( regionId );
			assignCurveColors( colorIndex, regionId );
		}
	}

	void SlicePlot::sliceFinished( int regionId) {

		clearCurves(  );
		addPlotCurve( regionId );

		replot();
	}

	void SlicePlot::deletePolyLine( int regionId) {
		ImageSlice* slice = sliceMap.take( regionId );
		removeStatistic( regionId );
		delete slice;
		if ( regionId == currentRegionId ) {
			currentRegionId = -1;
		}
		replot();
	}

	void SlicePlot::updateChannel( int channel ) {
		DisplayCoordinateSystem cSys = image->coordinates();
		int coordCount = image->ndim();
		coords.resize( coordCount );
		for ( int i = 0; i < coordCount; i++ ) {
			coords[i] = 0;
		}
		if ( cSys.hasSpectralAxis() ) {
			int index = cSys.spectralAxisNumber();
			IPosition imageShape = image->shape();
			if ( index < static_cast<int>(imageShape.size()) ){
				int spectralCount = imageShape(index);
				if ( channel < spectralCount ){
					coords[index] = channel;
				}
			}
		}
		for ( QMap<int,ImageSlice*>::iterator it = sliceMap.begin(); it != sliceMap.end(); ++it ) {
			(*it)->setCoords( coords );
		}
	}



//----------------------------------------------------------------------
//                    Saving Slices
//----------------------------------------------------------------------

	bool SlicePlot::toAscii( const QString& fileName ) {
		QFile file( fileName );
		bool success = file.open( QIODevice::WriteOnly | QIODevice::Text );
		const QString LINE_END( "\n");
		if ( success ) {
			QTextStream out( &file );
			out.setFieldWidth( 20 );
			QList<int> keys = sliceMap.keys();
			int keyCount = keys.size();
			String imageName = image->name();
			QString title = "1-D Slice(s) for "+QString(imageName.c_str());
			out << title << LINE_END << LINE_END;
			for ( int i = 0; i < keyCount; i++ ) {
				if ( keys[i] == currentRegionId ||
				        (accumulateSlices && sliceMap[keys[i]]->isSelected()) ) {
					sliceMap[keys[i]]->toAscii( out );
					out << LINE_END;
					out.flush();
				}
			}
			file.close();
		}
		return success;
	}

//------------------------------------------------------------------------------
//             Statistics
//------------------------------------------------------------------------------

	void SlicePlot::removeStatistics( ) {
		if ( statLayout != NULL ) {
			QList<int> keyList = sliceMap.keys();
			QList<int>::iterator iter = keyList.begin();
			while( iter != keyList.end()) {
				if ( (!accumulateSlices)||
				        ( accumulateSlices && !sliceMap[*iter]->isSelected() )) {
					removeStatistic(*iter);
				}
				iter++;
			}
		}
	}

	void SlicePlot::removeStatistic( int regionId ) {
		if ( statLayout != NULL ) {
			if ( sliceMap.contains(regionId)) {
				statLayout->removeWidget( sliceMap[regionId] );
				//sliceMap[regionId]->setParent( NULL );
				sliceMap[regionId]->hide();
				statLayout->update();

			}
		}
	}



	void SlicePlot::addStatistic( int regionId ) {
		if ( statLayout != NULL ) {
			if ( sliceMap.contains(regionId)) {
				int widgetIndex = statLayout->indexOf( sliceMap[regionId]);
				if ( widgetIndex < 0 ) {
					statLayout->addWidget( sliceMap[regionId] );
					sliceMap[regionId]->show();
					statLayout->update();
				}
			}
		}
	}

	void SlicePlot::setStatisticsLayout( QLayout* layout ) {
		statLayout = layout;
	}

	void SlicePlot::updatePositionInformation( int id, const QVector<String>& info ) {
		if ( sliceMap.contains( id )) {
			sliceMap[id]->updatePositionInformation( info );
		}
	}

	void SlicePlot::markPositionChanged(int regionId,int segmentIndex,float percentage) {
		emit markerPositionChanged( regionId, segmentIndex, percentage );
	}

	void SlicePlot::markVisibilityChanged(int regionId,bool showMarker) {
		emit markerVisibilityChanged( regionId, showMarker );
	}

	bool SlicePlot::isFullVersion() const {
		return fullVersion;
	}

	SlicePlot::~SlicePlot() {
		QList<int> sliceKeys = sliceMap.keys();
		for ( int i = 0; i < sliceKeys.size(); i++ ) {
			ImageSlice* slice = sliceMap.take( sliceKeys[i]);
			delete slice;
		}
		delete factory;
		delete imageAnalysis;
	}

} /* namespace casa */
