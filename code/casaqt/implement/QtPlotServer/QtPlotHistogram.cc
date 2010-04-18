//# QtPlotHistogram.cc: QwtPlotItem specialization for histograms
//# Copyright (C) 2010
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
//# $Id: $

#include <qstring.h>
#include <qpainter.h>
#include <qwt_painter.h>
#include <qwt_scale_map.h>
// #include <casaqt/QtPlotServer/QtPlotHistogram.h>
#include "QtPlotHistogram.h"

QtPlotHistogram::QtPlotHistogram( const QString &label ) : reference_(0.0), QwtPlotItem(QwtText(label)) { }
QtPlotHistogram::QtPlotHistogram( const QwtText &label ) : reference_(0.0), QwtPlotItem(label) { }

void QtPlotHistogram::setData(const QwtIntervalData &data) {
    data_ = data;
    itemChanged();
}

QwtDoubleRect QtPlotHistogram::boundingRect() const {
    QwtDoubleRect rect = data_.boundingRect();

    if ( !rect.isValid() )
        return rect;

    if ( rect.bottom() < reference_ )
	rect.setBottom( reference_ );
    else if ( rect.top() > reference_ )
	rect.setTop( reference_ );
    return rect;
}

void QtPlotHistogram::setBaseline(double reference) {
    if ( reference_ != reference ) {
        reference_ = reference;
        itemChanged();
    }
}

void QtPlotHistogram::setColor(const QColor &color) {
    if ( color_ != color ) {
	color_ = color;
        itemChanged();
    }
}

void QtPlotHistogram::draw(QPainter *painter, const QwtScaleMap &xMap,  const QwtScaleMap &yMap, const QRect &) const {
    const QwtIntervalData &iData = data_;

    painter->setPen(QPen(color_));

    const int x0 = xMap.transform(baseline());
    const int y0 = yMap.transform(baseline());

    for ( int i = 0; i < (int)iData.size(); i++ ) {
 	const int y2 = yMap.transform(iData.value(i));
	if ( y2 == y0 )
	    continue;

	int x1 = xMap.transform(iData.interval(i).minValue());
	int x2 = xMap.transform(iData.interval(i).maxValue());
	if ( x1 > x2 ) qSwap(x1, x2);

	if ( i < (int)iData.size() - 2 ) {
	    const int xx1 = xMap.transform(iData.interval(i+1).minValue());
	    const int xx2 = xMap.transform(iData.interval(i+1).maxValue());

	    if ( x2 == qwtMin(xx1, xx2) ) {
		const int yy2 = yMap.transform(iData.value(i+1));
		if ( yy2 != y0 && ( (yy2 < y0 && y2 < y0) || (yy2 > y0 && y2 > y0) ) ) {
		    // One pixel distance between neighboured bars
		    x2--;
		}
	    }
	}
	draw_bar(painter, Qt::Vertical, QRect(x1, y0, x2 - x1, y2 - y0) );
    }
}

void QtPlotHistogram::draw_bar(QPainter *painter, Qt::Orientation, const QRect& rect) const {
    painter->save();

    const QColor color(painter->pen().color());
#if QT_VERSION >= 0x040000
    const QRect r = rect.normalized();
#else
    const QRect r = rect.normalize();
#endif

    const int factor = 125;
    const QColor light(color.light(factor));
    const QColor dark(color.dark(factor));

    painter->setBrush(color);
    painter->setPen(Qt::NoPen);
    QwtPainter::drawRect(painter, r.x() + 1, r.y() + 1, r.width() - 2, r.height() - 2);
    painter->setBrush(Qt::NoBrush);

    painter->setPen(QPen(light, 2));
#if QT_VERSION >= 0x040000
    QwtPainter::drawLine(painter, r.left() + 1, r.top() + 2, r.right() + 1, r.top() + 2);
#else
    QwtPainter::drawLine(painter, r.left(), r.top() + 2, r.right() + 1, r.top() + 2);
#endif

    painter->setPen(QPen(dark, 2));
#if QT_VERSION >= 0x040000
    QwtPainter::drawLine(painter, r.left() + 1, r.bottom(), r.right() + 1, r.bottom());
#else
    QwtPainter::drawLine(painter, r.left(), r.bottom(), r.right() + 1, r.bottom());
#endif

    painter->setPen(QPen(light, 1));

#if QT_VERSION >= 0x040000
    QwtPainter::drawLine(painter, r.left(), r.top() + 1, r.left(), r.bottom());
    QwtPainter::drawLine(painter, r.left() + 1, r.top() + 2, r.left() + 1, r.bottom() - 1);
#else
    QwtPainter::drawLine(painter, r.left(), r.top() + 1, r.left(), r.bottom() + 1);
    QwtPainter::drawLine(painter, r.left() + 1, r.top() + 2, r.left() + 1, r.bottom());
#endif

    painter->setPen(QPen(dark, 1));

#if QT_VERSION >= 0x040000
    QwtPainter::drawLine(painter, r.right() + 1, r.top() + 1, r.right() + 1, r.bottom());
    QwtPainter::drawLine(painter, r.right(), r.top() + 2, r.right(), r.bottom() - 1);
#else
    QwtPainter::drawLine(painter, r.right() + 1, r.top() + 1, r.right() + 1, r.bottom() + 1);
    QwtPainter::drawLine(painter, r.right(), r.top() + 2, r.right(), r.bottom());
#endif
    painter->restore();
}
