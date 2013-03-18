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


#include "SliceStatistics.h"
#include <QVector>
#include <QtCore/qmath.h>
#include <QDebug>

namespace casa {

SliceStatistics::SliceStatistics(SliceStatisticsFactory::AxisXUnits units) {
	xUnits = units;

}

double SliceStatistics::radiansToArcseconds( double rad ) const {
	double piValue = qAtan(1) * 4;
	double degrees = 180 * rad / piValue;
	double arcseconds = degrees * 3600;
	return arcseconds;
}

double SliceStatistics::getAngle( std::pair<int,int> pixelStart,
		std::pair<int,int> pixelEnd ) const {
	double distanceX = pixelEnd.first - pixelStart.first;
	double distanceY = pixelEnd.second - pixelStart.second;
	double distanceSquared = qPow(distanceX, 2) + qPow( distanceY, 2);
	double hypotenuse = qPow( distanceSquared, 0.5);
	double angle = qAsin( qAbs(distanceX) / hypotenuse );
	if ( ( distanceX * distanceY ) > 0 ){
		double piValue = qAtan(1)*4;
		angle = piValue/2 + angle;
	}
	return angle;
}

QVector<double> SliceStatistics::interpolate( double start, double end,
		const QVector<double>& values ) const {
	double min = start;
	double max = end;
	if ( min > max ){
		min = end;
		max = start;
	}

	//Find the data min and max;
	double minValue = std::numeric_limits<double>::max();
	double maxValue = std::numeric_limits<double>::min();
	int valueCount = values.size();
	for ( int i = 0; i < valueCount; i++ ){
		if ( values[i] < minValue ){
			minValue = values[i];
		}
		if ( values[i] > maxValue ){
			maxValue = values[i];
		}
	}

	//Use a proportion to set the data withen min/max.
	double span = max - min;
	double valueSpan = maxValue - minValue;
	QVector<double> results(valueCount);
	for ( int i = 0; i <values.size(); i++ ){
		results[i] = (values[i] - minValue)*span / valueSpan + min;
	}
	return results;
}

QVector<double> SliceStatistics::adjustStart( double /*newStart*/, const QVector<double>& values ) const {
	return values;
}

QString SliceStatistics::getUnitText() const {
	QString unitText;
	if ( xUnits == SliceStatisticsFactory::RADIAN_UNIT ){
		unitText.append( "Radians");
	}
	else if ( xUnits == SliceStatisticsFactory::PIXEL_UNIT ){
		unitText.append( "Pixels");
	}
	else if ( xUnits == SliceStatisticsFactory::ARCSEC_UNIT ){
		unitText.append( "Arcseconds");
	}
	return unitText;
}

SliceStatistics::~SliceStatistics() {
	// TODO Auto-generated destructor stub
}

} /* namespace casa */
