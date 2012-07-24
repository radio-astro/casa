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

#include "MolecularLine.h"
#include <QDebug>
#include <QPainter>
#include <QTextDocument>
#include <qwt_scale_map.h>
#include <qwt_double_rect.h>

namespace casa {

MolecularLine::MolecularLine() : QwtPlotMarker(),
		lineColor("#00957B"){
	setLineStyle( QwtPlotMarker::HLine);
}

int MolecularLine::rtti() const {
	return QwtPlotItem::Rtti_PlotUserItem;
}

void MolecularLine::setCenter( float center ){
	this->center = center;
	setXValue( center );
}

float MolecularLine::getCenter( ) const {
	return center;
}


void MolecularLine::setPeak( float peak ){
	this->peak = peak;
}

float MolecularLine::getPeak() const {
	return peak;
}

void MolecularLine::setLabel( const QString& label ){
	this->label = label;
}

QString MolecularLine::getLabel() const {
	return label;
}

void MolecularLine::draw (QPainter * painter, const QwtScaleMap & xMap,
	const QwtScaleMap & yMap, const QRect & canvasRect ) const {

	//Draw a line
	int centerPixel = static_cast<int>(xMap.transform( center ));
	int peakPixel = static_cast<int>(yMap.transform( peak ));
	int zeroPixel = static_cast<int>(yMap.transform( 0 ));
	painter->save();
	QPen pen( lineColor );
	pen.setWidth(2);
	painter->setPen( pen );
	painter->drawLine(centerPixel, zeroPixel, centerPixel, peakPixel );

	//Label the line
	QFont font("Helvetica [Cronyx]", 9);
	font.setBold( true );
	int minY = qMin( peakPixel,zeroPixel);
	painter->translate( centerPixel + 3, minY );
	painter->setFont( font );
	painter->rotate(90);

	//Have to use HTML because of
	//some of the molecular formula symbols
	QTextDocument document;
	QString coloredFormula = "<font color='"+ lineColor.name()+"'>"+label+"</font>";
	document.setHtml( coloredFormula );
	document.drawContents(painter, QRect(0, 0, canvasRect.height(), canvasRect.width()));
	painter->restore();
}



MolecularLine::~MolecularLine() {
}

} /* namespace casa */
