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
#include <display/Slicer/ImageSlice.h>
#include <display/Slicer/SliceAxisDraw.h>
#include <imageanalysis/ImageAnalysis/ImageAnalysis.h>
#include <synthesis/MSVis/UtilJ.h>
#include <qwt_plot_curve.h>
#include <QFile>
#include <QDebug>

namespace casa {

const QString SlicePlot::DISTANCE_AXIS = "Distance";
const QString SlicePlot::POSITION_X_AXIS = "Position X";
const QString SlicePlot::POSITION_Y_AXIS = "Position Y";

SlicePlot::SlicePlot(QWidget *parent, bool allFunctionality ) :QwtPlot(parent),
		image( NULL ), imageAnalysis( NULL ), AXIS_FONT_SIZE(8) {
	setCanvasBackground( Qt::white );

	sliceCurveColor = Qt::blue;
	accumulateSlices = false;
	viewerColors = true;
	sampleCount = 0;
	fullVersion = allFunctionality;
	interpolationMethod = "";
	currentRegionId = -1;
	xAxis = DISTANCE_AXIS;
	initAxisFont( QwtPlot::xBottom, xAxis );

	//Set up the axis
	SliceAxisDraw* xAxisDraw = new SliceAxisDraw();
	SliceAxisDraw* yAxisDraw = new SliceAxisDraw();
	if ( fullVersion ){
		xAxisDraw->setTickFontSize( AXIS_FONT_SIZE );
		yAxisDraw->setTickFontSize( AXIS_FONT_SIZE );
	}
	setAxisScaleDraw( QwtPlot::yLeft, yAxisDraw );
	setAxisScaleDraw( QwtPlot::xBottom, xAxisDraw );

	axes.resize( 2 );
	axes[0] = 0;
	axes[1] = 1;
}

void SlicePlot::initAxisFont( int axisId, const QString& axisTitle ){
	if ( fullVersion ){
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

void SlicePlot::setViewerCurveColor( int regionId, const QString& colorName ){
	//We need to store the colors in the map so we have them if we
	//are toggling back and forth between viewer/non-viewer color mode.
	ImageSlice* slice = getSlicerFor( regionId );
	slice->setViewerCurveColor( colorName );
	if ( viewerColors ){
		replot();
	}
}

void SlicePlot::resetCurveColors( bool viewerColors, QColor curveColor,
		QList<QColor> accumulateCurveColors ){
	//Store the new colors
	sliceCurveColor = curveColor;
	int accumulatedColorCount = accumulateCurveColors.size();
	accumulatedSliceCurveColors.clear();
	for ( int i = 0; i < accumulatedColorCount; i++ ){
		accumulatedSliceCurveColors.append( accumulateCurveColors[i] );
	}
	setUseViewerColors( viewerColors );
}

void SlicePlot::setUseViewerColors( bool viewerColors ){
	this->viewerColors = viewerColors;
	for ( QMap<int,ImageSlice*>::iterator it = sliceMap.begin(); it != sliceMap.end(); ++it ){
		(*it)->setUseViewerColors( viewerColors );
	}
	resetExistingCurveColors();
}

void SlicePlot::resetExistingCurveColors(){
	//Update the colors in the curves;
	int keyCount = sliceMap.size();
	QList<int> keys = sliceMap.keys();
	for ( int i = 0; i < keyCount; i++ ){
		if ( keys[i] == currentRegionId ){
			sliceMap[keys[i]]->setCurveColor( sliceCurveColor );
		}
		else {
			setCurveColor( keys[i] );
		}
	}
	if ( keyCount >= 1 ){
		replot();
	}
}

int SlicePlot::getColorIndex( int regionId ) const {
	QList<int> keys = sliceMap.keys();
	int colorIndex = keys.indexOf( regionId );
	if ( colorIndex >= 0 ){
		int accumulatedColorCount = accumulatedSliceCurveColors.size();
		colorIndex = colorIndex & accumulatedColorCount;
	}
	return colorIndex;
}

void SlicePlot::setCurveColor( int regionId ){
	if ( accumulatedSliceCurveColors.size() == 0 || !accumulateSlices ){
		sliceMap[regionId]->setCurveColor( sliceCurveColor );
	}
	else {
		int colorIndex = getColorIndex( regionId );
		sliceMap[regionId]->setCurveColor( accumulatedSliceCurveColors[colorIndex]);
	}
}


void SlicePlot::setAccumulateSlices( bool accumulate ){
	if ( accumulateSlices != accumulate ){
		accumulateSlices = accumulate;
		if ( !accumulate ){
			clearCurves();
		}
	}
}

void SlicePlot::clearCurves(){
	QList<int> keys = sliceMap.keys();
	int keyCount = keys.size();
	for( int i = 0; i < keyCount; i++ ){
		sliceMap[keys[i]]->clearCurve();
	}
	replot();
}

void SlicePlot::setSampleCount( int count ){
	if ( sampleCount != count ){
		sampleCount = count;
		for ( QMap<int,ImageSlice*>::iterator it = sliceMap.begin(); it != sliceMap.end(); ++it ){
			(*it)->setSampleCount( sampleCount );
		}
		replot();
	}
}

void SlicePlot::setInterpolationMethod( const String& method ){
	if ( interpolationMethod != method ){
		interpolationMethod = method;
		for ( QMap<int,ImageSlice*>::iterator it = sliceMap.begin(); it != sliceMap.end(); ++it ){
			(*it)->setInterpolationMethod( method );
		}
		replot();
	}
}

void SlicePlot::setXAxis( const QString& newAxis ){
	if ( newAxis != xAxis ){
		xAxis = newAxis;
		initAxisFont( QwtPlot::xBottom, xAxis );
		this->clearCurves();
		if ( accumulateSlices ){
			QList<int> regionIds = this->sliceMap.keys();
			QList<int>::iterator idIter = regionIds.begin();
			while ( idIter != regionIds.end() ){
				sliceFinished( *idIter );
				idIter++;
			}
		}
		else {
			if ( currentRegionId >= 0 ){
				sliceFinished( currentRegionId );
			}
		}
	}
}



//-------------------------------------------------------------------
//                      Data Processing
//-------------------------------------------------------------------

void SlicePlot::setImage( ImageInterface<float>* img ){
	if ( img != NULL && image != img ){
		image = img;
		delete imageAnalysis;
		imageAnalysis = new ImageAnalysis( image );
		for ( QMap<int,ImageSlice*>::iterator it = sliceMap.begin(); it != sliceMap.end(); ++it ){
			(*it)->setImageAnalysis( imageAnalysis );
		}
		updateChannel( 0 );
	}
}

ImageSlice* SlicePlot::getSlicerFor( int regionId ){
	ImageSlice* slice = NULL;
	if ( sliceMap.contains( regionId )){
		slice = sliceMap[regionId];
	}
	else {
		slice = new ImageSlice( regionId );
		slice->setImageAnalysis( imageAnalysis );
		slice->setInterpolationMethod( interpolationMethod );
		slice->setSampleCount( sampleCount );
		slice->setAxes( axes );
		slice->setCoords( coords );
		slice->setUseViewerColors( viewerColors );
		sliceMap.insert( regionId, slice );
	}
	return slice;
}

void SlicePlot::updatePolyLine(  int regionId, viewer::region::RegionChanges regionChanges,
		const QList<double> & /*worldX*/, const QList<double> & /*worldY*/,
		const QList<int> &pixelX, const QList<int> & pixelY ){
	if ( regionChanges == viewer::region::RegionChangeDelete ){
		deletePolyLine( regionId );
	}
	else {
		if ( regionChanges == viewer::region::RegionChangeSelected ){
			//Redraw the current selected region in a different color reserving the
			//selected color for this one.
			updateSelectedRegionId( regionId );
			updatePolyLine( regionId, pixelX, pixelY );
		}
		else if ( regionChanges == viewer::region::RegionChangeNewChannel ){
			bool selected = false;
			if ( regionId == currentRegionId ){
				selected = true;
			}
			updatePolyLine( regionId, pixelX, pixelY, selected );
		}
		else if ( regionChanges == viewer::region::RegionChangeCreate ){
			updatePolyLine( regionId, pixelX, pixelY, false );
		}
	}
}

void SlicePlot::updatePolyLine(  int regionId, const QList<int> &pixelX,
		const QList<int> & pixelY, bool selected ){
	if ( imageAnalysis != NULL ){
		ImageSlice* slice = getSlicerFor( regionId );
		slice->updatePolyLine( pixelX, pixelY );
		sliceFinished( regionId, selected );
	}
}

void SlicePlot::updateSelectedRegionId( int selectedRegionId ){
	if ( currentRegionId != selectedRegionId ){
		if ( accumulateSlices && currentRegionId >= 0 ){
			//Find an unused accumulated color for the previously
			//selected curve.
			setCurveColor( currentRegionId );
		}
		currentRegionId = selectedRegionId;
	}
}

void SlicePlot::sliceFinished( int regionId, bool selected ){
	ImageSlice* slice = getSlicerFor( regionId );
	if ( xAxis == DISTANCE_AXIS ){
		slice->setAxisXChoice( slice->DISTANCE );
	}
	else if ( xAxis == POSITION_X_AXIS ){
		slice->setAxisXChoice( slice->X_POSITION );
	}
	else {
		slice->setAxisXChoice( slice->Y_POSITION );
	}

	if ( !accumulateSlices ){
		clearCurves();
	}

	sliceMap[regionId]->addPlotCurve( this );
	if ( selected ){
		sliceMap[regionId]->setCurveColor( sliceCurveColor );
	}
	else {
		setCurveColor( regionId );
	}
	replot();
}

void SlicePlot::deletePolyLine( int regionId){
	ImageSlice* slice = sliceMap.take( regionId );
	delete slice;
	if ( regionId == currentRegionId ){
		currentRegionId = -1;
	}
	replot();
}

void SlicePlot::updateChannel( int channel ){
	CoordinateSystem cSys = image->coordinates();
	int coordCount = image->ndim();
	coords.resize( coordCount );
	for ( int i = 0; i < coordCount; i++ ){
		coords[i] = 0;
	}
	if ( cSys.hasSpectralAxis() ){
		int index = cSys.spectralAxisNumber();
		coords[index] = channel;
	}
	for ( QMap<int,ImageSlice*>::iterator it = sliceMap.begin(); it != sliceMap.end(); ++it ){
		(*it)->setCoords( coords );
	}
}



//----------------------------------------------------------------------
//                    Saving Slices
//----------------------------------------------------------------------

bool SlicePlot::toAscii( const QString& fileName ){
	QFile file( fileName );
	bool success = file.open( QIODevice::WriteOnly | QIODevice::Text );
	const QString LINE_END( "\n");
	if ( success ){
		QTextStream out( &file );
		out.setFieldWidth( 20 );
		QList<int> keys = sliceMap.keys();
		int keyCount = keys.size();
		String imageName = image->name();
		QString title = "1-D Slice(s) for "+QString(imageName.c_str());
		out << title << LINE_END << LINE_END;
		for ( int i = 0; i < keyCount; i++ ){
			sliceMap[keys[i]]->toAscii( out );
			out << LINE_END;
			out.flush();
		}
		file.close();
	}
	return success;
}


SlicePlot::~SlicePlot() {
	QList<int> sliceKeys = sliceMap.keys();
	for ( int i = 0; i < sliceKeys.size(); i++ ){
		ImageSlice* slice = sliceMap.take( sliceKeys[i]);
		delete slice;
	}
}

} /* namespace casa */
