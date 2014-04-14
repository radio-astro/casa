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
#include <QPainter>
#include <QDebug>
#include <QtCore/qmath.h>
#include <qwt_plot_canvas.h>
#include <qwt_scale_div.h>
#include <casaqt/QwtPlotter/QPCanvasHelpers.qo.h>

namespace casa {

ExternalAxisWidget::ExternalAxisWidget(QWidget* parent, QwtPlot* managedPlot,
		bool leftAxisInternal, bool bottomAxisInternal,
		bool rightAxisInternal ) :QWidget( parent ),
		plot( managedPlot ), scaleDraw(NULL),
		AXIS_SMALL_SIDE(100), TICK_LENGTH(5),
		MARGIN(5), MIN_START_Y(22){
	this->leftAxisInternal = leftAxisInternal;
	this->bottomAxisInternal = bottomAxisInternal;
	this->rightAxisInternal = rightAxisInternal;
}

void ExternalAxisWidget::setDateFormat(const String& newFormat){
	if ( scaleDraw != NULL ){
		scaleDraw->setDateFormat( newFormat );
	}
}



void ExternalAxisWidget::setRelativeDateFormat(const String& newFormat){
	if ( scaleDraw != NULL ){
		scaleDraw->setRelativeDateFormat( newFormat );
	}
}

void ExternalAxisWidget::setAxisScale(PlotAxisScale scale){
	if ( scaleDraw != NULL ){
		scaleDraw->setScale( scale );
	}
}

void ExternalAxisWidget::setReferenceValue(bool on, double value){
	if ( scaleDraw != NULL ){
			scaleDraw->setReferenceValue( on, value );
		}
}

int ExternalAxisWidget::getStartY() const {
	return MIN_START_Y;
}

int ExternalAxisWidget::getStartX() const {
	return 0;
}

int ExternalAxisWidget::getEndY() const {
	int canvasBound = getCanvasHeight();
	return canvasBound;
}

void ExternalAxisWidget::print( QPainter* painter, QRect imageRect ){
	QPixmap pm = QPixmap::grabWidget( this );
	if ( painter != NULL ){
		painter->drawPixmap(imageRect, pm );
	}
}

double ExternalAxisWidget::getTickStartPixel( QwtPlot::Axis axis ){
	//Figure out where to start the first tick.  There will be a small distance
	//between the first tick and the start of the axis due to the difference between
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

	//The length of the axis.
	int canvasBound = getAxisLength();

	double startPixel = canvasBound * endDistancePercentage;
	if ( axis != QwtPlot::xBottom && axis != QwtPlot::xTop ){
		startPixel = getStartY() + startPixel;
	}
	else {
		startPixel = getStartX() + startPixel;
	}
	return startPixel;
}

void ExternalAxisWidget::setAxisFont(const QFont& font){
	axisFont = font;
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

int ExternalAxisWidget::getCanvasHeight() const {
	int canvasHeight = height();
	QwtPlotCanvas* canvas = plot->canvas();
	if ( canvas != NULL ){
		canvasHeight = canvas->height();
	}
	return canvasHeight;
}

int ExternalAxisWidget::getCanvasWidth() const {
	int canvasWidth = width();
	QwtPlotCanvas* canvas = plot->canvas();
	if ( canvas != NULL ){
		canvasWidth = canvas->width();
	}
	return canvasWidth;
}

double ExternalAxisWidget::getTickIncrement( double tickDistance, QwtPlot::Axis axis ){
	QwtScaleDiv* scaleDiv = plot->axisScaleDiv( axis );
	double axisExtent = scaleDiv->upperBound() - scaleDiv->lowerBound();
	double tickPercentage = tickDistance / axisExtent;

	int canvasLimit = getCanvasWidth() - 2 * MARGIN;
	if ( axis != QwtPlot::xBottom && axis != QwtPlot::xTop ){
		canvasLimit = getCanvasHeight();
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
	QFont axisFont = plot->axisFont( plotAxis);
	painter->setFont( axisFont );
	drawTicks( painter, TICK_LENGTH  );
}

void ExternalAxisWidget::drawBackBone( QPainter* painter ){
	QLine line;
	defineAxis( line );
	painter->drawLine( line );
}

void ExternalAxisWidget::drawLabel( QPainter* painter ){
	  axisFont.setBold( true );
	  painter->setFont( axisFont );

	  drawAxisLabel( painter );
}



QString ExternalAxisWidget::formatLabel( double value ) const {
	QString numberStr = QString::number( value, 'g', 3 );
	if ( scaleDraw != NULL ){
		QwtText tickText = scaleDraw->label( value );
		numberStr = tickText.text();
	}
	return numberStr;
}

void ExternalAxisWidget::setAxisLabel( const QString& label ){
	axisLabel = label;
}

QString ExternalAxisWidget::getAxisLabel() const {
	return axisLabel;
}

ExternalAxisWidget::~ExternalAxisWidget() {

}

} /* namespace casa */
