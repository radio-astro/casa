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

#include "ExternalAxisWidgetTop.h"
#include <QDebug>
#include <QPainter>
#include <qwt_plot.h>
#include <qwt_plot_canvas.h>
#include <qwt_scale_div.h>
#include <casaqt/QwtPlotter/QPCanvasHelpers.qo.h>

namespace casa {

ExternalAxisWidgetTop::ExternalAxisWidgetTop( QWidget* parent, QwtPlot* plot ):
	ExternalAxisWidget( parent, plot ){
	//setFixedHeight( AXIS_SMALL_SIDE / 2 );
	setMinimumHeight( AXIS_SMALL_SIDE / 2 );
	scaleDraw = new QPScaleDraw( NULL, QwtPlot::xTop );
	setSizePolicy( QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding );

}

int ExternalAxisWidgetTop::getStartX() const {
	int internalAxisWidth = 0;
	QwtPlotCanvas* plotCanvas = plot->canvas();
	if ( plotCanvas != NULL ){
		internalAxisWidth = width() - plotCanvas->width();
	}
	return internalAxisWidth;
}

void ExternalAxisWidgetTop::defineAxis( QLine& axisLine ){
	int axisStart = getStartX();
	int left = MARGIN + axisStart;
	int right = width() - MARGIN;
	int top = height() - 1;
	int bottom = height() - 1;
	QPoint firstPt( left, top );
	QPoint secondPt( right, bottom );
	axisLine.setP1( firstPt );
	axisLine.setP2( secondPt );
}

void ExternalAxisWidgetTop::drawTick( QPainter* painter, double xPixel, double value,
		int tickLength ){
	int yEnd = height();
	int xPosition = static_cast<int>(xPixel);
	painter->drawLine( xPosition, yEnd, xPosition, yEnd - tickLength  );
	QFont font = painter->font();
	QString tickLabel( QString::number( value ) );
	if ( scaleDraw != NULL ){
		QwtText tickText = scaleDraw->label( value );
		tickLabel = tickText.text();
	}
	QRect fontBoundingRect = QFontMetrics( font ).boundingRect( tickLabel );
	int letterWidth = fontBoundingRect.width();
	int letterHeight = fontBoundingRect.height();
	xPosition = static_cast<int>( xPixel - letterWidth / 2 );
	painter->drawText( xPosition, yEnd - tickLength - letterHeight, tickLabel );
}



void ExternalAxisWidgetTop::drawTicks( QPainter* painter, int tickLength ){

	//Figure out how far out to start drawing ticks.
	double startPixelX = getTickStartPixel(QwtPlot::xTop)-MARGIN;

	//We don't want to draw too many ticks so adjust the number
	//of ticks we draw accordingly.
	QwtScaleDiv* scaleDiv = plot->axisScaleDiv( QwtPlot::xBottom );
	const QList<double> axisTicks = scaleDiv->ticks(QwtPlot::xBottom);
	int originalTickCount = axisTicks.size();
	int tickIncrement = getTickIncrement( originalTickCount );

	//Now figure out the xIncrement, how far apart the ticks should be.
	double tickDistance = getTickDistance( QwtPlot::xBottom);
	double xIncrement = getTickIncrement( tickDistance, QwtPlot::xBottom );

	for ( int i = 0; i < originalTickCount; i = i + tickIncrement ){
		//Sometimes the automatic tick system puts uneven number of ticks in.
		//Definitely weird - but that is why the incrementCount;
		int incrementCount = qRound((axisTicks[i] - axisTicks[0]) / tickDistance);
		double tickPosition = startPixelX + incrementCount * xIncrement;
		drawTick( painter, tickPosition, axisTicks[i], tickLength);
	}
}

void ExternalAxisWidgetTop::drawAxisLabel( QPainter* painter ){
	 QFont font = painter->font();
	 QRect fontBoundingRect = QFontMetrics( font ).boundingRect( axisLabel );
	 int yPosition = height() - 2 * fontBoundingRect.height() - TICK_LENGTH - MARGIN;

	 int xPosition = width()/2 - fontBoundingRect.width() / 2;
	 painter->drawText( xPosition, yPosition, fontBoundingRect.width(), height(),
					  Qt::AlignHCenter|Qt::AlignTop, axisLabel);
}

ExternalAxisWidgetTop::~ExternalAxisWidgetTop() {
}

} /* namespace casa */
