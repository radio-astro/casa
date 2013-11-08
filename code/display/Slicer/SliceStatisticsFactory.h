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


#ifndef SLICESTATISTICSFACTORY_H_
#define SLICESTATISTICSFACTORY_H_

namespace casa {

	class SliceStatistics;

	/**
	 * Returns SliceStatistics appropriate to what is displayed on the
	 * x-axis and the units being used.
	 */

	class SliceStatisticsFactory {
	public:
		enum AxisXUnits {PIXEL_UNIT, ARCSEC_UNIT, ARCMIN_UNIT, ARCDEG_UNIT };
		enum AxisXChoice {DISTANCE,X_POSITION, Y_POSITION};
		void setAxisXChoice( AxisXChoice choice );
		void setXUnits( AxisXUnits unitMode );
		bool isDistance() const;
		bool isPositionX() const;
		bool isPositionY() const;

		SliceStatistics* getStatistics();
		SliceStatisticsFactory();


		virtual ~SliceStatisticsFactory();
	private:
		SliceStatistics* statisticsDistance;
		SliceStatistics* statisticsPosition;
		AxisXUnits xUnits;
		AxisXChoice xAxis;

	};

} /* namespace casa */
#endif /* SLICESTATISTICSFACTORY_H_ */
