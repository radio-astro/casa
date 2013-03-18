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


#include "SliceStatisticsPosition.h"
#include <QVector>
#include <QDebug>

namespace casa {

SliceStatisticsPosition::SliceStatisticsPosition(SliceStatisticsFactory::AxisXUnits units):
	SliceStatistics( units ){
	xPosition = false;

}

double SliceStatisticsPosition::getLength(std::pair<double,double> worldStart,
	std::pair<double,double> worldEnd,
	std::pair<int,int> pixelStart,
	std::pair<int,int> pixelEnd ) const {
	double distance = 0;
	if ( xUnits == SliceStatisticsFactory::RADIAN_UNIT ){
		if ( xPosition ){
			distance = qAbs( worldEnd.first - worldStart.first );
		}
		else {
			distance = qAbs( worldEnd.second - worldStart.second );
		}
	}
	else if ( xUnits == SliceStatisticsFactory::PIXEL_UNIT ){
		if ( xPosition ){
			distance = qAbs( pixelEnd.first - pixelStart.first );
		}
		else {
			distance = qAbs( pixelEnd.second - pixelStart.second );
		}
	}
	else {
		if ( xPosition ){
			double startX = radiansToArcseconds( pixelStart.first );
			double endX = radiansToArcseconds( pixelEnd.first );
			distance = qAbs( endX - startX );
		}
		else {
			double startY = radiansToArcseconds( pixelStart.second );
			double endY = radiansToArcseconds( pixelEnd.second );
			distance = qAbs( endY - startY );
		}
	}
	return distance;
}



QVector<double> SliceStatisticsPosition::interpolate( double start, double end,
			const QVector<double>& values ) const{
	QVector<double> results( values.size());
	if ( xUnits == SliceStatisticsFactory::RADIAN_UNIT ){
		results = SliceStatistics::interpolate( start, end, values );
	}
	else if ( xUnits == SliceStatisticsFactory::PIXEL_UNIT ){
		results = values;
	}
	else {
		double startArc = radiansToArcseconds( start );
		double endArc = radiansToArcseconds( end );
		int valueCount = values.size();
		QVector<double> arcValues( valueCount );
		for ( int i = 0; i < valueCount; i++ ){
			arcValues[i] = radiansToArcseconds( values[i] );
		}
		results = SliceStatistics::interpolate( startArc, endArc, arcValues );
	}
	return results;
}

double SliceStatisticsPosition::getLength( double value1World, double value2World,
			double value1Pixel, double value2Pixel ) const {
	double distance = 0;
	if ( xUnits == SliceStatisticsFactory::RADIAN_UNIT ){
		if ( xPosition ){
			distance = value1World;
		}
		else {
			distance = value2World;
		}
	}
	else if ( xUnits == SliceStatisticsFactory::PIXEL_UNIT ){
		if ( xPosition ){
			distance = value1Pixel;
		}
		else {
			distance = value2Pixel;
		}
	}
	else {
		if ( xPosition ){
			distance = radiansToArcseconds( value1World );
		}
		else {
			distance = radiansToArcseconds( value2World );
		}
	}
	return distance;
}

QString SliceStatisticsPosition::getLengthLabel() const {
	QString labelStr;
	if ( xPosition ){
		labelStr.append( "Right Ascension (");
	}
	else {
		labelStr.append( "Declination (");
	}
	labelStr.append( getUnitText() );
	labelStr.append( "):");
	return labelStr;
}

void SliceStatisticsPosition::setXPosition( bool xAxis ){
	xPosition = xAxis;
}

SliceStatisticsPosition::~SliceStatisticsPosition() {
}

} /* namespace casa */
