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


#ifndef SLICESTATISTICS_H_
#define SLICESTATISTICS_H_
#include <casa/Arrays/Vector.h>
#include <display/Slicer/SliceStatistics.h>
#include <display/Slicer/SliceStatisticsFactory.h>

#include <QString>

namespace casa {

	class Record;

	/**
	 * Computes the angle and distance for a single line segment in the
	 * slice.  Subclasses are used for computation depending on what is
	 * displayed on the X-Axis of the slice and on the units of the X-Axis,
	 */

	class SliceStatistics {
	public:
		SliceStatistics(SliceStatisticsFactory::AxisXUnits units );
		void setXUnits( SliceStatisticsFactory::AxisXUnits units );
		double getAngle( std::pair<int,int> pixelStart,
		                 std::pair<int,int> pixelEnd ) const;
		virtual double getLength(std::pair<double,double> worldStart,
		                         std::pair<double,double> worldEnd,
		                         std::pair<int,int> pixelStart,
		                         std::pair<int,int> pixelEnd ) const = 0;
		virtual double getLength( double value1World, double value2World,
		                          double value1Pixel, double value2Pixel ) const = 0;
		virtual double getStart( double value1World, double value2World,
		                         double value1Pixel, double value2Pixel ) const = 0;
		virtual double getEnd( double value1World, double value2World,
		                       double value1Pixel, double value2Pixel ) const = 0;
		virtual QString getLengthLabel() const = 0;
		virtual QVector<double> interpolate( double start, double end,
		                                     const QVector<double>& values ) const;
		virtual void adjustStart( QVector<double>& values, double start ) const=0;
		QVector<double> convertArcUnits( QVector<double> arcseconds ) const;
		double convertArcUnits( double value ) const;
		virtual QVector<double> fromResults( Record* record  )const = 0;
		virtual void storeIncrement( double* incr, QVector<double>& values, int index) const = 0;
		static QVector<double> getFromArray( const Array<float>& source );
		virtual ~SliceStatistics();
	protected:
		QString getUnitText() const;

		double radiansToArcseconds( double rad ) const;
		SliceStatisticsFactory::AxisXUnits xUnits;
	};

} /* namespace casa */
#endif /* SLICESTATISTICS_H_ */
