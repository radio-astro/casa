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

#ifndef SLICER_WORKER_H_
#define SLICER_WORKER_H_

#include <casa/Arrays/Vector.h>
#include <QVector>
#include <QTextStream>

namespace casa {

	class ImageAnalysis;
	class Record;
	class SliceStatistics;

	/**
	 * Responsible for computing the (x,y)-values that represent a
	 * slice cut.
	 */

	class SliceWorker {

	public:
		SliceWorker( int id );
		void setImageAnalysis( ImageAnalysis* analysis );
		void setVertices( const QList<int>& xValues, const QList<int>& yValues,
		                  const QList<double>& xValuesWorld, const QList<double>& yValuesWorld);
		void setAxes( const Vector<Int>& axes );
		void setCoords( const Vector<Int>& coords );
		void setSampleCount( int count );

		void setMethod( const String& method );

		void toAscii( QTextStream& stream, SliceStatistics* statistics ) const;
		void compute();
		int getSegmentCount() const;
		QVector<double> getPixels(int index) const;
		QVector<double> getData( int index, SliceStatistics* statistics ) const;
		virtual ~SliceWorker();

	private:
		SliceWorker( const SliceWorker& other );
		SliceWorker& operator=( const SliceWorker other );

		double getDistance( double x, double y ) const;
		QVector<double> interpolate( double start, double end,
		                             const QVector<double>& values ) const;
		void clearResults();

		QVector<double> getValues( int index, const QVector<double>& pixels, SliceStatistics* statistic ) const;
		void computeSlice( const Vector<double>& xValues,
		                   const Vector<double>& yValues );
		ImageAnalysis* imageAnalysis;
		QList<Record*> sliceResults;
		Vector<Double> verticesX;
		Vector<Double> verticesY;
		Vector<Double> verticesXWorld;
		Vector<Double> verticesYWorld;

		Vector<Int> axes;
		Vector<Int> coords;

		int sampleCount;
		int id;
		String method;
	};

} /* namespace casa */
#endif /* SLICER_H_ */
