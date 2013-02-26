//# Copyright (C) 1994,1995,1996,1997,1998,1999,2000
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

#include "SliceSegment.qo.h"
#include <display/Slicer/SliceStatistics.h>
#include <QtCore/qmath.h>
#include <QDebug>
#include <qwt_plot.h>
#include <qwt_plot_curve.h>

namespace casa {

SliceSegment::SliceSegment(QWidget *parent)
    : QFrame(parent), plotCurve( NULL ){
	ui.setupUi(this);
	setAutoFillBackground( true );
	setColor( Qt::green );
}

void SliceSegment::addCurve( QwtPlot* plot,
		const QVector<double>& xValues, const QVector<double>& yValues ){
	plotCurve = new QwtPlotCurve();
	plotCurve->attach( plot );
	plotCurve->setData( xValues, yValues );
	setCurveColor();
}


void SliceSegment::clearCurve(){
	if ( plotCurve != NULL ){
		plotCurve->detach();
		delete plotCurve;
		plotCurve = NULL;
	}
}
void SliceSegment::setEndPointsPixel( int pixelX1, int pixelY1,
		int pixelX2, int pixelY2 ){
	pixelStart.first = pixelX1;
	pixelStart.second = pixelY1;
	pixelEnd.first = pixelX2;
	pixelEnd.second = pixelY2;
}

void SliceSegment::setEndPointsWorld( double worldX1, double worldY1,
		double worldX2, double worldY2 ){
	worldStart.first = worldX1;
	worldStart.second = worldY1;
	worldEnd.first = worldX2;
	worldEnd.second = worldY2;
}



void SliceSegment::updateStatistics(){
	SliceStatistics* statistics = SliceStatisticsFactory::getInstance()->getStatistics();
	double angle = statistics->getAngle( pixelStart, pixelEnd );
	double value = statistics->getLength( worldStart, worldEnd, pixelStart, pixelEnd );
	QString labelText = statistics->getLengthLabel();
	if ( value < 100000 ){
		ui.distanceLineEdit->setText( QString::number( value ));
	}
	else {
		ui.distanceLineEdit->setText( QString::number(value, 'e'));
	}
	ui.distanceLabel->setText( labelText );
	ui.angleLineEdit->setText( QString::number( angle ));
	delete statistics;
}


void SliceSegment::setCurveColor(){
	if ( plotCurve != NULL ){
		QPen curvePen = plotCurve->pen();
		curvePen.setColor( defaultColor );
		plotCurve->setPen( curvePen );
	}
}

void SliceSegment::setColor( QColor color ){
	int red;
	int green;
	int blue;
	color.getRgb(&red,&green,&blue);
	QString objName = objectName();
	QString borderStyle("QFrame#");
	borderStyle.append( objName);
	borderStyle.append("{ border: 5px solid rgb(");
	borderStyle.append(QString::number(red)+",");
	borderStyle.append(QString::number(green)+",");
	borderStyle.append(QString::number(blue)+") }");
	setStyleSheet( borderStyle );

	defaultColor = color;
	setCurveColor();
}

QColor SliceSegment::getCurveColor() const {
	QColor curveColor;
	if ( plotCurve != NULL ){
		QPen curvePen = plotCurve->pen();
		curveColor = curvePen.color();
	}
	return curveColor;
}

QwtPlot* SliceSegment::getPlot(){
	QwtPlot* curvePlot = NULL;
	if ( plotCurve != NULL ){
		curvePlot = plotCurve->plot();
	}
	return curvePlot;
}

SliceSegment::~SliceSegment(){
	clearCurve();
}
}
