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

#include "SegmentTracer.h"
#include <display/Slicer/SlicePlot.qo.h>
#include <qwt_plot_marker.h>
#include <qwt_plot_canvas.h>
#include <qwt_plot.h>
#include <qwt_symbol.h>
#include <QDebug>

namespace casa {

	SegmentTracer::SegmentTracer( int regionId, int index, QwtPlot* plot ):
		QwtPlotPicker( QwtPlot::xBottom, QwtPlot::yLeft,
		               QwtPlotPicker::PointSelection,
		               QwtPlotPicker::NoRubberBand, QwtPlotPicker::AlwaysOn, plot->canvas() ) {
		marker = new QwtPlotMarker();
		QwtSymbol* traceSymbol = new QwtSymbol( QwtSymbol::Star1, QBrush(Qt::red), QPen(Qt::black), QSize(10,10));
		marker->setSymbol( *traceSymbol );
		marker->attach( plot );
		marker->hide();
		slicePlot = plot;
		this->index = index;
		this->regionId = regionId;
	}


	void SegmentTracer::setData( const QVector<double> xVals,
	                             const QVector<double> yVals ) {
		int count = xVals.size();
		xValues.resize( count );
		yValues.resize( count );
		for ( int i = 0; i < count; i++ ) {
			xValues[i] = xVals[i];
			yValues[i] = yVals[i];
		}
	}

	bool SegmentTracer::isBetween( double value, double start, double end ) const {
		bool between = false;
		if ( start <= value && value <= end ) {
			between = true;
		} else if ( end <= value && value <= start ) {
			between = true;
		}
		return between;
	}

	QwtText SegmentTracer::trackerText( const QwtDoublePoint & pos ) const {
		bool found = false;
		int count = xValues.size() - 1;
		for ( int i = 0; i < count; i++ ) {
			if ( isBetween( pos.x(), xValues[i], xValues[i+1]) &&
			        isBetween( pos.y(), yValues[i], yValues[i+1])) {
				//Interpolate the y value to figure out a point on the curve.
				double xVal = pos.x();
				double xDistance = xValues[i+1] - xValues[i];
				double yVal = pos.y();
				if ( xDistance != 0 ) {
					double slope = (yValues[i+1] - yValues[i]) / xDistance;
					yVal = yValues[i] + slope * (xVal - xValues[i]);
				}
				marker->setValue( xVal, yVal );

				float totalLength = qAbs( xValues[count] - xValues[0]);
				float markerLength = qAbs( xVal - xValues[0]);
				float percentage = markerLength / totalLength;
				(dynamic_cast<SlicePlot*>(slicePlot))->markPositionChanged( regionId,index, percentage );
				found = true;

				break;
			}
		}
		SlicePlot* slicerPlot = dynamic_cast<SlicePlot*>(slicePlot);
		if ( slicerPlot->isFullVersion()) {
			bool markerVisible = marker->isVisible();
			if ( markerVisible != found ) {
				if ( found ) {
					marker->show();
				} else {
					marker->hide();
				}
				slicerPlot->markVisibilityChanged( regionId, found );
			}
			slicePlot->replot();
		}
		QwtText blankText;
		return blankText;
	}
	SegmentTracer::~SegmentTracer() {
		// TODO Auto-generated destructor stub
	}

} /* namespace casa */
