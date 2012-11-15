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

#include "ExternalAxisWidget.h"
#include <guitools/Feather/ExternalAxis.h>
#include <QPainter>
#include <QDebug>
#include <qwt_plot_canvas.h>

namespace casa {

ExternalAxisWidget::ExternalAxisWidget(QWidget* parent) :QWidget( parent ),
		canvas( NULL ), scaleDiv( NULL ),
		AXIS_SMALL_SIDE(100),
		FONT_SIZE(9), FONT_SIZE_AXIS_LABEL(10){
}

void ExternalAxisWidget::setScaleDiv( QwtScaleDiv* axisScaleDiv ){
	scaleDiv = axisScaleDiv;
}

void ExternalAxisWidget::setPlotCanvas( QwtPlotCanvas* plotCanvas ){
	canvas = plotCanvas;
}


int ExternalAxisWidget::getTickIncrement( int tickCount, bool horizontal ) const {

	//Adjust the allowable number of ticks by how much space
	//we have available for them.
	int TICK_LIMIT = 10;
	int spaceLimit = height();
	if ( horizontal ){
		spaceLimit = width();
	}

	const int SPACE_MINIMUM = 300;
	const int SPACE_INCREMENT = 200;
	const int TICK_INCREMENT = 5;
	int spaceAdjustment = (spaceLimit - SPACE_MINIMUM) / SPACE_INCREMENT;
	if ( spaceAdjustment > 0 ){
		TICK_LIMIT = TICK_LIMIT + TICK_INCREMENT * spaceAdjustment;
	}

	//Decrease our count until we are withen the allowable
	//limit
	int dynamicCount = tickCount;
	int increment = 1;
	const int DIVISOR = 2;
	while ( dynamicCount > TICK_LIMIT ){
		increment = increment * DIVISOR;
		dynamicCount = dynamicCount / DIVISOR;
	}
	return increment;
}

void ExternalAxisWidget::paintEvent( QPaintEvent* event ){
	QWidget::paintEvent( event );
	if ( scaleDiv != NULL && canvas != NULL ){
		QPainter painter( this );
		QPen pen( Qt::black );
		pen.setWidth( 2 );
		painter.setPen( pen );
		drawBackBone( &painter );
		drawTicks( &painter );
		drawLabel( &painter );
	}
}

void ExternalAxisWidget::drawTicks( QPainter* painter ){
	QFont font = painter->font();
	font.setPointSize( FONT_SIZE );
	painter->setFont( font );
	drawTicks( painter, 5  );
}

void ExternalAxisWidget::drawBackBone( QPainter* painter ){
	QLine line;
	defineAxis( line );
	painter->drawLine( line );
}

void ExternalAxisWidget::drawLabel( QPainter* painter ){
	  QFont font("Helvetica [Cronyx]", FONT_SIZE_AXIS_LABEL );
	  font.setBold( true );
	  painter->setFont( font );
	  drawAxisLabel( painter );
}



void ExternalAxisWidget::setAxisLabel( const QString& label ){
	axisLabel = label;
}

ExternalAxisWidget::~ExternalAxisWidget() {

}

} /* namespace casa */
