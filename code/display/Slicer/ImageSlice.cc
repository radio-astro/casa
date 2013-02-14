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

#include "ImageSlice.h"
#include <display/Slicer/SliceWorker.h>
#include <imageanalysis/ImageAnalysis/ImageAnalysis.h>
#include <QPen>
#include <QDebug>
#include <qwt_plot_curve.h>

namespace casa {

ImageSlice::ImageSlice( int id ): plotCurve( NULL ){
	sliceWorker = new SliceWorker( id );
	xAxisChoice = DISTANCE;
	useViewerColors = true;
}

//------------------------------------------------------------------------
//                         Setters
//-------------------------------------------------------------------------

void ImageSlice::setImageAnalysis( ImageAnalysis* analysis ){
	sliceWorker->setImageAnalysis( analysis );
}

void ImageSlice::setInterpolationMethod( const String& method ){
	sliceWorker->setMethod( method );
	if ( plotCurve != NULL ){
		//We have to rerun the slicer using the new method to feed
		//new data into the curve.
		sliceWorker->run();
		resetPlotCurve();
	}
}

void ImageSlice::setSampleCount( int count ){
	sliceWorker->setSampleCount( count );
	if ( plotCurve != NULL ){
		//We have to rerun the slicer with the new sample count
		//in order to get new data into the curve.
		sliceWorker->run();
		resetPlotCurve();
	}
}

void ImageSlice::setAxes( const Vector<Int>& axes ){
	sliceWorker->setAxes( axes );
}

void ImageSlice::setCoords( const Vector<Int>& coords ){
	sliceWorker->setCoords( coords );
}

void ImageSlice::setAxisXChoice( AxisXChoice choice ){
	xAxisChoice = choice;
}

void ImageSlice::setUseViewerColors( bool viewerColors ){
	useViewerColors = viewerColors;
}

//------------------------------------------------------------------
//                  Color Management
//------------------------------------------------------------------

void ImageSlice::setCurveColor( QColor color ){
	if ( plotCurve != NULL ){
		QColor actualColor = color;
		if ( useViewerColors ){
			actualColor = viewerColor;
		}
		QPen curvePen = plotCurve->pen();
		curvePen.setColor( actualColor );
		plotCurve->setPen( curvePen );
	}
}

QColor ImageSlice::getCurveColor() const {
	QColor curveColor;
	if ( plotCurve != NULL ){
		QPen curvePen = plotCurve->pen();
		curveColor = curvePen.color();
	}
	return curveColor;

}

void ImageSlice::setViewerCurveColor( const QString& colorName ){
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
		setCurveColor( viewerColor );
	}
}



void ImageSlice::toAscii( QTextStream& out ){
	if ( plotCurve != NULL && sliceWorker != NULL ){
		sliceWorker->toAscii( out );
	}
}

void ImageSlice::resetPlotCurve(){
	//Now reset the curve with the new data.
	QColor curveColor = getCurveColor();
	QwtPlot* curvePlot = plotCurve->plot();
	clearCurve();
	addPlotCurve( curvePlot );
	setCurveColor( curveColor );
}

void ImageSlice::updatePolyLine(  const QList<int> &pixelX, const QList<int> & pixelY){
	sliceWorker->setVertices( pixelX, pixelY );
	sliceWorker->run();
}

//------------------------------------------------------------------
//                 Data & Curve Management
//------------------------------------------------------------------

void ImageSlice::addPlotCurve( QwtPlot* plot){
	clearCurve();
	plotCurve = new QwtPlotCurve();
	plotCurve->attach( plot );
	QVector<double> xValues;
	if ( xAxisChoice == DISTANCE ){
		xValues = sliceWorker->getDistances();
	}
	else if ( xAxisChoice == X_POSITION ){
		xValues = sliceWorker->getXPositions();
	}
	else {
		xValues = sliceWorker->getYPositions();
	}
	QVector<double> pixels = sliceWorker->getPixels();
	plotCurve->setData( xValues, pixels );

}


void ImageSlice::clearCurve(){
	if ( plotCurve != NULL ){
		plotCurve->detach();
		delete plotCurve;
		plotCurve = NULL;
	}
}

ImageSlice::~ImageSlice() {
	delete sliceWorker;
	if ( plotCurve != NULL ){
		plotCurve->detach();
		delete plotCurve;
	}
}


} /* namespace casa */
