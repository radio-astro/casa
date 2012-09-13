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
#include <QDebug>
#include <cmath>

namespace casa {

const QString CanvasCurve::FRACTION_OF_PEAK = "Fraction of Peak";
const QString CanvasCurve::JY_BEAM = "Jy/beam";

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

ErrorData CanvasCurve::getErrorData(){
	return errorData;
}

void CanvasCurve::setLegend( const QString& legendStr ){
	legend = legendStr;
}

void CanvasCurve::scaleYValues( const QString& oldDisplayUnits, const QString& yUnitDisplay ){
	scaleYValuesCurve( oldDisplayUnits, yUnitDisplay );
	scaleYValuesError( oldDisplayUnits, yUnitDisplay  );
}

void CanvasCurve::scaleYValuesCurve( const QString& oldUnits, const QString& newUnits ){

	QString baseConvertUnits = oldUnits;
	int maxPoints = curveData.size() / 2;
	//Change them back to the original units before converting.
	if ( oldUnits == FRACTION_OF_PEAK ){
		for ( int i = 0; i < maxPoints; i++ ){
			curveData[2*i+1] = percentToValue( curveData[2*i+1]);
		}
		baseConvertUnits = storedUnits;
	}

	if ( newUnits == FRACTION_OF_PEAK ){

		//If we are going to a percent, store the max and units so we can later go
		//back
		storeData( oldUnits );

		//Scale the vector
		for ( int i = 0; i < maxPoints; i++ ){
			curveData[ 2*i+1 ] = valueToPercent( curveData[2*i+1] );
		}
	}
	//Converting between Jy/beam.
	else if ( oldUnits.indexOf( JY_BEAM) > 0  && newUnits.indexOf( JY_BEAM )> 0 ){
		for ( int i = 0; i < maxPoints; i++ ){
			curveData[2*i+1] = Converter::convertJyBeams( oldUnits, newUnits, curveData[2*i+1]);
		}
	}
	else {
		//Use Quanta to convert
		for ( int i = 0; i < maxPoints; i++ ){
			curveData[ 2*i+1 ] = convert( curveData[2*i+1], baseConvertUnits, newUnits );
		}
	}
}


void CanvasCurve::scaleYValuesError( const QString& oldUnits, const QString& newUnits ){

	QString baseConvertUnits = oldUnits;
	int maxPoints = errorData.size();
	if ( maxPoints > 0 ){

		//Change them back to the original units before converting.
		if ( oldUnits == FRACTION_OF_PEAK ){
			for ( int i = 0; i < maxPoints; i++ ){
				errorData[i] = percentToValue( errorData[i]);
			}
			baseConvertUnits = storedUnits;
		}

		if ( newUnits == FRACTION_OF_PEAK ){

			//Scale the vector
			for ( int i = 0; i < maxPoints; i++ ){
				errorData[i] = valueToPercent( errorData[i] );
			}
		}
		//Converting between Jy/beam.
		else if ( oldUnits.indexOf( JY_BEAM) > 0  && newUnits.indexOf( JY_BEAM ) > 0 ){
			for ( int i = 0; i < maxPoints; i++ ){
				errorData[i] = Converter::convertJyBeams( oldUnits, newUnits, errorData[i]);
			}
		}
		else {
			//Use Quanta to convert
			for ( int i = 0; i < maxPoints; i++ ){
				errorData[i] = convert( errorData[i], baseConvertUnits, newUnits );
			}
		}
	}
}




double CanvasCurve::percentToValue( double yValue ) const {
	double convertedYValue = yValue * storedMax;
	return convertedYValue;
}

double CanvasCurve::getMax() const {
	int maxPoints = curveData.size() / 2;
	double maxValue = 0;
	if ( maxPoints >= 1 ){
		maxValue = curveData[1];
		for ( int i = 0; i < maxPoints; i++ ){
			double yValue = curveData[2*i+1];
			if ( yValue > maxValue ){
				maxValue = yValue;
			}
		}
	}
	return maxValue;
}

void CanvasCurve::storeData( const QString& oldUnits ){
	//Find the max data value we will scale by.
	storedUnits = oldUnits;
	storedMax = getMax();

}

double CanvasCurve::valueToPercent( double yValue ) const {
	double convertedYValue = yValue / storedMax;
	return convertedYValue;
}

double CanvasCurve::convert( double yValue, const QString oldUnits, const QString newUnits ) const {

	String oldUnitStr( oldUnits.toStdString() );
	if ( oldUnits == "Kelvin"){
		oldUnitStr = "K";
	}
	Quantity quantity( yValue, oldUnitStr );
	String newUnitStr( newUnits.toStdString() );
	if ( newUnitStr == "Kelvin"){
		newUnitStr = "K";
	}
	quantity.convert( newUnitStr );
	double convertedYValue = quantity.getValue();
	return convertedYValue;
}

double CanvasCurve::convertValue( double value, const QString& oldUnits, const QString& newUnits ) const {
	double convertedValue = value;
	if ( oldUnits != newUnits ){
		if ( oldUnits == FRACTION_OF_PEAK ){
			convertedValue = storedMax * value;
		}
		else if ( newUnits == FRACTION_OF_PEAK ){
			convertedValue = value / storedMax;
		}
		else {
			convertedValue = convert( value, oldUnits, newUnits );
		}
	}
	return convertedValue;
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
	// TODO Auto-generated destructor stub
}

} /* namespace casa */
