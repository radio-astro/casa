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


#ifndef SLICESTATISTICSPOSITION_H_
#define SLICESTATISTICSPOSITION_H_

#include <display/Slicer/SliceStatistics.h>

namespace casa {

	class SliceStatisticsPosition : public SliceStatistics {
	public:
		SliceStatisticsPosition(SliceStatisticsFactory::AxisXUnits units);
		virtual double getLength(std::pair<double,double> worldStart,
		                         std::pair<double,double> worldEnd,
		                         std::pair<int,int> pixelStart,
		                         std::pair<int,int> pixelEnd ) const;
		virtual void adjustStart( QVector<double>& values, double newStart ) const;
		virtual double getLength( double value1World, double value2World,
		                          double value1Pixel, double value2Pixel ) const ;
		virtual double getStart( double value1WorldX, double value1WorldY,
		                         double value2WorldX, double value2WorldY ) const;
		virtual double getEnd( double value1WorldX, double value1WorldY,
		                       double value2WorldX, double value2WorldY ) const;
		virtual void storeIncrement( double* incr, QVector<double>& values, int index) const;
		virtual QString getLengthLabel() const;
		void setXPosition( bool xPosition );
		virtual QVector<double> fromResults( Record* record  )const;
		virtual QVector<double> interpolate( double start, double end,
		                                     const QVector<double>& values ) const;
		virtual ~SliceStatisticsPosition();
	private:
		bool xPosition;
	};

} /* namespace casa */
#endif /* SLICESTATISTICSPOSITION_H_ */
