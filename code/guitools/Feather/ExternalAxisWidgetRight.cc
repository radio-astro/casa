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

#include "ExternalAxisWidgetRight.h"
#include <QDebug>
#include <QPainter>
#include <qwt_plot.h>
#include <qwt_plot_canvas.h>
#include <qwt_scale_div.h>
#include <qwt_text_label.h>

namespace casa {

ExternalAxisWidgetRight::ExternalAxisWidgetRight(QWidget* parent ) :
	ExternalAxisWidget( parent ){
	setSizePolicy( QSizePolicy::Fixed, QSizePolicy::MinimumExpanding );
	setFixedWidth( AXIS_SMALL_SIDE);
	useLeftScale = false;
}

void ExternalAxisWidgetRight::setUseLeftScale( bool b ){
	useLeftScale = b;
}

int ExternalAxisWidgetRight::getStartY() const {
	int plotHeight = height();
	int canvasHeight = getCanvasHeight();
	int heightDiff = plotHeight - canvasHeight;
	if ( heightDiff < MIN_START_Y ){
		heightDiff = MIN_START_Y;
	}
	return heightDiff;
}

int ExternalAxisWidgetRight::getCanvasHeight() const {
	//We should use canvas->height()
	//But because the main canvas is not laid out yet when this
	//gets drawn, canvas->height gives a bogus value.  Thus, we
	//are using plot->height() - (height of the plot title plus margins) instead.

	int plotHeight = height();
	QwtTextLabel* titleText = plot->titleLabel();
	QRect titleRect = titleText->textRect();
	int titleHeight =  titleRect.height() + 7;
	int canvasHeight = plotHeight - titleHeight;
	return canvasHeight;
}

void ExternalAxisWidgetRight::defineAxis( QLine& axisLine ){
	const int MARGIN = 1;
	int x = MARGIN;
	int top = getStartY();
	int bottom = height() - 2 * MARGIN;
	QPoint firstPt( x, top );
	QPoint secondPt( x, bottom );
	axisLine.setP1( firstPt );
	axisLine.setP2( secondPt );
}

void ExternalAxisWidgetRight::drawTick( QPainter* painter, double yPixel, double value,
		int tickLength){

	//Draw the tick
	int xStart = 1;
	int xEnd = xStart + tickLength;
	int yPosition = static_cast<int>(yPixel);
	painter->drawLine( xStart, yPosition, xEnd, yPosition );

	//Draw the tick label
	QString numberStr = QString::number( value );
	QFont font = painter->font();
	QRect fontBoundingRect = QFontMetrics(font).boundingRect( numberStr );
	int labelStart = xEnd + 6;
	int letterHeight = fontBoundingRect.height();
	yPosition = static_cast<int>( yPixel + letterHeight/3);
	painter->drawText( labelStart, yPosition, numberStr);
}



void ExternalAxisWidgetRight::drawTicks( QPainter* painter, int tickLength ){

	QwtPlot::Axis axisScale = QwtPlot::yRight;
	if ( useLeftScale ){
		axisScale = QwtPlot::yLeft;
	}

	//Figure out how far out to start drawing ticks.
	double startPixelY = getTickStartPixel(QwtPlot::yRight);

	//We don't want to draw too many ticks so adjust the number
	//of ticks we draw accordingly.
	QwtScaleDiv* scaleDiv = plot->axisScaleDiv( axisScale );
	const QList<double> axisTicks = scaleDiv->ticks( axisScale );
	int originalTickCount = axisTicks.size();
	int tickIncrement = getTickIncrement( originalTickCount );

	//Now figure out the yIncrement, how far apart the ticks should be.
	double tickDistance = getTickDistance( axisScale );
	double yIncrement = getTickIncrement( tickDistance, axisScale );

	for ( int i = 0; i < originalTickCount; i = i + tickIncrement ){
		//Sometimes the automatic tick system puts uneven number of ticks in.
		//Definitely weird - but that is why the incrementCount;
		int incrementCount = qRound((axisTicks[i] - axisTicks[0]) / tickDistance);
		double tickPosition = startPixelY +incrementCount * yIncrement;
		int tickIndex = originalTickCount - i - 1;
		drawTick( painter, tickPosition, axisTicks[tickIndex], tickLength);
	}
}

void ExternalAxisWidgetRight::drawAxisLabel( QPainter* painter ){
	 QFont font = painter->font();
	 QString mainLabel = axisLabel.trimmed();

	 painter->rotate(90);

	 QRect fontBoundingRect = QFontMetrics(font).boundingRect( mainLabel );
	 int startY = -2 * width() / 3;
	 int yPosition = startY - fontBoundingRect.height();
	 int xPosition = (height() - fontBoundingRect.width())/2;
	 painter->drawText( xPosition, yPosition, fontBoundingRect.width(), fontBoundingRect.height(),
				  Qt::AlignHCenter|Qt::AlignTop, mainLabel);

	 painter->rotate(-90);
}


ExternalAxisWidgetRight::~ExternalAxisWidgetRight() {

}

} /* namespace casa */
