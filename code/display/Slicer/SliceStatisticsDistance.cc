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


#include "SliceStatisticsDistance.h"
#include <QtCore/qmath.h>
#include <QDebug>

namespace casa {

SliceStatisticsDistance::SliceStatisticsDistance(SliceStatisticsFactory::AxisXUnits units):
	SliceStatistics( units ){

}

double SliceStatisticsDistance::getHypotenuse( double side1, double side2 ) const {
	double distanceSquared = qPow( side1, 2) + qPow( side2, 2 );
	double hypotenuse = qPow( distanceSquared, 0.5 );
	return hypotenuse;
}

double SliceStatisticsDistance::getHypotenuse( double x1, double x2,
		double y1, double y2 ) const {
	double xDistance = qAbs( x1 - x2 );
	double yDistance = qAbs( y1 - y2 );
	return getHypotenuse( xDistance, yDistance );
}

QString SliceStatisticsDistance::getLengthLabel() const {
	QString labelText( "Distance (");
	labelText.append( getUnitText() );
	labelText.append( "):");
	return labelText;
}

QVector<double> SliceStatisticsDistance::adjustStart( double newStart,
		const QVector<double>& values ) const {
	int valueCount = values.size();
	QVector<double> adjustedValues( valueCount );
	for ( int i = 0; i < valueCount; i++ ){
		if ( xUnits == SliceStatisticsFactory::PIXEL_UNIT ){
			adjustedValues[i] = values[i] + newStart;
		}
		else {
			adjustedValues[i] = values[i];
		}
	}
	return adjustedValues;
}

QVector<double> SliceStatisticsDistance::interpolate( double start, double end,
			const QVector<double>& values ) const{
	int valueCount = values.size();
	QVector<double> results( valueCount);
	if ( xUnits == SliceStatisticsFactory::RADIAN_UNIT ){
		results = SliceStatistics::interpolate( start, end, values );
	}
	else if ( xUnits == SliceStatisticsFactory::PIXEL_UNIT ){
		results = SliceStatistics::interpolate( start, end, values );
	}
	else {
		QVector<double> arcSecValues( valueCount );
		for ( int i = 0; i < valueCount; i++ ){
			arcSecValues[i] = radiansToArcseconds(values[i]);
		}
		double startArc = radiansToArcseconds( start );
		double endArc = radiansToArcseconds( end );
		results = SliceStatistics::interpolate( startArc, endArc, arcSecValues );
	}
	return results;
}

double SliceStatisticsDistance::getLength( double side1World, double side2World,
	double side1Pixel, double side2Pixel ) const {
	double distance = 0;
	if ( xUnits == SliceStatisticsFactory::RADIAN_UNIT ){
		distance = getHypotenuse( side1World, side2World);
	}
	else if ( xUnits == SliceStatisticsFactory::PIXEL_UNIT ){
		distance = getHypotenuse( side1Pixel, side2Pixel );
	}
	else {
		distance = getHypotenuse( radiansToArcseconds( side1World),
				radiansToArcseconds(side2World));
	}
	return distance;
}



double SliceStatisticsDistance::getLength(std::pair<double,double> worldStart,
		std::pair<double,double> worldEnd,
		std::pair<int,int> pixelStart,
		std::pair<int,int> pixelEnd) const {
	double distance = 0;
	if ( xUnits == SliceStatisticsFactory::RADIAN_UNIT ){
		distance = getHypotenuse( worldStart.first, worldEnd.first, worldStart.second, worldEnd.second );
	}
	else if ( xUnits == SliceStatisticsFactory::PIXEL_UNIT ){
		distance = getHypotenuse( pixelStart.first, pixelEnd.first, pixelStart.second, pixelEnd.second );
	}
	else {
		double x1 = radiansToArcseconds( worldStart.first );
		double x2 = radiansToArcseconds( worldEnd.first );
		double y1 = radiansToArcseconds( worldStart.second );
		double y2 = radiansToArcseconds( worldEnd.second );
		distance = getHypotenuse( x1, x2, y1, y2 );
	}
	return distance;
}

SliceStatisticsDistance::~SliceStatisticsDistance() {
}

} /* namespace casa */
