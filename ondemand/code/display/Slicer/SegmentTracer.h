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

#ifndef SEGMENTTRACER_H_
#define SEGMENTTRACER_H_

#include <qwt_plot_picker.h>

class QwtPlotMarker;
class QwtPlot;

namespace casa {

	class SegmentTracer : public QwtPlotPicker {
	public:
		SegmentTracer( int regionId, int index, QwtPlot* canvas );
		void setData( const QVector<double> xValues, const QVector<double> yValues );
		virtual QwtText trackerText( const QwtDoublePoint & pos ) const;
		virtual ~SegmentTracer();

	private:
		bool isBetween( double value, double start, double end ) const;
		QwtPlotMarker* marker;
		QwtPlot* slicePlot;
		int regionId;
		int index;
		QVector<double> xValues;
		QVector<double> yValues;
	};

} /* namespace casa */
#endif /* SEGMENTTRACER_H_ */
