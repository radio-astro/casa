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
#include <qwt_plot.h>
#include <qwt_plot_canvas.h>
#include <qwt_scale_div.h>
#include <casaqt/QwtPlotter/QPCanvasHelpers.qo.h>

namespace casa {

ExternalAxisWidgetLeft::ExternalAxisWidgetLeft(QWidget* parent, QwtPlot* plot,
		bool leftAxisInternal, bool bottomAxisInternal,
		bool rightAxisInternal ):
	ExternalAxisWidgetVertical( parent, plot, leftAxisInternal, bottomAxisInternal,
			rightAxisInternal ){
	setSizePolicy( QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding );
	scaleDraw = new QPScaleDraw( NULL, QwtPlot::yLeft );
	setMinimumWidth( 2 * AXIS_SMALL_SIDE/3 );
	plotAxis = QwtPlot::yLeft;
}



void ExternalAxisWidgetLeft::defineAxis( QLine& line ){
	int x = width() - 2;
	int top = getStartY();
	int bottom = getEndY();

	QPoint firstPt( x, top );
	QPoint secondPt( x, bottom );
	line.setP1( firstPt );
	line.setP2( secondPt );
}

void ExternalAxisWidgetLeft::drawTick( QPainter* painter, double yPixel, double value,
		int tickLength ){

	//Draw the tick
	int xEnd = width();
	int tickStart = xEnd - tickLength;
	int yValue = static_cast<int>( yPixel );
	painter->drawLine( tickStart, yValue, xEnd, yValue  );

	//Draw the tick label
	QString numberStr = formatLabel( value );
	QFont font = painter->font();
	QRect fontBoundingRect = QFontMetrics( font ).boundingRect( numberStr );
	int labelStart = tickStart - fontBoundingRect.width() - 5;
	int letterHeight = fontBoundingRect.height();
	int position = static_cast<int>(yPixel + letterHeight/3);
	painter->drawText( labelStart, position, numberStr);
}


void ExternalAxisWidgetLeft::drawAxisLabel( QPainter* painter ){
	  QFont font = painter->font();
	  QString mainLabel = axisLabel.trimmed();
	  painter->rotate(-90);

	  //Draw the main label
	  QRect fontBoundingRect = QFontMetrics( font).boundingRect( mainLabel );
	  int yPosition = width() - fontBoundingRect.width() -
			  fontBoundingRect.height()- TICK_LENGTH - MARGIN;

	  int xPosition = -height() + (height() - fontBoundingRect.width())/2;
	  painter->drawText( xPosition, yPosition, fontBoundingRect.width(),
			  fontBoundingRect.height(), Qt::AlignHCenter|Qt::AlignTop, mainLabel);
	  painter->rotate(90);
}

ExternalAxisWidgetLeft::~ExternalAxisWidgetLeft() {
}

} /* namespace casa */
