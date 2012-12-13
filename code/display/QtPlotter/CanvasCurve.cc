//# Copyright (C) 2005
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
#include "CanvasCurve.h"
#include <casa/Quanta/Quantum.h>
#include <display/QtPlotter/conversion/Converter.h>
#include <display/QtPlotter/conversion/ConverterIntensity.h>
#include <components/ComponentModels/Flux.h>
#include <QDebug>
#include <cmath>
#include <complex>

namespace casa {


CanvasCurve::CanvasCurve(){
}


CanvasCurve::CanvasCurve( CurveData cData, ErrorData eData,
			QString legendTitle, QColor cColor, int curveLevel ){
	curveData = cData;
	errorData = eData;
	legend = legendTitle;
	curveColor = cColor;
	curveType = curveLevel;
}


QColor CanvasCurve::getColor() const{
	return curveColor;
}


void CanvasCurve::setColor( QColor color ){
	curveColor = color;
}


QString CanvasCurve::getLegend() const{
	return legend;
}


int CanvasCurve::getCurveType() const {
	return curveType;
}


CurveData CanvasCurve::getCurveData(){
	return curveData;
}


Vector<float> CanvasCurve::getXValues() const{
	int xCount = curveData.size() / 2;
	Vector<float> xValues( xCount );
	for ( int i = 0; i < xCount; i++ ){
		xValues[i] = curveData[2*i];
	}
	return xValues;
}


Vector<float> CanvasCurve::getYValues() const{
	int yCount = curveData.size() / 2;
	Vector<float> yValues( yCount );
	for ( int i = 0; i < yCount; i++ ){
		yValues[i] = curveData[2*i+1];
	}
	return yValues;
}

pair<double,double> CanvasCurve::getRangeFor(double xMin, double xMax, Bool& exists ){
	pair<double,double> yRange;
	exists = false;
	int yCount = curveData.size() / 2;
	for ( int i = 0; i < yCount; i++ ){
		if ( xMin <= curveData[2*i] && curveData[2*i]<= xMax ){
			if ( !exists ){
				yRange.first = curveData[2*i+1];
				yRange.second = curveData[2*i+1];
				exists = true;
			}
			else {
				if ( curveData[2*i+1] < yRange.first ){
					yRange.first = curveData[2*i+1];
				}
				else if ( curveData[2*i+1] > yRange.second ){
					yRange.second = curveData[2*i+1];
				}
			}
		}
	}
	return yRange;
}


Vector<float> CanvasCurve::getErrorValues() const{
	int errorCount = errorData.size();
	Vector<float> errorValues( errorCount );
	for ( int i = 0; i < errorCount; i++ ){
		errorValues[i] = errorData[i];
	}
	return errorValues;
}


ErrorData CanvasCurve::getErrorData(){
	return errorData;
}


void CanvasCurve::setLegend( const QString& legendStr ){
	legend = legendStr;
}


double CanvasCurve::convertValue( double value, double freqValue, const QString& oldDisplayUnits,
		const QString& yUnitDisplay, const QString& xUnits){

	//Frequency value must be in Hertz
	const QString HERTZ = "Hz";
	double freqHertz = freqValue;
	if ( xUnits != HERTZ ){
		Converter* converter = Converter::getConverter( xUnits, HERTZ );
		freqHertz = converter->convert( freqValue );
		delete converter;
	}
	//Convert the y values
 	Vector<float> yValues(1);
 	Vector<float> xValues(1);
 	yValues[0] = value;
 	xValues[0] = freqHertz;
	ConverterIntensity::convert( yValues, xValues, oldDisplayUnits, yUnitDisplay,
			maxValue, maxUnits );
	return yValues[0];
}


void CanvasCurve::scaleYValues( const QString& oldDisplayUnits,
		const QString& yUnitDisplay, const QString& xUnits ){
	//If we are going to fraction of peak, we need to store our old max and units so
	//we can later go back.
	if ( yUnitDisplay == ConverterIntensity::FRACTION_OF_PEAK &&
			oldDisplayUnits != ConverterIntensity::FRACTION_OF_PEAK ){
		storeData( oldDisplayUnits );
	}

	//Convert xValues to Hz
	Vector<float> xValues = getXValues();
	const QString HERTZ = "Hz";
	if ( xUnits != HERTZ ){
		Converter* converter = Converter::getConverter( xUnits, HERTZ );
		for ( int i = 0; i < static_cast<int>(xValues.size()); i++ ){
			xValues[i] = converter->convert( xValues[i] );
		}
		delete converter;
	}

	//Convert the y values
 	Vector<float> yValues = getYValues();
	ConverterIntensity::convert( yValues, xValues, oldDisplayUnits, yUnitDisplay,
			maxValue, maxUnits );

	//Convert the error values
	Vector<float> errorValues = getErrorValues();
	ConverterIntensity::convert( errorValues, xValues,
			oldDisplayUnits, yUnitDisplay, maxErrorValue, maxUnits  );

	//Copy the yvalues and error values back.
	setYValues( yValues );
	setErrorValues( errorValues );
}

void CanvasCurve::setYValues( const Vector<float>& yValues ){
	int maxPoints = max( static_cast<int>(curveData.size() / 2), static_cast<int>(yValues.size()) );
	for ( int i = 0; i < maxPoints; i++ ){
		curveData[2*i+1] = yValues[i];
	}
}

void CanvasCurve::setErrorValues( const Vector<float>& errorValues ){
	int maxPoints = max( static_cast<int>(errorData.size()), static_cast<int>(errorValues.size()) );
	for ( int i = 0; i < maxPoints; i++ ){
		errorData[i] = errorValues[i];
	}
}

double CanvasCurve::getMaxError( ) const {
	int maxPoints = errorData.size();
	double maxValue = 0;
	if ( maxPoints > 0 ){
		maxValue = errorData[0];
		for ( int i = 0; i < maxPoints; i++ ){
			if ( errorData[i] > maxValue ){
				maxValue = errorData[i];
			}
		}
	}
	return maxValue;
}


double CanvasCurve::getMaxY( ) const {
	int maxPoints = curveData.size()/2;
	double maxValue = 0;
	if ( maxPoints >= 1 ){
		maxValue = curveData[1];
		for ( int i = 0; i < maxPoints; i++ ){
			if ( curveData[2*i+1] > maxValue ){
				maxValue = curveData[2*i+1];
			}
		}
	}
	return maxValue;
}


void CanvasCurve::storeData( const QString& oldUnits ){
	//Find the max data value we will scale by.
	maxUnits = oldUnits;
	maxValue = getMaxY(  );
	maxErrorValue = getMaxError();
}


QString CanvasCurve::getToolTip( double x, double y , const double X_ERROR,
		const double Y_ERROR, const QString& xUnit, const QString& yUnit ) const {
	QString toolTipStr;
	int maxPoints = curveData.size() / 2;
	for ( int i = 0; i < maxPoints; i++ ){
		double curveX = curveData[2*i];
		double curveY = curveData[2*i+1];
		if ( fabs(curveX - x )< X_ERROR && fabs(curveY-y) < Y_ERROR ){
			toolTipStr.append( "(" );
			toolTipStr.append(QString::number( curveX ));
			toolTipStr.append( " " +xUnit +", " );
			toolTipStr.append(QString::number( curveY ));
			toolTipStr.append( " " + yUnit+ ")");
			break;
		}
	}
	return toolTipStr;
}


void CanvasCurve::getMinMax(Double& xmin, Double& xmax, Double& ymin,
		Double& ymax, bool plotError ) const {

	int maxPoints = curveData.size() / 2;
	int nErrPoints= errorData.size();

	if (plotError && nErrPoints>0){
		for (int i = 0; i < maxPoints; ++i){
			double dx = curveData[2 * i];
			double dyl = curveData[2 * i + 1] - errorData[i];
			double dyu = curveData[2 * i + 1] + errorData[i];
			xmin = (xmin > dx)  ? dx : xmin;
			xmax = (xmax < dx)  ? dx : xmax;
			ymin = (ymin > dyl) ? dyl : ymin;
			ymax = (ymax < dyu) ? dyu : ymax;
		}
	}
	else {
		for (int i = 0; i < maxPoints; ++i){
			double dx = curveData[2 * i];
			double dy = curveData[2 * i + 1];
			xmin = (xmin > dx) ? dx : xmin;
			xmax = (xmax < dx) ? dx : xmax;
			ymin = (ymin > dy) ? dy : ymin;
			ymax = (ymax < dy) ? dy : ymax;
		}
	}
}

CanvasCurve::~CanvasCurve() {
}

} /* namespace casa */
