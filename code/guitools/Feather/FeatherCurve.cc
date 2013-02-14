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

#include "FeatherCurve.h"
#include <guitools/Feather/FeatherPlot.h>
#include <synthesis/MSVis/UtilJ.h>
#include <QDebug>
#include <QtCore/qmath.h>
#include <qwt_plot_curve.h>
#include <qwt_symbol.h>

namespace casa {

FeatherCurve::FeatherCurve(FeatherPlot* plot, QwtPlot::Axis xAxis,
		QwtPlot::Axis yAxis){
	plotCurve = new QwtPlotCurve();
	plotCurve->setAxis( xAxis, yAxis );
	plotCurve->attach( plot );

	scatterPlot = false;
	scaleLogUV = false;
	scaleLogAmplitude = false;;

	resetDataBounds();
}


void FeatherCurve::initScatterPlot( int dotSize ){
	plotCurve->setStyle( QwtPlotCurve::Dots );
	QwtSymbol* symbol = new QwtSymbol();
	symbol->setSize( dotSize, dotSize );
	symbol->setStyle( QwtSymbol::XCross );
	plotCurve->setSymbol( *symbol );
	scatterPlot = true;
}


void FeatherCurve::setTitle( const QString& title ){
	plotCurve->setTitle( title );
}


void FeatherCurve::setCurveSize( bool scatterPlot, bool diagonalLine,
		int dotSize, int lineThickness ){
	this->scatterPlot = scatterPlot;
	QPen curvePen = plotCurve->pen();
	QString curveTitle = plotCurve->title().text();
	if ( scatterPlot ){
		if ( !diagonalLine ){
			QwtSymbol symbol = plotCurve->symbol();
			symbol.setSize( dotSize, dotSize );
			plotCurve->setSymbol( symbol );
		}
		//y=x
		else {
			curvePen.setWidth( lineThickness );
			plotCurve->setPen( curvePen );
		}
	}
	else {
		curvePen.setWidth( lineThickness );
		plotCurve->setPen( curvePen );
	}
}


std::pair<double,double> FeatherCurve::getBoundsX() const {
	double minValue = minX;
	double maxValue = maxX;
	if ( !scatterPlot ){
		if ( scaleLogUV ){
			minValue = logarithm( minValue );
			maxValue = logarithm( maxValue );
		}
	}
	else {
		if ( scaleLogAmplitude ){
			minValue = logarithm( minValue );
			maxValue = logarithm( maxValue );
		}
	}
	return std::pair<double,double>(minValue,maxValue);
}


std::pair<double,double> FeatherCurve::getBoundsY() const {
	double minValue = minY;
	double maxValue = maxY;
	if ( scaleLogAmplitude ){
		if ( !isWeightCurve() ){
			minValue = logarithm( minValue );
			maxValue = logarithm( maxValue );
		}
	}
	return std::pair<double,double>(minValue,maxValue);
}


QwtPlot::Axis FeatherCurve::getVerticalAxis() const {
	QwtPlot::Axis verticalAxis = static_cast<QwtPlot::Axis>( plotCurve->yAxis());
	return verticalAxis;
}


bool FeatherCurve::isWeightCurve() const {
	bool weightCurve = false;
	QwtPlot::Axis verticalAxis = getVerticalAxis();
	if ( verticalAxis == QwtPlot::yRight ){
		weightCurve = true;
	}
	return weightCurve;
}


void FeatherCurve::setCurveData( const QVector<double>& xVals, const QVector<double>& yVals ){
	xValues.resize( xVals.size());
	yValues.resize( yVals.size());
	resetDataBounds();
	for ( int i = 0; i < xVals.size(); i++ ){
		xValues[i] = xVals[i];
		if ( xValues[i] < minX ){
			minX = xValues[i];
		}
		if ( xValues[i] > maxX ){
			maxX = xValues[i];
		}
	}
	for ( int j = 0; j < yVals.size(); j++ ){
		yValues[j] = yVals[j];
		if ( yValues[j] < minY ){
			minY = yValues[j];
		}
		if ( yValues[j] > maxY ){
			maxY = yValues[j];
		}
	}
}


void FeatherCurve::adjustData( bool uvLog, bool ampLog ){
	if ( uvLog != scaleLogUV || ampLog != scaleLogAmplitude ){
		//plotCurve->detach();
		double* scaledXValues = new double[ xValues.size() ];
		for ( int i = 0; i < xValues.size(); i++ ){
			scaledXValues[i] = xValues[i];
		}
		double* scaledYValues = new double[ yValues.size() ];
		for ( int i = 0; i < yValues.size(); i++ ){
			scaledYValues[i] = yValues[i];
		}
		//Decide whether to use a log scale or not.
		if ( uvLog != scaleLogUV ){
			scaleLogUV = uvLog;
			if ( scaleLogUV && !scatterPlot ){
				doLogs( scaledXValues, xValues.size() );
			}
		}
		if ( ampLog != scaleLogAmplitude ){
			scaleLogAmplitude = ampLog;
			if ( scaleLogAmplitude ){
				if ( !isWeightCurve()){
					doLogs( scaledYValues, yValues.size() );
				}
				if ( scatterPlot ){
					doLogs( scaledXValues, xValues.size() );
				}
			}
		}
		plotCurve->setData( scaledXValues, scaledYValues, xValues.size() );
		delete[] scaledXValues;
		delete[] scaledYValues;
	}
}


double FeatherCurve::logarithm( double value ) const {
	double logValue = value;
	Assert( value >= 0 );
	if ( value != 0 ){
		logValue = qLn( value ) / qLn( 10 );
	}
	/*else {
		qDebug() << "Got a zero data value";
	}*/
	return logValue;
}


void FeatherCurve::doLogs( double* values, int count ) const {
	for ( int i = 0; i < count; i++ ){
		values[i] = logarithm( values[i]);
	}
}


void FeatherCurve::resetDataBounds(){
	minX = std::numeric_limits<float>::max();
	maxX = std::numeric_limits<float>::min();
	minY = std::numeric_limits<float>::max();
	maxY = std::numeric_limits<float>::min();
}


QString FeatherCurve::getTitle() const{
	return plotCurve->title().text();
}


void FeatherCurve::setCurvePenColor( const QColor& color ){
	QPen curvePen = plotCurve->pen();
	curvePen.setColor( color );
	plotCurve->setPen( curvePen );
}


void FeatherCurve::setFunctionColor( const QColor& color, bool diagonalLine ){
	if ( !scatterPlot ){
		setCurvePenColor( color );
	}
	else {
		if ( diagonalLine ){
			setCurvePenColor( color );
		}
		else {
			QwtSymbol curveSymbol = plotCurve->symbol();
			QPen curvePen = curveSymbol.pen();
			curvePen.setColor( color );
			curveSymbol.setPen( curvePen );
			plotCurve->setSymbol( curveSymbol );
		}
	}
}


FeatherCurve::~FeatherCurve() {
	plotCurve->detach();
	delete plotCurve;
}

} /* namespace casa */
