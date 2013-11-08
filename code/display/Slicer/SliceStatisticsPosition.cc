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
#include <casa/Containers/Record.h>
#include <QVector>
#include <QDebug>

namespace casa {

	SliceStatisticsPosition::SliceStatisticsPosition(SliceStatisticsFactory::AxisXUnits units):
		SliceStatistics( units ) {
		xPosition = false;

	}

	void SliceStatisticsPosition::storeIncrement( double* incr,
	        QVector<double>& values, int index) const {
		if ( index < 0 && !values.isEmpty() ) {
			*incr = -1 * values[0];
		}
	}

	double SliceStatisticsPosition::getLength(std::pair<double,double> worldStart,
	        std::pair<double,double> worldEnd,
	        std::pair<int,int> pixelStart,
	        std::pair<int,int> pixelEnd ) const {
		double distance = 0;
		if ( xUnits == SliceStatisticsFactory::PIXEL_UNIT ) {
			if ( xPosition ) {
				distance = qAbs( pixelEnd.first - pixelStart.first );
			} else {
				distance = qAbs( pixelEnd.second - pixelStart.second );
			}
		} else {
			if ( xPosition ) {
				distance = qAbs( worldEnd.first - worldStart.first );
			} else {
				distance = qAbs( worldEnd.second - worldStart.second );
			}
			distance = convertArcUnits( distance );
		}

		return distance;
	}

	QVector<double> SliceStatisticsPosition::fromResults( Record* record  )const {
		Array<float> positionArray;
		if ( xPosition ) {
			positionArray = record->asArrayFloat("xpos");
		} else {
			positionArray = record->asArrayFloat("ypos");
		}
		QVector<double> positionPixels= getFromArray( positionArray );
		return positionPixels;
	}

	void SliceStatisticsPosition::adjustStart( QVector<double>& values,
	        double newStart ) const {
		int count = values.size();
		for ( int i = 0; i < count; i++ ) {
			values[i] = values[i] + newStart;
		}
	}

	QVector<double> SliceStatisticsPosition::interpolate( double start, double end,
	        const QVector<double>& values ) const {
		QVector<double> results( values.size());
		if ( xUnits == SliceStatisticsFactory::PIXEL_UNIT ) {
			results = values;
		} else {
			results = SliceStatistics::interpolate( start, end, values );
			results = convertArcUnits( results );
		}
		return results;
	}

	double SliceStatisticsPosition::getLength( double value1World, double value2World,
	        double value1Pixel, double value2Pixel ) const {
		double distance = 0;
		if ( xUnits == SliceStatisticsFactory::PIXEL_UNIT ) {
			if ( xPosition ) {
				distance = value1Pixel;
			} else {
				distance = value2Pixel;
			}
		} else {
			if ( xPosition ) {
				distance = value1World;
			} else {
				distance = value2World;
			}
			distance = convertArcUnits( distance );
		}

		return distance;
	}

	double SliceStatisticsPosition::getStart( double value1WorldX, double value1WorldY,
	        double /*value2WorldX*/, double /*value2WorldY*/ ) const {
		double start = 0;
		if ( xPosition ) {
			start = value1WorldX;
		} else {
			start = value1WorldY;
		}
		return start;
	}
	double SliceStatisticsPosition::getEnd( double /*value1WorldX*/, double /*value1WorldY*/,
	                                        double value2WorldX, double value2WorldY ) const {
		double end = 0;
		if ( xPosition ) {
			end = value2WorldX;
		} else {
			end = value2WorldY;
		}
		return end;
	}

	QString SliceStatisticsPosition::getLengthLabel() const {
		QString labelStr;
		if ( xPosition ) {
			labelStr.append( "X Position (");
		} else {
			labelStr.append( "Y Position (");
		}
		labelStr.append( getUnitText() );
		labelStr.append( "):");
		return labelStr;
	}

	void SliceStatisticsPosition::setXPosition( bool xAxis ) {
		xPosition = xAxis;
	}

	SliceStatisticsPosition::~SliceStatisticsPosition() {
	}

} /* namespace casa */
