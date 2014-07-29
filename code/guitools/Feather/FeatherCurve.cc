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
#include <guitools/Feather/LegendCurve.h>
#include <msvis/MSVis/UtilJ.h>
#include <QDebug>
#include <QtCore/qmath.h>
#include <qwt_plot_curve.h>
#include <qwt_symbol.h>
#include <qwt_legend_item.h>

namespace casa {

FeatherCurve::FeatherCurve(FeatherPlot* plot, QwtPlot::Axis xAxis,
		QwtPlot::Axis yAxis, bool scaledCurve ){
	plotCurve = new LegendCurve( this );
	plotCurve->setAxis( xAxis, yAxis );
	plotCurve->attach( plot );
	scatterPlot = false;
	scaleLogUV = false;
	firstTime = true;
	scaleLogAmplitude = false;
	sumCurve = scaledCurve;
	functionColor = Qt::black;
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

bool FeatherCurve::isSumCurve() const {
	return sumCurve;
}

bool FeatherCurve::isWeightCurve() const {
	bool weightCurve = false;
	QwtPlot::Axis verticalAxis = getVerticalAxis();
	if ( verticalAxis == QwtPlot::yRight ){
		weightCurve = true;
	}
	return weightCurve;
}

QColor FeatherCurve::getRectColor() const {
	return functionColor;
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
			Bool valid = true;
			minValue = logarithm( minValue, valid );
			maxValue = logarithm( maxValue, valid );
		}
	}
	else {
		if ( scaleLogAmplitude && !isSumCurve()){
			Bool valid = true;
			minValue = logarithm( minValue, valid );
			maxValue = logarithm( maxValue, valid );
		}
	}
	return std::pair<double,double>(minValue,maxValue);
}


std::pair<double,double> FeatherCurve::getBoundsY() const {
	double minValue = minY;
	double maxValue = maxY;
	if ( scaleLogAmplitude ){
		if ( !isWeightCurve() && !isSumCurve()){
			Bool valid = true;
			minValue = logarithm( minValue, valid );
			maxValue = logarithm( maxValue, valid );
		}
	}
	return std::pair<double,double>(minValue,maxValue);
}

QwtPlot::Axis FeatherCurve::getVerticalAxis() const {
	QwtPlot::Axis verticalAxis = static_cast<QwtPlot::Axis>( plotCurve->yAxis());
	return verticalAxis;
}


QVector<double> FeatherCurve::getXValues() const{
	return xValues;
}
QVector<double> FeatherCurve::getYValues() const {
	return yValues;
}

void FeatherCurve::setCurveData( const QVector<double>& xVals, const QVector<double>& yVals ){
	xValues.resize( xVals.size());
	yValues.resize( yVals.size());
	for ( int i = 0; i < xVals.size(); i++ ){
		xValues[i] = xVals[i];
	}
	for ( int j = 0; j < yVals.size(); j++ ){
		yValues[j] = yVals[j];
	}
	resetDataBounds();
}


void FeatherCurve::adjustData( bool uvLog, bool ampLog ){
	if ( uvLog != scaleLogUV || ampLog != scaleLogAmplitude || firstTime ){
		firstTime = false;
		double* scaledXValues = new double[ xValues.size() ];
		for ( int i = 0; i < xValues.size(); i++ ){
			scaledXValues[i] = xValues[i];
		}
		double* scaledYValues = new double[ yValues.size() ];
		for ( int i = 0; i < yValues.size(); i++ ){
			scaledYValues[i] = yValues[i];
		}
		//Decide whether to use a log scale or not.
		scaleLogUV = uvLog;
		QList<int> badIndices;
		if ( scaleLogUV && !scatterPlot ){
			badIndices = doLogs( scaledXValues, xValues.size() );
		}

		scaleLogAmplitude = ampLog;
		QList<int> badYIndices;
		if ( scaleLogAmplitude ){
			if ( !isWeightCurve() && !isSumCurve()){
				badYIndices = doLogs( scaledYValues, yValues.size() );
				if ( scatterPlot ){
					doLogs( scaledXValues, xValues.size() );
				}
			}
		}

		for ( int i = 0; i < badYIndices.size(); i++ ){
			if ( ! badIndices.contains( badYIndices[i])){
				badIndices.append( badYIndices[i]);
			}
		}

		int scaledSize = xValues.size();
		//Sometimes the curves contain zero values.  We can't take the
		//logarithm of those.  Apparently we are supposed to ignore zero
		//values so we are removing any indices that are "bad", i.e.,
		//zero or negative.
		int totalBad = badIndices.size();
		if ( totalBad > 0 ){
			scaledSize = scaledSize - totalBad;
			double* goodXValues = new double[ scaledSize ];
			double* goodYValues = new double[ scaledSize ];
			int j = 0;
			for ( int i = 0; i < xValues.size(); i++ ){
				if ( ! badIndices.contains( i ) ){
					goodXValues[j] = scaledXValues[i];
					goodYValues[j] = scaledYValues[i];
					j++;
				}
			}
			plotCurve->setData( goodXValues, goodYValues, scaledSize );
		}
		else {
			plotCurve->setData( scaledXValues, scaledYValues, scaledSize );
		}
		delete[] scaledXValues;
		delete[] scaledYValues;
	}
}


double FeatherCurve::logarithm( double value, Bool& okay ) const {
	double logValue = value;
	okay = true;
	const double BASICALLY_ZERO = 0;
	if ( value <= BASICALLY_ZERO ){
		okay = false;
	}
	if ( value > BASICALLY_ZERO ){
		logValue = qLn( value ) / qLn( 10 );
	}
	return logValue;
}


QList<int> FeatherCurve::doLogs( double* values, int count ) const {
	QList<int> badIndices;
	for ( int i = 0; i < count; i++ ){
		double original = values[i];
		Bool okay = true;
		values[i] = logarithm( original, okay );
		if ( !okay ){
			badIndices.append( i );
		}
	}
	return badIndices;
}


void FeatherCurve::resetDataBounds(){
	//x and y values represent distance and amplitude.
	//As such, they should always be positive.  That is
	//why we are using std::numeric_limits<float>::min()
	//As the largest possible initial value.  It is important
	//Not to let them be negative, because we will be taking
	//logs.
	minX = std::numeric_limits<float>::max();
	maxX = std::numeric_limits<float>::min();
	minY = std::numeric_limits<float>::max();
	maxY = std::numeric_limits<float>::min();

	//However, if it is a sum curve, the values will already be logs
	//so negative values are fine.
	if ( sumCurve ){
		maxX = -1 * minX;
		maxY = -1 * minY;
	}
	for ( int i = 0; i < xValues.size(); i++ ){
		float testValue = xValues[i];
		if ( testValue < minX && (testValue > 0 || sumCurve) ){
			minX = testValue;
		}
		if ( testValue > maxX ){
			maxX = testValue;
		}
	}
	for ( int j = 0; j < yValues.size(); j++ ){
		float testValue = yValues[j];
		if ( testValue < minY && (testValue > 0 || sumCurve) ){
			minY = testValue;
		}
		if ( testValue > maxY ){
			maxY = testValue;
		}
	}
}


QString FeatherCurve::getTitle() const{
	return plotCurve->title().text();
}


void FeatherCurve::setCurvePenColor( const QColor& color ){
	functionColor = color;
	QPen curvePen = plotCurve->pen();
	curvePen.setColor( color );
	plotCurve->setPen( curvePen );
}




void FeatherCurve::setFunctionColor( const QColor& color, bool diagonalLine ){
	functionColor = color;
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
