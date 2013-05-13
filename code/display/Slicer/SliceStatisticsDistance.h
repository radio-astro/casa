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


#ifndef SLICESTATISTICSDISTANCE_H_
#define SLICESTATISTICSDISTANCE_H_

#include <display/Slicer/SliceStatistics.h>

namespace casa {

	class SliceStatisticsDistance : public SliceStatistics {
	public:
		SliceStatisticsDistance(SliceStatisticsFactory::AxisXUnits units );
		virtual double getLength(std::pair<double,double> worldStart,
		                         std::pair<double,double> worldEnd,
		                         std::pair<int,int> pixelStart,
		                         std::pair<int,int> pixelEnd) const;
		virtual double getLength( double side1World, double side2World,
		                          double side1Pixel, double side2Pixel ) const;
		virtual double getStart( double value1World, double value2World,
		                         double value1Pixel, double value2Pixel ) const;
		virtual double getEnd( double value1World, double value2World,
		                       double value1Pixel, double value2Pixel ) const;
		virtual QString getLengthLabel() const;
		virtual QVector<double> interpolate( double start, double end,
		                                     const QVector<double>& values ) const;
		virtual void adjustStart( QVector<double>& values, double newStart ) const;
		virtual void storeIncrement( double* incr, QVector<double>& values, int index) const;
		virtual ~SliceStatisticsDistance();
		virtual QVector<double> fromResults( Record* record  )const;
	private:
		double getHypotenuse( double side1, double side2 ) const ;
		double getHypotenuse( double x1, double x2, double y1, double y2 ) const;
	};

} /* namespace casa */
#endif /* SLICESTATISTICSDISTANCE_H_ */
