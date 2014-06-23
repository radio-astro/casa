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

#include "ExternalAxisWidgetHorizontal.h"
#include <qwt_scale_div.h>
#include <QDebug>

namespace casa {

ExternalAxisWidgetHorizontal::ExternalAxisWidgetHorizontal(QWidget* parent, QwtPlot* plot,
		bool leftAxisInternal, bool bottomAxisInternal,
		bool rightAxisInternal)
	:ExternalAxisWidget( parent, plot, leftAxisInternal, bottomAxisInternal,
			rightAxisInternal){
}

ExternalAxisWidgetHorizontal::~ExternalAxisWidgetHorizontal() {
	// TODO Auto-generated destructor stub
}

int ExternalAxisWidgetHorizontal::getStartX() const {
	int internalAxisWidth = width() - plot->width();
	if ( leftAxisInternal ){
		int canvasWidth = getCanvasWidth();
		internalAxisWidth = width() - canvasWidth;
	}
	return internalAxisWidth;
}

int ExternalAxisWidgetHorizontal::getEndX() const {
	int endX = width();
	if ( rightAxisInternal ){
		endX = getCanvasWidth();
	}
	return endX;
}


int ExternalAxisWidgetHorizontal::getAxisLength() const {
	return getEndX() - getStartX() + MARGIN;
}

void ExternalAxisWidgetHorizontal::drawTicks( QPainter* painter, int tickLength ){

	//Figure out how far out to start drawing ticks.
	double startPixelX = getTickStartPixel(QwtPlot::xBottom)+MARGIN;

	//We don't want to draw too many ticks so adjust the number
	//of ticks we draw accordingly.
	QwtScaleDiv* scaleDiv = plot->axisScaleDiv( QwtPlot::xBottom );
	const QList<double> axisTicks = scaleDiv->ticks(QwtPlot::xBottom);
	int originalTickCount = axisTicks.size();
	int tickIncrement = getTickIncrement( originalTickCount );

	//Now figure out the xIncrement, how far apart the ticks should be.
	double tickDistance = getTickDistance( QwtPlot::xBottom );
	double xIncrement = getTickIncrement( tickDistance, QwtPlot::xBottom );

	for ( int i = 0; i < originalTickCount; i = i + tickIncrement ){
		//Sometimes the automatic tick system puts uneven number of ticks in.
		//Definitely weird - but that is why the incrementCount;
		int incrementCount = qRound((axisTicks[i] - axisTicks[0]) / tickDistance);
		double tickPosition = startPixelX + incrementCount * xIncrement;
		drawTick( painter, tickPosition, axisTicks[i], tickLength);
	}
}


} /* namespace casa */
