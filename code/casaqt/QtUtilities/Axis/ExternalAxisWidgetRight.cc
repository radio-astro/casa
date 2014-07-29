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
#include <casaqt/QwtPlotter/QPCanvasHelpers.qo.h>

namespace casa {

ExternalAxisWidgetRight::ExternalAxisWidgetRight(QWidget* parent, QwtPlot* plot,
		bool leftAxisInternal, bool bottomAxisInternal,
		bool rightAxisInternal ) :
	ExternalAxisWidgetVertical( parent, plot, leftAxisInternal, bottomAxisInternal,
			rightAxisInternal ){
	setSizePolicy( QSizePolicy::Fixed, QSizePolicy::MinimumExpanding );
	setFixedWidth( AXIS_SMALL_SIDE);
	scaleDraw = new QPScaleDraw( NULL, QwtPlot::yRight );
	useLeftScale = false;
	plotAxis = QwtPlot::yRight;
}

void ExternalAxisWidgetRight::setUseLeftScale( bool b ){
	useLeftScale = b;
}



/*int ExternalAxisWidgetRight::getCanvasHeight() const {
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
}*/


void ExternalAxisWidgetRight::defineAxis( QLine& axisLine ){
	int x = 1;
	int top = getStartY();
	int bottom = getEndY();
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
	QString numberStr = formatLabel( value );
	QFont font = painter->font();
	QRect fontBoundingRect = QFontMetrics( font ).boundingRect( numberStr );
	int labelStart = xEnd + 6;
	int letterHeight = fontBoundingRect.height();
	yPosition = static_cast<int>( yPixel + letterHeight/3);
	painter->drawText( labelStart, yPosition, numberStr);
}


void ExternalAxisWidgetRight::drawAxisLabel( QPainter* painter ){
	 QFont font = painter->font();
	 QString mainLabel = axisLabel.trimmed();

	 painter->rotate(90);

	 QRect fontBoundingRect = QFontMetrics( font ).boundingRect( mainLabel );
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
