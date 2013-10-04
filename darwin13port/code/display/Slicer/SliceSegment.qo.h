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

#ifndef SLICE_SEGMENT_QO_H
#define SLICE_SEGMENT_QO_H

#include <QtGui/QFrame>
#include <casa/BasicSL/String.h>
#include <display/Slicer/SliceSegment.ui.h>
#include <qwt_double_rect.h>

class QwtPlot;
class QwtPlotCurve;

namespace casa {

	/**
	 * Represents a single line segment of an image slice cut.  If
	 * the image slice cut is a polygonal chain, many SliceSegments
	 * make up the image slice cut.  Otherwise, the image slice cut
	 * may be just a single SliceSegment.
	 */

	class SliceStatistics;
	class SegmentTracer;

	class SliceSegment : public QFrame {
		Q_OBJECT

	public:
		SliceSegment(int regionId, int index, QWidget *parent = 0);
		void addCurve( QwtPlot* plot, const QVector<double>& xValues, const QVector<double>& yValues );
		void setEndPointsWorld( double worldX1, double worldY1, double worldX2, double worldY2 );
		void setEndPointsPixel( int pixelX1, int pixelY1, int pixelX2, int pixelY2 );
		void setColor( QColor color );
		void setCurveWidth( int width );
		void updateEnds( const String& start, const String& end);
		QColor getCurveColor() const;
		void clearCurve();
		void updateStatistics( SliceStatistics* statistics );
		QwtPlot* getPlot();
		~SliceSegment();

	signals:
		void statisticsSelected( int index );

	private:
		QString parseEndInfo( const String& info ) const;
		void resetCurveWidth();
		void setCurveColor();
		SliceSegment( const SliceSegment& other );
		SliceSegment operator=( const SliceSegment& other );
		std::pair<double,double> worldStart;
		std::pair<double,double> worldEnd;
		std::pair<int,int> pixelStart;
		std::pair<int,int> pixelEnd;
		int curveWidth;
		int regionId;
		int index;

		QColor defaultColor;
		QwtPlotCurve* plotCurve;
		SegmentTracer* segmentTracer;
		Ui::SliceSegmentClass ui;
	};

}

#endif // SLICESEGMENT_QO_H
