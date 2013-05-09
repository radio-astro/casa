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
#include <casa/Containers/Record.h>
#include <QtCore/qmath.h>
#include <QDebug>

namespace casa {

	SliceStatisticsDistance::SliceStatisticsDistance(SliceStatisticsFactory::AxisXUnits units):
		SliceStatistics( units ) {

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

	void SliceStatisticsDistance::adjustStart( QVector<double>& values, double newStart ) const {
		int valueCount = values.size();
		for ( int i = 0; i < valueCount; i++ ) {
			values[i] = values[i] + newStart;
		}
	}

	QVector<double> SliceStatisticsDistance::fromResults( Record* record  )const {
		Array<float> distanceArray = record->asArrayFloat("distance");
		QVector<double> distancePixels= getFromArray( distanceArray );
		return distancePixels;
	}

	QVector<double> SliceStatisticsDistance::interpolate( double start, double end,
	        const QVector<double>& values ) const {
		int valueCount = values.size();
		QVector<double> results( valueCount);

		if ( xUnits == SliceStatisticsFactory::PIXEL_UNIT ) {
			results = values;
		} else {
			results = SliceStatistics::interpolate( start, end, values );
			results = convertArcUnits( results );
		}
		return results;
	}



	double SliceStatisticsDistance::getLength( double firstX, double firstY,
	        double secondX, double secondY ) const {
		double distanceX = qAbs( secondX - firstX );
		double distanceY = qAbs( secondY - firstY );
		double distance = getHypotenuse( distanceX, distanceY );
		return distance;
	}

	double SliceStatisticsDistance::getStart( double /*value1World*/, double /*value2World*/,
	        double /*value1Pixel*/, double /*value2Pixel*/ ) const {
		return 0;
	}
	double SliceStatisticsDistance::getEnd( double value1World, double value2World,
	                                        double value1Pixel, double value2Pixel ) const {
		return getLength( value1World, value2World, value1Pixel, value2Pixel );
	}

	void SliceStatisticsDistance::storeIncrement( double* incr,
	        QVector<double>& values, int index) const {
		if ( index >= 0 ) {
			int xCount = values.size();
			if ( xCount > 0 ) {
				*incr = *incr + (values[xCount - 1] - values[0]);
			}
		}
	}

	double SliceStatisticsDistance::getLength(std::pair<double,double> worldStart,
	        std::pair<double,double> worldEnd,
	        std::pair<int,int> pixelStart,
	        std::pair<int,int> pixelEnd) const {
		double distance = 0;

		if ( xUnits == SliceStatisticsFactory::PIXEL_UNIT ) {
			distance = getHypotenuse( pixelStart.first, pixelEnd.first, pixelStart.second, pixelEnd.second );
		} else {
			distance = getHypotenuse( worldStart.first, worldEnd.first, worldStart.second, worldEnd.second );
			distance = convertArcUnits( distance );
		}
		return distance;
	}

	SliceStatisticsDistance::~SliceStatisticsDistance() {
	}

} /* namespace casa */
