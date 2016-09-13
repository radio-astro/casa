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


#include "SliceStatisticsFactory.h"
#include <display/Slicer/SliceStatistics.h>
#include <display/Slicer/SliceStatisticsDistance.h>
#include <display/Slicer/SliceStatisticsPosition.h>
#include <QDebug>

namespace casa {


	SliceStatisticsFactory::SliceStatisticsFactory():
		statisticsDistance(NULL), statisticsPosition(NULL) {
		xUnits = PIXEL_UNIT;
		xAxis = DISTANCE;
	}

	SliceStatistics* SliceStatisticsFactory::getStatistics() {
		SliceStatistics* stats = NULL;
		if ( xAxis == DISTANCE ) {
			if ( statisticsDistance == NULL ) {
				statisticsDistance = new SliceStatisticsDistance( xUnits );
			}
			stats = statisticsDistance;
		} else if ( xAxis == X_POSITION || xAxis == Y_POSITION ) {
			if ( statisticsPosition == NULL ) {
				statisticsPosition = new SliceStatisticsPosition( xUnits );
			}
			bool xPosition = false;
			if ( xAxis == X_POSITION ) {
				xPosition = true;
			}
			dynamic_cast<SliceStatisticsPosition*>(statisticsPosition)->setXPosition( xPosition );
			stats = statisticsPosition;

		}
		return stats;
	}

	void SliceStatisticsFactory::setAxisXChoice( AxisXChoice choice ) {
		xAxis = choice;
	}

	void SliceStatisticsFactory::setXUnits( AxisXUnits unitMode ) {
		xUnits = unitMode;
		if ( statisticsDistance != NULL ) {
			statisticsDistance->setXUnits( xUnits );
		}
		if ( statisticsPosition != NULL ) {
			statisticsPosition->setXUnits( xUnits );
		}
	}

	bool SliceStatisticsFactory::isDistance() const {
		bool distanceUnits = false;
		if ( xAxis == DISTANCE ) {
			distanceUnits = true;
		}
		return distanceUnits;
	}
	bool SliceStatisticsFactory::isPositionX() const {
		bool positionUnits = false;
		if ( xAxis == X_POSITION ) {
			positionUnits = true;
		}
		return positionUnits;
	}
	bool SliceStatisticsFactory::isPositionY() const {
		bool positionUnits = false;
		if ( xAxis == Y_POSITION ) {
			positionUnits = true;
		}
		return positionUnits;
	}

	SliceStatisticsFactory::~SliceStatisticsFactory() {
		delete statisticsDistance;
		delete statisticsPosition;
	}

} /* namespace casa */
