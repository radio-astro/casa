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

#include "ExternalAxisWidgetLeft.h"
#include <QDebug>
#include <QPainter>
#include <qwt_plot_canvas.h>
#include <qwt_scale_div.h>

namespace casa {

ExternalAxisWidgetLeft::ExternalAxisWidgetLeft(QWidget* parent/*, bool manageTicksManually*/):
	ExternalAxisWidget( parent ){
	setSizePolicy( QSizePolicy::Fixed, QSizePolicy::MinimumExpanding );
	setFixedWidth( AXIS_SMALL_SIDE );
	//manageTicks = manageTicksManually;
}

int ExternalAxisWidgetLeft::getStartY() const {
	return height() - canvas->height();
}

void ExternalAxisWidgetLeft::defineAxis( QLine& line ){
	const int MARGIN = 1;
	int x = width() - MARGIN;;
	int top = getStartY();
	int bottom = height() - 2*MARGIN;
	QPoint firstPt( x, top );
	QPoint secondPt( x, bottom );
	line.setP1( firstPt );
	line.setP2( secondPt );
}

void ExternalAxisWidgetLeft::drawTick( QPainter* painter, int yPixel, double value,
		int tickLength ){

	//Draw the tick
	int xEnd = width();
	int tickStart = xEnd - tickLength;
	painter->drawLine( tickStart, yPixel, xEnd, yPixel  );

	//Draw the tick label
	QString numberStr = QString::number( value );
	QFont font = painter->font();
	QRect fontBoundingRect = QFontMetrics(font).boundingRect( numberStr );
	int labelStart = tickStart - fontBoundingRect.width() - 5;
	int letterHeight = fontBoundingRect.height();
	painter->drawText( labelStart, yPixel + letterHeight/3, numberStr);
}



void ExternalAxisWidgetLeft::drawTicks( QPainter* painter, int tickLength){

	//First tick
	double upperBound = scaleDiv->upperBound();
	int startPixelY = getStartY();
	drawTick( painter, startPixelY, upperBound, tickLength);

	//Middle ticks
	const QList<double> axisTicks = scaleDiv->ticks(QwtPlot::yLeft);
	int originalTickCount = axisTicks.size();
	//We don't want to draw too many ticks so adjust the number
	//of ticks we draw accordingly.
	int tickIncrement = getTickIncrement( originalTickCount, false );
	int yIncrement = canvas->height() / (originalTickCount + 1);
	for ( int i = tickIncrement - 1; i < originalTickCount; i = i + tickIncrement ){
		int tickPosition = startPixelY + (i+1)*yIncrement;
		drawTick( painter, tickPosition, axisTicks[originalTickCount - i - 1],
				tickLength);
	}
}

void ExternalAxisWidgetLeft::drawAxisLabel( QPainter* painter ){
	  QFont font = painter->font();
	  QRect fontBoundingRect = QFontMetrics(font).boundingRect( axisLabel );
	  painter->rotate(-90);
	  int yPosition = fontBoundingRect.height() / 2;
	  int xPosition = -height() + (height() - fontBoundingRect.width())/2;
	  painter->drawText( xPosition, yPosition, fontBoundingRect.width(),
			  fontBoundingRect.height(), Qt::AlignHCenter|Qt::AlignTop, axisLabel);
	  painter->rotate(90);
}

ExternalAxisWidgetLeft::~ExternalAxisWidgetLeft() {
}

} /* namespace casa */
