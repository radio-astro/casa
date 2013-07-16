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
		plot( NULL ), AXIS_SMALL_SIDE(100), MARGIN(5),
		FONT_SIZE(8), FONT_SIZE_AXIS_LABEL(8){
}


void ExternalAxisWidget::setPlot( QwtPlot* plotOwner ){
	plot = plotOwner;
}

int ExternalAxisWidget::getStartY() const {
	return 0;
}

double ExternalAxisWidget::getTickStartPixel( QwtPlot::Axis axis ){
	//Figure out where to start the first tick.  There will be a small distance
	//between the first tick and the start of the axis do to the difference between
	//the upper bound and the first tick location.
	QwtScaleDiv* scaleDiv = plot->axisScaleDiv( axis );
	double upperBound = scaleDiv->upperBound();
	double lowerBound = scaleDiv->lowerBound();
	double axisExtent = upperBound - lowerBound;
	const QList<double> axisTicks = scaleDiv->ticks(axis);
	double endDistancePercentage = 0;
	if ( axisTicks.size() > 0  ){
		double lowerBoundDistance = qAbs(lowerBound - axisTicks[0]);
		endDistancePercentage = lowerBoundDistance / axisExtent;
	}

	int canvasBound = width();
	if ( axis != QwtPlot::xBottom ){
		canvasBound = height() - getStartY();
	}
	double startPixel = canvasBound * endDistancePercentage;
	if ( axis != QwtPlot::xBottom ){
		startPixel = getStartY() + startPixel;
	}
	return startPixel;
}

double ExternalAxisWidget::getTickDistance(QwtPlot::Axis axis ){
	QwtScaleDiv* scaleDiv = plot->axisScaleDiv( axis );
	const QList<double> axisTicks = scaleDiv->ticks( axis);
	double tickDistance = scaleDiv->upperBound() - scaleDiv->lowerBound();
	for ( int i = 0; i < axisTicks.size() - 1; i++ ){
		double tDistance = axisTicks[i+1] - axisTicks[i];
		if ( tDistance < tickDistance ){
			tickDistance = tDistance;
		}
	}
	return tickDistance;
}

double ExternalAxisWidget::getTickIncrement( double tickDistance, QwtPlot::Axis axis ){
	QwtScaleDiv* scaleDiv = plot->axisScaleDiv( axis );
	double axisExtent = scaleDiv->upperBound() - scaleDiv->lowerBound();
	double tickPercentage = tickDistance / axisExtent;

	int canvasLimit = width();
	if ( axis != QwtPlot::xBottom ){
		canvasLimit = height();
	}

	double xIncrement = canvasLimit * tickPercentage;
	return xIncrement;
}

int ExternalAxisWidget::getTickIncrement( int tickCount ) const {

	//Adjust the allowable number of ticks by how much space
	//we have available for them.
	int TICK_LIMIT = 10;
	int increment = 1;
	int dynamicCount = tickCount;
	while( dynamicCount > TICK_LIMIT ){
		if ( dynamicCount % 2 == 0 ){
			dynamicCount = dynamicCount / 2;
			increment = increment * 2;
		}
		else if ( dynamicCount % 3 == 0 ){
			dynamicCount = dynamicCount / 3;
			increment = increment * 3;
		}
		else if ( dynamicCount % 5 == 0 ){
			dynamicCount = dynamicCount / 5;
			increment = increment * 5;
		}
		else if ( dynamicCount % 7 == 0 ){
			dynamicCount = dynamicCount / 7;
			increment = increment * 7;
		}
		else {
			//At least make it into a multiple of 2.
			dynamicCount = dynamicCount + 1;
		}
	}
	return increment;
}

void ExternalAxisWidget::paintEvent( QPaintEvent* event ){
	QWidget::paintEvent( event );
	if ( plot != NULL ){
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
