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

#include "ExternalAxisWidgetVertical.h"
#include <qwt_scale_div.h>
#include <QDebug>

namespace casa {

ExternalAxisWidgetVertical::ExternalAxisWidgetVertical(QWidget* parent, QwtPlot* plot,
		bool leftAxisInternal, bool bottomAxisInternal, bool rightAxisInternal):
		ExternalAxisWidget( parent, plot, leftAxisInternal, bottomAxisInternal,
				rightAxisInternal){
}

ExternalAxisWidgetVertical::~ExternalAxisWidgetVertical() {
	// TODO Auto-generated destructor stub
}

int ExternalAxisWidgetVertical::getStartY() const {
	int canvasHeight = getCanvasHeight();
	int heightDiff =  height() - canvasHeight;
	if ( canvasHeight < MIN_START_Y ){
		heightDiff = MIN_START_Y;
	}
	return heightDiff;
}

int ExternalAxisWidgetVertical::getEndY() const {
	const int MARGIN = 2;
	int bottom = height() - MARGIN;
	if ( bottomAxisInternal ){
		int canvasHeight = getCanvasHeight();
		bottom = height() - canvasHeight - MARGIN;
	}
	return bottom;
}

int ExternalAxisWidgetVertical::getAxisLength() const {
	return getEndY() - getStartY();
}

void ExternalAxisWidgetVertical::drawTicks( QPainter* painter, int tickLength ){

	QwtPlot::Axis axisScale = QwtPlot::yLeft;

	//Figure out how far out to start drawing ticks.
	double startPixelY = getTickStartPixel(QwtPlot::yLeft);

	//Draw the ticks.
	QwtScaleDiv* scaleDiv = plot->axisScaleDiv( axisScale );
	double lowerBound = scaleDiv->lowerBound();
	double upperBound = scaleDiv->upperBound();
	int tickCount = 10;
	double tickDistance = (upperBound - lowerBound) / tickCount;
	double pixelDistance = (getCanvasHeight()*1.0) / tickCount;
	for ( int i = 0; i < tickCount; i++ ){
		double tickPosition = startPixelY + pixelDistance*i;
		double tickValue = upperBound - i * tickDistance;
		drawTick( painter, tickPosition, tickValue, tickLength );
	}
}



} /* namespace casa */
