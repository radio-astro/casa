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

#include "ImageSlice.qo.h"
#include <display/Slicer/SliceWorker.h>
#include <display/Slicer/SliceSegment.qo.h>
#include <display/Slicer/ImageSliceColorBar.h>
#include <imageanalysis/ImageAnalysis/ImageAnalysis.h>
#include <synthesis/MSVis/UtilJ.h>
#include <QPen>
#include <QDebug>
#include <QtCore/qmath.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_marker.h>
#include <qwt_symbol.h>

namespace casa {

ImageSlice::ImageSlice( int id, QWidget* parent ): QFrame(parent){

	ui.setupUi(this);

	sliceWorker = new SliceWorker( id );
	xAxisChoice = SliceStatisticsFactory::DISTANCE;
	useViewerColors = true;
	showCorners = false;
	polylineUnit = true;
	segmentColors.append(Qt::magenta);

	QHBoxLayout* layout = new QHBoxLayout();
	colorBar = new ImageSliceColorBar( ui.colorBarHolder );
	layout->addWidget( colorBar );
	layout->setContentsMargins(0,0,0,0);
	ui.colorBarHolder->setLayout( layout );

	QButtonGroup* sizeButtons = new QButtonGroup( this );
	sizeButtons->addButton( ui.minimizeButton );
	sizeButtons->addButton( ui.maximizeButton );
	ui.maximizeButton->setChecked( true );
	connect( ui.minimizeButton, SIGNAL(clicked()), this, SLOT(minimizeDisplay()));
	connect( ui.maximizeButton, SIGNAL(clicked()), this, SLOT(maximizeDisplay()));
}

//------------------------------------------------------------------------
//                         Setters
//-------------------------------------------------------------------------

void ImageSlice::setImageAnalysis( ImageAnalysis* analysis ){
	sliceWorker->setImageAnalysis( analysis );
}

void ImageSlice::setInterpolationMethod( const String& method ){
	sliceWorker->setMethod( method );
	if ( segments.size() > 0 ){
		//We have to rerun the slicer using the new method to feed
		//new data into the curve.
		runSliceWorker();
		resetPlotCurve();
	}
}

void ImageSlice::runSliceWorker(){
	sliceWorker->compute();

	clearCorners();

	int segmentCount = sliceWorker->getSegmentCount();

	//Don't think this will happen, but we need to delete segments until
	//we have the right number.
	int currentSegmentCount = segments.size();
	while ( segmentCount < currentSegmentCount ){
		SliceSegment* segment = segments.takeLast();
		removeSegment( segment );
		delete segment;
		currentSegmentCount = segments.size();
	}

	//Add segments until we have the right number
	while ( currentSegmentCount < segmentCount ){
		SliceSegment* sliceSegment = new SliceSegment(this );
		addSegment( sliceSegment );
		segments.append( sliceSegment );
		currentSegmentCount = segments.size();
	}

	resetSegmentColors();
}

void ImageSlice::setSampleCount( int count ){
	sliceWorker->setSampleCount( count );
	if ( segments.size() > 0 ){
		//We have to rerun the slicer with the new sample count
		//in order to get new data into the curve.
		runSliceWorker();
		resetPlotCurve();
	}
}

void ImageSlice::setAxes( const Vector<Int>& axes ){
	sliceWorker->setAxes( axes );
}

void ImageSlice::setCoords( const Vector<Int>& coords ){
	sliceWorker->setCoords( coords );
}

void ImageSlice::setAxisXChoice( SliceStatisticsFactory::AxisXChoice choice ){
	xAxisChoice = choice;
	SliceStatisticsFactory::getInstance()->setAxisXChoice( choice );
	if ( segments.size() > 0 ){
		resetPlotCurve();
		updateSliceStatistics();
	}
}

void ImageSlice::setXUnits( SliceStatisticsFactory::AxisXUnits unitMode ){
	SliceStatisticsFactory::getInstance()->setXUnits( unitMode );
	if ( segments.size() > 0 ){
		resetPlotCurve();
		updateSliceStatistics();
	}
}

void ImageSlice::updateSliceStatistics(){
	QList<SliceSegment*>::iterator statIter = segments.begin();
	while ( statIter != segments.end()){
		(*statIter)->updateStatistics();
		statIter++;
	}
}



//------------------------------------------------------------------
//                  Color Management
//------------------------------------------------------------------
void ImageSlice::setUseViewerColors( bool viewerColors ){
	if ( viewerColors != useViewerColors ){
		useViewerColors = viewerColors;
		if ( useViewerColors ){
			segmentColors.clear();
			segmentColors.append( viewerColor );
		}
	}
}

void ImageSlice::setCurveColor( QList<QColor> colors ){
	if ( !useViewerColors ){
		segmentColors.clear();
		int colorCount = colors.size();
		for( int i = 0; i < colorCount; i++ ){
			segmentColors.append( colors[i]);
		}
	}
	resetSegmentColors();
}

void ImageSlice::resetSegmentColors(){
	int colorCount = segmentColors.size();
	if ( colorCount > 0 ){
		for ( int i = 0; i < segments.size(); i++ ){
			int colorIndex = i % colorCount;
			segments[i]->setColor( segmentColors[colorIndex] );
		}
	}
	colorBar->setColors( segmentColors );
	colorBar->update();
}


void ImageSlice::setViewerCurveColor( const QString& colorName ){
	viewerColor = Qt::black;
	if ( colorName == "green"){
		viewerColor = Qt::green;
	}
	else if ( colorName == "gray"){
		viewerColor = Qt::gray;
	}
	else {
		viewerColor.setNamedColor( colorName );
	}

	if ( useViewerColors ){
		segmentColors.clear();
		segmentColors.append( viewerColor );
		resetSegmentColors();
	}
}

void ImageSlice::setPolylineColorUnit( bool polyline ){
	polylineUnit = polyline;
}

int ImageSlice::getColorCount() const {
	int colorCount = 1;
	if ( !polylineUnit ){
		colorCount = sliceWorker->getSegmentCount();
	}
	return colorCount;
}


void ImageSlice::toAscii( QTextStream& out ){
	if ( segments.size() > 0 && sliceWorker != NULL ){
		sliceWorker->toAscii( out );
	}
}

void ImageSlice::resetPlotCurve(){
	if ( segments.size() > 0 ){
		//Now reset the curve with the new data.
		QwtPlot* curvePlot = segments[0]->getPlot();
		addPlotCurve( curvePlot );
		resetSegmentColors();
	}
}



//------------------------------------------------------------------
//                 Data & Curve Management
//------------------------------------------------------------------

void ImageSlice::addPlotCurve( QwtPlot* plot){
	clearCurve();
	int segmentCount = segments.size();
	int cornerCount = sliceWorker->getSegmentCount();
	int cornerCountPerSegment = 1;
	if ( segmentCount > 0 ){
		cornerCountPerSegment = cornerCount / segmentCount;
	}

	double xIncr = 0;
	for ( int i = 0; i < segmentCount; i++ ){

		SliceSegment* sliceSegment = segments[i];
		QVector<double> xValues;
		if ( xAxisChoice == SliceStatisticsFactory::DISTANCE ){
			xValues = sliceWorker->getDistances( i, xIncr );

		}
		else if ( xAxisChoice == SliceStatisticsFactory::X_POSITION ){
			xValues = sliceWorker->getXPositions( i );
		}
		else {
			xValues = sliceWorker->getYPositions( i );
		}

		QVector<double> pixels = sliceWorker->getPixels( i );
		sliceSegment->addCurve( plot, xValues, pixels );

		//Add the corner points
		int pointCountPerSegment = qFloor(xValues.size() / cornerCountPerSegment);
		for ( int j = 0; j < cornerCountPerSegment; j++ ){
			int cornerIndex = (j+1)*pointCountPerSegment - 1;
			addCorner( xValues[cornerIndex], pixels[cornerIndex], plot );
		}

		int xCount = xValues.size();
		if ( xCount > 0 ){
			xIncr = xValues[xCount - 1] - xValues[0];
		}
	}

}

void ImageSlice::updatePolyLine(  const QList<int>& pixelX,
		const QList<int>& pixelY, const QList<double>& worldX,
		const QList<double>& worldY){
	sliceWorker->setVertices( pixelX, pixelY, worldX, worldY );

	runSliceWorker();

	//Update the segments with new data.
	int segmentCount = segments.size();
	for ( int i = 0; i < segmentCount; i++ ){
		segments[i]->setEndPointsWorld( worldX[i], worldY[i],
						worldX[i+1], worldY[i+1]);
		segments[i]->setEndPointsPixel( pixelX[i], pixelY[i],
						pixelX[i+1], pixelY[i+1]);
		segments[i]->updateStatistics();
	}
}




void ImageSlice::clearCurve(){
	QList<SliceSegment*>::iterator iter = segments.begin();
	while ( iter != segments.end() ){
		(*iter)->clearCurve();
		iter++;
	}
	clearCorners();
}

//---------------------------------------------------------------------
//                  Markers between Segments
//---------------------------------------------------------------------

void ImageSlice::setShowCorners( bool show ){
	if ( show != showCorners ){
		showCorners = show;
		if ( showCorners ){
			resetPlotCurve();
		}
		else {
			clearCorners();
		}
	}
}

void ImageSlice::clearCorners(){
	while ( !segmentCorners.isEmpty() ){
		QwtPlotMarker* corner = segmentCorners.takeLast();
		corner->detach();
		delete corner;
	}
}

void ImageSlice::addCorner( double xValue, double yValue, QwtPlot* plot ){
	if ( showCorners ){
		QwtSymbol* sym = new QwtSymbol( QwtSymbol::Cross, QBrush(Qt::black), QPen( Qt::black), QSize(5,5));
		QwtPlotMarker* corner = new QwtPlotMarker();
		corner->setSymbol( *sym );
		corner->setValue( xValue, yValue );
		corner->attach( plot );
		segmentCorners.append( corner );
	}
}

//-----------------------------------------------------------------------------
//               Statistics
//-----------------------------------------------------------------------------

void ImageSlice::addSegment( SliceSegment* segment ){
	QLayout* layout = ui.segmentHolder->layout();
	if ( layout == NULL ){
		layout = new QHBoxLayout();
		ui.segmentHolder->setLayout( layout );
	}
	QHBoxLayout* hLayout = dynamic_cast<QHBoxLayout*>(layout);
	hLayout->addWidget( segment );
}

void ImageSlice::removeSegment( SliceSegment* segment ){
	QLayout* layout = ui.segmentHolder->layout();
	if ( layout != NULL ){
		layout->removeWidget( segment );
	}
}


void ImageSlice::minimizeDisplay(){
	QLayout* statLayout = this->layout();
	statLayout->removeWidget( ui.regionInfoWidget );
	ui.regionInfoWidget->setParent( NULL );
}

void ImageSlice::maximizeDisplay(){
	QLayout* statLayout = this->layout();
	statLayout->addWidget( ui.regionInfoWidget );
}



ImageSlice::~ImageSlice() {
	if ( ui.minimizeButton->isChecked()){
		delete ui.regionInfoWidget;
	}
	delete sliceWorker;
	clearCorners();
	clearCurve();
}


} /* namespace casa */
