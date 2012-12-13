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

#include "RangePicker.h"
#include <guitools/Histogram/HeightSource.h>
#include <QPainter>
#include <QDebug>
#include <qwt_plot.h>

namespace casa {

RangePicker::RangePicker(){
	reset();
}

void RangePicker::setHeightSource( HeightSource* source ){
	heightSource = source;
}

int RangePicker::getLowerBound() const {
	int value = lowerBound;
	if ( upperBound < lowerBound ){
		value = upperBound;
	}
	return value;
}

int RangePicker::getUpperBound() const {
	int value = upperBound;
	if ( lowerBound > upperBound ){
		value = lowerBound;
	}
	return value;
}

void RangePicker::reset(){
	rangeSet = false;
	lowerBound = 0;
	upperBound = 0;
}

void RangePicker::setBoundaryValues( int minX, int maxX ){
	lowerBound = minX;
	upperBound = maxX;
}

void RangePicker::boundaryLineMoved( const QPoint& pos ){
	int xValue = pos.x();
	if ( !rangeSet ){
		lowerBound = xValue;
		upperBound = xValue;
		rangeSet = true;
	}
	else {
		int lowDistance = qAbs( xValue - lowerBound );
		int highDistance = qAbs( xValue - upperBound );
		if ( lowDistance <= highDistance ){
			lowerBound = xValue;
		}
		else {
			upperBound = xValue;
		}
	}
}

void RangePicker::draw (QPainter* painter, const QwtScaleMap & /*xMap*/,
		const QwtScaleMap& /*yMap*/, const QRect &) const {
	if ( lowerBound != upperBound ){

		//Draw the rectangle
		QColor shadeColor( 200,200,200 );
		shadeColor.setAlpha( 100 );
		int startX = lowerBound;
		if ( upperBound < lowerBound ){
			startX = upperBound;
		}
		int rectHeight = heightSource->getCanvasHeight();
		int rectWidth = qAbs( lowerBound - upperBound );
		QRect rect( startX, 0, rectWidth, rectHeight );
		painter->fillRect( rect , shadeColor );

		//Mark the vertical boundary lines of the rectangle
		QPen oldPen = painter->pen();
		QPen boundaryPen( Qt::black );
		painter->setPen( boundaryPen );
		painter->drawLine( lowerBound, 0, lowerBound, rectHeight );
		painter->drawLine( upperBound, 0, upperBound, rectHeight );
		painter->setPen( oldPen );
	}
}

RangePicker::~RangePicker(){
}
} /* namespace casa */
