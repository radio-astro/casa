//# Copyright (C) 2005
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
#include "HistogramMarkerPoisson.h"
#include <QPaintEvent>
#include <QPainter>
#include <QDebug>
#include <qwt_plot.h>
#include <qwt_plot_canvas.h>

namespace casa {

HistogramMarkerPoisson::HistogramMarkerPoisson():markerColor(Qt::black){
	setXAxis( QwtPlot::xBottom );
	setYAxis( QwtPlot::yLeft );
	lambdaSpecified = false;
}

void HistogramMarkerPoisson::setColor( QColor color ){
	markerColor = color;
}

void HistogramMarkerPoisson::setLambda( int valueX ){
	lambda = valueX;
	lambdaSpecified = true;
}



void HistogramMarkerPoisson::draw(QPainter* painter, const QwtScaleMap&,
		const QwtScaleMap&, const QRect& rect) const {

	//If there is nothing to draw get out quick.
	if ( !lambdaSpecified ){
		return;
	}

	//Set-up the pen
	const QPen& pen = painter->pen();
	QPen penCopy( pen );
	const int ESTIMATE_SIZE = 3;
	penCopy.setColor( markerColor );
	penCopy.setWidth( ESTIMATE_SIZE );
	painter->setPen(penCopy);

	//Draw a vertical line at lambda
	int height = rect.y() + rect.height();
	painter->drawLine( lambda, 0, lambda, height );
	painter->setPen( pen );
}

HistogramMarkerPoisson::~HistogramMarkerPoisson() {
}

} /* namespace casa */
